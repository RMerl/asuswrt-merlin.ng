cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

if (POLICY CMP0075)
  cmake_policy(SET CMP0075 NEW)
endif ()

# The following must be set BEFORE doing project() or enable_language().
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type defined; defaulting to 'Debug'")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
    "The type of build. Possible values are: Debug, Release, RelWithDebInfo and MinSizeRel.")
endif ()

set(PACKAGE "stubby")
set(PACKAGE_NAME "Stubby")
set(PACKAGE_VERSION_MAJOR 0)
set(PACKAGE_VERSION_MINOR 4)
set(PACKAGE_VERSION_RELEASE 0)
set(PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_RELEASE}")
set(PACKAGE_BUGREPORT "team@getdnsapi.net")
set(RELEASE_CANDIDATE "")

set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}${RELEASE_CANDIDATE}")
set(PACKAGE_TARNAME "${PACKAGE}-${PACKAGE_VERSION}${RELEASE_CANDIDATE}")

set(STUBBY_PACKAGE "${PACKAGE}")
set(STUBBY_PACKAGE_STRING "${PACKAGE_STRING}")

if (WIN32)
  option(PATCH_LEVEL_GIT "Get patch level from git." OFF)

  set(STUBBY_ON_WINDOWS 1)
  project (stubby VERSION ${PACKAGE_VERSION} LANGUAGES C RC)
else()
  project (stubby VERSION ${PACKAGE_VERSION} LANGUAGES C)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(CheckIncludeFile)
include(CheckSymbolExists)
include(GNUInstallDirs)
include(CMakeDependentOption)

# Options.
find_package(Libsystemd)
if (Libsystemd_FOUND)
  option(ENABLE_SYSTEMD "Enable systemd support." ON)
endif()
option(ENABLE_GETDNS_STATIC_LINK "Link GetDNS statically." ON)
if (ENABLE_GETDNS_STATIC_LINK)
  set(GETDNS_STATIC ON)
endif ()

# Directories
if (DEFINED CMAKE_INSTALL_FULL_RUNSTATEDIR)
  set(RUNSTATEDIR "${CMAKE_INSTALL_FULL_RUNSTATEDIR}")
