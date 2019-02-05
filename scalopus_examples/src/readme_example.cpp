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

#include <scalopus_tracing/tracing.h>
#include <scalopus_transport/transport_unix.h>

void fooBarBuz()
{
  TRACE_PRETTY_FUNCTION();  // RAII tracepoint using __PRETTY_FUNCTION__
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void c()
{
  TRACE_SCOPE_RAII("void c()");  // RAII tracepoint, name will be "in c()"
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  std::cout << "  c" << std::endl;
  fooBarBuz();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void b()
{
  // Or handle the start and stop events yourself.
  // Strings at the start and end need to be compile time constant, identical and in the same file.
  // It is recommended to use the RAII tracepoints.
  TRACE_SCOPE_START("void b()");
  std::cout << " b" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  c();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  TRACE_SCOPE_END("void b()");
}

void a()
{
  TRACE_PRETTY_FUNCTION();
  std::cout << "a" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  b();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

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

  while (true)
  {
    a();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  return 0;
}
