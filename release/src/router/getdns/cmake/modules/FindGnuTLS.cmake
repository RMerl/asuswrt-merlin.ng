#[=======================================================================[.rst:
FindGnuTLS
----------

Find the GnuTLS library.

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``GnuTLS::GnuTLS``
  The GnuTLS library, if found.
``GnuTLS::Dane``
  The GnuTLS DANE library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``GnuTLS_FOUND``
  If false, do not try to use GnuTLS.
``GNUTLS_INCLUDE_DIR``
  where to find GnuTLS headers.
``GNUTLS_LIBRARIES``
  the libraries needed to use GnuTLS.
``GNUTLS_VERSION``
  the version of the GnuTLS library found

#]=======================================================================]

find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(PkgGnuTLS IMPORTED_TARGET GLOBAL QUIET gnutls)
  pkg_check_modules(PkgGnuTLSDane IMPORTED_TARGET GLOBAL QUIET gnutls-dane)
endif ()

if (PkgGnuTLS_FOUND AND PkgGnuTLSDane_FOUND)
  set(GNUTLS_INCLUDE_DIR ${PkgGnuTLS_INCLUDE_DIRS} $PkgGnuTLSDane_INCLUDE_DIRS} CACHE FILEPATH "GnuTLS include path")
  set(NETTLE_LIBRARIES ${PkgGnuTLS_LIBRARIES} ${PkgGnuTLSDane_LIBRARIES} CACHE STRING "GnuTLS libraries")
  set(NETTLE_VERSION ${PkgGnuTLS_VERSION})
  add_library(GnuTLS::GnuTLS ALIAS PkgConfig::PkgGnuTLS)
  add_library(GnuTLS::Dane ALIAS PkgConfig::PkgGnuTLSDane)
  set(GnuTLS_FOUND ON)
else ()
  find_path(GNUTLS_INCLUDE_DIR gnutls/gnutls.h
    HINTS
    "${GNUTLS_DIR}"
    "${GNUTLS_DIR}/include"
  )
  
  find_library(GNUTLS_LIBRARY NAMES gnutls libgnutls
    HINTS
    "${GNUTLS_DIR}"
    "${GNUTLS_DIR}/lib"
  )
  
  find_library(GNUTLS_DANE_LIBRARY NAMES gnutls-dane libgnutls-dane
    HINTS
    "${GNUTLS_DIR}"
    "${GNUTLS_DIR}/lib"
  )
  
  set(_GNUTLS_LIBRARIES "")
  
  if (GNUTLS_INCLUDE_DIR AND GNUTLS_LIBRARY AND GNUTLS_DANE_LIBRARY)
    if (NOT TARGET GnuTLS::GnuTLS)
      add_library(GnuTLS::GnuTLS UNKNOWN IMPORTED)
      set_target_properties(GnuTLS::GnuTLS PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${GNUTLS_INCLUDE_DIR}"
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${GNUTLS_LIBRARY}"
        )
    endif ()
    if (NOT TARGET GnuTLS::Dane)
      add_library(GnuTLS::Dane UNKNOWN IMPORTED)
      set_target_properties(GnuTLS::Dane PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${GNUTLS_INCLUDE_DIR}"
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${GNUTLS_DANE_LIBRARY}"
        )
    endif ()
  
    if (NOT GNUTLS_VERSION AND GNUTLS_INCLUDE_DIR)
      file(STRINGS "${GNUTLS_INCLUDE_DIR}/gnutls/gnutls.h" GNUTLS_VER_H REGEX "^#define GNUTLS_VERSION_(MAJOR|MINOR|PATCH) ")
      string(REGEX REPLACE "^.*_MAJOR ([0-9]+).*_MINOR ([0-9]+).*_PATCH ([0-9]+).*$" "\\1.\\2.\\3c" GNUTLS_VERSION "${GNUTLS_VER_H}")
    endif ()
  endif ()
  
  list(APPEND _GNUTLS_LIBRARIES "${GNUTLS_LIBRARY}" "${GNUTLS_DANE_LIBRARY}")
  set(GNUTLS_LIBRARIES ${_GNUTLS_LIBRARIES} CACHE STRING "GnuTLS libraries")

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(GnuTLS
    REQUIRED_VARS GNUTLS_LIBRARIES GNUTLS_INCLUDE_DIR
    VERSION_VAR GNUTLS_VERSION
    )
endif ()

mark_as_advanced(GNUTLS_INCLUDE_DIR GNUTLS_LIBRARIES GNUTLS_LIBRARY GNUTLS_DANE_LIBRARY)
