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

#ifndef SCALOPUS_CATAPULT_TRACE_SESSION_H
#define SCALOPUS_CATAPULT_TRACE_SESSION_H

#include "scalopus_catapult/trace_event_source.h"
#include <functional>
#include <string>
#include <thread>
#include <mutex>

namespace scalopus
{

class TraceSession
{
public:
  using ResponseFunction = std::function<bool(std::string)>;
  using Ptr = std::shared_ptr<TraceSession>;

  TraceSession(ResponseFunction&& response_function);
  void addSource(TraceEventSource::Ptr&& source);

  void incoming(const std::string& data);

  ~TraceSession();

private:
  void loop();
  void startInterval();
  void stopInterval();

  std::vector<TraceEventSource::Ptr> sources_;

  std::mutex incoming_mutex_;
  std::vector<std::string> incoming_msg_;
  std::thread worker_;
  ResponseFunction response_;
  bool running_ { false };
};

}
#endif  // SCALOPUS_CATAPULT_TRACE_SESSION_H
