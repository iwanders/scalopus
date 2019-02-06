add_library(scalopus_general SHARED
  src/endpoint_process_info.cpp
  src/endpoint_introspect.cpp
  src/thread_name_tracker.cpp
)
target_compile_options(scalopus_general PRIVATE ${SCALOPUS_COMPILE_OPTIONS})

# need shared_timed_mutex, which is C++14, this is a feature that requires C++14, and only cmake version 3.1
target_compile_features(scalopus_general PUBLIC cxx_relaxed_constexpr)

target_include_directories(scalopus_general
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_general ALIAS scalopus_general)

target_link_libraries(scalopus_general
  PUBLIC
    Scalopus::scalopus_transport
    Threads::Threads
  PRIVATE
    nlohmann_json::nlohmann_json
)

add_library(scalopus_general_consumer SHARED
  src/general_source.cpp
  src/general_provider.cpp
  src/endpoint_manager_poll.cpp
)
target_compile_options(scalopus_general_consumer PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_include_directories(scalopus_general_consumer
  PUBLIC 
    $<INSTALL_INTERFACE:include_consumer>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include_consumer>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_general_consumer ALIAS scalopus_general_consumer)
target_link_libraries(scalopus_general_consumer
  PUBLIC
    scalopus_general
    Scalopus::scalopus_consumer
)

export(
  TARGETS scalopus_general scalopus_general_consumer
  NAMESPACE Scalopus::
  FILE ${SCALOPUS_COMPONENT_CMAKE_CONFIG_DIR}/ScalopusGeneralConfig.cmake
)
install(TARGETS scalopus_general scalopus_general_consumer EXPORT ScalopusGeneral
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ScalopusGeneral DESTINATION share/scalopus_general/cmake)

add_subdirectory(test)