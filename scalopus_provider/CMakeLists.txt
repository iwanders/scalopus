cmake_minimum_required(VERSION 3.5.0)

project(scalopus_provider VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CLANG_ONLY_COMPILE_FLAGS "-Wconversion;")
  message("Compiler is clang, adding options: ${CLANG_ONLY_COMPILE_FLAGS}")
endif()

add_compile_options("-Werror;-Wall;-Wextra;-Wshadow;-Wnon-virtual-dtor;-Wpedantic;${CLANG_ONLY_COMPILE_FLAGS}")

include(FindThreads)

include (FindPkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(LTTngUST lttng-ust)
endif()
include(GNUInstallDirs)

if (!LTTngUST_FOUND)
  error("LTTNG could not be found")
endif()

# Create the library that provides the scope tracepoints 
add_library(scalopus_scope_tracepoint
  src/scope_tracepoint.cpp
  src/scope_trace_raii.cpp
  src/scope_trace_tracker.cpp
  src/exposer.cpp
)

target_include_directories(scalopus_scope_tracepoint
    PUBLIC 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${LTTngUST_INCLUDE_DIRS}
        
)

target_link_libraries(scalopus_scope_tracepoint
  PRIVATE
    Threads::Threads
  INTERFACE
    ${LTTngUST_LIBRARIES}
)


add_executable(example_scope_tracepoints
  example/example_scope_tracepoints.cpp
)
target_link_libraries(example_scope_tracepoints scalopus_scope_tracepoint)


# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS scalopus_scope_tracepoint EXPORT scalopus_provider
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT scalopus_providerConfig DESTINATION share/scalopus_provider/cmake)

# This makes the project importable from the build directory
export(TARGETS scalopus_scope_tracepoint FILE scalopus_providerConfig.cmake)


