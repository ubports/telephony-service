# copy the services we want to use
set(DBUS_SERVICES_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/dbus-1/services)

file(COPY ${DBUS_SERVICES_DIR}/org.freedesktop.Telepathy.MissionControl5.service
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${DBUS_SERVICES_DIR}/org.freedesktop.Telepathy.AccountManager.service
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${DBUS_SERVICES_DIR}/ca.desrt.dconf.service
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
