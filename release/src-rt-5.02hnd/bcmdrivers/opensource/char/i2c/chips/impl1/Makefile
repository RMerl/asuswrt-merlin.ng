# File: bcmdrivers/broadcom/char/i2c/chips/Makefile
#
# Makefile for BCM3450(MOCA Laser), GPON Phy I2C, and generic Laser client drivers 
#
# 63138_A0 will not come to here, so don't need to screen A0
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_4908_),)
obj-$(CONFIG_BCM_I2C_CHIP) += bcm3450.o
obj-$(CONFIG_BCM_I2C_CHIP) += gpon_i2c.o
obj-$(CONFIG_BCM_I2C_CHIP) += laser_i2c.o
else
obj-$(CONFIG_BCM_I2C_CHIP) += sfp_i2c.o
endif


EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

#MOCA_I2C_TEST for proc/b3450reg
#EXTRA_CFLAGS += -Werror -DPROCFS_HOOKS -DMOCA_I2C_TEST
#GPON_I2C_TEST for proc/i2c_gpon/gponPhyTest
EXTRA_CFLAGS += -Werror -DPROCFS_HOOKS -DGPON_I2C_TEST -DMOCA_I2C_TEST
#PROCFS_HOOKS for proc/b3450_reg and proc/i2c_gpon/*
#EXTRA_CFLAGS += -Werror -DPROCFS_HOOKS 

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
