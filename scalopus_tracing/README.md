# scalopus_tracing

As described in the main readme, scope tracepoints are identified by a single 32 bit integer. The mapping between the
integer and the provided string is maintained in a singleton, the `ScopeTraceTracker`. This mapping is exposed through
the `EndpointTraceMapping` endpoint.

## Backends

Two backends for handling the tracepoints themselves:
- `lttng`: Uses the lttng framework to get the tracepoints out of the process.
- `native`: Uses the plumbing provided by scalopus itself to get the tracepoints out of the process.

One can select at compile time against which backend to link. The backends merely define the following functions:
```cpp
scalopus::scope_entry(const unsigned int id);
scalopus::scope_exit(const unsigned int id);
```

You have to chose which backend you use at the time of linking your binary. This is done by linking to either:
- `Scalopus::scalopus_tracing_lttng` for the lttng tracepoints.
- `Scalopus::scalopus_tracing_native` for the native tracepoints.


## LTTng


To be able to receive the LTTng tracepoints, be sure to start a session of the appropriate name.
The [start](/scalopus_tracing/test/start), [stop](/scalopus_tracing/test/stop) and
[listen](/scalopus_tracing/test/listen) scripts provide some starting point for this. Only one viewer can be connected
to the live session. The catapult server starts babeltrace internally and parses the text output it produces.
We need this ascii conversion step because that's the [only implemented](https://github.com/efficios/babeltrace/blob/5223ed80d6517378def2da969c96b177ccc98e4d/formats/lttng-live/lttng-live-plugin.c#L325-L330)
output plugin for live sessions.



## How does a trace macro work?

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
std::integral_constant<uint32_t, scalopus_crcdetail::compute(__FILE__,sizeof(__FILE__) -1)>::value + __LINE__
```
Which guarantees a compile time constant CRC is calculated from the file name, to which we add the line number to ensure
we get a unique trace id.

