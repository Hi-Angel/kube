cmake_minimum_required(VERSION 3.0)

project(kube VERSION 0.4.0)

set (QT_MIN_VERSION "5.7.0")

cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0053 NEW)
cmake_policy(SET CMP0063 NEW)

include(CPack)
include(FeatureSummary)
find_package(PkgConfig REQUIRED)
find_package(ECM 5.29.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECompilerSettings)
#Avoid building appstreamtest
set(KDE_SKIP_TEST_SETTINGS true)
include(KDECMakeSettings NO_POLICY_SCOPE)

enable_testing()

add_subdirectory(framework)
add_subdirectory(components)
add_subdirectory(icons)
add_subdirectory(applications)
add_subdirectory(accounts)
add_subdirectory(tests)
