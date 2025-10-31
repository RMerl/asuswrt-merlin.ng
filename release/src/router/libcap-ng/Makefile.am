# Makefile.am -- 
# Copyright 2009 Red Hat Inc.
# All Rights Reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING. If not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
# Boston, MA 02110-1335, USA.
#
# Authors:
#   Steve Grubb <sgrubb@redhat.com>
#
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src utils m4 docs
if HAVE_SWIG
SUBDIRS += bindings
endif
EXTRA_DIST = ChangeLog AUTHORS README.md INSTALL COPYING COPYING.LIB \
	libcap-ng.spec autogen.sh 

CONFIG_CLEAN_FILES = debug*.list config/*
clean-generic:
	rm -rf autom4te*.cache 
	rm -f *.rej *.orig *.lang
