set(LIBTELEPHONYSERVICE_DIR ${CMAKE_SOURCE_DIR}/libtelephonyservice)
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${LIBTELEPHONYSERVICE_DIR}
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/tests/common
    ${CMAKE_BINARY_DIR}/tests/common
    )

add_executable(GreeterContactsTestServerExe GreeterContactsTestServer.cpp)
qt5_use_modules(GreeterContactsTestServerExe Core DBus)

generate_test(GreeterContactsTest USE_DBUS
              SOURCES GreeterContactsTest.cpp ${LIBTELEPHONYSERVICE_DIR}/greetercontacts.cpp
              QT5_MODULES Contacts Core DBus Test
              ENVIRONMENT XDG_SESSION_CLASS=greeter XDG_GREETER_DATA_DIR=${CMAKE_BINARY_DIR}/Testing/Temporary
              TASKS --task env --ignore-return --task ${CMAKE_CURRENT_BINARY_DIR}/GreeterContactsTestServerExe --task-name server --ignore-return
              WAIT_FOR org.freedesktop.Accounts)
set_target_properties(GreeterContactsTest PROPERTIES COMPILE_DEFINITIONS "AS_BUSNAME=sessionBus;CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
add_dependencies(GreeterContactsTest GreeterContactsTestServerExe)

generate_test(ContactUtilsTest SOURCES ContactUtilsTest.cpp QT5_MODULES Contacts Core Test LIBRARIES telephonyservice USE_UI)
generate_test(PhoneUtilsTest SOURCES PhoneUtilsTest.cpp LIBRARIES telephonyservice USE_UI)

generate_telepathy_test(AccountEntryFactoryTest SOURCES AccountEntryFactoryTest.cpp)
generate_telepathy_test(AccountEntryTest SOURCES AccountEntryTest.cpp)
generate_telepathy_test(ChatManagerTest SOURCES ChatManagerTest.cpp)
generate_telepathy_test(OfonoAccountEntryTest SOURCES OfonoAccountEntryTest.cpp)
generate_telepathy_test(TelepathyHelperTest SOURCES TelepathyHelperTest.cpp)
generate_telepathy_test(USSDManagerTest SOURCES USSDManagerTest.cpp)
