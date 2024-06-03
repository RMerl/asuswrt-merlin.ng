/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Freescale DMA Controller
 *
 * Copyright 2006 Freescale Semiconductor, Inc.
 */

#ifndef _ASM_FSL_DMA_H_
#define _ASM_FSL_DMA_H_

#include <asm/types.h>

#ifdef CONFIG_MPC83xx
typedef struct fsl_dma {
	uint	mr;		/* DMA mode register */
#define FSL_DMA_MR_CS		0x00000001	/* Channel start */
#define FSL_DMA_MR_CC		0x00000002	/* Channel continue */
#define FSL_DMA_MR_CTM		0x00000004	/* Channel xfer mode */
#define FSL_DMA_MR_CTM_DIRECT	0x00000004	/* Direct channel xfer mode */
#define FSL_DMA_MR_EOTIE	0x00000080	/* End-of-transfer interrupt en */
#define FSL_DMA_MR_PRC_MASK	0x00000c00	/* PCI read command */
#define FSL_DMA_MR_SAHE		0x00001000	/* Source addr hold enable */
#define FSL_DMA_MR_DAHE		0x00002000	/* Dest addr hold enable */
#define FSL_DMA_MR_SAHTS_MASK	0x0000c000	/* Source addr hold xfer size */
#define FSL_DMA_MR_DAHTS_MASK	0x00030000	/* Dest addr hold xfer size */
#define FSL_DMA_MR_EMS_EN	0x00040000	/* Ext master start en */
#define FSL_DMA_MR_IRQS		0x00080000	/* Interrupt steer */
#define FSL_DMA_MR_DMSEN	0x00100000	/* Direct mode snooping en */
#define FSL_DMA_MR_BWC_MASK	0x00e00000	/* Bandwidth/pause ctl */
#define FSL_DMA_MR_DRCNT	0x0f000000	/* DMA request count */
	uint	sr;		/* DMA status register */
#define FSL_DMA_SR_EOCDI	0x00000001	/* End-of-chain/direct interrupt */
#define FSL_DMA_SR_EOSI		0x00000002	/* End-of-segment interrupt */
#define FSL_DMA_SR_CB		0x00000004	/* Channel busy */
#define FSL_DMA_SR_TE		0x00000080	/* Transfer error */
	uint	cdar;		/* DMA current descriptor address register */
	char	res0[4];
	uint	sar;		/* DMA source address register */
	char	res1[4];
	uint	dar;		/* DMA destination address register */
	char	res2[4];
	uint	bcr;		/* DMA byte count register */
	uint	ndar;		/* DMA next descriptor address register */
	uint	gsr;		/* DMA general status register (DMA3 ONLY!) */
	char	res3[84];
} fsl_dma_t;
#else
typedef struct fsl_dma {
	uint	mr;		/* DMA mode register */
#define FSL_DMA_MR_CS		0x00000001	/* Channel start */
#define FSL_DMA_MR_CC		0x00000002	/* Channel continue */
#define FSL_DMA_MR_CTM		0x00000004	/* Channel xfer mode */
#define FSL_DMA_MR_CTM_DIRECT	0x00000004	/* Direct channel xfer mode */
#define FSL_DMA_MR_CA		0x00000008	/* Channel abort */
#define FSL_DMA_MR_CDSM		0x00000010
#define FSL_DMA_MR_XFE		0x00000020	/* Extended features en */
#define FSL_DMA_MR_EIE		0x00000040	/* Error interrupt en */
#define FSL_DMA_MR_EOLSIE	0x00000080	/* End-of-lists interrupt en */
#define FSL_DMA_MR_EOLNIE	0x00000100	/* End-of-links interrupt en */
#define FSL_DMA_MR_EOSIE	0x00000200	/* End-of-seg interrupt en */
#define FSL_DMA_MR_SRW		0x00000400	/* Single register write */
#define FSL_DMA_MR_SAHE		0x00001000	/* Source addr hold enable */
#define FSL_DMA_MR_DAHE		0x00002000	/* Dest addr hold enable */
#define FSL_DMA_MR_SAHTS_MASK	0x0000c000	/* Source addr hold xfer size */
#define FSL_DMA_MR_DAHTS_MASK	0x00030000	/* Dest addr hold xfer size */
#define FSL_DMA_MR_EMS_EN	0x00040000	/* Ext master start en */
#define FSL_DMA_MR_EMP_EN	0x00200000	/* Ext master pause en */
#define FSL_DMA_MR_BWC_MASK	0x0f000000	/* Bandwidth/pause ctl */
#define FSL_DMA_MR_BWC_DIS	0x0f000000	/* Bandwidth/pause ctl disable */
	uint	sr;		/* DMA status register */
#define FSL_DMA_SR_EOLSI	0x00000001	/* End-of-list interrupt */
#define FSL_DMA_SR_EOSI		0x00000002	/* End-of-segment interrupt */
#define FSL_DMA_SR_CB		0x00000004	/* Channel busy */
#define FSL_DMA_SR_EOLNI	0x00000008	/* End-of-links interrupt */
#define FSL_DMA_SR_PE		0x00000010	/* Programming error */
#define FSL_DMA_SR_CH		0x00000020	/* Channel halted */
#define FSL_DMA_SR_TE		0x00000080	/* Transfer error */
	char	res0[4];
	uint	clndar;		/* DMA current link descriptor address register */
	uint	satr;		/* DMA source attributes register */
#define FSL_DMA_SATR_ESAD_MASK		0x000001ff	/* Extended source addr */
#define FSL_DMA_SATR_SREAD_NO_SNOOP	0x00040000	/* Read, don't snoop */
#define FSL_DMA_SATR_SREAD_SNOOP	0x00050000	/* Read, snoop */
#define FSL_DMA_SATR_SREAD_UNLOCK	0x00070000	/* Read, unlock l2 */
#define FSL_DMA_SATR_STRAN_MASK		0x00f00000	/* Source interface  */
#define FSL_DMA_SATR_SSME		0x01000000	/* Source stride en */
#define FSL_DMA_SATR_SPCIORDER		0x02000000	/* PCI transaction order */
#define FSL_DMA_SATR_STFLOWLVL_MASK	0x0c000000	/* RIO flow level */
#define FSL_DMA_SATR_SBPATRMU		0x20000000	/* Bypass ATMU */
	uint	sar;		/* DMA source address register */
	uint	datr;		/* DMA destination attributes register */
#define FSL_DMA_DATR_EDAD_MASK		0x000001ff	/* Extended dest addr */
#define FSL_DMA_DATR_DWRITE_NO_SNOOP	0x00040000	/* Write, don't snoop */
#define FSL_DMA_DATR_DWRITE_SNOOP	0x00050000	/* Write, snoop */
#define FSL_DMA_DATR_DWRITE_ALLOC	0x00060000	/* Write, alloc l2 */
#define FSL_DMA_DATR_DWRITE_LOCK	0x00070000	/* Write, lock l2 */
#define FSL_DMA_DATR_DTRAN_MASK		0x00f00000	/* Dest interface  */
#define FSL_DMA_DATR_DSME		0x01000000	/* Dest stride en */
#define FSL_DMA_DATR_DPCIORDER		0x02000000	/* PCI transaction order */
#define FSL_DMA_DATR_DTFLOWLVL_MASK	0x0c000000	/* RIO flow level */
#define FSL_DMA_DATR_DBPATRMU		0x20000000	/* Bypass ATMU */
	uint	dar;		/* DMA destination address register */
	uint	bcr;		/* DMA byte count register */
	char	res1[4];
	uint	nlndar;		/* DMA next link descriptor address register */
	char	res2[8];
	uint	clabdar;	/* DMA current List - alternate base descriptor address Register */
	char	res3[4];
	uint	nlsdar;		/* DMA next list descriptor address register */
	uint	ssr;		/* DMA source stride register */
	uint	dsr;		/* DMA destination stride register */
	char	res4[56];
} fsl_dma_t;
#endif /* !CONFIG_MPC83xx */

#ifdef CONFIG_FSL_DMA
void dma_init(void);
int dmacpy(phys_addr_t dest, phys_addr_t src, phys_size_t n);
#if (defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER))
void dma_meminit(uint val, uint size);
#endif
#endif

#endif	/* _ASM_DMA_H_ */
