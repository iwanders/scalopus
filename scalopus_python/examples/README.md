# Scalopus Python Examples

## `readme_example.py`
The most important example is the [`readme_example.py`](/scalopus_python/examples/readme_example.py). This example shows
how the a Python program may be traced, this example uses native tracepoints. An identical example using lttng
tracepoints is provided as well: [`readme_example_lttng.py`](/scalopus_python/examples/readme_example_lttng.py)

## `embedded_catapult_server.py`
This shows how one could embed the catapult server in the process that produces the tracepoints.

## `python_provider_example.py`
The [`python_provider_example.py`](/scalopus_python/examples/python_provider_example.py) example shows how to create a
pure Python provider that outputs fictitional traces of 1 second duration appearing to originate from a fake process.
