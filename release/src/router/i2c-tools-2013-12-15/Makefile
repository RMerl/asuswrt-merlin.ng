# I2C tools for Linux
#
# Copyright (C) 2007  Jean Delvare <khali@linux-fr.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

DESTDIR	=
prefix	= /usr/local
bindir	= $(prefix)/bin
sbindir	= $(prefix)/sbin
mandir	= $(prefix)/share/man
man8dir	= $(mandir)/man8
incdir	= $(prefix)/include
libdir	= $(prefix)/lib

INSTALL		:= install
INSTALL_DATA	:= $(INSTALL) -m 644
INSTALL_DIR	:= $(INSTALL) -m 755 -d
INSTALL_PROGRAM	:= $(INSTALL) -m 755
LN		:= ln -sf
RM		:= rm -f

CC	?= gcc
AR	?= ar

CFLAGS		?= -O2
# When debugging, use the following instead
#CFLAGS		:= -O -g
CFLAGS		+= -Wall
SOCFLAGS	:= -fpic -D_REENTRANT $(CFLAGS)

BUILD_STATIC_LIB ?= 1

KERNELVERSION	:= $(shell uname -r)

.PHONY: all strip clean install uninstall

all:

EXTRA	:=
#EXTRA	+= py-smbus
SRCDIRS	:= include lib eeprom stub tools $(EXTRA)
include $(SRCDIRS:%=%/Module.mk)
