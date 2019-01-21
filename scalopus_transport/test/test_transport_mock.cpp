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
#include <unistd.h>
#include <chrono>
#include <iostream>
#include "test_transport_util.h"
#include "transport_mock.h"

int main(int /* argc */, char** /* argv */)
{
  auto factory = std::make_shared<scalopus::TransportMockFactory>();

  // Create the server
  auto server = factory->serve();

  // Put an echo endpoint in the server.
  auto endpoint0_at_server = std::make_shared<scalopus::EndpointTest>();
  endpoint0_at_server->handle_ = [](scalopus::Transport& /* transport */, const auto& incoming,
                                    auto& outgoing) -> bool {
    outgoing = incoming;
    return true;
  };
  server->addEndpoint(endpoint0_at_server);

  // Create a client that's connected to the mock server.
  auto client0 = factory->connect(server);
  test(client0->isConnected(), true);

  // Check if we can retrieve the introspect endpoint from the server over the transport.
  const scalopus::Data request{ 't', 'e', 's', 't' };
  const auto pending_response = client0->request("endpoint_test", request);
  const auto status = pending_response->wait_for(std::chrono::seconds(1));
  if (status != std::future_status::ready)
  {
    test("future status was not ", " ready");
  }
  const auto value = pending_response->get();
  test(value.size(), request.size());
  test(std::equal(request.begin(), request.end(), value.begin()), true);

  // Add an endpoint to the client that allows us to detect broadcasts.
  const auto test_endpoint = std::make_shared<scalopus::EndpointTest>();
  char received_unsolicited = 0;
  test_endpoint->unsolicited_ = [&](scalopus::Transport& /* transport */, const scalopus::Data& incoming,
                                    scalopus::Data &
                                    /* outgoing */) -> bool {
    received_unsolicited = incoming.front();
    return false;
  };
  client0->addEndpoint(test_endpoint);

  // next, send a broadcast.
  server->broadcast(test_endpoint->name_, scalopus::Data{ 'a', 'b' });

  // Wait for the broadcast to propagate.
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  test(received_unsolicited, 'a');

  return 0;
}