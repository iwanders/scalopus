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
#include "scalopus_tracing/endpoint_native_trace_sender.h"
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>
#include "tracepoint_collector_native.h"
#include "cbor.h"

namespace scalopus
{
using json = nlohmann::json;
using EventMap = std::map<unsigned long, tracepoint_collector_types::EventContainer>;

EndpointNativeTraceSender::EndpointNativeTraceSender()
{
  // Start the worker thread.
  worker_ = std::thread([&]() { work(); });
}

EndpointNativeTraceSender::~EndpointNativeTraceSender()
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
  std::map<std::string, cbor::cbor_object> my_data;
  my_data["pid"] = cbor::cbor_object::make(static_cast<unsigned long>(::getpid()));

  std::vector<std::tuple<EventMap::key_type, EventMap::mapped_type>> event_entries;
  for (const auto& tid_events : tid_event_map)
  {
    event_entries.emplace_back(tid_events.first, tid_events.second);
  }
  my_data["events"] = cbor::cbor_object::make(event_entries);
  Data output;
  cbor::serialize(my_data, output);
  return output;
}

void EndpointNativeTraceSender::work()
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

      // Collect all samples from this buffer.
      const auto available = buffer->size();
      auto& output_buffer = events[thread_id];
      output_buffer.reserve(output_buffer.size() + available);
      collected += buffer->pop_into(std::back_inserter(output_buffer), available);
    }
    if (collected)
    {
      if (transport_ != nullptr)
      {
        transport_->broadcast("native_trace_receiver", process_events(events));
      }
    }
    // @TODO; do some real rate limiting here.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

std::string EndpointNativeTraceSender::getName() const
{
  return name;
}
}  // namespace scalopus
