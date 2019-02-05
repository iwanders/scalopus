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
#ifndef SCALOPUS_TRACING_SCOPE_TRACING_H
#define SCALOPUS_TRACING_SCOPE_TRACING_H

#include <scalopus_tracing/internal/scope_trace_raii.h>
#include <scalopus_tracing/internal/scope_trace_tracker.h>
#include <scalopus_tracing/internal/scope_tracing.h>
#include <scalopus_tracing/internal/compile_time_crc.hpp>

/**
 * Public Macros
 * Tracked tracepoints store the actual tracepoint by ID, the name is tracked by the trace point tracking singleton.
 * The ID must be unique per tracing session, it can be automatically generated with the filename and line number.
 */

// Macro to create a tracker RAII tracepoint. The ID is automatically generated with the last part of the filename and
// the line number.
#define TRACE_SCOPE_RAII(name) TRACE_SCOPE_RAII_ID(name, SCALOPUS_TRACKED_TRACE_ID_CREATOR())

// Macro to create a traced RAII tracepoint using __PRETTY_FUNCTION__ as name.
#define TRACE_PRETTY_FUNCTION() TRACE_SCOPE_RAII_ID(__PRETTY_FUNCTION__, SCALOPUS_TRACKED_TRACE_ID_CREATOR())

// Macro to explicitly emit a start scope, needs to be paired with TRACE_SCOPE_END(name) with the same name.
#define TRACE_SCOPE_START(name) TRACE_SCOPE_START_NAMED_ID(name, SCALOPUS_TRACKED_TRACE_ID_STRING(name))

// Macro to explicitly emit a start scope, needs to be paired with TRACE_SCOPE_START(name) with the same name.
#define TRACE_SCOPE_END(name) TRACE_SCOPE_END_NAMED_ID(name, SCALOPUS_TRACKED_TRACE_ID_STRING(name))

#endif  // SCALOPUS_TRACING_SCOPE_TRACING_H
