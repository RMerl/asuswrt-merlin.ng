// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Viresh Kumar, ST Microelectronics, viresh.kumar@st.com
 * Vipin Kumar, ST Microelectronics, vipin.kumar@st.com
 */

#include <common.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/arch/spr_misc.h>
#include <asm/arch/spr_defs.h>

void spear_late_init(void)
{
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;

	writel(0x80000007, &misc_p->arb_icm_ml1);
	writel(0x80000007, &misc_p->arb_icm_ml2);
	writel(0x80000007, &misc_p->arb_icm_ml3);
	writel(0x80000007, &misc_p->arb_icm_ml4);
	writel(0x80000007, &misc_p->arb_icm_ml5);
	writel(0x80000007, &misc_p->arb_icm_ml6);
	writel(0x80000007, &misc_p->arb_icm_ml7);
	writel(0x80000007, &misc_p->arb_icm_ml8);
	writel(0x80000007, &misc_p->arb_icm_ml9);
}

static void sel_1v8(void)
{
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	u32 ddr1v8, ddr2v5;

	ddr2v5 = readl(&misc_p->ddr_2v5_compensation);
	ddr2v5 &= 0x8080ffc0;
	ddr2v5 |= 0x78000003;
	writel(ddr2v5, &misc_p->ddr_2v5_compensation);

	ddr1v8 = readl(&misc_p->ddr_1v8_compensation);
	ddr1v8 &= 0x8080ffc0;
	ddr1v8 |= 0x78000010;
	writel(ddr1v8, &misc_p->ddr_1v8_compensation);

	while (!(readl(&misc_p->ddr_1v8_compensation) & DDR_COMP_ACCURATE))
		;
}

static void sel_2v5(void)
{
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	u32 ddr1v8, ddr2v5;

	ddr1v8 = readl(&misc_p->ddr_1v8_compensation);
	ddr1v8 &= 0x8080ffc0;
	ddr1v8 |= 0x78000003;
	writel(ddr1v8, &misc_p->ddr_1v8_compensation);

	ddr2v5 = readl(&misc_p->ddr_2v5_compensation);
	ddr2v5 &= 0x8080ffc0;
	ddr2v5 |= 0x78000010;
	writel(ddr2v5, &misc_p->ddr_2v5_compensation);

	while (!(readl(&misc_p->ddr_2v5_compensation) & DDR_COMP_ACCURATE))
		;
}

/*
 * plat_ddr_init:
 */
void plat_ddr_init(void)
{
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	u32 ddrpad;
	u32 core3v3, ddr1v8, ddr2v5;

	/* DDR pad register configurations */
	ddrpad = readl(&misc_p->ddr_pad);
	ddrpad &= ~DDR_PAD_CNF_MSK;

#if (CONFIG_DDR_HCLK)
	ddrpad |= 0xEAAB;
#elif (CONFIG_DDR_2HCLK)
	ddrpad |= 0xEAAD;
#elif (CONFIG_DDR_PLL2)
	ddrpad |= 0xEAAD;
#endif
	writel(ddrpad, &misc_p->ddr_pad);

	/* Compensation register configurations */
	core3v3 = readl(&misc_p->core_3v3_compensation);
	core3v3 &= 0x8080ffe0;
	core3v3 |= 0x78000002;
	writel(core3v3, &misc_p->core_3v3_compensation);

	ddr1v8 = readl(&misc_p->ddr_1v8_compensation);
	ddr1v8 &= 0x8080ffc0;
	ddr1v8 |= 0x78000004;
	writel(ddr1v8, &misc_p->ddr_1v8_compensation);

	ddr2v5 = readl(&misc_p->ddr_2v5_compensation);
	ddr2v5 &= 0x8080ffc0;
	ddr2v5 |= 0x78000004;
	writel(ddr2v5, &misc_p->ddr_2v5_compensation);

	if ((readl(&misc_p->ddr_pad) & DDR_PAD_SW_CONF) == DDR_PAD_SW_CONF) {
		/* Software memory configuration */
		if (readl(&misc_p->ddr_pad) & DDR_PAD_SSTL_SEL)
			sel_1v8();
		else
			sel_2v5();
	} else {
		/* Hardware memory configuration */
		if (readl(&misc_p->ddr_pad) & DDR_PAD_DRAM_TYPE)
			sel_1v8();
		else
			sel_2v5();
	}
}

/*
 * xxx_boot_selected:
 *
 * return true if the particular booting option is selected
 * return false otherwise
 */
static u32 read_bootstrap(void)
{
	return (readl(CONFIG_SPEAR_BOOTSTRAPCFG) >> CONFIG_SPEAR_BOOTSTRAPSHFT)
		& CONFIG_SPEAR_BOOTSTRAPMASK;
}

int snor_boot_selected(void)
{
	u32 bootstrap = read_bootstrap();

	if (SNOR_BOOT_SUPPORTED) {
		/* Check whether SNOR boot is selected */
		if ((bootstrap & CONFIG_SPEAR_ONLYSNORBOOT) ==
			CONFIG_SPEAR_ONLYSNORBOOT)
			return true;

		if ((bootstrap & CONFIG_SPEAR_NORNANDBOOT) ==
			CONFIG_SPEAR_NORNAND8BOOT)
			return true;

		if ((bootstrap & CONFIG_SPEAR_NORNANDBOOT) ==
			CONFIG_SPEAR_NORNAND16BOOT)
			return true;
	}

	return false;
}

int nand_boot_selected(void)
{
	u32 bootstrap = read_bootstrap();

	if (NAND_BOOT_SUPPORTED) {
		/* Check whether NAND boot is selected */
		if ((bootstrap & CONFIG_SPEAR_NORNANDBOOT) ==
			CONFIG_SPEAR_NORNAND8BOOT)
			return true;

		if ((bootstrap & CONFIG_SPEAR_NORNANDBOOT) ==
			CONFIG_SPEAR_NORNAND16BOOT)
			return true;
	}

	return false;
}

int pnor_boot_selected(void)
{
	/* Parallel NOR boot is not selected in any SPEAr600 revision */
	return false;
}

int usb_boot_selected(void)
{
	u32 bootstrap = read_bootstrap();

	if (USB_BOOT_SUPPORTED) {
		/* Check whether USB boot is selected */
		if (!(bootstrap & CONFIG_SPEAR_USBBOOT))
			return true;
	}

	return false;
}

int tftp_boot_selected(void)
{
	/* TFTP boot is not selected in any SPEAr600 revision */
	return false;
}

int uart_boot_selected(void)
{
	/* UART boot is not selected in any SPEAr600 revision */
	return false;
}

int spi_boot_selected(void)
{
	/* SPI boot is not selected in any SPEAr600 revision */
	return false;
}

int i2c_boot_selected(void)
{
	/* I2C boot is not selected in any SPEAr600 revision */
	return false;
}

int mmc_boot_selected(void)
{
	return false;
}

void plat_late_init(void)
{
	spear_late_init();
}
