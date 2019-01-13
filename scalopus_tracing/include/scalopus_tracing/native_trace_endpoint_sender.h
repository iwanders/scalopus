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

#ifndef SCALOPUS_TRACING_NATIVE_TRACE_ENDPOINT_SENDER_H
#define SCALOPUS_TRACING_NATIVE_TRACE_ENDPOINT_SENDER_H

#include <scalopus_interface/transport.h>
#include <map>
#include <string>

namespace scalopus
{
class NativeTraceEndpointSender : public Endpoint
{
public:
  constexpr static const char* name = "native_tracepoint_sender";

  NativeTraceEndpointSender();
  ~NativeTraceEndpointSender();

  // From the endpoint
  std::string getName() const;
  bool handle(Transport& server, const Data& request, Data& response);
private:
  void work();
  bool running_ { true };
  std::thread worker_;
};

}  // namespace scalopus

#endif  // SCALOPUS_TRACING_ENDPOINT_NATIVE_TRACEPOINT_COLLECTOR_H
