
include ../common.mak

ifeq ($(HND_ROUTER_BE_4916),y)
-include sdk_profile.mak
endif

ifeq ($(HND_ROUTER_BE_4916),y)
export RC_HOSTAPD_BE := y
endif

ifeq ($(HND_ROUTER_AX),y)
ifeq ($(BRCM_CHIP),$(filter $(BRCM_CHIP),4908 4912))
CFLAGS += -DBCM49XX
endif
endif
CFLAGS += -Wall $(if $(RTCONFIG_DEBUG),-g,-Os) $(EXTRACFLAGS)
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(TOP)/networkmap
ifneq ($(HND_ROUTER),y)
CFLAGS += -idirafter$(LINUXDIR)/include
endif
ifneq ($(or $(findstring linux-3.10.14,$(LINUXDIR))),)
CFLAGS += -idirafter$(LINUXDIR)/include/uapi
endif
ifneq ($(or $(findstring linux-3.10.108,$(LINUXDIR))),)
CFLAGS += -idirafter$(LINUXDIR)/include/uapi
endif
ifneq ($(or $(findstring linux-4.4.177,$(LINUXDIR))),)
CFLAGS += -idirafter$(LINUXDIR)/include/
endif
CFLAGS += $(PRIVATE_EXTRACFLAGS)
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include
ifneq ($(HND_ROUTER),y)
CFLAGS += -idirafter$(LINUXDIR)/include
endif
ifeq ($(RTCONFIG_DPSTA),y)
CFLAGS += -I$(TOP_PLATFORM)/dpsta
endif
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -DDLM -DASUS_DDNS -DASUS_EXT -DBTN_SETUP -DWEB_REDIRECT

CFLAGS += -DDEBUG_RCTEST

ifeq ($(RTCONFIG_WPS),y)
CFLAGS += -I$(SRCBASE)/wps/brcm_apps/include
endif

ifeq ($(RTCONFIG_EXT_RTL8365MB),y)
CFLAGS += -I$(LINUXDIR)/drivers/char/rtl8365mb
endif

ifeq ($(RTCONFIG_REALTEK),y)
CFLAGS += -Wl,--gc-sections -ldl
CFLAGS += -I$(TOP)/shared/sysdeps/realtek
ifneq ($(findstring CONFIG_RTL_USERSPACE_WTDOG=, $(shell cat $(LINUXDIR)/.config)),)
export CFLAGS += -DRTL_WTDOG
endif
else ifeq ($(RTCONFIG_RALINK),y)
CFLAGS += -I$(TOP)/shared/sysdeps/ralink -I$(TOP)/wireless_tools$(WTOOL_SUFFIX)
ifneq ($(findstring MAC_REPEATER=, $(shell cat $(LINUXDIR)/.config)),)
export CFLAGS += -DMAC_REPEATER
endif
ifneq ($(findstring CONFIG_RT2860V2_AP_EDCCA_MONITOR=, $(shell cat $(LINUXDIR)/.config)),)
export CFLAGS += -DCE_ADAPTIVITY
else
ifeq (y,$(filter y,$(RTN56UB1) $(RTN56UB2)))
export CFLAGS += -DCE_ADAPTIVITY
endif
endif
ifeq ($(RTCONFIG_RALINK_EDCCA),y)
export CFLAGS += -DCE_ADAPTIVITY
endif
endif

ifeq ($(RTCONFIG_QCA),y)
CFLAGS += -I$(TOP)/shared/sysdeps/qca -I$(TOP)/wireless_tools$(WTOOL_SUFFIX)
CFLAGS += $(if $(or $(RTCONFIG_SOC_IPQ8064),$(RTCONFIG_SOC_IPQ8074)),-DSMP)
endif
ifeq ($(RTCONFIG_WBD),y)
CFLAGS += -D__CONFIG_WBD__
endif

ifeq ($(RTCONFIG_ALPINE),y)
CFLAGS += -I$(TOP)/shared/sysdeps/alpine -I$(TOP)/wireless_tools$(WTOOL_SUFFIX) -I$(TOP)/mtd-utils/include/
endif

ifeq ($(RTCONFIG_LANTIQ),y)
CFLAGS += -I$(TOP)/shared/sysdeps/lantiq -I$(TOP)/wireless_tools$(WTOOL_SUFFIX) -I$(TOP)/fapi_wlan_common-05.04.00.131/include -I$(TOP)/libhelper-1.4.0.2/include -DPACKAGE_ID=\"fapi_wlan_common\" -DLOGGING_ID="fapi_wlan_common"
CFLAGS  += -I$(TOP)/wlc_nt
endif

ifeq ($(RTCONFIG_REALTEK),y)
LDFLAGS = -L$(TOP_PLATFORM)/nvram${BCMEX} -lnvram -L$(TOP)/shared -lshared -lcrypt -ldl
else
LDFLAGS = -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared -lcrypt -ldl
endif

