
all dynamic install: conditional_build

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common


ifneq ($(strip $(BRCM_DRIVER_EMMC)),)
conditional_build: mmc
	install -m 755 mmc $(INSTALL_DIR)/bin

else

conditional_build: 
	@echo "BRCM_DRIVER_EMMC not configured, skipping mmc-utils"

endif

CC ?= gcc
AM_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_FORTIFY_SOURCE=2
CFLAGS ?= -g -O2
objects = \
	mmc.o \
	mmc_cmds.o \
	3rdparty/hmac_sha/hmac_sha2.o \
	3rdparty/hmac_sha/sha2.o

CHECKFLAGS = -Wall -Werror -Wuninitialized -Wundef

DEPFLAGS = -Wp,-MMD,$(@D)/.$(@F).d,-MT,$@

override CFLAGS := $(CHECKFLAGS) $(AM_CFLAGS) $(CFLAGS)

INSTALL = install
prefix ?= /usr/local
bindir = $(prefix)/bin
LIBS=
RESTORE_LIBS=

progs = mmc

# make C=1 to enable sparse
ifdef C
	check = sparse $(CHECKFLAGS)
endif

all: $(progs) manpages

.c.o:
ifdef C
	$(check) $<
endif
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

mmc: $(objects)
	$(CC) $(CFLAGS) -o $@ $(objects) $(LDFLAGS) $(LIBS)

manpages:
	$(MAKE) -C man

install-man:
	$(MAKE) -C man install

clean:
	rm -f $(progs) $(objects) $(objects:.o=.d) .*.o.d */*/.*.o.d
	$(MAKE) -C man clean

install: $(progs) install-man
	$(INSTALL) -m755 -d $(INSTALLDIR)$(bindir)
	$(INSTALL) $(progs) $(INSTALLDIR)$(bindir)

.PHONY: all clean install manpages install-man
