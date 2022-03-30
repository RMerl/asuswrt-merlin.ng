include lib/libfdt/libfdt.mk

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/		\
				-Iinclude/plat/common/			\
				-Iinclude/plat/arm/common/aarch64/	\
				-Iplat/bcm/include			\
				-Iplat/bcm/include/bcm963xx		\
				-Iplat/bcm/drivers			\
				-Iinclude/drivers/			\
				-Iinclude/drivers/arm/			\
				-Iinclude/common/			\
				-Iinclude/common/tbbr			\
				-Iinclude/lib/				\
				-Iinclude/lib/psci/			\
				-Iinclude/bl31/


ifeq ($(strip $(KERNEL_ARCH)),arm)
PLAT_BL_A9_SOURCE :=		plat/bcm/aarch32/pl310_a32.S	\
				plat/bcm/aarch32/cortexa9_mmu.S

PLAT_BL_COMMON_SOURCES	:= drivers/arm/gic/v2/gicv2_helpers.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/common/gic_common.c	\
				drivers/arm/pl011/aarch32/pl011_console.S	\
				drivers/delay_timer/delay_timer.c	\
				drivers/delay_timer/generic_delay_timer.c \
				lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch32/xlat_tables.c		\
				plat/common/plat_psci_common.c	\
				plat/common/aarch32/platform_mp_stack.S		\
				plat/bcm/aarch32/plat_helpers.S	\
				plat/arm/common/aarch32/arm_helpers.S		\
				plat/arm/common/arm_common.c	\
				plat/bcm/brcm_bl32_setup.c		\
				plat/bcm/brcm_pm.c	\
				plat/bcm/topology.c			\
				plat/bcm/drivers/pmc_drv.c	\
				plat/bcm/drivers/pmc_cpu_core.c	\
				plat/bcm/sp_min/src/opteed_main.c	\
				plat/bcm/aarch32/opteed_helper.S

ifeq ($(strip $(BRCM_CHIP)),63138)
PLAT_BL_COMMON_SOURCES += $(PLAT_BL_A9_SOURCE)
# following is needed to compile out code that A9 does not support
PL011_GENERIC_UART := 1
endif

else
PLAT_BL_COMMON_SOURCES	:= drivers/arm/pl011/aarch64/pl011_console.S \
				lib/xlat_tables/xlat_tables_common.c	\
				lib/xlat_tables/aarch64/xlat_tables.c

BL31_SOURCES		+=	\
				lib/cpus/aarch64/cortex_a53.S		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/common/gic_common.c	\
				plat/common/plat_psci_common.c	\
				plat/bcm/brcm_pm.c	\
				plat/bcm/topology.c			\
				plat/bcm/aarch64/plat_helpers.S	\
				plat/bcm/brcm_bl31_setup.c		\
				plat/bcm/bcm_gic.c \
				plat/bcm/drivers/pmc_drv.c \
				plat/bcm/drivers/pmc_cpu_core.c	\
				plat/arm/common/arm_common.c \
				drivers/delay_timer/delay_timer.c	\
				drivers/delay_timer/generic_delay_timer.c
endif

override WARMBOOT_ENABLE_DCACHE_EARLY	:=	1
override PROGRAMMABLE_RESET_ADDRESS	:=	1

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT	:= 	0

BL32_RAM_LOCATION	:=	tdram
ifeq (${BL32_RAM_LOCATION}, tsram)
  BL32_RAM_LOCATION_ID = SEC_SRAM_ID
else ifeq (${BL32_RAM_LOCATION}, tdram)
  BL32_RAM_LOCATION_ID = SEC_DRAM_ID
else
  $(error "Unsupported BL32_RAM_LOCATION value")
endif

# Process flags
$(eval $(call add_define,BL32_RAM_LOCATION_ID))
