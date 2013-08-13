include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    )

macro(generate_tests)
    foreach(test ${ARGN})
        add_executable(${test} ${test}.cpp)
        qt5_use_modules(${test} Contacts Core DBus Qml Test)
        target_link_libraries(${test}
            telephonyservice
            )
        add_test(${test} ${CMAKE_CURRENT_BINARY_DIR}/${test} -xunitxml -o ${CMAKE_BINARY_DIR}/test_${test}.xml)
    endforeach(test)
endmacro(generate_tests)

generate_tests(
    ContactUtilsTest
    PhoneUtilsTest
    )
