ifneq ($(strip $(CONFIG_BCM_KF_NBUFF)),)
ifneq ($(strip $(CONFIG_BLOG)),)
EXTRA_CFLAGS += -DBCM_NBUFF_WLMCAST 
endif
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

# dhd flags
# enable fast flow-ring delete between dhd and dongle
EXTRA_CFLAGS += -DBCM_DHD_DNGL_FFRD
# use max interfaces advertized by dongle
EXTRA_CFLAGS += -DBCM_DHD_DNGL_MAXIFS
# new dhd lock flag
EXTRA_CFLAGS += -DBCM_DHD_LOCK
# AWL TX in DPC
EXTRA_CFLAGS += -DAWL_TX_DPC

# This flag can be used in wl code to include wl/shared
# header files. It indicates we have the WLAN_SHARED_DIR
# and are pulling code from there.
EXTRA_CFLAGS += -I$(WLAN_SHARED_DIR) -DBCA_CPE_BSP_SHARED

# SROM MAP support
EXTRA_CFLAGS += -DBCA_SROMMAP
# specify the path for srommap files (default: /etc/wlan)
# EXTRA_CFLAGS += -DSROMMAP_PATH=\"/mnt/defaults/wl\"

