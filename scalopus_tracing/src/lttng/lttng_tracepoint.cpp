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
#define TRACEPOINT_DEFINE
#define TRACEPOINT_CREATE_PROBES
#include <scalopus_tracing/internal/marker_tracepoint.h>
#include <scalopus_tracing/internal/scope_tracepoint.h>
#include <scalopus_tracing/trace_configurator.h>
#include "lttng/scope_tracepoint_lttng_definition.h"

namespace scalopus
{
namespace lttng
{
void scope_entry(const unsigned int id)
{
  static auto configurator_ptr = TraceConfigurator::getInstance();
  static auto process_state = configurator_ptr->getProcessStatePtr();
  thread_local auto thread_state = configurator_ptr->getThreadStatePtr();
  if (!(process_state->load() && thread_state->load()))
  {
    return;
  }
  tracepoint(scalopus_scope_id, scope_entry, id);
}

void scope_exit(const unsigned int id)
{
  static auto configurator_ptr = TraceConfigurator::getInstance();
  static auto process_state = configurator_ptr->getProcessStatePtr();
  thread_local auto thread_state = configurator_ptr->getThreadStatePtr();
  if (!(process_state->load() && thread_state->load()))
  {
    return;
  }
  tracepoint(scalopus_scope_id, scope_exit, id);
}

void mark_event(const unsigned int id, const MarkLevel mark_level)
{
  static auto configurator_ptr = TraceConfigurator::getInstance();
  static auto process_state = configurator_ptr->getProcessStatePtr();
  thread_local auto thread_state = configurator_ptr->getThreadStatePtr();
  if (!(process_state->load() && thread_state->load()))
  {
    return;
  }
  switch (mark_level)
  {
    case MarkLevel::GLOBAL:
      tracepoint(scalopus_scope_id, mark_event_global, id);
      break;
    case MarkLevel::PROCESS:
      tracepoint(scalopus_scope_id, mark_event_process, id);
      break;
    case MarkLevel::THREAD:
      tracepoint(scalopus_scope_id, mark_event_thread, id);
      break;
  }
}

}  // namespace lttng
}  // namespace scalopus