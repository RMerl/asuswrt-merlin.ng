#
# U2EC Makefile
#
# Copyright (C) 2008 ASUSTeK Corporation
#
#

include ../.config

#ifeq ($(strip $(CROSS_COMPILE)),)
#CROSS_COMPILE := mipsel-linux-
#CC := $(CROSS_COMPILE)gcc
#AR := $(CROSS_COMPILE)ar
#AS := $(CROSS_COMPILE)as
#LD := $(CROSS_COMPILE)ld
#NM := $(CROSS_COMPILE)nm
#RANLIB := $(CROSS_COMPILE)ranlib
#STRIP := $(CROSS_COMPILE)strip
#SIZE := $(CROSS_COMPILE)size
#endif

PROG	= u2ec
OBJS	= usbsock.o testusb.o
DEBUG	= n

ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DU2EC_DEBUG
	DEBFLAGS += -DPDEBUG_SENDSECV
	DEBFLAGS += -DPDEBUG_DECODE
	OBJS += debug.o decode.o
else
	DEBFLAGS = -s -O2
endif

#ifeq ($(strip $(CROSS_COMPILE)),)
#	CC = gcc
#	DEBFLAGS += -DU2EC_ONPC
#	LDFLAGS = -L/usr/local/lib -lusb -lpthread -Wl,--rpath -Wl,/usr/local/lib
#else
	CFLAGS = -DSUPPORT_LPRng
	CFLAGS += -I. -I$(TOP)/shared
	CFLAGS += -I$(TOP)/kernel_header/include -I$(SRCBASE)/include
	CFLAGS += -I$(TOP)/libusb/libusb -DASUS_U2EC
#	CFLAGS += -I$(TOP)/libusb-0.1.12
	LDFLAGS = -lpthread -ldl
	LDFLAGS += -L$(TOP)/libusb10/libusb/.libs -lusb-1.0
	LDFLAGS += -L$(TOP)/libusb/libusb/.libs -lusb
#	LDFLAGS += -L$(TOP)/libusb-0.1.12/.libs -l:libusb.a
	LDFLAGS += -L$(TOP)/shared -lshared
	LDFLAGS += -L$(TOP_PLATFORM)/nvram${BCMEX}$(EX7) -lnvram
#endif

ifeq ($(HND_ROUTER),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm
endif

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
LDFLAGS += -lgcc_s
endif

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

ifeq ($(RTCONFIG_AMAS),y)
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
ifeq ($(RTCONFIG_QCA),y)
LDFLAGS += -lm
endif
endif

CFLAGS += -Wall -I.
CFLAGS += $(DEBFLAGS)


ifneq ($(wildcard ./prebuild/$(BUILD_NAME)),)
$(PROG):
	cp prebuild/$(BUILD_NAME)/$(PROG) .
else
$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
endif

install: $(PROG)
	install -D $(PROG) $(INSTALLDIR)/usr/sbin/$(PROG)
	$(STRIP) $(INSTALLDIR)/usr/sbin/$(PROG)

clean:
	rm -f $(PROG) *.o *~
