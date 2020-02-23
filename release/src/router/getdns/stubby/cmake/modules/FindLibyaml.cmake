#[=======================================================================[.rst:
FindLibyaml
----------

Find the Libyaml library

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``Libyaml::Libyaml``
  The Libyaml library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``Libyaml_FOUND``
  If false, do not try to use Libyaml.
``LIBYAML_INCLUDE_DIR``
  where to find getdns/getdns.h, etc.
``LIBYAML_LIBRARIES``
  the libraries needed to use Libyaml.

#]=======================================================================]

find_path(LIBYAML_INCLUDE_DIR yaml.h
  HINTS
  "${LIBYAML_DIR}"
  "${LIBYAML_DIR}/include"
  )

find_library(LIBYAML_LIBRARY NAMES yaml libyaml
  HINTS
  "${LIBYAML_DIR}"
  "${LIBYAML_DIR}/lib"
  )

set(LIBYAML_LIBRARIES "")

if (LIBYAML_INCLUDE_DIR AND LIBYAML_LIBRARY)
  if (NOT TARGET Libyaml::Libyaml)
    add_library(Libyaml::Libyaml UNKNOWN IMPORTED)
    set_target_properties(Libyaml::Libyaml PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LIBYAML_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES "${LIBYAML_LIBRARIES}"
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      IMPORTED_LOCATION "${LIBYAML_LIBRARY}"
      )
  endif ()
endif ()

list(APPEND LIBYAML_LIBRARIES "${LIBYAML_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libyaml
  REQUIRED_VARS LIBYAML_LIBRARIES LIBYAML_INCLUDE_DIR
  )

mark_as_advanced(LIBYAML_INCLUDE_DIR LIBYAML_LIBRARIES LIBYAML_LIBRARY)
