add_library(vendor
    vendor.c
    vendor_factory_reset_example.c)

target_include_directories(vendor
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/src/vendor
    ${PROJECT_SOURCE_DIR}/src/include)

target_compile_definitions(vendor
  PRIVATE
    $<IF:$<BOOL:${ENABLE_STOMP}>,ENABLE_STOMP,DISABLE_STOMP>  # DISABLE_STOMP is the define used in core
    $<$<BOOL:${ENABLE_MQTT}>:ENABLE_MQTT>
    $<$<BOOL:${ENABLE_COAP}>:ENABLE_COAP>
    $<$<BOOL:${ENABLE_WEBSOCKETS}>:ENABLE_WEBSOCKETS>
    $<$<BOOL:${ENABLE_UDS}>:ENABLE_UDS>
    _GNU_SOURCE=1)
