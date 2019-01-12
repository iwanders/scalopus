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

#ifndef SCALOPUS_CATAPULT_NATIVE_TRACE_PROVIDER_H
#define SCALOPUS_CATAPULT_NATIVE_TRACE_PROVIDER_H

#include <scalopus_lttng/scope_tracing_provider.h>
#include <scalopus_interface/endpoint.h>
#include <set>

namespace scalopus
{
/**
 * @brief This provider creates trace events from the native tracepoint collector endpoint.
 */
class NativeTraceSource;
class NativeTraceProvider : public ScopeTracingProvider, public std::enable_shared_from_this<NativeTraceProvider>
{
public:
  using Ptr = std::shared_ptr<NativeTraceProvider>;
  using WeakPtr = std::weak_ptr<NativeTraceProvider>;

  /**
   * @brief Create the lttng provider.
   * @param manager The endpoint manager that provides the endpoints to resolve the trace id's.
   */
  NativeTraceProvider(EndpointManager::Ptr manager);

  /**
   * @brief This function acts as the endpoint factory for the receiving end.
   */
  Endpoint::Ptr receiveEndpoint();

  // From TraceEventProvider.
  TraceEventSource::Ptr makeSource();

private:
  /**
   * @brief The receiving endpoint calls this method whenever it received unsolicited data.
   */
  void incoming(const Data& incoming);

  std::mutex source_mutex_;
  std::set<std::shared_ptr<NativeTraceSource>> sources_;
};

}  // namespace scalopus
#endif  // SCALOPUS_CATAPULT_NATIVE_TRACE_PROVIDER_H
