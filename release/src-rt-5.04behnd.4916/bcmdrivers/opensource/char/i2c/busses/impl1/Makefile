# File: bcmdrivers/broadcom/char/i2c/busses/Makefile
#
# Makefile for the BCM6816 I2C Adapter+Algorithm driver.
#

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63178_),)
obj-$(CONFIG_BCM_I2C_BUS) += i2c_bcm6xxx.o
endif

EXTRA_CFLAGS += -Werror
 
clean:
	rm -f core *.o *.a *.s .*.cmd *.ko

