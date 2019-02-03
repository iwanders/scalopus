# scalopus_python

This subpackage provides the Python bindings and unit tests.

## The `scalopus` Python module and bindings.

The main readme provides information about the necessary packages to succesfully build and install the Python bindings
for Scalopus. During the build process the installable Python module will be created in `/build/scalopus_python/`.

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

## scalopus.tracing

The tracing functionality from Python uses the C++ implementation of the tracepoints and trackers. 
To make the tracing Python code more convenient the [`tracing`](/scalopus_python/scalopus/tracing.py) file has
some helpers to allow decorating functions and tracing a `with` context:

- `TraceContext`: This is the main context manager. It takes a name and optionally a trace id. It can then be used in a
  `with` statement. Can be instantiated through `scalopus.tracing.TraceContext`, the following options are preffered.
- `TraceContextHelper`: This allows efficient lookups of already existing trace context managers. This relies on
  Python's quick attribute lookups. The sections must be unique throughout your Python process. Exposed as
  `scalopus.tracing.trace_section`.
- `trace_function`: This is a decorator that wraps your function in a trace context. The function name is used as the
  trace name. Exposed as `scalopus.tracing.trace_function`.

```python
import scalopus.tracing as tracing
@tracing.trace_function
def a():
    print("a")
    time.sleep(0.2)

# fastest, one attribute lookup, name will be 'my_relevant_scope'
with tracing.trace_section.my_relevant_scope:
    b()

# Less fast then above, 1 method lookup and one call, allows spaces. Name will be "My Section"
with tracing.trace_section("My Section"):
    b()
```
For tracing Python code the [readme_example](/scalopus_python/examples/readme_example.py) is a good starting point.

## scalopus.interface
Counterpart to the main [scalopus_interface](/scalopus_interface/) component. On the C++ side it contains what's called
a 'trampoline class' in the Pybind documentation for the Endpoint, allowing pure Python implementations for endpoints.
The Python [`interface.py`](/scalopus_python/scalopus/interface.py) adds a thin wrapper with some documentation that
subclassses from the bindings.

The [`test_endpoint.py`](/scalopus_python/test/test_endpoint.py) unit test shows how a pure python implementation of an
endpoint would work.

## scalopus.general
Counterpart to the main [scalopus_general](/scalopus_general/) component. This provides:
- `EndpointIntrospect`: See the readme in the general folder.
- `EndpointProcessInfo`: See the readme in the general folder.
- `EndpointManagerPoll`: See the readme in the general folder.
- `setThreadName("name")`: Sets the name of the calling thread.

## scalopus.transport
Counterpart to the main [scalopus_transport](/scalopus_transport/) component. This provides:
- `TransportLoopbackFactory`: See the readme in the transport folder.
- `TransportUnixFactory`: See the readme in the transport folder.

[pybind11]: https://github.com/pybind/pybind11
