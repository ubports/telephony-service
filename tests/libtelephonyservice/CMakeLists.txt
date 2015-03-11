set(LIBTELEPHONYSERVICE_DIR ${CMAKE_SOURCE_DIR}/libtelephonyservice)
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${LIBTELEPHONYSERVICE_DIR}
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/tests/common
    )

add_executable(GreeterContactsTestServerExe GreeterContactsTestServer.cpp)
qt5_use_modules(GreeterContactsTestServerExe Core DBus)

add_executable(GreeterContactsTestExe GreeterContactsTest.cpp ${LIBTELEPHONYSERVICE_DIR}/greetercontacts.cpp)
set_target_properties(GreeterContactsTestExe PROPERTIES COMPILE_DEFINITIONS "AS_BUSNAME=sessionBus;CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
qt5_use_modules(GreeterContactsTestExe Contacts Core DBus Test)

add_test(NAME GreeterContactsTest
    COMMAND env
        -u LD_PRELOAD  # fakeroot's preload doesn't play well with dbus-test-runner
        -u LD_LIBRARY_PATH  # fakeroot fills this too
        XDG_SESSION_CLASS=greeter
        XDG_GREETER_DATA_DIR=${CMAKE_BINARY_DIR}/Testing/Temporary
        dbus-test-runner
        --task ${CMAKE_CURRENT_BINARY_DIR}/GreeterContactsTestServerExe
        --task-name server
        --ignore-return
        --task ${CMAKE_CURRENT_BINARY_DIR}/GreeterContactsTestExe
        --task-name test
        --wait-for org.freedesktop.Accounts
        -p -xunitxml -p -o -p ${CMAKE_BINARY_DIR}/test_GreeterContactsTest.xml
    DEPENDENCIES GreeterContactsTestServerExe GreeterContactsTestExe
    )

generate_test(ContactUtilsTest False)
generate_test(PhoneUtilsTest False)
generate_test(AccountEntryFactoryTest True)
generate_test(AccountEntryTest True)
generate_test(ChatManagerTest True)
generate_test(OfonoAccountEntryTest True)
