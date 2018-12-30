cmake_minimum_required(VERSION 3.5.0)

find_package(ScalopusTransport REQUIRED)

include (FindPkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(LTTngUST lttng-ust)
endif()

find_package(nlohmann_json 3.2.0 REQUIRED)

if (!LTTngUST_FOUND)
  error("LTTNG could not be found")
endif()

include(FindThreads)

# Create the library that provides the scope tracepoints 
add_library(scalopus_lttng SHARED
  src/scope_tracepoint.cpp
  src/scope_trace_raii.cpp
  src/scope_trace_tracker.cpp
  src/endpoint_scope_tracing.cpp
)

target_include_directories(scalopus_lttng
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_lttng ALIAS scalopus_lttng)

target_link_libraries(scalopus_lttng
  PUBLIC
    Scalopus::scalopus_transport
  PRIVATE
    nlohmann_json::nlohmann_json
    ${LTTngUST_LIBRARIES}
)


add_library(scalopus_babeltrace
  src/ctfevent.cpp
  src/babeltrace_tool.cpp
  src/babeltrace_parser.cpp)
target_link_libraries(scalopus_babeltrace
  PUBLIC
    scalopus_lttng
  PRIVATE
    Threads::Threads
)
add_library(Scalopus::scalopus_babeltrace ALIAS scalopus_babeltrace)


add_executable(babeltrace_test
  src/babeltrace_test.cpp)
target_link_libraries(babeltrace_test
  PRIVATE
    scalopus_babeltrace
)

export(
  TARGETS scalopus_lttng scalopus_babeltrace
  NAMESPACE Scalopus::
  FILE ScalopusLttngConfig.cmake
)
install(TARGETS scalopus_lttng scalopus_babeltrace EXPORT ScalopusLttng
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ScalopusLttng DESTINATION share/scalopus_lttng/cmake)

include(CTest)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()
