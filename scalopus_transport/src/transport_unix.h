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

#ifndef SCALOPUS_PROVIDER_H
#define SCALOPUS_PROVIDER_H

#include <scalopus_transport/interface/endpoint.h>
#include <scalopus_transport/interface/transport.h>
#include <map>
#include <set>
#include <thread>
#include <future>
#include <utility>
#include <vector>
#include "protocol.h"

namespace scalopus
{
/**
 * @brief The exposer class that is used to get the data about the trace mappings out of the proces.
 */
class TransportUnix : public Transport
{
public:
  TransportUnix();
  ~TransportUnix();

  bool serve();
  bool connect(std::size_t pid, const std::string& suffix = "_scalopus");
  void disconnect();

  std::shared_future<Data> request(const std::string& remote_endpoint_name, const Data& outgoing);

  bool isConnected() const;

  static std::vector<std::size_t> getProviders(const std::string& suffix = "_scalopus");

private:

  std::thread thread_;
  void work();
  int server_fd_{ 0 };
  int client_fd_{ 0 };
  bool running_{ false };

  std::set<int> connections_;

  bool processMsg(const protocol::Msg& request, protocol::Msg& response);

  size_t request_counter_{ 1 };  // 0 is reserved for broadcasts
  std::mutex write_lock_;  //!< Lock to ensure only one thread is writing to the socket.

  std::mutex request_lock_; //!< Lock to guard modification of ongoing_requests_ map.
  std::map<std::pair<std::string, size_t>, std::promise<Data>> ongoing_requests_;
};

std::unique_ptr<Transport> transportUnix();

}  // namespace scalopus
#endif  // SCALOPUS_PROVIDER_H