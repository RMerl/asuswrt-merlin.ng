#CPU makefile for ARMv8 AArch64 mode. AArch32 use armv7 source but should we need the new feature of
#ARMv8 Aarch32, we can add support for that in armv8 source tree

ALLOBJS += armv8_l1cache.o armv8_cpuinit.o armv8_mmu.o armv8_arena.o

# arm architecture
CFLAGS += -DCONFIG_ARM64 -DCFG_ARMV8_AARCH64 -march=armv8-a -mtune=cortex-a53 -mstrict-align

#CFLAGS += -fno-common

# Aach64 gcc does not have the builtin definition as the armv7 gcc does.
# define here in cflags

CFLAGS += -D__ARMEL__
