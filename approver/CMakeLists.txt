
set(qt_SRCS
    applicationutils.cpp
    approver.cpp
    approverdbus.cpp
    messagingmenu.cpp
    textchannelobserver.cpp
    voicemailindicator.cpp
    )

set(approver_SRCS main.cpp ${qt_SRCS})
qt5_add_dbus_adaptor(approver_SRCS Approver.xml approver/approverdbus.h ApproverDBus)

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${MESSAGING_MENU_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    ${CMAKE_CURRENT_BINARY_DIR}
    )

link_directories(${MESSAGING_MENU_LIBRARY_DIRS})

add_executable(telephony-service-approver ${approver_SRCS} ${approver_HDRS})
qt5_use_modules(telephony-service-approver Contacts Core DBus Gui Multimedia Qml)

if (USE_UBUNTU_PLATFORM_API)
    set(UBUNTU_APP_LIB "-lubuntu_application_api")
endif (USE_UBUNTU_PLATFORM_API)

target_link_libraries(telephony-service-approver
    ${TP_QT5_LIBRARIES}
    ${NOTIFY_LIBRARIES}
    ${MESSAGING_MENU_LIBRARIES}
    ${UBUNTU_APP_LIB}
    telephonyservice
    )

configure_file(org.freedesktop.Telepathy.Client.TelephonyServiceApprover.service.in org.freedesktop.Telepathy.Client.TelephonyServiceApprover.service)
install(TARGETS telephony-service-approver RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.TelephonyServiceApprover.service DESTINATION share/dbus-1/services)
install(FILES TelephonyServiceApprover.client DESTINATION share/telepathy/clients)

# Handle i18n in desktop files
set(desktop_FILES telephony-service-sms.desktop telephony-service-call.desktop telephony-service-approver.desktop)

foreach(DESKTOP_FILE ${desktop_FILES})
    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE})
    file(STRINGS ${DESKTOP_FILE}.in DESKTOP_FILE_CONTENTS)
    foreach(LINE ${DESKTOP_FILE_CONTENTS})
        string(REGEX REPLACE "tr\\\(\"(.*)\"\\\)" "\\1" LINE "${LINE}")
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE} "${LINE}\n")
    endforeach(LINE)
endforeach(DESKTOP_FILE)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/telephony-service-sms.desktop ${CMAKE_CURRENT_BINARY_DIR}/telephony-service-call.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/telephony-service-approver.desktop DESTINATION /etc/xdg/autostart)

add_subdirectory(icons)
