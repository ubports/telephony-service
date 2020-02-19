project(telephony-service)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# Standard install paths
include(GNUInstallDirs)

# Check for include files
include(CheckIncludeFileCXX)
include(CheckIncludeFile)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# just to make debug easier, print the system processor
message(STATUS "System processor: ${CMAKE_SYSTEM_PROCESSOR}")

set(TELEPHONY_SERVICE_DIR ${CMAKE_INSTALL_DATADIR}/telephony-service)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

find_package(Qt5Contacts)
find_package(Qt5DBus)
find_package(Qt5Multimedia)
find_package(Qt5Qml)
find_package(Qt5Test)
find_package(Qt5Feedback)
find_package(Qt5Network)
find_package(LibPhoneNumber REQUIRED)

include(qt5)

option(SKIP_QML_TESTS "Skip QML tests" OFF)
option(WANT_UI_SERVICES "Enable build of UI services" ON)

if(CMAKE_CROSSCOMPILING)
    find_program(QMAKE_EXECUTABLE qmake)
    if(QMAKE_EXECUTABLE STREQUAL "QMAKE_EXECUTABLE-NOTFOUND")
        message(FATAL_ERROR "qmake not found")
    endif()
    execute_process(
        COMMAND ${QMAKE_EXECUTABLE} -query QT_INSTALL_QML
        RESULT_VARIABLE RESULT
        OUTPUT_VARIABLE QT_INSTALL_QML
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to determine QT_INSTALL_QML from qmake")
    endif()
else()
    # qmake isn't multi-arch aware as it installs arch-specific mkspec files
    # in to /usr/share, so we can't use it here (we'd need a qmake binary
    # for the host arch using data for the target arch)
    set(QT_INSTALL_QML "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(TP_QT5 REQUIRED TelepathyQt5)
pkg_check_modules(TP_QT5_FS REQUIRED TelepathyQt5Farstream)
pkg_check_modules(NOTIFY REQUIRED libnotify)
pkg_check_modules(TPFS REQUIRED telepathy-farstream)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(FS REQUIRED farstream-0.2)
pkg_check_modules(PULSEAUDIO libpulse)

if (WANT_UI_SERVICES)
    pkg_check_modules(MESSAGING_MENU REQUIRED messaging-menu)
    pkg_check_modules(UserMetrics REQUIRED libusermetricsinput-1)
    pkg_check_modules(HISTORY REQUIRED history-service)
    pkg_check_modules(URL_DISPATCHER REQUIRED url-dispatcher-1)

    add_definitions(-DWANT_UI_SERVICES)
    message("Building with UI services")
else()
    message("Bulding of UI services is disabled")
endif()

add_definitions(-DQT_NO_KEYWORDS)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

# generate a macro to make it easier to enable coverage support on targets
function(ENABLE_COVERAGE)
    get_directory_property(COVERAGE_TARGETS DIRECTORY ${CMAKE_SOURCE_DIR} COVERAGE_TARGETS)
    list(APPEND COVERAGE_TARGETS ${ARGN})
    MESSAGE(STATUS "Enabling coverage report for target(s): ${ARGN}")
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY COVERAGE_TARGETS ${COVERAGE_TARGETS})
endfunction()

enable_testing()

add_definitions(-std=c++11)

set(DATA_DIR "${CMAKE_SOURCE_DIR}/data")
# install assets
set(ASSETS_DIR assets)
install(DIRECTORY ${ASSETS_DIR} DESTINATION ${TELEPHONY_SERVICE_DIR})

add_subdirectory(libtelephonyservice)
add_subdirectory(Ubuntu)
add_subdirectory(handler)
add_subdirectory(tools)
add_subdirectory(icons)
add_subdirectory(po)
add_subdirectory(tests)
add_subdirectory(protocols)
add_subdirectory(accounts)

if (WANT_UI_SERVICES)
    add_subdirectory(indicator)
    add_subdirectory(approver)
endif()

include(EnableCoverageReport)
#####################################################################
# Enable code coverage calculation with gcov/gcovr/lcov
# Usage:
#  * Switch build type to coverage (use ccmake or cmake-gui)
#  * Invoke make, make test, make coverage
#  * Find html report in subdir coveragereport
#  * Find xml report feasible for jenkins in coverage.xml
#####################################################################
IF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -coverage" )
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -coverage" )
  GET_DIRECTORY_PROPERTY(COVERAGE_TARGETS DIRECTORY ${CMAKE_SOURCE_DIR} COVERAGE_TARGETS)
  ENABLE_COVERAGE_REPORT(TARGETS ${COVERAGE_TARGETS})
ENDIF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])

