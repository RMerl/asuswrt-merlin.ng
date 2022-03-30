// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *
 * Dave Liu <daveliu@freescale.com>
 * based on source code of Shlomi Gridish
 */

#include <common.h>
#include <malloc.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <linux/immap_qe.h>
#include "uccf.h"
#include <fsl_qe.h>

void ucc_fast_transmit_on_demand(ucc_fast_private_t *uccf)
{
	out_be16(&uccf->uf_regs->utodr, UCC_FAST_TOD);
}

u32 ucc_fast_get_qe_cr_subblock(int ucc_num)
{
	switch (ucc_num) {
		case 0:	return QE_CR_SUBBLOCK_UCCFAST1;
		case 1:	return QE_CR_SUBBLOCK_UCCFAST2;
		case 2:	return QE_CR_SUBBLOCK_UCCFAST3;
		case 3:	return QE_CR_SUBBLOCK_UCCFAST4;
		case 4:	return QE_CR_SUBBLOCK_UCCFAST5;
		case 5:	return QE_CR_SUBBLOCK_UCCFAST6;
		case 6:	return QE_CR_SUBBLOCK_UCCFAST7;
		case 7:	return QE_CR_SUBBLOCK_UCCFAST8;
		default:	return QE_CR_SUBBLOCK_INVALID;
	}
}

static void ucc_get_cmxucr_reg(int ucc_num, volatile u32 **p_cmxucr,
				 u8 *reg_num, u8 *shift)
{
	switch (ucc_num) {
		case 0:	/* UCC1 */
			*p_cmxucr  = &(qe_immr->qmx.cmxucr1);
			*reg_num = 1;
			*shift  = 16;
			break;
		case 2:	/* UCC3 */
			*p_cmxucr  = &(qe_immr->qmx.cmxucr1);
			*reg_num = 1;
			*shift  = 0;
			break;
		case 4:	/* UCC5 */
			*p_cmxucr  = &(qe_immr->qmx.cmxucr2);
			*reg_num = 2;
			*shift  = 16;
			break;
		case 6:	/* UCC7 */
			*p_cmxucr  = &(qe_immr->qmx.cmxucr2);
			*reg_num = 2;
			*shift  = 0;
			break;
		case 1:	/* UCC2 */
			*p_cmxucr  = &(qe_immr->qmx.cmxucr3);
			*reg_num = 3;
			*shift  = 16;
			break;
		case 3:	/* UCC4 */
			*p_cmxucr  = &(qe_immr->qmx.cmxucr3);
			*reg_num = 3;
			*shift  = 0;
			break;
		case 5:	/* UCC6 */
			*p_cmxucr  = &(qe_immr->qmx.cmxucr4);
			*reg_num = 4;
			*shift  = 16;
			break;
		case 7:	/* UCC8 */
			*p_cmxucr  = &(qe_immr->qmx.cmxucr4);
			*reg_num = 4;
			*shift  = 0;
			break;
		default:
			break;
	}
}

static int ucc_set_clk_src(int ucc_num, qe_clock_e clock, comm_dir_e mode)
{
	volatile u32	*p_cmxucr = NULL;
	u8		reg_num = 0;
	u8		shift = 0;
	u32		clockBits;
	u32		clockMask;
	int		source = -1;

	/* check if the UCC number is in range. */
	if ((ucc_num > UCC_MAX_NUM - 1) || (ucc_num < 0))
		return -EINVAL;

	if (! ((mode == COMM_DIR_RX) || (mode == COMM_DIR_TX))) {
		printf("%s: bad comm mode type passed\n", __FUNCTION__);
		return -EINVAL;
	}

	ucc_get_cmxucr_reg(ucc_num, &p_cmxucr, &reg_num, &shift);

	switch (reg_num) {
		case 1:
			switch (clock) {
				case QE_BRG1:	source = 1; break;
				case QE_BRG2:	source = 2; break;
				case QE_BRG7:	source = 3; break;
				case QE_BRG8:	source = 4; break;
				case QE_CLK9:	source = 5; break;
				case QE_CLK10:	source = 6; break;
				case QE_CLK11:	source = 7; break;
				case QE_CLK12:	source = 8; break;
				case QE_CLK15:	source = 9; break;
				case QE_CLK16:	source = 10; break;
				default:	source = -1; break;
			}
			break;
		case 2:
			switch (clock) {
				case QE_BRG5:	source = 1; break;
				case QE_BRG6:	source = 2; break;
				case QE_BRG7:	source = 3; break;
				case QE_BRG8:	source = 4; break;
				case QE_CLK13:	source = 5; break;
				case QE_CLK14:	source = 6; break;
				case QE_CLK19:	source = 7; break;
				case QE_CLK20:	source = 8; break;
				case QE_CLK15:	source = 9; break;
				case QE_CLK16:	source = 10; break;
				default:	source = -1; break;
			}
			break;
		case 3:
			switch (clock) {
				case QE_BRG9:	source = 1; break;
				case QE_BRG10:	source = 2; break;
				case QE_BRG15:	source = 3; break;
				case QE_BRG16:	source = 4; break;
				case QE_CLK3:	source = 5; break;
				case QE_CLK4:	source = 6; break;
				case QE_CLK17:	source = 7; break;
				case QE_CLK18:	source = 8; break;
				case QE_CLK7:	source = 9; break;
				case QE_CLK8:	source = 10; break;
				case QE_CLK16:	source = 11; break;
				default:	source = -1; break;
			}
			break;
		case 4:
			switch (clock) {
				case QE_BRG13:	source = 1; break;
				case QE_BRG14:	source = 2; break;
				case QE_BRG15:	source = 3; break;
				case QE_BRG16:	source = 4; break;
				case QE_CLK5:	source = 5; break;
				case QE_CLK6:	source = 6; break;
				case QE_CLK21:	source = 7; break;
				case QE_CLK22:	source = 8; break;
				case QE_CLK7:	source = 9; break;
				case QE_CLK8:	source = 10; break;
				case QE_CLK16:	source = 11; break;
				default:	source = -1; break;
			}
			break;
		default:
			source = -1;
			break;
	}

	if (source == -1) {
		printf("%s: Bad combination of clock and UCC\n", __FUNCTION__);
		return -ENOENT;
	}

	clockBits = (u32) source;
	clockMask = QE_CMXUCR_TX_CLK_SRC_MASK;
	if (mode == COMM_DIR_RX) {
		clockBits <<= 4; /* Rx field is 4 bits to left of Tx field */
		clockMask <<= 4; /* Rx field is 4 bits to left of Tx field */
	}
	clockBits <<= shift;
	clockMask <<= shift;

	out_be32(p_cmxucr, (in_be32(p_cmxucr) & ~clockMask) | clockBits);

	return 0;
}

