# QML plugin

set(plugin_SRCS
    abstractloggermodel.cpp
    calllogmodel.cpp
    contactproxymodel.cpp
    conversationfeeditem.cpp
    conversationfeedmodel.cpp
    conversationaggregatormodel.cpp
    conversationproxymodel.cpp
    components.cpp
    messagelogmodel.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${TPL_QT5_INCLUDE_DIRS}
    ${QTGLIB_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libtelephonyapp
    )

add_library(telephony-qml SHARED ${plugin_SRCS} ${plugin_HDRS})

qt5_use_modules(telephony-qml Contacts Core Qml Quick)

target_link_libraries(telephony-qml
    ${TP_QT5_LIBRARIES}
    ${TPL_QT5_LIBRARIES}
    ${GLIB_LIBRARIES}
    telephonyapp
    )

set(PLUGIN_DIR ${QT_INSTALL_IMPORTS}/TelephonyApp)
install(TARGETS telephony-qml DESTINATION ${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${PLUGIN_DIR})

add_subdirectory(tests)

