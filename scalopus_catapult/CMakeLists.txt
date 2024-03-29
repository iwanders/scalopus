
add_library(scalopus_catapult SHARED
  src/catapult_backend.cpp
  src/catapult_server.cpp
  src/trace_session.cpp
  src/catapult_recorder.cpp
)

target_include_directories(scalopus_catapult
  PUBLIC 
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_catapult/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
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

if (SCALOPUS_TRACING_HAVE_BUILT_LTTNG)
  target_compile_definitions(scalopus_catapult_server
    PRIVATE
      SCALOPUS_TRACING_HAVE_LTTNG=1
  )
endif()



export(
  TARGETS scalopus_catapult
  NAMESPACE Scalopus::
  FILE ${SCALOPUS_EXPORT_CMAKE_DIR}/ScalopusCatapultConfig.cmake
)
install(TARGETS scalopus_catapult scalopus_catapult_server EXPORT ScalopusCatapultConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scalopus_catapult/)
install(
  EXPORT ScalopusCatapultConfig
  FILE ScalopusCatapultConfig.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${SCALOPUS_INSTALL_EXPORT_CMAKE_DIR}
)
