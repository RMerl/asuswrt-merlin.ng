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

# Note: Don't confuse ETC_DIR (the directory in which this file resides)
# with ETCDIR (the system directory in which configuration files will be
# installed; typically /etc).
ETC_DIR := etc

install-etc:
	$(MKDIR) $(DESTDIR)$(ETCDIR) $(DESTDIR)$(ETCDIR)/sensors.d
	if [ ! -e $(DESTDIR)$(ETCDIR)/sensors3.conf ] ; then \
	  $(INSTALL) -m 644 $(ETC_DIR)/sensors.conf.default $(DESTDIR)$(ETCDIR)/sensors3.conf ; \
	fi
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 755 $(ETC_DIR)/sensors-conf-convert $(DESTDIR)$(BINDIR)
	if [ -e $(DESTDIR)$(ETCDIR)/modprobe.d/lm_sensors \
	     -a ! -e $(DESTDIR)$(ETCDIR)/modprobe.d/lm_sensors.conf ] ; then \
	  $(MV) $(DESTDIR)$(ETCDIR)/modprobe.d/lm_sensors $(DESTDIR)$(ETCDIR)/modprobe.d/lm_sensors.conf ; \
	fi
	$(MKDIR) $(DESTDIR)$(MANDIR)/man8
	$(INSTALL) -m 644 $(ETC_DIR)/sensors-conf-convert.8 $(DESTDIR)$(MANDIR)/man8

user_install :: install-etc

uninstall-etc:
	$(RM) $(DESTDIR)$(BINDIR)/sensors-conf-convert
	$(RM) $(DESTDIR)$(MANDIR)/man8/sensors-conf-convert.8

user_uninstall :: uninstall-etc
