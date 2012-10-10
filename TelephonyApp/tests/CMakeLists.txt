include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/TelephonyApp
    )

macro(generate_tests)
    foreach(test ${ARGN})
        add_executable(${test} ${test}.cpp)
        qt5_use_modules(${test} Contacts Core DBus Gui Qml Test)
        target_link_libraries(${test}
            ${Qt5Contacts_LIBRARIES}
            ${Qt5Core_LIBRARIES}
            ${Qt5DBus_LIBRARIES}
            ${Qt5Gui_LIBRARIES}
            ${Qt5Qml_LIBRARIES}
            ${Qt5Test_LIBRARIES}
            ${TPL_QT5_LIBRARIES}
            telephony-qml
            )
        add_test(${test} ${CMAKE_CURRENT_BINARY_DIR}/${test})
    endforeach(test)
endmacro(generate_tests)

generate_tests(
    TemplateTest
    ContactModelTest
    ContactEntryTest
    ContactProxyModelTest
    ContactDetailsTest
#    ModelSectionCounterTest --> disabled due to bug #1046821
    )
