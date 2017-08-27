SET(DBUS_DIR ${CMAKE_SOURCE_DIR}/../dbus)

configure_file(${DBUS_DIR}/dbus-arch-deps.h.in ${CMAKE_CURRENT_BINARY_DIR}/dbus-arch-deps.h )

add_definitions(-DDBUS_COMPILATION)

set (dbusinclude_HEADERS
	${DBUS_DIR}/dbus.h
	${DBUS_DIR}/dbus-address.h
	${DBUS_DIR}/dbus-bus.h
	${DBUS_DIR}/dbus-connection.h
	${DBUS_DIR}/dbus-errors.h
	${DBUS_DIR}/dbus-macros.h
	${DBUS_DIR}/dbus-memory.h
	${DBUS_DIR}/dbus-message.h
	${DBUS_DIR}/dbus-misc.h
	${DBUS_DIR}/dbus-pending-call.h
	${DBUS_DIR}/dbus-protocol.h
	${DBUS_DIR}/dbus-server.h
	${DBUS_DIR}/dbus-shared.h
	${DBUS_DIR}/dbus-signature.h
	${DBUS_DIR}/dbus-syntax.h
	${DBUS_DIR}/dbus-threads.h
	${DBUS_DIR}/dbus-types.h
	dbus-arch-deps.h
)

### source code that goes in the installed client library
### and is specific to library functionality
set (DBUS_LIB_SOURCES
	${DBUS_DIR}/dbus-address.c
	${DBUS_DIR}/dbus-auth.c
	${DBUS_DIR}/dbus-bus.c
	${DBUS_DIR}/dbus-connection.c
	${DBUS_DIR}/dbus-credentials.c
	${DBUS_DIR}/dbus-errors.c
	${DBUS_DIR}/dbus-keyring.c
	${DBUS_DIR}/dbus-marshal-header.c
	${DBUS_DIR}/dbus-marshal-byteswap.c
	${DBUS_DIR}/dbus-marshal-recursive.c
	${DBUS_DIR}/dbus-marshal-validate.c
	${DBUS_DIR}/dbus-message.c
	${DBUS_DIR}/dbus-misc.c
	${DBUS_DIR}/dbus-nonce.c
	${DBUS_DIR}/dbus-object-tree.c
	${DBUS_DIR}/dbus-pending-call.c
	${DBUS_DIR}/dbus-resources.c
	${DBUS_DIR}/dbus-server.c
	${DBUS_DIR}/dbus-server-socket.c
	${DBUS_DIR}/dbus-server-debug-pipe.c
	${DBUS_DIR}/dbus-sha.c
	${DBUS_DIR}/dbus-signature.c
	${DBUS_DIR}/dbus-syntax.c
	${DBUS_DIR}/dbus-timeout.c
	${DBUS_DIR}/dbus-threads.c
	${DBUS_DIR}/dbus-transport.c
	${DBUS_DIR}/dbus-transport-socket.c
	${DBUS_DIR}/dbus-watch.c
)


if(UNIX)
	set (DBUS_LIB_SOURCES ${DBUS_LIB_SOURCES} 
		${DBUS_DIR}/dbus-transport-unix.c
		${DBUS_DIR}/dbus-server-unix.c
	)
else(UNIX)
	set (DBUS_LIB_SOURCES ${DBUS_LIB_SOURCES} 
		${DBUS_DIR}/dbus-transport-win.c
		${DBUS_DIR}/dbus-server-win.c
	)
endif(UNIX)

set (DBUS_LIB_HEADERS
	${DBUS_DIR}/dbus-auth.h
	${DBUS_DIR}/dbus-connection-internal.h
	${DBUS_DIR}/dbus-credentials.h
	${DBUS_DIR}/dbus-keyring.h
	${DBUS_DIR}/dbus-marshal-header.h
	${DBUS_DIR}/dbus-marshal-byteswap.h
	${DBUS_DIR}/dbus-marshal-recursive.h
	${DBUS_DIR}/dbus-marshal-validate.h
	${DBUS_DIR}/dbus-message-internal.h
	${DBUS_DIR}/dbus-message-private.h
	${DBUS_DIR}/dbus-misc.h
	${DBUS_DIR}/dbus-object-tree.h
	${DBUS_DIR}/dbus-protocol.h
	${DBUS_DIR}/dbus-resources.h
	${DBUS_DIR}/dbus-server-debug-pipe.h
	${DBUS_DIR}/dbus-server-protected.h
	${DBUS_DIR}/dbus-server-unix.h
	${DBUS_DIR}/dbus-sha.h
	${DBUS_DIR}/dbus-timeout.h
	${DBUS_DIR}/dbus-threads.h
	${DBUS_DIR}/dbus-threads-internal.h
	${DBUS_DIR}/dbus-transport.h
	${DBUS_DIR}/dbus-transport-protected.h
	${DBUS_DIR}/dbus-watch.h
	${CMAKE_BINARY_DIR}/config.h
)
if(UNIX)
	set (DBUS_LIB_HEADERS ${DBUS_LIB_HEADERS} 
		${DBUS_DIR}/dbus-transport-unix.h
	)
