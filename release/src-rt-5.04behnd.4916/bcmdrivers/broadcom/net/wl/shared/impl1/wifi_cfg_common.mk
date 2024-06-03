ifneq ($(strip $(CONFIG_BCM_KF_NBUFF)),)
ifneq ($(strip $(CONFIG_BLOG)),)
EXTRA_CFLAGS += -DBCM_NBUFF_WLMCAST 
ifneq ($(strip $(CONFIG_BCM_UNKNOWN_UCAST)),)
# Enable only when DHD code supports processing connect event that
# flushes flow cache flows on station connect (needed for unknown ucast support)
DHD_SYS_DIR = $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/sys/src/dhd/sys/
CNCT_EVNT = $(shell grep 'DHD_BLOG_CONNECT_EVENT' $(DHD_SYS_DIR)/dhd_common.c)
ifneq ($(CNCT_EVNT),)
EXTRA_CFLAGS += -DBCM_BLOG_UNKNOWN_UCAST
endif
endif
endif

ifneq ($(strip $(CONFIG_BCM_WLAN_RDK_ONEWIFI)),)
EXTRA_CFLAGS += -DRDKB_ONE_WIFI
export RDKB_ONE_WIFI=y
endif

ifneq ($(strip $(CONFIG_BCM_WLAN_RDKB)),)
EXTRA_CFLAGS += -DRDKB -DBCA_CPEROUTER_RDK
export RDKB=y
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
EXTRA_CFLAGS += -DWLCSM_DEBUG -I$(BRCMDRIVERS_DIR)/opensource/char/bcm_knvram/impl1/include
endif

ifneq ($(strip $(CONFIG_BCM_WLAN_NIC_RX_RNR_ACCEL)),)
EXTRA_CFLAGS += -DBCM_WLAN_NIC_RX_RNR_ACCEL -DBCM_WLAN_RXMC_RNR_OFFLOAD
endif

# for binary compatible
ifneq ($(strip $(BCA_CPEROUTER)),)
EXTRA_CFLAGS += -DWLBIN_COMPAT
endif

#Add MLD address in external auth event sent from FW, all the way upto supplicant.
ifneq ($(strip $(CONFIG_BCM_KF_NL80211_EXTAUTH_MLD_ADDR)),)
EXTRA_CFLAGS += -DBCM_WLAN_EXTAUTH_MLD_ADDR
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
# to get DWDS peer's macaddr
EXTRA_CFLAGS += -DBCM_DHD_DWDS_PEER_MACADDR

# This flag can be used in wl code to include wl/shared
# header files. It indicates we have the WLAN_SHARED_DIR
# and are pulling code from there.
EXTRA_CFLAGS += -I$(WLAN_SHARED_DIR) -DBCA_CPE_BSP_SHARED

# SROM MAP support
EXTRA_CFLAGS += -DBCA_SROMMAP
# specify the path for srommap files (default: /etc/wlan)
# EXTRA_CFLAGS += -DSROMMAP_PATH=\"/mnt/defaults/wl\"

# Internal NIC flags
# core ID and interrupt info from VPCIE
ifneq ($(strip $(BCA_CPEROUTER)),)
EXTRA_CFLAGS += -DBCM_VPCIE_WIFI_CORE_INFO
endif

# WLAN Interface Link Update
EXTRA_CFLAGS += -DBCM_PKTFWD_LINK_UPDATE

# BAR Configuration API by PCIe driver
EXTRA_CFLAGS += -DBCM_PCIE_BARCFG

# backward compatible w/ old archer APIs
CL474762 = $(shell grep 'archer_wlan_rx_send' $(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx/bcm_archer.h | grep void_p)
ifneq ($(CL474762),)
EXTRA_CFLAGS += -DCL474762
endif
