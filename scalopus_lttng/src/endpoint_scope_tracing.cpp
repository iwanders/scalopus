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
#include <scalopus_lttng/endpoint_scope_tracing.h>
#include <scalopus_lttng/internal/scope_trace_tracker.h>
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>

namespace scalopus
{
using json = nlohmann::json;

std::string EndpointScopeTracing::getName() const
{
  return name;
}

bool EndpointScopeTracing::handle(Transport& /* server */, const Data& request, Data& response)
{
  auto mapping = scalopus::ScopeTraceTracker::getInstance().getMap();
  // cool, we have the mappings... now we need to serialize this...

  if (request.front() == 'm')
  {
    response = serializeMapping(mapping);
    return true;
  }
  return false;
}

std::map<unsigned int, std::string> EndpointScopeTracing::mapping()
{
  // send message...
  auto transport = transport_.lock();
  if (transport == nullptr)
  {
    throw communication_error("No transport provided to endpoint, cannot communicate.");
  }

  Data resp = transport->request(getName(), { 'm' })->get();
  if (!resp.empty())
  {
    return deserializeMapping(resp);
  }

  return {};
}

Data EndpointScopeTracing::serializeMapping(const std::map<unsigned int, std::string>& mapping)
{
  json jdata = json::object();
  jdata["mapping"] = mapping;  // need to serialize an object, not an array.
  return json::to_bson(jdata);
}

std::map<unsigned int, std::string> EndpointScopeTracing::deserializeMapping(const Data& data)
{
  std::map<unsigned int, std::string> res;
  json jdata = json::from_bson(data);  // This line may throw
  res = jdata["mapping"].get<decltype(res)>();
  return res;
}

}  // namespace scalopus
