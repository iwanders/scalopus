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
#ifndef SCALOPUS_TRANSPORT_TRANSPORT_LOOPBACK_INTERNAL_H
#define SCALOPUS_TRANSPORT_TRANSPORT_LOOPBACK_INTERNAL_H

#include <scalopus_interface/transport_factory.h>
#include <scalopus_transport/transport_loopback.h>
#include <future>
#include <map>
#include <memory>
#include <set>
#include <thread>
#include <utility>
#include <vector>

namespace scalopus
{
/**
 * @brief A loopback transport that just uses pointers to talk to the other side, no serialization or bus. It does
 *        create a worker thread at the server.
 */
class TransportLoopback : public Transport, public std::enable_shared_from_this<TransportLoopback>
{
public:
  using Ptr = std::shared_ptr<TransportLoopback>;
  using WeakPtr = std::weak_ptr<TransportLoopback>;

  /**
   * @brief Constructor without argument is the server side, this starts a worker thread.
   */
  TransportLoopback();

  /**
   * @brief Create the client side of the loopback and connect it to the provided server.
   * @param server The server to connect this client instance to.
   */
  TransportLoopback(Ptr server);

  // From Transport superclass.
  PendingResponse request(const std::string& remote_endpoint_name, const Data& outgoing);
  std::size_t pendingRequests() const;

  void addClient(Transport::WeakPtr client);
  bool isConnected() const;

  Destination::Ptr getAddress();

  ~TransportLoopback();

private:
  //! Type to keep a list of the pending requests.
  using PendingRequest = std::tuple<std::string, Data, std::promise<Data>, std::weak_ptr<std::future<Data>>>;

  Ptr server_;  //!< If populated this is the server the client is connected to.

  std::vector<Transport::WeakPtr> clients_;  //!< List of the clients connected to the server.

  /**
   * @brief Function in which we server the ongoing requests and broadcasts.
   */
  void work();

  bool running_{ true };  //!< Bool to stop the worker loop.
  std::thread thread_;    //!< The thread which drops into work.

  mutable std::mutex request_lock_;  //!< Lock to guard modification of ongoing_requests_ map.

  //! The outstanding requests and their promised data.
  std::vector<PendingRequest> ongoing_requests_;
};

/**
 * @brief DestinationLoopback class. This is just a thin wrapper around a weak pointer to a TransportLoopback instance.
 */
class DestinationLoopback : public Destination
{
public:
  DestinationLoopback(TransportLoopback::WeakPtr loopback_dest);
  operator std::string() const;
  std::size_t hash_code() const;

  TransportLoopback::WeakPtr dest_;
};

}  // namespace scalopus
#endif  // SCALOPUS_TRANSPORT_TRANSPORT_MOCK_INTERNAL_H