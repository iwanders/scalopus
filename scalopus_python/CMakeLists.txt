
if(NOT TARGET pybind11::pybind11)
  message(WARNING "Did not detect pybind11, skipping building the Python bindings.")
  return()
else()
  message(STATUS "Detected pybind11 target, building python bindings.")
endif()

option(SCALOPUS_PYTHON_USE_LTTNG_TRACEPOINTS "Use LTTNG tracepoints for the python module" OFF)
option(SCALOPUS_PYTHON_USE_NATIVE_TRACEPOINTS "Use Native tracepoints for the python module" ON)

pybind11_add_module(scalopus_python_lib lib/main.cpp)
target_link_libraries(scalopus_python_lib
  PRIVATE
    Scalopus::scalopus_transport
    Scalopus::scalopus_general
    Scalopus::scalopus_scope_tracing
)

if (SCALOPUS_PYTHON_USE_LTTNG_TRACEPOINTS AND SCALOPUS_PYTHON_USE_NATIVE_TRACEPOINTS)
  message(FATAL_ERROR "Both LTTng and native tracepoints requested for python bindings, this cannot work.")
endif()

if (SCALOPUS_PYTHON_USE_LTTNG_TRACEPOINTS)
  if (SCALOPUS_TRACING_HAVE_BUILT_LTTNG)
    target_compile_definitions(scalopus_python_lib
      PRIVATE
        SCALOPUS_TRACING_HAVE_BUILT_LTTNG=1
    )
    target_link_libraries(scalopus_python_lib
      PRIVATE
        Scalopus::scalopus_tracing_lttng
    )
  else()
    message(WARNING "Python bindings with LTTng tracepoints requested, but these are not build.")
  endif()
endif()

if (SCALOPUS_PYTHON_USE_NATIVE_TRACEPOINTS)
    target_link_libraries(scalopus_python_lib
      PRIVATE
        Scalopus::scalopus_tracing_native
    )
endif()


file(COPY scalopus DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/setup.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/setup.py
  @ONLY
)

