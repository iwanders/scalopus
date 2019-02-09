# Create the library that provides the scope tracepoints 
add_library(scalopus_transport SHARED
  src/transport_unix.cpp
  src/transport_loopback.cpp
  src/protocol.cpp
)
add_library(Scalopus::scalopus_transport ALIAS scalopus_transport)

target_include_directories(scalopus_transport
  PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_transport/include>
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
target_compile_options(scalopus_transport PRIVATE ${SCALOPUS_COMPILE_OPTIONS})

export(
  TARGETS scalopus_transport
  NAMESPACE Scalopus::
  FILE ${SCALOPUS_EXPORT_CMAKE_DIR}/ScalopusTransportConfig.cmake
)
install(TARGETS scalopus_transport EXPORT ScalopusTransportConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scalopus_transport/)
install(EXPORT ScalopusTransportConfig DESTINATION ${SCALOPUS_INSTALL_EXPORT_CMAKE_DIR})

add_subdirectory(test)

