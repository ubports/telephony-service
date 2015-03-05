include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Ubuntu/Telephony
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    )

macro(generate_tests)
    foreach(test ${ARGN})
        add_executable(${test} ${test}.cpp)
        qt5_use_modules(${test} Contacts Core DBus Qml Test)
        target_link_libraries(${test}
            telephonyservice-qml
            )
        add_test(${test} ${CMAKE_CURRENT_BINARY_DIR}/${test} -platform offscreen -xunitxml -o ${CMAKE_BINARY_DIR}/test_${test}.xml)
    endforeach(test)
endmacro(generate_tests)

macro(DECLARE_QML_TEST TST_NAME TST_QML_FILE)
    add_test(NAME ${TST_NAME}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND xvfb-run -a -s "-screen 0 1024x768x24" qmltestrunner -import ${CMAKE_BINARY_DIR} -input ${CMAKE_CURRENT_SOURCE_DIR}/${TST_QML_FILE}
    )
endmacro()


generate_tests(
    ContactWatcherTest
    )

declare_qml_test("context_properties" tst_contextProperties.qml)
declare_qml_test("phonenumber_field" tst_PhoneNumberField.qml)
declare_qml_test("phonenumber_input" tst_PhoneNumberInput.qml)
declare_qml_test("phonenumber_utils" tst_PhoneNumberPhoneUtils.qml)

# make the files visible on qtcreator
file(GLOB QML_TESTS *.qml *.js)
add_custom_target(telephonyservice_QMLTESTS ALL SOURCES ${QML_TESTS})
