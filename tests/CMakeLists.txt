macro(generate_test TESTNAME USE_DBUS)
    add_executable(${TESTNAME} ${ARGN} ${TESTNAME}.cpp)
    qt5_use_modules(${TESTNAME} Core DBus Test Qml)
    set(TEST_COMMAND )
    if (${USE_DBUS})
        set(TEST_COMMAND -p ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME} -p -xunitxml -p -o -p ${CMAKE_BINARY_DIR}/test_${TESTNAME}.xml)
        add_test(${TESTNAME} ${DBUS_RUNNER} --keep-env
                                            --task ${CMAKE_BINARY_DIR}/tests/common/dbus-test-wrapper.sh ${TEST_COMMAND})
    else (${USE_DBUS})
        add_test(${TESTNAME} ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME} -xunitxml -o ${CMAKE_BINARY_DIR}/test_${TESTNAME}.xml)
    endif(${USE_DBUS})
    # force telepathy not to use system approvers when available,
    # also force usage of memory backend in history-service
    set(TMPDIR "/tmp/telephony_service_test_home")
    set(TEST_ENVIRONMENT "HOME=${TMPDIR};"
                         "HISTORY_SQLITE_DBPATH=:memory:;"
                         "XDG_CONFIG_HOME=${TMPDIR};
                          XDG_DATA_HOME=${TMPDIR};
                          XDG_CACHE_DIR=${TMPDIR};
                          XDG_CACHE_HOME=${TMPDIR};
                          XDG_DATA_DIRS=${TMPDIR};
                          MC_ACCOUNT_DIR=${TMPDIR};
                          MC_MANAGER_DIR=${TMPDIR}")
    set_tests_properties(${TESTNAME} PROPERTIES
                          ENVIRONMENT "${TEST_ENVIRONMENT}"
                          TIMEOUT 30)
    target_link_libraries(${TESTNAME}
                          ${TP_QT5_LIBRARIES}
                          telephonyservice
                          mockcontroller
                         )

    # as the tests link to telephonyservice, we need to enable coverage on them too
    enable_coverage(${TESTNAME})
endmacro(generate_test)

add_subdirectory(common)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc*")
    message(STATUS "Handler tests disabled on ppc")
else()
    add_subdirectory(handler)
endif()

add_subdirectory(libtelephonyservice)
add_subdirectory(Ubuntu.Telephony)
