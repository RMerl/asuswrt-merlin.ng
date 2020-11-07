# File: bcmdrivers/char/ext_bonding/bcm963xx/Makefile
#
# Makefile for the BCM63xx ext_bonding driver.
#

obj-$(CONFIG_BCM_EXT_BONDING) += ext_bonding.o
ext_bonding-objs := ext_bonding_drv.o

EXTRA_CFLAGS += -DCONFIG_$(BRCM_CHIP) -fno-exceptions -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_ADSLDRV_PATH) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -Werror -Wfatal-errors

# Some compiler flags which are defined at the top level Makefile are not
# applicable to C++ files.  Filter them out to avoid warnings.  This is a
# brute force approach, but is safer than modifying the top level Makefiles.
STRIP_FROM_CPP := -Wstrict-prototypes -Werror-implicit-function-declaration \
                  -Wdeclaration-after-statement -Wno-pointer-sign
clean:
	rm -f core *.o *.a *.s