static uint ucc_get_reg_baseaddr(int ucc_num)
{
	uint base = 0;

	/* check if the UCC number is in range */
	if ((ucc_num > UCC_MAX_NUM - 1) || (ucc_num < 0)) {
		printf("%s: the UCC num not in ranges\n", __FUNCTION__);
		return 0;
	}

	switch (ucc_num) {
		case 0:	base = 0x00002000; break;
		case 1:	base = 0x00003000; break;
		case 2:	base = 0x00002200; break;
		case 3:	base = 0x00003200; break;
		case 4:	base = 0x00002400; break;
		case 5:	base = 0x00003400; break;
		case 6:	base = 0x00002600; break;
		case 7:	base = 0x00003600; break;
		default: break;
	}

	base = (uint)qe_immr + base;
	return base;
}

void ucc_fast_enable(ucc_fast_private_t *uccf, comm_dir_e mode)
{
	ucc_fast_t	*uf_regs;
	u32		gumr;

	uf_regs = uccf->uf_regs;

	/* Enable reception and/or transmission on this UCC. */
	gumr = in_be32(&uf_regs->gumr);
	if (mode & COMM_DIR_TX) {
		gumr |= UCC_FAST_GUMR_ENT;
		uccf->enabled_tx = 1;
	}
	if (mode & COMM_DIR_RX) {
		gumr |= UCC_FAST_GUMR_ENR;
		uccf->enabled_rx = 1;
	}
	out_be32(&uf_regs->gumr, gumr);
}

void ucc_fast_disable(ucc_fast_private_t *uccf, comm_dir_e mode)
{
	ucc_fast_t	*uf_regs;
	u32		gumr;

	uf_regs = uccf->uf_regs;

	/* Disable reception and/or transmission on this UCC. */
	gumr = in_be32(&uf_regs->gumr);
	if (mode & COMM_DIR_TX) {
		gumr &= ~UCC_FAST_GUMR_ENT;
		uccf->enabled_tx = 0;
	}
	if (mode & COMM_DIR_RX) {
		gumr &= ~UCC_FAST_GUMR_ENR;
		uccf->enabled_rx = 0;
	}
	out_be32(&uf_regs->gumr, gumr);
}

