/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * am3517evm.h - Header file for the AM3517 EVM.
 *
 * Author: Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * Based on ti/evm/evm.h
 *
 * Copyright (C) 2010
 * Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef _AM3517EVM_H_
#define _AM3517EVM_H_

const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"AM3517EVM Board",
	"NAND",
};

/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */
#define MUX_AM3517EVM() \
	/* SDRC */\
	MUX_VAL(CP(SDRC_D0),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D1),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D2),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D3),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D4),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D5),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D6),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D7),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D8),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D9),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D10),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D11),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D12),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D13),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D14),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D15),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D16),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D17),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D18),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D19),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D20),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D21),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D22),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D23),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D24),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D25),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D26),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D27),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D28),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D29),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D30),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D31),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_CLK),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS0),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS1),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS2),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS3),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS0N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS1N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS2N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS3N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_CKE0),		(M0)) \
	MUX_VAL(CP(SDRC_CKE1),		(M0)) \
	/*sdrc_strben_dly0*/\
	MUX_VAL(CP(STRBEN_DLY0),	(IEN  | PTD | EN  | M0)) \
	 /*sdrc_strben_dly1*/\
	MUX_VAL(CP(STRBEN_DLY1),	(IEN  | PTD | EN  | M0)) \
	/* GPMC */\
	MUX_VAL(CP(GPMC_A1),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A2),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A3),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A4),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A5),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A6),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A7),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A8),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A9),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A10),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D0),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D1),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D2),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D3),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D4),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D5),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D6),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D7),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D8),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D9),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D10),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D11),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D12),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D13),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D14),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D15),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS0),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS1),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS2),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS3),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS4),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS5),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS6),		(IDIS | PTD | DIS | M4)) \
	MUX_VAL(CP(GPMC_NCS7),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_CLK),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NADV_ALE),	(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_NOE),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_NWE),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_NBE0_CLE),	(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NBE1),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NWP),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_WAIT0),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_WAIT1),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_WAIT3),		(IEN  | PTU | EN  | M0)) \
	/* MMC */\
	MUX_VAL(CP(MMC1_CLK),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(MMC1_CMD),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_DAT0),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_DAT1),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_DAT2),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_DAT3),		(IEN  | PTU | DIS | M0)) \
	/* UART */\
	MUX_VAL(CP(UART3_CTS_RCTX),	(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(UART3_RTS_SD),	(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(UART3_RX_IRRX),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(UART3_TX_IRTX),	(IDIS | PTD | DIS | M0)) \
	/* Control and debug */\
	MUX_VAL(CP(SYS_32K),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_CLKREQ),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_NIRQ),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(SYS_BOOT7),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SYS_BOOT8),		(IEN  | PTD | EN  | M0)) \
	\
	MUX_VAL(CP(SYS_OFF_MODE),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_CLKOUT1),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_CLKOUT2),	(IEN  | PTU | EN  | M0)) \
	/* JTAG */\
	MUX_VAL(CP(JTAG_NTRST),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TCK),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TMS),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TDI),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_EMU0),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_EMU1),		(IEN  | PTD | DIS | M0)) \
	/* ETK (ES2 onwards) */\
	MUX_VAL(CP(ETK_D10_ES2),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(ETK_D11_ES2),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(ETK_D12_ES2),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(ETK_D13_ES2),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(ETK_D14_ES2),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(ETK_D15_ES2),	(IEN  | PTD | DIS | M0)) \
	/* Die to Die */\
	MUX_VAL(CP(D2D_MCAD34),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(D2D_MCAD35),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(D2D_MCAD36),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(D2D_CLK26MI),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_NRESPWRON),	(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(D2D_NRESWARM),	(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(D2D_ARM9NIRQ),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_UMA2P6FIQ),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_SPINT),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(D2D_FRINT),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(D2D_DMAREQ0),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_DMAREQ1),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_DMAREQ2),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_DMAREQ3),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_N3GTRST),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_N3GTDI),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_N3GTDO),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_N3GTMS),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_N3GTCK),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_N3GRTCK),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_MSTDBY),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(D2D_SWAKEUP),	(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(D2D_IDLEREQ),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_IDLEACK),	(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(D2D_MWRITE),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_SWRITE),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_MREAD),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_SREAD),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_MBUSFLAG),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(D2D_SBUSFLAG),	(IEN  | PTD | DIS | M0)) \

#endif
