# QML plugin

set(plugin_SRCS
    abstractloggermodel.cpp
    calllogmodel.cpp
    calllogproxymodel.cpp
    contactaddress.cpp
    contactcustomid.cpp
    contactdetail.cpp
    contactemailaddress.cpp
    contactentry.cpp
    contactmodel.cpp
    contactname.cpp
    contactonlineaccount.cpp
    contactphonenumber.cpp
    contactproxymodel.cpp
    components.cpp
    channelhandler.cpp
    channelobserver.cpp
    chatmanager.cpp
    callentry.cpp
    callmanager.cpp
    conversationlogmodel.cpp
    messagelogmodel.cpp
    messagesproxymodel.cpp
    telepathyhelper.cpp
    buttonmaskeffect.cpp
    )

set(plugin_HDRS
    abstractloggermodel.h
    calllogmodel.h
    calllogproxymodel.h
    contactaddress.h
    contactdetail.h
    contactemailaddress.h
    contactentry.h
    contactmodel.h
    contactname.h
    contactonlineaccount.h
    contactphonenumber.h
    contactproxymodel.h
    components.h
    channelhandler.h
    channelobserver.h
    chatmanager.h
    callentry.h
    callmanager.h
    conversationlogmodel.h
    messagelogmodel.h
    messagesproxymodel.h
    telepathyhelper.h
    buttonmaskeffect.h
    )

qt4_wrap_cpp(plugin_MOC_SRCS ${plugin_HDRS})

include_directories(
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTDECLARATIVE_INCLUDE_DIR}
    ${QTMOBILITY_CONTACTS_INCLUDE_DIR}
    ${QTMOBILITY_INCLUDE_DIR}
    ${TP_QT4_INCLUDE_DIRS}
    ${TPL_QT4_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    )

add_library(telephony-qml SHARED ${plugin_SRCS} ${plugin_HDRS} ${plugin_MOC_SRCS})

target_link_libraries(telephony-qml
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTDECLARATIVE_LIBRARIES}
    ${QTMOBILITY_CONTACTS_LIBRARY}
    ${TP_QT4_LIBRARIES}
    ${TPL_QT4_LIBRARIES}
    ${GLIB_LIBRARIES}
    )

set(PLUGIN_DIR ${QT_IMPORTS_DIR}/TelephonyApp)
install(TARGETS telephony-qml DESTINATION ${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${PLUGIN_DIR})

