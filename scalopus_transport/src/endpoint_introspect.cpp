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
#include "scalopus_transport/endpoint_introspect.h"
#include "scalopus_transport/interface/transport.h"
#include <iostream>

namespace scalopus
{
EndpointIntrospect::EndpointIntrospect()
{
}

std::string EndpointIntrospect::getName() const
{
  return "introspect";
}

bool EndpointIntrospect::handle(Transport& server, const Data& /* request */, Data& response)
{
  const auto endpoints = server.endpoints();
  for (const auto& name : endpoints)
  {
    response.insert(response.end(), name.begin(), name.end());
    response.push_back('\n');
  }
  return true;
}

std::vector<std::string> EndpointIntrospect::supported()
{
  // send message...
  auto transport = transport_.lock();
  if (transport == nullptr)
  {
    std::cout << "No transport :( " << std::endl;
    return {};  // @todo(iwanders) probably better to throw...
  }

  std::vector<std::string> endpoints{ "" };
  auto future = transport->request(getName(), {'a'});
  
  Data resp = future.get();

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

  return endpoints;
}

}  // namespace scalopus
