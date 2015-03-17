configure_file(dbus-test-wrapper.sh.in ${CMAKE_CURRENT_BINARY_DIR}/dbus-test-wrapper.sh)

add_library(mockcontroller STATIC mockcontroller.cpp mockcontroller.h)
qt5_use_modules(mockcontroller Core DBus)

add_subdirectory(mock)
