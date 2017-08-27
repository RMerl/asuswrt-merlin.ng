# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

# we do not need to have WIN32 defined
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

project(dbus)

# we need to be up to date
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.4 FATAL_ERROR)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


# detect version
include(MacrosAutotools)
autoversion(../configure.ac dbus)
# used by file version info
set (DBUS_PATCH_VERSION "0")

# set PACKAGE_... variables
autopackage(
    dbus
    ${DBUS_VERSION_STRING}
    "http://dbus.freedesktop.org"
    "https://bugs.freedesktop.org/enter_bug.cgi?product=dbus"
)

include(Macros)
TIMESTAMP(DBUS_BUILD_TIMESTAMP)

########### basic vars ###############


if (DBUSDIR)
	set(DBUS_INSTALL_DIR "${DBUSDIR}")
endif (DBUSDIR)
if ($ENV{DBUSDIR})
	set(DBUS_INSTALL_DIR "$ENV{DBUSDIR}")
endif ($ENV{DBUSDIR})

if (DBUS_INSTALL_DIR)
	set(CMAKE_INSTALL_PREFIX "${DBUS_INSTALL_DIR}" CACHE PATH "install prefix" FORCE)
else (DBUS_INSTALL_DIR)
	set(DBUS_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
endif (DBUS_INSTALL_DIR)

# autotools style
if (NOT DATAROOTDIR)
    set (DATAROOTDIR share)
endif()

if (NOT DATADIR)
    set (DATADIR ${DATAROOTDIR})
endif()

if (NOT DOCDIR)
    SET(DOCDIR ${DATAROOTDIR}/doc/dbus)
endif()

if (NOT DBUS_DATADIR)
    SET(DBUS_DATADIR ${DATADIR})
endif()

set(prefix                   ${DBUS_INSTALL_DIR})
set(exec_prefix              ${prefix})
set(EXPANDED_LIBDIR          ${DBUS_INSTALL_DIR}/lib)
set(EXPANDED_INCLUDEDIR      ${DBUS_INSTALL_DIR}/include)
set(EXPANDED_BINDIR          ${DBUS_INSTALL_DIR}/bin)
set(EXPANDED_SYSCONFDIR      ${DBUS_INSTALL_DIR}/etc)
set(EXPANDED_DATADIR         ${DBUS_INSTALL_DIR}/${DBUS_DATADIR})
set(DBUS_MACHINE_UUID_FILE   ${DBUS_INSTALL_DIR}/lib/dbus/machine-id)
set(DBUS_BINDIR              ${EXPANDED_BINDIR})
set(DBUS_DAEMONDIR           ${EXPANDED_BINDIR})


#enable building of shared library
SET(BUILD_SHARED_LIBS ON)

if(WIN32)
    set(INSTALL_TARGETS_DEFAULT_ARGS  RUNTIME DESTINATION "bin" LIBRARY DESTINATION "lib" ARCHIVE DESTINATION "lib")
else()
    set(INSTALL_TARGETS_DEFAULT_ARGS  RUNTIME DESTINATION "${EXPANDED_LIBDIR}" LIBRARY DESTINATION "${EXPANDED_LIBDIR}" ARCHIVE DESTINATION "${EXPANDED_LIBDIR}")
endif()

if (CYGWIN)
   set (WIN32)
endif (CYGWIN)

# search for required packages
if (WIN32)
    # include local header first to avoid using old installed header
    set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/..)
	find_package(LibIconv)
    include(Win32Macros)
    addExplorerWrapper(${CMAKE_PROJECT_NAME})
endif (WIN32)

if(NOT WIN32)
	option (DBUS_ENABLE_ABSTRACT_SOCKETS "enable support for abstract sockets" ON)
	set (CMAKE_THREAD_PREFER_PTHREAD ON)
	include (FindThreads)
endif(NOT WIN32)

option (DBUS_DISABLE_ASSERT "Disable assertion checking" OFF)

option (DBUS_ENABLE_STATS "enable bus daemon usage statistics" OFF)

if(WIN32)
    set(FD_SETSIZE "8192" CACHE STRING "The maximum number of connections that can be handled at once")
endif()

