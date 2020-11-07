# File: bcmdrivers/char/xtmcfg/bcm963xx/Makefile
#
# Makefile for the BCM63xx bcmxtmcfg driver.
#

obj-$(CONFIG_BCM_XTMCFG) += bcmxtmcfg.o
bcmxtmcfg-objs := xtmcfgdrv.o xtmcfgmain.o xtmprocessor.o xtminterface.o xtmconnection.o xtmoamhandler.o xtmoslinux.o xtmasmhandler.o xtmautosense.o

EXTRA_CFLAGS += ${DSL_XTM_SAR_TX_RX_IUDMA_631XX_DEFINES}

EXTRA_CFLAGS += -DCONFIG_$(BRCM_CHIP) -fno-exceptions -fno-rtti -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_ADSLDRV_PATH) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc
EXTRA_CFLAGS += -Werror -Wfatal-errors

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63268_63138_63381_63148_63158_63178_),_$(strip $(BRCM_CHIP))_)
EXTRA_CFLAGS += -DCONFIG_VDSL_SUPPORTED
endif

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63158_),_$(strip $(BRCM_CHIP))_)
EXTRA_CFLAGS += -DXTM_PORT_SHAPING
endif

EXTRA_CFLAGS += ${BUILD_DSL_FLAGS}

# no-rtti is not applicable to c files
CFLAGS_REMOVE_xtmcfgdrv.o := -fno-rtti
CFLAGS_REMOVE_xtmoslinux.o := -fno-rtti

# Some compiler flags which are defined at the top level Makefile are not
# applicable to C++ files.  Filter them out to avoid warnings.  This is a
# brute force approach, but is safer than modifying the top level Makefiles.
STRIP_FROM_CPP := -Wstrict-prototypes -Werror-implicit-function-declaration \
                  -Wdeclaration-after-statement -Wno-pointer-sign -std=gnu89 -std=gnu90
CFLAGS_REMOVE_xtmcfgmain.o := $(STRIP_FROM_CPP)
CFLAGS_REMOVE_xtmprocessor.o := $(STRIP_FROM_CPP)
CFLAGS_REMOVE_xtminterface.o := $(STRIP_FROM_CPP)
CFLAGS_REMOVE_xtmconnection.o := $(STRIP_FROM_CPP)
CFLAGS_REMOVE_xtmoamhandler.o := $(STRIP_FROM_CPP)
CFLAGS_REMOVE_xtmasmhandler.o := $(STRIP_FROM_CPP)
CFLAGS_REMOVE_xtmautosense.o := $(STRIP_FROM_CPP)


clean:
	rm -f core *.o *.a *.s

