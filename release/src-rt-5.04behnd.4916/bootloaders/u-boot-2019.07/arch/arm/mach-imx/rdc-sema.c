// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/rdc-sema.h>
#include <asm/arch/imx-rdc.h>
#include <linux/errno.h>

/*
 * Check if the RDC Semaphore is required for this peripheral.
 */
static inline int imx_rdc_check_sema_required(int per_id)
{
	struct rdc_regs *imx_rdc = (struct rdc_regs *)RDC_BASE_ADDR;
	u32 reg;

	reg = readl(&imx_rdc->pdap[per_id]);
	/*
	 * No semaphore:
	 * Intial value or this peripheral is assigned to only one domain
	 */
	if (!(reg & RDC_PDAP_SREQ_MASK))
		return -ENOENT;

	return 0;
}

/*
 * Check the peripheral read / write access permission on Domain [dom_id].
 */
int imx_rdc_check_permission(int per_id, int dom_id)
{
	struct rdc_regs *imx_rdc = (struct rdc_regs *)RDC_BASE_ADDR;
	u32 reg;

	reg = readl(&imx_rdc->pdap[per_id]);
	if (!(reg & RDC_PDAP_DRW_MASK(dom_id)))
		return -EACCES;  /*No access*/

	return 0;
}

/*
 * Lock up the RDC semaphore for this peripheral if semaphore is required.
 */
int imx_rdc_sema_lock(int per_id)
{
	struct rdc_sema_regs *imx_rdc_sema;
	int ret;
	u8 reg;

	ret = imx_rdc_check_sema_required(per_id);
	if (ret)
		return ret;

	if (per_id < SEMA_GATES_NUM)
		imx_rdc_sema  = (struct rdc_sema_regs *)SEMAPHORE1_BASE_ADDR;
	else
		imx_rdc_sema  = (struct rdc_sema_regs *)SEMAPHORE2_BASE_ADDR;

	do {
		writeb(RDC_SEMA_PROC_ID,
		       &imx_rdc_sema->gate[per_id % SEMA_GATES_NUM]);
		reg = readb(&imx_rdc_sema->gate[per_id % SEMA_GATES_NUM]);
		if ((reg & RDC_SEMA_GATE_GTFSM_MASK) == RDC_SEMA_PROC_ID)
			break;  /* Get the Semaphore*/
	} while (1);

	return 0;
}

/*
 * Unlock the RDC semaphore for this peripheral if main CPU is the
 * semaphore owner.
 */
int imx_rdc_sema_unlock(int per_id)
{
	struct rdc_sema_regs *imx_rdc_sema;
	int ret;
	u8 reg;

	ret = imx_rdc_check_sema_required(per_id);
	if (ret)
		return ret;

	if (per_id < SEMA_GATES_NUM)
		imx_rdc_sema  = (struct rdc_sema_regs *)SEMAPHORE1_BASE_ADDR;
	else
		imx_rdc_sema  = (struct rdc_sema_regs *)SEMAPHORE2_BASE_ADDR;

	reg = readb(&imx_rdc_sema->gate[per_id % SEMA_GATES_NUM]);
	if ((reg & RDC_SEMA_GATE_GTFSM_MASK) != RDC_SEMA_PROC_ID)
		return -EACCES;	/*Not the semaphore owner */

	writeb(0x0, &imx_rdc_sema->gate[per_id % SEMA_GATES_NUM]);

	return 0;
}

/*
 * Setup RDC setting for one peripheral
 */
int imx_rdc_setup_peri(rdc_peri_cfg_t p)
{
	struct rdc_regs *imx_rdc = (struct rdc_regs *)RDC_BASE_ADDR;
	u32 reg = 0;
	u32 share_count = 0;
	u32 peri_id = p & RDC_PERI_MASK;
	u32 domain = (p & RDC_DOMAIN_MASK) >> RDC_DOMAIN_SHIFT_BASE;

	/* No domain assigned */
	if (domain == 0)
		return -EINVAL;

	reg |= domain;

	share_count = (domain & 0x3)
		+ ((domain >> 2) & 0x3)
		+ ((domain >> 4) & 0x3)
		+ ((domain >> 6) & 0x3);

	if (share_count > 0x3)
		reg |= RDC_PDAP_SREQ_MASK;

	writel(reg, &imx_rdc->pdap[peri_id]);

	return 0;
}

/*
 * Setup RDC settings for multiple peripherals
 */
int imx_rdc_setup_peripherals(rdc_peri_cfg_t const *peripherals_list,
				     unsigned count)
{
	rdc_peri_cfg_t const *p = peripherals_list;
	int i, ret;

	for (i = 0; i < count; i++) {
		ret = imx_rdc_setup_peri(*p);
		if (ret)
			return ret;
		p++;
	}

	return 0;
}

/*
 * Setup RDC setting for one master
 */
int imx_rdc_setup_ma(rdc_ma_cfg_t p)
{
	struct rdc_regs *imx_rdc = (struct rdc_regs *)RDC_BASE_ADDR;
	u32 master_id = (p & RDC_MASTER_MASK) >> RDC_MASTER_SHIFT;
	u32 domain = (p & RDC_DOMAIN_MASK) >> RDC_DOMAIN_SHIFT_BASE;

	writel((domain & RDC_MDA_DID_MASK), &imx_rdc->mda[master_id]);

	return 0;
}

/*
 * Setup RDC settings for multiple masters
 */
int imx_rdc_setup_masters(rdc_ma_cfg_t const *masters_list, unsigned count)
{
	rdc_ma_cfg_t const *p = masters_list;
	int i, ret;

	for (i = 0; i < count; i++) {
		ret = imx_rdc_setup_ma(*p);
		if (ret)
			return ret;
		p++;
	}

	return 0;
}
