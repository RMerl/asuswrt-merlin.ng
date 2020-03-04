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
set(PACKAGE_VERSION "0.3.0")
set(PACKAGE_BUGREPORT "team@getdnsapi.net")
set(RELEASE_CANDIDATE "")

set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}${RELEASE_CANDIDATE}")
set(PACKAGE_TARNAME "${PACKAGE}-${PACKAGE_VERSION}${RELEASE_CANDIDATE}")

set(STUBBY_PACKAGE "${PACKAGE}")
set(STUBBY_PACKAGE_STRING "${PACKAGE_STRING}")

project (stubby VERSION ${PACKAGE_VERSION} LANGUAGES C)

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

if (WIN32)
  set(STUBBY_ON_WINDOWS 1)
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

add_executable(stubby
  src/stubby.c
  src/yaml/convert_yaml_to_json.c
  src/sldns/sbuffer.c
  )
if (NOT HAVE_GETOPT)
  target_sources(stubby PRIVATE src/compat/getopt.c)
  target_include_directories(stubby PRIVATE src/compat)
endif ()

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

install(TARGETS stubby DESTINATION bin)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/stubby.1 DESTINATION share/man/man1)
install(FILES AUTHORS COPYING ChangeLog NEWS README.md DESTINATION share/doc/stubby)

# Ensure the file gets CRLF line endings on Windows.
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/stubby.yml INPUT ${CMAKE_CURRENT_SOURCE_DIR}/stubby.yml.example)
# Copy to destination iff no destination file exists.
install(CODE "\
  set(targetdir ${STUBBYCONFDIR})\n\
  if (\$ENV{DESTDIR})\n\
    set(targetdir \$ENV{DESTDIR}/\${targetdir})\n\
  endif ()\n\
  if (NOT EXISTS \${targetdir}/stubby.yml)\n\
    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/stubby.yml DESTINATION \${targetdir})\n\
  endif ()")

if (APPLE)
  find_library(security Security REQUIRED)
  add_executable(stubby-ui-helper macos/stubby-ui-helper.c)
  target_include_directories(stubby-ui-helper PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries(stubby-ui-helper ${security})

  install(FILES macos/stubby-setdns-macos.sh DESTINATION sbin)
endif ()
