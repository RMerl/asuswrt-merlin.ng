include ../common.mak
include $(SRCBASE)/router/.config

CFLAGS += -Wall -Os $(EXTRACFLAGS) -ffunction-sections -fdata-sections
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE) -I$(SRCBASE)/include -I./include -I$(TOP)/networkmap
CFLAGS += -I./sysdeps
CFLAGS += -I$(TOP)/openssl/include
CFLAGS += -I$(TOP)/json-c

LDFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
LDFLAGS += -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl
LDFLAGS += -lpthread
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
LDFLAGS += -lm

ifeq ($(RTCONFIG_AMAS),y)
CFLAGS += -I$(TOP)/amas-utils -I$(TOP)/shared/sysdeps/amas
LDFLAGS += -L$(TOP)/amas-utils -lamas-utils
LDFLAGS += -L$(TOP)/lldpd-0.9.8/src/lib/.libs -llldpctl -L$(STAGEDIR)/usr/lib/ -ljansson
ifeq ($(RTCONFIG_AVBLCHAN),y)
CFLAGS += -DAVBLCHAN
endif
CFLAGS += -DFREE_MEMORY_DOWNLOAD
endif

ifeq ($(HND_ROUTER),y)
CFLAGS += -DPTHREAD_STACK_SIZE_256K -DPTHREAD_EXIT
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm
else ifeq ($(RTCONFIG_QCA),y)
CFLAGS += -DPTHREAD_STACK_SIZE_256K -DPTHREAD_EXIT
endif

ifeq ($(RTCONFIG_HND_ROUTER_AX),y)
CFLAGS +=-I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/wl/sys
endif

ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
CFLAGS  += -I$(TOP)/sqlite
CFLAGS  += -I$(TOP)/nt_center/lib
CFLAGS  += -I$(TOP)/nt_center/actMail
CFLAGS  += -I$(TOP)/wlc_nt
LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3 -lpthread -L$(TOP)/nt_center/lib -lnt
LDFLAGS += -L$(TOP)/wlc_nt -lwlc_nt_client
endif

ifeq ($(RTCONFIG_DHDAP), y)
CFLAGS += -DDEBUG_WL
endif

ifeq ($(RTCONFIG_INTEL),y)
CFLAGS += -fPIC
endif
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
CFLAGS += -I$(SRCBASE)/common/include
else ifeq ($(RTCONFIG_RALINK),y)
CFLAGS += -I$(TOP)/shared/sysdeps/ralink -I$(TOP)/wireless_tools
else ifeq ($(RTCONFIG_QCA),y)
CFLAGS += -fPIC
else ifeq ($(RTCONFIG_REALTEK),y)
CFLAGS += -fPIC
endif

ifeq ($(RTCONFIG_SW_HW_AUTH),y)
CFLAGS  += -I$(TOP)/sw-hw-auth
endif

ifeq ($(RTCONFIG_LIBASUSLOG),y)
CFLAGS += -I$(TOP)/libasuslog
LDFLAGS += -L$(TOP)/libasuslog -lasuslog
endif

ifeq ($(RTCONFIG_RALINK),y)
COMMON_OBJS += ralink.o
vpath %.c sysdeps/ralink
else ifeq ($(RTCONFIG_QCA),y)
CFLAGS += -I$(TOP)/shared/sysdeps/qca -I$(TOP)/wireless_tools
COMMON_OBJS += qca.o
vpath %.c sysdeps/qca
else ifeq ($(RTCONFIG_REALTEK),y)
COMMON_OBJS += realtek.o
vpath %.c sysdeps/realtek
else ifeq ($(RTCONFIG_INTEL),y)
COMMON_OBJS += intel.o
vpath %.c sysdeps/intel
else ifeq ($(RTCONFIG_LANTIQ),y)
COMMON_OBJS += lantiq.o
CFLAGS += -fPIC
vpath %.c sysdeps/lantiq
else
COMMON_OBJS += broadcom.o
vpath %.c sysdeps/broadcom
endif

