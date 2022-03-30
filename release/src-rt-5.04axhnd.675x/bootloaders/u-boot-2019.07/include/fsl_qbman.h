/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 */

#ifndef __FSL_QBMAN_H__
#define __FSL_QBMAN_H__
void fdt_fixup_qportals(void *blob);
void fdt_fixup_bportals(void *blob);
void inhibit_portals(void __iomem *addr, int max_portals,
		     int arch_max_portals, int portal_cinh_size);
void setup_qbman_portals(void);

struct ccsr_qman {
#ifdef CONFIG_SYS_FSL_QMAN_V3
	u8	res0[0x200];
#else
	struct {
		u32	qcsp_lio_cfg;	/* 0x0 - SW Portal n LIO cfg */
		u32	qcsp_io_cfg;	/* 0x4 - SW Portal n IO cfg */
		u32	res;
		u32	qcsp_dd_cfg;	/* 0xc - SW Portal Dynamic Debug cfg */
	} qcsp[32];
#endif
	/* Not actually reserved, but irrelevant to u-boot */
	u8	res[0xbf8 - 0x200];
	u32	ip_rev_1;
	u32	ip_rev_2;
	u32	fqd_bare;	/* FQD Extended Base Addr Register */
	u32	fqd_bar;	/* FQD Base Addr Register */
	u8	res1[0x8];
	u32	fqd_ar;		/* FQD Attributes Register */
	u8	res2[0xc];
	u32	pfdr_bare;	/* PFDR Extended Base Addr Register */
	u32	pfdr_bar;	/* PFDR Base Addr Register */
	u8	res3[0x8];
	u32	pfdr_ar;	/* PFDR Attributes Register */
	u8	res4[0x4c];
	u32	qcsp_bare;	/* QCSP Extended Base Addr Register */
	u32	qcsp_bar;	/* QCSP Base Addr Register */
	u8	res5[0x78];
	u32	ci_sched_cfg;	/* Initiator Scheduling Configuration */
	u32	srcidr;		/* Source ID Register */
	u32	liodnr;		/* LIODN Register */
	u8	res6[4];
	u32	ci_rlm_cfg;	/* Initiator Read Latency Monitor Cfg */
	u32	ci_rlm_avg;	/* Initiator Read Latency Monitor Avg */
	u8	res7[0x2e8];
#ifdef CONFIG_SYS_FSL_QMAN_V3
	struct {
		u32	qcsp_lio_cfg;	/* 0x0 - SW Portal n LIO cfg */
		u32	qcsp_io_cfg;	/* 0x4 - SW Portal n IO cfg */
		u32	res;
		u32	qcsp_dd_cfg;	/* 0xc - SW Portal n Dynamic Debug cfg*/
	} qcsp[50];
#endif
};

struct ccsr_bman {
	/* Not actually reserved, but irrelevant to u-boot */
	u8	res[0xbf8];
	u32	ip_rev_1;
	u32	ip_rev_2;
	u32	fbpr_bare;	/* FBPR Extended Base Addr Register */
	u32	fbpr_bar;	/* FBPR Base Addr Register */
	u8	res1[0x8];
	u32	fbpr_ar;	/* FBPR Attributes Register */
	u8	res2[0xf0];
	u32	srcidr;		/* Source ID Register */
	u32	liodnr;		/* LIODN Register */
	u8	res7[0x2f4];
};

#endif /* __FSL_QBMAN_H__ */
