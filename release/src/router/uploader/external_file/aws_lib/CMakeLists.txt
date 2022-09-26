cmake_minimum_required (VERSION 3.0)

# Set project name
project (aws_sigv4_lib)

find_library(LIBOPENSSL ssl)
find_library(LIBCRYPTO crypto)

add_library(aws_sigv4 SHARED
    aws_sigv4_common.c
    aws_sigv4.c
)

target_link_libraries(aws_sigv4
    "${LIBOPENSSL}"
    "${LIBCRYPTO}"
)
