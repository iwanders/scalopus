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
#include "scalopus_interface/transport.h"

namespace scalopus
{
bool Transport::isConnected() const
{
  return false;
}

void Transport::broadcast(const std::string& remote_endpoint_name, const Data& outgoing)
{
  std::lock_guard<std::mutex> lock(broadcast_message_mutex_);
  broadcast_messages_.emplace_back(remote_endpoint_name, outgoing);
}

std::pair<std::string, Data> Transport::popBroadcast()
{
  std::lock_guard<std::mutex> lock(broadcast_message_mutex_);
  std::pair<std::string, Data> tmp = std::move(broadcast_messages_.back());
  broadcast_messages_.pop_back();
  return tmp;
}

bool Transport::haveBroadcast() const
{
  std::lock_guard<std::mutex> lock(broadcast_message_mutex_);
  return !broadcast_messages_.empty();
}

void Transport::addEndpoint(const std::shared_ptr<Endpoint>& endpoint)
{
  std::lock_guard<std::mutex> lock(endpoint_mutex_);
  endpoints_[endpoint->getName()] = endpoint;
  endpoint->setTransport(this);
}

std::map<std::string, Endpoint::Ptr> Transport::endpoints() const
{
  std::lock_guard<std::mutex> lock(endpoint_mutex_);
  return endpoints_;
}

Endpoint::Ptr Transport::getEndpoint(const std::string& name) const
{
  std::lock_guard<std::mutex> lock(endpoint_mutex_);
  const auto it = endpoints_.find(name);
  if (it != endpoints_.end())
  {
    return it->second;
  }
  return nullptr;
}

void Transport::setLogger(LoggingFunction logger)
{
  logger_ = logger;
}
Destination::Ptr Transport::getAddress()
{
  return nullptr;
}

}  // namespace scalopus
