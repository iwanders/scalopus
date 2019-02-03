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
#include <scalopus_general/general.h>
#include <scalopus_tracing/tracing.h>
#include <scalopus_transport/transport_unix.h>
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>

/**
 * This example retrieves data from the discovered servers. It writes human readable information to stderr while it
 * queries the remote endpoints. At the end it dumps pretty printed json to stdout containing the info it retrieved.
 */
int main(int /* argc */, char** /* argv */)
{
  using json = nlohmann::json;

  json data;  // container for the data.

  auto factory = std::make_shared<scalopus::TransportUnixFactory>();
  const auto found_destinations = factory->discover();

  // Iterate through the found servers.
  for (const auto& destination : found_destinations)
  {
    json server_info;

    std::cerr << "Discovered: " << destination << ", connecting!" << std::endl;

    auto transport = factory->connect(destination);
    auto introspect_client = std::make_shared<scalopus::EndpointIntrospect>();
    introspect_client->setTransport(transport);

    auto endpoints = introspect_client->supported();
    for (const auto& name : endpoints)
    {
      std::cerr << " remote endpoint: " << name << std::endl;

      // Handle all endpoints from which we can extract data.
      if (name == scalopus::EndpointIntrospect::name)
      {
        auto client = std::make_shared<scalopus::EndpointIntrospect>();
        client->setTransport(transport);
        const auto supported = client->supported();
        server_info[scalopus::EndpointIntrospect::name] = supported;
        std::cerr << "  Found " << supported.size() << " endpoints." << std::endl;
        continue;
      }

      if (name == scalopus::EndpointProcessInfo::name)
      {
        auto client = std::make_shared<scalopus::EndpointProcessInfo>();
        client->setTransport(transport);
        const auto process_info = client->processInfo();
        server_info[scalopus::EndpointProcessInfo::name] = { { "name", process_info.name },
                                                             { "threads", process_info.threads },
                                                             { "pid", process_info.pid } };
        std::cerr << "  Pid:     " << process_info.pid << std::endl;
        std::cerr << "  Name:    " << process_info.name << std::endl;
        std::cerr << "  Threads: " << process_info.threads.size() << std::endl;
        continue;
      }

      if (name == scalopus::EndpointTraceMapping::name)
      {
        auto client = std::make_shared<scalopus::EndpointTraceMapping>();
        client->setTransport(transport);
        const auto mapping = client->mapping();
        server_info[scalopus::EndpointTraceMapping::name] = mapping;
        if (mapping.size())
        {
          std::cerr << "  Retrieved " << mapping.begin()->second.size() << " trace mappings." << std::endl;
          for (const auto& id_name : mapping.begin()->second)
          {
            std::cerr << "   " << id_name.first << " -> \"" << id_name.second << "\"" << std::endl;
          }
        }
        continue;
      }
      std::cerr << "  Ignoring " << name << std::endl;
    }
    data.push_back(server_info);
  }

  // Write pretty printed json to stdout.
  std::cout << data.dump(2) << std::endl;
}