# File: bcmdrivers/char/spudd/bcm963xx/Makefile
#
# Makefile for crypto acceleration files for Broadcom SPU driver
#

bcmspu-objs += spu.o cipher.o util.o
bcmspu-objs += spu2.o spu2_test.o
ifneq ($(strip $(CONFIG_BLOG)),)
bcmspu-objs += spu_blog.o
endif
ifneq ($(strip $(CONFIG_BCM_SPU_HW_OFFLOAD)),)
bcmspu-objs += spu_blog_offload.o
ifneq ($(strip $(CONFIG_BCM_RDPA)),)
bcmspu-objs += spu_offload_xrdp.o
endif
ifneq ($(strip $(CONFIG_BCM_CROSSBOW_FULL_OFFLOAD)),)
bcmspu-objs += spu_offload_cspu.o
endif
endif


obj-$(CONFIG_BCM_SPU) += bcmspu.o

EXTRA_CFLAGS += -I$(src) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/pdc/impl1
EXTRA_CFLAGS += -Werror -Wfatal-errors

ifneq ($(strip $(CONFIG_BCM_SPU_HW_OFFLOAD)),)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH) $(INC_RDP_FLAGS)
endif
clean:
	rm -f core *.o *.a .*.cmd *.ko

