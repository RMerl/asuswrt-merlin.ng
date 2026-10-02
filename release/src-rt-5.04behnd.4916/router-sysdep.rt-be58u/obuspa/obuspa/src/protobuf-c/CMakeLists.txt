add_library(protobuf-c
    usp-msg.pb-c.c
    usp-record.pb-c.c
    protobuf-c.c)

target_include_directories(protobuf-c
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src)  # Because <protobuf-c/protobuf-c.h> is search from parent folder by USP pb files
