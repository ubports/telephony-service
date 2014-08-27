set(library_SRCS
    accountentry.cpp
    audiooutput.cpp
    applicationutils.cpp
    callentry.cpp
    callmanager.cpp
    callnotification.cpp
    channelobserver.cpp
    chatmanager.cpp
    contactutils.cpp
    greetercontacts.cpp
    phoneutils.cpp
    ringtone.cpp
    telepathyhelper.cpp
    tonegenerator.cpp
    ussdmanager.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${GSETTINGS_QT_INCLUDE_DIRS}
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
                      ${GSETTINGS_QT_LDFLAGS})

qt5_use_modules(telephonyservice Contacts Core DBus Feedback Multimedia Qml Quick)

add_subdirectory(tests)
