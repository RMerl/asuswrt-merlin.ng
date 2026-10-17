/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019 Google LLC
 */

#ifndef __ACPI_PMC_H
#define __ACPI_PMC_H

#ifndef __ASSEMBLY__

enum {
	GPE0_REG_MAX	= 4,
};

enum {
	PM1_STS		= 0x00,
	PM1_EN		= 0x02,
	PM1_CNT		= 0x04,
	PM1_TMR		= 0x08,

	GPE0_STS	= 0x20,
	GPE0_EN		= 0x30,
};

/**
 * struct acpi_pmc_upriv - holds common data for the x86 PMC
 *
 * @pmc_bar0: Base address 0 of PMC
 * @pmc_bar1: Base address 2 of PMC
 * @acpi_base: Base address of ACPI block
 * @pm1_sts: PM1 status
 * @pm1_en: PM1 enable
 * @pm1_cnt: PM1 control
 * @gpe_cfg: Address of GPE_CFG register
 * @gpe0_dwx_mask: Mask to use for each GPE0 (typically 7 or 0xf)
 * @gpe0_dwx_shift_base: Base shift value to use for GPE0 (0 or 4)
 * @gpe0_sts_req: GPE0 status register offset
 * @gpe0_en_req: GPE0 enable register offset
 * @gpe0_sts: GPE0 status values
 * @gpe0_en: GPE0 enable values
 * @gpe0_dw: GPE0 DW values
 * @gpe0_count: Number of GPE0 registers
 * @tco1_sts: TCO1 status
 * @tco2_sts: TCO2 status
 * @prsts: Power and reset status
 * @gen_pmcon1: General power mgmt configuration 1
 * @gen_pmcon2: General power mgmt configuration 2
 * @gen_pmcon3: General power mgmt configuration 3
 */
struct acpi_pmc_upriv {
	void *pmc_bar0;
	void *pmc_bar2;
	u32 acpi_base;
	u16 pm1_sts;
	u16 pm1_en;
	u32 pm1_cnt;
	u32 *gpe_cfg;
	u32 gpe0_dwx_mask;
	u32 gpe0_dwx_shift_base;
	u32 gpe0_sts_reg;
	u32 gpe0_en_reg;
	u32 gpe0_sts[GPE0_REG_MAX];
	u32 gpe0_en[GPE0_REG_MAX];
	u32 gpe0_dw[GPE0_REG_MAX];
	int gpe0_count;
	u16 tco1_sts;
	u16 tco2_sts;
	u32 prsts;
	u32 gen_pmcon1;
	u32 gen_pmcon2;
	u32 gen_pmcon3;
};

struct acpi_pmc_ops {
	/**
	 * init() - Set up the PMC for use
	 *
	 * This reads the current state of the PMC. Most of the state is read
	 * automatically by the uclass since it is common.
	 *
	 * This is optional.
	 *
	 * @dev: PMC device to use
	 * @return 0 if OK, -ve on error
	 */
	int (*init)(struct udevice *dev);

	/**
	 * prev_sleep_state() - Get the previous sleep state (optional)
	 *
	 * This reads various state registers and returns the sleep state from
	 * which the system woke. If this method is not provided, the uclass
	 * will return a calculated value.
	 *
	 * This is optional.
	 *
	 * @dev: PMC device to use
	 * @prev_sleep_state: Previous sleep state as calculated by the uclass.
	 *	The method can use this as the return value or calculate its
	 *	own.
	 *
	 * @return enum acpi_sleep_state indicating the previous sleep state
	 *	(ACPI_S0, ACPI_S3 or ACPI_S5), or -ve on error
	 */
	int (*prev_sleep_state)(struct udevice *dev, int prev_sleep_state);

	/**
	 * disable_tco() - Disable the timer/counter
	 *
	 * Disables the timer/counter in the PMC
	 *
	 * This is optional.
	 *
	 * @dev: PMC device to use
	 * @return 0
	 */
	int (*disable_tco)(struct udevice *dev);

	/**
	 * global_reset_set_enable() - Enable/Disable global reset
	 *
	 * Enable or disable global reset. If global reset is enabled, both hard
	 * reset and soft reset will trigger global reset, where both host and
	 * TXE are reset. This is cleared on cold boot, hard reset, soft reset
	 * and Sx.
	 *
	 * This is optional.
	 *
	 * @dev: PMC device to use
	 * @enable: true to enable global reset, false to disable
	 * @return 0
	 */
	int (*global_reset_set_enable)(struct udevice *dev, bool enable);
};

#define acpi_pmc_get_ops(dev)	((struct acpi_pmc_ops *)(dev)->driver->ops)

/**
 * init() - Set up the PMC for use
 *
 * This reads the current state of the PMC. This reads in the common registers,
 * then calls the device's init() method to read the SoC-specific registers.
 *
 * Return: 0 if OK, -ve on error
 */
int pmc_init(struct udevice *dev);

/**
 * pmc_prev_sleep_state() - Get the previous sleep state
 *
 * This reads various state registers and returns the sleep state from
 * which the system woke.
 *
 * Return: enum acpi_sleep_state indicating the previous sleep state
 *	(ACPI_S0, ACPI_S3 or ACPI_S5), or -ve on error
 */
int pmc_prev_sleep_state(struct udevice *dev);

/**
 * pmc_disable_tco() - Disable the timer/counter
 *
 * Disables the timer/counter in the PMC
 *
 * @dev: PMC device to use
 * Return: 0
 */
int pmc_disable_tco(struct udevice *dev);

/**
 * pmc_global_reset_set_enable() - Enable/Disable global reset
 *
 * Enable or disable global reset. If global reset is enabled, both hard
 * reset and soft reset will trigger global reset, where both host and
 * TXE are reset. This is cleared on cold boot, hard reset, soft reset
 * and Sx.
 *
 * @dev: PMC device to use
 * @enable: true to enable global reset, false to disable
 * Return: 0
 */
int pmc_global_reset_set_enable(struct udevice *dev, bool enable);

int pmc_ofdata_to_uc_plat(struct udevice *dev);

int pmc_disable_tco_base(ulong tco_base);

void pmc_dump_info(struct udevice *dev);

/**
 * pmc_gpe_init() - Set up general-purpose events
 *
 * @dev: PMC device
 * Return: 0 if OK, -ve on error
 */
int pmc_gpe_init(struct udevice *dev);

#endif /* !__ASSEMBLY__ */

#endif
