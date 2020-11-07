#
# Makefile for the bcm63xx clock modules
#

obj-$(CONFIG_BCM_COMMON_CLK_I2S)	+= clk-bcm63xxx-i2s.o
obj-$(CONFIG_SND_BCM6856_SOC_I2S)	+= clk-bcm68560-i2s.o
obj-$(CONFIG_BCM_COMMON_CLK)		+= clk-bcm63xx.o

EXTRA_CFLAGS += -I. -I$(src)/../include/
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I${INC_BRCMSHARED_PUB_PATH}
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc

EXTRA_CFLAGS += -Werror -Wfatal-errors

clean:
	rm -f core *.ko *.o *.a *.s