#  Makefile - Makefile for a Linux module for reading sensor data.
#  Copyright (c) 1998, 1999  Frodo Looijaard <frodol@dds.nl>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301 USA.

# Everything you may want to change is in the top of this file. Usually, you
# can just use the defaults, fortunately.

# You need a full complement of GNU utilities to run this Makefile
# successfully; most notably, you need GNU make, flex (>= 2.5.1)
# and bison.

# Uncomment the second line if you are a developer. This will enable many
# additional warnings at compile-time
#WARN := 0
WARN := 1

# Uncomment the second line if you want to get (loads of) debug information
# at run-time.
# Not recommended, unless you are actually debugging the code
DEBUG := 0
#DEBUG := 1

# Note that all the installation paths below can also be set on the make
# command line (e.g. "make PREFIX=/usr").

# If you want to install at some other place then at from which you will run
# everything, set DESTDIR to the extra prefix.
DESTDIR :=

# This is the prefix that will be used for almost all directories below.
PREFIX := /usr/local

# Your C compiler
CC := gcc

# This is the directory where sensors3.conf will be installed, if no other
# configuration file is found
ETCDIR := /etc

# You should not need to change this. It is the directory into which the
# library files (both static and shared) will be installed.
LIBDIR := $(PREFIX)/lib

EXLDFLAGS := -Wl,-rpath,$(LIBDIR)

# You should not need to change this. It is the directory into which the
# executable program files will be installed. BINDIR for programs that are
# also useful for normal users, SBINDIR for programs that can only be run
# by the superuser.
# Note that not all programs in this package are really installed;
# some are just examples. You can always install them by hand, of
# course.
BINDIR := $(PREFIX)/bin
SBINDIR := $(PREFIX)/sbin

# You should not need to change this. It is the basic directory into which
# include files will be installed. The actual directory will be 
# $(INCLUDEDIR)/sensors for library include files.
INCLUDEDIR := $(PREFIX)/include
LIBINCLUDEDIR := $(INCLUDEDIR)/sensors

# You should not need to change this. It is the base directory under which the
# manual pages will be installed.
MANDIR := $(PREFIX)/man

MACHINE := $(shell uname -m)

# Extra non-default programs to build; e.g., sensord
#PROG_EXTRA := sensord

# Build and install static library
BUILD_STATIC_LIB := 1

# Set these to add preprocessor or compiler flags, or use
# environment variables
# CFLAGS :=
# CPPFLAGS :=

##################################################
# Below this, nothing should need to be changed. #
##################################################

# Note that this is a monolithic Makefile; it calls no sub-Makefiles,
# but instead, it compiles everything right from here. Yes, there are
# some distinct advantages to this; see the following paper for more info:
#   http://www.tip.net.au/~millerp/rmch/recu-make-cons-harm.html
# Note that is still uses Makefile fragments in sub-directories; these
# are called 'Module.mk'.

# Within each Module.mk, rules and dependencies can be added to targets
# all, install and clean. Use double colons instead of single ones
# to do this. 

# The subdirectories we need to build things in 
SRCDIRS := lib prog/detect prog/pwm \
           prog/sensors ${PROG_EXTRA:%=prog/%} etc
# Only build isadump and isaset on x86 machines.
ifneq (,$(findstring $(MACHINE), i386 i486 i586 i686 x86_64))
SRCDIRS += prog/dump
endif
SRCDIRS += lib/test

# Some often-used commands with default options
MKDIR := mkdir -p
RMDIR := rmdir
RM := rm -f
MV := mv -f
BISON := bison
FLEX := flex
AR := ar
INSTALL := install
LN := ln -sf
GREP := grep
AWK := awk
SED := sed

# Determine the default compiler flags
# Set CFLAGS or CPPFLAGS above to add your own flags to all.
# ALLCPPFLAGS/ALLCFLAGS are common flags, plus any user-specified overrides from the environment or make command line.
# PROGCPPFLAGS/PROGCFLAGS is to create regular object files (which are linked into executables).
# ARCPPFLAGS/ARCFLAGS are used to create archive object files (static libraries).
# LIBCPPFLAGS/LIBCFLAGS are for shared library objects.
ALL_CPPFLAGS := -I.
ALL_CFLAGS := -Wall

ifeq ($(DEBUG),1)
ALL_CPPFLAGS += -DDEBUG
ALL_CFLAGS += -O -g
else
ALL_CFLAGS += -O2
endif

ifeq ($(WARN),1)
ALL_CFLAGS += -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
            -Wcast-align -Wwrite-strings -Wnested-externs -Winline -W \
            -Wmissing-prototypes -Wundef
endif

ALL_CPPFLAGS += $(CPPFLAGS)
ALL_CFLAGS += $(CFLAGS)

PROGCPPFLAGS := -DETCDIR="\"$(ETCDIR)\"" $(ALL_CPPFLAGS)
PROGCFLAGS := $(ALL_CFLAGS)
ARCPPFLAGS := -DETCDIR="\"$(ETCDIR)\"" $(ALL_CPPFLAGS)
ARCFLAGS := $(ALL_CFLAGS)
LIBCPPFLAGS := -DETCDIR="\"$(ETCDIR)\"" $(ALL_CPPFLAGS)
LIBCFLAGS := -fpic -D_REENTRANT $(ALL_CFLAGS)

