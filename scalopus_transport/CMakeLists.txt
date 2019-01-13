# Create the library that provides the scope tracepoints 
add_library(scalopus_transport SHARED
  src/transport_unix.cpp
  src/transport_mock.cpp
  src/protocol.cpp
)
add_library(Scalopus::scalopus_transport ALIAS scalopus_transport)

target_include_directories(scalopus_transport
  PUBLIC
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(scalopus_transport
  PUBLIC
    Scalopus::scalopus_interface
  PRIVATE
    Threads::Threads
    nlohmann_json::nlohmann_json
)

export(
  TARGETS scalopus_transport
  NAMESPACE Scalopus::
  FILE ${SCALOPUS_COMPONENT_CMAKE_CONFIG_DIR}/ScalopusTransportConfig.cmake
)
install(TARGETS scalopus_transport EXPORT ScalopusTransport
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ScalopusTransport DESTINATION share/scalopus_transport/cmake)

add_subdirectory(test)

