
add_executable(test_ctfevent test_ctfevent.cpp)
target_link_libraries(test_ctfevent
  PRIVATE
    scalopus_lttng_babeltrace
)
add_test(ctf_event test_ctfevent)

if(TARGET Scalopus::scalopus_tracing_lttng)
  add_executable(endpoint_scope_tracing test_endpoint_scope_tracing.cpp)
  target_link_libraries(endpoint_scope_tracing
    PRIVATE
      scalopus_tracing_lttng
  )
  add_test(test_endpoint_scope_tracing endpoint_scope_tracing)

  add_executable(lttng_tracing_macros test_lttng_tracing_macros.cpp)
  target_link_libraries(lttng_tracing_macros
    PRIVATE
      scalopus_tracing_lttng
  )
  add_test(test_lttng_tracing_macros lttng_tracing_macros)
endif()


# Setup the ringbuffer test, allow access to the private header files.
add_executable(spsc_ringbuffer test_ringbuffer.cpp)
target_link_libraries(spsc_ringbuffer
  PRIVATE
    Scalopus::scalopus_tracing_native
)
target_include_directories(spsc_ringbuffer
  PRIVATE
    $<TARGET_PROPERTY:Scalopus::scalopus_tracing_native,INCLUDE_DIRECTORIES>
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
add_test(test_ringbuffer spsc_ringbuffer)


add_executable(native_tracing_macros test_native_tracing_macros.cpp)
target_link_libraries(native_tracing_macros
  PRIVATE
    Scalopus::scalopus_tracing_native
)
add_test(test_native_tracing_macros native_tracing_macros)


# Setup the ringbuffer test, allow access to the private header files.
add_executable(cbor_serializer test_cbor_serializer.cpp)
target_link_libraries(cbor_serializer
  PRIVATE
    Scalopus::scalopus_tracing_native
    nlohmann_json::nlohmann_json
)
target_include_directories(cbor_serializer
  PRIVATE
    $<TARGET_PROPERTY:Scalopus::scalopus_tracing_native,INCLUDE_DIRECTORIES>
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/native/
)
add_test(test_cbor_serializer cbor_serializer)

add_executable(tracepoint_enable_disable test_tracepoint_enable_disable.cpp)
target_link_libraries(tracepoint_enable_disable
  PRIVATE
    Scalopus::scalopus_tracing_native
    Scalopus::scalopus_tracing_consumer
)
add_test(test_tracepoint_enable_disable tracepoint_enable_disable)
