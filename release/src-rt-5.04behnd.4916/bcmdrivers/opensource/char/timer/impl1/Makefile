# File: bcmdrivers/opensource/char/timer
#
# Makefile for the BCM963xx external timer driver

ext_timer-objs := bcm963xx_timer.o

ifneq ($(strip $(CONFIG_BCM_TIMER)),)
ext_timer-objs += bcm_timer.o
endif

obj-$(CONFIG_BCM_EXT_TIMER) += ext_timer.o
obj-$(CONFIG_BCM96766)	+= timer_bcm63xx.o

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/archer/impl1
EXTRA_CFLAGS += -Werror -Wfatal-errors

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

