
include ../common.mak

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(SRCBASE)/common/include -Wall -Werror -fPIC
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
CFLAGS	+= -s -O2
LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)${EX7} ${EXTRA_NV_LDFLAGS} -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared

ifeq ($(HND_ROUTER),y)
LDFLAGS += $(EXTRALDFLAGS)
endif

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
CFLAGS  += -I$(TOP)/json-c
CFLAGS  += -I$(TOP)/sqlite
CFLAGS  += -I$(TOP)/nt_center/lib
CFLAGS  += -I$(TOP)/nt_center/actMail
CFLAGS  += -DASUSWRT_SDK
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3 -L$(TOP)/nt_center/lib -lnt
ifneq ($(RTCONFIG_QCA),y)
LDFLAGS += -lpthread
endif
else
ifeq ($(RTCONFIG_AMAS),y)
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
endif
endif

ifeq ($(RTCONFIG_SW_HW_AUTH),y)
CFLAGS  += -I$(TOP)/sw-hw-auth
endif

vpath %.c $(SRCBASE)/shared $(WLAN_ComponentSrcDirs)

ifeq ($(wildcard $(SRCBASE)/router/wlc_nt/*.c),)
all:
	-cp -f prebuild/$(BUILD_NAME)/wlc_nt ./
	-cp -f prebuild/$(BUILD_NAME)/libwlc_nt_client.so ./
else
all: wlc_nt libwlc_nt_client.so
endif

# this command is for debug only
wlc_nt: wlc_nt.o
	@echo " [wlc_nt] CC $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@$(STRIP) wlc_nt

libwlc_nt_client.so : wlc_nt_client.o
	@echo " [wlc_nt] LD $@"
	@$(LD) $(LDFLAGS) -shared -o $@ $^
	@$(STRIP) libwlc_nt_client.so

install: all
	@echo "[wlc_nt] Installing..."
	@install -d $(INSTALLDIR)/usr/sbin
	@install -m 755 wlc_nt $(INSTALLDIR)/usr/sbin
	@install -d $(INSTALLDIR)/usr/lib
	@install -m 755 libwlc_nt_client.so $(INSTALLDIR)/usr/lib

%.o: %.c
	@echo " [wlc_nt] CC $@"
	@$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.so *.o wlc_nt
