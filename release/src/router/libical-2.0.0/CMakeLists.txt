# This is the top-level CMakeLists.txt file for the libical project.
#
# Pass the following variables to cmake to control the build:
# (See doc/UsingLibical.txt for more information)
#
# -DWITH_CXX_BINDINGS=[true|false]
#  Build the C++ bindings.
#  Default=true
#
# -DWITH_BDB=[true|false] [MAY BE BUGGY. DO NOT USE FOR PRODUCTION]
#  Build with the optional Berkeley DB storage.
#  Requires the Berkeley DB development toolkit pre-installed.
#  Default=false
#
# -DICAL_ERRORS_ARE_FATAL=[true|false]
#  Set to make icalerror_* calls abort instead of internally signaling an error
#  Default=false
#  Notes:
#   Change the behavior at runtime using the icalerror_set_errors_are_fatal() function.
#   Query the behavior at runtime using the icalerror_get_errors_are_fatal() function.
#
# -DNO_WARN_DEPRECATED=[true|false]
#  Set if you DO NOT WANT to see deprecated messages.
#  Default=true
#
# -DICAL_ALLOW_EMPTY_PROPERTIES=[true|false]
#  Set to prevent empty properties from being replaced with X-LIC-ERROR properties.
#  Default=false
#
# -DUSE_BUILTIN_TZDATA=[true|false]
#  Set to build using our own timezone data.
#  Default=false (use the system timezone data on non-Windows systems)
#  ALWAYS true on Windows systems
#
# -DSTATIC_ONLY=[true|false]
#  Set to build static libraries only.
#  Default=false (build shared and static libs)
#
# -DSHARED_ONLY=[true|false]
#  Set to build shared (dynamic) libraries only.
#  Default=false (build shared and static libs)
#  Takes precedence over STATIC_ONLY
#
# -DGOBJECT_INTROSPECTION=[true|false]
#  Set to build GObject introspection "typelib" files
#  Requires GObject Introspection development package v0.6.7 or higher
#  Default=false (do not generate the introspection files)
#
# -DUSE_32BIT_TIME_T=[true|false]
#  Set to build using a 32bit time_t (ignored unless building with MSVC on Windows)
#  Default=false (use the default size of time_t)
#
# -DUSE_INTEROPERABLE_VTIMEZONES=[true|false]
#  Set to use inter-operable rather than exact vtimezones.
#  Default=false (build exact vtimezones)
#  Notes:
#   Change the behavior at runtime using the icaltzutil_set_exact_vtimezones_support() function.
#   Query the behavior at runtime using the icaltzutil_get_exact_vtimezones_support() function.
#

cmake_minimum_required(VERSION 2.8.9) #first line, to shutup a cygwin warning
project(libical C CXX)

cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0005 OLD)
if(POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()
if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

enable_testing()

if(WINCE)
  find_package(Wcecompat REQUIRED)
  include_directories(${WCECOMPAT_INCLUDE_DIR})
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${WCECOMPAT_INCLUDE_DIR})
  set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${WCECOMPAT_LIBRARIES})
endif()

set(LIBICAL_LIB_MAJOR_VERSION "2")
set(LIBICAL_LIB_MINOR_VERSION "0")
set(LIBICAL_LIB_PATCH_VERSION "0")
set(LIBICAL_LIB_VERSION_STRING
  "${LIBICAL_LIB_MAJOR_VERSION}.${LIBICAL_LIB_MINOR_VERSION}.${LIBICAL_LIB_PATCH_VERSION}"
)

set(PROJECT_VERSION "${LIBICAL_LIB_MAJOR_VERSION}.${LIBICAL_LIB_MINOR_VERSION}")
set(PROJECT_URL "http://libical.github.io/libical/")

# compile in Berkeley DB support
option(WITH_BDB "Build in Berkeley DB Support.  Requires pre-installed Berkeley DB development toolkit")
if(WITH_BDB)
  find_package(BDB)
  if(BDB_FOUND)
    add_definitions(-DWITH_BDB)
  else()
    message(FATAL_ERROR "Cannot build the Berkeley DB storage support as requested. Unable to the locate the pre-installed Berkeley DB development toolkit.")
  endif()
endif()

# library build types
set(LIBRARY_TYPE SHARED)

option(STATIC_ONLY "Build static libraries only.")
if(STATIC_ONLY)
  set(LIBRARY_TYPE STATIC)
endif()

option(SHARED_ONLY "Build shared (dynamic) libraries only. Takes precedence over STATIC_ONLY")
if(SHARED_ONLY)
  set(STATIC_ONLY False)
  set(LIBRARY_TYPE SHARED)
endif()

# must have Perl to create the derived stuff
find_package(Perl REQUIRED)

# Ensure finding 64bit libs when using 64-bit compilers
if(CMAKE_CL_64)
  set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS True)
