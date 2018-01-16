include ../common.mak
CFLAGS += -Wall -Os $(EXTRACFLAGS)
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include
ifneq ($(HND_ROUTER),y)
CFLAGS += -idirafter$(LINUXDIR)/include
endif
ifneq ($(or $(findstring linux-3.10.14,$(LINUXDIR))),)
CFLAGS += -idirafter$(LINUXDIR)/include/uapi
endif
CFLAGS += $(PRIVATE_EXTRACFLAGS)
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include
ifneq ($(HND_ROUTER),y)
CFLAGS += -idirafter$(LINUXDIR)/include
endif
ifeq ($(RTCONFIG_DPSTA),y)
CFLAGS += -I$(TOP)/dpsta
endif
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -DDLM -DASUS_DDNS -DASUS_EXT -DBTN_SETUP -DWEB_REDIRECT -DNORESOLV
CFLAGS += -DRTCONFIG_CROND
CFLAGS += -DDEBUG_RCTEST

ifeq ($(RTCONFIG_WPS),y)
CFLAGS += -I$(SRCBASE)/wps/brcm_apps/include
endif

ifeq ($(RTCONFIG_REALTEK),y)
CFLAGS += -Wl,--gc-sections -ldl
ifneq ($(findstring CONFIG_RTL_USERSPACE_WTDOG=, $(shell cat $(LINUXDIR)/.config)),)
export CFLAGS += -DRTL_WTDOG
endif
else ifeq ($(RTCONFIG_RALINK),y)
CFLAGS += -I$(TOP)/shared/sysdeps/ralink -I$(TOP)/wireless_tools
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
CFLAGS += -I$(TOP)/shared/sysdeps/qca -I$(TOP)/wireless_tools
CFLAGS += $(if $(RTCONFIG_SOC_IPQ8064),-DSMP)
endif
ifeq ($(RTCONFIG_WBD),y)
CFLAGS += -D__CONFIG_WBD__
endif

ifeq ($(RTCONFIG_ALPINE),y)
CFLAGS += -I$(TOP)/shared/sysdeps/alpine -I$(TOP)/wireless_tools -I$(TOP)/mtd-utils/include/
endif

ifeq ($(RTCONFIG_LANTIQ),y)
CFLAGS += -I$(TOP)/shared/sysdeps/lantiq -I$(TOP)/wireless_tools -I$(TOP)/fapi_wlan_common-1.0.0.1/include -I$(TOP)/libhelper-1.4.0.2/include -DPACKAGE_ID=\"fapi_wlan_common\" -DLOGGING_ID="fapi_wlan_common"
CFLAGS  += -I$(TOP)/wlc_nt
endif

include $(SRCBASE)/.config

ifeq ($(RTCONFIG_REALTEK),y)
LDFLAGS = -L$(TOP)/nvram${BCMEX} -lnvram -L$(TOP)/shared -lshared -lcrypt -ldl
else
LDFLAGS = -L$(TOP)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared -lcrypt -L$(TOP)/libbcmcrypto -lbcmcrypto -ldl
endif
ifeq ($(HND_ROUTER),y)
CFLAGS += -I$(HND_SRC)/userspace/public/include -I$(HND_SRC)/userspace/public/include/linux -I$(HND_SRC)/userspace/private/include
LDFLAGS += -L$(TOP)/wlcsm -lwlcsm -L$(TOP)/bcm_flashutil -lbcm_flashutil -L$(TOP)/bcm_util -lbcm_crc -L$(TOP)/bcm_boardctl -lbcm_boardctl
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
LDFLAGS += -L$(TOP)/fapi_wlan_common-1.0.0.1 -lfapiwlancommon -ldl -L$(TOP)/libhelper-1.4.0.2 -lhelper -L$(TOP)/libfapi-0.1 -lfapi
LDFLAGS += -L$(TOP)/wlc_nt -lwlc_nt_client
endif

ifeq ($(RTCONFIG_HTTPS),y)
CFLAGS += -I$(TOP)/openssl/include
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl
endif

ifeq ($(RTCONFIG_BWDPI),y)
CFLAGS += -I$(TOP)/bwdpi_source/asus_include
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

ifeq ($(RTCONFIG_USB),y)
CFLAGS += -I$(TOP)/libdisk
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

