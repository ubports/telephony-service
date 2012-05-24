project(telephony-app)
cmake_minimum_required(VERSION 2.8)

# Standard install paths
include(GNUInstallDirs)

set(TELEPHONY_APP_DIR ${CMAKE_INSTALL_DATADIR}/telephony-app)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

find_package(Qt4 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(TP_QT4 REQUIRED TelepathyQt4)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_INCLUDE_DIR}
    )

set(TELEPHONY_APP telephony-app)

add_executable(${TELEPHONY_APP} main.cpp)

target_link_libraries(${TELEPHONY_APP}
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTGUI_LIBRARIES}
    ${QT_QTDECLARATIVE_LIBRARIES}
    )

install(TARGETS ${TELEPHONY_APP}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

file(GLOB QML_JS_FILES *.qml *.js)

install(FILES ${QML_JS_FILES}
    DESTINATION ${TELEPHONY_APP_DIR}
    )

set(QML_DIRS
    ContactUtils
    DetailViewContact
    DetailViewKeypad
    DetailViewLiveCall
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

add_subdirectory(TelephonyApp)
