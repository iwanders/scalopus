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

void TraceSession::loop()
{
  // do work.
  while (running_)
  {
    // Process incoming
    // handle logic.
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
  };
}

void TraceSession::startInterval()
{
}

void TraceSession::stopInterval()
{
}


}  // namespace scalopus
