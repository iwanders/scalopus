# Interface
The interface specifies how the interaction between the components in scalopus happens. This is the 'root' package of
the project.

## Components needed by process under test
These interfaces must be exposed to the process under test, they make the transport and the enpoints. These interfaces only make use of the standard library.

### Endpoint
The Endpoint is a class instantiated at the server and client sides of the transport. An endpoint can interact with the transport and thus with the endpoints at the other end of a transport. Endpoints are stored by name and their name must be unique. The Endpoint interface is the same at both sides of the transport. If data comes in for an endpoint the transport will call the appropriate method, if part of a request (client initiated) this will be `handle`, sent by the server side it will call `unsolicited`. When an Endpoint's `handle` method is called it can immediately respond from the server' thread with a response.

### TransportFactory
The TransportFactory provides an abstracted way of creating a server of a specific type, discovering other servers and returning a list of Destinations and creating a Transport that's connected to a certain Destination.

### Transport
A Transport provides a means of storing a list of Endpoints and allowing those to communicate with the Transport and receive data from the Transport. On the server side the Endpoints can send data through the `broadcast` method, which sends the data to all connected clients. At the client side of the Transport the main way of interacting is with the `request` method, that sends a request and returns a `std::future` that will be populated with the response.

## Components needed by the consumers

These interface are necessary on the consumer side, so in the catapult server or elsewhere were we consume the traces and combine it with the trace mappings to produce data representations. These interfaces expose the [json library][nlohmann_json] in their headers because it is used as a return type.

### Endpoint Manager
This interface is part of the `scalopus_consumer` target, it provides a way to query the available Endpoints from something that manages endpoints. This is necessary because the Providers need to be able to obtain the Endpoint in the catapult server.

### TraceEventProvider
This interface is part of the `scalopus_consumer` target. The provider is a class that persists for the lifetime of the catapult server and has one method called `makeSource`, this method returns a TraceEventSource for tracing sessions to use.

### TraceEventSource
This interface is part of the `scalopus_consumer` target. A source is created from its associated Provider, it is responsible for producing json representations of [traces][trace_event_format] that will be sent to the browser. The browser must start an interval, during which the source should collect traces. At the end of the interval the source must provide valid trace events ready for consumption by catapult's trace viewer. 

[trace_event_format]: https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit
[nlohmann_json]: https://github.com/nlohmann/json