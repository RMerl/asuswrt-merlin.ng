# I2C library for Linux
#
# Copyright (C) 2012  Jean Delvare <khali@linux-fr.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.

LIB_DIR		:= lib

LIB_CFLAGS	:= -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
		   -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
		   -W -Wundef -Wmissing-prototypes -Iinclude

# The main and minor version of the library
# The library soname (major number) must be changed if and only if the
# interface is changed in a backward incompatible way.  The interface is
# defined by the public header files - in this case they are only smbus.h.
LIB_MAINVER	:= 0
LIB_MINORVER	:= 1.0
LIB_VER		:= $(LIB_MAINVER).$(LIB_MINORVER)

# The shared and static library names
LIB_SHBASENAME	:= libi2c.so
LIB_SHSONAME	:= $(LIB_SHBASENAME).$(LIB_MAINVER)
LIB_SHLIBNAME	:= $(LIB_SHBASENAME).$(LIB_VER)
LIB_STLIBNAME	:= libi2c.a

LIB_TARGETS	:= $(LIB_SHLIBNAME)
LIB_LINKS	:= $(LIB_SHSONAME) $(LIB_SHBASENAME)
LIB_OBJECTS	:= smbus.o
ifeq ($(BUILD_STATIC_LIB),1)
LIB_TARGETS	+= $(LIB_STLIBNAME)
LIB_OBJECTS	+= smbus.ao
endif

#
# Libraries
#

$(LIB_DIR)/$(LIB_SHLIBNAME): $(LIB_DIR)/smbus.o
	$(CC) -shared $(LDFLAGS) -Wl,--version-script=$(LIB_DIR)/libi2c.map -Wl,-soname,$(LIB_SHSONAME) -o $@ $^ -lc

$(LIB_DIR)/$(LIB_SHSONAME):
	$(RM) $@
	$(LN) $(LIB_SHLIBNAME) $@

$(LIB_DIR)/$(LIB_SHBASENAME):
	$(RM) $@
	$(LN) $(LIB_SHLIBNAME) $@

$(LIB_DIR)/$(LIB_STLIBNAME): $(LIB_DIR)/smbus.ao
	$(RM) $@
	$(AR) rcvs $@ $^

#
# Objects
# Each object must be built twice, once for the shared library and
# once again for the static library.
#

$(LIB_DIR)/smbus.o: $(LIB_DIR)/smbus.c $(INCLUDE_DIR)/i2c/smbus.h
	$(CC) $(SOCFLAGS) $(LIB_CFLAGS) -c $< -o $@

$(LIB_DIR)/smbus.ao: $(LIB_DIR)/smbus.c $(INCLUDE_DIR)/i2c/smbus.h
	$(CC) $(CFLAGS) $(LIB_CFLAGS) -c $< -o $@

#
# Commands
#

all-lib: $(addprefix $(LIB_DIR)/,$(LIB_TARGETS) $(LIB_LINKS))

strip-lib: $(addprefix $(LIB_DIR)/,$(LIB_TARGETS))
	strip $(addprefix $(LIB_DIR)/,$(LIB_TARGETS))

clean-lib:
	$(RM) $(addprefix $(LIB_DIR)/,*.o *.ao $(LIB_TARGETS) $(LIB_LINKS))

install-lib: $(addprefix $(LIB_DIR)/,$(LIB_TARGETS))
	$(INSTALL_DIR) $(DESTDIR)$(libdir)
	$(INSTALL_PROGRAM) $(LIB_DIR)/$(LIB_SHLIBNAME) $(DESTDIR)$(libdir)
	$(LN) $(LIB_SHLIBNAME) $(DESTDIR)$(libdir)/$(LIB_SHSONAME)
	$(LN) $(LIB_SHSONAME) $(DESTDIR)$(libdir)/$(LIB_SHBASENAME)
ifeq ($(BUILD_STATIC_LIB),1)
	$(INSTALL_DATA) $(LIB_DIR)/$(LIB_STLIBNAME) $(DESTDIR)$(libdir)
endif

uninstall-lib:
	for library in $(LIB_TARGETS) $(LIB_LINKS) ; do \
	$(RM) $(DESTDIR)$(libdir)/$$library ; done

all: all-lib

strip: strip-lib

clean: clean-lib

install: install-lib

uninstall: uninstall-lib
