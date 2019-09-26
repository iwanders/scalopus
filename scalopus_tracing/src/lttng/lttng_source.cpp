/*
  Copyright (c) 2018-2019, Ivor Wanders
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the name of the author nor the names of contributors may be used to
    endorse or promote products derived from this software without specific
    prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "scalopus_tracing/lttng_source.h"

#include <sstream>

namespace scalopus
{
LttngSource::LttngSource(BabeltraceTool::Ptr tool, LttngProvider::Ptr provider) : tool_(tool), provider_(provider)
{
}

LttngSource::~LttngSource()
{
  stopInterval();
}

void LttngSource::startInterval()
{
  stopInterval();
  events_.clear();
  callback_ = tool_->addCallback([this](const CTFEvent& event) { events_.push_back(event); });
}

void LttngSource::stopInterval()
{
  if (callback_ != nullptr)
  {
    callback_->disable();
    callback_.reset();
  }
}

void LttngSource::work()
{
}

std::vector<json> LttngSource::finishInterval()
{
  stopInterval();
  return convertEvents();
}

std::vector<json> LttngSource::convertEvents()
{
  if (events_.empty())
  {
    return {};
  }

  std::vector<json> result;
  result.reserve(events_.size());

  // Map for the counter states.
  using SeriesMap = std::map<std::string, std::int64_t>;
  using CounterMap = std::map<std::string, SeriesMap>;
  CounterMap counter_values;

  provider_->updateMapping();
  const auto mapping = provider_->getMapping();
  for (auto& event : events_)
  {
    // Time stamp, relative to start.
    double ts = event.time();

    if (event.domain() == "scalopus_scope_id")
    {
      // entry scope:
      // , {"name": "function name", "pid": 5976, "ts": 77033.2, "cat": "PERF", "tid": 140501248366336, "ph": "B"}
      // exit of scope:
      // , {"name": "function name", "pid": 5976, "ts": 77118.0, "cat": "PERF", "tid": 140501248366336, "ph": "E"}

      json entry;
      entry["ts"] = ts * 1e6;  // Time is specified in microseconds in devtools tracing format.
      entry["cat"] = "PERF";
      entry["tid"] = event.tid();
      entry["pid"] = event.pid();

      // try to look up the mapping for this pid
      std::uint32_t id = 0;
      if (event.eventData().find("id") != event.eventData().end())
      {
        id = static_cast<std::uint32_t>(event.eventData().at("id"));
      }

      // Populate the name
      const auto trace_id_string = provider_->getScopeName(mapping, static_cast<int>(event.pid()), id);
      entry["name"] = trace_id_string;  // Overwritten for counters later on.

      if (event.name() == "scope_entry")
      {
        entry["ph"] = "B";
      }
      else if (event.name() == "scope_exit")
      {
        entry["ph"] = "E";
      }
      else if (event.name() == "mark_event_global")
      {
        entry["ph"] = "i";
        entry["s"] = "g";
      }
      else if (event.name() == "mark_event_process")
      {
        entry["ph"] = "i";
        entry["s"] = "p";
      }
      else if (event.name() == "mark_event_thread")
      {
        entry["ph"] = "i";
        entry["s"] = "t";
      }
      else if (event.name() == "count_event")
      {
        entry["ph"] = "C";
        const auto counter_series = LttngProvider::splitCounterSeriesName(trace_id_string);
        entry["name"] = counter_series.first;
        const std::int64_t z = static_cast<std::int64_t>(event.eventData().at("value"));
        // Update the current counters.
        counter_values[counter_series.first][counter_series.second] = z;
        entry["args"] = counter_values[counter_series.first];
      }
      result.push_back(entry);
    }
  }

  // Sort res by "ts" because sometimes the "E"nd events are out of order wrt the "B"egin, leading to unfinished scope
  // events.
  std::stable_sort(result.begin(), result.end(), [](const json& lhs, const json& rhs) {
    return lhs.at("ts").get<double>() < rhs.at("ts").get<double>();
  });

  // Need a reverse iteration here, to populate all counters with all series seen in the entire interval.
  CounterMap count_all_series;
  for (auto it = result.rbegin(); it < result.rend(); it++)
  {
    auto& entry = *it;
    if (entry.at("ph").get<std::string>() == "C")
    {
      auto values = entry.at("args").get<SeriesMap>();
      const auto& name = entry.at("name").get<std::string>();
      values.insert(count_all_series[name].begin(), count_all_series[name].end());  // add future keys to this entry
      entry["args"] = values;           // update values to include the series used in the future.
      count_all_series[name] = values;  // store most recent value in the map.
    }
  }

  // Return converted entries
  return result;
}

}  // namespace scalopus
