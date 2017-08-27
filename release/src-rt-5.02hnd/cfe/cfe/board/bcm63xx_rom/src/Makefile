# The ddr module are arch depend so moved to arch/$CPU/board/bcm63xx makefile
ifeq ($(strip ${INC_NAND_FLASH_DRIVER}),1)
BSPOBJS +=               \
	nandflash.o \
        bcm_ubi.o

# meaning if (BRCM_CHIP == 63381) || (BRCM_CHIP == 63138)
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63381_63138_6848_4908_6858_68360_),)
ifeq ($(strip ${INC_SPI_NAND_DRIVER}),1)
BSPOBJS +=               \
	bcmSpiRes.o \
	bcmHsSpi.o  \
	spinandflash.o
endif
endif
endif

ifeq ($(strip $(BRCM_CHIP)),63381)
BSPOBJS +=               \
	pmc_drv.o
endif

ifeq ($(strip $(BRCM_CHIP)),6858)
BSPOBJS +=               \
	bcm_otp_impl1.o      \
	pmc_drv.o            \
	clk_rst.o
ifeq ($(strip ${BOOT_PRE_CFE}),1)
BSPOBJS +=               \
    nandflash.o          \
	bcm_pinmux_impl1.o
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_4908_6838_63381_63268_68360_),)
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


BSPOBJS +=           \
	bcm63xx_main.o 

