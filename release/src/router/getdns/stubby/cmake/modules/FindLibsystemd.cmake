#[=======================================================================[.rst:
FindLibsystemd
----------

Find the Libsystemd library

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``Libsystemd::Libsystemd``
  The Libsystemd library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``Libsystemd_FOUND``
  If false, do not try to use Libsystemd.
``LIBSYSTEMD_INCLUDE_DIR``
  where to find systemd/sd-daemon.h, etc.
``LIBSYSTEMD_LIBRARIES``
  the libraries needed to use Libsystemd.

#]=======================================================================]

find_path(LIBSYSTEMD_INCLUDE_DIR systemd/sd-daemon.h
  HINTS
  "${LIBSYSTEMD_DIR}"
  "${LIBSYSTEMD_DIR}/include"
  )

find_library(LIBSYSTEMD_LIBRARY NAMES systemd libsystemd
  HINTS
  "${LIBSYSTEMD_DIR}"
  "${LIBSYSTEMD_DIR}/lib"
  )

set(LIBSYSTEMD_LIBRARIES "")

if (LIBSYSTEMD_INCLUDE_DIR AND LIBSYSTEMD_LIBRARY)
  if (NOT TARGET Libsystemd::Libsystemd)
    add_library(Libsystemd::Libsystemd UNKNOWN IMPORTED)
    set_target_properties(Libsystemd::Libsystemd PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LIBSYSTEMD_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES "${LIBSYSTEMD_LIBRARIES}"
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      IMPORTED_LOCATION "${LIBSYSTEMD_LIBRARY}"
      )
  endif ()
endif ()

list(APPEND LIBSYSTEMD_LIBRARIES "${LIBSYSTEMD_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libsystemd
  REQUIRED_VARS LIBSYSTEMD_LIBRARIES LIBSYSTEMD_INCLUDE_DIR
  )

mark_as_advanced(LIBSYSTEMD_INCLUDE_DIR LIBSYSTEMD_LIBRARIES LIBSYSTEMD_LIBRARY)
