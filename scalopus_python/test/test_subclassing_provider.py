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
import gc
import time

import scalopus
import scalopus.tracing as tracing
scalopus.lib.lib.test_helpers.clearTraceNames()

overload_called = False
class PythonSource(scalopus.interface.TraceEventSource):
    def __init__(self):
        scalopus.interface.TraceEventSource.__init__(self)
        print("Made a PythonSource: {}".format(id(self)))

    def startInterval(self):
        print("The interval was started")

    def stopInterval(self):
        print("The interval was stopped")

    def finishInterval(self):
        global overload_called
        print("Overload called")
        overload_called = True
        return []

    # Having a custom destructor here with a print seems to cause deadlock with GCC only.
    #def __del__(self):
    #   print("Delete is called on: {}".format(id(self)))

class PythonProvider(scalopus.interface.TraceEventProvider):
    def __init__(self):
        scalopus.interface.TraceEventProvider.__init__(self)
        print("Made a PythonProvider: {}".format(id(self)))

    def makeSource(self):
        return PythonSource()


class PythonEndpoint(unittest.TestCase):

    def foo(self):
        spawner = scalopus.lib.lib.test_helpers.PythonSubclasserSpawner()
        my_provider = PythonProvider()
        spawner.addProvider(my_provider)
        print("make:")
        spawner.makeSourceFrom()
        print("call:")
        gc.collect()
        spawner.call()
        gc.collect()
        # check that we don't segfault if destruction happens from another
        # thread without that throwing owning the GIL.
        print("Staging destruction from another thread.")
        spawner.stage_destroy(50) # stage 50 milliseconds
        for i in range(200):
            time.sleep(0.01)
            print(".", end="")
        spawner.join()

    def test_endpoint(self):
        self.foo()
        global overload_called
        self.assertTrue(overload_called)

        


if __name__ == '__main__':
    unittest.main()
