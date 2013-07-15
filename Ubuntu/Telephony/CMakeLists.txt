# QML plugin

set(plugin_SRCS
    calllogmodel.cpp
    contactproxymodel.cpp
    conversationfeeditem.cpp
    conversationfeedmodel.cpp
    conversationaggregatormodel.cpp
    conversationproxymodel.cpp
    components.cpp
    messagelogmodel.cpp
    telepathylogreader.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${TPL_QT5_INCLUDE_DIRS}
    ${QTGLIB_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libphoneapp
    )

add_library(phone-qml SHARED ${plugin_SRCS} ${plugin_HDRS})

qt5_use_modules(phone-qml Contacts Core Qml Quick)

target_link_libraries(phone-qml
    ${TP_QT5_LIBRARIES}
    ${TPL_QT5_LIBRARIES}
    ${GLIB_LIBRARIES}
    phoneapp
    )

set(PLUGIN_DIR ${QT_INSTALL_QML}/Ubuntu/Telephony)
install(TARGETS phone-qml DESTINATION ${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${PLUGIN_DIR})

add_subdirectory(tests)

