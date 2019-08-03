
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

import sys
import argparse
import time

def run_catapult_server(args):
    # Embedding catapult server
    factory = scalopus.transport.TransportUnixFactory()
    poller = scalopus.general.EndpointManagerPoll(factory)
    native_provider = scalopus.tracing.native.NativeTraceProvider(poller)

    poller.addEndpointFactory(scalopus.tracing.EndpointNativeTraceSender.name,
                              native_provider.factory)
    poller.addEndpointFactory(scalopus.tracing.EndpointTraceMapping.name,
                              scalopus.tracing.EndpointTraceMapping.factory)
    poller.addEndpointFactory(scalopus.general.EndpointProcessInfo.name,
                              scalopus.general.EndpointProcessInfo.factory)
    poller.addEndpointFactory(scalopus.tracing.EndpointTraceConfigurator.name,
                              scalopus.tracing.EndpointTraceConfigurator.factory)

    poller.startPolling(args.poll_interval)  # Start polling at interval.

    catapult = scalopus.lib.catapult.CatapultServer()
    catapult.addProvider(native_provider)
    general_provider = scalopus.general.GeneralProvider(poller)
    catapult.addProvider(general_provider)

    def logger(s):
        print(s)

    if (args.catapult_log):
        catapult.setLogger(logger)

    if (args.poll_log):
        poller.setLogger(logger)

    if scalopus.tracing.have_lttng:
        lttng_provider = scalopus.tracing.lttng.LttngProvider(args.lttng_session,
                                                     poller)
        catapult.addProvider(lttng_provider)

    # Finally, start the server on the desired port.
    catapult.start(port=args.port)

    try:
        while True:
            time.sleep(10)
    except KeyboardInterrupt:
        pass

def run_discover(args):
    factory = scalopus.transport.TransportUnixFactory()
    poller = scalopus.general.EndpointManagerPoll(factory)
    poller.addEndpointFactory(scalopus.general.EndpointProcessInfo.name,
                              scalopus.general.EndpointProcessInfo.factory)
    # Perform just one discovery round
    poller.manage()

    # Retrieve active endpoints
    endpoints = poller.endpoints()

    entries = []
    for transport, endpoint_map in endpoints.items():
        data = {}
        for name, endpoint in endpoint_map.items():
            if isinstance(endpoint, scalopus.general.EndpointProcessInfo):
                pinfo = endpoint.processInfo()
                data["pid"] = pinfo.pid
                data["process_info"] = pinfo.to_dict()
            if isinstance(endpoint, scalopus.general.EndpointIntrospect):
                data["supported"] = endpoint.supported()
        entries.append((data["pid"], data))

    entries.sort()

    for pid, data in entries:
        print("PID: {pid: >6d}  \"{name}\"".format(**data["process_info"]))
        doffset = " " * 13
        print(doffset + "Endpoints: {}".format(
              ("\n" + doffset + "  ").join([""] + sorted(data["supported"]))))
        threads = data["process_info"]["threads"]
        if (threads):
            print(doffset + "Threads:")
            for thread_id, thread_name in sorted(threads.items()):
                print(doffset + "  {}    \"{}\"".format(thread_id, thread_name))
        print()

def run_trace_configure(args):
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
    parser_catapult_server.add_argument("--no-poll-log", dest="poll_log",
        default=True, action="store_false",
        help="Disable log output for the endpoint poller.")
    parser_catapult_server.add_argument("--no-catapult-log",
        dest="catapult_log", default=True, action="store_false",
        help="Disable log output for the catapult server.")

    if scalopus.tracing.have_lttng:
        parser_catapult_server.add_argument("--lttng-session", type=str,
        default="scalopus_target_session",
        help="The lttng session name to connect to, defaults to %(default)s.")

    parser_catapult_server.set_defaults(func=run_catapult_server)


    discover_parser = subparsers.add_parser("discover", help="Discover "
                                            "processes and show short info"
                                            " about them.")
    discover_parser.set_defaults(func=run_discover)

    trace_configure_parser = subparsers.add_parser("trace_configure", help="Configure "
                                            " a processes' trace state.")
    trace_configure_parser.set_defaults(func=run_trace_configure)

    args = parser.parse_args()

    # no command
    if (args.command is None):
        parser.print_help()
        parser.exit()
        sys.exit(1)

    args.func(args)
    sys.exit(0)
