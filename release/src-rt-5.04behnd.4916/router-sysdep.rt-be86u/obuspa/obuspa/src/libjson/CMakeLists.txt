add_library(libjson
    ccan/json/json.c)

target_include_directories(libjson
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ccan
    ${CMAKE_CURRENT_SOURCE_DIR}/ccan/json)
