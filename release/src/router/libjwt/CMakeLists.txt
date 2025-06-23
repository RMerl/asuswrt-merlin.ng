cmake_minimum_required (VERSION 3.7...3.15)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (${CMAKE_VERSION} VERSION_LESS 3.14.7 AND ${CMAKE_GENERATOR} STREQUAL "Visual Studio 16 2019")
	message(FATAL_ERROR "Please update your version of CMake to version 3.14.7 or greater in order to use Visual Studio 2019")
endif()

if (MSVC)
	set (CMAKE_SYSTEM_VERSION 8.1 CACHE TYPE INTERNAL FORCE)
endif ()

project (jwt)

string(REPLACE "\\" "/" CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

option (ENABLE_DEBUG_INFO_IN_RELEASE "Enable adding debug info to release build." OFF)
option (ENABLE_LTO "Enable link time optimizations." OFF)
option (BUILD_EXAMPLES "Build example projects." ON)

if (MSVC)
	option (STATIC_RUNTIME "Link runtome library statically." OFF)
endif ()

if (UNIX)
	option (BUILD_TESTS "Build test projects." OFF)
endif ()

if (APPLE)
	add_definitions(-DNO_WEAK_ALIASES)
endif ()
include_directories (include)

if (MSVC)
	set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Ox")
endif ()

if (STATIC_RUNTIME)
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
	set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
	set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MT")
	set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MT")
endif ()

if (ENABLE_DEBUG_INFO_IN_RELEASE)
	if (MSVC)
		set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
		set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
		set (CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
	else ()
		set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g")
	endif ()
endif ()

if (ENABLE_LTO)
	if (MSVC)
		set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /GL")
		set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /GL")
		set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /GL")

		set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
		set (CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} /LTCG")
		set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")

		set (CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
		set (CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL} /LTCG")
		set (CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")
	endif ()
endif ()

add_subdirectory (libjwt)

if (${BUILD_EXAMPLES})
	add_subdirectory (examples)
endif()

if (${BUILD_TESTS})
	add_subdirectory (tests)
endif ()


# extract version from configure.ac
FILE(READ "${CMAKE_CURRENT_SOURCE_DIR}/configure.ac" configure_ac_contents)
STRING(REGEX REPLACE ";" "\\\\;" configure_ac_contents "${configure_ac_contents}")
STRING(REGEX REPLACE "\n" ";" configure_ac_contents "${configure_ac_contents}")
foreach(configure_ac_line ${configure_ac_contents})
	if("${configure_ac_line}" MATCHES "^[ ]*AC_INIT")
		string(REGEX MATCH "[0-9]+\.[0-9]+\.[0-9]+" version_string "${configure_ac_line}")
		message(STATUS "version from configure.ac: '${version_string}'")
		break()
	endif()
endforeach()

set(INCLUDE_INSTALL_DIR include/)
set(LIB_INSTALL_DIR lib/)
set(LIBJWT_VERSION ${version_string})
include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/libjwtConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/libjwtConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/libjwt/cmake
  PATH_VARS INCLUDE_INSTALL_DIR)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/libjwtConfigVersion.cmake
  VERSION ${LIBJWT_VERSION}
  COMPATIBILITY SameMajorVersion )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libjwtConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/libjwtConfigVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/libjwt/cmake )
