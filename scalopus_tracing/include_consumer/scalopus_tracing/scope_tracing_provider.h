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
#ifndef SCALOPUS_CATAPULT_SCOPE_TRACING_PROVIDER_H
#define SCALOPUS_CATAPULT_SCOPE_TRACING_PROVIDER_H

#include <scalopus_interface/endpoint_manager.h>
#include <scalopus_interface/trace_event_provider.h>
#include <scalopus_tracing/endpoint_trace_mapping.h>

namespace scalopus
{
/**
 * @brief This provider provides the generic aspects of a scope tracing provider.
 */
class ScopeTracingProvider : public TraceEventProvider
{
public:
  using Ptr = std::shared_ptr<ScopeTracingProvider>;
  using ProcessTraceMap = EndpointTraceMapping::ProcessTraceMap;

  /**
   * @brief Create the scope tracing provider.
   * @param manager The endpoint manager that provides the endpoints to resolve the trace id's.
   */
  ScopeTracingProvider(EndpointManager::Ptr manager);

  /**
   * @brief Return all currently known mappings.
   */
  EndpointTraceMapping::ProcessTraceMap getMapping();

  /**
   * @brief Update the current mapping by retrieving the currently known maps from the endpoints.
   */
  void updateMapping();

  /**
   * @brief Format the scope name using a mapping, prociess id and trace_id.
   * @param mapping The mapping as retrieved from the providers' getMapping() call.
   * @param pid The process id this trace id is associated with.
   * @param trace_id The id of the tracepoint encountered.
   */
  static std::string getScopeName(const ProcessTraceMap& mapping, const int pid, const unsigned int trace_id);

  /**
   * @brief Splits a string associated to a counter id into the counter and series name.
   * @param trace_string The string that was associated to the trace_id of this counter event.
   * @return The counter name and the series name in the counter.
   */
  static std::pair<std::string, std::string> splitCounterSeriesName(const std::string& trace_string);

private:
  EndpointManager::WeakPtr manager_;  //!< Manager for connections.

  std::mutex mapping_mutex_;  //!< Mutex for the mapping.
  ProcessTraceMap mapping_;   //!< The currently known mappings.
};

}  // namespace scalopus
#endif  // SCALOPUS_CATAPULT_SCOPE_TRACING_PROVIDER_H
