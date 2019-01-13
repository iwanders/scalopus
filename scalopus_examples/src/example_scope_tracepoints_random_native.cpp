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

/**
 * This example can create random callstacks at arbritrary rate and number of threads.
 * ./scalopus_examples/example_scope_tracepoints_random [time_base (usec) [thread_count]]
 * Start with a time base of 10000, that's the default and puts each scope transition at least 10 ms apart.
 * Default thread count is 1.
 */

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>

#include <signal.h>

#include <scalopus_general/endpoint_introspect.h>
#include <scalopus_general/endpoint_process_info.h>
#include <scalopus_general/thread_naming.h>
#include <scalopus_tracing/native_trace_endpoint_sender.h>
#include <scalopus_tracing/endpoint_scope_tracing.h>
#include <scalopus_tracing/scope_tracing.h>
#include <scalopus_transport/transport_unix.h>

int randint(int min, int max)
{
  static std::mt19937 gen;
  std::uniform_int_distribution<> dis(min, max);
  return dis(gen);
}

void random_callstack(unsigned int level, size_t time_base)
{
  std::this_thread::sleep_for(std::chrono::microseconds(2 * time_base));
  {
    scalopus::TraceRAII tracepoint{ level };
    std::this_thread::sleep_for(std::chrono::microseconds(1 * time_base));
    if (randint(0, 2) && (level <= 10))
    {
      random_callstack(level + 1, time_base);
    }
    else
    {
    }
  }
  // Slim chance of growing again.
  if ((randint(0, 2) == 0) && (level <= 10))
  {
    random_callstack(level, time_base);
  }
  std::this_thread::sleep_for(std::chrono::microseconds(2 * time_base));
}

// Signal handling.
bool running{ true };
void sigint_handler(int /* s */)
{
  running = false;
}
int main(int argc, char** argv)
{
  // hook control+c for graceful quitting
  ::signal(SIGINT, &sigint_handler);

  // Option handling.
  size_t time_base = 10000;
  size_t thread_count = 1;
  if (argc >= 2)  // got a time base.
  {
    time_base = std::atoi(argv[1]);
  }
  if (argc >= 3)  // got a thread count as well
  {
    thread_count = std::atoi(argv[2]);
  }
  if (time_base == 0)  // failed to parse time base, like --help... :)
  {
    std::cerr << "" << argv[0] << " [time_base (usec) [thread_count]]" << std::endl;
    exit(1);
  }
  std::cout << "Using time base of: " << time_base << " usec" << std::endl;
  std::cout << "Using thread_count: " << thread_count << "" << std::endl;

  // Instantiating the scalopus transport.
  auto factory = std::make_shared<scalopus::TransportUnixFactory>();
  const auto server = factory->serve();
  server->addEndpoint(std::make_unique<scalopus::EndpointScopeTracing>());
  server->addEndpoint(std::make_unique<scalopus::EndpointIntrospect>());
  auto collector = std::make_shared<scalopus::NativeTraceEndpointSender>();
  collector->setTransport(server);
  server->addEndpoint(collector);
  auto endpoint_process_info = std::make_shared<scalopus::EndpointProcessInfo>();
  endpoint_process_info->setProcessName(argv[0]);
  server->addEndpoint(endpoint_process_info);

  TRACE_THREAD_NAME("main");
  TRACE_PRETTY_FUNCTION();

  // Create the threads
  std::vector<std::thread> active_threads;
  std::atomic_bool threads_running{ true };
  for (size_t i = 0; i < thread_count; i++)
  {
    active_threads.emplace_back([&]() {
      std::stringstream thread_name;
      thread_name << "Thread 0x" << std::hex << i;
      TRACE_THREAD_NAME(thread_name.str());
      while (threads_running.load())
      {
        random_callstack(0, time_base);
      }
    });
  }

  // Block until interrupt is received.
  while (running)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  threads_running.store(false);
  std::cout << "Joining threads" << std::endl;
  for (auto& t : active_threads)
  {
    t.join();
  }

  return 0;
}
