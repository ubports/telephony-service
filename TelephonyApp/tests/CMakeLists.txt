include_directories(
    ${QT_QTTEST_INCLUDE_DIR}
    )

macro(generate_tests)
    foreach(test ${ARGN})
        qt4_generate_moc(${test}.cpp ${test}.moc)
        add_executable(${test} ${test}.cpp ${test}.moc)
        target_link_libraries(${test}
            ${QT_QTCORE_LIBRARIES}
            ${QT_QTTEST_LIBRARIES}
            )
        add_test(${test} ${CMAKE_CURRENT_BINARY_DIR}/${test})
    endforeach(test)
endmacro(generate_tests)

generate_tests(
    TemplateTest
    )
