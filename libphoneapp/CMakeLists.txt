set(library_SRCS
    contactmodel.cpp
    contactaddress.cpp
    contactdetail.cpp
    contactemailaddress.cpp
    contactentry.cpp
    contactmodel.cpp
    contactname.cpp
    contactonlineaccount.cpp
    contactphonenumber.cpp
    callentry.cpp
    callmanager.cpp
    channelobserver.cpp
    chatmanager.cpp
    ringtone.cpp
    telepathyhelper.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${TPL_QT5_INCLUDE_DIRS}
    ${QTGLIB_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    )


add_library(phoneapp STATIC ${library_SRCS} ${library_HDRS})
target_link_libraries(phoneapp ${TP_QT5_LIBRARIES})

qt5_use_modules(phoneapp Contacts Core DBus Multimedia Qml Quick)

add_subdirectory(tests)
