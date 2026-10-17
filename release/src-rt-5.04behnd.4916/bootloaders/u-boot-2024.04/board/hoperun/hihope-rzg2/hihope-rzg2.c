// SPDX-License-Identifier: GPL-2.0+
/*
 * board/hoperun/hihope-rzg2/hihope-rzg2.c
 *     This file is HiHope RZ/G2[HMN] board support.
 *
 * Copyright (C) 2021 Renesas Electronics Corporation
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/arch/renesas.h>
#include <asm/arch/rcar-mstp.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/libfdt.h>

#define RST_BASE	0xE6160000
#define RST_CA57RESCNT	(RST_BASE + 0x40)
#define RST_CA53RESCNT	(RST_BASE + 0x44)
#define RST_CA57_CODE	0xA5A5000F
#define RST_CA53_CODE	0x5A5A000F

DECLARE_GLOBAL_DATA_PTR;
#define HSUSB_MSTP704		BIT(4)	/* HSUSB */

/* HSUSB block registers */
#define HSUSB_REG_LPSTS			0xE6590102
#define HSUSB_REG_LPSTS_SUSPM_NORMAL	BIT(14)
#define HSUSB_REG_UGCTRL2		0xE6590184
#define HSUSB_REG_UGCTRL2_USB0SEL_EHCI	0x10
#define HSUSB_REG_UGCTRL2_RESERVED_3	0x1 /* bit[3:0] should be B'0001 */

#define PRR_REGISTER (0xFFF00044)

int board_init(void)
{
	u32 i;

	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;

	/* Configure the HSUSB block */
	mstp_clrbits_le32(SMSTPCR7, SMSTPCR7, HSUSB_MSTP704);
	/*
	 * We need to add a barrier instruction after HSUSB module stop release.
	 * This barrier instruction can be either reading back the same MSTP
	 * register or any other register in the same IP block. So like linux
	 * adding check for MSTPSR register, which indicates the clock has been
	 * started.
	 */
	for (i = 1000; i > 0; --i) {
		if (!(readl(MSTPSR7) & HSUSB_MSTP704))
			break;
		cpu_relax();
	}

	/* Select EHCI/OHCI host module for USB2.0 ch0 */
	writel(HSUSB_REG_UGCTRL2_USB0SEL_EHCI | HSUSB_REG_UGCTRL2_RESERVED_3,
	       HSUSB_REG_UGCTRL2);
	/* low power status */
	setbits_le16(HSUSB_REG_LPSTS, HSUSB_REG_LPSTS_SUSPM_NORMAL);

	return 0;
}

void reset_cpu(void)
{
	unsigned long midr, cputype;

	asm volatile("mrs %0, midr_el1" : "=r" (midr));
	cputype = (midr >> 4) & 0xfff;

	if (cputype == 0xd03)
		writel(RST_CA53_CODE, RST_CA53RESCNT);
	else
		writel(RST_CA57_CODE, RST_CA57RESCNT);
}

#if defined(CONFIG_MULTI_DTB_FIT)
/* If the firmware passed a device tree, use it for board identification. */
extern u64 rcar_atf_boot_args[];

static bool is_hoperun_hihope_rzg2_board(const char *board_name)
{
	void *atf_fdt_blob = (void *)(rcar_atf_boot_args[1]);
	bool ret = false;

	if ((fdt_magic(atf_fdt_blob) == FDT_MAGIC) &&
	    (fdt_node_check_compatible(atf_fdt_blob, 0, board_name) == 0))
		ret = true;

	return ret;
}

int board_fit_config_name_match(const char *name)
{
	if (is_hoperun_hihope_rzg2_board("hoperun,hihope-rzg2m") &&
	    !strcmp(name, "r8a774a1-hihope-rzg2m-u-boot"))
		return 0;

	if (is_hoperun_hihope_rzg2_board("hoperun,hihope-rzg2n") &&
	    !strcmp(name, "r8a774b1-hihope-rzg2n-u-boot"))
		return 0;

	if (is_hoperun_hihope_rzg2_board("hoperun,hihope-rzg2h") &&
	    !strcmp(name, "r8a774e1-hihope-rzg2h-u-boot"))
		return 0;

	return -1;
}
#endif
