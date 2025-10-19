cmake_minimum_required(VERSION 3.18...3.24)

# Read the AGENT_SOFTWARE_VERSION from version.h file
set(VERSION_REGEX "^#define AGENT_SOFTWARE_VERSION[ ]+\"([0-9.]+)\"$")
file(STRINGS "src/core/version.h" VERSION_STRING REGEX ${VERSION_REGEX})
string(REGEX REPLACE ${VERSION_REGEX} "\\1" VERSION_STRING "${VERSION_STRING}")

project(obuspa
  DESCRIPTION "OB-USP Agent"
  VERSION ${VERSION_STRING}
  HOMEPAGE_URL "https://github.com/BroadbandForum/obuspa/"
  LANGUAGES C)

include(GNUInstallDirs)

set(OBUSPA_LOCAL_STATE_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LOCALSTATEDIR}/obuspa)
message(STATUS "OBUSPA_LOCAL_STATE_DIR: ${OBUSPA_LOCAL_STATE_DIR}")

option(ENABLE_STOMP "enable STOMP Message support" ON)
option(ENABLE_MQTT "enable MQTT Message support" ON)
option(ENABLE_COAP "enable CoAP Message support" ON)
option(ENABLE_WEBSOCKETS "enable WebSockets Message support" ON)
message(STATUS "MTP to support: STOMP:${ENABLE_STOMP} MQTT:${ENABLE_MQTT} COAP:${ENABLE_COAP} WEBSOCKETS:${ENABLE_WEBSOCKETS}")

add_executable(obuspa
    src/core/main.c)

target_include_directories(obuspa
  PRIVATE
    src
    src/include
    src/protobuf-c
    src/core
    src/vendor)

# These definitions are needed because core/main.c requires these definitions.
target_compile_definitions(obuspa
  PRIVATE
    OBUSPA_LOCAL_STATE_DIR="${OBUSPA_LOCAL_STATE_DIR}"
    $<IF:$<BOOL:${ENABLE_STOMP}>,ENABLE_STOMP,DISABLE_STOMP>  # DISABLE_STOMP is the define used in core
    $<$<BOOL:${ENABLE_MQTT}>:ENABLE_MQTT>
    $<$<BOOL:${ENABLE_COAP}>:ENABLE_COAP>
    $<$<BOOL:${ENABLE_WEBSOCKETS}>:ENABLE_WEBSOCKETS>
    _GNU_SOURCE=1)

target_link_libraries(obuspa
  PRIVATE
    vendor
    core)

add_subdirectory(src/libjson)
add_subdirectory(src/protobuf-c)
add_subdirectory(src/vendor)
add_subdirectory(src/core)

# Install executable and create working directory.
# This depends on your prefix setting (default CMAKE_INSTALL_PREFIX=/usr/local/)
# - Default executable path: /usr/local/bin/obuspa
# - Default database path: /usr/local/var/obuspa/usp.db
install(TARGETS obuspa RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY DESTINATION ${OBUSPA_LOCAL_STATE_DIR})
