add_library(mockcontroller STATIC mockcontroller.cpp mockcontroller.h)
qt5_use_modules(mockcontroller Core DBus)

add_subdirectory(mock)
