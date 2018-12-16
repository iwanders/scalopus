#include "transport_unix.h"
#include "scalopus_transport/endpoint_introspect.h"
#include <iostream>
#include <chrono>
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

namespace scalopus
{
class EndpointTest : public Endpoint
{
public:
  std::function<bool(Transport& transport, const Data& incoming, Data& outgoing)> handle_;
  std::function<bool(Transport& transport, const Data& incoming, Data& outgoing)> unsolicited_;
  std::string name_{"endpoint_test"};
  std::string getName() const
  {
    return name_;
  }

  bool handle(Transport& transport, const Data& incoming, Data& outgoing)
  {
    if (handle_)
    {
      return handle_(transport, incoming, outgoing);
    }
    return false;
  }

  bool unsolicited(Transport& transport, const Data& incoming, Data& outgoing)
  {
    if (unsolicited_)
    {
      return unsolicited_(transport, incoming, outgoing);
    }
    return false;
  }
};
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

  // Add an endpoint to the client that allows us to detect broadcasts.
  const auto test_endpoint = std::make_shared<scalopus::EndpointTest>();
  char received_unsolicited = 0;
  test_endpoint->unsolicited_ = [&](scalopus::Transport& /* transport */, const scalopus::Data& incoming, scalopus::Data& /* outgoing */) -> bool
  {
    received_unsolicited = incoming.front();
    return false;
  };
  client0->addEndpoint(test_endpoint);

  // next, send a broadcast.
  server->broadcast(test_endpoint->name_, scalopus::Data{'a', 'b'});

  // Wait for the broadcast to propagate.
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  test(received_unsolicited, 'a');

  return 0;
}