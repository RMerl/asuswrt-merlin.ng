#
# Makefile for wsdd2 WSD/LLMNR server
#
#	Copyright (c) 2016 NETGEAR
#	Copyright (c) 2016 Hiro Sugawara
#

CFLAGS        ?= -Wall -Wextra -g -O0
LDFLAGS       ?= -g
OBJFILES      = wsdd2.o wsd.o llmnr.o
HEADERS       = wsdd.h wsd.h

PREFIX  ?= /usr
SBINDIR ?= $(PREFIX)/sbin
MANDIR  ?= $(PREFIX)/share/man
LIBDIR  ?= $(PREFIX)/lib

all: wsdd2

nl_debug: CPPFLAGS+=-DMAIN
nl_debug: nl_debug.c; $(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

wsdd2: $(OBJFILES)
$(OBJFILES): $(HEADERS) Makefile

install: wsdd2
	install -d $(DESTDIR)$(SBINDIR)
	install wsdd2 $(DESTDIR)$(SBINDIR)
	install -d $(DESTDIR)$(MANDIR)/man8
	install -m 0644 wsdd2.8 $(DESTDIR)$(MANDIR)/man8
	install -d $(DESTDIR)$(LIBDIR)/systemd/system
	install -m 0644 wsdd2.service $(DESTDIR)$(LIBDIR)/systemd/system

clean:
	rm -f wsdd2 nl_debug $(OBJFILES)
