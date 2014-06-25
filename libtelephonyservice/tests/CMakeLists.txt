include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    )

add_library(GreeterContactsLibSystem MODULE libsystem.c)
set_target_properties(GreeterContactsLibSystem PROPERTIES OUTPUT_NAME system)

add_executable(GreeterContactsTestServerExe GreeterContactsTestServer.cpp)
qt5_use_modules(GreeterContactsTestServerExe Core DBus)

add_executable(GreeterContactsTestExe GreeterContactsTest.cpp ../greetercontacts.cpp)
set_target_properties(GreeterContactsTestExe PROPERTIES COMPILE_DEFINITIONS "AS_BUSNAME=sessionBus;CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
qt5_use_modules(GreeterContactsTestExe Contacts Core DBus Test)

add_test(NAME GreeterContactsTest
    COMMAND env
        LD_PRELOAD=${CMAKE_CURRENT_BINARY_DIR}/libsystem.so
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
    DEPENDENCIES GreeterContactsTestServerExe GreeterContactsTestExe GreeterContactsLibSystem
    )

macro(generate_tests)
    foreach(test ${ARGN})
        add_executable(${test} ${test}.cpp)
        qt5_use_modules(${test} Contacts Core DBus Qml Test)
        target_link_libraries(${test}
            telephonyservice
            )
        add_test(${test} ${CMAKE_CURRENT_BINARY_DIR}/${test} -platform offscreen -xunitxml -o ${CMAKE_BINARY_DIR}/test_${test}.xml)
    endforeach(test)
endmacro(generate_tests)

generate_tests(
    ContactUtilsTest
    PhoneUtilsTest
    )
