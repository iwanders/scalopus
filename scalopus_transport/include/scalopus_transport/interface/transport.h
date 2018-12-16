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

#ifndef SCALOPUS_INTERFACE_TRANSPORT_H
#define SCALOPUS_INTERFACE_TRANSPORT_H

#include <scalopus_transport/interface/endpoint.h>
#include <memory>
#include <map>
#include <future>
#include <vector>
#include <mutex>

namespace scalopus
{

class Transport
{
public:
  virtual ~Transport();
  virtual void addEndpoint(const std::shared_ptr<Endpoint>& endpoint);

  virtual std::shared_future<Data> request(const std::string& remote_endpoint_name, const Data& outgoing) = 0;

  /**
   * @brief Broadcast is an asynchronous call, this is queued for broadcast, the worker than sends it at it's discretion
   */
  virtual void broadcast(const std::string& remote_endpoint_name, const Data& outgoing);

  /**
   * @brief 
   */
  bool isConnected() const;

  std::vector<std::string> endpoints() const;
  Endpoint::Ptr getEndpoint(const std::string& name) const;
protected:
  std::map<std::string, std::shared_ptr<Endpoint>> endpoints_;
  mutable std::mutex endpoint_mutex_;

  std::pair<std::string, Data> popBroadcast();
  bool haveBroadcast() const;

  std::vector<std::pair<std::string, Data>> broadcast_messages_;
  mutable std::mutex broadcast_message_mutex_;
};


}  // namespace scalopus
#endif  // SCALOPUS_INTERFACE_TRANSPORT_SERVER_H