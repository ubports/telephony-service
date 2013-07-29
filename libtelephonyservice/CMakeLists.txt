set(library_SRCS
    applicationutils.cpp
    callentry.cpp
    callmanager.cpp
    channelobserver.cpp
    chatmanager.cpp
    phoneutils.cpp
    ringtone.cpp
    telepathyhelper.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    )

if (USE_UBUNTU_PLATFORM_API)
    set(UBUNTU_APP_LIB "-lubuntu_application_api")
endif (USE_UBUNTU_PLATFORM_API)

add_library(telephonyservice STATIC ${library_SRCS} ${library_HDRS})
target_link_libraries(telephonyservice ${TP_QT5_LIBRARIES} ${UBUNTU_APP_LIB})

qt5_use_modules(telephonyservice Contacts Core DBus Multimedia Qml Quick)

add_subdirectory(tests)
