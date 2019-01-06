#include <scalopus_transport/transport_unix.h>
#include <cstring>
#include <iostream>
#include "scalopus_lttng/endpoint_scope_tracing.h"

int main(int /* argc */, char** /* argv */)
{
  const auto providers = scalopus::getTransportServersUnix();
  for (const auto& pid : providers)
  {
    std::cout << pid << std::endl;

    auto transport = scalopus::transportClientUnix(pid);
    auto tracing_client = std::make_shared<scalopus::EndpointScopeTracing>();
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