#!/usr/bin/env python

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
import sys
import time

import scalopus
import scalopus.tracing as tracing

class PythonSource(scalopus.interface.TraceEventSource):
    def __init__(self):
        scalopus.interface.TraceEventSource.__init__(self)
        print("Made a PythonSource: {:x}".format(id(self)))

    def startInterval(self):
        print("The interval was started.")
        self.start_time = time.time()

    def stopInterval(self):
        print("The interval was stopped.")
        self.stop_time = time.time()

    def finishInterval(self):
        print("Interval from: {} - {}.".format(self.start_time, self.stop_time))
        # make a list to be filled with events.
        events = []
        # Iterate through time.
        t = self.start_time
        while t < self.stop_time:
            # Add start and stop duration events.
            events.append({"tid": "MyThread", "pid": "DataFromPythonSource",
                           "cat": "PERF","ph": "B","ts": t * 1e6, "name": 
                            "t: {:.2f}".format(t - self.start_time)})
            events.append({"tid": "MyThread", "pid": "DataFromPythonSource",
                           "cat": "PERF","ph": "E","ts": (t + 1) * 1e6, "name":
                            "t: {:.2f}".format(t - self.start_time)})
            t += 1.0
        # return the list of events to the C++ side.
        return events

    def __del__(self):
        print("Destroyed PythonSource: {:x}".format(id(self)))

class PythonProvider(scalopus.interface.TraceEventProvider):
    def __init__(self):
        scalopus.interface.TraceEventProvider.__init__(self)
        print("Made a PythonProvider: {:x}".format(id(self)))

    def makeSource(self):
        return PythonSource()

@tracing.traced
def fooBarBuz():
    time.sleep(0.2)

@tracing.traced
def c():
    time.sleep(0.2)
    print("  c")
    fooBarBuz()
    time.sleep(0.2)

@tracing.traced
def b():
    time.sleep(0.2)
    print(" b")
    c()
    time.sleep(0.2)

@tracing.traced
def a():
    print("a")
    time.sleep(0.2)
    b()
    time.sleep(0.2)

if __name__ == "__main__":
    factory = scalopus.transport.TransportLoopbackFactory()
    exposer = scalopus.common.DefaultExposer(process_name=sys.argv[0], transport_factory=factory)

    # Embedding catapult server
    poller = scalopus.general.EndpointManagerPoll(factory)
    native_provider = scalopus.tracing.native.NativeTraceProvider(poller)
    poller.addEndpointFactory(scalopus.tracing.EndpointNativeTraceSender.name, native_provider.factory)
    poller.addEndpointFactory(scalopus.tracing.EndpointTraceMapping.name, scalopus.tracing.EndpointTraceMapping.factory)
    poller.addEndpointFactory(scalopus.general.EndpointProcessInfo.name, scalopus.general.EndpointProcessInfo.factory)
    poller.manage()  # do one round of discovery

    catapult = scalopus.catapult.CatapultServer()
    my_python_provider = PythonProvider()
    catapult.addProvider(native_provider)
    catapult.addProvider(my_python_provider)
    catapult.addProvider(scalopus.general.GeneralProvider(poller))

    catapult.start(port=9222) # start the catapult server, defaults to 9222.

    # execution
    scalopus.general.setThreadName("main")

    while True:
        # fastest, one attribute lookup, name will be 'my_relevant_scope'
        with tracing.trace_section.my_relevant_scope:
            time.sleep(0.1)
            a()
        # Less fast then above, 1 method lookup and one call, allows spaces. Name will be "My Section"
        with tracing.trace_section("My Section"):
            time.sleep(0.1)
            a()
