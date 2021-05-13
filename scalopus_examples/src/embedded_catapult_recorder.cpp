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
#include <sstream>
#include <string>
#include <thread>

// The producer side.
#include <scalopus_tracing/tracing.h>
#include <scalopus_transport/transport_loopback.h>

// The catapult recorder side.
#include <scalopus_catapult/catapult_recorder.h>
#include <scalopus_general/general_provider.h>
#include <scalopus_general/endpoint_manager_poll.h>
#include <scalopus_tracing/native_trace_provider.h>

void c()
{
  TRACE_PRETTY_FUNCTION();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  std::cout << "  c" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void b()
{
  TRACE_PRETTY_FUNCTION();
  std::cout << " b" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  c();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
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
  // Client side to produce the tracepoints.
  auto factory = std::make_shared<scalopus::TransportLoopbackFactory>();
  const auto server = factory->serve();
  server->addEndpoint(std::make_shared<scalopus::EndpointTraceMapping>());
  server->addEndpoint(std::make_shared<scalopus::EndpointIntrospect>());
  server->addEndpoint(std::make_shared<scalopus::EndpointNativeTraceSender>());
  auto endpoint_process_info = std::make_shared<scalopus::EndpointProcessInfo>();
  endpoint_process_info->setProcessName(argv[0]);
  server->addEndpoint(endpoint_process_info);

  // Catapult recorder side.
  auto manager = std::make_shared<scalopus::EndpointManagerPoll>(factory);
  manager->addEndpointFactory<scalopus::EndpointTraceMapping>();
  manager->addEndpointFactory<scalopus::EndpointProcessInfo>();
  auto native_trace_provider = std::make_shared<scalopus::NativeTraceProvider>(manager);
  manager->addEndpointFactory(scalopus::EndpointNativeTraceSender::name, native_trace_provider);

  auto catapult_recorder = std::make_shared<scalopus::CatapultRecorder>();
  catapult_recorder->addProvider(native_trace_provider);
  catapult_recorder->addProvider(std::make_shared<scalopus::GeneralProvider>(manager));

  auto logging_function = [](const std::string& msg) { std::cout << msg << std::endl; };
  logging_function("Logging can be enabled in the source, uncomment the lines below this one.");

  manager->connect(server->getAddress());  // Connect the manager to the server.
  catapult_recorder->start();  // start the recorder thread.
  catapult_recorder->startInterval();  // start recording.

  TRACE_THREAD_NAME("main");

  for (std::size_t i=0; i < 5; i++)
  {
    a();
  }
  // need to guarantee we propagate the tracepoints before we stop the interval.
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  catapult_recorder->stopInterval();
  catapult_recorder->setupDumpFile("/tmp/traces.json");

  return 0;
}