ifeq ($(HND_ROUTER),y)
CFLAGS += -I$(HND_SRC)/userspace/public/include -I$(HND_SRC)/userspace/public/include/linux -I$(HND_SRC)/userspace/private/include
CFLAGS += -I$(HND_SRC)/shared/opensource/include/$(BRCM_BOARD)
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif
ifeq ($(or $(HND_ROUTER_AX_675X),$(HND_ROUTER_AX_6756),$(HND_ROUTER_BE_4916),$(HND_ROUTER_AX_6710),$(BCM_502L07P2)),y)
LDFLAGS += -L$(TOP_PLATFORM)/gen_util -lgen_util -L$(TOP_PLATFORM)/sys_util -lsys_util
endif
ifeq ($(HND_ROUTER_AX_6756),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm -L$(TOP_PLATFORM)/bcm_flashutil -lbcm_flashutil -L$(TOP_PLATFORM)/bcm_util $(if $(HND_ROUTER_AX_6756),-lbcm_util) -L$(TOP_PLATFORM)/bcm_boardctl -lbcm_boardctl
CFLAGS += -I$(TOP_PLATFORM)/bcm_flashutil -I$(HND_SRC)/userspace/public/include -I$(HND_SRC)/userspace/public/include/linux
else ifeq ($(HND_ROUTER_BE_4916),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm -L$(TOP_PLATFORM)/bcm_flashutil -lbcm_flashutil -L$(TOP_PLATFORM)/bcm_util $(if $(HND_ROUTER_BE_4916),-lbcm_util) -L$(TOP_PLATFORM)/bcm_boardctl -lbcm_boardctl
CFLAGS += -I$(TOP_PLATFORM)/bcm_flashutil -I$(HND_SRC)/userspace/public/include -I$(HND_SRC)/userspace/public/libs/cms_util/ -I$(HND_SRC)/userspace/public/include/linux
else
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm -L$(TOP_PLATFORM)/bcm_flashutil -lbcm_flashutil -L$(TOP_PLATFORM)/bcm_util $(if $(or $(HND_ROUTER_AX_675X),$(HND_ROUTER_AX_6710),$(BCM_502L07P2)),-lbcm_util,-lbcm_crc) -L$(TOP_PLATFORM)/bcm_boardctl -lbcm_boardctl
endif
endif
ifeq ($(HND_ROUTER_AX),y)
CFLAGS += -DWL11AC_80P80 -DWL11AC_160 -DBCMDONGLEHOST
CFLAGS += -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/math/include
CFLAGS += -I$(SRCBASE)/wl -I$(SRCBASE)/wl/sys -I$(SRCBASE)/../components/apps/wps/brcm_apps/include/
endif
ifeq ($(RTCONFIG_BRCM_HOSTAPD),y)
CFLAGS += -I$(SRCBASE)/../components/bcmcrypto/include
CFLAGS += -I$(HND_SRC)/bcmdrivers/opensource/include/bcm963xx -I$(HND_SRC)/shared/opensource/include/bcm963xx
CFLAGS += -I$(WPA_SUPPLICANT_DIR)/src/common
LDFLAGS += -L$(WPA_SUPPLICANT_DIR)/wpa_supplicant -L$(INSTALLDIR)/wpa_supplicant/usr/lib
WPS_PBCD := wps_pbcd
ifeq ($(or $(RTCONFIG_WIFI6E),$(HND_ROUTER_AX_6756),$(HND_ROUTER_BE_4916),$(RTCONFIG_BCM_502L07P2)),y)
ifeq ($(RC_HOSTAPD_BE),y)
ifeq ($(ASUSWRT_BRCM_SDK_VERSION),WIFI7_SDK_20231126)
OBJS_WPS_PBCD := $(if $(wildcard sysdeps/broadcom/wps_pbcd_be.c),wps_pbcd_be.o,prebuild/wps_pbcd_be.o)
OBJS_WPS_PBCD += $(if $(wildcard sysdeps/broadcom/hostapd_config_be.c),hostapd_config_be.o,prebuild/hostapd_config_be.o)
else ifeq ($(ASUSWRT_BRCM_SDK_VERSION),WIFI7_SDK_20240103)
OBJS_WPS_PBCD := $(if $(wildcard sysdeps/broadcom/wps_pbcd_be_369_1808.c),wps_pbcd_be_369_1808.o,prebuild/wps_pbcd_be_369_1808.o)
OBJS_WPS_PBCD += $(if $(wildcard sysdeps/broadcom/hostapd_config_be_369_1808.c),hostapd_config_be_369_1808.o,prebuild/hostapd_config_be_369_1808.o)
else ifeq ($(ASUSWRT_BRCM_SDK_VERSION),WIFI7_SDK_20230426)
OBJS_WPS_PBCD := $(if $(wildcard sysdeps/broadcom/wps_pbcd_be_332_21.c),wps_pbcd_be_332_21.o,prebuild/wps_pbcd_be_332_21.o)
OBJS_WPS_PBCD += $(if $(wildcard sysdeps/broadcom/hostapd_config_be_332_21.c),hostapd_config_be_332_21.o,prebuild/hostapd_config_be_332_21.o)
else # WIFI7_SDK_20230426
OBJS_WPS_PBCD := $(if $(wildcard sysdeps/broadcom/wps_pbcd_be_305_16.c),wps_pbcd_be_305_16.o,prebuild/wps_pbcd_be_305_16.o)
OBJS_WPS_PBCD += $(if $(wildcard sysdeps/broadcom/hostapd_config_be_305_16.c),hostapd_config_be_305_16.o,prebuild/hostapd_config_be_305_16.o)
endif # WIFI7_SDK_20230426
else
OBJS_WPS_PBCD := $(if $(wildcard sysdeps/broadcom/wps_pbcd_6e.c),wps_pbcd_6e.o,prebuild/wps_pbcd_6e.o)
OBJS_WPS_PBCD += $(if $(wildcard sysdeps/broadcom/hostapd_config_6e.c),hostapd_config_6e.o,prebuild/hostapd_config_6e.o)
endif
ifeq ($(HND_ROUTER_BE_4916),y)
CFLAGS += -DCONFIG_DRIVER_BRCM_MLO
endif
ifeq ($(RTCONFIG_AMAS_WGN), y)
OBJS_WPS_PBCD += $(if $(wildcard amas_wgn.c),amas_wgn.o,prebuild/amas_wgn.o)
endif
ifeq ($(RTCONFIG_MULTILAN_CFG), y)
OBJS_WPS_PBCD += $(if $(wildcard amas_apg.c),amas_apg.o,prebuild/amas_apg.o)
endif
else
OBJS_WPS_PBCD := $(if $(wildcard sysdeps/broadcom/wps_pbcd.c),wps_pbcd.o,prebuild/wps_pbcd.o)
endif
LDFLAGS += -lwpa_client
ifeq ($(or $(RTCONFIG_WIFI6E),$(RTCONFIG_BCM_502L07P2)),y)
# For now, enable HAPD_WDS only for router builds
# CFLAGS += -DHAPD_WDS_DRAFT
# For now, enable HAPD_WDS_HOTPLUG only for router builds
CFLAGS += -DHAPD_WDS
endif
vpath %.c sysdeps/broadcom
endif

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
CFLAGS += -I$(SRCBASE)/common/include
ifneq ($(findstring CONFIG_DUMP_PREV_OOPS_MSG=, $(shell cat $(LINUXDIR)/.config)),)
CFLAGS += -DDUMP_PREV_OOPS_MSG
endif
LDFLAGS += -L$(TOP)/libbcm -lbcm
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
# LDFLAGS += -L$(TOP)/fapi_wlan_common-1.0.0.1 -lfapiwlancommon -ldl -L$(TOP)/libhelper-1.4.0.2 -lhelper -lrt
LDFLAGS += -L$(TOP)/fapi_wlan_common-05.04.00.131 -lfapiwlancommon -ldl -L$(TOP)/libhelper-1.4.0.2 -lhelper -L$(TOP)/libfapi-0.1 -lfapi
LDFLAGS += -L$(TOP)/wlc_nt -lwlc_nt_client
endif

ifeq ($(RTCONFIG_HTTPS),y)
CFLAGS += -I$(TOP)/openssl/include
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl $(if $(RTCONFIG_OPENSSL11),-lpthread,)
endif

ifeq ($(RTCONFIG_BWDPI),y)
CFLAGS += -I$(TOP)/bwdpi_source/asus_include -I$(TOP)/bwdpi_source/src/tdts_udb_sample/linux
CFLAGS += -I$(TOP)/bwdpi_source/src/tdts_udb_sample
CFLAGS += -I$(TOP)/bwdpi_source/include
CFLAGS += -I$(TOP)/httpd/
CFLAGS += -I$(TOP)/util-linux/include
LDFLAGS += -L$(TOP)/bwdpi_source/asus -lbwdpi
endif

ifeq ($(RTCONFIG_SPEEDTEST),y)
LDFLAGS += -lm
LDFLAGS += -lpthread
endif

ifeq ($(RTCONFIG_BCMSMP),y)
CFLAGS += -DSMP
endif

ifeq ($(RTCONFIG_4352_5G),y)
CFLAGS += -DBCM4352
endif

CFLAGS += -I$(TOP)/libdisk
ifeq ($(RTCONFIG_USB),y)
CFLAGS += -DU2EC
LDFLAGS += -L$(TOP)/libdisk -ldisk
ifeq ($(BCM_MMC),y)
CFLAGS += -DBCM_MMC
endif
ifeq ($(RTCONFIG_USB_SMS_MODEM),y)
CFLAGS += -I$(TOP)/smspdu
LDFLAGS += -L$(TOP)/smspdu -lsmspdu
endif

ifeq ($(RTCONFIG_USB_MODEM),y)
CFLAGS += -DSET_USB_MODEM_MTU_ETH
CFLAGS += -DSET_USB_MODEM_MTU_PPP
ifeq ($(RTCONFIG_USB_SMS_MODEM),y)
CFLAGS += -I$(TOP)/smspdu
LDFLAGS += -L$(TOP)/smspdu -lsmspdu
endif
endif
endif

ifeq ($(RTCONFIG_HTTPS),y)
ifeq ($(RTCONFIG_FRS_FEEDBACK),y)
CFLAGS  += -I$(TOP)/openssl/include
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl
endif
endif

ifeq ($(RTCONFIG_FBWIFI),y)
LDFLAGS += -L$(TOP)/fb_wifi -lfbwifi
endif

ifeq ($(RTCONFIG_CAPTIVE_PORTAL),y)
CFLAGS  += -I$(TOP)/sqlite
endif

ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
CFLAGS  += -I$(TOP)/sqlite
CFLAGS  += -I$(TOP)/nt_center/lib
CFLAGS  += -I$(TOP)/nt_center/actMail
CFLAGS  += -DASUSWRT_SDK
endif

ifeq ($(RTCONFIG_ASD),y)
CFLAGS  += -I$(TOP)/asd
endif

ifeq ($(RTCONFIG_PROTECTION_SERVER),y)
CFLAGS  += -I$(TOP)/protect_srv/include
endif

ifeq ($(RTCONFIG_BT_CONN),y)
CFLAGS  += -I$(TOP)/bluez-5.56/lib
endif

#CFLAGS += -DRC_BUILDTIME=$(shell date -d "now" +"%s")

ifneq ($(findstring NET_CLS_POLICE=, $(shell cat $(LINUXDIR)/.config)),)
else
ifneq ($(findstring IMQ=, $(shell cat $(LINUXDIR)/.config)),)
export CFLAGS += -DCLS_ACT
endif
endif

ifneq ($(findstring DEVTMPFS=, $(shell cat $(LINUXDIR)/.config)),)
export CFLAGS += -DDEVTMPFS
endif

ifeq ($(RA_SKU),y)
export CFLAGS += -DRA_SINGLE_SKU
endif

ifeq ($(RA_SKU_IN_DRV),y)
export CFLAGS += -DRA_SINGLE_SKU
endif

ifeq ($(or $(RTAC68A),$(RT4GAC68U)),y)
CFLAGS += -DRTAC68U
endif

ifeq ($(or $(RTAC85U),$(RTAC85P),$(RTACRH26),$(TUFAC1750)),y)
TARGET_MODEL = $(subst -,,$(subst +,P,$(BUILD_NAME)))
CFLAGS += -D$(TARGET_MODEL)
endif

ifeq ($(RTCONFIG_PERMISSION_MANAGEMENT),y)
CFLAGS += -I$(TOP)/sqlCipher
CFLAGS += -I$(TOP)/PMS_DBapis
LDFLAGS += -L $(TOP)/sqlCipher/.libs -lsqlcipher
LDFLAGS += -L$(TOP)/PMS_DBapis -lpms_sql
endif

LDFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
ifeq ($(STATIC),1)
LDFLAGS += -static
endif
LDFLAGS += $(EXTRA_LDFLAGS)

ifeq ($(RTCONFIG_TRACEROUTE),y)
CFLAGS += -DRTCONFIG_TRACEROUTE
endif

ifeq ($(HND_ROUTER),y)
CFLAGS += -DNON_P_THREAD -DWATCHDOG_PERIOD2
endif

ifeq ($(RTCONFIG_CFGSYNC),y)
CFLAGS += -I$(TOP)/cfg_mnt
LDFLAGS += -L$(TOP)/cfg_mnt -lcfgmnt
endif

ifeq ($(RTCONFIG_SW_HW_AUTH), y)
CFLAGS += -I$(TOP)/sw-hw-auth
endif

ifneq ($(findstring CONFIG_BCM_KF_NETFILTER=, $(shell cat $(LINUXDIR)/.config)),)
ifeq ($(or $(HND_ROUTER_AX_6756),$(HND_ROUTER_BE_4916)),y)
else
CFLAGS         += -DBCM_KF_NETFILTER
endif
endif

ifeq ($(and $(BR63),$(NEW_SWITCH_ORDER)),y)
CFLAGS += -DNEW_SWITCH_ORDER
endif

ifeq ($(HND_ROUTER_BE_4916), y)
CFLAGS+= -fcommon
ifeq ($(ASUSWRT_BRCM_SDK_VERSION),WIFI7_SDK_20230426)
CFLAGS += -DWIFI7_SDK_20230426
endif
ifeq ($(ASUSWRT_BRCM_SDK_VERSION),WIFI7_SDK_20231126)
CFLAGS += -DWIFI7_SDK_20231126
endif
ifeq ($(ASUSWRT_BRCM_SDK_VERSION),WIFI7_SDK_20240103)
CFLAGS += -DWIFI7_SDK_20240103
endif
endif

ifeq ($(or $(HND_ROUTER_AX_6756),$(HND_ROUTER_BE_4916)),y)
CFLAGS += -DJFFS2_UBI_VOLUME_SIZE=$(JFFS2_UBI_VOLUME_SIZE)
endif

OBJS := rc.o init.o interface.o lan.o wireless.o $(if $(RTCONFIG_SOFTWIRE46), s46comm.o) wan.o pppd.o auth.o services.o
OBJS += firewall.o ppp.o services.o common.o conn_diag_log.o
OBJS += watchdog.o check_watchdog.o fwupg_flashing.o ntp.o udhcpc.o ate.o
ifeq ($(RTCONFIG_MULTIWAN_IF),y)
OBJS += qos_multi_wan_if.o
else ifeq ($(RTCONFIG_MULTIWAN_CFG),y)
OBJS += qos_multiwan.o
else
OBJS += qos.o
endif
OBJS += format.o
ifeq ($(RTCONFIG_SINGLEIMG_B),y)
OBJS += fwupg_flashing.o
endif
ifeq ($(RTCONFIG_CONNTRACK),y)
OBJS += pctime.o
endif

ifeq ($(RTCONFIG_NETOOL),y)
OBJS += netool.o linklist.o
LDFLAGS += -lpthread
endif

ifeq ($(RTAC1200G),y)
OBJS += mtd_sflash.o
else
OBJS += mtd.o
endif
ifeq ($(RTCONFIG_URLFW),y)
OBJS += mtd_curl.o
endif

ifeq ($(and $(CONFIG_BCMWL5),$(RTCONFIG_FANCTRL)),y)
OBJS += phy_tempsense.o
endif

ifeq ($(CONFIG_BCMWL5),y)
OBJS += init-broadcom.o wps-broadcom.o broadcom.o ate-broadcom.o
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RP-AX56))
OBJS += rpmac.o
endif
ifeq ($(RTCONFIG_AMAS),y)
OBJS += obd-broadcom.o
ifeq ($(RTCONFIG_BHCOST_OPT),y)
OBJS += amas-ssd-broadcom.o
endif
OBJS += amas-ssd-cd-broadcom.o
endif
ifeq ($(RTCONFIG_BCM_MFG),y)
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AC68U RT-AC3200 4G-AC68U GT-AC2900 RT-AX58U TUF-AX3000 TUF-AX5400 RT-AX82U RT-AX55 RT-AX1800 DSL-AX82U GS-AX3000 GS-AX5400 GT-AX6000))
OBJS += cfe.o
endif
else
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AC68U RT-AC3200 4G-AC68U GT-AC2900 RT-AX58U TUF-AX3000 RT-AX82U DSL-AX82U GS-AX3000 GS-AX5400 GT-AX6000))
OBJS += cfe.o
endif
endif
ifeq ($(RTCONFIG_ADTBW),y)
OBJS += adtbw-broadcom.o
endif
ifeq ($(RTCONFIG_AMAS_ADTBW),y)
OBJS += amas-adtbw-broadcom.o
endif
ifeq ($(RTCONFIG_DSL),y)
OBJS += init-broadcom-dsl.o
endif
else ifeq ($(RTCONFIG_REALTEK),y)
OBJS += init-realtek.o wps-realtek.o realtek.o  ate-realtek.o
ifeq ($(RTCONFIG_RTL8198D),y)
OBJS += rtl8198d.o
else
OBJS += rtk_wifi_drvmib.o
endif
ifeq ($(RTCONFIG_AMAS),y)
OBJS += obd-realtek.o
ifeq ($(RTCONFIG_BHCOST_OPT),y)
OBJS += amas-ssd-realtek.o
endif
endif
else ifeq ($(RTCONFIG_RALINK),y)
OBJS += init-ralink.o wps-ralink.o ralink.o ate-ralink.o
ifeq ($(RTACRH18),y)
OBJS += swlib.o
CFLAGS += -I$(TOP)/libnl-tiny-0.1/include
LDFLAGS += -L$(TOP)/libnl-tiny-0.1 -lnl-tiny
endif
ifeq ($(RTCONFIG_AMAS),y)
OBJS += obd-ralink.o
ifeq ($(RTCONFIG_BHCOST_OPT),y)
OBJS += amas-ssd-ralink.o
endif
endif
ifeq ($(RTCONFIG_CONCURRENTREPEATER),y)
OBJS += air_monitor.o
endif
ifeq ($(RTCONFIG_DSL),y)
OBJS += init-ralink-dsl.o
endif
else ifeq ($(RTCONFIG_QCA),y)
OBJS += init-qca.o wps-qca.o qca.o ate-qca.o ctl.obj
ifeq ($(RTCONFIG_AMAS),y)
OBJS += obd-qca.o
ifeq ($(RTCONFIG_BHCOST_OPT),y)
OBJS += amas-ssd-qca.o
endif
ifeq ($(RTCONFIG_CONNDIAG),y)
OBJS += $(if $(wildcard sysdeps/qca/amas-ssd-cd-qca.c),amas-ssd-cd-qca.o,prebuild/amas-ssd-cd-qca.o)
endif
endif
ifeq ($(RTCONFIG_WIFI_SON),y)
OBJS += repacd.o
endif
ifeq ($(RTCONFIG_QCA_EZMESH),y)
OBJS += ezmesh_1905.o
OBJS += ezmesh_file.o
endif
else ifeq ($(RTCONFIG_ALPINE),y)
OBJS += init-alpine.o wps-alpine.o alpine.o ate-alpine.o
ifeq ($(RTCONFIG_AMAS),y)
OBJS += obd-alpine.o
ifeq ($(RTCONFIG_BHCOST_OPT),y)
OBJS += amas-ssd-alpine.o
endif
endif
else ifeq ($(RTCONFIG_LANTIQ),y)
OBJS += init-lantiq.o lantiq.o ate-lantiq.o lantiq-wave.o
OBJS += $(if $(wildcard sysdeps/wps-lantiq.c),wps-lantiq.o,prebuild/wps-lantiq.o)
ifeq ($(RTCONFIG_AMAS),y)
OBJS += $(if $(wildcard sysdeps/lantiq/obd-lantiq.c),obd-lantiq.o,prebuild/obd-lantiq.o)
endif
endif

