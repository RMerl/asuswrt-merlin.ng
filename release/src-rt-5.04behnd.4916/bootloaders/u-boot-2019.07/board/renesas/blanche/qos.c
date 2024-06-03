// SPDX-License-Identifier: GPL-2.0
/*
 * board/renesas/blanche/qos.c
 *
 * Copyright (C) 2016 Renesas Electronics Corporation
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/arch/rmobile.h>

#if defined(CONFIG_RMOBILE_EXTRAM_BOOT)
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
	writel(0x20082004, DBSC3_0_DBADJ2);

	/* S3C -QoS */
	s3c = (struct rcar_s3c *)S3C_BASE;
	// writel(0x00000000, &s3c->s3cadsplcr);
	writel(0x1F0D0C0C, &s3c->s3crorr);
	writel(0x1F1F0C0C, &s3c->s3cworr);

	/* QoS Control Registers */
	s3c_qos = (struct rcar_s3c_qos *)S3C_QOS_CCI0_BASE;
	writel(0x00890089, &s3c_qos->s3cqos0);
	writel(0x20960010, &s3c_qos->s3cqos1);
	writel(0x20302030, &s3c_qos->s3cqos2);
	writel(0x20AA2200, &s3c_qos->s3cqos3);
	writel(0x00002032, &s3c_qos->s3cqos4);
	writel(0x20960010, &s3c_qos->s3cqos5);
	writel(0x20302030, &s3c_qos->s3cqos6);
	writel(0x20AA2200, &s3c_qos->s3cqos7);
	writel(0x00002032, &s3c_qos->s3cqos8);

	s3c_qos = (struct rcar_s3c_qos *)S3C_QOS_CCI1_BASE;
	writel(0x00890089, &s3c_qos->s3cqos0);
	writel(0x20960010, &s3c_qos->s3cqos1);
	writel(0x20302030, &s3c_qos->s3cqos2);
	writel(0x20AA2200, &s3c_qos->s3cqos3);
	writel(0x00002032, &s3c_qos->s3cqos4);
	writel(0x20960010, &s3c_qos->s3cqos5);
	writel(0x20302030, &s3c_qos->s3cqos6);
	writel(0x20AA2200, &s3c_qos->s3cqos7);
	writel(0x00002032, &s3c_qos->s3cqos8);

	s3c_qos = (struct rcar_s3c_qos *)S3C_QOS_MXI_BASE;
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
	writel(0x80918099, &s3c_qos->s3cqos0);
	writel(0x20410010, &s3c_qos->s3cqos1);
	writel(0x200A2023, &s3c_qos->s3cqos2);
	writel(0x20502001, &s3c_qos->s3cqos3);
	writel(0x00002032, &s3c_qos->s3cqos4);
	writel(0x20410FFF, &s3c_qos->s3cqos5);
	writel(0x200A2023, &s3c_qos->s3cqos6);
	writel(0x20502001, &s3c_qos->s3cqos7);
	writel(0x20142032, &s3c_qos->s3cqos8);

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
		writel(0x0000204B, &qos_addr->dbthres0);
		writel(0x0000204B, &qos_addr->dbthres1);
		writel(0x00001FC4, &qos_addr->dbthres2);
		writel(0x00000001, &qos_addr->dblgqon);
	}

	/* DBSC0 - Write */
	for (i = DBSC3_00; i < DBSC3_NR; i++) {
		qos_addr = (struct rcar_dbsc3_qos *)dbsc3_0_w_qos_addr[i];
		writel(0x00000002, &qos_addr->dblgcnt);
		writel(0x00002096, &qos_addr->dbtmval0);
		writel(0x0000206E, &qos_addr->dbtmval1);
		writel(0x00002050, &qos_addr->dbtmval2);
		writel(0x0000203A, &qos_addr->dbtmval3);
		writel(0x00000001, &qos_addr->dbrqctr);
		writel(0x0000205A, &qos_addr->dbthres0);
		writel(0x0000205A, &qos_addr->dbthres1);
		writel(0x0000203C, &qos_addr->dbthres2);
		writel(0x00000001, &qos_addr->dblgqon);
	}

	/* MXI -QoS */
	/* Transaction Control (MXI) */
	mxi = (struct rcar_mxi *)MXI_BASE;
	writel(0x00000100, &mxi->mxaxirtcr);
	writel(0xFF530100, &mxi->mxaxiwtcr);
	writel(0x00000100, &mxi->mxs3crtcr);
	writel(0xFF530100, &mxi->mxs3cwtcr);
	writel(0x004000C0, &mxi->mxsaar0);
	writel(0x02000800, &mxi->mxsaar1);

	/* QoS Control (MXI) */
	mxi_qos = (struct rcar_mxi_qos *)MXI_QOS_BASE;
	writel(0x0000000C, &mxi_qos->du0);

	/* AXI -QoS */
	/* Transaction Control (MXI) */
	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SYX64TO128_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_AVB_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_CC50_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002029, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_CCI_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_CS_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_G2D_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x000020A6, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMP1_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x000020A6, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMUX0_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMUX1_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_LBS_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUDS_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002008, &axi_qos->qosctset0);
	writel(0x00000010, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUM_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002008, &axi_qos->qosctset0);
	writel(0x00000010, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUS0_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002008, &axi_qos->qosctset0);
	writel(0x00000010, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_MMUS1_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002008, &axi_qos->qosctset0);
	writel(0x00000010, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_RTX_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDM0_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDM1_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDS0_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDS1_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_TRAB_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x000020A6, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_ADM_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_ADS_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SYX_BASE;
	writel(0x00002041, &axi_qos->qosctset1);
	writel(0x00002023, &axi_qos->qosctset2);
	writel(0x0000200A, &axi_qos->qosctset3);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_AXI64TO128W_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_AVBW_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_CC50W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002029, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_CCIW_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_CCSW_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_G2DW_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x000020A6, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMUX0W_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMUX1W_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_IMUX2W_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_LBSW_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_RTXBW_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDM0W_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDM1W_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDS0W_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SDS1W_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_TRABW_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x000020A6, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_ADMW_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_ADSW_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x0000214C, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI_SYXW_BASE;
	writel(0x00002041, &axi_qos->qosctset1);
	writel(0x00002023, &axi_qos->qosctset2);
	writel(0x0000200A, &axi_qos->qosctset3);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);

	/* QoS Register (SYS-AXI256) */
	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_AXI128TO256_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_AXI_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_MXI_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_IMP0_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x0000211B, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_SY2_BASE;
	writel(0x00002041, &axi_qos->qosctset1);
	writel(0x00002023, &axi_qos->qosctset2);
	writel(0x0000200A, &axi_qos->qosctset3);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256W_AXI128TO256_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_AXMW_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_MXIW_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_IMP0W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002029, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00000001, &axi_qos->qosqon);
	writel(0x00000005, &axi_qos->qosin);

	axi_qos = (struct rcar_axi_qos *)SYS_AXI256_SY2W_BASE;
	writel(0x00002041, &axi_qos->qosctset1);
	writel(0x00002023, &axi_qos->qosctset2);
	writel(0x0000200A, &axi_qos->qosctset3);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);

	/* QoS Register (RT-AXI) */
	axi_qos = (struct rcar_axi_qos *)RT_AXI_SHX_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002055, &axi_qos->qosctset0);
	writel(0x00000000, &axi_qos->qosreqctr);
	writel(0x00000000, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_DBG_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002055, &axi_qos->qosctset0);
	writel(0x00000000, &axi_qos->qosreqctr);
	writel(0x00000000, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_RTX64TO128_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00000000, &axi_qos->qosreqctr);
	writel(0x00000000, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_RT_BASE;
	writel(0x00002001, &axi_qos->qosctset1);
	writel(0x00002001, &axi_qos->qosctset2);
	writel(0x00002001, &axi_qos->qosctset3);
	writel(0x00000000, &axi_qos->qosthres0);
	writel(0x00000000, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_SHXW_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002055, &axi_qos->qosctset0);
	writel(0x00000000, &axi_qos->qosreqctr);
	writel(0x00000000, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_DBGW_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002055, &axi_qos->qosctset0);
	writel(0x00000000, &axi_qos->qosreqctr);
	writel(0x00000000, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_RTX64TO128W_BASE;
	writel(0x00000000, &axi_qos->qosconf);
	writel(0x00002001, &axi_qos->qosctset0);
	writel(0x00000000, &axi_qos->qosreqctr);
	writel(0x00000000, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)RT_AXI_RTW_BASE;
	writel(0x00002001, &axi_qos->qosctset1);
	writel(0x00002001, &axi_qos->qosctset2);
	writel(0x00002001, &axi_qos->qosctset3);
	writel(0x00000000, &axi_qos->qosthres0);
	writel(0x00000000, &axi_qos->qosthres1);
	writel(0x00000000, &axi_qos->qosthres2);

	/* QoS Register (CCI-AXI) */
	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUS0_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002008, &axi_qos->qosctset0);
	writel(0x00002041, &axi_qos->qosctset1);
	writel(0x00002023, &axi_qos->qosctset2);
	writel(0x0000200A, &axi_qos->qosctset3);
	writel(0x00000010, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_SYX2_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00002041, &axi_qos->qosctset1);
	writel(0x00002023, &axi_qos->qosctset2);
	writel(0x0000200A, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUR_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002008, &axi_qos->qosctset0);
	writel(0x00002041, &axi_qos->qosctset1);
	writel(0x00002023, &axi_qos->qosctset2);
	writel(0x0000000A, &axi_qos->qosctset3);
	writel(0x00000010, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002018, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUDS_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002008, &axi_qos->qosctset0);
	writel(0x00002041, &axi_qos->qosctset1);
	writel(0x00002023, &axi_qos->qosctset2);
	writel(0x0000200A, &axi_qos->qosctset3);
	writel(0x00000010, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUM_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002008, &axi_qos->qosctset0);
	writel(0x00002041, &axi_qos->qosctset1);
	writel(0x00002023, &axi_qos->qosctset2);
	writel(0x0000200A, &axi_qos->qosctset3);
	writel(0x00000010, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MXI_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x0000205F, &axi_qos->qosctset0);
	writel(0x00002041, &axi_qos->qosctset1);
	writel(0x00002023, &axi_qos->qosctset2);
	writel(0x0000200A, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUS1_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002008, &axi_qos->qosctset0);
	writel(0x00002041, &axi_qos->qosctset1);
	writel(0x00002023, &axi_qos->qosctset2);
	writel(0x0000200A, &axi_qos->qosctset3);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)CCI_AXI_MMUMP_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002008, &axi_qos->qosctset0);
	writel(0x00002041, &axi_qos->qosctset1);
	writel(0x00002023, &axi_qos->qosctset2);
	writel(0x0000200A, &axi_qos->qosctset3);
	writel(0x00000010, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	/* QoS Register (Media-AXI) */
	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_MXR_BASE;
	writel(0x00000102, &axi_qos->qosconf);
	writel(0x000020DC, &axi_qos->qosctset0);
	writel(0x00002096, &axi_qos->qosctset1);
	writel(0x00002030, &axi_qos->qosctset2);
	writel(0x00002030, &axi_qos->qosctset3);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x000020AA, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_MXW_BASE;
	writel(0x00000102, &axi_qos->qosconf);
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
	writel(0x00002018, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002006, &axi_qos->qosthres0);
	writel(0x00002001, &axi_qos->qosthres1);
	writel(0x00000001, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_JPW_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002259, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCTU0R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VCTU0W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VDCTU0R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VDCTU0W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VDCTU1R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VDCTU1W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002053, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VIN0W_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002046, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VIN1W_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002046, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_RDRW_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x000020D0, &axi_qos->qosctset0);
	writel(0x00000020, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMS01R_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002034, &axi_qos->qosctset0);
	writel(0x0000000C, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMS01W_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x0000200D, &axi_qos->qosctset0);
	writel(0x000000C0, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMS23R_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002034, &axi_qos->qosctset0);
	writel(0x0000000C, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMS23W_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x0000200D, &axi_qos->qosctset0);
	writel(0x000000C0, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMS45R_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002034, &axi_qos->qosctset0);
	writel(0x0000000C, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMS45W_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x0000200D, &axi_qos->qosctset0);
	writel(0x000000C0, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMRR_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002069, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_IMRW_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002069, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTCE4R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x0000204C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTCE4W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002200, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTVLC4R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002455, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTVLC4W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002455, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPD0R_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002034, &axi_qos->qosctset0);
	writel(0x00000008, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPD0W_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x000020D3, &axi_qos->qosctset0);
	writel(0x00000008, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPD1R_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002034, &axi_qos->qosctset0);
	writel(0x00000008, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSPD1W_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x000020D3, &axi_qos->qosctset0);
	writel(0x00000008, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_DU0R_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x0000201A, &axi_qos->qosctset0);
	writel(0x00000018, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_DU0W_BASE;
	writel(0x00000101, &axi_qos->qosconf);
	writel(0x00002006, &axi_qos->qosctset0);
	writel(0x00000018, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP0R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x0000201A, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_VSP0W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002042, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTCE0R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x0000204C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTCE0W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002200, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTVLC0R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002455, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTVLC0W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002455, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTCE1R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x0000204C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTCE1W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002200, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTVLC1R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002455, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTVLC1W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002455, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTCE2R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x0000204C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTCE2W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002200, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTVLC2R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002455, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTVLC2W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002455, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTCE3R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x0000204C, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTCE3W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002200, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTVLC3R_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002455, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	axi_qos = (struct rcar_axi_qos *)MEDIA_AXI_ROTVLC3W_BASE;
	writel(0x00000100, &axi_qos->qosconf);
	writel(0x00002455, &axi_qos->qosctset0);
	writel(0x00000001, &axi_qos->qosreqctr);
	writel(0x00002050, &axi_qos->qosthres0);
	writel(0x00002032, &axi_qos->qosthres1);
	writel(0x00002014, &axi_qos->qosthres2);
	writel(0x00000001, &axi_qos->qosqon);

	/* DMS Register(SYS-AXI) */
	writel(0x00000000, SYS_AXI_AVBDMSCR);
	writel(0x00000000, SYS_AXI_AX2MDMSCR);
	writel(0x00000000, SYS_AXI_CC50DMSCR);
	writel(0x00000000, SYS_AXI_CCIDMSCR);
	writel(0x00000000, SYS_AXI_CSDMSCR);
	writel(0x00000000, SYS_AXI_G2DDMSCR);
	writel(0x00000000, SYS_AXI_IMP1DMSCR);
	writel(0x00000000, SYS_AXI_LBSMDMSCR);
	writel(0x00000000, SYS_AXI_MMUDSDMSCR);
	writel(0x00000000, SYS_AXI_MMUMXDMSCR);
	writel(0x00000000, SYS_AXI_MMUS0DMSCR);
	writel(0x00000000, SYS_AXI_MMUS1DMSCR);
	writel(0x00000000, SYS_AXI_RTMXDMSCR);
	writel(0x00000000, SYS_AXI_SDM0DMSCR);
	writel(0x00000000, SYS_AXI_SDM1DMSCR);
	writel(0x00000000, SYS_AXI_SDS0DMSCR);
	writel(0x00000000, SYS_AXI_SDS1DMSCR);
	writel(0x00000000, SYS_AXI_TRABDMSCR);
	writel(0x00000000, SYS_AXI_X128TO64SLVDMSCR);
	writel(0x00000000, SYS_AXI_X64TO128SLVDMSCR);
	writel(0x00000000, SYS_AXI_AVBSLVDMSCR);
	writel(0x00000000, SYS_AXI_AX2SLVDMSCR);
	writel(0x00000000, SYS_AXI_GICSLVDMSCR);
	writel(0x00000000, SYS_AXI_IMPSLVDMSCR);
	writel(0x00000000, SYS_AXI_IMPSLVDMSCR);
	writel(0x00000000, SYS_AXI_IMX0SLVDMSCR);
	writel(0x00000000, SYS_AXI_IMX1SLVDMSCR);
	writel(0x00000000, SYS_AXI_IMX2SLVDMSCR);
	writel(0x00000000, SYS_AXI_LBSSLVDMSCR);
	writel(0x00000000, SYS_AXI_MXTSLVDMSCR);
	writel(0x00000000, SYS_AXI_SYAPBSLVDMSCR);
	writel(0x00000000, SYS_AXI_QSAPBSLVDMSCR);
	writel(0x00000000, SYS_AXI_RTXSLVDMSCR);
	writel(0x00000000, SYS_AXI_SAPC1SLVDMSCR);
	writel(0x00000000, SYS_AXI_SAPC2SLVDMSCR);
	writel(0x00000000, SYS_AXI_SAPC3SLVDMSCR);
	writel(0x00000000, SYS_AXI_SAPC65SLVDMSCR);
	writel(0x00000000, SYS_AXI_SAPC8SLVDMSCR);
	writel(0x00000000, SYS_AXI_SDAP0SLVDMSCR);
	writel(0x00000000, SYS_AXI_SGXSLV1SLVDMSCR);
	writel(0x00000000, SYS_AXI_STBSLVDMSCR);
	writel(0x00000000, SYS_AXI_STMSLVDMSCR);
	writel(0x00000000, SYS_AXI_SYXXDEFAULTSLAVESLVDMSCR);
	writel(0x00000000, SYS_AXI_TSPL0SLVDMSCR);
	writel(0x00000000, SYS_AXI_TSPL1SLVDMSCR);
	writel(0x00000000, SYS_AXI_TSPL2SLVDMSCR);
	writel(0x00000000, SYS_AXI_UTLBDSSLVDMSCR);
	writel(0x00000000, SYS_AXI_UTLBS0SLVDMSCR);
	writel(0x00000000, SYS_AXI_UTLBS1SLVDMSCR);
	writel(0x00000000, SYS_AXI_ROT0DMSCR);
	writel(0x00000000, SYS_AXI_ROT1DMSCR);
	writel(0x00000000, SYS_AXI_ROT2DMSCR);
	writel(0x00000000, SYS_AXI_ROT3DMSCR);
	writel(0x00000000, SYS_AXI_ROT4DMSCR);
	writel(0x00000000, SYS_AXI_IMUX3SLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR0SLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR0PSLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR0XSLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR1SLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR1PSLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR1XSLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR2SLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR2PSLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR2XSLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR3SLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR3PSLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR3XSLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR4SLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR4PSLVDMSCR);
	writel(0x00000000, SYS_AXI_STBR4XSLVDMSCR);
	writel(0x00000000, SYS_AXI_ADM_DMSCR);
	writel(0x00000000, SYS_AXI_ADS_DMSCR);

	/* DMS Register(RT-AXI) */
	writel(0x00000000, DM_AXI_DMAXICONF);
	writel(0x00000019, DM_AXI_DMAPBCONF);
	writel(0x00000000, DM_AXI_DMADMCONF);
	writel(0x00000000, DM_AXI_DMSDM0CONF);
	writel(0x00000000, DM_AXI_DMSDM1CONF);
	writel(0x00000004, DM_AXI_DMQSPAPSLVCONF);
	writel(0x00000004, DM_AXI_RAPD4SLVCONF);
	writel(0x00000004, DM_AXI_SAPD4SLVCONF);
	writel(0x00000004, DM_AXI_SAPD5SLVCONF);
	writel(0x00000004, DM_AXI_SAPD6SLVCONF);
	writel(0x00000004, DM_AXI_SAPD65DSLVCONF);
	writel(0x00000004, DM_AXI_SDAP0SLVCONF);
	writel(0x00000004, DM_AXI_MAPD2SLVCONF);
	writel(0x00000004, DM_AXI_MAPD3SLVCONF);
	writel(0x00000000, DM_AXI_DMXXDEFAULTSLAVESLVCONF);
	writel(0x00000100, DM_AXI_DMADMRQOSCONF);
	writel(0x0000214C, DM_AXI_DMADMRQOSCTSET0);
	writel(0x00000001, DM_AXI_DMADMRQOSREQCTR);
	writel(0x00000001, DM_AXI_DMADMRQOSQON);
	writel(0x00000005, DM_AXI_DMADMRQOSIN);
	writel(0x00000000, DM_AXI_DMADMRQOSSTAT);
	writel(0x00000000, DM_AXI_DMSDM0RQOSCONF);
	writel(0x0000214C, DM_AXI_DMSDM0RQOSCTSET0);
	writel(0x00000001, DM_AXI_DMSDM0RQOSREQCTR);
	writel(0x00000001, DM_AXI_DMSDM0RQOSQON);
	writel(0x00000005, DM_AXI_DMSDM0RQOSIN);
	writel(0x00000000, DM_AXI_DMSDM0RQOSSTAT);
	writel(0x00000000, DM_AXI_DMSDM1RQOSCONF);
	writel(0x0000214C, DM_AXI_DMSDM1RQOSCTSET0);
	writel(0x00000001, DM_AXI_DMSDM1RQOSREQCTR);
	writel(0x00000001, DM_AXI_DMSDM1RQOSQON);
	writel(0x00000005, DM_AXI_DMSDM1RQOSIN);
	writel(0x00000000, DM_AXI_DMSDM1RQOSSTAT);
	writel(0x00002041, DM_AXI_DMRQOSCTSET1);
	writel(0x00002023, DM_AXI_DMRQOSCTSET2);
	writel(0x0000200A, DM_AXI_DMRQOSCTSET3);
	writel(0x00002050, DM_AXI_DMRQOSTHRES0);
	writel(0x00002032, DM_AXI_DMRQOSTHRES1);
	writel(0x00002014, DM_AXI_DMRQOSTHRES2);
	writel(0x00000100, DM_AXI_DMADMWQOSCONF);
	writel(0x0000214C, DM_AXI_DMADMWQOSCTSET0);
	writel(0x00000001, DM_AXI_DMADMWQOSREQCTR);
	writel(0x00000001, DM_AXI_DMADMWQOSQON);
	writel(0x00000005, DM_AXI_DMADMWQOSIN);
	writel(0x00000000, DM_AXI_DMADMWQOSSTAT);
	writel(0x00000000, DM_AXI_DMSDM0WQOSCONF);
	writel(0x0000214C, DM_AXI_DMSDM0WQOSCTSET0);
	writel(0x00000001, DM_AXI_DMSDM0WQOSREQCTR);
	writel(0x00000001, DM_AXI_DMSDM0WQOSQON);
	writel(0x00000005, DM_AXI_DMSDM0WQOSIN);
	writel(0x00000000, DM_AXI_DMSDM0WQOSSTAT);
	writel(0x00000000, DM_AXI_DMSDM1WQOSCONF);
	writel(0x0000214C, DM_AXI_DMSDM1WQOSCTSET0);
	writel(0x00000001, DM_AXI_DMSDM1WQOSREQCTR);
	writel(0x00000001, DM_AXI_DMSDM1WQOSQON);
	writel(0x00000005, DM_AXI_DMSDM1WQOSIN);
	writel(0x00000000, DM_AXI_DMSDM1WQOSSTAT);
	writel(0x00002041, DM_AXI_DMWQOSCTSET1);
	writel(0x00002023, DM_AXI_DMWQOSCTSET2);
	writel(0x0000200A, DM_AXI_DMWQOSCTSET3);
	writel(0x00002050, DM_AXI_DMWQOSTHRES0);
	writel(0x00002032, DM_AXI_DMWQOSTHRES1);
	writel(0x00002014, DM_AXI_DMWQOSTHRES2);
	writel(0x00000000, DM_AXI_RDMDMSCR);
	writel(0x00000000, DM_AXI_SDM0DMSCR);
	writel(0x00000000, DM_AXI_SDM1DMSCR);
	writel(0x00000000, DM_AXI_DMQSPAPSLVDMSCR);
	writel(0x00000000, DM_AXI_RAPD4SLVDMSCR);
	writel(0x00000000, DM_AXI_SAPD4SLVDMSCR);
	writel(0x00000000, DM_AXI_SAPD5SLVDMSCR);
	writel(0x00000000, DM_AXI_SAPD6SLVDMSCR);
	writel(0x00000000, DM_AXI_SAPD65DSLVDMSCR);
	writel(0x00000000, DM_AXI_SDAP0SLVDMSCR);
	writel(0x00000000, DM_AXI_MAPD2SLVDMSCR);
	writel(0x00000000, DM_AXI_MAPD3SLVDMSCR);
	writel(0x00000000, DM_AXI_DMXXDEFAULTSLAVESLVDMSCR);
	writel(0x00000001, DM_AXI_DMXREGDMSENN);

	/* DMS Register(SYS-AXI256) */
	writel(0x00000000, SYS_AXI256_SYXDMSCR);
	writel(0x00000000, SYS_AXI256_MXIDMSCR);
	writel(0x00000000, SYS_AXI256_X128TO256SLVDMSCR);
	writel(0x00000000, SYS_AXI256_X256TO128SLVDMSCR);
	writel(0x00000000, SYS_AXI256_SYXSLVDMSCR);
	writel(0x00000000, SYS_AXI256_CCXSLVDMSCR);
	writel(0x00000000, SYS_AXI256_S3CSLVDMSCR);

	/* DMS Register(MXT) */
	writel(0x00000000, MXT_SYXDMSCR);
	writel(0x00000000, MXT_IMRSLVDMSCR);
	writel(0x00000000, MXT_VINSLVDMSCR);
	writel(0x00000000, MXT_VPC1SLVDMSCR);
	writel(0x00000000, MXT_VSPD0SLVDMSCR);
	writel(0x00000000, MXT_VSPD1SLVDMSCR);
	writel(0x00000000, MXT_MAP1SLVDMSCR);
	writel(0x00000000, MXT_MAP2SLVDMSCR);
	writel(0x00000000, MXT_MAP2BSLVDMSCR);

	/* DMS Register(MXI) */
	writel(0x00000002, MXI_JPURDMSCR);
	writel(0x00000002, MXI_JPUWDMSCR);
	writel(0x00000002, MXI_VCTU0RDMSCR);
	writel(0x00000002, MXI_VCTU0WDMSCR);
	writel(0x00000002, MXI_VDCTU0RDMSCR);
	writel(0x00000002, MXI_VDCTU0WDMSCR);
	writel(0x00000002, MXI_VDCTU1RDMSCR);
	writel(0x00000002, MXI_VDCTU1WDMSCR);
	writel(0x00000002, MXI_VIN0WDMSCR);
	writel(0x00000002, MXI_VIN1WDMSCR);
	writel(0x00000002, MXI_RDRWDMSCR);
	writel(0x00000002, MXI_IMS01RDMSCR);
	writel(0x00000002, MXI_IMS01WDMSCR);
	writel(0x00000002, MXI_IMS23RDMSCR);
	writel(0x00000002, MXI_IMS23WDMSCR);
	writel(0x00000002, MXI_IMS45RDMSCR);
	writel(0x00000002, MXI_IMS45WDMSCR);
	writel(0x00000002, MXI_IMRRDMSCR);
	writel(0x00000002, MXI_IMRWDMSCR);
	writel(0x00000002, MXI_ROTCE4RDMSCR);
	writel(0x00000002, MXI_ROTCE4WDMSCR);
	writel(0x00000002, MXI_ROTVLC4RDMSCR);
	writel(0x00000002, MXI_ROTVLC4WDMSCR);
	writel(0x00000002, MXI_VSPD0RDMSCR);
	writel(0x00000002, MXI_VSPD0WDMSCR);
	writel(0x00000002, MXI_VSPD1RDMSCR);
	writel(0x00000002, MXI_VSPD1WDMSCR);
	writel(0x00000002, MXI_DU0RDMSCR);
	writel(0x00000002, MXI_DU0WDMSCR);
	writel(0x00000002, MXI_VSP0RDMSCR);
	writel(0x00000002, MXI_VSP0WDMSCR);
	writel(0x00000002, MXI_ROTCE0RDMSCR);
	writel(0x00000002, MXI_ROTCE0WDMSCR);
	writel(0x00000002, MXI_ROTVLC0RDMSCR);
	writel(0x00000002, MXI_ROTVLC0WDMSCR);
	writel(0x00000002, MXI_ROTCE1RDMSCR);
	writel(0x00000002, MXI_ROTCE1WDMSCR);
	writel(0x00000002, MXI_ROTVLC1RDMSCR);
	writel(0x00000002, MXI_ROTVLC1WDMSCR);
	writel(0x00000002, MXI_ROTCE2RDMSCR);
	writel(0x00000002, MXI_ROTCE2WDMSCR);
	writel(0x00000002, MXI_ROTVLC2RDMSCR);
	writel(0x00000002, MXI_ROTVLC2WDMSCR);
	writel(0x00000002, MXI_ROTCE3RDMSCR);
	writel(0x00000002, MXI_ROTCE3WDMSCR);
	writel(0x00000002, MXI_ROTVLC3RDMSCR);
	writel(0x00000002, MXI_ROTVLC3WDMSCR);

	/* DMS Register(CCI-AXI) */
	writel(0x00000000, CCI_AXI_MMUS0DMSCR);
	writel(0x00000000, CCI_AXI_SYX2DMSCR);
	writel(0x00000000, CCI_AXI_MMURDMSCR);
	writel(0x00000000, CCI_AXI_MMUDSDMSCR);
	writel(0x00000000, CCI_AXI_MMUMDMSCR);
	writel(0x00000000, CCI_AXI_MXIDMSCR);
	writel(0x00000000, CCI_AXI_MMUS1DMSCR);
	writel(0x00000000, CCI_AXI_MMUMPDMSCR);
	writel(0x00000000, CCI_AXI_DVMDMSCR);
	writel(0x00000000, CCI_AXI_CCISLVDMSCR);

	/* CC-AXI Function Register */
	writel(0x00000011, CCI_AXI_IPMMUIDVMCR);
	writel(0x00000011, CCI_AXI_IPMMURDVMCR);
	writel(0x00000011, CCI_AXI_IPMMUS0DVMCR);
	writel(0x00000011, CCI_AXI_IPMMUS1DVMCR);
	writel(0x00000011, CCI_AXI_IPMMUMPDVMCR);
	writel(0x00000011, CCI_AXI_IPMMUDSDVMCR);
	writel(0x0000F700, CCI_AXI_AX2ADDRMASK);

}
#else /* CONFIG_RMOBILE_EXTRAM_BOOT */
void qos_init(void)
{
}
#endif /* CONFIG_RMOBILE_EXTRAM_BOOT */
