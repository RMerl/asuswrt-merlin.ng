// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2001 Navin Boppuri / Prashant Patel
 *	<nboppuri@trinetcommunication.com>,
 *	<pmpatel@trinetcommunication.com>
 * Copyright (c) 2001 Gerd Mennchen <Gerd.Mennchen@icn.siemens.de>
 * Copyright (c) 2001 Wolfgang Denk, DENX Software Engineering, <wd@denx.de>.
 */

/*
 * MPC8xx CPM SPI interface.
 *
 * Parts of this code are probably not portable and/or specific to
 * the board which I used for the tests. Please send fixes/complaints
 * to wd@denx.de
 *
 */

#include <common.h>
#include <dm.h>
#include <mpc8xx.h>
#include <spi.h>

#include <asm/cpm_8xx.h>
#include <asm/io.h>

#define CPM_SPI_BASE_RX	CPM_SPI_BASE
#define CPM_SPI_BASE_TX	(CPM_SPI_BASE + sizeof(cbd_t))

#define MAX_BUFFER	0x104

static int mpc8xx_spi_probe(struct udevice *dev)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t __iomem *cp = &immr->im_cpm;
	spi_t __iomem *spi = (spi_t __iomem *)&cp->cp_dparam[PROFF_SPI];
	cbd_t __iomem *tbdf, *rbdf;

	/* Disable relocation */
	out_be16(&spi->spi_rpbase, 0);

/* 1 */
	/* ------------------------------------------------
	 * Initialize Port B SPI pins -> page 34-8 MPC860UM
	 * (we are only in Master Mode !)
	 * ------------------------------------------------ */

	/* --------------------------------------------
	 * GPIO or per. Function
	 * PBPAR[28] = 1 [0x00000008] -> PERI: (SPIMISO)
	 * PBPAR[29] = 1 [0x00000004] -> PERI: (SPIMOSI)
	 * PBPAR[30] = 1 [0x00000002] -> PERI: (SPICLK)
	 * PBPAR[31] = 0 [0x00000001] -> GPIO: (CS for PCUE/CCM-EEPROM)
	 * -------------------------------------------- */
	clrsetbits_be32(&cp->cp_pbpar, 0x00000001, 0x0000000E);	/* set  bits */

	/* ----------------------------------------------
	 * In/Out or per. Function 0/1
	 * PBDIR[28] = 1 [0x00000008] -> PERI1: SPIMISO
	 * PBDIR[29] = 1 [0x00000004] -> PERI1: SPIMOSI
	 * PBDIR[30] = 1 [0x00000002] -> PERI1: SPICLK
	 * PBDIR[31] = 1 [0x00000001] -> GPIO OUT: CS for PCUE/CCM-EEPROM
	 * ---------------------------------------------- */
	setbits_be32(&cp->cp_pbdir, 0x0000000F);

	/* ----------------------------------------------
	 * open drain or active output
	 * PBODR[28] = 1 [0x00000008] -> open drain: SPIMISO
	 * PBODR[29] = 0 [0x00000004] -> active output SPIMOSI
	 * PBODR[30] = 0 [0x00000002] -> active output: SPICLK
	 * PBODR[31] = 0 [0x00000001] -> active output GPIO OUT: CS for PCUE/CCM
	 * ---------------------------------------------- */

	clrsetbits_be16(&cp->cp_pbodr, 0x00000007, 0x00000008);

	/* Initialize the parameter ram.
	 * We need to make sure many things are initialized to zero
	 */
	out_be32(&spi->spi_rstate, 0);
	out_be32(&spi->spi_rdp, 0);
	out_be16(&spi->spi_rbptr, 0);
	out_be16(&spi->spi_rbc, 0);
	out_be32(&spi->spi_rxtmp, 0);
	out_be32(&spi->spi_tstate, 0);
	out_be32(&spi->spi_tdp, 0);
	out_be16(&spi->spi_tbptr, 0);
	out_be16(&spi->spi_tbc, 0);
	out_be32(&spi->spi_txtmp, 0);

/* 3 */
	/* Set up the SPI parameters in the parameter ram */
	out_be16(&spi->spi_rbase, CPM_SPI_BASE_RX);
	out_be16(&spi->spi_tbase, CPM_SPI_BASE_TX);

	/***********IMPORTANT******************/

	/*
	 * Setting transmit and receive buffer descriptor pointers
	 * initially to rbase and tbase. Only the microcode patches
	 * documentation talks about initializing this pointer. This
	 * is missing from the sample I2C driver. If you dont
	 * initialize these pointers, the kernel hangs.
	 */
	out_be16(&spi->spi_rbptr, CPM_SPI_BASE_RX);
	out_be16(&spi->spi_tbptr, CPM_SPI_BASE_TX);

