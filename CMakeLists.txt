project(telephony-app)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# Standard install paths
include(GNUInstallDirs)

set(TELEPHONY_APP_DIR ${CMAKE_INSTALL_DATADIR}/telephony-app)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

find_package(Qt4 REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(QtMobility COMPONENTS Contacts REQUIRED)

pkg_check_modules(TP_QT4 REQUIRED TelepathyQt4)
pkg_check_modules(TPL_QT4 REQUIRED TelepathyLoggerQt4)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(NOTIFY REQUIRED libnotify)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/qtsingleapplication
    )

set(TELEPHONY_APP telephony-app)

set(telephony_app_HDRS
    telephonyapplication.h
    telephonyappdbus.h)

set(telephony_app_SRCS
    telephonyapplication.cpp
    telephonyappdbus.cpp
    main.cpp)

qt4_add_dbus_adaptor(telephony_app_SRCS TelephonyApp.xml telephonyappdbus.h TelephonyAppDBus)
qt4_wrap_cpp(telephonyapplication_MOC_SRCS ${telephony_app_HDRS})
add_executable(${TELEPHONY_APP}
    ${telephonyapplication_MOC_SRCS}
    ${telephony_app_SRCS}
    )

target_link_libraries(${TELEPHONY_APP}
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTGUI_LIBRARIES}
    ${QT_QTNETWORK_LIBRARIES}
    ${QT_QTDECLARATIVE_LIBRARIES}
    ${QT_QTDBUS_LIBRARIES}
    qtsingleapplication
    )

install(TARGETS ${TELEPHONY_APP}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

configure_file(org.freedesktop.Telepathy.Client.TelephonyApp.service.in org.freedesktop.Telepathy.Client.TelephonyApp.service)
install(FILES ${CMAKE_BINARY_DIR}/org.freedesktop.Telepathy.Client.TelephonyApp.service DESTINATION share/dbus-1/services)
install(FILES TelephonyApp.client DESTINATION share/telepathy/clients)
file(GLOB QML_JS_FILES *.qml *.js)

configure_file(com.canonical.TelephonyApp.service.in com.canonical.TelephonyApp.in.service)
install(FILES ${CMAKE_BINARY_DIR}/org.freedesktop.Telepathy.Client.TelephonyApp.service DESTINATION share/dbus-1/services)

install(FILES ${QML_JS_FILES}
    DESTINATION ${TELEPHONY_APP_DIR}
    )

set(QML_DIRS
    ContactUtils
    DetailViewCallLog
    DetailViewContact
    DetailViewKeypad
    DetailViewLiveCall
    DetailViewVoicemail
    DetailViewMessages
    LiveCallDetailView
    PanelCalls
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

install(FILES ${TELEPHONY_APP}.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons
    )

add_subdirectory(qtsingleapplication)
add_subdirectory(TelephonyApp)
add_subdirectory(TelephonyAppApprover)
