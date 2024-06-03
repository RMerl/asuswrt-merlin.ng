// SPDX-License-Identifier: GPL-2.0
/*
 * board/renesas/koelsch/qos.c
 *
 * Copyright (C) 2013,2014 Renesas Electronics Corporation
 *
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/arch/rmobile.h>

/* QoS version 0.240 for ES1 and version 0.411 for ES2 */
#if defined(CONFIG_ARCH_RMOBILE_EXTRAM_BOOT)
enum {
	DBSC3_00, DBSC3_01, DBSC3_02, DBSC3_03, DBSC3_04,
	DBSC3_05, DBSC3_06, DBSC3_07, DBSC3_08, DBSC3_09,
	DBSC3_10, DBSC3_11, DBSC3_12, DBSC3_13, DBSC3_14,
	DBSC3_15,
	DBSC3_NR,
};

static u32 dbsc3_0_r_qos_addr[DBSC3_NR] = {
	[DBSC3_00] = DBSC3_0_QOS_R0_BASE,
	[DBSC3_01] = DBSC3_0_QOS_R1_BASE,
	[DBSC3_02] = DBSC3_0_QOS_R2_BASE,
	[DBSC3_03] = DBSC3_0_QOS_R3_BASE,
	[DBSC3_04] = DBSC3_0_QOS_R4_BASE,
	[DBSC3_05] = DBSC3_0_QOS_R5_BASE,
	[DBSC3_06] = DBSC3_0_QOS_R6_BASE,
	[DBSC3_07] = DBSC3_0_QOS_R7_BASE,
	[DBSC3_08] = DBSC3_0_QOS_R8_BASE,
	[DBSC3_09] = DBSC3_0_QOS_R9_BASE,
	[DBSC3_10] = DBSC3_0_QOS_R10_BASE,
	[DBSC3_11] = DBSC3_0_QOS_R11_BASE,
	[DBSC3_12] = DBSC3_0_QOS_R12_BASE,
	[DBSC3_13] = DBSC3_0_QOS_R13_BASE,
	[DBSC3_14] = DBSC3_0_QOS_R14_BASE,
	[DBSC3_15] = DBSC3_0_QOS_R15_BASE,
};

static u32 dbsc3_0_w_qos_addr[DBSC3_NR] = {
	[DBSC3_00] = DBSC3_0_QOS_W0_BASE,
	[DBSC3_01] = DBSC3_0_QOS_W1_BASE,
	[DBSC3_02] = DBSC3_0_QOS_W2_BASE,
	[DBSC3_03] = DBSC3_0_QOS_W3_BASE,
	[DBSC3_04] = DBSC3_0_QOS_W4_BASE,
	[DBSC3_05] = DBSC3_0_QOS_W5_BASE,
	[DBSC3_06] = DBSC3_0_QOS_W6_BASE,
	[DBSC3_07] = DBSC3_0_QOS_W7_BASE,
	[DBSC3_08] = DBSC3_0_QOS_W8_BASE,
	[DBSC3_09] = DBSC3_0_QOS_W9_BASE,
	[DBSC3_10] = DBSC3_0_QOS_W10_BASE,
	[DBSC3_11] = DBSC3_0_QOS_W11_BASE,
	[DBSC3_12] = DBSC3_0_QOS_W12_BASE,
	[DBSC3_13] = DBSC3_0_QOS_W13_BASE,
	[DBSC3_14] = DBSC3_0_QOS_W14_BASE,
	[DBSC3_15] = DBSC3_0_QOS_W15_BASE,
};

static u32 dbsc3_1_r_qos_addr[DBSC3_NR] = {
	[DBSC3_00] = DBSC3_1_QOS_R0_BASE,
	[DBSC3_01] = DBSC3_1_QOS_R1_BASE,
	[DBSC3_02] = DBSC3_1_QOS_R2_BASE,
	[DBSC3_03] = DBSC3_1_QOS_R3_BASE,
	[DBSC3_04] = DBSC3_1_QOS_R4_BASE,
	[DBSC3_05] = DBSC3_1_QOS_R5_BASE,
	[DBSC3_06] = DBSC3_1_QOS_R6_BASE,
	[DBSC3_07] = DBSC3_1_QOS_R7_BASE,
	[DBSC3_08] = DBSC3_1_QOS_R8_BASE,
	[DBSC3_09] = DBSC3_1_QOS_R9_BASE,
	[DBSC3_10] = DBSC3_1_QOS_R10_BASE,
	[DBSC3_11] = DBSC3_1_QOS_R11_BASE,
	[DBSC3_12] = DBSC3_1_QOS_R12_BASE,
	[DBSC3_13] = DBSC3_1_QOS_R13_BASE,
	[DBSC3_14] = DBSC3_1_QOS_R14_BASE,
	[DBSC3_15] = DBSC3_1_QOS_R15_BASE,
};

