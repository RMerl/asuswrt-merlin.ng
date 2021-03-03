# SPDX-License-Identifier: GPL-2.0
include ../config.mk

TARGETS :=

ifeq ($(HAVE_MNL),y)

TIPCOBJ=bearer.o \
    cmdl.o link.o \
    media.o misc.o \
    msg.o nametable.o \
    node.o socket.o \
    peer.o tipc.o

TARGETS += tipc

endif

all: $(TARGETS) $(LIBS)

tipc: $(TIPCOBJ)
	$(QUIET_LINK)$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

install: all
	for i in $(TARGETS); \
	do install -m 0755 $$i $(DESTDIR)$(SBINDIR); \
	done

clean:
	rm -f $(TIPCOBJ) $(TARGETS)
