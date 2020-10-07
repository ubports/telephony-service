# QML plugin

set(plugin_SRCS
    presencerequest.cpp
    participantsmodel.cpp
    components.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    )

add_library(telephonyservice-qml SHARED ${plugin_SRCS} ${plugin_HDRS})

target_link_libraries(telephonyservice-qml
    ${TP_QT5_LIBRARIES}
    telephonyservice
    Qt5::Contacts
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    )

enable_coverage(telephonyservice-qml)

configure_file(qmldir.in ${CMAKE_CURRENT_BINARY_DIR}/qmldir)
set(PLUGIN_DIR ${QT_INSTALL_QML}/Ubuntu/Telephony)
install(TARGETS telephonyservice-qml DESTINATION ${PLUGIN_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qmldir DESTINATION ${PLUGIN_DIR})

add_subdirectory(PhoneNumber)