.PHONY: all user clean install user_install uninstall user_uninstall

# Make all the default rule
all::

# Include all makefiles for sub-modules
INCLUDEFILES := 
include $(patsubst %,%/Module.mk,$(SRCDIRS))
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),uninstall)
ifneq ($(MAKECMDGOALS),user_uninstall)
ifneq ($(MAKECMDGOALS),help)
include $(INCLUDEFILES)
endif
endif
endif
endif

# Man pages
MANPAGES := $(LIBMAN3FILES) $(LIBMAN5FILES) $(PROGDETECTMAN8FILES) $(PROGDUMPMAN8FILES) \
            $(PROGSENSORSMAN1FILES) $(PROGPWMMAN8FILES) prog/sensord/sensord.8

user ::
user_install::
	@echo "*** Important notes:"
	@echo "***  * The libsensors configuration file ($(ETCDIR)/sensors3.conf) is never"
	@echo "***    overwritten by our installation process, so that you won't lose"
	@echo "***    your personal settings in that file. You still can get our latest"
	@echo "***    default config file in etc/sensors.conf.default and manually copy"
	@echo "***    it to $(ETCDIR)/sensors3.conf if you want. You will then want to"
	@echo "***    edit it to fit your needs again."
	@echo "***  * The format of $(ETCDIR)/sensors3.conf changed with lm-sensors 3.0.0."
	@echo "***    If you have a custom configuration file using the old format, you"
	@echo "***    can convert it using the sensors-conf-convert script. Otherwise just"
	@echo "***    overwrite your old configuration file with the new default one."
	@echo "***  * As of lm-sensors 3.1.0, the default configuration file only"
	@echo "***    contains statements which do not depend on how chips are wired."
	@echo "***    If you miss parts of the bigger configuration file that used to be"
	@echo "***    the default, copy the relevant parts from etc/sensors.conf.eg to"
	@echo "***    $(ETCDIR)/sensors3.conf."
all :: user
install :: all user_install

clean::
	$(RM) lm_sensors-* lex.backup

user_uninstall::

uninstall :: user_uninstall

help:
	@echo 'Make targets are:'
	@echo '  all (default): build library and userspace programs'
	@echo '  install: install library and userspace programs'
	@echo '  uninstall: uninstall library and userspace programs'
	@echo '  clean: cleanup'

# Generate html man pages to be copied to the lm_sensors website.
# This uses the man2html from here
# http://ftp.math.utah.edu/pub/sgml/
# which works directly from the nroff source
manhtml:
	$(MKDIR) html
	cp $(MANPAGES) html
	cd html ; \
	export LOGNAME=sensors ; \
	export HOSTNAME=www.lm-sensors.org ; \
	man2html *.[1-8] ; \
	$(RM) *.[1-8]

# Here, we define all implicit rules we want to use.

.SUFFIXES:

# We need to create dependency files. Tricky. The sed rule puts dir/file.d and
# dir/file.c in front of the dependency file rule.


# .ro files are used for programs (as opposed to modules)
%.ro: %.c
	$(CC) $(PROGCPPFLAGS) $(PROGCFLAGS) -c $< -o $@

%.rd: %.c
	$(CC) -M -MG $(PROGCPPFLAGS) $(PROGCFLAGS) $< | \
	$(SED) -e 's@^\(.*\)\.o:@$*.rd $*.ro: Makefile '`dirname $*.rd`/Module.mk' @' > $@


# .ao files are used for static archives
%.ao: %.c
	$(CC) $(ARCPPFLAGS) $(ARCFLAGS) -c $< -o $@

%.ad: %.c
	$(CC) -M -MG $(ARCPPFLAGS) $(ARCFLAGS) $< | \
	$(SED) -e 's@^\(.*\)\.o:@$*.ad $*.ao: Makefile '`dirname $*.ad`/Module.mk' @' > $@


# .lo files are used for shared libraries
%.lo: %.c
	$(CC) $(LIBCPPFLAGS) $(LIBCFLAGS) -c $< -o $@

%.ld: %.c
	$(CC) -M -MG $(LIBCPPFLAGS) $(LIBCFLAGS) $< | \
	$(SED) -e 's@^\(.*\)\.o:@$*.ld $*.lo: Makefile '`dirname $*.ld`/Module.mk' @' > $@


# Flex and Bison
%.c: %.y
	@if ! which $(BISON) 2> /dev/null ; then \
		echo "Please install $(BISON), then run \"make clean\" and try again" ; \
		false ; \
	fi
	$(BISON) -p sensors_yy -d $< -o $@

ifeq ($(DEBUG),1)
FLEX_FLAGS := -Psensors_yy -t -b -Cfe -8
else
FLEX_FLAGS := -Psensors_yy -t -Cfe -8
endif

%.c: %.l
	@if ! which $(FLEX) 2> /dev/null ; then \
		echo "Please install $(FLEX), then run \"make clean\" and try again" ; \
		false ; \
	fi
	$(FLEX) $(FLEX_FLAGS) $< > $@
