
add_library(scalopus_catapult SHARED
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
  Scalopus::scalopus_general_consumer
  Scalopus::scalopus_tracing_consumer
PRIVATE
  Seasocks::seasocks
)

add_executable(scalopus_catapult_server src/scalopus_catapult_server.cpp)
target_link_libraries(scalopus_catapult_server PRIVATE Scalopus::scalopus_catapult)
target_compile_options(scalopus_catapult_server PRIVATE ${SCALOPUS_COMPILE_OPTIONS})



export(
  TARGETS scalopus_catapult
  NAMESPACE Scalopus::
  FILE ${SCALOPUS_COMPONENT_CMAKE_CONFIG_DIR}/ScalopusCatapultConfig.cmake
)
install(TARGETS scalopus_catapult EXPORT ScalopusCatapult
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ScalopusCatapult DESTINATION share/scalopus_catapult/cmake)