OBJS += $(if $(wildcard private.c),private.o,prebuild/private.o)

# TODO, speedtest
ifeq ($(RTCONFIG_SPEEDTEST),y)
OBJS += $(if $(wildcard speedtest.c),speedtest.o,prebuild/speedtest.o)
endif

OBJS += wanduck.o
OBJS += tcpcheck.o

ifeq ($(or $(RTCONFIG_SOC_IPQ8064),$(RTCONFIG_SOC_IPQ8074)),y)
CFLAGS += -DBUILD_READMEM
OBJS += readmem.o
endif

ifeq ($(RTCONFIG_USB),y)
OBJS += usb.o
ifneq ($(RTCONFIG_NO_USBPORT),y)
OBJS += usbled.o
endif
OBJS += usb_devices.o
endif

ifeq ($(RTCONFIG_JFFS2),y)
OBJS += jffs2.o
endif

ifeq ($(RTCONFIG_BRCM_NAND_JFFS2),y)
OBJS += jffs2.o
endif

ifeq ($(RTCONFIG_UBIFS),y)
OBJS += ubifs.o
endif

ifeq ($(RTCONFIG_YAFFS),y)
OBJS += yaffs.o
endif

ifeq ($(RTCONFIG_CIFS),y)
OBJS += cifs.o
endif

ifeq ($(RTCONFIG_SSH),y)
OBJS += ssh.o
endif

ifeq ($(RTCONFIG_WTFAST),y)
OBJS += wtfast.o
endif

ifeq ($(RTCONFIG_TCPLUGIN),y)
OBJS += qmacc.o
endif

ifeq ($(or $(RTCONFIG_PPTPD),$(RTCONFIG_ACCEL_PPTPD)),y)
OBJS += vpn.o
endif

ifeq ($(RTCONFIG_SNMPD),y)
OBJS += snmpd.o
endif
CFLAGS += -DRTCONFIG_CROND
ifeq ($(RTCONFIG_OPENVPN),y)
CFLAGS += -I$(TOP)/libovpn
LDFLAGS += -L$(TOP)/libovpn -lovpn
OBJS += ovpn.o openvpn.o
endif

ifeq ($(RTCONFIG_TPVPN),y)
CFLAGS += -I$(TOP)/libasc
CFLAGS += -I$(TOP)/libdisk
LDFLAGS += -L$(TOP)/libasc -lasc
OBJS += tpvpn.o
endif

ifeq ($(RTCONFIG_IPSEC),y)
OBJS += rc_ipsec.o
endif

ifeq ($(RTCONFIG_PARENTALCTRL),y)
OBJS += pc.o
OBJS += pc_block.o
OBJS += pc_tmp.o
ifeq ($(RTCONFIG_PC_REWARD),y)
OBJS += pc_reward.o
endif
endif

ifeq ($(RTCONFIG_INTERNETCTRL),y)
OBJS += ic.o
endif

ifeq ($(RTCONFIG_MXL371X), y)
CFLAGS += -I$(TOP)/mxl371x_sdk/MxLWare/lib -I$(TOP)/mxl371x_sdk/MxLWare/inc
LDFLAGS += -lclnkapi -L$(TOP)/mxl371x_sdk/MxLWare/lib
endif

ifeq ($(RTCONFIG_MOCA), y)
OBJS += moca.o
endif

#ifeq ($(RTCONFIG_HTTPS),y)
ifeq ($(RTCONFIG_FRS_FEEDBACK),y)
OBJS += dsl_fb.o
endif
#endif

#ifeq ($(RTCONFIG_FRS_LIVE_UPDATE),y)
# Link it, private.o needs read_asd_enc_file() from it
OBJS += frs_service.o
#endif

ifeq ($(and $(RTCONFIG_TUNNEL),$(RTCONFIG_ACCOUNT_BINDING)),y)
OBJS += asus_ddns.o
endif

ifeq ($(RTCONFIG_DSL),y)
OBJS += asustest.o
endif

ifeq ($(RTCONFIG_DSL),y)
OBJS += dsl.o
ifeq ($(RTCONFIG_RALINK),y)
OBJS += dsl-upg.o
endif
ifeq ($(RTCONFIG_DSL),y)
OBJS += dsl_diag.o
endif
ifeq ($(RTCONFIG_DSL_BCM),y)
OBJS += dsl-broadcom.o
endif
ifeq ($(RTCONFIG_DSL_HOST),y)
ifeq ($(RTCONFIG_DSL_BCM),y)
CFLAGS += -I$(HND_SRC)/bcmdrivers/broadcom/include/bcm963xx
endif
OBJS += dsld.o
endif
endif

ifeq ($(and $(RTCONFIG_BCMWL6),$(RTCONFIG_PROXYSTA)),y)
OBJS += arpreq.o
OBJS += psta_monitor.o
endif


ifeq ($(RTCONFIG_NBR_RPT),y)
OBJS += nbr_monitor.o
endif

ifeq ($(or $(RTCONFIG_BCMWL6),$(RTCONFIG_LANTIQ),$(RTCONFIG_QCA),$(RTCONFIG_REALTEK),$(RTCONFIG_RALINK)),y)
ifeq ($(RTCONFIG_AMAS),y)
ifeq ($(RTCONFIG_BHCOST_OPT),y)
vpath %.c amas
else
vpath %.c sysdeps/amas
endif
OBJS += obd.o
OBJS += obd_eth.o
OBJS += obd_monitor.o
CFLAGS  += -I$(TOP)/openssl/include
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl
ifeq ($(RTCONFIG_CFGSYNC),y)
ifeq ($(HND_ROUTER_AX),y)
OBJS += rmd.o
endif
endif
endif
endif

ifeq ($(RTCONFIG_WPS),y)
OBJS += wpsaide.o
endif

ifeq ($(or $(RTCONFIG_CONCURRENTREPEATER),$(RTCONFIG_AMAS)),y)
ifeq ($(RTCONFIG_RALINK),y)
OBJS += re_wpsc.o
endif
endif
ifeq ($(RTCONFIG_CONCURRENTREPEATER),y)
ifneq ($(RPAC68U),y)
OBJS += led_monitor.o
endif
endif

ifeq ($(RTN65U),y)
OBJS += asm1042.o
endif

ifeq ($(RTCONFIG_VPNC),y)
ifeq ($(RTCONFIG_VPN_FUSION),y)
OBJS += vpnc_fusion.o
ifeq ($(RTCONFIG_MULTILAN_CFG),y)
OBJS += sdn.o
OBJS += firewall_sdn.o
endif
else
OBJS += vpnc.o
endif
endif

ifeq ($(or $(HND_ROUTER_AX_6756),$(HND_ROUTER_BE_4916)),y)
ifeq ($(RTCONFIG_MULTILAN_CFG),y)
OBJS += sdn-interface-broadcom.o
endif
endif

ifeq ($(RTCONFIG_SHP),y)
CFLAGS += -DSHP
endif

ifeq ($(and $(RTCONFIG_USB),$(RTCONFIG_BCM_7114)),y)
SW_USBLED=y
endif

ifeq ($(RTCONFIG_DSL),y)
ifneq ($(RTCONFIG_DUALWAN),y)
SW_DSLWANLED=y
endif
endif

ifneq (,$(filter y,$(RTCONFIG_WLAN_LED) $(RTN18U) $(RTCONFIG_FAKE_ETLAN_LED) $(SW_USBLED) $(RTCONFIG_MMC_LED) $(RTCONFIG_BRCM_USBAP) $(RTAC66U) $(BCM4352) $(SW_DSLWANLED) $(RTCONFIG_SW_DEVLED)))
CFLAGS += -DSW_DEVLED
endif

ifeq ($(RTCONFIG_TIMEMACHINE),y)
OBJS += timemachine.o
endif

ifeq ($(RTCONFIG_LETSENCRYPT),y)
#CFLAGS += -DRTCONFIG_CROND
CFLAGS += -I$(TOP)/libletsencrypt
LDFLAGS += -L$(TOP)/libletsencrypt -lletsencrypt
endif

CFLAGS += $(if $(RTCONFIG_BLINK_LED),-I$(TOP)/bled)

export CFLAGS

ifeq ($(RTCONFIG_QTN),y)
OBJS += icmp.o
OBJS += qtn.o
OBJS += find_host_addr.o
OBJS += qtn_monitor.o
endif

ifeq ($(RTCONFIG_QSR10G),y)
OBJS += qsr10g.o
endif

ifeq ($(RTCONFIG_BWDPI),y)
OBJS += bwdpi.o
OBJS += bwdpi_check.o
OBJS += bwdpi_wred_alive.o
OBJS += bwdpi_db_10.o

OBJS += hour_monitor.o
endif

ifeq ($(RTCONFIG_AMAS),y)
OBJS += amas_lib.o
OBJS += linklist.o
endif

ifeq ($(RTCONFIG_TRAFFIC_LIMITER),y)
OBJS += traffic_limiter.o
endif

ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
OBJS += nt_mail.o
endif

ifeq ($(RTCONFIG_IPERF),y)
OBJS += monitor.o
endif

ifeq ($(or $(RTCONFIG_FRS_LIVE_UPDATE),$(RTCONFIG_FRS_FEEDBACK),$(RTCONFIG_URLFW),$(RTCONFIG_SOFTWIRE46),$(RTCONFIG_ACCOUNT_BINDING)),y)
CFLAGS += -I$(TOP)/curl-7.21.7/include
LDFLAGS += -L$(TOP)/curl-7.21.7/lib/.libs -lcurl
ifeq ($(RTCONFIG_TR069),y)
CFLAGS += -I$(TOP)/libssh2-1.10.0/include -I$(TOP)/zlib/include
LDFLAGS += -L$(TOP)/libssh2-1.10.0/src/.libs -lssh2 -L$(TOP)/zlib -lz
endif
endif

ifeq ($(RTCONFIG_HTTPS),y)
ifeq ($(or $(RTCONFIG_PUSH_EMAIL),$(RTCONFIG_NOTIFICATION_CENTER)),y)
OBJS += pwdec.o
endif
endif

ifeq ($(RTCONFIG_SOFTWIRE46),y)
CFLAGS += -I$(TOP)/json-c
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
OBJS += v6plusd.o ocnvcd.o dslited.o
endif

ifeq ($(RTCONFIG_INTERNAL_GOBI),y)
OBJS += lteled.o
endif

ifeq ($(RTCONFIG_TR069),y)
OBJS += tr069.o
endif

ifeq ($(RTCONFIG_PRESSURE_SENSOR),y)
OBJS += pressure.o
endif

ifeq ($(RTCONFIG_NEW_USER_LOW_RSSI),y)
ifeq ($(RTCONFIG_ADV_RAST),y)
ifeq ($(RTCONFIG_CFGSYNC),y)
CFLAGS += -I$(TOP)/cfg_mnt
LDFLAGS += -lm -lpthread
endif
endif
OBJS += roamast.o
ifeq ($(RTCONFIG_RALINK),y)
OBJS += roamast-ralink.o
endif
ifeq ($(CONFIG_BCMWL5),y)
OBJS += roamast-broadcom.o
endif
ifeq ($(RTCONFIG_REALTEK),y)
OBJS += roamast-realtek.o
endif
ifeq ($(RTCONFIG_LANTIQ),y)
CFLAGS += -I$(TOP)/hostapd-2.6/src/utils/
OBJS += roamast-lantiq.o
ifeq ($(RTCONFIG_AMAS),y)
OBJS += ../hostapd-2.6/src/common/wpa_ctrl.o
OBJS += ../hostapd-2.6/src/utils/os_unix.o
endif
endif
ifeq ($(RTCONFIG_QCA),y)
OBJS += roamast-qca.o
endif
endif

