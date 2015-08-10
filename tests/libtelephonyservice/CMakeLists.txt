set(LIBTELEPHONYSERVICE_DIR ${CMAKE_SOURCE_DIR}/libtelephonyservice)
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${LIBTELEPHONYSERVICE_DIR}
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/tests/common
    ${CMAKE_BINARY_DIR}/tests/common
    ${GSETTINGS_QT_INCLUDE_DIRS}
    )

add_executable(GreeterContactsTestServerExe GreeterContactsTestServer.cpp)
qt5_use_modules(GreeterContactsTestServerExe Core DBus)

add_executable(ToneGeneratorMock ToneGeneratorMock.cpp)
qt5_use_modules(ToneGeneratorMock Core DBus)

add_executable(IndicatorMock IndicatorMock.cpp)
qt5_use_modules(IndicatorMock Core DBus)

generate_test(GreeterContactsTest USE_DBUS
              SOURCES GreeterContactsTest.cpp ${LIBTELEPHONYSERVICE_DIR}/greetercontacts.cpp
              QT5_MODULES Contacts Core DBus Test
              ENVIRONMENT XDG_SESSION_CLASS=greeter XDG_GREETER_DATA_DIR=${CMAKE_BINARY_DIR}/Testing/Temporary
              TASKS --task ${CMAKE_CURRENT_BINARY_DIR}/GreeterContactsTestServerExe --task-name server --ignore-return
              WAIT_FOR org.freedesktop.Accounts)
set_target_properties(GreeterContactsTest PROPERTIES COMPILE_DEFINITIONS "AS_BUSNAME=sessionBus;CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
add_dependencies(GreeterContactsTest GreeterContactsTestServerExe)

generate_test(ToneGeneratorTest USE_DBUS
              SOURCES ToneGeneratorTest.cpp ${LIBTELEPHONYSERVICE_DIR}/tonegenerator.cpp
              QT5_MODULES Core DBus Test
              TASKS --task ${CMAKE_CURRENT_BINARY_DIR}/ToneGeneratorMock --task-name tone-gen --ignore-return
              WAIT_FOR com.Nokia.Telephony.Tones)
add_dependencies(ToneGeneratorTest ToneGeneratorMock)

generate_test(CallNotificationTest USE_DBUS
              SOURCES CallNotificationTest.cpp ${LIBTELEPHONYSERVICE_DIR}/callnotification.cpp
              QT5_MODULES Core DBus Test
              TASKS --task ${CMAKE_CURRENT_BINARY_DIR}/IndicatorMock --task-name indicator --ignore-return
              WAIT_FOR com.canonical.TelephonyServiceIndicator)

generate_test(ContactUtilsTest SOURCES ContactUtilsTest.cpp QT5_MODULES Contacts Core Test LIBRARIES telephonyservice USE_UI)
generate_test(PhoneUtilsTest SOURCES PhoneUtilsTest.cpp LIBRARIES telephonyservice USE_UI)
generate_test(ProtocolTest
              SOURCES ProtocolTest.cpp ${LIBTELEPHONYSERVICE_DIR}/protocol.cpp
              ENVIRONMENT TELEPHONY_SERVICE_PROTOCOLS_DIR=${CMAKE_CURRENT_SOURCE_DIR}/testProtocols)
generate_test(ProtocolManagerTest
              SOURCES ProtocolManagerTest.cpp ${LIBTELEPHONYSERVICE_DIR}/protocol.cpp ${LIBTELEPHONYSERVICE_DIR}/protocolmanager.cpp
              QT5_MODULES Core Qml Test
              ENVIRONMENT TELEPHONY_SERVICE_PROTOCOLS_DIR=${CMAKE_CURRENT_SOURCE_DIR}/testProtocols)

generate_telepathy_test(AccountEntryFactoryTest SOURCES AccountEntryFactoryTest.cpp)
generate_telepathy_test(AccountEntryTest SOURCES AccountEntryTest.cpp)
generate_telepathy_test(CallEntryTest SOURCES CallEntryTest.cpp)
generate_telepathy_test(ChatManagerTest SOURCES ChatManagerTest.cpp)
generate_telepathy_test(ChatEntryTest SOURCES ChatEntryTest.cpp)
generate_telepathy_test(MultimediaAccountEntryTest SOURCES MultimediaAccountEntryTest.cpp)
generate_telepathy_test(OfonoAccountEntryTest SOURCES OfonoAccountEntryTest.cpp)
generate_telepathy_test(TelepathyHelperTest SOURCES TelepathyHelperTest.cpp)
generate_telepathy_test(USSDManagerTest SOURCES USSDManagerTest.cpp)
