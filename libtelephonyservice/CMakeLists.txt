set(library_SRCS
    accountentry.cpp
    accountentryfactory.cpp
    accountlist.cpp
    audiooutput.cpp
    applicationutils.cpp
    callentry.cpp
    callmanager.cpp
    callnotification.cpp
    channelobserver.cpp
    chatmanager.cpp
    chatentry.cpp
    contactutils.cpp
    contactwatcher.cpp
    greetercontacts.cpp
    ofonoaccountentry.cpp
    participant.cpp
    phoneutils.cpp
    protocol.cpp
    protocolmanager.cpp
    ringtone.cpp
    rolesinterface.cpp
    telepathyhelper.cpp
    tonegenerator.cpp
    ussdmanager.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${LibPhoneNumber_INCLUDE_DIRS}
    ${URL_DISPATCHER_INCLUDE_DIRS})

add_library(telephonyservice STATIC ${library_SRCS} ${library_HDRS})
set_target_properties(telephonyservice PROPERTIES COMPILE_DEFINITIONS AS_BUSNAME=systemBus)
target_link_libraries(telephonyservice
                      ${TP_QT5_LIBRARIES}
                      ${NOTIFY_LIBRARIES}
                      ${LibPhoneNumber_LIBRARIES}
                      ${URL_DISPATCHER_LIBRARIES})

qt5_use_modules(telephonyservice Contacts Core DBus Feedback Multimedia Qml Quick Gui)

enable_coverage(telephonyservice)
