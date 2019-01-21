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
#ifndef SCALOPUS_TRACING_INTERNAL_SCOPE_TRACING_H
#define SCALOPUS_TRACING_INTERNAL_SCOPE_TRACING_H

#include <scalopus_general/internal/helper_macros.h>
#include <scalopus_tracing/internal/compile_time_crc.hpp>

// Create a unique ID based on the crc32 of the filename and the line number.
#define SCALOPUS_TRACKED_TRACE_ID_CREATOR() (CRC32_STR(__FILE__) + __LINE__)

// Create a unique ID based on the crc32 of the filename and the provided string.
#define SCALOPUS_TRACKED_TRACE_ID_STRING(s) (CRC32_STR(__FILE__) + CRC32_STR(s))

// Macro that ensures that tracking the map between name and id is only performed once, this is achieved by using a
// static boolean.
#define TRACE_TRACKED_MAPPING_REGISTER_ONCE(name, id, have_done_setup_varname)                                         \
  static bool have_done_setup_varname = false;                                                                         \
  if (!have_done_setup_varname)                                                                                        \
  {                                                                                                                    \
    have_done_setup_varname = true;                                                                                    \
    scalopus::ScopeTraceTracker::getInstance().insert(id, name);                                                       \
  }

// Macro to create a tracked RAII tracepoint. The tracepoint itself will store only the ID, but the singleton trace
// tracker stores the ID -> name relation provided.
#define TRACE_TRACKED_RAII_ID(name, id)                                                                                \
  TRACE_TRACKED_MAPPING_REGISTER_ONCE(name, id, SCALOPUS_MAKE_UNIQUE(scalopus_trace_id_))                              \
  scalopus::TraceRAII SCALOPUS_MAKE_UNIQUE(scalopus_trace_id_)(id);                                                    \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)

#define TRACE_SCOPE_START_NAMED_ID(name, id)                                                                           \
  TRACE_TRACKED_MAPPING_REGISTER_ONCE(name, id, SCALOPUS_MAKE_UNIQUE(scalopus_trace_id_))                              \
  scalopus::scope_entry(id);                                                                                           \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)

#define TRACE_SCOPE_END_NAMED_ID(name, id) scalopus::scope_exit(id);                                                   \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)

#endif  // SCALOPUS_TRACING_INTERNAL_SCOPE_TRACING_H
