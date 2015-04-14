include_directories(${TP_QT5_INCLUDE_DIRS}
                    ${CMAKE_SOURCE_DIR}/libtelephonyservice)

configure_file(dbus-session.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dbus-session.conf)

add_library(mockcontroller STATIC mockcontroller.cpp mockcontroller.h)
qt5_use_modules(mockcontroller Core DBus)

add_library(telepathytest STATIC telepathytest.cpp telepathytest.h)
qt5_use_modules(telepathytest Core DBus)
target_link_libraries(telepathytest telephonyservice ${TP_QT5_LIBRARIES})

add_subdirectory(mock)
add_subdirectory(dbus-services)
