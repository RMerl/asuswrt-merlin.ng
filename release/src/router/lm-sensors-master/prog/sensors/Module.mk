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
MODULE_DIR := prog/sensors
PROGSENSORSDIR := $(MODULE_DIR)

PROGSENSORSMAN1DIR := $(MANDIR)/man1
PROGSENSORSMAN1FILES := $(MODULE_DIR)/sensors.1

# Regrettably, even 'simply expanded variables' will not put their currently
# defined value verbatim into the command-list of rules...
PROGSENSORSTARGETS := $(MODULE_DIR)/sensors
PROGSENSORSSOURCES := $(MODULE_DIR)/main.c $(MODULE_DIR)/chips.c

# Include all dependency files. We use '.rd' to indicate this will create
# executables.
INCLUDEFILES += $(PROGSENSORSSOURCES:.c=.rd)

REMOVESENSORSBIN := $(patsubst $(MODULE_DIR)/%,$(DESTDIR)$(BINDIR)/%,$(PROGSENSORSTARGETS))
REMOVESENSORSMAN := $(patsubst $(MODULE_DIR)/%,$(DESTDIR)$(PROGSENSORSMAN1DIR)/%,$(PROGSENSORSMAN1FILES))

LIBICONV := $(shell if /sbin/ldconfig -p | grep -q '/libiconv\.so$$' ; then echo \-liconv; else echo; fi)

$(PROGSENSORSTARGETS): $(PROGSENSORSSOURCES:.c=.ro) lib/$(LIBSHBASENAME)
	$(CC) $(EXLDFLAGS) -o $@ $(PROGSENSORSSOURCES:.c=.ro) $(LIBICONV) -Llib -lsensors

all-prog-sensors: $(PROGSENSORSTARGETS)
user :: all-prog-sensors

install-prog-sensors: all-prog-sensors
	$(MKDIR) $(DESTDIR)$(BINDIR) $(DESTDIR)$(PROGSENSORSMAN1DIR)
	$(INSTALL) -m 755 $(PROGSENSORSTARGETS) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 644 $(PROGSENSORSMAN1FILES) $(DESTDIR)$(PROGSENSORSMAN1DIR)
user_install :: install-prog-sensors

user_uninstall::
	$(RM) $(REMOVESENSORSBIN)
	$(RM) $(REMOVESENSORSMAN)

clean-prog-sensors:
	$(RM) $(PROGSENSORSDIR)/*.rd $(PROGSENSORSDIR)/*.ro 
	$(RM) $(PROGSENSORSTARGETS)
clean :: clean-prog-sensors
