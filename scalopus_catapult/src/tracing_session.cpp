/*
  Copyright (c) 2018, Ivor Wanders
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

#include "scalopus_catapult/tracing_session.h"
#include <sstream>
#include "scalopus_lttng/ctfevent.h"
#include "scalopus_lttng/endpoint_scope_tracing.h"
#include "scalopus_transport/transport_unix.h"

namespace scalopus
{
TracingSession::TracingSession(BabeltraceTool::Ptr tool, EndpointManager::Ptr manager)
{
  tool_ = tool;
  manager_ = manager;
}

void TracingSession::start()
{
  stop();
  callback_ = tool_->addCallback([this](const CTFEvent& event) { events_.push_back(event); });
}

void TracingSession::stop()
{
  if (callback_ != nullptr)
  {
    callback_->disable();
    callback_.reset();
  }
}

std::vector<json> TracingSession::events()
{
  stop();
  updateMapping();

  std::vector<json> result;

  if (events_.empty())
  {
    return result;
  }

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
      auto pid_info = process_info_.find(event.pid());
      if (pid_info != process_info_.end())
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

std::vector<json> TracingSession::metadata()
{
  std::vector<json> result;
  // Iterate over all mappings by process ID.
  for (const auto pid_process_info : process_info_)
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

void TracingSession::updateMapping()
{
  auto endpoints = manager_->endpoints();
  for (const auto& transport_endpoints : endpoints)
  {
    ProcessMapping remote_info;
    // try to find the EndpointProcessInfo
    {
      auto it = transport_endpoints.second.find(scalopus::EndpointProcessInfo::name);
      if (it != transport_endpoints.second.end())
      {
        const auto endpoint_instance = std::dynamic_pointer_cast<scalopus::EndpointProcessInfo>(it->second);
        if (endpoint_instance != nullptr)
        {
          remote_info.info = endpoint_instance->processInfo();
        }
        else
        {
          std::cerr << "[scalopus] Pointer cast did not result in correct pointer, this should not happen."
                    << std::endl;
        }
      }
    }

    // Try to find the scope tracing endpoint and obtain its data.
    {
      auto it = transport_endpoints.second.find(scalopus::EndpointScopeTracing::name);
      if (it != transport_endpoints.second.end())
      {
        // found a trace mapping endpoint.
        const auto endpoint_scope_tracing = std::dynamic_pointer_cast<scalopus::EndpointScopeTracing>(it->second);
        if (endpoint_scope_tracing != nullptr)
        {
          remote_info.trace_ids = endpoint_scope_tracing->mapping();
        }
        else
        {
          std::cerr << "[scalopus] Pointer cast did not result in correct pointer, this should not happen."
                    << std::endl;
        }
      }
    }
    process_info_[remote_info.info.id] = remote_info;
  }
}
}  // namespace scalopus
