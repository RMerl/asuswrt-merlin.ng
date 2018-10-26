ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_6846_),)
BSPOBJS +=                        \
	bcm63xx_impl1_common.o \
	bcm63xx_impl1_otp.o
ifeq ($(strip ${CFG_RAMAPP}),1)
BSPOBJS +=                        \
    bcm63xx_impl2_smp.o
endif
ifneq ($(strip $(BRCM_CHIP)),6846)
ifneq ($(strip ${CFG_RAMAPP}),1)
BSPOBJS +=                        \
	bcm63xx_impl1_pmc.o
endif
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_47189_),)
BSPOBJS +=                        \
	bcm63xx_impl3_common.o
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_6858_63158_6856_),)
BSPOBJS +=                        \
	bcm63xx_impl2_common.o    \
	bcm63xx_impl2_otp.o
ifeq ($(strip ${CFG_RAMAPP}),1)
BSPOBJS +=                        \
	bcm63xx_impl2_smp.o
endif
endif

# for cfe_rom/ram 
ifneq ($(strip ${INC_BTRM_BUILD}),1)

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_),)
BSPOBJS += 63138_mmu_map.o
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6846_),)
BSPOBJS += 6846_mmu_map.o
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_47189_),)
BSPOBJS += 47189_mmu_map.o
endif

endif
