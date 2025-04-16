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
#include <scalopus_tracing/endpoint_trace_configurator.h>
#include <scalopus_tracing/trace_configurator.h>
#include <iostream>
#include <nlohmann/json.hpp>

namespace scalopus
{
using json = nlohmann::json;

const char* EndpointTraceConfigurator::name = "trace_configurator";

std::string EndpointTraceConfigurator::getName() const
{
  return name;
}

void to_json(json& j, const EndpointTraceConfigurator::TraceConfiguration& state)
{
  j["p"] = state.process_state;
  j["sp"] = state.set_process_state;
  j["nt"] = state.new_thread_state;
  j["snt"] = state.set_new_thread_state;
  j["t"] = state.thread_state;
}

void from_json(const json& j, EndpointTraceConfigurator::TraceConfiguration& state)
{
  j.at("p").get_to(state.process_state);
  j.at("sp").get_to(state.set_process_state);
  j.at("nt").get_to(state.new_thread_state);
  j.at("snt").get_to(state.set_new_thread_state);
  j.at("t").get_to(state.thread_state);
}

EndpointTraceConfigurator::TraceConfiguration
EndpointTraceConfigurator::setTraceState(const TraceConfiguration& state) const
{
  // send message...
  if (transport_ == nullptr)
  {
    throw communication_error("No transport provided to endpoint, cannot communicate.");
  }

  json request = json::object();
  request["cmd"] = "set";
  request["state"] = state;
  auto future_ptr = transport_->request(getName(), json::to_bson(request));

  if (future_ptr->wait_for(std::chrono::milliseconds(200)) == std::future_status::ready)
  {
    json jdata = json::from_bson(future_ptr->get());  // This line may throw
    auto new_state = jdata.at("state").get<TraceConfiguration>();
    new_state.cmd_success = true;
    return new_state;
  }
  return {};
}

EndpointTraceConfigurator::TraceConfiguration EndpointTraceConfigurator::getTraceState() const
{
  // send message...
  if (transport_ == nullptr)
  {
    throw communication_error("No transport provided to endpoint, cannot communicate.");
  }

  json request = json::object();
  request["cmd"] = "get";
  auto future_ptr = transport_->request(getName(), json::to_bson(request));

  if (future_ptr->wait_for(std::chrono::milliseconds(200)) == std::future_status::ready)
  {
    json jdata = json::from_bson(future_ptr->get());  // This line may throw
    auto new_state = jdata.at("state").get<TraceConfiguration>();
    new_state.cmd_success = true;
    return new_state;
  }
  return {};
}

bool EndpointTraceConfigurator::handle(Transport& /* server */, const Data& request, Data& response)
{
  json req = json::from_bson(request);
  auto configurator_instance = TraceConfigurator::getInstance();

  auto thread_map = configurator_instance->getThreadMap();
  auto process_state = configurator_instance->getProcessStatePtr();
  auto new_thread_state = configurator_instance->getNewThreadStatePtr();

  if (req.at("cmd").get<std::string>() == "set")
  {
    const auto new_state = req.at("state").get<TraceConfiguration>();
    // Store the new process state
    if (new_state.set_process_state)
    {
      process_state->store(new_state.process_state);
    }

    // Store the new thread state
    if (new_state.set_new_thread_state)
    {
      new_thread_state->store(new_state.new_thread_state);
    }

    // Iterate over the provided thread id's and try to set their state.
    for (const auto& k_v : new_state.thread_state)
    {
      auto it = thread_map.find(k_v.first);
      if (it != thread_map.end())
      {
        it->second->store(k_v.second);
      }
    }
  }

  // Now, create a response with the current state.
  json jdata = json::object();
  TraceConfiguration updated_state;

  // Store the process state
  updated_state.process_state = process_state->load();
  updated_state.new_thread_state =new_thread_state->load();

  // Store the thread state
  std::for_each(thread_map.begin(), thread_map.end(),
                [&](const auto& p) { updated_state.thread_state[p.first] = p.second->load(); });
  jdata["state"] = updated_state;
  response = json::to_bson(jdata);
  return true;
}

EndpointTraceConfigurator::Ptr EndpointTraceConfigurator::factory(const Transport::Ptr& transport)
{
  auto endpoint = std::make_shared<EndpointTraceConfigurator>();
  endpoint->setTransport(transport);
  return endpoint;
}

}  // namespace scalopus
