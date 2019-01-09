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
#include <iostream>
#include <nlohmann/json.hpp>
#include <scalopus_interface/transport.h>

namespace scalopus
{
using json = nlohmann::json;

std::string EndpointIntrospect::getName() const
{
  return name;
}

bool EndpointIntrospect::handle(Transport& server, const Data& /* request */, Data& response)
{
  const auto endpoints = server.endpoints();

  json jdata = json::object();
  jdata["endpoints"] = endpoints;
  response = json::to_bson(jdata);
  return true;
}

std::vector<std::string> EndpointIntrospect::supported()
{
  // send message...
  auto transport = transport_.lock();
  if (transport == nullptr)
  {
    throw communication_error("No transport provided to endpoint, cannot communicate.");
  }

  // Obtain the response data
  auto future_ptr = transport->request(name, {});
  if (future_ptr->wait_for(std::chrono::milliseconds(200)) == std::future_status::ready)
  {
    json jdata = json::from_bson(future_ptr->get());  // This line may throw
    return jdata["endpoints"].get<std::vector<std::string>>();
  }

  return {};
}

}  // namespace scalopus
