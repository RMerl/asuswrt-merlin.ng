# File: bcmdrivers/char/vlan/bcm963xx/Makefile
#
# Makefile for the BCM68xx VLAN driver.
#

obj-$(CONFIG_BCM_VLAN) += bcmvlan.o
bcmvlan-objs += bcm_vlan.o bcm_vlan_local.o bcm_vlan_user.o bcm_vlan_dev.o bcm_vlan_rule.o bcm_vlan_test.o bcm_vlan_flows.o bcm_vlan_dp.o

EXTRA_CFLAGS += -I$(obj)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += $(EXTRA_INCLUDES) $(INC_RDP_FLAGS)
ifneq ($(strip $(CONFIG_BCM_RDPA)),)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH)
endif
EXTRA_CFLAGS += -Werror -Wall
ifneq ($(strip $(CONFIG_BCM_GPON)),)
EXTRA_CFLAGS += -DCC_DROP_PRECEDENCE
endif

ifneq ($(strip $(BCM_PON)),)
EXTRA_CFLAGS += -DBCM_PON
EXTRA_CFLAGS += -DCONFIG_BCM_VLAN_ISOLATION
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko

#
# This target is to support builds of binary only vlan module
# in the customer builds.  This target is called from the
# kernel/linux/Makefile before MODPOST.
#
getrealobjects:
#       if [ -e vlan.impl2.tar.gz -a -e bcm_vlan.o ]; then \
#         echo "Restoring real vlan objects..."; \
#         tar -zxf vlan.impl2.tar.gz; \
#         touch bcm_vlan.o PPloamDS.o bcm_vlan_omci.o bcm_vlan_omci_usr.o; \
#         touch bcm_vlan_omci_proc.o bcm_vlan_fpga.o \
#         touch built-in.o; \
#         sleep 1; \
#         touch bcmvlan.o; \
#         touch bcmvlan.mod.o; \
#         touch bcmvlan.ko; \
#       fi

