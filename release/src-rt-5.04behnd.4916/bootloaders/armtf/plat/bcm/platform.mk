include lib/libfdt/libfdt.mk
include drivers/arm/gic/v2/gicv2.mk

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

PLAT_BL_COMMON_SOURCES	:=	${GICV2_SOURCES} \
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
PLAT_BL_COMMON_SOURCES	= plat/bcm/brcm_pm.c

ifeq ($(strip $(BRCM_CHIP)), $(filter $(strip $(BRCM_CHIP)),68880 6837))
PLAT_BL_COMMON_SOURCES  +=	plat/bcm/drivers/console_8250.c
HW_ASSISTED_COHERENCY	:= 1
ERRATA_A55_1530923	:= 1
USE_COHERENT_MEM	:= 0
BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a55.S \
				lib/xlat_tables_v2/xlat_tables_context.c \
				lib/xlat_tables_v2/xlat_tables_core.c \
				lib/xlat_tables_v2/xlat_tables_utils.c \
				lib/xlat_tables_v2/aarch64/xlat_tables_arch.c \
				lib/xlat_tables_v2/aarch64/enable_mmu.S \
				plat/bcm/itc_rpc/ba_svc.c \
				plat/bcm/itc_rpc/dqm_lite.c \
				plat/bcm/itc_rpc/itc_msg_q.c \
				plat/bcm/itc_rpc/itc_rpc.c

PLAT_INCLUDES		+=	-Iplat/bcm/itc_rpc/include
include				lib/xlat_tables_v2/xlat_tables.mk
else
ERRATA_A53_819472	:= 1
ERRATA_A53_824069	:= 1
ERRATA_A53_826319	:= 1
ERRATA_A53_827319	:= 1
ERRATA_A53_835769	:= 1
ERRATA_A53_843419	:= 1
ERRATA_A53_1530924	:= 1
PLAT_BL_COMMON_SOURCES  +=	drivers/arm/pl011/aarch64/pl011_console.S
BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				plat/bcm/drivers/pmc_drv.c		\
				plat/bcm/drivers/pmc_cpu_core.c		\
				lib/xlat_tables/xlat_tables_common.c	\
				lib/xlat_tables/aarch64/xlat_tables.c
endif

BL31_SOURCES		+=	${GICV2_SOURCES} \
				plat/common/plat_psci_common.c		\
				plat/bcm/topology.c			\
				plat/bcm/aarch64/plat_helpers.S		\
				plat/bcm/brcm_bl31_setup.c		\
				plat/bcm/bcm_gic.c			\
				plat/arm/common/arm_common.c		\
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
$(eval $(call add_define,PLATFORM_FLAVOR_${BRCM_CHIP}))
$(eval $(call add_define,_BCM9${BRCM_CHIP}_))
ifeq ($(strip $(KERNEL_ARCH)),arm)
$(eval $(call add_define,AARCH32))
endif
