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
MODULE_DIR := prog/detect
PROGDETECTDIR := $(MODULE_DIR)

PROGDETECTMAN8DIR := $(MANDIR)/man8
PROGDETECTMAN8FILES := $(MODULE_DIR)/sensors-detect.8

# Regrettably, even 'simply expanded variables' will not put their currently
# defined value verbatim into the command-list of rules...
PROGDETECTSBININSTALL := $(MODULE_DIR)/sensors-detect

REMOVEDETECTBIN := $(patsubst $(MODULE_DIR)/%,$(DESTDIR)$(SBINDIR)/%,$(PROGDETECTSBININSTALL))
REMOVEDETECTMAN := $(patsubst $(MODULE_DIR)/%,$(DESTDIR)$(PROGDETECTMAN8DIR)/%,$(PROGDETECTMAN8FILES))

all-prog-detect:
user :: all-prog-detect

install-prog-detect: all-prog-detect
	$(MKDIR) $(DESTDIR)$(SBINDIR) $(DESTDIR)$(PROGDETECTMAN8DIR)
	$(INSTALL) -m 755 $(PROGDETECTSBININSTALL) $(DESTDIR)$(SBINDIR)
	$(INSTALL) -m 644 $(PROGDETECTMAN8FILES) $(DESTDIR)$(PROGDETECTMAN8DIR)
user_install :: install-prog-detect

user_uninstall::
	$(RM) $(REMOVEDETECTBIN)
	$(RM) $(REMOVEDETECTMAN)

clean-prog-detect:
clean :: clean-prog-detect
