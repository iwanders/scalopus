# scalopus_tracing

As described in the main readme, scope tracepoints are identified by a single 32 bit integer. The mapping between the
integer and the provided string is maintained in a singleton, the `ScopeTraceTracker`. This mapping is exposed through
the `EndpointTraceMapping` endpoint.

## Provided tracepoints
The currently supported tracepoints are scope entry and exit tracepoints. These are converted to duration events in the
[Trace Event Format][trace_event_format] that's displayed in the trace viewer.

The tracepoint macro's themselves can be found in
[scope_tracing.h](/scalopus_tracing/include/scalopus_tracing/scope_tracing.h), the following are available:

- `TRACE_TRACKED_RAII("name")` Places a [RAII][RAII] tracepoint in this scope, duration starts on custruction and ends
  when the tracepoint goes out of scope. Name of the duration is provided by the argument, trace id is based on line
  number and file.
- `TRACE_PRETTY_FUNCTION()` Uses the value of `__PRETTY_FUNCTION__` as defined by the preprocessor as name for the RAII
  tracepoint. Trace id is based on line number and file.
- `TRACE_SCOPE_START("name")` Starts a duration and uses the provided name for it. The trace id is based on the file
  name and the provided name.
- `TRACE_SCOPE_END("name")` Ends the duration started by the `TRACE_SCOPE_START` call with the same name. Trace id is
  based on the filename and the provided name. So this must be in the same file as its counterpart `TRACE_SCOPE_START`.


Fictitous code to demo their respective use cases:
```cpp
void my_function()
{
  TRACE_PRETTY_FUNCTION(); // name will be 'void my_function()'
  {
    std::lock_guard<std::mutex> lock(some_mutex_);
    TRACE_TRACKED_RAII("The mutex is locked");  // Constructed after we've acquired the mutex.
    work();  // takes long.
  }  // the RAII tracepoint goes out of scope.

  // The start and end tracepoints can be used like this:
  TRACE_SCOPE_START("foo and bar");  // start duration named "foo and bar"
  foo();
  int x = bar();
  TRACE_SCOPE_END("foo and bar");
  buz(x);

  // But in such a case it's preferable to use the RAII tracepoint:
  int x = 0;
  {
    TRACE_TRACKED_RAII("foo and bar");  // names can be reused, the trace_id is all that needs to be unique.
    foo();
    x = bar();
  }
  buz(x);
}
```

Note; Trace durations may interleave, but the trace viewer doesn't show this. Use the RAII tracepoints as much as
possible.


### How does a trace macro work?

Macro's are used for tracepoint insertion in the source code which makes them look reasonably pretty:
```cpp
void fooBarBuz()
{
  TRACE_PRETTY_FUNCTION();

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}
```

This macro will expand to something like this (put in some whitespace and comments):
```cpp
void fooBarBuz() {
  static bool scalopus_trace_id_0 = false;  // static bool to ensure we only store the mapping once.
  if (!scalopus_trace_id_0)
  {
    scalopus_trace_id_0 = true;
    scalopus::ScopeTraceTracker::getInstance().insert(trace_id, __PRETTY_FUNCTION__);  // mapping insert is thread safe
  }
  scalopus::TraceRAII scalopus_trace_id_1(trace_id);  // emits ENTRY on constructor, EXIT on destructor

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}
```
Where the `trace_id` should be replaced with the following:
```cpp
std::integral_constant<uint32_t, scalopus::crcdetail::compute(__FILE__,sizeof(__FILE__) -1)>::value + __LINE__
```
Which guarantees a compile time constant CRC is calculated from the file name, to which we add the line number to ensure
we get a unique trace id. The 32 bit CRC polynomial `crc-32` from Python's crcmod module is used, the generated code
was then modified to use a C++14 constexpr for loop. By wrapping the output of this into the template parameter of
`std::integral_constant` we know for sure that it is a compile time constant value.


## Backends

Two backends for handling the tracepoints themselves and one that disables tracepoints by default:
- `lttng`: Uses the lttng framework to get the tracepoints out of the process.
- `native`: Uses the transports provided by Scalopus itself to get the tracepoints out of the process.
- `nop`: These tracepoints don't do anything, this allows for disabling all tracepoints by default. Allows using
  `LD_PRELOAD` to swap in one of the other backends when desired.

One can select at compile time against which backend to link. The backends merely define the following functions:
```cpp
scalopus::scope_entry(const unsigned int id);
scalopus::scope_exit(const unsigned int id);
```

You can select which tracepoints your target uses by default by linking against one of:
- `Scalopus::scalopus_tracing_lttng` for the lttng tracepoints.
- `Scalopus::scalopus_tracing_native` for the native tracepoints.
- `Scalopus::scalopus_tracing_nop` for the 'no operation' tracepoints.


### LTTng

The [LTTng][lttng] tracepoints require `lttng-ust` and it's header files to be installed on the system. This component
is built optionally when `lttng-ust` is detected. The tracepoint definition as LTTng requires it is found in
[this](/scalopus_tracing/src/lttng/scope_tracepoint_lttng_definition.h) file, these are then
[used](/scalopus_tracing/src/lttng/lttng_tracepoint.cpp) inside the functions Scalopus requires from trace backends.

The LTTng tracepoints are very efficient and allow enabling and disabling tracepoints through LTTng's
[tracing control][tracing_control], which allows disabling and enabling per process id for example. Basically they
allow use of most of LTTng's features and this makes them very versatile and recommended for larger ecosystems in which
tracing is desired.

To be able to receive the LTTng tracepoints, be sure to start a session of the appropriate name
(`scalopus_target_session`).
The [start](/scalopus_tracing/test/start), [stop](/scalopus_tracing/test/stop) and
[listen](/scalopus_tracing/test/listen) scripts provide some starting point for this.

Good to know is that only one viewer can be connected to the live session, so only a single babeltrace process can be
connected to each live session. The catapult server starts babeltrace internally and parses the text output it produces.
We need this ascii conversion step because that's the [only implemented](https://github.com/efficios/babeltrace/blob/5223ed80d6517378def2da969c96b177ccc98e4d/formats/lttng-live/lttng-live-plugin.c#L325-L330)
output plugin for live sessions.


### Native

### No Operation


[trace_event_format]: https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/
[RAII]: https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization
[lttng]: https://lttng.org/
[tracing_control]: https://lttng.org/docs/v2.10/#doc-controlling-tracing