#
# This is the Makefile for gpnrd.
#
# Copyright (C) 2014-2015, AAA Internet Publishing, Inc. (DBA WTFast)
#

CFLAGS += --std=c99 -Wall -Wextra -O -g
ifneq ($(ROUTER_BUILD), y)
CFLAGS += -pedantic-errors
endif
LDFLAGS += -Wl,-rpath -Wl,$(LIB)
LIBS = -lgpnutils -lgpnclient -lrt -lssl -lcrypto -lpthread 

ifeq ($(ROUTER_BUILD), y)
	LIBS += -lshared -lgcc_s -lnvram $(if $(RTCONFIG_HND_ROUTER),-L$(TOP)/wlcsm -lwlcsm,)
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

# asuswrt
PREBUILT_SRCS = $(filter-out $(wildcard $(SRCS)),$(SRCS))
ifneq ($(PREBUILT_SRCS),)
include ../../common.mak
ifeq ($(RTCONFIG_BCMARM),y)
PREBUILT_ARCH := $(BCMEX)$(EX7)
endif
$(PREBUILT_SRCS:.c=.o): %.o: prebuilt$(PREBUILT_ARCH)/%.o
	cp -f $< $@
endif

