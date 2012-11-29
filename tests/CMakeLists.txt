set(AUTOPILOT_DIR autopilot/telephony_app)

install(DIRECTORY ${AUTOPILOT_DIR}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python2.7/dist-packages/
    )

