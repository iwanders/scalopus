
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

from . import tracing, general, transport
import sys

class DefaultExposer(object):
    """Default Exposer object that instantiates a server and all endpoints that
       are natively supported."""
    def __init__(self, process_name=sys.argv[0], transport_factory=transport.TransportUnixFactory()):
        """Constructs the exposer and sets up the server and endpoints.

        :param process_name: The process name to register to the processinfo endpoint. Defaults to sys.argv[0].
        :type process_name: str
        :param transport_factory: The transport factory to use for the exposer. Defaults to TransportUnixFactory.
        :type transport_factory: TransportFactory.
        """
        self.factory = transport_factory
        self.server = self.factory.serve()
        self.server.addEndpoint(general.EndpointIntrospect())
        processinfo = general.EndpointProcessInfo()
        processinfo.setProcessName(process_name)
        self.server.addEndpoint(processinfo)
        self.server.addEndpoint(tracing.EndpointTraceMapping())
        self.server.addEndpoint(tracing.EndpointNativeTraceSender())
