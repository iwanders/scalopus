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

#ifndef SCALOPUS_INTERFACE_ENDPOINT_MANAGER_H
#define SCALOPUS_INTERFACE_ENDPOINT_MANAGER_H

#include <scalopus_interface/transport_factory.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace scalopus
{
/**
 * @brief The endpoint manager provides access to transports and their endpoints. This is used by the trace event
 *        providers and trace event sources to obtain the data or metadata.
 */
class EndpointManager
{
public:
  using Ptr = std::shared_ptr<EndpointManager>;
  using EndpointFactory = std::function<std::shared_ptr<Endpoint>(const std::shared_ptr<Transport>& transport)>;
  using EndpointMap = std::map<std::string, Endpoint::Ptr>;
  using TransportEndpoints = std::map<Transport::Ptr, EndpointMap>;

  virtual ~EndpointManager() = default;

  /**
   * @brief Return the map of the currently known endpoints.
   */
  virtual TransportEndpoints endpoints() const = 0;

  /**
   * @brief Add an endpoint factory function to the manager.
   */
  virtual void addEndpointFactory(const std::string& name, EndpointFactory&& factory_function) = 0;

  /**
   * @brief Helper function to find a specific endpoint in a map of functions.
   * @code auto endpoint_ptr = EndpointManager::findEndpoint<scalopus::EndpointProcessInfo>(endpoint_map);
   */
  template <typename T>
  static std::shared_ptr<T> findEndpoint(const EndpointMap& transport_endpoints)
  {
    auto it = transport_endpoints.find(T::name);
    if (it != transport_endpoints.end())
    {
      const auto endpoint_instance = std::dynamic_pointer_cast<T>(it->second);
      if (endpoint_instance == nullptr)
      {
        // This should never happen, as the pointer by the name of T::name should be of the correct type.
        throw std::runtime_error("Endpoint's name does not match its real type.");
      }
      return endpoint_instance;
    }
    return nullptr;
  }
};
}  // namespace scalopus
#endif  // SCALOPUS_INTERFACE_ENDPOINT_MANAGER_H
