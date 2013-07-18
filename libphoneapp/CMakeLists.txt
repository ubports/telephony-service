set(library_SRCS
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


add_library(phoneapp STATIC ${library_SRCS} ${library_HDRS})
target_link_libraries(phoneapp ${TP_QT5_LIBRARIES})

qt5_use_modules(phoneapp Contacts Core DBus Multimedia Qml Quick)

add_subdirectory(tests)