ifeq ($(RTCONFIG_TCODE),y)
OBJS += tcode_rc.o
ifeq ($(CONFIG_BCMWL5),y)
OBJS += tcode_brcm.o
endif
endif

ifeq ($(or $(RTCONFIG_LACP),$(RTCONFIG_BONDING)),y)
OBJS += $(if $(CONFIG_BCMWL5),agg_brcm.o)
endif

ifeq ($(and $(CONFIG_BCMWL5),$(RTCONFIG_DUALWAN)),y)
ifneq ($(HND_ROUTER),y)
OBJS += dualwan.o
endif
endif

ifeq ($(RTCONFIG_MULTIWAN_IF),y)
OBJS += multi_wan.o
OBJS += mtwanduck.o
OBJS += det_dns.o
OBJS += mtwan_detect_internet.o
ifeq ($(RTCONFIG_IPV6),y)
OBJS += multi_wan_ipv6.o
endif
endif

ifeq ($(RTCONFIG_ADTBW),y)
OBJS += adtbw.o
endif

ifeq ($(RTCONFIG_DHCP_OVERRIDE),y)
OBJS += detectWAN_arp.o
endif

ifeq ($(RTCONFIG_KEY_GUARD),y)
OBJS += keyguard.o
endif

ifeq ($(RTCONFIG_PORT_BASED_VLAN),y)
OBJS += vlan.o
endif

ifeq ($(RTCONFIG_TAGGED_BASED_VLAN),y)
OBJS += tagged_based_vlan.o
endif

ifeq ($(RTCONFIG_DNSFILTER),y)
OBJS += dnsfilter.o
endif

ifeq ($(RTCONFIG_NTPD),y)
OBJS += ntpd.o
else
OBJS += ntp.o
endif

ifeq ($(RTCONFIG_DNSFILTER),y)
OBJS += dnsfilter.o
endif

ifeq ($(RTCONFIG_AMAS_WGN), y)
OBJS += amas_wgn.o
endif

#Tencent download
ifeq ($(RTCONFIG_TC_DOWNLOAD), y)
OBJS += tc_utils.o
CFLAGS += -D _FILE_OFFSET_BITS=64
endif

ifeq ($(RTCONFIG_MULTILAN_CFG), y)
OBJS += amas_apg.o
endif

# _FILE_OFFSET_BITS=64 is defined in shared, it must be defined in rc accordingly if shared pass pointer to struct dirent to rc,
# e.g., readdir_wrapper() v.s. find_uboot_for_next_fw().
CFLAGS += $(if $(RTCONFIG_QCA),-D _FILE_OFFSET_BITS=64)

ifeq ($(RTCONFIG_HND_ROUTER_AX),y)
OBJS += gpy211_war.o
endif

ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_QTN),y)
vpath %.c sysdeps sysdeps/broadcom sysdeps/qtn $(TOP)/libqcsapi_client/qcsapi_rpc_common/client $(TOP)/sysdeps/qtn $(SRCBASE)/shared
else
vpath %.c sysdeps sysdeps/broadcom $(SRCBASE)/shared
endif
else ifeq ($(RTCONFIG_RALINK),y)
vpath %.c sysdeps sysdeps/ralink
else ifeq ($(RTCONFIG_QCA),y)
vpath %.c sysdeps sysdeps/qca
else ifeq ($(RTCONFIG_REALTEK),y)
vpath %.c sysdeps sysdeps/realtek sysdeps/realtek/mib_adapter $(SRCBASE)/shared
else ifeq ($(RTCONFIG_ALPINE),y)
vpath %.c sysdeps sysdeps/alpine
else ifeq ($(RTCONFIG_LANTIQ),y)
vpath %.c sysdeps sysdeps/lantiq
else
vpath %.c sysdeps sysdeps/broadcom
endif

ifeq ($(RTCONFIG_TUNNEL),y)
OBJS += natnl_api.o
CFLAGS += -I$(TOP)/aaews
endif

ifneq ($(RTCONFIG_RALINK)$(RTCONFIG_QCA)$(RTCONFIG_REALTEK),y)
OBJS += erp_monitor.o
endif
OBJS += $(if $(RTCONFIG_SOC_IPQ8074),erp_monitor.o)

ifeq ($(RTCONFIG_GN_WBL),y)
OBJS += gn_wbl.o
endif

ifeq ($(RCONFIG_SW_HW_AUTH), y)
CLFAGS += -I(TOP)/sw-hw-auth
endif

ifeq ($(RTCONFIG_AMAS),y)
ifeq ($(RTCONFIG_BHCOST_OPT),y)
vpath %.c amas
CFLAGS += -I./amas
else
vpath %.c sysdeps/amas
endif
ifeq ($(RTCONFIG_LLDPD_1_0_11),y)
LLDPD_DIR = $(TOP)/lldpd-1.0.11
else
LLDPD_DIR = $(TOP)/lldpd-0.9.8
endif
LDFLAGS += -L$(TOP)/amas-utils -lamas-utils -lpthread -L$(LLDPD_DIR)/src/lib/.libs -llldpctl -L$(STAGEDIR)/usr/lib/ -ljansson
CFLAGS += -I$(TOP)/amas-utils -I$(TOP)/shared/sysdeps/amas -D_REENTRANT -pthread -DPTHREAD_STACK_SIZE_128K -DUSE_GET_TLV_SUPPORT_MAC
OBJS += amas_wlcconnect.o
OBJS += amas_bhctrl.o
ifeq ($(RTCONFIG_BHCOST_OPT),y)
OBJS += amas_status.o
OBJS += amas_misc.o
endif
OBJS += amas_lanctrl.o
ifeq ($(RTCONFIG_AMAS_ADTBW),y)
OBJS += amas_adtbw.o
endif
ifeq ($(RTCONFIG_BHCOST_OPT),y)
OBJS += amas_ssd.o amas_ipc.o
endif
ifeq ($(RTCONFIG_CONNDIAG),y)
OBJS += amas_ssd_cd.o
endif
ifeq ($(RTCONFIG_PRELINK),y)
OBJS += amas_prelink.o
endif
endif
ifeq ($(RTCONFIG_LIBASUSLOG),y)
LDFLAGS += -L$(TOP)/libasuslog -lasuslog
CFLAGS += -I$(TOP)/libasuslog
endif

ifeq ($(RTCONFIG_BRCM_HOSTAPD),y)
ifeq ($(or $(RTCONFIG_WIFI6E),$(HND_ROUTER_AX_6756),$(HND_ROUTER_BE_4916),$(RTCONFIG_BCM_502L07P2)),y)
ifeq ($(RC_HOSTAPD_BE),y)
ifeq ($(ASUSWRT_BRCM_SDK_VERSION),WIFI7_SDK_20231126)
OBJS += $(if $(wildcard sysdeps/broadcom/hostapd_config_be.c),hostapd_config_be.o,prebuild/hostapd_config_be.o)
else ifeq ($(ASUSWRT_BRCM_SDK_VERSION),WIFI7_SDK_20240103)
OBJS += $(if $(wildcard sysdeps/broadcom/hostapd_config_be_369_1808.c),hostapd_config_be_369_1808.o,prebuild/hostapd_config_be_369_1808.o)
else ifeq ($(ASUSWRT_BRCM_SDK_VERSION),WIFI7_SDK_20230426)
OBJS += $(if $(wildcard sysdeps/broadcom/hostapd_config_be_332_21.c),hostapd_config_be_332_21.o,prebuild/hostapd_config_be_332_21.o)
else # WIFI7_SDK_20230426
OBJS += $(if $(wildcard sysdeps/broadcom/hostapd_config_be_305_16.c),hostapd_config_be_305_16.o,prebuild/hostapd_config_be_305_16.o)
endif # WIFI7_SDK_20230426
else # RC_HOSTAPD_BE
OBJS += $(if $(wildcard sysdeps/broadcom/hostapd_config_6e.c),hostapd_config_6e.o,prebuild/hostapd_config_6e.o)
endif # RC_HOSTAPD_BE
else
OBJS += $(if $(wildcard sysdeps/broadcom/hostapd_config.c),hostapd_config.o,prebuild/hostapd_config.o)
endif
vpath %.c sysdeps/broadcom

endif

CFLAGS += -I$(TOP)/wlceventd
LDFLAGS2 = $(LDFLAGS)

ifeq ($(RTCONFIG_NFCM),y)
## added by Andrew (2011/10/08)
OBJS += alt_watchdog.o
CFLAGS += -I$(TOP)/libev-4.33
LDFLAGS2 += -L$(TOP)/libev-4.33/.libs -lev
## end of add
endif

ifeq ($(RTCONFIG_QCA_LBD),y)
OBJS += $(if $(wildcard sysdeps/qca/lbd-qca.c),lbd-qca.o,prebuild/lbd-qca.o)
endif

ifeq ($(RTCONFIG_MTK_BSD),y)
OBJS += $(if $(wildcard sysdeps/ralink/bsd-ralink.c),bsd-ralink.o,prebuild/bsd-ralink.o)
endif


ifeq ($(BUILD_NAME), RT-AC1200_V2)
CFLAGS += -DRTAC1200V2
endif

ifeq ($(BUILD_NAME), RT-ACRH18)
CFLAGS += -DRTACRH18
endif

ifeq ($(BUILD_NAME), 4G-AX56)
CFLAGS += -DRT4GAX56
endif		

ifeq ($(BUILD_NAME), RT-AX53U)
CFLAGS += -DRTAX53U
endif

ifeq ($(BUILD_NAME), RT-AX54)
CFLAGS += -DRTAX54
endif

ifeq ($(BUILD_NAME), 4G-AC86U)
CFLAGS += -DRT4GAC86U
endif

ifeq ($(BUILD_NAME), XD4S)
CFLAGS += -DXD4S
endif

ifeq ($(RTCONFIG_ISP_CUSTOMIZE),y)
OBJS += $(if $(wildcard customize_rc.c),customize_rc.o,prebuild/customize_rc.o)
endif

ifeq ($(RTCONFIG_CONNDIAG),y)
ifeq ($(RTCONFIG_BCMBSD),y)
CFLAGS += -I$(TOP)/bsd
endif
ifeq ($(or $(RTCONFIG_LANTIQ),$(RTAC59_CD6R),$(RTAC59_CD6N),$(TUFAX4200),$(TUFAX6000),$(RTAX59U)),y)
CFLAGS += -fPIC
endif
ifeq ($(RTCONFIG_HND_ROUTER_AX),y)
CFLAGS += $(ACSDV2_DIR)
endif
CFLAGS += -I$(TOP)/acsd$(BCMEX)$(EX7)
CFLAGS  += -I$(TOP)/sqlite
LDFLAGS2 += -L$(TOP)/sqlite/.libs -lsqlite3

ifeq ($(RTCONFIG_LIB_CODB),y)
CFLAGS += -I$(TOP)/libcodb
LDFLAGS2 += -L$(TOP)/libcodb -lcodb
endif

LDFLAGS2 += -L. -lconn_diag

OBJS += conn_diag.o
ifeq ($(CONFIG_BCMWL5),y)
OBJS += conn_diag-broadcom.o
else ifeq ($(RTCONFIG_LANTIQ),y)
OBJS += conn_diag-lantiq.o
else ifeq ($(RTCONFIG_QCA),y)
OBJS += conn_diag-qca.o
else ifeq ($(RTCONFIG_RALINK),y)
OBJS += conn_diag-ralink.o
endif
DIAG_OBJS += conn_diag-sql.o
DIAG_OBJS += conn_diag_log.o
endif

ifeq ($(BUILD_NAME), GT10)
export GT10=y
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AX82U RT-AX82U_V2 GS-AX3000 GS-AX5400 TUF-AX5400 TUF-AX5400_V2 GT-AX11000_PRO GT-AXE16000 GT-BE98 GT-AX6000 GT10 GT-BE98_PRO GT-BE96 GT-BE19000))
LDFLAGS += -lm
OBJS += ledg.o
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AX82U RT-AX82U_V2 GS-AX3000 GS-AX5400 TUF-AX5400 TUF-AX5400_V2 GT-AX11000_PRO GT-AXE16000 GT-AX6000 GT10))
OBJS += ledbtn.o
endif
endif
ifeq ($(DSL_AX82U),y)
OBJS += ledg.o
endif
ifeq ($(GTAX6000),y)
OBJS += antled.o
endif
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),GT-BE98 GT-BE98_PRO GT-BE96 GT-BE19000 RT-BE58U TUF-BE3600 RT-BE92U RT-BE95U))
OBJS += rtkmonitor.o
endif

