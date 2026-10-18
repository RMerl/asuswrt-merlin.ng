set(CMAKE_C_STANDARD 99)

include(CheckSymbolExists)
include(CheckIncludeFiles)
include(FindPkgConfig)

find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
if (ENABLE_MQTT)
  pkg_check_modules(MQTT REQUIRED libmosquitto)
endif ()
if (ENABLE_WEBSOCKETS)
  pkg_check_modules(WebSockets REQUIRED libwebsockets>=4.1.0)
endif ()

check_include_files(execinfo.h HAVE_EXECINFO_H)
check_include_files(malloc.h HAVE_MALLOC_H)
check_symbol_exists(mallinfo2 malloc.h HAVE_MALLINFO2)
check_symbol_exists(mallinfo malloc.h HAVE_MALLINFO)

add_library(core
    mtp_exec.c
    dm_exec.c
    bdc_exec.c
    stomp.c
    socket_set.c
    msg_handler.c
    handle_get.c
    handle_set.c
    handle_add.c
    handle_delete.c
    handle_notify.c
    handle_operate.c
    handle_get_supported_protocol.c
    handle_get_instances.c
    handle_get_supported_dm.c
    group_get_vector.c
    group_set_vector.c
    group_add_vector.c
    group_del_vector.c
    proto_trace.c
    data_model.c
    error_resp.c
    usp_register.c
    usp_api.c
    dm_access.c
    device_local_agent.c
    device_controller.c
    device_mtp.c
    device_stomp.c
    device_subscription.c
    device_security.c
    device_ctrust.c
    device_bulkdata.c
    device_selftest_example.c
    device_time.c
    uptime.c
    rfc1123.c
    database.c
    usp_err.c
    usp_log.c
    usp_mem.c
    nu_ipaddr.c
    nu_macaddr.c
    retry_wait.c
    path_resolver.c
    str_vector.c
    int_vector.c
    kv_vector.c
    dm_inst_vector.c
    expr_vector.c
    dm_trans.c
    subs_vector.c
    subs_retry.c
    sync_timer.c
    cli_server.c
    cli_client.c
    iso8601.c
    text_utils.c
    os_utils.c
    device_request.c
    dllist.c
    coap_common.c
    coap_client.c
    coap_server.c
    uri.c
    mqtt.c
    device_mqtt.c
    wsclient.c
    wsserver.c
    usp_record.c
    sar_vector.c
    e2e_context.c
    device_uds.c
    uds.c
    plugin.c
    msg_utils.c
    usp_broker.c
    usp_service.c)

target_include_directories(core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/protobuf-c
    ${PROJECT_SOURCE_DIR}/src/vendor
    ${PROJECT_SOURCE_DIR}/src/libjson
    ${PROJECT_SOURCE_DIR}/src/include
    $<$<BOOL:${ENABLE_MQTT}>:${MQTT_INCLUDE_DIRS}>
    $<$<BOOL:${ENABLE_WEBSOCKETS}>:${WebSockets_INCLUDE_DIRS}>)

target_compile_definitions(core
  PRIVATE
    $<$<BOOL:${HAVE_MALLOC_H}>:HAVE_MALLOC_H>
    $<$<BOOL:${HAVE_MALLINFO}>:HAVE_MALLINFO>
    $<$<BOOL:${HAVE_MALLINFO2}>:HAVE_MALLINFO2>
    $<$<BOOL:${HAVE_EXECINFO_H}>:HAVE_EXECINFO_H>
    OBUSPA_LOCAL_STATE_DIR="${OBUSPA_LOCAL_STATE_DIR}"
    $<IF:$<BOOL:${ENABLE_STOMP}>,ENABLE_STOMP,DISABLE_STOMP>  # DISABLE_STOMP is the define used in core
    $<$<BOOL:${ENABLE_MQTT}>:ENABLE_MQTT>
    $<$<BOOL:${ENABLE_COAP}>:ENABLE_COAP>
    $<$<BOOL:${ENABLE_WEBSOCKETS}>:ENABLE_WEBSOCKETS>
    $<$<BOOL:${ENABLE_UDS}>:ENABLE_UDS>
    _GNU_SOURCE=1)

target_link_libraries(core
  PRIVATE
    Threads::Threads
    SQLite::SQLite3
    ZLIB::ZLIB
    OpenSSL::SSL
    CURL::libcurl
    $<$<BOOL:${ENABLE_MQTT}>:${MQTT_LIBRARIES}>
    $<$<BOOL:${ENABLE_WEBSOCKETS}>:${WebSockets_LIBRARIES}>
    m
    dl
    libjson
    protobuf-c
    vendor)

target_compile_options(core
  PRIVATE
    -Wall
    -Wfatal-errors
    -Werror
    -Werror=unused-value
    -Werror=format
    -Winit-self
    -Wparentheses
    -Werror=parentheses
    -Wuninitialized
    -Werror=uninitialized
    -Wpointer-arith
    -Wno-portability)
