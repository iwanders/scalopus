#/usr/bin/env python

import sys
print("Running tests with Python: {}".format(sys.version))

import scalopus
import time
import os
import unittest


class PythonEndpoint(unittest.TestCase):
    def __init__(self, *args):
        unittest.TestCase.__init__(self, *args)
        # self.factory = scalopus.lib.transport.TransportLoopbackFactory()
        self.factory = scalopus.lib.transport.TransportUnixFactory()
        self.server = self.factory.serve()
        self.server.addEndpoint(scalopus.lib.general.EndpointIntrospect())
        processinfo = scalopus.lib.general.EndpointProcessInfo()
        processinfo.setProcessName("MyPythonProcess")
        self.server.addEndpoint(processinfo)
        self.server.addEndpoint(scalopus.lib.tracing.EndpointTraceMapping())
        self.server.addEndpoint(scalopus.lib.tracing.native.EndpointNativeTraceSender())

    def test_tracing(self):
        trace_point = scalopus.TraceContext("MyTraceContext")
        scalopus.lib.general.setThreadName("test_thread")
        for i in range(3):
            with trace_point:
                time.sleep(0.1)
            time.sleep(0.1)

        # now try to retrieve as much as possible.
        clients = self.factory.discover()
        self.assertEqual(len(clients), 1)
        client = self.factory.connect(clients[0])
        mapping_client = scalopus.lib.tracing.EndpointTraceMapping()
        client.addEndpoint(mapping_client)

        # check if the mappings were stored.
        mappings = mapping_client.mapping()
        pid = os.getpid()
        self.assertIn(pid, mappings)
        self.assertDictEqual({trace_point.trace_id: "MyTraceContext"}, mappings[pid])

        

if __name__ == '__main__':
    unittest.main()
