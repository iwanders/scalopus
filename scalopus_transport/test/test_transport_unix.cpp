#include "transport_unix.h"
#include "scalopus_transport/endpoint_introspect.h"
#include <iostream>
#include <unistd.h>

template <typename A, typename B>
void test(const A& a, const B& b)
{
  if (a != b)
  {
    std::cerr << "a (" << a << ") != b (" << b << ")" << std::endl;
    exit(1);
  }
}
int main(int /* argc */, char** /* argv */)
{
  // Create the server
  auto server = std::make_shared<scalopus::TransportUnix>();
  test(server->serve(), true);

  // Put the introspection endpoint in the server.
  auto endpoint0_at_server = std::make_shared<scalopus::EndpointIntrospect>();
  server->addEndpoint(endpoint0_at_server);

  // Connect a client to the newly created server using our own process id.
  auto client0 = std::make_shared<scalopus::TransportUnix>();
  test(client0->connect(::getpid()), true);

  // Create an endpoint to use the client. This is not YET inside the client.
  auto endpoint0_for_client = std::make_shared<scalopus::EndpointIntrospect>();
  endpoint0_for_client->setTransport(client0);

  // Check if we can retrieve the introspect endpoint from the server over the transport.
  const auto remote_supported = endpoint0_for_client->supported();
  test(remote_supported.size(), 1U);
  test(remote_supported.front(), "introspect");
  
  return 0;
}