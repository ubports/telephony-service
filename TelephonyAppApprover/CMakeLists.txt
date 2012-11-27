
set(qt_SRCS
    messagingmenu.cpp
    telephonyappapprover.cpp
    textchannelobserver.cpp
    voicemailindicator.cpp
    telepathyhelper.cpp
    )

set(approver_SRCS main.cpp ${qt_SRCS})
set(desktop_FILES telephony-app-sms.desktop telephony-app-phone.desktop)

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${INDICATE_QT_INCLUDE_DIRS}
    ${MESSAGING_MENU_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libtelephonyapp
    )

link_directories(${MESSAGING_MENU_LIBRARY_DIRS})

add_executable(telephony-app-approver ${approver_SRCS} ${approver_HDRS})
qt5_use_modules(telephony-app-approver Contacts Core Gui DBus Qml Widgets)

target_link_libraries(telephony-app-approver
    ${TP_QT5_LIBRARIES}
    ${NOTIFY_LIBRARIES}
    ${INDICATE_QT_LIBRARIES}
    ${MESSAGING_MENU_LIBRARIES}
    telephonyapp
    )

install(TARGETS telephony-app-approver RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES org.freedesktop.Telepathy.Client.TelephonyAppApprover.service DESTINATION share/dbus-1/services)
install(FILES TelephonyAppApprover.client DESTINATION share/telepathy/clients)
install(FILES ${desktop_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES telephony-app-approver.desktop DESTINATION /etc/xdg/autostart)

add_subdirectory(icons)
