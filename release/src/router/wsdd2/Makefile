#
# Makefile for wsdd2 WSD/LLMNR server
#
#	Copyright (c) 2016 NETGEAR
#	Copyright (c) 2016 Hiro Sugawara
#
include ../common.mak

OBJFILES	= wsdd2.o wsd.o llmnr.o

ifeq ($(RTCONFIG_BCMARM),y)
ifneq ($(HND_ROUTER),y)
	OBJFILES += ifaddrs.o
endif
endif

HEADERS		= wsdd.h wsd.h

INSTALLPREFIX ?= $(PREFIX)/usr
SBININSTALLDIR = $(INSTALLPREFIX)/sbin
MANINSTALLDIR = $(INSTALLPREFIX)/share/man

all: wsdd2

wsdd2: $(OBJFILES)

$(OBJFILES) : $(HEADERS) Makefile

install:	wsdd2
	install -d $(DESTDIR)/$(SBININSTALLDIR)
	install wsdd2 $(DESTDIR)/$(SBININSTALLDIR)
	install -d $(DESTDIR)/$(MANINSTALLDIR)/man8
	install wsdd2.8 $(DESTDIR)/$(MANINSTALLDIR)/man8/wsdd2.8
	install -d $(DESTDIR)/lib/systemd/system
	install -m 0644 wsdd2.service $(DESTDIR)/lib/systemd/system

clean:
	rm -f wsdd2 $(OBJFILES)
