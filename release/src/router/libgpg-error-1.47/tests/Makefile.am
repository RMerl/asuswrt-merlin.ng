# Makefile.am for libgpg-error/tests.
# Copyright (C) 2003 g10 Code GmbH
#
# This file is part of libgpg-error.
#
# libgpg-error is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
#
# libgpg-error is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

## Process this file with automake to produce Makefile.in

EXTRA_DIST = t-argparse.conf etc/t-argparse.conf

gpg_error_lib = ../src/libgpg-error.la

TESTS = t-version t-strerror t-syserror t-lock t-printf t-poll t-b64 \
	t-argparse t-logging t-stringutils t-malloc

if HAVE_LOCK_OPTIMIZATION
TESTS += t-lock-single-posix
endif

AM_CPPFLAGS = -I$(top_builddir)/src

AM_LDFLAGS = -no-install
LDADD = $(gpg_error_lib) @LDADD_FOR_TESTS_KLUDGE@

noinst_PROGRAMS = $(TESTS)
noinst_HEADERS = t-common.h

t_lock_LDADD = $(LDADD) $(LIBMULTITHREAD)
t_poll_LDADD = $(LDADD) $(LIBMULTITHREAD)
