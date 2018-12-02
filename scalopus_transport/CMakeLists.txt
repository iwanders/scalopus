cmake_minimum_required(VERSION 3.5.0)

project(ScalopusTransport VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CLANG_ONLY_COMPILE_FLAGS "-Wconversion;")
  message(STATUS "Compiler is clang, adding options: ${CLANG_ONLY_COMPILE_FLAGS}")
endif()

add_compile_options("-Werror;-Wall;-Wextra;-Wshadow;-Wnon-virtual-dtor;-Wpedantic;${CLANG_ONLY_COMPILE_FLAGS}")

include(GNUInstallDirs)
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
target_link_libraries(introspect_endpoints scalopus_transport)


# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS scalopus_transport EXPORT ScalopusTransport
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT ScalopusTransport DESTINATION share/scalopus_transport/cmake)

# This makes the project importable from the build directory
export(TARGETS scalopus_transport FILE ScalopusTransportConfig.cmake)


