#
# Makefile for the bcm63xx clock modules
#

obj-$(CONFIG_BCM_COMMON_CLK_I2S)	+= clk-bcm63xxx-i2s.o
obj-$(CONFIG_BCM_COMMON_CLK_I2S_TDM)	+= clk-bcm63xxx-i2s-tx-merritt.o clk-bcm63xxx-i2s-rx-merritt.o
obj-$(CONFIG_BCM_COMMON_CLK_I2S_TDM_SINGLE_CLK) += clk-bcm63xxx-i2s-merritt-single-clk.o
obj-$(CONFIG_BCM_COMMON_CLK_PCM)	+= clk-bcm63xxx-pcm.o
obj-$(CONFIG_SND_BCM6856_SOC_I2S)	+= clk-bcm68560-i2s.o
obj-$(CONFIG_BCM_COMMON_CLK)		+= clk-bcm63xx.o
ifneq ($(strip $(CONFIG_BCM_COMMON_CLK)),)
obj-$(CONFIG_BCM963138)				+= clk-bcm-arma9.o
obj-$(CONFIG_BCM963148)				+= clk-bcm-b15.o
obj-$(CONFIG_BCM94908)				+= clk-bcm-b53.o
obj-$(CONFIG_BCM96858)				+= clk-bcm-b53.o
endif

EXTRA_CFLAGS += -I. -I$(src)/../include/
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I${INC_BRCMSHARED_PUB_PATH}

EXTRA_CFLAGS += -Werror -Wfatal-errors

clean:
	rm -f core *.ko *.o *.a *.s
