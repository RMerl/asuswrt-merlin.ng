// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 *
 * Modified from coreboot
 */

#include <common.h>
#include <errno.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/arch/pch.h>
#include <linux/delay.h>

#define IOBP_RETRY 1000

/* IO Buffer Programming */
#define IOBPIRI		0x2330
#define IOBPD		0x2334
#define IOBPS		0x2338
#define  IOBPS_READY	0x0001
#define  IOBPS_TX_MASK	0x0006
#define  IOBPS_MASK     0xff00
#define  IOBPS_READ     0x0600
#define  IOBPS_WRITE	0x0700
#define IOBPU		0x233a
#define  IOBPU_MAGIC	0xf000
#define  IOBP_PCICFG_READ	0x0400
#define  IOBP_PCICFG_WRITE	0x0500

static inline int iobp_poll(void)
{
	unsigned try;

	for (try = IOBP_RETRY; try > 0; try--) {
		u16 status = readw(RCB_REG(IOBPS));
		if ((status & IOBPS_READY) == 0)
			return 1;
		udelay(10);
	}

	printf("IOBP: timeout waiting for transaction to complete\n");
	return 0;
}

int pch_iobp_trans_start(u32 address, int op)
{
	if (!iobp_poll())
		return 0;

	/* Set the address */
	writel(address, RCB_REG(IOBPIRI));

	/* READ OPCODE */
	clrsetbits_le16(RCB_REG(IOBPS), IOBPS_MASK, op);

	return 1;
}

int pch_iobp_trans_finish(void)
{
	u16 status;

	/* Undocumented magic */
	writew(IOBPU_MAGIC, RCB_REG(IOBPU));

	/* Set ready bit */
	setbits_le16(RCB_REG(IOBPS), IOBPS_READY);

	if (!iobp_poll())
		return 1;

	/* Check for successful transaction */
	status = readw(RCB_REG(IOBPS));
	if (status & IOBPS_TX_MASK)
		return 1;

	return 0;
}

u32 pch_iobp_read(u32 address)
{
	if (!pch_iobp_trans_start(address, IOBPS_READ))
		return 0;
	if (pch_iobp_trans_finish()) {
		printf("IOBP: read 0x%08x failed\n", address);
		return 0;
	}

	/* Read IOBP data */
	return readl(RCB_REG(IOBPD));
}

int pch_iobp_write(u32 address, u32 data)
{
	if (!pch_iobp_trans_start(address, IOBPS_WRITE))
		return -EIO;

	writel(data, RCB_REG(IOBPD));

	if (pch_iobp_trans_finish()) {
		printf("IOBP: write 0x%08x failed\n", address);
		return -EIO;
	}

	return 0;
}

int pch_iobp_update(u32 address, u32 andvalue, u32 orvalue)
{
	u32 data = pch_iobp_read(address);

	/* Update the data */
	data &= andvalue;
	data |= orvalue;

	return pch_iobp_write(address, data);
}

int pch_iobp_exec(u32 addr, u16 op_code, u8 route_id, u32 *data, u8 *resp)
{
	if (!data || !resp)
		return 0;

	*resp = -1;
	if (!iobp_poll())
		return -EIO;

	writel(addr, RCB_REG(IOBPIRI));
	clrsetbits_le16(RCB_REG(IOBPS), 0xff00, op_code);
	writew(IOBPU_MAGIC | route_id, RCB_REG(IOBPU));

	writel(*data, RCB_REG(IOBPD));
	/* Set IOBPS[0] to trigger IOBP transaction*/
	setbits_le16(RCB_REG(IOBPS), 1);

	if (!iobp_poll())
		return -EIO;

	*resp = (readw(RCB_REG(IOBPS)) & IOBPS_TX_MASK) >> 1;
	*data = readl(RCB_REG(IOBPD));

	return 0;
}
