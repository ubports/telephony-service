include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Ubuntu/Telephony
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    )

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/Ubuntu/Telephony)
generate_test(ContactWatcherTest USE_UI
              SOURCES ContactWatcherTest.cpp
              LIBRARIES telephonyservice-qml
              QT5_MODULES Contacts Core DBus Qml Test)

generate_test(context_properties QML_TEST tst_contextProperties.qml)
generate_test(phonenumber_field QML_TEST tst_PhoneNumberField.qml)
generate_test(phonenumber_input QML_TEST tst_PhoneNumberInput.qml)
generate_test(phonenumber_utils QML_TEST tst_PhoneNumberPhoneUtils.qml)

# make the files visible on qtcreator
file(GLOB QML_TESTS *.qml *.js)
add_custom_target(telephonyservice_QMLTESTS ALL SOURCES ${QML_TESTS})