else ()
  set(RUNSTATEDIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/run")
endif ()
install(DIRECTORY
  DESTINATION ${RUNSTATEDIR}
  DIRECTORY_PERMISSIONS
  OWNER_READ OWNER_WRITE OWNER_EXECUTE
  GROUP_READ GROUP_EXECUTE
  WORLD_READ WORLD_EXECUTE
  )
set(STUBBYCONFDIR "${CMAKE_INSTALL_FULL_SYSCONFDIR}/stubby")

find_package(Libyaml REQUIRED)

if (STUBBY_ON_WINDOWS)
  check_include_file(windows.h HAVE_WINDOWS_H)

  option(ENABLE_WINDOWS_SERVICE "Enable Windows service interface." ON)

  find_program(CMAKE_MC_COMPILER mc.exe DOC "path to message compiler")
  if (NOT CMAKE_MC_COMPILER)
    message(FATAL_ERROR "message compiler not found: required to build")
  endif (NOT CMAKE_MC_COMPILER)
  message(STATUS "Found message compiler: ${CMAKE_MC_COMPILER}")
  mark_as_advanced(CMAKE_MC_COMPILER)

  set(PACKAGE_VERSION_PATCH 0)
  if (PATCH_LEVEL_GIT)
    execute_process(COMMAND git describe --long OUTPUT_VARIABLE ver)
    if (ver)
      string(REGEX REPLACE "v[^-]*-([0-9]+)-.*" "\\1" PACKAGE_VERSION_PATCH ${ver})
    endif ()
  endif ()
else ()
  set(ENABLE_WINDOWS_SERVICE OFF)
endif ()

check_include_file(os/log.h HAVE_OS_LOG_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)

option(ENABLE_DEBUG_ALL "Enable all debugging messages.")
cmake_dependent_option(ENABLE_DEBUG_SERVER "Enable server debugging messages." OFF "NOT ENABLE_DEBUG_ALL" ON)
set(SERVER_DEBUG ${ENABLE_DEBUG_SERVER})



check_symbol_exists(getopt "unistd.h" HAVE_GETOPT)

# Does the compiler accept the "format" attribute?
try_compile(HAVE_ATTR_FORMAT
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/tests/test_format_attr.c
  )
# Does the compiler accept the "unused" attribute?
try_compile(HAVE_ATTR_UNUSED
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/tests/test_unused_attr.c
  )

configure_file(cmake/include/cmakeconfig.h.in config.h)
set(ETCDIR "${STUBBYCONFDIR}")
configure_file(doc/stubby.1.in stubby.1 @ONLY)

if (STUBBY_ON_WINDOWS)
  set(VER_PRODUCTVERSION "${PACKAGE_VERSION_MAJOR},${PACKAGE_VERSION_MINOR},${PACKAGE_VERSION_RELEASE},${PACKAGE_VERSION_PATCH}")
  set(VER_PRODUCTVERSION_STR "${PACKAGE_VERSION}${RELEASE_CANDIDATE}")
  set(PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_RELEASE}.${PACKAGE_VERSION_PATCH}")
  configure_file(src/windows/stubby.rc.in stubby.rc @ONLY)
  configure_file(src/windows/stubby.ico stubby.ico COPYONLY)
  configure_file(package_version.in package_version @ONLY)
  if (ENABLE_WINDOWS_SERVICE)
    configure_file(src/windows/stubres.rc.in stubres.rc @ONLY)
  endif ()

  add_custom_command(
    OUTPUT service.rc service.h
    MAIN_DEPENDENCY src/windows/service.mc
    COMMAND ${CMAKE_MC_COMPILER}
    ARGS -U
         -n
         -r ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/src/windows/service.mc
    )
endif ()

add_executable(stubby
  src/stubby.c
  src/configfile.c
  src/log.c
  src/server.c
  src/util.c
  src/yaml/convert_yaml_to_json.c
  src/sldns/sbuffer.c
  )
if (NOT HAVE_GETOPT)
  target_sources(stubby PRIVATE src/compat/getopt.c)
  target_include_directories(stubby PRIVATE src/compat)
endif ()
if (STUBBY_ON_WINDOWS)
  target_sources(stubby PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/stubby.rc)
endif ()
if (ENABLE_WINDOWS_SERVICE)
  target_sources(stubby PRIVATE src/windows/service.c ${CMAKE_CURRENT_BINARY_DIR}/service.h)
  target_include_directories(stubby PRIVATE src/windows ${CMAKE_CURRENT_BINARY_DIR})

  add_library(stubres SHARED
    src/windows/stubres.c
    ${CMAKE_CURRENT_BINARY_DIR}/service.rc
    ${CMAKE_CURRENT_BINARY_DIR}/stubres.rc
    )
  set_property(TARGET stubres PROPERTY C_STANDARD 11)
endif()

target_include_directories(stubby PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(stubby PRIVATE Libyaml::Libyaml)
if (ENABLE_SYSTEMD)
  target_link_libraries(stubby PRIVATE Libsystemd::Libsystemd)
endif()
# Are we being built from getdns? If so, use the build tree getdns.
if (TARGET getdns)
  target_link_libraries(stubby PRIVATE getdns)
else ()
  find_package(Getdns "1.5.0" REQUIRED)
  target_link_libraries(stubby PRIVATE Getdns::Getdns)
endif ()
set_property(TARGET stubby PROPERTY C_STANDARD 11)

install(TARGETS stubby RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )
if (ENABLE_WINDOWS_SERVICE)
  install(TARGETS stubres LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/stubby.1 DESTINATION share/man/man1)
install(FILES AUTHORS COPYING ChangeLog NEWS README.md DESTINATION share/doc/stubby)

# Ensure the file gets CRLF line endings on Windows.
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/stubby.yml INPUT ${CMAKE_CURRENT_SOURCE_DIR}/stubby.yml.example)
# Copy stubby.yml to destination iff no destination file exists.
# This is complicated by (a) not being able to use generators, due to
# CMake minimum version requirement, and (b) to account for DESTDIR.
# And in the latter case, if we're adding DESTDIR to the start of the
# path, we must on Windows remove any initial drive letter. That's what
# INSTALL appears to do.
install(CODE "\
  set(targetdir \"${STUBBYCONFDIR}\")\n\
  set(destdir \"\$ENV{DESTDIR}\")\n\
  if (destdir)\n\
    string(REGEX REPLACE \"^([A-Z]:)?/(.*)\" \"\\\\2\" newtarget \"\${targetdir}\")\n\
    if (newtarget)\n\
      set(targetdir \"\${newtarget}\")\n\
    endif ()\n\
    set(targetdir \"\${destdir}/\${newtarget}\")\n\
  endif ()\n\
  if (NOT EXISTS \"\${targetdir}/stubby.yml\")\n\
    file(COPY \"${CMAKE_CURRENT_BINARY_DIR}/stubby.yml\" DESTINATION \"\${targetdir}\")\n\
    message(\"-- Installing: \${targetdir}/stubby.yml\")\n\
  endif ()")

if (APPLE)
  find_library(security Security REQUIRED)
  add_executable(stubby-ui-helper macos/stubby-ui-helper.c)
  target_include_directories(stubby-ui-helper PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries(stubby-ui-helper ${security})

  install(FILES macos/stubby-setdns-macos.sh DESTINATION sbin)
endif ()