/* 4 */
	/* Init SPI Tx + Rx Parameters */
	while (in_be16(&cp->cp_cpcr) & CPM_CR_FLG)
		;

	out_be16(&cp->cp_cpcr, mk_cr_cmd(CPM_CR_CH_SPI, CPM_CR_INIT_TRX) |
			       CPM_CR_FLG);
	while (in_be16(&cp->cp_cpcr) & CPM_CR_FLG)
		;

/* 5 */
	/* Set SDMA configuration register */
	out_be32(&immr->im_siu_conf.sc_sdcr, 0x0001);

/* 6 */
	/* Set to big endian. */
	out_8(&spi->spi_tfcr, SMC_EB);
	out_8(&spi->spi_rfcr, SMC_EB);

/* 7 */
	/* Set maximum receive size. */
	out_be16(&spi->spi_mrblr, MAX_BUFFER);

/* 8 + 9 */
	/* tx and rx buffer descriptors */
	tbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_TX];
	rbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_RX];

	clrbits_be16(&tbdf->cbd_sc, BD_SC_READY);
	clrbits_be16(&rbdf->cbd_sc, BD_SC_EMPTY);

/* 10 + 11 */
	out_8(&cp->cp_spim, 0);			/* Mask  all SPI events */
	out_8(&cp->cp_spie, SPI_EMASK);		/* Clear all SPI events	*/

	return 0;
}

static int mpc8xx_spi_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t __iomem *cp = &immr->im_cpm;
	cbd_t __iomem *tbdf, *rbdf;
	int tm;
	size_t count = (bitlen + 7) / 8;

	if (count > MAX_BUFFER)
		return -EINVAL;

	tbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_TX];
	rbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_RX];

	/* Set CS for device */
	clrbits_be32(&cp->cp_pbdat, 0x0001);

	/* Setting tx bd status and data length */
	out_be32(&tbdf->cbd_bufaddr, (ulong)dout);
	out_be16(&tbdf->cbd_sc, BD_SC_READY | BD_SC_LAST | BD_SC_WRAP);
	out_be16(&tbdf->cbd_datlen, count);

	/* Setting rx bd status and data length */
	out_be32(&rbdf->cbd_bufaddr, (ulong)din);
	out_be16(&rbdf->cbd_sc, BD_SC_EMPTY | BD_SC_WRAP);
	out_be16(&rbdf->cbd_datlen, 0);	 /* rx length has no significance */

	clrsetbits_be16(&cp->cp_spmode, ~SPMODE_LOOP, SPMODE_REV | SPMODE_MSTR |
			SPMODE_EN | SPMODE_LEN(8) | SPMODE_PM(0x8));
	out_8(&cp->cp_spim, 0);		/* Mask  all SPI events */
	out_8(&cp->cp_spie, SPI_EMASK);	/* Clear all SPI events	*/

	/* start spi transfer */
	setbits_8(&cp->cp_spcom, SPI_STR);		/* Start transmit */

	/* --------------------------------
	 * Wait for SPI transmit to get out
	 * or time out (1 second = 1000 ms)
	 * -------------------------------- */
	for (tm = 0; tm < 1000; ++tm) {
		if (in_8(&cp->cp_spie) & SPI_TXB)	/* Tx Buffer Empty */
			break;
		if ((in_be16(&tbdf->cbd_sc) & BD_SC_READY) == 0)
			break;
		udelay(1000);
	}
	if (tm >= 1000)
		printf("*** spi_xfer: Time out while xferring to/from SPI!\n");

	/* Clear CS for device */
	setbits_be32(&cp->cp_pbdat, 0x0001);

	return count;
}

static const struct dm_spi_ops mpc8xx_spi_ops = {
	.xfer		= mpc8xx_spi_xfer,
};

static const struct udevice_id mpc8xx_spi_ids[] = {
	{ .compatible = "fsl,mpc8xx-spi" },
	{ }
};

U_BOOT_DRIVER(mpc8xx_spi) = {
	.name	= "mpc8xx_spi",
	.id	= UCLASS_SPI,
	.of_match = mpc8xx_spi_ids,
	.ops	= &mpc8xx_spi_ops,
	.probe	= mpc8xx_spi_probe,
};
