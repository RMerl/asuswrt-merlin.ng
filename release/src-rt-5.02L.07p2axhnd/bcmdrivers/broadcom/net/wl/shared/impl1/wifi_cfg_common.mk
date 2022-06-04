ifneq ($(strip $(CONFIG_BCM_KF_NBUFF)),)
EXTRA_CFLAGS += -DBCM_NBUFF_WLMCAST 
ifneq ($(strip $(CONFIG_IPV6)),)
ifneq ($(strip $(CONFIG_BR_MLD_SNOOP)),)
export BUILD_NBUFF_WLMCAST_IPV6=y
EXTRA_CFLAGS += -DBCM_NBUFF_WLMCAST_IPV6  -I$(BRCMDRIVERS_DIR)/opensource/include/bcm963xx
EXTRA_CFLAGS += -DBCM_WMF_MCAST_DBG
endif
endif
endif
ifeq ($(strip $(CONFIG_BCM_WLCSM_DEBUG)),y)
EXTRA_CFLAGS += -DWLCSM_DEBUG -I$(BRCMDRIVERS_DIR)/broadcom/char/wlcsm_ext/impl1/include
endif

# for binary compatible
ifneq ($(strip $(BCA_CPEROUTER)),)
EXTRA_CFLAGS += -DWLBIN_COMPAT
endif

# new dhd lock flag
PWD = $(shell pwd)
DHD_SRC_FILE=$(PWD)/../../bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)/sys/src/dhd/sys/dhd_linux.c
DHD_LOCK = $(shell grep 'DHD_LOCK' $(DHD_SRC_FILE))
ifneq ($(DHD_LOCK),)
EXTRA_CFLAGS += -DBCM_DHD_LOCK
endif

EXTRA_CFLAGS += -I$(WLAN_SHARED_DIR) -DBCA_CPE_BSP_SHARED
