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
#include <chrono>
#include <iostream>
#include <thread>
#include <cmath>

#include <scalopus_tracing/tracing.h>
#include <scalopus_transport/transport_unix.h>

// This example shows how counters can be used.

int main(int /* argc */, char** argv)
{
  auto factory = std::make_shared<scalopus::TransportUnixFactory>();
  const auto server = factory->serve();
  server->addEndpoint(std::make_unique<scalopus::EndpointTraceMapping>());
  server->addEndpoint(std::make_unique<scalopus::EndpointIntrospect>());
  // Native sender is not needed for LTTng or NOP tracepoints, but it must be present if the native tracepoints are
  // ever to be used.
  server->addEndpoint(std::make_shared<scalopus::EndpointNativeTraceSender>());
  auto endpoint_process_info = std::make_shared<scalopus::EndpointProcessInfo>();

  // Set the process name here:
  endpoint_process_info->setProcessName(argv[0]);
  server->addEndpoint(endpoint_process_info);

  // Set the thread name.
  TRACE_THREAD_NAME("main");

  double t{ 0 };
  int steps{ 0 };
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    steps++;
    t += 0.1;
    std::int64_t a = static_cast<std::int64_t>(100.0 * std::sin(t * 10.0) + 100.0);
    std::int64_t b = static_cast<std::int64_t>(50.0 * std::sin(t * 2.0) + 50.0);
    
    TRACE_COUNTER_SERIES("sinusoids", "fast", b);
    TRACE_COUNTER_SERIES("sinusoids", "slow", a);
    TRACE_COUNTER("ramp", steps % 100);
  }

  return 0;
}
