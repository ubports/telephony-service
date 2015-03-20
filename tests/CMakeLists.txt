macro(generate_test TESTNAME USE_DBUS)
    execute_process(COMMAND mktemp -d OUTPUT_VARIABLE TMPDIR)
    string(REPLACE "\n" "" TMPDIR ${TMPDIR})
    add_executable(${TESTNAME} ${ARGN} ${TESTNAME}.cpp)
    qt5_use_modules(${TESTNAME} Core DBus Test Qml)
    MESSAGE(STATUS "Adding test: ${TESTNAME}")
    set(TEST_COMMAND )
    if (${USE_DBUS})
        set(TEST_COMMAND -p ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME} -p -xunitxml -p -o -p ${CMAKE_BINARY_DIR}/test_${TESTNAME}.xml)
        add_test(${TESTNAME} xvfb-run -a -s "-screen 0 1024x768x24"
                 ${DBUS_RUNNER} --keep-env --dbus-config=${CMAKE_BINARY_DIR}/tests/common/dbus-session.conf --max-wait=60
                                --task dbus-monitor --ignore-return
                                --task ${CMAKE_BINARY_DIR}/tests/common/dbus-test-wrapper.sh -p ${TMPDIR} ${TEST_COMMAND})
    else (${USE_DBUS})
        add_test(${TESTNAME} xvfb-run -a -s "-screen 0 1024x768x24" ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME} -xunitxml -o ${CMAKE_BINARY_DIR}/test_${TESTNAME}.xml)
    endif(${USE_DBUS})
    # force telepathy not to use system approvers when available,
    # also force usage of memory backend in history-service
    set(TEST_ENVIRONMENT "HOME=${TMPDIR};"
                         "HISTORY_SQLITE_DBPATH=:memory:;"
                         "XDG_CONFIG_HOME=${TMPDIR};
                          XDG_DATA_HOME=${TMPDIR};
                          XDG_CACHE_DIR=${TMPDIR};
                          XDG_CACHE_HOME=${TMPDIR};
                          XDG_DATA_DIRS=${TMPDIR};
                          MC_ACCOUNT_DIR=${TMPDIR};
                          MC_MANAGER_DIR=${TMPDIR};
                          TEST_DIR=${TMPDIR}")
    set_tests_properties(${TESTNAME} PROPERTIES
                          ENVIRONMENT "${TEST_ENVIRONMENT}"
                          TIMEOUT 60)
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
