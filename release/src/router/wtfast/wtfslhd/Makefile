#
# This is the Makefile for wtfslhd.
#
# Copyright (c) 2014, AAA Internet Publishing, Inc. (DBA WTFast)
#

LIBS = -lev -lwtf -lgpnutils -lssl -lcrypto -lcurl -lm -lpthread

ifeq ($(ROUTER_BUILD), y)
	LIBS += -lshared -lgcc_s -lnvram $(if $(RTCONFIG_HND_ROUTER),-L$(TOP)/wlcsm -lwlcsm,)
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
