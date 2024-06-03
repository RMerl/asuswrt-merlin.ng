#
# Makefile for the sw_gso
#

MODULE_NAME=sw_gso
BCMARCH=$(subst ",,$(CONFIG_BCM_CPU_ARCH_NAME))

obj-$(CONFIG_BCM_SW_GSO_THREAD) = sw_gso.o
$(MODULE_NAME)-objs := sw_gso_hook.o sw_gso_bin.o

SRC_DIR = $(BRCMDRIVERS_DIR)/broadcom/char/sw_gso/bcm9$(BRCM_CHIP)
HAVESRC = $(wildcard $(SRC_DIR)/sw_gso_bin.c)

ifeq ($(strip $(HAVESRC)),)
$(warning module $(MODULE_NAME) Binary Build)
$(warning use binary $(obj)/sw_gso_bin.$(BCMARCH).o_saved)
else
$(warning module $(MODULE_NAME) Source Build)
endif

EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx -I$(BRCMDRIVERS_DIR)/opensource/include/bcm963xx
EXTRA_CFLAGS += $(EXTRA_INCLUDES) -Werror -Wall

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko

# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precidence

%.o : %.o_tmp
	$(Q)mv $< $@

%.o_tmp : %.$(BCMARCH).o_saved
	@echo "Using $<"
	$(Q)cp $< $@
