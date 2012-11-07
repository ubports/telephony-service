project(telephony-app)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# Standard install paths
include(GNUInstallDirs)

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
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
ENDIF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])

enable_testing()


set(TELEPHONY_APP_DIR ${CMAKE_INSTALL_DATADIR}/telephony-app)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

find_package(Qt5Core)
find_package(Qt5Contacts)
find_package(Qt5DBus)
find_package(Qt5Gui)
find_package(Qt5Qml)
find_package(Qt5Quick)
find_package(Qt5Test)
find_package(Qt5Widgets)
find_package(PkgConfig REQUIRED)

pkg_check_modules(TP_QT5 REQUIRED TelepathyQt5)
pkg_check_modules(TPL_QT5 REQUIRED TelepathyLoggerQt5)
pkg_check_modules(QTGLIB REQUIRED QtGLib-2.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(NOTIFY REQUIRED libnotify)
pkg_check_modules(INDICATE_QT REQUIRED indicate-qt)

add_definitions(-DQT_NO_KEYWORDS)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

set(TELEPHONY_APP telephony-app)

set(telephony_app_HDRS
    telephonyapplication.h
    telephonyappdbus.h)

set(telephony_app_SRCS
    telephonyapplication.cpp
    telephonyappdbus.cpp
    main.cpp)

qt5_add_dbus_adaptor(telephony_app_SRCS TelephonyApp.xml telephonyappdbus.h TelephonyAppDBus)
add_executable(${TELEPHONY_APP}
    ${telephony_app_SRCS}
    )
qt5_use_modules(${TELEPHONY_APP} Core DBus Gui Qml Quick Widgets)

install(TARGETS ${TELEPHONY_APP}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

install(FILES launch-telephony-app 
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

configure_file(org.freedesktop.Telepathy.Client.TelephonyApp.service.in org.freedesktop.Telepathy.Client.TelephonyApp.service)
install(FILES ${CMAKE_BINARY_DIR}/org.freedesktop.Telepathy.Client.TelephonyApp.service DESTINATION share/dbus-1/services)
install(FILES TelephonyApp.client DESTINATION share/telepathy/clients)
file(GLOB QML_JS_FILES *.qml *.js)

configure_file(com.canonical.TelephonyApp.service.in com.canonical.TelephonyApp.service)
install(FILES ${CMAKE_BINARY_DIR}/com.canonical.TelephonyApp.service DESTINATION share/dbus-1/services)

install(FILES ${QML_JS_FILES}
    DESTINATION ${TELEPHONY_APP_DIR}
    )

set(QML_DIRS
    DetailViewCallLog
    DetailViewContact
    DetailViewLiveCall
    DetailViewVoicemail
    DetailViewMessages
    LiveCallDetailView
    PanelDialer
    PanelContacts
    PanelMessages
    Panes
    Widgets
    )

install(DIRECTORY ${QML_DIRS}
    DESTINATION ${TELEPHONY_APP_DIR}
    )

set(ASSETS_DIR assets)

install(DIRECTORY ${ASSETS_DIR}
    DESTINATION ${TELEPHONY_APP_DIR}
    )

set(DUMMYDATA_DIR dummydata)

install(DIRECTORY ${DUMMYDATA_DIR}
    DESTINATION ${TELEPHONY_APP_DIR}
    )

install(FILES ${TELEPHONY_APP}.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )

enable_testing()

add_subdirectory(TelephonyApp)
add_subdirectory(TelephonyAppApprover)
add_subdirectory(icons)
