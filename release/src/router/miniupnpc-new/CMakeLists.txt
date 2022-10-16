cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project (miniupnpc C)

set (MINIUPNPC_VERSION 2.2.2)
set (MINIUPNPC_API_VERSION 17)

option (UPNPC_BUILD_STATIC "Build static library" TRUE)
option (UPNPC_BUILD_SHARED "Build shared library" TRUE)
option (UPNPC_BUILD_TESTS "Build test executables" TRUE)
option (UPNPC_BUILD_SAMPLE "Build sample executables" TRUE)
option (NO_GETADDRINFO "Define NO_GETADDRINFO" FALSE)
option (UPNPC_NO_INSTALL "Disable installation" FALSE)

if (NOT UPNPC_BUILD_STATIC AND NOT UPNPC_BUILD_SHARED)
    message (FATAL "Both shared and static libraries are disabled!")
endif ()

include(GNUInstallDirs)

# Interface library for compile definitions, flags and option
add_library(miniupnpc-private INTERFACE)

if (NO_GETADDRINFO)
  target_compile_definitions(miniupnpc-private INTERFACE NO_GETADDRINFO)
endif ()

if (NOT WIN32)
  target_compile_definitions(miniupnpc-private INTERFACE
    MINIUPNPC_SET_SOCKET_TIMEOUT
    _BSD_SOURCE _DEFAULT_SOURCE)
  if (NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" AND NOT CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    # add_definitions (-D_POSIX_C_SOURCE=200112L)
    target_compile_definitions(miniupnpc-private INTERFACE _XOPEN_SOURCE=600)
  endif ()
else ()
  target_compile_definitions(miniupnpc-private INTERFACE _WIN32_WINNT=0x0501) # XP or higher for getnameinfo and friends
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_compile_definitions(miniupnpc-private INTERFACE _DARWIN_C_SOURCE)
endif ()

# Set compiler specific build flags
if (CMAKE_COMPILER_IS_GNUCC AND NOT CMAKE_SYSTEM_NAME STREQUAL "AmigaOS")
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  target_compile_options(miniupnpc-private INTERFACE -Wall)
endif ()

# Suppress noise warnings
if (MSVC)
  target_compile_definitions(miniupnpc-private INTERFACE _CRT_SECURE_NO_WARNINGS)
endif()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/miniupnpcstrings.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/miniupnpcstrings.h)
target_include_directories(miniupnpc-private INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

set (MINIUPNPC_SOURCES
  igd_desc_parse.c
  miniupnpc.c
  minixml.c
  minisoap.c
  minissdpc.c
  miniwget.c
  upnpcommands.c
  upnpdev.c
  upnpreplyparse.c
  upnperrors.c
  connecthostport.c
  portlistingparse.c
  receivedata.c
  listdevices.c
	addr_is_reserved.c
  connecthostport.h
  igd_desc_parse.h
  minisoap.h
  minissdpc.h
  miniupnpc.h
  miniupnpctypes.h
  miniwget.h
  minixml.h
  portlistingparse.h
  receivedata.h
  upnpcommands.h
  upnpdev.h
  upnperrors.h
  upnpreplyparse.h
	addr_is_reserved.h
  ${CMAKE_CURRENT_BINARY_DIR}/miniupnpcstrings.h
)

if (NOT WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "AmigaOS")
  set (MINIUPNPC_SOURCES ${MINIUPNPC_SOURCES} minissdpc.c)
endif ()

if (WIN32)
  target_link_libraries(miniupnpc-private INTERFACE ws2_32 iphlpapi)
elseif (CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  target_link_libraries(miniupnpc-private INTERFACE socket nsl resolv)
  find_library (SOCKET_LIBRARY NAMES socket)
  find_library (NSL_LIBRARY NAMES nsl)
  find_library (RESOLV_LIBRARY NAMES resolv)
  set (LDLIBS ${SOCKET_LIBRARY} ${NSL_LIBRARY} ${RESOLV_LIBRARY} ${LDLIBS})
elseif (HAIKU)
  target_link_libraries(miniupnpc-private INTERFACE network)
  find_library (SOCKET_LIBRARY NAMES network)
  find_library (NSL_LIBRARY NAMES network)
  find_library (RESOLV_LIBRARY NAMES network)
  set (LDLIBS ${SOCKET_LIBRARY} ${NSL_LIBRARY} ${RESOLV_LIBRARY} ${LDLIBS})  
endif ()


if (UPNPC_BUILD_STATIC)
  add_library (libminiupnpc-static STATIC ${MINIUPNPC_SOURCES})
  if (NOT UPNPC_BUILD_SHARED)
    add_library (miniupnpc::miniupnpc ALIAS libminiupnpc-static)
  endif()
  set_target_properties (libminiupnpc-static PROPERTIES EXPORT_NAME miniupnpc)
  if (WIN32)
    set_target_properties (libminiupnpc-static PROPERTIES OUTPUT_NAME "libminiupnpc")
  else()
    set_target_properties (libminiupnpc-static PROPERTIES OUTPUT_NAME "miniupnpc")
  endif()
  target_link_libraries (libminiupnpc-static PRIVATE miniupnpc-private)
  target_include_directories(libminiupnpc-static INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  target_compile_definitions(libminiupnpc-static PUBLIC MINIUPNP_STATICLIB)

  if (NOT UPNPC_NO_INSTALL)
    install (TARGETS miniupnpc-private EXPORT miniupnpc-private)

    install (EXPORT miniupnpc-private
      DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/miniupnpc"
      NAMESPACE miniupnpc::)

    install (TARGETS libminiupnpc-static
      EXPORT libminiupnpc-static
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

    install (EXPORT libminiupnpc-static
      DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/miniupnpc"
      NAMESPACE miniupnpc::)
  endif()

  if (UPNPC_BUILD_SAMPLE)
    add_executable (upnpc-static upnpc.c)
    target_link_libraries (upnpc-static PRIVATE libminiupnpc-static)
    target_include_directories(upnpc-static PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  endif ()
endif ()

if (UPNPC_BUILD_SHARED)
  add_library (libminiupnpc-shared SHARED ${MINIUPNPC_SOURCES})
  add_library (miniupnpc::miniupnpc ALIAS libminiupnpc-shared)
  set_target_properties (libminiupnpc-shared PROPERTIES EXPORT_NAME miniupnpc)
  set_target_properties (libminiupnpc-shared PROPERTIES OUTPUT_NAME "miniupnpc")
  set_target_properties (libminiupnpc-shared PROPERTIES VERSION ${MINIUPNPC_VERSION})
  set_target_properties (libminiupnpc-shared PROPERTIES SOVERSION ${MINIUPNPC_API_VERSION})
  target_link_libraries (libminiupnpc-shared PRIVATE miniupnpc-private)
  target_compile_definitions(libminiupnpc-shared PRIVATE MINIUPNP_EXPORTS)

  target_include_directories(libminiupnpc-shared INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  if (WIN32)
    target_link_libraries(libminiupnpc-shared INTERFACE ws2_32 iphlpapi)
  endif()

  if (NOT UPNPC_NO_INSTALL)
    install (TARGETS libminiupnpc-shared
      EXPORT libminiupnpc-shared
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

    install (EXPORT libminiupnpc-shared
      DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/miniupnpc"
      NAMESPACE miniupnpc::)
  endif()

  if (UPNPC_BUILD_SAMPLE)
    add_executable (upnpc-shared upnpc.c)
    target_link_libraries (upnpc-shared PRIVATE libminiupnpc-shared)
    target_include_directories(upnpc-shared PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  endif ()

  add_executable (listdevices listdevices.c)
  target_link_libraries (listdevices PRIVATE libminiupnpc-shared)
  target_include_directories(listdevices PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif ()

if (UPNPC_BUILD_TESTS)
  add_library(miniupnpc-tests INTERFACE)
  target_link_libraries(miniupnpc-tests INTERFACE miniupnpc-private)
  target_compile_definitions(miniupnpc-tests INTERFACE MINIUPNP_STATICLIB)

  add_executable (testminixml testminixml.c minixml.c igd_desc_parse.c)
  target_link_libraries (testminixml PRIVATE miniupnpc-tests)

  add_executable (minixmlvalid minixmlvalid.c minixml.c)
  target_link_libraries (minixmlvalid PRIVATE miniupnpc-tests)

  add_executable (testupnpreplyparse testupnpreplyparse.c
                                     minixml.c upnpreplyparse.c)
  target_link_libraries (testupnpreplyparse PRIVATE miniupnpc-tests)

  add_executable (testigddescparse testigddescparse.c
                                   igd_desc_parse.c minixml.c miniupnpc.c miniwget.c minissdpc.c
                                   upnpcommands.c upnpreplyparse.c minisoap.c connecthostport.c
                                   portlistingparse.c receivedata.c addr_is_reserved.c
  )
  target_link_libraries (testigddescparse PRIVATE miniupnpc-tests)

  add_executable (testminiwget testminiwget.c
                               miniwget.c miniupnpc.c minisoap.c upnpcommands.c minissdpc.c
                               upnpreplyparse.c minixml.c igd_desc_parse.c connecthostport.c
                               portlistingparse.c receivedata.c addr_is_reserved.c
  )
  target_link_libraries (testminiwget PRIVATE miniupnpc-tests)

  add_executable (testaddr_is_reserved testaddr_is_reserved.c
	                                     addr_is_reserved.c
	)
  target_link_libraries (testaddr_is_reserved PRIVATE miniupnpc-tests)

# set (UPNPC_INSTALL_TARGETS ${UPNPC_INSTALL_TARGETS} testminixml minixmlvalid testupnpreplyparse testigddescparse testminiwget)
endif ()

if (NOT UPNPC_NO_INSTALL)
  install (FILES
	  miniupnpc.h
    miniwget.h
    upnpcommands.h
    igd_desc_parse.h
    upnpreplyparse.h
    upnperrors.h
    upnpdev.h
    miniupnpctypes.h
    portlistingparse.h
    miniupnpc_declspec.h
    DESTINATION include/miniupnpc
  )

  install(FILES miniupnpc-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/miniupnpc
  )
endif()

# vim: ts=2:sw=2
