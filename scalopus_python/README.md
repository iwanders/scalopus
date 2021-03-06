# scalopus_python

This subpackage provides the Python bindings in the form of the shared object and the `scalopus` Python module.
As well as unit tests and examples for Python use of Scalopus.

## The `scalopus` Python module and bindings.

The main readme provides information about the necessary packages to succesfully build and install the Python bindings
for Scalopus. During the build process the installable Python module will be created in `build/scalopus_python/`.

The Python bindings themselves are inside a shared object created through [Pybind11][pybind11], this shared object
refers to the paths of the other libraries in your build directory. So the dependent shared objects need to be at their
original location or in a folder in which the library loader will look. If Scalopus is installed its shared objects
will be installed in `/usr/local/lib`, which should be searched automatically. If using a prefix be sure to set the
`LD_LIBRARY_PATH` accordingly.

The actual Python bindings are created with the files in the [`lib`](/scalopus_python/lib/) folder. These files are
compiled into a shared object that implements a Python module in C. This module `scalopus_python_lib`, contains the real
 bindings. This library is be different between Python 2 and Python 3 and the compilation of Scalopus will produce only
one of them. The version can be selected by setting the `SCALOPUS_USE_PYTHON2=on` or setting `SCALOPUS_USE_PYTHON3=on`
as a build option to cmake.

A convenience module is provided called [`scalopus`](/scalopus_python/scalopus/). In general the Python functions take
the same arguments as their C++ counterparts. For documentation as to how to use classes or functions from Python
it's best to look at the header files of the C++ implementations. The arguments should be the same for the Python
function.

## scalopus.interface
Counterpart to the main [scalopus_interface](/scalopus_interface/) component. On the C++ side it contains what's called
a 'trampoline class' in the Pybind documentation for the Endpoint, TraceEventProvider and TraceEventSource,
allowing pure Python implementations of these classes. The [`interface.py`](/scalopus_python/scalopus/interface.py)
adds a thin wrapper with documentation that subclassses from the bindings.

The [`test_endpoint.py`](/scalopus_python/test/test_endpoint.py) unit test shows how a pure python implementation of an
endpoint would work. An example of how to implement a pure Python `TraceEventSource` and `TraceEventProvider`  can be
found in [here](/scalopus_python/examples/python_provider_example.py).

## scalopus.tracing

The tracing functionality from Python uses the C++ implementation of the tracepoints and trackers.
To make the tracing Python code more convenient the [`tracing`](/scalopus_python/scalopus/tracing.py) file has
some helpers to allow decorating functions and tracing a `with` context:

- `TraceContext`: This is the main context manager. It takes a name and optionally a trace id. It can then be used in a
  `with` statement. Can be instantiated through `scalopus.tracing.TraceContext`, the following options are preffered.
- `TraceContextHelper`: This allows efficient lookups of already existing trace context managers. This relies on
  Python's quick attribute lookups. The sections must be unique throughout your Python process. Exposed as
  `scalopus.tracing.trace_section`.
- `traced`: This is a decorator that wraps your function in a trace context. The function name is used as the
  trace name, unless an explicit name is given. Exposed as `scalopus.tracing.traced`.
- `ThreadStateSwitcher`: This is a context manager and decorator that switches a thread's tracing
state to the target state within its scope.
- `suppressed`: This is a decrator that wraps your function in a thread state switcher that
supresses all trace points within it, and anything it calls.

```python
import scalopus.tracing as tracing

# use function name as trace name
@tracing.traced
def a():
    print("a")
    time.sleep(0.2)

# use given name as trace name
@tracing.traced('my name')
def b():
    print("b")
    time.sleep(0.2)

# fastest, one attribute lookup, name will be 'my_relevant_scope'
with tracing.trace_section.my_relevant_scope:
    b()

# Less fast then above, 1 method lookup and one call, allows spaces. Name will be "My Section"
with tracing.trace_section("My Section"):
    b()

# No traces will be generated within the scope of the suppressed block
with tracing.ThreadStateSwitcher(False):
    a()
```
For tracing Python code the [readme_example](/scalopus_python/examples/readme_example.py) is a good starting point.

The backend for the tracepoints can be swapped at runtime to switch between the `lttng`, `native` or `nop` tracepoints
through the `setBackend` method.

## scalopus.common
The [`common`](/scalopus_python/scalopus/common.py) component provides a convenient object one can instantiate in the
main section of the Python program to start a transport and set everything up to transfer the tracepoints out of the
program. This is used in the [readme_example](/scalopus_python/examples/readme_example.py) like:
```python
import scalopus
if __name__ == "__main__":
    exposer = scalopus.common.DefaultExposer(process_name=sys.argv[0])
    scalopus.general.setThreadName("main")
```
It creates a unix transport server and instantiates all endpoints that are natively provided by scalopus. Make sure that
this `exposer` object stays in scope for the duration of your program.

## scalopus.general
Counterpart to the main [scalopus_general](/scalopus_general/) component. This provides:
- `EndpointIntrospect`: See the readme in the general folder.
- `EndpointProcessInfo`: See the readme in the general folder.
- `EndpointManagerPoll`: See the readme in the general folder.
- `setThreadName("name")`: Sets the name of the calling thread.
- `weak_provider_endpoint_factory`: If you have a provider that has a `.factory()` method to create endpoints that are
 associated to it. You may need this function to wrap the provider before passing it to the `EndpointManagerPoll`
 to prevent a circular reference being formed.

## scalopus.transport
Counterpart to the main [scalopus_transport](/scalopus_transport/) component. This provides:
- `TransportLoopbackFactory`: See the readme in the transport folder.
- `TransportUnixFactory`: See the readme in the transport folder.

## scalopus commandline
The `scalopus` module is an executable module. At the moment one command is supported:
```
$ python3 -m scalopus --help
usage: __main__.py [-h] {catapult_server} ...

Scalopus, a tracing framework for C++ and Python.

positional arguments:
  {catapult_server}
    catapult_server  Start a catapult server that allows connecting from
                     chrome://inspect?tracing. It always uses the Unix
                     transport, and initialises all providers supported
                     natively by scalopus.

optional arguments:
  -h, --help         show this help message and exit
$ python3 -m scalopus catapult_server --help
usage: __main__.py catapult_server [-h] [-p PORT]
                                   [--poll-interval POLL_INTERVAL]
                                   [--lttng-session LTTNG_SESSION]

Start a catapult server that allows connecting from chrome://inspect?tracing.
It always uses the Unix transport, and initialises all providers supported
natively by scalopus.

optional arguments:
  -h, --help            show this help message and exit
  -p PORT, --port PORT  The port to bind the webserver on. Defaults to 9222.
  --poll-interval POLL_INTERVAL
                        The interval in seconds between server discovery.
                        Defaults to 1.0.
  --lttng-session LTTNG_SESSION
                        The lttng session name to connect to, defaults to
                        scalopus_target_session.

```

[pybind11]: https://github.com/pybind/pybind11
