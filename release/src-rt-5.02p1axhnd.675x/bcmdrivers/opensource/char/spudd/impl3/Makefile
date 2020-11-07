# File: bcmdrivers/char/spudd/bcm963xx/Makefile
#
# Makefile for the BCM63xx IPSec SPU module
#

bcmspu-objs += spu.o spu_runner.o 
ifneq ($(strip $(CONFIG_BLOG)),)
bcmspu-objs += spu_blog.o
endif

obj-$(CONFIG_BCM_SPU) += bcmspu.o

EXTRA_CFLAGS += -I$(src) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -Werror -Wfatal-errors

clean:
	rm -f core *.o *.a .*.cmd *.ko

