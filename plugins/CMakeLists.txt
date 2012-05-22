
set(plugin_SRCS components.cpp channelhandler.cpp)
set(plugin_HDRS components.h channelhandler.h)

qt4_wrap_cpp(plugin_MOC_SRCS ${plugin_HDRS})

include_directories(${QT_INCLUDE_DIR})

add_library(telephony-qml SHARED ${plugin_SRCS} ${plugin_HDRS})

target_link_libraries(telephony-qml
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTDECLARATIVE_LIBRARIES})


install(DIRECTORY TelephonyApp DESTINATION ${QT_IMPORTS_DIR})
install(TARGETS telephony-qml DESTINATION ${QT_IMPORTS_DIR}/TelephonyApp)
