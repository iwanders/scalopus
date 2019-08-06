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
#include <time.h>
#include <iostream>

#include <scalopus_tracing/internal/marker_tracepoint.h>
#include <scalopus_tracing/internal/scope_tracepoint.h>
#include <scalopus_tracing/trace_configurator.h>
#include "scalopus_tracing/native_tracepoint.h"
#include "tracepoint_collector_native.h"

namespace scalopus
{
namespace native
{
/*
static uint64_t nativeGetTime()
{
  timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL) + ts.tv_nsec;
}
*/

static int64_t nativeGetChrono()
{
  using Clock = std::chrono::high_resolution_clock;
  auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(Clock::now());
  auto epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(now_ns.time_since_epoch());
  return epoch.count();
}

void scope_entry(const unsigned int id)
{
  static auto configurator_ptr = TraceConfigurator::getInstance();
  thread_local auto buffer_ptr = TracePointCollectorNative::getInstance();
  thread_local auto buffer = buffer_ptr->getBuffer();
  static auto process_state = configurator_ptr->getProcessStatePtr();
  thread_local auto thread_state = configurator_ptr->getThreadStatePtr();
  if (!process_state->load() || !thread_state->load())
  {
    return;
  }
  // @TODO Do something with overrun, count lost events?
  buffer->push(
      tracepoint_collector_types::StaticTraceEvent{ nativeGetChrono(), id, TracePointCollectorNative::SCOPE_ENTRY });
}

void scope_exit(const unsigned int id)
{
  static auto configurator_ptr = TraceConfigurator::getInstance();
  thread_local auto buffer_ptr = TracePointCollectorNative::getInstance();
  thread_local auto buffer = buffer_ptr->getBuffer();
  static auto process_state = configurator_ptr->getProcessStatePtr();
  thread_local auto thread_state = configurator_ptr->getThreadStatePtr();
  if (!process_state->load() || !thread_state->load())
  {
    return;
  }
  // @TODO Do something with overrun, count lost events?
  buffer->push(
      tracepoint_collector_types::StaticTraceEvent{ nativeGetChrono(), id, TracePointCollectorNative::SCOPE_EXIT });
}

void mark_event(const unsigned int id, const MarkLevel mark_level)
{
  static auto configurator_ptr = TraceConfigurator::getInstance();
  thread_local auto buffer_ptr = TracePointCollectorNative::getInstance();
  thread_local auto buffer = buffer_ptr->getBuffer();
  static auto process_state = configurator_ptr->getProcessStatePtr();
  thread_local auto thread_state = configurator_ptr->getThreadStatePtr();
  if (!process_state->load() || !thread_state->load())
  {
    return;
  }
  // @TODO Do something with overrun, count lost events?
  switch (mark_level)
  {
    case MarkLevel::GLOBAL:
      buffer->push(tracepoint_collector_types::StaticTraceEvent{ nativeGetChrono(), id,
                                                                 TracePointCollectorNative::MARK_GLOBAL });
      break;
    case MarkLevel::PROCESS:
      buffer->push(tracepoint_collector_types::StaticTraceEvent{ nativeGetChrono(), id,
                                                                 TracePointCollectorNative::MARK_PROCESS });
      break;
    case MarkLevel::THREAD:
      buffer->push(tracepoint_collector_types::StaticTraceEvent{ nativeGetChrono(), id,
                                                                 TracePointCollectorNative::MARK_THREAD });
      break;
  }
}

}  // namespace native
}  // namespace scalopus
