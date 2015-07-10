include_directories(${TP_QT5_INCLUDE_DIRS}
                    ${CMAKE_SOURCE_DIR}/libtelephonyservice
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${GSETTINGS_QT_INCLUDE_DIRS})

configure_file(dbus-session.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dbus-session.conf)

add_executable(NotificationsMock NotificationsMock.cpp)
qt5_use_modules(NotificationsMock Core DBus)

qt5_add_dbus_interface(mockcontroller_SRCS mock/MockConnection.xml MockConnectionInterface)
add_library(mockcontroller STATIC mockcontroller.cpp mockcontroller.h ${mockcontroller_SRCS})
qt5_use_modules(mockcontroller Core DBus)

add_library(telepathytest STATIC telepathytest.cpp telepathytest.h)
qt5_use_modules(telepathytest Core DBus)
target_link_libraries(telepathytest telephonyservice ${TP_QT5_LIBRARIES} ${GSETTINGS_QT_LDFLAGS})

add_subdirectory(mock)
add_subdirectory(dbus-services)
