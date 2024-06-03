// SPDX-License-Identifier: GPL-2.0+
/*
 * board/renesas/salvator-x/salvator-x.c
 *     This file is Salvator-X/Salvator-XS board support.
 *
 * Copyright (C) 2015-2017 Renesas Electronics Corporation
 * Copyright (C) 2015 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <common.h>
#include <malloc.h>
#include <netdev.h>
#include <dm.h>
#include <dm/platform_data/serial_sh.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

void s_init(void)
{
}

#define DVFS_MSTP926		BIT(26)
#define HSUSB_MSTP704		BIT(4)	/* HSUSB */

int board_early_init_f(void)
{
#if defined(CONFIG_SYS_I2C) && defined(CONFIG_SYS_I2C_SH)
	/* DVFS for reset */
	mstp_clrbits_le32(SMSTPCR9, SMSTPCR9, DVFS_MSTP926);
#endif
	return 0;
}

/* HSUSB block registers */
#define HSUSB_REG_LPSTS			0xE6590102
#define HSUSB_REG_LPSTS_SUSPM_NORMAL	BIT(14)
#define HSUSB_REG_UGCTRL2		0xE6590184
#define HSUSB_REG_UGCTRL2_USB0SEL	0x30
#define HSUSB_REG_UGCTRL2_USB0SEL_EHCI	0x10

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	/* USB1 pull-up */
	setbits_le32(PFC_PUEN6, PUEN_USB1_OVC | PUEN_USB1_PWEN);

	/* Configure the HSUSB block */
	mstp_clrbits_le32(SMSTPCR7, SMSTPCR7, HSUSB_MSTP704);
	/* Choice USB0SEL */
	clrsetbits_le32(HSUSB_REG_UGCTRL2, HSUSB_REG_UGCTRL2_USB0SEL,
			HSUSB_REG_UGCTRL2_USB0SEL_EHCI);
	/* low power status */
	setbits_le16(HSUSB_REG_LPSTS, HSUSB_REG_LPSTS_SUSPM_NORMAL);

	return 0;
}

#define RST_BASE	0xE6160000
#define RST_CA57RESCNT	(RST_BASE + 0x40)
#define RST_CA53RESCNT	(RST_BASE + 0x44)
#define RST_RSTOUTCR	(RST_BASE + 0x58)
#define RST_CODE	0xA5A5000F

void reset_cpu(ulong addr)
{
#if defined(CONFIG_SYS_I2C) && defined(CONFIG_SYS_I2C_SH)
	i2c_reg_write(CONFIG_SYS_I2C_POWERIC_ADDR, 0x20, 0x80);
#else
	/* only CA57 ? */
	writel(RST_CODE, RST_CA57RESCNT);
#endif
}

#ifdef CONFIG_MULTI_DTB_FIT
int board_fit_config_name_match(const char *name)
{
	/* PRR driver is not available yet */
	u32 cpu_type = rmobile_get_cpu_type();

	if ((cpu_type == RMOBILE_CPU_TYPE_R8A7795) &&
	    !strcmp(name, "r8a7795-salvator-x-u-boot"))
		return 0;

	if ((cpu_type == RMOBILE_CPU_TYPE_R8A7796) &&
	    !strcmp(name, "r8a7796-salvator-x-u-boot"))
		return 0;

	if ((cpu_type == RMOBILE_CPU_TYPE_R8A77965) &&
	    !strcmp(name, "r8a77965-salvator-x-u-boot"))
		return 0;

	return -1;
}
#endif
