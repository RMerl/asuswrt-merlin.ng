include ../common.mak

UPNPLIB_NAME = libupnp.so
UPNPLIB = $(SRCBASE)/router/libupnp$(BCMEX)$(EX7)
SRCPATH = $(UPNPLIB)/upnp

INCLUDES += -I$(UPNPLIB)/include -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -I$(SRCBASE)/include/bcmcrypto -I$(SRCBASE)/router/shared
CFLAGS += ${INCLUDES}

CFLAGS += -Wall -Wunused -g -s -Werror -fPIC


LDFLAGS = -L$(TOP)/libbcmcrypto -lbcmcrypto

vpath %.c $(SRCPATH) $(UPNPLIB)/linux
vpath %.o $(UPNPLIB)/prebuilt

SRCFILES = upnp.c upnp_ssdp.c upnp_http.c upnp_gena.c upnp_soap.c \
	upnp_description.c upnp_device.c upnp_util.c upnp_msg.c \
	upnp_linux_osl.c

OBJFILES = ${SRCFILES:.c=.o}

ifneq ($(wildcard ./prebuilt/$(UPNPLIB_NAME)),)
all:
	echo use prebuilt 
	cp -f ./prebuilt/$(UPNPLIB_NAME) .
else
all: $(OBJFILES)
	echo rebuild upnp 
	$(LD) -shared -o $(UPNPLIB_NAME) $^
endif


install: all
	install -d $(INSTALLDIR)/usr/lib
	install -m 755 $(UPNPLIB_NAME) $(INSTALLDIR)/usr/lib
	$(STRIP) $(INSTALLDIR)/usr/lib/$(UPNPLIB_NAME)

clean:
	rm -f $(UPNPLIB_NAME) $(OBJFILES)

.PHONY: clean
