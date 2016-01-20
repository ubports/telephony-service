if (PULSEAUDIO_FOUND)
    add_definitions(-DUSE_PULSEAUDIO)
    set(USE_PULSEAUDIO ON)
    set(QPULSEAUDIOENGINE_CPP qpulseaudioengine.cpp)
endif (PULSEAUDIO_FOUND)

set(qt_SRCS
    audioroutemanager.cpp
    callhandler.cpp
    displaynamesettings.cpp
    handler.cpp
    handlerdbus.cpp
    powerdaudiomodemediator.cpp
    powerddbus.cpp
    texthandler.cpp
    ${QPULSEAUDIOENGINE_CPP}
    )

set(handler_SRCS main.cpp ${qt_SRCS})
qt5_add_dbus_adaptor(handler_SRCS Handler.xml handler/handlerdbus.h HandlerDBus)

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GSETTINGS_QT_INCLUDE_DIRS}
    ${PULSEAUDIO_INCLUDE_DIRS}
    )

add_executable(telephony-service-handler ${handler_SRCS} ${handler_HDRS})
qt5_use_modules(telephony-service-handler Contacts Core DBus Qml)

target_link_libraries(telephony-service-handler
    ${TP_QT5_LIBRARIES}
    ${GSETTINGS_QT_LDFLAGS}
    telephonyservice
    ${PULSEAUDIO_LIBRARIES}
    )

enable_coverage(telephony-service-handler)

configure_file(com.canonical.TelephonyServiceHandler.service.in com.canonical.TelephonyServiceHandler.service)
configure_file(org.freedesktop.Telepathy.Client.TelephonyServiceHandler.service.in org.freedesktop.Telepathy.Client.TelephonyServiceHandler.service)

install(TARGETS telephony-service-handler RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.TelephonyServiceHandler.service DESTINATION share/dbus-1/services)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/com.canonical.TelephonyServiceHandler.service DESTINATION share/dbus-1/services)
install(FILES TelephonyServiceHandler.client DESTINATION share/telepathy/clients)
