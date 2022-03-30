// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
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
	SET_QP_INFO(9, 35, 1, 4),
	SET_QP_INFO(10, 36, 1, 4),
	SET_QP_INFO(11, 37, 1, 5),
	SET_QP_INFO(12, 38, 1, 5),
	SET_QP_INFO(13, 39, 1, 6),
	SET_QP_INFO(14, 40, 1, 6),
	SET_QP_INFO(15, 41, 1, 7),
	SET_QP_INFO(16, 42, 1, 7),
	SET_QP_INFO(17, 43, 1, 8),
	SET_QP_INFO(18, 44, 1, 8),
	SET_QP_INFO(19, 45, 1, 9),
	SET_QP_INFO(20, 46, 1, 9),
	SET_QP_INFO(21, 47, 1, 10),
	SET_QP_INFO(22, 48, 1, 10),
	SET_QP_INFO(23, 49, 1, 11),
	SET_QP_INFO(24, 50, 1, 11),
	SET_QP_INFO(65, 89, 1, 0),
	SET_QP_INFO(66, 90, 1, 0),
	SET_QP_INFO(67, 91, 1, 1),
	SET_QP_INFO(68, 92, 1, 1),
	SET_QP_INFO(69, 93, 1, 2),
	SET_QP_INFO(70, 94, 1, 2),
	SET_QP_INFO(71, 95, 1, 3),
	SET_QP_INFO(72, 96, 1, 3),
	SET_QP_INFO(73, 97, 1, 4),
	SET_QP_INFO(74, 98, 1, 4),
	SET_QP_INFO(75, 99, 1, 5),
	SET_QP_INFO(76, 100, 1, 5),
	SET_QP_INFO(77, 101, 1, 6),
	SET_QP_INFO(78, 102, 1, 6),
	SET_QP_INFO(79, 103, 1, 7),
	SET_QP_INFO(80, 104, 1, 7),
	SET_QP_INFO(81, 105, 1, 8),
	SET_QP_INFO(82, 106, 1, 8),
	SET_QP_INFO(83, 107, 1, 9),
	SET_QP_INFO(84, 108, 1, 9),
	SET_QP_INFO(85, 109, 1, 10),
	SET_QP_INFO(86, 110, 1, 10),
	SET_QP_INFO(87, 111, 1, 11),
	SET_QP_INFO(88, 112, 1, 11),
	SET_QP_INFO(25, 51, 1, 0),
	SET_QP_INFO(26, 52, 1, 0),
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

	SET_SATA_LIODN(1, 555),
	SET_SATA_LIODN(2, 556),

	SET_PCI_LIODN_BASE(CONFIG_SYS_FSL_PCIE_COMPAT, 1, 148),
	SET_PCI_LIODN_BASE(CONFIG_SYS_FSL_PCIE_COMPAT, 2, 228),
	SET_PCI_LIODN_BASE(CONFIG_SYS_FSL_PCIE_COMPAT, 3, 308),
	SET_PCI_LIODN_BASE(CONFIG_SYS_FSL_PCIE_COMPAT, 4, 388),

	SET_DMA_LIODN(1, "fsl,elo3-dma", 147),
	SET_DMA_LIODN(2, "fsl,elo3-dma", 227),

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
#if (CONFIG_SYS_NUM_FMAN == 2)
struct fman_liodn_id_table fman2_liodn_tbl[] = {
	SET_FMAN_RX_1G_LIODN(2, 0, 88),
	SET_FMAN_RX_1G_LIODN(2, 1, 89),
	SET_FMAN_RX_1G_LIODN(2, 2, 90),
	SET_FMAN_RX_1G_LIODN(2, 3, 91),
	SET_FMAN_RX_1G_LIODN(2, 4, 92),
	SET_FMAN_RX_1G_LIODN(2, 5, 93),
	SET_FMAN_RX_10G_LIODN(2, 0, 94),
	SET_FMAN_RX_10G_LIODN(2, 1, 95),
};
int fman2_liodn_tbl_sz = ARRAY_SIZE(fman2_liodn_tbl);
#endif
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
#if (CONFIG_SYS_NUM_FMAN == 2)
	[FSL_HW_PORTAL_FMAN2] = SET_LIODN_BASE_1(1069),
#endif
#endif
#ifdef CONFIG_SYS_DPAA_PME
	[FSL_HW_PORTAL_PME]   = SET_LIODN_BASE_2(770, 846),
#endif
#ifdef CONFIG_SYS_DPAA_RMAN
	[FSL_HW_PORTAL_RMAN] = SET_LIODN_BASE_1(922),
#endif
};