ifeq ($(RTCONFIG_PUSH_EMAIL),y)
CFLAGS  += -I$(TOP)/openssl/include
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl
endif

ifeq ($(RTCONFIG_FBWIFI),y)
CFLAGS  += -I$(TOP)/fb_wifi
LDFLAGS += -L$(TOP)/fb_wifi -lfbwifi
endif

ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
CFLAGS  += -I$(TOP)/sqlite
CFLAGS  += -I$(TOP)/nt_center/lib
CFLAGS  += -I$(TOP)/nt_center/actMail
CFLAGS  += -DASUSWRT_SDK
endif

ifeq ($(RTCONFIG_PROTECTION_SERVER),y)
CFLAGS  += -I$(TOP)/protect_srv/include
endif

ifeq ($(RTCONFIG_BT_CONN),y)
CFLAGS  += -I$(TOP)/bluez-5.41/lib
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

ifeq ($(or $(RTAC85U)),y)
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

OBJS := rc.o init.o interface.o lan.o wireless.o wan.o pppd.o auth.o services.o
OBJS += firewall.o ppp.o services.o common.o
OBJS += watchdog.o ntp.o btnsetup.o $(if $(RTCONFIG_MULTIWAN_CFG),qos_multiwan.o,qos.o) udhcpc.o ate.o
OBJS += format.o

ifeq ($(RTCONFIG_NETOOL),y)
OBJS += netool.o linklist.o
LDFLAGS += -lpthread
endif

ifeq ($(RTAC1200G),y)
OBJS += mtd_sflash.o
else
OBJS += mtd.o
endif

ifeq ($(RTCONFIG_FANCTRL),y)
OBJS += phy_tempsense.o
endif
ifeq ($(RTCONFIG_BCMARM),y)
OBJS += bcmutils.o
endif

ifeq ($(CONFIG_BCMWL5),y)
OBJS += init-broadcom.o wps-broadcom.o broadcom.o ate-broadcom.o
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AC68U RT-AC3200))
OBJS += cfe.o
endif
ifeq ($(RTCONFIG_ADTBW),y)
OBJS += adtbw-broadcom.o
endif
ifeq ($(RTCONFIG_DSL),y)
OBJS += init-broadcom-dsl.o
endif
else ifeq ($(RTCONFIG_REALTEK),y)
OBJS += init-realtek.o wps-realtek.o realtek.o rtk_wifi_drvmib.o ate-realtek.o
else ifeq ($(RTCONFIG_RALINK),y)
OBJS += init-ralink.o wps-ralink.o ralink.o ate-ralink.o
ifeq ($(RTCONFIG_CONCURRENTREPEATER),y)
OBJS += air_monitor.o
endif
ifeq ($(RTCONFIG_DSL),y)
OBJS += init-ralink-dsl.o
endif
else ifeq ($(RTCONFIG_QCA),y)
OBJS += init-qca.o wps-qca.o qca.o ate-qca.o $(if $(wildcard sysdeps/qca/ctl.c),ctl.obj,prebuild/ctl.obj)
ifeq ($(RTCONFIG_WIFI_SON),y)
OBJS += repacd.o
endif
else ifeq ($(RTCONFIG_ALPINE),y)
OBJS += init-alpine.o wps-alpine.o alpine.o ate-alpine.o
else ifeq ($(RTCONFIG_LANTIQ),y)
OBJS += init-lantiq.o wps-lantiq.o lantiq.o ate-lantiq.o lantiq-wave.o
endif

OBJS += $(if $(wildcard private.c),private.o,prebuild/private.o)
OBJS += $(if $(wildcard conn_diag.c),conn_diag.o,prebuild/conn_diag.o)

# TODO, speedtest
ifeq ($(RTCONFIG_SPEEDTEST),y)
OBJS += $(if $(wildcard speedtest.c),speedtest.o,prebuild/speedtest.o)
endif

OBJS += wanduck.o
OBJS += tcpcheck.o

ifeq ($(RTCONFIG_SOC_IPQ8064),y)
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

ifeq ($(or $(RTCONFIG_PPTPD),$(RTCONFIG_ACCEL_PPTPD)),y)
OBJS += vpn.o
endif

