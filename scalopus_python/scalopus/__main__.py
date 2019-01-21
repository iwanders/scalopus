try:
    import scalopus_python_lib as scalopus_lib
except ModuleNotFoundError as e:
    print("{}\nWas the shared object in your PYTHONPATH variable?".format(str(e)))

class MyEndpoint(scalopus_lib.PyEndpoint):
    def __init__(self):
        scalopus_lib.PyEndpoint.__init__(self)
        pass
    def getName(self):
        print("getname called")
        return "foo"


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
