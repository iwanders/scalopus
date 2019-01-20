
if(NOT TARGET pybind11::pybind11)
  message(WARNING "Did not detect pybind11, skipping building the Python bindings.")
  return()
else()
  message(STATUS "Detected pybind11 target, building python bindings.")
endif()

pybind11_add_module(scalopus_python_lib lib/main.cpp)
target_link_libraries(scalopus_python_lib
  PRIVATE
    Scalopus::scalopus_transport
    Scalopus::scalopus_general
    Scalopus::scalopus_scope_tracing
    Scalopus::scalopus_general_consumer
    Scalopus::scalopus_tracepoint_native
)

if (SCALOPUS_TRACING_HAVE_BUILT_LTTNG)
  target_compile_definitions(scalopus_python_lib
    PRIVATE
      SCALOPUS_TRACING_HAVE_LTTNG=1
  )
  target_link_libraries(scalopus_python_lib
    PRIVATE
      Scalopus::scalopus_tracepoint_lttng
  )
else()
  message(WARNING "Not providing LTTng tracepoints in python module, they don't exist.")
endif()


file(COPY scalopus DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/setup.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/setup.py
  @ONLY
)

