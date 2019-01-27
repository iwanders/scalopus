#/usr/bin/env python

import scalopus
import unittest

class MyEndpoint(scalopus.lib.Endpoint):
    def __init__(self):
        scalopus.lib.Endpoint.__init__(self)

    def handle(self, transport, incoming):
        print("PyHandle")
        return [66,2,3]

    def getName(self):
        return "MyEndpointName"

    def query_remote(self):
        response = self.getTransport().request(self.getName(), [1,3,3])
        result = response.wait_for(20)
        import time
        time.sleep(0.1)
        print("Result: {}".format(repr(result)))
        time.sleep(0.1)
        print(response)
        # print(response.get())


class PythonEndpoint(unittest.TestCase):

    def test_endpoint(self):
        # Create my endpoint.
        my_remote_endpoint = MyEndpoint()

        # create a server and add the endpoint to it.
        factory = scalopus.lib.transport.TransportLoopbackFactory()
        server = factory.serve()
        server.addEndpoint(my_remote_endpoint)

        # also add an introspect endpoint.
        server.addEndpoint(scalopus.lib.general.EndpointIntrospect())

        discovered_servers = factory.discover()
        self.assertEqual(len(discovered_servers), 1)

        client = factory.connect(discovered_servers[0])

        introspecter = scalopus.lib.general.EndpointIntrospect()
        client.addEndpoint(introspecter)

        remote_endpoints = introspecter.supported()
        print(remote_endpoints)
        self.assertEqual(remote_endpoints, ["MyEndpointName", "introspect"])

        # Try to add the endpoint on the client side, and use the query method.
        my_local_endpoint = MyEndpoint()
        client.addEndpoint(my_local_endpoint)
        my_local_endpoint.query_remote()
        


if __name__ == '__main__':
    unittest.main()