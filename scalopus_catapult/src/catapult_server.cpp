/*
  Copyright (c) 2018, Ivor Wanders
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

#include <scalopus_general/endpoint_process_info.h>
#include <scalopus_general/general_provider.h>

#include <scalopus_tracing/endpoint_scope_tracing.h>
#include <scalopus_tracing/native_trace_endpoint_sender.h>
#include <scalopus_tracing/lttng_provider.h>
#include <scalopus_tracing/native_trace_provider.h>

#include <scalopus_transport/transport_unix.h>

#include "scalopus_catapult/catapult_backend.h"
#include "scalopus_catapult/endpoint_manager_poll.h"

#include <seasocks/PrintfLogger.h>
#include <seasocks/Server.h>

#include <chrono>
#include <thread>

#include <signal.h>
#include <cstdlib>

// Signal handling.
bool running{ true };
void sigint_handler(int /* s */)
{
  running = false;
}

int main(int /* argc */, char** /* argv */)
{
  // hook control+c for graceful quitting
  ::signal(SIGINT, &sigint_handler);

  // retrieve the port number to bind the webserver on
  int port = 9222;  // 9222 is default chrom(e/ium) remote debugging port.
  std::cout << "[main] Using port: " << port << ", 9222 is default, it is default remote debugging port" << std::endl;

  // Retrieve the path to run babeltrace on.
  std::string path = "";  // empty path defaults to 'lttng view'
  std::cout << "[main] Using path: \"" << path << "\"  (empty defaults to lttng view scalopus_target_session)"
            << std::endl;

  // Create the transport & endpoint manager.
  auto manager = std::make_shared<scalopus::EndpointManagerPoll>(std::make_shared<scalopus::TransportUnixFactory>());

  // Add scope tracing endpoint factory function.
  manager->addEndpointFactory(scalopus::EndpointScopeTracing::name, [](const auto& transport) {
    auto tracing_endpoint = std::make_shared<scalopus::EndpointScopeTracing>();
    tracing_endpoint->setTransport(transport);
    return tracing_endpoint;
  });

  // Add endpoint factory function for the process information.
  manager->addEndpointFactory(scalopus::EndpointProcessInfo::name, [](const auto& transport) {
    auto endpoint = std::make_shared<scalopus::EndpointProcessInfo>();
    endpoint->setTransport(transport);
    return endpoint;
  });

  auto native_trace_provider = std::make_shared<scalopus::NativeTraceProvider>(manager);
  manager->addEndpointFactory(scalopus::NativeTraceEndpointSender::name,
    [provider = std::weak_ptr<scalopus::NativeTraceProvider>(native_trace_provider)](const auto& transport) {
    auto ptr = provider.lock();
    if (ptr)
    {
      auto endpoint = ptr->receiveEndpoint();
      endpoint->setTransport(transport);
      return endpoint;
    }
    return scalopus::Endpoint::Ptr{nullptr};
  });

  // Create the providers.
  std::vector<scalopus::TraceEventProvider::Ptr> providers;
  providers.push_back(std::make_shared<scalopus::LttngProvider>(path, manager));
  providers.push_back(native_trace_provider);


  providers.push_back(std::make_shared<scalopus::GeneralProvider>(manager));

  // Create the catapult backend.
  auto backend = std::make_shared<scalopus::CatapultBackend>(providers);

  // Make a webserver and add the endpoints
  namespace ss = seasocks;
  auto logger = std::make_shared<ss::PrintfLogger>(ss::Logger::Level::WARNING);
  ss::Server server(logger);

  // Set the function that's to be used to execute functions on the seasocks thread.
  backend->setExecutor(
      [&server](scalopus::CatapultBackend::Runnable&& runnable) { server.execute(std::move(runnable)); });

  // Set the send buffer to 128 mb. At the end of the trace, the json representation needs to fit in this buffer.
  server.setClientBufferSize(128 * 1024 * 1024u);

  server.addWebSocketHandler("/devtools/page/bar", backend);  // needed for chrom(e/ium) 65.0+
  server.addWebSocketHandler("/devtools/browser", backend);   // needed for chrome 60.0
  server.addPageHandler(backend);                             // This is retrieved in the overview page.

  // Start the server in a separate thread.
  server.startListening(port);
  std::thread seasocksThread([&] { server.loop(); });

  std::cout << "[main] Everything started, falling into loop to detect transports. Use ctrl + c to quit." << std::endl;

  // block while we serve requests.
  while (running)
  {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    manager->manage();
  }

  std::cout << "[main] Shutting down." << std::endl;
  server.terminate();  // send terminate signal to the server

  // Wait for the webserver thread to join.
  seasocksThread.join();

  return 0;
}
