
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


add_executable(test_transport_mock test_transport_mock.cpp)
target_link_libraries(test_transport_mock
  PRIVATE
    scalopus_transport
)
# Allow using private headers, reach into target's include directories.
target_include_directories(test_transport_mock
  PRIVATE
    $<TARGET_PROPERTY:Scalopus::scalopus_transport,INCLUDE_DIRECTORIES>
)
add_test(test_transport_mock test_transport_mock)

# https://gitlab.kitware.com/cmake/cmake/issues/8774
add_custom_target(check_transport COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS test_transport_unix test_transport_mock)
