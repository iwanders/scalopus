
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

add_executable(lttng_tracing_macros test_lttng_tracing_macros.cpp)
target_link_libraries(lttng_tracing_macros
  PRIVATE
    scalopus_lttng
)
add_test(test_lttng_tracing_macros lttng_tracing_macros)


# Setup the ringbuffer test, allow access to the private header files.
add_executable(spsc_ringbuffer test_ringbuffer.cpp)
target_link_libraries(spsc_ringbuffer
  PRIVATE
    Scalopus::scalopus_scope_tracing_native
)
target_include_directories(spsc_ringbuffer
  PRIVATE
    $<TARGET_PROPERTY:Scalopus::scalopus_scope_tracing_native,INCLUDE_DIRECTORIES>
)
add_test(test_ringbuffer spsc_ringbuffer)


add_executable(native_tracing_macros test_native_tracing_macros.cpp)
target_link_libraries(native_tracing_macros
  PRIVATE
    Scalopus::scalopus_scope_tracing_native
)
add_test(test_native_tracing_macros native_tracing_macros)
