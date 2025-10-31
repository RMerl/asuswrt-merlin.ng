# Makefile.am -- 
# Copyright 2009,2015,2020 Red Hat Inc.
# All Rights Reserved.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with this program; see the file COPYING.LIB. If not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
# Boston, MA 02110-1335, USA.
# 
# Authors:
#   Steve Grubb <sgrubb@redhat.com>
#

SUBDIRS = test
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libcap-ng.pc
DISTCLEANFILES = $(pkgconfig_DATA)
CLEANFILES = $(BUILT_SOURCES)
CONFIG_CLEAN_FILES = *.loT *.rej *.orig
AM_CFLAGS = -fPIC -DPIC -W -Wall -Wshadow ${WFLAGS} -Wundef -D_GNU_SOURCE
AM_CPPFLAGS = -I. -I${top_srcdir}
lib_LTLIBRARIES = libcap-ng.la
if BUILD_LIBDROP_AMBIENT
  lib_LTLIBRARIES += libdrop_ambient.la
endif
include_HEADERS = cap-ng.h
noinst_HEADERS = captab.h
libcap_ng_la_SOURCES = cap-ng.c lookup_table.c
libcap_ng_la_LIBADD =
libcap_ng_la_DEPENDENCIES = $(libcap_ng_la_SOURCES) ../config.h
libcap_ng_la_LDFLAGS = -Wl,-z,relro
libdrop_ambient_la_SOURCES = libdrop_ambient.c

