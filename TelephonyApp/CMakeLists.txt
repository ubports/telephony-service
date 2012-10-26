# QML plugin

set(plugin_SRCS
    abstractloggermodel.cpp
    calllogmodel.cpp
    contactaddress.cpp
    contactdetail.cpp
    contactemailaddress.cpp
    contactentry.cpp
    contactmodel.cpp
    contactname.cpp
    contactonlineaccount.cpp
    contactphonenumber.cpp
    contactproxymodel.cpp
    conversationfeeditem.cpp
    conversationfeedmodel.cpp
    conversationaggregatormodel.cpp
    conversationproxymodel.cpp
    components.cpp
    channelhandler.cpp
    channelobserver.cpp
    chatmanager.cpp
    callentry.cpp
    callmanager.cpp
    messagelogmodel.cpp
    telepathyhelper.cpp
    modelsectioncounter.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${TPL_QT5_INCLUDE_DIRS}
    # TODO: enable this include once we get the proper Qt5 packages
    #${Qt5Quick_PRIVATE_INCLUDE_DIRS}
    ${QTGLIB_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    )

add_library(telephony-qml SHARED ${plugin_SRCS} ${plugin_HDRS})

qt5_use_modules(telephony-qml Contacts Core Qml Quick)

target_link_libraries(telephony-qml
    ${TP_QT5_LIBRARIES}
    ${TPL_QT5_LIBRARIES}
    ${GLIB_LIBRARIES}
    )

# FIXME: check how to detect that using cmake
set(QT_IMPORTS_DIR /opt/qt5/imports)
set(PLUGIN_DIR ${QT_IMPORTS_DIR}/TelephonyApp)
install(TARGETS telephony-qml DESTINATION ${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${PLUGIN_DIR})

add_subdirectory(tests)

