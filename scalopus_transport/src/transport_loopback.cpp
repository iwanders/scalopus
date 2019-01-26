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
#include "transport_loopback.h"
#include <iostream>

namespace scalopus
{
TransportLoopback::TransportLoopback()
{
  running_ = true;
  thread_ = std::thread([&]() { work(); });
}

TransportLoopback::~TransportLoopback()
{
  running_ = false;
  thread_.join();
}

TransportLoopback::TransportLoopback(Ptr server) : TransportLoopback{}
{
  server_ = server;
}

void TransportLoopback::addClient(Transport::WeakPtr client)
{
  clients_.push_back(client);
}

bool TransportLoopback::isConnected() const
{
  return server_ != nullptr;
}

TransportLoopback::PendingResponse TransportLoopback::request(const std::string& remote_endpoint_name,
                                                              const Data& outgoing)
{
  auto promise = std::promise<Data>();
  auto response = std::make_shared<std::future<Data>>(promise.get_future());

  std::lock_guard<decltype(request_lock_)> lock(request_lock_);
  ongoing_requests_.emplace_back(remote_endpoint_name, outgoing, std::move(promise), response);

  return response;
}

std::size_t TransportLoopback::pendingRequests() const
{
  std::lock_guard<decltype(request_lock_)> lock(request_lock_);
  return ongoing_requests_.size();
}

void TransportLoopback::work()
{
  while (running_)
  {
    {
      std::lock_guard<decltype(request_lock_)> lock(request_lock_);
      for (auto& req : ongoing_requests_)
      {
        const auto& remote_endpoint_name = std::get<0>(req);
        const auto& outgoing = std::get<1>(req);
        auto& promise = std::get<2>(req);
        auto& request_ptr = std::get<3>(req);
        if (request_ptr.lock() == nullptr)
        {
          continue;  // request went stale.
        }

        // get the appropriate endpoint from the server.
        auto endpoint = server_->getEndpoint(remote_endpoint_name);

        // If the endpoint doesn't exist on the remote end, throw
        if (endpoint == nullptr)
        {
          promise.set_exception(std::make_exception_ptr(std::runtime_error(
              "The TransportLoopback does not sending data to non-existant endpoints on the server.")));
        }

        Data resp;
        auto res = endpoint->handle(*server_, outgoing, resp);
        if (res)
        {
          promise.set_value(resp);
        }
      }
      ongoing_requests_.clear();
    }
    {
      while (haveBroadcast())
      {
        auto broadcast = Transport::popBroadcast();
        const auto& remote_endpoint_name = broadcast.first;
        const auto& outgoing = broadcast.second;
        for (auto weak_client : clients_)
        {
          auto client = weak_client.lock();
          if (client != nullptr)
          {
            auto endpoint = client->getEndpoint(remote_endpoint_name);
            Data resp;
            if (endpoint->unsolicited(*client, outgoing, resp))
            {
              Data resp2;
              if (getEndpoint(remote_endpoint_name)->handle(*client, resp, resp2))
              {
                // We stop here...
                throw std::runtime_error("The TransportLoopback does not handle responses on unsolicited data.");
              }
            }
          }
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

// Methods for the loopback factory
std::vector<Destination::Ptr> TransportLoopbackFactory::discover()
{
  return {};
}

Transport::Ptr TransportLoopbackFactory::serve()
{
  return std::make_shared<TransportLoopback>();
}

Transport::Ptr TransportLoopbackFactory::connect(const Destination::Ptr& destination)
{
  auto mock_server = std::dynamic_pointer_cast<TransportLoopback>(destination);
  auto client = std::make_shared<TransportLoopback>(mock_server);
  mock_server->addClient(client);
  return client;
}

Transport::Ptr TransportLoopbackFactory::connect(const Transport::Ptr& destination)
{
  auto mock_server = std::dynamic_pointer_cast<TransportLoopback>(destination);
  auto client = std::make_shared<TransportLoopback>(mock_server);
  mock_server->addClient(client);
  return client;
}
}  // namespace scalopus
