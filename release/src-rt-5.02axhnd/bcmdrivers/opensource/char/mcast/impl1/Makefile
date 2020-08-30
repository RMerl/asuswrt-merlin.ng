#
# Makefile for the BCM Multicast module
#

bcmmcast-objs  = bcm_mcast.o bcm_mcast_netlink.o bcm_mcast_if.o 
ifneq ($(strip $(CONFIG_BR_IGMP_SNOOP)),)
bcmmcast-objs += bcm_mcast_igmp.o
endif
ifneq ($(strip $(CONFIG_BR_MLD_SNOOP)),)
bcmmcast-objs += bcm_mcast_mld.o
endif

ifneq ($(and $(strip $(CONFIG_BR_IGMP_SNOOP))$(strip $(CONFIG_BR_MLD_SNOOP)), $(strip $(CONFIG_BLOG))),)
bcmmcast-objs += bcm_mcast_blog.o
endif

obj-$(CONFIG_BCM_MCAST) += bcmmcast.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(KERNEL_DIR)/net/bridge
EXTRA_CFLAGS += -Werror -Wfatal-errors

clean:
	rm -f core *.o *.a .*.cmd *.ko

