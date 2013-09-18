
set(qt_SRCS
    callchannelobserver.cpp
    messagingmenu.cpp
    textchannelobserver.cpp
    voicemailindicator.cpp
    )

set(indicator_SRCS main.cpp ${qt_SRCS})

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${MESSAGING_MENU_INCLUDE_DIRS}
    ${GSETTINGS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    ${CMAKE_CURRENT_BINARY_DIR}
    )

link_directories(${MESSAGING_MENU_LIBRARY_DIRS})

add_executable(telephony-service-indicator ${indicator_SRCS} ${indicator_HDRS})
qt5_use_modules(telephony-service-indicator Contacts Core DBus Gui Multimedia Qml)

target_link_libraries(telephony-service-indicator
    ${TP_QT5_LIBRARIES}
    ${NOTIFY_LIBRARIES}
    ${MESSAGING_MENU_LIBRARIES}
    ${GSETTINGS_LIBRARIES}
    telephonyservice
    )

configure_file(org.freedesktop.Telepathy.Client.TelephonyServiceIndicator.service.in org.freedesktop.Telepathy.Client.TelephonyServiceIndicator.service)
install(TARGETS telephony-service-indicator RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.TelephonyServiceIndicator.service DESTINATION share/dbus-1/services)
install(FILES TelephonyServiceIndicator.client DESTINATION share/telepathy/clients)

# Handle i18n in desktop files
set(desktop_FILES telephony-service-sms.desktop telephony-service-call.desktop)

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

add_subdirectory(icons)