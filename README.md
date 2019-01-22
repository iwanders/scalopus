# Scalopus

This project provides a bridge to utilize [Catapult's Trace Viewer][catapult_trace_viewer] found inside Chrom(e/ium) to
display traces from an instrumented C++ program. This project was inspired by [this][cppcon_2016_quest_for_performance]
slide from the cppcon 2016 "Rainbow Six Siege: Quest for Performance" presentation.

The main focus is on obtaining traces for each scope / stack frame of interest, this provides good information about
where time is spent in the program. It requires instrumenting the source code of the program under inspection with
tracepoints to indicate which scopes need to be tracked. To get the tracepoints out of the program [LTTng][lttng] is
used preferably, minimal knowledge or interaction with LTTng is necessary to use Scalopus. An alternative, 
_experimental_ way is also provided to test scope tracing without having to install LTTng.

The trace viewer used is available in all recent Chrome and Chromium browsers and can be opened by typing 
[chrome://inspect?tracing][chrome_tracing] in the address bar. This is normally used to display traces from Android
or from within the browser itself. However, the trace viewer can also load traces from a remote target using the
[Devtools Protocol][devtools_protocol]'s tracing domain. The specification for the trace events of that domain can be
found in the [Trace Event Format][trace_event_format] documentation. Major benefit of using this interface is that
almost everyone has it already installed and can consume and view traces.

## Scope tracing
For a brief explanation what we mean by tracing a scope, watch one minute of [this][cppcon_2016_quest_for_performance]
video. After watching that video myself and doing some research I discovered that [LTTng][lttng] offers a way to obtain
traces through `-finstrument-functions` and the [provided][liblttng-ust-cyg-profile] library. This emits a tracepoint
for each function that was compiled with the instrumentation flag and the tracepoint itself contains the address of the
function. Using the object files you can then figure out which function was associated to that address. This is nice and
convenient but it quickly breaks down for non-trivial programs because of the amount of tracepoints that are produced or
because resolving the mapping between the function address and the function name becomes tricky.

To make this useful in a production environment we need to be able to manually specify which scopes we are interested
in and we need to be able to attach a human readable string to a scope that was opened or closed. Putting this string
into the tracepoint is possible, but this will result in a lot of duplicate data being sent through the tracing system
and this will be detrimental to performance. 

Scalopus aims to solve these problems by:
1. Tracepoints are placed manually in the source code of the program under test, the instrumentation step.
2. Scope tracepoints have a payload of just one 32 bit integer. This (opaque) number is an automatically generated
   (compile-time constant) 32 bit integer. The developer can specify a human readable string / name that is to be
   associated with this number.
3. Store this mapping between the tracepoint name and the tracepoint id and provide access to this mapping from outside
   of the process where the traces are being consumed.

To get the tracepoints out of the program LTTng is a good fit because it is performant and provides very little overhead
if tracepoints are not being recorded. Additionally it allows toggling tracepoints by process id and other niceties.

## Architecture
A picture is worth a thousand words, so here goes:

![Overview of Scalopus](/doc/overview.png "Overview of Scalopus")

The subcomponents of scalopus are clearly separated:
- [scalopus_interface](/scalopus_interface) Specifies the interfaces how the various components interact with each other.
- [scalopus_transport](/scalopus_transport) Provides two implementations of the `Transport` interface.
- [scalopus_general](/scalopus_general) This provides the process information endpoint, which allows naming the process and its threads.
- [scalopus_tracing](/scalopus_tracing) This provides means of tracing scopes and the `Provider` and `Source` to visualise them.
- [scalopus_catapult](/scalopus_catapult) Provides the chrome devtools protocol endpoint webserver that allows consuming the traces.
- [scalopus_examples](/scalopus_examples) This provides some examples on how to write instrumented source code.

### Interface
The interface specifies how the interaction between the components happens.

#### Endpoint
The Endpoint is a class instantiated at the server and client sides of the transport. An endpoint can interact with the transport and thus with the endpoints at the other end of a transport. Endpoints are stored by name and their name must be unique. The Endpoint interface is the same at both sides of the transport. If data comes in for an endpoint the transport will call the appropriate method, if part of a request (client initiated) this will be `handle`, sent by the server side it will call `unsolicited`. When an Endpoint's `handle` method is called it can immediately respond from the server' thread with a response.

#### TransportFactory
The TransportFactory provides an abstracted way of creating a server of a specific type, discovering other servers and returning a list of Destinations and creating a Transport that's connected to a certain Destination.

#### Transport
A Transport provides a means of storing a list of Endpoints and allowing those to communicate with the Transport and receive data from the Transport. On the server side the Endpoints can send data through the `broadcast` method, which sends the data to all connected clients. At the client side of the Transport the main way of interacting is with the `request` method, that sends a request and returns a `std::Future` that will be populated with the response.

#### Endpoint Manager
This interface is part of the `scalopus_consumer` target, it provides a way to query the available Endpoints from something that manages endpoints. This is necessary because the Providers need to be able to obtain the Endpoint in the catapult server.

#### TraceEventProvider
This interface is part of the `scalopus_consumer` target. The provider is a class that persists for the lifetime of the catapult server and has one method called `makeSource`, this method returns a TraceEventSource for tracing sessions to use.

#### TraceEventSource
This interface is part of the `scalopus_consumer` target. A source is created from its associated Provider, it is responsible for producing json representations of [traces][trace_event_format] that will be sent to the browser. The browser must start an interval, during which the source should collect traces. At the end of the interval the source must provide valid trace events ready for consumption by catapult's trace viewer. 

## Building

The three required dependencies are embedded in the `thirdparty` folder and use git submodules. Cmake 3.5.0 or higher is required and a compiler that supports C++14 features.

In order to use LTTng to get the traces out of the program under test one must install `liblttng-ust-dev` to consume
the traces from lttng the `babeltrace` package is required:
```bash
apt-get install liblttng-ust-dev babeltrace
```

Then, building should be as simple as:
```bash
# Clone repo, recursively to ensure git submodules are cloned as well.
git clone --recurse-submodules https://github.com/iwanders/scalopus
# Build:
mkdir build; cd build
cmake ../scalopus/
make -j8
# run tests with:
ctest .
```

## Quickstart
After building and succesfully being able to run the tests, use the following steps to view some tracepoints:
1. Run `./scalopus_examples/example_scope_tracepoints_random_native` to start a process that produces tracepoints.
2. Run `./scalopus_catapult/catapult_server`, this should output something like:
```
[main] Using port: 9222, 9222 is default, it is default remote debugging port
[main] Using path: ""  (empty defaults to lttng view scalopus_target_session)
[main] Everything started, falling into loop to detect transports. Use ctrl + c to quit.
[BabeltraceParser] Reached end of file, quiting parser function.
[scalopus] Creating transport to: <unix:8343>
```
3. Go go [chrome://inspect?tracing][chrome_tracing] (copy the link, clicking doesn't work), next to `Target (Scalopus Devtools Target)` click trace. You should now be in the tracing viewer and see `This about:tracing is connected to a remote device...` at the top. Click record, record, wait a bit and press stop.
4. Profit.

## Legal

- The Python bindings are produced using [Pybind11][pybind11]. This uses the BSD-3-clause [license](https://github.com/pybind/pybind11/blob/master/LICENSE).
- Json and bson handling is done with the [json for modern C++][nlohmann_json] library. This uses the MIT [license](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT).
- The webserver and websocket handling is done with [seasocks][seasocks]. This uses the BSD-2-clause [license](https://github.com/mattgodbolt/seasocks/blob/master/LICENSE)
- The Scalopus project itself is licensed under the BSD-3-clause [license](/LICENSE).
  





[catapult_trace_viewer]: https://github.com/catapult-project/catapult/blob/master/tracing/README.md
[catapult]: https://github.com/catapult-project/catapult
[devtools_protocol]: https://chromedevtools.github.io/devtools-protocol/tot/Tracing
[trace_event_format]: https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit
[lttng]: https://lttng.org/
[chrome_tracing]: chrome://inspect?tracing
[cppcon_2016_quest_for_performance]: https://youtu.be/tD4xRNB0M_Q?t=468
[liblttng-ust-cyg-profile]: https://lttng.org/docs/v2.10/#doc-liblttng-ust-cyg-profile
[pybind11]: https://github.com/pybind/pybind11
[nlohmann_json]: https://github.com/nlohmann/json
[seasocks]: https://github.com/mattgodbolt/seasocks/
