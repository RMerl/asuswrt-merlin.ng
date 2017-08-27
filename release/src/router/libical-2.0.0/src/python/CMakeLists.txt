include_directories(
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/libical
  ${CMAKE_SOURCE_DIR}/src/libicalss
)

########### next target ###############

set(LibicalWrap_LIB_SRCS
  LibicalWrap.c
)

add_library(LibicalWrap ${LIBRARY_TYPE} ${LibicalWrap_LIB_SRCS})

target_link_libraries(LibicalWrap)

set_target_properties(LibicalWrap PROPERTIES
  VERSION ${LIBICAL_LIB_VERSION_STRING}
  SOVERSION ${LIBICAL_LIB_MAJOR_VERSION}
)
install(TARGETS LibicalWrap DESTINATION lib)

########### install files ###############

