
ifeq ($(strip ${CFG_BOARDNAME}),"BCM963268")
BSPOBJS +=               \
	bcm63xx_impl1_rom_boot.o \
	bcm63xx_impl2_ddr_init.o    
endif

ifeq ($(strip ${CFG_BOARDNAME}),"BCM960333")
BSPOBJS +=               \
	bcm63xx_impl1_rom_boot.o \
	bcm63xx_impl3_ddr_init.o
endif

ifeq ($(strip ${CFG_BOARDNAME}),"BCM963381")
BSPOBJS +=               \
	bcm63xx_impl1_rom_boot.o \
	bcm63xx_impl3_ddr_init.o
endif

ifeq ($(strip ${CFG_BOARDNAME}),"BCM96838")
BSPOBJS +=               \
	bcm63xx_impl2_rom_boot.o \
	bcm63xx_impl4_ddr_init.o
endif

ifeq ($(strip ${CFG_BOARDNAME}),"BCM96848")
LDLIBS += -L $(BOARD_ARCH_SRC) -lmemsysinitlib
BSPOBJS +=               \
    bcm63xx_impl1_rom_boot.o \
    bcm63xx_impl1_ddr_cinit.o \
    bcm63xx_impl1_ddr_mcb.o
endif

ifeq ($(strip ${JTAG_DELAY}),1)
BSPOBJS +=               \
	gpio_loop.o
endif
