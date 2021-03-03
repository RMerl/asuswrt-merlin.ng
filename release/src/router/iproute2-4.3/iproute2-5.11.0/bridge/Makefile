# SPDX-License-Identifier: GPL-2.0
BROBJ = bridge.o fdb.o monitor.o link.o mdb.o vlan.o

include ../config.mk

all: bridge

bridge: $(BROBJ) $(LIBNETLINK)
	$(QUIET_LINK)$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

install: all
	install -m 0755 bridge $(DESTDIR)$(SBINDIR)

clean:
	rm -f $(BROBJ) bridge
