#  Module.mk - Makefile for a Linux module for reading sensor data.
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

# Note that MODULE_DIR (the directory in which this file resides) is a
# 'simply expanded variable'. That means that its value is substituted
# verbatim in the rules, until it is redefined. 
MODULE_DIR := prog/dump
PROGDUMPDIR := $(MODULE_DIR)

PROGDUMPMAN8DIR := $(MANDIR)/man8
PROGDUMPMAN8FILES := $(MODULE_DIR)/isadump.8 $(MODULE_DIR)/isaset.8

# Regrettably, even 'simply expanded variables' will not put their currently
# defined value verbatim into the command-list of rules...
PROGDUMPTARGETS := $(MODULE_DIR)/isadump $(MODULE_DIR)/isaset
PROGDUMPSOURCES := $(MODULE_DIR)/util.c $(MODULE_DIR)/isadump.c \
		   $(MODULE_DIR)/isaset.c $(MODULE_DIR)/superio.c
PROGDUMPBININSTALL := $(MODULE_DIR)/isadump $(MODULE_DIR)/isaset

# Include all dependency files. We use '.rd' to indicate this will create
# executables.
INCLUDEFILES += $(PROGDUMPSOURCES:.c=.rd)

REMOVEDUMPBIN := $(patsubst $(MODULE_DIR)/%,$(DESTDIR)$(SBINDIR)/%,$(PROGDUMPBININSTALL))
REMOVEDUMPMAN := $(patsubst $(MODULE_DIR)/%,$(DESTDIR)$(PROGDUMPMAN8DIR)/%,$(PROGDUMPMAN8FILES))

all-prog-dump: $(PROGDUMPTARGETS)
user :: all-prog-dump

$(MODULE_DIR)/isadump: $(MODULE_DIR)/isadump.ro $(MODULE_DIR)/superio.ro $(MODULE_DIR)/util.ro
	$(CC) $(EXLDFLAGS) -o $@ $^

$(MODULE_DIR)/isaset: $(MODULE_DIR)/isaset.ro $(MODULE_DIR)/util.ro
	$(CC) $(EXLDFLAGS) -o $@ $^

install-prog-dump: all-prog-dump
	$(MKDIR) $(DESTDIR)$(SBINDIR) $(DESTDIR)$(PROGDUMPMAN8DIR)
	$(INSTALL) -m 755 $(PROGDUMPBININSTALL) $(DESTDIR)$(SBINDIR)
	$(INSTALL) -m 644 $(PROGDUMPMAN8FILES) $(DESTDIR)$(PROGDUMPMAN8DIR)
user_install :: install-prog-dump

user_uninstall::
	$(RM) $(REMOVEDUMPBIN)
	$(RM) $(REMOVEDUMPMAN)

clean-prog-dump:
	$(RM) $(PROGDUMPDIR)/*.rd $(PROGDUMPDIR)/*.ro $(PROGDUMPTARGETS)
clean :: clean-prog-dump