OBJS += dhcpc_vc_query.o
CFLAGS += -I$(TOP)/json-c
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c

ifeq ($(RTCONFIG_MULTISERVICE_WAN),y)
ifeq ($(CONFIG_BCMWL5),y)
OBJS += mswan-broadcom.o
else ifeq ($(RTCONFIG_RALINK),y)
OBJS += mswan-ralink.o
endif
endif

ifeq ($(RTCONFIG_BCM_OAM),y)
OBJS += oam.o
endif

ifeq ($(RTCONFIG_SCHED_DAEMON),y)
OBJS += sched_daemon.o
endif

ifeq ($(RTCONFIG_TIME_QUOTA),y)
OBJS += time_quota.o
endif

ifeq ($(RTCONFIG_WIREGUARD),y)
CFLAGS += -DRTCONFIG_CROND
OBJS += wireguard.o
endif

ifeq ($(RTCONFIG_QCA_PLC2),y)
OBJS += detect_plc.o
endif

ifeq ($(RTCONFIG_GRE),y)
OBJS += gre.o
endif

ifeq ($(RTCONFIG_IG_SITE2SITE),y)
OBJS += ig_s2s_client.o
endif

ifeq ($(RTCONFIG_TRUSTZONE),y)
CFLAGS += -I$(TOP)/ateed/
LDFLAGS += -L$(TOP)/ateed/ -latee
endif

all: PB rc $(if $(RTCONFIG_BRCM_HOSTAPD), $(WPS_PBCD)) init.prep

