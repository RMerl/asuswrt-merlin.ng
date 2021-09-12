cmake_minimum_required(VERSION 3.1.0)

project(QRencode VERSION 4.1.1 LANGUAGES C)

option(WITH_TOOLS "Build utility tools"  YES )
option(WITH_TESTS "Build tests"  NO )
option(WITHOUT_PNG "Disable PNG support" NO)
option(GPROF "Generate extra code to write profile information" OFF)
option(COVERAGE "Generate extra code to write coverage information" OFF)
option(ASAN "Use AddressSanitizer" OFF)
option(BUILD_SHARED_LIBS "Enable build of shared libraries" NO)

if(BUILD_TESTING)
    set(WITH_TESTS ON)
    message(DEPRECATION "use WITH_TESTS option instead BUILD_TESTING")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads)
find_package(PNG)
find_package(Iconv)

if(CMAKE_USE_PTHREADS_INIT)
    add_definitions(-DHAVE_LIBPTHREAD=1)
    # for libqrencode.pc
    set(LIBPTHREAD ${CMAKE_THREAD_LIBS_INIT})
endif()

## Check for system include files
include(CheckIncludeFile)
include(CheckFunctionExists)

check_include_file(dlfcn.h    HAVE_DLFCN_H   )
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(memory.h   HAVE_MEMORY_H  )
check_include_file(stdint.h   HAVE_STDINT_H  )
check_include_file(stdlib.h   HAVE_STDLIB_H  )
check_include_file(strings.h  HAVE_STRINGS_H )
check_include_file(string.h   HAVE_STRING_H  )
check_include_file(getopt.h   HAVE_GETOPT_H  )
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(time.h     HAVE_TIME_H    )
check_include_file(pthread.h  HAVE_PTHREAD_H )

check_function_exists(strdup HAVE_STRDUP)

if(HAVE_STRDUP)
    add_definitions(-DHAVE_STRDUP=1)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()

if(GPROF)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
endif()

if(COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
endif()

if(ASAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
endif()

add_definitions(-DMAJOR_VERSION=${PROJECT_VERSION_MAJOR})
add_definitions(-DMINOR_VERSION=${PROJECT_VERSION_MINOR})
add_definitions(-DMICRO_VERSION=${PROJECT_VERSION_PATCH})
add_definitions(-DVERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
add_definitions(-DHAVE_SDL=0)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")

    add_definitions(-Dstrcasecmp=_stricmp)
    add_definitions(-Dstrncasecmp=_strnicmp)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    
    if(WITH_TOOLS)
        find_path(GETOPT_INCLUDE_DIR getopt.h PATH_SUFFIXES include)
        find_library(GETOPT_LIBRARIES getopt PATH_SUFFIXES lib)
        include_directories(${GETOPT_INCLUDE_DIR})
    endif(WITH_TOOLS)
endif(MSVC)

set(QRENCODE_SRCS qrencode.c
                  qrinput.c
                  bitstream.c
                  qrspec.c
                  rsecc.c
                  split.c
                  mask.c
                  mqrspec.c
                  mmask.c)

set(QRENCODE_HDRS qrencode_inner.h
                  qrinput.h
                  bitstream.h
                  qrspec.h
                  rsecc.h
                  split.h
                  mask.h
                  mqrspec.h
                  mmask.h)

if(BUILD_SHARED_LIBS)
    if(MSVC)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()
    add_library(qrencode SHARED ${QRENCODE_SRCS} ${QRENCODE_HDRS})
    set_target_properties(qrencode PROPERTIES VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} SOVERSION ${PROJECT_VERSION_MAJOR})
else()
    add_library(qrencode ${QRENCODE_SRCS} ${QRENCODE_HDRS})
endif()
if(CMAKE_USE_PTHREADS_INIT)
    target_link_libraries(qrencode Threads::Threads)
endif()

include(GNUInstallDirs)
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "${CMAKE_INSTALL_FULL_BINDIR}")
set(libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
set(includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
set(VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

configure_file(qrencode.1.in qrencode.1 @ONLY)
configure_file(libqrencode.pc.in libqrencode.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qrencode.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libqrencode.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(FILES qrencode.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS qrencode DESTINATION ${CMAKE_INSTALL_LIBDIR})

## Build utility tools
if(WITH_TOOLS)
    if(NOT WITHOUT_PNG)
        add_definitions(-DHAVE_PNG=1)
    endif()
    add_executable(qrenc qrenc.c)
    set_target_properties(qrenc PROPERTIES OUTPUT_NAME qrencode)

    if(NOT WITHOUT_PNG)
        target_link_libraries(qrenc qrencode PNG::PNG)
    else()
        target_link_libraries(qrenc qrencode)
    endif()

    if(MSVC)
        target_link_libraries(qrenc ${GETOPT_LIBRARIES})
    endif(MSVC)

    install(TARGETS qrenc DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(WITH_TESTS)
    enable_testing()
    add_definitions(-DWITH_TESTS=)
    add_definitions(-DSTATIC_IN_RELEASE=)
    add_subdirectory(tests)
else()
    add_definitions(-DSTATIC_IN_RELEASE=static)
endif()

## ==============================================================================
##
##  Configuration summary
##
## ==============================================================================

message(STATUS "------------------------------------------------------------" )
message(STATUS "[QRencode] Configuration summary."                            )
message(STATUS "------------------------------------------------------------ ")
message(STATUS " System configuration:"                                       )
message(STATUS " .. Processor type .............. = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS " .. CMake executable ............ = ${CMAKE_COMMAND}"         )
message(STATUS " .. CMake version ............... = ${CMAKE_VERSION}"         )
message(STATUS " .. System name ................. = ${CMAKE_SYSTEM}"          )
message(STATUS " .. C++ compiler ................ = ${CMAKE_CXX_COMPILER}"    )
message(STATUS " .. C compiler .................. = ${CMAKE_C_COMPILER}"      )
message(STATUS " .. size(void*) ................. = ${CMAKE_SIZEOF_VOID_P}"   )
message(STATUS " Dependencies:"                                               )
#message(STATUS " .. Doxygen ..................... = ${DOXYGEN_EXECUTABLE}"    )
message(STATUS " .. Thread library of the system  = ${CMAKE_THREAD_LIBS_INIT}")
message(STATUS " .. Iconv ....................... = ${ICONV_FOUND}"           )
message(STATUS " .... Iconv includes ............ = ${ICONV_INCLUDE_DIR}"     )
message(STATUS " .... Iconv library ............. = ${ICONV_LIBRARIES}"       )
message(STATUS " .. ZLIB ........................ = ${ZLIB_FOUND}"            )
message(STATUS " .. PNG ......................... = ${PNG_FOUND}"             )
message(STATUS " .... PNG includes .............. = ${PNG_INCLUDE_DIR}"       )
message(STATUS " .... PNG library ............... = ${PNG_LIBRARIES}"         )
#message(STATUS " .. Memory checker .............. = ${MEMORYCHECK_COMMAND}"   )
message(STATUS " Project configuration:"                                      )
message(STATUS " .. Build test programs  ........ = ${WITH_TESTS}"            )
message(STATUS " .. Build utility tools  ........ = ${WITH_TOOLS}"            )
message(STATUS " .. Disable PNG support  ........ = ${WITHOUT_PNG}"           )
message(STATUS " .. Installation prefix ......... = ${CMAKE_INSTALL_PREFIX}"  )
message(STATUS "------------------------------------------------------------ ")
