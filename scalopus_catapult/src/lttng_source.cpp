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

  start_time_ = 0;
  provider_->updateMapping();

  callback_ = tool_->addCallback([this](const CTFEvent& event)
  {
    std::lock_guard<decltype(event_mutex_)> lock(event_mutex_);
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

std::vector<json> LttngSource::sendableEvents()
{
  std::lock_guard<decltype(event_mutex_)> lock(event_mutex_);
  // consume events as far as possible.
  bool halt_on_fail = true;
  std::vector<json> result = convertEvents(halt_on_fail);
  if (halt_on_fail)
  {
    // we failed resolving a mapping...
    update_mapping_needed_ = true;
  }

  return result;
}

void LttngSource::work()
{
  if (update_mapping_needed_)
  {
    provider_->updateMapping();
  }
}

std::vector<json> LttngSource::finishInterval()
{
  stopInterval();
  std::lock_guard<decltype(event_mutex_)> lock(event_mutex_);

  provider_->updateMapping();

  bool halt_on_fail = false;
  std::vector<json> result = convertEvents(halt_on_fail);

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

  events_.clear();

  return result;
}

std::vector<json> LttngSource::convertEvents(bool& halt_on_fail)
{
  std::vector<json> result;
  bool had_resolve_fail = false;
  auto it = events_.begin();
  for (; it != events_.end(); it++)
  {
    auto& event = *it;

    if (start_time_ == 0)
    {
      start_time_ = event.time();
    }
    // Time stamp, relative to start.
    double ts = event.time() - start_time_;

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

      if ((!provider_->getScopeName(event.pid(), id, name)) && (halt_on_fail))
      {
        // erase everything that has been procesed.
        break;
        had_resolve_fail = true;
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
  // Remove all events that are processed.
  events_.erase(events_.begin(), it);

  // Set the resolve failure return bool
  halt_on_fail = had_resolve_fail;

  // Return converted entries
  return result;
}

}  // namespace scalopus
