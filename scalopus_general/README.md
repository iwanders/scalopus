# scalopus_general

The general package provides several generic endpoints or functions. Additionally it contains some components that are
used by other packages.
- `EndpointIntrospect`: Allows querying which endpoints are available.
- `EndpointProcessInfo`: Provides information about thread names and the process' name.
- `EndpointManagerPoll`: A polling implementation of the `EndpointManager` as defined
  in the interface package.
- `GeneralProvider`: The `TraceEventProvider` that annotates the threads and gives the process the provided name.

## EndpointIntrospect
The [EndpointIntrospect](/scalopus_general/include/scalopus_general/endpoint_introspect.h) endpoint provides one method
that returns a list of supported endpoint names from the remote endpoint. The listed endpoints are those attached to the
remote transport.

## EndpointProcessInfo
The [EndpointProcessInfo](/scalopus_general/include/scalopus_general/endpoint_processinfo.h) provides the process name
and thread names.

Thread names are tracked by a singleton. The `TRACE_THREAD_NAME("name")` macro is provided in
[scope_tracing.h](/scalopus_general/include/scalopus_general/thread_naming.h). It uses the same method from the tracked
trace points to ensure the mapping is only stored once. The provided string does not need to be constant at compile
time, so it can be used as follows:
```cpp
// Create the threads
std::vector<std::thread> active_threads;
std::atomic_bool threads_running{ true };
for (size_t i = 0; i < thread_count; i++)
{
  active_threads.emplace_back([i, &threads_running, time_base]() {
    std::stringstream thread_name;
    thread_name << "Thread 0x" << std::hex << i;
    TRACE_THREAD_NAME(thread_name.str());
    while (threads_running.load())
    {
      random_callstack(0, time_base);
    }
  });
}
```

The process name can only be set by calling the `setProcessName` method on the endpoint. In general this is done like:
```cpp
auto endpoint_process_info = std::make_shared<scalopus::EndpointProcessInfo>();
endpoint_process_info->setProcessName(argv[0]);  // or any other std::string here.
server->addEndpoint(endpoint_process_info);
```

When used from the client side, the `processInfo()` method returns a struct from the endpoint containing the process
name as speicified, a map of its thread names and the process id.

## GeneralProvider and GeneralSource
The [`GeneralProvider`](/scalopus_general/include_consumer/scalopus_general/general_provider.h)  and 
[`GeneralSource`](/scalopus_general/include_consumer/scalopus_general/general_source.h) are the providers for the
thread names and process names they are part of the `scalopus_general_consumer` target. They merely obtain the process
information using `EndpointProcessInfo` and then create the appropriate trace event format events that contain the
necessary metadata. 

## EndpointManagerPoll
The [`EndpointManagerPoll`](/scalopus_general/include_consumer/scalopus_general/endpoint_manager_poll.h) class is part
of the `scalopus_general_consumer` target. During construction it takes a pointer to the transport factory and endpoint
factory functions can be registered. When `manage()` is called, or the polling is started it will periodically call the
discover function on the transport factory and try to connect to servers it isn't connected to yet. When a connection is
instantiated the `EndpointIntrospect` endpoint is used to determine which endpoints are available on the remote end. For
each available endpoint at the remote it tries to call the locally known factory function that's associated to the same
name.

It subclasses from the [`EndpointManager`](/scalopus_interface/include_consumer/scalopus_interface/endpoint_manager.h)
and can be used by the providers to retrieve information from processes. It also works with the loopback transport.

Providers have a pointer to the `EndpointManager`, if endpoints are to be created from a method the provider has, be
sure to capture a weak pointer to the provider to break the circular reference. One of the overloads of
`addEndpointFactory` in the base class does this, but if using this with a lambda from Python one may have to use a 
weakref.
