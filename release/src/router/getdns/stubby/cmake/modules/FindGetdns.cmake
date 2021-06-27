#[=======================================================================[.rst:
FindGetdns
----------

Find the Getdns library

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``Getdns::Getdns``
  The Getdns library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``Getdns_FOUND``
  If false, do not try to use Getdns.
``GETDNS_INCLUDE_DIR``
  where to find getdns/getdns.h, etc.
``GETDNS_LIBRARIES``
  the libraries needed to use Getdns.
``GETDNS_VERSION``
  the version of the Getdns library found

Hints
^^^^^

``GETDNS_DIR``
  look here for a getdns install tree (include/ and lib/)
``GETDNS_STATIC``
  include common static library dependents, on Windows find the static library.

#]=======================================================================]

find_path(GETDNS_INCLUDE_DIR getdns/getdns.h
  HINTS
  "${GETDNS_DIR}"
  "${GETDNS_DIR}/include"
  )

if (GETDNS_STATIC AND (WIN32 OR MINGW OR MSYS OR CYGWIN))
  find_library(GETDNS_LIBRARY NAMES getdns_static
    HINTS
    "${GETDNS_DIR}"
    "${GETDNS_DIR}/lib"
    )
else()
  find_library(GETDNS_LIBRARY NAMES getdns libgetdns
    HINTS
    "${GETDNS_DIR}"
    "${GETDNS_DIR}/lib"
    )
endif()

set(GETDNS_LIBRARIES "")
if (GETDNS_STATIC)
  find_package(OpenSSL "1.0.2" REQUIRED)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  list(APPEND GETDNS_LIBRARIES
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    )
  find_package(Libidn2 "2.0.0")
  if (Libidn2_FOUND)
    list(APPEND GETDNS_LIBRARIES Libidn2::Libidn2)
  endif ()
  find_package(Libunbound "1.5.9")
  if (Libunbound_FOUND)
    list(APPEND GETDNS_LIBRARIES Libunbound::Libunbound)
  endif ()
  if (WIN32 OR MINGW OR MSYS OR CYGWIN)
    list(APPEND GETDNS_LIBRARIES
    "ws2_32"
    "crypt32"
    "gdi32"
    "iphlpapi"
    "psapi"
    "userenv"
    )
  endif ()
endif ()

if (GETDNS_INCLUDE_DIR AND GETDNS_LIBRARY)
  if (NOT TARGET Getdns::Getdns)
    add_library(Getdns::Getdns UNKNOWN IMPORTED)
    set_target_properties(Getdns::Getdns PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${GETDNS_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES "${GETDNS_LIBRARIES}"
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      IMPORTED_LOCATION "${GETDNS_LIBRARY}"
      )
  endif ()

  if (NOT GETDNS_VERSION AND GETDNS_INCLUDE_DIR AND EXISTS "${GETDNS_INCLUDE_DIR}/getdns/getdns.h")
    file(STRINGS "${GETDNS_INCLUDE_DIR}/getdns/getdns_extra.h" GETDNS_H REGEX "^#define GETDNS_VERSION")
    string(REGEX REPLACE "^.*GETDNS_VERSION \"([0-9.]+)[A-Za-z0-9.+-]*\".*$" "\\1" GETDNS_VERSION "${GETDNS_H}")
  endif ()
endif()

list(APPEND GETDNS_LIBRARIES "${GETDNS_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Getdns
  REQUIRED_VARS GETDNS_LIBRARIES GETDNS_INCLUDE_DIR
  VERSION_VAR GETDNS_VERSION
  )

mark_as_advanced(GETDNS_INCLUDE_DIR GETDNS_LIBRARIES GETDNS_LIBRARY)
