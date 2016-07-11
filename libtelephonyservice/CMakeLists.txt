set(library_SRCS
    accountentry.cpp
    accountentryfactory.cpp
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
    multimediaaccountentry.cpp
    ofonoaccountentry.cpp
    participant.cpp
    phoneutils.cpp
    protocol.cpp
    protocolmanager.cpp
    ringtone.cpp
    telepathyhelper.cpp
    tonegenerator.cpp
    ussdmanager.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${LibPhoneNumber_INCLUDE_DIRS}
    )

if (USE_UBUNTU_PLATFORM_API)
    set(UBUNTU_APP_LIB "-lubuntu_application_api")
endif (USE_UBUNTU_PLATFORM_API)

add_library(telephonyservice STATIC ${library_SRCS} ${library_HDRS})
set_target_properties(telephonyservice PROPERTIES COMPILE_DEFINITIONS AS_BUSNAME=systemBus)
target_link_libraries(telephonyservice
                      ${TP_QT5_LIBRARIES}
                      ${UBUNTU_APP_LIB}
                      ${NOTIFY_LIBRARIES}
                      ${LibPhoneNumber_LIBRARIES})

qt5_use_modules(telephonyservice Contacts Core DBus Feedback Multimedia Qml Quick)

enable_coverage(telephonyservice)