ifeq ($(RTCONFIG_SNMPD),y)
OBJS += snmpd.o
endif
ifeq ($(RTCONFIG_OPENVPN),y)
CFLAGS += -I$(TOP)/libvpn
LDFLAGS += -L$(TOP)/libvpn -lvpn
OBJS += openvpn.o
endif

ifeq ($(RTCONFIG_IPSEC),y)
OBJS += rc_ipsec.o
endif

ifeq ($(RTCONFIG_PARENTALCTRL),y)
OBJS += pc.o
OBJS += pc_block.o
OBJS += pc_tmp.o
endif

ifeq ($(RTCONFIG_HTTPS),y)
ifeq ($(RTCONFIG_PUSH_EMAIL),y)
OBJS += dsl_fb.o
endif
endif

ifeq ($(RTCONFIG_DSL),y)
OBJS += asustest.o
endif

ifeq ($(RTCONFIG_DSL),y)
OBJS += dsl.o
ifeq ($(RTCONFIG_RALINK),y)
OBJS += dsl-upg.o
endif
ifeq ($(RTCONFIG_DSL_TCLINUX),y)
OBJS += dsl_diag.o
endif
endif

ifeq ($(RTCONFIG_BCMWL6),y)
ifeq ($(RTCONFIG_PROXYSTA),y)
OBJS += psta_monitor.o
endif
ifeq ($(RTCONFIG_AMAS),y)
ifneq ($(RTCONFIG_DISABLE_REPEATER_UI),y)
OBJS += obd.o
endif
endif
endif

ifeq ($(RTCONFIG_LANTIQ),y)
ifeq ($(RTCONFIG_AMAS),y)
ifneq ($(RTCONFIG_DISABLE_REPEATER_UI),y)
OBJS += obd_bluecave.o
endif
endif
endif

ifeq ($(RTCONFIG_WPS),y)
OBJS += wpsaide.o
endif

ifeq ($(RTCONFIG_CONCURRENTREPEATER),y)
ifneq ($(RPAC68U),y)
OBJS += led_monitor.o
endif
ifeq ($(RTCONFIG_RALINK),y)
OBJS += re_wpsc.o
endif
endif

ifeq ($(RTN65U),y)
OBJS += asm1042.o
endif

ifeq ($(RTCONFIG_VPNC),y)
ifeq ($(RTCONFIG_VPN_FUSION),y)
OBJS += vpnc_fusion.o
else
OBJS += vpnc.o
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

ifneq ($(BUILD_NAME), RT-AC56U)
ifneq (,$(filter y,$(RTCONFIG_WLAN_LED) $(RTN18U) $(RTCONFIG_FAKE_ETLAN_LED) $(SW_USBLED) $(RTCONFIG_MMC_LED) $(RTCONFIG_BRCM_USBAP) $(RTAC66U) $(BCM4352) $(SW_DSLWANLED) $(RTCONFIG_SW_DEVLED)))
CFLAGS += -DSW_DEVLED
endif
endif

ifeq ($(RTCONFIG_TIMEMACHINE),y)
OBJS += timemachine.o
endif

ifeq ($(RTCONFIG_LETSENCRYPT),y)
CFLAGS += -I$(TOP)/libletsencrypt
LDFLAGS += -L$(TOP)/libletsencrypt -lletsencrypt
endif

ifeq ($(BUILD_NAME), GT-AC5300)
CFLAGS += -DSUPPORT_GAME_PROFILE
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
endif

OBJS += hour_monitor.o

ifeq ($(RTCONFIG_TRAFFIC_LIMITER),y)
OBJS += traffic_limiter.o
endif

ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
OBJS += nt_mail.o
endif

ifeq ($(RTCONFIG_IPERF),y)
OBJS += monitor.o
endif


ifeq ($(RTCONFIG_HTTPS),y)
ifeq ($(or $(RTCONFIG_PUSH_EMAIL),$(RTCONFIG_NOTIFICATION_CENTER)),y)
OBJS += pwdec.o
endif
endif

ifeq ($(RTCONFIG_INTERNAL_GOBI),y)
OBJS += lteled.o
endif

ifeq ($(RTCONFIG_TR069),y)
OBJS += tr069.o
endif

