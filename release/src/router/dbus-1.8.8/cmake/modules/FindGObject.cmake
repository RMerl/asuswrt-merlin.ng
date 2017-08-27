# - Try to find GObject
# Once done this will define
#
#  GOBJECT_FOUND - system has GObject
#  GOBJECT_INCLUDE_DIR - the GObject include directory
#  GOBJECT_LIBRARIES - the libraries needed to use GObject
#  GOBJECT_DEFINITIONS - Compiler switches required for using GObject

# Copyright (c) 2011, Raphael Kubo da Costa <kubito@gmail.com>
# Copyright (c) 2006, Tim Beaulen <tbscope@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_GOBJECT gobject-2.0)
SET(GOBJECT_DEFINITIONS ${PC_GOBJECT_CFLAGS_OTHER})

FIND_PATH(GOBJECT_INCLUDE_DIR gobject.h
   HINTS
   ${PC_GOBJECT_INCLUDEDIR}
   ${PC_GOBJECT_INCLUDE_DIRS}
   PATH_SUFFIXES glib-2.0/gobject/
   )

FIND_LIBRARY(_GObjectLibs NAMES gobject-2.0
   HINTS
   ${PC_GOBJECT_LIBDIR}
   ${PC_GOBJECT_LIBRARY_DIRS}
   )
FIND_LIBRARY(_GModuleLibs NAMES gmodule-2.0
   HINTS
   ${PC_GOBJECT_LIBDIR}
   ${PC_GOBJECT_LIBRARY_DIRS}
   )
FIND_LIBRARY(_GThreadLibs NAMES gthread-2.0
   HINTS
   ${PC_GOBJECT_LIBDIR}
   ${PC_GOBJECT_LIBRARY_DIRS}
   )
FIND_LIBRARY(_GLibs NAMES glib-2.0
   HINTS
   ${PC_GOBJECT_LIBDIR}
   ${PC_GOBJECT_LIBRARY_DIRS}
   )

SET( GOBJECT_LIBRARIES ${_GObjectLibs} ${_GModuleLibs} ${_GThreadLibs} ${_GLibs} )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GOBJECT DEFAULT_MSG GOBJECT_LIBRARIES GOBJECT_INCLUDE_DIR)

MARK_AS_ADVANCED(GOBJECT_INCLUDE_DIR _GObjectLibs _GModuleLibs _GThreadLibs _GLibs)
