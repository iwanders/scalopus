
add_executable(test_ctfevent test_ctfevent.cpp)
target_link_libraries(test_ctfevent
  PRIVATE
    scalopus_babeltrace
    scalopus_lttng
)
add_test(ctf_event test_ctfevent)

# https://gitlab.kitware.com/cmake/cmake/issues/8774
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS test_ctfevent)