endif()

# libicu is highly recommended for RSCALE support
#  libicu can be found at http://www.icu-project.org
#  RSCALE info at http://tools.ietf.org/html/rfc7529
find_package(ICU)
if(ICU_FOUND)
  set(HAVE_LIBICU 1)
  if(ICU_MAJOR_VERSION VERSION_GREATER 50)
    set(HAVE_ICU_DANGI TRUE)
  else()
    set(HAVE_ICU_DANGI FALSE)
  endif()
endif()
if(ICU_I18N_FOUND)
  set(HAVE_LIBICU_I18N 1)
endif()

# MSVC specific definitions
if(WIN32)
  if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -DYY_NO_UNISTD_H)
    option(USE_32BIT_TIME_T "Build using a 32bit time_t (ignored unless building with MSVC on Windows).")
    if(USE_32BIT_TIME_T)
      add_definitions(-D_USE_32BIT_TIME_T)
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4290")
  endif()
  add_definitions(-DBIG_ENDIAN=0 -DLITTLE_ENDIAN=1 -DBYTE_ORDER=BIG_ENDIAN)
endif()

# Use GNUInstallDirs

include(GNUInstallDirs)
set(LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR} CACHE STRING "Library directory name" FORCE)
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE STRING "Include directory name" FORCE)
set(SHARE_INSTALL_DIR ${CMAKE_INSTALL_DATAROOTDIR} CACHE STRING "Share directory name")

# set the output paths
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
if(WIN32)
  set(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})
  set(LIB_INSTALL_DIR lib)
  set(BIN_INSTALL_DIR bin)
else()
  set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
endif()

if(APPLE)
  set(CMAKE_INSTALL_NAME_DIR ${LIB_DESTINATION})
endif()

option(ICAL_ERRORS_ARE_FATAL "icalerror_* calls will abort instead of internally signaling an error.")
if(ICAL_ERRORS_ARE_FATAL)
  set(ICAL_ERRORS_ARE_FATAL 1)
else()
  set(ICAL_ERRORS_ARE_FATAL 0)
endif()

option(ICAL_ALLOW_EMPTY_PROPERTIES "Prevent empty properties from being replaced with X-LIC-ERROR properties.")
if(ICAL_ALLOW_EMPTY_PROPERTIES)
  set(ICAL_ALLOW_EMPTY_PROPERTIES 1)
else()
  set(ICAL_ALLOW_EMPTY_PROPERTIES 0)
endif()

option(NO_WARN_DEPRECATED "DO NOT print deprecated messages." True)
if(NO_WARN_DEPRECATED)
  set(NO_WARN_DEPRECATED 1)
else()
  set(NO_WARN_DEPRECATED 0)
endif()

option(USE_BUILTIN_TZDATA "build using our own timezone data, else use the system timezone data on non-Windows systems. ALWAYS true on Windows.")
if(USE_BUILTIN_TZDATA)
  set(USE_BUILTIN_TZDATA 1)
else()
  set(USE_BUILTIN_TZDATA 0)
endif()
if(WIN32 OR WINCE)
  #Always use builtin tzdata on Windows systems.
  if(NOT USE_BUILTIN_TZDATA)
    message(STATUS "Currently unable to use system tzdata on Windows. Falling back to builtin tzdata")
    set(USE_BUILTIN_TZDATA 1)
  endif()
endif()

option(USE_INTEROPERABLE_VTIMEZONES "use interoperable rather than exact vtimezones." False)
if(USE_INTEROPERABLE_VTIMEZONES)
  set(USE_INTEROPERABLE_VTIMEZONES 1)
else()
  set(USE_INTEROPERABLE_VTIMEZONES 0)
endif()

