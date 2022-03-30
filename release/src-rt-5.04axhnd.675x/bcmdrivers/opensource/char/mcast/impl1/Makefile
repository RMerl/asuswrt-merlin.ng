#
# Makefile for the BCM Multicast module
#

bcmmcast-objs  = bcm_mcast.o bcm_mcast_netlink.o bcm_mcast_if.o 
ifneq ($(strip $(CONFIG_BR_IGMP_SNOOP)),)
bcmmcast-objs += bcm_mcast_igmp.o bcm_mcast_igmpsnp.o
endif
ifneq ($(strip $(CONFIG_BR_MLD_SNOOP)),)
bcmmcast-objs += bcm_mcast_mld.o bcm_mcast_mldsnp.o
endif

ifneq ($(and $(strip $(CONFIG_BR_IGMP_SNOOP))$(strip $(CONFIG_BR_MLD_SNOOP)), $(strip $(CONFIG_BLOG))),)
bcmmcast-objs += bcm_mcast_blog.o bcm_mcast_blogrule.o bcm_mcast_fc.o bcm_mcast_whitelist.o bcm_mcast_sysfs.o
endif

ifeq ($(strip $(CONFIG_BCM_PLATFORM_RDP_PRV)),y)
ifeq ($(strip $(CONFIG_BCM_55153_DPU)),)
bcmmcast-objs += bcm_mcast_prv.o
endif
endif

obj-$(CONFIG_BCM_MCAST) += bcmmcast.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(KERNEL_DIR)/net/bridge
EXTRA_CFLAGS += -Werror -Wfatal-errors

ifneq ($(strip $(CONFIG_BCM_XRDP)),)
	EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH) $(INC_RDP_FLAGS)
endif

clean:
	rm -f core *.o *.a .*.cmd *.ko

