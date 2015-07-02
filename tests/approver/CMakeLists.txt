include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    ${CMAKE_BINARY_DIR}/tests/common
    ${TP_QT5_INCLUDE_DIRS}
    ${GSETTINGS_QT_INCLUDE_DIRS}
    )

generate_telepathy_test(ApproverTest
                        SOURCES ApproverTest.cpp approvercontroller.cpp
                        TASKS --task dbus-monitor --ignore-return --task gdb -p -x -p /tmp/gdbcomm -p --args -p ${CMAKE_BINARY_DIR}/approver/telephony-service-approver --task-name telephony-service-approver --wait-for com.canonical.TelephonyServiceHandler --ignore-return
                        #TASKS --task ${CMAKE_BINARY_DIR}/approver/telephony-service-approver --task-name telephony-service-approver --wait-for com.canonical.TelephonyServiceHandler --ignore-return
                        WAIT_FOR org.freedesktop.Telepathy.Client.TelephonyServiceApprover)
