#
# Makefile for the BCM63xx Speed Service GPL driver.
#

obj-$(CONFIG_BCM_SPDSVC) += bcm_spdsvc_gpl.o
bcm_spdsvc_gpl-objs += spdsvc_gpl.o

EXTRA_CFLAGS += -I$(obj)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
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

