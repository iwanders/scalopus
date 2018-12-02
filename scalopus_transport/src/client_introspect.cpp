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
#include <scalopus_transport/client_introspect.h>
#include <scalopus_transport/interface/transport_client.h>
#include <algorithm>
#include <iostream>

namespace scalopus
{

std::string ClientIntrospect::getName() const
{
  return "introspect";
}

std::vector<std::string> ClientIntrospect::supported()
{
  // send message...
  auto transport = transport_.lock();
  if (transport == nullptr)
  {
    std::cout << "No transport :( " << std::endl;
    return {};  // @todo(iwanders) probably better to throw...
  }

  std::vector<char> resp;
  std::vector<std::string> endpoints{""};
  if (transport->send(getName(), {'a'}, resp))
  {
    resp.pop_back();
    for (const auto z : resp)
    {
      if (z == '\n')
      {
        endpoints.push_back("");
        continue;
      }
      endpoints.back() += z;
    }
  }
  
  return endpoints;
}

}
