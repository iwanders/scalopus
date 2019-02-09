
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

from . import tracing, general, transport, common, interface, lib

import sys
import argparse
import time

def run_catapult_server(args):
    # Embedding catapult server
    factory = transport.TransportUnixFactory()
    poller = general.EndpointManagerPoll(factory)
    native_provider = tracing.native.NativeTraceProvider(poller)

    poller.addEndpointFactory(tracing.EndpointNativeTraceSender.name,
                              native_provider.factory)
    poller.addEndpointFactory(tracing.EndpointTraceMapping.name,
                              tracing.EndpointTraceMapping.factory)
    poller.addEndpointFactory(general.EndpointProcessInfo.name,
                              general.EndpointProcessInfo.factory)

    poller.startPolling(args.poll_interval)  # Start polling at interval.

    catapult = lib.catapult.CatapultServer()
    catapult.addProvider(native_provider)
    general_provider = general.GeneralProvider(poller)
    catapult.addProvider(general_provider)

    if tracing.have_lttng:
        lttng_provider = tracing.lttng.LttngProvider(args.lttng_session,
                                                     poller)
        catapult.addProvider(lttng_provider)

    # Finally, start the server on the desired port.
    catapult.start(port=args.port)

    try:
        while True:
            time.sleep(10)
    except KeyboardInterrupt:
        pass

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Scalopus, a tracing framework for C++ and Python."
        )

    subparsers = parser.add_subparsers(dest="command")

    # Catapult server, to create the devtools endpoint.
    catapult_server_help = ("Start a catapult server that allows connecting "
      "from chrome://inspect?tracing. It always uses the Unix transport, and "
      " initialises all providers supported natively by scalopus.")
    parser_catapult_server = subparsers.add_parser("catapult_server",
        help=catapult_server_help, description=catapult_server_help)
    parser_catapult_server.add_argument("-p", "--port", type=int,
        default=9222, help="The port to bind the webserver on. Defaults to "
        "%(default)s.")
    parser_catapult_server.add_argument("--poll-interval", type=float,
        default=1.0, help="The interval in seconds between server discovery."
        " Defaults to %(default)s.")

    if tracing.have_lttng:
        parser_catapult_server.add_argument("--lttng-session", type=str,
        default="scalopus_target_session",
        help="The lttng session name to connect to, defaults to %(default)s.")

    parser_catapult_server.set_defaults(func=run_catapult_server)

    args = parser.parse_args()

    # no command
    if (args.command is None):
        parser.print_help()
        parser.exit()
        sys.exit(1)

    args.func(args)
    sys.exit(0)
