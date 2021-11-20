#
# This is the Makefile for gpnrd.
#
# Copyright (C) 2014-2015, AAA Internet Publishing, Inc. (DBA WTFast)
#
include ../../common.mak
ifneq ($(ROUTER_BUILD), y)
CFLAGS += -pedantic-errors
endif
LDFLAGS += -Wl,-rpath -Wl,$(LIB)
LIBS = -lgpnutils -lgpnclient -lrt -lshared -lnvram -lssl -lcrypto

ifeq ($(RTCONFIG_AMAS),y)
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
endif

SRCS = gpnrd.c
OBJS = $(SRCS:.c=.o)
HDRS = $(SRCS:.c=.h)

TARGETS = gpnrd

INC = $(DESTDIR)/usr/include
LIB = $(DESTDIR)/usr/lib
SBIN = $(DESTDIR)/usr/sbin

.PHONY: all clean tags install

all: $(TARGETS)

$(TARGETS):	$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(TARGETS) $(OBJS) ioperf

tags:
	@ctags -t $(SRCS) $(HDRS)

install:
	install -d -o 0 -g 0 -m 755 $(SBIN)
	install -o 0 -g 0 -m 755 gpnrd $(SBIN)

ioperf: ioperf.c
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LIBS) $<


