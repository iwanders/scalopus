
# Copyright (c) 2018-2019, Ivor Wanders
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of the author nor the names of contributors may be used to
#   endorse or promote products derived from this software without specific
#   prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from .lib import tracing
import random
from functools import wraps


# Make some shorthands for convenience.
setTraceName = tracing.setTraceName
EndpointTraceMapping = tracing.EndpointTraceMapping
EndpointNativeTraceSender = tracing.native.EndpointNativeTraceSender
NativeTraceProvider = tracing.native.NativeTraceProvider

# Also for the tracing providers.
native = tracing.native
try:
    have_lttng = True
    lttng = tracing.lttng
except AttributeError as e:
    have_lttng = False

# Using this name here should allow swapping out the backend.
tracing_backend = native

def setBackend(new_backend):
    global tracing_backend
    tracing_backend = new_backend 


trace_id_cur = 0

def new_trace_id():
    global trace_id_cur
    result = trace_id_cur
    trace_id_cur += 1
    return result


class TraceContextHelper(object):
    def __init__(self, prefix=''):
        self.prefix = prefix

    def __getattr__(self, name):
        ctx = TraceContext(self.prefix + name)
        setattr(self, name, ctx)
        return ctx


class TraceContext(object):
    def __init__(self, name, trace_id=None):
        self.trace_id = trace_id if trace_id is not None else new_trace_id()
        self.name = name
        setTraceName(self.trace_id, self.name)

    def enter(self):
        self.__enter__()

    def exit(self):
        self.__exit__(None, None, None)

    def __enter__(self):
        tracing_backend.scope_entry(self.trace_id)

    def __exit__(self, context_type, context_value, context_traceback):
        tracing_backend.scope_exit(self.trace_id)

trace_sections = TraceContextHelper()

def trace_function(f):
    tracer = TraceContext(f.__name__)
    @wraps(f)
    def wrapper(*args, **kwargs):
        with tracer:
            return f(*args, **kwargs)
    return wrapper
