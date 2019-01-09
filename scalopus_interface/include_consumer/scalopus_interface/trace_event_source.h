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

#ifndef SCALOPUS_INTERFACE_TRACE_EVENT_SOURCE_H
#define SCALOPUS_INTERFACE_TRACE_EVENT_SOURCE_H

#include <nlohmann/json.hpp>

namespace scalopus
{
using json = nlohmann::json;

/**
 * @brief A trace event source creates json representations of the trace events as used by catapult's trace viewer.
 * This format is detailed here: https://github.com/catapult-project/catapult/wiki/Trace-Event-Format
 */
class TraceEventSource
{
public:
  using Ptr = std::shared_ptr<TraceEventSource>;

  /**
   * @brief This function is called on each source when a recording interval is started.
   */
  virtual void startInterval();

  /**
   * @brief This function is called on each source when a recording interval is stopped.
   */
  virtual void stopInterval();

  /**
   * @brief This function should stop the interval and yield all events that were recorded during the interval for
   *        the frontend to consume.
   */
  virtual std::vector<json> finishInterval();

  /**
   * @brief This function is called periodically from the session thread.
   */
  virtual void work();

  virtual ~TraceEventSource() = default;
};
}  // namespace scalopus
#endif  // SCALOPUS_INTERFACE_TRACE_EVENT_SOURCE_H
