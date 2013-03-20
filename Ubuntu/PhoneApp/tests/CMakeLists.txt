include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Ubuntu/PhoneApp
    )

macro(generate_tests)
    foreach(test ${ARGN})
        add_executable(${test} ${test}.cpp)
        qt5_use_modules(${test} Contacts Core DBus Qml Test)
        target_link_libraries(${test}
            ${TPL_QT5_LIBRARIES}
            phone-qml
            )
        add_test(${test} ${CMAKE_CURRENT_BINARY_DIR}/${test})
    endforeach(test)
endmacro(generate_tests)

generate_tests(
    TemplateTest
    CallLogModelTest
    ContactProxyModelTest
    ConversationAggregatorModelTest
    ConversationFeedItemTest
    ConversationFeedModelTest
    MessageLogModelTest
    )
