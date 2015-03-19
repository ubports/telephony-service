configure_file(dbus-test-wrapper.sh.in ${CMAKE_CURRENT_BINARY_DIR}/dbus-test-wrapper.sh)
configure_file(dbus-session.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dbus-session.conf)

add_library(mockcontroller STATIC mockcontroller.cpp mockcontroller.h)
qt5_use_modules(mockcontroller Core DBus)

add_subdirectory(mock)
add_subdirectory(dbus-services)
