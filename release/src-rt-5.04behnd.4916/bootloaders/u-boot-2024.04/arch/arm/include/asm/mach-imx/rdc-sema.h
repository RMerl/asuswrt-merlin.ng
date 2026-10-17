/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#ifndef __RDC_SEMA_H__
#define __RDC_SEMA_H__

/*
 * rdc_peri_cfg_t and rdc_ma_cft_t use the same layout.
 *
 *   [ 23 22 | 21 20 | 19 18 | 17 16 ] | [ 15 - 8 ] | [ 7 - 0 ]
 *      d3      d2      d1       d0    | master id  |  peri id
 *   d[x] means domain[x], x can be [3 - 0].
 */
typedef u32 rdc_peri_cfg_t;
typedef u32 rdc_ma_cfg_t;

#define RDC_PERI_SHIFT		0
#define RDC_PERI_MASK		0xFF

#define RDC_DOMAIN_SHIFT_BASE	16
#define RDC_DOMAIN_MASK		0xFF0000
#define RDC_DOMAIN_SHIFT(x)	(RDC_DOMAIN_SHIFT_BASE + ((x << 1)))
#define RDC_DOMAIN(x)		((rdc_peri_cfg_t)(0x3 << RDC_DOMAIN_SHIFT(x)))

#define RDC_MASTER_SHIFT	8
#define RDC_MASTER_MASK		0xFF00
#define RDC_MASTER_CFG(master_id, domain_id) (rdc_ma_cfg_t)((master_id << 8) | \
					(domain_id << RDC_DOMAIN_SHIFT_BASE))

/* The Following macro definitions are common to i.MX6SX and i.MX7D */
#define SEMA_GATES_NUM		64

#define RDC_MDA_DID_SHIFT	0
#define RDC_MDA_DID_MASK	(0x3 << RDC_MDA_DID_SHIFT)
#define RDC_MDA_LCK_SHIFT	31
#define RDC_MDA_LCK_MASK	(0x1 << RDC_MDA_LCK_SHIFT)

#define RDC_PDAP_DW_SHIFT(domain)	((domain) << 1)
#define RDC_PDAP_DR_SHIFT(domain)	(1 + RDC_PDAP_DW_SHIFT(domain))
#define RDC_PDAP_DW_MASK(domain)	(1 << RDC_PDAP_DW_SHIFT(domain))
#define RDC_PDAP_DR_MASK(domain)	(1 << RDC_PDAP_DR_SHIFT(domain))
#define RDC_PDAP_DRW_MASK(domain)	(RDC_PDAP_DW_MASK(domain) | \
					 RDC_PDAP_DR_MASK(domain))

#define RDC_PDAP_SREQ_SHIFT	30
#define RDC_PDAP_SREQ_MASK	(0x1 << RDC_PDAP_SREQ_SHIFT)
#define RDC_PDAP_LCK_SHIFT	31
#define RDC_PDAP_LCK_MASK	(0x1 << RDC_PDAP_LCK_SHIFT)

#define RDC_MRSA_SADR_SHIFT	7
#define RDC_MRSA_SADR_MASK	(0x1ffffff << RDC_MRSA_SADR_SHIFT)

#define RDC_MREA_EADR_SHIFT	7
#define RDC_MREA_EADR_MASK	(0x1ffffff << RDC_MREA_EADR_SHIFT)

#define RDC_MRC_DW_SHIFT(domain)	(domain)
#define RDC_MRC_DR_SHIFT(domain)	(1 + RDC_MRC_DW_SHIFT(domain))
#define RDC_MRC_DW_MASK(domain)		(1 << RDC_MRC_DW_SHIFT(domain))
#define RDC_MRC_DR_MASK(domain)		(1 << RDC_MRC_DR_SHIFT(domain))
#define RDC_MRC_DRW_MASK(domain)	(RDC_MRC_DW_MASK(domain) | \
					 RDC_MRC_DR_MASK(domain))
#define RDC_MRC_ENA_SHIFT	30
#define RDC_MRC_ENA_MASK	(0x1 << RDC_MRC_ENA_SHIFT)
#define RDC_MRC_LCK_SHIFT	31
#define RDC_MRC_LCK_MASK	(0x1 << RDC_MRC_LCK_SHIFT)

#define RDC_MRVS_VDID_SHIFT	0
#define RDC_MRVS_VDID_MASK	(0x3 << RDC_MRVS_VDID_SHIFT)
#define RDC_MRVS_AD_SHIFT	4
#define RDC_MRVS_AD_MASK	(0x1 << RDC_MRVS_AD_SHIFT)
#define RDC_MRVS_VADDR_SHIFT	5
#define RDC_MRVS_VADDR_MASK	(0x7ffffff << RDC_MRVS_VADDR_SHIFT)

#define RDC_SEMA_GATE_GTFSM_SHIFT	0
#define RDC_SEMA_GATE_GTFSM_MASK	(0xf << RDC_SEMA_GATE_GTFSM_SHIFT)
#define RDC_SEMA_GATE_LDOM_SHIFT	5
#define RDC_SEMA_GATE_LDOM_MASK		(0x3 << RDC_SEMA_GATE_LDOM_SHIFT)

#define RDC_SEMA_RSTGT_RSTGDP_SHIFT	0
#define RDC_SEMA_RSTGT_RSTGDP_MASK	(0xff << RDC_SEMA_RSTGT_RSTGDP_SHIFT)
#define RDC_SEMA_RSTGT_RSTGSM_SHIFT	2
#define RDC_SEMA_RSTGT_RSTGSM_MASK	(0x3 << RDC_SEMA_RSTGT_RSTGSM_SHIFT)
#define RDC_SEMA_RSTGT_RSTGMS_SHIFT	4
#define RDC_SEMA_RSTGT_RSTGMS_MASK	(0xf << RDC_SEMA_RSTGT_RSTGMS_SHIFT)
#define RDC_SEMA_RSTGT_RSTGTN_SHIFT	8
#define RDC_SEMA_RSTGT_RSTGTN_MASK	(0xff << RDC_SEMA_RSTGT_RSTGTN_SHIFT)

int imx_rdc_check_permission(int per_id, int dom_id);
int imx_rdc_sema_lock(int per_id);
int imx_rdc_sema_unlock(int per_id);
int imx_rdc_setup_peri(rdc_peri_cfg_t p);
int imx_rdc_setup_peripherals(rdc_peri_cfg_t const *peripherals_list,
			      unsigned count);
int imx_rdc_setup_ma(rdc_ma_cfg_t p);
int imx_rdc_setup_masters(rdc_ma_cfg_t const *masters_list, unsigned count);

#endif	/* __RDC_SEMA_H__*/
