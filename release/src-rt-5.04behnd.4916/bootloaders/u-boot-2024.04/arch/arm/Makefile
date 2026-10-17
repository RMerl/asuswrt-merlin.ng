# SPDX-License-Identifier: GPL-2.0+

ifeq ($(CONFIG_SPL_BUILD)$(CONFIG_ARCH_TEGRA),yy)
CONFIG_CPU_V7A=
CONFIG_CPU_ARM720T=y
endif

# This selects which instruction set is used.
arch-$(CONFIG_CPU_ARM720T)	=-march=armv4
arch-$(CONFIG_CPU_ARM920T)	=-march=armv4t
arch-$(CONFIG_CPU_ARM926EJS)	=-march=armv5te
arch-$(CONFIG_CPU_ARM946ES)	=-march=armv5te
arch-$(CONFIG_CPU_ARM1136)	=-march=armv5t
arch-$(CONFIG_CPU_ARM1176)	=-march=armv5t
arch-$(CONFIG_CPU_V7A)		=$(call cc-option, -march=armv7-a, \
				 $(call cc-option, -march=armv7))
arch-$(CONFIG_CPU_V7M)		=-march=armv7-m
arch-$(CONFIG_CPU_V7R)		=-march=armv7-r
ifeq ($(CONFIG_ARM64_CRC32),y)
arch-$(CONFIG_ARM64)		=-march=armv8-a+crc
else
arch-$(CONFIG_ARM64)		=-march=armv8-a
endif

# On Tegra systems we must build SPL for the armv4 core on the device
# but otherwise we can use the value in CONFIG_SYS_ARM_ARCH
ifeq ($(CONFIG_SPL_BUILD)$(CONFIG_ARCH_TEGRA),yy)
arch-y += -D__LINUX_ARM_ARCH__=4
else
arch-y += -D__LINUX_ARM_ARCH__=$(CONFIG_SYS_ARM_ARCH)
endif

# Evaluate arch cc-option calls now
arch-y := $(arch-y)

# This selects how we optimise for the processor.
tune-$(CONFIG_CPU_ARM720T)	=-mtune=arm7tdmi
tune-$(CONFIG_CPU_ARM920T)	=
tune-$(CONFIG_CPU_ARM926EJS)	=
tune-$(CONFIG_CPU_ARM946ES)	=
tune-$(CONFIG_CPU_ARM1136)	=
tune-$(CONFIG_CPU_ARM1176)	=
tune-$(CONFIG_CPU_V7A)		=-mtune=generic-armv7-a
tune-$(CONFIG_CPU_V7R)		=
tune-$(CONFIG_ARM64)		=

# Evaluate tune cc-option calls now
tune-y := $(tune-y)

PLATFORM_CPPFLAGS += $(arch-y) $(tune-y)

# Machine directory name.  This list is sorted alphanumerically
# by CONFIG_* macro name.
machine-$(CONFIG_ARCH_APPLE)		+= apple
machine-$(CONFIG_ARCH_ASPEED)		+= aspeed
machine-$(CONFIG_ARCH_AT91)		+= at91
machine-$(CONFIG_ARCH_BCM283X)		+= bcm283x
machine-$(CONFIG_ARCH_BCMSTB)		+= bcmstb
machine-$(CONFIG_ARCH_BCMBCA)		+= bcmbca
machine-$(CONFIG_ARCH_DAVINCI)		+= davinci
machine-$(CONFIG_ARCH_EXYNOS)		+= exynos
machine-$(CONFIG_ARCH_GXP)		+= hpe
machine-$(CONFIG_ARCH_HIGHBANK)		+= highbank
machine-$(CONFIG_ARCH_HISTB)		+= histb
machine-$(CONFIG_ARCH_IPQ40XX)		+= ipq40xx
machine-$(CONFIG_ARCH_K3)		+= k3
machine-$(CONFIG_ARCH_KEYSTONE)		+= keystone
machine-$(CONFIG_ARCH_KIRKWOOD)		+= kirkwood
machine-$(CONFIG_ARCH_LPC32XX)		+= lpc32xx
machine-$(CONFIG_ARCH_MEDIATEK)		+= mediatek
machine-$(CONFIG_ARCH_MESON)		+= meson
machine-$(CONFIG_ARCH_MVEBU)		+= mvebu
machine-$(CONFIG_ARCH_NEXELL)		+= nexell
machine-$(CONFIG_ARCH_NPCM)             += npcm
machine-$(CONFIG_ARCH_OMAP2PLUS)	+= omap2
machine-$(CONFIG_ARCH_ORION5X)		+= orion5x
machine-$(CONFIG_ARCH_OWL)		+= owl
machine-$(CONFIG_ARCH_RENESAS)		+= renesas
machine-$(CONFIG_ARCH_ROCKCHIP)		+= rockchip
machine-$(CONFIG_ARCH_S5PC1XX)		+= s5pc1xx
machine-$(CONFIG_ARCH_SNAPDRAGON)	+= snapdragon
machine-$(CONFIG_ARCH_SOCFPGA)		+= socfpga
machine-$(CONFIG_ARCH_STM32)		+= stm32
machine-$(CONFIG_ARCH_STM32MP)		+= stm32mp
machine-$(CONFIG_ARCH_SUNXI)		+= sunxi
machine-$(CONFIG_ARCH_TEGRA)		+= tegra
machine-$(CONFIG_ARCH_U8500)		+= u8500
machine-$(CONFIG_ARCH_OCTEONTX)		+= octeontx
machine-$(CONFIG_ARCH_OCTEONTX2)	+= octeontx2
machine-$(CONFIG_ARCH_UNIPHIER)		+= uniphier
machine-$(CONFIG_ARCH_VERSAL)		+= versal
machine-$(CONFIG_ARCH_VERSAL_NET)	+= versal-net
machine-$(CONFIG_ARCH_ZYNQ)		+= zynq
machine-$(CONFIG_ARCH_ZYNQMP)		+= zynqmp
machine-$(CONFIG_ARCH_ZYNQMP_R5)	+= zynqmp-r5

machine-$(CONFIG_MACH_IMX)		+= imx

machdirs := $(patsubst %,arch/arm/mach-%/,$(machine-y))

PLATFORM_CPPFLAGS += $(patsubst %,-I$(srctree)/%include,$(machdirs))

libs-y += $(machdirs)

head-y := arch/arm/cpu/$(CPU)/start.o

ifeq ($(CONFIG_SPL_BUILD),y)
ifeq ($(CONFIG_SYS_SOC)$(CONFIG_SPL_FRAMEWORK),"mxs")
head-y := arch/arm/cpu/arm926ejs/mxs/start.o
endif
endif

libs-y += arch/arm/cpu/$(CPU)/
libs-y += arch/arm/cpu/
libs-y += arch/arm/lib/

ifneq (,$(filter $(SOC), kirkwood))
libs-y += arch/arm/mach-mvebu/
endif

# deprecated
-include $(machdirs)/config.mk
