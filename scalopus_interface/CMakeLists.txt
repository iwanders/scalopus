# The interface for endpoint and transport, this is all a process under test should need.
add_library(scalopus_interface SHARED
  src/endpoint.cpp
  src/destination.cpp
  src/transport.cpp
  src/transport_factory.cpp
)
target_compile_features(scalopus_interface PUBLIC cxx_relaxed_constexpr)
target_compile_options(scalopus_interface PRIVATE ${SCALOPUS_COMPILE_OPTIONS})

target_include_directories(scalopus_interface
  PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_interface/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
add_library(Scalopus::scalopus_interface ALIAS scalopus_interface)


# Interface for providers, requires exposing nlohmann_json.
add_library(scalopus_consumer SHARED
  src/trace_event_source.cpp
)
target_compile_options(scalopus_consumer PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_include_directories(scalopus_consumer
  PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_interface/include_consumer>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include_consumer>
)
target_link_libraries(scalopus_consumer
  PUBLIC
    nlohmann_json::nlohmann_json
    Scalopus::scalopus_interface
)
add_library(Scalopus::scalopus_consumer ALIAS scalopus_consumer)


export(
  TARGETS scalopus_interface scalopus_consumer
  NAMESPACE Scalopus::
  FILE ${SCALOPUS_EXPORT_CMAKE_DIR}/ScalopusInterfaceConfig.cmake
)
install(TARGETS scalopus_interface scalopus_consumer EXPORT ScalopusInterfaceConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scalopus_interface/)
install(DIRECTORY include_consumer DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scalopus_interface/)
install(
  EXPORT ScalopusInterfaceConfig
  FILE ScalopusInterfaceConfig.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${SCALOPUS_INSTALL_EXPORT_CMAKE_DIR}
)