else(UNIX)
	set (DBUS_LIB_HEADERS ${DBUS_LIB_HEADERS} 
		${DBUS_DIR}/dbus-transport-win.h
	)
endif(UNIX)


### source code that goes in the installed client library
### AND is generic utility functionality used by the 
### daemon or test programs (all symbols in here should 
### be underscore-prefixed)
set (DBUS_SHARED_SOURCES
	${DBUS_DIR}/dbus-dataslot.c
	${DBUS_DIR}/dbus-file.c
	${DBUS_DIR}/dbus-hash.c
	${DBUS_DIR}/dbus-internals.c
	${DBUS_DIR}/dbus-list.c
	${DBUS_DIR}/dbus-marshal-basic.c
	${DBUS_DIR}/dbus-memory.c
	${DBUS_DIR}/dbus-mempool.c
	${DBUS_DIR}/dbus-string.c
	${DBUS_DIR}/dbus-sysdeps.c
	${DBUS_DIR}/dbus-pipe.c
)

set (DBUS_SHARED_HEADERS
	${DBUS_DIR}/dbus-dataslot.h
	${DBUS_DIR}/dbus-file.h
	${DBUS_DIR}/dbus-hash.h
	${DBUS_DIR}/dbus-internals.h
	${DBUS_DIR}/dbus-list.h
	${DBUS_DIR}/dbus-marshal-basic.h
	${DBUS_DIR}/dbus-mempool.h
	${DBUS_DIR}/dbus-string.h
	${DBUS_DIR}/dbus-string-private.h
	${DBUS_DIR}/dbus-pipe.h
	${DBUS_DIR}/dbus-sysdeps.h
)

### source code that is generic utility functionality used
### by the bus daemon or test apps, but is NOT included
### in the D-BUS client library (all symbols in here 
### should be underscore-prefixed but don't really need 
### to be unless they move to DBUS_SHARED_SOURCES later)
set (DBUS_UTIL_SOURCES
	${DBUS_DIR}/dbus-asv-util.c
	${DBUS_DIR}/dbus-auth-script.c
	${DBUS_DIR}/dbus-auth-util.c
	${DBUS_DIR}/dbus-credentials-util.c
	${DBUS_DIR}/dbus-mainloop.c
	${DBUS_DIR}/dbus-marshal-byteswap-util.c
	${DBUS_DIR}/dbus-marshal-recursive-util.c
	${DBUS_DIR}/dbus-marshal-validate-util.c
	${DBUS_DIR}/dbus-message-factory.c
	${DBUS_DIR}/dbus-message-util.c
	${DBUS_DIR}/dbus-shell.c
	${DBUS_DIR}/dbus-socket-set.c
	${DBUS_DIR}/dbus-socket-set-poll.c
	${DBUS_DIR}/dbus-string-util.c
	${DBUS_DIR}/dbus-sysdeps-util.c
)

if (DBUS_ENABLE_EMBEDDED_TESTS)
	set (DBUS_UTIL_SOURCES 
		${DBUS_UTIL_SOURCES}
		${DBUS_DIR}/dbus-test.c
	)
endif (DBUS_ENABLE_EMBEDDED_TESTS)

set (DBUS_UTIL_HEADERS
	${DBUS_DIR}/dbus-asv-util.h
	${DBUS_DIR}/dbus-auth-script.h
	${DBUS_DIR}/dbus-mainloop.h
	${DBUS_DIR}/dbus-message-factory.h
	${DBUS_DIR}/dbus-shell.h
	${DBUS_DIR}/dbus-socket-set.h
	${DBUS_DIR}/dbus-spawn.h
	${DBUS_DIR}/dbus-test.h
)

### platform specific settings
if (WIN32)
	set (DBUS_SHARED_SOURCES ${DBUS_SHARED_SOURCES} 
		${DBUS_DIR}/dbus-file-win.c
		${DBUS_DIR}/dbus-init-win.cpp
		${DBUS_DIR}/dbus-sysdeps-win.c
		${DBUS_DIR}/dbus-pipe-win.c
		${DBUS_DIR}/dbus-sysdeps-thread-win.c
	)
	set (DBUS_SHARED_HEADERS ${DBUS_SHARED_HEADERS} 
		${DBUS_DIR}/dbus-sockets-win.h
		${DBUS_DIR}/dbus-sysdeps-win.h
	)
	set (DBUS_UTIL_SOURCES ${DBUS_UTIL_SOURCES}
		${DBUS_DIR}/dbus-spawn-win.c
		${DBUS_DIR}/dbus-sysdeps-util-win.c
	)
	if(WINCE)
	set (DBUS_SHARED_SOURCES ${DBUS_SHARED_SOURCES}
		${DBUS_DIR}/dbus-sysdeps-wince-glue.c
	)
	set (DBUS_SHARED_HEADERS ${DBUS_SHARED_HEADERS}
		${DBUS_DIR}/dbus-sysdeps-wince-glue.h
	)
	endif(WINCE)
