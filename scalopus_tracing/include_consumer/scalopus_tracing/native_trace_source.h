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

#ifndef SCALOPUS_TRACING_NATIVE_TRACE_SOURCE_H
#define SCALOPUS_TRACING_NATIVE_TRACE_SOURCE_H

#include <scalopus_interface/trace_event_source.h>
#include <scalopus_tracing/babeltrace_tool.h>
#include "scalopus_tracing/native_trace_provider.h"

namespace scalopus
{
/**
 * @brief The actual source that provides the trace event format json entries.
 */
class NativeTraceSource : public TraceEventSource
{
public:
  using Ptr = std::shared_ptr<NativeTraceSource>;
  using DataPtr = std::shared_ptr<Data>;

  /**
   * @brief Constructor for this soure.
   * @param provider The native trace provider that can be used to resolve the trace point names.
   */
  NativeTraceSource(NativeTraceProvider::WeakPtr provider);

  // from the TraceEventSource
  void startInterval();
  void stopInterval();
  void work();
  std::vector<json> finishInterval();

  ~NativeTraceSource();

  /**
   * @brief Return whether or not this source is currently in an recording interval.
   */
  bool isRecording() const;

  /**
   * @brief This is called by the server's receiving thread and contains chunks of incoming data.
   */
  void addData(const DataPtr& incoming_data);
private:
  NativeTraceProvider::WeakPtr provider_;  //!< Pointer to the provider.

  std::atomic_bool in_interval_ { false };

  std::mutex data_mutex_;
  std::vector<DataPtr> recorded_data_;
};
}  // namespace scalopus
#endif  // SCALOPUS_TRACING_NATIVE_TRACE_SOURCE_H
