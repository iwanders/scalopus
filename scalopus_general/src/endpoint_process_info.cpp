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
#include <scalopus_general/endpoint_process_info.h>
#include <scalopus_general/internal/thread_name_tracker.h>
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>

#include <sys/types.h>
#include <unistd.h>

namespace scalopus
{
using json = nlohmann::json;

std::string EndpointProcessInfo::getName() const
{
  return name;
}

EndpointProcessInfo::EndpointProcessInfo()
{
  info_.pid = ::getpid();
}

void EndpointProcessInfo::setProcessName(const std::string& name)
{
  info_.name = name;
}

bool EndpointProcessInfo::handle(Transport& /* server */, const Data& request, Data& response)
{
  json req = json::from_bson(request);

  // Request is process name:
  if (req["cmd"].get<std::string>() == "info")
  {
    json jdata = json::object();
    jdata["pid"] = info_.pid;
    jdata["name"] = info_.name;
    jdata["threads"] = scalopus::ThreadNameTracker::getInstance().getMap();
    response = json::to_bson(jdata);
    return true;
  }
  return false;
}

EndpointProcessInfo::ProcessInfo EndpointProcessInfo::processInfo()
{
  // send message...
  if (transport_ == nullptr)
  {
    throw communication_error("No transport provided to endpoint, cannot communicate.");
  }

  ProcessInfo info;
  json request = json::object();
  request["cmd"] = "info";
  auto future_ptr = transport_->request(getName(), json::to_bson(request));

  if (future_ptr->wait_for(std::chrono::milliseconds(200)) == std::future_status::ready)
  {
    json jdata = json::from_bson(future_ptr->get());  // This line may throw
    jdata["name"].get_to(info.name);
    jdata["threads"].get_to(info.threads);
    jdata["pid"].get_to(info.pid);
  }

  return info;
}

}  // namespace scalopus
