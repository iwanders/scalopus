#include <iostream>
#include <cstring>
#include <scalopus_transport/transport_unix.h>
#include "scalopus_lttng/client_scope_tracing.h"

int main(int /* argc */, char** /* argv */)
{
  const auto providers = scalopus::getTransportServersUnix();
  for (const auto& pid : providers)
  {
    std::cout << pid << std::endl;

    auto transport = scalopus::transportClientUnix(pid);
    auto tracing_client = std::make_shared<scalopus::ClientScopeTracing>();

    std::cout << "Connected? " << transport->isConnected() << std::endl;

    tracing_client->setTransport(transport);
    std::cout << transport << std::endl;

    auto mappings = tracing_client->mapping();
    for (const auto& id_name : mappings)
    {
      std::cout << "" << id_name.first << " -> " << id_name.second << std::endl;
    }
  }

}