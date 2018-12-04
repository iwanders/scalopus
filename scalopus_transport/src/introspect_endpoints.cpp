#include <cstring>
#include <iostream>
#include "scalopus_transport/client_introspect.h"
#include "scalopus_transport/transport_unix.h"

int main(int /* argc */, char** /* argv */)
{
  const auto providers = scalopus::getTransportServersUnix();
  for (const auto& pid : providers)
  {
    std::cout << pid << std::endl;

    auto transport = scalopus::transportClientUnix(pid);
    auto introspect_client = std::make_shared<scalopus::ClientIntrospect>();

    std::cout << "Connected? " << transport->isConnected() << std::endl;

    introspect_client->setTransport(transport);
    std::cout << transport << std::endl;

    auto endpoints = introspect_client->supported();
    for (const auto& name : endpoints)
    {
      std::cout << "" << name << ",";
    }
    std::cout << std::endl;
  }
}