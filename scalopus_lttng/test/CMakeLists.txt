
add_executable(test_ctfevent test_ctfevent.cpp)
target_link_libraries(test_ctfevent
  PRIVATE
    scalopus_babeltrace
    scalopus_lttng
)
add_test(ctf_event test_ctfevent)


add_executable(endpoint_scope_tracing test_endpoint_scope_tracing.cpp)
target_link_libraries(endpoint_scope_tracing
  PRIVATE
    scalopus_lttng
)
add_test(test_endpoint_scope_tracing endpoint_scope_tracing)


# https://gitlab.kitware.com/cmake/cmake/issues/8774
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS test_ctfevent test_endpoint_scope_tracing)
