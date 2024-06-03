// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2012 ADVANSEE
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
 *
 * Based on the Linux rtc-imxdi.c driver, which is:
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright 2010 Orex Computed Radiography
 */

/*
 * Date & Time support for Freescale i.MX DryIce RTC
 */

#include <common.h>
#include <command.h>
#include <linux/compat.h>
#include <rtc.h>

#include <asm/io.h>
#include <asm/arch/imx-regs.h>

/* DryIce Register Definitions */

struct imxdi_regs {
	u32 dtcmr;			/* Time Counter MSB Reg */
	u32 dtclr;			/* Time Counter LSB Reg */
	u32 dcamr;			/* Clock Alarm MSB Reg */
	u32 dcalr;			/* Clock Alarm LSB Reg */
	u32 dcr;			/* Control Reg */
	u32 dsr;			/* Status Reg */
	u32 dier;			/* Interrupt Enable Reg */
};

#define DCAMR_UNSET	0xFFFFFFFF	/* doomsday - 1 sec */

#define DCR_TCE		(1 << 3)	/* Time Counter Enable */

#define DSR_WBF		(1 << 10)	/* Write Busy Flag */
#define DSR_WNF		(1 << 9)	/* Write Next Flag */
#define DSR_WCF		(1 << 8)	/* Write Complete Flag */
#define DSR_WEF		(1 << 7)	/* Write Error Flag */
#define DSR_CAF		(1 << 4)	/* Clock Alarm Flag */
#define DSR_NVF		(1 << 1)	/* Non-Valid Flag */
#define DSR_SVF		(1 << 0)	/* Security Violation Flag */

#define DIER_WNIE	(1 << 9)	/* Write Next Interrupt Enable */
#define DIER_WCIE	(1 << 8)	/* Write Complete Interrupt Enable */
#define DIER_WEIE	(1 << 7)	/* Write Error Interrupt Enable */
#define DIER_CAIE	(1 << 4)	/* Clock Alarm Interrupt Enable */

/* Driver Private Data */

struct imxdi_data {
	struct imxdi_regs __iomem	*regs;
	int				init_done;
};

static struct imxdi_data data;

/*
 * This function attempts to clear the dryice write-error flag.
 *
 * A dryice write error is similar to a bus fault and should not occur in
 * normal operation.  Clearing the flag requires another write, so the root
 * cause of the problem may need to be fixed before the flag can be cleared.
 */
static void clear_write_error(void)
{
	int cnt;

	puts("### Warning: RTC - Register write error!\n");

	/* clear the write error flag */
	__raw_writel(DSR_WEF, &data.regs->dsr);

	/* wait for it to take effect */
	for (cnt = 0; cnt < 1000; cnt++) {
		if ((__raw_readl(&data.regs->dsr) & DSR_WEF) == 0)
			return;
		udelay(10);
	}
	puts("### Error: RTC - Cannot clear write-error flag!\n");
}

/*
 * Write a dryice register and wait until it completes.
 *
 * Use interrupt flags to determine when the write has completed.
 */
#define DI_WRITE_WAIT(val, reg)						\
(									\
	/* do the register write */					\
	__raw_writel((val), &data.regs->reg),				\
									\
	di_write_wait((val), #reg)					\
)
static int di_write_wait(u32 val, const char *reg)
{
	int cnt;
	int ret = 0;
	int rc = 0;

	/* wait for the write to finish */
	for (cnt = 0; cnt < 100; cnt++) {
		if ((__raw_readl(&data.regs->dsr) & (DSR_WCF | DSR_WEF)) != 0) {
			ret = 1;
			break;
		}
		udelay(10);
	}
	if (ret == 0)
		printf("### Warning: RTC - Write-wait timeout "
				"val = 0x%.8x reg = %s\n", val, reg);

	/* check for write error */
	if (__raw_readl(&data.regs->dsr) & DSR_WEF) {
		clear_write_error();
		rc = -1;
	}

	return rc;
}

/*
 * Initialize dryice hardware
 */
static int di_init(void)
{
	int rc = 0;

	data.regs = (struct imxdi_regs __iomem *)IMX_DRYICE_BASE;

	/* mask all interrupts */
	__raw_writel(0, &data.regs->dier);

	/* put dryice into valid state */
	if (__raw_readl(&data.regs->dsr) & DSR_NVF) {
		rc = DI_WRITE_WAIT(DSR_NVF | DSR_SVF, dsr);
		if (rc)
			goto err;
	}

	/* initialize alarm */
	rc = DI_WRITE_WAIT(DCAMR_UNSET, dcamr);
	if (rc)
		goto err;
	rc = DI_WRITE_WAIT(0, dcalr);
	if (rc)
		goto err;

	/* clear alarm flag */
	if (__raw_readl(&data.regs->dsr) & DSR_CAF) {
		rc = DI_WRITE_WAIT(DSR_CAF, dsr);
		if (rc)
			goto err;
	}

	/* the timer won't count if it has never been written to */
	if (__raw_readl(&data.regs->dtcmr) == 0) {
		rc = DI_WRITE_WAIT(0, dtcmr);
		if (rc)
			goto err;
	}

	/* start keeping time */
	if (!(__raw_readl(&data.regs->dcr) & DCR_TCE)) {
		rc = DI_WRITE_WAIT(__raw_readl(&data.regs->dcr) | DCR_TCE, dcr);
		if (rc)
			goto err;
	}

	data.init_done = 1;
	return 0;

err:
	return rc;
}

int rtc_get(struct rtc_time *tmp)
{
	unsigned long now;
	int rc = 0;

	if (!data.init_done) {
		rc = di_init();
		if (rc)
			goto err;
	}

	now = __raw_readl(&data.regs->dtcmr);
	rtc_to_tm(now, tmp);

err:
	return rc;
}

int rtc_set(struct rtc_time *tmp)
{
	unsigned long now;
	int rc;

	if (!data.init_done) {
		rc = di_init();
		if (rc)
			goto err;
	}

	now = rtc_mktime(tmp);
	/* zero the fractional part first */
	rc = DI_WRITE_WAIT(0, dtclr);
	if (rc == 0)
		rc = DI_WRITE_WAIT(now, dtcmr);

err:
	return rc;
}

void rtc_reset(void)
{
	di_init();
}
