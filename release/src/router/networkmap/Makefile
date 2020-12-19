include ../.config

EXEC = networkmap
OBJS = function.o connections.o sm.o networkmap.o

ifeq ($(HND_ROUTER),y)
CFLAGS += -Werror=return-type -Werror=uninitialized -Wno-date-time 
endif
CFLAGS  += -I. -I$(TOP)/shared -I$(TOP)/kernel_header/include -I$(SRCBASE)/include -I$(TOP)/json-c
CFLAGS  += -s -O2 -DNO_PARALLEL

ifeq ($(RTCONFIG_RALINK),y)
CFLAGS += -I$(TOP)/shared/sysdeps/ralink -I$(TOP)/wireless_tools
endif

ifeq ($(RTCONFIG_QCA),y)
CFLAGS += -I$(TOP)/shared/sysdeps/qca -I$(TOP)/wireless_tools
CFLAGS += $(if $(RTCONFIG_SAMBA36X),-DSMP)
endif

ifeq ($(CONFIG_REALTEK),y)
CFLAGS += -I$(TOP)/shared/sysdeps/realtek
endif

ifeq ($(RTCONFIG_ALPINE),y)
CFLAGS += -DALPINE_WPS
CFLAGS += -I$(TOP)/httpd/sysdeps -I$(TOP)/shared/sysdeps/alpine -I$(TOP)/wireless_tools
endif

ifeq ($(RTCONFIG_LANTIQ),y)
CFLAGS += -DLANTIQ_WPS
CFLAGS += -I$(TOP)/shared/sysdeps/lantiq -I$(TOP)/wireless_tools
endif
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
CFLAGS += -I$(SRCBASE)/common/include
#LDFLAGS += -L$(TOP)/libbcm -lbcm
LDFLAGS += -lgcc_s
ifeq ($(RTCONFIG_QTN),y)
CFLAGS += -I$(TOP)/libqcsapi_client -I$(TOP)/shared/sysdeps/qtn
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif
endif

ifeq ($(RTCONFIG_QSR10G),y)
CFLAGS += -I$(TOP)/libqcsapi_client_10g
LDFLAGS += -L$(TOP)/libqcsapi_client_10g -lqcsapi_client
endif

ifeq ($(RTCONFIG_LANTIQ),y)
LDFLAGS += -L$(TOP)/fapi_wlan_common-05.04.00.131 -lfapiwlancommon -ldl -L$(TOP)/libhelper-1.4.0.2 -lhelper -L$(TOP)/libfapi-0.1 -lfapi
endif

ifeq ($(HND_ROUTER_AX), y)
CFLAGS += -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/wl -I$(SRCBASE)/wl/sys
endif

ifeq ($(RTCONFIG_BWDPI),y)
CFLAGS += -I$(TOP)/sqlite -I$(TOP)/httpd
CFLAGS += -I$(TOP)/bwdpi_source/src/tdts_udb_sample
CFLAGS += -I$(TOP)/bwdpi_source/src/tdts_udb_sample/linux
CFLAGS += -I$(TOP)/bwdpi_source/include
CFLAGS += -I$(TOP)/bwdpi_source/asus_include
endif
ifeq ($(RTCONFIG_BWDPI_DEP),y)
CFLAGS += -I$(TOP)/httpd
CFLAGS += -I$(TOP)/bwdpi_dep/bwdpi
endif

LDFLAGS += -lm -L$(TOP)/shared -lshared -L$(TOP_PLATFORM)/nvram${BCMEX} -lnvram -L$(TOP)/json-c/.libs -ljson-c
LDFLAGS += -L$(TOP)/shared -lshared -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram 
ifeq ($(HND_ROUTER),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
endif

ifeq ($(RTCONFIG_BWDPI),y)
LDFLAGS += -L$(TOP)/bwdpi_source/asus -lbwdpi
endif
ifeq ($(RTCONFIG_BWDPI_DEP),y)
LDFLAGS += -L$(TOP)/bwdpi_dep/bwdpi -lbwdpi
endif

ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
CFLAGS  += -I$(TOP)/sqlite
CFLAGS  += -I$(TOP)/nt_center/lib
CFLAGS  += -DASUSWRT_SDK
LDFLAGS += -L$(TOP)/nt_center/lib/ -lnt -lm
LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3
LDFLAGS += -lpthread
endif

ifeq ($(RTCONFIG_PERMISSION_MANAGEMENT),y)
CFLAGS += -I$(TOP)/sqlCipher
CFLAGS += -I$(TOP)/PMS_DBapis
CFLAGS += -I$(TOP)/openssl/include/openssl
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl
LDFLAGS += -L $(TOP)/sqlCipher/.libs -lsqlcipher
LDFLAGS += -L$(TOP)/PMS_DBapis -lpms_sql
endif

ifeq ($(RTCONFIG_CFGSYNC),y)
CFLAGS += -I$(TOP)/cfg_mnt
LIBS += -L$(TOP)/cfg_mnt -lcfgmnt
endif

include $(SRCBASE)/.config

CFLAGS += -DNMP_DB
CFLAGS += -DBTN_SETUP
ifeq ($(HND_ROUTER),y)
CFLAGS += -DHND_ROUTER
endif

ifeq ($(RTCONFIG_QTN),y)
	QTN = _qtn
else
	QTN =
endif
ifneq ($(AC2900),y)
ifeq ($(wildcard $(SRCBASE)/router/networkmap/*.c),)
all: 
	-cp -f prebuild/$(BUILD_NAME)/networkmap networkmap
	-cp -f prebuild/$(BUILD_NAME)/asusdiscovery asusdiscovery
	-cp -f prebuild/$(BUILD_NAME)/arpstorm arpstorm
else
all: $(EXEC) asusdiscovery arpstorm
endif
else
ifeq ($(wildcard $(SRCBASE)/router/networkmap/*.c),)
all: 
	-cp -f prebuild/$(BUILD_NAME)/networkmap networkmap
else
all: $(EXEC)
endif
endif

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

arpstorm: arpstorm.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

ifneq ($(AC2900),y)
asusdiscovery: packet.o ASUS_Discovery.o ASUS_Discovery_Debug.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
endif

install:
	install -D $(EXEC) $(INSTALLDIR)/usr/sbin/$(EXEC)
	$(STRIP) $(INSTALLDIR)/usr/sbin/$(EXEC)
ifneq ($(AC2900),y)
	install -D asusdiscovery $(INSTALLDIR)/usr/sbin/asusdiscovery
	$(STRIP) $(INSTALLDIR)/usr/sbin/asusdiscovery
ifneq ($(wildcard $(SRCBASE)/router/networkmap/arpstorm),)
	install -D arpstorm $(INSTALLDIR)/usr/sbin/arpstorm
	$(STRIP) $(INSTALLDIR)/usr/sbin/arpstorm
endif
endif
	install -d $(INSTALLDIR)/usr/networkmap
	cp -f networkmap.oui.js $(INSTALLDIR)/usr/networkmap/networkmap.oui.js

clean:
ifneq ($(AC2900),y)
	rm -rf *.o $(EXEC) *~ asusdiscovery arpstorm
else
	rm -rf *.o $(EXEC) *~
endif
