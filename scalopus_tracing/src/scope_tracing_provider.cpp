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

#include "scalopus_tracing/scope_tracing_provider.h"
#include <scalopus_general/endpoint_process_info.h>
#include <sstream>

namespace scalopus
{
ScopeTracingProvider::ScopeTracingProvider(EndpointManager::Ptr manager) : manager_{manager}
{
}

EndpointScopeTracing::ProcessTraceMap ScopeTracingProvider::getMapping()
{
  std::lock_guard<decltype(mapping_mutex_)> lock(mapping_mutex_);
  return mapping_;
}

void ScopeTracingProvider::updateMapping()
{
  // Make a new mapping
  EndpointScopeTracing::ProcessTraceMap mapping;

  // Get the current transports and their endpoints from the manager.
  auto endpoints_by_transport = manager_->endpoints();
  for (const auto& transport_endpoints : endpoints_by_transport)
  {
    // Try to find the scope tracing endpoint from this transports' endpoints and obtain its data.
    auto endpoint_scope_tracing =
        EndpointManager::findEndpoint<scalopus::EndpointScopeTracing>(transport_endpoints.second);

    if (endpoint_scope_tracing != nullptr)
    {
      // We found the correct endpoint, retrieve its mappings.
      const auto process_mapping = endpoint_scope_tracing->mapping();

      // Insert the mappings into the accumulated map.
      mapping.insert(process_mapping.begin(), process_mapping.end());
    }
  }

  // Under the lock, swap the old mapping with the new one.
  {
    std::lock_guard<decltype(mapping_mutex_)> lock(mapping_mutex_);
    mapping_.swap(mapping);
  }
}

std::string ScopeTracingProvider::getScopeName(const ProcessTraceMap& mapping, const unsigned int pid, const unsigned int trace_id)
{
  auto pid_info = mapping.find(pid);
  if (pid_info != mapping.end())
  {
    auto entry_mapping = pid_info->second.find(trace_id);
    if (entry_mapping != pid_info->second.end())
    {
      // yay! We found the appopriate mapping for this trace id.
      return entry_mapping->second;
    }
  }
  std::stringstream z;
  z << "Unknown 0x" << std::hex << trace_id;
  return z.str();
}

}  // namespace scalopus
