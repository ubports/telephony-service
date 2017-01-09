file(GLOB PROVIDER_FILES *.provider)
install(FILES ${PROVIDER_FILES} DESTINATION share/accounts/providers/)

file(GLOB SERVICE_FILES *.service)
install(FILES ${SERVICE_FILES} DESTINATION share/accounts/services/)
