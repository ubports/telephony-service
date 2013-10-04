set(library_SRCS
    applicationutils.cpp
    callentry.cpp
    callmanager.cpp
    channelobserver.cpp
    chatmanager.cpp
    contactutils.cpp
    phoneutils.cpp
    ringtone.cpp
    telepathyhelper.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${GSETTINGS_INCLUDE_DIRS}
    )

add_library(telephonyservice STATIC ${library_SRCS} ${library_HDRS})
target_link_libraries(telephonyservice ${TP_QT5_LIBRARIES} ${GSETTINGS_LIBRARIES})

qt5_use_modules(telephonyservice Contacts Core DBus Multimedia Qml Quick)

add_subdirectory(tests)
