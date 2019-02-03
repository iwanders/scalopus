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

import scalopus
import unittest

class MyEndpoint(scalopus.interface.Endpoint):
    def __init__(self):
        scalopus.interface.Endpoint.__init__(self)
        self.clear_incoming()

    def clear_incoming(self):
        self.incoming = None

    def handle(self, transport, incoming):
        print("MyEndpoint::handle: {}".format(repr(incoming)))
        self.incoming = incoming
        # return string, bytearray or list of integers containing the response
        # to be sent. If no response is to be sent, return None.
        return incoming

    def getName(self):
        # This function MUST be implemented.
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
        my_server_endpoint.clear_incoming()
        result = my_client_endpoint.query_remote(bytearray([1,2,3]))
        self.assertEqual(bytearray([1,2,3]), result)
        self.assertEqual(bytearray([1,2,3]), my_server_endpoint.incoming)

        # check if we can handle nullbytes.
        my_server_endpoint.clear_incoming()
        result = my_client_endpoint.query_remote([0xFF, 0, 5])
        self.assertEqual(result, bytearray([0xFF, 0, 5]))
        self.assertEqual(bytearray([0xFF, 0, 5]), my_server_endpoint.incoming)

        # Also check strings
        my_server_endpoint.clear_incoming()
        result = my_client_endpoint.query_remote(b"foobar")
        self.assertEqual(result, b"foobar")
        self.assertEqual(b"foobar", my_server_endpoint.incoming)

        # Incorrect type must throw, not crash.
        my_server_endpoint.clear_incoming()
        def will_throw():
            result = my_client_endpoint.query_remote(3)
        self.assertRaises(ValueError, will_throw)

if __name__ == '__main__':
    unittest.main()
