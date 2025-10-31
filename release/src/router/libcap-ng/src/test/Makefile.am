# Makefile.am --
# Copyright 2009,10,15 Red Hat Inc.
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

AM_CPPFLAGS = -I${top_srcdir} -I${top_srcdir}/src
check_PROGRAMS = lib_test thread_test
TESTS = $(check_PROGRAMS)

lib_test_LDADD = ${top_builddir}/src/libcap-ng.la
thread_test_LDADD = ${top_builddir}/src/libcap-ng.la -lpthread
