
set(qt_SRCS telephonyappapprover.cpp)
set(qt_HDRS telephonyappapprover.h)
set(approver_SRCS main.cpp ${qt_SRCS})
set(approver_HDRS ${qt_HDRS})
qt4_automoc(${qt_SRCS})
QT4_WRAP_CPP(qt_moc_SRCS ${qt_HDRS})

include_directories(
    ${QT_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${TP_QT4_INCLUDE_DIRS})

add_executable(telephony-app-approver ${approver_SRCS} ${approver_HDRS} ${qt_moc_SRCS})

target_link_libraries(telephony-app-approver
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTGUI_LIBRARIES}
    ${QT_QTDBUS_LIBRARIES}    
    ${TP_QT4_LIBRARIES})

install(TARGETS telephony-app-approver RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES org.freedesktop.Telepathy.Client.TelephonyAppApprover.service DESTINATION share/dbus-1/services)
install(FILES TelephonyAppApprover.client DESTINATION share/telepathy/clients)
