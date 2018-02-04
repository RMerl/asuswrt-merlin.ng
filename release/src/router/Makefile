#
# Broadcom Linux Router Makefile
#
# Copyright 2016, Broadcom
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
#

export HND_ROUTER := $(shell pwd | sed 's/.*hnd.*/y/g')
ifeq ($(HND_ROUTER),y)
export BCM_CHIP := 4908
export HND_SRC := $(shell pwd | sed 's/\(.*src-rt-.*hnd\).*/\1/')
export SRCBASE := $(shell pwd | sed 's/\(.*src-rt-.*hnd\).*/\1\/bcmdrivers\/broadcom\/net\/wl\/bcm9$(BCM_CHIP)\/main\/src/')
PREBUILT_BCMBIN=$(shell if [ ! -f "$(HND_SRC)/upb.mak" -o "$(PRBM)" = "1" ]; then echo 1; else echo 0; fi)
export PREBUILT_BCMBIN
export PRBM_DIR=prebuilt
include $(SRCBASE)/.config
ifeq ($(BCM_FSBUILD_DIR),)
include $(HND_SRC)/make.common
endif
include $(HND_SRC)/targets/$(PROFILE)/$(PROFILE).$(BUILD_NAME)
BUILD_DIR:=$(HND_SRC)
export ALLSRCBASE= $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)
export WLSRCBASE= $(ALLSRCBASE)/main/src
export WLCFGDIR = $(ALLSRCBASE)/main/src/wl/config
export WLMAIN = main/src
else
export HND_ROUTER := n
endif

include common.mak
ifneq ($(HND_ROUTER),y)
include $(SRCBASE)/.config
endif

ifeq ($(RTCONFIG_RALINK),y)
else ifeq ($(or $(RTCONFIG_QCA),$(RTCONFIG_REALTEK)),y)
export CFLAGS += $(EXTRACFLAGS)
else
# Broadcom
export CFLAGS += -DBCMWPA2
ifeq ($(RTCONFIG_BCMWL6),y)
export CFLAGS += -DBCMQOS
export CFLAGS += -DEXT_ACS
export CFLAGS += -DD11AC_IOTYPES
export CFLAGS += -DPHYMON
export CFLAGS += -DPROXYARP
export CFLAGS += -DTRAFFIC_MGMT
export CFLAGS += -DTRAFFIC_MGMT_RSSI_POLICY
ifeq ($(RTCONFIG_BCMARM),y)
export CONFIG_MFP=y
export CFLAGS += -DMFP
export CFLAGS += -D__CONFIG_MFP__
endif
endif
ifeq ($(HND_ROUTER),n)
export CFLAGS += $(EXTRACFLAGS)
else
export CFLAGS += -DHND_ROUTER -DBCA_HNDROUTER -DMCPD_PROXY
export EXTRA_NV_LDFLAGS := -L$(TOP)/wlcsm -lwlcsm
export EXTRA_LDFLAGS := -lgcc_s
ifeq ($(TOPBUILD),y)
export CFLAGS += $(HND_CMN_CFLAGS)
endif
endif
endif

# New Broadcom/Ralink/MTK/QCA/RTK platform.
KPATH_2636_OR_3X = $(if $(or $(findstring linux-2.6.36,$(LINUXDIR)),$(findstring linux-3.x,$(LINUXDIR)),$(findstring linux-3.3.x,$(LINUXDIR)),$(findstring linux-3.3.8,$(LINUXDIR)),$(findstring linux-3.4.x,$(LINUXDIR)),$(findstring linux-4.1,$(LINUXDIR)),$(findstring linux-3.10,$(LINUXDIR)),$(findstring linux-3.14.x,$(LINUXDIR))),y)

# New Ralink/MTK/QCA platform.
KPATH_2636X_OR_3X = $(if $(or $(findstring linux-2.6.36.x,$(LINUXDIR)),$(findstring linux-3.x,$(LINUXDIR)),$(findstring linux-3.3.x,$(LINUXDIR)),$(findstring linux-3.3.8,$(LINUXDIR)),$(findstring linux-3.4.x,$(LINUXDIR)),$(findstring linux-3.14.x,$(LINUXDIR))),y)

# New Ralink/MTK/QCA/RTK platform.
KPATH_3X = $(if $(or $(findstring linux-3.x,$(LINUXDIR)),$(findstring linux-3.3.x,$(LINUXDIR)),$(findstring linux-3.3.8,$(LINUXDIR)),$(findstring linux-3.4.x,$(LINUXDIR)),$(findstring linux-3.10,$(LINUXDIR)),$(findstring linux-3.14.x,$(LINUXDIR))),y)

ifneq ($(BASE_MODEL),)
MODEL = $(subst -,,$(subst +,P,$(BASE_MODEL)))
else ifneq ($(findstring 4G-,$(BUILD_NAME)),)
MODEL = RT$(subst -,,$(BUILD_NAME))
else ifneq ($(findstring DSL,$(BUILD_NAME)),)
MODEL = $(subst -,_,$(BUILD_NAME))
else
MODEL = $(subst -,,$(subst +,P,$(BUILD_NAME)))
endif
export CFLAGS += -D$(MODEL)
export $(MODEL)=y

export OLD_SRC=$(SRCBASE)/../src
ifeq ($(RTCONFIG_REALTEK),y)
# do nothing #
else ifeq ($(RTCONFIG_BCMARM),y)
#export CFLAGS += -I$(SRCBASE)/common/include

ifeq ($(HND_ROUTER),y)
export LINUX_VERSION=4_1_0
export LINUX_KERNEL=4.1.27
export KBUILD_VERBOSE := 1
export BUILD_MFG := 0
SUBMAKE_SETTINGS = SRCBASE=$(SRCBASE) BASEDIR=$(BASEDIR)
else
LINUX_VERSION=2_6_36
LINUX_KERNEL=2.6.36
endif
export PLATFORM_ARCH LIBDIR USRLIBDIR LINUX_VERSION

ifeq ($(RTCONFIG_BCM7),y)
export BCMSRC=src-rt-7.x.main/src
export OLD_SRC=$(SRCBASE)/../../src
export BUILD_MFG := 0
export EX7 := _7
else ifeq ($(HND_ROUTER),y)
export BCMSRC=src-rt-5.02hnd
export OLD_SRC=$(HND_SRC)/../src
export BUILD_MFG := 0
export EX7 := _94908hnd
export EXHND := _hnd
else ifeq ($(RTCONFIG_BCM_7114),y)
export BCMSRC=src-rt-7.14.114.x/src
export OLD_SRC=$(SRCBASE)/../../src
export BUILD_MFG := 0
export EX7 := _7114
else ifeq ($(RTCONFIG_BCM9),y)
export BCMSRC=src-rt-9.x/src
export OLD_SRC=$(SRCBASE)/../../src
export BUILD_MFG := 0
export EX7 := _9
else
WLAN_ComponentsInUse := bcmwifi clm ppr olpc
export BCMSRC=src-rt-6.x.4708
endif
ifeq ($(HND_ROUTER),y)
export EXTRALDFLAGS = -lgcc_s
export LD_LIBRARY_PATH := $(TOOLCHAIN)/lib
export BASEDIR := $(SRCBASE)/../
else
include $(SRCBASE)/makefiles/WLAN_Common.mk
export BASEDIR := $(WLAN_TreeBaseA)
export LINUXDIR := $(SRCBASE)/linux/linux-2.6.36

export EXTRALDFLAGS = -lgcc_s
export EXTRALDFLAGS2 = -L$(TOP)/nvram$(BCMEX)$(EX7) -lnvram -L$(TOP)/shared -lshared


export LD_LIBRARY_PATH := $(SRCBASE)/toolchains/hndtools-arm-linux-2.6.36-uclibc-4.5.3/lib
endif
ifeq (2_6_36,$(LINUX_VERSION))
export BUILD_MFG := 0
endif
SUBMAKE_SETTINGS = SRCBASE=$(SRCBASE) BASEDIR=$(BASEDIR)
SUBMAKE_SETTINGS += ARCH=$(ARCH)
export CFLAGS += -O2
export OPTCFLAGS = -O2
WLCFGDIR=$(SRCBASE)/wl/config

CRAMFSDIR := cramfs

export MKSYM := $(shell which $(TOP)/misc/mksym.pl)
endif

ifeq ($(RTCONFIG_REALTEK),y)
# do nothing #
else ifeq ($(RTCONFIG_BCMWL6),y)
ifneq ($(RTCONFIG_BCMARM),y)
LINUX_VERSION=2_6
LINUX_KERNEL=2.6.22.19
endif
ifeq ($(or $(RTCONFIG_BCMWL6),$(RTAC53U)),y)
WLAN_ComponentsInUse := bcmwifi clm ppr
ifeq ($(or $(RTCONFIG_BCMWL6A),$(RTAC53U)),y)
WLAN_ComponentsInUse += olpc
endif
-include ../../$(SRCBASE)/makefiles/WLAN_Common.mk
endif
endif

ifeq ($(RTCONFIG_REALTEK),y)
KDIR=$(TOP)/kernel_header
KERNEL_HEADER_DIR=$(TOP)/kernel_header/include
else ifeq ($(RTCONFIG_LANTIQ),y)
KDIR=$(TOP)/kernel_header
KERNEL_HEADER_DIR=$(TOP)/kernel_header/include
else ifneq ($(RTCONFIG_BCMARM),y)
KDIR=$(TOP)/kernel_header
KERNEL_HEADER_DIR=$(TOP)/kernel_header/include
else
ifeq ($(HND_ROUTER),y)
KDIR=$(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/usr
KERNEL_HEADER_DIR=$(KDIR)/include
TOOL_SYS_INCLUDE=$(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/usr/include
ifeq ($(RTCONFIG_VISUALIZATION),y)
export TOOLCHAIN_LIB_DIR=$(KDIR)/lib
export SRCBASE_ROUTER=$(SRCBASE)/router
endif
export CROSS_COMPILE_64 := /opt/toolchains/crosstools-aarch64-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25/usr/bin/aarch64-buildroot-linux-gnu-
export STRIP_64 := $(CROSS_COMPILE_64)strip
export TOOLCHAIN_64 := $(shell cd $(dir $(shell which $(CROSS_COMPILE_64)gcc))/.. && pwd -P)
export TOOL_SYS_INCLUDE_64=$(TOOLCHAIN_64)/aarch64-buildroot-linux-gnu/sysroot/usr/include
export PATH:=$(PATH):/opt/toolchains/crosstools-aarch64-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25/usr/bin
else
KDIR=$(LINUXDIR)
KERNEL_HEADER_DIR=$(LINUXDIR)/include
endif
endif

ifeq ($(HND_ROUTER),y)
export LINUX_INC_DIR := $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/usr
else
export LINUX_INC_DIR := $(LINUXDIR)
endif

export KDIR KERNEL_HEADER_DIR
ifeq ($(RTCONFIG_DHDAP),y)
export CONFIG_DHDAP=y
export CFLAGS += -D__CONFIG_DHDAP__
ifeq ($(RTCONFIG_BCM_7114),y)
export SRCBASE_DHD := $(SRCBASE)/../dhd/src
export SRCBASE_DHD24 := $(SRCBASE)/../dhd24/src
export SRCBASE_FW := $(SRCBASE)/../4365/src
export SRCBASE_SYS := $(SRCBASE_DHD)
include Makefile.fw
else ifeq ($(HND_ROUTER),y)
export SRCBASE_DHD := $(SRCBASE)/../../dhd/src
export SRCBASE_FW := $(SRCBASE)/../../4365/src
export SRCBASE_SYS := $(SRCBASE_DHD)
include Makefile.fw
else	# 7114
export DHDAP_USE_SEPARATE_CHECKOUTS := 1
export SRCBASE_DHD := $(SRCBASE)/../../dhd/src
export SRCBASE_FW  := $(SRCBASE)/../../43602/src
PCIEFD_TARGETS_LIST	:= 43602a1-roml
ifeq ($(WLTEST),1)
PCIEFD_TARGET_NAME	:= pcie-ag-splitrx-fdap-mbss-mfgtest-seqcmds-phydbg-txbf-pktctx-amsdutx-ampduretry-chkd2hdma
else
PCIEFD_TARGET_NAME	:= pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus
endif

PCIEFD_EMBED_HEADER_TEMPLATE := $(SRCBASE_DHD)/shared/rtecdc_router.h.in
PCIEFD_EMBED_HEADER	:= $(SRCBASE_DHD)/shared/rtecdc_router.h
obj-pciefd		:= $(patsubst %,%-obj,$(PCIEFD_TARGETS_LIST))
install-pciefd		:= $(patsubst %,%-install,$(PCIEFD_TARGETS_LIST))
endif	# 7114
endif

ifeq ($(HND_ROUTER),y)
ifeq ($(RTCONFIG_VISUALIZATION),y)
VISSOURCE_BASE_DIR := $(BASEDIR)/components/apps/visualization
VISDCOLL_DIR := $(VISSOURCE_BASE_DIR)/datacollector
VISDCON_DIR := $(VISSOURCE_BASE_DIR)/dataconcentrator
export SQLITE3_DIR := $(BASEDIR)/components/opensource/sqlite3
endif
export WBDSOURCE_DIR := $(BASEDIR)/components/apps/wbd
export EX_ROM := rom
else ifeq ($(RTCONFIG_BCM_7114),y)
export WBDSOURCE_DIR := $(BASEDIR)/components/apps/wbd
endif

ifeq ($(RTCONFIG_WLEXE),y)
ifeq ($(RTCONFIG_BCM9),y)
export SRCBASE_SYS := $(SRCBASE)/../sys/src
export CONFIG_WLEXE
export RWL ?= 0
endif
endif

ifeq ($(RTCONFIG_LBR_AGGR), y)
export CONFIG_LBR_AGGR
export CFLAGS += -D__CONFIG_LBR_AGGR__
endif

ifeq ($(RTCONFIG_GMAC3),y)
export CFLAGS += -D__CONFIG_GMAC3__
endif

ifeq ($(RTCONFIG_BRCM_USBAP),y)
export CFLAGS += -D__CONFIG_USBAP__
endif

ifeq ($(RTCONFIG_TOAD),y)
export CFLAGS += -D__CONFIG_TOAD__
endif

ifeq ($(RTCONFIG_BCM_APPEVENTD),y)
export CFLAGS += -DBCM_APPEVENTD
endif

ifeq ($(RTCONFIG_BCMBSD),y)
export CFLAGS += -DBCM_BSD
endif

ifeq ($(RTCONFIG_LANTIQ),y)
export CFLAGS += -DLANTIQ_BSD
endif

ifeq ($(RTCONFIG_BCMSSD),y)
export CFLAGS += -DBCM_SSD
endif

ifeq ($(RTCONFIG_WLCLMLOAD),y)
export CFLAGS += -DWLCLMLOAD
export WLCLMLOAD := 1
endif

# BCA_HNDROUTER mfg flag
ifneq ($(BUILD_HND_MFG),)
export BUILD_MFG := 1
export WLTEST := 1
endif

ifeq ($(RTCONFIG_FBT),y)
export CFLAGS  += -DWLHOSTFBT
endif

ifeq ($(RTCONFIG_BCMASPMD),y)
export CFLAGS += -DBCM_ASPMD
endif

ifeq ($(RTCONFIG_WBD),y)
export CFLAGS += -DBCM_WBD
ifeq ($(RTCONFIG_PLCWBD), y)
export PLC_WBD := 1
endif # PLC_WBD
endif

ifeq ($(RTCONFIG_WBD),y)
export CFLAGS += -DBCM_WBD
ifeq ($(RTCONFIG_PLCWBD), y)
export PLC_WBD := 1
endif # PLC_WBD
endif # WBD

ifeq ($(RTCONFIG_BCMEVENTD),y)
export CFLAGS += -DBCM_EVENTD
endif

ifeq ($(RTCONFIG_BCMDCS),y)
export CFLAGS += -DBCM_DCS
endif

ifeq ($(RTCONFIG_EMF),y)
export CFLAGS += -D__CONFIG_EMF__
export CONFIG_EMF_ENABLED := $(RTCONFIG_EMF)
endif

ifeq ($(RTCONFIG_HSPOT),y)
export CONFIG_HSPOT=y
#export CFLAGS += -D__CONFIG_HSPOT__
export CFLAGS += -DNAS_GTK_PER_STA
ifeq ($(or $(RTCONFIG_BCM7),$(RTCONFIG_BCM_7114)),y)
export CFLAGS += -DHSPOT_OSEN
endif
ifeq ($(or $(RTCONFIG_BCM7114),$(RTCONFIG_HND_ROUTER)),y)
export ICONPATH := /www/hspot
export CFLAGS += -DICONPATH=\"$(ICONPATH)\"
endif
endif

ifeq ($(RTCONFIG_VISUALIZATION),y)
export CFLAGS += -D__CONFIG_VISUALIZATION__ -DCONFIG_VISUALIZATION_ENABLED
endif

ifeq ($(RTCONFIG_WPS),y)
ifeq ($(CONFIG_BCMWL5),y)
endif
export CFLAGS += -D__CONFIG_WPS__
# WFA WPS 2.0 Testbed extra caps
#export CFLAGS += -DWFA_WPS_20_TESTBED
endif

ifeq ($(RTCONFIG_NFC),y)
# WPS_NFC
export CFLAGS += -D__CONFIG_NFC__
endif

ifeq ($(RTCONFIG_NORTON),y)
export CFLAGS += -D__CONFIG_NORTON__
endif

ifeq ($(RTCONFIG_JFFS_NVRAM),y)
export CFLAGS += -DJFFS_NVRAM
endif

ifeq ($(RTCONFIG_DEBUG),y)
export CFLAGS += -g
endif

ifeq ($(strip $(BRCM_USER_SSP)),y)
export CFLAGS += $(SSP_MIN_COMPILER_OPTS)
export EXTRA_LDFLAGS += $(SSP_LIBS)
endif

ifneq (,$(filter y,$(RTCONFIG_SFP) $(RTCONFIG_4M_SFP)))
export MUVER := _1.4
endif

export NONOPT_CFLAGS := $(shell echo $(CFLAGS)| sed 's/-O2//g')
ifeq ($(HND_ROUTER),y)
export CFLAGS := $(shell echo $(CFLAGS)| sed 's/-fno-common//g')
endif

ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
KERNEL_PTHREAD_ONE = $(if $(or $(findstring linux-3.4.x,$(LINUXDIR)),$(findstring linux-4.1,$(LINUXDIR)),$(findstring linux-3.10,$(LINUXDIR)),$(findstring linux-3.14.x,$(LINUXDIR))),y)
export CONFIG_KERNEL_PTHREAD_ONE = $(KERNEL_PTHREAD_ONE)
endif
#
#
#
SEP=echo -e "\033[41;1m   $@   \033[0m"

#
# standard packages
#
ifeq ($(HND_ROUTER),y)
export BUSYBOX ?= busybox-1.24.1
export BUSYBOX_DIR := busybox-1.24.1/busybox-1.24.1
else
export BUSYBOX ?= busybox
export BUSYBOX_DIR := $(BUSYBOX)
endif
obj-y += $(BUSYBOX)
obj-y += shared
obj-y += nvram$(BCMEX)$(EX7)
ifeq ($(RTCONFIG_BCMARM),y)
ifneq ($(HND_ROUTER),y)
obj-y += nvram_arm
endif
endif

obj-$(RTCONFIG_LIBASUSLOG) += libasuslog

obj-$(RTCONFIG_SW_HW_AUTH) += sw-hw-auth
ifeq ($(RTCONFIG_TUNNEL),y)
obj-$(RTCONFIG_NOTIFICATION_CENTER) += wb
endif
obj-$(RTCONFIG_NOTIFICATION_CENTER) += json-c
obj-$(RTCONFIG_NOTIFICATION_CENTER) += sqlite
obj-$(RTCONFIG_NOTIFICATION_CENTER) += nt_center
obj-$(RTCONFIG_NOTIFICATION_CENTER) += email-3.1.3
obj-$(RTCONFIG_NOTIFICATION_CENTER) += wlc_nt

obj-$(RTCONFIG_PROTECTION_SERVER) += protect_srv

obj-$(RTCONFIG_CFGSYNC) += openssl libevent-2.0.21 cfg_mnt

ifneq ($(HND_ROUTER),y)
obj-$(RTCONFIG_QTN) += libqcsapi_client
obj-$(RTCONFIG_QTN) += qtnimage
obj-$(RTCONFIG_QSR10G) += libqcsapi_client_10g
obj-$(RTCONFIG_DMALLOC) += dmalloc

obj-$(RTCONFIG_BCMARM) += ctf_arm
obj-$(RTAC53U) += ctf_5358
ifneq ($(RTCONFIG_RALINK)$(RTCONFIG_QCA)$(RTCONFIG_REALTEK)$(RTCONFIG_QCA)$(RTCONFIG_ALPINE),y)
obj-y += ctf
endif
endif
obj-$(RTCONFIG_DHDAP) += dhd $(if $(RTCONFIG_BCM7),pciefd)

ifeq ($(or $(RTCONFIG_BCM_7114),$(HND_ROUTER)),y)
obj-y += dhd_monitor
endif
obj-$(RTCONFIG_DPSTA) += dpsta

ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_CFEZ),y)
obj-y += envram_bin
endif
endif

ifeq ($(RTCONFIG_RALINK),y)
#################################################################################
# Ralink/MTK-specific packages
#################################################################################
obj-y += wireless_tools

obj-y += $(if $(RTN65U),rt2880_app,flash)
ifeq ($(RTN65U),)
obj-y += $(if $(or $(RTN14U),$(RTAC52U),$(RTAC51U),$(RTN11P),$(RTN300),$(RTN54U),$(RTAC1200HP),$(RTN56UB1),$(RTN56UB2),$(RTAC54U),$(RTAC51UP),$(RTAC53),$(RTAC1200),$(RTAC1200GA1),$(RTAC1200GU),$(RTN600),$(RTN11PB1),$(RPAC87),$(RTAC85U),$(RTN800HP)),reg,hw_nat)
obj-y += $(if $(RTCONFIG_ATED122),ated_1.2.2,ated)
endif
obj-y += $(if $(RTCONFIG_MII_MGR),mii_mgr)

ifeq ($(RTN800HP),y)
obj-y += switch
endif

ifeq ($(RPAC87),y)
obj-y += mpstat-0.1.1
obj-y += tcpdump-4.4.0
obj-y += switch
endif

else ifeq ($(RTCONFIG_QCA),y)
#################################################################################
# QCA-specific packages
#################################################################################
OLD_QCA :=
obj-y += wireless_tools
obj-y += libnl-tiny-0.1
ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
SWCONFIG_PKG := qca-swconfig.ipq40xx
else
SWCONFIG_PKG := swconfig
endif
obj-y += $(SWCONFIG_PKG)

# QCA9558.ILQ 2.0
obj-$(RTCONFIG_SOC_QCA9557) += shortcut-fe
obj-$(PLAC66U) += shortcut-fe
obj-$(RPAC51) += shortcut-fe.collard
ifeq ($(SWITCH_CHIP),QCA8337N)
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-ssdk30
endif

ifeq ($(or $(RTCONFIG_WIFI_QCA9557_QCA9882),$(QCA953X),$(QCA956X)),y)
ifeq ($(RPAC51),y)
obj-y += qca-wifi.collard-10.4
else ifeq ($(MESH),y)
else
obj-y += qca-wifi
endif
endif
ifeq ($(or $(RTCONFIG_WIFI_QCA9557_QCA9882),$(QCA953X),$(QCA956X)),y)
ifeq ($(RPAC51),y)
obj-y += qca-hostap-collard.10.4
else ifeq ($(MESH),y)
else
obj-y += qca-hostap
obj-$(ART2) += LinuxART2
endif
endif

ifeq ($(RPAC51),y)
obj-y += libnl-bf
obj-y += qcmbr.collard
obj-y += qca-wifi-fw-qca9888
#obj-y += qca-ssdk.collard
#obj-y += qca-ssdk-shell.collard
endif
obj-$(MAPAC1750) += libnl-bf

# IPQ8064.ILQ 3.0
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-wifi-qca9994
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-wifi-fw-qca9994
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qcmbr
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-hostap-qca9994 libnl-bf openssl
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += $(if $(RTCONFIG_SWITCH_QCA8337N),qca-mcs30-lkm)
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-nss30-fw
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-nss30-drv
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-nss30-gmac
#obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-nss30-macsec	# need IPQ8066, IPQ8068
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-nss30-ecm shortcut-fe30
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-nss30-crypto
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-nss30-cfi	# need kmod-crypto-ocf
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-nss30-clients
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-thermald
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += shortcut-fe30 libnl-bf
obj-$(RTCONFIG_WIFI_QCA9994_QCA9994) += qca-whc30-lbd libqcacommon

# IPQ8064.ILQ 1.3.7
obj-$(RTCONFIG_WIGIG) += qca-backports wigig-firmware iw iwinfo libnl-tiny-0.1

# IPQ8064.ILQ 1.3, IPQ8064.ILQ 3.0
obj-$(RTCONFIG_SOC_IPQ8064) += devmem2
obj-$(RTCONFIG_SOC_IPQ8064) += taskset

# IPQ4019 ILQ 1.1
#obj-$(RTCONFIG_SOC_IPQ40XX) += qca-nss-drv.ipq40xx
#obj-$(RTCONFIG_SOC_IPQ40XX) += qca-nss-gmac.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-wifi.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-wifi-fw.ipq40xx
obj-$(RTCONFIG_PCIE_QCA9984) += qca-wifi-fw.qca9984
obj-$(RTCONFIG_PCIE_QCA9888) += qca-wifi-fw.qca9888
obj-$(RTCONFIG_SOC_IPQ40XX) += qcmbr.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-hostap.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-rfs.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += shortcut-fe.ipq40xx libnl-bf
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-ssdk.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-ssdk-shell.ipq40xx

obj-$(RTCONFIG_SOC_IPQ40XX) += ethtool-3.7
obj-$(RTCONFIG_SOC_IPQ40XX) += stress-1.x
ifeq ($(MESH),y)
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-hyfi-qdisc.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-hyfi-bridge.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-libhyfi-bridge.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-libhyficommon.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-libieee1905.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-libwpa2.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-whc-lbd.ipq40xx
obj-$(RTCONFIG_QCA) += jansson-2.7
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-wsplcd.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-hyctl.ipq40xx
obj-$(RTCONFIG_SOC_IPQ40XX) += qca-hyd.ipq40xx
endif

obj-$(RTCONFIG_NOTIFICATION_CENTER) += qca-wifi-assoc-eventd
obj-$(PLC_UTILS) += plc-utils

#################################################################################
# Alpine-specific packages
#################################################################################
else ifeq ($(RTCONFIG_ALPINE),y)
obj-y += wireless_tools
#################################################################################
# Lantiq-specific packages
#################################################################################
else ifeq ($(RTCONFIG_LANTIQ),y)
obj-y += wireless_tools
else
#################################################################################
# Broadcom-specific packages
#################################################################################
ifneq ($(RTCONFIG_BCMARM),y)
obj-y += lzma-loader
endif

endif

obj-$(RTCONFIG_CLOUDSYNC) += neon
LIBBLUETOOTH_VER=3.18.13
ifeq ($(RTCONFIG_REALTEK)$(RTCONFIG_ALPINE),y)
obj-$(RTCONFIG_BT_CONN) += rtk_hciattach
obj-$(RTCONFIG_BT_CONN) += rtk_bluetooth_firmware
obj-$(RTCONFIG_BT_CONN) += ncurses-6.0
obj-$(RTCONFIG_BT_CONN) += libical-2.0.0
obj-$(RTCONFIG_BT_CONN) += udev-173
obj-$(RTCONFIG_BT_CONN) += readline-6.2
obj-$(RTCONFIG_BT_CONN) += libffi-3.0.11
obj-$(RTCONFIG_BT_CONN) += libiconv-1.14
obj-$(RTCONFIG_BT_CONN) += zlib
obj-$(RTCONFIG_BT_CONN) += glib-2.37.7
obj-$(RTCONFIG_BT_CONN) += expat-2.0.1
obj-$(RTCONFIG_BT_CONN) += dbus-1.8.8
obj-$(RTCONFIG_BT_CONN) += bluez-5.41
else
obj-$(RTCONFIG_BT_CONN) += bluez-5.41
obj-$(RTCONFIG_BT_CONN) += glib-2.37.7
obj-$(RTCONFIG_BT_CONN) += libffi-3.0.11
obj-$(RTCONFIG_BT_CONN) += dbus-1.8.8
obj-$(RTCONFIG_BT_CONN) += libiconv-1.14
obj-$(RTCONFIG_BT_CONN) += zlib
obj-$(RTCONFIG_BT_CONN) += expat-2.0.1
obj-$(RTCONFIG_BT_CONN) += udev-173
obj-$(RTCONFIG_BT_CONN) += libical-2.0.0
obj-$(RTCONFIG_BT_CONN) += ncurses-6.0
obj-$(RTCONFIG_BT_CONN) += readline-6.2
ifneq ($(RTCONFIG_LANTIQ),y)
obj-$(RTCONFIG_BT_CONN) += i2c-tools-2013-12-15
endif
obj-$(RTCONFIG_BT_CONN) += qca-ar3k-fw
endif

obj-$(RTCONFIG_I2CTOOLS) += i2c-tools-3.1.2
obj-$(RTCONFIG_RGBLED) += aura_sw

ifeq ($(RTCONFIG_DSL),y)
obj-y += dsl_drv_tool
endif

ifneq ($(RTCONFIG_ALPINE)$(RTCONFIG_LANTIQ),y)
ifeq ($(KPATH_2636_OR_3X),y)
IPTABLES := iptables-1.4.x
IPTC_LIBDIR := $(TOP)/$(IPTABLES)/libiptc/.libs
IPTC_LIBS := -lip4tc $(if $(RTCONFIG_IPV6),-lip6tc,) -liptc
obj-y += iptables-1.4.x
ifeq ($(HND_ROUTER),y)
obj-y += iproute2-4.3
else
obj-y += iproute2-3.x
endif
else
IPTABLES := iptables
IPTC_LIBDIR := $(TOP)/$(IPTABLES)
IPTC_LIBS := -liptc
obj-y += iptables
obj-y += iproute2
endif
endif

ifeq ($(RTCONFIG_ALPINE)$(RTCONFIG_LANTIQ),y)
IPTABLES := iptables-1.4.21
IPTC_LIBDIR := $(TOP)/$(IPTABLES)/libiptc/.libs
IPTC_LIBS := -lip4tc $(if $(RTCONFIG_IPV6),-lip6tc,) -liptc
obj-y += iptables-1.4.21
obj-y += iproute2-3.15.0
endif

obj-$(RTCONFIG_PERMISSION_MANAGEMENT) += sqlite
obj-$(RTCONFIG_PERMISSION_MANAGEMENT) += sqlCipher
obj-$(RTCONFIG_PERMISSION_MANAGEMENT) += PMS_DBapis

obj-$(RTCONFIG_SPEEDTEST) += curl-7.21.7

obj-$(RTCONFIG_BWDPI) += sqlite
obj-$(RTCONFIG_BWDPI) += bwdpi_source
obj-$(RTCONFIG_TRAFFIC_LIMITER) += sqlite
obj-$(RTCONFIG_TRAFFIC_LIMITER) += traffic_limiter

obj-$(RTCONFIG_LETSENCRYPT) += openssl
obj-$(RTCONFIG_LETSENCRYPT) += acme-client-portable
ifneq ($(RTCONFIG_WEBDAV),y)
obj-$(RTCONFIG_LETSENCRYPT) += lighttpd-le
endif
obj-$(RTCONFIG_LETSENCRYPT) += libletsencrypt

#obj-$(RTCONFIG_SYSSTATE) += sysstate
obj-y += sysstate

obj-$(RTCONFIG_OPENVPN) += openssl
obj-$(RTCONFIG_OPENVPN) += libvpn

obj-y += rc
obj-y += rom
obj-y += others
obj-y += libpasswd
obj-y += httpd json-c
obj-$(RTCONFIG_VISUALIZATION) += sqlite3 json-c libxml2 visdcoll visdcon
obj-y += www
ifneq ($(HND_ROUTER),y)
obj-y += bridge
endif
obj-y += dnsmasq
ifneq ($(HND_ROUTER),y)
obj-y += etc
endif
# obj-y += vlan # use Busybox vconfig
#obj-y += utils
obj-y += ntpclient

ifeq ($(RTCONFIG_REALTEK),y)
obj-y += rtk_flash
obj-y += mini_upnp
obj-y += wsc
obj-y += auth
obj-y += ft_daemon
obj-y += dot11k
endif

ifneq ($(RTCONFIG_4M_SFP),y)
ifeq ($(RTL819X),y)
## do nothing
else
obj-y += rstats
endif
endif
ifeq ($(RTCONFIG_DSL),y)
obj-y += spectrum
endif
ifeq ($(RTCONFIG_RALINK),y)
obj-y += $(if $(RTCONFIG_MTK_8021X3000),802.1x-3.0.0.0,802.1x)
obj-y += libupnp-1.3.1
obj-y += wsc_upnp
endif

# !!TB - updated Broadcom Wireless driver
ifeq ($(RTCONFIG_RALINK),y)
else ifeq ($(RTCONFIG_QCA),y)
else ifeq ($(RTCONFIG_REALTEK),y)
obj-y += wireless_tools
else ifeq ($(RTCONFIG_ALPINE),y)
obj-y += taskset
else ifeq ($(RTCONFIG_LANTIQ),y)
obj-$(RTCONFIG_BT_CONN) += btconfig
# obj-y += ppacmd
obj-y += libhelper-1.4.0.2
obj-y += fapi_wlan_common-1.0.0.1
#obj-y += libnl-bf
#obj-y += openssl
#obj-y += hostapd-2.6
obj-y += libfapi-0.1
obj-y += taskset
else
obj-y += utils$(BCMEX)$(EX7)
obj-$(RTCONFIG_EMF) += emf$(BCMEX)$(EX7)
obj-$(RTCONFIG_EMF) += igs$(BCMEX)$(EX7)
obj-y += wlconf$(BCMEX)$(EX7)
obj-y += nas$(BCMEX)$(EX7)
obj-$(RTCONFIG_HSPOT) += hspot_ap$(BCMEX)$(EX7)
obj-y += eapd$(BCMEX)$(EX7)
obj-$(RTCONFIG_WPS) += wps$(BCMEX)$(EX7)
obj-$(RTCONFIG_WLEXE) += wlexe
obj-$(CONFIG_LIBBCM) += libbcm
obj-$(CONFIG_LIBUPNP) += libupnp$(BCMEX)$(EX7)
ifeq ($(RTCONFIG_BCMWL6),y)
obj-y += acsd$(BCMEX)$(EX7)
endif
obj-$(RTCONFIG_BCMARM) += wl$(BCMEX)$(EX7)
obj-$(RTCONFIG_BCMARM) += et_arm$(EX7)
endif

ifeq ($(RTCONFIG_AMAS),y)
obj-y  += jansson-2.7
obj-y  += lldpd-0.9.8
endif

.PHONY: emf$(BCMEX)$(EX7) igs$(BCMEX)$(EX7) hspot_ap$(BCMEX)$(EX7) wps$(BCMEX)$(EX7)

obj-$(RTCONFIG_BCMEVENTD) += eventd
obj-$(RTCONFIG_BCMASPMD) += aspmd
obj-$(RTCONFIG_WBD) += json-c wbd
obj-$(RTCONFIG_WLCLMLOAD) += clmb

ifneq ($(RTCONFIG_REALTEK),y)
obj-y += libbcmcrypto
endif
#obj-y += cyassl
obj-$(RTCONFIG_HTTPS) += mssl
obj-$(RTCONFIG_TFTP) += tftp
obj-$(RTCONFIG_TFTP_SERVER) += tftp-hpa

define CMAKE_CrossOptions
	( \
		echo "SET(CMAKE_CROSSCOMPILING \"TRUE\")" >>$(1); \
		echo "SET(TOP $(TOP))" >>$(1); \
		echo "SET(CMAKE_SYSTEM_NAME Linux)" >>$(1); \
		echo "SET(CMAKE_SYSTEM_VERSION $(LINUX_KERNEL))" >>$(1); \
		echo "SET(CMAKE_SYSTEM $(PLATFORM))" >>$(1); \
		echo "SET(CMAKE_SYSTEM_PROCESSOR $(ARCH))" >>$(1); \
		echo "SET(CMAKE_C_COMPILER $(CROSS_COMPILE)gcc)" >>$(1); \
		echo "SET(CMAKE_CXX_COMPILER $(CROSS_COMPILE)g++)" >>$(1); \
		echo "SET(CMAKE_AR $(CROSS_COMPILE)ar)" >>$(1); \
		echo "SET(CMAKE_LINKER $(CROSS_COMPILE)ld)" >>$(1); \
		echo "SET(CMAKE_NM $(CROSS_COMPILE)nm)" >>$(1); \
		echo "SET(CMAKE_OBJCOPY $(CROSS_COMPILE)objcopy)" >>$(1); \
		echo "SET(CMAKE_OBJDUMP $(CROSS_COMPILE)objdump)" >>$(1); \
		echo "SET(CMAKE_RANLIB $(CROSS_COMPILE)ranlib)" >>$(1); \
		echo "SET(CMAKE_STRIP $(CROSS_COMPILE)strip)" >>$(1); \
		echo "SET(CMAKE_FIND_ROOT_PATH $(TOOLS))" >>$(1); \
		echo "SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)" >>$(1); \
		echo "SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)" >>$(1); \
		echo "SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)" >>$(1); \
	)
endef

define CMAKE_CrossOptions
	( \
		echo "SET(CMAKE_CROSSCOMPILING \"TRUE\")" >>$(1); \
		echo "SET(TOP $(TOP))" >>$(1); \
		echo "SET(CMAKE_SYSTEM_NAME Linux)" >>$(1); \
		echo "SET(CMAKE_SYSTEM_VERSION $(LINUX_KERNEL))" >>$(1); \
		echo "SET(CMAKE_SYSTEM $(PLATFORM))" >>$(1); \
		echo "SET(CMAKE_SYSTEM_PROCESSOR $(ARCH))" >>$(1); \
		echo "SET(CMAKE_C_COMPILER $(CROSS_COMPILE)gcc)" >>$(1); \
		echo "SET(CMAKE_CXX_COMPILER $(CROSS_COMPILE)g++)" >>$(1); \
		echo "SET(CMAKE_AR $(CROSS_COMPILE)ar)" >>$(1); \
		echo "SET(CMAKE_LINKER $(CROSS_COMPILE)ld)" >>$(1); \
		echo "SET(CMAKE_NM $(CROSS_COMPILE)nm)" >>$(1); \
		echo "SET(CMAKE_OBJCOPY $(CROSS_COMPILE)objcopy)" >>$(1); \
		echo "SET(CMAKE_OBJDUMP $(CROSS_COMPILE)objdump)" >>$(1); \
		echo "SET(CMAKE_RANLIB $(CROSS_COMPILE)ranlib)" >>$(1); \
		echo "SET(CMAKE_STRIP $(CROSS_COMPILE)strip)" >>$(1); \
		echo "SET(CMAKE_FIND_ROOT_PATH $(TOOLS))" >>$(1); \
		echo "SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)" >>$(1); \
		echo "SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)" >>$(1); \
		echo "SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)" >>$(1); \
	)
endef

# !!TB
ifeq ($(RTCONFIG_USB),y)
#obj-y += p910nd
ifneq ($(HND_ROUTER),y)
obj-y += scsi-idle
endif
obj-y += libusb10
obj-y += libusb
obj-y += libusb-0.1.12
ifeq ($(RTCONFIG_USB),y)
ifneq ($(RTCONFIG_8M_SFP),y)
obj-y += hub-ctrl
endif
endif
obj-$(RTCONFIG_USB_PRINTER) += u2ec
obj-$(RTCONFIG_USB_PRINTER) += LPRng

ifeq ($(RTCONFIG_INTERNAL_GOBI),y)
obj-$(RTCONFIG_INTERNAL_GOBI) += usb-gobi
endif

ifeq ($(RTCONFIG_USB_MULTIMODEM),y)
obj-$(RTCONFIG_USB_MODEM) += sdparm-1.02
obj-$(RTCONFIG_USB_MODEM) += comgt-0.32
obj-$(RTCONFIG_USB_MODEM) += json-c libubox uqmi
obj-$(RTCONFIG_USB_MODEM) += usbmode
else
ifneq ($(RTCONFIG_INTERNAL_GOBI),y)
obj-$(RTCONFIG_USB_MODEM) += sdparm-1.02
obj-$(RTCONFIG_USB_MODEM) += comgt-0.32
obj-$(RTCONFIG_USB_MODEM) += json-c libubox uqmi
obj-$(RTCONFIG_USB_MODEM) += usbmode
endif
endif

ifeq ($(KPATH_3X),y)
ifeq ($(or $(RTCONFIG_SOC_IPQ8064),$(RTCONFIG_ALPINE),$(RTCONFIG_LANTIQ)),y)
obj-$(RTCONFIG_USB_MODEM) += usb-modeswitch
else
obj-$(RTCONFIG_USB_MODEM) += usb-modeswitch-1.2.3
endif
else ifeq ($(KPATH_2636_OR_3X),y)
obj-$(RTCONFIG_USB_MODEM) += $(if $(or $(RTN56UB1),$(RTN56UB2)),usb-modeswitch-1.2.3,usb-modeswitch)
else
ifeq ($(RTCONFIG_BCMWL6),y)
obj-$(RTCONFIG_USB_MODEM) += usb-modeswitch
else
obj-$(RTCONFIG_USB_MODEM) += usb-modeswitch-1.2.3
endif
endif

# Remove all possible usbmodeswitch from obj-y for 4G-AC55U.
# We may need to remove usbmodeswitch from obj-y for all LTE router.
ifeq ($(RTCONFIG_INTERNAL_GOBI),y)
obj-y := $(filter-out usbmodeswitch% usb-modeswitch%, $(obj-y))
endif

obj-$(RTCONFIG_USB_MULTIMODEM) += usb-modeswitch

obj-$(RTCONFIG_USB_BECEEM) += madwimax-0.1.1 # for Samsung WiMAX
obj-$(RTCONFIG_USB_BECEEM) += openssl-1.0.0q # for Beceem, GCT WiMAX
obj-$(RTCONFIG_USB_BECEEM) += Beceem_BCMS250$(BCMEX) # for Beceem WiMAX
obj-$(RTCONFIG_USB_BECEEM) += wpa_supplicant-0.7.3 # for GCT WiMAX
obj-$(RTCONFIG_USB_BECEEM) += zlib # for GCT WiMAX
obj-$(RTCONFIG_USB_BECEEM) += gctwimax-0.0.3rc4 # for GCT WiMAX

obj-$(RTCONFIG_USB_SMS_MODEM) += libiconv-1.14
obj-$(RTCONFIG_USB_SMS_MODEM) += smspdu

# temporary
RTCONFIG_EXT4FS := n
obj-$(RTCONFIG_E2FSPROGS) += e2fsprogs # including libuuid
obj-$(RTCONFIG_EXT4FS) += e2fsprogs

obj-y += libdisk

#obj-y += dosfstools
#obj-$(RTCONFIG_FTP) += $(if $(RTCONFIG_QCA),vsftpd-3.x,vsftpd)
obj-$(RTCONFIG_FTP) += vsftpd-3.x

ifeq ($(CONFIG_LINUX26),y)
ifeq ($(RTCONFIG_SAMBASRV),y)
NEED_EX_NLS = y
endif
ifeq ($(RTCONFIG_USB_EXTRAS),y)
NEED_EX_USB = y
endif
endif

ifeq ($(RTCONFIG_SAMBASRV),y)
ifeq ($(or $(RTCONFIG_SAMBA3),$(RTCONFIG_SAMBA36X)),y)
NEED_SAMBA3 = y
else
NEED_SAMBA2 = y
endif
endif
endif

ifeq ($(RTCONFIG_IPV6),y)
export RTCONFIG_IPV6 := y
else
RTCONFIG_IPV6 :=
endif
# install before samba3 for not overwriting some binaries
obj-$(RTCONFIG_WEBDAV) += samba-3.5.8
obj-$(NEED_SAMBA2) += samba
ifeq ($(or $(CONFIG_BCMWL5),$(RTCONFIG_RALINK),$(RTCONFIG_REALTEK),$(RTCONFIG_LANTIQ)),y)
ifneq ($(RTCONFIG_8M_SFP),y)
#ifeq ($(HND_ROUTER),y)
#obj-$(NEED_SAMBA3) += brcmssp_util
#endif
obj-$(NEED_SAMBA3) += $(if $(RTCONFIG_SAMBA36X),samba-3.6.x,samba-3.0.33)
else
obj-$(NEED_SAMBA3) += $(if $(RTCONFIG_SAMBA36X),samba-3.6.x,samba3)
endif
else
obj-$(NEED_SAMBA3) += $(if $(RTCONFIG_SAMBA36X),samba-3.6.x,samba3)
endif

ifeq ($(RTCONFIG_NTFS),y)
obj-$(RTCONFIG_OPEN_NTFS3G) += ntfs-3g
obj-$(RTCONFIG_PARAGON_NTFS) += ufsd
obj-$(RTCONFIG_TUXERA_NTFS) += tuxera
endif
ifeq ($(RTCONFIG_HFS),y)
obj-$(RTCONFIG_PARAGON_HFS) += ufsd
obj-$(RTCONFIG_TUXERA_HFS) += tuxera
obj-$(RTCONFIG_KERNEL_HFSPLUS) += diskdev_cmds-332.14
endif
obj-$(RTCONFIG_TFAT) += tuxera

obj-$(RTCONFIG_EBTABLES) += ebtables
obj-$(RTCONFIG_IPV6) += odhcp6c
obj-$(RTCONFIG_6RELAYD) += 6relayd
#obj-$(RTCONFIG_IPV6) += ecmh

obj-$(RTCONFIG_MEDIA_SERVER) += zlib
obj-$(RTCONFIG_MEDIA_SERVER) += sqlite
obj-$(RTCONFIG_MEDIA_SERVER) += ffmpeg
obj-$(RTCONFIG_MEDIA_SERVER) += libogg
obj-$(RTCONFIG_MEDIA_SERVER) += flac
obj-$(RTCONFIG_MEDIA_SERVER) += jpeg
obj-$(RTCONFIG_MEDIA_SERVER) += libexif
obj-$(RTCONFIG_MEDIA_SERVER) += libid3tag
obj-$(RTCONFIG_MEDIA_SERVER) += libvorbis
obj-$(RTCONFIG_MEDIA_SERVER) += minidlna
obj-$(RTCONFIG_MEDIA_SERVER) += libgdbm
ifneq ($(RTCONFIG_NO_DAAPD),y)
obj-$(RTCONFIG_MEDIA_SERVER) += mt-daapd
#obj-$(RTCONFIG_MEDIA_SERVER) += mt-daapd-svn-1696
endif

#MEDIA_SERVER_STATIC=y

#add by gauss for cloudsync
obj-$(RTCONFIG_CLOUDSYNC) += openssl
obj-$(RTCONFIG_CLOUDSYNC) += libxml2
obj-$(RTCONFIG_CLOUDSYNC) += curl-7.21.7
obj-$(RTCONFIG_CLOUDSYNC) += smartsync_api
obj-$(RTCONFIG_CLOUDSYNC) += asuswebstorage
obj-$(RTCONFIG_CLOUDSYNC) += inotify

obj-$(RTCONFIG_CLOUDSYNC) += zlib # for neon
obj-$(RTCONFIG_CLOUDSYNC) += neon
obj-$(RTCONFIG_CLOUDSYNC) += webdav_client
obj-$(RTCONFIG_DROPBOXCLIENT) += dropbox_client
obj-$(RTCONFIG_GOOGLECLIENT) += google_client

obj-$(RTCONFIG_FTPCLIENT) += libiconv-1.14
obj-$(RTCONFIG_FTPCLIENT) += ftpclient
obj-$(RTCONFIG_SAMBACLIENT) += sambaclient
obj-$(RTCONFIG_USBCLIENT) += usbclient

#aaews,asusnatnl related
obj-$(RTCONFIG_TUNNEL) += libxml2
obj-$(RTCONFIG_TUNNEL) += curl-7.21.7
obj-$(RTCONFIG_TUNNEL) += wb
obj-$(RTCONFIG_TUNNEL) += aaews
obj-$(RTCONFIG_TUNNEL) += mastiff
obj-$(RTCONFIG_TUNNEL) += asusnatnl

ifeq ($(RTCONFIG_AMAS),y)
#amas-utils
obj-y += amas-utils
endif
#For timemachine
obj-$(RTCONFIG_TIMEMACHINE) += openssl
obj-$(RTCONFIG_TIMEMACHINE) += libgpg-error-1.10
obj-$(RTCONFIG_TIMEMACHINE) += libgcrypt-1.5.1
obj-$(RTCONFIG_TIMEMACHINE) += db-4.8.30
obj-$(RTCONFIG_TIMEMACHINE) += netatalk-3.0.5
ifeq ($(or $(RTCONFIG_TIMEMACHINE),$(RTCONFIG_MDNS)),y)
obj-y += libdaemon
obj-y += expat-2.0.1
obj-y += avahi-0.6.31
else
ifeq ($(or $(RTCONFIG_MEDIA_SERVER),$(RTCONFIG_BWDPI)),y)
obj-y += mDNSResponder
endif
endif

#add for snmpd
SNMPD_CFLAGS += -D$(BUILD_NAME)
ifeq ($(BUILD_NAME), $(filter $(BUILD_NAME), RT-N66U RT-AC66U RT-AC68U))
SNMPD_CFLAGS += -DDUAL_BAND
endif
obj-$(RTCONFIG_SNMPD) += openssl
#obj-$(RTCONFIG_SNMPD) += libnmp
obj-$(RTCONFIG_SNMPD) += net-snmp-5.7.2
ifeq ($(RTCONFIG_SNMPD),y)
export MIB_MODULE_PASSPOINT =
endif

#for chillispot
obj-$(RTCONFIG_COOVACHILLI) += coovachilli
obj-$(RTCONFIG_FREERADIUS) += sqlite pcre-8.31
obj-$(RTCONFIG_FREERADIUS) += talloc-2.1.1
obj-$(RTCONFIG_FREERADIUS) += freeradius-server-3.0.0
obj-$(RTCONFIG_CAPTIVE_PORTAL) += sqlite libxml2 pcre-8.31 lighttpd-1.4.39 libexif curl-7.21.7
obj-$(RTCONFIG_CAPTIVE_PORTAL) += samba-3.5.8

obj-$(RTCONFIG_FBWIFI) += fb_wifi
obj-$(RTCONFIG_FBWIFI) += httpd_uam

#
# configurable packages
#
RTCONFIG_PPP_BASE = y
obj-$(RTCONFIG_PPP_BASE) += pppd rp-pppoe
obj-$(RTCONFIG_L2TP) += rp-l2tp
obj-$(RTCONFIG_PPTP) += accel-pptp
obj-$(RTCONFIG_EAPOL) += wpa_supplicant
obj-$(RTCONFIG_HTTPS) += openssl
obj-$(RTCONFIG_HTTPS) += wget
obj-$(RTCONFIG_HTTPS) += zlib
obj-$(RTCONFIG_SSH) += dropbear
obj-$(RTCONFIG_WTFAST) += wtfast
#obj-$(RTCONFIG_ZEBRA) += zebra
obj-$(RTCONFIG_QUAGGA) += quagga
#obj-$(RTCONFIG_IPP2P) += ipp2p
obj-$(RTCONFIG_LZO) += lzo
obj-$(RTCONFIG_OPENVPN) += openpam
obj-$(RTCONFIG_OPENVPN) += openvpn

obj-$(CONFIG_LINUX26) += hotplug2
obj-$(CONFIG_LINUX26) += udev

obj-$(RTCONFIG_ACCEL_PPTPD) += accel-pptpd
obj-$(RTCONFIG_PPTPD) += pptpd

obj-$(RTCONFIG_WEBDAV) += sqlite
obj-$(RTCONFIG_WEBDAV) += libxml2
obj-$(RTCONFIG_WEBDAV) += pcre-8.31
obj-$(RTCONFIG_WEBDAV) += lighttpd-1.4.39
obj-$(RTCONFIG_WEBDAV) += libexif

obj-$(RTCONFIG_IXIAEP) += ixia_endpoint$(BCMEX)

# Add for ASUS Components
obj-$(RTCONFIG_UPNPC) += miniupnpc
obj-y += miniupnpd$(MUVER)
obj-y += igmpproxy
obj-y += snooper
ifeq ($(RTCONFIG_BCMWL6),y)
ifneq ($(RTCONFIG_BCMARM),y)
obj-y += netconf
obj-y += igmp
endif
endif
obj-y += udpxy
ifeq ($(CONFIG_BCMWL5),y)
ifeq ($(RTCONFIG_BCMARM),y)
obj-y += lltd.arm
obj-y += taskset
obj-$(RTCONFIG_COMA) += comad
else
obj-y += lltd
endif
else
obj-y += lldt
endif
obj-$(RTCONFIG_JFFS2USERICON) += lltdc

obj-$(RTCONFIG_TOAD) += toad
ifeq ($(RTCONFIG_BCMARM),y)
ifneq ($(RTCONFIG_QTN),y)
obj-$(RTCONFIG_BCMBSD) += bsd
endif
endif
obj-$(RTCONFIG_BCM_APPEVENTD) += appeventd
obj-$(RTCONFIG_BCMSSD) += ssd
ifeq ($(RTCONFIG_WLCEVENTD),y)
ifeq ($(CONFIG_BCMWL5),y)
obj-$(RTCONFIG_WLCEVENTD) += wlceventd
endif
ifeq ($(RTCONFIG_RALINK),y)
obj-$(RTCONFIG_WLCEVENTD) += iwevent
endif
endif
ifeq ($(RTCONFIG_HAPDEVENT),y)
obj-$(RTCONFIG_HAPDEVENT) += hapdevent
endif
obj-y += networkmap
ifneq ($(BUILD_NAME), $(filter $(BUILD_NAME), AC2900))
obj-y += infosvr
endif
obj-y += ez-ipupdate
ifneq ($(or $(RTN56U),$(DSLN55U)),y)
obj-y += phddns
endif

obj-$(RTCONFIG_STRACE) += strace

obj-$(RTCONFIG_LLDP) += openlldp

obj-n += lsof
obj-$(RTCONFIG_IPERF) += iperf

obj-$(RTCONFIG_PUSH_EMAIL) += openssl
obj-$(RTCONFIG_PUSH_EMAIL) += email-3.1.3
ifeq ($(RTCONFIG_PUSH_EMAIL),y)
obj-$(RTCONFIG_DBLOG) += dblog
endif

obj-$(RTCONFIG_NORTON) += norton${BCMEX}

obj-$(UBI) += mtd-utils
obj-$(UBIFS) += mtd-utils
obj-$(UBIFS) += e2fsprogs lzo zlib
ifeq ($(RTCONFIG_RALINK),y)
obj-$(RA_SKU) += ra_SingleSKU
obj-$(RA_SKU_IN_DRV) += ra_SingleSKU
endif

ifeq ($(HND_ROUTER),y)
obj-y += hnd_extra
endif
obj-$(RTCONFIG_EXT_RTL8365MB) += rtl_bin
obj-$(RTCONFIG_EXT_RTL8370MB) += rtl_bin
obj-$(RTCONFIG_LACP) += lacp

#obj-y += bonnie
#obj-y += stress-1.x

ifeq ($(RTN65U), y)
obj-y += asm1042
endif

obj-$(RTCONFIG_QUICKSEC) += quicksec-7.0
obj-$(RTCONFIG_STRONGSWAN) += strongswan-5.2.1

ifneq ($(RTCONFIG_4M_SFP),y)
obj-y += netstat-nat
endif

obj-$(RTCONFIG_TCPDUMP) += libpcap
obj-$(RTCONFIG_TCPDUMP) += tcpdump-4.4.0
obj-$(RTCONFIG_TRACEROUTE) += traceroute-2.1.0

obj-$(RTCONFIG_BLINK_LED) += bled
obj-$(RTCONFIG_BONDING) += ifenslave

obj-$(RTCONFIG_GEOIP) += GeoIP-1.6.2

obj-$(RTCONFIG_TRANSMISSION) += Transmission curl-7.21.7 libevent-2.0.21

# Begin merlin add-ons
obj-$(RTCONFIG_NANO) += ncurses-6.0
obj-$(RTCONFIG_NANO) += nano
obj-$(RTCONFIG_OPENVPN) += ministun
obj-$(RTCONFIG_DNSSEC) += nettle
obj-$(RTCONFIG_SAMBA36X) += libiconv-1.14
obj-$(RTCONFIG_TELENET) += lanauth

ifneq ($(HND_ROUTER),y)
obj-y += cstats
endif

ifeq ($(RTCONFIG_BCMARM),y)
obj-y += libnfnetlink
obj-y += libmnl
obj-y += ipset_arm
else
obj-y += ipset
endif
obj-$(RTCONFIG_NFS) += nfs-utils-1.3.4
obj-$(RTCONFIG_NFS) += portmap
# End merlin add-ons

ifeq ($(RTCONFIG_TOR),y)
obj-y += openssl
obj-y += zlib
obj-y += libevent-2.0.21
obj-y += tor
endif

#For TR-069 agent
obj-$(RTCONFIG_TR069) += openssl
obj-$(RTCONFIG_TR069) += tr069

obj-$(RTCONFIG_CLOUDCHECK) += cloudcheck

obj-$(RTCONFIG_GETREALIP) += ministun

CFGSYNC_PKG := openssl libevent-2.0.21 cfg_mnt
obj-$(RTCONFIG_CFGSYNC) += $(CFGSYNC_PKG)
ifeq ($(RTCONFIG_CFGSYNC),y)
ifeq ($(RTCONFIG_QCA),y)
obj-y += qca-wifi-assoc-eventd
endif
endif

# move from HND_ROUTER userspace
obj-$(HND_ROUTER) += wlcsm
obj-$(HND_ROUTER) += wlan
obj-$(HND_ROUTER) += websockets
obj-$(HND_ROUTER) += bcm_util
#obj-$(HND_ROUTER) += cms_msg
#obj-$(HND_ROUTER) += cms_util
#obj-$(HND_ROUTER) += rastatus6
obj-$(HND_ROUTER) += bcm_flashutil
obj-$(HND_ROUTER) += bcm_flasher
#obj-$(HND_ROUTER) += cms_boardctl
obj-$(HND_ROUTER) += bcm_boardctl
obj-$(HND_ROUTER) += tmctl
obj-$(HND_ROUTER) += tmctl_lib
obj-$(HND_ROUTER) += ethswctl
obj-$(HND_ROUTER) += ethswctl_lib
obj-$(HND_ROUTER) += bcmtm
obj-$(HND_ROUTER) += bcmtm_lib
obj-$(HND_ROUTER) += rdpactl
obj-$(HND_ROUTER) += vlanctl
obj-$(HND_ROUTER) += vlanctl_lib
obj-$(HND_ROUTER) += seltctl
obj-$(HND_ROUTER) += bridgeutil
obj-$(HND_ROUTER) += pwrctl
obj-$(HND_ROUTER) += pwrctl_lib
obj-$(HND_ROUTER) += bridge-utils	# ig
obj-$(HND_ROUTER) += stlport
#obj-$(HND_ROUTER) += iperf-2.0.5
obj-$(HND_ROUTER) += mmc-utils
obj-$(HND_ROUTER) += mtd
#obj-$(HND_ROUTER) += sysstat
obj-$(HND_ROUTER) += urlfilterd
obj-$(HND_ROUTER) += bpmctl
obj-$(HND_ROUTER) += dnsspoof
obj-$(HND_ROUTER) += ethctl
obj-$(HND_ROUTER) += ethctl_lib
obj-$(HND_ROUTER) += fcctl
obj-$(HND_ROUTER) += fcctl_lib
obj-$(HND_ROUTER) += scripts
obj-$(HND_ROUTER) += stress
obj-$(HND_ROUTER) += vpmstats
obj-$(HND_ROUTER) += bcm_boot_launcher
obj-$(HND_ROUTER) += bdmf_shell
obj-$(HND_ROUTER) += dhrystone
#obj-$(HND_ROUTER) += ipsec-tools
obj-$(HND_ROUTER) += psictl
obj-$(HND_ROUTER) += scratchpadctl
obj-$(HND_ROUTER) += drvlogs_scripts
obj-$(HND_ROUTER) += wdtctl
#obj-$(HND_ROUTER) += expat
obj-$(HND_ROUTER) += httpdshared
obj-$(HND_ROUTER) += swmdk
obj-$(HND_ROUTER) += mdk
obj-$(HND_ROUTER) += ivitcl
obj-$(HND_ROUTER) += bcmmcast
obj-$(HND_ROUTER) += bcmmcastctl
obj-$(HND_ROUTER) += mcpctl
obj-$(HND_ROUTER) += mcpd
#obj-$(HND_ROUTER) += bcm_sslconf
#obj-$(HND_ROUTER) += jqplot
#obj-$(HND_ROUTER) += jquery
#obj-$(HND_ROUTER) += tablesorter

# ALPINE compile package
obj-$(RTCONFIG_ALPINE) += qsr10g_image
obj-$(RTCONFIG_ALPINE) += aqr107_image

ifeq ($(RTCONFIG_BCMARM),y)
obj-prelibs =$(filter busybox% nvram$(BCMEX)$(EX7) libbcmcrypto shared netconf libupnp$(BCMEX)$(EX7) libz libbcm pciefd, $(obj-y))
obj-postlibs := $(filter-out $(obj-prelibs), $(obj-y))
endif

ifneq ($(find lighttpd-1.4.39, $(obj-y)),"")
released_dir += APP-IPK
endif

ifeq ($(RTCONFIG_RALINK_MT7621),y)
ifeq ($(RTCONFIG_APP_NETINSTALLED),y)
obj-y += taskset
endif
endif
obj-$(RTCONFIG_HD_SPINDOWN) += sd-idle


obj-clean := $(foreach obj, $(obj-y) $(obj-n) $(obj-), $(obj)-clean)
obj-install := $(foreach obj,$(obj-y),$(obj)-install)

MKSQUASHFS_TARGET = mksquashfs
MKSQUASHFS = $(MKSQUASHFS_TARGET)

LINUX_ARCH_ASM_INCL_DIR = $(if ($KPATH_3X),$(LINUXDIR)/arch/mips/include/asm,$(LINUXDIR)/include/asm-mips)

#
# Basic rules
#

ifeq ($(HND_ROUTER),y)
all: clean-build kernel_header version fsbuild $(obj-y)
else ifeq ($(RTCONFIG_ARM),y)
all: clean-build kernel_header libc version obj_prelibs kernel $(obj-postlibs)
else
all: clean-build kernel_header libc version obj_prelibs kernel $(obj-y)
endif

ifeq ($(RTCONFIG_BCMARM),y)
version: $(SRCBASE)/include/epivers.h

$(SRCBASE)/include/epivers.h:
	$(MAKE) -C $(SRCBASE)/include
ifeq ($(RTCONFIG_DHDAP),y)
	$(MAKE) -C $(SRCBASE_DHD)/include
	$(MAKE) -C $(SRCBASE_FW)/include
endif
endif

ifeq ($(RTCONFIG_BCMARM),y)
	+$(MAKE) $(MAKE_ARGS) post_preplibs
endif

ifeq ($(RTCONFIG_BCMARM),y)
list_obj_prelibs:
	@echo obj-prelibs:
	@( \
	for i in ${obj-prelibs}; do \
		echo $${i}; \
	done; \
	);

list_obj_postlibs:
	@echo obj-postlibs:
	@( \
	for i in ${obj-postlibs}; do \
		echo $${i}; \
	done; \
	);

obj_prelibs: list_obj_prelibs
	+$(MAKE) parallel=true $(MAKE_ARGS) $(obj-prelibs)

obj_postlibs: list_obj_postlibs
	+$(MAKE) parallel=true $(MAKE_ARGS) $(obj-postlibs)

post_preplibs: obj_postlibs
else
obj_prelibs: dummy
endif

ifneq ($(RTCONFIG_BCMARM),y)
kernel_header: $(LINUXDIR)/.config
	$(MAKE) -C $(LINUXDIR) ARCH=$(ARCH) headers_install INSTALL_HDR_PATH=$(TOP)/$@
ifeq ($(RTCONFIG_RALINK),y)
	@if [ -e $(LINUX_ARCH_ASM_INCL_DIR)/rt2880/rt_mmap.h ] ; then \
		mkdir -p $@/include/asm/rt2880 ; \
		cp $(LINUX_ARCH_ASM_INCL_DIR)/rt2880/rt_mmap.h $@/include/asm/rt2880/ ; \
	fi
endif
ifeq ($(RTCONFIG_QCA),y)
ifeq ($(CONFIG_LINUX3_14),y)
	cp -fr $(LINUXDIR)/include/uapi/* $@/include/uapi/
else
	cp $(LINUXDIR)/include/net/switch.h $@/include/linux/
endif
endif
ifeq ($(RTCONFIG_REALTEK),y)
	@echo "-------- cp kernel_header --------" >> $(SRCBASE)/build.log
	cp -fr $(SRCBASE)/linux/realtek/files/kernel_header $(SRCBASE)/router/
endif
	$(MAKE) -C $(LINUXDIR) prepare scripts
	find $(LINUXDIR)/include -name autoconf.h -exec cp {} $@/include/linux/ \;
	cp $(LINUXDIR)/Makefile $@/
else
kernel_header: $(LINUXDIR)/.config

endif

kernel: $(LINUXDIR)/.config
	$(SEP)

ifneq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_RALINK),y)
	@if ! grep -q "CONFIG_EMBEDDED_RAMDISK=y" $(LINUXDIR)/.config ; then \
		$(MAKE) -C $(LINUXDIR) vmlinux CC=$(KERNELCC) LD=$(KERNELLD); \
	fi
	if grep -q "CONFIG_MODULES=y" $(LINUXDIR)/.config ; then \
		$(MAKE) -C $(LINUXDIR) modules CC=$(KERNELCC) LD=$(KERNELLD); \
	fi
else ifeq ($(RTCONFIG_QCA),y)
	$(MAKE) $(PARALLEL_BUILD) -C $(LINUXDIR) vmlinux CC=$(KERNELCC) LD=$(KERNELLD)
	if grep -q "CONFIG_MODULES=y" $(LINUXDIR)/.config ; then \
	    $(MAKE) $(PARALLEL_BUILD) -C $(LINUXDIR) modules CC=$(KERNELCC) LD=$(KERNELLD); \
	fi
ifeq ($(CONFIG_LINUX3_14),y)
	$(MAKE) -C $(LINUXDIR) dtbs
endif
else ifeq ($(RTCONFIG_REALTEK),y)
	@$(MAKE) -C $(LINUXDIR)/rtkload rtk-clean
	@$(MAKE) -C $(LINUXDIR)/.. linux
else ifeq ($(RTCONFIG_ALPINE),y)
	# uImage (Host Linux)
	make -j 9 -C ${LINUXDIR} HOSTCFLAGS="-O2 -I${STAGING_DIR}/host/include -Wall -Wmissing-prototypes -Wstrict-prototypes" KBUILD_HAVE_NLS=no CONFIG_SHELL="/bin/bash" V='' uImage
	make -j 9 -C ${LINUXDIR} HOSTCFLAGS="-O2 -I${STAGING_DIR}/host/include -Wall -Wmissing-prototypes -Wstrict-prototypes" KBUILD_HAVE_NLS=no CONFIG_SHELL="/bin/bash" V='' modules
else ifeq ($(RTCONFIG_LANTIQ),y)
	# uImage (Host Linux)
	make -j3 -C ${LINUXDIR} HOSTCFLAGS="-O2 -I${SRCBASE}/tools/host/include -I${SRCBASE}/tools/host/usr/include -Wall -Wmissing-prototypes -Wmissing-prototypes -Wstrict-prototypes" KBUILD_HAVE_NLS=no CONFIG_DEBUG_SECTION_MISMATCH=y CONFIG_SHELL="/bin/bash" V='' INSTALL_HDR_PATH=${LINUXDIR}/user_headers headers_install
	make -j3 -C ${LINUXDIR} HOSTCFLAGS="-O2 -I${SRCBASE}/tools/host/include -I${SRCBASE}/tools/host/usr/include -Wall -Wmissing-prototypes -Wmissing-prototypes -Wstrict-prototypes" KBUILD_HAVE_NLS=no CONFIG_DEBUG_SECTION_MISMATCH=y CONFIG_SHELL="/bin/bash" V='' INSTALL_HDR_PATH=${LINUXDIR} modules
	make -j3 -C ${LINUXDIR} HOSTCFLAGS="-O2 -I${SRCBASE}/tools/host/include -I${SRCBASE}/tools/host/usr/include -Wall -Wmissing-prototypes -Wmissing-prototypes -Wstrict-prototypes" KBUILD_HAVE_NLS=no CONFIG_DEBUG_SECTION_MISMATCH=y CONFIG_SHELL="/bin/bash" V='' INSTALL_HDR_PATH=${LINUXDIR} all modules
else
	# Broadcrom MIPS SoC
	@if ! grep -q "CONFIG_EMBEDDED_RAMDISK=y" $(LINUXDIR)/.config ; then \
		$(MAKE) -C $(LINUXDIR) zImage CC=$(KERNELCC); \
	fi
	if grep -q "CONFIG_MODULES=y" $(LINUXDIR)/.config ; then \
		$(MAKE) -C $(LINUXDIR) modules CC=$(KERNELCC); \
	fi

ifeq ($(CONFIG_LINUX26),y)
	$(MAKE) -C $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed srctree=$(LINUXDIR)
endif
endif
else	# RTCONFIG_BCMARM
	(echo '.NOTPARALLEL:' ; cat ${LINUXDIR}/Makefile) |\
		$(MAKE) -C ${LINUXDIR} -f - $(SUBMAKE_SETTINGS) zImage
	+$(MAKE) CONFIG_SQUASHFS=$(CONFIG_SQUASHFS) -C $(SRCBASE)/router/compressed ARCH=$(ARCH)

	$(if $(shell grep "CONFIG_MODULES=y" ${LINUXDIR}/.config), \
	(echo '.NOTPARALLEL:' ; cat ${LINUXDIR}/Makefile) | $(MAKE) -C ${LINUXDIR} -f - $(SUBMAKE_SETTINGS) MFG_WAR=1 zImage ; \
	(echo '.NOTPARALLEL:' ; cat ${LINUXDIR}/Makefile) | $(MAKE) -C ${LINUXDIR} -f - ARCH=$(ARCH) modules)
	# Preserve the debug versions of these and strip for release
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/vmlinux)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/wl/wl.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/et/et.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/ctf/ctf.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/dhd/dhd.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/dhd/dhd24.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/bcm57xx/bcm57xx.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/emf/emf.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/igs/igs.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/dpsta/dpsta.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/connector/cn.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/scsi/scsi_wait_scan.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/usb/host/xhci-hcd.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/usb/host/ehci-hcd.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/usb/host/ohci-hcd.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/lib/libcrc32c.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/net/sched/sch_tbf.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/net/sched/sch_hfsc.ko)
ifeq ($(RTCONFIG_EXT_RTL8365MB),y)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/char/rtl8365mb/rtl8365mb.ko)
endif
ifeq ($(RTCONFIG_RTL8370MB),y)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/char/rtl8365mb/rtl8365mb.ko)
endif
endif # RTCONFIG_BCMARM

ifeq ($(RTCONFIG_RALINK),y)
else ifeq ($(RTCONFIG_QCA),y)
else ifeq ($(RTCONFIG_REALTEK),y)
	#@$(MAKE) -C $(LINUXDIR)/rtkload rtk-clean
else
ifneq ($(RTCONFIG_BCMARM),y)
lzma-loader:
	@[ ! -d $(SRCBASE)/lzma-loader ] || \
		$(MAKE) -C $(SRCBASE)/lzma-loader CROSS_COMPILE=$(CROSS_COMPILE) LD=$(LD)

lzma-loader-clean:
	@[ ! -d $(SRCBASE)/lzma-loader ] || \
		$(MAKE) -C $(SRCBASE)/lzma-loader clean

lzma-loader-install: lzma-loader
	@$(SEP)
endif
endif

kmod: dummy
	$(MAKE) -C $(LINUXDIR) modules CC=$(KERNELCC)

testfind:
	cd $(TARGETDIR)/lib/modules/* && find -name "*.o" -exec mv -i {} . \; || true
	cd $(TARGETDIR)/lib/modules/* && find -type d -delete || true

install package: $(obj-install) $(LINUXDIR)/.config gen_kernelrelease gen_target gen_gpl_excludes_router

gen_kernelrelease:
ifeq ($(RTCONFIG_REALTEK), y)
	@rm -f $(LINUXDIR)/.kernelrelease
	@$(MAKE) -C $(LINUXDIR) kernelrelease

KERNELRELEASE=$(shell cat $(LINUXDIR)/.kernelrelease 2> /dev/null)
endif


ifeq ($(HND_ROUTER),y)
%-rebuild: %
	$(MAKE) $*
	$(MAKE) $*-install
	$(MAKE) $*-reinstall

%-reinstall: %
	if [ -d $(INSTALLDIR)/$* ] ; then \
		sdir=$(INSTALLDIR)/$*; \
		ex=; \
		if [ -d $(INSTALLDIR)/$*/$* ] ; then \
			ex=y ; \
			mv $(INSTALLDIR)/$* $(INSTALLDIR)/$*$$ex; \
		fi ; \
		(cd $${sdir}$${ex} && tar cpf - . && cd ../ && rm -rf $${sdir}$${ex}) | (cd $(INSTALLDIR) && tar xpf -) \
	fi

reinstall:
	for sdir in $(wildcard $(patsubst %,$(INSTALLDIR)/%,$(obj-y))) ; do \
		if [ -d $$sdir ] ; then \
			mv $$sdir $${sdir}_reinstall; \
			(cd $${sdir}_reinstall && tar cpf - . && cd ../ && rm -rf $${sdir}_reinstall) | (cd $(INSTALLDIR) && tar xpf -) \
		fi; \
	done

	@cd $(TARGETDIR) && $(TOP)/others/rootprep${BCMEX}${EX7}.sh
	@echo ---

	# extra install in case build-process skip them
# why precompiled?
	install -D $(TOOLCHAIN)/lib/libexpat.so $(INSTALLDIR)/lib/public/libexpat.so
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/ld-linux.so.3 $(INSTALLDIR)/lib/ld-linux.so.3
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libresolv.so.2 $(INSTALLDIR)/lib/libresolv.so.2
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libcrypt.so.1 $(INSTALLDIR)/lib/libcrypt.so.1
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libutil.so.1 $(INSTALLDIR)/lib/libutil.so.1
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libm.so.6 $(INSTALLDIR)/lib/libm.so.6
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libnss_dns.so.2 $(INSTALLDIR)/lib/libnss_dns.so.2
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libnsl.so.1 $(INSTALLDIR)/lib/libnsl.so.1
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libnss_files.so.2 $(INSTALLDIR)/lib/libnss_files.so.2
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libc.so.6 $(INSTALLDIR)/lib/libc.so.6
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libpthread.so.0 $(INSTALLDIR)/lib/libpthread.so.0
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libdl.so.2 $(INSTALLDIR)/lib/libdl.so.2
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libgcc_s.so.1 $(INSTALLDIR)/lib/libgcc_s.so.1
	install -D $(TOOLCHAIN_64)/aarch64-buildroot-linux-gnu/sysroot/lib/ld-linux-aarch64.so.1 $(INSTALLDIR)/lib/ld-linux-aarch64.so.1
	install -D $(TOOLCHAIN_64)/aarch64-buildroot-linux-gnu/sysroot/lib/libc.so.6 $(INSTALLDIR)/lib/aarch64/libc.so.6
	install -D $(TOOLCHAIN_64)/aarch64-buildroot-linux-gnu/sysroot/lib/libnss_dns.so.2 $(INSTALLDIR)/lib/aarch64/libnss_dns.so.2
	install -D $(TOOLCHAIN_64)/aarch64-buildroot-linux-gnu/sysroot/lib/libnss_files.so.2 $(INSTALLDIR)/lib/aarch64/libnss_files.so.2

ifeq ($(RTCONFIG_CFEZ),y)
	install -D $(TOP)/envram_bin/envram$(BCMEX)$(EX7)/envrams $(INSTALLDIR)/usr/sbin/envrams
	install -D $(TOP)/envram_bin/envram$(BCMEX)$(EX7)/envram $(INSTALLDIR)/usr/sbin/envram
endif

	$(if $(KNM),NM=$(KNM),) \
	$(BUSYBOX_DIR)/examples/depmod.pl -k $(LINUXDIR)/vmlinux -b $(TARGETDIR)/../modules/lib/modules/$(LINUX_KERNEL)

ifeq ($(HND_ROUTER),y)
ifeq ($(RTCONFIG_TFAT),y)
	echo "kernel/fs/tfat.ko:" >> $(TARGETDIR)/../modules/lib/modules/$(LINUX_KERNEL)/modules.dep
endif
ifeq ($(RTCONFIG_TUXERA_NTFS),y)
	echo "kernel/fs/tntfs.ko:" >> $(TARGETDIR)/../modules/lib/modules/$(LINUX_KERNEL)/modules.dep
endif
ifeq ($(RTCONFIG_TUXERA_HFS),y)
	echo "kernel/fs/thfsplus.ko:" >> $(TARGETDIR)/../modules/lib/modules/$(LINUX_KERNEL)/modules.dep
endif
endif
	cd $(TARGETDIR)/../modules/lib/modules && rm -f */source || true
	cd $(TARGETDIR)/../modules/lib/modules && rm -f */build || true

	# prune misc
	rm -rf $(INSTALLDIR)/eapd${BCMEX}${EX7}
	rm -rf $(INSTALLDIR)/rom/rom/etc

	# strip size for tmp
	#rm -rf $(INSTALLDIR)/rom/*.ipk
	#rm -rf $(INSTALLDIR)/rom/*.tgz
	#rm -rf $(INSTALLDIR)/usr/lighttpd/css
	#rm -rf $(INSTALLDIR)/usr/lighttpd/js
	#rm -rf $(INSTALLDIR)/www/images/New_ui

.PHONY: reinstall

gen_target:
	echo $@
	if [ ! -L $(PLATFORMDIR) ]; then \
		echo "create platform link"; \
		ln -sf $(HND_SRC)/targets/$(PROFILE) $(PLATFORMDIR); \
	else \
		echo "platform link exists"; \
	fi
	@$(SEP)
else
gen_target:
	@$(SEP)

	[ -d $(TARGETDIR) ] || install -d $(TARGETDIR)


# kernel modules
	$(MAKE) -C $(LINUXDIR) modules_install DEPMOD=/bin/true INSTALL_MOD_PATH=$(TARGETDIR)
	@if grep -q "CONFIG_RT3352_INIC_AP=y" $(LINUXDIR)/.config ; then \
		install -d $(TARGETDIR)/iNIC_RT3352/ ; \
		install -D $(LINUXDIR)/drivers/net/wireless/iNIC_RT3352/firmware/mii/iNIC_ap.bin $(TARGETDIR)/iNIC_RT3352/ ; \
	else \
		rm -rf $(TARGETDIR)/iNIC_RT3352/ ; \
	fi
##!!TB	find $(TARGETDIR)/lib/modules -name *.o -exec mipsel-linux-strip --strip-unneeded {} \;
	find $(TARGETDIR)/lib/modules -name *.o -exec $(STRIP) --strip-debug -x -R .comment -R .pdr -R .mdebug.abi32 -R .note.gnu.build-id -R .gnu.attributes -R .reginfo {} \;
	find $(TARGETDIR)/lib/modules -name *.ko -exec $(STRIP) --strip-debug -x -R .comment -R .pdr -R .mdebug.abi32 -R .note.gnu.build-id -R .gnu.attributes -R .reginfo {} \;
#	-cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv diag/* . && rm -rf diag

# nice and clean
ifeq ($(RTCONFIG_RALINK)$(RTCONFIG_QCA)$(RTCONFIG_ALPINE)$(RTCONFIG_LANTIQ),y)
	-cd $(TARGETDIR)/lib/modules/*/kernel/drivers/ && mv nvram$(BCMEX)$(EX7)/* . && rm -rf nvram$(BCMEX)$(EX7) || true
else
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv et.4702/* . && rm -rf et.4702 || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv et/* . && rm -rf et || true
ifeq ($(RTCONFIG_BRCM_USBAP),y)
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv wl/wl_high.ko . || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv wl/wl_high/wl_high.ko . || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv wl/wl_sta/wl_high.ko . || true
endif
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv wl/wl.ko . && rm -rf wl || true
endif
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv cifs/* . && rm -rf cifs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv jffs2/* . && rm -rf jffs2 || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv jffs/* . && rm -rf jffs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/lib && mv zlib_inflate/* . && rm -rf zlib_inflate || true
	cd $(TARGETDIR)/lib/modules/*/kernel/lib && mv zlib_deflate/* . && rm -rf zlib_deflate || true
	cd $(TARGETDIR)/lib/modules/*/kernel/lib && mv lzo/* . && rm -rf lzo || true
	rm -rf $(TARGETDIR)/lib/modules/*/pcmcia

##!!TB
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv ext2/* . && rm -rf ext2 || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv ext3/* . && rm -rf ext3 || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv jbd/* . && rm -rf jbd || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv fat/* . && rm -rf fat || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv vfat/* . && rm -rf vfat || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv msdos/* . && rm -rf msdos || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv fuse/* . && rm -rf fuse || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv ntfs/* . && rm -rf ntfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv smbfs/* . && rm -rf smbfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv reiserfs/* . && rm -rf reiserfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv hfsplus/* . && rm -rf hfsplus || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv lockd/* . && rm -rf lockd || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv nfsd/* . && rm -rf nfsd || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv nfs/* . && rm -rf nfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv xfs/* . && rm -rf xfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv nls/* . && rm -rf nls || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv exportfs/* . && rm -rf exportfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/net && mv sunrpc/* . && rm -rf sunrpc || true
	cd $(TARGETDIR)/lib/modules/*/kernel/net && mv auth_gss/* . && rm -rf auth_gss || true
	cd $(TARGETDIR)/lib/modules/*/kernel/sound/core && mv oss/* . && rm -rf oss || true
	cd $(TARGETDIR)/lib/modules/*/kernel/sound/core && mv seq/* . && rm -rf seq || true
	cd $(TARGETDIR)/lib/modules/*/kernel/sound && mv core/* . && rm -rf core || true
	cd $(TARGETDIR)/lib/modules/*/kernel/sound && mv usb/* . && rm -rf usb || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv hcd/* . && rm -rf hcd || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv host/* . && rm -rf host || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv storage/* . && rm -rf storage || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv serial/* . && rm -rf serial || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv core/* . && rm -rf core || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv class/* . && rm -rf class || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv misc/* . && rm -rf misc || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/hid && mv usbhid/* . && rm -rf usbhid || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/input && mv joystick/* . && rm -rf joystick || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/input && mv keyboard/* . && rm -rf keyboard || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/input && mv misc/* . && rm -rf misc || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/input && mv mouse/* . && rm -rf mouse || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/media/video && mv uvc/* . && rm -rf uvc || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/media && mv video/* . && rm -rf video || true

	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv bcm57xx/* . && rm -rf bcm57xx || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv emf/* . && rm -rf emf || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv igs/* . && rm -rf igs || true
ifeq ($(RTAC53U),y)
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv ctf_5358/* . && rm -rf ctf_5358 || true
else
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv ctf/* . && rm -rf ctf || true
endif
ifeq ($(RTCONFIG_EXT_RTL8365MB),y)
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/char && mv rtl8365mb/* . && rm -rf rtl8365mb || true
endif
ifeq ($(RTCONFIG_RTL8370MB),y)
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/char && mv rtl8365mb/* . && rm -rf rtl8365mb || true
endif
	cd $(TARGETDIR)/lib/modules && rm -f */source || true
	cd $(TARGETDIR)/lib/modules && rm -f */build || true

# misc
	for dir in $(wildcard $(patsubst %,$(INSTALLDIR)/%,$(obj-y))) ; do \
		(cd $${dir} && tar cpf - .) | (cd $(TARGETDIR) && tar xpf -) \
	done

ifneq ($(RTCONFIG_L7),y)
	rm -f $(TARGETDIR)/usr/lib/iptables/libipt_layer7.so
endif
ifneq ($(RTCONFIG_SNMPD),y)
	rm -f $(TARGETDIR)/usr/lib/libnmp.so
endif

# uClibc
ifeq ($(RTL8197F), y) # Realtek RTL8197F platform.
	install $(LIBDIR)/ld-uClibc.so.1 $(TARGETDIR)/lib/
	install $(LIBDIR)/libcrypt.so.1 $(TARGETDIR)/lib/
	install $(LIBDIR)/libpthread.so.1 $(TARGETDIR)/lib/
	install $(LIBDIR)/libgcc_s.so.1 $(TARGETDIR)/lib/
	$(STRIP) $(TARGETDIR)/lib/libgcc_s.so.1
	install $(LIBDIR)/libc.so.1 $(TARGETDIR)/lib/
	install $(LIBDIR)/libdl.so.1 $(TARGETDIR)/lib/
	install $(LIBDIR)/libm.so.1 $(TARGETDIR)/lib/
	install $(LIBDIR)/libnsl.so.1 $(TARGETDIR)/lib/
ifeq ($(RTCONFIG_SSH),y)
	install $(LIBDIR)/libutil.so.1 $(TARGETDIR)/lib/
endif
else
	install $(LIBDIR)/ld-uClibc.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libcrypt.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libpthread.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libgcc_s.so.1 $(TARGETDIR)/lib/
	$(STRIP) $(TARGETDIR)/lib/libgcc_s.so.1
	install $(LIBDIR)/libc.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libdl.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libm.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libnsl.so.0 $(TARGETDIR)/lib/
ifeq ($(RTCONFIG_SSH),y)
	install $(LIBDIR)/libutil.so.0 $(TARGETDIR)/lib/
endif
endif

ifneq ($(RTCONFIG_OPTIMIZE_SHARED_LIBS),y)
ifeq ($(RTL8197F), y) # Realtek RTL8197F platform.
	install $(LIBDIR)/libresolv.so.1 $(TARGETDIR)/lib/
	$(STRIP) $(TARGETDIR)/lib/*.so.1
else
	install $(LIBDIR)/libresolv.so.0 $(TARGETDIR)/lib/
	$(STRIP) $(TARGETDIR)/lib/*.so.0
endif
endif

ifeq ($(RTCONFIG_LANTIQ),y)
	@cd $(TARGETDIR) && $(TOP)/others/rootprep-opt.sh
	@cd $(TARGETDIR) && ln -sf /tmp/wireless/lantiq opt/lantiq
	@echo ---
else
	@cd $(TARGETDIR) && $(TOP)/others/rootprep${BCMEX}.sh
	@echo ---
endif

ifeq ($(RTCONFIG_QCA),y)
	#FIXME
ifeq ($(MAPAC1300)$(MAPAC2200)$(VZWAC1300),y)
	cd $(TARGETDIR)/usr/lib && \
		ln -sf ../../lib/ld-uClibc.so.0 ld-uClibc.so.1 && \
		ln -sf ../../lib/libc.so.0 libc.so.1 && \
		ln -sf ../../lib/libdl.so.0 libdl.so.1 && \
		ln -sf ../../lib/libpthread.so.0 libpthread.so.1
endif
else ifeq ($(RTCONFIG_REALTEK),y)
	# do nothibg #
else ifeq ($(RTCONFIG_ALPINE)$(RTCONFIG_LANTIQ),y)
	#FIXME
else
ifeq ($(RTCONFIG_OPTIMIZE_SHARED_LIBS),y)
ifneq ($(RTCONFIG_BCMARM),y)
	@$(SRCBASE)/btools/libfoo.pl
endif
else
	@$(SRCBASE)/btools/libfoo.pl --noopt
endif
endif
	@chmod 0555 $(TARGETDIR)/lib/*.so*
	@chmod 0555 $(TARGETDIR)/usr/lib/*.so*

ifeq ($(RTCONFIG_QCA),y)
ifeq ($(RPAC51),y)
	@mv $(INSTALLDIR)/lib/firmware $(TARGETDIR)/lib/ || true
endif
endif

# !!TB - moved to run after libfoo.pl - to make sure shared libs include all symbols needed by extras
# separated/copied extra stuff
	@rm -rf $(PLATFORMDIR)/extras
	@mkdir $(PLATFORMDIR)/extras
	@mv $(TARGETDIR)/lib/modules/*/kernel/net/ipv4/ip_gre.*o $(PLATFORMDIR)/extras/ || true

	$(if $(RTCONFIG_OPENVPN),@cp -f,$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv)) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/tun.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_EBTABLES),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/bridge/netfilter/ebt*.*o $(PLATFORMDIR)/extras/ || true

	@cp $(TARGETDIR)/lib/modules/*/kernel/net/ipv4/netfilter/ip_set*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/net/ifb.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/net/sched/sch_red.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/fs/ntfs.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/fs/smbfs.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/fs/reiserfs.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/fs/hfsplus.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_NFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nfs.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_NFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nfsd.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_NFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/lockd.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_NFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/exportfs.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_NFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/sunrpc.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/net/auth_rpcgss.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/net/rpcsec_gss_krb5.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/fs/xfs.*o $(PLATFORMDIR)/extras/ || true
#	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/scsi/sr_mod.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/scanner.*o $(PLATFORMDIR)/extras/ || true

	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/usbserial.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/option.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/*acm.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/cdc-wdm.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/usb/usbnet.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/usb/asix.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/usb/cdc_ether.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/usb/rndis_host.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/usb/cdc_ncm.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/usb/qmi_wwan.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/usb/cdc_mbim.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -rf,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/usb/hso* $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/usb/ipheth.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/sierra.*o $(PLATFORMDIR)/extras/ || true

	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/usbkbd.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/usbmouse.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/hid*.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/ipw.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/audio.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/ov51*.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/pwc*.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/emi*.*o $(PLATFORMDIR)/extras/ || true
	@cp -f $(TARGETDIR)/lib/modules/*/kernel/drivers/net/mii.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/media/* $(PLATFORMDIR)/extras/ || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/media || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/sound/* $(PLATFORMDIR)/extras/ || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/sound || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/sound/* $(PLATFORMDIR)/extras/ || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/sound || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/input/* $(PLATFORMDIR)/extras/ || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/input || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/hid/* $(PLATFORMDIR)/extras/ || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/hid || true
	@cp -f $(TARGETDIR)/lib/modules/*/kernel/drivers/net/bcm57*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_PPTP),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/pptp.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_L2TP),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/pppol2tp.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/net/ppp_deflate.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/crypto/* $(PLATFORMDIR)/extras/ || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/crypto || true

	$(if $(NEED_EX_NLS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_cp9*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_NLS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_cp1251.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_NLS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_euc-jp.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_NLS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_sjis.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_NLS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_gb2312.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_NLS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_euc-kr.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_NLS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_big5.*o $(PLATFORMDIR)/extras/ || true

	$(if $(RTCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/cdrom/*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/scsi/*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/ext2.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/ext3.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/jbd.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/mbcache.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/fat.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/vfat.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/msdos.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/fuse.*o $(PLATFORMDIR)/extras/ || true
ifneq ($(RTCONFIG_USB),y)
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/usb || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/scsi || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/net/usb || true
endif

	$(if $(RTCONFIG_USB_EXTRAS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/connector/cn.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_USB_EXTRAS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/block/loop.*o $(PLATFORMDIR)/extras/ || true
ifneq ($(RTCONFIG_USB_EXTRAS),y)
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/connector || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/block || true
endif
#	$(if $(RTCONFIG_CIFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/cifs.*o $(PLATFORMDIR)/extras/ || true
	@cp -f $(TARGETDIR)/lib/modules/*/kernel/fs/cifs.*o $(PLATFORMDIR)/extras/ || true
	$(if $(or $(RTCONFIG_BRCM_NAND_JFFS2),$(RTCONFIG_JFFS2)),$(if $(RTCONFIG_JFFSV1),@mv,@cp -f),@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/jffs2.*o $(PLATFORMDIR)/extras/ || true
	$(if $(or $(RTCONFIG_BRCM_NAND_JFFS2),$(RTCONFIG_JFFS2)),$(if $(RTCONFIG_JFFSV1),@mv,@cp -f),@mv) $(TARGETDIR)/lib/modules/*/kernel/lib/zlib_*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(or $(RTCONFIG_BRCM_NAND_JFFS2),$(RTCONFIG_JFFS2)),$(if $(RTCONFIG_JFFSV1),@cp -f,@mv),@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/jffs.*o $(PLATFORMDIR)/extras/ || true
	[ ! -f $(TARGETDIR)/lib/modules/*/kernel/lib/* ] && rm -rf $(TARGETDIR)/lib/modules/*/kernel/lib || true
	$(if $(RTCONFIG_L7),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/ipv4/netfilter/ipt_layer7.*o $(PLATFORMDIR)/extras/ || true
	$(if $(RTCONFIG_L7),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/netfilter/xt_layer7.*o $(PLATFORMDIR)/extras/ || true

ifeq ($(RTCONFIG_JFFS2ND_BACKUP),y)
	@mkdir -p $(TARGETDIR)/asus_jffs
endif
	@mkdir -p $(PLATFORMDIR)/extras/apps
	@mkdir -p $(PLATFORMDIR)/extras/lib

	@mv $(TARGETDIR)/usr/sbin/ttcp $(PLATFORMDIR)/extras/apps/ || true
	@cp -f $(TARGETDIR)/usr/sbin/mii-tool $(PLATFORMDIR)/extras/apps/ || true
	$(if $(RTCONFIG_RALINK),@mv,@cp -f) $(TARGETDIR)/usr/sbin/robocfg $(PLATFORMDIR)/extras/apps/ || true

	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/usr/lib/libusb* $(PLATFORMDIR)/extras/lib/ || true
	$(if $(RTCONFIG_USB_MODEM),@cp -f,@mv) $(TARGETDIR)/usr/sbin/chat $(PLATFORMDIR)/extras/apps/ || true

	@mkdir -p $(TARGETDIR)/rom/etc/l7-protocols
ifeq ($(RTCONFIG_L7PAT),y)
	@cd layer7 && ./squish.sh
	cp layer7/squished/*.pat $(TARGETDIR)/rom/etc/l7-protocols
endif

ifneq ($(RTCONFIG_REALTEK),y)
	$(BUSYBOX_DIR)/examples/depmod.pl -k $(LINUXDIR)/vmlinux -b $(TARGETDIR)/lib/modules/*/
else
	$(BUSYBOX_DIR)/examples/depmod.pl -k $(LINUXDIR)/vmlinux -b $(TARGETDIR)/lib/modules/$(if $(RTCONFIG_REALTEK),$(KERNELRELEASE),*)/
endif

ifeq ($(RTCONFIG_ROMCFE),y)
	-cp $(SRCBASE)/cfe/cfe_`echo $(BUILD_NAME)|tr A-Z a-z`.bin $(TARGETDIR)/rom/cfe
endif
ifeq ($(RTCONFIG_ROMATECCODEFIX),y)
	-cp $(SRCBASE)/router/rom/cfe/`echo $(BUILD_NAME)|tr A-Z a-z`_fixlist.txt $(TARGETDIR)/rom/mac_chklist.txt
endif
ifeq ($(RTCONFIG_LANTIQ),y)
	cp -R rom_lantiq/opt/ $(TARGETDIR)/rom/opt/
	cp rom_lantiq/etc/wave_components.ver $(TARGETDIR)/rom/opt/lantiq/etc/
	cp rom_lantiq/etc/rc.d/config.sh $(TARGETDIR)/rom/opt/lantiq/etc/rc.d/
	cp rom_lantiq/etc/preinit.d/load_gphy_firmware_preinit.sh $(TARGETDIR)/sbin/
	rm -rf $(TARGETDIR)/rom/opt/lantiq/www
	install -D rom_lantiq/opt/lantiq/bin/dutserver $(TARGETDIR)/bin/
	# install -D rom_lantiq/bin/wave_wlan_dut_drvctrl.sh $(TARGETDIR)/bin/
	# install -D rom_lantiq/bin/wave_wlan_dut_file_saver.sh $(TARGETDIR)/bin/
	install -D rom_lantiq/opt/lantiq/bin/hostapd $(TARGETDIR)/bin/
	install -D rom_lantiq/opt/lantiq/bin/hostapd_cli $(TARGETDIR)/bin/
	install -D rom_lantiq/opt/lantiq/bin/hostapd.eap_user $(TARGETDIR)/bin/
	install -D rom_lantiq/usr/bin/unlzma $(TARGETDIR)/usr/bin/
	install -D rom_lantiq/opt/lantiq/usr/sbin/qosd $(TARGETDIR)/usr/sbin/
	install -D rom_lantiq/opt/lantiq/usr/sbin/qos_cli $(TARGETDIR)/usr/sbin/
	cd $(TARGETDIR)/usr/sbin && ln -sf qos_cli ./qoscfg
	cd $(TARGETDIR)/usr/sbin && ln -sf qos_cli ./ifcfg
	cd $(TARGETDIR)/usr/sbin && ln -sf qos_cli ./qcfg
	cd $(TARGETDIR)/usr/sbin && ln -sf qos_cli ./classcfg
	cp $(SRCBASE)/proprietary/sdk_7.2.1.21_05.04.00.61.7_AE/usr/lib/libncurses.so.5.9 $(TARGETDIR)/usr/lib/libncurses.so.5.9
	cd $(TARGETDIR)/usr/lib && ln -sf libncurses.so.5.9 ./libncurses.so.5
	# install -D rom_lantiq/opt/lantiq/lib/modules/3.10.104/directconnect_datapath.ko $(TARGETDIR)/lib/modules/3.10.104/
	cd $(TARGETDIR)/lib/modules/3.10.104/; ln -sf ../../../rom/opt/lantiq/lib/modules/3.10.104/directconnect_datapath.ko ./directconnect_datapath.ko
	cd $(TARGETDIR)/lib/modules/3.10.104/; ln -sf ../../../rom/opt/lantiq/lib/modules/3.10.104/drv_ifxos.ko ./drv_ifxos.ko
	cd $(TARGETDIR)/lib/modules/3.10.104/; ln -sf ../../../rom/opt/lantiq/lib/modules/3.10.104/drv_event_logger.ko ./drv_event_logger.ko
	cd $(TARGETDIR)/lib/modules/3.10.104/; ln -sf ../../../rom/opt/lantiq/lib/modules/3.10.104/ltq_pmcu.ko ./ltq_pmcu.ko
	cd $(TARGETDIR)/lib/modules/3.10.104/; ln -sf ../../../rom/opt/lantiq/lib/modules/3.10.104/ltq_temp.ko ./ltq_temp.ko
	cd $(TARGETDIR)/lib/modules/3.10.104/; ln -sf ../../../rom/opt/lantiq/lib/modules/3.10.104/dc_mode0-xrx500.ko ./dc_mode0-xrx500.ko
	install -D rom_lantiq/lib/firmware/mpe_fw_be.img $(TARGETDIR)/lib/firmware/
	install -D rom_lantiq/lib/firmware/ppe_fw.bin $(TARGETDIR)/lib/firmware/
	install -D rom_lantiq/usr/sbin/upgrade $(TARGETDIR)/usr/sbin/
	install -D rom_lantiq/usr/sbin/vol_mgmt $(TARGETDIR)/usr/sbin/
	install -D rom_lantiq/usr/sbin/ethtool $(TARGETDIR)/usr/sbin/
	install -D rom_lantiq/usr/sbin/read_img $(TARGETDIR)/usr/sbin/
	install -D rom_lantiq/usr/sbin/nandwrite $(TARGETDIR)/usr/sbin/
	install -D rom_lantiq/usr/sbin/nanddump $(TARGETDIR)/usr/sbin/
	install -D rom_lantiq/usr/sbin/uboot_env $(TARGETDIR)/usr/sbin/
	install -D rom_lantiq/usr/sbin/mem $(TARGETDIR)/usr/sbin/
	cd $(TARGETDIR)/usr/sbin && ln -sf ../../rom/opt/lantiq/bin/mtdump ./mtdump
	install -D rom_lantiq/usr/sbin/fapi_wlan_dbXml $(TARGETDIR)/usr/sbin/
	install -D rom_lantiq/usr/sbin/fapi_wlan_cli $(TARGETDIR)/usr/sbin/
	install -D rom_lantiq/sbin/udevd $(TARGETDIR)/sbin/
	install -D rom_lantiq/sbin/crda $(TARGETDIR)/sbin/
	install -D rom_lantiq/usr/sbin/iw $(TARGETDIR)/usr/sbin/
	install -D rom_lantiq/lib/librt-0.9.33.2.so $(TARGETDIR)/lib/
	install -D rom_lantiq/lib/librt.so.0 $(TARGETDIR)/lib/
	install -D rom_lantiq/usr/lib/libnl-tiny.so $(TARGETDIR)/usr/lib/
	install -D rom_lantiq/lib/librpc.so $(TARGETDIR)/lib/
	install -D rom_lantiq/usr/bin/switch_cli $(TARGETDIR)/usr/bin/
	install -D rom_lantiq/opt/lantiq/sbin/ppacmd $(TARGETDIR)/sbin/
	install -D rom_lantiq/opt/lantiq/usr/lib/libwwanfapi.so $(TARGETDIR)/usr/lib/
	install -D rom_lantiq/opt/lantiq/usr/lib/libqosipc.so $(TARGETDIR)/usr/lib/
	install -D rom_lantiq/opt/lantiq/usr/lib/libqosfapi.so $(TARGETDIR)/usr/lib/
	install -D rom_lantiq/usr/lib/libezxml.so $(TARGETDIR)/usr/lib/
	install -D rom_lantiq/usr/lib/libscapi.so $(TARGETDIR)/usr/lib/
	install -D rom_lantiq/usr/lib/libnl-3.so.200.20.0 $(TARGETDIR)/usr/lib/
	install -D rom_lantiq/usr/lib/libnl-3.so.200 $(TARGETDIR)/usr/lib/
	install -D rom_lantiq/usr/lib/libnl-genl-3.so.200.20.0 $(TARGETDIR)/usr/lib/
	install -D rom_lantiq/usr/lib/libnl-genl-3.so.200 $(TARGETDIR)/usr/lib/
	mkdir -p $(TARGETDIR)/usr/lib/crda/
	install -D rom_lantiq/usr/lib/crda/regulatory.bin $(TARGETDIR)/usr/lib/crda/
	# install -D rom_lantiq/usr/lib/libfapiwlancommon.so $(TARGETDIR)/usr/lib/
	install -D rom_lantiq/opt/lantiq/usr/lib/libhelper.so $(TARGETDIR)/usr/lib/
	install -D rom_lantiq/opt/lantiq/lib/libfapiwave.so $(TARGETDIR)/usr/lib/
	install -D rom_lantiq/etc/rc.d/wlan_discover $(TARGETDIR)/sbin/wlan_discover
	mkdir $(TARGETDIR)/rom/etc/; cp -Rf rom_lantiq/etc/udev $(TARGETDIR)/rom/etc/
endif
ifeq ($(RTCONFIG_ALPINE),y)
	install -D qsr10g_image/qsr10g-pcie.ko $(TARGETDIR)/lib/firmware/qsr10g-pcie.ko
	install -D qsr10g_image/pearl-linux.lzma.img $(TARGETDIR)/lib/firmware/pearl-linux.lzma.img
	install -D aqr107_image/aq-fw-download $(TARGETDIR)/sbin/aq-fw-download
	install -D aqr107_image/aqr_firmware.cld $(TARGETDIR)/lib/firmware/aqr_firmware.cld
	install -D rtk_bluetooth_firmware/serial/rtl8761a/rtl8761a_fw $(TARGETDIR)/lib/firmware/rtlbt/rtl8761a_fw
	install -D rtk_bluetooth_firmware/serial/rtl8761a/rtl8761at_fw $(TARGETDIR)/lib/firmware/rtlbt/rtl8761at_fw
	install -D rtk_bluetooth_firmware/serial/rtl8761a/rtl8761a_config $(TARGETDIR)/lib/firmware/rtlbt/rtl8761a_config
	install -D rtk_bluetooth_firmware/serial/rtl8761a/rtl8761at_config $(TARGETDIR)/lib/firmware/rtlbt/rtl8761at_config
	install -D ${SRCBASE}/tools/${TOOLCHAIN_NAME}/lib/libstdc++.so.6.0.19 $(TARGETDIR)/lib/libstdc++.so.6
	$(STRIP) $(TARGETDIR)/lib/libstdc++.so.6
	cd ${PLATFORMDIR}/target; cp ${SRCBASE}/proprietary/sbin/fw_print sbin/
	cd ${PLATFORMDIR}/target; cp ${SRCBASE}/proprietary/etc/fw_env.config sbin/
endif
ifeq ($(RTCONFIG_QTN),y)
	# install qtn boot-code and firmware
	install -D qtnimage/topaz-linux.lzma.img $(TARGETDIR)/rom/qtn/boot/topaz-linux.lzma.img
	install -D qtnimage/u-boot.bin $(TARGETDIR)/rom/qtn/boot/u-boot.bin
	ln -s /rom/qtn/boot/topaz-linux.lzma.img $(TARGETDIR)/rom/qtn/topaz-linux.lzma.img
	ln -s /rom/qtn/boot/u-boot.bin $(TARGETDIR)/rom/qtn/u-boot.bin
	install -D qtnimage/router_command.sh $(TARGETDIR)/rom/qtn/router_command.sh
	install -D qtnimage/qtn_dbg.sh $(TARGETDIR)/rom/qtn/qtn_dbg.sh
	install -D qtnimage/start-stateless-slave $(TARGETDIR)/rom/qtn/start-stateless-slave
	install -D qtnimage/tweak_qcomm $(TARGETDIR)/rom/qtn/tweak_qcomm
endif
ifeq ($(RTCONFIG_BWDPI),y)
	@install -d $(TARGETDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.so $(TARGETDIR)/usr/lib/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.ko $(TARGETDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.trf $(TARGETDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.cert $(TARGETDIR)/usr/bwdpi/
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred $(TARGETDIR)/usr/sbin/wred
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred_set_conf $(TARGETDIR)/usr/sbin/wred_set_conf
	-@install -D $(TOP)/bwdpi_source/RC_INDEP/wred_set_wbl $(TARGETDIR)/usr/sbin/wred_set_wbl
	@install -D $(TOP)/bwdpi_source/RC_INDEP/dcd $(TARGETDIR)/usr/sbin/dcd
	-@install -D $(TOP)/bwdpi_source/RC_INDEP/tcd $(TARGETDIR)/usr/sbin/tcd
	@install -D $(TOP)/bwdpi_source/RC_INDEP/shn_ctrl $(TARGETDIR)/usr/sbin/shn_ctrl
	@install -D $(TOP)/bwdpi_source/RC_INDEP/tdts_rule_agent $(TARGETDIR)/usr/sbin/tdts_rule_agent
	@install -D $(shell dirname $(shell which $(CXX)))/../lib/librt.so.0 $(TARGETDIR)/lib/librt.so.0
ifeq ($(RTCONFIG_RALINK),y)
	# Due to some library strip will make wred crash, need to rollback library from toolchains
	install $(LIBDIR)/libpthread.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libc.so.0 $(TARGETDIR)/lib/
endif
endif

ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_CFEZ),y)
	install -D envram_bin/envram_arm_7114/envrams $(TARGETDIR)/usr/sbin/envrams
	install -D envram_bin/envram_arm_7114/envram $(TARGETDIR)/usr/sbin/envram
endif
endif

	@echo ---

	@rm -f $(TARGETDIR)/lib/modules/*/build
endif
	@# Fix /usr/lib permission for OpenVPN authentication.
	@chmod 755 $(TARGETDIR)/usr/lib

gen_gpl_excludes_router:
	@[ ! -d ../../../buildtools/ ] || ../../../buildtools/gpl_excludes_router.sh $(SRCBASE) "$(obj-y) $(released_dir)"

image:
	@rm -f $(PLATFORMDIR)/target.image
ifneq (,$(filter y,$(RTCONFIG_RALINK) $(RTCONFIG_QCA) $(RTCONFIG_REALTEK) $(RTCONFIG_ALPINE) $(RTCONFIG_LANTIQ)))
	@$(MAKE) -C $(LINUXDIR)/scripts/squashfs $(MKSQUASHFS_TARGET)
	@$(LINUXDIR)/scripts/squashfs/$(MKSQUASHFS) $(TARGETDIR) $(PLATFORMDIR)/target.image -all-root -noappend -nopad
else ifeq ($(RTCONFIG_BCMARM),y)
	@$(SRCBASE)/ctools/mksquashfs $(TARGETDIR) $(PLATFORMDIR)/target.image -all-root -noappend
else ifeq ($(RTCONFIG_REALTEK),y)
	@$(MAKE) -C $(LINUXDIR)/rtkload rtk-clean
	@$(MAKE) -C $(LINUXDIR)/.. image
else
	@$(MAKE) -C $(LINUXDIR)/scripts/squashfs $(MKSQUASHFS_TARGET)
	@$(LINUXDIR)/scripts/squashfs/$(MKSQUASHFS) $(TARGETDIR) $(PLATFORMDIR)/target.image -all-root -noappend
endif
#	Package kernel and filesystem
#	if grep -q "CONFIG_EMBEDDED_RAMDISK=y" $(LINUXDIR)/.config ; then \
#		cp $(PLATFORMDIR)/target.image $(LINUXDIR)/arch/mips/ramdisk/$${CONFIG_EMBEDDED_RAMDISK_IMAGE} ; \
#		$(MAKE) -C $(LINUXDIR) zImage ; \
#	else \
#		cp $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz $(PLATFORMDIR)/ ; \
#		trx -o $(PLATFORMDIR)/linux.trx $(PLATFORMDIR)/vmlinuz $(PLATFORMDIR)/target.image ; \
#	fi

# 	Pad self-booting Linux to a 64 KB boundary
#	cp $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/zImage $(PLATFORMDIR)/
#	dd conv=sync bs=64k < $(PLATFORMDIR)/zImage > $(PLATFORMDIR)/linux.bin
# 	Append filesystem to self-booting Linux
#	cat $(PLATFORMDIR)/target.image >> $(PLATFORMDIR)/linux.bin

ifeq ($(RTL8197F), y) # Realtek RTL8197F platform.
libc:	$(LIBDIR)/ld-uClibc.so.1
else
libc:	$(LIBDIR)/ld-uClibc.so.0
endif
#	$(MAKE) -C ../../../tools-src/uClibc all
#	$(MAKE) -C ../../../tools-src/uClibc install


#
# cleaners
#

clean: clean-build $(obj-clean)
	rm -rf layer7/squished
	make -C config clean

clean-build: dummy
ifeq ($(HND_ROUTER),y)
	rm -rf $(TARGETDIR)/[a-z]*
	rm -rf $(INSTALLDIR)
	rm -rf $(BCM_FSBUILD_DIR)
else
	rm -rf $(TARGETDIR)
	rm -rf $(INSTALLDIR)
	rm -f $(PLATFORMDIR)/linux.trx $(PLATFORMDIR)/vmlinuz $(PLATFORMDIR)/target.image
	rm -rf $(PLATFORMDIR)/extras
	rm -rf kernel_header
endif

distclean: clean
ifneq ($(INSIDE_MAK),1)
	$(MAKE) -C $(SRCBASE) $@ INSIDE_MAK=1
endif
#	-rm -f $(LIBDIR)/*.so.0 $(LIBDIR)/*.so

#
# configuration
#

CONFIG_IN := config/config.in

config/conf config/mconf:
	$(MAKE) -C config

rconf: config/conf
	config/conf $(CONFIG_IN)

rmconf: config/mconf
	config/mconf $(CONFIG_IN)

roldconf: config/conf
	config/conf -o $(CONFIG_IN)
	$(MAKE) shared-clean libdisk-clean rc-clean nvram$(BCMEX)$(EX7)-clean httpd-clean prebuilt-clean libbcmcrypto-clean
#	@$(MAKE) iptables-clean ebtables-clean pppd-clean zebra-clean openssl-clean mssl-clean pppd-clean
	$(MAKE) dnsmasq-clean iproute2-clean networkmap-clean
ifeq ($(RTCONFIG_BCMARM),y)
	$(MAKE) compressed-clean
endif
#ifeq ($(RTCONFIG_DHDAP),y)
#ifeq ($(RTCONFIG_BCM7),y)
#ifneq ($(wildcard $(SRCBASE_FW)/wl/sys),)
#ifeq ($(wildcard /projects/hnd/tools/linux/hndtools-armeabi-2011.09),)
#	# build 43602 src and to match its path
#	sudo mkdir /projects/hnd/tools/linux -p
#	sudo rm -rf /projects/hnd/tools/linux/hndtools-armeabi-2011.09
#	sudo ln -sf $(SRCBASE)/toolchains/hndtools-armeabi-2011.09 /projects/hnd/tools/linux/hndtools-armeabi-2011.09
#endif
#endif
#else ifeq ($(RTCONFIG_BCM_7114),y) # hnd models use same dongle toolchain as 7114
#ifeq ($(wildcard /projects/hnd/tools/linux/hndtools-armeabi-2013.11),)
#	# build pciefd
#	sudo mkdir /projects/hnd/tools/linux/bin -p
#	sudo rm -rf /projects/hnd/tools/linux/hndtools-armeabi-2013.11
#	sudo ln -sf $(SRCBASE)/toolchains/hndtools-armeabi-2013.11 /projects/hnd/tools/linux/hndtools-armeabi-2013.11
#	sudo ln -sf $(SRCBASE)/toolchains/fwtag.ini /projects/hnd/tools/linux/bin/fwtag.ini
#endif
#endif
#endif
#ifeq ($(HND_ROUTER),y)
#ifeq ($(wildcard /opt/toolchains),)
#	sudo mkdir /opt/toolchains -p
#endif
#ifeq ($(wildcard /opt/toolchains/crosstools-aarch64-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25),)
#	sudo ln -sf $(SRCBASE)/toolchains/crosstools-aarch64-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25 /opt/toolchains/crosstools-aarch64-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25
#endif
#ifeq ($(wildcard /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25),)
#	sudo ln -sf $(SRCBASE)/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25 /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25
#endif
#endif

kconf:
	$(MAKE) -C $(LINUXDIR) config

kmconf:
	$(MAKE) -C $(LINUXDIR) menuconfig

koldconf:
ifneq ($(HND_ROUTER),y)
	$(MAKE) -C $(LINUXDIR) oldconfig
ifeq ($(ALPINE)$(LANTIQ),y)
	$(MAKE) -C $(LINUXDIR) include/generated/uapi/linux/version.h
else
	$(MAKE) -C $(LINUXDIR) include/linux/version.h
endif
endif

bboldconf:
	$(MAKE) -C $(BUSYBOX_DIR) oldconfig

config conf: rconf kconf

menuconfig mconf: rmconf kmconf

oldconfig oldconf: koldconf roldconf bboldconf


#
# overrides and extra dependencies
#

ifeq ($(HND_ROUTER),y)
define makeBusybox
	@$(MAKE) -C $(BUSYBOX_DIR) $(1) $(PARALLEL_BUILD) \
		EXTRA_CFLAGS="-fPIC $(PRIVATE_EXTRACFLAGS) -I$(STAGEDIR)/usr/include -I$(TOOL_SYS_INCLUDE) \
			$(if $(RTCONFIG_PROTECTION_SERVER),-DSECURITY_NOTIFY,)" \
		EXTRA_LDFLAGS="-L$(STAGEDIR)/usr/lib" \
		EXTRA_LDLIBS="$(if $(RTCONFIG_PROTECTION_SERVER),ptcsrv,)"
endef
else
define makeBusybox
	@$(MAKE) -C $(BUSYBOX_DIR) $(1) $(PARALLEL_BUILD) \
		EXTRA_CFLAGS="-fPIC $(EXTRACFLAGS) -I$(STAGEDIR)/usr/include -idirafter $(KDIR)/include \
			$(if $(RTCONFIG_PROTECTION_SERVER),-DSECURITY_NOTIFY,)" \
		EXTRA_LDFLAGS="-L$(STAGEDIR)/usr/lib" \
		EXTRA_LDLIBS="$(if $(RTCONFIG_PROTECTION_SERVER),ptcsrv,)"
endef
endif

busybox busybox-1.17.4 busybox-1.24.1: dummy $(if $(RTCONFIG_PROTECTION_SERVER),protect_srv-stage,)
	$(call makeBusybox)

busybox-install: dummy $(if $(RTCONFIG_PROTECTION_SERVER),protect_srv-stage,)
	$(call makeBusybox,install CONFIG_PREFIX=$(INSTALLDIR)/$(BUSYBOX))
	chmod 4755 $(INSTALLDIR)/$(BUSYBOX)/bin/busybox

busybox-%-install: dummy $(if $(RTCONFIG_PROTECTION_SERVER),protect_srv-stage,)
	$(call makeBusybox,install CONFIG_PREFIX=$(INSTALLDIR)/$(BUSYBOX))
	chmod 4755 $(INSTALLDIR)/$(BUSYBOX)/bin/busybox

busybox-clean:
	$(MAKE) -C $(BUSYBOX_DIR) distclean

busybox-%-clean:
	$(MAKE) -C $(BUSYBOX_DIR) distclean

busybox-config:
	$(MAKE) -C $(BUSYBOX_DIR) menuconfig

busybox-%-config:
	$(MAKE) -C $(BUSYBOX_DIR) menuconfig

infosvr: shared nvram${BCMEX}$(EX7)

infosvr-install:
	$(MAKE) -C infosvr INSTALLDIR=$(INSTALLDIR)/infosvr install

#libdisk: shared nvram$(BCMEX)$(EX7)

httpd: shared nvram$(BCMEX)$(EX7) libdisk $(if $(RTCONFIG_HTTPS),mssl) $(if $(RTCONFIG_OPENVPN),libvpn) $(if $(RTCONFIG_PUSH_EMAIL),push_log) $(if $(RTCONFIG_BWDPI),bwdpi_source) json-c $(if $(RTCONFIG_CFGSYNC), $(CFGSYNC_PKG))
	@$(SEP)
ifeq ($(HND_ROUTER),y)
	cd httpd && rm -f wlioctl.h && ln -sf $(SRCBASE)/../../dhd/src/include/wlioctl.h  wlioctl.h && rm -f bcmwifi_rates.h && ln -sf $(SRCBASE)/../../dhd/src/shared/bcmwifi/include/bcmwifi_rates.h bcmwifi_rates.h && rm -f wlioctl_defs.h && ln -sf $(SRCBASE)/../../dhd/components/shared/devctrl_if/wlioctl_defs.h wlioctl_defs.h
endif
	@-rm -f httpd/prebuild/*.o || true
	@-cp -f httpd/prebuild/$(BUILD_NAME)/* httpd/prebuild/
	$(MAKE) -C httpd

httpd-install:
	$(MAKE) -C httpd INSTALLDIR=$(INSTALLDIR)/httpd install

httpd_uam: shared nvram$(BCMEX)$(EX7) libdisk $(if $(RTCONFIG_HTTPS),mssl) $(if $(RTCONFIG_PUSH_EMAIL),push_log) $(if $(RTCONFIG_FBWIFI),fb_wifi)
	@$(SEP)
	@$(MAKE) -C httpd_uam

httpd_uam-install:
	$(MAKE) -C httpd_uam INSTALLDIR=$(INSTALLDIR)/httpd_uam install

bonnie: bonnie/Makefile
	$(MAKE) CXX=$(CXX) -C bonnie

bonnie/Makefile:
	$(MAKE) bonnie-configure

bonnie-configure:
	( cd bonnie ; \
		$(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
	)

bonnie-install:
	install -D bonnie/bonnie++ $(INSTALLDIR)/bonnie/sbin/bonnie++
	$(STRIP) $(INSTALLDIR)/bonnie/sbin/bonnie++
ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(HND_ROUTER),y)
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
else
	install -D $(shell dirname $(shell which $(CXX)))/../arm-brcm-linux-uclibcgnueabi/lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
endif
else
	install -D $(shell dirname $(shell which $(CXX)))/../lib/libstdc++.so.6 $(INSTALLDIR)/bonnie/usr/lib/libstdc++.so.6
endif

bonnie-clean:
	[ ! -f bonnie/Makefile ] || $(MAKE) -C bonnie clean
	@rm -f bonnie/Makefile

stress-1.x: stress-1.x/Makefile
	$(MAKE) -C stress-1.x

stress-1.x/Makefile:
	$(MAKE) stress-1.x-configure

stress-1.x-configure:
	( cd stress-1.x ; \
		./autogen.sh && $(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
	)

stress-1.x-install:
	install -D stress-1.x/src/stress $(INSTALLDIR)/stress-1.x/usr/sbin/stress
	$(STRIP) $(INSTALLDIR)/stress-1.x/usr/sbin/stress

stress-1.x-clean:
	[ ! -f stress-1.x/Makefile ] || $(MAKE) -C stress-1.x clean
	@rm -f stress-1.x/Makefile

www-install:
ifeq ($(HND_ROUTER),y)
	-mkdir $(SRCBASE)/image
endif
	@$(MAKE) -C www install INSTALLDIR=$(INSTALLDIR)/www TOMATO_EXPERIMENTAL=$(TOMATO_EXPERIMENTAL)

matrixssl:
	@$(SEP)
	@$(MAKE) -C matrixssl/src

matrixssl-install:
	@true

matrixssl-clean:
	-@$(MAKE) -C matrixssl/src clean

cyassl/stamp-h1:
	@cd cyassl && CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) \
	CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections \
	-DNO_MD4 -DNO_AES -DNO_ERROR_STRINGS -DNO_HC128 -DNO_RABBIT -DNO_PSK -DNO_DSA -DNO_DH -DNO_PWDBASED" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
	PTHREAD_CFLAGS="-lpthread" PTHREAD_LIBS="-lpthread" \
	$(CONFIGURE) --with-libz=no
	@touch $@

cyassl: cyassl/stamp-h1
	@$(SEP)
	@$(MAKE) -C cyassl

cyassl-clean:
	-@$(MAKE) -C cyassl clean
	@rm -f cyassl/stamp-h1

cyassl-install:
	@true

ifeq ($(RTCONFIG_OPENVPN),y)
OPENSSL_CIPHERS:=enable-rc5 no-rc4
else
#OPENSSL_CIPHERS:=no-dh no-idea no-rc2 no-rc5 no-aes no-aes192 no-cast no-des no-modes no-tls1 no-tlsext
OPENSSL_CIPHERS:=no-rc4
endif

#OPENSSL_CIPHERS:=enable-aes enable-tls1 enable-tlsext

openssl-1.0.0q/Makefile:
	cd openssl-1.0.0q && \
	./Configure $(HOSTCONFIG) --prefix=/usr --openssldir=/etc --cross-compile-prefix=' ' \
	-ffunction-sections -fdata-sections -Wl,--gc-sections \
	shared $(OPENSSL_CIPHERS)
#	no-sha0 no-smime no-camellia no-krb5 no-rmd160 no-ripemd \
#	no-seed no-capieng no-cms no-gms no-gmp no-rfc3779 \
#	no-ec no-ecdh no-ecdsa no-err no-hw no-jpake no-threads \
#	no-zlib no-engine no-engines no-sse2 no-perlasm \
#	no-dtls1 no-store no-psk no-md2 no-mdc2 no-ts

	-@$(MAKE) -C openssl-1.0.0q clean depend
	@touch $@

openssl-1.0.0q: openssl-1.0.0q/Makefile
	$(MAKE) -C openssl-1.0.0q && $(MAKE) $@-stage

openssl-1.0.0q-clean:
	[ ! -f openssl-1.0.0q/Makefile ] || $(MAKE) -C openssl-1.0.0q clean
	@rm -f openssl-1.0.0q/Makefile

openssl-1.0.0q-install:
	if [ -f openssl-1.0.0q/Makefile ] ; then \
		install -D openssl-1.0.0q/libcrypto-1.0.0q.so.1.0.0 $(INSTALLDIR)/openssl-1.0.0q/usr/lib/libcrypto-1.0.0q.so.1.0.0 ; \
		install -D openssl-1.0.0q/libssl-1.0.0q.so.1.0.0 $(INSTALLDIR)/openssl-1.0.0q/usr/lib/libssl-1.0.0q.so.1.0.0 ; \
		$(STRIP) $(INSTALLDIR)/openssl-1.0.0q/usr/lib/libssl-1.0.0q.so.1.0.0 ; \
		$(STRIP) $(INSTALLDIR)/openssl-1.0.0q/usr/lib/libcrypto-1.0.0q.so.1.0.0 ; \
		cd $(INSTALLDIR)/openssl-1.0.0q/usr/lib && ln -sf libcrypto-1.0.0q.so.1.0.0 libcrypto-1.0.0q.so ; \
		cd $(INSTALLDIR)/openssl-1.0.0q/usr/lib && ln -sf libssl-1.0.0q.so.1.0.0 libssl-1.0.0q.so ; \
	fi

openssl-1.0.0q-stage:
#	only main staged
#	@$(MAKE) -C openssl-1.0.0q install_sw INSTALL_PREFIX=$(STAGEDIR)
	@true

openssl/Makefile:
	cd openssl && \
	./Configure $(HOSTCONFIG) --prefix=/usr --openssldir=/etc --cross-compile-prefix=' ' \
	-ffunction-sections -fdata-sections -Wl,--gc-sections \
	shared $(OPENSSL_CIPHERS) no-ssl2 no-ssl3
#	no-sha0 no-smime no-camellia no-krb5 no-rmd160 no-ripemd \
#	no-seed no-capieng no-cms no-gms no-gmp no-rfc3779 \
#	no-ec no-ecdh no-ecdsa no-err no-hw no-jpake no-threads \
#	no-zlib no-engine no-engines no-sse2 no-perlasm \
#	no-dtls1 no-store no-psk no-md2 no-mdc2 no-ts

	-@$(MAKE) -C openssl clean depend
	@touch $@

fsbuild:
	-mkdir -p $(BCM_FSBUILD_DIR)/public/lib
	-mkdir -p $(BCM_FSBUILD_DIR)/public/bin
	-mkdir -p $(BCM_FSBUILD_DIR)/public/sbin
	-mkdir -p $(BCM_FSBUILD_DIR)/usr/lib
	-mkdir -p $(BCM_FSBUILD_DIR)/usr/sbin
	-mkdir -p $(BCM_FSBUILD_DIR)/public/include/openssl
	-cp -u openssl/include/openssl/* $(BCM_FSBUILD_DIR)/public/include/openssl
	-mkdir -p $(INSTALL_DIR)/bin
	-mkdir -p $(INSTALL_DIR)/rom/rom
	#-mkdir -p $(INSTALL_DIR)/etc
	-mkdir -p $(INSTALL_DIR)/lib/public
	-mkdir -p $(INSTALL_DIR)/lib/private
	-mkdir -p $(INSTALL_DIR)/lib/aarch64
	#-mkdir -p $(INSTALL_DIR)/opt
	-mkdir -p $(INSTALL_DIR)/sbin
	-mkdir -p $(INSTALL_DIR)/usr
	-mkdir -p $(INSTALL_DIR)/var
	-mkdir -p $(INSTALL_DIR)/mnt
	-mkdir -p $(INSTALL_DIR)/tmp/etc
	-mkdir -p $(INSTALL_DIR)/www
	#-cp $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/lib/libgcc_s* $(INSTALL_DIR)/lib/public
	#-cd $(INSTALL_DIR)/lib/public && rm -f libgcc_s.so && ln -sf libgcc_s.so.1 libgcc_s.so


ifeq ($(HND_ROUTER),y)
openssl: openssl/Makefile fsbuild
else
openssl: openssl/Makefile
endif
	$(MAKE) -C openssl && $(MAKE) $@-stage

openssl-clean:
	[ ! -f openssl/Makefile ] || $(MAKE) -C openssl clean depend
	@rm -f openssl/Makefile

ifeq ($(HND_ROUTER),y)
INSTALLSUBDIR=
else
INSTALLSUBDIR=/openssl
endif

openssl-install:
	[ ! -f openssl/Makefile ] || install -D openssl/libcrypto.so.1.0.0 $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib/libcrypto.so.1.0.0
	[ ! -f openssl/Makefile ] || $(STRIP) $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib/libcrypto.so.1.0.0
	[ ! -f openssl/Makefile ] || cd $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib && rm -f libcrypto.so && ln -sf libcrypto.so.1.0.0 libcrypto.so

	[ ! -f openssl/Makefile ] || install -D openssl/apps/openssl $(INSTALLDIR)$(INSTALLSUBDIR)/usr/sbin/openssl
	[ ! -f openssl/Makefile ] || $(STRIP) $(INSTALLDIR)$(INSTALLSUBDIR)/usr/sbin/openssl
	[ ! -f openssl/Makefile ] || chmod 0500 $(INSTALLDIR)$(INSTALLSUBDIR)/usr/sbin/openssl

ifeq ($(or $(RTCONFIG_WEBDAV),$(RTCONFIG_FTP_SSL),$(RTCONFIG_OPENVPN),$(RTCONFIG_HTTPS)),y)
	[ ! -f openssl/Makefile ] || install -D openssl/libssl.so.1.0.0 $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib/libssl.so.1.0.0
	[ ! -f openssl/Makefile ] || $(STRIP) $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib/libssl.so.1.0.0
	[ ! -f openssl/Makefile ] || cd $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib && rm -f libssl.so && ln -sf libssl.so.1.0.0 libssl.so
endif
ifeq ($(HND_ROUTER),y)
	[ ! -f openssl/Makefile ] || mkdir -p $(BCM_FSBUILD_DIR)/public/include/openssl
	[ ! -f openssl/Makefile ] || cp -u openssl/include/openssl/* $(BCM_FSBUILD_DIR)/public/include/openssl
endif
	[ ! -f openssl/Makefile ] || install -D -m 0500 httpd/gencert.sh $(INSTALLDIR)$(INSTALLSUBDIR)/usr/sbin/gencert.sh

openssl-stage:
	$(MAKE) -C openssl install_sw INSTALL_PREFIX=$(STAGEDIR)

mssl: openssl

mssl-install:
	$(MAKE) -C mssl INSTALLDIR=$(INSTALLDIR)/mssl install

wb: json-c openssl libxml2 curl-7.21.7

wb-install:
	$(MAKE) -C wb INSTALLDIR=$(INSTALLDIR)/wb install

push_log: wb

rc: shared nvram$(BCMEX)$(EX7) libbcmcrypto libdisk $(if $(RTCONFIG_FBWIFI),fb_wifi) $(if $(RTCONFIG_QTN),libqcsapi_client) $(if $(CONFIG_LIBBCM),libbcm) $(if $(RTCONFIG_BWDPI),bwdpi_source) $(if $(RTCONFIG_USB_SMS_MODEM),smspdu) $(if $(RTCONFIG_HND_ROUTER),bcm_flashutil bcm_util bcm_boardctl) $(if $(RTCONFIG_OPENVPN),libvpn) $(if $(RTCONFIG_LANTIQ),libfapi-0.1) $(if $(RTCONFIG_CFGSYNC), $(CFGSYNC_PKG) $(if $(RTCONFIG_AMAS), amas-utils) $(if $(RTCONFIG_LANTIQ),hostapd-2.6))
	@$(SEP)
# Dig into our collection of prebuilt objects based on model
	@-rm -f rc/prebuild/*.o || true
	@-cp -f rc/prebuild/$(BUILD_NAME)/* rc/prebuild/
	@$(MAKE) -C rc

rc-install:
	$(MAKE) -C rc INSTALLDIR=$(INSTALLDIR)/rc install

networkmap: shared json-c

networkmap-install:
	$(MAKE) -C networkmap INSTALLDIR=$(INSTALLDIR)/networkmap install

aura_sw: aura_sw 

aura_sw-install:
	$(MAKE) -C aura_sw INSTALLDIR=$(INSTALLDIR)/aura_sw install

bridge/stamp-h1:
ifneq ($(or $(CONFIG_LINUX3_14),$(findstring linux-3.10,$(LINUXDIR))),)
	cd bridge && CFLAGS="-Os -g $(EXTRACFLAGS)" \
	$(CONFIGURE) --prefix="" --with-linux-headers=$(KDIR)/include
else
	cd bridge && CFLAGS="-Os -g $(EXTRACFLAGS)" \
	$(CONFIGURE) --prefix="" --with-linux-headers=$(LINUX_INC_DIR)/include
endif
	touch $@

bridge: bridge/stamp-h1
	@$(SEP)
	@$(MAKE) -C bridge

bridge-clean:
	-@$(MAKE) -C bridge clean
	@rm -f bridge/stamp-h1

bridge-install:
	install -D bridge/brctl/brctl $(INSTALLDIR)/bridge/usr/sbin/brctl
	$(STRIP) $(INSTALLDIR)/bridge/usr/sbin/brctl

rstpd:
	$(MAKE) -C rstpd

rstpd-clean:
	$(MAKE) -C rstpd clean

rstpd-install:
	install -D rstpd/rstpd $(INSTALLDIR)/rstpd/usr/sbin/rstpd
	$(STRIP) $(INSTALLDIR)/rstpd/usr/sbin/rstpd

dmalloc: dmalloc/Makefile dmalloc/dmalloc.h.2 dmalloc/settings.h
	$(MAKE) -C dmalloc all shlib cxx && $(MAKE) $@-stage

dmalloc/Makefile dmalloc/dmalloc.h.2 dmalloc/settings.h:
	$(MAKE) dmalloc-configure

dmalloc-configure:
	( cd dmalloc ; \
		$(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
		--enable-cxx --enable-threads --enable-shlib --with-pagesize=12 \
	)

dmalloc-install: dmalloc
	install -D $(STAGEDIR)/usr/sbin/dmalloc $(INSTALLDIR)/dmalloc/usr/sbin/dmalloc
	install -d $(INSTALLDIR)/dmalloc/usr/lib
	install -D $(STAGEDIR)/usr/lib/libdmalloc*.so* $(INSTALLDIR)/dmalloc/usr/lib
	$(STRIP) $(INSTALLDIR)/dmalloc/usr/sbin/dmalloc
	$(STRIP) $(INSTALLDIR)/dmalloc/usr/lib/*.so*

dmalloc-clean:
	[ ! -f dmalloc/Makefile ] || $(MAKE) -C dmalloc clean
	@rm -f dmalloc/{conf.h,dmalloc.h,dmalloc.h.2,settings.h,Makefile}

ifeq ($(RTCONFIG_BCM7),y)
$(obj-pciefd) :
# Build PCIEFD firmware only if it is not prebuilt
ifeq ($(RTCONFIG_DHDAP),y)
ifneq ($(wildcard $(SRCBASE_FW)/wl/sys),)
	+$(MAKE) CROSS_COMPILE=arm-none-eabi -C $(SRCBASE_FW)/dongle/rte/wl $(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)
	if [ -f $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/rtecdc_$(patsubst %-roml-obj,%,$@).h ]; then \
		cp $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/rtecdc_$(patsubst %-roml-obj,%,$@).h $(SRCBASE_DHD)/shared/rtecdc_$(patsubst %-roml-obj,%,$@).h && \
		echo "#include <rtecdc_$(patsubst %-roml-obj,%,$@).h>" >> $(PCIEFD_EMBED_HEADER); \
	fi;
	if [ -f $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/rtecdc_$(patsubst %-ram-obj,%,$@).h ]; then \
		cp $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/rtecdc_$(patsubst %-ram-obj,%,$@).h $(SRCBASE_DHD)/shared/rtecdc_$(patsubst %-ram-obj,%,$@).h && \
		echo "#include <rtecdc_$(patsubst %-ram-obj,%,$@).h>" >> $(PCIEFD_EMBED_HEADER); \
	fi;
	if [ -f $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/wlc_clm_data.c ]; then \
		cp $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/wlc_clm_data.c $(SRCBASE_FW)/wl/clm/src/wlc_clm_data.c.GEN && \
		cp $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/wlc_clm_data_inc.c $(SRCBASE_FW)/wl/clm/src/wlc_clm_data_inc.c.GEN; \
	fi;
endif
endif

pciefd-cleangen:
# Clean PCIEFD firmware only if it is not prebuilt
ifeq ($(RTCONFIG_DHDAP),y)
ifneq ($(wildcard $(SRCBASE_FW)/wl/sys),)
	rm -f $(PCIEFD_EMBED_HEADER)
	cp -f $(PCIEFD_EMBED_HEADER_TEMPLATE) $(PCIEFD_EMBED_HEADER)
endif
endif

$(clean-pciefd):
ifeq ($(RTCONFIG_DHDAP),y)
ifneq ($(wildcard $(SRCBASE_FW)/wl/sys),)
	+$(MAKE) CROSS_COMPILE=arm-none-eabi -C $(SRCBASE_FW)/dongle/rte/wl clean
	rm -f $(SRCBASE_DHD)/shared/rtecdc*.h
endif
endif

pciefd: pciefd-cleangen $(obj-pciefd)

pciefd-clean: pciefd-cleangen $(clean-pciefd)

pciefd-install:
	# Nothing to be done here
	@true
endif	# BCM7

#ifeq ($(HND_ROUTER),y)
ifeq ($(RTCONFIG_DHDAP),y)
dhd:
	@true
ifneq ($(wildcard $(SRCBASE_DHD)/dhd/exe),)
	$(MAKE) TARGET_PREFIX=$(CROSS_COMPILE) -C $(SRCBASE_DHD)/dhd/exe
endif

dhd-clean :
ifneq ($(wildcard $(SRCBASE_DHD)/dhd/exe),)
	$(MAKE) TARGET_PREFIX=$(CROSS_COMPILE) -C $(SRCBASE_DHD)/dhd/exe clean
	rm -f $(INSTALLDIR)/dhd/usr/sbin/dhd
	cd $(SRCBASE_DHD) && rm -f `find ./ -name "*.cmd" && find ./ -name "*.o"`
endif

dhd-install :
ifneq ($(wildcard $(SRCBASE_DHD)/dhd/exe),)
	install -d $(INSTALLDIR)/dhd/usr/sbin
	install $(SRCBASE_DHD)/dhd/exe/dhd $(INSTALLDIR)/dhd/usr/sbin
	$(STRIP) $(INSTALLDIR)/dhd/usr/sbin/dhd
endif
#endif

.PHONY: dhd_monitor

dhd_monitor: shared nvram$(BCMEX)$(EX7)
ifeq ($(RTCONFIG_DHDAP),y)
	+$(MAKE) LINUXDIR=$(LINUX_INC_DIR) EXTRA_LDFLAGS=$(EXTRA_LDFLAGS) -C dhd_monitor dhd_monitor
endif

dhd_monitor-install:
ifeq ($(RTCONFIG_DHDAP),y)
	+$(MAKE) -C dhd_monitor INSTALLDIR=$(INSTALLDIR)/dhd_monitor install
endif

dhd_monitor-clean:
ifeq ($(RTCONFIG_DHDAP),y)
	+$(MAKE) -C dhd_monitor clean
endif
endif


dnsmasq:  $(if $(RTCONFIG_DNSSEC),nettle,)
	@$(SEP)
	@$(MAKE) -C dnsmasq \
	COPTS="-DHAVE_BROKEN_RTC -DHAVE_LEASEFILE_EXPIRE -DNO_ID -DNO_AUTH -DNO_INOTIFY -DNO_GMP \
		$(if $(RTCONFIG_IPV6),-DUSE_IPV6,-DNO_IPV6) \
		$(if $(RTCONFIG_USB_EXTRAS)||$(RTCONFIG_TR069),,-DNO_SCRIPT) \
		$(if $(RTCONFIG_USB_EXTRAS),,-DNO_TFTP) \
		$(if $(RTCONFIG_DNSSEC),-I$(TOP)/nettle/include -I$(TOP)/gmp -DHAVE_DNSSEC -DHAVE_DNSSEC_STATIC,)" \
	CFLAGS="-Os -ffunction-sections -fdata-sections $(EXTRACFLAGS)" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections $(EXTRALDFLAGS) \
		$(if $(RTCONFIG_DNSSEC),-L$(TOP)/nettle/lib -L$(TOP)/gmp/.libs,)" \
	$(if $(RTCONFIG_DNSSEC),PKG_CONFIG_PATH="$(TOP)/nettle/lib/pkgconfig",)

dnsmasq-install:
	install -D dnsmasq/src/dnsmasq $(INSTALLDIR)/dnsmasq/usr/sbin/dnsmasq
	$(STRIP) $(INSTALLDIR)/dnsmasq/usr/sbin/dnsmasq

iptables:
	@$(SEP)
	$(MAKE) -C iptables BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUX_INC_DIR) COPT_FLAGS="-Os $(EXTRACFLAGS) -U CONFIG_NVRAM_SIZE"

iptables-install:
	install -D iptables/iptables $(INSTALLDIR)/iptables/usr/sbin/iptables
	cd $(INSTALLDIR)/iptables/usr/sbin && \
		ln -sf iptables iptables-save && \
		ln -sf iptables iptables-restore

	install -d $(INSTALLDIR)/iptables/usr/lib/iptables
	install -D iptables/extensions/*.so $(INSTALLDIR)/iptables/usr/lib/iptables/

	install -D iptables/libiptc.so $(INSTALLDIR)/iptables/usr/lib/libiptc.so

	$(STRIP) $(INSTALLDIR)/iptables/usr/sbin/iptables
	$(STRIP) $(INSTALLDIR)/iptables/usr/lib/iptables/*.so
	$(STRIP) $(INSTALLDIR)/iptables/usr/lib/libiptc.so

ifeq ($(RTCONFIG_IPV6),y)
	install iptables/ip6tables $(INSTALLDIR)/iptables/usr/sbin/ip6tables
	$(STRIP) $(INSTALLDIR)/iptables/usr/sbin/ip6tables
	cd $(INSTALLDIR)/iptables/usr/sbin && \
		ln -sf ip6tables ip6tables-save && \
		ln -sf ip6tables ip6tables-restore
endif

iptables-clean:
	-@$(MAKE) -C iptables KERNEL_DIR=$(LINUX_INC_DIR) clean

iptables-1.4.21/configure:
	cd iptables-1.4.21 && ./autogen.sh

ifeq ($(RTCONFIG_ALPINE)$(RTCONFIG_LANTIQ),y)
iptables-1.4.21/remove_configure:
	rm -f iptables-1.4.21/configure
endif

ifeq ($(RTCONFIG_ALPINE)$(RTCONFIG_LANTIQ),y)
iptables-1.4.21/Makefile: iptables-1.4.21/remove_configure iptables-1.4.21/configure
else
iptables-1.4.21/Makefile: iptables-1.4.21/configure
endif
	cd iptables-1.4.21 && $(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
		--with-kernel=$(LINUXDIR) \
		$(if $(RTCONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
		CFLAGS="-Os $(EXTRACFLAGS) -U CONFIG_NVRAM_SIZE"

iptables-1.4.21: iptables-1.4.21/Makefile
	@$(SEP)
	$(MAKE) -C $@ KERNEL_DIR=$(LINUXDIR) COPT_FLAGS="-Os $(EXTRACFLAGS) -U CONFIG_NVRAM_SIZE"

iptables-1.4.21-install:
	install -D iptables-1.4.21/iptables/.libs/xtables-multi $(INSTALLDIR)/iptables-1.4.21/usr/sbin/xtables-multi
	cd $(INSTALLDIR)/iptables-1.4.21/usr/sbin && \
		ln -sf xtables-multi iptables-restore && \
		ln -sf xtables-multi iptables-save && \
		ln -sf xtables-multi iptables
	install -d $(INSTALLDIR)/iptables-1.4.21/usr/lib/xtables
	install -D iptables-1.4.21/libiptc/.libs/libiptc.so $(INSTALLDIR)/iptables-1.4.21/usr/lib/libiptc.so
	install -D iptables-1.4.21/libiptc/.libs/libip4tc.so $(INSTALLDIR)/iptables-1.4.21/usr/lib/libip4tc.so
	cd $(INSTALLDIR)/iptables-1.4.21/usr/lib && \
		ln -sf libiptc.so  libiptc.so.0 && \
		ln -sf libiptc.so  libiptc.so.0.0.0 && \
		ln -sf libip4tc.so libip4tc.so.0 && \
		ln -sf libip4tc.so libip4tc.so.0.0.0
	install -D iptables-1.4.21/libxtables/.libs/lib*.so $(INSTALLDIR)/iptables-1.4.21/usr/lib/
	cd $(INSTALLDIR)/iptables-1.4.21/usr/lib && \
		ln -sf libxtables.so libxtables.so.10 && \
		ln -sf libxtables.so libxtables.so.10.0.0
	install -D iptables-1.4.21/extensions/*.so $(INSTALLDIR)/iptables-1.4.21/usr/lib/xtables
ifeq ($(RTCONFIG_IPV6),y)
	cd $(INSTALLDIR)/iptables-1.4.21/usr/sbin && \
		ln -sf xtables-multi ip6tables-restore && \
		ln -sf xtables-multi ip6tables
	install -D iptables-1.4.21/libiptc/.libs/libip6tc.so $(INSTALLDIR)/iptables-1.4.21/usr/lib/libip6tc.so
	cd $(INSTALLDIR)/iptables-1.4.21/usr/lib && \
		ln -sf libip6tc.so libip6tc.so.0 && \
		ln -sf libip6tc.so libip6tc.so.0.0.0
endif

iptables-1.4.x/configure:
	cd iptables-1.4.x && ./autogen.sh

iptables-1.4.x/Makefile: iptables-1.4.x/configure
ifeq ($(HND_ROUTER),y)
	cd iptables-1.4.x && $(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
		--enable-ipv6 \
		--with-kbuild=$(TOOLCHAIN_INCLUDE_DIR) \
		LDFLAGS=-L$(INSTALL_DIR)/lib \
		CFLAGS="-Os $(BRCM_FLAG) -U CONFIG_NVRAM_SIZE"
else ifneq ($(or $(findstring linux-3.10,$(LINUXDIR))),)
	cd iptables-1.4.x && $(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
		--with-kernel=$(KDIR)/include \
		$(if $(RTCONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
		CFLAGS="-Os $(EXTRACFLAGS) -U CONFIG_NVRAM_SIZEi -I$(KDIR)/include"
else
	cd iptables-1.4.x && $(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
		--with-kernel=$(LINUXDIR) \
		$(if $(RTCONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
		CFLAGS="-Os $(EXTRACFLAGS) -U CONFIG_NVRAM_SIZE"
endif

iptables-1.4.x: iptables-1.4.x/Makefile libnfnetlink
	@$(SEP)
	$(MAKE) -C $@ KERNEL_DIR=$(LINUX_INC_DIR) COPT_FLAGS="-Os $(EXTRACFLAGS) -U CONFIG_NVRAM_SIZE"
#		CFLAGS="-Wall -Os -D_GNU_SOURCE $(EXTRACFLAGS) -I$(TOP)/libnfnetlink/include" \
#		LDFLAGS="-L$(TOP)/libnfnetlink/src/.libs -lnfnetlink"

iptables-1.4.x-install:
	install -D iptables-1.4.x/iptables/.libs/xtables-multi $(INSTALLDIR)/iptables-1.4.x/usr/sbin/xtables-multi
	cd $(INSTALLDIR)/iptables-1.4.x/usr/sbin && \
		ln -sf xtables-multi iptables-restore && \
		ln -sf xtables-multi iptables-save && \
		ln -sf xtables-multi iptables
	install -d $(INSTALLDIR)/iptables-1.4.x/usr/lib/xtables
	install -D iptables-1.4.x/libiptc/.libs/libiptc.so $(INSTALLDIR)/iptables-1.4.x/usr/lib/libiptc.so
	install -D iptables-1.4.x/libiptc/.libs/libip4tc.so $(INSTALLDIR)/iptables-1.4.x/usr/lib/libip4tc.so
	cd $(INSTALLDIR)/iptables-1.4.x/usr/lib && \
		ln -sf libiptc.so libiptc.so.0 && \
		ln -sf libiptc.so libiptc.so.0.0.0 && \
		ln -sf libip4tc.so libip4tc.so.0 && \
		ln -sf libip4tc.so libip4tc.so.0.0.0
	install -D iptables-1.4.x/libxtables/.libs/lib*.so $(INSTALLDIR)/iptables-1.4.x/usr/lib/
	cd $(INSTALLDIR)/iptables-1.4.x/usr/lib && \
		ln -sf libxtables.so libxtables.so.7 && \
		ln -sf libxtables.so libxtables.so.7.0.0
	install -D iptables-1.4.x/extensions/*.so $(INSTALLDIR)/iptables-1.4.x/usr/lib/xtables

ifeq ($(RTCONFIG_IPV6),y)
	cd $(INSTALLDIR)/iptables-1.4.x/usr/sbin && \
		ln -sf xtables-multi ip6tables-restore && \
		ln -sf xtables-multi ip6tables
	install -D iptables-1.4.x/libiptc/.libs/libip6tc.so $(INSTALLDIR)/iptables-1.4.x/usr/lib/libip6tc.so
	cd $(INSTALLDIR)/iptables-1.4.x/usr/lib && \
		ln -sf libip6tc.so libip6tc.so.0 && \
		ln -sf libip6tc.so libip6tc.so.0.0.0
endif

	$(STRIP) $(INSTALLDIR)/iptables-1.4.x/usr/sbin/xtables-multi
	$(STRIP) $(INSTALLDIR)/iptables-1.4.x/usr/lib/*.so*
	$(STRIP) $(INSTALLDIR)/iptables-1.4.x/usr/lib/xtables/*.so*

iptables-1.4.x-clean:
	[ ! -f iptables-1.4.x/Makefile ] || $(MAKE) -C iptables-1.4.x KERNEL_DIR=$(LINUX_INC_DIR) distclean
	@rm -f iptables-1.4.x/Makefile

miniupnpd/config.h: shared/version.h
ifeq ($(RTCONFIG_IGD2),y)
	cd miniupnpd && ./genconfig.sh --vendorcfg --leasefile $(if $(RTCONFIG_IPV6),--ipv6,) --igd2 --portinuse
else
	cd miniupnpd && ./genconfig.sh --vendorcfg --leasefile --portinuse
endif

miniupnpd: $(IPTABLES) e2fsprogs miniupnpd/config.h
	@$(SEP)
	cp -f ./shared/version.h miniupnpd$(MUVER)/.
	PKG_CONFIG=false ARCH=$(PLATFORM) \
	$(MAKE) -C $@ -f Makefile.merlin $(PARALLEL_BUILD) \
	    IPTABLESPATH=$(TOP)/$(IPTABLES) \
	    EXTRACFLAGS="-Os $(EXTRACFLAGS) -idirafter$(KERNEL_HEADER_DIR) -ffunction-sections -fdata-sections -I$(TOP)/e2fsprogs/lib" \
	    LDFLAGS="$(EXTRALDFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(IPTC_LIBDIR) -L$(TOP)/e2fsprogs/lib" \
	    LDLIBS="-Wl,--as-needed $(IPTC_LIBS) -luuid"

miniupnpd-clean:
	-@$(MAKE) -C miniupnpd -f Makefile.merlin clean
	-@rm miniupnpd/config.h

miniupnpd_1.4: $(IPTABLES) shared/version.h
	@$(SEP)
	cp -f shared/version.h $@/
	$(MAKE) -C $@ -f Makefile.asus \
		IPTABLESPATH=$(TOP)/$(IPTABLES) \
		EXTRACFLAGS="-Os $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="$(EXTRALDFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(IPTC_LIBDIR)" \
		LDLIBS="-Wl,--as-needed $(IPTC_LIBS)"

miniupnpd_1.4-clean:
	-@$(MAKE) -C miniupnpd_1.4 -f Makefile.asus clean

miniupnpd$(MUVER)-install: miniupnpd$(MUVER)
	install -D miniupnpd$(MUVER)/miniupnpd $(INSTALLDIR)/miniupnpd$(MUVER)/usr/sbin/miniupnpd
	$(STRIP) $(INSTALLDIR)/miniupnpd$(MUVER)/usr/sbin/miniupnpd

miniupnpc:
	$(MAKE) -C miniupnpc

miniupnpc-clean:
	-@$(MAKE) -C miniupnpc clean

miniupnpc-install:
	install -D miniupnpc/upnpc-static $(INSTALLDIR)/miniupnpc/usr/sbin/miniupnpc
	$(STRIP) $(INSTALLDIR)/miniupnpc/usr/sbin/miniupnpc

# !!TB
#shared:
shared: $(if $(RTCONFIG_QTN),libqcsapi_client) $(if $(RTCONFIG_HTTPS),openssl)
# Dig into our collection of prebuilt objects based on model.
	@-rm -f shared/prebuild/*.o || true
	@-cp -f shared/prebuild/$(BUILD_NAME)/* shared/prebuild/
ifeq ($(RTCONFIG_RALINK)$(RTCONFIG_QCA)$(RTCONFIG_ALPINE)$(RTCONFIG_LANTIQ),y)
	make -C wireless_tools wireless.h
endif
	$(MAKE) -C shared

shared-install:
	$(MAKE) -C shared INSTALLDIR=$(INSTALLDIR)/shared install

ifeq ($(RTCONFIG_FTP_SSL),y)
ftp_dep=openssl
else
ftp_dep=
endif

vsftpd: nvram$(BCMEX)$(EX7) shared libdisk $(ftp_dep)
	@$(SEP)
ifeq ($(RTCONFIG_FTP_SSL),y)
	-cp vsftpd/builddefs-ssl.h vsftpd/builddefs.h
else
	-cp vsftpd/builddefs-nossl.h vsftpd/builddefs.h
endif
	$(MAKE) -C vsftpd

vsftpd-install:
	install -D vsftpd/vsftpd $(INSTALLDIR)/vsftpd/usr/sbin/vsftpd
	$(STRIP) -s $(INSTALLDIR)/vsftpd/usr/sbin/vsftpd

vsftpd-3.x: nvram$(BCMEX)$(EX7) shared libdisk $(ftp_dep)
	@$(SEP)
ifeq ($(RTCONFIG_FTP_SSL),y)
	-cp vsftpd-3.x/builddefs-ssl.h vsftpd-3.x/builddefs.h
else
	-cp vsftpd-3.x/builddefs-nossl.h vsftpd-3.x/builddefs.h
endif
	$(MAKE) -C $@

vsftpd-3.x-install:
	install -D vsftpd-3.x/vsftpd $(INSTALLDIR)/vsftpd-3.x/usr/sbin/vsftpd
	$(STRIP) -s $(INSTALLDIR)/vsftpd-3.x/usr/sbin/vsftpd

ntfs-3g/stamp-h1:
	cd ntfs-3g && \
	CC=$(CC) CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC -ldl" \
		$(CONFIGURE) --enable-shared=no --enable-static=no \
		--disable-library --disable-ldconfig --disable-mount-helper --with-fuse=internal \
		--disable-posix-acls --disable-nfconv --disable-dependency-tracking
	touch $@

ntfs-3g: ntfs-3g/stamp-h1
	@$(MAKE) -C ntfs-3g

ntfs-3g-clean:
	-@$(MAKE) -C ntfs-3g clean
	@rm -f ntfs-3g/stamp-h1

ntfs-3g-install: ntfs-3g
	install -D ntfs-3g/src/ntfs-3g $(INSTALLDIR)/ntfs-3g/bin/ntfs-3g
	$(STRIP) -s $(INSTALLDIR)/ntfs-3g/bin/ntfs-3g
	install -d $(INSTALLDIR)/ntfs-3g/sbin && cd $(INSTALLDIR)/ntfs-3g/sbin && \
		ln -sf ../bin/ntfs-3g mount.ntfs-3g && \
		ln -sf ../bin/ntfs-3g mount.ntfs

linux-ntfs-2.0.0/stamp-h1:
	cd linux-ntfs-2.0.0 && \
	$(CONFIGURE)
	touch $@

linux-ntfs-2.0.0: linux-ntfs-2.0.0/stamp-h1
	@$(MAKE) -C linux-ntfs-2.0.0 all

linux-ntfs-2.0.0-install:
	install -D linux-ntfs-2.0.0/libntfs/.libs/libntfs.so.10.0.0 $(INSTALLDIR)/linux-ntfs-2.0.0/usr/lib/libntfs.so.10.0.0
	$(STRIP) $(INSTALLDIR)/linux-ntfs-2.0.0/usr/lib/libntfs.so.10.0.0
	cd $(INSTALLDIR)/linux-ntfs-2.0.0/usr/lib && \
		ln -sf libntfs.so.10.0.0 libntfs.so.10 && \
		ln -sf libntfs.so.10.0.0 libntfs.so
	install -D linux-ntfs-2.0.0/ntfsprogs/.libs/ntfsfix $(INSTALLDIR)/linux-ntfs-2.0.0/usr/sbin/ntfsfix
	$(STRIP) $(INSTALLDIR)/linux-ntfs-2.0.0/usr/sbin/*

linux-ntfs-2.0.0-clean:
	-@$(MAKE) -C linux-ntfs-2.0.0 clean
	@rm -f linux-ntfs-2.0.0/stamp-h1

libupnp-1.3.1: libupnp-1.3.1/Makefile
	$(MAKE) -C $@

libupnp-1.3.1-install: libupnp-1.3.1
	@echo "do nothing"
#	install -d $(INSTALLDIR)/libupnp-1.3.1/usr/lib
#	install -m 775 libupnp-1.3.1/ixml/.libs/libixml.so.2.0.0 $(INSTALLDIR)/libupnp-1.3.1/usr/lib/libixml.so.2.0.0
#	install -m 775 libupnp-1.3.1/threadutil/.libs/libthreadutil.so.2.0.0 $(INSTALLDIR)/libupnp-1.3.1/usr/lib/libthreadutil.so.2.0.0
#	install -m 775 libupnp-1.3.1/upnp/.libs/libupnp.so.2.0.1 $(INSTALLDIR)/libupnp-1.3.1/usr/lib/libupnp.so.2.0.1
#	$(STRIP) $(INSTALLDIR)/libupnp-1.3.1/usr/lib/*.so.*
#	cd $(INSTALLDIR)/libupnp-1.3.1/usr/lib && \
#		ln -sf libixml.so.2.0.0 libixml.so.2 && \
#		ln -sf libixml.so.2.0.0 libixml.so
#	cd $(INSTALLDIR)/libupnp-1.3.1/usr/lib && \
#		ln -sf libthreadutil.so.2.0.0 libthreadutil.so.2 && \
#		ln -sf libthreadutil.so.2.0.0 libthreadutil.so
#	cd $(INSTALLDIR)/libupnp-1.3.1/usr/lib && \
#		ln -sf libupnp.so.2.0.1 libupnp.so.2 && \
#		ln -sf libupnp.so.2.0.1 libupnp.so

libupnp-1.3.1/Makefile: libupnp-1.3.1/Makefile.in
	( cd libupnp-1.3.1; \
		config.aux/missing aclocal; \
		config.aux/missing automake; \
		config.aux/missing autoconf; \
		$(CONFIGURE) --prefix=/usr --disable-dependency-tracking \
	)

libusb10/stamp-h1:
	cd libusb10 && CFLAGS="-Os -Wall $(EXTRACFLAGS)" LIBS="-lpthread -ldl -lc $(EXTRALDFLAGS)" \
	$(CONFIGURE) --enable-shared --prefix=/usr ac_cv_lib_rt_clock_gettime=no
	-@$(MAKE) -C libusb10 clean
	touch $@

libusb10: libusb10/stamp-h1
	$(MAKE) -C $@

libusb10-install: libusb10
	install -D libusb10/libusb/.libs/libusb-1.0.so.0.0.0 $(INSTALLDIR)/libusb10/usr/lib/libusb-1.0.so.0
	$(STRIP) $(INSTALLDIR)/libusb10/usr/lib/*.so.*
	cd $(INSTALLDIR)/libusb10/usr/lib && \
		ln -sf libusb-1.0.so.0 libusb-1.0.so.0.0.0 && \
		ln -sf libusb-1.0.so.0 libusb-1.0.so

libusb10-clean:
	-@$(MAKE) -C libusb10 clean
	@rm -f libusb10/stamp-h1

libusb/stamp-h1:
	cd libusb && CFLAGS="-Wall -Os $(EXTRACFLAGS)" LDFLAGS="$(EXTRALDFLAGS)" \
	$(CONFIGURE) --prefix=/usr \
		LIBUSB_1_0_CFLAGS="-I$(TOP)/libusb10/libusb" \
		LIBUSB_1_0_LIBS="-L$(TOP)/libusb10/libusb/.libs -lusb-1.0 -lpthread -ldl $(EXTRALDFLAGS)\
		-Wl,-R/lib:/usr/lib:/opt/usr/lib:/usr/local/share"
	-@$(MAKE) -C libusb clean
	touch $@

libusb: libusb10 libusb/stamp-h1
	$(MAKE) -C $@

libusb-install: libusb
	install -D libusb/libusb/.libs/libusb-0.1.so.4.4.4 $(INSTALLDIR)/libusb/usr/lib/libusb-0.1.so.4
	$(STRIP) $(INSTALLDIR)/libusb/usr/lib/*.so.*
	cd $(INSTALLDIR)/libusb/usr/lib && \
		ln -sf libusb-0.1.so.4 libusb-0.1.so.4.4.4 && \
		ln -sf libusb-0.1.so.4 libusb.so

libusb-clean:
	-@$(MAKE) -C libusb clean
	@rm -f libusb/stamp-h1

usb-modeswitch-1.2.3: libusb-0.1.12
	$(MAKE) -C $@ CC=$(CC) CFLAGS="-Os $(EXTRACFLAGS) -I$(TOP)/libusb-0.1.12" LIBS="\
		-Wl,-R/lib:/usr/lib:/opt/usr/lib:/usr/local/share -lpthread -ldl\
		-L$(TOP)/libusb-0.1.12/.libs -lusb"

usb-modeswitch-1.2.3-install: usb-modeswitch-1.2.3
	install -D usb-modeswitch-1.2.3/usb_modeswitch $(INSTALLDIR)/usb-modeswitch-1.2.3/usr/sbin/usb_modeswitch
	$(STRIP) -s $(INSTALLDIR)/usb-modeswitch-1.2.3/usr/sbin/usb_modeswitch
	@mkdir -p $(TARGETDIR)/rom/etc
	@sed 's/#.*//g;s/[ \t]\+/ /g;s/^[ \t]*//;s/[ \t]*$$//;/^$$/d' < $(TOP)/usb-modeswitch-1.2.3/usb_modeswitch.conf > $(TARGETDIR)/rom/etc/usb_modeswitch.conf

usb-modeswitch: libusb10
	$(MAKE) -C $@ CC=$(CC) CFLAGS="-Os $(EXTRACFLAGS) -I$(TOP)/libusb10/libusb" LIBS="\
		-Wl,-R/lib:/usr/lib:/opt/usr/lib:/usr/local/share -lpthread -ldl \
		-L$(TOP)/libusb10/libusb/.libs -lusb-1.0"

usb-modeswitch-install: usb-modeswitch
	install -D usb-modeswitch/usb_modeswitch $(INSTALLDIR)/usb-modeswitch/usr/sbin/usb_modeswitch
	$(STRIP) -s $(INSTALLDIR)/usb-modeswitch/usr/sbin/usb_modeswitch
	@mkdir -p $(TARGETDIR)/rom/$(EX_ROM)/etc
	@sed 's/#.*//g;s/[ \t]\+/ /g;s/^[ \t]*//;s/[ \t]*$$//;/^$$/d' < $(TOP)/usb-modeswitch/usb_modeswitch.conf > $(TARGETDIR)/rom/$(EX_ROM)/etc/usb_modeswitch.conf

usbmode/stamp-h1:
	@rm -f usbmode/crosscompiled.cmake
	$(call CMAKE_CrossOptions, usbmode/crosscompiled.cmake)
	cd usbmode && cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake .
	touch $@

usbmode: json-c libubox usbmode/stamp-h1
	$(MAKE) -C $@ EXTRACFLAGS="$(EXTRACFLAGS) -I$(TOP)/libubox"

usbmode-install: usbmode
	install -D $</$< $(INSTALLDIR)/$</usr/sbin/$<
	$(STRIP) $(INSTALLDIR)/$</usr/sbin/$<

usbmode-clean:
	[ ! -f usbmode/Makefile ] || $(MAKE) -C usbmode clean
	cd usbmode && rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake crosscompiled.cmake Makefile stamp-h1

libusb-0.1.12/stamp-h1:
	cd libusb-0.1.12 && CFLAGS="-Os -Wall $(EXTRACFLAGS)" LIBS="-lpthread -ldl" \
	$(CONFIGURE) --prefix=/usr --disable-build-docs --disable-dependency-tracking
	-@$(MAKE) -C libusb-0.1.12 clean
	touch $@

libusb-0.1.12: libusb-0.1.12/stamp-h1
	$(MAKE) -C $@

libusb-0.1.12-install: libusb-0.1.12
#	install -D libusb-0.1.12/.libs/libusb-0.1.so.4.4.4 $(INSTALLDIR)/libusb-0.1.12/usr/lib/libusb-0.1.so.4.4.4
#	$(STRIP) $(INSTALLDIR)/libusb-0.1.12/usr/lib/*.so.*
#	cd $(INSTALLDIR)/libusb-0.1.12/usr/lib && \
#	ln -sf libusb-0.1.so.4.4.4 libusb-0.1.so.4 && \
#	ln -sf libusb-0.1.so.4.4.4 libusb.so
	@echo "do nothing"

libusb-0.1.12-clean:
	-@$(MAKE) -C libusb-0.1.12 clean
	@rm -f libusb-0.1.12/stamp-h1

libubox: json-c
	$(MAKE) -C $@ EXTRACFLAGS="$(EXTRACFLAGS)" JSONC=$(TOP)/json-c

libubox-install: libubox
	install -D $</libubox.so $(INSTALLDIR)/$</usr/lib/libubox.so
	install -D $</libblobmsg_json.so $(INSTALLDIR)/$</usr/lib/libblobmsg_json.so
	$(STRIP) $(INSTALLDIR)/$</usr/lib/*

uqmi: json-c libubox
	$(MAKE) -C $@ EXTRACFLAGS="$(EXTRACFLAGS)" LIBUBOX=$(TOP)/libubox JSONC=$(TOP)/json-c

uqmi-install: uqmi
	install -D $</$< $(INSTALLDIR)/$</usr/sbin/$<
	$(STRIP) $(INSTALLDIR)/$</usr/sbin/$<

libdaemon/stamp-h1:
	cd libdaemon && autoreconf -i -f && $(CONFIGURE) --prefix=/usr \
	--disable-dependency-tracking \
	ac_cv_func_setpgrp_void=yes
	touch $@

libdaemon: libdaemon/stamp-h1
	$(MAKE) -C $@ && $(MAKE) $@-stage

libdaemon-install: libdaemon
	install -D libdaemon/libdaemon/.libs/libdaemon.so.0.5.0 $(INSTALLDIR)/libdaemon/usr/lib/libdaemon.so.0.5.0
	$(STRIP) $(INSTALLDIR)/libdaemon/usr/lib/*.so.*
	cd $(INSTALLDIR)/libdaemon/usr/lib && \
		ln -sf libdaemon.so.0.5.0 libdaemon.so && \
		ln -sf libdaemon.so.0.5.0 libdaemon.so.0

libdaemon-clean:
	-@$(MAKE) -C libdaemon distclean
	@rm -f libdaemon/stamp-h1

lsof: lsof/Makefile
	@$(SEP)
	$(MAKE) -C $@

lsof/Makefile:
	( cd lsof ; \
		LSOF_CC=$(CC) \
		LSOF_INCLUDE=$(TOOLCHAIN)/include \
		LSOF_VSTR="asuswrt" \
		./Configure -n linux \
	)

lsof-install:
	install -D lsof/lsof $(INSTALLDIR)/lsof/usr/sbin/lsof
	$(STRIP) $(INSTALLDIR)/lsof/usr/sbin/lsof

lsof-clean:
	( cd lsof ; ./Configure -clean )
	@rm -f lsof/Makefile

mtd-utils: e2fsprogs lzo zlib
ifeq ($(ALPINE)$(LANTIQ),y)
	$(MAKE) CPPFLAGS="-I$(STAGEDIR)/usr/include -DWITHOUT_XATTR" \
		CFLAGS="-I$(TOP)/mtd-utils/include -I$(TOP)/mtd-utils/ubi-utils/include" \
		LDFLAGS="-L$(STAGEDIR)/usr/lib" \
		WITHOUT_XATTR=1 -C $@
else
	$(MAKE) CPPFLAGS="-I$(STAGEDIR)/usr/include" \
		CFLAGS="-I$(TOP)/mtd-utils/include -I$(TOP)/mtd-utils/ubi-utils/include" \
		LDFLAGS="-L$(STAGEDIR)/usr/lib" \
		WITHOUT_XATTR=1 -C $@
endif

mtd-utils-install:
	$(MAKE) WITHOUT_XATTR=1 WITHOUT_LZO=1 DESTDIR=$(INSTALLDIR)/mtd-utils -C mtd-utils install

mtd-utils-clean:
	$(MAKE) -C mtd-utils clean

util-linux: util-linux/Makefile
	$(MAKE) -C $@/libuuid/src && $(MAKE) $@-stage

util-linux/Makefile: util-linux/configure
	$(MAKE) util-linux-configure

util-linux/configure:
	( cd util-linux ; ./autogen.sh )

util-linux-configure:
	( export scanf_cv_alloc_modifier=no; \
		cd util-linux ; \
		$(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
		--enable-libuuid \
		--disable-nls --disable-tls --disable-libblkid --disable-mount --disable-libmount \
		--disable-fsck --disable-cramfs --disable-partx --disable-uuidd --disable-mountpoint \
		--disable-fallocate --disable-unshare --disable-agetty \
		--disable-cramfs --disable-switch_root --disable-pivot_root \
		--disable-kill --disable-rename --disable-chsh-only-listed \
		--disable-schedutils --disable-wall --disable-pg-bell --disable-require-password \
		--disable-use-tty-group --disable-makeinstall-chown --disable-makeinstall-setuid\
		--without-ncurses --without-selinux --without-audit \
	)

util-linux-stage:
	$(MAKE) -C util-linux/libuuid/src DESTDIR=$(STAGEDIR) \
		install-usrlib_execLTLIBRARIES install-uuidincHEADERS

util-linux-install: util-linux
	install -D $(STAGEDIR)/usr/lib/libuuid.so.1 $(INSTALLDIR)/util-linux/usr/lib/libuuid.so.1
	$(STRIP) $(INSTALLDIR)/util-linux/usr/lib/*.so*

util-linux-clean:
	[ ! -f util-linux/Makefile ] || $(MAKE) -C util-linux distclean
	@rm -f util-linux/Makefile

odhcp6c: odhcp6c/Makefile
	@EXT_CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
	EXT_LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
	PREFIX="/usr" \
	$(MAKE) -C odhcp6c

odhcp6c-install: odhcp6c
	install -D odhcp6c/src/odhcp6c $(INSTALLDIR)/odhcp6c/usr/sbin/odhcp6c
	$(STRIP) $(INSTALLDIR)/odhcp6c/usr/sbin/odhcp6c

odhcp6c-clean:
	-@$(MAKE) -C odhcp6c clean

6relayd: 6relayd/Makefile
	@EXT_CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
	EXT_LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
	PREFIX="/usr" \
	$(MAKE) -C 6relayd

6relayd-install: 6relayd
	install -D 6relayd/src/6relayd $(INSTALLDIR)/6relayd/usr/sbin/6relayd
	$(STRIP) $(INSTALLDIR)/6relayd/usr/sbin/6relayd

6relayd-clean:
	-@$(MAKE) -C 6relayd clean

e2fsprogs: e2fsprogs/Makefile
	$(MAKE) -C e2fsprogs $(PARALLEL_BUILD) && $(MAKE) $@-stage

e2fsprogs/Makefile:
	$(MAKE) e2fsprogs-configure

e2fsprogs-configure: e2fsprogs/Makefile.in
	cd e2fsprogs && \
	CFLAGS="-Os -ffunction-sections -fdata-sections $(EXTRACFLAGS)" \
	LDFLAGS="-Wl,--gc-sections $(EXTRALDFLAGS)" \
	$(CONFIGURE) --prefix=/usr --sysconfdir=/etc --enable-elf-shlibs \
		--disable-rpath --disable-nls --disable-tls \
		--disable-jbd-debug --disable-blkid-debug --disable-testio-debug \
		--enable-libuuid --enable-libblkid \
		--disable-backtrace --disable-debugfs \
		--disable-imager --disable-resizer --disable-defrag \
		--disable-e2initrd-helper --disable-uuidd \
		$(if $(RTCONFIG_BCMARM),ac_cv_lib_pthread_sem_init=no,) \
		ac_cv_lib_dl_dlopen=no \
		ac_cv_have_decl_fgets_unlocked=yes

e2fsprogs-stage:
	$(MAKE) -C e2fsprogs/lib/uuid DESTDIR=$(STAGEDIR) LDCONFIG=true install

e2fsprogs-install:
# libs
	install -D e2fsprogs/lib/libuuid.so.1.2 $(INSTALLDIR)/e2fsprogs/usr/lib/libuuid.so.1.2
	cd $(INSTALLDIR)/e2fsprogs/usr/lib && \
		ln -sf libuuid.so.1.2 libuuid.so.1 && ln -sf libuuid.so.1.2 libuuid.so
ifneq (,$(filter y,$(RTCONFIG_E2FSPROGS) $(RTCONFIG_EXT4FS)))
	install -D e2fsprogs/lib/libblkid.so.1.0 $(INSTALLDIR)/e2fsprogs/usr/lib/libblkid.so.1.0
	install -D e2fsprogs/lib/libcom_err.so.2.1 $(INSTALLDIR)/e2fsprogs/usr/lib/libcom_err.so.2.1
	install -D e2fsprogs/lib/libe2p.so.2.3 $(INSTALLDIR)/e2fsprogs/usr/lib/libe2p.so.2.3
	install -D e2fsprogs/lib/libext2fs.so.2.4 $(INSTALLDIR)/e2fsprogs/usr/lib/libext2fs.so.2.4
	cd $(INSTALLDIR)/e2fsprogs/usr/lib && \
		ln -sf libblkid.so.1.0 libblkid.so.1 && ln -sf libblkid.so.1.0 libblkid.so && \
		ln -sf libcom_err.so.2.1 libcom_err.so.2 && ln -sf libcom_err.so.2.1 libcom_err.so && \
		ln -sf libe2p.so.2.3 libe2p.so.2 && ln -sf libe2p.so.2.3 libe2p.so && \
		ln -sf libext2fs.so.2.4 libext2fs.so.2 && ln -sf libext2fs.so.2.4 libext2fs.so
endif
	$(STRIP) $(INSTALLDIR)/e2fsprogs/usr/lib/*.so.*
# apps
ifneq (,$(filter y,$(RTCONFIG_E2FSPROGS) $(RTCONFIG_EXT4FS)))
ifeq ($(RTCONFIG_E2FSPROGS),y)
	install -D e2fsprogs/e2fsck/e2fsck $(INSTALLDIR)/e2fsprogs/usr/sbin/e2fsck
	ln -sf e2fsck $(INSTALLDIR)/e2fsprogs/usr/sbin/fsck.ext2
	ln -sf e2fsck $(INSTALLDIR)/e2fsprogs/usr/sbin/fsck.ext3
ifeq ($(RTCONFIG_EXT4FS),y)
	ln -sf e2fsck $(INSTALLDIR)/e2fsprogs/usr/sbin/fsck.ext4
endif
ifneq ($(NO_MKTOOLS),y)
	install -D e2fsprogs/misc/tune2fs $(INSTALLDIR)/e2fsprogs/usr/sbin/tune2fs
	install -D e2fsprogs/misc/mke2fs $(INSTALLDIR)/e2fsprogs/usr/sbin/mke2fs
	ln -sf mke2fs $(INSTALLDIR)/e2fsprogs/usr/sbin/mkfs.ext2
	ln -sf mke2fs $(INSTALLDIR)/e2fsprogs/usr/sbin/mkfs.ext3
ifeq ($(RTCONFIG_EXT4FS),y)
	ln -sf mke2fs $(INSTALLDIR)/e2fsprogs/usr/sbin/mkfs.ext4
endif
endif
else ifeq ($(RTCONFIG_EXT4FS),y)
	install -D e2fsprogs/e2fsck/e2fsck $(INSTALLDIR)/e2fsprogs/usr/sbin/fsck.ext4
ifneq ($(NO_MKTOOLS),y)
	install -D e2fsprogs/misc/mke2fs $(INSTALLDIR)/e2fsprogs/usr/sbin/mkfs.ext4
endif
endif
	$(STRIP) $(INSTALLDIR)/e2fsprogs/usr/sbin/*
	install -D -m 0644 e2fsprogs/e2fsck/e2fsck.conf $(INSTALLDIR)/e2fsprogs/rom/etc/e2fsck.conf
endif

e2fsprogs-clean:
	[ ! -f e2fsprogs/Makefile ] || $(MAKE) -C e2fsprogs clean
	@rm -f e2fsprogs/Makefile

ecmh/stamp-h1:
	@touch ecmh/src/stamp-h1

ecmh: ecmh/stamp-h1
	@$(MAKE) -C ecmh CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD)

ecmh-clean:
	-@$(MAKE) -C ecmh clean
	@rm -f ecmh/src/stamp-h1

ecmh-install: ecmh
	install -D ecmh/src/ecmh $(INSTALLDIR)/ecmh/bin/ecmh
	$(STRIP) $(INSTALLDIR)/ecmh/bin/ecmh

p910nd:
samba:
#samba3:
#samba-3.5.8:

#samba-3.5.8-install:
#	install -D samba-3.5.8/source3/bin/libsmbclient.so $(INSTALLDIR)/samba-3.5.8/usr/lib/libsmbclient.so
#	$(STRIP) $(INSTALLDIR)/samba-3.5.8/usr/lib/libsmbclient.so

ifeq ($(HND_ROUTER),y)
nvram_arm_94908hnd: wlcsm
	$(MAKE) -C wlan
else
nvram$(BCMEX)$(EX7): shared

nvram$(BCMEX)$(EX7)-install:
	$(MAKE) -C nvram$(BCMEX)$(EX7) INSTALLDIR=$(INSTALLDIR)/nvram$(BCMEX)$(EX7) install
endif

prebuilt: shared

vlan:
	@$(SEP)
	@$(MAKE) -C vlan CROSS=$(CROSS_COMPILE)	# STRIPTOOL=$(STRIP)

vlan-install:
	$(MAKE) -C vlan CROSS=$(CROSS_COMPILE) INSTALLDIR=$(INSTALLDIR)/vlan install	# STRIPTOOL=$(STRIP)
	$(STRIP) $(INSTALLDIR)/vlan/usr/sbin/vconfig

accel-pptpd: kernel_header pppd accel-pptpd/pptpd-1.3.3/Makefile
	@$(MAKE) -C accel-pptpd server

accel-pptpd/pptpd-1.3.3/Makefile:
	( cd accel-pptpd/pptpd-1.3.3 && CFLAGS="$(CFLAGS) -g -O2 $(EXTRACFLAGS) -idirafter $(KDIR)/include" \
		$(CONFIGURE) --prefix=/usr --bindir=/usr/sbin --libdir=/usr/lib \
			--enable-bcrelay KDIR=$(KDIR) PPPDIR=$(TOP)/pppd \
	)

accel-pptpd-clean:
	[ ! -f accel-pptpd/pptpd-1.3.3/Makefile ] || $(MAKE) -C accel-pptpd distclean
	@rm -f accel-pptpd/pptpd-1.3.3/Makefile

accel-pptpd-install: accel-pptpd
	$(MAKE) -C accel-pptpd server_install

pptpd/stamp-h1:
	cd $(TOP)/pptpd && $(CONFIGURE) --prefix=/usr --enable-bcrelay CC=$(CC) STRIP=$(STRIP) AR=$(AR) LD=$(LD) NM=$(NM) RANLIB=$(RANLIB)
	touch $@

pptpd: pptpd/stamp-h1

pptpd-install: pptpd
	@echo pptpd
	@install -D pptpd/pptpd $(INSTALLDIR)/pptpd/usr/sbin/pptpd
	@install -D pptpd/bcrelay $(INSTALLDIR)/pptpd/usr/sbin/bcrelay
	@install -D pptpd/pptpctrl $(INSTALLDIR)/pptpd/usr/sbin/pptpctrl
	@$(STRIP) $(INSTALLDIR)/pptpd/usr/sbin/pptpd
	@$(STRIP) $(INSTALLDIR)/pptpd/usr/sbin/bcrelay
	@$(STRIP) $(INSTALLDIR)/pptpd/usr/sbin/pptpctrl

pptpd-clean:
	-@make -C pptpd clean
	@rm -f pptpd/stamp-h1

rp-pppoe/src/stamp-h1:
ifeq ($(HND_ROUTER),y)
	cd rp-pppoe/src && CFLAGS="-g -O2 -I $(TOP)/pppd -idirafter $(KDIR)/include" \
	$(CONFIGURE) --prefix=/usr --enable-plugin=$(TOP)/pppd \
	ac_cv_linux_kernel_pppoe=yes rpppoe_cv_pack_bitfields=rev \
	PPPD_INCDIR=$(TOOLCHAIN)/include \
	ac_cv_header_linux_if_pppox_h=yes \
	ac_cv_pluginpath=$(TOOLCHAIN)/include
	touch $@
else
	cd rp-pppoe/src && CFLAGS="-g -O2 $(EXTRACFLAGS) -idirafter $(KDIR)/include" \
	$(CONFIGURE) --prefix=/usr --enable-plugin=$(TOP)/pppd \
	ac_cv_linux_kernel_pppoe=yes rpppoe_cv_pack_bitfields=rev --disable-debugging
	touch $@
endif

rp-pppoe: pppd rp-pppoe/src/stamp-h1
	@$(MAKE) -C rp-pppoe/src pppoe-relay rp-pppoe.so

rp-pppoe-clean:
	-@$(MAKE) -C rp-pppoe/src clean
	@rm -f rp-pppoe/src/stamp-h1

rp-pppoe-install: rp-pppoe
	install -D rp-pppoe/src/rp-pppoe.so $(INSTALLDIR)/rp-pppoe/usr/lib/pppd/rp-pppoe.so
	$(STRIP) $(INSTALLDIR)/rp-pppoe/usr/lib/pppd/*.so
	install -D rp-pppoe/src/pppoe-relay $(INSTALLDIR)/rp-pppoe/usr/sbin/pppoe-relay
	$(STRIP) $(INSTALLDIR)/rp-pppoe/usr/sbin/pppoe-relay

rp-l2tp/stamp-h1: kernel_header
	cd rp-l2tp && CFLAGS="-O2 -Wall $(EXTRACFLAGS) -idirafter $(KDIR)/include $(if $(RTCONFIG_VPNC), -DRTCONFIG_VPNC,)" \
	$(CONFIGURE) --prefix=/usr --sysconfdir=/tmp
	touch $@

rp-l2tp: pppd rp-l2tp/stamp-h1
	$(MAKE) -C rp-l2tp

rp-l2tp-clean:
	-@$(MAKE) -C rp-l2tp distclean
	@rm -f rp-l2tp/stamp-h1

rp-l2tp-install:
	install -d $(INSTALLDIR)/rp-l2tp/usr/lib/l2tp
	install rp-l2tp/handlers/*.so $(INSTALLDIR)/rp-l2tp/usr/lib/l2tp
	$(STRIP) $(INSTALLDIR)/rp-l2tp/usr/lib/l2tp/*.so
	install -D rp-l2tp/handlers/l2tp-control $(INSTALLDIR)/rp-l2tp/usr/sbin/l2tp-control
	$(STRIP) $(INSTALLDIR)/rp-l2tp/usr/sbin/l2tp-control
	install -D rp-l2tp/l2tpd $(INSTALLDIR)/rp-l2tp/usr/sbin/l2tpd
	$(STRIP) $(INSTALLDIR)/rp-l2tp/usr/sbin/l2tpd

xl2tpd: pppd
	CFLAGS="-g $(EXTRACFLAGS)" $(MAKE) -C $@ PREFIX=/usr xl2tpd

xl2tpd-install: xl2tpd
	install -D xl2tpd/xl2tpd $(INSTALLDIR)/xl2tpd/usr/sbin/xl2tpd
	$(STRIP) $(INSTALLDIR)/xl2tpd/usr/sbin/xl2tpd

pptp-client-install:
	install -D pptp-client/pptp $(INSTALLDIR)/pptp-client/usr/sbin/pptp
	$(STRIP) $(INSTALLDIR)/pptp-client/usr/sbin/pptp

ifeq ($(RTCONFIG_REALTEK),y)
accel-pptp/linux/version.h:
	$(MAKE) -C $(LINUXDIR) include/linux/version.h
endif

ifeq ($(RTCONFIG_REALTEK),y)
accel-pptp/stamp-h1: kernel_header $(if $(RTCONFIG_REALTEK), accel-pptp/linux/version.h, $(LINUXDIR)/include/linux/version.h)
else
ifeq ($(RTCONFIG_ALPINE)$(RTCONFIG_LANTIQ),y)
accel-pptp/stamp-h1: kernel_header $(LINUXDIR)/include/generated/uapi/linux/version.h
else
accel-pptp/stamp-h1: kernel_header $(LINUX_INC_DIR)/include/linux/version.h
endif
endif
	cd accel-pptp && CFLAGS="-g -O2 $(EXTRACFLAGS) -idirafter $(KDIR)/include $(if $(RTCONFIG_VPNC), -DRTCONFIG_VPNC,)" \
	$(CONFIGURE) --prefix=/usr --build=i686-linux KDIR=$(KDIR) PPPDIR=$(TOP)/pppd 
	touch $@

accel-pptp: pppd accel-pptp/stamp-h1
	@$(MAKE) -C accel-pptp

accel-pptp-clean:
	-@$(MAKE) -C accel-pptp clean
	@rm -f accel-pptp/stamp-h1

accel-pptp-install: accel-pptp
	install -D accel-pptp/src/.libs/pptp.so $(INSTALLDIR)/accel-pptp/usr/lib/pppd/pptp.so
	$(STRIP) $(INSTALLDIR)/accel-pptp/usr/lib/pppd/pptp.so

pppd/stamp-h1: kernel_header
	cd pppd && $(CONFIGURE) --prefix=/usr --sysconfdir=/tmp
	touch $@

pppd: pppd/stamp-h1
	@$(SEP)
	@$(MAKE) -C pppd MFLAGS='$(if $(RTCONFIG_IPV6),HAVE_INET6=y,) \
	EXTRACFLAGS="$(EXTRACFLAGS) -idirafter $(KDIR)/include $(if $(RTCONFIG_VPNC),-DRTCONFIG_VPNC,)"'

pppd-clean:
	-@$(MAKE) -C pppd clean
	@rm -f pppd/stamp-h1

pppd-install: pppd
	install -D pppd/pppd/pppd $(INSTALLDIR)/pppd/usr/sbin/pppd
	$(STRIP) $(INSTALLDIR)/pppd/usr/sbin/pppd
	install -D pppd/chat/chat $(INSTALLDIR)/pppd/usr/sbin/chat
	$(STRIP) $(INSTALLDIR)/pppd/usr/sbin/chat
ifeq ($(RTCONFIG_L2TP),y)
	install -D pppd/pppd/plugins/pppol2tp/pppol2tp.so $(INSTALLDIR)/pppd/usr/lib/pppd/pppol2tp.so
	$(STRIP) $(INSTALLDIR)/pppd/usr/lib/pppd/*.so
endif

ez-ipupdate-install:
	install -D ez-ipupdate/ez-ipupdate $(INSTALLDIR)/ez-ipupdate/usr/sbin/ez-ipupdate
	$(STRIP) $(INSTALLDIR)/ez-ipupdate/usr/sbin/ez-ipupdate

quagga/stamp-h1:
	@cd quagga && \
	CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) \
	CFLAGS="$(CFLAGS) -Os -Wall -ffunction-sections -fdata-sections -fPIC" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
	$(CONFIGURE) --enable-shared=lib --disable-ripngd --disable-ospfd --disable-doc --disable-ospfclient \
	--disable-ospf6d --disable-bgpd --disable-bgp-announce --disable-babeld \
	--disable-silent-rules \
	--disable-isisd
	@touch $@

quagga: quagga/stamp-h1
	@$(MAKE) -C quagga CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) $(PARALLEL_BUILD)

quagga-clean:
	-@$(MAKE) -C quagga clean
	@rm -f quagga/stamp-h1

quagga-install:
	install -d $(INSTALLDIR)/quagga/usr/lib
	libtool --mode=install install -c quagga/lib/libzebra.la $(INSTALLDIR)/quagga/usr/lib
	libtool --finish $(INSTALLDIR)/quagga/usr/lib
ifneq ($(HND_ROUTER),y)
	rm -f $(INSTALLDIR)/quagga/usr/lib/libzebra.a
	rm -f $(INSTALLDIR)/quagga/usr/lib/libzebra.la
endif
	install -d $(INSTALLDIR)/quagga/usr/sbin
	install -d $(INSTALLDIR)/quagga/usr/local/etc
	libtool --mode=install install -c quagga/zebra/zebra $(INSTALLDIR)/quagga/usr/sbin
	install -c -m 777 quagga/zebra/zebra.conf.sample $(INSTALLDIR)/quagga/usr/local/etc/zebra.conf
	libtool --mode=install install -c quagga/ripd/ripd $(INSTALLDIR)/quagga/usr/sbin
	install -c -m 777 quagga/ripd/ripd.conf.sample $(INSTALLDIR)/quagga/usr/local/etc/ripd.conf
	libtool --mode=install install -c quagga/watchquagga/watchquagga $(INSTALLDIR)/quagga/usr/sbin
	$(STRIP) $(INSTALLDIR)/quagga/usr/sbin/zebra
	$(STRIP) $(INSTALLDIR)/quagga/usr/sbin/ripd
	$(STRIP) $(INSTALLDIR)/quagga/usr/sbin/watchquagga
ifneq ($(HND_ROUTER),y)
	install -D $(shell dirname $(shell which $(CXX)))/../lib/librt.so.0 $(TARGETDIR)/lib/librt.so.0
else
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/librt-2.22.so $(INSTALLDIR)/lib/librt.so.1
endif

zebra/stamp-h1:
	@cd zebra && rm -f config.cache && \
	CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) \
	CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
	$(CONFIGURE) --sysconfdir=/etc \
	--enable-netlink $(if $(RTCONFIG_IPV6),--enable-ipv6,--disable-ipv6) --disable-ripngd --disable-ospfd --disable-doc \
	--disable-ospf6d --disable-bgpd --disable-bgpd-announce
	@touch $@

zebra: zebra/stamp-h1
	@$(MAKE) -C zebra CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD)

zebra-clean:
	-@$(MAKE) -C zebra clean
	@rm -f zebra/stamp-h1

zebra-install: zebra
	install -D zebra/zebra/zebra $(INSTALLDIR)/zebra/usr/sbin/zebra
	install -D zebra/ripd/ripd $(INSTALLDIR)/zebra/usr/sbin/ripd
	install -D zebra/lib/libzebra.so $(INSTALLDIR)/zebra/usr/lib/libzebra.so
	$(STRIP) $(INSTALLDIR)/zebra/usr/sbin/zebra
	$(STRIP) $(INSTALLDIR)/zebra/usr/sbin/ripd
	$(STRIP) $(INSTALLDIR)/zebra/usr/lib/libzebra.so

bpalogin-install:
	install -D bpalogin/bpalogin $(INSTALLDIR)/bpalogin/usr/sbin/bpalogin
	$(STRIP) $(INSTALLDIR)/bpalogin/usr/sbin/bpalogin

wpa_supplicant-0.7.3/stamp-h1:
	touch $@

wpa_supplicant-0.7.3: $@/stamp-h1
	$(MAKE) -C $@/src/eap_peer

wpa_supplicant-0.7.3-install: wpa_supplicant-0.7.3
	install -D wpa_supplicant-0.7.3/src/eap_peer/libeap.so.0.0.0 $(INSTALLDIR)/wpa_supplicant-0.7.3/usr/lib/libeap.so.0.0.0
	$(STRIP) $(INSTALLDIR)/wpa_supplicant-0.7.3/usr/lib/libeap.so.0.0.0
	cd $(INSTALLDIR)/wpa_supplicant-0.7.3/usr/lib && \
	ln -sf libeap.so.0.0.0 $(INSTALLDIR)/wpa_supplicant-0.7.3/usr/lib/libeap.so.0
	ln -sf libeap.so.0.0.0 $(INSTALLDIR)/wpa_supplicant-0.7.3/usr/lib/libeap.so

wpa_supplicant-0.7.3-clean:
	-@$(MAKE) -C wpa_supplicant-0.7.3/src/eap_peer clean
	@rm -f wpa_supplicant-0.7.3/stamp-h1

gctwimax-0.0.3rc4/stamp-h1:
	touch $@

gctwimax-0.0.3rc4: $@/stamp-h1
	$(MAKE) -C $@

gctwimax-0.0.3rc4-install: gctwimax-0.0.3rc4
	install -D gctwimax-0.0.3rc4/gctwimax $(INSTALLDIR)/gctwimax-0.0.3rc4/usr/sbin/gctwimax
	$(STRIP) $(INSTALLDIR)/gctwimax-0.0.3rc4/usr/sbin/gctwimax
	#install -D gctwimax-0.0.3rc4/src/event.sh $(INSTALLDIR)/gctwimax-0.0.3rc4/usr/share/event.sh
	#install -D gctwimax-0.0.3rc4/src/gctwimax.conf $(INSTALLDIR)/gctwimax-0.0.3rc4/usr/share/gctwimax.conf

gctwimax-0.0.3rc4-clean:
	-@$(MAKE) -C gctwimax-0.0.3rc4 clean
	@rm -f gctwimax-0.0.3rc4/stamp-h1

wpa_supplicant:
	$(MAKE) -C $@/wpa_supplicant EXTRACFLAGS="-Os $(EXTRACFLAGS)"

wpa_supplicant-install: wpa_supplicant
	install -D wpa_supplicant/wpa_supplicant/wpa_supplicant $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_supplicant
	install -D wpa_supplicant/wpa_supplicant/wpa_cli $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_cli
	$(STRIP) $(INSTALLDIR)/wpa_supplicant/usr/sbin/*

wpa_supplicant-clean:
	-@$(MAKE) -C wpa_supplicant/wpa_supplicant clean

tr069:
	cp tr069/Makefile.ASUSWRT tr069/Makefile
	$(MAKE) -C $@
	$(MAKE) -C $@/test

tr069-clean:
	[ ! -f tr069/Makefile ] || $(MAKE) -C tr069 clean
	[ ! -f tr069/test/Makefile ] || $(MAKE) -C tr069/test clean

tr069-install: tr069
	install -D tr069/build/bin/tr $(INSTALLDIR)/tr069/sbin/tr069
	install -D tr069/test/sendtocli $(INSTALLDIR)/tr069/sbin/sendtocli
	install -D tr069/test/notify $(INSTALLDIR)/tr069/sbin/notify
	install -D tr069/test/udpclient $(INSTALLDIR)/tr069/sbin/udpclient
ifeq ($(RTCONFIG_TR181),y)
ifeq ($(RTCONFIG_DSL),y)
	install -D tr069/conf/ASUSWRT/tr_181.xml.dsl $(INSTALLDIR)/tr069/usr/tr/tr.xml
else
	install -D tr069/conf/ASUSWRT/tr_181.xml $(INSTALLDIR)/tr069/usr/tr/tr.xml
endif
else
ifeq ($(RTCONFIG_DSL),y)
	install -D tr069/conf/ASUSWRT/tr_98.xml.dsl $(INSTALLDIR)/tr069/usr/tr/tr.xml
else
	install -D tr069/conf/ASUSWRT/tr_98.xml $(INSTALLDIR)/tr069/usr/tr/tr.xml
endif
endif

nt_center/stamp-h1:
	touch $@

nt_center: nt_center/stamp-h1
	$(MAKE) -C nt_center


nt_center-install: nt_center
	install -d $(INSTALLDIR)/nt_center/usr/lib/
	install -d $(INSTALLDIR)/nt_center/usr/sbin/
	install -D nt_center/lib/libnt.so $(INSTALLDIR)/nt_center/usr/lib/libnt.so
	install -D nt_center/nt_center $(INSTALLDIR)/nt_center/usr/sbin/nt_center
	install -D nt_center/nt_monitor $(INSTALLDIR)/nt_center/usr/sbin/nt_monitor
	install -D nt_center/actMail/nt_actMail $(INSTALLDIR)/nt_center/usr/sbin/nt_actMail
	install -D nt_center/Notify_Event2NC $(INSTALLDIR)/nt_center/usr/sbin/Notify_Event2NC
	install -D nt_center/lib/nt_db $(INSTALLDIR)/nt_center/usr/sbin/nt_db
	#install -D nt_center/lib/nt_db_test $(INSTALLDIR)/nt_center/usr/sbin/nt_db_test
	$(STRIP) $(INSTALLDIR)/nt_center/usr/lib/libnt.so
	$(STRIP) $(INSTALLDIR)/nt_center/usr/sbin/nt_center
	$(STRIP) $(INSTALLDIR)/nt_center/usr/sbin/nt_monitor
	$(STRIP) $(INSTALLDIR)/nt_center/usr/sbin/nt_actMail
	$(STRIP) $(INSTALLDIR)/nt_center/usr/sbin/Notify_Event2NC
	$(STRIP) $(INSTALLDIR)/nt_center/usr/sbin/nt_db
	#$(STRIP) $(INSTALLDIR)/nt_center/usr/sbin/nt_db_test

nt_center-clean:
	-$(MAKE) -C nt_center clean
	@rm -f nt_center/stamp-h1

protect_srv/stamp-h1:
	touch $@

protect_srv-stage:
	$(MAKE) -C protect_srv/lib
	install -D protect_srv/lib/libptcsrv.so $(STAGEDIR)/usr/lib/libptcsrv.so
	install -D -m 644 -p protect_srv/include/protect_srv.h $(STAGEDIR)/usr/include/protect_srv.h
	install -D -m 644 -p protect_srv/include/libptcsrv.h $(STAGEDIR)/usr/include/libptcsrv.h

protect_srv: protect_srv/stamp-h1
	$(MAKE) -C $@ && $(MAKE) $@-stage

protect_srv-install: protect_srv
	install -d $(INSTALLDIR)/protect_srv/usr/lib/
	install -d $(INSTALLDIR)/protect_srv/usr/sbin/
	install -D protect_srv/lib/libptcsrv.so $(INSTALLDIR)/protect_srv/usr/lib/libptcsrv.so
	install -D protect_srv/protect_srv $(INSTALLDIR)/protect_srv/usr/sbin/protect_srv
	install -D protect_srv/Send_Event2ptcsrv $(INSTALLDIR)/protect_srv/usr/sbin/Send_Event2ptcsrv
	$(STRIP) $(INSTALLDIR)/protect_srv/usr/lib/libptcsrv.so
	$(STRIP) $(INSTALLDIR)/protect_srv/usr/sbin/protect_srv

protect_srv-clean:
	-$(MAKE) -C protect_srv clean
	@rm -f protect_srv/stamp-h1

#	$(STRIP) $(INSTALLDIR)/tr/sbin/*

#	libnet:
#		@$(SEP)
#		@-mkdir -p libnet/lib
#		@$(MAKE) -C libnet CC=$(CC) AR=$(AR) RANLIB=$(RANLIB)

libpcap/stamp-h1:
	cd libpcap && $(CONFIGURE) --with-pcap=linux
	touch $@

libpcap: libpcap/stamp-h1
	@$(SEP)
	@$(MAKE) -C libpcap CC=$(CC) AR=$(AR) RANLIB=$(RANLIB)

libpcap-install: libpcap
	@echo "do nothing"
	#install -D libpcap/libpcap.so.1.4.0 $(INSTALLDIR)/libpcap/usr/lib/libpcap.so.1.4.0
	#$(STRIP) $(INSTALLDIR)/libpcap/usr/lib/libpcap.so.1.4.0
	#cd $(INSTALLDIR)/libpcap/usr/lib && ln -sf libpcap.so.1.4.0 libpcap.so

libpcap-clean:
	[ ! -f libpcap/Makefile ] || $(MAKE) -C libpcap clean
	@rm -f libpcap/stamp-h1

tcpdump-4.4.0/stamp-h1:
	cd tcpdump-4.4.0 && $(CONFIGURE) ac_cv_linux_vers=2
	touch $@

tcpdump-4.4.0: libpcap tcpdump-4.4.0/stamp-h1
	@$(SEP)
	@$(MAKE) -C tcpdump-4.4.0 CC=$(CC) AR=$(AR) RANLIB=$(RANLIB)

tcpdump-4.4.0-install: tcpdump-4.4.0
	install -D tcpdump-4.4.0/tcpdump $(INSTALLDIR)/tcpdump-4.4.0/usr/sbin/tcpdump
	$(STRIP) $(INSTALLDIR)/tcpdump-4.4.0/usr/sbin/tcpdump

tcpdump-4.4.0-clean:
	[ ! -f tcpdump-4.4.0/Makefile ] || $(MAKE) -C tcpdump-4.4.0 clean
	@rm -f tcpdump-4.4.0/stamp-h1

traceroute-2.1.0/stamp-h1: 
	touch $@

traceroute-2.1.0: traceroute-2.1.0/stamp-h1
	$(MAKE) -C traceroute-2.1.0


traceroute-2.1.0-install: traceroute-2.1.0
	install -d $(INSTALLDIR)/traceroute-2.1.0/usr/sbin/
	install -D traceroute-2.1.0/traceroute/traceroute $(INSTALLDIR)/traceroute-2.1.0/usr/sbin/traceroute
	$(STRIP) $(INSTALLDIR)/traceroute-2.1.0/usr/sbin/traceroute

traceroute-2.1.0-clean:
	-$(MAKE) -C traceroute-2.1.0 clean
	@rm -f traceroute-2.1.0/stamp-h1

libqcsapi_client-install: libqcsapi_client
	install -d $(INSTALLDIR)/libqcsapi_client/usr/lib/
	install -d $(INSTALLDIR)/libqcsapi_client/usr/sbin/
	install -D libqcsapi_client/libqcsapi_client.so.1.0.1 $(INSTALLDIR)/libqcsapi_client/usr/lib/
	install -D libqcsapi_client/qcsapi_sockrpc $(INSTALLDIR)/libqcsapi_client/usr/sbin/
	install -D libqcsapi_client/qcsapi_sockraw $(INSTALLDIR)/libqcsapi_client/usr/sbin/
	# install -D libqcsapi_client/c_rpc_qcsapi_sample $(INSTALLDIR)/libqcsapi_client/usr/sbin/
	$(STRIP) $(INSTALLDIR)/libqcsapi_client/usr/lib/*.so.*
	$(STRIP) $(INSTALLDIR)/libqcsapi_client/usr/sbin/*
	cd $(INSTALLDIR)/libqcsapi_client/usr/lib && \
		ln -sf libqcsapi_client.so.1.0.1 libqcsapi_client.so.1 && \
		ln -sf libqcsapi_client.so.1.0.1 libqcsapi_client.so

libqcsapi_client_10g-install: libqcsapi_client_10g
	install -d $(INSTALLDIR)/libqcsapi_client_10g/usr/lib/
	install -d $(INSTALLDIR)/libqcsapi_client_10g/usr/sbin/
	install -D libqcsapi_client_10g/libqcsapi_client.so.1.0.1 $(INSTALLDIR)/libqcsapi_client_10g/usr/lib/libqcsapi_client.so.1
	install -D libqcsapi_client_10g/qcsapi_pcie $(INSTALLDIR)/libqcsapi_client_10g/usr/sbin/
	install -D ${SRCBASE}/tools/${TOOLCHAIN_NAME}/lib/libuClibc-0.9.33.2.so $(TARGETDIR)/lib/libuClibc-0.9.33.2.so
	$(STRIP) $(INSTALLDIR)/libqcsapi_client_10g/usr/lib/*.so.*
	$(STRIP) $(INSTALLDIR)/libqcsapi_client_10g/usr/sbin/*
	$(STRIP) $(TARGETDIR)/lib/libuClibc-0.9.33.2.so

libbcm:
	@[ ! -f libbcm/Makefile ] || $(MAKE) -C libbcm

libbcm-install:
	install -D libbcm/libbcm.so $(INSTALLDIR)/libbcm/usr/lib/libbcm.so
	$(STRIP) $(INSTALLDIR)/libbcm/usr/lib/libbcm.so

.PHONY: libnl-tiny-0.1 $(SWCONFIG_PKG)
libnl-tiny-0.1:
	$(MAKE) -C $@ && $(MAKE) $@-stage

libnl-tiny-0.1-clean:
	$(MAKE) -C libnl-tiny-0.1 clean

libnl-tiny-0.1-install: libnl-tiny-0.1
	install -D libnl-tiny-0.1/libnl-tiny.so $(INSTALLDIR)/libnl-tiny-0.1/usr/lib/libnl-tiny.so
	$(STRIP) $(INSTALLDIR)/libnl-tiny-0.1/usr/lib/libnl-tiny.so

libnl-tiny-0.1-stage:
	$(MAKE) -C libnl-tiny-0.1 stage

$(SWCONFIG_PKG): libnl-tiny-0.1
	$(MAKE) -C $(SWCONFIG_PKG)

$(SWCONFIG_PKG)-clean:
	$(MAKE) -C $(SWCONFIG_PKG) clean

$(SWCONFIG_PKG)-install: $(SWCONFIG_PKG)
	install -D $(SWCONFIG_PKG)/swconfig $(INSTALLDIR)/$(SWCONFIG_PKG)/usr/sbin/swconfig
	$(STRIP) $(INSTALLDIR)/$(SWCONFIG_PKG)/usr/sbin/swconfig

iperf: iperf/Makefile
	@$(SEP)
	$(MAKE) -C $@

iperf/Makefile:
	# libstdc++.so.6 is required if you want to remove CFLAGS=-static below.
	( cd iperf ; CFLAGS=-static $(CONFIGURE) ac_cv_func_malloc_0_nonnull=yes )

iperf-install:
	install -D iperf/src/iperf $(INSTALLDIR)/iperf/usr/bin/iperf
	$(STRIP) $(INSTALLDIR)/iperf/usr/bin/iperf

iperf-clean:
	[ ! -f iperf/Makefile ] || $(MAKE) -C iperf distclean
	@rm -f iperf/Makefile

lldpd-0.9.8: lldpd-0.9.8/Makefile
	$(MAKE) -C $@

lldpd-0.9.8/Makefile:
ifeq ($(HND_ROUTER),y)
	(cd lldpd-0.9.8 ; autoreconf -i -f ; LDFLAGS="-L$(TOP)/lldpd-0.9.8/lib -lm -L$(STAGEDIR)/usr/lib -ljansson $(if $(RTCONFIG_AMAS),-L$(TOP)/json-c/.libs,)" \
	$(CONFIGURE) $(if $(RTCONFIG_AMAS),JSONC_CFLAGS='-I$(TOP)/json-c' JSONC_LIBS='-L$(TOP)/josn-c/.libs -ljson-c',) --prefix= --bindir=/usr/sbin --libdir=/usr/lib --with-privsep-user=admin --with-privsep-group=root --with-privsep-path=/var/run/lldpd $(if $(RTCONFIG_AMAS),--with-json=yes,))
else	
	(cd lldpd-0.9.8 ; autoreconf -i -f ; LDFLAGS="-L$(TOP)/lldpd-0.9.8/lib -lm -L$(STAGEDIR)/usr/lib -ljansson $(if $(RTCONFIG_AMAS),-L$(TOP)/json-c/.libs,)" \
	$(CONFIGURE) $(if $(RTCONFIG_AMAS),JSONC_CFLAGS='-I$(TOP)/json-c' JSONC_LIBS='-L$(TOP)/josn-c/.libs -ljson-c',) --prefix= --enable-oldies --bindir=/usr/sbin --libdir=/usr/lib --with-privsep-user=admin --with-privsep-group=root --with-privsep-path=/var/run/lldpd $(if $(RTCONFIG_AMAS),--with-json=yes,))
endif

lldpd-0.9.8-install: lldpd-0.9.8
	install -D lldpd-0.9.8/src/client/.libs/lldpcli $(INSTALLDIR)/lldpd-0.9.8/usr/sbin/lldpcli
	install -D lldpd-0.9.8/src/daemon/lldpd $(INSTALLDIR)/lldpd-0.9.8/usr/sbin/lldpd
	install -D lldpd-0.9.8/src/lib/.libs/liblldpctl.so.4.8.0 $(INSTALLDIR)/lldpd-0.9.8/usr/lib/liblldpctl.so.4.8.0
	cd $(INSTALLDIR)/lldpd-0.9.8/usr/lib && ln -sf liblldpctl.so.4.8.0 liblldpctl.so && ln -sf liblldpctl.so.4.8.0 liblldpctl.so.4

lldpd-0.9.8-clean:
	[ ! -f lldpd-0.9.8/Makefile ] || $(MAKE) -C lldpd-0.9.8 clean
	@rm -f lldpd-0.9.8/Makefile
	@cd lldpd-0.9.8/libevent && make clean

libevent-2.0.21: libevent-2.0.21/Makefile
	$(MAKE) -C $@ $(PARALLEL_BUILD) && $(MAKE) $@-stage && sed 's|/usr/lib|$(STAGEDIR)/usr/lib|g' -i $(STAGEDIR)/usr/lib/libevent.la

libevent-2.0.21/Makefile:
	( cd libevent-2.0.21 && autoreconf -i -f ; CFLAGS="$(CFLAGS) -Os -Wall $(EXTRACFLAGS)" \
		$(CONFIGURE) --prefix=/usr --bindir=/usr/sbin --libdir=/usr/lib \
	)

libevent-2.0.21-install: libevent-2.0.21
	install -D $(STAGEDIR)/usr/lib/libevent-2.0.so.5 $(INSTALLDIR)/$</usr/lib/libevent-2.0.so.5
	$(STRIP) $(INSTALLDIR)/$</usr/lib/libevent-2.0.so*

libevent-2.0.21-clean:
	[ ! -f libevent-2.0.21/Makefile ] || $(MAKE) -C libevent-2.0.21 clean
	@rm -f libevent-2.0.21/Makefile

tor: openssl zlib libevent-2.0.21 tor/Makefile
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)

tor/Makefile:
	(cd tor ; $(CONFIGURE) --enable-static-libevent --with-libevent-dir=$(STAGEDIR)/usr \
					--with-openssl-dir=$(TOP)/openssl \
					--with-zlib-dir=$(TOP)/zlib \
					--disable-asciidoc --disable-unittests \
					--disable-tool-name-check)

tor-install:
	install -D tor/src/or/tor $(INSTALLDIR)/tor/usr/sbin/Tor
	$(STRIP) $(INSTALLDIR)/tor/usr/sbin/Tor

tor-clean:
	[ ! -f tor/Makefile ] || $(MAKE) -C tor clean
	@rm -f tor/Makefile

iproute2:
	@$(SEP)
	@$(MAKE) -C $@ KERNEL_INCLUDE=$(LINUX_INC_DIR)/include EXTRACFLAGS="$(EXTRACFLAGS) $(if $(RTCONFIG_IPV6),-DUSE_IPV6,-DNO_IPV6)" $(PARALLEL_BUILD)

iproute2-install: iproute2
	install -D iproute2/tc/tc $(INSTALLDIR)/iproute2/usr/sbin/tc
	install -D iproute2/tc/tc $(INSTALLDIR)/iproute2/usr/sbin/realtc
	$(STRIP) $(INSTALLDIR)/iproute2/usr/sbin/tc
	$(STRIP) $(INSTALLDIR)/iproute2/usr/sbin/realtc
	install -D iproute2/ip/ip $(INSTALLDIR)/iproute2/usr/sbin/ip
	$(STRIP) $(INSTALLDIR)/iproute2/usr/sbin/ip

iproute2-3.x: kernel_header iptables-1.4.x
	@$(SEP)
	@$(MAKE) -C $@ IPTABLES_DIR=$(TOP)/iptables-1.4.x KERNEL_INCLUDE=$(TOP)/kernel_header/include EXTRACFLAGS="$(EXTRACFLAGS) $(if $(RTCONFIG_IPV6),-DUSE_IPV6,-DNO_IPV6)" && $(MAKE) -C $@ stage

iproute2-3.x-install: iproute2-3.x
	install -D iproute2-3.x/tc/tc $(INSTALLDIR)/iproute2-3.x/usr/sbin/tc
	install -D iproute2-3.x/tc/tc $(INSTALLDIR)/iproute2-3.x/usr/sbin/realtc
	$(STRIP) $(INSTALLDIR)/iproute2-3.x/usr/sbin/tc
	$(STRIP) $(INSTALLDIR)/iproute2-3.x/usr/sbin/realtc
	install -D iproute2-3.x/ip/ip $(INSTALLDIR)/iproute2-3.x/usr/sbin/ip
	$(STRIP) $(INSTALLDIR)/iproute2-3.x/usr/sbin/ip
	@if [ -e iproute2-3.x/tc/m_xt.so ] ; then \
		install -D iproute2-3.x/tc/m_xt.so $(INSTALLDIR)/iproute2-3.x/usr/lib/tc/m_xt.so ; \
		ln -sf m_xt.so $(INSTALLDIR)/iproute2-3.x/usr/lib/tc/m_ipt.so ; \
		$(STRIP) $(INSTALLDIR)/iproute2-3.x/usr/lib/tc/*.so ; \
	fi

iproute2-3.x-clean:
	-@$(MAKE) -C iproute2-3.x clean
	-rm -f iproute2-3.x/Config

iproute2-3.15.0: kernel_header iptables-1.4.21
	@$(SEP)
	@$(MAKE) -C $@ IPTABLES_DIR=$(TOP)/iptables-1.4.21 KERNEL_INCLUDE=$(TOP)/kernel_header/include EXTRACFLAGS="$(EXTRACFLAGS) $(if $(RTCONFIG_IPV6),-DUSE_IPV6,-DNO_IPV6)"

iproute2-3.15.0-install: iproute2-3.15.0
	install -D iproute2-3.15.0/tc/tc $(INSTALLDIR)/iproute2-3.15.0/usr/sbin/tc
	$(STRIP) $(INSTALLDIR)/iproute2-3.15.0/usr/sbin/tc
	install -D iproute2-3.15.0/ip/ip $(INSTALLDIR)/iproute2-3.15.0/usr/sbin/ip
	$(STRIP) $(INSTALLDIR)/iproute2-3.15.0/usr/sbin/ip
	@if [ -e iproute2-3.15.0/tc/m_xt.so ] ; then \
		install -D iproute2-3.15.0/tc/m_xt.so $(INSTALLDIR)/iproute2-3.15.0/usr/lib/tc/m_xt.so ; \
		ln -sf m_xt.so $(INSTALLDIR)/iproute2-3.15.0/usr/lib/tc/m_ipt.so ; \
		$(STRIP) $(INSTALLDIR)/iproute2-3.15.0/usr/lib/tc/*.so ; \
	fi

iproute2-3.15.0-clean:
	-@$(MAKE) -C iproute2-3.15.0 clean
	-rm -f iproute2-3.15.0/Config

iproute2-4.3:
	touch iproute2-4.3/iproute2-4.3.0/configure
	$(MAKE) -C iproute2-4.3

iproute2-4.3-install:
	install -D iproute2-4.3/iproute2-4.3.0/tc/tc $(INSTALLDIR)/iproute2-4.3/usr/sbin/tc
	$(STRIP) $(INSTALLDIR)/iproute2-4.3/usr/sbin/tc
	install -D iproute2-4.3/iproute2-4.3.0/ip/ip $(INSTALLDIR)/iproute2-4.3/usr/sbin/ip
	$(STRIP) $(INSTALLDIR)/iproute2-4.3/usr/sbin/ip

iproute2-4.3-clean:
	-@$(MAKE) -C iproute2-4.3 clean
	-rm -f iproute2-4.3/iproute2-4.3.0/Config

ntpclient: nvram$(BCMEX)$(EX7) shared

ntpclient-install:
	$(MAKE) -C ntpclient INSTALLDIR=$(INSTALLDIR)/ntpclient install

rstats: nvram$(BCMEX)$(EX7) shared

rstats-install:
	$(MAKE) -C rstats INSTALLDIR=$(INSTALLDIR)/rstats install

dropbear/config.h.in: $(if $(RTCONFIG_PROTECTION_SERVER),protect_srv-stage,)

dropbear/config.h: dropbear/config.h.in
	cd dropbear && \
		CPPFLAGS="-O2 -Wall $(EXTRACFLAGS) -DARGTYPE=3 -ffunction-sections -fdata-sections -I$(STAGEDIR)/usr/include \
			$(if $(RTCONFIG_PROTECTION_SERVER),-DSECURITY_NOTIFY,)" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(STAGEDIR)/usr/lib" \
		LIBS="$(if $(RTCONFIG_PROTECTION_SERVER),-lptcsrv)" \
		$(CONFIGURE) --disable-zlib --disable-pam \
		--enable-openpty --enable-syslog --enable-shadow --enable-bundled-libtom \
		--disable-lastlog \
		--disable-utmp --disable-utmpx \
		--disable-wtmp --disable-wtmpx \
		--disable-loginfunc \
		--disable-pututline --disable-pututxline

dropbear: dropbear/config.h
	@$(SEP)
	@$(MAKE) -C $@ PROGRAMS="dropbear dbclient dropbearkey scp" MULTI=1 $(PARALLEL_BUILD)

dropbear-install:
	install -D dropbear/dropbearmulti $(INSTALLDIR)/dropbear/usr/bin/dropbearmulti
	$(STRIP) $(INSTALLDIR)/dropbear/usr/bin/dropbearmulti
	cd $(INSTALLDIR)/dropbear/usr/bin && \
	ln -sf dropbearmulti dropbearkey && \
	ln -sf dropbearmulti dbclient && \
	ln -sf dropbearmulti ssh && \
	ln -sf dropbearmulti scp
	install -d $(INSTALLDIR)/dropbear/usr/sbin
	cd $(INSTALLDIR)/dropbear/usr/sbin && \
	ln -sf ../bin/dropbearmulti dropbear

dropbear-clean:
	-@$(MAKE) -C dropbear clean
	@rm -f dropbear/config.h

wtfast-configure:
	@true

wtfast-install:
	install -D wtfast/wtfslhd/wtfslhd$(BCMEX)$(EX7)/wtfslhd $(INSTALLDIR)/wtfast/usr/sbin/wtfslhd

# Media libraries

sqlite/stamp-h1:
	cd sqlite && \
	CC=$(CC) CFLAGS="-O2 $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -lpthread -ldl" \
		$(CONFIGURE) --prefix=/usr --enable-shared --enable-static \
		--disable-readline --disable-dynamic-extensions --enable-threadsafe \
		--disable-dependency-tracking
	touch $@

sqlite: sqlite/stamp-h1
	@$(MAKE) -C sqlite all

sqlite-clean:
	-@$(MAKE) -C sqlite clean
	@rm -f sqlite/stamp-h1

sqlite-install: sqlite
	@$(SEP)
	install -D sqlite/.libs/libsqlite3.so.0 $(INSTALLDIR)/sqlite/usr/lib/libsqlite3.so.0
	$(STRIP) $(INSTALLDIR)/sqlite/usr/lib/libsqlite3.so.0

# sqlite encrpted version
sqlCipher/stamp-h1:
	cd sqlCipher && \
        CC=$(CC) CFLAGS="-Os $(EXTRACFLAGS) -DSQLITE_HAS_CODEC -ffunction-sections -fdata-sections -I$(SRCBASE)/router/openssl/include  -fPIC" \
                LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -lpthread -ldl -L$(SRCBASE)/router/openssl -lcrypto" \
                $(CONFIGURE) --prefix=/usr --enable-shared --enable-static \
                --disable-readline --disable-dynamic-extensions --enable-threadsafe \
                --disable-dependency-tracking --enable-tempstore=yes --disable-tcl
	touch $@

sqlCipher: sqlCipher/stamp-h1
	@$(MAKE) -C sqlCipher all

sqlCipher-clean:
	-@$(MAKE) -C sqlCipher clean
	@rm -f sqlCipher/stamp-h1

sqlCipher-install: sqlCipher
	install -D sqlCipher/.libs/libsqlcipher.so.0.8.6 $(INSTALLDIR)/sqlCipher/usr/lib/libsqlcipher.so.0
	$(STRIP) $(INSTALLDIR)/sqlCipher/usr/lib/libsqlcipher.so.0
	@$(SEP)

# commented out for mt-daapd-svn-1696
ifneq ($(MEDIA_SERVER_STATIC),y)
	install -D sqlite/.libs/libsqlite3.so.0 $(INSTALLDIR)/sqlite/usr/lib/libsqlite3.so.0
	$(STRIP) $(INSTALLDIR)/sqlite/usr/lib/libsqlite3.so.0
endif
ifeq ($(RTCONFIG_FREERADIUS),y)
	install -D sqlite/.libs/sqlite3 $(INSTALLDIR)/sqlite/usr/sbin/sqlite3
	$(STRIP) $(INSTALLDIR)/sqlite/usr/sbin/sqlite3
endif

FFMPEG_FILTER_CONFIG= $(foreach c, $(2), --$(1)="$(c)")

FFMPEG_DECODERS:=aac ac3 atrac3 h264 jpegls mp3 mpeg1video mpeg2video mpeg4 mpeg4aac mpegvideo png wmav1 wmav2
FFMPEG_CONFIGURE_DECODERS:=$(call FFMPEG_FILTER_CONFIG,enable-decoder,$(FFMPEG_DECODERS))

FFMPEG_PARSERS:=aac ac3 h264 mpeg4video mpegaudio mpegvideo
FFMPEG_CONFIGURE_PARSERS:=$(call FFMPEG_FILTER_CONFIG,enable-parser,$(FFMPEG_PARSERS))

FFMPEG_PROTOCOLS:=file
FFMPEG_CONFIGURE_PROTOCOLS:=$(call FFMPEG_FILTER_CONFIG,enable-protocol,$(FFMPEG_PROTOCOLS))

FFMPEG_DISABLED_DEMUXERS:=amr apc ape ass bethsoftvid bfi c93 daud dnxhd dsicin dxa ffm gsm gxf idcin iff image2 image2pipe ingenient ipmovie lmlm4 mm mmf msnwc_tcp mtv mxf nsv nut oma pva rawvideo rl2 roq rpl segafilm shorten siff smacker sol str thp tiertexseq tta txd vmd voc wc3 wsaud wsvqa xa yuv4mpegpipe
FFMPEG_CONFIGURE_DEMUXERS:=$(call FFMPEG_FILTER_CONFIG,disable-demuxer,$(FFMPEG_DISABLED_DEMUXERS))

ffmpeg/stamp-h1: zlib
	cd ffmpeg && symver_asm_label=no symver_gnu_asm=no symver=no CC=$(CC) LDFLAGS="-ldl"\
		./configure --enable-cross-compile --arch=$(ARCH) --target_os=linux \
		--cross-prefix=$(CROSS_COMPILE) --enable-shared --enable-gpl --disable-doc \
		--enable-pthreads --enable-small --disable-encoders --disable-filters \
		--disable-muxers --disable-devices --disable-ffmpeg --disable-ffplay \
		--disable-ffserver --disable-ffprobe --disable-avdevice --disable-swscale \
		--disable-hwaccels --disable-network --disable-bsfs --disable-mpegaudio-hp \
		--enable-demuxers $(FFMPEG_CONFIGURE_DEMUXERS) \
		--disable-decoders $(FFMPEG_CONFIGURE_DECODERS) \
		--disable-parsers $(FFMPEG_CONFIGURE_PARSERS) \
		--disable-protocols $(FFMPEG_CONFIGURE_PROTOCOLS) \
		--extra-cflags="-Os $(EXTRACFLAGSx) -ffunction-sections -fdata-sections -fPIC -I$(TOP)/zlib" \
		--extra-ldflags="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
		--extra-libs="-L$(TOP)/zlib -lz" \
		--enable-zlib --disable-debug --prefix=''
	touch $@

ffmpeg: ffmpeg/stamp-h1 zlib
	$(MAKE) -C ffmpeg all $(PARALLEL_BUILD)

ffmpeg-clean:
	-@$(MAKE) -C ffmpeg clean
	@rm -f ffmpeg/stamp-h1 ffmpeg/config.h ffmpeg/config.mak

ffmpeg-install: ffmpeg
	@$(SEP)
ifneq ($(MEDIA_SERVER_STATIC),y)
	install -D ffmpeg/libavformat/libavformat.so.52 $(INSTALLDIR)/ffmpeg/usr/lib/libavformat.so.52
	install -D ffmpeg/libavcodec/libavcodec.so.52 $(INSTALLDIR)/ffmpeg/usr/lib/libavcodec.so.52
	install -D ffmpeg/libavutil/libavutil.so.50 $(INSTALLDIR)/ffmpeg/usr/lib/libavutil.so.50
	$(STRIP) $(INSTALLDIR)/ffmpeg/usr/lib/libavformat.so.52
	$(STRIP) $(INSTALLDIR)/ffmpeg/usr/lib/libavcodec.so.52
	$(STRIP) $(INSTALLDIR)/ffmpeg/usr/lib/libavutil.so.50
endif

.PHONY: ffmpeg

libogg/stamp-h1:
	cd libogg && autoreconf -i -f && \
	CFLAGS="-Os $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections" \
	LDFLAGS="-fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	$(CONFIGURE) --enable-shared --enable-static --prefix=''
	touch $@
	touch libogg/aclocal.m4

libogg: libogg/stamp-h1
	@$(MAKE) -C libogg all

libogg-clean:
	-@$(MAKE) -C libogg clean
	@rm -f libogg/stamp-h1

libogg-install: libogg
	@$(SEP)
ifneq ($(MEDIA_SERVER_STATIC),y)
	install -D libogg/src/.libs/libogg.so.0 $(INSTALLDIR)/libogg/usr/lib/libogg.so.0
	$(STRIP) $(INSTALLDIR)/libogg/usr/lib/libogg.so.0
endif

flac/stamp-h1: libogg
	cd flac && \
	CFLAGS="-Os $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections" \
	CPPFLAGS="-I$(TOP)/libogg/include" \
	LDFLAGS="-L$(TOP)/libogg/src/.libs -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	$(CONFIGURE) --enable-shared --enable-static --prefix='' --disable-rpath \
		--disable-doxygen-docs --disable-xmms-plugin --disable-cpplibs \
		--without-libiconv-prefix --disable-altivec --disable-3dnow --disable-sse
	touch $@

flac: flac/stamp-h1 libogg
	@$(MAKE) -C flac/src/libFLAC all $(PARALLEL_BUILD)

flac-clean:
	-@$(MAKE) -C flac clean
	@rm -f flac/stamp-h1

flac-install: flac
	@$(SEP)
ifneq ($(MEDIA_SERVER_STATIC),y)
	install -D flac/src/libFLAC/.libs/libFLAC.so.8 $(INSTALLDIR)/flac/usr/lib/libFLAC.so.8
	$(STRIP) $(INSTALLDIR)/flac/usr/lib/libFLAC.so.8
endif

jpeg/stamp-h1:
	cd jpeg && \
	CFLAGS="-Os $(EXTRACFLAGS) -fPIC" CC=$(CC) AR2="touch" $(CONFIGURE) --enable-maxmem=1 --enable-shared --enable-static --prefix=''
	touch $@

jpeg: jpeg/stamp-h1
	@$(MAKE) -C jpeg LIBTOOL="" O=o A=a CC=$(CC) AR2="touch" libjpeg.a libjpeg.so $(PARALLEL_BUILD)

jpeg-clean:
	-@$(MAKE) -C jpeg clean
	@rm -f jpeg/stamp-h1

jpeg-install:
	@$(SEP)
ifneq ($(MEDIA_SERVER_STATIC),y)
	install -D jpeg/libjpeg.so $(INSTALLDIR)/jpeg/usr/lib/libjpeg.so
	$(STRIP) $(INSTALLDIR)/jpeg/usr/lib/libjpeg.so
endif

libexif/stamp-h1:
	cd libexif && CFLAGS="-Os -Wall $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections" \
	LDFLAGS="-fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	$(CONFIGURE) --enable-shared --enable-static --prefix='' \
		--disable-docs --disable-rpath --disable-nls --without-libiconv-prefix --without-libintl-prefix
	touch $@

libexif: libexif/stamp-h1
	@$(MAKE) -C libexif all $(PARALLEL_BUILD)

libexif-clean:
	-@$(MAKE) -C libexif clean
	@rm -f libexif/stamp-h1

libexif-install: libexif
	@$(SEP)
ifneq ($(MEDIA_SERVER_STATIC),y)
	install -D libexif/libexif/.libs/libexif.so.12 $(INSTALLDIR)/libexif/usr/lib/libexif.so.12
	$(STRIP) $(INSTALLDIR)/libexif/usr/lib/libexif.so.12
endif

zlib/stamp-h1:
	cd zlib && \
	CC=$(CC) AR="ar rc" RANLIB=$(RANLIB) LD=$(LD) CFLAGS="-Os -Wall $(EXTRACFLAGS) -fPIC" LDSHAREDLIBC="$(EXTRALDFLAGS)" \
	./configure --shared --prefix=/usr --libdir=/usr/lib
	touch $@

zlib: zlib/stamp-h1
	@$(MAKE) -C zlib CC=$(CC) AR="ar rc" RANLIB=$(RANLIB) LD=$(LD) all && $(MAKE) $@-stage

zlib-clean:
	-@$(MAKE) -C zlib clean
	@rm -f zlib/stamp-h1

zlib-install:
	@$(SEP)
# commented out for mt-daapd-svn-1696
ifneq ($(MEDIA_SERVER_STATIC),y)
	install -d $(INSTALLDIR)/zlib/usr/lib
	install -D zlib/libz.so.1 $(INSTALLDIR)/zlib/usr/lib/
	$(STRIP) $(INSTALLDIR)/zlib/usr/lib/libz.so.1
endif
ifeq ($(or $(RTCONFIG_USB_BECEEM),$(RTCONFIG_MEDIA_SERVER),$(RTCONFIG_CLOUDSYNC),$(RTCONFIG_PUSH_EMAIL)),y)
	install -d $(INSTALLDIR)/zlib/usr/lib
	install -D zlib/libz.so.1 $(INSTALLDIR)/zlib/usr/lib/
	$(STRIP) $(INSTALLDIR)/zlib/usr/lib/libz.so.1
endif

libid3tag/stamp-h1: zlib
	cd libid3tag && \
	touch NEWS && touch AUTHORS && touch ChangeLog && autoreconf -i -f && \
	CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" CPPFLAGS="-I$(TOP)/zlib" \
	LDFLAGS="-L$(TOP)/zlib -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	$(CONFIGURE) --enable-shared --enable-static --prefix='' \
		--disable-debugging --disable-profiling --disable-dependency-tracking
	touch $@

libid3tag: libid3tag/stamp-h1 zlib
	@$(MAKE) -C libid3tag all $(PARALLEL_BUILD)

libid3tag-clean:
	-@$(MAKE) -C libid3tag clean
	@rm -f libid3tag/stamp-h1

libid3tag-install: libid3tag
	@$(SEP)
#commented out for mt-daapd-svn-1696
ifneq ($(MEDIA_SERVER_STATIC),y)
	install -D libid3tag/.libs/libid3tag.so.0 $(INSTALLDIR)/libid3tag/usr/lib/libid3tag.so.0
	$(STRIP) $(INSTALLDIR)/libid3tag/usr/lib/libid3tag.so.0
endif


libvorbis/stamp-h1: libogg
	cd libvorbis && \
	CFLAGS="-Os -Wall $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections" \
	CPPFLAGS="-I$(TOP)/libogg/include" \
	LDFLAGS="-L$(TOP)/libogg/src/.libs -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -ldl" \
	$(CONFIGURE) --enable-shared --enable-static --prefix='' --disable-oggtest \
		--with-ogg-includes="$(TOP)/libogg/include" \
		--with-ogg-libraries="$(TOP)/libogg/src/.libs"
	touch $@
	touch libvorbis/aclocal.m4

libvorbis: libvorbis/stamp-h1
	@$(MAKE) -C libvorbis/lib all $(PARALLEL_BUILD)

libvorbis-clean:
	-@$(MAKE) -C libvorbis clean
	@rm -f libvorbis/stamp-h1

libvorbis-install: libvorbis
	@$(SEP)
ifneq ($(MEDIA_SERVER_STATIC),y)
	install -D libvorbis/lib/.libs/libvorbis.so.0 $(INSTALLDIR)/libvorbis/usr/lib/libvorbis.so.0
	$(STRIP) $(INSTALLDIR)/libvorbis/usr/lib/libvorbis.so.0
endif

minidlna: zlib sqlite ffmpeg libogg flac jpeg libexif libid3tag libvorbis minidlna/Makefile
	@$(SEP)
	@$(MAKE) -C minidlna CC=$(CC) $(if $(RTCONFIG_LIMIT_MEDIA_SERVER),MS_LIMIT=1) $(PARALLEL_BUILD)

minidlna/Makefile: minidlna/configure
	$(MAKE) minidlna-configure

minidlna/configure:
	( cd minidlna; ./autogen.sh )

minidlna-configure:
	cd minidlna && \
	LIBS="-ldl -lm -lpthread -lz -lexif -ljpeg -lsqlite3 -lid3tag -lFLAC -lvorbis -logg -lavformat -lavcodec -lavutil" \
	CFLAGS="-Os -Wall $(EXTRACFLAGS) -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -ffunction-sections -fdata-sections \
		$(if $(RTN65U,y),-DRTN56U=1) $(if $(or $(RTN66U), $(RTAC66U)),-DRTN66U=1)" \
		$(if $(RTAC68U,y),-I$(SRCBASE)/include) \
	CPPFLAGS="-I$(TOP)/zlib \
		-I$(TOP)/ffmpeg/libavutil -I$(TOP)/ffmpeg/libavcodec -I$(TOP)/ffmpeg/libavformat \
		-I$(TOP)/ffmpeg/libswscale -I$(TOP)/ffmpeg \
		-I$(TOP)/flac/include -I$(TOP)/sqlite -I$(TOP)/jpeg \
		-I$(TOP)/libexif -I$(TOP)/libid3tag -I$(TOP)/libogg/include \
		-I$(TOP)/libvorbis/include -I$(TOP)/shared" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections \
		-L$(TOP)/zlib -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections \
		-L$(TOP)/zlib -L$(TOP)/sqlite/.libs -L$(TOP)/jpeg -L$(TOP)/libvorbis/lib/.libs \
		-L$(TOP)/libogg/src/.libs -L$(TOP)/libexif/libexif/.libs -L$(TOP)/flac/src/libFLAC/.libs \
		-L$(TOP)/ffmpeg/libavutil -L$(TOP)/ffmpeg/libavcodec -L$(TOP)/ffmpeg/libavformat \
		-L$(TOP)/libid3tag/.libs" \
		$(if $(RTAC68U,y),-L$(TOP)/nvram${BCMEX} -lnvram -L$(TOP)/shared -lshared) \
	ac_cv_header_linux_netlink_h=no \
	$(CONFIGURE)  --with-db-path=/tmp/minidlna --with-log-path=/tmp/minidlna \
		--with-os-name=Asuswrt --with-os-version=382.xx --with-os-url="https://asuswrt.lostrealm.ca/" \
		--enable-tivo

minidlna-clean:
	-@$(MAKE) -C minidlna clean
	@rm -f minidlna/Makefile
	@rm -f minidlna/configure

minidlna-install:
	install -D minidlna/minidlnad $(INSTALLDIR)/minidlna/usr/sbin/minidlna
	$(STRIP) $(INSTALLDIR)/minidlna/usr/sbin/minidlna

libgdbm/stamp-h1:
	cd libgdbm && \
	CFLAGS="-Os -Wall $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections" \
	CPPFLAGS="-I$(TOP)/zlib" \
	LDFLAGS="-fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	$(CONFIGURE) --enable-shared --enable-static --prefix=''
	touch $@

libgdbm: libgdbm/stamp-h1
	@$(MAKE) -C libgdbm

libgdbm-clean:
	-@$(MAKE) -C libgdbm clean
	@rm -f libgdbm/stamp-h1

libgdbm-install:
	@$(SEP)
ifneq ($(MEDIA_SERVER_STATIC),y)
	install -D libgdbm/.libs/libgdbm.so.3.0.0 $(INSTALLDIR)/libgdbm/usr/lib/libgdbm.so.3.0.0
	$(STRIP) $(INSTALLDIR)/libgdbm/usr/lib/libgdbm.so.3.0.0
	cd $(INSTALLDIR)/libgdbm/usr/lib && \
		ln -sf libgdbm.so.3.0.0 libgdbm.so.3 && \
		ln -sf libgdbm.so.3.0.0 libgdbm.so
endif

mt-daapd: zlib libid3tag libgdbm
	@$(SEP)
	@$(MAKE) -C mt-daapd CC=$(CC) $(if $(MEDIA_SERVER_STATIC),STATIC=1) all $(PARALLEL_BUILD)

mt-daapd-svn-1696/stamp-h1: zlib libid3tag sqlite
	cd mt-daapd-svn-1696 && $(CONFIGURE) --prefix=/usr \
	CC=$(CC) \
	LDFLAGS="-L$(TOP)/sqlite -L$(TOP)/libid3tag -L$(TOP)/zlib -ldl" \
	CFLAGS="-I$(TOP)/zlib" \
	--with-id3tag=$(TOP)/libid3tag \
	--enable-sqlite3 --with-sqlite3-includes=$(TOP)/sqlite \
	--disable-rpath --disable-dependency-tracking \
	ac_cv_func_setpgrp_void=yes ac_cv_lib_id3tag_id3_file_open=yes ac_cv_lib_sqlite3_sqlite3_open=yes
	touch $@

mt-daapd-svn-1696: mt-daapd-svn-1696/stamp-h1
	@$(SEP)
	@$(MAKE) -C mt-daapd-svn-1696 all

mt-daapd-svn-1696-clean:
	-@$(MAKE) -C mt-daapd-svn-1696 distclean
	@rm -f mt-daapd-svn-1696/stamp-h1

mt-daapd-svn-1696-install:
	rm -rf $(INSTALLDIR)/rom/mt-daapd/admin-root
	rm -rf $(INSTALLDIR)/rom/mt-daapd/plugins
	install -d $(INSTALLDIR)/mt-daapd-svn-1696/rom/mt-daapd/plugins
	cp -rf mt-daapd-svn-1696/admin-root $(INSTALLDIR)/mt-daapd-svn-1696/rom/mt-daapd
	cd $(INSTALLDIR)/mt-daapd-svn-1696/rom/mt-daapd/admin-root && rm -rf `find -name Makefile` && rm -rf `find -name Makefile.in` && rm -rf `find -name Makefile.am`
	cp -rf mt-daapd-svn-1696/src/plugins/.libs/*.so $(INSTALLDIR)/mt-daapd-svn-1696/rom/mt-daapd/plugins
	install -D mt-daapd-svn-1696/src/.libs/mt-daapd $(INSTALLDIR)/mt-daapd-svn-1696/usr/sbin/mt-daapd
	$(STRIP) $(INSTALLDIR)/mt-daapd-svn-1696/usr/sbin/mt-daapd

mDNSResponder:
	@$(SEP)
	@$(MAKE) -C mDNSResponder CC=$(CC) all $(PARALLEL_BUILD)

igmpproxy/Makefile:
	cd igmpproxy && autoreconf -i -f && CFLAGS="-O2 -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
	$(CONFIGURE) --prefix=/usr --disable-dependency-tracking

igmpproxy: igmpproxy/Makefile
	@$(SEP)
	@$(MAKE) -C igmpproxy/src

igmpproxy-install: igmpproxy
	install -D igmpproxy/src/igmpproxy $(INSTALLDIR)/igmpproxy/usr/sbin/igmpproxy
	$(STRIP) $(INSTALLDIR)/igmpproxy/usr/sbin/igmpproxy

igmpproxy-clean:
	[ ! -f igmpproxy/Makefile ] || $(MAKE) -C igmpproxy distclean
	@rm -f igmpproxy/Makefile

pci-utils:
	$(MAKE) -C pciutils-3.5.0 clean
	$(MAKE) -C pciutils-3.5.0

snooper:
ifeq ($(or $(RTCONFIG_QCA),$(RTCONFIG_REALTEK),$(RTCONFIG_ALPINE),$(RTCONFIG_LANTIQ)),y)
	@true
else
	$(MAKE) -C snooper EXTRACFLAGS="-I$(SRCBASE)/include $(EXTRACFLAGS)" \
		SWITCH=$(if $(RTCONFIG_RALINK),ralink,broadcom)
endif

snooper-install:
ifeq ($(or $(RTCONFIG_QCA),$(RTCONFIG_REALTEK),$(RTCONFIG_ALPINE),$(RTCONFIG_LANTIQ)),y)
	@true
else
	install -D snooper/snooper $(INSTALLDIR)/snooper/usr/sbin/snooper
	$(STRIP) $(INSTALLDIR)/snooper/usr/sbin/snooper
endif

snooper-clean:
	$(MAKE) -C snooper clean

udpxy:
	$(MAKE) -C $@ release NO_UDPXREC=yes \
	CDEFS="-O2 -ffunction-sections -fdata-sections $(if $(RTCONFIG_REALTEK),-fno-strict-aliasing,) $(EXTRACFLAGS)" \
	LDFLAGS="-Wl,--gc-sections"

udpxy-install:
	install -D udpxy/udpxy $(INSTALLDIR)/udpxy/usr/sbin/udpxy
	$(STRIP) $(INSTALLDIR)/udpxy/usr/sbin/udpxy

netconf: $(IPTABLES)
	+$(MAKE) LINUXDIR=$(LINUX_INC_DIR) -C netconf

igmp: netconf
	$(MAKE) -C igmp

igmp-install: netconf
	install -d $(TARGETDIR)
	+$(MAKE) -C igmp INSTALLDIR=$(INSTALLDIR) install

igmp-clean:
	$(MAKE) -C igmp clean

wps$(BCMEX)$(EX7): nvram$(BCMEX)$(EX7) shared libupnp$(BCMEX)$(EX7)
ifeq ($(RTCONFIG_WPS),y)
	-rm -rf $(OLD_SRC)/wps
	-ln -s $(SRCBASE)/wps $(OLD_SRC)/wps
	[ ! -f wps$(BCMEX)$(EX7)/Makefile ] || $(MAKE) -C wps$(BCMEX)$(EX7) EXTRA_LDFLAGS=$(EXTRALDFLAGS)
else
	# Prevent to use generic rules"
	@true
endif

wps$(BCMEX)$(EX7)-install:
ifeq ($(RTCONFIG_WPS),y)
	[ ! -f wps$(BCMEX)$(EX7)/Makefile ] || $(MAKE) -C wps$(BCMEX)$(EX7) INSTALLDIR=$(INSTALLDIR) install
else
	# Prevent to use generic rules"
	@true
endif

wps$(BCMEX)$(EX7)-clean:
ifeq ($(RTCONFIG_WPS),y)
	[ ! -f wps$(BCMEX)$(EX7)/Makefile ] || $(MAKE) -C wps$(BCMEX)$(EX7) clean
else
	# Prevent to use generic rules"
	@true
endif

sysstate:
	@$(SEP)
	echo "build sysstate"
	$(MAKE) -C sysstate

sysstate-clean:
	echo "$(RTCONFIG_SYSSTATE)"
	echo "clean sysstate"
	$(MAKE) -C sysstate clean

sysstate-install:
	echo "install sysstate"
	$(MAKE) -C sysstate install

qsr10g_image:
aqr107_image:

CHIP=0
CLM_BLOBS=router
CLM_BLOB_IMAGES := $(CLM_BLOBS:=.clm_bin)
OBJCOPY		:= $(CROSS_COMPILE)objcopy
OBJCOPYFLAGS	:= -R .reginfo -R .note -R .comment -R .mdebug -R .logstrs -R .lognums -S
OBJCOPYBIN	:= $(OBJCOPY) -O binary $(OBJCOPYFLAGS)
CLMBASE=$(SRCBASE_FW)/wl/clm
ifeq ($(wildcard $(CLMBASE)/private),)
%.clm_bin: $(info "CLM In $(CLMBASE)/src/ NOT CREATING new clm_blob ...");
else
%.clm_bin: CHIPID := $(shell if [[ $(CHIP) -gt 5999 ]] ; then echo $(CHIP); else echo $$((0x$(CHIP))); fi)
%.clm_bin: $(CLMBASE)/types/%.clm $(CLMBASE)/private/wlc_clm_data.xml
	@echo ClmCompiling config file $*.clm to $*.clm_bin and $*.clm_bin. Config file content:
	cat $(CLMBASE)/types/$*.clm
	$(SRCBASE_FW)/tools/build/ClmCompiler --clmapi_include_dir $(CLMBASE)/include --bcmwifi_include_dir $(SRCBASE_FW)/shared/bcmwifi/include --config_file $(CLMBASE)/types/$*.clm $(CLMBASE)/private/wlc_clm_data.xml $(CLMBASE)/src/$*.c
	#@echo C compile $*.c to $*.o
	##$(LOUD)$(CC) $(call file_cflags,$*.c) -D_FILENAME_=\"$*.c\" -c -o $*.o $*.c
	$(LOUD)$(CC) $(call file_cflags,$*.c) -D_FILENAME_=\"$*.c\" -I$(CLMBASE)/include -I$(SRCBASE_FW)/shared/bcmwifi/include -c -o $(CLMBASE)/src/$*.o $(CLMBASE)/src/$*.c
	#@echo Linking $*.o to $*.exe
	( \
		echo 'SECTIONS {'; \
		echo '  . = 0;'; \
		echo '  .clmdata : {'; \
		echo '      *(.*.clm_header)'; \
		echo '      *(.rodata.*)'; \
		echo '   }'; \
		echo '}'; \
	) >$(CLMBASE)/src/$*.lds
	@echo Done $*.lds .....
	##$(LD) -S -Map=$*.map -T $*.lds -o $*.exe $*.o
	$(LD) -S -Map=$(CLMBASE)/src/$*.map -T $(CLMBASE)/src/$*.lds -o $(CLMBASE)/src/$*.exe $(CLMBASE)/src/$*.o
	#$(SIZE) $(CLMBASE)/src/$*.exe
	@echo Done $*.exe .....
	#@echo objcopy $*.exe to $*.clm_bin
	$(OBJCOPYBIN) $(CLMBASE)/src/$*.exe $(CLMBASE)/src/$*.clm_bin
	@echo Done $*.clm_bin
	@echo Create clm_blob .....
	python $(SRCBASE_FW)/tools/build/blob.py $(CLMBASE)/src/$*.clm_bin $(CLMBASE)/src/$*.clm_blob '$(CHIPID)'
	$(RM) $(CLMBASE)/src/$*.o $(CLMBASE)/src/$*.lds $(CLMBASE)/src/$*.map $(CLMBASE)/src/$*.exe $(CLMBASE)/src/$*.clm_bin
endif

clmb: $(CLM_BLOB_IMAGES)
	@echo CLM_BLOBS = $(CLM_BLOBS) copy to generated - needed for release builds ....
	@if [ ! -d $(WLAN_SrcBaseA)/generated/wl/clm/src ]; then \
		echo "CLMB: create dir $(WLAN_SrcBaseA)/generated/wl/clm/src" && \
		mkdir -p $(WLAN_SrcBaseA)/generated/wl/clm/src; \
	fi
	$(foreach clmx,$(CLM_BLOBS), \
		if [ -e $(WLAN_SrcBaseA)/wl/clm/src/$(clmx).clm_blob ]; then \
			echo "CLMB: Copy $(WLAN_SrcBaseA)/wl/clm/src/$(clmx).clm_blob to generated" && \
			cp -p $(WLAN_SrcBaseA)/wl/clm/src/$(clmx).clm_blob $(WLAN_SrcBaseA)/generated/wl/clm/src/$(clmx).clm_blob; \
		fi )

clmb-install:
	@echo clmb-install INSTALLDIR = $(INSTALLDIR) ....
	@echo clmb-install TARGETDIR = $(TARGETDIR) ....
	@echo clmb-install WLAN_SrcBaseA = $(WLAN_SrcBaseA) .....
	@echo clmb-install CLM_BLOBS = $(CLM_BLOBS) ....
	rm -rf $(TARGETDIR)/brcm/clm
	-install -d $(TARGETDIR)/brcm/clm
	$(foreach clmx,$(CLM_BLOBS), \
		if [ -e $(WLAN_SrcBaseA)/generated/wl/clm/src/$(clmx).clm_blob ]; then \
			echo "clm-install-check $(WLAN_SrcBaseA)/generated/wl/clm/src/$(clmx).clm_blob" && \
			-install -D $(WLAN_SrcBaseA)/generated/wl/clm/src/$(clmx).clm_blob $(TARGETDIR)/brcm/clm/$(clmx).clm_blob; \
		fi )
	-install -D $(SRCBASE_FW)/wl/clm/src/*.clm_blob $(TARGETDIR)/brcm/clm

clmb-clean:
	@echo clmb-clean CLMBASE = $(CLMBASE) ....
	$(foreach clmx,$(CLM_BLOBS), \
		@echo Removing from $(CLMBASE)/src $(clmx).clm_bin \
		$(RM) $(WLAN_SrcBaseA)/src/$(clmx).clm_bin; \
	)

clmb-clean-all:
	@echo clmb-clean-all CLMBASE = $(CLMBASE) ....
	$(foreach clmb,$(CLM_BLOBS), \
		@echo Removing from $(CLMBASE)/src $(clmb).c $(clmb).clm_bin $(clmb).clm_blob; \
		$(RM) $(CLMBASE)/src/$(clmb).c; \
		$(RM) $(CLMBASE)/src/$(clmb).clm_bin; \
		$(RM) $(TARGETDIR)/etc/brcm/clm/$(clmb).clm_blob )

clm-install:
	@echo clm-install INSTALLDIR = $(INSTALLDIR) ....
	@echo clm-install TARGETDIR = $(TARGETDIR) ....
	-install -d $(TARGETDIR)/brcm/clm
	-install -D $(SRCBASE_FW)/wl/clm/src/*.clm_blob $(TARGETDIR)/brcm/clm

clm-clean:
	$(foreach clmb,$(CLM_BLOBS), \
		@echo Removing from $(CLMBASE)/src $(clmb).c $(clmb).clm_bin $(clmb).clm_blob; \
		$(RM) $(CLMBASE)/src/$(clmb).c; \
		$(RM) $(CLMBASE)/src/$(clmb).clm_bin; \
		$(RM) $(TARGETDIR)/brcm/clm/$(clmb).clm_blob )

SRC_WLEXE = $(SRCBASE_SYS)/wl/exe
SRC_PRE_WLEXE = $(SRCBASE)/wl/exe

.PHONY: wlexe

$(SRC_WLEXE)/../../include/epivers.h :
ifneq ($(wildcard $(SRC_WLEXE)/GNUmakefile),)
	$(MAKE) -C $(SRC_WLEXE)/../../include
endif

wlexe: $(SRC_WLEXE)/../../include/epivers.h
ifeq ($(RTCONFIG_WLEXE),y)
ifneq ($(wildcard $(SRC_WLEXE)/*.c),)

ifneq ($(wildcard $(SRC_WLEXE)/GNUmakefile),)
ifeq ($(ARCH),arm)
	$(MAKE) TARGETARCH=arm_le ARCH_SFX="" TARGET_PREFIX=$(CROSS_COMPILE) -C $(SRC_WLEXE)
else
	$(MAKE) TARGETARCH=mips ARCH_SFX="" TARGET_PREFIX=$(CROSS_COMPILE) -C $(SRC_WLEXE)
endif
	[ -d wlexe ] || install -d wlexe
	install $(SRC_WLEXE)/wl wlexe

ifeq ($(ARCH),arm)
	install $(SRC_WLEXE)/socket_noasd/arm_le/wl_server_socket wlexe
else
	install $(SRC_WLEXE)/socket_noasd/mips/wl_server_socket wlexe
endif
endif

else
	[ -d wlexe ] || install -d wlexe
	install $(SRC_PRE_WLEXE)/prebuilt/wl wlexe

endif
endif
	@true

.PHONY: wbd wbd-install wbd-clean
wbd:
ifeq ($(RTCONFIG_WBD), y)
ifneq ($(wildcard $(WBDSOURCE_DIR)),)
	+$(MAKE) -C $(WBDSOURCE_DIR) CROSS=$(CROSS_COMPILE) EXTRA_LDFLAGS=$(EXTRA_LDFLAGS)
endif
endif

wbd-install:
ifeq ($(RTCONFIG_WBD),y)
	+$(MAKE) -C $(WBDSOURCE_DIR) CROSS=$(CROSS_COMPILE) install
endif

wbd-clean:
ifeq ($(RTCONFIG_WBD),y)
ifneq ($(wildcard $(WBDSOURCE_DIR)),)
	+$(MAKE) -C $(WBDSOURCE_DIR) CROSS=$(CROSS_COMPILE) clean
endif
endif


.PHONY: wlexe-clean
wlexe-clean:
ifneq ($(wildcard $(SRC_WLEXE)/GNUmakefile),)
	$(MAKE) TARGETARCH=arm_le TARGET_PREFIX=$(CROSS_COMPILE) -C $(SRC_WLEXE) clean
	$(RM) wlexe/wl
	$(RM) wlexe/wl_server_socket
endif

.PHONY: wlexe-install
wlexe-install:
ifeq ($(RTCONFIG_WLEXE),y)
ifneq ($(wildcard $(SRC_WLEXE)/GNUmakefile),)
	[ ! -d wlexe ] || install -D -m 755 wlexe/wl $(INSTALLDIR)/wlexe/usr/sbin/wl
	[ ! -d wlexe ] || install -D -m 755 wlexe/wl_server_socket $(INSTALLDIR)/wlexe/usr/sbin/wl_server_socket
	[ ! -d wlexe ] || $(STRIP) $(INSTALLDIR)/wlexe/usr/sbin/wl
	[ ! -d wlexe ] || $(STRIP) $(INSTALLDIR)/wlexe/usr/sbin/wl_server_socket
endif
endif

udev:
	$(MAKE) -C $@ CROSS_COMPILE=$(CROSS_COMPILE) EXTRACFLAGS="$(EXTRACFLAGS)" \
		PROGRAMS=udevtrigger

udev-install: udev
	install -d $(INSTALLDIR)
	install -d $(TARGETDIR)
	$(MAKE) -C udev DESTDIR=$(INSTALLDIR) prefix=/udev install-udevtrigger

hotplug2:
	$(MAKE) -C $@ CROSS_COMPILE=$(CROSS_COMPILE) EXTRACFLAGS="$(EXTRACFLAGS)"

hotplug2-install: hotplug2
	$(MAKE) -C hotplug2 install PREFIX=$(INSTALLDIR)/hotplug2 SUBDIRS=""
	$(MAKE) -C hotplug2/examples install PREFIX=$(INSTALLDIR)/hotplug2/rom KERNELVER=$(LINUX_KERNEL)

hotplug2_hnd:
	+$(MAKE) -C hotplug2_hnd CROSS_COMPILE=$(CROSS_COMPILE)

hotplug2_hnd-install:
	install -d $(TARGETDIR)
	install -d $(INSTALLDIR)/hotplug2
	+$(MAKE) install -C hotplug2_hnd \
		CROSS_COMPILE=$(CROSS_COMPILE) PREFIX=$(INSTALLDIR)/hotplug2_hnd

hotplug2_hnd-clean:
	$(MAKE) -C hotplug2_hnd clean

emf$(BCMEX)$(EX7):
	$(MAKE) -C emf$(BCMEX)$(EX7)/emfconf CROSS=$(CROSS_COMPILE)

emf$(BCMEX)$(EX7)-install:
ifeq ($(RTCONFIG_EMF),y)
	install -d $(TARGETDIR)
	$(MAKE) -C emf$(BCMEX)$(EX7)/emfconf CROSS=$(CROSS_COMPILE) INSTALLDIR=$(INSTALLDIR) install
endif

emf$(BCMEX)$(EX7)-clean:
	-@$(MAKE) -C emf$(BCMEX)$(EX7)/emfconf clean

igs$(BCMEX)$(EX7):
	$(MAKE) -C emf$(BCMEX)$(EX7)/igsconf CROSS=$(CROSS_COMPILE)

igs$(BCMEX)$(EX7)-install:
ifeq ($(RTCONFIG_EMF),y)
	install -d $(TARGETDIR)
	$(MAKE) -C emf$(BCMEX)$(EX7)/igsconf CROSS=$(CROSS_COMPILE) INSTALLDIR=$(INSTALLDIR) install
endif

igs$(BCMEX)$(EX7)-clean:
	-@$(MAKE) -C emf$(BCMEX)$(EX7)/igsconf clean

ifeq ($(HND_ROUTER),y)
EBK_INC=$(TOOL_SYS_INCLUDE_64)
else
EBK_INC=$(TOP)/kernel_header/include
endif

ebtables: kernel_header dummy
	$(MAKE) -C ebtables $(PARALLEL_BUILD) $(if $(RTCONFIG_HND_ROUTER),CC=$(CROSS_COMPILE_64)gcc LD=$(CROSS_COMPILE_64)ld CURRENT_ARCH=aarch64,CC=$(CC) LD=$(LD)) \
	CFLAGS="-Os -Wall -Wunused $(if $(RTCONFIG_HND_ROUTER),-DHND_ROUTER,)" \
	BINDIR="/usr/sbin" LIBDIR="/usr/lib/ebtables" KERNEL_INCLUDES=$(EBK_INC) $(if $(RTCONFIG_IPV6),DO_IPV6=1,) $(if $(OLD_QCA),USE_ARPNAT=1,)

ebtables-install: ebtables
	install -D ebtables/ebtables $(INSTALLDIR)/ebtables/usr/sbin/ebtables

	install -d $(INSTALLDIR)/ebtables/usr/lib
	install -d $(INSTALLDIR)/ebtables/usr/lib/ebtables
	install -D ebtables/*.so $(INSTALLDIR)/ebtables/usr/lib/
	install -D ebtables/extensions/*.so $(INSTALLDIR)/ebtables/usr/lib/ebtables/

ifeq ($(HND_ROUTER),y)
	$(STRIP_64) $(INSTALLDIR)/ebtables/usr/sbin/ebtables
	$(STRIP_64) $(INSTALLDIR)/ebtables/usr/lib/ebtables/*.so
	$(STRIP_64) $(INSTALLDIR)/ebtables/usr/lib/libebt*.so
else
	$(STRIP) $(INSTALLDIR)/ebtables/usr/sbin/ebtables
	$(STRIP) $(INSTALLDIR)/ebtables/usr/lib/ebtables/*.so
	$(STRIP) $(INSTALLDIR)/ebtables/usr/lib/libebt*.so
endif

ebtables-clean:
	-@make -C ebtables clean

lzo/stamp-h1:
	cd lzo && \
	CFLAGS="-O3 -Wall $(EXTRACFLAGS)" $(CONFIGURE) --enable-shared --enable-static \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib
	touch $@

lzo: lzo/stamp-h1
	$(MAKE) -C lzo $(PARALLEL_BUILD) && $(MAKE) $@-stage

lzo-clean:
	-@$(MAKE) -C lzo clean
	@rm -f lzo/stamp-h1

lzo-install:
	install -D lzo/src/.libs/liblzo2.so $(INSTALLDIR)/lzo/usr/lib/liblzo2.so.2
	$(STRIP) $(INSTALLDIR)/lzo/usr/lib/liblzo2.so.2
	cd $(INSTALLDIR)/lzo/usr/lib && ln -sf liblzo2.so.2 liblzo2.so

openpam: openpam/Makefile
	$(MAKE) -C $@ $(PARALLEL_BUILD) && $(MAKE) $@-stage

openpam/Makefile:
	( cd openpam ; \
		LDFLAGS=-ldl \
		$(CONFIGURE) \
		--without-doc --with-pam-unix \
	)

openpam-install: openpam
	install -D openpam/lib/libpam/.libs/libpam.so.2.0.0 $(INSTALLDIR)/openpam/usr/lib/libpam.so.2
	$(STRIP) -s $(INSTALLDIR)/openpam/usr/lib/libpam.so.2
	install -D openpam/modules/pam_unix/.libs/pam_unix.so.2.0.0 $(INSTALLDIR)/openpam/usr/lib/pam_unix.so
	$(STRIP) -s $(INSTALLDIR)/openpam/usr/lib/pam_unix.so

openpam-clean:
	[ ! -f openpam/Makefile ] || $(MAKE) -C openpam distclean
	@rm -f openpam/Makefile

openvpn: shared openssl lzo openpam zlib openvpn/Makefile
	$(MAKE) -C $@ $(PARALLEL_BUILD)

openvpn/Makefile:
ifeq ($(HND_ROUTER),y)
	cd $(SRCBASE) && rm -f kernel
	( cd openvpn ; ./autogen.sh && export OPENSSL_CRYPTO_CFLAGS="-I$(TOP)/openssl -I$(LINUX_INC_DIR)/include"; \
		export OPENSSL_CRYPTO_LIBS="-L$(TOP)/openssl -lcrypto"; \
		export OPENSSL_SSL_CFLAGS="-I$(TOP)/openssl -I$(LINUX_INC_DIR)/include"; \
		export OPENSSL_SSL_LIBS="-L$(TOP)/openssl -lssl"; \
		CFLAGS="-O3 -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections $(if $(RTCONFIG_HTTPS),-I$(TOP)/openssl/include/openssl) -I$(LINUX_INC_DIR)/include" \
		LDFLAGS="$(EXTRALDFLAGS) -L$(TOP)/nvram$(BCMEX)$(EX7) -lnvram $(shell if [[ "$(HND_ROUTER)" = "y" ]] ; then echo "-L$(TOP)/wlcsm -lwlcsm"; else echo ""; fi) -L$(TOP)/shared -lshared $(if $(RTCONFIG_QTN),-L$(TOP)/libqcsapi_client -lqcsapi_client) $(if $(RTCONFIG_HTTPS),-L$(TOP)/openssl -lcrypto -lssl) -L$(TOP)/zlib -lz -lpthread -ldl -L$(TOP)/lzo/src/.libs -L$(TOP)/openpam/lib/libpam/.libs -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CPPFLAGS="-I$(TOP)/lzo/include -I$(TOP)/openssl/include -I$(TOP)/openpam/include -I$(TOP)/shared -I$(SRCBASE)/include -I$(LINUX_INC_DIR)/include" \
		IPROUTE="/bin/ip" IFCONFIG="/sbin/ifconfig" ROUTE="/sbin/route" \
		$(CONFIGURE) --enable-shared --bindir=/usr/sbin --libdir=/usr/lib \
			--disable-debug --enable-management --disable-small \
			--disable-selinux --disable-socks \
			--enable-plugin-auth-pam --enable-iproute2 \
			ac_cv_lib_resolv_gethostbyname=no \
	)
	#cd openvpn; sed -i 's/build_libtool_libs=no/build_libtool_libs=yes/g' libtool;
	cd $(SRCBASE) && ln -sf ../../../../../../../kernel kernel
else
	( cd openvpn ; ./autogen.sh && export OPENSSL_CRYPTO_CFLAGS="-I$(TOP)/openssl"; \
		export OPENSSL_CRYPTO_LIBS="-L$(TOP)/openssl -lcrypto"; \
		export OPENSSL_SSL_CFLAGS="-I$(TOP)/openssl"; \
		export OPENSSL_SSL_LIBS="-L$(TOP)/openssl -lssl"; \
		CFLAGS="-O3 -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections $(if $(RTCONFIG_HTTPS),-I$(TOP)/openssl/include/openssl)" \
		LDFLAGS="$(EXTRALDFLAGS) -L$(TOP)/nvram$(BCMEX)${EX7} ${EXTRA_NV_LDFLAGS} -lnvram -L$(TOP)/shared -lshared $(if $(RTCONFIG_QTN),-L$(TOP)/libqcsapi_client -lqcsapi_client) $(if $(RTCONFIG_HTTPS),-L$(TOP)/openssl -lcrypto -lssl) -L$(TOP)/zlib -lz -lpthread -ldl -L$(TOP)/lzo/src/.libs -L$(TOP)/openpam/lib/libpam/.libs -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CPPFLAGS="-I$(TOP)/lzo/include -I$(TOP)/openssl/include -I$(TOP)/openpam/include -I$(TOP)/shared -I$(SRCBASE)/include" \
		IPROUTE="/usr/sbin/ip" IFCONFIG="/sbin/ifconfig" ROUTE="/sbin/route" \
		$(CONFIGURE) --prefix=/usr --bindir=/usr/sbin --libdir=/usr/lib \
			--disable-debug --enable-management --disable-small \
			--disable-selinux --disable-socks \
			--enable-plugin-auth-pam --enable-iproute2 \
			ac_cv_lib_resolv_gethostbyname=no \
	)
endif

openvpn-clean:
	[ ! -f openvpn/Makefile ] || $(MAKE) -C openvpn clean
	@rm -f openvpn/Makefile

openvpn-install:
	if [ -f openvpn/Makefile ] ; then \
		install -D openvpn/src/openvpn/.libs/openvpn $(INSTALLDIR)/openvpn/usr/sbin/openvpn ; \
		$(STRIP) -s $(INSTALLDIR)/openvpn/usr/sbin/openvpn ; \
		chmod 0500 $(INSTALLDIR)/openvpn/usr/sbin/openvpn ; \
		install -D openvpn/src/plugins/auth-pam/.libs/openvpn-plugin-auth-pam.so $(INSTALLDIR)/openvpn/usr/lib/openvpn-plugin-auth-pam.so ; \
		$(STRIP) -s $(INSTALLDIR)/openvpn/usr/lib/openvpn-plugin-auth-pam.so ; \
		mkdir -p $(TARGETDIR)/rom/$(EX_ROM)/easy-rsa ; \
		install openvpn/easy-rsa/2.0/* $(TARGETDIR)/rom/$(EX_ROM)/easy-rsa ; \
#		install -D openvpn/dh2048.pem $(INSTALLDIR)/openvpn/rom/dh2048.pem ; \
	fi

sdparm-1.02: sdparm-1.02/Makefile
	$(MAKE) -C $@

sdparm-1.02/Makefile: sdparm-1.02/configure
	cd sdparm-1.02 && \
	CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
	LDFLAGS="$(EXTRALDFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	$(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/bin \
		--libdir=/usr/lib

sdparm-1.02/configure:
	cd sdparm-1.02 && ./autogen.sh

sdparm-1.02-install: sdparm-1.02
	@[ ! -d sdparm-1.02 ] || install -D sdparm-1.02/src/sdparm $(INSTALLDIR)/sdparm-1.02/usr/bin/sdparm
	@$(STRIP) $(INSTALLDIR)/sdparm-1.02/usr/bin/sdparm

sdparm-1.02-clean:
	[ ! -f sdparm-1.02/Makefile ] || $(MAKE) -C sdparm-1.02 distclean
	@rm -f sdparm-1.02/Makefile

.PHONY: strace
strace: strace/Makefile
	$(MAKE) -C $@

strace/Makefile: strace/configure
	$(MAKE) strace-configure

strace/configure:
	( cd strace && autoreconf -i -f )

strace-configure:
	( cd strace && $(CONFIGURE) --bindir=/sbin )

strace-install:
	@install -D strace/strace $(INSTALLDIR)/strace/sbin/strace
	@$(STRIP) $(INSTALLDIR)/strace/sbin/strace

strace-clean:
	[ ! -f strace/Makefile ] || $(MAKE) -C strace distclean
	@rm -f strace/Makefile

.PHONY: termcap
termcap: termcap/Makefile
	$(MAKE) -C $@

termcap/Makefile:
	( cd termcap && $(CONFIGURE) && touch termcap.info)

termcap-install:
	# We don't need to install termcap to target filesystem.

termcap-clean:
	[ ! -f termcap/Makefile ] || $(MAKE) -C termcap distclean
	@rm -f termcap/Makefile

.PHONY: gdb
gdb: termcap gdb/Makefile
	$(MAKE) -C $@

gdb/Makefile:
	( cd gdb && \
		TUI_LIBRARY=$(TOP)/termcap/libtermcap.a \
		LDFLAGS=-L$(TOP)/termcap \
		CFLAGS_FOR_BUILD= \
		$(CONFIGURE) --bindir=/sbin \
	)

gdb-install:
	@install -D gdb/gdb/gdb $(INSTALLDIR)/gdb/sbin/gdb
	@install -D gdb/gdb/gdbserver/gdbserver $(INSTALLDIR)/gdb/sbin/gdbserver
	@$(STRIP) $(INSTALLDIR)/gdb/sbin/gdb
	@$(STRIP) $(INSTALLDIR)/gdb/sbin/gdbserver

gdb-clean:
	[ ! -f gdb/Makefile ] || $(MAKE) -C gdb distclean
	@rm -f gdb/Makefile

.PHONY: openlldp
openlldp: openlldp/Makefile
	$(MAKE) -C $@

openlldp/Makefile:
	( cd openlldp && \
		$(CONFIGURE) --bindir=/sbin \
	)

openlldp-install:
	@install -D openlldp/src/lldpd $(INSTALLDIR)/openlldp/sbin/lldpd
	@$(STRIP) $(INSTALLDIR)/openlldp/sbin/lldpd

openlldp-clean:
	[ ! -f openlldp/Makefile ] || $(MAKE) -C openlldp distclean
	@rm -f openlldp/Makefile


nas$(BCMEX)$(EX7): nvram$(BCMEX)$(EX7)

wlconf$(BCMEX)$(EX7): nvram$(BCMEX)$(EX7)

hspot_ap$(BCMEX)$(EX7): shared nvram$(BCMEX)$(EX7)
	@echo "do nothing"

hspot_ap$(BCMEX)$(EX7)-install:
	@echo "do nothing"

eapd$(BCMEX)$(EX7):
	make -C eapd$(BCMEX)$(EX7)/linux

eapd$(BCMEX)$(EX7)-install:
	make -C eapd$(BCMEX)$(EX7)/linux install INSTALLDIR=$(INSTALLDIR)/eapd$(BCMEX)$(EX7)

eapd$(BCMEX)$(EX7)-clean:
	-@cd eapd$(BCMEX)$(EX7)/linux && make clean

libdisk: $(if $(RTCONFIG_PERMISSION_MANAGEMENT),PMS_DBapis)

lighttpd-1.4.39/Makefile:
	@$(SEP)
	cd lighttpd-1.4.39 && ./autogen.sh

lighttpd-1.4.39/stamp-h1: lighttpd-1.4.39/Makefile
ifeq ($(HND_ROUTER),y)
	cd lighttpd-1.4.39 && ./preconfigure-script-hnd
else
	cd lighttpd-1.4.39 && ./preconfigure-script
endif
	@touch $@

lighttpd-1.4.39: shared nvram$(BCMEX)$(EX7) libpasswd libdisk samba-3.5.8 pcre-8.31 libxml2 sqlite libexif openssl curl-7.21.7 $(if $(RTCONFIG_PERMISSION_MANAGEMENT),PMS_DBapis) lighttpd-1.4.39/stamp-h1
	$(MAKE) -C lighttpd-1.4.39 $(PARALLEL_BUILD)

lighttpd-1.4.39-install:
	if [ -f lighttpd-1.4.39/Makefile ] ; then \
		install -D lighttpd-1.4.39/src/.libs/lighttpd $(INSTALLDIR)/lighttpd-1.4.39/usr/sbin/lighttpd ; \
		install -D lighttpd-1.4.39/src/.libs/lighttpd-monitor $(INSTALLDIR)/lighttpd-1.4.39/usr/sbin/lighttpd-monitor ; \
		install -D lighttpd-1.4.39/src/.libs/lighttpd-arpping $(INSTALLDIR)/lighttpd-1.4.39/usr/sbin/lighttpd-arpping ; \
		ln -sf lighttpd $(INSTALLDIR)/lighttpd-1.4.39/usr/sbin/uamsrv ; \
		$(STRIP) -s $(INSTALLDIR)/lighttpd-1.4.39/usr/sbin/lighttpd ; \
		$(STRIP) -s $(INSTALLDIR)/lighttpd-1.4.39/usr/sbin/lighttpd-monitor ; \
		$(STRIP) -s $(INSTALLDIR)/lighttpd-1.4.39/usr/sbin/lighttpd-arpping ; \
		install -d $(INSTALLDIR)/lighttpd-1.4.39/usr/lib/ ; \
		install -D lighttpd-1.4.39/src/.libs/*.so $(INSTALLDIR)/lighttpd-1.4.39/usr/lib/ ; \
		if [ -d lighttpd-1.4.39/prebuild$(EXHND) ] ; then \
			install -D lighttpd-1.4.39/prebuild$(EXHND)/*.so $(INSTALLDIR)/lighttpd-1.4.39/usr/lib/ ; \
		fi ; \
		install -D samba-3.5.8/source3/bin/libsmbclient.so.0 $(INSTALLDIR)/lighttpd-1.4.39/usr/lib/libsmbclient.so.0 ; \
		install -D libexif/libexif/.libs/libexif.so.12 $(INSTALLDIR)/lighttpd-1.4.39/usr/lib/libexif.so.12 ; \
		mkdir -p $(INSTALLDIR)/lighttpd-1.4.39/usr/lighttpd/css && mkdir -p $(INSTALLDIR)/lighttpd-1.4.39/usr/lighttpd/js ; \
		cp -rf lighttpd-1.4.39/external_file/css/ $(INSTALLDIR)/lighttpd-1.4.39/usr/lighttpd ; \
		cp -rf lighttpd-1.4.39/external_file/js/ $(INSTALLDIR)/lighttpd-1.4.39/usr/lighttpd ; \
		cp -f APP-IPK/AiCloud-tmp/CONTROL/control-aicloud $(INSTALLDIR)/lighttpd-1.4.39/usr/lighttpd/control ; \
		cp -f APP-IPK/AiCloud-tmp/CONTROL/control-smartsync-arm $(INSTALLDIR)/lighttpd-1.4.39/usr/lighttpd/smartsync_control ; \
		$(STRIP) -s $(INSTALLDIR)/lighttpd-1.4.39/usr/lib/*.so ; \
		$(STRIP) -s $(INSTALLDIR)/lighttpd-1.4.39/usr/lib/*.so.0 ; \
	fi

lighttpd-1.4.39-clean:
	-@$(MAKE) -C lighttpd-1.4.39 clean
	@rm -f lighttpd-1.4.39/stamp-h1

pcre-8.12/stamp-h1:
	cd pcre-8.12 && \
	CC=$(CC) CXX=$(CXX) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) CFLAGS="-Os -Wall $(EXTRACFLAGS)" \
	./$(CONFIGURE) --prefix=/usr --disable-dependency-tracking
	touch $@
	[ -d pcre-8.12/m4 ] || mkdir pcre-8.12/m4

pcre-8.12: pcre-8.12/stamp-h1

pcre-8.12-install: pcre-8.12
	@$(SEP)
	install -D pcre-8.12/.libs/libpcre.so.0.0.1 $(INSTALLDIR)/pcre-8.12/usr/lib/libpcre.so.0
	$(STRIP) -s $(INSTALLDIR)/pcre-8.12/usr/lib/libpcre.so.0

pcre-8.12-clean:
	-@$(MAKE) -C pcre-8.12 clean
	@rm -f pcre-8.12/stamp-h1

pcre-8.31/stamp-h1:
	cd pcre-8.31 && \
	CC=$(CC) CXX=$(CXX) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) CFLAGS="-Os -Wall $(EXTRACFLAGS)" LIBS="$(EXTRALDFLAGS)" \
	./$(CONFIGURE) --prefix=/usr --disable-dependency-tracking
	touch $@
	[ -d pcre-8.31/m4 ] || mkdir pcre-8.31/m4

pcre-8.31: pcre-8.31/stamp-h1

pcre-8.31-install: pcre-8.31
	@$(SEP)
	install -D pcre-8.31/.libs/libpcre.so.1.0.1 $(INSTALLDIR)/pcre-8.31/usr/lib/libpcre.so.1.0.1
	cd $(INSTALLDIR)/pcre-8.31/usr/lib && ln -sf libpcre.so.1.0.1 libpcre.so.1
	cd $(INSTALLDIR)/pcre-8.31/usr/lib && ln -sf libpcre.so.1.0.1 libpcre.so
	$(STRIP) -s $(INSTALLDIR)/pcre-8.31/usr/lib/libpcre.so.1.0.1
	install -D pcre-8.31/.libs/libpcreposix.so.0.0.1 $(INSTALLDIR)/pcre-8.31/usr/lib/libpcreposix.so.0.0.1
	cd $(INSTALLDIR)/pcre-8.31/usr/lib && ln -sf libpcreposix.so.0.0.1 libpcreposix.so.0
	cd $(INSTALLDIR)/pcre-8.31/usr/lib && ln -sf libpcreposix.so.0.0.1 libpcreposix.so
	$(STRIP) -s $(INSTALLDIR)/pcre-8.31/usr/lib/libpcreposix.so.0.0.1

pcre-8.31-clean:
	-@$(MAKE) -C pcre-8.31 clean
	@rm -f pcre-8.31/stamp-h1

libxml2/stamp-h1:
	cd libxml2 && mkdir -p m4 && autoreconf -i -f && \
	CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) CFLAGS="-Os -Wall $(EXTRACFLAGS)" LDFLAGS="-ldl $(EXTRALDFLAGS)" \
	./$(CONFIGURE) --prefix=/usr --without-python --disable-dependency-tracking
	touch $@

libxml2: libxml2/stamp-h1
	$(MAKE) -C libxml2 all $(PARALLEL_BUILD)

libxml2-install:
	@$(SEP)
	install -D libxml2/.libs/libxml2.so.2 $(INSTALLDIR)/libxml2/usr/lib/libxml2.so.2
	$(STRIP) $(INSTALLDIR)/libxml2/usr/lib/libxml2.so.2
	cd $(INSTALLDIR)/libxml2/usr/lib && ln -sf libxml2.so.2 libxml2.so

libxml2-clean:
	-@$(MAKE) -C libxml2 clean
	@rm -f libxml2/stamp-h1

#libiconv-1.14
libiconv-1.14/stamp-h1:
	cd libiconv-1.14 && \
	$(CONFIGURE) CC=$(CC) --prefix=/usr
	touch $@

libiconv-1.14:libiconv-1.14/stamp-h1
	@$(MAKE) -C $@ CFLAGS="$(CFLAGS) $(if $(filter-out y,$(HND_ROUTER)),-DHND_ROUTER,)" && $(MAKE) $@-stage

libiconv-1.14-install:
	@$(SEP)
	install -D libiconv-1.14/lib/.libs/libiconv.so.2.5.1 $(INSTALLDIR)/libiconv-1.14/usr/lib/libiconv.so.2.5.1
	$(STRIP) $(INSTALLDIR)/libiconv-1.14/usr/lib/libiconv.so.2.5.1
	cd $(INSTALLDIR)/libiconv-1.14/usr/lib && ln -sf libiconv.so.2.5.1 libiconv.so && ln -sf libiconv.so.2.5.1 libiconv.so.2
ifeq ($(RTCONFIG_USB_SMS_MODEM),y)
	install -D libiconv-1.14/src/.libs/iconv_no_i18n $(INSTALLDIR)/libiconv-1.14/usr/bin/iconv
	$(STRIP) $(INSTALLDIR)/libiconv-1.14/usr/bin/iconv
endif

libiconv-1.14-clean:
	-@$(MAKE) -C libiconv-1.14 clean
	@rm -f libiconv-1.14/stamp-h1

#smartsync_api add by tina
smartsync_api:
	$(MAKE) -C smartsync_api all

smartsync_api-install:
	@$(SEP)
	install -D smartsync_api/libsmartsync_api.so $(INSTALLDIR)/smartsync_api/usr/lib/libsmartsync_api.so
	$(STRIP) $(INSTALLDIR)/smartsync_api/usr/lib/libsmartsync_api.so

smartsync_api-clean:
	-@$(MAKE) -C smartsync_api clean

#ftpclient
ftpclient/stamp-h1:
	@$(MAKE) -C ftpclient

ftpclient:ftpclient/stamp-h1 libiconv-1.14

ftpclient-install:
	@$(SEP)
	install -D ftpclient/ftpclient $(INSTALLDIR)/ftpclient/usr/sbin/ftpclient
	$(STRIP) $(INSTALLDIR)/ftpclient/usr/sbin/ftpclient
ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(HND_ROUTER),y)
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
else
	install -D $(shell dirname $(shell which $(CXX)))/../arm-brcm-linux-uclibcgnueabi/lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
endif
else
	install -D $(shell dirname $(shell which $(CXX)))/../lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
endif

ftpclient-clean:
	-@$(MAKE) -C ftpclient clean

#sambaclient
sambaclient/stamp-h1:
	touch $@

sambaclient: sambaclient/stamp-h1
	@$(MAKE) -C sambaclient

sambaclient-install:
	@$(SEP)
	install -D sambaclient/sambaclient $(INSTALLDIR)/sambaclient/usr/sbin/sambaclient
	$(STRIP) $(INSTALLDIR)/sambaclient/usr/sbin/sambaclient
ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(HND_ROUTER),y)
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
else
	install -D $(shell dirname $(shell which $(CXX)))/../arm-brcm-linux-uclibcgnueabi/lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
endif
else
	install -D $(shell dirname $(shell which $(CXX)))/../lib/libstdc++.so.6 $(INSTALLDIR)/sambaclient/usr/lib/libstdc++.so.6
endif

sambaclient-clean:
	-@$(MAKE) -C sambaclient clean
	rm -rf sambaclient/stamp-h1

#usbclient
usbclient/stamp-h1:
	touch $@

usbclient: usbclient/stamp-h1
	@$(MAKE) -C usbclient

usbclient-install:
	@$(SEP)
	install -D usbclient/usbclient $(INSTALLDIR)/usbclient/usr/sbin/usbclient
	$(STRIP) $(INSTALLDIR)/usbclient/usr/sbin/usbclient
ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(HND_ROUTER),y)
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
else
	install -D $(shell dirname $(shell which $(CXX)))/../arm-brcm-linux-uclibcgnueabi/lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
endif
else
	install -D $(shell dirname $(shell which $(CXX)))/../lib/libstdc++.so.6 $(INSTALLDIR)/usbclient/usr/lib/libstdc++.so.6
endif

usbclient-clean:
	-@$(MAKE) -C usbclient clean
	rm -rf usbclient/stamp-h1

##strongswan/IPSEC
strongswan-5.2.1/stamp-h1:
	cd strongswan-5.2.1 && autoreconf -i -f && \
	$(if $(RTCONFIG_HND_ROUTER),$(CONFIGURE_64) CC=$(CROSS_COMPILE_64)gcc RANLIB=$(CROSS_COMPILE_64)ranlib, $(CONFIGURE) CC=$(CC)) \
	--prefix=/usr/ --sysconfdir=/etc/ --localstatedir=/var/ \
	--bindir=/usr/sbin --libdir=/usr/lib \
	--libexecdir=/usr/lib --with-ipsecdir=/usr/lib/ipsec --with-user=admin \
	--disable-gmp --enable-openssl --enable-agent --enable-eap-peap \
	--enable-eap-md5 --enable-eap-mschapv2 --enable-eap-identity \
	--with-strongswan-conf=/etc/strongswan.conf --enable-md4 --enable-acert \
	--enable-cmd --enable-eap-tls --enable-libipsec --disable-static \
	CFLAGS="-O3 -I$(STAGEDIR)/usr/include -I$(TOP)/openssl/include $(if $(RTCONFIG_HND_ROUTER),-DHND_ROUTER,)" \
	LDFLAGS="$(if $(RTCONFIG_HND_ROUTER), -L$(TOP)/strongswan-5.2.1, -L$(STAGEDIR)/usr/lib -L$(TOP)/openssl)" \
	LIBS="-lpthread -ldl -lm"
	touch $@

strongswan-5.2.1: strongswan-5.2.1/stamp-h1
	$(MAKE) $(PARALLEL_BUILD) -C strongswan-5.2.1

strongswan-5.2.1-install: strongswan-5.2.1
	@$(SEP)
	@$(MAKE) -C strongswan-5.2.1 install DESTDIR=$(INSTALLDIR)/strongswan-5.2.1/ ; chmod 755 $(INSTALLDIR)/strongswan-5.2.1/usr/lib
	@rm -rf $(INSTALLDIR)/strongswan-5.2.1/usr/etc
	@rm -f $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/*.la
	@rm -f $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/plugins/*.la
ifeq ($(HND_ROUTER),y)
	$(STRIP_64) $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/*.so
	$(STRIP_64) $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/starter
	$(STRIP_64) $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/stroke
	$(STRIP_64) $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/scepclient
	$(STRIP_64) $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/charon
	$(STRIP_64) $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/plugins/*.so
else
	$(STRIP) $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/*.so
	$(STRIP) $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/starter
	$(STRIP) $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/stroke
	$(STRIP) $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/scepclient
	$(STRIP) $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/charon
	$(STRIP) $(INSTALLDIR)/strongswan-5.2.1/usr/lib/ipsec/plugins/*.so
endif

	mv $(INSTALLDIR)/strongswan-5.2.1/etc $(INSTALLDIR)/strongswan-5.2.1/usr/

strongswan-5.2.1-clean:
	-@$(MAKE) -C strongswan-5.2.1 clean
	@rm -f strongswan-5.2.1/stamp-h1
	@rm -rf $(INSTALLDIR)/strongswan-5.2.1/
	@rm -rf $(TARGETDIR)/usr/sbin/strongswan-5.2.1/*
	@rm -rf $(TARGETDIR)/usr/lib/strongswan-5.2.1/*

sw-hw-auth:
	cd sw-hw-auth && \
	CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) CFLAGS="-Os -Wall $(EXTRACFLAGS)" \
	make all

sw-hw-auth-clean:
	cd sw-hw-auth && \
	CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) CFLAGS="-Os -Wall $(EXTRACFLAGS)" \
        make clean

#add by alan
#add install neon
neon/stamp-h1 neon/config.h:
	cd neon && \
	$(CONFIGURE) CC=$(CC) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
		--enable-shared --disable-static --disable-nls --with-zlib --with-ssl=openssl \
		--with-libs=$(TOP)/openssl:$(TOP)/libxml2 \
		CFLAGS='-I$(STAGEDIR)/usr/include -I$(TOP)/openssl/include' \
		LDFLAGS='-L$(TOP)/libxml2/.libs -L$(STAGEDIR)/usr/lib -L$(TOP)/openssl' \
		LIBS='-lxml2 -lssl -lcrypto -lpthread -ldl -lm' \
		XML2_CONFIG='$(TOP)/libxml2/xml2-config'
	cp -f neon/config.h neon/src/config.h
	touch $@

neon: libxml2 neon/stamp-h1 neon/config.h
	$(MAKE) -C neon && $(MAKE) $@-stage

neon-install:
	@$(SEP)
	install -D neon/src/.libs/libneon.so.27.2.6 $(INSTALLDIR)/neon/usr/lib/libneon.so.27.2.6
	$(STRIP) $(INSTALLDIR)/neon/usr/lib/libneon.so.27.2.6
	cd $(INSTALLDIR)/neon/usr/lib && ln -sf libneon.so.27.2.6 libneon.so && ln -sf libneon.so.27.2.6 libneon.so.27

neon-clean:
	-@$(MAKE) -C neon clean
	@rm -f neon/stamp-h1

neon-stage:
	@$(MAKE) -C neon install DESTDIR=$(STAGEDIR)
	@cp -f neon/src/webdav_base.h $(STAGEDIR)/usr/include

#webdav_client
webdav_client/stamp-h1:
	touch $@

webdav_client: webdav_client/stamp-h1 nvram$(BCMEX)$(EX7) zlib libxml2 neon
	@$(MAKE) -C webdav_client

webdav_client-install: webdav_client
	@$(SEP)
	install -D webdav_client/webdav_client $(INSTALLDIR)/webdav_client/usr/sbin/webdav_client
	$(STRIP) $(INSTALLDIR)/webdav_client/usr/sbin/webdav_client

webdav_client-clean:
	-@$(MAKE) -C webdav_client clean
	rm -rf webdav_client/stamp-h1

#fbwifi
fb_wifi/stamp-h1:
	touch $@

fb_wifi: curl-7.21.7 fb_wifi/stamp-h1
	@$(MAKE) -C fb_wifi

fb_wifi-install: fb_wifi
	@$(SEP)
	install -D fb_wifi/fb_wifi_check $(INSTALLDIR)/fb_wifi/usr/sbin/fb_wifi_check
	install -D fb_wifi/fb_wifi_register $(INSTALLDIR)/fb_wifi/usr/sbin/fb_wifi_register
	install -D fb_wifi/libfbwifi.so $(INSTALLDIR)/fb_wifi/usr/lib/libfbwifi.so
	$(STRIP) $(INSTALLDIR)/fb_wifi/usr/sbin/fb_wifi_check
	$(STRIP) $(INSTALLDIR)/fb_wifi/usr/sbin/fb_wifi_register
	$(STRIP) $(INSTALLDIR)/fb_wifi/usr/lib/libfbwifi.so

fb_wifi-clean:
	-@$(MAKE) -C fb_wifi clean
	rm -rf fb_wifi/stamp-h1

#add by gauss
#add install libcurl
curl-7.21.7: curl-7.21.7/Makefile
	@$(MAKE) -C $@ $(PARALLEL_BUILD) && $(MAKE) $@-stage && sed 's|/usr/lib|$(STAGEDIR)/usr/lib|g' -i $(STAGEDIR)/usr/lib/libcurl.la

curl-7.21.7/Makefile: curl/configure
	@cd curl && $(CONFIGURE) CC=$(CC) \
		CFLAGS="-Os -Wall -ffunction-sections -fdata-sections" \
		--prefix=/usr --bindir=/usr/sbin --libdir=/usr/lib \
		--enable-http --with-ssl=$(TOP)/openssl/ssl \
		$(if $(RTCONFIG_IPV6),--enable-ipv6) \
		--disable-gopher --disable-dict --disable-telnet \
		--disable-proxy --disable-manual --disable-libcurl-option \
		CPPFLAGS='-I$(TOP)/openssl/include' \
		LDFLAGS='$(LDFLAGS) -L$(TOP)/openssl' LIBS='-lcrypto -lssl -ldl' \
		--with-ca-bundle=/etc/ssl/certs/ca-certificates.crt

curl-7.21.7-install: curl-7.21.7
	@$(SEP)
	install -D curl-7.21.7/lib/.libs/libcurl.so.4.5.0 $(INSTALLDIR)/curl-7.21.7/usr/lib/libcurl.so.4.5.0
	$(STRIP) $(INSTALLDIR)/curl-7.21.7/usr/lib/libcurl.so.4.5.0
	cd $(INSTALLDIR)/curl-7.21.7/usr/lib && ln -sf libcurl.so.4.5.0 libcurl.so && ln -sf libcurl.so.4.5.0 libcurl.so.4
#ifeq ($(RTCONFIG_SPEEDTEST),y)
	# for speedtest
	install -D curl-7.21.7/src/.libs/curl $(INSTALLDIR)/curl-7.21.7/usr/sbin/curl
	$(STRIP) $(INSTALLDIR)/curl-7.21.7/usr/sbin/curl
#endif

curl-7.21.7-clean:
	[ ! -f curl-7.21.7/Makefile ] || $(MAKE) -C curl-7.21.7 clean
	@rm -f curl-7.21.7/Makefile

#snmp
net-snmp-5.7.2/stamp-h1:
	cd $(TOP)/net-snmp-5.7.2 && $(CONFIGURE) --prefix=$(INSTALLDIR)/net-snmp-5.7.2/usr \
		--disable-embedded-perl --with-perl-modules=no --disable-debugging --enable-mini-agent \
		--with-transports=UDP --without-kmem-usage --disable-applications --disable-mib-loading \
		--disable-scripts --disable-mibs --disable-manuals --with-openssl=$(TOP)/openssl \
		--with-ldflags="-L$(TOP)/openssl" --with-libs="$(EXTRA_FLAG)" \
		--with-cflags="$(SNMPD_CFLAGS)" --with-default-snmp-version="3" \
		--with-sys-location=Unknown --with-logfile=/var/log/snmpd.log \
		--with-persistent-directory=/var/net-snmp --with-sys-contact=root@localhost \
		--enable-static=no --with-mib-modules="\
		host/hr_device,\
		host/hr_disk,\
		host/hr_filesys,\
		host/hr_network,\
		host/hr_partition,\
		host/hr_proc,\
		host/hr_storage,\
		host/hr_system,\
		if-mib/ifXTable,\
		mibII/at,\
		mibII/icmp,\
		mibII/ifTable,\
		mibII/ip,\
		mibII/snmp_mib,\
		mibII/sysORTable,\
		mibII/system_mib,\
		mibII/tcp,\
		mibII/udp,\
		mibII/vacm_context,\
		mibII/vacm_vars,\
		tunnel,\
		ucd-snmp/disk,\
		ucd-snmp/dlmod,\
		ucd-snmp/extensible,\
		ucd-snmp/loadave,\
		ucd-snmp/memory,\
		ucd-snmp/pass,\
		ucd-snmp/proc,\
		ucd-snmp/vmstat,\
		util_funcs"
	@touch net-snmp-5.7.2/stamp-h1

net-snmp-5.7.2: net-snmp-5.7.2/stamp-h1
	@$(MAKE) -C net-snmp-5.7.2

net-snmp-5.7.2-install:
	@$(MAKE) -C net-snmp-5.7.2 install

net-snmp-5.7.2-clean:
	@[ ! -f net-snmp-5.7.2/Makefile ] || $(MAKE) -C net-snmp-5.7.2 clean
	@rm -f net-snmp-5.7.2/stamp-h1

asuswebstorage/stamp-h1:
	touch $@

asuswebstorage: asuswebstorage/stamp-h1
	@$(MAKE) -C asuswebstorage

asuswebstorage-install:
	@$(SEP)
	install -D asuswebstorage/asuswebstorage $(INSTALLDIR)/asuswebstorage/usr/sbin/asuswebstorage
	$(STRIP) $(INSTALLDIR)/asuswebstorage/usr/sbin/asuswebstorage

asuswebstorage-clean:
	-@$(MAKE) -C asuswebstorage clean
	rm -rf asuswebstorage/stamp-h1

#google_client
google_client/stamp-h1:
	touch $@

google_client: google_client/stamp-h1
	@$(MAKE) -C google_client

google_client-install:
	@$(SEP)
	install -D google_client/google_client $(INSTALLDIR)/google_client/usr/sbin/google_client
	$(STRIP) $(INSTALLDIR)/google_client/usr/sbin/google_client

google_client-clean:
	-@$(MAKE) -C google_client clean
	rm -rf google_client/stamp-h1

#dropbox_client
dropbox_client/stamp-h1:
	touch $@

dropbox_client: dropbox_client/stamp-h1
	@$(MAKE) -C dropbox_client

dropbox_client-install:
	@$(SEP)
	install -D dropbox_client/dropbox_client $(INSTALLDIR)/dropbox_client/usr/sbin/dropbox_client
	$(STRIP) $(INSTALLDIR)/dropbox_client/usr/sbin/dropbox_client

dropbox_client-clean:
	-@$(MAKE) -C dropbox_client clean
	rm -rf dropbox_client/stamp-h1

#inotify
inotify:
	@$(MAKE) -C inotify

inotify-install: inotify
	@$(SEP)
	install -D inotify/inotify $(INSTALLDIR)/inotify/usr/sbin/inotify
	$(STRIP) $(INSTALLDIR)/inotify/usr/sbin/inotify

inotify-clean:
	-@$(MAKE) -C inotify clean

wl$(BCMEX)$(EX7):
	-@$(MAKE) -C wl$(BCMEX)$(EX7)

wl$(BCMEX)$(EX7)-install:
	@$(SEP)

ifeq ($(HND_ROUTER),y)
hnd_extra:
	@$(SEP)

hnd_extra-install:
	@$(SEP)
endif

rtl_bin:
	-@$(MAKE) -C rtl_bin

rtl_bin-install:
	@$(SEP)

#libgpg-error-1.10
libgpg-error-1.10/stamp-h1:
	cd $(TOP)/libgpg-error-1.10 && \
	$(CONFIGURE) --prefix=/usr
	touch $@

libgpg-error-1.10: libgpg-error-1.10/stamp-h1
	@$(SEP)
	$(MAKE) -C libgpg-error-1.10

libgpg-error-1.10-install:
	install -D libgpg-error-1.10/src/.libs/libgpg-error.so.0.8.0 $(INSTALLDIR)/libgpg-error-1.10/usr/lib/libgpg-error.so.0.8.0
	$(STRIP) $(INSTALLDIR)/libgpg-error-1.10/usr/lib/libgpg-error.so.0.8.0
	cd $(INSTALLDIR)/libgpg-error-1.10/usr/lib && ln -sf libgpg-error.so.0.8.0 libgpg-error.so.0

libgpg-error-1.10-clean:
	-@$(MAKE) -C libgpg-error-1.10 clean
	@rm -f libgpg-error-1.10/stamp-h1

#libgcrypt-1.5.1
libgcrypt-1.5.1/stamp-h1: libgpg-error-1.10
	cd $(TOP)/libgcrypt-1.5.1 && \
	$(CONFIGURE) --prefix=/usr --with-gpg-error-prefix=$(TOP)/libgpg-error-1.10/src \
	LDFLAGS="-L$(TOP)/libgpg-error-1.10/src/.libs"
	touch $@

libgcrypt-1.5.1: libgcrypt-1.5.1/stamp-h1
	@$(SEP)
	@$(MAKE) -C libgcrypt-1.5.1

libgcrypt-1.5.1-install:
	install -D libgcrypt-1.5.1/src/.libs/libgcrypt.so.11.8.0 $(INSTALLDIR)/libgcrypt-1.5.1/usr/lib/libgcrypt.so.11.8.0
	$(STRIP) $(INSTALLDIR)/libgcrypt-1.5.1/usr/lib/libgcrypt.so.11.8.0
	cd $(INSTALLDIR)/libgcrypt-1.5.1/usr/lib && ln -sf libgcrypt.so.11.8.0 libgcrypt.so.11

libgcrypt-1.5.1-clean:
	-@$(MAKE) -C libgcrypt-1.5.1 clean
	@rm -f libgcrypt-1.5.1/stamp-h1

#db-4.8.30
db-4.8.30/build_unix/stamp-h1:
	cd $(TOP)/db-4.8.30/build_unix && \
	../dist/$(CONFIGURE) \
	--prefix=/usr \
	--enable-cxx CFLAGS="-Os" \
	--disable-cryptography \
	--disable-hash \
	--disable-queue \
	--disable-replication \
	--disable-statistics \
	--disable-verify \
	--disable-compat185 \
	--disable-cxx \
	--disable-diagnostic \
	--disable-dump185 \
	--disable-java \
	--disable-mingw \
	--disable-o_direct \
	--enable-posixmutexes \
	--disable-smallbuild \
	--disable-tcl \
	--disable-test \
	--disable-uimutexes \
	--enable-umrw \
	--disable-libtool-lock
	touch $@

##	--enable-compat185 --enable-dbm --enable-cxx CFLAGS="-Os" --disable-mutexsupport

db-4.8.30: db-4.8.30/build_unix/stamp-h1
	@$(SEP)
	@$(MAKE) -C db-4.8.30/build_unix $(PARALLEL_BUILD)

db-4.8.30-install:
	install -D db-4.8.30/build_unix/.libs/libdb-4.8.so $(INSTALLDIR)/db-4.8.30/usr/lib/libdb-4.8.so
	$(STRIP) $(INSTALLDIR)/db-4.8.30/usr/lib/libdb-4.8.so
	cd $(INSTALLDIR)/db-4.8.30/usr/lib && ln -sf libdb-4.8.so libdb.so
	cd $(INSTALLDIR)/db-4.8.30/usr/lib && ln -sf libdb-4.8.so libdb-4.so

#	@$(MAKE) -C db-4.8.30/build_unix install

db-4.8.30-clean:
	-@$(MAKE) -C db-4.8.30 clean
	-@$(MAKE) -C db-4.8.30/build_unix clean
	@rm -f db-4.8.30/build_unix/stamp-h1

netatalk-3.0.5-dep: openssl libgcrypt-1.5.1 db-4.8.30

netaconfig netatalk-3.0.5/stamp-h1:
	if [ ! -f netatalk-3.0.5/stamp-h1 ] ; then \
		$(MAKE) netatalk-3.0.5-dep ; \
		touch $(TOP)/netatalk-3.0.5/* ; \
		cd $(TOP)/netatalk-3.0.5 && \
		$(CONFIGURE) CPPFLAGS="-I$(TOP)/db-4.8.30/build_unix -I$(TOP)/libgcrypt-1.5.1/src" \
		LDFLAGS="-L$(TOP)/db-4.8.30/build_unix/.libs -L$(TOP)/libgcrypt-1.5.1/src/.libs" \
		LIBS="-L$(TOP)/libgpg-error-1.10/src/.libs -lgpg-error -L$(TOP)/libgcrypt-1.5.1/src/.libs -lgcrypt" \
		ac_cv_path_LIBGCRYPT_CONFIG=$(TOP)/libgcrypt-1.5.1/src/libgcrypt-config \
		--without-kerberos \
		--with-bdb=$(TOP)/db-4.8.30/build_unix \
		--prefix=/usr \
		--with-dtrace=no \
		--with-ssl-dir=$(TOP)/openssl \
		--with-libgcrypt-dir=$(TOP)/libgcrypt-1.5.1 \
		--with-ld-library-path=$(SRCBASE)/toolchains/hndtools-arm-linux-2.6.36-uclibc-4.5.3/lib ; \
		touch stamp-h1 ; \
	fi

netatalk-3.0.5: netatalk-3.0.5/stamp-h1
	@$(SEP)
	@$(MAKE) -C netatalk-3.0.5 $(PARALLEL_BUILD)

netatalk-3.0.5-install:
	if [ -f netatalk-3.0.5/Makefile ] ; then \
		install -D netatalk-3.0.5/etc/afpd/.libs/afpd $(INSTALLDIR)/netatalk-3.0.5/usr/sbin/afpd ; \
		install -D netatalk-3.0.5/etc/cnid_dbd/.libs/cnid_dbd $(INSTALLDIR)/netatalk-3.0.5/usr/sbin/cnid_dbd ; \
		install -D netatalk-3.0.5/etc/cnid_dbd/.libs/cnid_metad $(INSTALLDIR)/netatalk-3.0.5/usr/sbin/cnid_metad ; \
		if [ -e $(TOP)/netatalk-3.0.5/bin/ad/.libs/ad ] ; then \
			install -D netatalk-3.0.5/bin/ad/.libs/ad $(INSTALLDIR)/netatalk-3.0.5/usr/bin/ad ; \
		fi ; \
		install -D netatalk-3.0.5/bin/afppasswd/.libs/afppasswd $(INSTALLDIR)/netatalk-3.0.5/usr/bin/afppasswd ; \
		install -D netatalk-3.0.5/etc/cnid_dbd/.libs/dbd $(INSTALLDIR)/netatalk-3.0.5/usr/bin/dbd ; \
		install -D netatalk-3.0.5/bin/uniconv/.libs/uniconv $(INSTALLDIR)/netatalk-3.0.5/usr/bin/uniconv ; \
		install -D netatalk-3.0.5/libatalk/.libs/libatalk.so.6.0.0 $(INSTALLDIR)/netatalk-3.0.5/usr/lib/libatalk.so.6.0.0 ; \
		install -D netatalk-3.0.5/etc/uams/.libs/uams_randnum.so $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk/uams_randnum.so ; \
		install -D netatalk-3.0.5/etc/uams/.libs/uams_passwd.so $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk/uams_passwd.so ; \
		install -D netatalk-3.0.5/etc/uams/.libs/uams_guest.so $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk/uams_guest.so ; \
		install -D netatalk-3.0.5/etc/uams/.libs/uams_dhx_passwd.so $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk/uams_dhx_passwd.so ; \
		install -D netatalk-3.0.5/etc/uams/.libs/uams_dhx2_passwd.so $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk/uams_dhx2_passwd.so ; \
		$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/sbin/afpd ; \
		$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/sbin/cnid_dbd ; \
		$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/sbin/cnid_metad ; \
		if [ -e $(TOP)/netatalk-3.0.5/bin/ad/.libs/ad ] ; then \
			$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/bin/ad ; \
		fi ; \
		$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/bin/afppasswd ; \
		$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/bin/dbd ; \
		$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/bin/uniconv ; \
		$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/lib/libatalk.so.6.0.0 ; \
		$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk/uams_randnum.so ; \
		$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk/uams_passwd.so ; \
		$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk/uams_guest.so ; \
		$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk/uams_dhx_passwd.so ; \
		$(STRIP) $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk/uams_dhx2_passwd.so ; \
		cd $(INSTALLDIR)/netatalk-3.0.5/usr/lib && ln -sf libatalk.so.6.0.0 libatalk.so.6 ; \
		cd $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk && ln -sf uams_passwd.so uams_clrtxt.so ; \
		cd $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk && ln -sf uams_dhx_passwd.so uams_dhx.so ; \
		cd $(INSTALLDIR)/netatalk-3.0.5/usr/lib/netatalk && ln -sf uams_dhx2_passwd.so uams_dhx2.so ; \
	fi

netatalk-3.0.5-clean:
	-@$(MAKE) -C netatalk-3.0.5 distclean
	@rm -f netatalk-3.0.5/stamp-h1

hndck:
	echo $(obj-y)
expat-2.0.1/stamp-h1:
	cd $(TOP)/expat-2.0.1 && \
	$(CONFIGURE) --build=i686-linux --prefix=/usr
	touch $@

expat-2.0.1: expat-2.0.1/stamp-h1
	@$(SEP)
	@$(MAKE) -C expat-2.0.1 $(PARALLEL_BUILD) && $(MAKE) $@-stage
	sed 's|/usr/lib|$(STAGEDIR)/usr/lib|g' -i $(STAGEDIR)/usr/lib/libexpat.la

expat-2.0.1-install:
	install -D expat-2.0.1/.libs/libexpat.so.1.5.2 $(INSTALLDIR)/expat-2.0.1/usr/lib/libexpat.so.1.5.2
	$(STRIP) $(INSTALLDIR)/expat-2.0.1/usr/lib/libexpat.so.1.5.2
	cd $(INSTALLDIR)/expat-2.0.1/usr/lib && ln -sf libexpat.so.1.5.2 libexpat.so.1

expat-2.0.1-clean:
	-@$(MAKE) -C expat-2.0.1 clean
	@rm -f expat-2.0.1/stamp-h1

#avahi-0.6.31
avahi-0.6.31: shared nvram$(BCMEX)$(EX7) expat-2.0.1 libdaemon avahi-0.6.31/Makefile
	@$(SEP)
	@$(MAKE) -C avahi-0.6.31 $(PARALLEL_BUILD)

avahi-0.6.31/Makefile:
	( cd avahi-0.6.31 ; $(CONFIGURE) LDFLAGS="$(LDFLAGS) -L$(STAGEDIR)/usr/lib -ldl -lpthread $(EXTRA_LDFLAGS)" \
		CFLAGS="$(CFLAGS) -I$(STAGEDIR)/usr/include -I$(TOP)/shared -I$(SRCBASE)/include" \
		--prefix=/usr --with-distro=archlinux \
		--disable-glib --disable-gobject --disable-qt3 --disable-qt4 --disable-gtk \
		--disable-dbus --disable-gdbm --disable-python \
		--disable-pygtk --disable-python-dbus --disable-mono --disable-monodoc \
		--disable-gtk3 --with-xml=expat \
		--disable-silent-rules \
		LIBDAEMON_LIBS="-L$(STAGEDIR)/usr/lib -ldaemon -L$(TOP)/nvram$(BCMEX)${EX7} ${EXTRA_NV_LDFLAGS} -lnvram -L$(TOP)/shared -lshared $(if $(RTCONFIG_QTN),-L$(TOP)/libqcsapi_client -lqcsapi_client)" \
		LIBDAEMON_CFLAGS="-I$(STAGEDIR)/usr/include" --disable-autoipd --disable-stack-protector --with-avahi-user="admin" --with-avahi-group="root" \
	)
	touch $@

avahi-0.6.31-install:
	if [ -f avahi-0.6.31/Makefile ] ; then \
		install -D avahi-0.6.31/avahi-daemon/.libs/avahi-daemon $(INSTALLDIR)/avahi-0.6.31/usr/sbin/avahi-daemon ; \
		install -D avahi-0.6.31/avahi-common/.libs/libavahi-common.so.3.5.3 $(INSTALLDIR)/avahi-0.6.31/usr/lib/libavahi-common.so.3.5.3 ; \
		install -D avahi-0.6.31/avahi-core/.libs/libavahi-core.so.7.0.2 $(INSTALLDIR)/avahi-0.6.31/usr/lib/libavahi-core.so.7.0.2 ; \
		$(STRIP) $(INSTALLDIR)/avahi-0.6.31/usr/sbin/avahi-daemon ; \
		$(STRIP) $(INSTALLDIR)/avahi-0.6.31/usr/lib/libavahi-common.so.3.5.3 ; \
		$(STRIP) $(INSTALLDIR)/avahi-0.6.31/usr/lib/libavahi-core.so.7.0.2 ; \
		cd $(INSTALLDIR)/avahi-0.6.31/usr/lib && ln -sf libavahi-common.so.3.5.3 libavahi-common.so.3 ; \
		cd $(INSTALLDIR)/avahi-0.6.31/usr/lib && ln -sf libavahi-core.so.7.0.2 libavahi-core.so.7 ; \
	fi

avahi-0.6.31-clean:
	[ ! -f avahi-0.6.31/Makefile ] || $(MAKE) -C avahi-0.6.31 clean
	@rm -f avahi-0.6.31/Makefile

madwimax-0.1.1/stamp-h1:
	cd $(TOP)/madwimax-0.1.1 && \
	LDFLAGS="$(LDFLAGS) -ldl" $(CONFIGURE) --prefix=/usr --with-script=wl500g --without-man-pages --with-udev-dir=$(TOP)/madwimax-0.1.1/build libusb1_CFLAGS=-I$(TOP)/libusb10/libusb libusb1_LIBS="-L$(TOP)/libusb10/libusb/.libs -lusb-1.0"
	touch $@

madwimax-0.1.1: libusb10 madwimax-0.1.1/stamp-h1
	@$(SEP)
	@$(MAKE) -C madwimax-0.1.1 all

madwimax-0.1.1-install: madwimax-0.1.1
	#install -D madwimax-0.1.1/scripts/events/event.sh $(INSTALLDIR)/madwimax-0.1.1/usr/sbin/event.sh
	install -D madwimax-0.1.1/src/madwimax $(INSTALLDIR)/madwimax-0.1.1/usr/sbin/madwimax
	$(STRIP) $(INSTALLDIR)/madwimax-0.1.1/usr/sbin/madwimax

madwimax-0.1.1-clean:
	-@$(MAKE) -C madwimax-0.1.1 clean
	@cd $(TOP)/madwimax-0.1.1 && rm -f config.log config.status
	@cd $(TOP)/madwimax-0.1.1 && rm -f `find -name Makefile`
	@cd $(TOP)/madwimax-0.1.1 && rm -rf include/config.h scripts/udev/z60_madwimax.rules src/.deps
	@rm -f madwimax-0.1.1/stamp-h1

ufsd: kernel_header kernel
	@$(MAKE) -C ufsd all

ufsd-install: ufsd
	@$(MAKE) -C ufsd install INSTALLDIR=$(INSTALLDIR)/ufsd

tuxera:
	@echo "do nothing"

tuxera-install:
	@echo [tuxera-install]
	@[ ! -d tuxera ] || $(MAKE) -C tuxera install INSTALLDIR=$(INSTALLDIR)/tuxera

netstat-nat-clean:
	-@$(MAKE) -C netstat-nat clean
	@rm -f netstat-nat/Makefile
	@rm -f netstat-nat/stamp-h1

netstat-nat-install: netstat-nat
	install -D netstat-nat/netstat-nat $(INSTALLDIR)/netstat-nat/usr/sbin/netstat-nat
	$(STRIP) -s $(INSTALLDIR)/netstat-nat/usr/sbin/netstat-nat

netstat-nat/stamp-h1:
	cd netstat-nat && autoreconf -fi && \
	CC=$(CC) CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
		$(CONFIGURE)
	@touch $@

netstat-nat: netstat-nat/stamp-h1
	$(MAKE) -C netstat-nat

sd-idle: sd-idle/sd-idle-2.6
	cd sd-idle && \
	CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
	$(CC) sd-idle-2.6.c -o sd-idle-2.6

sd-idle-install: sd-idle-2.6
	install -D sd-idle/sd-idle-2.6 $(INSTALLDIR)/sd-idle/usr/sbin/sd-idle-2.6
	$(STRIP) -s $(INSTALLDIR)/sd-idle/usr/sbin/sd-idle-2.6

sd-idle-clean:
	@rm -f sd-idle/sd-idle-2.6


tftp:
	cd tftp && CC=$(CC) && $(MAKE)

tftp-clean:
	cd tftp && $(MAKE) clean

tftp-install:
	# TPTP server
	install -D tftp/tftpd $(INSTALLDIR)/tftp/usr/sbin/tftpd
	$(STRIP) -s $(INSTALLDIR)/tftp/usr/sbin/tftpd
	# TFTP client
	#install -D tftp/tftpc $(INSTALLDIR)/tftp/usr/sbin/tftpc
	#$(STRIP) -s $(INSTALLDIR)/tftp/usr/sbin/tftpc

tftp-hpa:
	@$(SEP)
	$(MAKE) -C $@

tftp-hpa-install: tftp-hpa
	install -D -m 755 tftp-hpa/tftpd/tftpd $(INSTALLDIR)/tftp-hpa/usr/sbin/in.tftpd
	$(STRIP) $(INSTALLDIR)/tftp-hpa/usr/sbin/in.tftpd

tftp-hpa-clean:
	[ ! -f tftp-hpa/Makefile ] || $(MAKE) -C tftp-hpa distclean

bwdpi_source: $(if $(RTCONFIG_HND_ROUTER),bcm_flashutil bcm_util bcm_boardctl)
	cd bwdpi_source && CC=$(CC) && $(MAKE) all

bwdpi_source-clean:
	cd bwdpi_source && $(MAKE) clean

bwdpi_source-install: bwdpi_source
	cd bwdpi_source && CC=$(CC) && $(MAKE) install

traffic_limiter: sqlite
	cd traffic_limiter && CC=$(CC) && $(MAKE)

traffic_limiter-clean:
	cd traffic_limiter && $(MAKE) clean

traffic_limiter-install: traffic_limiter

coovachilli: coovachilli/Makefile
	@$(MAKE) -C coovachilli $(PARALLEL_BUILD)

coovachilli/Makefile:
	$(MAKE) coovachilli-configure

coovachilli-configure:
	cd coovachilli && autoreconf -fi && \
	$(CONFIGURE) CC=$(CC) --prefix="" --exec_prefix=/usr --enable-shared=no --with-openssl --enable-multilan --with-ipv6 \
	CFLAGS="$(EXTRACFLAGS) -g -Os -pipe -funit-at-a-time -Drpl_malloc=malloc -I$(SRCBASE) -I$(SRCBASE)/include -I. -I$(TOP)/shared -I$(SRCBASE)/router/openssl/include" \
	LDFLAGS="-lgcc_s -L$(TOP)/nvram${BCMEX} -L$(TOP)/shared -lshared -lpthread -L$(SRCBASE)/router/openssl" \
	LIBS="-lnvram -lcrypto -lssl"

coovachilli-install:coovachilli
	@$(SEP)
	install -D coovachilli/src/chilli $(INSTALLDIR)/coovachilli/usr/sbin/chilli
	install -D coovachilli/src/chilli_opt $(INSTALLDIR)/coovachilli/usr/sbin/chilli_opt
	install -D coovachilli/src/chilli_query $(INSTALLDIR)/coovachilli/usr/sbin/chilli_query
	install -D coovachilli/src/chilli_radconfig $(INSTALLDIR)/coovachilli/usr/sbin/chilli_radconfig
	install -D coovachilli/src/chilli_response $(INSTALLDIR)/coovachilli/usr/sbin/chilli_response
	$(STRIP) $(INSTALLDIR)/coovachilli/usr/sbin/*

coovachilli-clean:
	-@$(MAKE) -C coovachilli clean

#talloc
talloc-2.1.1/stamp-h1:
ifeq ($(RTCONFIG_FREERADIUS),y)
ifeq ($(or $(wildcard talloc-2.1.1/$(SRCBASEDIR).answer),$(shell which qemu-${ARCH})),)
	$(error To configure talloc, please install qemu!)
endif
	echo $(CONFIGURE)
	cd talloc-2.1.1 && \
	./configure --cross-compile --without-gettext \
		$(if $(wildcard talloc-2.1.1/$(SRCBASEDIR).answer),--cross-answers=$(SRCBASEDIR).answer,--cross-execute="qemu-${ARCH} -L $(TOOLS)")
	touch $@
endif

talloc-2.1.1: talloc-2.1.1/stamp-h1
ifeq ($(RTCONFIG_FREERADIUS),y)
	@$(MAKE) -C talloc-2.1.1
	cd $(TOP)/talloc-2.1.1/bin/default && ln -sf libtalloc.so libtalloc.so.2
endif

talloc-2.1.1-install: talloc-2.1.1
ifeq ($(RTCONFIG_FREERADIUS),y)
	@$(SEP)
	install -D talloc-2.1.1/bin/default/libtalloc.so $(INSTALLDIR)/talloc-2.1.1/usr/lib/libtalloc.so
	$(STRIP) $(INSTALLDIR)/talloc-2.1.1/usr/lib/libtalloc.so
	cd $(INSTALLDIR)/talloc-2.1.1/usr/lib && ln -sf libtalloc.so libtalloc.so.2
endif

talloc-2.1.1-clean:
ifeq ($(RTCONFIG_FREERADIUS),y)
	-@$(MAKE) -C talloc-2.1.1 clean
endif
.PHONY: talloc-2.1.1-clean

#freeradius
freeradius-server-3.0.0/stamp-h1:
	cd freeradius-server-3.0.0 && ./autogen.sh && \
	$(CONFIGURE) CC=$(CC)  CFLAGS="$(EXTRACFLAGS) -I$(SRCBASE) -I$(SRCBASE)/include -I$(TOP)/pcrc-8.31" \
	LDFLAGS="$(EXTRACFLAGS) -L$(TOP)/pcre-8.31/.libs -lpcre -L$(TOP)/talloc-2.1.1/bin/default -Wl,-rpath=$(TOP)/talloc-2.1.1/bin/default -lpthread -ldl" \
	--prefix=/usr/freeradius --datarootdir=/usr/freeradius/share --with-raddbdir=/usr/freeradius/raddb \
	--with-openssl-includes=$(TOP)/openssl/include --with-openssl-libraries=$(TOP)/openssl \
	--with-talloc-lib-dir=$(TOP)/talloc-2.1.1/bin/default --with-talloc-include-dir=$(TOP)/talloc-2.1.1 \
	--with-sqlite-lib-dir=$(TOP)/sqlite/.libs --with-sqlite-include-dir=$(TOP)/sqlite \
	--with-pcre-lib-dir=$(TOP)/pcre-8.31/.libs  --with-pcre-include-dir=$(TOP)/pcrc-8.31
	touch $@

freeradius-server-3.0.0: pcre-8.31 freeradius-server-3.0.0/stamp-h1
	@$(MAKE) -C $@

freeradius-server-3.0.0-install: freeradius-server-3.0.0
	@$(SEP)
	install -D $</build/bin/radiusd $(INSTALLDIR)/$</usr/sbin/radiusd
	$(STRIP) $(INSTALLDIR)/$</usr/sbin/radiusd
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/lib
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/share/freeradius && mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/certs
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/attr_filter
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/preprocess
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/files
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/sql/cui/sqlite
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/sql/ippool/sqlite
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/sql/ippool-dhcp/sqlite
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/sql/main/sqlite
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/mods-enabled
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/policy.d
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/sites-available
	mkdir -p $(INSTALLDIR)/$</usr/freeradius/raddb/sites-enabled
	install -D $</build/lib/.libs/libfreeradius-eap.so $(INSTALLDIR)/$</usr/freeradius/lib/libfreeradius-eap.so
	install -D $</build/lib/.libs/libfreeradius-radius.so $(INSTALLDIR)/$</usr/freeradius/lib/libfreeradius-radius.so
	install -D $</build/lib/.libs/libfreeradius-server.so $(INSTALLDIR)/$</usr/freeradius/lib/libfreeradius-server.so
	install -D $</build/lib/.libs/rlm_always.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_always.so
	install -D $</build/lib/.libs/rlm_attr_filter.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_attr_filter.so
	install -D $</build/lib/.libs/rlm_cache.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_cache.so
	install -D $</build/lib/.libs/rlm_chap.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_chap.so
	install -D $</build/lib/.libs/rlm_detail.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_detail.so
	install -D $</build/lib/.libs/rlm_dhcp.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_dhcp.so
	install -D $</build/lib/.libs/rlm_digest.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_digest.so
	install -D $</build/lib/.libs/rlm_dynamic_clients.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_dynamic_clients.so
	install -D $</build/lib/.libs/rlm_eap.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap.so
	install -D $</build/lib/.libs/rlm_eap_gtc.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_gtc.so
	install -D $</build/lib/.libs/rlm_eap_leap.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_leap.so
	install -D $</build/lib/.libs/rlm_eap_md5.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_md5.so
	install -D $</build/lib/.libs/rlm_eap_mschapv2.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_mschapv2.so
	install -D $</build/lib/.libs/rlm_eap_peap.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_peap.so
#	install -D $</build/lib/.libs/rlm_eap_pwd.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_pwd.so
#	install -D $</build/lib/.libs/rlm_eap_sim.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_sim.so
	install -D $</build/lib/.libs/rlm_eap_tls.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_tls.so
	install -D $</build/lib/.libs/rlm_eap_ttls.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_ttls.so
	install -D $</build/lib/.libs/rlm_exec.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_exec.so
	install -D $</build/lib/.libs/rlm_expiration.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_expiration.so
	install -D $</build/lib/.libs/rlm_expr.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_expr.so
	install -D $</build/lib/.libs/rlm_files.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_files.so
	install -D $</build/lib/.libs/rlm_linelog.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_linelog.so
	install -D $</build/lib/.libs/rlm_logintime.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_logintime.so
	install -D $</build/lib/.libs/rlm_mschap.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_mschap.so
	install -D $</build/lib/.libs/rlm_pap.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_pap.so
	install -D $</build/lib/.libs/rlm_passwd.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_passwd.so
	install -D $</build/lib/.libs/rlm_preprocess.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_preprocess.so
	install -D $</build/lib/.libs/rlm_radutmp.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_radutmp.so
	install -D $</build/lib/.libs/rlm_realm.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_realm.so
	install -D $</build/lib/.libs/rlm_replicate.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_replicate.so
	install -D $</build/lib/.libs/rlm_soh.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_soh.so
	install -D $</build/lib/.libs/rlm_sql.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_sql.so
	install -D $</build/lib/.libs/rlm_sqlcounter.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_sqlcounter.so
	install -D $</build/lib/.libs/rlm_sqlippool.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_sqlippool.so
	install -D $</build/lib/.libs/rlm_sql_sqlite.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_sql_sqlite.so
	install -D $</build/lib/.libs/rlm_unix.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_unix.so
	install -D $</build/lib/.libs/rlm_utf8.so $(INSTALLDIR)/$</usr/freeradius/lib/rlm_utf8.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/libfreeradius-eap.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/libfreeradius-radius.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/libfreeradius-server.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_always.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_attr_filter.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_cache.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_chap.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_detail.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_dhcp.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_digest.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_dynamic_clients.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_gtc.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_leap.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_md5.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_mschapv2.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_peap.so
#	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_pwd.so
#	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_sim.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_tls.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_eap_ttls.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_exec.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_expiration.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_expr.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_files.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_linelog.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_logintime.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_mschap.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_pap.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_passwd.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_preprocess.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_radutmp.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_realm.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_replicate.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_soh.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_sql.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_sqlcounter.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_sqlippool.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_sql_sqlite.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_unix.so
	$(STRIP) $(INSTALLDIR)/$</usr/freeradius/lib/rlm_utf8.so
	cp -rf $</share/* $(INSTALLDIR)/$</usr/freeradius/share/freeradius
	cp -f $</raddb/dictionary $(INSTALLDIR)/$</usr/freeradius/raddb/dictionary
	cp -f $</raddb/radiusd.conf $(INSTALLDIR)/$</usr/freeradius/raddb/radiusd.conf
	cp -f $</raddb/test_user.sql $(INSTALLDIR)/$</usr/freeradius/raddb/test_user.sql
	install -D $</raddb/mods-available/always $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/always
	install -D $</raddb/mods-available/attr_filter $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/attr_filter
	install -D $</raddb/mods-available/cache_eap $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/cache_eap
	install -D $</raddb/mods-available/chap $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/chap
	install -D $</raddb/mods-available/detail $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/detail
	install -D $</raddb/mods-available/detail.log $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/detail.log
	install -D $</raddb/mods-available/dhcp $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/dhcp
	install -D $</raddb/mods-available/digest $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/digest
	install -D $</raddb/mods-available/dynamic_clients $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/dynamic_clients
	install -D $</raddb/mods-available/eap $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/eap
	install -D $</raddb/mods-available/echo $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/echo
	install -D $</raddb/mods-available/exec $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/exec
	install -D $</raddb/mods-available/expiration $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/expiration
	install -D $</raddb/mods-available/expr $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/expr
	install -D $</raddb/mods-available/files $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/files
	install -D $</raddb/mods-available/linelog $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/linelog
	install -D $</raddb/mods-available/logintime $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/logintime
	install -D $</raddb/mods-available/mschap $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/mschap
	install -D $</raddb/mods-available/ntlm_auth $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/ntlm_auth
	install -D $</raddb/mods-available/pap $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/pap
	install -D $</raddb/mods-available/passwd $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/passwd
	install -D $</raddb/mods-available/preprocess $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/preprocess
	install -D $</raddb/mods-available/radutmp $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/radutmp
	install -D $</raddb/mods-available/realm $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/realm
	install -D $</raddb/mods-available/replicate $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/replicate
	install -D $</raddb/mods-available/soh $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/soh
	install -D $</raddb/mods-available/sql $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/sql
	install -D $</raddb/mods-available/sradutmp $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/sradutmp
	install -D $</raddb/mods-available/unix $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/unix
	install -D $</raddb/mods-available/utf8 $(INSTALLDIR)/$</usr/freeradius/raddb/mods-available/utf8
	cp -rf $</raddb/mods-config/attr_filter/* $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/attr_filter/
	cp -rf $</raddb/mods-config/preprocess/* $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/preprocess/
	cp -rf $</raddb/mods-config/files/* $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/files/
	cp -rf $</raddb/mods-config/sql/cui/sqlite/* $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/sql/cui/sqlite/
	cp -rf $</raddb/mods-config/sql/ippool/sqlite/* $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/sql/ippool/sqlite/
	cp -rf $</raddb/mods-config/sql/ippool-dhcp/sqlite/* $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/sql/ippool-dhcp/sqlite/
	cp -rf $</raddb/mods-config/sql/main/sqlite/* $(INSTALLDIR)/$</usr/freeradius/raddb/mods-config/sql/main/sqlite/
	cp -rf $</raddb/mods-enabled/* $(INSTALLDIR)/$</usr/freeradius/raddb/mods-enabled/
	cp -rf $</raddb/policy.d/* $(INSTALLDIR)/$</usr/freeradius/raddb/policy.d/
	cp -rf $</raddb/sites-available/default $(INSTALLDIR)/$</usr/freeradius/raddb/sites-available/
	cp -rf $</raddb/sites-available/inner-tunnel $(INSTALLDIR)/$</usr/freeradius/raddb/sites-available/
	cp -rf $</raddb/sites-enabled/* $(INSTALLDIR)/$</usr/freeradius/raddb/sites-enabled/
	cd $(TOP)/$</raddb/certs && make
	cp -f $(TOP)/$</raddb/certs/server.pem $(INSTALLDIR)/$</usr/freeradius/raddb/certs/
	cp -f $(TOP)/$</raddb/certs/ca.pem $(INSTALLDIR)/$</usr/freeradius/raddb/certs/
	cp -f $(TOP)/$</raddb/certs/dh $(INSTALLDIR)/$</usr/freeradius/raddb/certs/
	cd $(INSTALLDIR)/$</usr/freeradius/raddb/mods-enabled && ln -sf ../mods-available/sql sql

freeradius-server-3.0.0-clean:
	-@$(MAKE) -C freeradius-server-3.0.0 distclean

NORTON_DIR := $(SRCBASE)/router/norton${BCMEX}

.PHONY: norton${BCMEX} norton${BCMEX}-install norton${BCMEX}-clean


norton${BCMEX}:
	+$(MAKE) -C $(NORTON_DIR)

norton${BCMEX}-install:
	+$(MAKE) -C $(NORTON_DIR) install INSTALLDIR=$(INSTALLDIR)/norton${BCMEX}

norton${BCMEX}-clean:
	[ ! -f $(NORTEON_DIR)/Makefile ] || -@$(MAKE) -C $(NORTON_DIR) clean

.PHONY : email-3.1.3 email-3.1.3-clean

ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
email-3.1.3/stamp-h1: openssl
	cd $(TOP)/email-3.1.3 && \
	$(CONFIGURE) --with-ssl --sysconfdir=/etc \
		CFLAGS="-I$(SRCBASE)/router/openssl/include -I$(TOP)/nt_center/lib -I$(TOP)/sqlite -DRTCONFIG_NOTIFICATION_CENTER" \
		LDFLAGS="-L$(SRCBASE)/router/openssl -L$(TOP)/nt_center/lib -L$(TOP)/sqlite/.libs" \
		LIBS="-lnt -lsqlite3 -lpthread -lcrypto -lssl $(if $(RTCONFIG_QCA), -ldl)"
	touch $@
else
email-3.1.3/stamp-h1: openssl
	cd $(TOP)/email-3.1.3 && \
	$(CONFIGURE) --with-ssl --sysconfdir=/etc \
		CFLAGS=-I$(SRCBASE)/router/openssl/include \
		LDFLAGS="-L$(SRCBASE)/router/openssl" \
		LIBS="-lcrypto -lssl $(if $(RTCONFIG_QCA), -ldl)"
	touch $@
endif

email-3.1.3: email-3.1.3/stamp-h1
	$(MAKE) -C email-3.1.3

email-3.1.3-clean:
	-@$(MAKE) -C email-3.1.3 clean
	@rm -f email-3.1.3/stamp-h1

email-3.1.3-install:
	if [ -f email-3.1.3/Makefile ] ; then \
		install -D email-3.1.3/src/email $(INSTALLDIR)/email-3.1.3/usr/sbin/email ; \
		$(STRIP) $(INSTALLDIR)/email-3.1.3/usr/sbin/email ; \
	fi

dblog:
	@$(SEP)
	echo "build dblog"
	$(MAKE) -C dblog

dblog-clean:
	echo "clean dblog"
	$(MAKE) -C dblog clean

dblog-install:
	echo "install dblog"
	$(MAKE) -C dblog install

#diskdev_cmds-332.14
diskdev_cmds-332.14: openssl
	@$(SEP)
	cd $(TOP)/diskdev_cmds-332.14 && \
	make -f Makefile.lnx

diskdev_cmds-332.14-install: diskdev_cmds-332.14
	install -D diskdev_cmds-332.14/newfs_hfs.tproj/newfs_hfs $(INSTALLDIR)/diskdev_cmds-332.14/usr/sbin/mkfs.hfsplus
	install -D diskdev_cmds-332.14/fsck_hfs.tproj/fsck_hfs $(INSTALLDIR)/diskdev_cmds-332.14/usr/sbin/fsck.hfsplus
	$(STRIP) $(INSTALLDIR)/diskdev_cmds-332.14/usr/sbin/mkfs.hfsplus
	$(STRIP) $(INSTALLDIR)/diskdev_cmds-332.14/usr/sbin/fsck.hfsplus
	cd $(INSTALLDIR)/diskdev_cmds-332.14/usr/sbin && \
	rm -f mkfs.hfs && \
	rm -f fsck.hfs && \
	ln -s mkfs.hfsplus mkfs.hfs && \
	ln -s fsck.hfsplus fsck.hfs

diskdev_cmds-332.14-clean:
	cd $(TOP)/diskdev_cmds-332.14 && \
	make -f Makefile.lnx clean
	rm -f $(INSTALLDIR)/diskdev_cmds-332.14/usr/sbin/mkfs.hfs
	rm -f $(INSTALLDIR)/diskdev_cmds-332.14/usr/sbin/fsck.hfs

GeoIP-1.6.2/stamp-h1:
	cd GeoIP-1.6.2 && autoreconf -i -f && $(CONFIGURE) \
	--bindir=/sbin ac_cv_func_malloc_0_nonnull=yes \
	ac_cv_func_realloc_0_nonnull=yes --disable-dependency-tracking \
	--datadir=/usr/share
	touch $@

GeoIP-1.6.2: GeoIP-1.6.2/stamp-h1
	$(MAKE) -C $@

GeoIP-1.6.2-configure:
	( cd GeoIP-1.6.2 && $(CONFIGURE) \
	--bindir=/sbin ac_cv_func_malloc_0_nonnull=yes \
	ac_cv_func_realloc_0_nonnull=yes --disable-dependency-tracking \
	--datadir=/usr/share)

GeoIP-1.6.2-install: GeoIP-1.6.2
#	install -D GeoIP-1.6.2/apps/.libs/geoiplookup $(INSTALLDIR)/GeoIP-1.6.2/usr/sbin/geoiplookup
	install -D GeoIP-1.6.2/data/GeoIP.dat $(INSTALLDIR)/GeoIP-1.6.2/usr/share/GeoIP/GeoIP.dat
	install -D GeoIP-1.6.2/libGeoIP/.libs/libGeoIP.so.1.6.2 $(INSTALLDIR)/GeoIP-1.6.2/usr/lib/libGeoIP.so.1.6.2
#	$(STRIP) $(INSTALLDIR)/GeoIP-1.6.2/usr/sbin/geoiplookup
	$(STRIP) $(INSTALLDIR)/GeoIP-1.6.2/usr/lib/*.so.*
	cd $(INSTALLDIR)/GeoIP-1.6.2/usr/lib && \
		ln -sf libGeoIP.so.1.6.2 libGeoIP.so && \
		ln -sf libGeoIP.so.1.6.2 libGeoIP.so.1

GeoIP-1.6.2-clean:
	-@[ ! -f GeoIP-1.6.2/Makefile ] || $(MAKE) -C GeoIP-1.6.2 distclean
	@rm -f GeoIP-1.6.2/stamp-h1

Transmission-configure:
	( cd Transmission && ./autogen.sh && \
		$(CONFIGURE) --prefix=/usr --bindir=/usr/sbin --libdir=/usr/lib \
			CFLAGS="$(CFLAGS) -I$(STAGEDIR)/usr/include" \
			LDFLAGS="$(LDFLAGS) -L$(STAGEDIR)/usr/lib" \
			--disable-nls --disable-gtk \
	)

Transmission/Makefile:
	$(MAKE) Transmission-configure

Transmission: curl-7.21.7 libevent-2.0.21 Transmission/Makefile
	@$(SEP)
	$(MAKE) -C $@

Transmission-install: Transmission
	install -D $</daemon/transmission-daemon $(INSTALLDIR)/$</usr/sbin/transmission-daemon
	install -D $</daemon/transmission-remote $(INSTALLDIR)/$</usr/sbin/transmission-remote
	$(STRIP) $(INSTALLDIR)/$</usr/sbin/*

Transmission-clean:
	[ ! -f Transmission/Makefile ] || $(MAKE) -C Transmission KERNEL_DIR=$(LINUX_INC_DIR) distclean
	@rm -f Transmission/Makefile

wget/Makefile.in: wget/Makefile.am
	cd wget && autoreconf -fi

wget/Makefile: wget/Makefile.in
	cd wget && $(CONFIGURE) \
		--with-ssl=openssl --with-libssl-prefix=$(TOP)/openssl --sysconfdir=/etc \
		--disable-opie --disable-ntlm --disable-debug --disable-nls --disable-rpath \
		$(if $(RTCONFIG_IPV6),,--disable-ipv6) --disable-iri --without-included-regex  \
		--disable-dependency-tracking \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -I$(TOP)/openssl/include -I$(TOP)/zlib" \
		LDFLAGS="$(EXTRALDFLAGS) -Wl,--gc-sections -L$(TOP)/openssl -lssl -lcrypto -L$(TOP)/zlib -lz"

wget: openssl zlib wget/Makefile
	$(MAKE) -C $@

wget-clean:
	[ ! -f wget/Makefile ] || $(MAKE) -C wget distclean
	@rm -f wget/Makefile

wget-install:
	install -D wget/src/wget $(INSTALLDIR)/wget/usr/sbin/wget
	$(STRIP) $(INSTALLDIR)/wget/usr/sbin/wget

libqcacommon:
	$(MAKE) -C $@ && $(MAKE) $@-stage

libqcacommon-install: libqcacommon
	$(MAKE) -C $< INSTALLDIR=$(INSTALLDIR)/$< install

libqcacommon-clean:
	$(MAKE) -C libqcacommon clean

libqcacommon-stage:
	$(MAKE) -C libqcacommon stage

qca-whc30-lbd: libqcacommon
	$(MAKE) -C $@ && $(MAKE) $@-stage

iw: libnl-tiny-0.1
	$(MAKE) -C $@

iwinfo: libnl-tiny-0.1 $(if $(RTCONFIG_WIFI_QCA9994_QCA9994),qca-wifi-qca9994)
	$(MAKE) -C $@ && $(MAKE) $@-stage

iwinfo-stage:
	$(MAKE) -C iwinfo stage

qca-backports: qca-nss30-drv
	$(MAKE) -C $@ && $(MAKE) $@-stage

cfg_mnt : libevent-2.0.21 $(if $(RTCONFIG_AMAS), amas-utils)

qca-wifi-assoc-eventd: $(if $(RTCONFIG_CFGSYNC),cfg_mnt)

qca-wifi:
	$(MAKE) -C $@ TARGET_CROSS=$(CROSS_COMPILE) LINUX_KARCH=$(ARCH) LINUX_VERSION=$(LINUX_KERNEL) LINUX_DIR=$(LINUXDIR) && $(MAKE) $@-stage

qca-wifi-install: qca-wifi
	$(MAKE) -C $< INSTALLDIR=$(INSTALLDIR)/$< LINUX_VERSION=$(LINUX_KERNEL) install

qca-wifi-clean:
	$(MAKE) -C qca-wifi clean KERNELPATH=$(LINUXDIR)
	$(RM) -f qca-wifi/lsdk_flags

qca-wifi-stage:
	$(MAKE) -C qca-wifi stage


qca-wifi-fw-qca9888/stamp-h1:
	touch $@

qca-wifi-fw-qca9888: qca-wifi-fw-qca9888/stamp-h1
	$(MAKE) -C $@

qca-wifi-fw-qca9888-install: 
	$(MAKE) -C qca-wifi-fw-qca9888 install

qca-wifi.collard-10.4: iproute2-3.x
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-wifi.collard-10.4-install: qca-wifi.collard-10.4
	$(MAKE) -C $< INSTALLDIR=$(INSTALLDIR)/$< install

qca-wifi.collard-10.4-clean:
	$(MAKE) -C qca-wifi.collard-10.4 clean
	$(RM) -f qca-wifi.collard-10.4/source/lsdk_flags

qca-wifi.collard-10.4-stage:
	$(MAKE) -C qca-wifi.collard-10.4 stage


#qca-wifi.ipq40xx: qca-nss-drv.ipq40xx iproute2-3.x
qca-wifi.ipq40xx: iproute2-3.x
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-wifi.ipq40xx-install: qca-wifi.ipq40xx
	$(MAKE) -C $< INSTALLDIR=$(INSTALLDIR)/$< install

qca-wifi.ipq40xx-clean:
	$(MAKE) -C qca-wifi.ipq40xx clean
	$(RM) -f qca-wifi.ipq40xx/source/lsdk_flags

qca-wifi.ipq40xx-stage:
	$(MAKE) -C qca-wifi.ipq40xx stage

qca-wifi-qca9990: qca-nss-drv
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-wifi-qca9990-install: qca-wifi-qca9990
	$(MAKE) -C $< INSTALLDIR=$(INSTALLDIR)/$< install

qca-wifi-qca9990-clean:
	$(MAKE) -C qca-wifi-qca9990 clean
	$(RM) -f qca-wifi-qca9990/lsdk_flags

qca-wifi-qca9990-stage:
	$(MAKE) -C qca-wifi-qca9990 stage

qca-wifi-qca9994: qca-nss30-drv iproute2-3.x
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-wifi-qca9994-install: qca-wifi-qca9994
	$(MAKE) -C $< INSTALLDIR=$(INSTALLDIR)/$< install

qca-wifi-qca9994-clean:
	$(MAKE) -C qca-wifi-qca9994 clean
	$(RM) -f qca-wifi-qca9994/lsdk_flags

qca-wifi-qca9994-stage:
	$(MAKE) -C qca-wifi-qca9994 stage

qca-hostap-collard.10.4: qca-wifi.collard-10.4 libnl-bf openssl
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-hostap: qca-wifi
	$(MAKE) -C $@

qca-hostap.ipq40xx: qca-wifi.ipq40xx libnl-bf openssl
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-nss-drv.ipq40xx: qca-nss-gmac.ipq40xx
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-nss-drv.ipq40xx-stage:
	$(MAKE) -C qca-nss-drv.ipq40xx stage

qca-nss-gmac.ipq40xx:
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-nss-gmac.ipq40xx-stage:
	$(MAKE) -C qca-nss-gmac.ipq40xx && $(MAKE) -C qca-nss-gmac.ipq40xx stage

qca-rfs.ipq40xx:
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-rfs.ipq40xx-stage:
	$(MAKE) -C qca-rfs.ipq40xx stage

qca-ssdk.ipq40xx:
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-ssdk.ipq40xx-stage:
	$(MAKE) -C qca-ssdk.ipq40xx stage

qca-ssdk-shell.ipq40xx:
	$(MAKE) -C $@

qca-ssdk.collard:
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-ssdk.collard-stage:
	$(MAKE) -C qca-ssdk.collard stage

qca-ssdk-shell.collard:
	$(MAKE) -C $@

qca-hyfi-qdisc.ipq40xx:
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-hyfi-qdisc.ipq40xx-stage:
	$(MAKE) -C qca-hyfi-qdisc.ipq40xx && $(MAKE) -C qca-hyfi-qdisc.ipq40xx stage

qca-hyfi-qdisc.ipq40xx-install: qca-hyfi-qdisc.ipq40xx
	$(MAKE) -C $< INSTALLDIR=$(INSTALLDIR)/$< install

qca-hyfi-bridge.ipq40xx:
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-hyfi-bridge.ipq40xx-stage:
	$(MAKE) -C qca-hyfi-bridge.ipq40xx && $(MAKE) -C qca-hyfi-bridge.ipq40xx stage

qca-hyfi-bridge.ipq40xx-install: qca-hyfi-bridge.ipq40xx
	$(MAKE) -C $< INSTALLDIR=$(INSTALLDIR)/$< install

shortcut-fe.ipq40xx: libnl-bf
	$(MAKE) -C $@ && $(MAKE) $@-stage

shortcut-fe.ipq40xx-stage:
	$(MAKE) -C shortcut-fe.ipq40xx stage

qca-hostap-qca9990: qca-wifi-qca9990
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-hostap-qca9994: qca-wifi-qca9994 libnl-bf openssl
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-mcs30-lkm:
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-mcs30-lkm-stage:
	$(MAKE) -C qca-mcs30-lkm stage

qca-nss-clients: qca-nss-drv
	$(MAKE) -C $@

qca-nss30-clients: qca-nss30-drv qca-nss30-crypto
	$(MAKE) -C $@

qca-nss-drv: qca-nss-gmac
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-nss-drv-stage:
	$(MAKE) -C qca-nss-drv stage

qca-nss30-drv: qca-nss30-gmac
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-nss30-drv-stage:
	$(MAKE) -C qca-nss30-drv stage

qca-nss-ecm: qca-nss-drv
	$(MAKE) -C $@

qca-nss30-ecm: qca-nss30-drv $(if $(CONFIG_SWITCH_QCA8337N),qca-mcs30-lkm) shortcut-fe30
	$(MAKE) -C $@

qca-nss-gmac:
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-nss-gmac-stage:
	$(MAKE) -C qca-nss-gmac && $(MAKE) -C qca-nss-gmac stage

qca-nss30-gmac:
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-nss30-gmac-stage:
	$(MAKE) -C qca-nss30-gmac && $(MAKE) -C qca-nss30-gmac stage

qca-nss-crypto: qca-nss-drv
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-nss-crypto-stage:
	$(MAKE) -C qca-nss-crypto stage

qca-nss30-crypto: qca-nss30-drv
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-nss30-crypto-stage:
	$(MAKE) -C qca-nss30-crypto stage

qca-nss-cfi: qca-nss-crypto
	$(MAKE) -C $@

qca-nss30-cfi: qca-nss30-crypto
	$(MAKE) -C $@

qca-nss-macsec: qca-nss-gmac
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-nss-macsec-stage:
	$(MAKE) -C qca-nss-macsec stage

qca-ssdk:
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-ssdk-stage:
	$(MAKE) -C qca-ssdk stage

qca-ssdk30:
	$(MAKE) -C $@ && $(MAKE) $@-stage

qca-ssdk30-stage:
	$(MAKE) -C qca-ssdk30 stage

.PHONY: lm-sensors-master btconfig ethtool-3.7 i2c-tools-3.1.2 i2c-tools-2013-12-15 bluez-5.41
ethtool-3.7/configure:
	cd ethtool-3.7 && ./autogen.sh

ethtool-3.7/Makefile: ethtool-3.7/configure
	cd ethtool-3.7 && $(CONFIGURE) \
		--with-kernel=$(LINUXDIR)

ethtool-3.7: ethtool-3.7/Makefile
	$(MAKE) -C $@ KERNEL_DIR=$(LINUXDIR)

ethtool-3.7-clean:
	$(MAKE) -C ethtool-3.7 distclean

ethtool-3.7-install: ethtool-3.7
	install -D ethtool-3.7/ethtool $(INSTALLDIR)/ethtool-3.7/usr/bin/ethtool
	$(STRIP) $(INSTALLDIR)/ethtool-3.7/usr/bin/ethtool

libnl-bf:
	$(MAKE) -C $@ && $(PARALLEL_BUILD) $(MAKE) $@-stage

i2c-tools-2013-12-15:
	$(MAKE) -C $@

i2c-tools-2013-12-15-clean:
	$(MAKE) -C i2c-tools-2013-12-15 clean

i2c-tools-2013-12-15-install: i2c-tools-2013-12-15
	install -D i2c-tools-2013-12-15/tools/i2cdump $(INSTALLDIR)/i2c-tools-2013-12-15/usr/bin/i2cdump
	install -D i2c-tools-2013-12-15/tools/i2cget $(INSTALLDIR)/i2c-tools-2013-12-15/usr/bin/i2cget
	install -D i2c-tools-2013-12-15/tools/i2cset $(INSTALLDIR)/i2c-tools-2013-12-15/usr/bin/i2cset
	install -D i2c-tools-2013-12-15/tools/i2cdetect $(INSTALLDIR)/i2c-tools-2013-12-15/usr/bin/i2cdetect
	install -D i2c-tools-2013-12-15/lib/libi2c.so.0.1.0 $(INSTALLDIR)/i2c-tools-2013-12-15/lib/libi2c.so.0.1.0
	$(STRIP) $(INSTALLDIR)/i2c-tools-2013-12-15/usr/bin/i2cdump
	$(STRIP) $(INSTALLDIR)/i2c-tools-2013-12-15/usr/bin/i2cget
	$(STRIP) $(INSTALLDIR)/i2c-tools-2013-12-15/usr/bin/i2cset
	$(STRIP) $(INSTALLDIR)/i2c-tools-2013-12-15/usr/bin/i2cdetect
	$(STRIP) $(INSTALLDIR)/i2c-tools-2013-12-15/lib/libi2c.so.0.1.0
	cd $(INSTALLDIR)/i2c-tools-2013-12-15/lib && \
		ln -sf libi2c.so.0.1.0 libi2c.so.0 && \
		ln -sf libi2c.so.0.1.0 libi2c.so

i2c-tools-3.1.2:
	$(MAKE) -C $@

i2c-tools-3.1.2-clean:
	$(MAKE) -C i2c-tools-3.1.2 clean

i2c-tools-3.1.2-install: i2c-tools-3.1.2
	install -D i2c-tools-3.1.2/tools/i2cdump $(INSTALLDIR)/i2c-tools-3.1.2/usr/bin/i2cdump
	install -D i2c-tools-3.1.2/tools/i2cget $(INSTALLDIR)/i2c-tools-3.1.2/usr/bin/i2cget
	install -D i2c-tools-3.1.2/tools/i2cset $(INSTALLDIR)/i2c-tools-3.1.2/usr/bin/i2cset
	install -D i2c-tools-3.1.2/tools/i2cdetect $(INSTALLDIR)/i2c-tools-3.1.2/usr/bin/i2cdetect
	# install -D i2c-tools-3.1.2/lib/libi2c.so.0.1.0 $(INSTALLDIR)/i2c-tools-3.1.2/lib/libi2c.so.0.1.0
	$(STRIP) $(INSTALLDIR)/i2c-tools-3.1.2/usr/bin/i2cdump
	$(STRIP) $(INSTALLDIR)/i2c-tools-3.1.2/usr/bin/i2cget
	$(STRIP) $(INSTALLDIR)/i2c-tools-3.1.2/usr/bin/i2cset
	$(STRIP) $(INSTALLDIR)/i2c-tools-3.1.2/usr/bin/i2cdetect
	# $(STRIP) $(INSTALLDIR)/i2c-tools-3.1.2/lib/libi2c.so.0.1.0
	# cd $(INSTALLDIR)/i2c-tools-3.1.2/lib && \
	#	ln -sf libi2c.so.0.1.0 libi2c.so.0 && \
	#	ln -sf libi2c.so.0.1.0 libi2c.so

ppacmd:
	cd ppacmd; make

libhelper-1.4.0.2:
	cd libhelper-1.4.0.2; make

libhelper-1.4.0.2-install:
	mkdir -p $(TARGETDIR)/usr/lib
	install -D libhelper-1.4.0.2/libhelper.so $(TARGETDIR)/usr/lib/

libfapi-0.1: fapi_wlan_common-1.0.0.1
	cd libfapi-0.1; make clean; make

libfapi-0.1-install:
	cd libfapi-0.1; make clean; make
	install -D libfapi-0.1/libfapi.so $(TARGETDIR)/usr/lib/

fapi_wlan_common-1.0.0.1: libhelper-1.4.0.2
	cd fapi_wlan_common-1.0.0.1; make clean; make

fapi_wlan_common-1.0.0.1-install:
	mkdir -p $(TARGETDIR)/usr/lib
	install -D fapi_wlan_common-1.0.0.1/libfapiwlancommon.so $(TARGETDIR)/usr/lib/
	install -D fapi_wlan_common-1.0.0.1/ipkg-lantiq/fapi_wlan_common/usr/lib/fapi_wlan_beerock_cli $(TARGETDIR)/usr/lib/

hostapd-2.6: libnl-bf openssl
	cd hostapd-2.6/hostapd;	make clean; make hostapd; make hostapd_cli
	cd hostapd-2.6/wpa_supplicant; make clean; make wpa_supplicant; make wpa_cli

hostapd-2.6-install:
	mkdir -p $(TARGETDIR)/bin
	install -D hostapd-2.6/hostapd/hostapd $(TARGETDIR)/bin/
	install -D hostapd-2.6/hostapd/hostapd_cli $(TARGETDIR)/bin/
#	install -D hostapd-2.6/wpa_supplicant/wpa_supplicant $(TARGETDIR)/bin/
#	install -D hostapd-2.6/wpa_supplicant/wpa_cli $(TARGETDIR)/bin/

hostapd-2.6-clean:
	cd hostapd-2.6/hostapd;	make clean
	cd hostapd-2.6/wpa_supplicant; make clean

btconfig:
ifeq ($(RTCONFIG_LANTIQ),y)
	install -D btconfig/btconfig $(TARGETDIR)/usr/bin/btconfig
endif

lm-sensors-master:
	cd lm-sensors-master; make clean; make all

bluez-5.41/stamp-h1:
ifeq ($(wildcard bluez-5.41/prebuild),)
	cd bluez-5.41 && ./autogen.sh --no-configure
ifeq ($(RTCONFIG_REALTEK),y)
	cd bluez-5.41 && $(CONFIGURE) \
		--enable-library --program-prefix= --program-suffix= --prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib --sysconfdir=/etc\
		--datadir=/usr/share --localstatedir=/var --mandir=/usr/man --infodir=/usr/info --disable-systemd --with-udevdir=$(STAGEDIR)/usr/lib/udev\
		--enable-experimental --enable-maintainer-mode\
		UDEV_CFLAGS="-I$(STAGEDIR)/usr/include" UDEV_LIBS="$(STAGEDIR)/usr/lib/libudev.so" \
		CFLAGS="-I$(STAGEDIR)/usr/local/include -I$(STAGEDIR)/usr/include -I$(TOP)/bluez-5.41/src/bleencrypt/include -DENCRYPT -I$(TOP)/shared -I$(SRCBASE)/include -I$(STAGEDIR)/usr/local/include/glib-2.0 -I$(STAGEDIR)/usr/local/lib/glib-2.0/include" \
		LDFLAGS="-L$(STAGEDIR)/lib -L$(STAGEDIR)/usr/lib -ldbus-1 -L$(TOP)/libiconv-1.14/lib/.libs -liconv -ldl -lpthread -L$(STAGEDIR)/usr/local/lib -lglib-2.0 -lglib-2.0 -lical -licalvcal -L$(TOP)/openssl -lssl -lcrypto -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared"
	touch $@
else
	cd bluez-5.41 && $(CONFIGURE) \
		--enable-library --program-prefix= --program-suffix= --prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib --sysconfdir=/etc\
		--datadir=/usr/share --localstatedir=/var --mandir=/usr/man --infodir=/usr/info --disable-systemd --with-udevdir=$(STAGEDIR)/usr/lib/udev\
		--enable-experimental --enable-maintainer-mode\
		UDEV_CFLAGS="-I$(STAGEDIR)/usr/include" UDEV_LIBS="$(STAGEDIR)/usr/lib/libudev.so" \
		CFLAGS="$(EXTRA_BLUEZ_CFLAGS) -I$(STAGEDIR)/usr/local/include -I$(STAGEDIR)/usr/local/include/glib-2.0 -I$(STAGEDIR)/usr/local/lib/glib-2.0/include -I$(STAGEDIR)/usr/include -I$(TOP)/shared -I$(SRCBASE)/include -I$(TOP)/bluez-5.41/src/bleencrypt/include -DENCRYPT -D$(MODEL)" \
		LDFLAGS="$(EXTRA_BLUEZ_LDFLAGS) -L$(STAGEDIR)/lib -L$(STAGEDIR)/usr/lib -ldbus-1 -liconv -ldl -lpthread -L$(STAGEDIR)/usr/local/lib -lglib-2.0 -lglib-2.0 -lical -licalvcal -L$(TOP)/openssl -lssl -lcrypto -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared"
	touch $@
endif
endif
bluez-5.41: libiconv-1.14 glib-2.37.7 udev-173 libical-2.0.0 readline-6.2 bluez-5.41/stamp-h1
	$(MAKE) -C $@ && $(MAKE) $@-stage

bluez-5.41-clean:
	[ ! -f bluez-5.41/Makefile ] || $(MAKE) -C bluez-5.41 distclean
	@rm -f bluez-5.41/stamp-h1

bluez-5.41-install: btconfig bluez-5.41
	mkdir -p $(TARGETDIR)/rom/etc/bluetooth
ifeq ($(RTCONFIG_REALTEK),y)
	mkdir -p $(TARGETDIR)/lib/
	install $(LIBDIR)/librt.so.1 $(TARGETDIR)/lib/
	install $(LIBDIR)/libubacktrace.so.1 $(TARGETDIR)/lib/
endif
	install -D bluez-5.41/tools/bccmd $(INSTALLDIR)/bluez-5.41/usr/bin/bccmd
	install -D bluez-5.41/tools/bluemoon $(INSTALLDIR)/bluez-5.41/usr/bin/bluemoon
	install -D bluez-5.41/tools/ciptool $(INSTALLDIR)/bluez-5.41/usr/bin/ciptool
	install -D bluez-5.41/tools/hcitool $(INSTALLDIR)/bluez-5.41/usr/bin/hcitool
	install -D bluez-5.41/tools/hcidump $(INSTALLDIR)/bluez-5.41/usr/bin/hcidump
	install -D bluez-5.41/tools/hciattach $(INSTALLDIR)/bluez-5.41/usr/bin/hciattach
	install -D bluez-5.41/tools/hciconfig $(INSTALLDIR)/bluez-5.41/usr/bin/hciconfig
	install -D bluez-5.41/tools/hid2hci $(INSTALLDIR)/bluez-5.41/usr/bin/hid2hci
	install -D bluez-5.41/tools/l2ping $(INSTALLDIR)/bluez-5.41/usr/bin/l2ping
	install -D bluez-5.41/tools/l2test $(INSTALLDIR)/bluez-5.41/usr/bin/l2test
	install -D bluez-5.41/client/bluetoothctl $(INSTALLDIR)/bluez-5.41/usr/bin/bluetoothctl
	install -D bluez-5.41/tools/mpris-proxy $(INSTALLDIR)/bluez-5.41/usr/bin/mpris-proxy
	install -D bluez-5.41/tools/rctest $(INSTALLDIR)/bluez-5.41/usr/bin/rctest
	install -D bluez-5.41/tools/rfcomm $(INSTALLDIR)/bluez-5.41/usr/bin/rfcomm
	install -D bluez-5.41/tools/sdptool $(INSTALLDIR)/bluez-5.41/usr/bin/sdptool
ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
	install -D bluez-5.41/btconfig_1.16 $(INSTALLDIR)/bluez-5.41/usr/bin/btconfig
	install -D bluez-5.41/btchk.sh $(INSTALLDIR)/bluez-5.41/usr/bin/btchk.sh
else ifeq ($(or $(QCA953X),$(QCA955X),$(QCA956X)),y)
	install -D bluez-5.41/btconfig_4.02 $(INSTALLDIR)/bluez-5.41/usr/bin/btconfig
else ifeq ($(RTCONFIG_LANTIQ),y)
	install -D btconfig/btconfig $(TARGETDIR)/usr/bin/btconfig
endif
	install -D bluez-5.41/lib/.libs/libbluetooth.so.$(LIBBLUETOOTH_VER) $(TARGETDIR)/usr/lib/libbluetooth.so.$(LIBBLUETOOTH_VER)
	install -D bluez-5.41/src/bluetoothd $(INSTALLDIR)/bluez-5.41/usr/bin/bluetoothd
	install -D bluez-5.41/attrib/gatttool $(INSTALLDIR)/bluez-5.41/usr/bin/gatttool
ifeq ($(BUILD_NAME), $(filter $(BUILD_NAME), MAP-AC3000))
	install -D bluez-5.41/3000000.psr $(TARGETDIR)/rom/etc/3000000.psr
endif
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/bccmd
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/bluemoon
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/ciptool
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/hcitool
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/hcidump
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/hciattach
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/hciconfig
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/hid2hci
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/l2ping
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/l2test
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/bluetoothctl
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/mpris-proxy
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/rctest
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/rfcomm
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/sdptool
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/bluetoothd
	$(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/gatttool
	# $(STRIP) $(INSTALLDIR)/bluez-5.41/usr/bin/btconfig
	$(STRIP) $(TARGETDIR)/usr/lib/libbluetooth.so.$(LIBBLUETOOTH_VER)
ifeq ($(RTCONFIG_LANTIQ),y)
	cp -f bluez-5.41/src/main.conf $(TARGETDIR)/rom/etc/bluetooth
endif
	cp -f bluez-5.41/src/bleencrypt/pem2048/public.pem $(TARGETDIR)/rom/etc/bluetooth
	cp -f bluez-5.41/src/bleencrypt/pem2048/private.pem $(TARGETDIR)/rom/etc/bluetooth
	rm -rf $(TARGETDIR)/usr/obex
	cd $(TARGETDIR)/usr/lib && \
		ln -sf libbluetooth.so.$(LIBBLUETOOTH_VER) libbluetooth.so.3 && \
		ln -sf libbluetooth.so.3 libbluetooth.so
	mkdir -p $(TARGETDIR)/lib
	cd $(TARGETDIR)/lib && \
		ln -sf libc.so.0 libc.so.1 && \
		ln -sf libpthread.so.0 libpthread.so.1

jansson-2.7/Makefile:
	@cd jansson-2.7 && autoreconf -i -f && $(CONFIGURE) CC=$(CC) \
		--prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib \
		--sysconfdir=/etc --datadir=/usr/share --localstatedir=/var --mandir=/usr/man --infodir=/usr/info

jansson-2.7: jansson-2.7/Makefile
	@$(MAKE) -C $@ && $(MAKE) $@-stage

jansson-2.7-install: jansson-2.7
	install -D jansson-2.7/src/.libs/libjansson.so $(INSTALLDIR)/jansson-2.7/usr/lib/libjansson.so
	$(STRIP) $(INSTALLDIR)/jansson-2.7/usr/lib/libjansson.so
	cd $(INSTALLDIR)/jansson-2.7/usr/lib && \
		ln -sf libjansson.so libjansson.so.4

jansson-2.7-clean:
	[ ! -f jansson-2.7/Makefile ] || $(MAKE) -C jansson-2.7 clean
	@rm -f jansson-2.7/Makefile

qca-whc-lbd.ipq40xx: jansson-2.7

libffi-3.0.11/Makefile: libffi-3.0.11/configure
	cd libffi-3.0.11 && $(CONFIGURE) --prefix=$(STAGEDIR)/usr/local

libffi-3.0.11: libffi-3.0.11/Makefile
	$(MAKE) -C $@ && $(MAKE) -C $@ install

libffi-3.0.11-clean:
	[ ! -f libffi-3.0.11/Makefile ] || $(MAKE) -C libffi-3.0.11 distclean

libffi-3.0.11-install: libffi-3.0.11
	install -D $(STAGEDIR)/usr/local/lib/libffi.so.6.0.0 $(INSTALLDIR)/libffi-3.0.11/usr/lib/libffi.so.6.0.0
	$(STRIP) $(INSTALLDIR)/libffi-3.0.11/usr/lib/libffi.so.6.0.0
	cd $(INSTALLDIR)/libffi-3.0.11/usr/lib && \
		ln -sf libffi.so.6.0.0 libffi.so.6 && \
		ln -sf libffi.so.6.0.0 libffi.so

udev-173/configure:
	-cd udev-173 && ./autogen.sh --host --force
	cd udev-173 &&  autoreconf --force --install

udev-173/Makefile: udev-173/configure
	cd udev-173 && $(CONFIGURE)\
		--libdir=/usr/lib --prefix=/usr --exec_prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib --sysconfdir=/etc\
		--datadir=/usr/share --localstatedir=/var --mandir=/usr/man --infodir=/usr/info --disable-nls --prefix=/usr --exec-prefix= --sysconfdir=/etc --disable-hwdb\
		--disable-keymap --disable-gudev --disable-introspection --libexecdir=/lib/udev --disable-gtk-doc-html --sbindir=/sbin

udev-173: udev-173/Makefile
	$(MAKE) -C $@ && $(MAKE) $@-stage

udev-173-clean:
	[ ! -f udev-173/Makefile ] || $(MAKE) -C udev-173 distclean

udev-173-install: udev-173
	install -D udev-173/libudev/.libs/libudev.so.0.12.0 $(INSTALLDIR)/udev-173/usr/lib/libudev.so.0.12.0
	$(STRIP) $(INSTALLDIR)/udev-173/usr/lib/libudev.so.0.12.0
	cd $(INSTALLDIR)/udev-173/usr/lib && \
		ln -sf libudev.so.0.12.0 libudev.so.0 && \
		ln -sf libudev.so.0.12.0 libudev.so

libical-2.0.0/Makefile: libical-2.0.0/CMakeLists.txt
	cd libical-2.0.0 && cmake -DCMAKE_INSTALL_PREFIX=/usr \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_EXE_LINKER_FLAGS="-L$(STAGEDIR)/usr/local/lib -ldl -lpthread" 

libical-2.0.0: libical-2.0.0/Makefile
	$(MAKE) -C $@ && $(MAKE) $@-stage

libical-2.0.0-clean:
	[ ! -f libical-2.0.0/Makefile ] || $(MAKE) -C libical-2.0.0 distclean

libical-2.0.0-install:
	install -D libical-2.0.0/lib/libical_cxx.so.2.0.0 $(INSTALLDIR)/libical-2.0.0/usr/lib/libical_cxx.so.2.0.0
	install -D libical-2.0.0/lib/libical.so.2.0.0 $(INSTALLDIR)/libical-2.0.0/usr/lib/libical.so.2.0.0
	install -D libical-2.0.0/lib/libicalss_cxx.so.2.0.0 $(INSTALLDIR)/libical-2.0.0/usr/lib/libicalss_cxx.so.2.0.0
	install -D libical-2.0.0/lib/libicalss.so.2.0.0 $(INSTALLDIR)/libical-2.0.0/usr/lib/libicalss.so.2.0.0
	install -D libical-2.0.0/lib/libicalvcal.so.2.0.0 $(INSTALLDIR)/libical-2.0.0/usr/lib/libicalvcal.so.2.0.0
	$(STRIP) $(INSTALLDIR)/libical-2.0.0/usr/lib/libical_cxx.so.2.0.0
	$(STRIP) $(INSTALLDIR)/libical-2.0.0/usr/lib/libical.so.2.0.0
	$(STRIP) $(INSTALLDIR)/libical-2.0.0/usr/lib/libicalss_cxx.so.2.0.0
	$(STRIP) $(INSTALLDIR)/libical-2.0.0/usr/lib/libicalss.so.2.0.0
	$(STRIP) $(INSTALLDIR)/libical-2.0.0/usr/lib/libicalvcal.so.2.0.0
	cd $(INSTALLDIR)/libical-2.0.0/usr/lib && \
		ln -sf libical_cxx.so.2.0.0 libical_cxx.so.2.0 &&\
		ln -sf libical_cxx.so.2.0.0 libical_cxx.so.2 &&\
		ln -sf libical.so.2.0.0 libical.so.2.0 &&\
		ln -sf libical.so.2.0.0 libical.so.2 &&\
		ln -sf libicalss_cxx.so.2.0.0 libicalss_cxx.so.2.0 &&\
		ln -sf libicalss_cxx.so.2.0.0 libicalss_cxx.so.2 &&\
		ln -sf libicalss.so.2.0.0 libicalss.so.2.0 &&\
		ln -sf libicalss.so.2.0.0 libicalss.so.2 &&\
		ln -sf libicalvcal.so.2.0.0 libicalvcal.so.2.0 &&\
		ln -sf libicalvcal.so.2.0.0 libicalvcal.so.2

ncurses-6.0/Makefile: ncurses-6.0/configure
	cd ncurses-6.0 && $(CONFIGURE) \
		CFLAGS="$(EXTRACFLAGS) -Os -I$(STAGEDIR)/usr/include -ffunction-sections -fdata-sections -fPIC" \
		LDFLAGS="$(LDFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(STAGEDIR)/usr/lib" \
		$(if $(HND_ROUTER),CPPFLAGS="-P",) \
		--prefix=/usr --without-cxx --without-cxx-binding \
		--enable-echo --enable-const --enable-overwrite --disable-rpath --without-ada \
		$(if $(RTCONFIG_BCMARM),--enable-widec,) \
		--without-debug --without-manpages --without-profile --without-progs --without-tests \
		--disable-home-terminfo --with-normal --with-shared --with-build-cppflags=-D_GNU_SOURCE \
		--enable-pc-files \
		--disable-termcap --disable-database --with-fallbacks="xterm,vt100,vt200,linux,ansi,xterm-256color"
#		--with-default-terminfo-dir=/usr/share/terminfo --with-terminfo-dirs=/usr/share/terminfo

ncurses-6.0: ncurses-6.0/Makefile
	@$(SEP)
	cd ncurses-6.0 && $(MAKE) -C ncurses && $(MAKE) -C misc pc-files

ncurses-6.0-clean:
	[ ! -f ncurses-6.0/Makefile ] || $(MAKE) -C ncurses-6.0 distclean

ncurses-6.0-install: ncurses-6.0
ifeq ($(RTCONFIG_BCMARM),y)
	install -D ncurses-6.0/lib/libncursesw.so.6.0 $(INSTALLDIR)/ncurses-6.0/usr/lib/libncursesw.so.6.0
	cd $(INSTALLDIR)/ncurses-6.0/usr/lib && \
		ln -sf libncursesw.so.6.0 libncursesw.so.6 &&\
		ln -sf libncursesw.so.6.0 libncursesw.so
else
	install -D ncurses-6.0/lib/libncurses.so.6.0 $(INSTALLDIR)/ncurses-6.0/usr/lib/libncurses.so.6.0
	cd $(INSTALLDIR)/ncurses-6.0/usr/lib && \
		ln -sf libncurses.so.6.0 libncurses.so.6 &&\
		ln -sf libncurses.so.6.0 libncurses.so
endif
	$(STRIP) $(INSTALLDIR)/ncurses-6.0/usr/lib/*.so.*

# Start merlin components
nano/Makefile:
	cd nano && \
	autoreconf -i -f && $(CONFIGURE) --prefix=$(INSTALLDIR)/nano/usr --sysconfdir=/jffs/configs \
		CFLAGS="$(EXTRACFLAGS) -Os -I$(TOP)/ncurses-6.0/include -ffunction-sections -fdata-sections" \
		LDFLAGS="$(LDFLAGS) -L$(TOP)/ncurses-6.0/lib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		PKG_CONFIG_LIBDIR="$(TOP)/ncurses-6.0/misc" PKG_CONFIG_PATH="$(TOP)/ncurses-6.0/misc" \
		$(if $(RTCONFIG_BCMARM),ac_cv_lib_ncursesw_get_wch=yes --enable-utf8,ac_cv_lib_ncursesw_get_wch=no --enable-tiny) \
		--disable-speller --disable-extra --disable-tabcomp --enable-wrapping --disable-glibtest \
		--disable-libmagic --disable-nls

nano-clean:
	-@$(MAKE) -C nano distclean
	@rm -f nano/Makefile

nano-install:
	@$(MAKE) -C nano install-exec INSTALLDIR=$(INSTALLDIR)/nano/usr/bin/nano
	$(STRIP) $(INSTALLDIR)/nano/usr/bin/nano

nano: ncurses-6.0 nano/Makefile
	@$(SEP)
	$(MAKE) -C $@

nettle/stamp-h1:
	@cd nettle && \
	CPPFLAGS="-I$(TOP)/gmp" \
	CFLAGS="-O2 -Wall $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections" \
	LDFLAGS="-L$(TOP)/gmp/.libs -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
	$(CONFIGURE) prefix=$(TOP)/nettle --enable-mini-gmp --disable-documentation --disable-shared
	@touch nettle/stamp-h1

nettle: nettle/stamp-h1
	@$(SEP)
	@$(MAKE) -C nettle $(PARALLEL_BUILD)
	@$(MAKE) -C nettle install

nettle-clean:
	-@$(MAKE) -C nettle clean
	@rm -f nettle/stamp-h1
	@rm -rf nettle/include nettle/lib

samba-3.6.x: libiconv-1.14
	@$(MAKE) -C samba-3.6.x $(PARALLEL_BUILD)
samba-3.5.8:
	@$(MAKE) -C samba-3.5.8 $(PARALLEL_BUILD)

ipset:
	$(MAKE) -C ipset binaries $(PARALLEL_BUILD)

ipset-install:
	install -D ipset/ipset $(INSTALLDIR)/ipset/usr/sbin/ipset
	install -d $(INSTALLDIR)/ipset/usr/lib/
	install ipset/*.so $(INSTALLDIR)/ipset/usr/lib/
	$(STRIP) $(INSTALLDIR)/ipset/usr/lib/*.so
	$(STRIP) $(INSTALLDIR)/ipset/usr/sbin/ipset

libnfnetlink/stamp-h1:
	cd $(TOP)/libnfnetlink && CC=$(CC) STRIP=$(STRIP) \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr --enable-shared --enable-static
	touch libnfnetlink/stamp-h1

libnfnetlink: libnfnetlink/stamp-h1
	@$(SEP)
	$(MAKE) -C libnfnetlink

libnfnetlink-install:
	install -D libnfnetlink/src/.libs/libnfnetlink.so.0.2.0 $(INSTALLDIR)/libnfnetlink/usr/lib/libnfnetlink.so.0.2.0
	$(STRIP) -s $(INSTALLDIR)/libnfnetlink/usr/lib/libnfnetlink.so.0.2.0
	cd $(INSTALLDIR)/libnfnetlink/usr/lib/ && \
		ln -sf libnfnetlink.so.0.2.0 libnfnetlink.so.0 && \
		ln -sf libnfnetlink.so.0.2.0 libnfnetlink.so

libmnl/stamp-h1:
	cd $(TOP)/libmnl && CC=$(CC) STRIP=$(STRIP) \
	CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
	$(CONFIGURE) --prefix=/usr
	touch libmnl/stamp-h1

libmnl: libmnl/stamp-h1
	$(MAKE) -C libmnl
	$(MAKE) -C libmnl DESTDIR=$(TOP)/libmnl/staged install

libmnl-install: libmnl
	install -d $(INSTALLDIR)/libmnl/usr/lib/
	install libmnl/src/.libs/libmnl.so.0.1.0 $(INSTALLDIR)/libmnl/usr/lib/libmnl.so.0.1.0
	$(STRIP) $(INSTALLDIR)/libmnl/usr/lib/libmnl.so.0.1.0
	cd $(INSTALLDIR)/libmnl/usr/lib/ && \
		ln -sf libmnl.so.0.1.0 libmnl.so.0 && \
		ln -sf libmnl.so.0.1.0 libmnl.so

libmnl-clean:
	-@$(MAKE) -C libmnl clean
	-@rm -rf libmnl/staged

ipset_arm/stamp-h1: libmnl
	cd $(TOP)/ipset_arm && CC=$(CC) STRIP=$(STRIP) \
	autoreconf -i -f && \
	CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
	libmnl_CFLAGS="-I$(TOP)/libmnl/staged/usr/include" \
	libmnl_LIBS="-L$(TOP)/libmnl/staged/usr/lib -lmnl" \
	$(CONFIGURE) --prefix=/usr --with-kmod=$(if $(HND_ROUTER),no,yes) --with-kbuild=$(LINUXDIR)
	touch ipset_arm/stamp-h1

ipset_arm: ipset_arm/stamp-h1
	$(MAKE) -C ipset_arm
	$(MAKE) -C ipset_arm modules

ipset_arm-install:
	install -D ipset_arm/src/ipset $(INSTALLDIR)/ipset_arm/usr/sbin/ipset
	install -d $(INSTALLDIR)/ipset_arm/usr/lib/
	install ipset_arm/lib/.libs/libipset.so.3.6.0 $(INSTALLDIR)/ipset_arm/usr/lib/libipset.so.3.6.0
	$(STRIP) $(INSTALLDIR)/ipset_arm/usr/lib/libipset.so.3.6.0
	$(STRIP) $(INSTALLDIR)/ipset_arm/usr/sbin/ipset
	cd $(INSTALLDIR)/ipset_arm/usr/lib/ && \
		ln -sf libipset.so.3.6.0 libipset.so.3 && \
		ln -sf libipset.so.3.6.0 libipset.so

ipset_arm-clean:
	-@$(MAKE) -C ipset_arm clean
	-@$(MAKE) -C ipset_arm modules_clean
	-@rm -rf ipset_arm/Makefile ipset_arm/stamp-h1

nfs-utils/stamp-h1:
	cd nfs-utils && CPPFLAGS="$(EXTRACFLAGS) -Os" knfsd_cv_bsd_signals=no \
	CC_FOR_BUILD=$(CC) $(CONFIGURE) --enable-nfsv3 --disable-nfsv4 --disable-ipv6 \
	--disable-uuid --disable-gss --disable-mount --without-tcp-wrappers
	@touch $@

nfs-utils: nfs-utils/stamp-h1
	$(MAKE) -C $@ $(PARALLEL_BUILD)

nfs-utils-clean:
	-@$(MAKE) -C nfs-utils distclean
	@rm -f nfs-utils/stamp-h1

nfs-utils-install:
	install -D nfs-utils/utils/nfsd/nfsd $(INSTALLDIR)/nfs-utils/usr/sbin/nfsd
	install -D nfs-utils/utils/mountd/mountd $(INSTALLDIR)/nfs-utils/usr/sbin/mountd
	install -D nfs-utils/utils/exportfs/exportfs $(INSTALLDIR)/nfs-utils/usr/sbin/exportfs
	install -D nfs-utils/utils/showmount/showmount $(INSTALLDIR)/nfs-utils/usr/sbin/showmount
	install -D nfs-utils/utils/statd/statd $(INSTALLDIR)/nfs-utils/usr/sbin/statd
	install -D nfs-utils/support/lib/libnfs.so $(INSTALLDIR)/nfs-utils/usr/lib/libnfs.so
	$(STRIP) $(INSTALLDIR)/nfs-utils/usr/sbin/* $(INSTALLDIR)/nfs-utils/usr/lib/libnfs.so

nfs-utils-1.3.4/stamp-h1:
	cd nfs-utils-1.3.4 && CPPFLAGS="$(EXTRACFLAGS) -Os" knfsd_cv_bsd_signals=no \
	CC_FOR_BUILD=$(CC) $(CONFIGURE) --enable-nfsv3 --disable-nfsv4 --disable-nfsv41 \
	--disable-ipv6 --disable-tirpc --disable-nfsdcltrack \
	--disable-uuid --disable-gss --disable-mount --without-tcp-wrappers
	@touch $@

nfs-utils-1.3.4: nfs-utils-1.3.4/stamp-h1
	$(MAKE) -C $@

nfs-utils-1.3.4-clean:
	-@$(MAKE) -C nfs-utils distclean
	@rm -f nfs-utils-1.3.4/stamp-h1

nfs-utils-1.3.4-install:
	install -D nfs-utils-1.3.4/utils/nfsd/nfsd $(INSTALLDIR)/nfs-utils-1.3.4/usr/sbin/nfsd
	install -D nfs-utils-1.3.4/utils/mountd/mountd $(INSTALLDIR)/nfs-utils-1.3.4/usr/sbin/mountd
	install -D nfs-utils-1.3.4/utils/exportfs/exportfs $(INSTALLDIR)/nfs-utils-1.3.4/usr/sbin/exportfs
	install -D nfs-utils-1.3.4/utils/showmount/showmount $(INSTALLDIR)/nfs-utils-1.3.4/usr/sbin/showmount
	install -D nfs-utils-1.3.4/utils/statd/statd $(INSTALLDIR)/nfs-utils-1.3.4/usr/sbin/statd
	$(STRIP) $(INSTALLDIR)/nfs-utils-1.3.4/usr/sbin/*

portmap: portmap/Makefile
	$(MAKE) -C $@  EXTRACFLAGS="$(EXTRACFLAGS) -Os" NO_TCP_WRAPPER=y RPCUSER=nobody

portmap-install:
	install -D portmap/portmap $(INSTALLDIR)/portmap/usr/sbin/portmap
	$(STRIP) $(INSTALLDIR)/portmap/usr/sbin/portmap
# End merlin components

readline-6.2/Makefile: readline-6.2/configure
	cd readline-6.2 && $(CONFIGURE)\
		--prefix=/usr --exec_prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib\
		--sysconfdir=/etc --disable-static --enable-shared\
		CFLAGS="-I$(STAGEDIR)/usr/local/include"\
		LDFLAGS="-L$(STAGEDIR)/usr/local/lib -L$(STAGEDIR)/usr/lib -L$(TOP)/ncurese-2.0/lib -ldl -lpthread" \

readline-6.2: ncurses-6.0 readline-6.2/Makefile
	$(MAKE) SHLIB_LIBS="-lncurses" -C $@ && $(MAKE) $@-stage

readline-6.2-clean:
	[ ! -f readline-6.2/Makefile ] || $(MAKE) -C readline-6.2 distclean

readline-6.2-install: readline-6.2
	install -D readline-6.2/shlib/libhistory.so.6.2 $(INSTALLDIR)/readline-6.2/usr/lib/libhistory.so.6.2
	install -D readline-6.2/shlib/libreadline.so.6.2 $(INSTALLDIR)/readline-6.2/usr/lib/libreadline.so.6.2
	$(STRIP) $(INSTALLDIR)/readline-6.2/usr/lib/libhistory.so.6.2
	$(STRIP) $(INSTALLDIR)/readline-6.2/usr/lib/libreadline.so.6.2
	cd $(INSTALLDIR)/readline-6.2/usr/lib && \
		ln -sf libhistory.so.6.2 libhistory.so.6 &&\
		ln -sf libhistory.so.6.2 libhistory.so &&\
		ln -sf libreadline.so.6.2 libreadline.so.6 &&\
		ln -sf libreadline.so.6.2 libreadline.so

dbus-1.8.8/stamp-h1:
	cd dbus-1.8.8 && ./autogen.sh --no-configure
	cd dbus-1.8.8 && $(CONFIGURE)\
		--prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib --sysconfdir=/etc \
		--datadir=/usr/share --localstatedir=/var --mandir=/usr/man --infodir=/usr/info --disable-nls --disable-abstract-sockets --disable-ansi --disable-asserts \
		--disable-console-owner-file --disable-doxygen-docs --disable-compiler_coverage --disable-selinux --disable-tests --disable-verbose-mode --disable-xml-docs --with-xml=expat \
		--with-dbus-user=root --with-dbus-daemondir=/usr/sbin --with-system-socket=/var/run/dbus/system_bus_socket --with-system-pid-file=/var/run/dbus.pid \
		--without-x --libexecdir=/usr/lib/dbus-1 \
		CFLAGS="-I$(STAGEDIR)/usr/include -I$(STAGEDIR)/usr/local/include" \
		LDFLAGS="-L$(STAGEDIR)/usr/lib -ldl -lpthread -L$(STAGEDIR)/usr/local/lib -ldl -lpthread"
	touch $@

dbus-1.8.8: expat-2.0.1 dbus-1.8.8/stamp-h1
	$(MAKE) -C $@ && $(MAKE) $@-stage

dbus-1.8.8-clean:
	[ ! -f dbus-1.8.8/Makefile ] || $(MAKE) -C dbus-1.8.8 distclean
	@rm -f dbus-1.8.8/stamp-h1

dbus-1.8.8-install: dbus-1.8.8
	install -D dbus-1.8.8/dbus/.libs/libdbus-1.so.3.8.7 $(INSTALLDIR)/dbus-1.8.8/usr/lib/libdbus-1.so.3.8.7
	install -D dbus-1.8.8/bus/dbus-daemon $(INSTALLDIR)/dbus-1.8.8/usr/sbin/dbus-daemon
	install -D dbus-1.8.8/tools/dbus-launch $(INSTALLDIR)/dbus-1.8.8/usr/bin/dbus-launch
	install -D dbus-1.8.8/tools/.libs/dbus-uuidgen $(INSTALLDIR)/dbus-1.8.8/usr/bin/dbus-uuidgen
	install -d $(TARGETDIR)/rom/etc/dbus-1
	@cp -af dbus-1.8.8/etc_dbus-1/* $(TARGETDIR)/rom/etc/dbus-1/
	$(STRIP) $(INSTALLDIR)/dbus-1.8.8/usr/lib/libdbus-1.so.3.8.7
	$(STRIP) $(INSTALLDIR)/dbus-1.8.8/usr/sbin/dbus-daemon
	$(STRIP) $(INSTALLDIR)/dbus-1.8.8/usr/bin/dbus-launch
	$(STRIP) $(INSTALLDIR)/dbus-1.8.8/usr/bin/dbus-uuidgen
	cd $(INSTALLDIR)/dbus-1.8.8/usr/lib && \
		ln -sf libdbus-1.so.3.8.7 libdbus-1.so.3 && \
		ln -sf libdbus-1.so.3.8.7 libdbus-1.so

glib-2.37.7/configure:
	cd glib-2.37.7 && ./autogen.sh

glib-2.37.7/Makefile: dbus-1.8.8 libffi-3.0.11 zlib glib-2.37.7/configure
ifeq ($(RTCONFIG_REALTEK),y)
	cd glib-2.37.7 && $(CONFIGURE) --disable-nls --enable-debug=no --disable-selinux --disable-fam\
		--enable-iconv=no --with-libiconv=gnu\
		LIBFFI_CFLAGS="-I$(STAGEDIR)/usr/local/lib/libffi-3.0.11/include" \
		LIBFFI_LIBS="$(STAGEDIR)/usr/local/lib/libffi.so" \
		CFLAGS="-I$(TOP)/libiconv-1.14/include -I$(TOP)/glib-2.37.7 -I$(STAGEDIR)/usr/include" \
		LDFLAGS="-L$(TOP)/libiconv-1.14/lib/.libs -liconv -L$(STAGEDIR)/usr/lib -ldl -lpthread -lz -L$(STAGEDIR)/usr/local/lib -lffi"
else
	cd glib-2.37.7 && $(CONFIGURE) --disable-nls --enable-debug=no --disable-selinux --disable-fam\
		LIBFFI_CFLAGS="-I$(STAGEDIR)/usr/local/lib/libffi-3.0.11/include" \
		LIBFFI_LIBS="$(STAGEDIR)/usr/local/lib/libffi.so" \
		CFLAGS="-I$(TOP)/libiconv-1.14/include -I$(TOP)/glib-2.37.7 -I$(STAGEDIR)/usr/include" \
		LDFLAGS="-L$(TOP)/libiconv-1.14/lib/.libs -liconv -L$(STAGEDIR)/usr/lib -ldl -lpthread -lz -L$(STAGEDIR)/usr/local/lib -lffi"
endif

glib-2.37.7: glib-2.37.7/Makefile
	$(MAKE) -C $@ && $(MAKE) $@-stage
	@sed 's|/usr/lib/libiconv.la|$(STAGEDIR)/usr/lib/libiconv.la|g' -i $(STAGEDIR)/usr/local/lib/libgio-2.0.la
	@sed 's|/usr/lib/libiconv.la|$(STAGEDIR)/usr/lib/libiconv.la|g' -i $(STAGEDIR)/usr/local/lib/libglib-2.0.la
	@sed 's|/usr/lib/libiconv.la|$(STAGEDIR)/usr/lib/libiconv.la|g' -i $(STAGEDIR)/usr/local/lib/libgmodule-2.0.la
	@sed 's|/usr/lib/libiconv.la|$(STAGEDIR)/usr/lib/libiconv.la|g' -i $(STAGEDIR)/usr/local/lib/libgobject-2.0.la
	@sed 's|/usr/lib/libiconv.la|$(STAGEDIR)/usr/lib/libiconv.la|g' -i $(STAGEDIR)/usr/local/lib/libgthread-2.0.la

glib-2.37.7-clean:
	[ ! -f glib-2.37.7/Makefile ] || $(MAKE) -C glib-2.37.7 distclean

glib-2.37.7-install: glib-2.37.7
	install -D glib-2.37.7/gio/.libs/libgio-2.0.so.0.3707.0 $(INSTALLDIR)/glib-2.37.7/usr/lib/libgio-2.0.so.0.3707.0
	install -D glib-2.37.7/glib/.libs/libglib-2.0.so.0.3707.0 $(INSTALLDIR)/glib-2.37.7/usr/lib/libglib-2.0.so.0.3707.0
	install -D glib-2.37.7/gmodule/.libs/libgmodule-2.0.so.0.3707.0 $(INSTALLDIR)/glib-2.37.7/usr/lib/libgmodule-2.0.so.0.3707.0
	install -D glib-2.37.7/gobject/.libs/libgobject-2.0.so.0.3707.0 $(INSTALLDIR)/glib-2.37.7/usr/lib/libgobject-2.0.so.0.3707.0
	install -D glib-2.37.7/gthread/.libs/libgthread-2.0.so.0.3707.0 $(INSTALLDIR)/glib-2.37.7/usr/lib/libgthread-2.0.so.0.3707.0
	$(STRIP) $(INSTALLDIR)/glib-2.37.7/usr/lib/libgio-2.0.so.0.3707.0
	$(STRIP) $(INSTALLDIR)/glib-2.37.7/usr/lib/libglib-2.0.so.0.3707.0
	$(STRIP) $(INSTALLDIR)/glib-2.37.7/usr/lib/libgmodule-2.0.so.0.3707.0
	$(STRIP) $(INSTALLDIR)/glib-2.37.7/usr/lib/libgobject-2.0.so.0.3707.0
	$(STRIP) $(INSTALLDIR)/glib-2.37.7/usr/lib/libgthread-2.0.so.0.3707.0
	cd $(INSTALLDIR)/glib-2.37.7/usr/lib && \
		ln -sf libgio-2.0.so.0.3707.0 libgio-2.0.so.0 && \
		ln -sf libgio-2.0.so.0.3707.0 libgio-2.0.so && \
		ln -sf libglib-2.0.so.0.3707.0 libglib-2.0.so.0 && \
		ln -sf libglib-2.0.so.0.3707.0 libglib-2.0.so && \
		ln -sf libgmodule-2.0.so.0.3707.0 libgmodule-2.0.so.0 && \
		ln -sf libgmodule-2.0.so.0.3707.0 libgmodule-2.0.so && \
		ln -sf libgobject-2.0.so.0.3707.0 libgobject-2.0.so.0 && \
		ln -sf libgobject-2.0.so.0.3707.0 libgobject-2.0.so && \
		ln -sf libgthread-2.0.so.0.3707.0 libgthread-2.0.so.0 && \
		ln -sf libgthread-2.0.so.0.3707.0 libgthread-2.0.so

shortcut-fe30: libnl-bf
	$(MAKE) -C $@ && $(MAKE) $@-stage

shortcut-fe30-stage:
	$(MAKE) -C shortcut-fe30 stage

LinuxART2-install: LinuxART2
ifneq ($(ART2_INSTALLDIR),)
	$(MAKE) -C $< install INSTALLDIR=$(ART2_INSTALLDIR)/$<
else
	$(MAKE) -C $< install INSTALLDIR=$(INSTALLDIR)/$<
endif

stage-tags: dummy
	ctags -aR -f $(SRCBASE)/tags $(STAGEDIR)/usr/include

sysstat-10.0.3-install:
	$(MAKE) -C sysstat-10.0.3 install_base DESTDIR=$(INSTALLDIR)/sysstat-10.0.3 INSTALL_DOC=n
	mv $(INSTALLDIR)/sysstat-10.0.3/etc $(INSTALLDIR)/sysstat-10.0.3/etc_sysstat
	rm -rf $(INSTALLDIR)/sysstat-10.0.3/var
#
# tunnel related
#

appx:
	$(MAKE) -C appx all

.PHONY : appx

asusnatnl: openssl
	$(MAKE) -C $@ $(shell if [[ "$(HND_ROUTER)" = "y" ]] ; then echo "LD="""; else echo ""; fi)

asusnatnl-clean:
	$(MAKE) -C asusnatnl clean

asusnatnl-install:
	if [ -f asusnatnl/natnl/libasusnatnl.so ] ; then \
		install -d $(INSTALLDIR)/asusnatnl/usr/lib/ ; \
		install -D asusnatnl/natnl/libasusnatnl.so $(INSTALLDIR)/asusnatnl/usr/lib/libasusnatnl.so ; \
		$(STRIP) -s $(INSTALLDIR)/asusnatnl/usr/lib/libasusnatnl.so ; \
	fi

mastiff: shared nvram${BCMEX}$(EX7) wb
	cd aaews && \
	CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) CFLAGS="-Os -Wall $(EXTRACFLAGS)" \
	make mastiff

mastiff-install:
	install -d $(INSTALLDIR)/wb/usr/lib/
	install -D aaews/mastiff $(INSTALLDIR)/wb/usr/sbin/mastiff
	$(STRIP) -s $(INSTALLDIR)/wb/usr/sbin/mastiff

mastiff-clean:
	cd aaews && make clean
	echo $(INSTALLDIR)

#aaews: wb

#wb: shared nvram curl-7.21.7 openssl libxml2 asusnatnl
aaews: wb
	cd aaews && \
	CC=$(CC) CXX=$(CXX) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) CFLAGS="-Os -Wall $(EXTRACFLAGS)" \
	make router
#make static-lib && \

#$(MAKE)
#CC=mipsel-uclibc-g++ AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) CFLAGS="-Os -Wall $(EXTRACFLAGS)" \

aaews-install:
	install -d $(INSTALLDIR)/wb/usr/lib/
	install -D wb/libws.so $(INSTALLDIR)/wb/usr/lib/libws.so
	$(STRIP) -s $(INSTALLDIR)/wb/usr/lib/libws.so
	install -D aaews/aaews $(INSTALLDIR)/wb/usr/sbin/aaews
	$(STRIP) -s $(INSTALLDIR)/wb/usr/sbin/aaews
ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(HND_ROUTER),y)
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
else
	install -D $(shell dirname $(shell which $(CXX)))/../arm-brcm-linux-uclibcgnueabi/lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
endif
else ifeq ($(RTCONFIG_REALTEK),y)
	install -D $(LIBDIR)/librt.so.0 $(TARGETDIR)/lib/librt.so.0
	install -D $(LIBDIR)/libc.so.0 $(TARGETDIR)/lib/libc.so.0
	install -D $(LIBDIR)/libgcc_s.so.1 $(TARGETDIR)/lib/libgcc_s.so.1
	install -D $(LIBDIR)/libstdc++.so.6 $(TARGETDIR)/lib/libstdc++.so.6
else ifeq ($(RTCONFIG_RALINK),y)
	install -D $(shell dirname $(shell which $(CXX)))/../lib/librt.so.0 $(INSTALLDIR)/wb/lib/librt.so.0
	install -D $(shell dirname $(shell which $(CXX)))/../lib/libstdc++.so.6 $(INSTALLDIR)/wb/lib/libstdc++.so.6
else
	install -D $(shell dirname $(shell which $(CXX)))/../lib/libstdc++.so.6 $(INSTALLDIR)/wb/usr/lib/libstdc++.so.6
endif

aaews-clean:
	cd aaews && make clean

amas-utils: json-c openssl nvram${BCMEX}$(EX7)
	cd amas-utils && \
        CC=$(CC) CXX=$(CXX) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) CFLAGS="-Os -Wall $(EXTRACFLAGS)" \
        make all
        
amas-utils-clean:
	cd amas-utils && make clean

amas-utils-install:
	install -d $(INSTALLDIR)/amas-utils/usr/lib/
	install -D amas-utils/libamas-utils.so $(INSTALLDIR)/amas-utils/usr/lib/libamas-utils.so
	$(STRIP) -s $(INSTALLDIR)/amas-utils/usr/lib/libamas-utils.so
ifneq ($(wildcard $(TOP)/amas-utils/*.c),)
	install -D amas-utils/amas-utils-cli $(INSTALLDIR)/amas-utils/usr/sbin/amas-utils-cli
	$(STRIP) -s $(INSTALLDIR)/amas-utils/usr/sbin/amas-utils-cli
endif

json-c/stamp-h1:
ifeq ($(RTCONFIG_LANTIQ),y)
	cd $(TOP)/json-c && \
	$(CONFIGURE) --enable-shared --disable-static \
	ac_cv_func_realloc_0_nonnull=yes ac_cv_func_malloc_0_nonnull=yes ac_cv_have_decl__isnan=no \
	CFLAGS="-Os -Wno-error $(EXTRACFLAGS)" \
	LDFLAGS="$(EXTRALDFLAGS) -Wl,--as-needed -lm"
	touch $@
else
	cd $(TOP)/json-c && \
	$(CONFIGURE) --enable-shared --disable-static \
	ac_cv_func_realloc_0_nonnull=yes ac_cv_func_malloc_0_nonnull=yes \
	CFLAGS="-Os -Wno-error $(EXTRACFLAGS)" \
	LDFLAGS="$(EXTRALDFLAGS) -Wl,--as-needed -lm"
	touch $@
endif

json-c: json-c/stamp-h1
	$(MAKE) -C $@

json-c-install:
	install -D json-c/.libs/libjson-c.so.2.0.2 $(INSTALLDIR)/json-c/usr/lib/libjson-c.so.2.0.2
	$(STRIP) $(INSTALLDIR)/json-c/usr/lib/libjson-c.so.2.0.2
	cd $(INSTALLDIR)/json-c/usr/lib && \
		ln -sf libjson-c.so.2.0.2 libjson-c.so && \
		ln -sf libjson-c.so.2.0.2 libjson-c.so.2

json-c-clean:
	-@[ ! -f json-c/Makefile ] || $(MAKE) -C json-c clean
	-@rm -f json-c/stamp-h1

phddns/stamp-h1:
ifeq ($(HND_ROUTER),y)
	cd phddns && $(CONFIGURE) --prefix="" \
		CFLAGS="-Os -I$(SRCBASE)/include -I$(TOP)/shared $(EXTRACFLAGS)" \
		LDFLAGS="-L$(TOP)/nvram$(BCMEX)$(EX7) -L$(TOP)/wlcsm -L$(TOP)/shared $(if $(RTCONFIG_QTN),-L$(TOP)/libqcsapi_client) $(EXTRALDFLAGS)" \
		LIBS="-lnvram -lwlcsm -lshared $(if $(RTCONFIG_QTN),-lqcsapi_client)"
	@touch $@
else
	cd phddns && $(CONFIGURE) --prefix="" \
		CFLAGS="-Os -I$(SRCBASE)/include -I$(TOP)/shared $(EXTRACFLAGS)" \
		LDFLAGS="-L$(TOP)/nvram$(BCMEX)$(EX7) -L$(TOP)/shared $(if $(RTCONFIG_QTN),-L$(TOP)/libqcsapi_client) $(EXTRALDFLAGS)" \
		LIBS="-lnvram -lshared $(if $(RTCONFIG_QTN),-lqcsapi_client)"
	@touch $@
endif

phddns: phddns/stamp-h1
	$(MAKE) -C phddns

phddns-install:
	install -D phddns/src/phddns $(INSTALLDIR)/phddns/usr/sbin/phddns
	$(STRIP) $(INSTALLDIR)/phddns/usr/sbin/phddns

phddns-clean:
	@[ ! -f phddns/Makefile ] || $(MAKE) -C phddns clean
	@rm -f phddns/stamp-h1

acme-client-portable:
	$(MAKE) -C acme-client-portable

acme-client-portable-install: acme-client-portable
	install -D acme-client-portable/acme-client $(INSTALLDIR)/acme-client-portable/usr/sbin/acme-client
	$(STRIP) $(INSTALLDIR)/acme-client-portable/usr/sbin/acme-client

lighttpd-le/stamp-h1:
	cd lighttpd-le && ./autogen.sh && \
	$(CONFIGURE) \
	--prefix=/usr/local \
	--without-pcre \
	--with-openssl=$(TOP)/openssl \
	--without-zlib \
	--without-bzip2
	@touch $@

lighttpd-le: lighttpd-le/stamp-h1
	$(MAKE) -C lighttpd-le

lighttpd-le-clean:
	-@$(MAKE) -C lighttpd-le clean
	@rm -f lighttpd-le/stamp-h1

lighttpd-le-install: lighttpd-le
	install -D lighttpd-le/src/lighttpd $(INSTALLDIR)/lighttpd-le/usr/sbin/lighttpd
	install -D lighttpd-le/src/.libs/mod_dirlisting.so $(INSTALLDIR)/lighttpd-le/usr/local/lib/mod_dirlisting.so
	install -D lighttpd-le/src/.libs/mod_staticfile.so $(INSTALLDIR)/lighttpd-le/usr/local/lib/mod_staticfile.so
	install -D lighttpd-le/src/.libs/mod_indexfile.so $(INSTALLDIR)/lighttpd-le/usr/local/lib/mod_indexfile.so
	$(STRIP) $(INSTALLDIR)/lighttpd-le/usr/sbin/lighttpd
	$(STRIP) $(INSTALLDIR)/lighttpd-le/usr/local/lib/mod_*.so

quicksec-7.0:
	@$(SEP)
	@$(MAKE) -C quicksec-7.0/build/quicksec/linux/policymanager

quicksec-7.0-install:
	@install -D quicksec-7.0/build/quicksec/linux/policymanager/quicksecpm $(INSTALLDIR)/quicksec-7.0/usr/sbin/quicksecpm
	@$(STRIP) $(INSTALLDIR)/quicksec-7.0/usr/sbin/quicksecpm

quicksec-7.0-clean:
	-@$(MAKE) -C quicksec-7.0/build/quicksec/linux/policymanager clean
	
PMS_DBapis/stamp-h1:
	@touch $@ 

PMS_DBapis:PMS_DBapis/stamp-h1
	CC=$(CC) $(MAKE) -C PMS_DBapis

PMS_DBapis-clean:
	cd PMS_DBapis && $(MAKE) clean
	-@rm -f PMS_DBapis/stamp-h1

PMS_DBapis-install:PMS_DBapis
	cd PMS_DBapis && CC=$(CC) && $(MAKE) install

wlceventd: $(if $(RTCONFIG_NOTIFICATION_CENTER),nt_center sqlite,)

iwevent: $(if $(RTCONFIG_NOTIFICATION_CENTER),nt_center sqlite,)

wlc_nt: $(if $(RTCONFIG_NOTIFICATION_CENTER),nt_center sqlite,)

visdcoll: libxml2 nvram$(BCMEX)$(EX7)
ifeq ($(RTCONFIG_VISUALIZATION),y)
ifneq ($(wildcard $(VISDCOLL_DIR)),)
	[ -f $(VISDCOLL_DIR)/Makefile ] || \
	(cd $(VISDCOLL_DIR) && autoreconf -i -f && $(CONFIGURE) CC=$(CC) \
		--prefix=$(VISSOURCE_BASE_DIR)/installbin)
	+$(MAKE) -C $(VISDCOLL_DIR) EXTRA_LDFLAGS=$(EXTRA_LDFLAGS)
	+$(MAKE) -C $(VISDCOLL_DIR) install-strip
endif
endif

visdcoll-install:
ifeq ($(RTCONFIG_VISUALIZATION),y)
	install -d $(INSTALLDIR)/visdcoll/usr/sbin
	cp -rf $(VISSOURCE_BASE_DIR)/installbin/bin/vis-datacollector $(INSTALLDIR)/visdcoll/usr/sbin
endif

visdcoll-clean:
ifeq ($(RTCONFIG_VISUALIZATION),y)
ifneq ($(wildcard $(VISDCOLL_DIR)),)
	+$(MAKE) -C $(VISDCOLL_DIR) clean
endif
endif

sqlite3/stamp-h1:
ifeq ($(RTCONFIG_VISUALIZATION),y)
	cd $(SQLITE3_DIR) && autoreconf -i -f && \
	CC=$(CC) CFLAGS="-Os $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -lpthread -ldl" \
		$(CONFIGURE) --prefix=/usr --enable-static \
		--disable-readline --disable-dynamic-extensions --enable-threadsafe \
		--disable-dependency-tracking
	touch $(SQLITE3_DIR)/stamp-h1
	touch $(SQLITE3_DIR)/aclocal.m4
endif

sqlite3: sqlite3/stamp-h1
ifeq ($(RTCONFIG_VISUALIZATION),y)
	@$(MAKE) -C $(SQLITE3_DIR) all
endif

sqlite3-clean:
ifeq ($(RTCONFIG_VISUALIZATION),y)
	-@$(MAKE) -C $(SQLITE3_DIR) clean
	@rm -f $(SQLITE3_DIR)/stamp-h1
endif

visdcon: sqlite3 json-c libxml2 nvram$(BCMEX)$(EX7)
ifeq ($(RTCONFIG_VISUALIZATION),y)
ifneq ($(wildcard $(VISDCON_DIR)),)
	[ -f $(VISDCON_DIR)/Makefile ] || \
	(cd $(VISDCON_DIR) && autoreconf -i -f && $(CONFIGURE) CC=$(CC) \
		--prefix=$(VISSOURCE_BASE_DIR)/installbin)
	+$(MAKE) -C $(VISDCON_DIR) EXTRA_LDFLAGS=$(EXTRA_LDFLAGS)
	+$(MAKE) -C $(VISDCON_DIR) install-strip
endif
endif

visdcon-install:
ifeq ($(RTCONFIG_VISUALIZATION),y)
	install -d $(INSTALLDIR)/visdcon/usr/sbin
	cp -rf $(VISSOURCE_BASE_DIR)/installbin/bin/vis-dcon $(INSTALLDIR)/visdcon/usr/sbin
endif

visdcon-clean:
ifeq ($(RTCONFIG_VISUALIZATION),y)
ifneq ($(wildcard $(VISDCON_DIR)),)
	+$(MAKE) -C $(VISDCON_DIR) clean
endif
endif

##### hnd_router apps/libs #####
wlcsm:
	$(MAKE) -C wlcsm MODEL=$(MODEL)
	install -m 755 $@/libwlcsm.so $(INSTALL_DIR)/lib

.PHONY : wlcsm


wlan: wlcsm libxml2 json-c sqlite
	$(MAKE) -C wlan

.PHONY : wlan

websockets: openssl openssl-install
	$(MAKE) -C websockets

websockets-install:
	$(MAKE) -C websockets INSTALLDIR=$(INSTALLDIR)/websockets install

.PHONY : websockets

bcm_util:
	$(MAKE) -C bcm_util

.PHONY : bcm_util

#cms_msg: bcm_util
#	$(MAKE) -C cms_msg CURRENT_ARCH=arm
#	$(MAKE) -C cms_msg CURRENT_ARCH=aarch64

#.PHONY : cms_msg

rastatus6: cms_msg cms_util bcm_flashutil cms_boardctl tmctl vlanctl seltctl bridgeutil pwrctl
	$(MAKE) -C rastatus6

.PHONY : rastatus6

#cms_util: bcm_util
#	$(MAKE) -C cms_util

#.PHONY : cms_util

bcm_flashutil: bcm_util
	$(MAKE) -C bcm_flashutil

bcm_flashutil-install:
	$(MAKE) -C bcm_flashutil INSTALLDIR=$(INSTALLDIR)/bcm_flashutil install

.PHONY : bcm_flashutil

bcm_flasher: bcm_flashutil
	$(MAKE) -C bcm_flasher

bcm_flasher-install:
	$(MAKE) -C bcm_flasher INSTALLDIR=$(INSTALLDIR)/bcm_flasher install

.PHONY : bcm_flasher

cms_boardctl: bcm_util
	$(MAKE) -C cms_boardctl

.PHONY : cms_boardctl


bcm_boardctl:
	$(MAKE) -C bcm_boardctl

.PHONY : bcm_boardctl

#tmctl:  tmctl_lib cms_msg cms_util bcm_flashutil cms_boardctl
ifneq ($(PREBUILT_BCMBIN),1)
tmctl:  tmctl_lib bcm_boardctl
else
tmctl:
endif
	$(MAKE) -C tmctl

tmctl-install:
	$(MAKE) -C tmctl INSTALLDIR=$(INSTALLDIR)/tmctl install

.PHONY : tmctl


ifneq ($(PREBUILT_BCMBIN),1)
tmctl_lib: ethswctl rdpactl bcmtm
else
tmctl_lib:
endif
	$(MAKE) -C tmctl_lib

tmctl_lib-install:
	$(MAKE) -C tmctl_lib INSTALLDIR=$(INSTALLDIR)/tmctl_lib install

.PHONY : tmctl_lib

ifneq ($(PREBUILT_BCMBIN),1)
ethswctl: ethswctl_lib bcm_flashutil public/libs/bcm_util
else
ethswctl:
endif
	$(MAKE) -C ethswctl

ethswctl-install:
	$(MAKE) -C ethswctl INSTALLDIR=$(INSTALLDIR)/ethswctl install

.PHONY : ethswctl

ethswctl_lib:
	$(MAKE) -C ethswctl_lib

.PHONY : ethswctl_lib

rdpactl:
	$(MAKE) -C rdpactl

.PHONY : rdpactl

bcmtm_lib:
	$(MAKE) -C bcmtm_lib

.PHONY : bcmtm_lib

ifneq ($(PREBUILT_BCMBIN),1)
bcmtm: bcmtm_lib
else
bcmtm:
endif
	$(MAKE) -C bcmtm

.PHONY : bcmtm

ifneq ($(PREBUILT_BCMBIN),1)
vlanctl: vlanctl_lib bcm_flashutil
else
vlanctl:
endif
	$(MAKE) -C vlanctl

vlanctl-install:
	$(MAKE) -C vlanctl INSTALLDIR=$(INSTALLDIR)/vlanctl install

.PHONY : vlanctl

vlanctl_lib:
	$(MAKE) -C vlanctl_lib

.PHONY : vlanctl_lib

#seltctl: cms_msg cms_util bcm_flashutil cms_boardctl
ifneq ($(PREBUILT_BCMBIN),1)
seltctl: bcm_flashutil
else
seltctl:
endif
	$(MAKE) -C seltctl

seltctl-install:
	$(MAKE) -C seltctl INSTALLDIR=$(INSTALLDIR)/seltctl install

.PHONY : seltctl

bridgeutil:
	$(MAKE) -C bridgeutil

.PHONY : bridgeutil

#pwrctl: pwrctl_lib cms_msg cms_util bcm_flashutil cms_boardctl bcm_util
ifneq ($(PREBUILT_BCMBIN),1)
pwrctl: pwrctl_lib bcm_flashutil bcm_util ethswctl ethswctl_lib
else
pwrctl:
endif
	$(MAKE) -C pwrctl

pwrctl-install:
	$(MAKE) -C pwrctl INSTALLDIR=$(INSTALLDIR)/pwrctl install

.PHONY : pwrctl

pwrctl_lib:
	$(MAKE) -C pwrctl_lib

.PHONY : pwrctl_lib

bridge-utils:
	$(MAKE) -C bridge-utils

.PHONY : bridge-utils

iperf-2.0.5: stlport
	$(MAKE) -C iperf-2.0.5

.PHONY : iperf-2.0.5

stlport:
	$(MAKE) -C stlport EXTRA_CFLAGS=""

stlport-install:
	@echo done

.PHONY : stlport

mmc-utils:
	$(MAKE) -C mmc-utils

.PHONY : mmc-utils

mtd:
	$(MAKE) -C mtd

.PHONY : mtd

sysstat:
	$(MAKE) -C sysstat

.PHONY : sysstat

urlfilterd:
	$(MAKE) -C urlfilterd $(BUILD_URLFILTER)

urlfilterd-install:
	$(MAKE) -C urlfilterd INSTALLDIR=$(INSTALLDIR)/urlfilterd install

.PHONY : urlfilterd

bpmctl:
	$(MAKE) -C bpmctl $(BUILD_BPMCTL)

.PHONY : bpmctl

dnsspoof:
	$(MAKE) -C dnsspoof $(BUILD_DNSSPOOF)

.PHONY : dnsspoof

ethctl_lib:
	$(MAKE) -C ethctl_lib

.PHONY : ethctl_lib

ifneq ($(PREBUILT_BCMBIN),1)
ethctl: ethctl_lib
else
ethctl:
endif
	$(MAKE) -C ethctl dynamic

.PHONY : ethctl

ifneq ($(PREBUILT_BCMBIN),1)
fcctl: fcctl_lib
else
fcctl:
endif
	$(MAKE) -C fcctl $(BUILD_FCCTL)

.PHONY : fcctl

fcctl_lib:
	$(MAKE) -C fcctl_lib

.PHONY : fcctl_lib

scripts:
	$(MAKE) -C scripts

.PHONY : scripts

stress:
	$(MAKE) -C stress

.PHONY : stress

vpmstats:
	$(MAKE) -C vpmstats

.PHONY : vpmstats

bcm_boot_launcher:
	$(MAKE) -C bcm_boot_launcher

.PHONY : bcm_boot_launcher

#bdmf_shell: cms_msg cms_util bcm_flashutil cms_boardctl
bdmf_shell: bcm_flashutil
	$(MAKE) -C bdmf_shell

bdmf_shell-install:
	$(MAKE) -C bdmf_shell INSTALLDIR=$(INSTALLDIR)/bdmf_shell install

.PHONY : bdmf_shell

dhrystone:
	$(MAKE) -C dhrystone

.PHONY : dhrystone

ipsec-tools: openssl
	$(MAKE) -C ipsec-tools

ipsec-tools-install:
	$(MAKE) -C ipsec-tools INSTALLDIR=$(INSTALLDIR)/ipsec-tools install

.PHONY : ipsec-tools

psictl:
	$(MAKE) -C psictl

.PHONY : psictl

scratchpadctl:
	$(MAKE) -C scratchpadctl

.PHONY : scratchpadctl

drvlogs_scripts:
	$(MAKE) -C drvlogs_scripts

.PHONY : drvlogs_scripts

wdtctl:
	$(MAKE) -C wdtctl

.PHONY : wdtctl

bcmmcastctl: bcmmcast
	$(MAKE) -C bcmmcastctl

.PHONY : bcmmcastctl

bcmmcast:
	$(MAKE) -C bcmmcast

.PHONY : bcmmcast

mcpctl:
	$(MAKE) -C mcpctl

.PHONY : mcpctl

mcpd: bcm_util bcmmcast bridgeutil bcm_flashutil
	$(MAKE) -C mcpd

.PHONY : mcpd

#bcm_sslconf: expat-2.0.1 openssl
#	$(MAKE) -C bcm_sslconf

#.PHONY : bcm_sslconf

#expat:
#	$(MAKE) -C expat
#	#install $(LIBDIR)/libexpat.so.1.6.0 $(TARGETDIR)/lib/

#.PHONY : expat

httpdshared:
	$(MAKE) -C httpdshared

.PHONY : httpdshared

mdk:
	$(MAKE) -C mdk

.PHONY : mdk

ifneq ($(PREBUILT_BCMBIN),1)
#swmdk: mdk cms_msg cms_util bcm_flashutil cms_boardctl ethswctl_lib
swmdk: mdk bcm_flashutil ethswctl_lib
else
swmdk:
endif
	$(MAKE) -C swmdk

swmdk-install:
	$(MAKE) -C swmdk INSTALLDIR=$(INSTALLDIR)/swmdk install

.PHONY : swmdk

jqplot:
	$(MAKE) -C jqplot

.PHONY : jqplot

jquery:
	$(MAKE) -C jquery

.PHONY : jquery

tablesorter:
	$(MAKE) -C tablesorter

.PHONY : tablesorter


#
# Generic rules
#

%:
	@[ ! -d $* ] || [ -f $*/Makefile ] || $(MAKE) $*-configure
	@[ ! -d $* ] || ( $(SEP); $(MAKE) -C $* )


%-clean:
	-@[ ! -d $* ] || $(MAKE) -C $* clean


%-install: %
	@echo $*
	@[ ! -d $* ] || $(MAKE) -C $* install INSTALLDIR=$(INSTALLDIR)/$*
ifeq ($(HND_ROUTER),y)
	@-mkdir -p $(BCM_FSBUILD_DIR)/public/lib$(BCM_INSTALL_SUFFIX_DIR)
	@-mkdir -p $(BCM_FSBUILD_DIR)/public/bin$(BCM_INSTALL_SUFFIX_DIR)
	@-mkdir -p $(BCM_FSBUILD_DIR)/public/sbin$(BCM_INSTALL_SUFFIX_DIR)
	@[ ! -d $* ] || $(MAKE) -C $* install INSTALLDIR=$(BCM_FSBUILD_DIR)/public
endif

%-stage:
	@echo $*
	@[ ! -d $* ] || $(MAKE) -C $* install DESTDIR=$(STAGEDIR) INSTALLDIR=$(INSTALLDIR)/$*

%-build:
	$(MAKE) $*-clean $*

%/Makefile:
	[ ! -d $* ] || $(MAKE) $*-configure

%-configure:
	@[ ! -d $* ] || ( cd $* ; \
		$(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
	)


$(obj-y) $(obj-n) $(obj-clean) $(obj-install): dummy

.PHONY: all clean distclean mrproper install package image
.PHONY: conf mconf oldconf kconf kmconf config menuconfig oldconfig
.PHONY: dummy libnet libpcap