else (WIN32)
	set (DBUS_SHARED_SOURCES ${DBUS_SHARED_SOURCES} 
		${DBUS_DIR}/dbus-file-unix.c
		${DBUS_DIR}/dbus-pipe-unix.c
		${DBUS_DIR}/dbus-sysdeps-unix.c
		${DBUS_DIR}/dbus-sysdeps-pthread.c
		${DBUS_DIR}/dbus-userdb.c
        ${DBUS_DIR}/sd-daemon.c
	)
	set (DBUS_SHARED_HEADERS ${DBUS_SHARED_HEADERS} 
		${DBUS_DIR}/dbus-server-unix.h
		${DBUS_DIR}/dbus-transport-unix.h
		${DBUS_DIR}/dbus-sysdeps-unix.h
		${DBUS_DIR}/dbus-userdb.h
        ${DBUS_DIR}/sd-daemon.h
	)
	set (DBUS_UTIL_SOURCES ${DBUS_UTIL_SOURCES}
		${DBUS_DIR}/dbus-spawn.c
		${DBUS_DIR}/dbus-userdb-util.c
		${DBUS_DIR}/dbus-sysdeps-util-unix.c
	)
endif (WIN32)

set(libdbus_SOURCES
	${DBUS_LIB_SOURCES}
	${DBUS_SHARED_SOURCES}
)

set(libdbus_HEADERS 
	${DBUS_LIB_HEADERS}
	${DBUS_SHARED_HEADERS}
)
if (MSVC)
    set (BUILD_FILEVERSION ${DBUS_MAJOR_VERSION},${DBUS_MINOR_VERSION},${DBUS_MICRO_VERSION},${DBUS_PATCH_VERSION})
    set (BUILD_TIMESTAMP ${DBUS_BUILD_TIMESTAMP})
    
    configure_file(${DBUS_DIR}/versioninfo.rc.in ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/afxres.h "")
    list(APPEND libdbus_SOURCES versioninfo.rc)
    set_source_files_properties(versioninfo.rc COMPILE_FLAGS "-D__LINE__=1")
endif (MSVC)

if(MSVC_IDE)
	project_source_group(${GROUP_CODE} DBUS_LIB_SOURCES DBUS_LIB_HEADERS)
	project_source_group(${GROUP_CODE} DBUS_SHARED_SOURCES DBUS_SHARED_HEADERS)
	project_source_group(${GROUP_CODE} DBUS_UTIL_SOURCES DBUS_UTIL_SOURCES)
endif(MSVC_IDE)

### Client library

add_library(dbus-1 SHARED
			${libdbus_SOURCES}
			${libdbus_HEADERS}
)
if(WIN32)
    if(WINCE)
        target_link_libraries(dbus-1 ws2)
    else(WINCE)
        target_link_libraries(dbus-1 ws2_32 advapi32 netapi32 iphlpapi)
    endif(WINCE)
else(WIN32)
    target_link_libraries(dbus-1 ${CMAKE_THREAD_LIBS_INIT} rt)
endif(WIN32)

install(TARGETS dbus-1 ${INSTALL_TARGETS_DEFAULT_ARGS})
install_files(/include/dbus FILES ${dbusinclude_HEADERS})

### Internal library, used for the daemon, tools and tests, compiled statically.

add_library(dbus-internal ${DBUS_INTERNAL_ADD_LIBRARY_OPTIONS}
			${DBUS_LIB_SOURCES}
			${DBUS_LIB_HEADERS}
			${DBUS_SHARED_SOURCES}
			${DBUS_SHARED_HEADERS}
			${DBUS_UTIL_SOURCES}
			${DBUS_UTIL_HEADERS}
)
target_link_libraries(dbus-internal)
set_target_properties(dbus-internal PROPERTIES COMPILE_FLAGS ${DBUS_INTERNAL_LIBRARY_DEFINITIONS})
if(WIN32)
    if(WINCE)
        target_link_libraries(dbus-internal ws2)
    else(WINCE)
        target_link_libraries(dbus-internal ws2_32 advapi32 netapi32 iphlpapi)
    endif(WINCE)
else(WIN32)
    target_link_libraries(dbus-internal ${CMAKE_THREAD_LIBS_INIT} rt)
endif(WIN32)

if (DBUS_ENABLE_EMBEDDED_TESTS)
	add_test_executable(test-dbus ${CMAKE_SOURCE_DIR}/../dbus/dbus-test-main.c ${DBUS_INTERNAL_LIBRARIES})
	set_target_properties(test-dbus PROPERTIES COMPILE_FLAGS ${DBUS_INTERNAL_CLIENT_DEFINITIONS})
ENDIF (DBUS_ENABLE_EMBEDDED_TESTS)

if (UNIX)
# set version info
ENDIF (UNIX)


## mop up the gcov files
#clean-local:
#/bin/rm *.bb *.bbg *.da *.gcov .libs/*.da .libs/*.bbg || true
