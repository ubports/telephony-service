include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    ${TP_QT5_INCLUDE_DIRS}
    )

if (DBUS_RUNNER)
    generate_test(HandlerTest True handlercontroller.cpp approver.cpp)
endif(DBUS_RUNNER)
