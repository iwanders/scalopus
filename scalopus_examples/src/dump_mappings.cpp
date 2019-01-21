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
#include <scalopus_tracing/tracing.h>
#include <scalopus_transport/transport_unix.h>
#include <cstring>
#include <iostream>

int main(int /* argc */, char** /* argv */)
{
  auto factory = std::make_shared<scalopus::TransportUnixFactory>();
  const auto providers = factory->discover();
  for (const auto& destination : providers)
  {
    std::cout << "Found: " << destination << std::endl;

    auto transport = factory->connect(destination);
    auto tracing_client = std::make_shared<scalopus::EndpointTraceMapping>();
    tracing_client->setTransport(transport);
    std::cout << transport << std::endl;

    auto map_from_endpoint = tracing_client->mapping();
    for (const auto& pid_mappings : map_from_endpoint)
    {
      std::cout << "  pid: " << pid_mappings.first << std::endl;
      const auto& mappings = pid_mappings.second;
      for (const auto& id_name : mappings)
      {
        std::cout << "  " << id_name.first << " -> " << id_name.second << std::endl;
      }
    }
  }
}