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

#ifndef SCALOPUS_INTERFACE_TRANSPORT_CLIENT_H
#define SCALOPUS_INTERFACE_TRANSPORT_CLIENT_H

#include <scalopus_transport/interface/client.h>
#include <memory>
#include <map>

namespace scalopus
{

class TransportClient
{
public:
  using WeakPtr = std::weak_ptr<TransportClient>;
  using Ptr = std::shared_ptr<TransportClient>;

  virtual bool send(const std::string& remote_endpoint_name, const std::vector<char>& outgoing, std::vector<char>& response) = 0;

  virtual ~TransportClient();
  virtual bool isConnected() const = 0;
  virtual void addClient(const std::shared_ptr<Client>& client);
protected:
  std::map<std::string, std::shared_ptr<Client>> clients_;
};


}  // namespace scalopus
#endif  // SCALOPUS_INTERFACE_TRANSPORT_CLIENT_H