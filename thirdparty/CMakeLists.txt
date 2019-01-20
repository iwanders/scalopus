# This file is used through include() from the root to ensure that the targets produced by the embedded libraries
# are available to the subcomponents of scalopus.

if(SCALOPUS_USE_EXTERNAL_JSON)
  # Minimum required version is 3.4.0 for bson support.
  set(SCALOPUS_REQUIRED_JSON_VERSION 3.4.0)
  # Retrieve the package from the system.
  find_package(nlohmann_json REQUIRED)
  # Confirm it is at least the desired version
  if(nlohmann_json_VERSION VERSION_LESS SCALOPUS_REQUIRED_JSON_VERSION)
    message( FATAL_ERROR "Scalopus requires nlohmann_json at version ${SCALOPUS_REQUIRED_JSON_VERSION} or greater, it found ${nlohmann_json_VERSION}." )
  endif()
else()
  set(JSON_BuildTests OFF CACHE INTERNAL "")
  add_subdirectory(thirdparty/nlohmann_json)
endif()

if(SCALOPUS_USE_EXTERNAL_SEASOCKS)
  find_package(Seasocks REQUIRED)
else()
  set(UNITTESTS "Build unittests." OFF)
  add_subdirectory(thirdparty/seasocks)
endif()


# Ideally we'd just build both, depending on availability, but that's hard at the moment:
# https://github.com/pybind/pybind11/issues/748
if(SCALOPUS_USE_PYTHON2)
  set(PYBIND11_PYTHON_VERSION 2)
endif()

if(SCALOPUS_USE_PYTHON3)
  set(PYBIND11_PYTHON_VERSION 3)
endif()

if(SCALOPUS_USE_EXTERNAL_PYBIND)
  find_package(pybind11 QUIET)
else()
  add_subdirectory(thirdparty/pybind11)
endif()

