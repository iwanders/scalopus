
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


try:
    import scalopus_python_lib as lib
except ModuleNotFoundError as e:
    print("{}\nWas the shared object in your PYTHONPATH variable?".format(str(e)))

foo = scalopus_lib.PyEndpoint()
print(dir(foo))
my_end = MyEndpoint()
print(dir(my_end))

fact = scalopus_lib.transport.TransportUnixFactory()

def server():
    import time
    my_end = MyEndpoint()
    ser = fact.serve()
    ser.addEndpoint(my_end)
    scalopus_lib.general.setThreadName("foo_thread")
    introspecter = scalopus_lib.general.EndpointIntrospect()
    ser.addEndpoint(introspecter)
    processinfo = scalopus_lib.general.EndpointProcessInfo()
    processinfo.setProcessName("MyPythonProcess")
    ser.addEndpoint(processinfo)
    mapping = scalopus_lib.tracing.EndpointTraceMapping()
    ser.addEndpoint(mapping)
    sender = scalopus_lib.tracing.native.EndpointNativeTraceSender()
    ser.addEndpoint(sender)

    scalopus_lib.tracing.setTraceName(0, "A")
    scalopus_lib.tracing.setTraceName(1, "B")
    def b():
        scalopus_lib.tracing.native.scope_entry(1)
        time.sleep(0.05)
        scalopus_lib.tracing.native.scope_exit(1)
        
    def a():
        scalopus_lib.tracing.native.scope_entry(0)
        time.sleep(0.1)
        b()
        time.sleep(0.1)
        scalopus_lib.tracing.native.scope_exit(0)
        

    try:
        while(True):
            time.sleep(0.5)
            a()
        
    except KeyboardInterrupt as e:
        pass
        

x = fact.discover()
if (len(x) == 0):
    server()
else:
    c = fact.connect(x[0])
    print(c.isConnected())

    introspecter = scalopus_lib.general.EndpointIntrospect()
    c.addEndpoint(introspecter)
    print(introspecter.supported())

    mapping = scalopus_lib.tracing.EndpointTraceMapping()
    c.addEndpoint(mapping)
    print(mapping.mapping())
    processinfo_retriever = scalopus_lib.general.EndpointProcessInfo()
    c.addEndpoint(processinfo_retriever)
    things = processinfo_retriever.processInfo()
    print(things.threads)
