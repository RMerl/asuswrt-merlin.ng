include(FetchContent)
include(FindPkgConfig)

set (TARGET_NAME ${PROJECT_NAME})

option (BUILD_SHARED_LIBS "Build libjwt as shared library instead as static one." OFF)
option (WITHOUT_OPENSSL "Use GnuTLS for encryption instead of OpenSSL" OFF)
option (USE_WINSSL "Use Windows crypto API for encryption instead of OpenSSL" OFF)
option (USE_INSTALLED_JANSSON "Use pre-installed jansson library" ON)

if (UNIX)
	option (ENABLE_PIC "Use position independent code in static library build." OFF)
endif ()

if (BUILD_SHARED_LIBS)
	set (LIBRARY_TYPE SHARED)
else ()
	set (LIBRARY_TYPE STATIC)
	if (MSVC)
		set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Z7")
		set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /Z7")
		if (${ENABLE_DEBUG_INFO_IN_RELEASE})
			set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Z7")
		endif ()
	endif ()
endif ()

if (USE_WINSSL)
	set (FILES_TO_REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/jwt-openssl.c ${CMAKE_CURRENT_SOURCE_DIR}/jwt-gnutls.c)
	set (SSL_LIBRARY_INCLUDE_DIR )
	set (SSL_LIBRARIES_DEBUG crypt32.lib ncrypt.lib bcrypt.lib)
	set (SSL_LIBRARIES_OPTIMIZED crypt32.lib ncrypt.lib bcrypt.lib)
elseif (WITHOUT_OPENSSL)
	find_package (GnuTLS REQUIRED)
	set (FILES_TO_REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/jwt-openssl.c ${CMAKE_CURRENT_SOURCE_DIR}/jwt-wincrypt.c)
	set (SSL_LIBRARY_INCLUDE_DIR ${GNUTLS_INCLUDE_DIR})
	set (SSL_LIBRARIES_DEBUG ${GNUTLS_LIBRARY})
	set (SSL_LIBRARIES_OPTIMIZED ${GNUTLS_LIBRARY})
else ()
	if (MSVC AND STATIC_RUNTIME)
		set (OPENSSL_MSVC_STATIC_RT TRUE)
	endif ()
	find_package (OpenSSL REQUIRED)
	set (FILES_TO_REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/jwt-gnutls.c ${CMAKE_CURRENT_SOURCE_DIR}/jwt-wincrypt.c)
	set (SSL_LIBRARY_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR})
	set (SSL_LIBRARIES_DEBUG ${OPENSSL_LIBRARIES})
	set (SSL_LIBRARIES_OPTIMIZED ${OPENSSL_LIBRARIES})
endif ()

if (USE_INSTALLED_JANSSON)
    PKG_SEARCH_MODULE( JANSSON jansson )
else()
    FetchContent_Declare(
    jansson
    GIT_REPOSITORY https://github.com/akheron/jansson.git
    GIT_TAG        v2.14
    )

    FetchContent_MakeAvailable(jansson)
endif()

write_file(${CMAKE_CURRENT_BINARY_DIR}/config.h "")

file (GLOB SOURCE_FILES "../include/*.h" "*.h" "*.c")
list (REMOVE_ITEM SOURCE_FILES ${FILES_TO_REMOVE})

add_library (${TARGET_NAME} ${LIBRARY_TYPE} ${SOURCE_FILES})

target_include_directories (${TARGET_NAME} PRIVATE
	${SSL_LIBRARY_INCLUDE_DIR}
       ${jansson_SOURCE_DIR}/src
       ${jansson_SOURCE_DIR}/android
       ${CMAKE_CURRENT_BINARY_DIR}
	)

# Need for using the project by add_subdirectory
target_include_directories (${TARGET_NAME} INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../include
  )

if (UNIX)
	target_compile_definitions (${TARGET_NAME} PUBLIC _GNU_SOURCE)
endif ()

if (UNIX AND ENABLE_LTO)
	set_property(TARGET ${TARGET_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif ()

if (MSVC)
	target_compile_definitions (${TARGET_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
	if (BUILD_SHARED_LIBS)
		target_compile_definitions (${TARGET_NAME}
			PUBLIC JWT_DLL_CONFIG
			PRIVATE JWT_BUILD_SHARED_LIBRARY
			)
	endif ()
endif ()

if (NOT BUILD_SHARED_LIBS AND ENABLE_PIC)
	target_compile_options (${TARGET_NAME} PRIVATE -fPIC)
endif ()

target_link_libraries (${TARGET_NAME}
	debug ${SSL_LIBRARIES_DEBUG} optimized ${SSL_LIBRARIES_OPTIMIZED}
	jansson
	)

install (TARGETS ${TARGET_NAME}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)

install (FILES
	${PROJECT_SOURCE_DIR}/include/jwt.h
	DESTINATION include/
	)
