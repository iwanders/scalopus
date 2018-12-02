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
#include <scalopus_lttng/client_scope_tracing.h>
#include <scalopus_transport/interface/transport_client.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <cstring>

namespace scalopus
{

std::string ClientScopeTracing::getName() const
{
  return "scope_tracing";
}

std::map<unsigned int, std::string> ClientScopeTracing::mapping()
{
  // send message...
  auto transport = transport_.lock();
  if (transport == nullptr)
  {
    std::cout << "No transport :( " << std::endl;
    return {};  // @todo(iwanders) probably better to throw...
  }

  std::vector<char> resp;
  std::map<unsigned int, std::string> res;
  size_t resp_index = 0;
  if (transport->send(getName(), {'m'}, resp))
  {
    while (resp_index < resp.size())
    {
      // now we need to deserialize this...
      unsigned int id = *reinterpret_cast<unsigned int*>(&resp[resp_index]);
      resp_index += sizeof(id);
      std::string::size_type string_length = *reinterpret_cast<std::string::size_type*>(&resp[resp_index]);
      resp_index += sizeof(string_length);

      std::string name;
      name.resize(string_length);
      std::memcpy(&name[0], &resp[resp_index], string_length);
      resp_index += string_length;
      res[id] = name;
    }
  }

  return res;
}

}
