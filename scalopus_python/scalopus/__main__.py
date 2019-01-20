try:
    import scalopus_python_lib
except ModuleNotFoundError as e:
    print("{}\nWas the shared object in your PYTHONPATH variable?".format(str(e)))

class MyEndpoint(scalopus_python_lib.PyEndpoint):
    def __init__(self):
        scalopus_python_lib.PyEndpoint.__init__(self)
        pass
    def getName(self):
        print("getname called")
        return "foo"


fact = scalopus_python_lib.TransportUnixFactory()


def server():
    import time
    my_end = MyEndpoint()
    ser = fact.serve()
    ser.addEndpoint(my_end)
    introspecter = scalopus_python_lib.EndpointIntrospect()
    ser.addEndpoint(introspecter)

    time.sleep(200);

x = fact.discover()
if (len(x) == 0):
    server()
else:
    c = fact.connect(x[0])
    print(c.isConnected())

    introspecter = scalopus_python_lib.EndpointIntrospect()
    c.addEndpoint(introspecter)
    print(introspecter.supported())

    mapping = scalopus_python_lib.EndpointTraceMapping()
    c.addEndpoint(mapping)
    print(mapping.mapping())
