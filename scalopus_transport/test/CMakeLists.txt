
add_executable(test_transport_unix test_transport_unix.cpp)
target_link_libraries(test_transport_unix
  PRIVATE
    scalopus_transport
)
# Allow using private headers, reach into target's include directories.
target_include_directories(test_transport_unix
  PRIVATE
    $<TARGET_PROPERTY:Scalopus::scalopus_transport,INCLUDE_DIRECTORIES>
)
add_test(test_transport_unix test_transport_unix)


add_executable(test_transport_loopback test_transport_loopback.cpp)
target_link_libraries(test_transport_loopback
  PRIVATE
    scalopus_transport
)
# Allow using private headers, reach into target's include directories.
target_include_directories(test_transport_loopback
  PRIVATE
    $<TARGET_PROPERTY:Scalopus::scalopus_transport,INCLUDE_DIRECTORIES>
)
add_test(test_transport_loopback test_transport_loopback)

# https://gitlab.kitware.com/cmake/cmake/issues/8774
add_custom_target(check_transport COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS test_transport_unix test_transport_loopback)
