# Makefile.am -- 
# Copyright 2009,2014-17 Red Hat Inc.
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
# along with this program; see the file COPYING. If not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
# Boston, MA 02110-1335, USA.
# 
# Authors:
#   Steve Grubb <sgrubb@redhat.com>
#
SUBDIRS = test
CONFIG_CLEAN_FILES = *.loT *.rej *.orig
AM_CFLAGS = -fPIC -DPIC $(PYTHON3_CFLAGS)
AM_CPPFLAGS = -I. -I$(top_builddir) $(PYTHON3_INCLUDES)
LIBS = ${top_builddir}/src/libcap-ng.la
SWIG_FLAGS = -python
SWIG_INCLUDES = ${AM_CPPFLAGS}
PYTHON = $(PYTHON3)
py3exec_PYTHON = capng.py
py3exec_LTLIBRARIES = _capng.la
py3exec_SOLIBRARIES = _capng.so
_capng_la_CFLAGS = -shared
_capng_la_LDFLAGS = -module -avoid-version -Wl,-z,relro
_capng_la_HEADERS: $(top_builddir)/config.h 
_capng_la_DEPENDENCIES =${top_srcdir}/src/cap-ng.h ${top_builddir}/src/libcap-ng.la
_capng_la_LIBADD = ${top_builddir}/src/libcap-ng.la
nodist__capng_la_SOURCES  = capng_wrap.c

capng.py capng_wrap.c: ${srcdir}/../src/capng_swig.i caps.h capng.h
	swig -o capng_wrap.c ${SWIG_FLAGS} ${SWIG_INCLUDES} ${srcdir}/../src/capng_swig.i
caps.h:
	cat $(CAPABILITY_HEADER) | grep '^#define CAP'  | grep -v '[()]' > caps.h
capng.h:
	cat ${top_srcdir}/src/cap-ng.h | grep -v '_state' > capng.h

CLEANFILES = capng.py* capng_wrap.c caps.h capng.h *~
clean-local:
	$(RM) -rf __pycache__
