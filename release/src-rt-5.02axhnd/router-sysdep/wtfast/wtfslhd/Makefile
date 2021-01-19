#
# This is the Makefile for wtfslhd.
#
# Copyright (c) 2014, AAA Internet Publishing, Inc. (DBA WTFast)
#
include ../../common.mak
LIBS = -lev -lwtf -lgpnutils -lcurl -lssl -lcrypto -lm

ifeq ($(ROUTER_BUILD), y)
	LIBS += -lshared -lgcc_s -lnvram
ifeq ($(RTCONFIG_AMAS),y)
	LIBS += -L$(TOP)/json-c/.libs -ljson-c
endif		
endif

LDFLAGS += -Wl,-rpath -Wl,$(LIB)

SRCS = wtfslhd.c nvram.c
OBJS = $(SRCS:.c=.o)
HDRS = $(SRCS:.c=.h)

TARGETS = wtfslhd

INC = $(DESTDIR)/usr/include

all: $(TARGETS)

$(TARGETS):	$(OBJS)
	$(CC) $(CFLAGS)  -o $@ $(OBJS)  $(LDFLAGS) $(LIBS)

clean:
	@rm -f $(TARGETS) $(OBJS)
