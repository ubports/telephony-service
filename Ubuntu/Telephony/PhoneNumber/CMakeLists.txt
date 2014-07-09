set(PHONENUMBER_SRCS
    phonenumber.h
    phonenumber.cpp
    asyoutypeformatter.h
    asyoutypeformatter.cpp
    )

set(PHONENUMBER_QMLS
    PhoneNumberField.qml
    PhoneNumberInput.qml
    PhoneNumber.js
    qmldir
)

include_directories(
    ${LibPhoneNumber_INCLUDE_DIRS}
    )

add_library(telephonyservice-phonenumber-qml SHARED ${PHONENUMBER_SRCS})

qt5_use_modules(telephonyservice-phonenumber-qml Core Qml Quick)

target_link_libraries(telephonyservice-phonenumber-qml
    ${LibPhoneNumber_LIBRARIES}
    )

set(PHONENUMBER_PLUGIN_DIR ${QT_INSTALL_QML}/Ubuntu/Telephony/PhoneNumber)
install(TARGETS telephonyservice-phonenumber-qml DESTINATION ${PHONENUMBER_PLUGIN_DIR})
install(FILES ${PHONENUMBER_QMLS} DESTINATION ${PHONENUMBER_PLUGIN_DIR})

# make the files visible on qtcreator
add_custom_target(phonenumber_QMlFiles ALL SOURCES ${PHONENUMBER_QMLS})


#copy qml files to build dir to make it possible to run without install
add_custom_target(copy_qml)
foreach(QML_FILE ${PHONENUMBER_QMLS})
  add_custom_command(TARGET copy_qml PRE_BUILD
                     COMMAND ${CMAKE_COMMAND} -E
                         copy ${CMAKE_CURRENT_SOURCE_DIR}/${QML_FILE} ${CMAKE_CURRENT_BINARY_DIR}/)
endforeach()

if (NOT ${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    add_dependencies(telephonyservice-phonenumber-qml copy_qml)
endif()