static u32 dbsc3_1_w_qos_addr[DBSC3_NR] = {
	[DBSC3_00] = DBSC3_1_QOS_W0_BASE,
	[DBSC3_01] = DBSC3_1_QOS_W1_BASE,
	[DBSC3_02] = DBSC3_1_QOS_W2_BASE,
	[DBSC3_03] = DBSC3_1_QOS_W3_BASE,
	[DBSC3_04] = DBSC3_1_QOS_W4_BASE,
	[DBSC3_05] = DBSC3_1_QOS_W5_BASE,
	[DBSC3_06] = DBSC3_1_QOS_W6_BASE,
	[DBSC3_07] = DBSC3_1_QOS_W7_BASE,
	[DBSC3_08] = DBSC3_1_QOS_W8_BASE,
	[DBSC3_09] = DBSC3_1_QOS_W9_BASE,
	[DBSC3_10] = DBSC3_1_QOS_W10_BASE,
	[DBSC3_11] = DBSC3_1_QOS_W11_BASE,
	[DBSC3_12] = DBSC3_1_QOS_W12_BASE,
	[DBSC3_13] = DBSC3_1_QOS_W13_BASE,
	[DBSC3_14] = DBSC3_1_QOS_W14_BASE,
	[DBSC3_15] = DBSC3_1_QOS_W15_BASE,
};

#if defined(CONFIG_QOS_PRI_MEDIA)
#define is_qos_pri_media()	1
#else
#define is_qos_pri_media()	0
#endif

#if defined(CONFIG_QOS_PRI_NORMAL)
#define is_qos_pri_normal()	1
#else
#define is_qos_pri_normal()	0
#endif

#if defined(CONFIG_QOS_PRI_GFX)
#define is_qos_pri_gfx()	1
#else
#define is_qos_pri_gfx()	0
#endif

