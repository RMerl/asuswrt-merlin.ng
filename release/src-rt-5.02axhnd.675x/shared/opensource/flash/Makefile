
ifeq ($(CONFIG_BCM_KERNEL_CUSTOM),y)

# Linux

obj-y		+= flash_api.o flash_common.o

# All current chips only support SPI NOR, no CFI flash support. 
INC_CFI_FLASH_DRIVER=0
INC_SPI_FLASH_DRIVER=1

ifeq ($(strip $(INC_CFI_FLASH_DRIVER)),1)
obj-y		+= cfiflash.o
endif
ifeq ($(strip $(INC_SPI_FLASH_DRIVER)),1)
ifeq ($(strip $(BRCM_CHIP)),47189)
obj-y		+= spiflash_impl2.o
else
obj-y		+= spiflash.o
endif
endif


EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -DINC_CFI_FLASH_DRIVER=$(INC_CFI_FLASH_DRIVER) -DINC_SPI_FLASH_DRIVER=$(INC_SPI_FLASH_DRIVER)

endif

