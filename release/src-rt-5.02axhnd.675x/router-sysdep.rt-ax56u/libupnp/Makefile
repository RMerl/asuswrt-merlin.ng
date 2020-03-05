include $(TOP)/common.mak
ifneq ($(CMWIFI),)
UPNPLIB_NAME = libwlupnp.so
else
UPNPLIB_NAME = libupnp.so
endif

UPNPLIB = $(SRCBASE)/router-sysdep/libupnp
SRCPATH = $(UPNPLIB)/upnp

INCLUDES += -I$(UPNPLIB)/include -I$(SRCBASE)/include ${WLAN_StdIncPathA} ${WLAN_ComponentIncPathA} -I$(SRCBASE)/../components/shared -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/common/include -I$(SRCBASE_ROUTER)/shared
INCLUDES += -I$(SRCBASE)/../components/bcmcrypto/include
CFLAGS += ${INCLUDES}

CFLAGS += -Wall -Wunused -g -s -Werror -fPIC

LDFLAGS = -L$(TOP)/libbcmcrypto -lbcmcrypto

vpath %.c $(SRCPATH) $(UPNPLIB)/linux
vpath %.o $(UPNPLIB)/prebuilt

SRCFILES = upnp.c upnp_ssdp.c upnp_http.c upnp_gena.c upnp_soap.c \
	upnp_description.c upnp_device.c upnp_util.c upnp_msg.c \
	upnp_linux_osl.c

OBJFILES = ${SRCFILES:.c=.o}

ifeq ($(wildcard ./upnp),)
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
