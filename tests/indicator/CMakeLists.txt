include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_BINARY_DIR}/../common
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    ${CMAKE_SOURCE_DIR}/indicator
    ${MESSAGING_MENU_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${TP_QT5_INCLUDE_DIRS}
    ${HISTORY_INCLUDE_DIRS}
    ${GSETTINGS_QT_INCLUDE_DIRS}
    )

generate_telepathy_test(MessagingMenuTest
              SOURCES MessagingMenuTest.cpp messagingmenumock.cpp ${CMAKE_SOURCE_DIR}/indicator/messagingmenu.cpp
              LIBRARIES  ${TP_QT5_LIBRARIES} telephonyservice mockcontroller telepathytest ${HISTORY_LIBRARIES}
              TASKS --task ${CMAKE_BINARY_DIR}/indicator/telephony-service-indicator -p -platform -p offscreen
                    --task-name telephony-service-indicator
                    --wait-for com.canonical.TelephonyServiceHandler
                    --ignore-return)

qt5_add_dbus_interface(
        qt_SRCS
        "${DATA_DIR}/org.freedesktop.Notifications.xml"
        NotificationsInterface
)
generate_telepathy_test(AuthHandlerTest
              SOURCES AuthHandlerTest.cpp ${CMAKE_SOURCE_DIR}/indicator/authhandler.cpp
                      ${CMAKE_SOURCE_DIR}/indicator/notificationmenu.cpp ${qt_SRCS}
              LIBRARIES ${TP_QT5_LIBRARIES} telephonyservice mockcontroller telepathytest ${HISTORY_LIBRARIES} ${NOTIFY_LIBRARIES})
generate_test(NotificationMenuTest
              SOURCES NotificationMenuTest.cpp ${CMAKE_SOURCE_DIR}/indicator/notificationmenu.cpp
              LIBRARIES ${NOTIFY_LIBRARIES}
              QT5_MODULES Core DBus Test
              USE_DBUS TRUE)
