# QML plugin

set(plugin_SRCS
    abstractloggermodel.cpp
    calllogmodel.cpp
    calllogproxymodel.cpp
    components.cpp
    channelhandler.cpp
    chatmanager.cpp
    callmanager.cpp
    conversationlogmodel.cpp
    messagelogmodel.cpp
    messagesproxymodel.cpp
    telepathyhelper.cpp
    )

set(plugin_HDRS
    abstractloggermodel.h
    calllogmodel.h
    calllogproxymodel.h
    components.h
    channelhandler.h
    chatmanager.h
    callmanager.h
    conversationlogmodel.h
    messagelogmodel.h
    messagesproxymodel.h
    telepathyhelper.h
    )

qt4_wrap_cpp(plugin_MOC_SRCS ${plugin_HDRS})

include_directories(
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
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

