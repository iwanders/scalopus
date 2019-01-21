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
#include <iostream>
#include <time.h>

#include <scalopus_tracing/internal/scope_tracepoint.h>
#include "tracepoint_collector_native.h"
#include "scalopus_tracing/native_tracepoint.h"

namespace scalopus
{
namespace native
{
uint64_t nativeGetTime()
{
  timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL) + ts.tv_nsec;
}

uint64_t nativeGetChrono()
{
  using Clock = std::chrono::high_resolution_clock;
  auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(Clock::now());
  auto epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(now_ns.time_since_epoch());
  return epoch.count();
}

void scope_entry(const unsigned int id)
{
  thread_local auto& buffer = *(TracePointCollectorNative::getInstance().getBuffer());
  // @TODO Do something with overrun, count lost events?
  buffer.push(tracepoint_collector_types::ScopeTraceEvent{nativeGetChrono(), id, TracePointCollectorNative::ENTRY});
}

void scope_exit(const unsigned int id)
{
  thread_local auto& buffer =  *(TracePointCollectorNative::getInstance().getBuffer());
  // @TODO Do something with overrun, count lost events?
  buffer.push(tracepoint_collector_types::ScopeTraceEvent{nativeGetChrono(), id, TracePointCollectorNative::EXIT});
}
}  // namespace native
}  // namespace scalopus