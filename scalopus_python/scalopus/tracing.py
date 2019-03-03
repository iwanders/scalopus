
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
getThreadState = tracing.getThreadState
setThreadState = tracing.setThreadState
getProcessState = tracing.getProcessState
setProcessState = tracing.setProcessState
MarkLevel = tracing.MarkLevel
EndpointTraceMapping = tracing.EndpointTraceMapping
EndpointNativeTraceSender = tracing.native.EndpointNativeTraceSender
NativeTraceProvider = tracing.native.NativeTraceProvider

# This function provides a new unique integer each time it is called.
# It is backed by an std::atomic_size_t on the C++ side.
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

class TraceContextHelper(object):
    """This is a helper to allow quick lookups of already created trace context managers.
    """
    def __init__(self, prefix=''):
        """Create the trace context helper.

        :param prefix: This prefix is prepended to the names given to the sections.
        :type prefix: str
        """
        self.ctx_type = TraceContext
        self.prefix = prefix

    def __getattr__(self, name):
        """Get the trace context if it exists, otherwise create it.

        :param name: The name of the trace context to retrieve or create.
        :type name: str
        """
        ctx = self.ctx_type(self.prefix + name)
        setattr(self, name, ctx)
        return ctx

    def __call__(self, name):
        """Create 

        :param name: The name of the trace context to retrieve or create.
        :type name: str
        """
        return getattr(self, name)

trace_section = TraceContextHelper()

class MarkerEvent(TraceContext):
    """This helper allows emitting marker events instead of scope traces.
    """
    def mark_event(self, level):
        """Emits a trace event at the provided level"""
        tracing_backend.mark_event(self.trace_id, level)
    def global_(self):
        """Emits a marker event at global level"""
        self.mark_event(tracing.MarkLevel.GLOBAL)
    def process(self):
        """Emits a marker event at process level"""
        self.mark_event(tracing.MarkLevel.PROCESS)
    def thread(self):
        """Emits a marker event at thread level"""
        self.mark_event(tracing.MarkLevel.THREAD)

trace_mark = TraceContextHelper()
trace_mark.ctx_type = MarkerEvent

def traced(f_or_name=None):
    """Decorator to trace the entire function execution. This decorator can be
    used in two ways. It can directly decorate a function, which will use the
    function's name as the tracepoint name:

        >>> @traced
        ... def work_func():
        ...     pass

    It can also be given an explicit tracepoint name which will then be used
    instead of the function's name:

        >>> @traced('work')
        ... def work_func():
        ...     pass

    The second variant is useful when decorating methods in a class.
    """
    # Determine the qualities of the passed argument
    f = f_or_name if callable(f_or_name) else None
    name = f_or_name if not callable(f_or_name) else None
    # registerer function that does the actual work of wrapping the function
    # with a trace context
    def registerer(f):
        tracer = TraceContext(name or f.__name__)
        @wraps(f)
        def wrapper(*args, **kwargs):
            with tracer:
                return f(*args, **kwargs)
        return wrapper
    # invoke the registerer or return it depending on whether the given argument
    # is callable or not
    return registerer(f) if callable(f) else registerer


class ThreadStateSwitcher:
    """Context manager to switch the thread state to the target state.

    To use this context manager:

        >>> with ThreadStateSwitcher(False):
        ...     work_func()

    There is also a convenience decorator available named `suppressed` which
    wraps a function in a switcher that temporarily sets the thread state to
    false.
    """
    def __init__(self, target_state):
        self.target_state = target_state
        self.old_states = None

    def enter(self):
        self.__enter__()

    def exit(self):
        self.__exit__(None, None, None)

    def __enter__(self):
        self.old_state = setThreadState(self.target_state)

    def __exit__(self, context_type, context_value, context_traceback):
        setThreadState(self.old_state)


def suppressed(f):
    """Convenience decorator that suppresses all trace points in the function
    and anything called from it.
    """
    supress_traces = ThreadStateSwitcher(False)
    @wraps(f)
    def wrapper(*args, **kwargs):
        with supress_traces:
            return f(*args, **kwargs)
    return wrapper
