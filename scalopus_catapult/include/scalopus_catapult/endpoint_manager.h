/*
  Copyright (c) 2018, Ivor Wanders
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

#ifndef SCALOPUS_CATAPULT_ENDPOINT_MANANGER_H
#define SCALOPUS_CATAPULT_ENDPOINT_MANANGER_H

#include <scalopus_transport/interface/endpoint.h>
#include <scalopus_transport/interface/transport.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace scalopus
{
/**
 * @brief Something that discovers transport servers and manages enpoints over these transports.
 * @TODO Refactor such that it is transport agnostic.
 */
class EndpointManager
{
public:
  using Ptr = std::shared_ptr<EndpointManager>;
  using EndpointFactory = std::function<std::shared_ptr<Endpoint>(const std::shared_ptr<Transport>& transport)>;
  using EndpointMap = std::map<std::string, Endpoint::Ptr>;
  using TransportEndpoints = std::map<Transport::Ptr, EndpointMap>;

  EndpointManager();
  /**
   * @brief This function should be called periodically to discover transports and connect.
   */
  void manage();

  /**
   * @brief Return the map of the currently known endpoints.
   */
  TransportEndpoints endpoints() const;

  /**
   * @brief Add an endpoint factory function to the manager.
   */
  void addEndpointFactory(const std::string& name, EndpointFactory&& factory_function);

private:
  mutable std::mutex mutex_;
  std::map<std::string, EndpointFactory> endpoint_factories_;  //!< Map of factory functions to construct endpoints.
  TransportEndpoints endpoints_;
  std::map<std::size_t, Transport::Ptr> transports_;
};
}  // namespace scalopus
#endif