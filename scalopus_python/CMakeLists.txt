
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
)