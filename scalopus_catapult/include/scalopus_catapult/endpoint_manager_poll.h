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
#ifndef SCALOPUS_CATAPULT_ENDPOINT_MANANGER_POLL_H
#define SCALOPUS_CATAPULT_ENDPOINT_MANANGER_POLL_H

#include <scalopus_interface/endpoint_manager.h>
#include <scalopus_interface/transport_factory.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace scalopus
{
/**
 * @brief Something that discovers transport servers and manages endpoints over these transports.
 */
class EndpointManagerPoll : public EndpointManager
{
public:
  using Ptr = std::shared_ptr<EndpointManager>;
  using EndpointFactory = std::function<std::shared_ptr<Endpoint>(const std::shared_ptr<Transport>& transport)>;
  using EndpointMap = std::map<std::string, Endpoint::Ptr>;
  using TransportEndpoints = std::map<Transport::Ptr, EndpointMap>;

  EndpointManagerPoll(TransportFactory::Ptr factory);
  ~EndpointManagerPoll();

  /**
   * @brief This function should be called periodically to discover transports and connect. Either from an external
   *        thread or from the polling thread in this one.
   */
  void manage();

  /**
   * @brief Return the map of the currently known endpoints.
   */
  virtual TransportEndpoints endpoints() const;

  /**
   * @brief Add an endpoint factory function to the manager.
   */
  void addEndpointFactory(const std::string& name, EndpointFactory&& factory_function);

  /**
   * @brief Start polling with the requested interval.
   * @param interval The interval to wait between manage calls in seconds.
   */
  void startPolling(const double interval);

  /**
   * @brief Disable polling.
   */
  void stopPolling();

private:
  mutable std::mutex mutex_;
  std::map<std::string, EndpointFactory> endpoint_factories_;  //!< Map of factory functions to construct endpoints.
  TransportEndpoints transport_endpoints_;  //!< Map of endpoints, each endpoint holds a map of [name] = endpoint

  //! Map to keep track of which transport is already created.
  std::map<std::size_t, Transport::Ptr> transports_;

  TransportFactory::Ptr factory_;  //!< The transport factory to use to discover and create connections.

  bool is_polling_{ false };     //!< Should the worker thread continue polling?
  double poll_interval_{ 1.0 };  //!< The poll interval in seconds.
  std::thread thread_;           //!< Thread in which the polling takes place.
  void work();                   //!< Function in which the work the poller does takes place.
};
}  // namespace scalopus
#endif  // SCALOPUS_CATAPULT_ENDPOINT_MANANGER_POLL_H
