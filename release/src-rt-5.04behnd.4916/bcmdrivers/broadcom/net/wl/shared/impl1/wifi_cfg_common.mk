ifneq ($(strip $(CONFIG_BCM_KF_NBUFF)),)
ifneq ($(strip $(CONFIG_BLOG)),)
EXTRA_CFLAGS += -DBCM_NBUFF_WLMCAST 
ifneq ($(strip $(CONFIG_BCM_UNKNOWN_UCAST)),)
# Enable unknown ucast support
EXTRA_CFLAGS += -DBCM_BLOG_UNKNOWN_UCAST
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
# Use 'wlan_cma' reserved memory if available
EXTRA_CFLAGS += -DBCM_CMA_RSV_MEM

# https://lore.kernel.org/all/20230116125058.1604843-1-quic_vjakkam@quicinc.com
# upstream commit 9a47c1ef5a95d adds mld_addr to cfg80211_external_auth_params
# which allows authentication offload to user space for MLO connections in STA mode.
# This backport is necessary for the WLAN driver to send MLD addr along with bssid
# to the supplicant during SAE authentication. This change has been back-ported
# from linux 6.3 to our router kernel, and we use the flag 'BCM_DHD_EXTAUTH_MLD_ADDR'
# to indicate that the backport exists in the current kernel.
EXTRA_CFLAGS += -DBCM_DHD_EXTAUTH_MLD_ADDR

# This flag can be used in wl code to include wl/shared
# header files. It indicates we have the WLAN_SHARED_DIR
# and are pulling code from there.
EXTRA_CFLAGS += -I$(WLAN_SHARED_DIR) -DBCA_CPE_BSP_SHARED

# Internal NIC flags
# core ID and interrupt info from VPCIE
ifneq ($(strip $(BCA_CPEROUTER)),)
EXTRA_CFLAGS += -DBCM_VPCIE_WIFI_CORE_INFO
endif

# WLAN Interface Link Update
EXTRA_CFLAGS += -DBCM_PKTFWD_LINK_UPDATE

# BAR Configuration API by PCIe driver
EXTRA_CFLAGS += -DBCM_PCIE_BARCFG

ifneq ($(strip $(CONFIG_BCM_FPI)),)
EXTRA_CFLAGS += -DBCM_WLAN_FPI
endif

# NIC-SQS/NIC-SPP depend on PKTFWD
ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
  # Enable NIC-SPP for SoC platforms
  # For all other platforms, enable NIC-SPP if MLO is supported
  ifneq ($(strip $(CONFIG_BCM_CROSSBOW))$(strip $(CONFIG_BCM_WLAN_MLO)),)
    WLSQS := 1
    WLSPP_NIC := 1
  endif
endif

# Enable only for the SoC platforms
ifneq ($(strip $(CONFIG_BCM_CROSSBOW)),)
  ifneq ($(strip $(CONFIG_BCM_BPM)),)
    PP_XPM := 1
    ifneq ($(strip $(CONFIG_BCM_PKTFWD_SPC)),)
      BCM_PKTFWD_SPC := 1
    endif # CONFIG_BCM_PKTFWD_SPC
    ifneq ($(strip $(CONFIG_HAVE_SKBUFF_WL_SPC_V2)),)
      ifneq ($(strip $(CONFIG_BCM_SPC_RX)),)
        # Enable SPC Rx only when full offload and SPC_V2 is enabled
        BCM_SPC_RX := 1
      endif # CONFIG_BCM_SPC_RX
    endif # CONFIG_HAVE_SKBUFF_WL_SPC_V2
  endif # CONFIG_BCM_BPM
endif # CONFIG_BCM_CROSSBOW

ifneq ($(strip $(CONFIG_BCM_SCORE)),)
  BCM_PKT_TLM := 1
endif # CONFIG_BCM_SCORE
