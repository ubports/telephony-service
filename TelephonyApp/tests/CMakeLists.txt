include_directories(
    ${QT_QTTEST_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

macro(generate_tests)
    foreach(test ${ARGN})
        qt4_generate_moc(${test}.cpp ${test}.moc)
        add_executable(${test} ${test}.cpp ${test}.moc)
        target_link_libraries(${test}
            ${QT_QTTEST_LIBRARIES}
            ${QT_QTCORE_LIBRARIES}
            ${QT_QTGUI_LIBRARIES}
            ${QT_QTDBUS_LIBRARIES}
            ${TPL_QT4_LIBRARIES}
            telephony-qml
            )
        add_test(${test} ${CMAKE_CURRENT_BINARY_DIR}/${test})
    endforeach(test)
endmacro(generate_tests)

generate_tests(
    TemplateTest
    ModelSectionCounterTest
    )