ifeq ($(RTCONFIG_NEW_USER_LOW_RSSI),y)
ifeq ($(RTCONFIG_ADV_RAST),y)
ifeq ($(RTCONFIG_CFGSYNC),y)
CFLAGS += -I$(TOP)/json-c -I$(TOP)/cfg_mnt
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
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
endif

ifeq ($(RTCONFIG_TCODE),y)
OBJS += tcode_rc.o
ifeq ($(CONFIG_BCMWL5),y)
OBJS += tcode_brcm.o
endif
endif

ifeq ($(or $(RTCONFIG_LACP),$(RTCONFIG_BONDING)),y)
OBJS += agg_brcm.o
endif

ifeq ($(and $(CONFIG_BCMWL5),$(RTCONFIG_DUALWAN)),y)
ifneq ($(HND_ROUTER),y)
OBJS += dualwan.o
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
endif

ifneq ($(RTCONFIG_RALINK)$(RTCONFIG_QCA)$(RTCONFIG_REALTEK),y)
OBJS += erp_monitor.o
endif

ifeq ($(RCONFIG_SW_HW_AUTH), y)
CLFAGS += -I(TOP)/sw-hw-auth
endif

ifeq ($(RTCONFIG_AMAS),y)
vpath %.c sysdeps/amas 
LDFLAGS += -L$(TOP)/amas-utils -lamas-utils -lpthread
CFLAGS += -I$(TOP)/amas-utils -I$(TOP)/shared/sysdeps/amas -D_REENTRANT -pthread -DPTHREAD_STACK_SIZE_128K -DUSE_GET_TLV_SUPPORT_MAC
ifneq ($(RTCONFIG_DISABLE_REPEATER_UI),y)
OBJS += amas_wlcconnect.o
OBJS += amas_bhctrl.o
endif
OBJS += amas_lanctrl.o
ifeq ($(RTCONFIG_LIBASUSLOG),y)
LDFLAGS += -L$(TOP)/libasuslog -lasuslog
CFLAGS += -I$(TOP)/libasuslog
endif
endif

all: rc init.prep

rc: $(OBJS)
	@echo " [rc] CC $@"
	@$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

	$(SIZECHECK)
	$(CPTMP)

install: all
	@echo "[rc] Installing..."
	@install -D rc $(INSTALLDIR)/sbin/rc
	@$(STRIP) $(INSTALLDIR)/sbin/rc
	@chmod 0500 $(INSTALLDIR)/sbin/rc

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
	@cd $(INSTALLDIR)/sbin && ln -sf rc reboot
	@cd $(INSTALLDIR)/sbin && ln -sf rc halt
	@cd $(INSTALLDIR)/sbin && ln -sf rc netool
