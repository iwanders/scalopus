# scalopus_catapult

The Scalopus catapult component is the very last one in the chain. This is where it all comes together and we utilise
the entire pipeline in one binary that creates a webserver that implements the tracing domain of the
[Chrome Devtools Protocol][devtools_protocol]. This serves as an remote tracing target that can be found via the
[`chrome://inspect?tracing`][chrome_tracing] page. 

It uses the `EndpointManager` and the `Provider`'s from the other components to deliver
[trace event format][trace_event_format] events to the browsers' [Catapult Trace Viewer][catapult_trace_viewer].

The [CatapultServer](/scalopus_catapult/include/scalopus_catapult/catapult_server.h) only requires the `Provider`s to be
handed in. The real work is handled by the [CatapultBackend](/scalopus_catapult/src/catapult_backend.h), this seperation
means we do not have to expose [Seasocks][seasocks] to the external header files. The backend implements the
`PageHandler` and `WebSocket::handler` for Seasocks. For every connected trace viewer it will create a
[TraceSession](/scalopus_catapult/src/trace_session.h), to this session it will add the sources that were created from
calling `makeSource()` on all providers. This `TraceSession` handles the actual communication with the websocket and it
will call `startInterval` and `finishInterval` on all the sources. Ultimately chunking the collected trace events and
sending them to the trace viewer.

## scalopus_catapult_server

Besides the `scalopus_catapult` shared object, this package builds the `scalopus_catapult_server` binary that can be
used to bring data from the endpoints and providers throughout Scalopus to the browser.

This binary can take two arguments:
```
./scalopus_catapult/scalopus_catapult_server [port [lttng_session_or_babeltrace_path]]
```

The port is pretty self explanatory, this is the TCP port to which the websocket binds. The inspect page
([`chrome://inspect?tracing`][chrome_tracing]) checks for port 9222 on localhost. So this is a logical choice to bind
on by default as it makes the catapult server without having to configure network targets.

The second argument `lttng_session_or_babeltrace_path` needs some more explanation. As detailed in the 
[tracing](/scalopus_tracing/) the use of LTTng traces requires starting the babeltrace viewer to view the already
started LTTng tracing session.

By default it will try to view the `scalopus_target_session` LTTng session. If this
does not exist (for example when using the native tracing backend) the parser will quit immediately. There are two
possible calls to get the babeltrace viewer. The default is to call [`lttng view`][lttng_view] with the session name,
this has the advantage that the hostname doesn't need to be known. In this case the parser will be instantiated using:
```
lttng view lttng_session_or_babeltrace_path -e "babeltrace --clock-seconds --clock-gmt --no-delta --input-format=lttng-live" 
```

If a `/` character is present in this path it is used as the full babeltrace path, which requires the hostname. In this
case the parser will be instantiated using:
```
babeltrace --clock-seconds --clock-gmt --no-delta --input-format=lttng-live lttng_session_or_babeltrace_path 
```

[catapult_trace_viewer]: https://github.com/catapult-project/catapult/blob/master/tracing/README.md
[trace_event_format]: https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/
[devtools_protocol]: https://chromedevtools.github.io/devtools-protocol/tot/Tracing
[seasocks]: https://github.com/mattgodbolt/seasocks/
[lttng_view]: https://lttng.org/man/1/lttng-view/v2.10/
[chrome_tracing]: chrome://inspect?tracing
