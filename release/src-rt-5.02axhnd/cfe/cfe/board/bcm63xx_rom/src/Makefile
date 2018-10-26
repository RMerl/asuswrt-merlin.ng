# The ddr module are arch depend so moved to arch/$CPU/board/bcm63xx makefile
ifeq ($(strip ${INC_SPI_FLASH_DRIVER}),1)
ifeq ($(strip ${INC_SPLIT_NOR_SECONDARY_STAGE}),1)
BSPOBJS += bcm63xx_boot_nor_split.o
else
BSPOBJS +=               \
	bcm63xx_boot_nor.o
endif
endif

ifeq ($(strip ${INC_NAND_FLASH_DRIVER}),1)
BSPOBJS +=               \
	nandflash.o \
        bcm_ubi.o   \
	bcm63xx_boot_nand.o

# supporting spi nand except the chips in the list
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_60333_63268_6838_63148_47189_),)
ifeq ($(strip ${INC_SPI_NAND_DRIVER}),1)
BSPOBJS +=               \
	bcmSpiRes.o \
	bcmHsSpi.o  \
	spinandflash.o
endif
endif
endif


ifeq ($(strip ${INC_EMMC_FLASH_DRIVER}),1)
BSPOBJS +=               \
	cfe_gpt_common.o      \
	dev_bcm63xx_emmc_common.o      \
	rom_emmc_drv.o \
	bcm63xx_boot_emmc.o
endif

ifeq ($(strip ${INC_PMC_DRIVER}),1)
BSPOBJS +=               \
	pmc_drv.o            \
	clk_rst.o
endif

ifeq ($(strip $(BRCM_CHIP)),6858)
BSPOBJS +=               \
	bcm_otp_impl1.o
ifeq ($(strip ${BOOT_PRE_CFE}),1)
BSPOBJS +=               \
	nandflash.o          \
	bcm_pinmux_impl1.o
endif
endif

ifeq ($(strip $(BRCM_CHIP)),4908)
ifeq ($(strip ${BOOT_PRE_CFE}),1)
BOARD_INC       += ${TOP}/board/${BOARD}_btrm/inc ${TOP}/otp
BSPOBJS +=               \
	bcm63xx_boot_precfe.o  \
	bcmSpiRes.o \
	bcmHsSpi.o \
	spinandflash.o
endif
endif

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_60333_6848_47189_6858_),)
BSPOBJS +=               \
	bcm_otp_impl1.o
endif

ifeq ($(strip $(BRCM_CHIP)),6838)
BSPOBJS +=               \
	bcmSpiRes.o \
	bcmHsSpi.o
ifeq ($(strip ${INC_SPI_NAND_DRIVER}),1)
BSPOBJS +=               \
	spinandflash.o
else
BSPOBJS +=               \
	spiflash.o
endif
endif

ifeq ($(strip ${INC_MEMSYS_INIT}),1)
BSPOBJS +=               \
	bcm63xx_ddr.o
endif

BSPOBJS +=              \
	bcm63xx_main.o  \
	bcm63xx_boot.o  \
	bcm63xx_clock.o \
	bcm63xx_init.o  \
	bcm63xx_utils.o



CFLAGS += $(CFE_PROFILE_DEFS)
