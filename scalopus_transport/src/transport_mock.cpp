/*
  Copyright (c) 2019, Ivor Wanders
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

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
#include "transport_mock.h"
#include <iostream>

namespace scalopus
{
TransportMock::TransportMock()
{
}

TransportMock::TransportMock(Ptr server) : server_(server)
{
}

void TransportMock::addClient(Transport::WeakPtr client)
{
  clients_.push_back(client);
}

bool TransportMock::isConnected() const
{
  return server_ != nullptr;
}

TransportMock::PendingResponse TransportMock::request(const std::string& remote_endpoint_name, const Data& outgoing)
{
  // This is from the client.
  auto promise = std::promise<Data>();
  auto response = std::make_shared<std::future<Data>>(promise.get_future());
  // get the appropriate endpoint from the server.
  auto endpoint = server_->getEndpoint(remote_endpoint_name);

  // If the endpoint doesn't exist on the remote end, throw
  if (endpoint == nullptr)
  {
    throw std::runtime_error("The TransportMock does not sending data to non-existant endpoints on the server.");
  }
  auto handle = std::async(std::launch::async, [&]()
  {
    Data resp;
    auto res = endpoint->handle(*server_, outgoing, resp);
    if (res)
    {
      promise.set_value(resp);
    }
  });
  return response;
}

void TransportMock::broadcast(const std::string& remote_endpoint_name, const Data& outgoing)
{
  for (auto weak_client : clients_)
  {
    auto client = weak_client.lock();
    if (client != nullptr)
    {
      auto endpoint = client->getEndpoint(remote_endpoint_name);
      std::async(std::launch::async, [&]()
      {
        Data resp;
        if (endpoint->unsolicited(*client, outgoing, resp))
        {
          Data resp2;
          if (getEndpoint(remote_endpoint_name)->handle(*client, resp, resp2))
          {
            // We don't handle the response of this endpoint here, it's a limitation of this mock class.
            throw std::runtime_error("The TransportMock does not handle responses on unsolicited data.");
          }
        }
      });
    }
  }
}


std::size_t TransportMock::pendingRequests() const
{
  return 0;
}

// Methods for the mock factory
std::vector<Destination::Ptr> TransportMockFactory::discover()
{
  return {};
}

Transport::Ptr TransportMockFactory::serve()
{
  return std::make_shared<TransportMock>();
}

Transport::Ptr TransportMockFactory::connect(const Destination::Ptr& destination)
{
  auto mock_server = std::dynamic_pointer_cast<TransportMock>(destination);
  auto client = std::make_shared<TransportMock>(mock_server);
  mock_server->addClient(client);
  return client;
}

Transport::Ptr TransportMockFactory::connect(const Transport::Ptr& destination)
{
  auto mock_server = std::dynamic_pointer_cast<TransportMock>(destination);
  auto client = std::make_shared<TransportMock>(mock_server);
  mock_server->addClient(client);
  return client;
}



}  // namespace scalopus
