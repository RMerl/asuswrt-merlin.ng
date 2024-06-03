// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 - 2017 Xilinx Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

#define SLCR_LOCK_MAGIC		0x767B
#define SLCR_UNLOCK_MAGIC	0xDF0D

#define SLCR_NAND_L2_SEL		0x10
#define SLCR_NAND_L2_SEL_MASK		0x1F

#define SLCR_USB_L1_SEL			0x04

#define SLCR_IDCODE_MASK	0x1F000
#define SLCR_IDCODE_SHIFT	12

/*
 * zynq_slcr_mio_get_status - Get the status of MIO peripheral.
 *
 * @peri_name: Name of the peripheral for checking MIO status
 * @get_pins: Pointer to array of get pin for this peripheral
 * @num_pins: Number of pins for this peripheral
 * @mask: Mask value
 * @check_val: Required check value to get the status of  periph
 */
struct zynq_slcr_mio_get_status {
	const char *peri_name;
	const int *get_pins;
	int num_pins;
	u32 mask;
	u32 check_val;
};

static const int nand8_pins[] = {
	0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

static const int nand16_pins[] = {
	16, 17, 18, 19, 20, 21, 22, 23
};

static const int usb0_pins[] = {
	28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39
};

static const int usb1_pins[] = {
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

static const struct zynq_slcr_mio_get_status mio_periphs[] = {
	{
		"nand8",
		nand8_pins,
		ARRAY_SIZE(nand8_pins),
		SLCR_NAND_L2_SEL_MASK,
		SLCR_NAND_L2_SEL,
	},
	{
		"nand16",
		nand16_pins,
		ARRAY_SIZE(nand16_pins),
		SLCR_NAND_L2_SEL_MASK,
		SLCR_NAND_L2_SEL,
	},
	{
		"usb0",
		usb0_pins,
		ARRAY_SIZE(usb0_pins),
		SLCR_USB_L1_SEL,
		SLCR_USB_L1_SEL,
	},
	{
		"usb1",
		usb1_pins,
		ARRAY_SIZE(usb1_pins),
		SLCR_USB_L1_SEL,
		SLCR_USB_L1_SEL,
	},
};

static int slcr_lock = 1; /* 1 means locked, 0 means unlocked */

void zynq_slcr_lock(void)
{
	if (!slcr_lock) {
		writel(SLCR_LOCK_MAGIC, &slcr_base->slcr_lock);
		slcr_lock = 1;
	}
}

void zynq_slcr_unlock(void)
{
	if (slcr_lock) {
		writel(SLCR_UNLOCK_MAGIC, &slcr_base->slcr_unlock);
		slcr_lock = 0;
	}
}

/* Reset the entire system */
void zynq_slcr_cpu_reset(void)
{
	/*
	 * Unlock the SLCR then reset the system.
	 * Note that this seems to require raw i/o
	 * functions or there's a lockup?
	 */
	zynq_slcr_unlock();

	/*
	 * Clear 0x0F000000 bits of reboot status register to workaround
	 * the FSBL not loading the bitstream after soft-reboot
	 * This is a temporary solution until we know more.
	 */
	clrbits_le32(&slcr_base->reboot_status, 0xF000000);

	writel(1, &slcr_base->pss_rst_ctrl);
}

void zynq_slcr_devcfg_disable(void)
{
	u32 reg_val;

	zynq_slcr_unlock();

	/* Disable AXI interface by asserting FPGA resets */
	writel(0xF, &slcr_base->fpga_rst_ctrl);

	/* Disable Level shifters before setting PS-PL */
	reg_val = readl(&slcr_base->lvl_shftr_en);
	reg_val &= ~0xF;
	writel(reg_val, &slcr_base->lvl_shftr_en);

	/* Set Level Shifters DT618760 */
	writel(0xA, &slcr_base->lvl_shftr_en);

	zynq_slcr_lock();
}

void zynq_slcr_devcfg_enable(void)
{
	zynq_slcr_unlock();

	/* Set Level Shifters DT618760 */
	writel(0xF, &slcr_base->lvl_shftr_en);

	/* Enable AXI interface by de-asserting FPGA resets */
	writel(0x0, &slcr_base->fpga_rst_ctrl);

	zynq_slcr_lock();
}

u32 zynq_slcr_get_boot_mode(void)
{
	/* Get the bootmode register value */
	return readl(&slcr_base->boot_mode);
}

u32 zynq_slcr_get_idcode(void)
{
	return (readl(&slcr_base->pss_idcode) & SLCR_IDCODE_MASK) >>
							SLCR_IDCODE_SHIFT;
}

/*
 * zynq_slcr_get_mio_pin_status - Get the MIO pin status of peripheral.
 *
 * @periph: Name of the peripheral
 *
 * Returns count to indicate the number of pins configured for the
 * given @periph.
 */
int zynq_slcr_get_mio_pin_status(const char *periph)
{
	const struct zynq_slcr_mio_get_status *mio_ptr;
	int val, j;
	int mio = 0;
	u32 i;

	for (i = 0; i < ARRAY_SIZE(mio_periphs); i++) {
		if (strcmp(periph, mio_periphs[i].peri_name) == 0) {
			mio_ptr = &mio_periphs[i];
			for (j = 0; j < mio_ptr->num_pins; j++) {
				val = readl(&slcr_base->mio_pin
						[mio_ptr->get_pins[j]]);
				if ((val & mio_ptr->mask) == mio_ptr->check_val)
					mio++;
			}
			break;
		}
	}

	return mio;
}
