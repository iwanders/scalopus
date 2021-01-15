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
#ifndef SCALOPUS_TRACING_TRACEPOINT_COLLECTOR_NATIVE_H
#define SCALOPUS_TRACING_TRACEPOINT_COLLECTOR_NATIVE_H

#include <cbor/stl.h>
#include <scalopus_general/map_tracker.h>
#include <scalopus_interface/types.h>
#include <chrono>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include "spsc_ringbuffer.h"
namespace scalopus
{
namespace tracepoint_collector_types
{
//! Timepoint of that clock.
using TimePoint = uint64_t;
//! Trace event as it is stored in the ringbuffer.
using TraceId = std::uint32_t;
using TraceType = std::uint8_t;
using DynamicDataType = std::unique_ptr<Data>;

struct StaticTraceEvent
{
  TimePoint time_point{ 0 };
  TraceId trace_id{ 0 };
  TraceType trace_type{ 0 };
  DynamicDataType dynamic_data{ nullptr };
};

// (de)serialization of the StaticTraceEvent struct.
template <typename Data>
cbor::result to_cbor(const StaticTraceEvent& b, Data& data)
{
  if (b.dynamic_data == nullptr)
  {
    cbor::result res = data.openArray(3);
    res += to_cbor(b.time_point, data);
    res += to_cbor(b.trace_id, data);
    res += to_cbor(b.trace_type, data);
    return res;
  }
  cbor::result res = data.openArray(4);
  res += to_cbor(b.time_point, data);
  res += to_cbor(b.trace_id, data);
  res += to_cbor(b.trace_type, data);
  res += to_cbor(b.dynamic_data, data);
  return res;
}

template <typename Data>
cbor::result from_cbor(StaticTraceEvent& b, Data& data)
{
  cbor::result res = data.expectArray();
  if (!res)
  {
    return res;
  }

  std::size_t length;
  res += data.readLength(length);
  if (!res)
  {
    return res;
  }
  res += from_cbor(b.time_point, data);
  res += from_cbor(b.trace_id, data);
  res += from_cbor(b.trace_type, data);
  if (length >= 4)
  {
    res += from_cbor(b.dynamic_data, data);
  }
  return res;
}
/**/
//! The container that backs the ringbuffer.
using EventContainer = std::vector<StaticTraceEvent>;
//! The single producer single consumer ringbuffer with the event container.
using ScopeBuffer = SPSCRingBuffer<EventContainer>;
//! Pointer type to the ringbuffer.
using ScopeBufferPtr = std::shared_ptr<ScopeBuffer>;
//! The (grouped by thread) events composed of native types that we can serialize to binary for transfer.
using ThreadedEvents = std::map<unsigned long, EventContainer>;
//! NamedCounter that represents a named counter event with its value.
using NamedCounter = std::tuple<std::string, unsigned int>;
}  // namespace tracepoint_collector_types

/**
 * @brief A singleton class that keeps track of the ringbuffer allocated to each thread to insert tracepoints into.
 *        If a thread goes out of scope the ringbuffer for that thread goes into the orphaned collection. This ensures
 *        that no events get lost when a thread goes out of scope before its events are exfiltrated by the endpoint
 *        native trace sender.
 */
class TracePointCollectorNative
{
public:
  using Ptr = std::shared_ptr<TracePointCollectorNative>;
  using WeakPtr = std::weak_ptr<TracePointCollectorNative>;
  using BufferMap = MapTracker<unsigned long, tracepoint_collector_types::ScopeBufferPtr>;
  using BufferVector = std::vector<std::pair<unsigned long, tracepoint_collector_types::ScopeBufferPtr>>;

  static const uint8_t SCOPE_ENTRY;  // If initialised here and made constexpr clang drops it during linking :(
  static const uint8_t SCOPE_EXIT;
  static const uint8_t MARK_GLOBAL;
  static const uint8_t MARK_PROCESS;
  static const uint8_t MARK_THREAD;
  static const uint8_t COUNTER;

  /**
   * @brief Static method through which the singleton instance can be retrieved.
   * @return Returns the singleton instance of the object.
   */
  static TracePointCollectorNative::Ptr getInstance();

  /**
   * @brief Called by each thread to obtain the ringbuffer in which it should store the trace events.
   */
  tracepoint_collector_types::ScopeBufferPtr getBuffer();

  /**
   * @brief Set the size of any new ringbuffers that will be created.
   */
  void setRingbufferSize(std::size_t size);

  /**
   * @brief Return the map of active threads and their ringbuffers
   */
  BufferMap::MapType getActiveMap() const;

  /**
   * @brief Retrieve the orphaned buffers and clear them.
   */
  BufferVector retrieveOrphanedBuffers();

private:
  TracePointCollectorNative() = default;
  TracePointCollectorNative(const TracePointCollectorNative&) = delete;
  TracePointCollectorNative& operator=(const TracePointCollectorNative&) = delete;
  TracePointCollectorNative& operator=(TracePointCollectorNative&&) = delete;

  /**
   * @brief The size of each thread's ringbuffer.
   * If this is too small, and the thread produces events quicker than the server thread collects them this will result
   * in lost events.
   */
  std::size_t ringbuffer_size_{ 10000 };

  /**
   *  @brief Thread safe map that tracks the event buffers per thread for currently active threads.
   */
  BufferMap active_tid_buffers_;


  mutable std::mutex orphaned_mutex_; //!< Mutex for the orphaned_tid_buffers_

  /**
   * @brief Vector of orphaned buffers, any thread that has a ringbuffer and goes out of scope is moved to this
   *        container, which is cleaned up when they are retrieved by retrieveOrphanedBuffers.
   */
  BufferVector orphaned_tid_buffers_;
};
}  // namespace scalopus

#endif  // SCALOPUS_TRACING_TRACEPOINT_COLLECTOR_NATIVE_H
