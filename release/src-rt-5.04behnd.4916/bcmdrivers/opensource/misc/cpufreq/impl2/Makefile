#
# Makefile for the bcmsmc cpufreq module
#
obj-$(CONFIG_BCM_CPUFREQ) += bcmsmc-cpufreq.o

EXTRA_CFLAGS += -I. -I$(src)/../include/
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

EXTRA_CFLAGS += -Werror -Wfatal-errors

clean:
	rm -f core *.ko *.o *.a *.s
