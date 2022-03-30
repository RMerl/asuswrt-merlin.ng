
ifeq ($(CONFIG_BCM_KERNEL_CUSTOM),y)

# Linux

obj-y		+= flash_api.o flash_common.o

# All current chips only support SPI NOR, no CFI flash support. 
INC_CFI_FLASH_DRIVER=0

ifeq ($(strip $(INC_CFI_FLASH_DRIVER)),1)
obj-y		+= cfiflash.o
endif


EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -DINC_CFI_FLASH_DRIVER=$(INC_CFI_FLASH_DRIVER)

endif

