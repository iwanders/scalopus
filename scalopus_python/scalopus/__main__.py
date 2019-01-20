try:
    import scalopus_python_lib
except ModuleNotFoundError as e:
    print("{}\nWas the shared object in your PYTHONPATH variable?".format(str(e)))


fact = scalopus_python_lib.TransportUnixFactory()
x=fact.discover()
c = fact.connect(x[0])
print(c.isConnected())

introspecter = scalopus_python_lib.EndpointIntrospect()
c.addEndpoint(introspecter)
print(introspecter.supported())

mapping = scalopus_python_lib.EndpointTraceMapping()
c.addEndpoint(mapping)
print(mapping.mapping())


