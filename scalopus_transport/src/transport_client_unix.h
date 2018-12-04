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

#ifndef SCALOPUS_CONSUMER_H
#define SCALOPUS_CONSUMER_H

#include <scalopus_transport/interface/client.h>
#include <scalopus_transport/interface/transport_client.h>

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace scalopus
{
class TransportClientUnix : public TransportClient
{
public:
  TransportClientUnix();
  ~TransportClientUnix();
  bool connect(std::size_t pid, const std::string& suffix = "_scalopus");
  void disconnect();

  bool send(const std::string& remote_endpoint_name, const std::vector<char>& data, std::vector<char>& response);

  bool isConnected() const;

  static std::vector<std::size_t> getProviders(const std::string& suffix = "_scalopus");

private:
  int fd_{ 0 };
};

std::shared_ptr<TransportClient> transportClientUnix();
std::vector<size_t> getTransportServersUnix();

}  // namespace scalopus
#endif  // SCALOPUS_CONSUMER_H