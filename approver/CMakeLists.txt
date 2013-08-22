
set(qt_SRCS
    approver.cpp
    approverdbus.cpp
    )

set(approver_SRCS main.cpp ${qt_SRCS})
qt5_add_dbus_adaptor(approver_SRCS Approver.xml approver/approverdbus.h ApproverDBus)

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${GSETTINGS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    ${CMAKE_CURRENT_BINARY_DIR}
    )

link_directories(${MESSAGING_MENU_LIBRARY_DIRS})

add_executable(telephony-service-approver ${approver_SRCS} ${approver_HDRS})
qt5_use_modules(telephony-service-approver Contacts Core DBus Gui Multimedia Qml)

target_link_libraries(telephony-service-approver
    ${TP_QT5_LIBRARIES}
    ${NOTIFY_LIBRARIES}
    ${GSETTINGS_LIBRARIES}
    telephonyservice
    )

configure_file(org.freedesktop.Telepathy.Client.TelephonyServiceApprover.service.in org.freedesktop.Telepathy.Client.TelephonyServiceApprover.service)
install(TARGETS telephony-service-approver RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.TelephonyServiceApprover.service DESTINATION share/dbus-1/services)
install(FILES TelephonyServiceApprover.client DESTINATION share/telepathy/clients)
