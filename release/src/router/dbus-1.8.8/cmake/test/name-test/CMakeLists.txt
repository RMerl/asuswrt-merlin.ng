if (DBUS_ENABLE_EMBEDDED_TESTS)

set (NAMEtest-DIR ../../../test/name-test)

add_definitions(${DBUS_INTERNAL_CLIENT_DEFINITIONS})

add_helper_executable(test-pending-call-dispatch ${NAMEtest-DIR}/test-pending-call-dispatch.c ${DBUS_INTERNAL_LIBRARIES})
add_helper_executable(test-pending-call-timeout ${NAMEtest-DIR}/test-pending-call-timeout.c ${DBUS_INTERNAL_LIBRARIES})
add_helper_executable(test-thread-init ${NAMEtest-DIR}/test-threads-init.c ${DBUS_INTERNAL_LIBRARIES})
add_helper_executable(test-ids ${NAMEtest-DIR}/test-ids.c ${DBUS_INTERNAL_LIBRARIES})
add_helper_executable(test-shutdown ${NAMEtest-DIR}/test-shutdown.c dbus-testutils)
add_helper_executable(test-privserver ${NAMEtest-DIR}/test-privserver.c dbus-testutils)
add_helper_executable(test-privserver-client ${NAMEtest-DIR}/test-privserver-client.c dbus-testutils)
add_helper_executable(test-autolaunch ${NAMEtest-DIR}/test-autolaunch.c dbus-testutils)
endif (DBUS_ENABLE_EMBEDDED_TESTS)
