#
# Makefile for the Command List driver
#

cmdlist-objs :=  cmdlist_driver.o cmdlist_ucast.o cmdlist_l2_ucast.o cmdlist_mcast.o

ifneq ($(strip $(CONFIG_BCM_ARCHER)),)
RUNNER_BUILD = n
else
ifneq ($(strip $(CONFIG_BCM_RDPA)),)
RUNNER_BUILD = y
endif
endif

ifeq ("$(RUNNER_BUILD)", "y")
# Runner
cmdlist-objs += gpe_api.o
ifneq ($(strip $(CONFIG_BCM_CMDLIST_SIM)),)
cmdlist-objs += gpe_sim.o
cmdlist-objs += runner_sim.o
endif
EXTRA_CFLAGS += -DCONFIG_CMDLIST_GPE
else
# System Port
cmdlist-objs += spe_api.o
ifneq ($(strip $(CONFIG_BCM_CMDLIST_SIM)),)
cmdlist-objs += spe_sim.o
cmdlist-objs += sysport_sim.o
cmdlist-objs += sysport_parser.o
cmdlist-objs += sysport_rsb.o
cmdlist-objs += sysport_crc.o
cmdlist-objs += sysport_classifier.o
ARCHER_PATH = $(BRCMDRIVERS_DIR)/broadcom/char/archer/impl1
CMDLIST_PATH = $(BRCMDRIVERS_DIR)/broadcom/char/cmdlist/impl1
$(shell rm -f $(CMDLIST_PATH)/sysport_parser_inline.h)
$(shell ln -s -f $(ARCHER_PATH)/sysport_parser_inline.h $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_parser.h)
$(shell ln -s -f $(ARCHER_PATH)/sysport_parser.h $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_rsb.c)
$(shell ln -s -f $(ARCHER_PATH)/sysport_rsb.c $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_rsb.h)
$(shell ln -s -f $(ARCHER_PATH)/sysport_rsb.h $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_crc.c)
$(shell ln -s -f $(ARCHER_PATH)/sysport_crc.c $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_classifier.c)
$(shell ln -s -f $(ARCHER_PATH)/sysport_classifier.c $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_classifier.h)
$(shell ln -s -f $(ARCHER_PATH)/sysport_classifier.h $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_classifier_inline.h)
$(shell ln -s -f $(ARCHER_PATH)/sysport_classifier_inline.h $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_spe.c)
$(shell ln -s -f $(ARCHER_PATH)/sysport_spe.c $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_spe.h)
$(shell ln -s -f $(ARCHER_PATH)/sysport_spe.h $(CMDLIST_PATH))
endif
EXTRA_CFLAGS += -DCONFIG_CMDLIST_SPE
endif

ifneq ($(PREBUILT_EXTRAMOD),1)
bcmcmdlist-objs += cmdlist.o
else
cmdlist-objs := cmdlist$(PRBM_EXT).o
endif

obj-$(CONFIG_BCM_CMDLIST) += cmdlist.o

EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/archer/impl1/
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH)

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
