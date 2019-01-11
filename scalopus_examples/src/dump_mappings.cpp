#include <scalopus_lttng/endpoint_scope_tracing.h>
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