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
  events_.clear();
  callback_ = tool_->addCallback([this](const CTFEvent& event)
  {
    events_.push_back(event);
  });
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

  provider_->updateMapping();
  std::vector<json> result = convertEvents();

  auto mapping = provider_->getMapping();

  // Iterate over all mappings by process ID.
  for (const auto pid_process_info : mapping)
  {
    // make a metadata entry to name a process.
    json process_entry;
    process_entry["tid"] = 0;
    process_entry["ph"] = "M";
    process_entry["name"] = "process_name";
    process_entry["args"] = { { "name", pid_process_info.second.info.name } };
    process_entry["pid"] = pid_process_info.first;
    result.push_back(process_entry);

    // For all thread mappings, make a metadata entry to name the thread.
    for (const auto thread_mapping : pid_process_info.second.info.threads)
    {
      json tid_entry;
      tid_entry["tid"] = thread_mapping.first;
      tid_entry["ph"] = "M";
      tid_entry["name"] = "thread_name";
      tid_entry["pid"] = pid_process_info.first;
      tid_entry["args"] = { { "name", thread_mapping.second } };
      result.push_back(tid_entry);
    }
  }

  return result;
}

std::vector<json> LttngSource::convertEvents()
{
  if (events_.empty())
  {
    return {};
  }
  
  std::vector<json> result;
  result.reserve(events_.size());

  double start_time = events_.front().time();

  for (auto& event : events_)
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
      unsigned long long id = 0;
      if (event.eventData().find("id") != event.eventData().end())
      {
        id = event.eventData().at("id");
      }

      // Populate the name
      entry["name"] = provider_->getScopeName(event.pid(), id);

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

  // Return converted entries
  return result;
}

}  // namespace scalopus
