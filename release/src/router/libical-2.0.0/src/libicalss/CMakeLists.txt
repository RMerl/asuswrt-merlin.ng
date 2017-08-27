add_definitions(-Dlibical_icalss_EXPORTS)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src/libical
  ${CMAKE_SOURCE_DIR}/src/libical
)
if(WITH_BDB)
  include_directories(${BDB_INCLUDE_DIR})
endif()

if(WIN32)
  set(TOPS "\"${CMAKE_SOURCE_DIR}\"")
  set(TOPB "\"${CMAKE_BINARY_DIR}\"")
else()
  set(TOPS "${CMAKE_SOURCE_DIR}")
  set(TOPB "${CMAKE_BINARY_DIR}")
endif()

add_custom_target(
  icalss-header ALL
  COMMAND
  ${CMAKE_COMMAND}
    -DTOPS:FILEPATH=${TOPS}
    -DTOPB:FILEPATH=${TOPB}
    -DICAL_FILE_H_FILE:FILEPATH=${CMAKE_BINARY_DIR}/src/libicalss/icalss.h
    -DWITH_BDB=${WITH_BDB}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/icalss_file.cmake
  DEPENDS
  ical-header
)

########### next target ###############

#these are generated sources, but we keep them in the repo
set(icalss_LIB_DEVSRCS icalsslexer.c icalssyacc.c)

set(icalss_LIB_SRCS
  libical_icalss_export.h
  icalcalendar.c
  icalcalendar.h
  icalclassify.c
  icalclassify.h
  icalcluster.c
  icalcluster.h
  icalclusterimpl.h
  icalgauge.c
  icalgauge.h
  icalgaugeimpl.h
  icaldirset.c
  icaldirset.h
  icaldirsetimpl.h
  icalfileset.c
  icalfileset.h
  icalfilesetimpl.h
  icalset.c
  icalset.h
  icalssyacc.h
  icalspanlist.c
  icalspanlist.h
  icalmessage.c
  icalmessage.h
  ${icalss_LIB_DEVSRCS}
)
if(WITH_BDB)
  list(APPEND icalss_LIB_SRCS
    icalbdbset.c
    icalbdbset.h
    icalbdbsetimpl.h
  )
endif()

add_library(icalss ${LIBRARY_TYPE} ${icalss_LIB_SRCS})
add_dependencies(icalss icalss-header)
if(NOT SHARED_ONLY)
  add_library(icalss-static STATIC ${icalss_LIB_SRCS})
  add_dependencies(icalss-static icalss-header)
endif()

target_link_libraries(icalss ical)
if(WITH_BDB)
  target_link_libraries(icalss ${BDB_LIBRARY})
endif()

if(MSVC)
  set_target_properties(icalss PROPERTIES OUTPUT_NAME "libicalss")
  if(NOT SHARED_ONLY)
    set_target_properties(icalss-static PROPERTIES OUTPUT_NAME "libicalss-static")
  endif()
else()
  if(NOT SHARED_ONLY)
    set_target_properties(icalss-static PROPERTIES OUTPUT_NAME "icalss")
  endif()
endif()
set_target_properties(icalss PROPERTIES
  VERSION ${LIBICAL_LIB_VERSION_STRING}
  SOVERSION ${LIBICAL_LIB_MAJOR_VERSION}
)
set_target_properties(icalss PROPERTIES CLEAN_DIRECT_OUTPUT 1)
if(NOT SHARED_ONLY)
  set_target_properties(icalss-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
endif()

install(
  TARGETS icalss
  EXPORT icalTargets
  DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS}
)
if(NOT SHARED_ONLY)
  install(
    TARGETS icalss-static
    EXPORT icalTargets
    DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS}
  )
endif()

########### next target ###############

if(WITH_CXX_BINDINGS)
  set(icalsscxx_LIB_SRCS
    icalspanlist_cxx.cpp
    icalspanlist_cxx.h
  )
  if(WITH_BDB)
    list(APPEND icalsscxx_LIB_SRCS
      icalbdbset_cxx.h
    )
  endif()
  add_library(icalss_cxx ${LIBRARY_TYPE} ${icalsscxx_LIB_SRCS})
  add_dependencies(icalss_cxx icalss-header)
  if(NOT SHARED_ONLY)
    add_library(icalss_cxx-static STATIC ${icalsscxx_LIB_SRCS})
    add_dependencies(icalss_cxx-static icalss-header)
  endif()

  target_link_libraries(icalss_cxx icalss ical_cxx ${CMAKE_THREAD_LIBS_INIT})

  if(MSVC)
    set_target_properties(icalss_cxx PROPERTIES OUTPUT_NAME "libicalss_cxx")
    if(NOT SHARED_ONLY)
      set_target_properties(icalss_cxx-static PROPERTIES OUTPUT_NAME "libicalss_cxx-static")
    endif()
  else()
    if(NOT SHARED_ONLY)
      set_target_properties(icalss_cxx-static PROPERTIES OUTPUT_NAME "icalss_cxx")
    endif()
  endif()
  set_target_properties(icalss_cxx PROPERTIES
    VERSION ${LIBICAL_LIB_VERSION_STRING}
    SOVERSION ${LIBICAL_LIB_MAJOR_VERSION}
  )
  set_target_properties(icalss_cxx PROPERTIES CLEAN_DIRECT_OUTPUT 1)
  if(NOT SHARED_ONLY)
    set_target_properties(icalss_cxx-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
  endif()

  install(
    TARGETS icalss_cxx
    EXPORT icalTargets
    DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS}
  )
  if(NOT SHARED_ONLY)
    install(
      TARGETS icalss_cxx-static
      EXPORT icalTargets
      DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS}
    )
  endif()
endif()

########### install files ###############

install(FILES
  ${CMAKE_BINARY_DIR}/src/libicalss/icalss.h
  icalcalendar.h
  icalclassify.h
  icalcluster.h
  icaldirset.h
  icaldirsetimpl.h
  icalfileset.h
  icalfilesetimpl.h
  icalgauge.h
  icalgaugeimpl.h
  icalmessage.h
  icalset.h
  icalspanlist.h
  icalssyacc.h
  libical_icalss_export.h
  DESTINATION
  ${INCLUDE_INSTALL_DIR}/libical
)

if(WITH_CXX_BINDINGS)
  install(FILES
    icalspanlist_cxx.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/libical
  )
  if(WITH_BDB)
    install(FILES icalbdbset_cxx.h
      DESTINATION
      ${INCLUDE_INSTALL_DIR}/libical
    )
  endif()
endif()
