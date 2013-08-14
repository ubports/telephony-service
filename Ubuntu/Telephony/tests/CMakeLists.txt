include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Ubuntu/Telephony
    )

macro(generate_tests)
    foreach(test ${ARGN})
        add_executable(${test} ${test}.cpp)
        qt5_use_modules(${test} Contacts Core DBus Qml Test)
        target_link_libraries(${test}
            telephonyservice-qml
            )
        add_test(${test} ${CMAKE_CURRENT_BINARY_DIR}/${test} -xunitxml -o ${CMAKE_BINARY_DIR}/test_${test}.xml)
    endforeach(test)
endmacro(generate_tests)


generate_tests(
    ContactWatcherTest
    )

file(GLOB QML_TESTS *.qml)

# make the files visible on qtcreator
add_custom_target(telephonyservice_QMLTESTS ALL SOURCES ${QML_TESTS})

add_test(qmltests qmltestrunner -input ${CMAKE_CURRENT_SOURCE_DIR} -import ${CMAKE_BINARY_DIR} -xunitxml -o ${CMAKE_BINARY_DIR}/test_qmltests.xml)
