include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)

set(FRAMEWORK_INSTALL_DIR ${QML_INSTALL_DIR}/org/kube/framework)

install(DIRECTORY qml/ DESTINATION ${FRAMEWORK_INSTALL_DIR})
install(FILES qmldir DESTINATION ${FRAMEWORK_INSTALL_DIR})

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
