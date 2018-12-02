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

#include <thread>
#include <map>
#include <scalopus/interface/endpoint.h>
#include <scalopus/interface/provider.h>
#include <set>
#include <vector>
#include <utility>
#include "protocol.h"

namespace scalopus
{
/**
 * @brief The exposer class that is used to get the data about the trace mappings out of the proces.
 */
class ProviderUnix: public Provider
{
public:

  ProviderUnix();
  ~ProviderUnix();

  void addEndpoint(std::unique_ptr<Endpoint>&& endpoint);
private:
  std::thread thread_;
  void work();
  int server_fd_ { 0 };
  bool running_ { true };

  std::map<std::string, std::unique_ptr<Endpoint>> endpoints_;
  std::set<int> connections_;

  bool readData(int connection, size_t max_length, std::vector<char>& received);

  bool handleIncoming(int connection, protocol::Msg& request);

  bool processMsg(const protocol::Msg& request, protocol::Msg& response);
};


std::unique_ptr<Provider> providerUnixSocket();


}  // namespace scalopus
#endif  // SCALOPUS_PROVIDER_H