ALLOBJS += armv7_arena.o armv7_cpuinit.o armv7_l1cache.o l2cache.o

# arm architecture
CFLAGS += -DCONFIG_ARM -marm -mabi=aapcs-linux -mno-thumb-interwork -mno-unaligned-access

# thumb will hang at ddr_init
CFLAGS += -fno-common -ffixed-r8 -msoft-float -march=armv7-a
#-march=armv6k
CFLAGS += -DCFG_ARM_ARCH_7A