ifeq ($(HND_ROUTER),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc hnd-write
	@cd $(INSTALLDIR)/sbin && ln -sf rc hnd-erase
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd_erase_image_update
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
	@cd $(INSTALLDIR)/sbin && ln -sf rc ots
	@cd $(INSTALLDIR)/sbin && ln -sf rc watchdog
	@cd $(INSTALLDIR)/sbin && ln -sf rc wave_monitor
ifeq ($(RTCONFIG_LANTIQ),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc restart_bluetoothd
	@cd $(INSTALLDIR)/sbin && ln -sf rc set_usb3_to_usb2
	@cd $(INSTALLDIR)/sbin && ln -sf rc set_usb2_to_usb3
	@cd $(INSTALLDIR)/sbin && ln -sf rc update_client
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
ifeq ($(RTCONFIG_FANCTRL),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc phy_tempsense
endif
ifeq ($(RTCONFIG_BCMWL6),y)
ifeq ($(RTCONFIG_PROXYSTA),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc psta_monitor
endif
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
	@cd $(INSTALLDIR)/sbin && ln -sf rc ntp
	@cd $(INSTALLDIR)/sbin && ln -sf rc autodet
ifeq ($(RTCONFIG_DETWAN),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc detwan
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc ddns_updated
	@cd $(INSTALLDIR)/sbin && ln -sf rc ddns_custom_updated

#	@cd $(INSTALLDIR)/sbin && ln -sf rc wldist

ifeq ($(RTCONFIG_CONCURRENTREPEATER),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc led_monitor
	@cd $(INSTALLDIR)/sbin && ln -sf rc re_wpsc
endif

ifeq ($(RTCONFIG_WPS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc wpsaide
ifeq ($(RTCONFIG_QCA),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc get_wps_er
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
ifeq ($(RTAC53)$(RTAC51UP),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtkswitch
endif
endif

ifeq ($(RTCONFIG_EXT_RTL8365MB),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc rtkswitch
endif
ifeq ($(RTCONFIG_EXT_RTL8370MB),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc rtkswitch
endif

ifeq ($(RTCONFIG_QCA),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc rtkswitch
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc delay_exec

	@cd $(INSTALLDIR)/sbin && ln -sf rc wanduck
	@cd $(INSTALLDIR)/sbin && ln -sf rc conn_diag
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
ifeq ($(RTCONFIG_LP5523),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc lp55xx_set_led
	@cd $(INSTALLDIR)/sbin && ln -sf rc lp55xx_user_set
endif
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
ifeq ($(RTCONFIG_SOC_IPQ8064),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc readmem
endif
ifeq ($(RTCONFIG_HTTPS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc rsasign_check
	@cd $(INSTALLDIR)/sbin && ln -sf rc rsarootca_check
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc test_endian
ifeq ($(RTCONFIG_BWDPI),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc rsasign_sig_check
	@cd $(INSTALLDIR)/sbin && ln -sf rc bwdpi
	@cd $(INSTALLDIR)/sbin && ln -sf rc bwdpi_check
	@cd $(INSTALLDIR)/sbin && ln -sf rc bwdpi_wred_alive
	@cd $(INSTALLDIR)/sbin && ln -sf rc bwdpi_db_10
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
endif
endif
ifeq ($(RTCONFIG_IPERF),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc monitor
endif
ifeq ($(RTCONFIG_INTERNAL_GOBI),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc lteled
endif
ifeq ($(RTCONFIG_TR069),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc dhcpc_lease
endif

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
endif

ifeq ($(RTCONFIG_QCA),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc dump_powertable
endif

ifeq ($(RTCONFIG_RALINK),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc dump_powertable
	@cd $(INSTALLDIR)/sbin && ln -sf rc dump_txbftable
	@cd $(INSTALLDIR)/sbin && ln -sf rc air_monitor
endif

ifneq ($(RTCONFIG_RALINK)$(RTCONFIG_QCA)$(RTCONFIG_REALTEK),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc erp_monitor
endif

ifeq ($(RTCONFIG_AMAS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_wlcconnect
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_bhctrl	
	@cd $(INSTALLDIR)/sbin && ln -sf rc amas_lanctrl
endif

ifeq ($(RTCONFIG_LETSENCRYPT),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc le_acme
endif

ifeq ($(RTCONFIG_WLCSCAN_RSSI),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc wlcscan_ssid_rssi
endif

ifeq ($(RTCONFIG_ADTBW),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc adtbw
endif

clean:
	rm -f rc *.o .*.depend *.prep

size: rc
	mipsel-uclibc-nm --print-size --size-sort rc

# handle prebuilt object here
ifneq ($(wildcard ./prebuild/dsl_fb.o),)
ifeq ($(RTCONFIG_PUSH_EMAIL),y)
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
ifeq ($(RTCONFIG_HTTPS),y)
ifeq ($(RTCONFIG_PUSH_EMAIL),y)
dsl_fb.o:dsl_fb.c
#	@echo "encrypt dsl password"
#	./dsl_fb_enc.sh
#	@$(CC) $(CFLAGS) -c dsl_fb.c
#	@echo "decrypt dsl password"
#	./dsl_fb_dec.sh
endif
endif
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
ifneq ($(wildcard ./prebuild/ate.o),)
ate.o:
	@-cp -f ./prebuild/ate.o .
endif
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AC68U RT-AC3200))
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
endif

ifeq ($(RTCONFIG_QCA),y)
ifneq ($(wildcard ./prebuild/ate-qca.o),)
ate-qca.o:
	@-cp -f ./prebuild/ate-qca.o .
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
endif

ifneq ($(wildcard ./prebuild/obd.o),)
obd.o:
	@-cp -f ./prebuild/obd.o .
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
ifneq ($(wildcard ./prebuild/roamast-latiq.o),)
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
