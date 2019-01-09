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

#include "scalopus_general/general_provider.h"
#include "scalopus_general/general_source.h"

#include <sstream>

namespace scalopus
{
GeneralProvider::GeneralProvider(EndpointManager::Ptr manager) : manager_(manager)
{
}

TraceEventSource::Ptr GeneralProvider::makeSource()
{
  return std::make_shared<GeneralSource>(shared_from_this());
}

GeneralProvider::ProcessInfoMap GeneralProvider::getMapping()
{
  std::lock_guard<decltype(mapping_mutex_)> lock(mapping_mutex_);
  return mapping_;
}

void GeneralProvider::updateMapping()
{
  std::lock_guard<decltype(mapping_mutex_)> lock(mapping_mutex_);
  mapping_.clear();

  auto endpoints = manager_->endpoints();
  for (const auto& transport_endpoints : endpoints)
  {
    // Try to find the scope tracing endpoint and obtain its data.
    auto endpoint_general = EndpointManager::findEndpoint<scalopus::EndpointProcessInfo>(transport_endpoints.second);
    if (endpoint_general != nullptr)
    {
      const auto process_mapping = endpoint_general->processInfo();
      mapping_[process_mapping.pid] = process_mapping;
    }
  }
}

}  // namespace scalopus
