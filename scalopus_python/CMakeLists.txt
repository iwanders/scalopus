find_package (Python COMPONENTS Interpreter Development)

if(NOT TARGET pybind11::pybind11)
  message(WARNING "Did not detect pybind11, skipping building the Python bindings.")
  return()
else()
  message(STATUS "Detected pybind11 target, building python bindings.")
endif()

pybind11_add_module(scalopus_python_lib
  lib/module.cpp
  lib/scalopus_interface.cpp
  lib/scalopus_transport.cpp
  lib/scalopus_general.cpp
  lib/scalopus_tracing.cpp
  lib/scalopus_catapult.cpp
  lib/json_util.cpp
  lib/python_test_helpers.cpp
  )
target_link_libraries(scalopus_python_lib
  PRIVATE
    Scalopus::scalopus_transport
    Scalopus::scalopus_general
    Scalopus::scalopus_scope_tracing
    Scalopus::scalopus_general_consumer
    Scalopus::scalopus_tracepoint_native
    Scalopus::scalopus_tracepoint_nop
    Scalopus::scalopus_tracing_consumer
    Scalopus::scalopus_catapult
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

get_target_property(setup_scalopus_python_lib_suffix scalopus_python_lib SUFFIX)
get_target_property(setup_scalopus_python_lib_prefix scalopus_python_lib PREFIX)
#message(WARNING "setup_scalopus_python_lib_suffix: ${setup_scalopus_python_lib_suffix}")
#message(WARNING "setup_scalopus_python_lib_prefix: ${setup_scalopus_python_lib_prefix}")

# Copy the scalopus module.
file(COPY scalopus DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copy the python unit tests
file(COPY test DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copy the python examples
file(COPY examples DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Write the setup file.
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/setup.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/setup.py
  @ONLY
)
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/native_libs.txt.in
  ${CMAKE_CURRENT_BINARY_DIR}/native_libs.txt
  @ONLY
)

# The following does look a bit fragile... but it works for now.
# It can find the scalopus_python_lib library because that's present in the working directory.
add_custom_target(test_python_module_run
  COMMAND python${PYTHON_VERSION_MAJOR} setup.py test
  DEPENDS scalopus_python_lib
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME python_test COMMAND make test_python_module_run
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
