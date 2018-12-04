cmake_minimum_required(VERSION 3.5.0)

include(FindThreads)

# Create the library that provides the scope tracepoints 
add_library(scalopus_transport SHARED
  src/transport_server.cpp
  src/transport_client.cpp
  src/transport_server_unix.cpp
  src/transport_client_unix.cpp
  src/endpoint.cpp
  src/client.cpp
  src/protocol.cpp
  src/client_introspect.cpp
  src/endpoint_introspect.cpp
)
add_library(Scalopus::Transport ALIAS scalopus_transport)

target_include_directories(scalopus_transport
  PUBLIC
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(scalopus_transport
  PRIVATE
    Threads::Threads
)

add_executable(introspect_endpoints
  src/introspect_endpoints.cpp
)
target_link_libraries(introspect_endpoints Scalopus::Transport)