include(ConfigureChecks.cmake)
add_definitions(-DHAVE_CONFIG_H)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(INSTALL_TARGETS_DEFAULT_ARGS
  RUNTIME DESTINATION ${BIN_INSTALL_DIR}
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

set(MIN_GOBJECT_INTROSPECTION "0.6.7")
option(GOBJECT_INTROSPECTION "Build GObject introspection \"typelib\" files. Requires GObject Introspection development package ${MIN_GOBJECT_INTROSPECTION} or higher.")
if(GOBJECT_INTROSPECTION)
  find_package(GObjectIntrospection ${MIN_GOBJECT_INTROSPECTION})
  if(INTROSPECTION_FOUND)
    set(HAVE_INTROSPECTION TRUE)
  else()
    message(FATAL_ERROR "You requested to build with GObject Introspection but the necessary development package is missing or too low a version (version ${MIN_GOBJECT_INTROSPECTION} or higher is required)")
  endif()
endif()

#
# Compiler settings
#
if(CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  include(CheckCCompilerFlag)
  check_c_compiler_flag(-Wunused-but-set-variable HAVE_GCC_UNUSED_BUT_SET)
  check_c_compiler_flag(-Wlogical-op HAVE_GCC_LOGICAL_OP)
  check_c_compiler_flag(-Wsizeof-pointer-memaccess HAVE_GCC_POINTER_MEMACCESS)
  check_c_compiler_flag(-Wformat-security HAVE_GCC_FORMAT_SECURITY)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -Wno-deprecated -Wall -Wextra -Winit-self -Wmissing-include-dirs -Wunused -Wno-div-by-zero -Wundef -Wpointer-arith -Werror=return-type")
  if(HAVE_GCC_UNUSED_BUT_SET)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wunused-but-set-variable")
  endif()
  if(HAVE_GCC_LOGICAL_OP)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wlogical-op")
  endif()
  if(HAVE_GCC_POINTER_MEMACCESS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsizeof-pointer-memaccess")
  endif()
  if(HAVE_GCC_FORMAT_SECURITY)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat -Wformat-security")
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME STREQUAL GNU)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE -D_GNU_SOURCE")
  endif()
endif()
if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Qunused-arguments")
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-Wunused-but-set-variable HAVE_GXX_UNUSED_BUT_SET)
  check_cxx_compiler_flag(-Wlogical-op HAVE_GXX_LOGICAL_OP)
  check_cxx_compiler_flag(-Wsizeof-pointer-memaccess HAVE_GXX_POINTER_MEMACCESS)
  check_cxx_compiler_flag(-Wreorder HAVE_GXX_REORDER)
  check_cxx_compiler_flag(-Wformat-security HAVE_GXX_FORMAT_SECURITY)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -Weffc++ -Wno-deprecated -Wall -Wextra -Woverloaded-virtual -Winit-self -Wmissing-include-dirs -Wunused -Wno-div-by-zero -Wundef -Wpointer-arith -Werror=return-type")
  if(HAVE_GXX_UNUSED_BUT_SET)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-but-set-variable")
  endif()
  if(HAVE_GXX_LOGICAL_OP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op")
  endif()
  if(HAVE_GXX_POINTER_MEMACCESS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsizeof-pointer-memaccess")
  endif()
  if(HAVE_GXX_REORDER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wreorder")
  endif()
  if(HAVE_GXX_FORMAT_SECURITY)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat -Wformat-security")
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE -D_GNU_SOURCE")
  endif()
endif()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
endif()

option(WITH_CXX_BINDINGS "Build the C++ bindings." True)
if(WITH_CXX_BINDINGS)
  add_definitions(-DWITH_CXX_BINDINGS)
endif()

#some test programs need to know if we are using 32-bit time
if(SIZEOF_TIME_T EQUAL 4)
  set(USE_32BIT_TIME_T TRUE)
endif()
if(WIN32 AND MSVC)
  if(SIZEOF_SIZE_T LESS SIZEOF_TIME_T)
    unset(SIZEOF_SIZE_T CACHE)
    unset(SIZEOF_TIME_T CACHE)
    message(FATAL_ERROR
      "You are using a Microsoft Visual 32-bit compiler with 64-bit time."
      "Run cmake again with the \"-DUSE_32BIT_TIME_T=true\" command line option."
    )
  endif()
endif()

################# build subdirs ########################

add_subdirectory(design-data)
add_subdirectory(doc)
add_subdirectory(scripts)
add_subdirectory(test-data)
add_subdirectory(src)
add_subdirectory(examples)
if(USE_BUILTIN_TZDATA)
  # use our zoneinfo if cmake is passed -DUSE_BUILTIN_TZDATA
  add_subdirectory(zoneinfo)
endif()

########### create and install pkg-config file #########

set(VERSION "${PROJECT_VERSION}")
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib")
set(includedir "\${prefix}/include")
set(PTHREAD_LIBS "${CMAKE_THREAD_LIBS_INIT}")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/libical.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/libical.pc
  @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libical.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

########### Create and install the CMake Config files ##########
include(CMakePackageConfigHelpers)

configure_package_config_file(
  LibIcalConfig.cmake.in ${libical_BINARY_DIR}/LibIcalConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/LibIcal
  PATH_VARS LIB_INSTALL_DIR INCLUDE_INSTALL_DIR
)

# Create a version file
write_basic_package_version_file(
  ${libical_BINARY_DIR}/LibIcalConfigVersion.cmake
  VERSION ${LIBICAL_LIB_VERSION_STRING}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES ${libical_BINARY_DIR}/LibIcalConfigVersion.cmake ${libical_BINARY_DIR}/LibIcalConfig.cmake
  DESTINATION ${LIB_INSTALL_DIR}/cmake/LibIcal
)

install(
  EXPORT icalTargets
  DESTINATION ${LIB_INSTALL_DIR}/cmake/LibIcal
  FILE LibIcalTargets.cmake
)