ifeq ($(RTCONFIG_AMAS),y)
COMMON_OBJS += chmgmt.o
endif
COMMON_OBJS += encrypt.o cfg_crc.o cfg_udp.o cfg_dencrypt.o cfg_common.o cfg_ipc.o cfg_wevent.o cfg_upgrade.o cfg_clientlist.o cfg_sched.o cfg_slavelist.o
COMMON_OBJS += cfg_capability.o cfg_action.o cfg_eventnotify.o
COMMON_OBJS += cfg_ctrllist.o
ifeq ($(RTCONFIG_DWB),y)
COMMON_OBJS += cfg_dwb.o
endif
COMMON_OBJS += cfg_chanspec.o
COMMON_OBJS += cfg_parammgnt.o
SERVER_OBJS += $(COMMON_OBJS) cfg_mnt.o
CLIENT_OBJS += $(COMMON_OBJS) cfg_client.o
REPORTSTATUS_OBJS += cfg_common.o cfg_reportstatus.o

ifeq ($(RTCONFIG_WIFI_SON),y)
CFLAGS += -DREPORT_PAP_INFO
endif
ifeq ($(RTCONFIG_AMAS),y)
COMMON_OBJS += cfg_roaming.o
CFLAGS += -DLEGACY_ROAMING
COMMON_OBJS += cfg_onboarding.o cfg_ethevent.o
CFLAGS += -DONBOARDING -DRSSI_LIST
ifeq ($(RTCONFIG_VIF_ONBOARDING),y)
CFLAGS += -DONBOARDING_VIA_VIF
endif
# for wireless channel sync
CFLAGS += -DSYNC_WCHANNEL
# for roaming info for sta
#COMMON_OBJS += cfg_roaminginfo.o
#CFLAGS += -DROAMING_INFO
# for master detection
#CFLAGS += -DMASTER_DET
# for radar detected
COMMON_OBJS += cfg_radardet.o
CFLAGS += -DRADAR_DET
COMMON_OBJS += cfg_conndiag.o
CFLAGS += -DCONN_DIAG
ifeq ($(RTCONFIG_STA_AP_BAND_BIND),y)
# for sta bind on AP
CFLAGS += -DSTA_BIND_AP
endif
endif

ifeq ($(RTCONFIG_AMASDB),y)
CFLAGS += -DDUAL_BAND_DETECTION
COMMON_OBJS += cfg_dualbandlist.o
endif
# for sending re join event
#ifeq ($(HND_ROUTER_AX),y)
#CFLAGS += -DNOTIFY_RE_JOIN_EVENT
#endif

ifeq ($(RTCONFIG_TCODE),y)
ifeq ($(RTCONFIG_CFGSYNC_LOCSYNC),y)
COMMON_OBJS += cfg_loclist.o
endif
endif

ifeq ($(RTCONFIG_HAS_5G_2), y)
CFLAGS += -DSUPPORT_TRI_BAND
else ifdef $(filter $(BUILD_NAME), RT-AC5300 GT-AC5300 RT-AC92U RT-AX92U GT-AX11000 RT-AX95Q RP-AC92 GT-AXE11000)
CFLAGS += -DSUPPORT_TRI_BAND
endif

ifeq ($(RTCONFIG_MASTER_DET),y)
CFLAGS += -DMASTER_DET
endif

ifeq ($(RTCONFIG_PRELINK),y)
CFLAGS += -DPRELINK
COMMON_OBJS += cfg_prelink.o
endif

ifeq ($(RTCONFIG_MSSID_PRELINK),y)
CFLAGS += -DMSSID_PRELINK
COMMON_OBJS += cfg_prelink.o
endif

ifeq ($(RTCONFIG_NBR_RPT),y)
COMMON_OBJS += cfg_nbr_rpt.o
endif

