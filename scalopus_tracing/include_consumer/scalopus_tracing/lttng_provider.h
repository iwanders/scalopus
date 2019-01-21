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
#ifndef SCALOPUS_CATAPULT_LTTNG_PROVIDER_H
#define SCALOPUS_CATAPULT_LTTNG_PROVIDER_H

#include <scalopus_interface/trace_event_provider.h>

#include <scalopus_general/endpoint_process_info.h>
#include <scalopus_tracing/babeltrace_tool.h>
#include <scalopus_tracing/ctfevent.h>
#include <scalopus_tracing/endpoint_trace_mapping.h>
#include <scalopus_tracing/scope_tracing_provider.h>

#include <scalopus_interface/endpoint_manager.h>

namespace scalopus
{
/**
 * @brief This is the provider for the lttng data source. It creates the babeltrace process and parses that. It also
 *        provides functionality to the sessions for resolving trace id's into scope names.
 */
class LttngProvider : public ScopeTracingProvider, public std::enable_shared_from_this<LttngProvider>
{
public:
  using Ptr = std::shared_ptr<LttngProvider>;

  /**
   * @brief Create the lttng provider.
   * @param path the path on which babeltrace's viewer will be called to obtain the traces.
   * @param manager The endpoint manager that provides the endpoints to resolve the trace id's.
   */
  LttngProvider(std::string path, EndpointManager::Ptr manager);

  // From TraceEventProvider.
  TraceEventSource::Ptr makeSource();

  ~LttngProvider();

private:
  BabeltraceTool::Ptr tracing_tool_;  //! Babeltrace tool, produces babeltrace sessions.
};

}  // namespace scalopus
#endif  // SCALOPUS_CATAPULT_LTTNG_PROVIDER_H
