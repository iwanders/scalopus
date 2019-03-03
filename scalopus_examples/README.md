# scalopus_examples

## `readme_example.cpp`
The most important example is the [`readme_example.cpp`](/scalopus_examples/src/readme_example.cpp). This example shows
how the C++ tracepoints should be used and how the transport and endpoints at the process under test should be setup.

This example is built for all three tracing backends. Resulting in the following binaries:
  - `readme_example_native`
  - `readme_example_lttng`
  - `readme_example_nop`

## `random_callstacks.cpp`
This example is not really an example of how to use Scalopus, it is more an executable that can create artificial
callstacks at the rate and thread count specified. The maximum callstack depth will be ten.

It may accept arguments:
```
./random_callstacks_native [time_base (usec) [thread_count]]
Where time_base defaults to 10000 usec, resulting in scopes that have a self time of (2 + 2 + 1) * 10 ms = 50 ms.
The thread_count defaults to 1.
```
This example is built for all three tracing backends. Resulting in the following binaries:
  - `random_callstacks_native`
  - `random_callstacks_lttng`
  - `random_callstacks_nop`

The output from this example looks like:
![Random callstack example catapult output](/doc/random_callstacks_crop.png "Random callstack example catapult output")

View locally by opening  [`chrome://tracing`](chrome://tracing) and loading
[trace_random_callstacks.json.gz](/doc/trace_random_callstacks.json.gz).

## `embedded_catapult_server.cpp`
This shows how one could embed the catapult server in the process that produces the tracepoints. This example is only
built using the `native` tracing backend, because that's the only one that makes sense for such a usecase. I don't
expect this use case to be particularly useful, but it's possible.

## `query_servers.cpp`
This results in a binary that will connect to all discovered servers and obtain the data provided through the introspect
, process info and trace mapping endpoints. The binary outputs human readable text to `stderr` while it is working, at
the end it outputs the data it collected as pretty printed json to `stdout`.

## `showcase_toggle_tracepoints.cpp`
This showcase how one could use the `TRACING_CONFIG_THREAD_STATE_RAII` macro to disable tracepoints conditionally
depending on the code path taken.

## `showcase_marker_events.cpp`
This is an example of the marker events that span the global, process or thread scopes.
![Marker events](/doc/marker_event.png "Global and process marker events.")
