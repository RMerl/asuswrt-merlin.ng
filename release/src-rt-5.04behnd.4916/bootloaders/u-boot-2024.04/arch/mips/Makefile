# SPDX-License-Identifier: GPL-2.0+

head-y := arch/mips/cpu/start.o

ifeq ($(CONFIG_SPL_BUILD),y)
head-$(CONFIG_ARCH_JZ47XX) := arch/mips/mach-jz47xx/start.o
head-$(CONFIG_SOC_MT7621) := arch/mips/mach-mtmips/mt7621/spl/start.o
endif

ifeq ($(CONFIG_TPL_BUILD),y)
head-$(CONFIG_SOC_MT7621) := arch/mips/mach-mtmips/mt7621/tpl/start.o
endif

libs-y += arch/mips/cpu/
libs-y += arch/mips/lib/

machine-$(CONFIG_ARCH_ATH79) += ath79
machine-$(CONFIG_ARCH_BMIPS) += bmips
machine-$(CONFIG_ARCH_JZ47XX) += jz47xx
machine-$(CONFIG_MACH_PIC32) += pic32
machine-$(CONFIG_ARCH_MTMIPS) += mtmips
machine-$(CONFIG_ARCH_MSCC) += mscc
machine-${CONFIG_ARCH_OCTEON} += octeon

machdirs := $(patsubst %,arch/mips/mach-%/,$(machine-y))
libs-y += $(machdirs)

PLATFORM_CPPFLAGS += $(patsubst %,-I$(srctree)/%include,$(machdirs))

# Optimize for MIPS architectures
arch-$(CONFIG_CPU_MIPS32_R1) += -march=mips32 -Wa,-mips32
arch-$(CONFIG_CPU_MIPS32_R2) += -march=mips32r2 -Wa,-mips32r2
arch-$(CONFIG_CPU_MIPS32_R6) += -march=mips32r6 -Wa,-mips32r6
arch-$(CONFIG_CPU_MIPS64_R1) += -march=mips64 -Wa,-mips64
arch-$(CONFIG_CPU_MIPS64_R2) += -march=mips64r2 -Wa,-mips64r2
arch-$(CONFIG_CPU_MIPS64_R6) += -march=mips64r6 -Wa,-mips64r6
arch-${CONFIG_CPU_MIPS64_OCTEON} += -march=octeon2

# Allow extra optimization for specific CPUs/SoCs
tune-$(CONFIG_MIPS_TUNE_4KC) += -mtune=4kc
tune-$(CONFIG_MIPS_TUNE_14KC) += -mtune=14kc
tune-$(CONFIG_MIPS_TUNE_24KC) += -mtune=24kc
tune-$(CONFIG_MIPS_TUNE_34KC) += -mtune=34kc
tune-$(CONFIG_MIPS_TUNE_74KC) += -mtune=74kc
tune-${CONFIG_MIPS_TUNE_OCTEON3} += -mtune=octeon2

# Include default header files
cflags-y += -I$(srctree)/arch/mips/include/asm/mach-generic

PLATFORM_CPPFLAGS += $(arch-y) $(tune-y) $(cflags-y)
