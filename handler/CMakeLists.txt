
set(qt_SRCS
    callhandler.cpp
    handler.cpp
    handlerdbus.cpp
    texthandler.cpp
    )

set(handler_SRCS main.cpp ${qt_SRCS})
qt5_add_dbus_adaptor(handler_SRCS Handler.xml handler/handlerdbus.h HandlerDBus)

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    ${CMAKE_CURRENT_BINARY_DIR}
    )

add_executable(telephony-service-handler ${handler_SRCS} ${handler_HDRS})
qt5_use_modules(telephony-service-handler Contacts Core DBus Qml)

target_link_libraries(telephony-service-handler
    ${TP_QT5_LIBRARIES}
    telephonyservice
    )

configure_file(com.canonical.TelephonyServiceHandler.service.in com.canonical.TelephonyServiceHandler.service)
configure_file(org.freedesktop.Telepathy.Client.TelephonyServiceHandler.service.in org.freedesktop.Telepathy.Client.TelephonyServiceHandler.service)

install(TARGETS telephony-service-handler RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.TelephonyServiceHandler.service DESTINATION share/dbus-1/services)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/com.canonical.TelephonyServiceHandler.service DESTINATION share/dbus-1/services)
install(FILES TelephonyServiceHandler.client DESTINATION share/telepathy/clients)
