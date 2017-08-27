ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_),)
BSPOBJS +=                        \
	bcm63xx_impl1_common.o    \
	bcm63xx_impl1_otp.o
ifneq ($(strip ${CFG_RAMAPP}),1)
BSPOBJS +=                        \
	bcm63xx_impl1_pmc.o
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_47189_),)
BSPOBJS +=                        \
	bcm63xx_impl3_common.o
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_6858_63158_68360_),)
BSPOBJS +=                        \
	bcm63xx_impl2_common.o    \
	bcm63xx_impl2_otp.o       \
	bcm63xx_impl2_smp.o
endif
