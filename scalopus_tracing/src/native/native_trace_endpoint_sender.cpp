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
#include <scalopus_tracing/native_trace_endpoint_sender.h>
#include "tracepoint_collector_native.h"
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>

namespace scalopus
{
using json = nlohmann::json;
using EventMap = std::map<unsigned long, tracepoint_collector_types::EventContainer>;

NativeTraceEndpointSender::NativeTraceEndpointSender()
{
  // Start the worker thread.
  worker_ = std::thread([&]()
  {
    work();
  });
}

NativeTraceEndpointSender::~NativeTraceEndpointSender()
{
  // Shut down the worker thread and join it.
  running_ = false;
  worker_.join();
}

/**
 * @brief This function serializes the thread event map into a bson message that can be broadcast.
 */
static Data process_events(const EventMap& tid_event_map)
{
  json events = json({});
  // Need to know the PID when we consume these traces.
  events["pid"] = static_cast<unsigned long>(::getpid());

  // Then, we construct a list of native types that we can serialize.
  tracepoint_collector_types::ThreadedEvents event_list;

  for (const auto& tid_event : tid_event_map)
  {
    const auto& thread_id = tid_event.first;
    event_list[thread_id].reserve(tid_event.second.size());
    for (const auto& event : tid_event.second)
    {
      const auto& time = std::get<0>(event);
      const auto& trace_id = std::get<1>(event);
      const auto& trace_type = std::get<2>(event);

      // Calculate the timestamp since the unix epoch in nanoseconds.
      auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(time);
      auto epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(now_ns.time_since_epoch());

      event_list[thread_id].emplace_back(epoch.count(), trace_id, trace_type);
    }
  }
  // Finally, add the now consumable events to the json object
  events["events"] = event_list;
  return json::to_bson(events);
}

void NativeTraceEndpointSender::work()
{
  // The collector is a singleton, just retrieve it once.
  const auto& collector = TracePointCollectorNative::getInstance();
  while (running_)
  {
    auto tid_buffers = collector.getMap();
    std::size_t collected{ 0 };
    EventMap events;
    for (const auto& tid_buffer : tid_buffers)
    {
      // collect all events...
      auto& thread_id = tid_buffer.first;
      auto& buffer = tid_buffer.second;
      tracepoint_collector_types::ScopeTraceEvent event;

      // Collect all samples from this buffer.
      // @TODO This is some low-hanging fruit, this call uses an atomic for each event, we can do better.
      while (buffer->pop(event))
      {
        events[thread_id].push_back(event);
        collected++;
      }
    }
    if (collected)
    {
      auto transport = transport_.lock();
      if (transport)
      {
        transport->broadcast("native_tracepoint_receiver", process_events(events));
      }
    }
    // @TODO; do some real rate limiting here.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

std::string NativeTraceEndpointSender::getName() const
{
  return name;
}
}  // namespace scalopus
