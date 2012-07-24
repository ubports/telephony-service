include_directories(
    ${QT_QTTEST_INCLUDE_DIR}
    )

qt4_generate_moc(TemplateTest.cpp TemplateTest.moc)
add_executable(TemplateTest TemplateTest.cpp TemplateTest.moc)

target_link_libraries(TemplateTest
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTTEST_LIBRARIES}
    )

add_test(TemplateTest ${CMAKE_CURRENT_BINARY_DIR}/TemplateTest)
