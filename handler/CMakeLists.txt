
set(qt_SRCS
    callhandler.cpp
    phoneapphandler.cpp
    phoneapphandlerdbus.cpp
    texthandler.cpp
    )

set(handler_SRCS main.cpp ${qt_SRCS})
qt5_add_dbus_adaptor(handler_SRCS PhoneAppHandler.xml handler/phoneapphandlerdbus.h PhoneAppHandlerDBus)

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libphoneapp
    ${CMAKE_CURRENT_BINARY_DIR}
    )

add_executable(phone-app-handler ${handler_SRCS} ${handler_HDRS})
qt5_use_modules(phone-app-handler Contacts Core DBus Qml)

target_link_libraries(phone-app-handler
    ${TP_QT5_LIBRARIES}
    phoneapp
    )

install(TARGETS phone-app-handler RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES org.freedesktop.Telepathy.Client.PhoneAppHandler.service DESTINATION share/dbus-1/services)
install(FILES PhoneAppHandler.client DESTINATION share/telepathy/clients)
