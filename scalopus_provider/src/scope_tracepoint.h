/*
  Copyright (c) 2018, Ivor Wanders
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

#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER scalopus_entry_exit_id

#if !defined(_TRACEPOINT_scalopus_entry_exit_id_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TRACEPOINT_scalopus_entry_exit_id_H

#include <lttng/tracepoint.h>

TRACEPOINT_EVENT_CLASS(scalopus_entry_exit_id, entry_exit_id_class, TP_ARGS(unsigned int, id_),
                       TP_FIELDS(ctf_integer(unsigned int, id, id_)))

TRACEPOINT_EVENT_INSTANCE(scalopus_entry_exit_id, entry_exit_id_class, entry, TP_ARGS(unsigned int, id_))

TRACEPOINT_LOGLEVEL(scalopus_entry_exit_id, entry, TRACE_DEBUG_FUNCTION)

TRACEPOINT_EVENT_INSTANCE(scalopus_entry_exit_id, entry_exit_id_class, exit, TP_ARGS(unsigned int, id_))

TRACEPOINT_LOGLEVEL(scalopus_entry_exit_id, exit, TRACE_DEBUG_FUNCTION)

#endif /* _TRACEPOINT_scalopus_entry_exit_id_H */

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "scope_tracepoint.h"

/* This part must be outside ifdef protection */
#include <lttng/tracepoint-event.h>