ifeq ($(RTCONFIG_LANTIQ),y)
# LDFLAGS += -L$(TOP)/fapi_wlan_common-1.0.0.1 -lfapiwlancommon -ldl -L$(TOP)/libhelper-1.4.0.2 -lhelper -lrt
LDFLAGS += -L$(TOP)/fapi_wlan_common-05.04.00.131 -lfapiwlancommon -ldl -L$(TOP)/libhelper-1.4.0.2 -lhelper -L$(TOP)/libfapi-0.1 -lfapi
LDFLAGS += -L$(TOP)/wlc_nt -lwlc_nt_client
endif

ifeq ($(RTCONFIG_LANTIQ),y)
CFLAGS += -I$(TOP)/shared/sysdeps/lantiq -I$(TOP)/wireless_tools -I$(TOP)/fapi_wlan_common-05.04.00.131/include -I$(TOP)/libhelper-1.4.0.2/include -DPACKAGE_ID=\"fapi_wlan_common\" -DLOGGING_ID="fapi_wlan_common"
CFLAGS  += -I$(TOP)/wlc_nt
endif

ifeq ($(RTCONFIG_BCMBSD),y)
CFLAGS += -DSMART_CONNECT
endif

ifeq ($(or $(HND_ROUTER_AX),$(RTCONFIG_WIFI_QCN5024_QCN5054)),y)
CFLAGS += -DSUPPORT_11AX
endif

ifeq ($(wildcard $(SRCBASE)/router/cfg_mnt/*.c),)
all:
	-cp -f prebuild/$(BUILD_NAME)/cfg_server cfg_server
	-cp -f prebuild/$(BUILD_NAME)/cfg_client cfg_client
	-cp -f prebuild/$(BUILD_NAME)/cfg_reportstatus cfg_reportstatus
	-cp -f prebuild/$(BUILD_NAME)/libcfgmnt.so libcfgmnt.so
else
all: cfg_server cfg_client cfg_reportstatus libcfgmnt.so
endif

cfg_server: $(SERVER_OBJS)
	echo " [cfg_mnt] CC $@"
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

cfg_client: $(CLIENT_OBJS)
	echo " [cfg_mnt] CC $@"
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

cfg_reportstatus: $(REPORTSTATUS_OBJS)
	echo " [cfg_mnt] CC $@"
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

libcfgmnt.so: cfg_lib.o
	@echo " [libcfgmnt] CC $@"
	@$(CC) -fPIC -shared $(CFLAGS) -o $@ $^

install: all
	@echo "[cfg_mnt] Installing..."
	@install -D cfg_server $(INSTALLDIR)/usr/sbin/cfg_server
	@install -D cfg_client $(INSTALLDIR)/usr/sbin/cfg_client
	@install -D cfg_reportstatus $(INSTALLDIR)/usr/sbin/cfg_reportstatus
	@install -D libcfgmnt.so $(INSTALLDIR)/usr/lib/libcfgmnt.so
	@install -D gencfgcert.sh $(INSTALLDIR)/usr/sbin/gencfgcert.sh
	@$(STRIP) $(INSTALLDIR)/usr/sbin/cfg_server
	@$(STRIP) $(INSTALLDIR)/usr/sbin/cfg_client
	@$(STRIP) $(INSTALLDIR)/usr/sbin/cfg_reportstatus
	@$(STRIP) $(INSTALLDIR)/usr/lib/libcfgmnt.so
	@chmod 0755 $(INSTALLDIR)/usr/sbin/cfg_server
	@chmod 0755 $(INSTALLDIR)/usr/sbin/cfg_client
	@chmod 0755 $(INSTALLDIR)/usr/sbin/cfg_reportstatus
	@chmod 0500 $(INSTALLDIR)/usr/sbin/gencfgcert.sh

clean:
	rm -f cfg_server cfg_client cfg_reportstatus *.so *.o .*.depend
