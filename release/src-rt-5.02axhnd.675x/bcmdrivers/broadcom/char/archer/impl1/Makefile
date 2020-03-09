# File: bcmdrivers/broadcom/char/archer/impl1/Makefile


archer-objs :=  archer_driver.o archer_host.o archer_ucast.o archer_ucast_l2.o 
archer-objs +=  archer_mcast.o sysport_classifier.o sysport_spe.o sysport_rsb.o
archer-objs +=  sysport_crc.o archer_socket.o archer_dpi.o archer_iq.o archer_tm.o
archer-objs +=  sysport_wol.o

ifeq ($(strip $(CONFIG_BCM_ARCHER_SIM)),)
archer-objs += archer_thread.o sysport_driver.o sysport_driver_config.o
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63178_4908_63268_63158_),)
archer-objs += iudma_driver.o archer_xtmrt.o
endif
ifneq ($(strip $(CONFIG_BCM_ARCHER_WLAN)),)
archer-objs += archer_wlan.o
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
