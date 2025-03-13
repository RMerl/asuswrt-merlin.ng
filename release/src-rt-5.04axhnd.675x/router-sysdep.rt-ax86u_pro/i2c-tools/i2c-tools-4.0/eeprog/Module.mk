# eeprog
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

EEPROG_DIR	:= eeprog

EEPROG_CFLAGS	:= -Iinclude
ifeq ($(USE_STATIC_LIB),1)
EEPROG_LDFLAGS	:= $(LIB_DIR)/$(LIB_STLIBNAME)
else
EEPROG_LDFLAGS	:= -L$(LIB_DIR) -li2c
endif

EEPROG_TARGETS	:= eeprog

#
# Programs
#

$(EEPROG_DIR)/eeprog: $(EEPROG_DIR)/eeprog.o $(EEPROG_DIR)/24cXX.o
	$(CC) $(LDFLAGS) -o $@ $^ $(EEPROG_LDFLAGS)

#
# Objects
#

$(EEPROG_DIR)/eeprog.o: $(EEPROG_DIR)/eeprog.c $(EEPROG_DIR)/24cXX.h
	$(CC) $(CFLAGS) $(EEPROG_CFLAGS) -c $< -o $@

$(EEPROG_DIR)/24cXX.o: $(EEPROG_DIR)/24cXX.c $(EEPROG_DIR)/24cXX.h $(INCLUDE_DIR)/i2c/smbus.h
	$(CC) $(CFLAGS) $(EEPROG_CFLAGS) -c $< -o $@

#
# Commands
#

all-eeprog: $(addprefix $(EEPROG_DIR)/,$(EEPROG_TARGETS))

strip-eeprog: $(addprefix $(EEPROG_DIR)/,$(EEPROG_TARGETS))
	strip $(addprefix $(EEPROG_DIR)/,$(EEPROG_TARGETS))

clean-eeprog:
	$(RM) $(addprefix $(EEPROG_DIR)/,*.o $(EEPROG_TARGETS))

install-eeprog: $(addprefix $(EEPROG_DIR)/,$(EEPROG_TARGETS))
	$(INSTALL_DIR) $(DESTDIR)$(sbindir) $(DESTDIR)$(man8dir)
	for program in $(EEPROG_TARGETS) ; do \
	$(INSTALL_PROGRAM) $(EEPROG_DIR)/$$program $(DESTDIR)$(sbindir) ; \
	$(INSTALL_DATA) $(EEPROG_DIR)/$$program.8 $(DESTDIR)$(man8dir) ; done

uninstall-eeprog:
	for program in $(EEPROG_TARGETS) ; do \
	$(RM) $(DESTDIR)$(sbindir)/$$program ; \
	$(RM) $(DESTDIR)$(man8dir)/$$program.8 ; done

all: all-eeprog

strip: strip-eeprog

clean: clean-eeprog

install: install-eeprog

uninstall: uninstall-eeprog
