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
#include "tracepoint_collector_native.h"
#include <scalopus_general/destructor_callback.h>

namespace scalopus
{
const uint8_t TracePointCollectorNative::SCOPE_ENTRY = 1;
const uint8_t TracePointCollectorNative::SCOPE_EXIT = 2;
const uint8_t TracePointCollectorNative::MARK_GLOBAL = 3;
const uint8_t TracePointCollectorNative::MARK_PROCESS = 4;
const uint8_t TracePointCollectorNative::MARK_THREAD = 5;

TracePointCollectorNative::Ptr TracePointCollectorNative::getInstance()
{
  // https://stackoverflow.com/questions/8147027/
  // Trick to allow make_shared with a private constructor.
  struct make_shared_enabler : public TracePointCollectorNative
  {
  };
  static TracePointCollectorNative::Ptr instance{ std::make_shared<make_shared_enabler>() };
  return instance;
}

tracepoint_collector_types::ScopeBufferPtr TracePointCollectorNative::getBuffer()
{
  const auto tid = static_cast<unsigned long>(pthread_self());
  // Register a destructor callback such that the thread gets removed from the map when the thread exits.
  auto instance_pointer = getInstance();
  thread_local DestructorCallback cleanup{ [instance = TracePointCollectorNative::WeakPtr(instance_pointer), tid]() {
    auto ptr = instance.lock();
    if (ptr != nullptr)
    {
      ptr->erase(tid);
    }
  } };

  if (exists(tid))
  {
    // Buffer already existed for this thread.
    return getMap()[tid];
  }
  else
  {
    // Buffer did not exist for this thread, make a new one.
    auto buffer = std::make_shared<tracepoint_collector_types::ScopeBuffer>(
        tracepoint_collector_types::EventContainer{ ringbuffer_size_ });
    insert(tid, buffer);
    return buffer;
  }
  return nullptr;
}

void TracePointCollectorNative::setRingbufferSize(std::size_t size)
{
  ringbuffer_size_ = size;
}

}  // namespace scalopus
