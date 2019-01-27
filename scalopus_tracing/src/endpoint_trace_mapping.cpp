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
#include <scalopus_tracing/endpoint_trace_mapping.h>
#include <scalopus_tracing/internal/scope_trace_tracker.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>

namespace scalopus
{
using json = nlohmann::json;

std::string EndpointTraceMapping::getName() const
{
  return name;
}

bool EndpointTraceMapping::handle(Transport& /* server */, const Data& request, Data& response)
{
  ProcessTraceMap mapping = { { ::getpid(), scalopus::ScopeTraceTracker::getInstance().getMap() } };
  // cool, we have the mappings... now we need to serialize this...

  if (request.front() == 'm')
  {
    json jdata = json::object();
    jdata["mapping"] = mapping;  // need to serialize an object, not an array.
    response = json::to_bson(jdata);
    return true;
  }
  return false;
}

EndpointTraceMapping::ProcessTraceMap EndpointTraceMapping::mapping()
{
  // send message...
  if (transport_ == nullptr)
  {
    throw communication_error("No transport provided to endpoint, cannot communicate.");
  }

  auto future_ptr = transport_->request(getName(), { 'm' });
  if (future_ptr->wait_for(std::chrono::milliseconds(200)) == std::future_status::ready)
  {
    ProcessTraceMap res;
    json jdata = json::from_bson(future_ptr->get());
    jdata["mapping"].get_to(res);
    return res;
  }

  return {};
}

EndpointTraceMapping::Ptr EndpointTraceMapping::factory(const Transport::Ptr& transport)
{
  auto endpoint = std::make_shared<scalopus::EndpointTraceMapping>();
  endpoint->setTransport(transport);
  return endpoint;
}

}  // namespace scalopus
