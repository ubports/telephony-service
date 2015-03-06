add_subdirectory(common)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc*")
    message(STATUS "Handler tests disabled on ppc")
else()
    add_subdirectory(handler)
endif()

add_subdirectory(libtelephonyservice)
add_subdirectory(Ubuntu.Telephony)
