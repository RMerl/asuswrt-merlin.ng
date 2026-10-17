/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Logic Product Development <www.logicpd.com>
 *
 * Author:
 * Peter Barada <peter.barada@logicpd.com>
 */
#ifndef _OMAP3LOGIC_H_
#define _OMAP3LOGIC_H_

/*
 * OMAP3 GPMC register settings for CS1 LAN922x
 */
#define NET_LAN92XX_GPMC_CONFIG1	0x00001000
#define NET_LAN92XX_GPMC_CONFIG2	0x00080801
#define NET_LAN92XX_GPMC_CONFIG3	0x00000000
#define NET_LAN92XX_GPMC_CONFIG4	0x08010801
#define NET_LAN92XX_GPMC_CONFIG5	0x00080a0a
#define NET_LAN92XX_GPMC_CONFIG6	0x03000280


const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"Logic DM37x/OMAP35x reference board",
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

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_VAL(CP(SDRC_D0), (IEN  | PTD | DIS | M0)) /*SDRC_D0*/
	MUX_VAL(CP(SDRC_D1), (IEN  | PTD | DIS | M0)) /*SDRC_D1*/
	MUX_VAL(CP(SDRC_D2), (IEN  | PTD | DIS | M0)) /*SDRC_D2*/
	MUX_VAL(CP(SDRC_D3), (IEN  | PTD | DIS | M0)) /*SDRC_D3*/
	MUX_VAL(CP(SDRC_D4), (IEN  | PTD | DIS | M0)) /*SDRC_D4*/
	MUX_VAL(CP(SDRC_D5), (IEN  | PTD | DIS | M0)) /*SDRC_D5*/
	MUX_VAL(CP(SDRC_D6), (IEN  | PTD | DIS | M0)) /*SDRC_D6*/
	MUX_VAL(CP(SDRC_D7), (IEN  | PTD | DIS | M0)) /*SDRC_D7*/
	MUX_VAL(CP(SDRC_D8), (IEN  | PTD | DIS | M0)) /*SDRC_D8*/
	MUX_VAL(CP(SDRC_D9), (IEN  | PTD | DIS | M0)) /*SDRC_D9*/
	MUX_VAL(CP(SDRC_D10), (IEN  | PTD | DIS | M0)) /*SDRC_D10*/
	MUX_VAL(CP(SDRC_D11), (IEN  | PTD | DIS | M0)) /*SDRC_D11*/
	MUX_VAL(CP(SDRC_D12), (IEN  | PTD | DIS | M0)) /*SDRC_D12*/
	MUX_VAL(CP(SDRC_D13), (IEN  | PTD | DIS | M0)) /*SDRC_D13*/
	MUX_VAL(CP(SDRC_D14), (IEN  | PTD | DIS | M0)) /*SDRC_D14*/
	MUX_VAL(CP(SDRC_D15), (IEN  | PTD | DIS | M0)) /*SDRC_D15*/
	MUX_VAL(CP(SDRC_D16), (IEN  | PTD | DIS | M0)) /*SDRC_D16*/
	MUX_VAL(CP(SDRC_D17), (IEN  | PTD | DIS | M0)) /*SDRC_D17*/
	MUX_VAL(CP(SDRC_D18), (IEN  | PTD | DIS | M0)) /*SDRC_D18*/
	MUX_VAL(CP(SDRC_D19), (IEN  | PTD | DIS | M0)) /*SDRC_D19*/
	MUX_VAL(CP(SDRC_D20), (IEN  | PTD | DIS | M0)) /*SDRC_D20*/
	MUX_VAL(CP(SDRC_D21), (IEN  | PTD | DIS | M0)) /*SDRC_D21*/
	MUX_VAL(CP(SDRC_D22), (IEN  | PTD | DIS | M0)) /*SDRC_D22*/
	MUX_VAL(CP(SDRC_D23), (IEN  | PTD | DIS | M0)) /*SDRC_D23*/
	MUX_VAL(CP(SDRC_D24), (IEN  | PTD | DIS | M0)) /*SDRC_D24*/
	MUX_VAL(CP(SDRC_D25), (IEN  | PTD | DIS | M0)) /*SDRC_D25*/
	MUX_VAL(CP(SDRC_D26), (IEN  | PTD | DIS | M0)) /*SDRC_D26*/
	MUX_VAL(CP(SDRC_D27), (IEN  | PTD | DIS | M0)) /*SDRC_D27*/
	MUX_VAL(CP(SDRC_D28), (IEN  | PTD | DIS | M0)) /*SDRC_D28*/
	MUX_VAL(CP(SDRC_D29), (IEN  | PTD | DIS | M0)) /*SDRC_D29*/
	MUX_VAL(CP(SDRC_D30), (IEN  | PTD | DIS | M0)) /*SDRC_D30*/
	MUX_VAL(CP(SDRC_D31), (IEN  | PTD | DIS | M0)) /*SDRC_D31*/
	MUX_VAL(CP(SDRC_CLK), (IEN  | PTD | DIS | M0)) /*SDRC_CLK*/
	MUX_VAL(CP(SDRC_DQS0), (IEN  | PTD | DIS | M0)) /*SDRC_DQS0*/
	MUX_VAL(CP(SDRC_DQS1), (IEN  | PTD | DIS | M0)) /*SDRC_DQS1*/
	MUX_VAL(CP(SDRC_DQS2), (IEN  | PTD | DIS | M0)) /*SDRC_DQS2*/
	MUX_VAL(CP(SDRC_DQS3), (IEN  | PTD | DIS | M0)) /*SDRC_DQS3*/
	MUX_VAL(CP(SDRC_CKE0), (IDIS | PTU | EN  | M0)) /*SDRC_CKE0*/
	MUX_VAL(CP(SDRC_CKE1), (IDIS | PTU | DIS | M0)) /*SDRC_CKE1*/

	MUX_VAL(CP(GPMC_A1), (IDIS | PTU | EN  | M0)) /*GPMC_A1*/
	MUX_VAL(CP(GPMC_A2), (IDIS | PTU | EN  | M0)) /*GPMC_A2*/
	MUX_VAL(CP(GPMC_A3), (IDIS | PTU | EN  | M0)) /*GPMC_A3*/
	MUX_VAL(CP(GPMC_A4), (IDIS | PTU | EN  | M0)) /*GPMC_A4*/
	MUX_VAL(CP(GPMC_A5), (IDIS | PTU | EN  | M0)) /*GPMC_A5*/
	MUX_VAL(CP(GPMC_A6), (IDIS | PTU | EN  | M0)) /*GPMC_A6*/
	MUX_VAL(CP(GPMC_A7), (IDIS | PTU | EN  | M0)) /*GPMC_A7*/
	MUX_VAL(CP(GPMC_A8), (IDIS | PTU | EN  | M0)) /*GPMC_A8*/
	MUX_VAL(CP(GPMC_A9), (IDIS | PTU | EN  | M0)) /*GPMC_A9*/
	MUX_VAL(CP(GPMC_A10), (IDIS | PTU | EN  | M0)) /*GPMC_A10*/
	MUX_VAL(CP(GPMC_D0), (IEN  | PTU | EN  | M0)) /*GPMC_D0*/
	MUX_VAL(CP(GPMC_D1), (IEN  | PTU | EN  | M0)) /*GPMC_D1*/
	MUX_VAL(CP(GPMC_D2), (IEN  | PTU | EN  | M0)) /*GPMC_D2*/
	MUX_VAL(CP(GPMC_D3), (IEN  | PTU | EN  | M0)) /*GPMC_D3*/
	MUX_VAL(CP(GPMC_D4), (IEN  | PTU | EN  | M0)) /*GPMC_D4*/
	MUX_VAL(CP(GPMC_D5), (IEN  | PTU | EN  | M0)) /*GPMC_D5*/
	MUX_VAL(CP(GPMC_D6), (IEN  | PTU | EN  | M0)) /*GPMC_D6*/
	MUX_VAL(CP(GPMC_D7), (IEN  | PTU | EN  | M0)) /*GPMC_D7*/
	MUX_VAL(CP(GPMC_D8), (IEN  | PTU | EN  | M0)) /*GPMC_D8*/
	MUX_VAL(CP(GPMC_D9), (IEN  | PTU | EN  | M0)) /*GPMC_D9*/
	MUX_VAL(CP(GPMC_D10), (IEN  | PTU | EN  | M0)) /*GPMC_D10*/
	MUX_VAL(CP(GPMC_D11), (IEN  | PTU | EN  | M0)) /*GPMC_D11*/
	MUX_VAL(CP(GPMC_D12), (IEN  | PTU | EN  | M0)) /*GPMC_D12*/
	MUX_VAL(CP(GPMC_D13), (IEN  | PTU | EN  | M0)) /*GPMC_D13*/
	MUX_VAL(CP(GPMC_D14), (IEN  | PTU | EN  | M0)) /*GPMC_D14*/
	MUX_VAL(CP(GPMC_D15), (IEN  | PTU | EN  | M0)) /*GPMC_D15*/
	MUX_VAL(CP(GPMC_NCS0), (IDIS | PTU | EN  | M0)) /*GPMC_nCS0*/
	MUX_VAL(CP(GPMC_NCS1), (IDIS | PTU | EN  | M0)) /*GPMC_nCS1*/
	MUX_VAL(CP(GPMC_NCS2), (IDIS | PTU | EN  | M0)) /*GPMC_nCS2*/
	MUX_VAL(CP(GPMC_NCS3), (IDIS | PTU | EN  | M0)) /*GPMC_nCS3*/
	MUX_VAL(CP(GPMC_NCS4), (IEN  | PTU | EN  | M0)) /*GPMC_nCS4*/
	MUX_VAL(CP(GPMC_NCS5), (IDIS | PTU | EN  | M0)) /*GPMC_nCS5*/
	MUX_VAL(CP(GPMC_NCS6), (IEN  | PTU | EN | M0)) /*GPMC_nCS6*/
	MUX_VAL(CP(GPMC_NCS7), (IEN  | PTU | EN  | M0)) /*GPMC_nCS7*/
	MUX_VAL(CP(GPMC_CLK), (IDIS | PTU | EN  | M0)) /*GPMC_CLK*/
	MUX_VAL(CP(GPMC_NADV_ALE), (IDIS | PTD | DIS | M0)) /*GPMC_nADV_ALE*/
	MUX_VAL(CP(GPMC_NOE), (IDIS | PTD | DIS | M0)) /*GPMC_nOE*/
	MUX_VAL(CP(GPMC_NWE), (IDIS | PTD | DIS | M0)) /*GPMC_nWE*/
	MUX_VAL(CP(GPMC_NBE0_CLE), (IDIS | PTU | EN  | M0)) /*GPMC_nBE0_CLE*/
	MUX_VAL(CP(GPMC_NBE1), (IEN  | PTU | EN  | M0)) /*GPMC_nBE1*/
	MUX_VAL(CP(GPMC_NWP),  (IEN  | PTD | DIS | M0)) /*GPMC_nWP*/
	MUX_VAL(CP(GPMC_WAIT0), (IEN  | PTU | EN  | M0)) /*GPMC_WAIT0*/
	MUX_VAL(CP(GPMC_WAIT1), (IEN  | PTU | EN  | M0)) /*GPMC_WAIT1*/
	MUX_VAL(CP(GPMC_WAIT2), (IEN  | PTU | EN  | M4)) /*GPIO_64*/
	MUX_VAL(CP(GPMC_WAIT3), (IEN  | PTU | EN  | M0)) /*GPMC_WAIT3*/

	MUX_VAL(CP(MMC1_CLK), (IDIS | PTU | EN  | M0)) /*MMC1_CLK*/
	MUX_VAL(CP(MMC1_CMD), (IEN  | PTU | EN  | M0)) /*MMC1_CMD*/
	MUX_VAL(CP(MMC1_DAT0), (IEN  | PTU | EN  | M0)) /*MMC1_DAT0*/
	MUX_VAL(CP(MMC1_DAT1), (IEN  | PTU | EN  | M0)) /*MMC1_DAT1*/
	MUX_VAL(CP(MMC1_DAT2), (IEN  | PTU | EN  | M0)) /*MMC1_DAT2*/
	MUX_VAL(CP(MMC1_DAT3), (IEN  | PTU | EN  | M0)) /*MMC1_DAT3*/

	MUX_VAL(CP(UART1_TX), (IDIS | PTD | DIS | M0)) /*UART1_TX*/
	MUX_VAL(CP(UART1_RTS), (IDIS | PTD | DIS | M0)) /*UART1_RTS*/
	MUX_VAL(CP(UART1_CTS), (IEN  | PTU | DIS | M0)) /*UART1_CTS*/
	MUX_VAL(CP(UART1_RX), (IEN  | PTD | DIS | M0)) /*UART1_RX*/

	MUX_VAL(CP(JTAG_TCK), (IEN  | PTD | DIS | M0)) /*JTAG_TCK*/
	MUX_VAL(CP(JTAG_TMS), (IEN  | PTD | DIS | M0)) /*JTAG_TMS*/
	MUX_VAL(CP(JTAG_TDI), (IEN  | PTD | DIS | M0)) /*JTAG_TDI*/
	MUX_VAL(CP(JTAG_EMU0), (IEN  | PTD | DIS | M0)) /*JTAG_EMU0*/
	MUX_VAL(CP(JTAG_EMU1), (IEN  | PTD | DIS | M0)) /*JTAG_EMU1*/

	MUX_VAL(CP(ETK_CLK_ES2), (IDIS | PTU | EN  | M0)) /*ETK_CLK*/
	MUX_VAL(CP(ETK_CTL_ES2), (IDIS | PTD | DIS | M0)) /*ETK_CTL*/
	MUX_VAL(CP(ETK_D0_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D0*/
	MUX_VAL(CP(ETK_D1_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D1*/
	MUX_VAL(CP(ETK_D2_ES2), (IEN  | PTD | EN  | M0)) /*ETK_D2*/
	MUX_VAL(CP(ETK_D3_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D3*/
	MUX_VAL(CP(ETK_D4_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D4*/
	MUX_VAL(CP(ETK_D5_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D5*/
	MUX_VAL(CP(ETK_D6_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D6*/
	MUX_VAL(CP(ETK_D7_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D7*/
	MUX_VAL(CP(ETK_D8_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D8*/
	MUX_VAL(CP(ETK_D9_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D9*/
#ifndef CONFIG_USB_EHCI_OMAP /* Torpedo does not use EHCI_OMAP */
	MUX_VAL(CP(ETK_D10_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D10*/
	MUX_VAL(CP(ETK_D11_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D11*/
	MUX_VAL(CP(ETK_D12_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D12*/
	MUX_VAL(CP(ETK_D13_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D13*/
	MUX_VAL(CP(ETK_D14_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D14*/
	MUX_VAL(CP(ETK_D15_ES2), (IEN  | PTD | DIS | M0)) /*ETK_D15*/
#endif

	MUX_VAL(CP(D2D_MCAD1), (IEN  | PTD | EN  | M0)) /*d2d_mcad1*/
	MUX_VAL(CP(D2D_MCAD2), (IEN  | PTD | EN  | M0)) /*d2d_mcad2*/
	MUX_VAL(CP(D2D_MCAD3), (IEN  | PTD | EN  | M0)) /*d2d_mcad3*/
	MUX_VAL(CP(D2D_MCAD4), (IEN  | PTD | EN  | M0)) /*d2d_mcad4*/
	MUX_VAL(CP(D2D_MCAD5), (IEN  | PTD | EN  | M0)) /*d2d_mcad5*/
	MUX_VAL(CP(D2D_MCAD6), (IEN  | PTD | EN  | M0)) /*d2d_mcad6*/
	MUX_VAL(CP(D2D_MCAD7), (IEN  | PTD | EN  | M0)) /*d2d_mcad7*/
	MUX_VAL(CP(D2D_MCAD8), (IEN  | PTD | EN  | M0)) /*d2d_mcad8*/
	MUX_VAL(CP(D2D_MCAD9), (IEN  | PTD | EN  | M0)) /*d2d_mcad9*/
	MUX_VAL(CP(D2D_MCAD10), (IEN  | PTD | EN  | M0)) /*d2d_mcad10*/
	MUX_VAL(CP(D2D_MCAD11), (IEN  | PTD | EN  | M0)) /*d2d_mcad11*/
	MUX_VAL(CP(D2D_MCAD12), (IEN  | PTD | EN  | M0)) /*d2d_mcad12*/
	MUX_VAL(CP(D2D_MCAD13), (IEN  | PTD | EN  | M0)) /*d2d_mcad13*/
	MUX_VAL(CP(D2D_MCAD14), (IEN  | PTD | EN  | M0)) /*d2d_mcad14*/
	MUX_VAL(CP(D2D_MCAD15), (IEN  | PTD | EN  | M0)) /*d2d_mcad15*/
	MUX_VAL(CP(D2D_MCAD16), (IEN  | PTD | EN  | M0)) /*d2d_mcad16*/
	MUX_VAL(CP(D2D_MCAD17), (IEN  | PTD | EN  | M0)) /*d2d_mcad17*/
	MUX_VAL(CP(D2D_MCAD18), (IEN  | PTD | EN  | M0)) /*d2d_mcad18*/
	MUX_VAL(CP(D2D_MCAD19), (IEN  | PTD | EN  | M0)) /*d2d_mcad19*/
	MUX_VAL(CP(D2D_MCAD20), (IEN  | PTD | EN  | M0)) /*d2d_mcad20*/
	MUX_VAL(CP(D2D_MCAD21), (IEN  | PTD | EN  | M0)) /*d2d_mcad21*/
	MUX_VAL(CP(D2D_MCAD22), (IEN  | PTD | EN  | M0)) /*d2d_mcad22*/
	MUX_VAL(CP(D2D_MCAD23), (IEN  | PTD | EN  | M0)) /*d2d_mcad23*/
	MUX_VAL(CP(D2D_MCAD24), (IEN  | PTD | EN  | M0)) /*d2d_mcad24*/
	MUX_VAL(CP(D2D_MCAD25), (IEN  | PTD | EN  | M0)) /*d2d_mcad25*/
	MUX_VAL(CP(D2D_MCAD26), (IEN  | PTD | EN  | M0)) /*d2d_mcad26*/
	MUX_VAL(CP(D2D_MCAD27), (IEN  | PTD | EN  | M0)) /*d2d_mcad27*/
	MUX_VAL(CP(D2D_MCAD28), (IEN  | PTD | EN  | M0)) /*d2d_mcad28*/
	MUX_VAL(CP(D2D_MCAD29), (IEN  | PTD | EN  | M0)) /*d2d_mcad29*/
	MUX_VAL(CP(D2D_MCAD30), (IEN  | PTD | EN  | M0)) /*d2d_mcad30*/
	MUX_VAL(CP(D2D_MCAD31), (IEN  | PTD | EN  | M0)) /*d2d_mcad31*/
	MUX_VAL(CP(D2D_MCAD32), (IEN  | PTD | EN  | M0)) /*d2d_mcad32*/
	MUX_VAL(CP(D2D_MCAD33), (IEN  | PTD | EN  | M0)) /*d2d_mcad33*/
	MUX_VAL(CP(D2D_MCAD34), (IEN  | PTD | EN  | M0)) /*d2d_mcad34*/
	MUX_VAL(CP(D2D_MCAD35), (IEN  | PTD | EN  | M0)) /*d2d_mcad35*/
	MUX_VAL(CP(D2D_MCAD36), (IEN  | PTD | EN  | M0)) /*d2d_mcad36*/
	MUX_VAL(CP(D2D_CLK26MI), (IEN  | PTD | DIS | M0)) /*d2d_clk26mi*/
	MUX_VAL(CP(D2D_NRESPWRON), (IEN  | PTD | EN  | M0)) /*d2d_nrespwron*/
	MUX_VAL(CP(D2D_NRESWARM), (IEN  | PTU | EN  | M0)) /*d2d_nreswarm */
	MUX_VAL(CP(D2D_ARM9NIRQ), (IEN  | PTD | DIS | M0)) /*d2d_arm9nirq */
	MUX_VAL(CP(D2D_UMA2P6FIQ), (IEN  | PTD | DIS | M0)) /*d2d_uma2p6fiq*/
	MUX_VAL(CP(D2D_SPINT), (IEN  | PTD | EN  | M0)) /*d2d_spint*/
	MUX_VAL(CP(D2D_FRINT), (IEN  | PTD | EN  | M0)) /*d2d_frint*/
	MUX_VAL(CP(D2D_DMAREQ0), (IEN  | PTD | DIS | M0)) /*d2d_dmareq0*/
	MUX_VAL(CP(D2D_DMAREQ1), (IEN  | PTD | DIS | M0)) /*d2d_dmareq1*/
	MUX_VAL(CP(D2D_DMAREQ2), (IEN  | PTD | DIS | M0)) /*d2d_dmareq2*/
	MUX_VAL(CP(D2D_DMAREQ3), (IEN  | PTD | DIS | M0)) /*d2d_dmareq3*/
	MUX_VAL(CP(D2D_N3GTRST), (IEN  | PTD | DIS | M0)) /*d2d_n3gtrst*/
	MUX_VAL(CP(D2D_N3GTDI), (IEN  | PTD | DIS | M0)) /*d2d_n3gtdi*/
	MUX_VAL(CP(D2D_N3GTDO), (IEN  | PTD | DIS | M0)) /*d2d_n3gtdo*/
	MUX_VAL(CP(D2D_N3GTMS), (IEN  | PTD | DIS | M0)) /*d2d_n3gtms*/
	MUX_VAL(CP(D2D_N3GTCK), (IEN  | PTD | DIS | M0)) /*d2d_n3gtck*/
	MUX_VAL(CP(D2D_N3GRTCK), (IEN  | PTD | DIS | M0)) /*d2d_n3grtck*/
	MUX_VAL(CP(D2D_MSTDBY),  (IEN  | PTU | EN  | M0)) /*d2d_mstdby*/
	MUX_VAL(CP(D2D_SWAKEUP), (IEN  | PTD | EN  | M0)) /*d2d_swakeup*/
	MUX_VAL(CP(D2D_IDLEREQ), (IEN  | PTD | DIS | M0)) /*d2d_idlereq*/
	MUX_VAL(CP(D2D_IDLEACK), (IEN  | PTU | EN  | M0)) /*d2d_idleack*/
	MUX_VAL(CP(D2D_MWRITE), (IEN  | PTD | DIS | M0)) /*d2d_mwrite*/
	MUX_VAL(CP(D2D_SWRITE), (IEN  | PTD | DIS | M0)) /*d2d_swrite*/
	MUX_VAL(CP(D2D_MREAD), (IEN  | PTD | DIS | M0)) /*d2d_mread*/
	MUX_VAL(CP(D2D_SREAD), (IEN  | PTD | DIS | M0)) /*d2d_sread*/
	MUX_VAL(CP(D2D_MBUSFLAG), (IEN  | PTD | DIS | M0)) /*d2d_mbusflag*/
	MUX_VAL(CP(D2D_SBUSFLAG), (IEN  | PTD | DIS | M0)) /*d2d_sbusflag*/
}

#endif
