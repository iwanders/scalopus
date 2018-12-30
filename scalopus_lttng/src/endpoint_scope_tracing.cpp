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
#include <type_traits>
#include <iostream>

namespace scalopus
{
std::string EndpointScopeTracing::getName() const
{
  return name;
}

bool EndpointScopeTracing::handle(Transport& /* server */, const Data& request, Data& response)
{
  auto mapping = scalopus::ScopeTraceTracker::getInstance().getEntryExitMapping();
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


  std::vector<char> resp = transport->request(getName(), { 'm' }).get();
  if (!resp.empty())
  {
    return deserializeMapping(resp);
  }

  return {};
}

Data EndpointScopeTracing::serializeMapping(const std::map<unsigned int, std::string>& mapping)
{
  Data response;
  size_t resp_index = 0;
  response.resize(0);
  for (const auto& id_name : mapping)
  {
    const auto& id = id_name.first;
    const auto& name = id_name.second;

    // Pack the id number
    response.resize(response.size() + sizeof(id));
    *reinterpret_cast<std::remove_reference_t<decltype(mapping)>::key_type*>(&response[resp_index]) = id;
    resp_index += sizeof(id);

    // Pack the string length.
    auto string_length = name.size();
    response.resize(response.size() + sizeof(string_length));
    *reinterpret_cast<decltype(string_length)*>(&response[resp_index]) = string_length;
    resp_index += sizeof(string_length);

    // Finally, we write the string.
    response.resize(response.size() + string_length);
    std::memcpy(&response[resp_index], name.c_str(), string_length);
    resp_index += string_length;
  }
  return response;
}


std::map<unsigned int, std::string> EndpointScopeTracing::deserializeMapping(const Data& data)
{
  std::map<unsigned int, std::string> res;
  size_t resp_index = 0;
  while (resp_index < data.size())
  {
    // now we need to deserialize this...
    unsigned int id = *reinterpret_cast<const decltype(res)::key_type*>(&data[resp_index]);
    resp_index += sizeof(id);
    std::string::size_type string_length =
        *reinterpret_cast<const decltype(res)::mapped_type::size_type*>(&data[resp_index]);
    resp_index += sizeof(string_length);

    std::string name;
    name.resize(string_length);
    std::memcpy(&name[0], &data[resp_index], string_length);
    resp_index += string_length;
    res[id] = name;
  }
  return res;
}

}  // namespace scalopus
