#
# Makefile for bcm2079x NFC driver
#
obj-${CONFIG_BCM_NFC_I2C} += bcm2079x-i2c.o

EXTRA_CFLAGS += -Werror -Wall
EXTRA_CFLAGS += -I${INC_BRCMDRIVER_PUB_PATH}/${BRCM_BOARD}
EXTRA_CFLAGS += -I${INC_BRCMSHARED_PUB_PATH}/bcm963xx
EXTRA_CFLAGS += -I${INC_BRCMDRIVER_PUB_PATH}/bcm963xx

-include ${TOPDIR}/Rules.make

clean:
	rm -f *.[ao] *.ko *.cmd
