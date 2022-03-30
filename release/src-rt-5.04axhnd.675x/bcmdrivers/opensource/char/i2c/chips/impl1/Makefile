# File: bcmdrivers/broadcom/char/i2c/chips/Makefile
#
# Makefile for I2C supporting device client driver
#
obj-$(CONFIG_BCM_I2C_CHIP) += bcmsfp_i2c.o


EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

#GPON_I2C_TEST for proc/i2c_pon/ponPhyTest
EXTRA_CFLAGS += -Werror -DPROCFS_HOOKS -DGPON_I2C_TEST -DSYSFS_HOOKS
#PROCFS_HOOKS for proc/b3450_reg and proc/i2c_pon/*

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
