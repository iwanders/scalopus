cmake_minimum_required(VERSION 3.5.0)

project(ScalopusLttng VERSION 0.0.1)

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
find_package(ScalopusTransport REQUIRED COMPONENTS
  scalopus_transport)

include (FindPkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(LTTngUST lttng-ust)
endif()

if (!LTTngUST_FOUND)
  error("LTTNG could not be found")
endif()

# Create the library that provides the scope tracepoints 
add_library(scalopus_lttng SHARED
  src/scope_tracepoint.cpp
  src/scope_trace_raii.cpp
  src/scope_trace_tracker.cpp
  src/endpoint_scope_tracing.cpp
  src/client_scope_tracing.cpp
)

target_include_directories(scalopus_lttng
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(scalopus_lttng
  PUBLIC
    scalopus_transport
  PRIVATE
    ${LTTngUST_LIBRARIES}
)



# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS scalopus_lttng EXPORT ScalopusLttng
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT ScalopusLttng DESTINATION share/scalopus_lttng/cmake)

# This makes the project importable from the build directory
export(TARGETS scalopus_lttng FILE ScalopusLttngConfig.cmake)


