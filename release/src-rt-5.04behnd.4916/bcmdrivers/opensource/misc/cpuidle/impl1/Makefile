# Makefile for bcm arm cpuidle driver

ifeq ($(filter-out 63148,${BRCM_CHIP}),)
# legacy b15 cpu idle dirver
obj-$(CONFIG_BCM_B15_CPUIDLE) += bcm_b15_wait.o
else
obj-$(CONFIG_BCM_ARM_CPUIDLE) += bcm_arm_cpuidle.o
endif

EXTRA_CFLAGS += -Werror -Wfatal-errors

-include $(TOPDIR)/Rules.make

clean:
	rm -f *.o *.a *.s
