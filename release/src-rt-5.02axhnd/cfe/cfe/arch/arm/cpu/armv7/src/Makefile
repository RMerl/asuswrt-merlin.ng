ALLOBJS += armv7_l1cache.o armv7_cpuinit.o mmu_map.o

ifneq ($(strip $(INC_BTRM_BOOT)),1)
ALLOBJS += armv7_arena.o
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_47189_6846_),)
ALLOBJS += l2cache.o
endif
else

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_6846_),)
#6846 btrm doesn't link to arena
ALLOBJS += armv7_arena.o
endif

endif

# arm architecture
CFLAGS += -DCONFIG_ARM -marm -mabi=aapcs-linux -mno-thumb-interwork -mno-unaligned-access

# thumb will hang at ddr_init
CFLAGS += -fno-common -ffixed-r8 -msoft-float -march=armv7-a
#-march=armv6k
CFLAGS += -DCFG_ARM_ARCH_7A
