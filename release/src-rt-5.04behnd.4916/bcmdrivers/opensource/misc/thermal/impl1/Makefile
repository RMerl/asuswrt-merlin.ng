# File: bcmdrivers/opensource/misc/thermal
#
# Makefile for the hermal throttling driver

obj-$(CONFIG_BCM_THERMAL) += bcm_thermal.o
ifneq ($(strip $(CONFIG_BCM_PON)),)
    bcm_thermal-objs += bcm_thermal_drv.o
    bcm_thermal-objs += bcm_thermal_pon.o
else
ifeq ($(filter-out 4908,${BRCM_CHIP}),)
    bcm_thermal-objs += bcm_thermal_drv.o
    bcm_thermal-objs += bcm_thermal_dsl.o
else
    bcm_thermal-objs += bcm_thermal_old.o
endif
endif

EXTRA_CFLAGS +=  -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -Werror -Wfatal-errors

-include $(TOPDIR)/Rules.make

clean:
	rm -f *.o *.a *.s

