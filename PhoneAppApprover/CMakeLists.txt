
set(qt_SRCS
    messagingmenu.cpp
    phoneappapprover.cpp
    phoneappapproverdbus.cpp
    phoneapputils.cpp
    textchannelobserver.cpp
    voicemailindicator.cpp
    )

set(approver_SRCS main.cpp ${qt_SRCS})
set(desktop_FILES phone-app-sms.desktop phone-app-call.desktop)
qt5_add_dbus_adaptor(approver_SRCS PhoneAppApprover.xml PhoneAppApprover/phoneappapproverdbus.h PhoneAppApproverDBus)

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${MESSAGING_MENU_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libphoneapp
    )

link_directories(${MESSAGING_MENU_LIBRARY_DIRS})

add_executable(phone-app-approver ${approver_SRCS} ${approver_HDRS})
qt5_use_modules(phone-app-approver Contacts Core DBus Gui Multimedia Qml)

if (USE_UBUNTU_PLATFORM_API)
    set(UBUNTU_APP_LIB "-lubuntu_application_api")
endif (USE_UBUNTU_PLATFORM_API)

target_link_libraries(phone-app-approver
    ${TP_QT5_LIBRARIES}
    ${NOTIFY_LIBRARIES}
    ${MESSAGING_MENU_LIBRARIES}
    ${UBUNTU_APP_LIB}
    phoneapp
    )

install(TARGETS phone-app-approver RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES org.freedesktop.Telepathy.Client.PhoneAppApprover.service DESTINATION share/dbus-1/services)
install(FILES PhoneAppApprover.client DESTINATION share/telepathy/clients)
install(FILES ${desktop_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES phone-app-approver.desktop DESTINATION /etc/xdg/autostart)

add_subdirectory(icons)
