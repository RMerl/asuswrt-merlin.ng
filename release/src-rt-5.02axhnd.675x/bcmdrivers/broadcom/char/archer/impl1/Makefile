# File: bcmdrivers/broadcom/char/archer/impl1/Makefile


archer-objs :=  archer_driver_$(BCMARCH).o_saved archer_host_$(BCMARCH).o_saved archer_ucast_$(BCMARCH).o_saved archer_ucast_l2_$(BCMARCH).o_saved 
archer-objs +=  archer_mcast_$(BCMARCH).o_saved sysport_classifier.o sysport_spe.o sysport_rsb_$(BCMARCH).o_saved
archer-objs +=  sysport_crc.o archer_socket.o archer_dpi.o archer_iq_$(BCMARCH).o_saved archer_tm.o
archer-objs +=  sysport_wol_$(BCMARCH).o_saved

ifeq ($(strip $(CONFIG_BCM_ARCHER_SIM)),)
archer-objs += archer_thread.o sysport_driver.o sysport_driver_config.o
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63178_4908_63268_63158_),)
archer-objs += iudma_driver.o archer_xtmrt_$(BCMARCH).o_saved
endif
ifneq ($(strip $(CONFIG_BCM_ARCHER_WLAN)),)
archer-objs += archer_wlan_$(BCMARCH).o_saved
else
archer-objs += archer_wfd.o
endif
EXTRA_CFLAGS += -DCC_ARCHER_PERFORMANCE
else
archer-objs += archer_sim.o sysport_mlt_hash.o sysport_mlt.o
EXTRA_CFLAGS += -DCC_ARCHER_SIM_FC_HOOK
ARCHER_PATH = $(BRCMDRIVERS_DIR)/broadcom/char/archer/impl1
CMDLIST_PATH = $(BRCMDRIVERS_DIR)/broadcom/char/cmdlist/impl1
endif

obj-$(CONFIG_BCM_ARCHER) += archer.o

ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
EXTRA_CFLAGS += -DBCM_PKTFWD
endif # CONFIG_BCM_PKTFWD

EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/cmdlist/impl1
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH)

EXTRA_CFLAGS += -Werror -Wall -Wfatal-errors -g
EXTRA_CFLAGS += -O3

clean:
	rm -f core *.o *.a *.s

# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precidence

%.o : %.o_tmp
	$(Q)mv $< $@

%.o_tmp : %.$(CONFIG_BCM_CHIP_NUMBER)_impl${CONFIG_BCM_WLAN_IMPL}.o_saved
	@echo "Using $<"
	$(Q)cp $< $@
