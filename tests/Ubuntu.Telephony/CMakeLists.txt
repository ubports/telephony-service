include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/Ubuntu/Telephony
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    ${CMAKE_SOURCE_DIR}/tests/common
    ${CMAKE_BINARY_DIR}/tests/common
    ${GSETTINGS_QT_INCLUDE_DIRS}
    )

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/Ubuntu/Telephony)
generate_test(ContactWatcherTest USE_UI
              SOURCES ContactWatcherTest.cpp
              LIBRARIES telephonyservice-qml Qt5::Contacts Qt5::Core Qt5::DBus Qt5::Qml Qt5::Test)

if(NOT SKIP_QML_TESTS)
    generate_test(context_properties QML_TEST tst_contextProperties.qml)
    generate_test(phonenumber_field QML_TEST tst_PhoneNumberField.qml)
    generate_test(phonenumber_input QML_TEST tst_PhoneNumberInput.qml)
    generate_test(phonenumber_utils QML_TEST tst_PhoneNumberPhoneUtils.qml)
endif()

generate_telepathy_test(PresenceRequestTest 
                        SOURCES PresenceRequestTest.cpp 
                        LIBRARIES ${TP_QT5_LIBRARIES} telephonyservice mockcontroller telepathytest telephonyservice-qml)

# make the files visible on qtcreator
file(GLOB QML_TESTS *.qml *.js)
add_custom_target(telephonyservice_QMLTESTS ALL SOURCES ${QML_TESTS})
