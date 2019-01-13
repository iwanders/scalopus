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

#ifndef SCALOPUS_TRACING_TRACEPOINT_COLLECTOR_NATIVE_H
#define SCALOPUS_TRACING_TRACEPOINT_COLLECTOR_NATIVE_H

#include <scalopus_general/map_tracker.h>
#include <chrono>
#include <vector>
#include "spsc_ringbuffer.h"
namespace scalopus
{
namespace tracepoint_collector_types
{

//! Clock to use for trace timestamps.
using Clock = std::chrono::high_resolution_clock;
//! Timepoint of that clock.
using TimePoint = std::chrono::time_point<Clock>;
//! Trace event as it is stored in the ringbuffer.
using ScopeTraceEvent = std::tuple<TimePoint, unsigned int, uint8_t>;
//! The container that backs the ringbuffer.
using EventContainer = std::vector<ScopeTraceEvent>;
//! The single producer single consumer ringbuffer with the event container.
using ScopeBuffer = SPSCRingBuffer<EventContainer>;
//! Pointer type to the ringbuffer.
using ScopeBufferPtr = std::shared_ptr<ScopeBuffer>;
//! The (grouped by thread) events composed of native types that we can serialize to binary for transfer.
using ThreadedEvents = std::map<unsigned long, std::vector<std::tuple<uint64_t, unsigned int, uint8_t>>>;
}

/**
 * @brief A singleton class that keeps track of the ringbuffer allocated to each thread to insert tracepoints into.
 */
class TracePointCollectorNative : public MapTracker<unsigned long, tracepoint_collector_types::ScopeBufferPtr>
{
private:
  TracePointCollectorNative() = default;

  /**
   * @brief The size of each thread's ringbuffer.
   * If this is too small, and the thread produces events quicker than the server thread collects them this will result
   * in lost events.
   */
  std::size_t ringbuffer_size_{ 10000 };
public:
  constexpr static const uint8_t ENTRY = 1;
  constexpr static const uint8_t EXIT = 2;
  /**
   * @brief Static method through which the singleton instance can be retrieved.
   * @return Returns the singleton instance of the object.
   */
  static TracePointCollectorNative& getInstance();

  /**
   * @brief Called by each thread to obtain the ringbuffer in which it should store the trace events.
   */
  tracepoint_collector_types::ScopeBufferPtr getBuffer();

  /**
   * @brief Set the size of any new ringbuffers that will be created.
   */
  void setRingbufferSize(std::size_t size);
  
};
}  // namespace scalopus

#endif  // SCALOPUS_TRACING_TRACEPOINT_COLLECTOR_NATIVE_H
