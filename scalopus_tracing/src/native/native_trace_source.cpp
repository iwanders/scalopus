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
#include "scalopus_tracing/native_trace_source.h"
#include <cbor/stl.h>
#include <sstream>
#include "tracepoint_collector_native.h"

namespace scalopus
{
NativeTraceSource::NativeTraceSource(NativeTraceProvider::WeakPtr provider) : provider_(provider)
{
}

NativeTraceSource::~NativeTraceSource()
{
  stopInterval();
}

void NativeTraceSource::startInterval()
{
  stopInterval();
  {
    std::lock_guard<decltype(data_mutex_)> lock(data_mutex_);
    recorded_data_.clear();
  }
  in_interval_.store(true);
}

void NativeTraceSource::stopInterval()
{
  in_interval_.store(false);
}

void NativeTraceSource::work()
{
}

bool NativeTraceSource::isRecording() const
{
  return in_interval_.load();
}

std::vector<json> NativeTraceSource::finishInterval()
{
  std::vector<json> res;

  stopInterval();

  // Update mappings.
  auto provider = provider_.lock();
  if (provider != nullptr)
  {
    provider->updateMapping();
  }
  const auto mapping = provider->getMapping();

  // Obtain the data chunks.
  std::vector<DataPtr> data;
  {
    std::lock_guard<decltype(data_mutex_)> lock(data_mutex_);
    recorded_data_.swap(data);
  }

  // Map for the counter states.
  using SeriesMap = std::map<std::string, std::int64_t>;
  using CounterMap = std::map<std::string, SeriesMap>;
  using ProcessCounter = std::map<int, CounterMap>;
  ProcessCounter counter_values;

  // Now, we start converting the chunks of data we obtain into trace events.
  for (const auto& dptr : data)
  {
    // First, we parse the bson we got and convert it to events.
    std::map<std::string, cbor::cbor_object> parsed;
    cbor::from_cbor(parsed, *dptr);
    const int pid = static_cast<int>(parsed.at("pid").get<unsigned long>());
    tracepoint_collector_types::ThreadedEvents events;
    parsed.at("events").get_to(events);

    for (const auto& thread_events : events)
    {
      // Events are grouped by thread id.
      const auto& tid = thread_events.first;
      for (const auto& event : thread_events.second)
      {
        const auto& timestamp_ns_since_epoch = event.time_point;
        const auto& trace_id = event.trace_id;
        const auto& type = event.trace_type;

        // Finally, we can create a trace type that can be used by devtools.
        json entry;
        entry["ts"] = static_cast<double>(timestamp_ns_since_epoch) / 1e3;
        entry["tid"] = tid;
        entry["pid"] = pid;
        entry["cat"] = "PERF";
        const auto trace_id_string = provider->getScopeName(mapping, pid, trace_id);
        entry["name"] = trace_id_string;  // Overwritten for counters later on.
        if (type == TracePointCollectorNative::SCOPE_ENTRY)
        {
          entry["ph"] = "B";
        }
        else if (type == TracePointCollectorNative::SCOPE_EXIT)
        {
          entry["ph"] = "E";
        }
        else if (type == TracePointCollectorNative::MARK_GLOBAL)
        {
          entry["ph"] = "i";
          entry["s"] = "g";
        }
        else if (type == TracePointCollectorNative::MARK_PROCESS)
        {
          entry["ph"] = "i";
          entry["s"] = "p";
        }
        else if (type == TracePointCollectorNative::MARK_THREAD)
        {
          entry["ph"] = "i";
          entry["s"] = "t";
        }
        else if (type == TracePointCollectorNative::COUNTER)
        {
          entry["ph"] = "C";
          const auto counter_series = NativeTraceProvider::splitCounterSeriesName(trace_id_string);
          entry["name"] = counter_series.first;
          std::int64_t z;
          cbor::from_cbor(z, *event.dynamic_data);
          // Update the current counters.
          counter_values[pid][counter_series.first][counter_series.second] = z;
          entry["args"] = counter_values[pid][counter_series.first];
        }
        res.push_back(entry);
      }
    }
  }

  // Sort res by "ts" because sometimes the "E"nd events are out of order wrt the "B"egin, leading to unfinished scope
  // events.
  std::stable_sort(res.begin(), res.end(), [](const json& lhs, const json& rhs) {
    return lhs.at("ts").get<double>() < rhs.at("ts").get<double>();
  });

  // Need a reverse iteration here, to populate all counters with all series seen in the entire interval.
  ProcessCounter counter_all_series;
  for (auto it = res.rbegin(); it < res.rend(); it++)
  {
    auto& entry = *it;
    if (entry.at("ph").get<std::string>() == "C")
    {
      auto values = entry.at("args").get<SeriesMap>();
      auto pid = entry.at("pid").get<int>();
      const auto& name = entry.at("name").get<std::string>();
      values.insert(counter_all_series[pid][name].begin(),
                    counter_all_series[pid][name].end());  // add future keys to this entry
      entry["args"] = values;                              // update values to include the series used in the future.
      counter_all_series[pid][name] = values;              // store most recent value in the map.
    }
  }

  return res;
}

void NativeTraceSource::addData(const DataPtr& incoming_data)
{
  std::lock_guard<decltype(data_mutex_)> lock(data_mutex_);
  recorded_data_.emplace_back(incoming_data);
}
}  // namespace scalopus
