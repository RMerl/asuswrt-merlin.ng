// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>

#ifdef CONFIG_SYS_DPAA_QBMAN
struct qportal_info qp_info[CONFIG_SYS_QMAN_NUM_PORTALS] = {
	/* dqrr liodn, frame data liodn, liodn off, sdest */
	SET_QP_INFO(1, 27, 1, 0),
	SET_QP_INFO(2, 28, 1, 0),
	SET_QP_INFO(3, 29, 1, 1),
	SET_QP_INFO(4, 30, 1, 1),
	SET_QP_INFO(5, 31, 1, 2),
	SET_QP_INFO(6, 32, 1, 2),
	SET_QP_INFO(7, 33, 1, 3),
	SET_QP_INFO(8, 34, 1, 3),
	SET_QP_INFO(9, 35, 1, 0),
	SET_QP_INFO(10, 36, 1, 0),
	SET_QP_INFO(11, 37, 1, 1),
	SET_QP_INFO(12, 38, 1, 1),
	SET_QP_INFO(13, 39, 1, 2),
	SET_QP_INFO(14, 40, 1, 2),
	SET_QP_INFO(15, 41, 1, 3),
	SET_QP_INFO(16, 42, 1, 3),
	SET_QP_INFO(17, 43, 1, 0),
	SET_QP_INFO(18, 44, 1, 0),
};
#endif

#ifdef CONFIG_SYS_SRIO
struct srio_liodn_id_table srio_liodn_tbl[] = {
	SET_SRIO_LIODN_BASE(1, 307),
	SET_SRIO_LIODN_BASE(2, 387),
};
int srio_liodn_tbl_sz = ARRAY_SIZE(srio_liodn_tbl);
#endif

struct liodn_id_table liodn_tbl[] = {
#ifdef CONFIG_SYS_DPAA_QBMAN
	SET_QMAN_LIODN(62),
	SET_BMAN_LIODN(63),
#endif

	SET_SDHC_LIODN(1, 552),

	SET_PME_LIODN(117),

	SET_USB_LIODN(1, "fsl-usb2-mph", 553),
	SET_USB_LIODN(2, "fsl-usb2-dr", 554),

#ifdef CONFIG_FSL_SATA_V2
	SET_SATA_LIODN(1, 555),
	SET_SATA_LIODN(2, 556),
#endif

	SET_PCI_LIODN_BASE(CONFIG_SYS_FSL_PCIE_COMPAT, 1, 148),
	SET_PCI_LIODN_BASE(CONFIG_SYS_FSL_PCIE_COMPAT, 2, 228),
	SET_PCI_LIODN_BASE(CONFIG_SYS_FSL_PCIE_COMPAT, 3, 308),
	SET_PCI_LIODN_BASE(CONFIG_SYS_FSL_PCIE_COMPAT, 4, 388),

	SET_DMA_LIODN(1, "fsl,elo3-dma", 147),
	SET_DMA_LIODN(2, "fsl,elo3-dma", 227),
	SET_DMA_LIODN(3, "fsl,elo3-dma", 226),

	SET_GUTS_LIODN("fsl,rapidio-delta", 199, rio1liodnr, 0),
	SET_GUTS_LIODN(NULL, 200, rio2liodnr, 0),
	SET_GUTS_LIODN(NULL, 201, rio1maintliodnr, 0),
	SET_GUTS_LIODN(NULL, 202, rio2maintliodnr, 0),

#ifdef CONFIG_SYS_PMAN
	SET_PMAN_LIODN(1, 513),
	SET_PMAN_LIODN(2, 514),
	SET_PMAN_LIODN(3, 515),
#endif

	/* SET_NEXUS_LIODN(557), -- not yet implemented */
};
int liodn_tbl_sz = ARRAY_SIZE(liodn_tbl);

#ifdef CONFIG_SYS_DPAA_FMAN
struct fman_liodn_id_table fman1_liodn_tbl[] = {
	SET_FMAN_RX_1G_LIODN(1, 0, 88),
	SET_FMAN_RX_1G_LIODN(1, 1, 89),
	SET_FMAN_RX_1G_LIODN(1, 2, 90),
	SET_FMAN_RX_1G_LIODN(1, 3, 91),
	SET_FMAN_RX_1G_LIODN(1, 4, 92),
	SET_FMAN_RX_1G_LIODN(1, 5, 93),
	SET_FMAN_RX_10G_LIODN(1, 0, 94),
	SET_FMAN_RX_10G_LIODN(1, 1, 95),
};
int fman1_liodn_tbl_sz = ARRAY_SIZE(fman1_liodn_tbl);
#endif

struct liodn_id_table sec_liodn_tbl[] = {
	SET_SEC_JR_LIODN_ENTRY(0, 454, 458),
	SET_SEC_JR_LIODN_ENTRY(1, 455, 459),
	SET_SEC_JR_LIODN_ENTRY(2, 456, 460),
	SET_SEC_JR_LIODN_ENTRY(3, 457, 461),
	SET_SEC_RTIC_LIODN_ENTRY(a, 453),
	SET_SEC_RTIC_LIODN_ENTRY(b, 549),
	SET_SEC_RTIC_LIODN_ENTRY(c, 550),
	SET_SEC_RTIC_LIODN_ENTRY(d, 551),
	SET_SEC_DECO_LIODN_ENTRY(0, 541, 610),
	SET_SEC_DECO_LIODN_ENTRY(1, 542, 611),
	SET_SEC_DECO_LIODN_ENTRY(2, 543, 612),
	SET_SEC_DECO_LIODN_ENTRY(3, 544, 613),
	SET_SEC_DECO_LIODN_ENTRY(4, 545, 614),
	SET_SEC_DECO_LIODN_ENTRY(5, 546, 615),
	SET_SEC_DECO_LIODN_ENTRY(6, 547, 616),
	SET_SEC_DECO_LIODN_ENTRY(7, 548, 617),
};
int sec_liodn_tbl_sz = ARRAY_SIZE(sec_liodn_tbl);

#ifdef CONFIG_SYS_DPAA_RMAN
struct liodn_id_table rman_liodn_tbl[] = {
	/* Set RMan block 0-3 liodn offset */
	SET_RMAN_LIODN(0, 6),
	SET_RMAN_LIODN(1, 7),
	SET_RMAN_LIODN(2, 8),
	SET_RMAN_LIODN(3, 9),
};
int rman_liodn_tbl_sz = ARRAY_SIZE(rman_liodn_tbl);
#endif

struct liodn_id_table liodn_bases[] = {
#ifdef CONFIG_SYS_DPAA_DCE
	[FSL_HW_PORTAL_DCE]  = SET_LIODN_BASE_2(618, 694),
#endif
	[FSL_HW_PORTAL_SEC]  = SET_LIODN_BASE_2(462, 558),
#ifdef CONFIG_SYS_DPAA_FMAN
	[FSL_HW_PORTAL_FMAN1] = SET_LIODN_BASE_1(973),
#endif
#ifdef CONFIG_SYS_DPAA_PME
	[FSL_HW_PORTAL_PME]   = SET_LIODN_BASE_2(770, 846),
#endif
#ifdef CONFIG_SYS_DPAA_RMAN
	[FSL_HW_PORTAL_RMAN] = SET_LIODN_BASE_1(922),
#endif
};
