archer-objs :=  archer_driver.o archer_host.o archer_ucast.o archer_ucast_l2.o 
archer-objs +=  archer_mcast.o sysport_rsb.o sysport_driver_config.o

ifneq ($(strip $(CONFIG_BCM_CROSSBOW)),)
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_6764_),)
archer-objs += crossbow/crossbow_driver.o
endif
archer-objs += crossbow/crossbow_hash_functions.o crossbow/crossbow_parser.o
EXTRA_CFLAGS += -DCC_SYSPORT_PARSER_V2
endif

ifneq ($(strip $(CONFIG_BCM_CROSSBOW_FULL_OFFLOAD)),)
ifeq ($(strip $(CONFIG_BCM_FPI)),)
EXTRA_CFLAGS += -DCC_CROSSBOW_FW_CLASSIFIER
EXTRA_CFLAGS += -DCC_CROSSBOW_FW_MULTICAST
endif
EXTRA_CFLAGS += -DCC_CROSSBOW_FW_CLASSIFIER_DTCM
EXTRA_CFLAGS += -DCC_CROSSBOW_FW_MULTICAST_DTCM
EXTRA_CFLAGS += -DCC_CROSSBOW_FW_PRINT
# Crossbow Full Offload Mode
archer-objs += crossbow/crossbow_sq.o crossbow/crossbow_spdsvc.o
archer-objs += crossbow/crossbow_classifier.o crossbow/crossbow_queue.o
archer-objs += crossbow/crossbow_natc.o crossbow/fw/crossbow_print.o
archer-objs += crossbow/crossbow_socket.o crossbow/crossbow_wlan.o
archer-objs += crossbow/crossbow_gdx.o
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6765_),)
archer-objs += crossbow/crossbow_fpi.o archer_fpi.o
archer-objs += crossbow/crossbow_cspu.o
EXTRA_CFLAGS += -DCC_CROSSBOW_CSPU -DCC_CROSSBOW_FW_CSPU_DTCM
endif
ifneq ($(strip $(CONFIG_BCM_DHD_ARCHER)),)
ifneq ($(strip $(CONFIG_BCM_DHD_CROSSBOW)),)
EXTRA_CFLAGS += -DCC_CROSSBOW_DoX
archer-objs += crossbow/crossbow_dhd.o crossbow/crossbow_dhd_driver.o archer_dhd_helper.o
else
EXTRA_CFLAGS += -DCC_CROSSBOW_DoAoX
archer-objs +=  archer_dhd.o archer_dhd_driver.o archer_dhd_helper.o archer_thread.o
endif
endif
else
# System Port or Crossbow Partial Offload Mode
archer-objs += sysport_classifier.o sysport_spe.o 
archer-objs += sysport_crc.o archer_socket.o archer_dpi.o archer_iq.o archer_tm.o
archer-objs += sysport_wol.o archer_drop.o archer_wlan.o archer_rl.o archer_spdsvc.o
archer-objs += archer_thread.o sysport_driver.o
ifneq ($(strip $(CONFIG_BCM_DHD_ARCHER)),)
archer-objs +=  archer_dhd.o archer_dhd_driver.o archer_dhd_helper.o
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63178_),)
archer-objs += iudma_driver.o archer_xtmrt.o
EXTRA_CFLAGS += -DCC_ARCHER_DSL
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6756_),)
EXTRA_CFLAGS += -DCC_SYSPORT_PARSER_V2
EXTRA_CFLAGS += -DCC_SYSPORT_MLT_V2
EXTRA_CFLAGS += -DCC_SYSPORT_SW_PADDING
endif

ifneq ($(strip $(CONFIG_BCM_ARCHER_WLAN)),)
# Enable flow control
EXTRA_CFLAGS += -DCC_AWL_FLCTL
# Enable credit based Host Flow Control
ifneq ($(strip $(CONFIG_BCM_PKTFWD_FLCTL)),)
    EXTRA_CFLAGS += -DBCM_PKTFWD_FLCTL
endif
endif

EXTRA_CFLAGS += -DCC_ARCHER_PERFORMANCE

obj-$(CONFIG_BCM_ARCHER) += archer.o

ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
EXTRA_CFLAGS += -DBCM_PKTFWD
endif # CONFIG_BCM_PKTFWD

EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/archer/impl1
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/cmdlist/impl1
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/archer/impl1
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/archer/impl1/crossbow
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/archer/impl1/crossbow/fw
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/archer/impl1/crossbow/fw/dhd
EXTRA_CFLAGS += -I$(BUILD_DIR)/kernel/bcmkernel/include/linux

EXTRA_CFLAGS += -Werror -Wall -Wfatal-errors
EXTRA_CFLAGS += -O3

clean:
	rm -f core *.o *.a *.s

# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precidence

%.o : %.o_tmp
	$(Q)mv $< $@

ifneq ($(CONFIG_BRCM_CHIP_COMPAT_NAME),"")
archer_chip_name := $(subst $\",,$(CONFIG_BRCM_CHIP_COMPAT_NAME))
else
archer_chip_name := $(BRCM_CHIP)
endif
$(info Archer Chip ID = $(archer_chip_name))

%.o_tmp : %.$(archer_chip_name).o_saved
	@echo "Using $<"
	$(Q)cp $< $@