find_package(EXPAT)
find_package(X11)
find_package(GLib2)
find_package(GObject)
if(GLIB2_FOUND AND GOBJECT_FOUND)
    option (DBUS_WITH_GLIB "build with glib" ON)
endif()

# analogous to AC_USE_SYSTEM_EXTENSIONS in configure.ac
add_definitions(-D_GNU_SOURCE)

# do config checks
INCLUDE(ConfigureChecks.cmake)

# @TODO: how to remove last dir from ${CMAKE_SOURCE_DIR} ?
SET(DBUS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/..)

# make some more macros available
include (MacroLibrary)

if(VCS)
	set(DBUS_VERBOSE_C_S 1 CACHE STRING "verbose mode" FORCE)
endif(VCS)

if(WIN32)
	set(CMAKE_DEBUG_POSTFIX "d")
	if(MSVC)
		# controll folders in msvc projects
		include(ProjectSourceGroup)
		if(NOT GROUP_CODE)
			#set(GROUP_CODE split) #cmake default
			set(GROUP_CODE flat)
		endif(NOT GROUP_CODE)
		ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)


		# Use the highest warning level
		if (WALL)
			set(WALL 1 CACHE STRING "all warnings"  FORCE)
			set(CMAKE_CXX_WARNING_LEVEL 4 CACHE STRING "warning level" FORCE)

			if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
				STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
			else(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
				SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
			endif(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")

			if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
				STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
			else(CMAKE_C_FLAGS MATCHES "/W[0-4]")
				SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
			endif(CMAKE_C_FLAGS MATCHES "/W[0-4]")
		else (WALL)
			set(CMAKE_CXX_WARNING_LEVEL 3 CACHE STRING "warning level" FORCE)
		endif (WALL)

		SET(MSVC_W_ERROR   " /we4028 /we4013 /we4133 /we4047 /we4031 /we4002 /we4003 /we4114")
		SET(MSVC_W_DISABLE " /wd4127 /wd4090 /wd4101 /wd4244")

		SET(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   /FIconfig.h ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /FIconfig.h ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
	endif(MSVC)
endif(WIN32)

if (UNIX AND NOT DBUS_DISABLE_ASSERT)
	# required for backtrace
	SET(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   -Wl,--export-dynamic")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wl,--export-dynamic")
	add_definitions(-DDBUS_BUILT_R_DYNAMIC)
endif (UNIX AND NOT DBUS_DISABLE_ASSERT)

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}  -D_DEBUG")

#########################################################################
# Windows CE (>= 5.0.0)
#
# WinCE support now relies on the presence of platform files, found in cmake/modules/platform
# Cmake 2.8.0 doesn't include WinCE platform files by default, but working ones can be found
# on CMake's bugtracker :
# http://public.kitware.com/Bug/view.php?id=7919
#
# for cmake 2.8.0 get the following patch only :
# http://public.kitware.com/Bug/file_download.php?file_id=2944&type=bug
#
# after applying the patch, you can enable the WinCE build by specifying :
# cmake [...] -DCMAKE_SYSTEM_NAME=WinCE -DCMAKE_SYSTEM_VERSION=X.XX
# (where X.XX is your actual WinCE version, e.g. 5.02 for Windows Mobile 6)
#
# Note that you should have a proper cross-compilation environment set up prior to running
# cmake, ie. the PATH, INCLUDE and LIB env vars pointing to your CE SDK/toolchain.
#
if(WINCE)

MESSAGE("Building for WinCE (${CMAKE_SYSTEM_VERSION})")

endif(WINCE)
#########################################################################


ENABLE_TESTING()

#########################################################################
# Disallow in-source build
#macro_ensure_out_of_source_build("dbus requires an out of source build. Please create a separate build directory and run 'cmake path_to_dbus [options]' there.")

# ... and warn in case of an earlier in-source build
#set(generatedFileInSourceDir EXISTS ${dbus_SOURCE_DIR}/config.h)
#if(${generatedFileInSourceDir})
#   message(STATUS "config.h exists in your source directory.")
#endif(${generatedFileInSourceDir})
#########################################################################

if (WIN32 OR CYGWIN)
	set (LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/bin)
else (WIN32 OR CYGWIN)
	set (LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
endif (WIN32 OR CYGWIN)

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# for including config.h and for includes like <dir/foo.h>
include_directories( ${CMAKE_SOURCE_DIR}/.. ${CMAKE_BINARY_DIR} ${CMAKE_INCLUDE_PATH} )

# linker search directories
link_directories(${DBUS_LIB_DIR} ${LIBRARY_OUTPUT_PATH} )
include_directories( ${CMAKE_LIBRARY_PATH}  )

set(DBUS_INCLUDES)

ENABLE_TESTING()

########### command line options ###############
# TODO: take check from configure.in

option (DBUS_BUILD_TESTS "enable unit test code" ON)
 
if(DBUS_BUILD_TESTS)
    set (DBUS_ENABLE_EMBEDDED_TESTS ON)
    set (DBUS_ENABLE_MODULAR_TESTS ON)
    add_definitions(-DDBUS_ENABLE_EMBEDDED_TESTS -DDBUS_ENABLE_MODULAR_TESTS)
endif(DBUS_BUILD_TESTS)

option (DBUS_USE_OUTPUT_DEBUG_STRING "enable win32 debug port for message output" OFF)
if(DBUS_USE_OUTPUT_DEBUG_STRING)
    add_definitions(-DDBUS_USE_OUTPUT_DEBUG_STRING)
endif(DBUS_USE_OUTPUT_DEBUG_STRING)

if(WIN32)
	# win32 dbus service support - this support is not complete
	option (DBUS_SERVICE "enable dbus service installer" OFF)
endif(WIN32)

option (DBUS_ENABLE_ANSI "enable -ansi -pedantic gcc flags" OFF)
if(DBUS_ENABLE_ANSI)
   if(NOT MSVC)
        add_definitions(-ansi -D_POSIX_C_SOURCE=199309L -D_BSD_SOURCE -pedantic)
   else(NOT MSVC)
        add_definitions(-Za -D_POSIX_C_SOURCE=199309L -D_BSD_SOURCE -W4)
   endif(NOT MSVC)
endif(DBUS_ENABLE_ANSI)

option (DBUS_ENABLE_VERBOSE_MODE "support verbose debug mode" ON)

option (DBUS_DISABLE_CHECKS "Disable public API sanity checking" OFF)

if(NOT MSVC)
    option (DBUS_GCOV_ENABLED "compile with coverage profiling instrumentation (gcc only)" OFF)
    if(DBUS_GCOV_ENABLED)
            add_definitions(-fprofile-arcs -ftest-coverage)
            # FIXME!!!!
            ## remove optimization
    #        CFLAGS=`echo "$CFLAGS" | sed -e 's/-O[0-9]*//g'`
    endif(DBUS_GCOV_ENABLED)
endif(NOT MSVC)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    option (DBUS_BUS_ENABLE_INOTIFY "build with inotify support (linux only)" ON)
    if(DBUS_BUS_ENABLE_INOTIFY)
        check_include_file(sys/inotify.h HAVE_SYS_INOTIFY_H)
        if(NOT HAVE_SYS_INOTIFY_H)
            message(FATAL_ERROR "sys/inotify.h not found!")
        endif(NOT HAVE_SYS_INOTIFY_H)
    endif(DBUS_BUS_ENABLE_INOTIFY)
elseif("${CMAKE_SYSTEM_NAME}" MATCHES ".*BSD")
    option (DBUS_BUS_ENABLE_KQUEUE "build with kqueue support (FreeBSD only)" ON)
    if(DBUS_BUS_ENABLE_KQUEUE)
        # cmake check a header by compiling a test program with
        # the header, sys/event.h needs stdint.h and sys/types.h
        # to work.
        check_include_files("stdint.h;sys/types.h;sys/event.h" HAVE_SYS_EVENT_H)
        if(NOT HAVE_SYS_EVENT_H)
            message(FATAL_ERROR "sys/event.h not found!")
        endif(NOT HAVE_SYS_EVENT_H)
    endif(DBUS_BUS_ENABLE_KQUEUE)
endif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")

STRING(TOUPPER ${CMAKE_SYSTEM_NAME} sysname)
if("${sysname}" MATCHES ".*SOLARIS.*")
    option (HAVE_CONSOLE_OWNER_FILE "enable console owner file (solaris only)" ON)
    if(HAVE_CONSOLE_OWNER_FILE)
        set (DBUS_CONSOLE_OWNER_FILE "/dev/console" CACHE STRING "Directory to check for console ownerhip")
    endif(HAVE_CONSOLE_OWNER_FILE)
endif("${sysname}" MATCHES ".*SOLARIS.*")

if(NOT EXPAT_FOUND)
    message(FATAL_ERROR "expat not found!")
endif(NOT EXPAT_FOUND)

SET(XML_LIB "Expat")
SET(XML_LIBRARY     ${EXPAT_LIBRARIES})
SET(XML_INCLUDE_DIR ${EXPAT_INCLUDE_DIR})

# all missing or hardcoded for now

# 'hidden' ones
set(atomic_int OFF)
set(atomic_int486 OFF)
if(CMAKE_COMPILER_IS_GNUCC AND NOT DBUS_ENABLE_ANSI)
    FIND_PROGRAM(UNAME_EXECUTABLE
                    NAMES uname
                    PATHS /bin /usr/bin /usr/local/bin c:/Programme/MSys/bin d:/Programme/MSys/bin)

    if(UNAME_EXECUTABLE)
        EXECUTE_PROCESS(COMMAND ${UNAME_EXECUTABLE} "-m"
                        OUTPUT_VARIABLE UNAME_OUTPUT)

        if("UNAME_OUTPUT" MATCHES "^.*i[0123]86.*$")
            set(atomic_int ON)
        else("UNAME_OUTPUT" MATCHES "^.*i[0123]86.*$")
            if("UNAME_OUTPUT" MATCHES "^.*i?86.*$")
                set(atomic_int ON)
                set(atomic_int_486 ON)
            endif("UNAME_OUTPUT" MATCHES "^.*i?86.*$")
        endif("UNAME_OUTPUT" MATCHES "^.*i[0123]86.*$")
    endif(UNAME_EXECUTABLE)
endif(CMAKE_COMPILER_IS_GNUCC AND NOT DBUS_ENABLE_ANSI)

set (DBUS_HAVE_ATOMIC_INT ${atomic_int} CACHE STRING "Some atomic integer implementation present")
set (DBUS_USE_ATOMIC_INT_486 ${atomic_int_486} CACHE STRING "Use atomic integer implementation for 486")

if(X11_FOUND)
  option (DBUS_BUILD_X11 "Build with X11 autolaunch support " ON)
endif(X11_FOUND)

# test binary names
if (WIN32)
    # follow Automake's naming convention so we can share .in files
    set (EXEEXT ${CMAKE_EXECUTABLE_SUFFIX})
endif(WIN32)

if (MSVC_IDE)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
		set(IDE_BIN /Debug )
		message(STATUS)
		message(STATUS "Visual Studio: test programs will only work with 'Debug' configuration!")
		message(STATUS "To run tests with 'Release' configuration use -DCMAKE_BUILD_TYPE=Release")
		message(STATUS "Add '..\\..\\test\\data' to the command line option of the test programs")
		message(STATUS)
    else(CMAKE_BUILD_TYPE MATCHES Debug)
		set(IDE_BIN /Release)
		message(STATUS)
		message(STATUS "Visual Studio: test programs will only work with 'Release' configuration!")
		message(STATUS "To run tests with 'Debug' configuration use -DCMAKE_BUILD_TYPE=Debug")
		message(STATUS "Add '..\\..\\test\\data' to the command line option of the test programs")
		message(STATUS)
    endif(CMAKE_BUILD_TYPE MATCHES Debug)
	set (TEST_PATH_FORCE FORCE)
	FILE(REMOVE ${CMAKE_BINARY_DIR}/data/dbus-1/services)
endif (MSVC_IDE)

#### Find socket directories
 if (NOT $ENV{TMPDIR} STREQUAL "")
     set (DBUS_SESSION_SOCKET_DIR $ENV{TMPDIR})
 else (NOT $ENV{TMPDIR} STREQUAL "")
     if (NOT $ENV{TEMP} STREQUAL "")
         set (DBUS_SESSION_SOCKET_DIR $ENV{TEMP})
     else (NOT $ENV{TEMP} STREQUAL "")
         if (NOT $ENV{TMP} STREQUAL "")
             set (DBUS_SESSION_SOCKET_DIR $ENV{TMP})
         else (NOT $ENV{TMP} STREQUAL "")
         if (WIN32)
             #Should never happen, both TMP and TEMP seem always set on Windows
             message(FATAL_ERROR "Could not determine a usable temporary directory")
         else(WIN32)
            set (DBUS_SESSION_SOCKET_DIR /tmp)
         endif(WIN32)
         endif (NOT $ENV{TMP} STREQUAL "")
     endif (NOT $ENV{TEMP} STREQUAL "")
 endif (NOT $ENV{TMPDIR} STREQUAL "")

# TODO: fix redhet
if (WIN32)
  # bus-test expects a non empty string
	set (DBUS_SYSTEM_PID_FILE "/dbus-pid")
else (WIN32)
	set (DBUS_SYSTEM_PID_FILE ${EXPANDED_LOCALSTATEDIR}/run/dbus/pid)
endif (WIN32)

if (WIN32)
	set (DBUS_CONSOLE_AUTH_DIR "")
else (WIN32)
	set (DBUS_CONSOLE_AUTH_DIR "/var/run/console/")
endif (WIN32)

set (DBUS_USER )

# This won't work on Windows. It's not meant to - the system bus is
# meaningless on Windows anyway.
#
# This has to be suitable for hard-coding in client libraries as well as
# in the dbus-daemon's configuration, so it has to be valid to listen on
# and also to connect to. If this ever changes, it'll need to be split into
# two variables, one for the listening address and one for the connecting
# address.
set (DBUS_SYSTEM_BUS_DEFAULT_ADDRESS "unix:path=${EXPANDED_LOCALSTATEDIR}/run/dbus/system_bus_socket" CACHE STRING "system bus default address")

if (WIN32)
  set (DBUS_SESSION_BUS_LISTEN_ADDRESS "autolaunch:" CACHE STRING "session bus default listening address")
  set (DBUS_SESSION_BUS_CONNECT_ADDRESS "autolaunch:" CACHE STRING "session bus fallback address for clients")

  set (DBUS_SYSTEM_CONFIG_FILE "etc/dbus-1/system.conf")
  set (DBUS_SESSION_CONFIG_FILE "etc/dbus-1/session.conf")
  # bus-test expects a non empty string
  set (DBUS_USER "Administrator")
else (WIN32)
  set (DBUS_SESSION_BUS_LISTEN_ADDRESS "unix:tmpdir=${DBUS_SESSION_SOCKET_DIR}" CACHE STRING "session bus default listening address")
  set (DBUS_SESSION_BUS_CONNECT_ADDRESS "autolaunch:" CACHE STRING "session bus fallback address for clients")
  set (sysconfdir "")
  set (configdir ${sysconfdir}/dbus-1 )
  set (DBUS_SYSTEM_CONFIG_FILE  ${configdir}/system.conf)
  set (DBUS_SESSION_CONFIG_FILE ${configdir}/session.conf)
  set (DBUS_USER "root")
endif (WIN32)

set (DBUS_DAEMON_NAME "dbus-daemon" CACHE STRING "The name of the dbus daemon executable")

########### create config.h ###############

#include(ConfigureChecks.cmake)

# better use flags for gcc
if (MINGW)
	set (HAVE_GNUC_VARARGS 1)
endif(MINGW)

# compiler definitions
add_definitions(-DHAVE_CONFIG_H=1)
add_definitions(${DBUS_BUS_CFLAGS})


if (DBUS_BUILD_TESTS)
    # set variables used for the .in files (substituted by configure_file) in test/data:
    set(DBUS_TEST_EXEC ${EXECUTABLE_OUTPUT_PATH}${IDE_BIN})
    set(DBUS_TEST_DATA ${CMAKE_BINARY_DIR}/test/data)
    set(TEST_SOCKET_DIR ${DBUS_SESSION_SOCKET_DIR} )
    set(TEST_LAUNCH_HELPER_BINARY ${EXECUTABLE_OUTPUT_PATH}/dbus-daemon-launch-helper-test)
    set(TEST_BUS_LAUNCH_BINARY ${EXECUTABLE_OUTPUT_PATH}/dbus-launch${EXEEXT})
    if (UNIX)
        set (TEST_LISTEN "unix:tmpdir=${TEST_SOCKET_DIR}")
    endif (UNIX)
    if (WIN32)
        set (TEST_LISTEN "tcp:host=localhost")
    endif (WIN32)
endif  (DBUS_BUILD_TESTS)

set(DBUS_LIBRARIES dbus-1)
set(DBUS_INTERNAL_LIBRARIES dbus-internal)

# settings for building and using static internal lib
# important note: DBUS_INTERNAL_xxxxx_DEFINITIONS must *not* be set when building dbus-1 library
set (DBUS_INTERNAL_ADD_LIBRARY_OPTIONS STATIC)
set (DBUS_INTERNAL_LIBRARY_DEFINITIONS "-DDBUS_STATIC_BUILD")
# For now, the CMake build system doesn't support replacing the internal
# main loop with dbus-glib
set (DBUS_INTERNAL_CLIENT_DEFINITIONS "-DDBUS_STATIC_BUILD -DDBUS_COMPILATION -DDBUS_TEST_USE_INTERNAL")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

if (WIN32)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dbus-env.bat.cmake ${CMAKE_BINARY_DIR}/bin/dbus-env.bat )
install_files(/bin FILES ${CMAKE_BINARY_DIR}/bin/dbus-env.bat)
endif()

add_definitions(-DHAVE_CONFIG_H=1)

########### subdirs ###############

add_subdirectory( dbus )
add_subdirectory( bus )
if (DBUS_BUILD_TESTS)
	add_subdirectory( test )
endif (DBUS_BUILD_TESTS)
add_subdirectory( tools )
add_subdirectory( doc )


OPTION(DBUS_INSTALL_SYSTEM_LIBS "install required system libraries" OFF)
MESSAGE(" ")
MESSAGE("set -DDBUS_INSTALL_SYSTEM_LIBS=1 to install runtime libraries too")
MESSAGE("set DBUSDIR (environment or cmake option) to overwrite the default install directory ")
MESSAGE(" ")
MESSAGE(" ")
GET_FILENAME_COMPONENT(C_COMPILER ${CMAKE_C_COMPILER} NAME)
GET_FILENAME_COMPONENT(CXX_COMPILER ${CMAKE_CXX_COMPILER} NAME)

message("                  D-BUS ${DBUS_VERSION}                               ")
message("                  ===========                                         ")
message("                                                                      ")
message("        install prefix:           ${prefix}                           ")
message("        install exec_prefix:      ${exec_prefix}                      ")
message("        install libdir:           ${EXPANDED_LIBDIR}                  ")
message("        install bindir:           ${EXPANDED_BINDIR}                  ")
message("        install sysconfdir:       ${EXPANDED_SYSCONFDIR}              ")
message("        install datadir:          ${EXPANDED_DATADIR}                 ")
message("        source code location:     ${DBUS_SOURCE_DIR}                  ")
message("        build dir:                ${CMAKE_BINARY_DIR}                 ")
message("        c compiler:               ${C_COMPILER}                       ")
message("        cflags:                   ${CMAKE_C_FLAGS}                    ")
message("        cflags debug:             ${CMAKE_C_FLAGS_DEBUG}              ")
message("        cflags release:           ${CMAKE_C_FLAGS_RELEASE}            ")
message("        cxx compiler:             ${CXX_COMPILER}                     ")
message("        cxxflags:                 ${CMAKE_CXX_FLAGS}                  ")
message("        cxxflags debug:           ${CMAKE_CXX_FLAGS_DEBUG}            ")
message("        cxxflags release:         ${CMAKE_CXX_FLAGS_RELEASE}          ")
message("        64-bit int:               ${DBUS_INT64_TYPE}                  ")
message("        32-bit int:               ${DBUS_INT32_TYPE}                  ")
message("        16-bit int:               ${DBUS_INT16_TYPE}                  ")
message("        Doxygen:                  ${DOXYGEN}                          ")
message("        Docbook Generator:        ${DOCBOOK_GENERATOR_NAME}           ")


message("        gcc coverage profiling:   ${DBUS_GCOV_ENABLED}                ")
message("        Building unit tests:      ${DBUS_BUILD_TESTS}                 ")
message("        Building with GLib:       ${DBUS_WITH_GLIB}                   ")
message("        Building verbose mode:    ${DBUS_ENABLE_VERBOSE_MODE}         ")
message("        Building w/o assertions:  ${DBUS_DISABLE_ASSERT}              ")
message("        Building w/o checks:      ${DBUS_DISABLE_CHECKS}              ")
message("        Building bus stats API:   ${DBUS_ENABLE_STATS}                ")
message("        installing system libs:   ${DBUS_INSTALL_SYSTEM_LIBS}         ")
message("        Building inotify support: ${DBUS_BUS_ENABLE_INOTIFY}          ")
message("        Building kqueue support:  ${DBUS_BUS_ENABLE_KQUEUE}           ")
message("        Building Doxygen docs:    ${DBUS_ENABLE_DOXYGEN_DOCS}         ")
message("        Building XML docs:        ${DBUS_ENABLE_XML_DOCS}             ")
message("        Daemon executable name:   ${DBUS_DAEMON_NAME}")
if (WIN32)
message("        System bus address:       ${DBUS_SYSTEM_BUS_DEFAULT_ADDRESS}  ")
message("        Session bus listens on:   ${DBUS_SESSION_BUS_LISTEN_ADDRESS}  ")
message("        Session clients connect to: ${DBUS_SESSION_BUS_CONNECT_ADDRESS} ")
else (WIN32)
message("        System bus socket:        ${DBUS_SYSTEM_SOCKET}               ")
message("        System bus address:       ${DBUS_SYSTEM_BUS_DEFAULT_ADDRESS}  ")
message("        System bus PID file:      ${DBUS_SYSTEM_PID_FILE}             ")
message("        Session bus socket dir:   ${DBUS_SESSION_SOCKET_DIR}          ")
message("        Console auth dir:         ${DBUS_CONSOLE_AUTH_DIR}            ")
message("        System bus user:          ${DBUS_USER}                        ")
message("        'make check' socket dir:  ${TEST_SOCKET_DIR}                  ")
endif (WIN32)
message("        Test listen address:      ${TEST_LISTEN}                      ")
if (MSVC)
message("        build timestamp:          ${DBUS_BUILD_TIMESTAMP}             ")
endif (MSVC)

MESSAGE(" ")
if (DBUS_BUILD_TESTS)
    message("NOTE: building with unit tests increases the size of the installed library and renders it insecure.")
endif(DBUS_BUILD_TESTS)

if (DBUS_BUILD_TESTS AND DBUS_DISABLE_ASSERT)
    message("NOTE: building with unit tests but without assertions means tests may not properly report failures (this configuration is only useful when doing something like profiling the tests)")
endif(DBUS_BUILD_TESTS AND DBUS_DISABLE_ASSERT)

if (DBUS_GCOV_ENABLED)
    message("NOTE: building with coverage profiling is definitely for developers only.")
endif(DBUS_GCOV_ENABLED)

if (DBUS_ENABLE_VERBOSE_MODE)
    message("NOTE: building with verbose mode increases library size, may slightly increase security risk, and decreases performance.")
endif(DBUS_ENABLE_VERBOSE_MODE)

if(NOT DBUS_DISABLE_ASSERT)
    message("NOTE: building with assertions increases library size and decreases performance.")
endif(NOT DBUS_DISABLE_ASSERT)

if (DBUS_DISABLE_CHECKS)
    message("NOTE: building without checks for arguments passed to public API makes it harder to debug apps using D-BUS, but will slightly decrease D-BUS library size and _very_ slightly improve performance.")
endif(DBUS_DISABLE_CHECKS)

foreach(_note ${FOOTNOTES})
    message(${_note})
endforeach()

MESSAGE(" ")

INCLUDE(modules/CPackInstallConfig.cmake)

add_custom_target(help-options
    cmake -LH 
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
