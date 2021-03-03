# SPDX-License-Identifier: GPL-2.0
include ../config.mk

TARGETS :=

ifeq ($(HAVE_MNL),y)

DEVLINKOBJ = devlink.o mnlg.o
TARGETS += devlink

endif

all: $(TARGETS) $(LIBS)

devlink: $(DEVLINKOBJ) $(LIBNETLINK)
	$(QUIET_LINK)$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

install: all
	for i in $(TARGETS); \
	do install -m 0755 $$i $(DESTDIR)$(SBINDIR); \
	done

clean:
	rm -f $(DEVLINKOBJ) $(TARGETS)
