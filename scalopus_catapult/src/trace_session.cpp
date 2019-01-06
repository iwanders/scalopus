/*
  Copyright (c) 2019, Ivor Wanders
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

#include "scalopus_catapult/trace_session.h"
#include <iostream>

namespace scalopus
{


TraceSession::TraceSession(ResponseFunction&& response_function) : response_(response_function)
{
  worker_ = std::thread([&]()
  {
    loop();
  });
}

TraceSession::~TraceSession()
{
  running_ = false;
  worker_.join();
}

void TraceSession::addSource(TraceEventSource::Ptr&& source)
{
  sources_.push_back(source);
}

void TraceSession::incoming(const std::string& data)
{
  std::lock_guard<decltype(incoming_mutex_)> lock(incoming_mutex_);
  incoming_msg_.push_back(data);
}

bool TraceSession::haveIncoming() const
{
  std::lock_guard<decltype(incoming_mutex_)> lock(incoming_mutex_);
  return !incoming_msg_.empty();
}

std::string TraceSession::popIncoming()
{
  std::lock_guard<decltype(incoming_mutex_)> lock(incoming_mutex_);
  auto res = incoming_msg_.front();
  incoming_msg_.pop_front();
  return res;
}

void TraceSession::loop()
{
  // do work.
  while (running_)
  {
    // Process incoming
    // handle logic.
    while(haveIncoming())
    {
      processMessage(popIncoming());
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  };
}

void TraceSession::processMessage(const std::string& incoming_msg)
{
  std::cout << "[session " << this <<  "] <- " << incoming_msg << std::endl;
  auto msg = json::parse(incoming_msg);

  if (msg["method"] == "Tracing.getCategories")
  {
    // This method is on the first record click, when we can specify the capture profile.
    // We can specify categories here, but the client does also show the default catapult categories.
    // So it's of limited use.
    json res = { { "id", msg["id"] },
                 { "result", { { "categories", { "!foo", "!bar", "disabled-by-default-buz" } } } } };
    outgoing(res.dump());
    return;
  }
}


void TraceSession::outgoing(const std::string& msg)
{
  std::cout << "[session " << this <<  "] -> " << msg << std::endl;
  response_(msg);
}


void TraceSession::startInterval()
{
}

void TraceSession::stopInterval()
{
}


}  // namespace scalopus
