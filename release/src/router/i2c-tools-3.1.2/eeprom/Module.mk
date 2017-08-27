# EEPROM decoding scripts for the Linux eeprom driver
#
# Copyright (C) 2007-2013  Jean Delvare <jdelvare@suse.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

EEPROM_DIR	:= eeprom

EEPROM_TARGETS	:= decode-dimms decode-vaio ddcmon decode-edid
EEPROM_MANPAGES	:= decode-dimms.1 decode-vaio.1

#
# Commands
#

install-eeprom: $(addprefix $(EEPROM_DIR)/,$(EEPROM_TARGETS))
	$(INSTALL_DIR) $(DESTDIR)$(bindir) $(DESTDIR)$(mandir)/man1
	for program in $(EEPROM_TARGETS) ; do \
	$(INSTALL_PROGRAM) $(EEPROM_DIR)/$$program $(DESTDIR)$(bindir) ; done
	for manual in $(EEPROM_MANPAGES) ; do \
	$(INSTALL_DATA) $(EEPROM_DIR)/$$manual $(DESTDIR)$(mandir)/man1 ; done

uninstall-eeprom:
	for program in $(EEPROM_TARGETS) ; do \
	$(RM) $(DESTDIR)$(bindir)/$$program ; done
	for manual in $(EEPROM_MANPAGES) ; do \
	$(RM) $(DESTDIR)$(mandir)/$$manual ; done

install: install-eeprom

uninstall: uninstall-eeprom
