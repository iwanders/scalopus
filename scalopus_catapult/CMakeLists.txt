
add_library(scalopus_catapult
  src/catapult_backend.cpp
  src/catapult_server.cpp
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
target_compile_options(scalopus_catapult PRIVATE ${SCALOPUS_COMPILE_OPTIONS})

target_link_libraries(scalopus_catapult
PUBLIC
  Seasocks::seasocks
  Scalopus::scalopus_general_consumer
  Scalopus::scalopus_tracing_consumer
)

add_executable(scalopus_catapult_server src/scalopus_catapult_server.cpp)
target_link_libraries(scalopus_catapult_server PRIVATE Scalopus::scalopus_catapult)
target_compile_options(scalopus_catapult_server PRIVATE ${SCALOPUS_COMPILE_OPTIONS})