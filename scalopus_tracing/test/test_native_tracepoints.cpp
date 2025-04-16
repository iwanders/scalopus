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
#include <scalopus_transport/transport_loopback.h>
#include <iostream>
#include <numeric>
#include <thread>
#include "scalopus_tracing/native_trace_provider.h"
#include "scalopus_tracing/tracing.h"

template <typename A, typename B>
void test(const A& a, const B& b)
{
  if (a != b)
  {
    std::cerr << "a (" << a << ") != b (" << b << ")" << std::endl;
    exit(1);
  }
}

template <typename A, typename B>
void test_less(const A& a, const B& b)
{
  if (a > b)
  {
    std::cerr << "a (" << a << ") > b (" << b << ")" << std::endl;
    exit(1);
  }
}

namespace scalopus
{
class TestEndpointManager : public EndpointManager
{
public:
  TransportEndpoints endpoints_;
  TransportEndpoints endpoints() const
  {
    return endpoints_;
  }
  void addEndpointFactory(const std::string& name, EndpointFactory&& factory_function){};
};
}  // namespace scalopus

int main(int /* argc */, char** /* argv */)
{
  // Create a loopback factory.
  auto factory = std::make_shared<scalopus::TransportLoopbackFactory>();

  // Create the 'server' this is the part that produces the tracepoints.
  auto server = factory->serve();
  auto server_endpoint = std::make_shared<scalopus::EndpointTraceMapping>();
  server->addEndpoint(server_endpoint);
  auto server_trace_sender = std::make_shared<scalopus::EndpointNativeTraceSender>();
  server_trace_sender->setTransport(server);

  // Create the dummy manager for the provider to use.
  auto dummy_manager = std::make_shared<scalopus::TestEndpointManager>();

  // Create the provider to consume the tracepoints.
  auto trace_provider = std::make_shared<scalopus::NativeTraceProvider>(dummy_manager);

  // Create the client and create the endpoint to receive the native tracepoints.
  auto client = factory->connect(server->getAddress());
  auto client_receiver = trace_provider->factory(client);
  client->addEndpoint(client_receiver);

  // Add the endpoint to the client.
  auto client_endpoint = std::make_shared<scalopus::EndpointTraceMapping>();
  client_endpoint->setTransport(client);
  dummy_manager->endpoints_[client] = { { scalopus::EndpointTraceMapping::name, client_endpoint } };

  // Create the source which allows enabling the recording.
  auto source = trace_provider->makeSource();

  // Start the interval to check whether we get tracepoints normally.
  source->startInterval();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  {
    TRACE_SCOPE_RAII("main");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  auto result = source->finishInterval();
  test(result.size(), 2u);  // expect two tracepoints.

  // Confirm values in the tracepoint.
  test(result[0]["name"], "main");
  test(result[1]["name"], "main");
  test(result[0]["ph"], "B");
  test(result[1]["ph"], "E");
  test(result[0]["tid"].get<unsigned long>(), pthread_self());
  test(result[1]["tid"].get<unsigned long>(), pthread_self());
  std::int64_t difference = result[1]["ts"].get<std::int64_t>() - result[0]["ts"].get<std::int64_t>();
  std::int64_t expected = 100 * 1000;        // 100 ms
  std::int64_t allow_difference = 1 * 1000;  // 1 ms

  test_less(std::abs(expected - difference), allow_difference);

  // Disable this threads' tracepoints
  scalopus::TraceConfigurator::getInstance()->setThreadState(false);
  source->startInterval();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // From another thread emit a tracepoint.
  std::thread different_thread = std::thread([]() {
    {
      TRACE_SCOPE_RAII("different_thread");
    }
  });
  {
    TRACE_SCOPE_RAII("enabled_again");
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  different_thread.join();
  result = source->finishInterval();
  test(result.size(), 2u);
  test(result[0]["name"], "different_thread");
  test(result[1]["name"], "different_thread");

  // Enable the tracepoints again.
  scalopus::TraceConfigurator::getInstance()->setThreadState(true);
  source->startInterval();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  {
    TRACE_SCOPE_RAII("enabled");
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  result = source->finishInterval();
  test(result.size(), 2u);  // expect the open and close of 'enabled' now.
  test(result[0]["name"], "enabled");
  test(result[1]["name"], "enabled");

  // Disable process state.
  scalopus::TraceConfigurator::getInstance()->setProcessState(false);
  source->startInterval();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  {
    TRACE_SCOPE_RAII("process_disabled");
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  result = source->finishInterval();
  test(result.size(), 0u);  // Expect no tracepoints.

  // Finally check whether we can flip the process state back to true.
  scalopus::TraceConfigurator::getInstance()->setProcessState(true);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  source->startInterval();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  {
    TRACE_SCOPE_RAII("process_enabled");
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  result = source->finishInterval();
  test(result.size(), 2u);
  test(result[0]["name"], "process_enabled");
  test(result[1]["name"], "process_enabled");

  source->startInterval();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  {
    TRACE_MARK_EVENT_GLOBAL("global_event");
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  result = source->finishInterval();
  test(result.size(), 1u);
  test(result[0]["name"], "global_event");

  source->startInterval();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  {
    TRACE_COUNT("counter", 5);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  result = source->finishInterval();
  test(result.size(), 1u);
  test(result[0]["name"], "counter");
  test(result[0]["ph"], "C");

  // Ensure we don't lose events if thread goes out of scope immediately
  source->startInterval();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // From another thread emit a tracepoint.
  std::thread immediately_closing_thread = std::thread([]() {
    {
      TRACE_SCOPE_RAII("immediately_closing_thread");
    }
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  immediately_closing_thread.join();
  result = source->finishInterval();
  test(result.size(), 2u);
  test(result[0]["name"], "immediately_closing_thread");
  test(result[1]["name"], "immediately_closing_thread");

  return 0;
}
