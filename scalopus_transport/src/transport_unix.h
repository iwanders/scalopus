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

#ifndef SCALOPUS_TRANSPORT_TRANSPORT_UNIX_INTERNAL_H
#define SCALOPUS_TRANSPORT_TRANSPORT_UNIX_INTERNAL_H

#include <scalopus_interface/transport_factory.h>
#include "scalopus_transport/transport_unix.h"
#include <future>
#include <map>
#include <set>
#include <thread>
#include <utility>
#include <vector>
#include "protocol.h"

namespace scalopus
{
/**
 * @brief A transport using unix domain sockets. This creates abstract unix domain socket. The name of which is
 * ss << "" << ::getpid() << "_scalopus". Discovery is performed via parsing of "/proc/net/unix".
 * Each request is associated with a request id on while on the wire. This allows interleaved communication. Broadcasts
 * always use request id 0.
 */
class TransportUnix : public Transport
{
public:
  using Ptr = std::shared_ptr<TransportUnix>;
  TransportUnix();
  ~TransportUnix();

  /**
   * @brief Bind the transport as a server.
   * @return true on success, false on error.
   */
  bool serve();

  /**
   * @brief Connect to a server.
   * @param pid The process id to connect to.
   */
  bool connect(std::size_t pid);

  /**
   * @brief Return a list of transport server process id's that are currently running.
   */
  static std::vector<std::size_t> getTransportServers();

  // From Transport superclass.
  PendingResponse request(const std::string& remote_endpoint_name, const Data& outgoing);
  std::size_t pendingRequests() const;

  bool isConnected() const;

  Destination::Ptr getAddress() const;
private:
  using PendingRequest = std::pair<std::promise<Data>, std::weak_ptr<std::future<Data>>>;

  std::thread thread_;  //!< Worker thread to handle connections and communication.
  void work();          //!< Function for the worker thread.
  int server_fd_{ 0 };  //!< File descriptor from the server bind.
  int client_fd_{ 0 };  //!< File descriptor from connecting to a server.

  bool running_{ false };  //!< Boolean to quit the worker thread.

  std::set<int> connections_;  //!< Open connections, also holds server_fd_ and client_fd_.

  /**
   * @brief Processes an incoming message by forwarding it to the appropriate endpoint and providing the response
   *        from the endpoint back to the caller.
   * @return true if a response was populated and should be sent back.
   */
  bool processMsg(const protocol::Msg& request, protocol::Msg& response);

  size_t request_counter_{ 1 };    // 0 is reserved for broadcasts
  mutable std::mutex write_lock_;  //!< Lock to ensure only one thread is writing to the socket.

  mutable std::mutex request_lock_;  //!< Lock to guard modification of ongoing_requests_ map.

  //! The outstanding requests and their promised data.
  std::map<std::pair<std::string, size_t>, PendingRequest> ongoing_requests_;
};

class TransportUnixDestination : public Destination
{
public:
  TransportUnixDestination(unsigned int pid);
  unsigned int pid_;
  operator std::string() const;
  std::size_t hash_code() const;
};

}  // namespace scalopus
#endif  // SCALOPUS_TRANSPORT_TRANSPORT_UNIX_INTERNAL_H
