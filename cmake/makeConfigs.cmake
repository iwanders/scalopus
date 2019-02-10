
include(CMakePackageConfigHelpers)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${SCALOPUS_EXPORT_CMAKE_DIR}
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ScalopusConfig.cmake.in
    ${SCALOPUS_EXPORT_CMAKE_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${SCALOPUS_INSTALL_EXPORT_CMAKE_DIR})

function(makeModuleConfig SUBMODULE_TARGET_NAME SUBMODULE_TARGET_LIB)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/moduleConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}${SUBMODULE_TARGET_NAME}Config.cmake
    INSTALL_DESTINATION ${SCALOPUS_INSTALL_EXPORT_CMAKE_DIR})
install(FILES ${SCALOPUS_EXPORT_CMAKE_DIR}/cmake/${PROJECT_NAME}${SUBMODULE_TARGET_NAME}Config.cmake
    DESTINATION ${SCALOPUS_INSTALL_EXPORT_CMAKE_DIR})
endfunction(makeModuleConfig)

makeModuleConfig(Interface scalopus_interface)
makeModuleConfig(Transport scalopus_transport)
makeModuleConfig(Catapult scalopus_catapult)
makeModuleConfig(General scalopus_general)
makeModuleConfig(Tracing scalopus_tracing_consumer)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${SCALOPUS_INSTALL_EXPORT_CMAKE_DIR}
)