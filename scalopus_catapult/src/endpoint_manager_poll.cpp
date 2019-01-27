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
#include "scalopus_catapult/endpoint_manager_poll.h"
#include <scalopus_general/endpoint_introspect.h>
#include <algorithm>
#include <iostream>

namespace scalopus
{
EndpointManagerPoll::EndpointManagerPoll(const TransportFactory::Ptr factory) : factory_(factory)
{
  std::lock_guard<std::mutex> lock(mutex_);
  endpoint_factories_[EndpointIntrospect::name] = [](const auto& transport) {
    return EndpointIntrospect::factory(transport);
  };
}

EndpointManagerPoll::~EndpointManagerPoll()
{
  stopPolling();
}

void EndpointManagerPoll::manage()
{
  std::lock_guard<std::mutex> lock(mutex_);
  const auto current_transports = transports_;

  // Clean up transports that have died on us.
  for (const auto& transport : current_transports)
  {
    if (!transport.second->isConnected())
    {
      std::cerr << "[scalopus] Cleaning up transport to: " << transport.first << std::endl;
      transports_.erase(transport.first);
      transport_endpoints_.erase(std::find_if(transport_endpoints_.begin(), transport_endpoints_.end(),
                                              [&](const auto& v) { return v.first == transport.second; }));
    }
  }

  // Try to make new connections and setup the endpoints.
  const auto providers = factory_->discover();
  for (const auto& destination : providers)
  {
    if (transports_.find(destination->hash_code()) != transports_.end())
    {
      continue;  // already have a connection to this transport, ignore it.
    }
    std::cerr << "[scalopus] Creating transport to: " << destination << std::endl;
    // Attempt to make a transport to this server.
    auto transport = factory_->connect(destination);
    if (!transport->isConnected())
    {
      std::cerr << "[scalopus] Client failed to connect to " << destination << std::endl;
      continue;
    }
    transports_[destination->hash_code()] = transport;
    transport_endpoints_[transport] = {};

    // Investigate which endpoints are supported by this transport.
    auto introspect_client = std::make_shared<EndpointIntrospect>();
    introspect_client->setTransport(transport);
    const auto supported = introspect_client->supported();
    for (const auto& supported_endpoint : supported)
    {
      const auto it = endpoint_factories_.find(supported_endpoint);
      if (it != endpoint_factories_.end())
      {
        // we support this endpoint, construct it and add it to the list.
        auto new_endpoint = endpoint_factories_[supported_endpoint](transport);
        transport_endpoints_[transport][supported_endpoint] = new_endpoint;
        // Also register the new endpoint with the transport in case it needs to listen to broadcast messages.
        transport->addEndpoint(new_endpoint);
      }
      else
      {
        std::cerr << "[scalopus] remote supports endpoint we don't support: " << supported_endpoint << std::endl;
      }
    }
  }
}

void EndpointManagerPoll::startPolling(const double interval)
{
  stopPolling();
  is_polling_ = true;
  poll_interval_ = interval;
  thread_ = std::thread([&]() { work(); });
}

void EndpointManagerPoll::stopPolling()
{
  if (is_polling_)
  {
    is_polling_ = false;
    thread_.join();
  }
}
void EndpointManagerPoll::work()
{
  while (is_polling_)
  {
    manage();
    std::this_thread::sleep_for(std::chrono::duration<double>(poll_interval_));
  }
}

EndpointManagerPoll::TransportEndpoints EndpointManagerPoll::endpoints() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return transport_endpoints_;
}

void EndpointManagerPoll::addEndpointFactory(const std::string& name, EndpointFactory&& factory_function)
{
  std::lock_guard<std::mutex> lock(mutex_);
  endpoint_factories_[name] = std::move(factory_function);
}

}  // namespace scalopus
