# File: bcmdrivers/broadcom/char/spdsvc/impl1/Makefile
#
# Makefile for the BCM63xx Speed Service driver.
#

obj-$(CONFIG_BCM_SPDSVC) += bcm_spdsvc.o
bcm_spdsvc-objs += spdsvc.o spdsvc_cpu.o
bcm_spdsvc-objs += spdsvc_procfs.o

bcm_spdsvc-objs += spdsvc_tr471_offload.o spdsvc_tr471.o
EXTRA_CFLAGS += -DTR471_TIME_DATA_DEBUG

ifneq ($(strip $(CONFIG_BCM_RDPA)),)
bcm_spdsvc-objs += spdsvc_runner.o
EXTRA_CFLAGS += -DSPDSVC_HW

EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH) $(INC_RDP_FLAGS)
endif

ifneq ($(strip $(CONFIG_BCM_ARCHER)),)
bcm_spdsvc-objs += spdsvc_archer.o
EXTRA_CFLAGS += -DSPDSVC_HW
endif

EXTRA_CFLAGS += -I$(obj)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/spdsvc/impl1
EXTRA_CFLAGS += $(EXTRA_INCLUDES)
EXTRA_CFLAGS += -Werror -Wall

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko

# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precedence

%.o : %.o_tmp
	$(Q)mv $< $@

%.o_tmp : %.$(CONFIG_BCM_CHIP_NUMBER).o_saved
	@echo "Using $<"
	$(Q)cp $< $@

