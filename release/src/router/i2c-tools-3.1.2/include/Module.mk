# Linux I2C header files
#
# Copyright (C) 2007  Jean Delvare <jdelvare@suse.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

INCLUDE_DIR	:= include

INCLUDE_TARGETS	:= linux/i2c-dev.h

#
# Commands
#

install-include: $(addprefix $(INCLUDE_DIR)/,$(INCLUDE_TARGETS))
	$(INSTALL_DIR) $(DESTDIR)$(incdir)/linux
	for file in $(INCLUDE_TARGETS) ; do \
	$(INSTALL_DATA) $(INCLUDE_DIR)/$$file $(DESTDIR)$(incdir)/$$file ; done

uninstall-include:
	for file in $(INCLUDE_TARGETS) ; do \
	$(RM) $(DESTDIR)$(incdir)/$$file ; done

install: install-include

uninstall: uninstall-include
