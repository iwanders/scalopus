# scalopus_tracing


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