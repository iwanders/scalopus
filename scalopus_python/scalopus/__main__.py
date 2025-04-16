
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
import json
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
    for endpoint_map in endpoints.values():
        data = {}
        for endpoint in endpoint_map.values():
            if isinstance(endpoint, scalopus.general.EndpointProcessInfo):
                pinfo = endpoint.processInfo()
                data["pid"] = pinfo.pid
                data["process_info"] = pinfo.to_dict()
            if isinstance(endpoint, scalopus.general.EndpointIntrospect):
                data["supported"] = endpoint.supported()
        entries.append((data["pid"], data))

    entries.sort()

    for _, data in entries:
        print("PID: {pid: >6d}  \"{name}\"".format(**data["process_info"]))
        doffset = " " * 13
        print(doffset + "Endpoints: {}".format(
              ("\n" + doffset + "  ").join([""] + sorted(data["supported"]))))
        threads = data["process_info"]["threads"]
        if threads:
            print(doffset + "Threads:")
            for thread_id, thread_name in sorted(threads.items()):
                print(doffset + "  {}    \"{}\"".format(thread_id, thread_name))
        print()

def run_record(args):
    factory = scalopus.transport.TransportUnixFactory()

    poller = scalopus.general.EndpointManagerPoll(factory)
    native_provider = scalopus.tracing.native.NativeTraceProvider(poller)
    general_provider = scalopus.general.GeneralProvider(poller)
    poller.addEndpointFactory(scalopus.tracing.EndpointNativeTraceSender.name, native_provider.factory)
    poller.addEndpointFactory(scalopus.tracing.EndpointTraceMapping.name, scalopus.tracing.EndpointTraceMapping.factory)
    poller.addEndpointFactory(scalopus.general.EndpointProcessInfo.name, scalopus.general.EndpointProcessInfo.factory)
    poller.startPolling(1.0)

    native_source = native_provider.makeSource()
    general_source = general_provider.makeSource()
    native_source.startInterval()
    general_source.startInterval()

    try:
        while True:
            time.sleep(10)
    except KeyboardInterrupt:
        pass

    native_source.stopInterval()
    general_source.stopInterval()
    poller.stopPolling()

    print('[')

    try:
        data = native_source.finishInterval()
        data.extend(general_source.finishInterval())

        # print(data) prints strings with ', but Chrome's Catapult viewer only accepts ".
        # python3 is also needed, so strings don't start with u, and integers don't end with L.
        # print(json.dumps(data)) gets the job done, but it prints everything in a single line.
        print(*[json.dumps(entry) for entry in data], sep=',\n', end='')
    except RuntimeError:
        pass

    print(']', end='')

def run_trace_configure(args):
    factory = scalopus.transport.TransportUnixFactory()
    poller = scalopus.general.EndpointManagerPoll(factory)
    poller.addEndpointFactory(scalopus.tracing.EndpointTraceConfigurator.name,
                              scalopus.tracing.EndpointTraceConfigurator.factory)
    poller.addEndpointFactory(scalopus.general.EndpointProcessInfo.name,
                              scalopus.general.EndpointProcessInfo.factory)
    # Perform just one discovery round
    poller.manage()

    # perform manipulations as requested.
    relevant_ids = set(args.id)
    new_trace_state = args.state == "on"
    new_unmatched_trace_state = args.unmatched_pid == "on"

    # Retrieve active endpoints
    endpoints = poller.endpoints()
    entries = []
    for transport, endpoint_map in endpoints.items():
        if not scalopus.tracing.EndpointTraceConfigurator.name in endpoint_map:
            continue

        data = {}
        pinfo = endpoint_map[scalopus.general.EndpointProcessInfo.name].processInfo()
        data["pid"] = pinfo.pid
        data["process_info"] = pinfo.to_dict()

        new_state = scalopus.tracing.EndpointTraceConfigurator.TraceConfiguration()


        if (pinfo.pid in relevant_ids):
            if args.new_thread:
                new_state.set_new_thread_state = True
                new_state.new_thread_state = new_trace_state
            else:
                new_state.set_process_state = True
                new_state.process_state = new_trace_state
        else:
            if args.unmatched_pid:
                if args.new_thread:
                    new_state.set_new_thread_state = True
                    new_state.new_thread_state = new_unmatched_trace_state
                else:
                    new_state.set_process_state = True
                    new_state.process_state = new_unmatched_trace_state

        for thread_id in pinfo.threads.keys():
            if thread_id in relevant_ids:
                new_state.add_thread_entry(thread_id, new_trace_state)

        state = endpoint_map[scalopus.tracing.EndpointTraceConfigurator.name].setTraceState(new_state)
        if not state.cmd_success:
            print("Failed to retrieve state for transport: {}".format(transport.getAddress()))
            continue
        data["state"] = state.to_dict()

        entries.append((data["pid"], data))
    entries.sort()

    def color_by_state(str, enabled):
        if enabled:
            return '\033[92m' + str + '\033[0m'
        else:
            return '\033[94m' + str + '\033[0m'

    for pid, data in entries:
        pid_str = color_by_state("{: >6d}".format(pid), data["state"]["process_state"])
        new_threads_str = color_by_state("new_threads", data["state"]["new_thread_state"])
        print("PID: {}  \"{}\" [{}]".format(pid_str, data["process_info"]["name"], new_threads_str))
        doffset = " " * 8
        threads = data["process_info"]["threads"]
        for thread_id, thread_name in sorted(threads.items()):
            thread_idstr  = str(thread_id)
            if thread_id in data["state"]["thread_state"]:
                thread_idstr = color_by_state(thread_idstr, data["state"]["thread_state"][thread_id])
            print(doffset + "  {}    \"{}\"".format(thread_idstr, thread_name))
        print()



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

    record_parser = subparsers.add_parser("record", help="Record traces and dump them in json format.")
    record_parser.set_defaults(func=run_record)

    trace_configure_parser = subparsers.add_parser("trace_configure",
        help="Configure the trace state.")
    trace_configure_parser.add_argument('state', choices=['on', 'off'],
        default=None, nargs="?", help="Matched id's will be set to this state.")
    trace_configure_parser.add_argument('-u','--unmatched-pid',default=None,
        choices=['on', 'off'], nargs="?",
        help="Set unmatched process id's state to this value.")
    
    trace_configure_parser.add_argument('-t','--new-thread',default=False,
        action="store_true",help="Set the value for new threads instead of current threads/process.")
    
    trace_configure_parser.add_argument("id", nargs="*", type=int,
        help="Process or thread ID to change.")

    trace_configure_parser.set_defaults(func=run_trace_configure)

    args = parser.parse_args()

    # no command
    if (args.command is None):
        parser.print_help()
        parser.exit()
        sys.exit(1)

    args.func(args)
    sys.exit(0)
