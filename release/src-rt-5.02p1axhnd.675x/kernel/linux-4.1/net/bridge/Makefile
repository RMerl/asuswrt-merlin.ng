#
# Makefile for the IEEE 802.1d ethernet bridging layer.
#

ifdef BCM_KF # defined(CONFIG_BCM_KF_WL)
ifneq ($(strip $(CONFIG_BCM_WLAN)),)
EXTRA_CFLAGS    += -DPKTC
ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
EXTRA_CFLAGS    += -DBCM_PKTFWD
endif
EXTRA_CFLAGS    += -DDSLCPE -DBCMDRIVER
EXTRA_CFLAGS	+= -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS    += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/include
EXTRA_CFLAGS    += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/include
EXTRA_CFLAGS    += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/shared/impl1
endif
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
endif #BCM_KF

obj-$(CONFIG_BRIDGE) += bridge.o

bridge-y	:= br.o br_device.o br_fdb.o br_forward.o br_if.o br_input.o \
			br_ioctl.o br_stp.o br_stp_bpdu.o \
			br_stp_if.o br_stp_timer.o br_netlink.o

bridge-$(CONFIG_SYSFS) += br_sysfs_if.o br_sysfs_br.o

bridge-$(subst m,y,$(CONFIG_BRIDGE_NETFILTER)) += br_nf_core.o

obj-$(CONFIG_BRIDGE_NETFILTER) += br_netfilter.o

bridge-$(CONFIG_BRIDGE_OOP) += br_oop.o

bridge-$(CONFIG_BRIDGE_IGMP_SNOOPING) += br_multicast.o br_mdb.o

bridge-$(CONFIG_BRIDGE_VLAN_FILTERING) += br_vlan.o

obj-$(CONFIG_NETFILTER) += netfilter/

ifdef BCM_KF # defined(CONFIG_BCM_KF_MCAST)
ifneq ($(strip $(CONFIG_BCM_MCAST))$(strip $(CONFIG_BCM_MCAST_MODULE)),)
bridge-y += br_bcm_mcast.o
endif
endif # BCM_KF

ifdef BCM_KF # defined(CONFIG_BCM_KF_BRIDGE_PORT_ISOLATION) || defined(CONFIG_BCM_KF_BRIDGE_STP)
bridge-y += br_notifier.o
endif # BCM_KF

ifdef BCM_KF # defined(CONFIG_BCM_KF_RUNNER)
ifdef CONFIG_BCM_RDPA_BRIDGE
bridge-y += br_fp.o
endif # CONFIG_BCM_RDPA_BRIDGE
endif # BCM_KF