int ucc_fast_init(ucc_fast_info_t *uf_info, ucc_fast_private_t  **uccf_ret)
{
	ucc_fast_private_t	*uccf;
	ucc_fast_t		*uf_regs;

	if (!uf_info)
		return -EINVAL;

	if ((uf_info->ucc_num < 0) || (uf_info->ucc_num > UCC_MAX_NUM - 1)) {
		printf("%s: Illagal UCC number!\n", __FUNCTION__);
		return -EINVAL;
	}

	uccf = (ucc_fast_private_t *)malloc(sizeof(ucc_fast_private_t));
	if (!uccf) {
		printf("%s: No memory for UCC fast data structure!\n",
			 __FUNCTION__);
		return -ENOMEM;
	}
	memset(uccf, 0, sizeof(ucc_fast_private_t));

	/* Save fast UCC structure */
	uccf->uf_info	= uf_info;
	uccf->uf_regs	= (ucc_fast_t *)ucc_get_reg_baseaddr(uf_info->ucc_num);

	if (uccf->uf_regs == NULL) {
		printf("%s: No memory map for UCC fast controller!\n",
			 __FUNCTION__);
		return -ENOMEM;
	}

	uccf->enabled_tx	= 0;
	uccf->enabled_rx	= 0;

	uf_regs			= uccf->uf_regs;
	uccf->p_ucce		= (u32 *) &(uf_regs->ucce);
	uccf->p_uccm		= (u32 *) &(uf_regs->uccm);

	/* Init GUEMR register, UCC both Rx and Tx is Fast protocol */
	out_8(&uf_regs->guemr, UCC_GUEMR_SET_RESERVED3 | UCC_GUEMR_MODE_FAST_RX
				 | UCC_GUEMR_MODE_FAST_TX);

	/* Set GUMR, disable UCC both Rx and Tx, Ethernet protocol */
	out_be32(&uf_regs->gumr, UCC_FAST_GUMR_ETH);

	/* Set the Giga ethernet VFIFO stuff */
	if (uf_info->eth_type == GIGA_ETH) {
		/* Allocate memory for Tx Virtual Fifo */
		uccf->ucc_fast_tx_virtual_fifo_base_offset =
		qe_muram_alloc(UCC_GETH_UTFS_GIGA_INIT,
				 UCC_FAST_VIRT_FIFO_REGS_ALIGNMENT);

		/* Allocate memory for Rx Virtual Fifo */
		uccf->ucc_fast_rx_virtual_fifo_base_offset =
		qe_muram_alloc(UCC_GETH_URFS_GIGA_INIT +
				 UCC_FAST_RX_VIRTUAL_FIFO_SIZE_PAD,
				UCC_FAST_VIRT_FIFO_REGS_ALIGNMENT);

		/* utfb, urfb are offsets from MURAM base */
		out_be32(&uf_regs->utfb,
			 uccf->ucc_fast_tx_virtual_fifo_base_offset);
		out_be32(&uf_regs->urfb,
			 uccf->ucc_fast_rx_virtual_fifo_base_offset);

		/* Set Virtual Fifo registers */
		out_be16(&uf_regs->urfs, UCC_GETH_URFS_GIGA_INIT);
		out_be16(&uf_regs->urfet, UCC_GETH_URFET_GIGA_INIT);
		out_be16(&uf_regs->urfset, UCC_GETH_URFSET_GIGA_INIT);
		out_be16(&uf_regs->utfs, UCC_GETH_UTFS_GIGA_INIT);
		out_be16(&uf_regs->utfet, UCC_GETH_UTFET_GIGA_INIT);
		out_be16(&uf_regs->utftt, UCC_GETH_UTFTT_GIGA_INIT);
	}

	/* Set the Fast ethernet VFIFO stuff */
	if (uf_info->eth_type == FAST_ETH) {
		/* Allocate memory for Tx Virtual Fifo */
		uccf->ucc_fast_tx_virtual_fifo_base_offset =
		qe_muram_alloc(UCC_GETH_UTFS_INIT,
				 UCC_FAST_VIRT_FIFO_REGS_ALIGNMENT);

		/* Allocate memory for Rx Virtual Fifo */
		uccf->ucc_fast_rx_virtual_fifo_base_offset =
		qe_muram_alloc(UCC_GETH_URFS_INIT +
				 UCC_FAST_RX_VIRTUAL_FIFO_SIZE_PAD,
				UCC_FAST_VIRT_FIFO_REGS_ALIGNMENT);

		/* utfb, urfb are offsets from MURAM base */
		out_be32(&uf_regs->utfb,
			 uccf->ucc_fast_tx_virtual_fifo_base_offset);
		out_be32(&uf_regs->urfb,
			 uccf->ucc_fast_rx_virtual_fifo_base_offset);

		/* Set Virtual Fifo registers */
		out_be16(&uf_regs->urfs, UCC_GETH_URFS_INIT);
		out_be16(&uf_regs->urfet, UCC_GETH_URFET_INIT);
		out_be16(&uf_regs->urfset, UCC_GETH_URFSET_INIT);
		out_be16(&uf_regs->utfs, UCC_GETH_UTFS_INIT);
		out_be16(&uf_regs->utfet, UCC_GETH_UTFET_INIT);
		out_be16(&uf_regs->utftt, UCC_GETH_UTFTT_INIT);
	}

	/* Rx clock routing */
	if (uf_info->rx_clock != QE_CLK_NONE) {
		if (ucc_set_clk_src(uf_info->ucc_num,
			 uf_info->rx_clock, COMM_DIR_RX)) {
			printf("%s: Illegal value for parameter 'RxClock'.\n",
				 __FUNCTION__);
			return -EINVAL;
		}
	}

	/* Tx clock routing */
	if (uf_info->tx_clock != QE_CLK_NONE) {
		if (ucc_set_clk_src(uf_info->ucc_num,
			 uf_info->tx_clock, COMM_DIR_TX)) {
			printf("%s: Illegal value for parameter 'TxClock'.\n",
				 __FUNCTION__);
			return -EINVAL;
		}
	}

	/* Clear interrupt mask register to disable all of interrupts */
	out_be32(&uf_regs->uccm, 0x0);

	/* Writing '1' to clear all of envents */
	out_be32(&uf_regs->ucce, 0xffffffff);

	*uccf_ret = uccf;
	return 0;
}
