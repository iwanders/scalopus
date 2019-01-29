#/usr/bin/env python

import scalopus
import unittest

class MyEndpoint(scalopus.lib.Endpoint):
    def __init__(self):
        scalopus.lib.Endpoint.__init__(self)
        self.reset()

    def reset(self):
        self.incoming = None

    def handle(self, transport, incoming):
        print("MyEndpoint::handle: {}".format(repr(incoming)))
        self.incoming = incoming
        return incoming

    def getName(self):
        return "MyEndpointName"

    def query_remote(self, request_data, wait_for=0.2):
        response = self.getTransport().request(self.getName(), request_data)
        result = response.wait_for(wait_for)
        return result


class PythonEndpoint(unittest.TestCase):

    def test_endpoint(self):
        # Create my endpoint.
        my_server_endpoint = MyEndpoint()
        my_client_endpoint = MyEndpoint()

        # create a server and add the endpoint to it.
        factory = scalopus.lib.transport.TransportLoopbackFactory()
        server = factory.serve()
        server.addEndpoint(my_server_endpoint)
        server.addEndpoint(scalopus.lib.general.EndpointIntrospect())

        # Connect a client
        discovered_servers = factory.discover()
        self.assertEqual(len(discovered_servers), 1)

        # Add the client endpoint.
        client = factory.connect(discovered_servers[0])
        client.addEndpoint(my_client_endpoint)

        # add a introspect endpoint.
        client_introspecter = scalopus.lib.general.EndpointIntrospect()
        client.addEndpoint(client_introspecter)

        remote_endpoints = client_introspecter.supported()
        self.assertEqual(remote_endpoints, ["MyEndpointName", "introspect"])

        # Try to add the endpoint on the client side, and use the query method.
        my_server_endpoint.reset()
        result = my_client_endpoint.query_remote(bytearray([1,2,3]))
        self.assertEqual(bytearray([1,2,3]), result)
        self.assertEqual(bytearray([1,2,3]), my_server_endpoint.incoming)

        # check if we can handle nullbytes.
        my_server_endpoint.reset()
        result = my_client_endpoint.query_remote([0xFF, 0, 5])
        self.assertEqual(result, bytearray([0xFF, 0, 5]))
        self.assertEqual(bytearray([0xFF, 0, 5]), my_server_endpoint.incoming)

        # Also check strings
        my_server_endpoint.reset()
        result = my_client_endpoint.query_remote(b"foobar")
        self.assertEqual(result, b"foobar")
        self.assertEqual(b"foobar", my_server_endpoint.incoming)

        # Incorrect type must throw, not crash.
        my_server_endpoint.reset()
        def will_throw():
            result = my_client_endpoint.query_remote(3)
        self.assertRaises(ValueError, will_throw)

if __name__ == '__main__':
    unittest.main()
