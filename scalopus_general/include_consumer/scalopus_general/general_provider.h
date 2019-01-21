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
#ifndef SCALOPUS_CATAPULT_GENERAL_PROVIDER_H
#define SCALOPUS_CATAPULT_GENERAL_PROVIDER_H

#include <scalopus_interface/endpoint_manager.h>
#include <scalopus_interface/trace_event_provider.h>
#include "scalopus_general/endpoint_process_info.h"

namespace scalopus
{
/**
 * @brief This provides the general data such as the process name and thread names.
 */
class GeneralProvider : public TraceEventProvider, public std::enable_shared_from_this<GeneralProvider>
{
public:
  using Ptr = std::shared_ptr<GeneralProvider>;
  using ProcessInfoMap = std::map<unsigned int, EndpointProcessInfo::ProcessInfo>;

  /**
   * @brief Create the general provider.
   * @param manager The endpoint manager that provides the endpoints to resolve the thread name and process name.
   */
  GeneralProvider(EndpointManager::Ptr manager);

  /**
   * @brief Return all currently known mappings.
   */
  ProcessInfoMap getMapping();

  /**
   * @brief Update the current mapping by retrieving the currently known maps from the endpoints.
   */
  void updateMapping();

  // From TraceEventProvider.
  TraceEventSource::Ptr makeSource();

private:
  EndpointManager::Ptr manager_;  //!< Manager for connections.

  std::mutex mapping_mutex_;  //!< Mutex for the mapping.
  ProcessInfoMap mapping_;    //!< The currently known mappings.
};

}  // namespace scalopus
#endif  // SCALOPUS_CATAPULT_GENERAL_PROVIDER_H
