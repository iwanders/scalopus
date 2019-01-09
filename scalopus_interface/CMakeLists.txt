cmake_minimum_required(VERSION 3.5.0)

add_library(scalopus_interface SHARED
  src/endpoint.cpp
  src/transport.cpp
)

target_include_directories(scalopus_interface
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
add_library(Scalopus::scalopus_interface ALIAS scalopus_interface)


export(
  TARGETS scalopus_interface
  NAMESPACE Scalopus::
  FILE ScalopusInterfaceConfig.cmake
)
install(TARGETS scalopus_interface EXPORT ScalopusInterface
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ScalopusInterface DESTINATION share/scalopus_interface/cmake)
