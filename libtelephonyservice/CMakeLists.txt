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
    ${TPL_QT5_INCLUDE_DIRS}
    ${QTGLIB_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    )


add_library(telephonyservice STATIC ${library_SRCS} ${library_HDRS})
target_link_libraries(telephonyservice ${TP_QT5_LIBRARIES})

qt5_use_modules(telephonyservice Contacts Core DBus Multimedia Qml Quick)

add_subdirectory(tests)
