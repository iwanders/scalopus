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
#include "scalopus_lttng/ctfevent.h"
#include "scalopus_lttng/endpoint_scope_tracing.h"
#include "scalopus_transport/transport_unix.h"
#include <sstream>

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
  callback_ = tool_->addCallback([this](const CTFEvent& event)
  {
    events_.push_back(event);
  });
}

void TracingSession::stop()
{
  if (callback_ != nullptr)
  {
    callback_->disable();
    callback_.reset();
  }
}


std::vector<Json> TracingSession::events()
{
  stop();
  updateMapping();

  std::vector<Json> result;

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

      Json entry;
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
      auto entry_mapping = trace_mapping_.find(id);
      if (entry_mapping != trace_mapping_.end())
      {
        // yay! We found the appopriate mapping for this trace id.
        name = entry_mapping->second;
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

std::vector<Json> TracingSession::metadata()
{
  std::vector<Json> result;
  // Iterate over all mappings by process ID.
  //  for (const auto pid_mapping : mappings_)
  //  {

    // make a metadata entry to name a process.
    Json process_entry;
    process_entry["tid"] = 0;
    process_entry["ph"] = "M";
    process_entry["name"] = "process_name";
    process_entry["args"] = { { "name", "processName" } };
    process_entry["pid"] = 0;
    result.push_back(process_entry);

    // For all thread mappings, make a metadata entry to name the thread.
    //  for (const auto thread_mapping : mapping.thread_name_mapping)
    //  {
      Json tid_entry;
      tid_entry["tid"] = 0;
      tid_entry["ph"] = "M";
      tid_entry["name"] = "thread_name";
      tid_entry["pid"] = 0;
      tid_entry["args"] = { { "name", "threadName" } };
      result.push_back(tid_entry);
    //  }
  //  }
  return result;
}

void TracingSession::updateMapping()
{
  auto endpoints = manager_->endpoints();
  for (const auto& transport_endpoints: endpoints)
  {
    auto it = transport_endpoints.second.find(scalopus::EndpointScopeTracing::name);
    if (it != transport_endpoints.second.end())
    {
      // found a trace mapping endpoint.
      const auto endpoint_scope_tracing = std::dynamic_pointer_cast<scalopus::EndpointScopeTracing>(it->second);
      if (endpoint_scope_tracing != nullptr)
      {
        const auto mapping = endpoint_scope_tracing->mapping();
        trace_mapping_.insert(mapping.begin(), mapping.end());
      }
      else
      {
        std::cerr << "[scalopus] Pointer cast did not result in correct pointer, this should not happen." << std::endl;
      }
    }
  }
}
}  // namespace scalopus
