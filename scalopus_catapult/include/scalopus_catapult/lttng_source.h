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

#ifndef SCALOPUS_CATAPULT_LTTNG_SOURCE_H
#define SCALOPUS_CATAPULT_LTTNG_SOURCE_H

#include <scalopus_lttng/babeltrace_tool.h>
#include "scalopus_catapult/lttng_provider.h"
#include "scalopus_catapult/trace_event_source.h"

namespace scalopus
{
/**
 * @brief The actual lttng source that provides the trace event format json entries.
 */
class LttngSource : public TraceEventSource
{
public:
  using Ptr = std::shared_ptr<LttngSource>;

  /**
   * @brief Constructor for this soure.
   * @param tool Pointer to the running babeltrace tool, a callback is registered with the tool that's called for each
   *        event.
   * @param provider The lttng provider that can be used to resolve the trace point names.
   */
  LttngSource(BabeltraceTool::Ptr tool, LttngProvider::Ptr provider);

  // from the TraceEventSource
  void startInterval();
  void stopInterval();
  void work();
  std::vector<json> finishInterval();

  ~LttngSource();

private:
  BabeltraceTool::Ptr tool_;     //!< Pointer to the babeltrace tool.
  LttngProvider::Ptr provider_;  //!< Pointer to the provider.

  //! Vector of the events of this interval, between start and stop the BabeltraceTool will freely write into this
  //! without locks.
  std::vector<CTFEvent> events_;

  std::shared_ptr<BabeltraceParser::EventCallback> callback_;  //!< Pointer to the registered event callback struct.

  /**
   * @brief Convert the stored events into the trace event format representation.
   */
  std::vector<json> convertEvents();
};
}  // namespace scalopus
#endif  // SCALOPUS_CATAPULT_LTTNG_SOURCE_H
