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

#include <scalopus_general/general_provider.h>
#include <scalopus_tracing/endpoint_native_trace_sender.h>
#include <scalopus_tracing/endpoint_trace_configurator.h>
#include <scalopus_tracing/lttng_provider.h>
#include <scalopus_tracing/native_trace_provider.h>
#include <scalopus_transport/transport_unix.h>

#include "scalopus_catapult/catapult_server.h"

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

int main(int argc, char** argv)
{
  // hook control+c for graceful quitting
  ::signal(SIGINT, &sigint_handler);

  int port = 9222;                               // 9222 is default chrom(e/ium) remote debugging port.
  std::string path = "scalopus_target_session";  // empty path defaults to 'lttng view'
  if (argc >= 2)
  {
    // retrieve the port number to bind the webserver on
    port = std::atoi(argv[1]);
  }
  // Retrieve the path to run babeltrace on.
  if (argc >= 3)
  {
    path = std::string(argv[2]);
  }
  if (port == 0)  // failed to parse the port, like --help... :)
  {
    std::cerr << "" << argv[0] << " [port [lttng_session_or_babeltrace_path]]" << std::endl;
    std::cerr << std::endl;
    std::cerr << " port:" << std::endl;
    std::cerr << "      the port on which the catapult backend will bind. Defaults to 9222." << std::endl << std::endl;
    std::cerr << " lttng_session_or_babeltrace_path:" << std::endl;
    std::cerr << "      If no \"/\" is present in the string, this is used as the lttng" << std::endl;
    std::cerr << "      session, the default is \"scalopus_target_session\". If a \"/\"" << std::endl;
    std::cerr << "      is present in the path it is used as the full babeltrace path." << std::endl;
    std::cerr << "      This means the hostname and protocol must be specified, this" << std::endl;
    std::cerr << "      allows connecting to sessions other than on localhost. Using" << std::endl;
    std::cerr << "      \"net://localhost/host/$HOSTNAME/scalopus_target_session\" has " << std::endl;
    std::cerr << "      the same result as setting \"scalopus_target_session\"." << std::endl;
    exit(1);
  }

  std::cout << "[main] Using port: " << port << ", 9222 is default, it is default remote debugging port" << std::endl;

  std::cout << "[main] Using path: \"" << path << "\"  (defaults to lttng view scalopus_target_session)" << std::endl;

  // Create the transport & endpoint manager.
  auto factory = std::make_shared<scalopus::TransportUnixFactory>();
  auto manager = std::make_shared<scalopus::EndpointManagerPoll>(factory);
  auto logging_function = [](const std::string& msg) { std::cout << msg << std::endl; };
  manager->setLogger(logging_function);
  //  factory->setLogger(logging_function);

  // Add scope tracing endpoint factory function.
  manager->addEndpointFactory<scalopus::EndpointTraceMapping>();

  // Add endpoint factory function for the process information.
  manager->addEndpointFactory<scalopus::EndpointProcessInfo>();

  // Adding this endpoint, just to prevent it from complaining for now.
  // In the future we could use this endpoint to toggle threads from the UI, but the UI's support for this is quite
  // poor, and once a setting is seen it will be present indefinitely.
  manager->addEndpointFactory<scalopus::EndpointTraceConfigurator>();

  auto native_trace_provider = std::make_shared<scalopus::NativeTraceProvider>(manager);
  manager->addEndpointFactory(scalopus::EndpointNativeTraceSender::name, native_trace_provider);

  // Setup logging for the native logger.
  auto native_logging = [](const std::string& msg) { std::cout << "[nativeProvider] " << msg << std::endl; };
  native_trace_provider->setLogger(native_logging);

  // Create the server and add the providers
  auto catapult_server = std::make_shared<scalopus::CatapultServer>();
  auto lttng_provider = std::make_shared<scalopus::LttngProvider>(path, manager);
  lttng_provider->setLogger(logging_function);
  catapult_server->addProvider(lttng_provider);
  catapult_server->addProvider(native_trace_provider);
  catapult_server->addProvider(std::make_shared<scalopus::GeneralProvider>(manager));
  catapult_server->setLogger(logging_function);

  // Use default logging at warn level.
  catapult_server->setSeasocksWarningLogger();

  // Bind and start.
  catapult_server->start();

  std::cout << "[main] Everything started, falling into loop to detect transports. Use ctrl + c to quit." << std::endl;

  manager->startPolling(1.0);

  // block while we serve requests.
  while (running)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::cout << "[main] Shutting down." << std::endl;
  return 0;
}
