#include <cstring>
#include <iostream>
#include <scalopus_general/endpoint_introspect.h>
#include "scalopus_transport/transport_unix.h"

int main(int /* argc */, char** /* argv */)
{
  auto factory = std::make_shared<scalopus::TransportUnixFactory>();
  const auto providers = factory->discover();
  for (const auto& destination : providers)
  {
    std::cout << destination << std::endl;

    auto transport = factory->connect(destination);
    auto introspect_client = std::make_shared<scalopus::EndpointIntrospect>();

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