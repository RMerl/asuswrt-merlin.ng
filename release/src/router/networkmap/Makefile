include ../.config

EXEC = networkmap
OBJS = JSON_checker.o function.o connections.o sm.o download.o shared_func.o protect_name.o networkmap.o

ifeq ($(HND_ROUTER),y)
CFLAGS += -Werror=return-type -Werror=uninitialized -Wno-date-time 
endif
CFLAGS  += -I. -I$(TOP)/shared -I$(TOP)/kernel_header/include -I$(SRCBASE)/include -I$(TOP)/json-c
CFLAGS  += $(if $(RTCONFIG_DEBUG),,-s) -O2 -DNO_PARALLEL
CFLAGS  += -I$(TOP)/libasc
CFLAGS  += -I$(TOP)/curl/include

ifeq ($(RTCONFIG_TR069),y)
CFLAGS += -I$(TOP)/libssh2-1.10.0/include -I$(TOP)/zlib/include
LDFLAGS += -L$(TOP)/libssh2-1.10.0/src/.libs -lssh2 -L$(TOP)/zlib -lz
endif

ifeq ($(RTCONFIG_RALINK),y)
CFLAGS += -I$(TOP)/shared/sysdeps/ralink -I$(TOP)/wireless_tools$(WTOOL_SUFFIX)
endif

ifeq ($(RTCONFIG_QCA),y)
CFLAGS += -I$(TOP)/shared/sysdeps/qca -I$(TOP)/wireless_tools$(WTOOL_SUFFIX)
CFLAGS += $(if $(RTCONFIG_SAMBA36X),-DSMP)
endif

ifeq ($(CONFIG_REALTEK),y)
CFLAGS += -I$(TOP)/shared/sysdeps/realtek
endif

ifeq ($(RTCONFIG_ALPINE),y)
CFLAGS += -DALPINE_WPS
CFLAGS += -I$(TOP)/httpd/sysdeps -I$(TOP)/shared/sysdeps/alpine -I$(TOP)/wireless_tools$(WTOOL_SUFFIX)
endif

ifeq ($(RTCONFIG_LANTIQ),y)
CFLAGS += -DLANTIQ_WPS
CFLAGS += -I$(TOP)/shared/sysdeps/lantiq -I$(TOP)/wireless_tools$(WTOOL_SUFFIX)
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

LDFLAGS += -L$(TOP)/curl/lib/.libs -lcurl -L$(TOP)/
LDFLAGS += -L$(TOP)/libasc -lasc


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

CFLAGS += -I$(TOP)/openssl/include
ifeq ($(RTCONFIG_AMAS),y)
CFLAGS += -I$(TOP)/json-c
endif

LDFLAGS += -lm -L$(TOP)/shared -lshared -L$(TOP_PLATFORM)/nvram${BCMEX} -lnvram -L$(TOP)/json-c/.libs -ljson-c
LDFLAGS += -L$(TOP)/shared -lshared -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram 
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl

ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif
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

ifeq ($(RTCONFIG_CONNDIAG),y)
ifeq ($(RTCONFIG_LIB_CODB),y)
CFLAGS += -I$(TOP)/libcodb
LDFLAGS += -L$(TOP)/libcodb -lcodb
endif
CFLAGS += -I$(TOP)/rc
LDFLAGS += -L$(TOP)/rc -lconn_diag
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

ifeq ($(RTCONFIG_AMAS),y)
ifeq ($(RTCONFIG_LLDPD_1_0_11),y)
LLDPD_DIR = $(TOP)/lldpd-1.0.11
else
LLDPD_DIR = $(TOP)/lldpd-0.9.8
endif
CFLAGS += -I$(TOP)/amas-utils -I$(TOP)/shared/sysdeps/amas -pthread
LDFLAGS += -L$(TOP)/amas-utils -lamas-utils -lpthread
LDFLAGS += -L$(LLDPD_DIR)/src/lib/.libs -llldpctl -L$(STAGEDIR)/usr/lib/ -ljansson
endif

ifneq ($(AC2900),y)
ifeq ($(wildcard $(SRCBASE)/router/networkmap/*.c),)
all: 
	-cp -f prebuild/$(BUILD_NAME)/networkmap networkmap
	-cp -f prebuild/$(BUILD_NAME)/asusdiscovery asusdiscovery
	-cp -f prebuild/$(BUILD_NAME)/arpstorm arpstorm
ifeq ($(RTCONFIG_AMAS),y)
	-cp -f prebuild/$(BUILD_NAME)/find_cap find_cap
endif
else
ifeq ($(RTCONFIG_AMAS),y)
all: $(EXEC) asusdiscovery arpstorm find_cap
else
all: $(EXEC) asusdiscovery arpstorm
endif
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

ifeq ($(RTCONFIG_AMAS),y)
find_cap: packet.o ASUS_Discovery_for_find_cap.o ASUS_Discovery_Debug.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
endif

install:
	install -D $(EXEC) $(INSTALLDIR)/usr/sbin/$(EXEC)
	$(if $(RTCONFIG_DEBUG),,$(STRIP) $(INSTALLDIR)/usr/sbin/$(EXEC))
ifneq ($(AC2900),y)
	install -D asusdiscovery $(INSTALLDIR)/usr/sbin/asusdiscovery
	$(if $(RTCONFIG_DEBUG),,$(STRIP) $(INSTALLDIR)/usr/sbin/asusdiscovery)
ifeq ($(RTCONFIG_AMAS),y)
	install -D find_cap $(INSTALLDIR)/usr/sbin/find_cap
	$(if $(RTCONFIG_DEBUG),,$(STRIP) $(INSTALLDIR)/usr/sbin/find_cap)
endif
ifneq ($(wildcard $(SRCBASE)/router/networkmap/arpstorm),)
	install -D arpstorm $(INSTALLDIR)/usr/sbin/arpstorm
	$(if $(RTCONFIG_DEBUG),,$(STRIP) $(INSTALLDIR)/usr/sbin/arpstorm)
endif
endif
	install -d $(INSTALLDIR)/usr/networkmap
	cp -f db/networkmap.oui.js $(INSTALLDIR)/usr/networkmap/networkmap.oui.js
	cp -f db/nmp_conv_type.js $(INSTALLDIR)/usr/networkmap/nmp_conv_type.js
	cp -f db/nmp_vendor_type.js $(INSTALLDIR)/usr/networkmap/nmp_vendor_type.js
	cp -f db/nmp_bwdpi_type.js $(INSTALLDIR)/usr/networkmap/nmp_bwdpi_type.js

clean:
ifneq ($(AC2900),y)
	rm -rf *.o $(EXEC) *~ asusdiscovery arpstorm
ifeq ($(RTCONFIG_AMAS),y)
	rm -rf find_cap
endif
else
	rm -rf *.o $(EXEC) *~
endif
