#[=======================================================================[.rst:
FindLibidn2
-----------

Find the Libidn2 library

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``Libidn2::Libidn2``
  The Libidn2 library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``Libidn2_FOUND``
  If false, do not try to use Libidn2.
``LIBIDN2_INCLUDE_DIR``
  where to find libidn2 headers.
``LIBIDN2_LIBRARIES``
  the libraries needed to use Libidn2.
``LIBIDN2_VERSION``
  the version of the Libidn2 library found

#]=======================================================================]

find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(PkgLibIdn2 IMPORTED_TARGET GLOBAL libidn2)
endif ()

if (PkgLibIdn2_FOUND)
  set(LIBIDN2_INCLUDE_DIR ${PkgLibIdn2_INCLUDE_DIRS} CACHE FILEPATH "libidn2 include path")
  set(LIBIDN2_LIBRARIES ${PkgLibIdn2_LIBRARIES} CACHE STRING "libidn2 libraries")
  set(LIBIDN2_VERSION ${PkgLibIdn2_VERSION})
  add_library(Libidn2::Libidn2 ALIAS PkgConfig::PkgLibIdn2)
  set(Libidn2_FOUND ON)
else ()
  find_path(LIBIDN2_INCLUDE_DIR idn2.h
    HINTS
      "${LIBIDN2_DIR}"
      "${LIBIDN2_DIR}/include"
  )

  find_library(LIBIDN2_LIBRARIES NAMES idn2 libidn2
    HINTS
      "${LIBIDN2_DIR}"
      "${LIBIDN2_DIR}/lib"
  )

  if (LIBIDN2_INCLUDE_DIR AND LIBIDN2_LIBRARIES)
    if (NOT TARGET Libidn2::Libidn2)
      add_library(Libidn2::Libidn2 UNKNOWN IMPORTED)
      set_target_properties(Libidn2::Libidn2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${LIBIDN2_INCLUDE_DIR}"
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${LIBIDN2_LIBRARIES}"
      )
    endif ()

    if (NOT LIBIDN2_VERSION AND LIBIDN2_INCLUDE_DIR AND EXISTS "${LIBIDN2_INCLUDE_DIR}/idn2.h")
      file(STRINGS "${LIBIDN2_INCLUDE_DIR}/idn2.h" LIBIDN2_H REGEX "^[ \t]*#[ \t]*define[ \t]+IDN2_VERSION[ \t]")
      string(REGEX REPLACE "^.*IDN2_VERSION[ \t]+\"([0-9.]+)\".*$" "\\1" LIBIDN2_VERSION "${LIBIDN2_H}")
    endif ()
  endif ()
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Libidn2
    REQUIRED_VARS LIBIDN2_LIBRARIES LIBIDN2_INCLUDE_DIR
    VERSION_VAR LIBIDN2_VERSION
  )
endif ()

mark_as_advanced(LIBIDN2_INCLUDE_DIR LIBIDN2_LIBRARIES)
