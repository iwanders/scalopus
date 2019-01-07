cmake_minimum_required(VERSION 3.5.0)

find_package(ScalopusGeneral REQUIRED)

find_package(nlohmann_json 3.2.0 REQUIRED)

include(FindThreads)

add_library(scalopus_general SHARED
  src/endpoint_process_info.cpp
  src/thread_name_tracker.cpp
)

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


export(
  TARGETS scalopus_general
  NAMESPACE Scalopus::
  FILE ScalopusGeneralConfig.cmake
)
install(TARGETS scalopus_general EXPORT ScalopusGeneral
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ScalopusGeneral DESTINATION share/scalopus_general/cmake)

add_subdirectory(test)