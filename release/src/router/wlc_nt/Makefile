
include ../common.mak
$(info RTCONFIG_DEBUG=$(RTCONFIG_DEBUG))

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(SRCBASE)/common/include -Wall $(if $(RTCONFIG_DEBUG),,-Os) -fPIC
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
CFLAGS	+= $(if $(RTCONFIG_DEBUG),-g,-s -O2)
ifeq ($(HND_ROUTER_AX_6756),y)
CFLAGS += -Wno-stringop-overflow
endif
LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)${EX7} ${EXTRA_NV_LDFLAGS} -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif

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
ifeq ($(RTCONFIG_QCA),y)
LDFLAGS += -L$(LIBDIR) -ldl
else ifeq ($(RTCONFIG_MT798X),y)
LDFLAGS += -L$(LIBDIR)
endif
ifneq ($(RTCONFIG_WLMODULE_MT7915D_AP),y)
LDFLAGS += -lpthread
endif
ifeq ($(RTCONFIG_RALINK_MT7622),y)
LDFLAGS += -lm -ldl
endif
else
ifeq ($(RTCONFIG_AMAS),y)
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
endif
endif

ifeq ($(RTCONFIG_SW_HW_AUTH),y)
CFLAGS  += -I$(TOP)/sw-hw-auth
endif

ifeq ($(RTCONFIG_DMALLOC),y)
CFLAGS += -DDMALLOC -DDMALLOC_FUNC_CHECK -I$(STAGEDIR)/usr/include
LDFLAGS += -L$(STAGEDIR)/usr/lib -ldmalloc
endif

ifeq ($(RTCONFIG_TRUSTZONE),y)
CFLAGS += -I$(TOP)/ateed/
LDFLAGS += -L$(TOP)/ateed/ -latee
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-BE82M GS-BE18000 GS-BE12000 GS7_PRO GT7))
LDFLAGS += -L$(TOP)/libhostapi -lhostapi
endif

vpath %.c $(SRCBASE)/shared $(WLAN_ComponentSrcDirs)

ifeq ($(wildcard $(SRCBASE)/router/wlc_nt/*.c),)
all:
	-cp -f prebuild/$(BUILD_NAME)/wlc_nt ./
	-cp -f prebuild/$(BUILD_NAME)/wlc_monitor ./
	-cp -f prebuild/$(BUILD_NAME)/libwlc_nt_lib.so ./
	-cp -f prebuild/$(BUILD_NAME)/libwlc_nt_client.so ./
else
all: wlc_nt wlc_monitor libwlc_nt_client.so
endif

# this command is for debug only
wlc_debug: main.o wlc_nt_lib.o wlc_nt_client.o
	@echo " [wlc_debug] CC $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@$(STRIP) wlc_debug

wlc_nt: wlc_nt.o
	@echo " [wlc_nt] CC $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@$(if $(RTCONFIG_DEBUG),,$(STRIP) wlc_nt)

wlc_monitor: wlc_monitor.o
	@echo " [wlc_monitor] CC $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@$(STRIP) wlc_monitor

libwlc_nt_client.so : wlc_nt_client.o
	@echo " [wlc_nt] LD $@"
	@$(LD) $(LDFLAGS) -shared -o $@ $^
	@$(if $(RTCONFIG_DEBUG),,$(STRIP) libwlc_nt_client.so)

libwlc_nt_lib.so : wlc_nt_lib.o
	@echo " [wlc_nt] LD $@"
	@$(LD) $(LDFLAGS) -shared -o $@ $^
	@$(STRIP) libwlc_nt_lib.so

install: all
	@echo "[wlc_nt] Installing..."
	@install -d $(INSTALLDIR)/usr/sbin
	@install -m 755 wlc_nt $(INSTALLDIR)/usr/sbin
	@install -d $(INSTALLDIR)/usr/sbin
	@install -m 755 wlc_monitor $(INSTALLDIR)/usr/sbin
	@install -d $(INSTALLDIR)/usr/lib
	@install -m 755 libwlc_nt_client.so $(INSTALLDIR)/usr/lib
	-@install -m 755 wlc_debug $(INSTALLDIR)/usr/sbin

%.o: %.c
	@echo " [wlc_nt] CC $@"
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.so *.o wlc_nt wlc_monitor
