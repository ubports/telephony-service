# QML plugin

set(plugin_SRCS components.cpp channelhandler.cpp chatmanager.cpp callmanager.cpp telepathyhelper.cpp)
set(plugin_HDRS components.h channelhandler.h chatmanager.h callmanager.h telepathyhelper.h)

qt4_wrap_cpp(plugin_MOC_SRCS ${plugin_HDRS})

include_directories(
    ${QT_INCLUDE_DIR}
    ${TP_QT4_INCLUDE_DIRS})

add_library(telephony-qml SHARED ${plugin_SRCS} ${plugin_HDRS} ${plugin_MOC_SRCS})

target_link_libraries(telephony-qml
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTDECLARATIVE_LIBRARIES}
    ${TP_QT4_LIBRARIES})

set(PLUGIN_DIR ${QT_IMPORTS_DIR}/TelephonyApp)
install(TARGETS telephony-qml DESTINATION ${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${PLUGIN_DIR})

