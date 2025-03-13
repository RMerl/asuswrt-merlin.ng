# I2C tools for Linux
#
# Copyright (C) 2007, 2012  Jean Delvare <jdelvare@suse.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

TOOLS_DIR	:= tools

TOOLS_CFLAGS	:= -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
		   -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
		   -W -Wundef -Wmissing-prototypes -Iinclude
ifeq ($(USE_STATIC_LIB),1)
TOOLS_LDFLAGS	:= $(LIB_DIR)/$(LIB_STLIBNAME)
else
TOOLS_LDFLAGS	:= -L$(LIB_DIR) -li2c
endif

TOOLS_TARGETS	:= i2cdetect i2cdump i2cset i2cget i2ctransfer

#
# Programs
#

$(TOOLS_DIR)/i2cdetect: $(TOOLS_DIR)/i2cdetect.o $(TOOLS_DIR)/i2cbusses.o
	$(CC) $(LDFLAGS) -o $@ $^ $(TOOLS_LDFLAGS)

$(TOOLS_DIR)/i2cdump: $(TOOLS_DIR)/i2cdump.o $(TOOLS_DIR)/i2cbusses.o $(TOOLS_DIR)/util.o
	$(CC) $(LDFLAGS) -o $@ $^ $(TOOLS_LDFLAGS)

$(TOOLS_DIR)/i2cset: $(TOOLS_DIR)/i2cset.o $(TOOLS_DIR)/i2cbusses.o $(TOOLS_DIR)/util.o
	$(CC) $(LDFLAGS) -o $@ $^ $(TOOLS_LDFLAGS)

$(TOOLS_DIR)/i2cget: $(TOOLS_DIR)/i2cget.o $(TOOLS_DIR)/i2cbusses.o $(TOOLS_DIR)/util.o
	$(CC) $(LDFLAGS) -o $@ $^ $(TOOLS_LDFLAGS)

$(TOOLS_DIR)/i2ctransfer: $(TOOLS_DIR)/i2ctransfer.o $(TOOLS_DIR)/i2cbusses.o $(TOOLS_DIR)/util.o
	$(CC) $(LDFLAGS) -o $@ $^ $(TOOLS_LDFLAGS)

#
# Objects
#

$(TOOLS_DIR)/i2cdetect.o: $(TOOLS_DIR)/i2cdetect.c $(TOOLS_DIR)/i2cbusses.h version.h $(INCLUDE_DIR)/i2c/smbus.h
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

$(TOOLS_DIR)/i2cdump.o: $(TOOLS_DIR)/i2cdump.c $(TOOLS_DIR)/i2cbusses.h $(TOOLS_DIR)/util.h version.h $(INCLUDE_DIR)/i2c/smbus.h
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

$(TOOLS_DIR)/i2cset.o: $(TOOLS_DIR)/i2cset.c $(TOOLS_DIR)/i2cbusses.h $(TOOLS_DIR)/util.h version.h $(INCLUDE_DIR)/i2c/smbus.h
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

$(TOOLS_DIR)/i2cget.o: $(TOOLS_DIR)/i2cget.c $(TOOLS_DIR)/i2cbusses.h $(TOOLS_DIR)/util.h version.h $(INCLUDE_DIR)/i2c/smbus.h
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

$(TOOLS_DIR)/i2ctransfer.o: $(TOOLS_DIR)/i2ctransfer.c $(TOOLS_DIR)/i2cbusses.h $(TOOLS_DIR)/util.h version.h
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

$(TOOLS_DIR)/i2cbusses.o: $(TOOLS_DIR)/i2cbusses.c $(TOOLS_DIR)/i2cbusses.h
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

$(TOOLS_DIR)/util.o: $(TOOLS_DIR)/util.c $(TOOLS_DIR)/util.h
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

#
# Commands
#

all-tools: $(addprefix $(TOOLS_DIR)/,$(TOOLS_TARGETS))

strip-tools: $(addprefix $(TOOLS_DIR)/,$(TOOLS_TARGETS))
	strip $(addprefix $(TOOLS_DIR)/,$(TOOLS_TARGETS))

clean-tools:
	$(RM) $(addprefix $(TOOLS_DIR)/,*.o $(TOOLS_TARGETS))

install-tools: $(addprefix $(TOOLS_DIR)/,$(TOOLS_TARGETS))
	$(INSTALL_DIR) $(DESTDIR)$(sbindir) $(DESTDIR)$(man8dir)
	for program in $(TOOLS_TARGETS) ; do \
	$(INSTALL_PROGRAM) $(TOOLS_DIR)/$$program $(DESTDIR)$(sbindir) ; \
	$(INSTALL_DATA) $(TOOLS_DIR)/$$program.8 $(DESTDIR)$(man8dir) ; done

uninstall-tools:
	for program in $(TOOLS_TARGETS) ; do \
	$(RM) $(DESTDIR)$(sbindir)/$$program ; \
	$(RM) $(DESTDIR)$(man8dir)/$$program.8 ; done

all: all-tools

strip: strip-tools

clean: clean-tools

install: install-tools

uninstall: uninstall-tools
