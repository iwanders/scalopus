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
  add_subdirectory(thirdparty/seasocks)
endif()

