# The interface for endpoint and transport, this is all a process under test should need.
add_library(scalopus_interface SHARED
  src/endpoint.cpp
  src/destination.cpp
  src/transport.cpp
)

target_include_directories(scalopus_interface
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(scalopus_interface
  PRIVATE
    nlohmann_json::nlohmann_json
)
add_library(Scalopus::scalopus_interface ALIAS scalopus_interface)


# Interface for providers, requires exposing nlohmann_json.
add_library(scalopus_consumer SHARED
  src/trace_event_source.cpp
)
target_include_directories(scalopus_consumer
  PUBLIC
    $<INSTALL_INTERFACE:include_consumer>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include_consumer>
)
target_link_libraries(scalopus_consumer
  PUBLIC
    nlohmann_json::nlohmann_json
    scalopus_interface
)
add_library(Scalopus::scalopus_consumer ALIAS scalopus_consumer)


export(
  TARGETS scalopus_interface scalopus_consumer
  NAMESPACE Scalopus::
  FILE ScalopusInterfaceConfig.cmake
)
install(TARGETS scalopus_interface scalopus_consumer EXPORT ScalopusInterface
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ScalopusInterface DESTINATION share/scalopus_interface/cmake)
