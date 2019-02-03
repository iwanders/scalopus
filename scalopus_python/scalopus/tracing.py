
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
uniqueTraceId = tracing.uniqueTraceId

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
    """Sets the backend to use by all tracepoints.

    :param new_backend: The backend to use for trace points.
    :type new_backend: Tracing backend (tracing.native, tracing.lttng or tracing.nop).
    """
    global tracing_backend
    tracing_backend = new_backend 

class TraceContextHelper(object):
    """This is a helper to allow quick lookups of already created trace context managers.
    """
    def __init__(self, prefix=''):
        """Create the trace context helper.

        :param prefix: This prefix is prepended to the names given to the sections.
        :type prefix: str
        """
        self.prefix = prefix

    def __getattr__(self, name):
        """Get the trace context if it exists, otherwise create it.

        :param name: The name of the trace context to retrieve or create.
        :type name: str
        """
        ctx = TraceContext(self.prefix + name)
        setattr(self, name, ctx)
        return ctx

    def __call__(self, name):
        """Create 

        :param name: The name of the trace context to retrieve or create.
        :type name: str
        """
        return getattr(self, name)

trace_section = TraceContextHelper()

class TraceContext(object):
    """This provides a traced context, tracing the duration of a with statements.
    """
    def __init__(self, name, trace_id=None):
        """Sets the name and optionally the trace id to be used for this context.

        :param name: Sets the name that will be associated to the trace id.
        :type name: str
        :param trace_id: The trace id to use.
        :type trace_id: int
        """
        self.trace_id = trace_id if trace_id is not None else uniqueTraceId()
        self.name = name
        setTraceName(self.trace_id, self.name)

    def enter(self):
        self.__enter__()

    def exit(self):
        self.__exit__(None, None, None)

    def __enter__(self):
        """Enter the with statement, this emits the entry tracepoint.
        """
        tracing_backend.scope_entry(self.trace_id)

    def __exit__(self, context_type, context_value, context_traceback):
        """Exit the with statement, this emits the exit tracepoint.
        """
        tracing_backend.scope_exit(self.trace_id)

def trace_function(f):
    """Decorator to trace the entire function execution. Name used for the
       tracepoint is the function name.
    """
    tracer = TraceContext(f.__name__)
    @wraps(f)
    def wrapper(*args, **kwargs):
        with tracer:
            return f(*args, **kwargs)
    return wrapper
