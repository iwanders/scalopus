
add_library(scalopus_catapult
  src/catapult_backend.cpp
  src/endpoint_manager_poll.cpp
  src/trace_session.cpp
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
  Scalopus::scalopus_interface
  Scalopus::scalopus_consumer
  Scalopus::scalopus_general_consumer
  Scalopus::scalopus_lttng_consumer
)

add_executable(catapult_server
  src/catapult_server.cpp)

target_link_libraries(catapult_server PRIVATE Scalopus::scalopus_catapult)