void qos_init(void)
{
	int i;
	struct rcar_s3c *s3c;
	struct rcar_s3c_qos *s3c_qos;
	struct rcar_dbsc3_qos *qos_addr;
	struct rcar_mxi *mxi;
	struct rcar_mxi_qos *mxi_qos;
	struct rcar_axi_qos *axi_qos;

	/* DBSC DBADJ2 */
	writel(0x20042004, DBSC3_0_DBADJ2);
	writel(0x20042004, DBSC3_1_DBADJ2);

	/* S3C -QoS */
	s3c = (struct rcar_s3c *)S3C_BASE;
	if (IS_R8A7791_ES2()) {
		/* Linear All mode */
		/* writel(0x00000000, &s3c->s3cadsplcr); */
		/* Linear Linear 0x7000 to 0x7800 mode */
		writel(0x00BF1B0C, &s3c->s3cadsplcr);
		/* Split Linear 0x6800 t 0x7000 mode */
		/* writel(0x00DF1B0C, &s3c->s3cadsplcr); */
		/* Ssplit All mode */
		/* writel(0x00FF1B0C, &s3c->s3cadsplcr); */

		if (is_qos_pri_media()) {
			writel(0x1F0B0604, &s3c->s3crorr);
			writel(0x1F0E0705, &s3c->s3cworr);
		} else if (is_qos_pri_normal()) {
			writel(0x1F0B0908, &s3c->s3crorr);
			writel(0x1F0E0A08, &s3c->s3cworr);
		} else if (is_qos_pri_gfx()) {
			writel(0x1F0B0B0B, &s3c->s3crorr);
			writel(0x1F0E0C0C, &s3c->s3cworr);
		}
	} else {
		writel(0x00FF1B1D, &s3c->s3cadsplcr);
		writel(0x1F0D0C0C, &s3c->s3crorr);
		writel(0x1F0D0C0A, &s3c->s3cworr);
	}
	/* QoS Control Registers */
	s3c_qos = (struct rcar_s3c_qos *)S3C_QOS_CCI0_BASE;
	writel(0x00890089, &s3c_qos->s3cqos0);
	writel(0x20960010, &s3c_qos->s3cqos1);
	writel(0x20302030, &s3c_qos->s3cqos2);

	if (IS_R8A7791_ES2()) {
		if (is_qos_pri_media())
			writel(0x20AA2300, &s3c_qos->s3cqos3);
		else if (is_qos_pri_normal())
			writel(0x20AA2200, &s3c_qos->s3cqos3);
		else if (is_qos_pri_gfx())
			writel(0x20AA2100, &s3c_qos->s3cqos3);
	} else {
		writel(0x20AA2200, &s3c_qos->s3cqos3);
	}
	writel(0x00002032, &s3c_qos->s3cqos4);
	writel(0x20960010, &s3c_qos->s3cqos5);
	writel(0x20302030, &s3c_qos->s3cqos6);

	if (IS_R8A7791_ES2()) {
		if (is_qos_pri_media())
			writel(0x20AA2300, &s3c_qos->s3cqos7);
		else if (is_qos_pri_normal())
			writel(0x20AA2200, &s3c_qos->s3cqos7);
		else if (is_qos_pri_gfx())
			writel(0x20AA2100, &s3c_qos->s3cqos7);
	} else {
		writel(0x20AA2200, &s3c_qos->s3cqos7);
	}
	writel(0x00002032, &s3c_qos->s3cqos8);

	s3c_qos = (struct rcar_s3c_qos *)S3C_QOS_CCI1_BASE;
	writel(0x00890089, &s3c_qos->s3cqos0);
	writel(0x20960010, &s3c_qos->s3cqos1);
	writel(0x20302030, &s3c_qos->s3cqos2);
	if (IS_R8A7791_ES2()) {
		if (is_qos_pri_media())
			writel(0x20AA2300, &s3c_qos->s3cqos3);
		else if (is_qos_pri_normal())
			writel(0x20AA2200, &s3c_qos->s3cqos3);
		else if (is_qos_pri_gfx())
			writel(0x20AA2100, &s3c_qos->s3cqos3);
	} else {
		writel(0x20AA2200, &s3c_qos->s3cqos3);
	}
	writel(0x00002032, &s3c_qos->s3cqos4);
	writel(0x20960010, &s3c_qos->s3cqos5);
	writel(0x20302030, &s3c_qos->s3cqos6);
	if (IS_R8A7791_ES2()) {
		if (is_qos_pri_media())
			writel(0x20AA2300, &s3c_qos->s3cqos7);
		else if (is_qos_pri_normal())
			writel(0x20AA2200, &s3c_qos->s3cqos7);
		else if (is_qos_pri_gfx())
			writel(0x20AA2100, &s3c_qos->s3cqos7);
	} else {
		writel(0x20AA2200, &s3c_qos->s3cqos7);
	}
	writel(0x00002032, &s3c_qos->s3cqos8);

	s3c_qos = (struct rcar_s3c_qos *)S3C_QOS_MXI_BASE;
	if (IS_R8A7791_ES2())
		writel(0x80928092, &s3c_qos->s3cqos0);
	else
		writel(0x00820082, &s3c_qos->s3cqos0);
	writel(0x20960020, &s3c_qos->s3cqos1);
	writel(0x20302030, &s3c_qos->s3cqos2);
	writel(0x20AA20DC, &s3c_qos->s3cqos3);
	writel(0x00002032, &s3c_qos->s3cqos4);
	writel(0x20960020, &s3c_qos->s3cqos5);
	writel(0x20302030, &s3c_qos->s3cqos6);
	writel(0x20AA20DC, &s3c_qos->s3cqos7);
	writel(0x00002032, &s3c_qos->s3cqos8);

	s3c_qos = (struct rcar_s3c_qos *)S3C_QOS_AXI_BASE;
	if (IS_R8A7791_ES2())
		writel(0x80928092, &s3c_qos->s3cqos0);
	else
		writel(0x00820082, &s3c_qos->s3cqos0);
	writel(0x20960020, &s3c_qos->s3cqos1);
	writel(0x20302030, &s3c_qos->s3cqos2);
	writel(0x20AA20FA, &s3c_qos->s3cqos3);
	writel(0x00002032, &s3c_qos->s3cqos4);
	writel(0x20960020, &s3c_qos->s3cqos5);
	writel(0x20302030, &s3c_qos->s3cqos6);
	writel(0x20AA20FA, &s3c_qos->s3cqos7);
	writel(0x00002032, &s3c_qos->s3cqos8);

	/* DBSC -QoS */
	/* DBSC0 - Read */
	for (i = DBSC3_00; i < DBSC3_NR; i++) {
		qos_addr = (struct rcar_dbsc3_qos *)dbsc3_0_r_qos_addr[i];
		writel(0x00000002, &qos_addr->dblgcnt);
		writel(0x00002096, &qos_addr->dbtmval0);
		writel(0x00002064, &qos_addr->dbtmval1);
		writel(0x00002032, &qos_addr->dbtmval2);
		writel(0x00001FB0, &qos_addr->dbtmval3);
		writel(0x00000001, &qos_addr->dbrqctr);
		writel(0x00002078, &qos_addr->dbthres0);
		writel(0x0000204B, &qos_addr->dbthres1);
		writel(0x0000201E, &qos_addr->dbthres2);
		writel(0x00000001, &qos_addr->dblgqon);
	}

	/* DBSC0 - Write */
	for (i = DBSC3_00; i < DBSC3_NR; i++) {
		qos_addr = (struct rcar_dbsc3_qos *)dbsc3_0_w_qos_addr[i];
		writel(0x00000002, &qos_addr->dblgcnt);
		writel(0x00002096, &qos_addr->dbtmval0);
		writel(0x00002064, &qos_addr->dbtmval1);
		writel(0x00002050, &qos_addr->dbtmval2);
		writel(0x0000203A, &qos_addr->dbtmval3);
		writel(0x00000001, &qos_addr->dbrqctr);
		writel(0x00002078, &qos_addr->dbthres0);
		writel(0x0000204B, &qos_addr->dbthres1);
		writel(0x0000203C, &qos_addr->dbthres2);
		writel(0x00000001, &qos_addr->dblgqon);
	}

	/* DBSC1 - Read */
	for (i = DBSC3_00; i < DBSC3_NR; i++) {
		qos_addr = (struct rcar_dbsc3_qos *)dbsc3_1_r_qos_addr[i];
		writel(0x00000002, &qos_addr->dblgcnt);
		writel(0x00002096, &qos_addr->dbtmval0);
		writel(0x00002064, &qos_addr->dbtmval1);
		writel(0x00002032, &qos_addr->dbtmval2);
		writel(0x00001FB0, &qos_addr->dbtmval3);
		writel(0x00000001, &qos_addr->dbrqctr);
		writel(0x00002078, &qos_addr->dbthres0);
		writel(0x0000204B, &qos_addr->dbthres1);
		writel(0x0000201E, &qos_addr->dbthres2);
		writel(0x00000001, &qos_addr->dblgqon);
	}

	/* DBSC1 - Write */
	for (i = DBSC3_00; i < DBSC3_NR; i++) {
		qos_addr = (struct rcar_dbsc3_qos *)dbsc3_1_w_qos_addr[i];
		writel(0x00000002, &qos_addr->dblgcnt);
		writel(0x00002096, &qos_addr->dbtmval0);
		writel(0x00002064, &qos_addr->dbtmval1);
		writel(0x00002050, &qos_addr->dbtmval2);
		writel(0x0000203A, &qos_addr->dbtmval3);
		writel(0x00000001, &qos_addr->dbrqctr);
		writel(0x00002078, &qos_addr->dbthres0);
		writel(0x0000204B, &qos_addr->dbthres1);
		writel(0x0000203C, &qos_addr->dbthres2);
		writel(0x00000001, &qos_addr->dblgqon);
	}

	/* CCI-400 -QoS */
	writel(0x20001000, CCI_400_MAXOT_1);
	writel(0x20001000, CCI_400_MAXOT_2);
	writel(0x0000000C, CCI_400_QOSCNTL_1);
	writel(0x0000000C, CCI_400_QOSCNTL_2);

	/* MXI -QoS */
	/* Transaction Control (MXI) */
	mxi = (struct rcar_mxi *)XI_BASE;
	writel(0x00000013, &mxi->mxrtcr);
	if (IS_R8A7791_ES2()) {
		writel(0x00000016, &mxi->mxwtcr);
		writel(0x00780080, &mxi->mxsaar0);
		writel(0x02000800, &mxi->mxsaar1);
	} else {
		writel(0x00000013, &mxi->mxwtcr);
	}

	/* QoS Control (MXI) */
	mxi_qos = (struct rcar_mxi_qos *)MXI_QOS_BASE;
	writel(0x0000000C, &mxi_qos->vspdu0);
	writel(0x0000000C, &mxi_qos->vspdu1);
	writel(0x0000000E, &mxi_qos->du0);
	writel(0x0000000D, &mxi_qos->du1);

	/* AXI -QoS */
	/* Transaction Control (MXI) */
	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SYX64TO128_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_AVB_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x000020A6, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_G2D_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x000020A6, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMP0_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002021, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMP1_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002037, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMUX0_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMUX1_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMUX2_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_LBS_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUDS_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUM_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUS0_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUS1_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MTSB0_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002021, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MTSB1_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002021, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_PCI_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_RTX_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDS0_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x000020A6, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDS1_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x000020A6, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_USB20_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_USB21_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_USB22_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_USB30_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_AX2M_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_CC50_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002029, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_CCI_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_CS_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_DDM_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x000020A6, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_ETH_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MPXM_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SAT0_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SAT1_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDM0_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDM1_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_TRAB_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x000020A6, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_UDM0_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_UDM1_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	/* QoS Register (RT-AXI) */
	axi_qos = (struct rcar_axi_qos *)RT_AXI_SHX_BASE;
	if (IS_R8A7791_ES2())
		writel(0x00000001, &axi_qos->qosconf);
	else
		writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_DBG_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_RDM_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002299, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_RDS_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002029, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_RTX64TO128_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_STPRO_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002029, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_SY2RT_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	/* QoS Register (MP-AXI) */
	axi_qos = (struct rcar_axi_qos *)MP_AXI_ADSP_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002037, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MP_AXI_ASDS0_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002014, &axi_qos->qosctset0);
	writel(0x00000040, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MP_AXI_ASDS1_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002014, &axi_qos->qosctset0);
	writel(0x00000040, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MP_AXI_MLP_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00001FF0, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00002001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MP_AXI_MMUMP_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MP_AXI_SPU_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MP_AXI_SPUC_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000206E, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	/* QoS Register (SYS-AXI256) */
	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_AXI128TO256_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	if (IS_R8A7791_ES2())
		writel(0x000020EB, &axi_qos->qosctset0);
	else
		writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_SYX_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	if (IS_R8A7791_ES2())
		writel(0x000020EB, &axi_qos->qosctset0);
	else
		writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_MPX_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	if (IS_R8A7791_ES2())
		writel(0x000020EB, &axi_qos->qosctset0);
	else
		writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_MXI_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	/* QoS Register (CCI-AXI) */
	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUS0_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_SYX2_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUDS_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUM_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MXI_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x00002245, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUS1_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUMP_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002004, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	/* QoS Register (Media-AXI) */
	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_MXR_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x000020DC, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x000020AA, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_MXW_BASE;
	writel(0x00000002, &axi_qos->qosconf);
	writel(0x000020DC, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x000020AA, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_JPR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002190, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_JPW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002190, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	if (IS_R8A7791_ES2()) {
		writel(0x00000001, &axi_qos->qosthres0);
		writel(0x00000001, &axi_qos->qosthres1);
	} else {
		writel(0x00002064, &axi_qos->qosthres0);
		writel(0x00002004, &axi_qos->qosthres1);
	}
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_TDMR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002190, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_TDMW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002190, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP1CR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002190, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP1CW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002190, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	if (IS_R8A7791_ES2()) {
		writel(0x00000001, &axi_qos->qosthres0);
		writel(0x00000001, &axi_qos->qosthres1);
	} else {
		writel(0x00002064, &axi_qos->qosthres0);
		writel(0x00002004, &axi_qos->qosthres1);
	}
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPDU0CR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002190, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPDU0CW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002190, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	if (IS_R8A7791_ES2()) {
		writel(0x00000001, &axi_qos->qosthres0);
		writel(0x00000001, &axi_qos->qosthres1);
	} else {
		writel(0x00002064, &axi_qos->qosthres0);
		writel(0x00002004, &axi_qos->qosthres1);
	}
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPDU1CR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002190, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPDU1CW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002190, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	if (IS_R8A7791_ES2()) {
		writel(0x00000001, &axi_qos->qosthres0);
		writel(0x00000001, &axi_qos->qosthres1);
	} else {
		writel(0x00002064, &axi_qos->qosthres0);
		writel(0x00002004, &axi_qos->qosthres1);
	}
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VIN0W_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	if (IS_R8A7791_ES2())
		writel(0x00001FF0, &axi_qos->qosctset0);
	else
		writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	if (IS_R8A7791_ES2())
		writel(0x00002001, &axi_qos->qosthres2);
	else
		writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_FDP0R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_FDP0W_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	if (IS_R8A7791_ES2()) {
		writel(0x00000001, &axi_qos->qosthres0);
		writel(0x00000001, &axi_qos->qosthres1);
	} else {
		writel(0x00002064, &axi_qos->qosthres0);
		writel(0x00002004, &axi_qos->qosthres1);
	}
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMSR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMSW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP1R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP1W_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	if (IS_R8A7791_ES2()) {
		writel(0x00000001, &axi_qos->qosthres0);
		writel(0x00000001, &axi_qos->qosthres1);
	} else {
		writel(0x00002064, &axi_qos->qosthres0);
		writel(0x00002004, &axi_qos->qosthres1);
	}
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_FDP1R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_FDP1W_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	if (IS_R8A7791_ES2()) {
		writel(0x00000001, &axi_qos->qosthres0);
		writel(0x00000001, &axi_qos->qosthres1);
	} else {
		writel(0x00002064, &axi_qos->qosthres0);
		writel(0x00002004, &axi_qos->qosthres1);
	}
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMRR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMRW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPD0R_BASE;
	if (IS_R8A7791_ES2())
		writel(0x00000003, &axi_qos->qosconf);
	else
		writel(0x00000000, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPD0W_BASE;
	if (IS_R8A7791_ES2())
		writel(0x00000003, &axi_qos->qosconf);
	else
		writel(0x00000000, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPD1R_BASE;
	if (IS_R8A7791_ES2())
		writel(0x00000003, &axi_qos->qosconf);
	else
		writel(0x00000000, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPD1W_BASE;
	if (IS_R8A7791_ES2())
		writel(0x00000003, &axi_qos->qosconf);
	else
		writel(0x00000000, &axi_qos->qosconf);
	writel(0x000020C8, &axi_qos->qosctset0);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_DU0R_BASE;
	if (IS_R8A7791_ES2())
		writel(0x00000003, &axi_qos->qosconf);
	else
		writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002063, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_DU0W_BASE;
	if (IS_R8A7791_ES2())
		writel(0x00000000, &axi_qos->qosconf);
	else
		writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002063, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCP0CR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002073, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCP0CW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002073, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	if (IS_R8A7791_ES2()) {
		writel(0x00000001, &axi_qos->qosthres0);
		writel(0x00000001, &axi_qos->qosthres1);
	} else {
		writel(0x00002064, &axi_qos->qosthres0);
		writel(0x00002004, &axi_qos->qosthres1);
	}
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCP0VR_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002073, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCP0VW_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002073, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	if (IS_R8A7791_ES2()) {
		writel(0x00000001, &axi_qos->qosthres0);
		writel(0x00000001, &axi_qos->qosthres1);
	} else {
		writel(0x00002064, &axi_qos->qosthres0);
		writel(0x00002004, &axi_qos->qosthres1);
	}
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VPC0R_BASE;
	writel(0x00000001, &axi_qos->qosconf);
	writel(0x00002073, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002064, &axi_qos->qosthres0);
	writel(0x00002004, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);
}
#else /* CONFIG_ARCH_RMOBILE_EXTRAM_BOOT */
void qos_init(void)
{
}
#endif /* CONFIG_ARCH_RMOBILE_EXTRAM_BOOT */
