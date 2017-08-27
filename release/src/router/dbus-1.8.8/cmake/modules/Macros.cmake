if(DBUS_BUILD_TESTS AND CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
        find_file(WINE_EXECUTABLE
            NAMES wine
            PATHS /usr/bin /usr/local/bin
            NO_CMAKE_FIND_ROOT_PATH
        )
        find_file(HAVE_BINFMT_WINE_SUPPORT
            NAMES DOSWin wine Wine windows Windows
            PATHS /proc/sys/fs/binfmt_misc
            NO_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH
        )
        if(WINE_EXECUTABLE AND HAVE_BINFMT_WINE_SUPPORT)
            list(APPEND FOOTNOTES "NOTE: The requirements to run cross compiled applications on your host system are achieved. You may run 'make check'.")
        endif()
        if(NOT WINE_EXECUTABLE)
            list(APPEND FOOTNOTES "NOTE: You may install the Windows emulator 'wine' to be able to run cross compiled test applications.")
        endif()
        if(NOT HAVE_BINFMT_WINE_SUPPORT)
            list(APPEND FOOTNOTES "NOTE: You may activate binfmt_misc support for wine to be able to run cross compiled test applications.")
        endif()
    else()
        list(APPEND FOOTNOTES "NOTE: You will not be able to run cross compiled applications on your host system.")
    endif()
endif()

MACRO(TIMESTAMP RESULT)
    if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        EXECUTE_PROCESS(COMMAND "cmd" " /C date /T" OUTPUT_VARIABLE DATE)
        string(REGEX REPLACE "(..)[/.](..)[/.](....).*" "\\3\\2\\1" DATE ${DATE})
        EXECUTE_PROCESS(COMMAND "cmd" " /C time /T" OUTPUT_VARIABLE TIME)
        string(REGEX REPLACE "(..):(..)" "\\1\\2" TIME ${TIME})
        set (${RESULT} "${DATE}${TIME}")
    else ()
        EXECUTE_PROCESS(COMMAND "date" "+%Y%m%d%H%M" OUTPUT_VARIABLE ${RESULT})
    endif ()
ENDMACRO()

macro(add_test_executable _target _source)
    add_executable(${_target} ${_source})
    target_link_libraries(${_target} ${ARGN})
    if (CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # run tests with binfmt_misc
        set(PREFIX "z:")
        set(_env "DBUS_TEST_DAEMON=${PREFIX}${CMAKE_BINARY_DIR}/bin/dbus-daemon${EXEEXT}")
        add_test(NAME ${_target} COMMAND $<TARGET_FILE:${_target}>)
    else()
        set(PREFIX)
        set(_env "DBUS_TEST_DAEMON=${CMAKE_BINARY_DIR}/bin/dbus-daemon${EXEEXT}")
        add_test(NAME ${_target} COMMAND $<TARGET_FILE:${_target}>)
    endif()
    list(APPEND _env "DBUS_SESSION_BUS_ADDRESS=")
    list(APPEND _env "DBUS_FATAL_WARNINGS=1")
    list(APPEND _env "DBUS_TEST_DATA=${PREFIX}${CMAKE_BINARY_DIR}/test/data")
    list(APPEND _env "DBUS_TEST_HOMEDIR=${PREFIX}${CMAKE_BINARY_DIR}/dbus")
    set_tests_properties(${_target} PROPERTIES ENVIRONMENT "${_env}")
endmacro(add_test_executable)

macro(add_helper_executable _target _source)
    add_executable(${_target} ${_source})
    target_link_libraries(${_target} ${ARGN})
endmacro(add_helper_executable)
