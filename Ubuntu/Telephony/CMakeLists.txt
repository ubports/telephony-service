# QML plugin

set(plugin_SRCS
    components.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    )

add_library(telephonyservice-qml SHARED ${plugin_SRCS} ${plugin_HDRS})

qt5_use_modules(telephonyservice-qml Contacts Core Qml Quick)

target_link_libraries(telephonyservice-qml
    ${TP_QT5_LIBRARIES}
    telephonyservice
    )

set(PLUGIN_DIR ${QT_INSTALL_QML}/Ubuntu/Telephony)
install(TARGETS telephonyservice-qml DESTINATION ${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${PLUGIN_DIR})

add_subdirectory(tests)

