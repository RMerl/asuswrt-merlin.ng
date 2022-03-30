# Helper for the Linux i2c-stub bus driver
#
# Copyright (C) 2007-2008  Jean Delvare <khali@linux-fr.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

STUB_DIR	:= stub

#
# Commands
#

install-stub: $(STUB_DIR)/i2c-stub-from-dump
	$(INSTALL_DIR) $(DESTDIR)$(sbindir) $(DESTDIR)$(man8dir)
	$(INSTALL_PROGRAM) $(STUB_DIR)/i2c-stub-from-dump $(DESTDIR)$(sbindir)
	$(INSTALL_DATA) $(STUB_DIR)/i2c-stub-from-dump.8 $(DESTDIR)$(man8dir)

uninstall-stub:
	$(RM) $(DESTDIR)$(sbindir)/i2c-stub-from-dump
	$(RM) $(DESTDIR)$(man8dir)/i2c-stub-from-dump.8

install: install-stub

uninstall: uninstall-stub
