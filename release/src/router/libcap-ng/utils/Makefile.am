# Makefile.am --
# Copyright 2009,2015 Red Hat Inc.
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

CONFIG_CLEAN_FILES = *.loT *.rej *.orig
AUTOMAKE_OPTIONS = no-dependencies
EXTRA_DIST = $(man_MANS)
AM_CPPFLAGS = -I${top_srcdir} -I${top_srcdir}/src
LIBS = -L${top_builddir}/src -lcap-ng
AM_CFLAGS = -W -Wall -Wshadow ${WFLAGS} -Wundef -D_GNU_SOURCE
bin_PROGRAMS = pscap netcap filecap captest
man_MANS = pscap.8 netcap.8 filecap.8 captest.8

pscap_SOURCES = pscap.c
netcap_SOURCES = netcap.c proc-llist.c proc-llist.h
filecap_SOURCES = filecap.c
captest_SOURCES = captest.c
