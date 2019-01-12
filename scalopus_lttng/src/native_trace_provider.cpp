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

#include "scalopus_lttng/native_trace_provider.h"
#include "scalopus_lttng/native_trace_source.h"
#include "native_trace_endpoint_receive.h"

#include <nlohmann/json.hpp>
#include <sstream>

namespace scalopus
{
using json = nlohmann::json;
NativeTraceProvider::NativeTraceProvider(EndpointManager::Ptr manager) : ScopeTracingProvider{manager}
{
}

TraceEventSource::Ptr NativeTraceProvider::makeSource()
{
  auto source = std::make_shared<NativeTraceSource>(shared_from_this());
  std::lock_guard<decltype(source_mutex_)> lock(source_mutex_);
  sources_.insert(source);
  return source;
}

Endpoint::Ptr NativeTraceProvider::receiveEndpoint()
{
  return std::make_shared<NativeTraceEndpointReceive>([provider = WeakPtr{shared_from_this()}](const Data& data) {
    // This function is called from the server thread
    auto ptr = provider.lock();
    if (ptr)
    {
      ptr->incoming(data);
    }
  });
}

void NativeTraceProvider::incoming(const Data& incoming)
{
  std::set<NativeTraceSource::Ptr> recording_sources;
  {
    std::lock_guard<decltype(source_mutex_)> lock(source_mutex_);
    if (sources_.empty())
    {
      return;  // no sources active, bail out.
    }

    for (auto& source : sources_)
    {
      if (source->isRecording())
      {
        recording_sources.insert(source);
      }
    }
  }

  if (recording_sources.empty())
  {
    return;  // no sources are recording.
  }

  // Ensure we only make one copy of the data from the server thread.
  auto data = std::make_shared<Data>(incoming);
  for (auto& source : recording_sources)
  {
    source->addData(data);
  }
}

}  // namespace scalopus
