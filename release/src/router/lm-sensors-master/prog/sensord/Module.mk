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

# The round robin database (RRD) development headers and libraries are
# REQUIRED. Get this package from:
#   http://people.ee.ethz.ch/~oetiker/webtools/rrdtool/

# Note that MODULE_DIR (the directory in which this file resides) is a
# 'simply expanded variable'. That means that its value is substituted
# verbatim in the rules, until it is redefined. 
MODULE_DIR := prog/sensord
PROGSENSORDDIR := $(MODULE_DIR)

PROGSENSORDMAN8DIR := $(MANDIR)/man8
PROGSENSORDMAN8FILES := $(MODULE_DIR)/sensord.8

# Regrettably, even 'simply expanded variables' will not put their currently
# defined value verbatim into the command-list of rules...
PROGSENSORDTARGETS := $(MODULE_DIR)/sensord
PROGSENSORDSOURCES := $(MODULE_DIR)/args.c $(MODULE_DIR)/chips.c $(MODULE_DIR)/lib.c $(MODULE_DIR)/rrd.c $(MODULE_DIR)/sense.c $(MODULE_DIR)/sensord.c

# Include all dependency files. We use '.rd' to indicate this will create
# executables.
INCLUDEFILES += $(PROGSENSORDSOURCES:.c=.rd)

REMOVESENSORDBIN := $(patsubst $(MODULE_DIR)/%,$(DESTDIR)$(SBINDIR)/%,$(PROGSENSORDTARGETS))
REMOVESENSORDMAN := $(patsubst $(MODULE_DIR)/%,$(DESTDIR)$(PROGSENSORDMAN8DIR)/%,$(PROGSENSORDMAN8FILES))

$(PROGSENSORDTARGETS): $(PROGSENSORDSOURCES:.c=.ro) lib/$(LIBSHBASENAME)
	$(CC) $(EXLDFLAGS) -o $@ $(PROGSENSORDSOURCES:.c=.ro) -Llib -lsensors -lrrd

all-prog-sensord: $(PROGSENSORDTARGETS)
user :: all-prog-sensord

install-prog-sensord: all-prog-sensord
	$(MKDIR) $(DESTDIR)$(SBINDIR) $(DESTDIR)$(PROGSENSORDMAN8DIR)
	$(INSTALL) -m 755 $(PROGSENSORDTARGETS) $(DESTDIR)$(SBINDIR)
	$(INSTALL) -m 644 $(PROGSENSORDMAN8FILES) $(DESTDIR)$(PROGSENSORDMAN8DIR)
user_install :: install-prog-sensord

user_uninstall::
	$(RM) $(REMOVESENSORDBIN)
	$(RM) $(REMOVESENSORDMAN)

clean-prog-sensord:
	$(RM) $(PROGSENSORDDIR)/*.rd $(PROGSENSORDDIR)/*.ro 
	$(RM) $(PROGSENSORDTARGETS)
clean :: clean-prog-sensord
