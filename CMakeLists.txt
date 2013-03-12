project(phone-app)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# Standard install paths
include(GNUInstallDirs)

# Check for include files
include(CheckIncludeFileCXX)

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
  ENABLE_COVERAGE_REPORT(TARGETS ${PHONE_APP})
ENDIF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])

set(PHONE_APP_DIR ${CMAKE_INSTALL_DATADIR}/phone-app)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Check if should build using ubuntu platform api
check_include_file_cxx("ubuntu/ui/ubuntu_ui_session_service.h" USE_UBUNTU_PLATFORM_API)

if (USE_UBUNTU_PLATFORM_API)
    add_definitions(-DUSE_UBUNTU_PLATFORM_API)
endif (USE_UBUNTU_PLATFORM_API)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

find_package(Qt5Core)
find_package(Qt5Contacts)
find_package(Qt5DBus)
find_package(Qt5Gui)
find_package(Qt5Multimedia)
find_package(Qt5Qml)
find_package(Qt5Quick)
find_package(Qt5Test)

execute_process(
    COMMAND qmake -query QT_INSTALL_QML
    OUTPUT_VARIABLE QT_INSTALL_QML
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TP_QT5 REQUIRED TelepathyQt5)
pkg_check_modules(TPL_QT5 REQUIRED TelepathyLoggerQt5)
pkg_check_modules(QTGLIB REQUIRED QtGLib-2.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(NOTIFY REQUIRED libnotify)
pkg_check_modules(MESSAGING_MENU REQUIRED messaging-menu)

add_definitions(-DQT_NO_KEYWORDS)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

set(PHONE_APP phone-app)

set(phone_app_HDRS
    phoneapplication.h
    phoneappdbus.h)

set(phone_app_SRCS
    phoneapplication.cpp
    phoneappdbus.cpp
    main.cpp)

qt5_add_dbus_adaptor(phone_app_SRCS PhoneApp.xml phoneappdbus.h PhoneAppDBus)
add_executable(${PHONE_APP}
    ${phone_app_SRCS}
    )
qt5_use_modules(${PHONE_APP} Core DBus Gui Qml Quick)

install(TARGETS ${PHONE_APP}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

install(PROGRAMS ofono-setup
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

configure_file(org.freedesktop.Telepathy.Client.PhoneApp.service.in org.freedesktop.Telepathy.Client.PhoneApp.service)
install(FILES ${CMAKE_BINARY_DIR}/org.freedesktop.Telepathy.Client.PhoneApp.service DESTINATION share/dbus-1/services)
install(FILES PhoneApp.client DESTINATION share/telepathy/clients)
file(GLOB QML_JS_FILES *.qml *.js)

configure_file(com.canonical.PhoneApp.service.in com.canonical.PhoneApp.service)
install(FILES ${CMAKE_BINARY_DIR}/com.canonical.PhoneApp.service DESTINATION share/dbus-1/services)

install(FILES ${QML_JS_FILES}
    DESTINATION ${PHONE_APP_DIR}
    )

set(QML_DIRS
    DetailViewContact
    DetailViewLiveCall
    DetailViewVoicemail
    DetailViewCommunication
    PanelDialer
    PanelContacts
    PanelCommunications
    Panes
    Widgets
    )

install(DIRECTORY ${QML_DIRS}
    DESTINATION ${PHONE_APP_DIR}
    )

set(ASSETS_DIR assets)

install(DIRECTORY ${ASSETS_DIR}
    DESTINATION ${PHONE_APP_DIR}
    )

install(FILES ${PHONE_APP}.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )

enable_testing()

add_subdirectory(libphoneapp)
add_subdirectory(Ubuntu)
add_subdirectory(PhoneAppApprover)
add_subdirectory(tools)
add_subdirectory(icons)
add_subdirectory(tests)