PB:
	[ ! -d prebuild ] || cp prebuild/*.o ./
ifeq ($(RTCONFIG_REALTEK),y)
ifneq ($(wildcard ./prebuild/rtk_wifi_drvmib.h),)
	@echo " [rc] PB_REALTEK "
	[ ! -d prebuild ] || mkdir -p sysdeps/realtek/mib_adapter
	cp prebuild/rtk_wifi_drvmib.h ./sysdeps/realtek/mib_adapter/rtk_wifi_drvmib.h
	rm -rf ./rtk_wifi_drvmib.h
endif
endif

ifeq ($(RTCONFIG_BRCM_HOSTAPD),y)
$(WPS_PBCD): $(OBJS_WPS_PBCD)
	@$(CC) -o $@ $(OBJS_WPS_PBCD) $(LDFLAGS)
endif

rc: $(OBJS) $(if $(RTCONFIG_CONNDIAG),libconn_diag.so,)
	@echo " [rc] CC $@"
	@$(CC) -o $@ $^ $(LDFLAGS2) $(CFLAGS)

	$(SIZECHECK)
	$(CPTMP)

install: all
	@echo "[rc] Installing..."
	@install -D rc $(INSTALLDIR)/sbin/rc
	$(if $(RTCONFIG_DEBUG),,@$(STRIP) $(INSTALLDIR)/sbin/rc)
	@chmod 0500 $(INSTALLDIR)/sbin/rc
ifeq ($(RTCONFIG_BRCM_HOSTAPD),y)
	@install $(WPS_PBCD) $(INSTALLDIR)/sbin
	@$(STRIP) $(INSTALLDIR)/sbin/$(WPS_PBCD)
endif

ifeq ($(RTCONFIG_CONNDIAG),y)
	@install -D libconn_diag.so $(INSTALLDIR)/usr/lib/libconn_diag.so
	$(if $(RTCONFIG_DEBUG),,@$(STRIP) $(INSTALLDIR)/usr/lib/libconn_diag.so)
endif

ifeq ($(RTCONFIG_PRESSURE_SENSOR),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc pressure
endif

ifeq ($(RTCONFIG_BCMARM),y)
ifneq ($(HND_ROUTER),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc preinit
else
	@cd $(INSTALLDIR)/sbin && ln -sf rc init
endif
else
	@cd $(INSTALLDIR)/sbin && ln -sf rc init
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc console
	@cd $(INSTALLDIR)/sbin && ln -sf rc hotplug
	@cd $(INSTALLDIR)/sbin && ln -sf rc service
	@cd $(INSTALLDIR)/sbin && ln -sf rc rcheck
	@cd $(INSTALLDIR)/sbin && ln -sf rc radio
	@cd $(INSTALLDIR)/sbin && ln -sf rc led_ctrl
ifeq ($(RT4GAC86U),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc led_modem
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc led_str_ctrl
	@cd $(INSTALLDIR)/sbin && ln -sf rc reboot
	@cd $(INSTALLDIR)/sbin && ln -sf rc halt
	@cd $(INSTALLDIR)/sbin && ln -sf rc netool
ifeq ($(HND_ROUTER),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc hnd-write
	@cd $(INSTALLDIR)/sbin && ln -sf rc hnd-erase
ifneq ($(BUILD_NAME), RT-AC68U_V4)
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd_erase_image_update
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd_erase_misc2
else
	@cd $(INSTALLDIR)/sbin && ln -sf rc nvram_erase
ifeq ($(RTCONFIG_BCMARM),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc gpio
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd-write2
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd-erase2
else
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd-write
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd-erase
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd-unlock
endif
ifeq ($(RTCONFIG_DUAL_TRX2),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc fixdmgfw
endif
#	@cd $(INSTALLDIR)/sbin && ln -sf rc ots
	@cd $(INSTALLDIR)/sbin && ln -sf rc watchdog
	@cd $(INSTALLDIR)/sbin && ln -sf rc check_watchdog
ifeq ($(RTCONFIG_NFCM),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc alt_watchdog
endif
ifeq ($(RTCONFIG_SINGLEIMG_B),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc fwupg_flashing
endif
ifeq ($(RTCONFIG_CONNTRACK),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc pctime
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc wave_monitor
ifeq ($(RTCONFIG_LANTIQ),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc restart_bluetoothd
	@cd $(INSTALLDIR)/sbin && ln -sf rc set_usb3_to_usb2
	@cd $(INSTALLDIR)/sbin && ln -sf rc set_usb2_to_usb3
	@cd $(INSTALLDIR)/sbin && ln -sf rc start_repeater
endif
ifneq ($(RTCONFIG_QCA)$(RTCONFIG_RALINK)$(RTCONFIG_ALPINE)$(RTCONFIG_LANTIQ),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc watchdog02
endif
ifneq (,$(filter y,$(RTCONFIG_WLAN_LED) $(RTN18U) $(RTCONFIG_FAKE_ETLAN_LED) $(SW_USBLED) $(RTCONFIG_MMC_LED) $(RTCONFIG_BRCM_USBAP) $(RTAC66U) $(BCM4352) $(SW_DSLWANLED) $(RTCONFIG_SW_DEVLED)))
	@cd $(INSTALLDIR)/sbin && ln -sf rc sw_devled
endif
ifeq ($(RTAC1200G),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc wdg_monitor
endif
ifeq ($(and $(CONFIG_BCMWL5),$(RTCONFIG_FANCTRL)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc phy_tempsense
endif

ifeq ($(and $(RTCONFIG_BCMWL6),$(RTCONFIG_PROXYSTA)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc sendarp
	@cd $(INSTALLDIR)/sbin && ln -sf rc psta_monitor
endif

ifeq ($(RTCONFIG_NBR_RPT),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc nbr_monitor
endif

ifeq ($(or $(RTCONFIG_BCMWL6),$(RTCONFIG_LANTIQ),$(RTCONFIG_QCA),$(RTCONFIG_REALTEK),$(RTCONFIG_RALINK)),y)
ifeq ($(RTCONFIG_AMAS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc obd
	@cd $(INSTALLDIR)/sbin && ln -sf rc obd_eth
	@cd $(INSTALLDIR)/sbin && ln -sf rc obd_monitor
endif
endif
ifeq ($(RTCONFIG_LANTIQ),y)
ifeq ($(RTCONFIG_AMAS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc obd
endif
endif
ifeq ($(RTCONFIG_QTN),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc qtn_monitor
	@cd $(INSTALLDIR)/sbin && ln -sf rc start_psta_qtn
	@cd $(INSTALLDIR)/sbin && ln -sf rc start_ap_qtn
	@cd $(INSTALLDIR)/sbin && ln -sf rc start_channel_scan_qtn
	@cd $(INSTALLDIR)/sbin && ln -sf rc start_qtn_stateless
	@cd $(INSTALLDIR)/sbin && ln -sf rc restart_qtn
endif
ifneq ($(RTCONFIG_NTPD),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc ntp
else
	@cd $(INSTALLDIR)/sbin && ln -sf rc ntpd_synced
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc autodet
ifeq ($(RTCONFIG_DETWAN),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc detwan
endif
ifeq ($(RTCONFIG_AUTO_WANPORT),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc autowan
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc ddns_updated
	@cd $(INSTALLDIR)/sbin && ln -sf rc ddns_custom_updated

#	@cd $(INSTALLDIR)/sbin && ln -sf rc wldist

ifeq ($(RTCONFIG_CONCURRENTREPEATER),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc led_monitor
endif
ifeq ($(or $(RTCONFIG_CONCURRENTREPEATER),$(RTCONFIG_AMAS)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc re_wpsc
endif

ifeq ($(RTCONFIG_WPS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc wpsaide
ifeq ($(RTCONFIG_QCA),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_wps_er
ifeq ($(RTCONFIG_CFG80211),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc vap_evhandler
endif
endif
endif
ifeq ($(RTCONFIG_QCA_PLC_UTILS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc autodet_plc
endif
ifeq ($(MAPAC2200),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc dpdt_ant
endif
ifeq ($(or $(MAPAC1300),$(VZWAC1300)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc thermal_txpwr
endif

#	ifeq ($(RTCONFIG_HEARTBEAT),y)
#		@cd $(INSTALLDIR)/sbin && ln -sf rc hb_connect
#		@cd $(INSTALLDIR)/sbin && ln -sf rc hb_disconnect
#		@cd $(INSTALLDIR)/sbin && ln -sf rc hb_idle
#	endif

ifeq ($(RTCONFIG_CIFS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc mount-cifs
endif

ifeq ($(RTCONFIG_RALINK),y)
ifeq ($(RTCONFIG_DSL),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc convert_dsl_wan_settings
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc rtkswitch
ifeq ($(RTAC53)$(RTAC51UP)$(RT4GAC86U),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtkswitch
endif
endif

ifeq ($(RTCONFIG_EXT_RTL8365MB),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc rtkswitch
endif
ifeq ($(RTCONFIG_EXT_RTL8370MB),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc rtkswitch
endif
#ifeq ($(RTCONFIG_EXT_RTL8367S),y)
#	@cd $(INSTALLDIR)/sbin && ln -sf rc rtkswitch
#endif
ifeq ($(or $(RTAX55),$(RTAX1800),$(RTAX58U_V2),$(RTAX3000N)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc rtkswitch
endif
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),BR63 GT-BE98 GT-BE98_PRO GT-BE96 RT-BE58U TUF-BE3600 GT-BE19000 RT-BE92U RT-BE95U))
	@cd $(INSTALLDIR)/sbin && ln -sf rc rtkswitch
endif
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),GT-BE98 GT-BE98_PRO GT-BE96 GT-BE19000 RT-BE58U TUF-BE3600 RT-BE92U RT-BE95U))
	@cd $(INSTALLDIR)/sbin && ln -sf rc rtkmonitor
endif
ifeq ($(RTCONFIG_QCA),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc rtkswitch
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc delay_exec

	@cd $(INSTALLDIR)/sbin && ln -sf rc wanduck
ifeq ($(RTCONFIG_MULTIWAN_IF),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtwanduck
	@cd $(INSTALLDIR)/sbin && ln -sf rc det_dns
endif
ifeq ($(RTCONFIG_CONNDIAG),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc conn_diag
	@cd $(INSTALLDIR)/sbin && ln -sf rc diag_data
endif
ifeq ($(and $(CONFIG_BCMWL5),$(RTCONFIG_DUALWAN)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc dualwan
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc tcpcheck
	@cd $(INSTALLDIR)/sbin && ln -sf rc auto_macclone
ifeq ($(RTCONFIG_USB),y)
ifneq ($(RTCONFIG_NO_USBPORT),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc usbled
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_apps_name
ifeq ($(BCM_MMC),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc asus_mmc
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc asus_sd
	@cd $(INSTALLDIR)/sbin && ln -sf rc asus_lp
	@cd $(INSTALLDIR)/sbin && ln -sf rc asus_sg
	@cd $(INSTALLDIR)/sbin && ln -sf rc asus_sr
	@cd $(INSTALLDIR)/sbin && ln -sf rc asus_tty
	@cd $(INSTALLDIR)/sbin && ln -sf rc asus_usbbcm
	@cd $(INSTALLDIR)/sbin && ln -sf rc asus_usb_interface
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_usb_node_by_string
	@cd $(INSTALLDIR)/sbin && ln -sf rc unset_usb_nvram
	@cd $(INSTALLDIR)/sbin && ln -sf rc detect_usb_devices
	@cd $(INSTALLDIR)/sbin && ln -sf rc usb_notify
	@cd $(INSTALLDIR)/sbin && ln -sf rc ejusb
ifeq ($(RTCONFIG_DISK_MONITOR),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc disk_monitor
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc disk_remove
ifeq ($(RTCONFIG_APP_PREINSTALLED)$(RTCONFIG_APP_NETINSTALLED),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc run_app_script
	@cd $(INSTALLDIR)/sbin && ln -sf rc chk_app_state
endif
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc restart_wireless
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AX55 RT-AX1800 RT-AX58U_V2 RT-AX3000N BR63 GT-BE98 GT-BE98_PRO GT-BE96 RT-BE58U TUF-BE3600 GT-BE19000 RT-BE92U RT-BE95U))
	@cd $(INSTALLDIR)/sbin && ln -sf rc config_switch
endif
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),GT-BE98 GT-BE98_PRO GT-BE96 RT-BE58U TUF-BE3600 GT-BE19000 RT-BE92U RT-BE95U))
	@cd $(INSTALLDIR)/sbin && ln -sf rc config_extwan
	@cd $(INSTALLDIR)/sbin && ln -sf rc reset_extwan
endif
ifeq ($(RTCONFIG_HW_DOG), y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc hwdog
endif
ifeq ($(RTCONFIG_PTHSAFE_POPEN), y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc PS_pod
endif
ifeq ($(RTCONFIG_WIFI_SON),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc hive_cap
	@cd $(INSTALLDIR)/sbin && ln -sf rc hive_re
	@cd $(INSTALLDIR)/sbin && ln -sf rc wifimon_check
	@cd $(INSTALLDIR)/sbin && ln -sf rc hive_cmd
	@cd $(INSTALLDIR)/sbin && ln -sf rc hive_wsplcd
	@cd $(INSTALLDIR)/sbin && ln -sf rc hive_hyd
ifeq ($(RTCONFIG_ETHBACKHAUL),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc eth_bh_mon
	@cd $(INSTALLDIR)/sbin && ln -sf rc ethbh_peer_detect
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc hive_eth
endif

ifeq ($(RTCONFIG_LP5523),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc lp55xx_set_led
	@cd $(INSTALLDIR)/sbin && ln -sf rc lp55xx_user_set
endif

ifeq ($(RTCONFIG_BCM_7114),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc stop_wl
endif
ifeq ($(RTCONFIG_ERPTEST),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc restart_usb
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc ATE
ifeq ($(RTCONFIG_DSL),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc asustest
endif
ifeq ($(or $(RTCONFIG_RALINK),$(RTCONFIG_QCA)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc FWRITE
	@cd $(INSTALLDIR)/sbin && ln -sf rc FREAD
endif
ifeq ($(RTCONFIG_RALINK),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc asuscfe_2g
	@cd $(INSTALLDIR)/sbin && ln -sf rc asuscfe_5g
	@cd $(INSTALLDIR)/sbin && ln -sf rc stainfo_2g
	@cd $(INSTALLDIR)/sbin && ln -sf rc stainfo_5g
ifeq ($(RTCONFIG_AMAS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc gstat
	@cd $(INSTALLDIR)/sbin && ln -sf rc groam
	@cd $(INSTALLDIR)/sbin && ln -sf rc gApCliStatus
	@cd $(INSTALLDIR)/sbin && ln -sf rc gcliq
	@cd $(INSTALLDIR)/sbin && ln -sf rc gclrssi
	@cd $(INSTALLDIR)/sbin && ln -sf rc gMonitorRssi
	@cd $(INSTALLDIR)/sbin && ln -sf rc gMacList
	@cd $(INSTALLDIR)/sbin && ln -sf rc gdfsChannel
	@cd $(INSTALLDIR)/sbin && ln -sf rc gdfsStatus
	@cd $(INSTALLDIR)/sbin && ln -sf rc gChannelInfo
	@cd $(INSTALLDIR)/sbin && ln -sf rc apclien
	@cd $(INSTALLDIR)/sbin && ln -sf rc send_bcnreq
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_rrm_bcnrep
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_rclass
ifeq ($(RTCONFIG_BTM_11V),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc send_11v
endif
endif
ifeq ($(RTCONFIG_DSL),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc gen_ralink_config
endif
endif

ifeq ($(or $(CONFIG_BCMWL5),$(RTCONFIG_RALINK),$(RTCONFIG_QCA),$(RTCONFIG_REALTEK)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc set_factory_mode
endif
ifeq ($(or $(RTCONFIG_OPENPLUS_TFAT),$(RTCONFIG_OPENPLUSPARAGON_NTFS),$(RTCONFIG_OPENPLUSTUXERA_NTFS),$(RTCONFIG_OPENPLUSPARAGON_HFS),$(RTCONFIG_OPENPLUSTUXERA_HFS)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc set_fs_coexist
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc run_telnetd
ifeq ($(RTCONFIG_SSH),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc run_sshd
endif
ifeq ($(RTCONFIG_WTFAST),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc run_wtfast
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc run_pptpd
ifeq ($(RTCONFIG_PARENTALCTRL),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc pc
	@cd $(INSTALLDIR)/sbin && ln -sf rc pc_block
	@cd $(INSTALLDIR)/sbin && ln -sf rc pc_tmp
ifeq ($(RTCONFIG_PC_REWARD),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc pc_reward
endif
endif
ifeq ($(RTCONFIG_INTERNETCTRL),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc ic
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc wlcscan
ifeq ($(RTCONFIG_WIRELESSREPEATER),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc wlcconnect
endif
ifeq ($(CONFIG_BCMWL5),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc setup_dnsmq
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc add_multi_routes

	@cd $(INSTALLDIR)/sbin && ln -sf rc free_caches
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_phy_status
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_phy_speed
	@cd $(INSTALLDIR)/sbin && ln -sf rc GetPhyStatus
ifeq ($(HND_ROUTER),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc set_ex53134_ctrl
	@cd $(INSTALLDIR)/sbin && ln -sf rc set_phy_ctrl
	@cd $(INSTALLDIR)/sbin && ln -sf rc regr
	@cd $(INSTALLDIR)/sbin && ln -sf rc regw
	@cd $(INSTALLDIR)/sbin && ln -sf rc pregr
	@cd $(INSTALLDIR)/sbin && ln -sf rc pregw
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc lanports_ctrl
	@cd $(INSTALLDIR)/sbin && ln -sf rc GetExtPhyStatus
ifeq ($(HND_ROUTER),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc memdw
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc firmware_check
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),BQ16 BQ16_PRO))
	@cd $(INSTALLDIR)/sbin && ln -sf rc wl_defer_conf
endif
ifeq ($(or $(RTCONFIG_SOC_IPQ8064),$(RTCONFIG_SOC_IPQ8074)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc readmem
endif
ifeq ($(or $(RTCONFIG_WIFI_QCN5024_QCN5054),$(RTCONFIG_QCA_AXCHIP)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc stress_pktgen
endif
ifeq ($(RTCONFIG_SOC_IPQ8074),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc test_blu
	@cd $(INSTALLDIR)/sbin && ln -sf rc split_q6mem
endif
ifeq ($(or $(RTCONFIG_QCA),$(RTCONFIG_RALINK)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc test_wifi_stats_log
endif
ifeq ($(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc test_switch_log
endif
ifeq ($(RTCONFIG_HTTPS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc rsasign_check
	@cd $(INSTALLDIR)/sbin && ln -sf rc rsarootca_check
ifeq ($(RTCONFIG_FRS_FEEDBACK),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc firmware_check_update
	@cd $(INSTALLDIR)/sbin && ln -sf rc sendfeedback
endif
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc test_endian
ifeq ($(RTCONFIG_BWDPI),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc rsasign_sig_check
	@cd $(INSTALLDIR)/sbin && ln -sf rc bwdpi
	@cd $(INSTALLDIR)/sbin && ln -sf rc bwdpi_check
	@cd $(INSTALLDIR)/sbin && ln -sf rc bwdpi_wred_alive
	@cd $(INSTALLDIR)/sbin && ln -sf rc bwdpi_db_10
endif
ifeq ($(RTCONFIG_AMAS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_lib
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc hour_monitor
ifeq ($(RTCONFIG_SPEEDTEST),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc speedtest
endif
ifeq ($(RTCONFIG_USB_MODEM),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc write_3g_ppp_conf
ifeq ($(or $(RTCONFIG_JFFS2),$(RTCONFIG_BRCM_NAND_JFFS2),$(RTCONFIG_UBIFS)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc modem_data
	@cd $(INSTALLDIR)/sbin && ln -sf rc lplus
	@cd $(INSTALLDIR)/sbin && ln -sf rc lminus
	@cd $(INSTALLDIR)/sbin && ln -sf rc order
endif
endif
ifeq ($(RTCONFIG_IPERF),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc monitor
endif
ifeq ($(RTCONFIG_INTERNAL_GOBI),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc lteled
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc dhcpc_lease

ifeq ($(RTCONFIG_NEW_USER_LOW_RSSI),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc roamast
endif

ifeq ($(RTCONFIG_DHCP_OVERRIDE),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc detectWAN_arp
endif
ifeq ($(RTCONFIG_KEY_GUARD),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc keyguard
endif

ifeq ($(RTCONFIG_BT_CONN),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc start_bluetooth_service
ifeq ($(RTCONFIG_BT_CONN_UART),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc execute_bt_bscp
endif
endif

ifeq ($(RTCONFIG_QCA),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc dump_powertable
	@cd $(INSTALLDIR)/sbin && ln -sf rc tmpsta
endif

ifeq ($(RTCONFIG_RALINK),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc dump_powertable
	@cd $(INSTALLDIR)/sbin && ln -sf rc dump_txbftable
	@cd $(INSTALLDIR)/sbin && ln -sf rc air_monitor
endif

ifneq ($(RTCONFIG_RALINK)$(RTCONFIG_REALTEK),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc erp_monitor
endif

ifeq ($(RTCONFIG_AMAS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_wlcconnect
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_bhctrl
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_lanctrl
ifeq ($(RTCONFIG_AMAS_ADTBW),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_adtbw
endif
ifeq ($(RTCONFIG_BHCOST_OPT),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_status
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_ssd
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_ipc
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_misc
endif
ifeq ($(RTCONFIG_CONNDIAG),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_ssd_cd
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_portstatus
endif
ifeq ($(RTCONFIG_CFGSYNC),y)
ifeq ($(HND_ROUTER_AX),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc rmd
endif
endif
ifeq ($(RTCONFIG_AMAS_CENTRAL_ADS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc start_ds_measure
	@cd $(INSTALLDIR)/sbin && ln -sf rc monitor_ds_ap_status
	@cd $(INSTALLDIR)/sbin && ln -sf rc monitor_ds_sta_status
endif
endif

ifeq ($(RTCONFIG_LETSENCRYPT),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc le_acme
endif

ifeq ($(RTCONFIG_SOFTWIRE46),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc init_wan46
	@cd $(INSTALLDIR)/sbin && ln -sf rc auto46det
	@cd $(INSTALLDIR)/sbin && ln -sf rc s46reset
	@cd $(INSTALLDIR)/sbin && ln -sf rc mapcalc
	@cd $(INSTALLDIR)/sbin && ln -sf rc v6plusd
	@cd $(INSTALLDIR)/sbin && ln -sf rc ocnvcd
	@cd $(INSTALLDIR)/sbin && ln -sf rc dslited
endif

ifeq ($(RTCONFIG_WLCSCAN_RSSI),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc wlcscan_ssid_rssi
endif

ifeq ($(RTCONFIG_ADTBW),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc adtbw
endif

ifeq ($(or $(RTAC68U),$(RT4GAC68U)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc firmware_enc_crc
	@cd $(INSTALLDIR)/sbin && ln -sf rc fw_check
endif

ifeq ($(RTCONFIG_EXTPHY_BCM84880),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc extphy_bit_op
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_ext_phy_id
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AX82U RT-AX82U_V2 GS-AX3000 GS-AX5400 TUF-AX5400 TUF-AX5400_V2 GT-AX11000_PRO GT-AXE16000 GT-BE98 GT-AX6000 GT10 GT-BE98_PRO GT-BE96 GT-BE19000))
	@cd $(INSTALLDIR)/sbin && ln -sf rc ledg
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AX82U RT-AX82U_V2 GS-AX3000 GS-AX5400 TUF-AX5400 TUF-AX5400_V2 GT-AX11000_PRO GT-AXE16000 GT-AX6000 GT10))
	@cd $(INSTALLDIR)/sbin && ln -sf rc ledbtn
endif
endif
ifeq ($(DSL_AX82U),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc ledg
endif

ifeq ($(GTAX6000),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc antled
endif

ifeq ($(RTCONFIG_DSL_HOST),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc dsld
endif

ifeq ($(RTCONFIG_QCA_PLC2),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc restart_plc
	@cd $(INSTALLDIR)/sbin && ln -sf rc detect_plc
endif

ifeq ($(RTCONFIG_SCHED_DAEMON),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc sched_daemon
endif

# tencent download
ifeq ($(RTCONFIG_TC_DOWNLOAD), y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc tc_download
endif

ifeq ($(and $(RTCONFIG_TUNNEL),$(RTCONFIG_ACCOUNT_BINDING)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc update_asus_ddns_token
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc collect_debuglog

ifeq ($(RTCONFIG_TPVPN),y)
ifeq ($(RTCONFIG_HMA),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc hmavpn
endif
ifeq ($(RTCONFIG_NORDVPN),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc nordvpn
endif
endif

ifeq ($(RTCONFIG_ISP_CUSTOMIZE_TOOL),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc tci
	@rm -rf $(INSTALLDIR)/../rom/${EX_ROM}/ict/
	@if [ -f $(TOP)/rom/ict/settings.ini ] ; then \
		install -D $(TOP)/rom/ict/settings.ini $(INSTALLDIR)/../rom/${EX_ROM}/ict/settings.ini ; \
	fi;
	@if [ -f $(TOP)/rom/ict/settings.cfg ] ; then \
		install -D $(TOP)/rom/ict/settings.cfg $(INSTALLDIR)/../rom/${EX_ROM}/ict/settings.cfg ; \
	fi;
	@if [ -f $(TOP)/rom/ict/package.tar.gz ] ; then \
		install -D $(TOP)/rom/ict/package.tar.gz $(INSTALLDIR)/../rom/${EX_ROM}/ict/package.tar.gz ; \
	fi;
endif

ifeq ($(RTCONFIG_MULTILAN_CFG),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc reset_mtlan
	@cd $(INSTALLDIR)/sbin && ln -sf rc reset_rm_mtlan
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_mtsubnet
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_mtlan
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_mtlan_by_vid
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_mtlan_by_idx
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_rm_mtsubnet
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_rm_mtlan
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_rm_mtlan_by_vid
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_rm_mtlan_by_idx
	@cd $(INSTALLDIR)/sbin && ln -sf rc apg_get_status
	@cd $(INSTALLDIR)/sbin && ln -sf rc check_wifi_band_cap
	@cd $(INSTALLDIR)/sbin && ln -sf rc check_lan_port_cap
endif

ifeq ($(RTCONFIG_IG_SITE2SITE),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc ig_s2s_client
endif

ifeq ($(RTCONFIG_GETREALIP),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc realip
endif

ifeq ($(HND_ROUTER),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc gpy211_monitor
endif

ifeq ($(RTCONFIG_MOCA),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc moca_monitor
endif

clean:
	rm -f rc *.o .*.depend *.prep libconn_diag.so
ifeq ($(RTCONFIG_BRCM_HOSTAPD),y)
	rm -f $(WPS_PBCD)
endif

size: rc
	mipsel-uclibc-nm --print-size --size-sort rc

# handle prebuilt object here
ifneq ($(wildcard ./prebuild/dsl_fb.o),)
ifeq ($(RTCONFIG_FRS_FEEDBACK),y)
dsl_fb.o:
	@-cp -f ./prebuild/dsl_fb.o .
endif
ifeq ($(RTCONFIG_DSL),y)
ifeq ($(RTCONFIG_DSL_TCLINUX),y)
dsl_diag.o:
	@-cp -f ./prebuild/dsl_diag.o .
endif
endif
else
#ifeq ($(RTCONFIG_HTTPS),y)
ifeq ($(RTCONFIG_FRS_FEEDBACK),y)
dsl_fb.o:dsl_fb.c
#	@echo "encrypt dsl password"
#	./dsl_fb_enc.sh
#	@$(CC) $(CFLAGS) -c dsl_fb.c
#	@echo "decrypt dsl password"
#	./dsl_fb_dec.sh
endif
#endif
endif

ifeq ($(RTCONFIG_MOCA),y)
ifneq ($(wildcard ./prebuild/moca.o),)
moca.o:
	@-cp -f ./prebuild/moca.o .
endif
endif

ifneq ($(wildcard ./prebuild/frs_service.o),)
frs_service.o:
	@-cp -f ./prebuild/frs_service.o .
endif

ifneq ($(wildcard ./prebuild/bwdpi*.o),)
bwdpi.o:
	@-cp -f ./prebuild/bwdpi.o .
bwdpi_check.o:
	@-cp -f ./prebuild/bwdpi_check.o .
bwdpi_db_10.o:
	@-cp -f ./prebuild/bwdpi_db_10.o .
bwdpi_wred_alive.o:
	@-cp -f ./prebuild/bwdpi_wred_alive.o .
endif

ifneq ($(wildcard ./prebuild/nt_mail.o),)
nt_mail.o:
	@-cp -f ./prebuild/nt_mail.o .
endif

# handle prebuilt object here
ifeq ($(RTCONFIG_TAGGED_BASED_VLAN),y)
ifneq ($(wildcard ./prebuild/tagged_based_vlan.o),)
tagged_based_vlan.o:
	@-cp -f ./prebuild/tagged_based_vlan.o .
endif
endif

ifeq ($(RTCONFIG_AMAS_WGN), y)
ifneq ($(wildcard ./prebuild/amas_wgn.o),)
amas_wgn:
	@-cp -f ./prebuild/amas_wgn.o .
endif
endif

ifeq ($(RTCONFIG_MULTILAN_CFG), y)
ifneq ($(wildcard ./prebuild/amas_apg.o),)
amas_apg:
	@-cp -f ./prebuild/amas_apg.o .
endif
endif

# asustest command is for DSL models only, debugging purpose.
ifeq ($(RTCONFIG_DSL),y)
ifneq ($(wildcard ./prebuild/asustest.o),)
asustest.o:
	@-cp -f ./prebuild/asustest.o .
else
asustest.o:asustest.c
	@echo "compile asustest"
	@$(CC) $(CFLAGS) -c asustest.c
endif
endif

ifeq ($(RTCONFIG_HTTPS),y)
ifeq ($(or $(RTCONFIG_PUSH_EMAIL),$(RTCONFIG_NOTIFICATION_CENTER)),y)
ifneq ($(wildcard ./prebuild/pwdec.o),)
pwdec.o:
	@-cp -f ./prebuild/pwdec.o .
endif
endif
endif

ifeq ($(CONFIG_BCMWL5),y)
ifneq ($(wildcard ./prebuild/ate-broadcom.o),)
ate-broadcom.o:
	@-cp -f ./prebuild/ate-broadcom.o .
endif
ifneq ($(wildcard ./prebuild/rpmac.o),)
rpmac.o:
	@-cp -f ./prebuild/rpmac.o .
endif
ifneq ($(wildcard ./prebuild/init-broadcom.o),)
init-broadcom.o:
	@-cp -f ./prebuild/init-broadcom.o .
endif
ifneq ($(wildcard ./prebuild/ate.o),)
ate.o:
	@-cp -f ./prebuild/ate.o .
endif
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AC68U RT-AC3200 4G-AC68U GT-AC2900 RT-AX58U TUF-AX3000 RT-AX82U DSL-AX82U GS-AX3000 GS-AX5400))
ifneq ($(wildcard ./prebuild/cfe.o),)
cfe.o:
	@-cp -f ./prebuild/cfe.o .
endif
endif
ifeq ($(RTCONFIG_ADTBW),y)
ifneq ($(wildcard ./prebuild/adtbw-broadcom.o),)
adtbw-broadcom.o:
	@-cp -f ./prebuild/adtbw-broadcom.o .
endif
endif

ifeq ($(RTCONFIG_AMAS_ADTBW),y)
ifneq ($(wildcard ./prebuild/amas_adtbw.o),)
amas_adtbw.o:
	@-cp -f ./prebuild/amas_adtbw.o .
endif

ifneq ($(wildcard ./prebuild/amas-adtbw-broadcom.o),)
amas-adtbw-broadcom.o:
	@-cp -f ./prebuild/amas-adtbw-broadcom.o .
endif
endif

ifeq ($(and $(RTCONFIG_BCMWL6),$(RTCONFIG_PROXYSTA)),y)
ifneq ($(wildcard ./prebuild/psta_monitor.o),)
psta_monitor.o:
	@-cp -f ./prebuild/psta_monitor.o .
endif
endif
ifneq ($(wildcard ./prebuild/broadcom.o),)
broadcom.o:
	@-cp -f ./prebuild/broadcom.o .
endif
ifeq ($(RTCONFIG_AMAS),y)
ifneq ($(wildcard ./prebuild/obd-broadcom.o),)
obd-broadcom.o:
	@-cp -f ./prebuild/obd-broadcom.o .
endif
ifneq ($(wildcard ./prebuild/amas-ssd-broadcom),)
amas-ssd-broadcom:
	@-cp -f ./prebuild/amas-ssd-broadcom .
endif
ifneq ($(wildcard ./prebuild/amas-ssd-cd-broadcom),)
amas-ssd-cd-broadcom:
	@-cp -f ./prebuild/amas-ssd-cd-broadcom .
endif
ifneq ($(wildcard ./prebuild/amas-ssd-cd-broadcom.o),)
amas-ssd-cd-broadcom.o:
	@-cp -f ./prebuild/amas-ssd-cd-broadcom.o .
endif
endif
endif

ifeq ($(or $(HND_ROUTER_AX_6756),$(HND_ROUTER_BE_4916)),y)
ifeq ($(RTCONFIG_MULTILAN_CFG),y)
ifneq ($(wildcard ./prebuild/sdn-interface-broadcom.o),)
sdn-interface-broadcom.o:
	@-cp -f ./prebuild/sdn-interface-broadcom.o .
endif
endif
endif

ifeq ($(RTCONFIG_RALINK),y)
ifneq ($(wildcard ./prebuild/ate-ralink.o),)
ate-ralink.o:
	@-cp -f ./prebuild/ate-ralink.o .
endif
ifneq ($(wildcard ./prebuild/air_monitor.o),)
air_monitor.o:
	@-cp -f ./prebuild/air_monitor.o .
endif
ifneq ($(wildcard ./prebuild/ralink.o),)
ralink.o:
	@-cp -f ./prebuild/ralink.o .
endif
ifneq ($(wildcard ./prebuild/roamast-ralink.o),)
roamast-ralink.o:
	@-cp -f ./prebuild/roamast-ralink.o .
endif
ifeq ($(RTCONFIG_AMAS),y)
ifneq ($(wildcard ./prebuild/obd-ralink.o),)
obd-ralink.o:
	@-cp -f ./prebuild/obd-ralink.o .
endif
ifneq ($(wildcard ./prebuild/amas-ssd-ralink),)
amas-ssd-ralink:
	@-cp -f ./prebuild/amas-ssd-ralink .
endif
endif
endif

ifeq ($(RTCONFIG_QCA),y)
ifneq ($(wildcard ./prebuild/ate-qca.o),)
ate-qca.o:
	@-cp -f ./prebuild/ate-qca.o .
endif
ifeq ($(RTCONFIG_AMAS),y)
ifneq ($(wildcard ./prebuild/obd-qca.o),)
obd-qca.o:
	@-cp -f ./prebuild/obd-qca.o .
endif
ifneq ($(wildcard ./prebuild/amas-ssd-qca.o),)
amas-ssd-qca:
	@-cp -f ./prebuild/amas-ssd-qca.o .
endif
ifneq ($(wildcard ./prebuild/amas-ssd-cd-qca.o),)
amas-ssd-cd-qca:
	@-cp -f ./prebuild/amas-ssd-cd-qca.o .
endif
endif
endif

ifeq ($(RTCONFIG_REALTEK),y)
ifneq ($(wildcard ./prebuild/ate-realtek.o),)
ate-realtek.o:
	@-cp -f ./prebuild/ate-realtek.o .
endif
ifneq ($(wildcard ./prebuild/realtek.o),)
realtek.o:
	@-cp -f ./prebuild/realtek.o .
endif
ifneq ($(wildcard ./prebuild/rtk_wifi_drvmib.o),)
rtk_wifi_drvmib.o:
	@-cp -f ./prebuild/rtk_wifi_drvmib.o .
endif
ifeq ($(RTCONFIG_AMAS),y)
ifneq ($(wildcard ./prebuild/obd-realtek.o),)
obd-realtek.o:
	@-cp -f ./prebuild/obd-realtek.o .
endif
ifneq ($(wildcard ./prebuild/amas-ssd-realtek),)
amas-ssd-realtek:
	@-cp -f ./prebuild/amas-ssd-realtek .
endif
endif
endif

ifeq ($(RTCONFIG_TCODE),y)
ifneq ($(wildcard ./prebuild/tcode_rc.o),)
tcode_rc.o:
	@-cp -f ./prebuild/tcode_rc.o .
endif
ifeq ($(CONFIG_BCMWL5),y)
ifneq ($(wildcard ./prebuild/tcode_brcm.o),)
tcode_brcm.o:
	@-cp -f ./prebuild/tcode_brcm.o .
endif
endif
endif

ifeq ($(RTCONFIG_LACP),y)
ifneq ($(wildcard ./prebuild/agg_brcm.o),)
agg_brcm.o:
	@-cp -f ./prebuild/agg_brcm.o .
endif
endif

ifeq ($(and $(CONFIG_BCMWL5),$(RTCONFIG_DUALWAN)),y)
ifneq ($(wildcard ./prebuild/dualwan.o),)
dualwan.o:
	@-cp -f ./prebuild/dualwan.o .
endif
endif

ifeq ($(RTCONFIG_MULTIWAN_IF),y)
ifneq ($(wildcard ./prebuild/multi_wan.o),)
multi_wan.o:
	@-cp -f ./prebuild/multi_wan.o .
endif
endif

ifeq ($(RTCONFIG_ALPINE),y)
ifneq ($(wildcard ./prebuild/init-alpine.o),)
init-alpine.o:
	@-cp -f ./prebuild/init-alpine.o .
endif
ifneq ($(wildcard ./prebuild/ate-alpine.o),)
ate-alpine.o:
	@-cp -f ./prebuild/ate-alpine.o .
endif
ifneq ($(wildcard ./prebuild/alpine.o),)
alpine.o:
	@-cp -f ./prebuild/alpine.o .
endif
ifneq ($(wildcard ./prebuild/qsr10g.o),)
qsr10g.o:
	@-cp -f ./prebuild/qsr10g.o .
endif
endif

ifeq ($(RTCONFIG_LANTIQ),y)
ifneq ($(wildcard ./prebuild/lantiq.o),)
lantiq.o:
	@-cp -f ./prebuild/lantiq.o .
endif
ifneq ($(wildcard ./prebuild/ate-lantiq.o),)
ate-lantiq.o:
	@-cp -f ./prebuild/ate-lantiq.o .
endif
ifneq ($(wildcard ./prebuild/init-lantiq.o),)
init-lantiq.o:
	@-cp -f ./prebuild/init-lantiq.o .
endif
ifneq ($(wildcard ./prebuild/lantiq-wave.o),)
lantiq-wave.o:
	@-cp -f ./prebuild/lantiq-wave.o .
endif
ifneq ($(wildcard ./prebuild/client.o),)
client.o:
	@-cp -f ./prebuild/client.o .
endif
ifeq ($(RTCONFIG_AMAS),y)
ifneq ($(wildcard ./prebuild/obd-lantiq.o),)
obd-lantiq.o:
	@-cp -f ./prebuild/obd-lantiq.o .
endif
ifneq ($(wildcard ./prebuild/amas-ssd-lantiq),)
amas-ssd-lantiq:
	@-cp -f ./prebuild/amas-ssd-lantiq .
endif
endif
endif

ifneq ($(wildcard ./prebuild/nbr_monitor.o),)
nbr_monitor.o:
	@-cp -f ./prebuild/nbr_monitor.o .
endif

ifneq ($(wildcard ./prebuild/obd.o),)
obd.o:
	@-cp -f ./prebuild/obd.o .
endif

ifneq ($(wildcard ./prebuild/obd_eth.o),)
obd_eth.o:
	@-cp -f ./prebuild/obd_eth.o .
endif

ifneq ($(wildcard ./prebuild/obd_monitor.o),)
obd_monitor.o:
	@-cp -f ./prebuild/obd_monitor.o .
endif

ifeq ($(RTCONFIG_NEW_USER_LOW_RSSI),y)
ifneq ($(wildcard ./prebuild/roamast.o),)
roamast.o:
	@-cp -f ./prebuild/roamast.o .
endif
ifneq ($(wildcard ./prebuild/roamast-broadcom.o),)
roamast-broadcom.o:
	@-cp -f ./prebuild/roamast-broadcom.o .
endif
ifneq ($(wildcard ./prebuild/roamast-lantiq.o),)
roamast-lantiq.o:
	@-cp -f ./prebuild/roamast-lantiq.o .
endif
endif

ifeq ($(RTCONFIG_TR069),y)
ifneq ($(wildcard ./prebuild/tr069.o),)
tr069.o:
	@-cp -f ./prebuild/tr069.o .
endif
endif

ifeq ($(RTCONFIG_PRESSURE_SENSOR),y)
ifneq ($(wildcard ./prebuild/pressure.o),)
pressure.o:
	@-cp -f ./prebuild/pressure.o .
endif
endif


ifneq ($(wildcard ./prebuild/dhcpc_vc_query.o),)
dhcpc_vc_query.o:
	@-cp -f ./prebuild/dhcpc_vc_query.o .
endif

ifeq ($(RTCONFIG_AMAS),y)
ifneq ($(wildcard ./prebuild/amas_lanctrl.o),)
amas_lanctrl.o:
	@-cp -f ./prebuild/amas_lanctrl.o .
endif
ifneq ($(wildcard ./prebuild/amas_bhctrl.o),)
amas_bhctrl.o:
	@-cp -f ./prebuild/amas_bhctrl.o .
endif
ifneq ($(wildcard ./prebuild/amas_wlcconnect.o),)
amas_wlcconnect.o:
	@-cp -f ./prebuild/amas_wlcconnect.o .
endif
ifneq ($(wildcard ./prebuild/amas_lib.o),)
amas_lib.o:
	@-cp -f ./prebuild/amas_lib.o .
endif
ifeq ($(RTCONFIG_BHCOST_OPT),y)
ifneq ($(wildcard ./prebuild/amas_status.o),)
amas_status.o:
	@-cp -f ./prebuild/amas_status.o .
endif
ifneq ($(wildcard ./prebuild/amas_misc.o),)
amas_misc.o:
	@-cp -f ./prebuild/amas_misc.o .
endif
ifneq ($(wildcard ./prebuild/amas_ssd.o),)
amas_ssd.o:
	@-cp -f ./prebuild/amas_ssd.o .
endif
ifneq ($(wildcard ./prebuild/amas_ssd_cd.o),)
amas_ssd_cd.o:
	@-cp -f ./prebuild/amas_ssd_cd.o .
endif
ifneq ($(wildcard ./prebuild/amas_ipc.o),)
amas_ipc.o:
	@-cp -f ./prebuild/amas_ipc.o .
endif
endif
ifeq ($(RTCONFIG_PRELINK),y)
ifneq ($(wildcard ./prebuild/amas_prelink.o),)
amas_prelink.o:
	@-cp -f ./prebuild/amas_prelink.o .
endif
endif
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AX82U RT-AX82U_V2 GS-AX3000 GS-AX5400 TUF-AX5400 TUF-AX5400_V2 GT-AX11000_PRO GT-AXE16000 GT-BE98 GT-AX6000 GT10 GT-BE98_PRO GT-BE96 GT-BE19000))
ifneq ($(wildcard ./prebuild/ledg.o),)
ledg.o:
	@-cp -f ./prebuild/ledg.o .
endif
endif
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),GT-BE98 GT-BE98_PRO GT-BE96 GT-BE19000 RT-BE58U TUF-BE3600 RT-BE92U RT-BE95U))
ifneq ($(wildcard ./prebuild/rtkmonitor.o),)
rtkmonitor.o:
	@-cp -f ./prebuild/rtkmonitor.o .
endif
endif
ifeq ($(RTCONFIG_SOFTWIRE46),y)
ifneq ($(wildcard ./prebuild/s46comm.o),)
s46comm.o:
	@-cp -f ./prebuild/s46comm.o .
endif
ifneq ($(wildcard ./prebuild/v6plusd.o),)
v6plusd.o:
	@-cp -f ./prebuild/v6plusd.o .
endif
ifneq ($(wildcard ./prebuild/ocnvcd.o),)
ocnvcd.o:
	@-cp -f ./prebuild/ocnvcd.o .
endif
ifneq ($(wildcard ./prebuild/dslited.o),)
dslited.o:
	@-cp -f ./prebuild/dslited.o .
endif
endif
ifeq ($(RTCONFIG_TPVPN),y)
ifneq ($(wildcard ./prebuild/tpvpn.o),)
tpvpn.o:
	@-cp -f ./prebuild/tpvpn.o .
endif
endif

ifeq ($(RTCONFIG_SCHED_DAEMON),y)
ifneq ($(wildcard ./prebuild/sched_daemon.o),)
sched_daemon.o.o:
	@-cp -f ./prebuild/sched_daemon.o .
endif
endif

ifeq ($(GTAX6000),y)
ifneq ($(wildcard ./prebuild/antled.o),)
broadcom.o:
	@-cp -f ./prebuild/antled.o .
endif
endif

ifeq ($(RTCONFIG_IG_SITE2SITE),y)
ifneq ($(wildcard ./prebuild/ig_s2s_client.o),)
ig_s2s_client.o:
	@-cp -f ./prebuild/ig_s2s_client.o .
endif
endif

ifneq ($(wildcard ./prebuild/gpy211_war.o),)
gpy211_war.o:
	@-cp -f ./prebuild/gpy211_war.o .
endif

%.o: %.c
	@echo " [rc] CC $@"
	@$(CC) $(CFLAGS) -c $<

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

init.prep: init.c
	@$(CC) $(CFLAGS) -E $< > $@

ifeq ($(RTCONFIG_QCA),y)
-include sysdeps/qca/Makefile.hook
endif

pswatch: pswatch.c
	@$(CC) -o $@ $^
	$(STRIP) $@
	$(CPTMP)

ifeq ($(RTCONFIG_CONNDIAG),y)
libconn_diag.so:
	@for f in $(patsubst %.o,%,$(DIAG_OBJS)) ; do \
		if [ -f $${f}.c ] ; then \
			$(CC) $(CFLAGS) -fPIC -c $${f}.c; \
		else \
			cp -f ./prebuild/$${f}.o .; \
		fi \
	done
ifeq ($(HND_ROUTER_AX),y)
	$(LD) -L$(TOP_PLATFORM)/nvram -lnvram -L$(TOP)/shared -lshared -shared $(if $(RTCONFIG_BCM_502L07P2),-L$(TOP_PLATFORM)/ethctl_lib -lethctl) -o $@ $(DIAG_OBJS)
else
	$(LD) -L$(TOP)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared -shared -o $@ $(DIAG_OBJS)
endif
endif
