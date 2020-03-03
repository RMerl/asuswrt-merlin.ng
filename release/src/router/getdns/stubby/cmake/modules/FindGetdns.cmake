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

#]=======================================================================]

find_path(GETDNS_INCLUDE_DIR getdns/getdns.h
  HINTS
  "${GETDNS_DIR}"
  "${GETDNS_DIR}/include"
  )

find_library(GETDNS_LIBRARY NAMES getdns libgetdns
  HINTS
  "${GETDNS_DIR}"
  "${GETDNS_DIR}/lib"
  )

set(GETDNS_LIBRARIES "")

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
