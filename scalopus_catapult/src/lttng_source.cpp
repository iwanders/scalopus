/*
  Copyright (c) 2019, Ivor Wanders
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

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

#include "scalopus_catapult/lttng_source.h"

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

std::vector<json> LttngSource::sendableEvents()
{
  return {};
}

std::vector<json> LttngSource::finishInterval()
{
  stopInterval();

  // process events.


  std::vector<json> result;

  if (events_.empty())
  {
    return result;
  }

  provider_->updateMapping();
  auto mapping = provider_->getMapping();

  double start_time = events_.front().time();
  for (const auto& event : events_)
  {
    // Time stamp, relative to start.
    double ts = event.time() - start_time;

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
      std::string name;  // scope name.
      unsigned long long id = 0;
      if (event.eventData().find("id") != event.eventData().end())
      {
        id = event.eventData().at("id");
      }
      // try to retrieve the correct mapping for this trace point id.
      auto pid_info = mapping.find(event.pid());
      if (pid_info != mapping.end())
      {
        auto entry_mapping = pid_info->second.trace_ids.find(id);
        if (entry_mapping != pid_info->second.trace_ids.end())
        {
          // yay! We found the appopriate mapping for this trace id.
          name = entry_mapping->second;
        }
      }
      if (name.empty())
      {
        std::stringstream z;
        z << "Unknown trace id." << std::hex << id;
        name = z.str();
      }
      entry["name"] = name;  // assign the name.

      if (event.name() == "entry")
      {
        entry["ph"] = "B";
      }
      else if (event.name() == "exit")
      {
        entry["ph"] = "E";
      }
      result.push_back(entry);
    }
  }
  events_.clear();


  return result;
}

}  // namespace scalopus
