cmake_minimum_required(VERSION 3.5.0)

find_package(ScalopusTransport REQUIRED)
find_package(ScalopusLttng REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(Seasocks REQUIRED)


add_library(scalopus_catapult
  src/tracing_endpoint.cpp
  src/tracing_session.cpp
)

target_include_directories(scalopus_catapult
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_catapult ALIAS scalopus_catapult)

target_link_libraries(scalopus_catapult
PUBLIC
  Seasocks::seasocks
  nlohmann_json::nlohmann_json
  Scalopus::scalopus_babeltrace
)

add_executable(tracing_target
  src/tracing_target.cpp)

target_link_libraries(tracing_target PRIVATE Scalopus::scalopus_catapult)
