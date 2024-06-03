/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010-2015
 * NVIDIA Corporation <www.nvidia.com>
 */
#include <asm/types.h>

/* Stabilization delays, in usec */
#define PLL_STABILIZATION_DELAY	(300)
#define IO_STABILIZATION_DELAY	(1000)

#define PLLX_ENABLED		(1 << 30)
#define CCLK_BURST_POLICY	0x20008888
#define SUPER_CCLK_DIVIDER	0x80000000

/* Calculate clock fractional divider value from ref and target frequencies */
#define CLK_DIVIDER(REF, FREQ)	((((REF) * 2) / FREQ) - 2)

/* Calculate clock frequency value from reference and clock divider value */
#define CLK_FREQUENCY(REF, REG)	(((REF) * 2) / (REG + 2))

/* AVP/CPU ID */
#define PG_UP_TAG_0_PID_CPU	0x55555555	/* CPU aka "a9" aka "mpcore" */
#define PG_UP_TAG_0		0x0

/* AP base physical address of internal SRAM */
#define NV_PA_BASE_SRAM		0x40000000

#define EXCEP_VECTOR_CPU_RESET_VECTOR	(NV_PA_EVP_BASE + 0x100)
#define CSITE_CPU_DBG0_LAR		(NV_PA_CSITE_BASE + 0x10FB0)
#define CSITE_CPU_DBG1_LAR		(NV_PA_CSITE_BASE + 0x12FB0)

#define FLOW_CTLR_HALT_COP_EVENTS	(NV_PA_FLOW_BASE + 4)
#define FLOW_MODE_STOP			2
#define HALT_COP_EVENT_JTAG		(1 << 28)
#define HALT_COP_EVENT_IRQ_1		(1 << 11)
#define HALT_COP_EVENT_FIQ_1		(1 << 9)

/* This is the main entry into U-Boot, used by the Cortex-A9 */
extern void _start(void);

/**
 * Works out the SOC/SKU type used for clocks settings
 *
 * @return	SOC type - see TEGRA_SOC...
 */
int tegra_get_chip_sku(void);

/**
 * Returns the pure SOC (chip ID) from the HIDREV register
 *
 * @return	SOC ID - see CHIPID_TEGRAxx...
 */
int tegra_get_chip(void);

/**
 * Returns the SKU ID from the sku_info register
 *
 * @return	SKU ID - see SKU_ID_Txx...
 */
int tegra_get_sku_info(void);

/* Do any chip-specific cache config */
void config_cache(void);

#if defined(CONFIG_TEGRA_SUPPORT_NON_SECURE)
bool tegra_cpu_is_non_secure(void);
#endif
