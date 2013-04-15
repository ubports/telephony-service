set(PHONE_APP phone-app)

set(phone_app_HDRS
    phoneapplication.h
    phoneappdbus.h)

set(phone_app_SRCS
    phoneapplication.cpp
    phoneappdbus.cpp
    main.cpp)

qt5_add_dbus_adaptor(phone_app_SRCS PhoneApp.xml phoneappdbus.h PhoneAppDBus)
add_executable(${PHONE_APP}
    ${phone_app_SRCS}
    )
qt5_use_modules(${PHONE_APP} Core DBus Gui Qml Quick)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

install(TARGETS ${PHONE_APP}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )


configure_file(com.canonical.PhoneApp.service.in com.canonical.PhoneApp.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/com.canonical.PhoneApp.service DESTINATION share/dbus-1/services)
install(FILES ${PHONE_APP}.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )

add_subdirectory(qml)
