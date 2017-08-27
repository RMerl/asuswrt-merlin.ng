if(NOT SHARED_ONLY)
  add_definitions(-DLIBICAL_ICAL_STATIC_DEFINE)
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/libical
  ${CMAKE_BINARY_DIR}/src/libical
  ${CMAKE_SOURCE_DIR}/src/libicalss
)

if(ICU_FOUND)
  include_directories(${ICU_INCLUDE_DIR})
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

set(TEST_DATADIR "\\\"${CMAKE_SOURCE_DIR}/test-data\\\"")
add_definitions(-DTEST_DATADIR=${TEST_DATADIR})

macro(setprops _name)
  if(UNIX)
    set_tests_properties(${_name} PROPERTIES
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
  endif()
  if(WIN32)
    set_tests_properties(${_name} PROPERTIES
      ENVIRONMENT "PATH=${EXECUTABLE_OUTPUT_PATH};${CMAKE_BINARY_DIR}/bin;%PATH%"
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
  endif()
  if(CYGWIN OR MSYS)
    set_tests_properties(${_name} PROPERTIES
      ENVIRONMENT "PATH=${EXECUTABLE_OUTPUT_PATH}:${CMAKE_BINARY_DIR}/lib:$ENV{PATH}"
    )
  endif()
endmacro()

#macro to build (but do not create a test)
macro(buildme _name _srcs)
  add_executable(${_name} ${_srcs})
  if(NOT STATIC_ONLY)
    add_dependencies(${_name} ical icalss icalvcal)
    target_link_libraries(${_name} ical icalss icalvcal)
    if(WITH_CXX_BINDINGS)
      target_link_libraries(${_name} ical_cxx icalss_cxx)
    endif()
  else()
    add_dependencies(${_name} ical-static icalss-static icalvcal-static)
    if(NOT WIN32)
      target_link_libraries(${_name}
        ${CMAKE_BINARY_DIR}/lib/libical.a
        ${CMAKE_BINARY_DIR}/lib/libicalss.a
        ${CMAKE_BINARY_DIR}/lib/libicalvcal.a
      )
    else()
      target_link_libraries(${_name}
        ${CMAKE_BINARY_DIR}/bin/libical-static.lib
        ${CMAKE_BINARY_DIR}/bin/libicalss-static.lib
        ${CMAKE_BINARY_DIR}/bin/libicalvcal-static.lib
      )
    endif()
    target_link_libraries(${_name} ${CMAKE_THREAD_LIBS_INIT})
    if(ICU_FOUND)
      target_link_libraries(${_name} ${ICU_LIBRARY})
    endif()
    if(ICU_I18N_FOUND)
      target_link_libraries(${_name} ${ICU_I18N_LIBRARY})
    endif()
    if(WITH_BDB)
      target_link_libraries(${_name} ${BDB_LIBRARY})
    endif()
    if(WITH_CXX_BINDINGS)
      if(NOT WIN32)
        target_link_libraries(${_name}
          ${CMAKE_BINARY_DIR}/lib/libical_cxx.a
          ${CMAKE_BINARY_DIR}/lib/libicalss_cxx.a
        )
      else()
        target_link_libraries(${_name}
          ${CMAKE_BINARY_DIR}/bin/libical_cxx-static.lib
          ${CMAKE_BINARY_DIR}/bin/libicalss_cxx-static.lib
        )
      endif()
    endif()
  endif()
endmacro()

macro(testme _name _srcs)
  buildme(${_name} "${_srcs}")
  add_test(NAME ${_name} COMMAND ${_name})
  setprops(${_name})
endmacro()

########### next target ###############

set(copycluster_SRCS copycluster.c)
buildme(copycluster "${copycluster_SRCS}")

########### next target ###############

set(regression_SRCS
  regression.c
  regression.h
  regression-component.c
  regression-classify.c
  regression-utils.c
  regression-recur.c
  regression-storage.c
)
if(WITH_CXX_BINDINGS)
  list(APPEND regression_SRCS regression-cxx.cpp)
endif()
testme(regression "${regression_SRCS}")

########### next target ###############

set(parser_SRCS icaltestparser.c)
buildme(parser "${parser_SRCS}")

########### next target ###############

if(NOT WIN32)
  #stow is very Unix specific
  set(stow_SRCS stow.c)
  buildme(stow "${stow_SRCS}")
endif()

########### next target ###############

set(recur_SRCS recur.c)
testme(recur "${recur_SRCS}")

########### next target ###############

if(HAVE_GETOPT) #getopt is required
  set(icalrecur_test_SRCS icalrecur_test.c)
  buildme(icalrecur_test "${icalrecur_test_SRCS}")
endif()

########### next target ###############

if(HAVE_GETOPT) #getopt is required
  set(testmime_SRCS testmime.c)
  testme(testmime "${testmime_SRCS}")
endif()

########### next target ###############

set(testvcal_SRCS testvcal.c)
testme(testvcal "${testvcal_SRCS}")

########### next target ###############

set(process_SRCS process.c)
testme(process "${process_SRCS}")

########### next target ###############

if(NOT USE_BUILTIN_TZDATA)
  set(timezones_SRCS timezones.c)
  testme(timezones "${timezones_SRCS}")
endif()

########### next target ###############

set(builtin_timezones_SRCS builtin_timezones.c)
testme(builtin_timezones "${builtin_timezones_SRCS}")

########### next target ###############

if(NOT WIN32) #since we currently do not have a Windows reference file
  if(HAVE_GETOPT) #getopt is required
    if(NOT USE_32BIT_TIME_T) #tests for years greater than 2037 will fail
      set(icalrecurtest_SRCS icalrecur_test.c)
      add_executable(icalrecurtest ${icalrecurtest_SRCS})
      target_link_libraries(icalrecurtest ical icalss icalvcal)
      set(test_cmd "${CMAKE_BINARY_DIR}/src/test/icalrecurtest${CMAKE_EXECUTABLE_SUFFIX}")

      if(ICU_FOUND)
        #test rscale capable rrules
        set(test_args "-r")
        if(HAVE_ICU_DANGI)
          set(reference_data "icalrecur_withicu_dangi_test.out")
        else()
          set(reference_data "icalrecur_withicu_test.out")
        endif()
        add_test(NAME icalrecurtest-r
          COMMAND ${CMAKE_COMMAND}
          -D test_cmd=${test_cmd}
          -D test_args:string=${test_args}
          -D output_blessed=${CMAKE_SOURCE_DIR}/src/test/${reference_data}
          -D output_test=${CMAKE_BINARY_DIR}/bin/test.out
          -P ${CMAKE_SOURCE_DIR}/cmake/run_test.cmake
        )
        setprops(icalrecurtest-r)
      endif()

      #test non-rscale rrules
      set(test_args "")
      set(reference_data "icalrecur_test.out")
      add_test(NAME icalrecurtest
        COMMAND ${CMAKE_COMMAND}
        -D test_cmd=${test_cmd}
        -D test_args:string=${test_args}
        -D output_blessed=${CMAKE_SOURCE_DIR}/src/test/${reference_data}
        -D output_test=${CMAKE_BINARY_DIR}/bin/test.out
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test.cmake
      )
      setprops(icalrecurtest)
    endif()
  endif()
endif()
