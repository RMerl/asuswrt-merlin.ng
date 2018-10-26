#
# Makefile for the RUNNER packet driver
#

pktrunner-objs += pktrunner_preb.o

obj-$(CONFIG_BCM_PKTRUNNER) += pktrunner.o

ifneq ($(strip $(CONFIG_BCM_RDPA)),)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH)
endif

EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc

EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/pktrunner/shared

EXTRA_CFLAGS += -Werror -Wall -Wfatal-errors 

ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
EXTRA_CFLAGS += -DCONFIG_DHD_RUNNER
endif

clean:
	rm -f core *.o *.a *.s

# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precidence

%.o : %.o_tmp
	$(Q)mv $< $@

%.o_tmp : %.$(BCMARCH).o_saved
	@echo "Using $<"
	$(Q)cp $< $@
