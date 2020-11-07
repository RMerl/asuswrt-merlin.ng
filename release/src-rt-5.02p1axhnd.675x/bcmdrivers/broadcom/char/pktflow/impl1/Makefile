# File: bcmdrivers/char/fcache/bcm963xx/Makefile
#
# Makefile for the BCM63xx Flow Forwarding Cache module
#
BCMARCH=$(subst ",,$(CONFIG_BCM_CPU_ARCH_NAME))
# " #(matching quote so some editors don't complain)

EXTRA_CFLAGS += ${DSL_XTM_SAR_TX_RX_IUDMA_631XX_DEFINES}

ifneq ($(CONFIG_BCM_WLAN),)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/$(LN_NAME)/wl/wlcsm_ext/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/$(LN_NAME)/main/src/wl/wlcsm_ext/include
endif

pktflow-objs   := fcache.o fcachedrv.o flwstats.o pathstats.o

ifeq  ($(strip $(CONFIG_BCM_FHW)),y)
pktflow-objs   += fcachehw.o
endif

obj-$(CONFIG_BCM_PKTFLOW) += pktflow.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) $()
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared -I$(INC_BCMLIBS_PATH)

ifneq ($(strip $(CONFIG_BCM_RDPA)),)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
endif
EXTRA_CFLAGS += -Werror -Wfatal-errors

ifeq  ($(strip $(BRCM_DRIVER_PKTFLOW_DEBUG)),y)
EXTRA_CFLAGS += -DPKTDBG
endif

ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
EXTRA_CFLAGS += -DCONFIG_DHD_RUNNER
endif

# -include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s


# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precidence

%.o : %.o_tmp
	$(Q)mv $< $@

%.o_tmp : %.$(BCMARCH).o_saved
	@echo "Using $<"
	$(Q)cp $< $@


