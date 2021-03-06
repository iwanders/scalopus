
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

from . import lib
"""
https://pybind11.readthedocs.io/en/master/advanced/classes.html#overriding-virtuals

Note that a direct __init__ constructor should be called, and super() should
not be used. For simple cases of linear inheritance, super() may work, but
once you begin mixing Python and C++ multiple inheritance, things will fall
apart due to differences between Python's MRO and C++'s mechanisms.
"""
class Endpoint(lib.interface.Endpoint):
    """Endpoint base class for Python endpoints.
    """
    def __init__(self):
        """This function must be called at all times by subclasses.
        """
        lib.interface.Endpoint.__init__(self)

    def handle(self, transport, incoming):
        """Function to handle requests coming from the transport.

            :param transport: The transport over which the data came in.
            :type transport: scalopus.interface.Transport
            :param incoming: The incoming data.
            :type incoming: Byte string (b"")
            :return: Response data if a response is to be sent.
            :rtype: None, list of integers, string or bytearray.
        """
        pass

    def getName(self):
        """Function that returns the name of the endpoint.
       
            :return: The name of this endpoint.
            :rtype: str
        """
        raise NotImplemented("getName must be implemented by the subclass.")

class TraceEventSource(lib.interface.TraceEventSource):
    """TraceEventSource base class for Python providers.
    """
    def __init__(self):
        """This function must be called at all times by subclasses.
        """
        lib.interface.TraceEventSource.__init__(self)

    def startInterval(self):
        """Function to start the recording interval.
            :return: None
            :rtype: None
        """
        pass

    def stopInterval(self):
        """Function to stop the recording interval.
            :return: None
            :rtype: None
        """
        pass

    def finishInterval(self):
        """Function to finish the recording interval and provide results.
            :return: List of dictionaries representing the events captured
                     between start and stop interval. In valid
                     Trace Event Format events.
            :rtype: list of dictionaries.
        """
        return []

class TraceEventProvider(lib.interface.TraceEventProvider):
    """TraceEventProvider base class for Python providers.
    """
    def __init__(self):
        """This function must be called at all times by subclasses.
        """
        lib.interface.TraceEventProvider.__init__(self)
    
    def makeSource(self):
        """Function that creates a new source for this provider.
       
            :return: New instance of the trace event source.
            :rtype: TraceEventSource
        """
        raise NotImplemented("makeSource must be implemented by the subclass.")
