// SPDX-License-Identifier: Intel
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Ported from Intel released Quark UEFI BIOS
 * QuarkSocPkg/QuarkNorthCluster/MemoryInit/Pei
 */

#include <common.h>
#include <pci.h>
#include <asm/arch/device.h>
#include <asm/arch/mrc.h>
#include <asm/arch/msg_port.h>
#include "mrc_util.h"
#include "hte.h"
#include "smc.h"

/* t_ck clock period in picoseconds per speed index 800, 1066, 1333 */
static const uint32_t t_ck[3] = {
	2500,
	1875,
	1500
};

/* Global variables */
static const uint16_t ddr_wclk[] = {193, 158};
#ifdef BACKUP_WCTL
static const uint16_t ddr_wctl[] = {1, 217};
#endif
#ifdef BACKUP_WCMD
static const uint16_t ddr_wcmd[] = {1, 220};
#endif

#ifdef BACKUP_RCVN
static const uint16_t ddr_rcvn[] = {129, 498};
#endif

#ifdef BACKUP_WDQS
static const uint16_t ddr_wdqs[] = {65, 289};
#endif

#ifdef BACKUP_RDQS
static const uint8_t ddr_rdqs[] = {32, 24};
#endif

#ifdef BACKUP_WDQ
static const uint16_t ddr_wdq[] = {32, 257};
#endif

/* Stop self refresh driven by MCU */
void clear_self_refresh(struct mrc_params *mrc_params)
{
	ENTERFN();

	/* clear the PMSTS Channel Self Refresh bits */
	mrc_write_mask(MEM_CTLR, PMSTS, PMSTS_DISR, PMSTS_DISR);

	LEAVEFN();
}

/* It will initialize timing registers in the MCU (DTR0..DTR4) */
void prog_ddr_timing_control(struct mrc_params *mrc_params)
{
	uint8_t tcl, wl;
	uint8_t trp, trcd, tras, twr, twtr, trrd, trtp, tfaw;
	uint32_t tck;
	u32 dtr0, dtr1, dtr2, dtr3, dtr4;
	u32 tmp1, tmp2;

	ENTERFN();

	/* mcu_init starts */
	mrc_post_code(0x02, 0x00);

	dtr0 = msg_port_read(MEM_CTLR, DTR0);
	dtr1 = msg_port_read(MEM_CTLR, DTR1);
	dtr2 = msg_port_read(MEM_CTLR, DTR2);
	dtr3 = msg_port_read(MEM_CTLR, DTR3);
	dtr4 = msg_port_read(MEM_CTLR, DTR4);

	tck = t_ck[mrc_params->ddr_speed];	/* Clock in picoseconds */
	tcl = mrc_params->params.cl;		/* CAS latency in clocks */
	trp = tcl;	/* Per CAT MRC */
	trcd = tcl;	/* Per CAT MRC */
	tras = MCEIL(mrc_params->params.ras, tck);

	/* Per JEDEC: tWR=15000ps DDR2/3 from 800-1600 */
	twr = MCEIL(15000, tck);

	twtr = MCEIL(mrc_params->params.wtr, tck);
	trrd = MCEIL(mrc_params->params.rrd, tck);
	trtp = 4;	/* Valid for 800 and 1066, use 5 for 1333 */
	tfaw = MCEIL(mrc_params->params.faw, tck);

	wl = 5 + mrc_params->ddr_speed;

	dtr0 &= ~DTR0_DFREQ_MASK;
	dtr0 |= mrc_params->ddr_speed;
	dtr0 &= ~DTR0_TCL_MASK;
	tmp1 = tcl - 5;
	dtr0 |= ((tcl - 5) << 12);
	dtr0 &= ~DTR0_TRP_MASK;
	dtr0 |= ((trp - 5) << 4);	/* 5 bit DRAM Clock */
	dtr0 &= ~DTR0_TRCD_MASK;
	dtr0 |= ((trcd - 5) << 8);	/* 5 bit DRAM Clock */

	dtr1 &= ~DTR1_TWCL_MASK;
	tmp2 = wl - 3;
	dtr1 |= (wl - 3);
	dtr1 &= ~DTR1_TWTP_MASK;
	dtr1 |= ((wl + 4 + twr - 14) << 8);	/* Change to tWTP */
	dtr1 &= ~DTR1_TRTP_MASK;
	dtr1 |= ((MMAX(trtp, 4) - 3) << 28);	/* 4 bit DRAM Clock */
	dtr1 &= ~DTR1_TRRD_MASK;
	dtr1 |= ((trrd - 4) << 24);		/* 4 bit DRAM Clock */
	dtr1 &= ~DTR1_TCMD_MASK;
	dtr1 |= (1 << 4);
	dtr1 &= ~DTR1_TRAS_MASK;
	dtr1 |= ((tras - 14) << 20);		/* 6 bit DRAM Clock */
	dtr1 &= ~DTR1_TFAW_MASK;
	dtr1 |= ((((tfaw + 1) >> 1) - 5) << 16);/* 4 bit DRAM Clock */
	/* Set 4 Clock CAS to CAS delay (multi-burst) */
	dtr1 &= ~DTR1_TCCD_MASK;

	dtr2 &= ~DTR2_TRRDR_MASK;
	dtr2 |= 1;
	dtr2 &= ~DTR2_TWWDR_MASK;
	dtr2 |= (2 << 8);
	dtr2 &= ~DTR2_TRWDR_MASK;
	dtr2 |= (2 << 16);

	dtr3 &= ~DTR3_TWRDR_MASK;
	dtr3 |= 2;
	dtr3 &= ~DTR3_TXXXX_MASK;
	dtr3 |= (2 << 4);

	dtr3 &= ~DTR3_TRWSR_MASK;
	if (mrc_params->ddr_speed == DDRFREQ_800) {
		/* Extended RW delay (+1) */
		dtr3 |= ((tcl - 5 + 1) << 8);
	} else if (mrc_params->ddr_speed == DDRFREQ_1066) {
		/* Extended RW delay (+1) */
		dtr3 |= ((tcl - 5 + 1) << 8);
	}

	dtr3 &= ~DTR3_TWRSR_MASK;
	dtr3 |= ((4 + wl + twtr - 11) << 13);

	dtr3 &= ~DTR3_TXP_MASK;
	if (mrc_params->ddr_speed == DDRFREQ_800)
		dtr3 |= ((MMAX(0, 1 - 1)) << 22);
	else
		dtr3 |= ((MMAX(0, 2 - 1)) << 22);

	dtr4 &= ~DTR4_WRODTSTRT_MASK;
	dtr4 |= 1;
	dtr4 &= ~DTR4_WRODTSTOP_MASK;
	dtr4 |= (1 << 4);
	dtr4 &= ~DTR4_XXXX1_MASK;
	dtr4 |= ((1 + tmp1 - tmp2 + 2) << 8);
	dtr4 &= ~DTR4_XXXX2_MASK;
	dtr4 |= ((1 + tmp1 - tmp2 + 2) << 12);
	dtr4 &= ~(DTR4_ODTDIS | DTR4_TRGSTRDIS);

	msg_port_write(MEM_CTLR, DTR0, dtr0);
	msg_port_write(MEM_CTLR, DTR1, dtr1);
	msg_port_write(MEM_CTLR, DTR2, dtr2);
	msg_port_write(MEM_CTLR, DTR3, dtr3);
	msg_port_write(MEM_CTLR, DTR4, dtr4);

	LEAVEFN();
}

/* Configure MCU before jedec init sequence */
void prog_decode_before_jedec(struct mrc_params *mrc_params)
{
	u32 drp;
	u32 drfc;
	u32 dcal;
	u32 dsch;
	u32 dpmc0;

	ENTERFN();

	/* Disable power saving features */
	dpmc0 = msg_port_read(MEM_CTLR, DPMC0);
	dpmc0 |= (DPMC0_CLKGTDIS | DPMC0_DISPWRDN);
	dpmc0 &= ~DPMC0_PCLSTO_MASK;
	dpmc0 &= ~DPMC0_DYNSREN;
	msg_port_write(MEM_CTLR, DPMC0, dpmc0);

	/* Disable out of order transactions */
	dsch = msg_port_read(MEM_CTLR, DSCH);
	dsch |= (DSCH_OOODIS | DSCH_NEWBYPDIS);
	msg_port_write(MEM_CTLR, DSCH, dsch);

	/* Disable issuing the REF command */
	drfc = msg_port_read(MEM_CTLR, DRFC);
	drfc &= ~DRFC_TREFI_MASK;
	msg_port_write(MEM_CTLR, DRFC, drfc);

	/* Disable ZQ calibration short */
	dcal = msg_port_read(MEM_CTLR, DCAL);
	dcal &= ~DCAL_ZQCINT_MASK;
	dcal &= ~DCAL_SRXZQCL_MASK;
	msg_port_write(MEM_CTLR, DCAL, dcal);

	/*
	 * Training performed in address mode 0, rank population has limited
	 * impact, however simulator complains if enabled non-existing rank.
	 */
	drp = 0;
	if (mrc_params->rank_enables & 1)
		drp |= DRP_RKEN0;
	if (mrc_params->rank_enables & 2)
		drp |= DRP_RKEN1;
	msg_port_write(MEM_CTLR, DRP, drp);

	LEAVEFN();
}

/*
 * After Cold Reset, BIOS should set COLDWAKE bit to 1 before
 * sending the WAKE message to the Dunit.
 *
 * For Standby Exit, or any other mode in which the DRAM is in
 * SR, this bit must be set to 0.
 */
void perform_ddr_reset(struct mrc_params *mrc_params)
{
	ENTERFN();

	/* Set COLDWAKE bit before sending the WAKE message */
	mrc_write_mask(MEM_CTLR, DRMC, DRMC_COLDWAKE, DRMC_COLDWAKE);

	/* Send wake command to DUNIT (MUST be done before JEDEC) */
	dram_wake_command();

	/* Set default value */
	msg_port_write(MEM_CTLR, DRMC,
		       mrc_params->rd_odt_value == 0 ? DRMC_ODTMODE : 0);

	LEAVEFN();
}


/*
 * This function performs some initialization on the DDRIO unit.
 * This function is dependent on BOARD_ID, DDR_SPEED, and CHANNEL_ENABLES.
 */
void ddrphy_init(struct mrc_params *mrc_params)
{
	uint32_t temp;
	uint8_t ch;	/* channel counter */
	uint8_t rk;	/* rank counter */
	uint8_t bl_grp;	/*  byte lane group counter (2 BLs per module) */
	uint8_t bl_divisor = 1;	/* byte lane divisor */
	/* For DDR3 --> 0 == 800, 1 == 1066, 2 == 1333 */
	uint8_t speed = mrc_params->ddr_speed & 3;
	uint8_t cas;
	uint8_t cwl;

	ENTERFN();

	cas = mrc_params->params.cl;
	cwl = 5 + mrc_params->ddr_speed;

	/* ddrphy_init starts */
	mrc_post_code(0x03, 0x00);

	/*
	 * HSD#231531
	 * Make sure IOBUFACT is deasserted before initializing the DDR PHY
	 *
	 * HSD#234845
	 * Make sure WRPTRENABLE is deasserted before initializing the DDR PHY
	 */
	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			/* Deassert DDRPHY Initialization Complete */
			mrc_alt_write_mask(DDRPHY,
				CMDPMCONFIG0 + ch * DDRIOCCC_CH_OFFSET,
				~(1 << 20), 1 << 20);	/* SPID_INIT_COMPLETE=0 */
			/* Deassert IOBUFACT */
			mrc_alt_write_mask(DDRPHY,
				CMDCFGREG0 + ch * DDRIOCCC_CH_OFFSET,
				~(1 << 2), 1 << 2);	/* IOBUFACTRST_N=0 */
			/* Disable WRPTR */
			mrc_alt_write_mask(DDRPHY,
				CMDPTRREG + ch * DDRIOCCC_CH_OFFSET,
				~(1 << 0), 1 << 0);	/* WRPTRENABLE=0 */
		}
	}

	/* Put PHY in reset */
	mrc_alt_write_mask(DDRPHY, MASTERRSTN, 0, 1);

	/* Initialize DQ01, DQ23, CMD, CLK-CTL, COMP modules */

	/* STEP0 */
	mrc_post_code(0x03, 0x10);
	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			/* DQ01-DQ23 */
			for (bl_grp = 0;
			     bl_grp < (NUM_BYTE_LANES / bl_divisor) / 2;
			     bl_grp++) {
				/* Analog MUX select - IO2xCLKSEL */
				mrc_alt_write_mask(DDRPHY,
					DQOBSCKEBBCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					bl_grp ? 0 : (1 << 22), 1 << 22);

				/* ODT Strength */
				switch (mrc_params->rd_odt_value) {
				case 1:
					temp = 0x3;
					break;	/* 60 ohm */
				case 2:
					temp = 0x3;
					break;	/* 120 ohm */
				case 3:
					temp = 0x3;
					break;	/* 180 ohm */
				default:
					temp = 0x3;
					break;	/* 120 ohm */
				}

				/* ODT strength */
				mrc_alt_write_mask(DDRPHY,
					B0RXIOBUFCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					temp << 5, 0x60);
				/* ODT strength */
				mrc_alt_write_mask(DDRPHY,
					B1RXIOBUFCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					temp << 5, 0x60);

				/* Dynamic ODT/DIFFAMP */
				temp = (cas << 24) | (cas << 16) |
					(cas << 8) | (cas << 0);
				switch (speed) {
				case 0:
					temp -= 0x01010101;
					break;	/* 800 */
				case 1:
					temp -= 0x02020202;
					break;	/* 1066 */
				case 2:
					temp -= 0x03030303;
					break;	/* 1333 */
				case 3:
					temp -= 0x04040404;
					break;	/* 1600 */
				}

				/* Launch Time: ODT, DIFFAMP, ODT, DIFFAMP */
				mrc_alt_write_mask(DDRPHY,
					B01LATCTL1 +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					temp, 0x1f1f1f1f);
				switch (speed) {
				/* HSD#234715 */
				case 0:
					temp = (0x06 << 16) | (0x07 << 8);
					break;	/* 800 */
				case 1:
					temp = (0x07 << 16) | (0x08 << 8);
					break;	/* 1066 */
				case 2:
					temp = (0x09 << 16) | (0x0a << 8);
					break;	/* 1333 */
				case 3:
					temp = (0x0a << 16) | (0x0b << 8);
					break;	/* 1600 */
				}

				/* On Duration: ODT, DIFFAMP */
				mrc_alt_write_mask(DDRPHY,
					B0ONDURCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					temp, 0x003f3f00);
				/* On Duration: ODT, DIFFAMP */
				mrc_alt_write_mask(DDRPHY,
					B1ONDURCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					temp, 0x003f3f00);

				switch (mrc_params->rd_odt_value) {
				case 0:
					/* override DIFFAMP=on, ODT=off */
					temp = (0x3f << 16) | (0x3f << 10);
					break;
				default:
					/* override DIFFAMP=on, ODT=on */
					temp = (0x3f << 16) | (0x2a << 10);
					break;
				}

				/* Override: DIFFAMP, ODT */
				mrc_alt_write_mask(DDRPHY,
					B0OVRCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					temp, 0x003ffc00);
				/* Override: DIFFAMP, ODT */
				mrc_alt_write_mask(DDRPHY,
					B1OVRCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					temp, 0x003ffc00);

				/* DLL Setup */

				/* 1xCLK Domain Timings: tEDP,RCVEN,WDQS (PO) */
				mrc_alt_write_mask(DDRPHY,
					B0LATCTL0 +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					((cas + 7) << 16) | ((cas - 4) << 8) |
					((cwl - 2) << 0), 0x003f1f1f);
				mrc_alt_write_mask(DDRPHY,
					B1LATCTL0 +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					((cas + 7) << 16) | ((cas - 4) << 8) |
					((cwl - 2) << 0), 0x003f1f1f);

				/* RCVEN Bypass (PO) */
				mrc_alt_write_mask(DDRPHY,
					B0RXIOBUFCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					0, 0x81);
				mrc_alt_write_mask(DDRPHY,
					B1RXIOBUFCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					0, 0x81);

				/* TX */
				mrc_alt_write_mask(DDRPHY,
					DQCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					1 << 16, 1 << 16);
				mrc_alt_write_mask(DDRPHY,
					B01PTRCTL1 +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					1 << 8, 1 << 8);

				/* RX (PO) */
				/* Internal Vref Code, Enable#, Ext_or_Int (1=Ext) */
				mrc_alt_write_mask(DDRPHY,
					B0VREFCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					(0x03 << 2) | (0x0 << 1) | (0x0 << 0),
					0xff);
				/* Internal Vref Code, Enable#, Ext_or_Int (1=Ext) */
				mrc_alt_write_mask(DDRPHY,
					B1VREFCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					(0x03 << 2) | (0x0 << 1) | (0x0 << 0),
					0xff);
				/* Per-Bit De-Skew Enable */
				mrc_alt_write_mask(DDRPHY,
					B0RXIOBUFCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					0, 0x10);
				/* Per-Bit De-Skew Enable */
				mrc_alt_write_mask(DDRPHY,
					B1RXIOBUFCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					0, 0x10);
			}

			/* CLKEBB */
			mrc_alt_write_mask(DDRPHY,
				CMDOBSCKEBBCTL + ch * DDRIOCCC_CH_OFFSET,
				0, 1 << 23);

			/* Enable tristate control of cmd/address bus */
			mrc_alt_write_mask(DDRPHY,
				CMDCFGREG0 + ch * DDRIOCCC_CH_OFFSET,
				0, 0x03);

			/* ODT RCOMP */
			mrc_alt_write_mask(DDRPHY,
				CMDRCOMPODT + ch * DDRIOCCC_CH_OFFSET,
				(0x03 << 5) | (0x03 << 0), 0x3ff);

			/* CMDPM* registers must be programmed in this order */

			/* Turn On Delays: SFR (regulator), MPLL */
			mrc_alt_write_mask(DDRPHY,
				CMDPMDLYREG4 + ch * DDRIOCCC_CH_OFFSET,
				0xffffffff, 0xffffffff);
			/*
			 * Delays: ASSERT_IOBUFACT_to_ALLON0_for_PM_MSG_3,
			 * VREG (MDLL) Turn On, ALLON0_to_DEASSERT_IOBUFACT
			 * for_PM_MSG_gt0, MDLL Turn On
			 */
			mrc_alt_write_mask(DDRPHY,
				CMDPMDLYREG3 + ch * DDRIOCCC_CH_OFFSET,
				0xfffff616, 0xffffffff);
			/* MPLL Divider Reset Delays */
			mrc_alt_write_mask(DDRPHY,
				CMDPMDLYREG2 + ch * DDRIOCCC_CH_OFFSET,
				0xffffffff, 0xffffffff);
			/* Turn Off Delays: VREG, Staggered MDLL, MDLL, PI */
			mrc_alt_write_mask(DDRPHY,
				CMDPMDLYREG1 + ch * DDRIOCCC_CH_OFFSET,
				0xffffffff, 0xffffffff);
			/* Turn On Delays: MPLL, Staggered MDLL, PI, IOBUFACT */
			mrc_alt_write_mask(DDRPHY,
				CMDPMDLYREG0 + ch * DDRIOCCC_CH_OFFSET,
				0xffffffff, 0xffffffff);
			/* Allow PUnit signals */
			mrc_alt_write_mask(DDRPHY,
				CMDPMCONFIG0 + ch * DDRIOCCC_CH_OFFSET,
				(0x6 << 8) | (0x1 << 6) | (0x4 << 0),
				0xffe00f4f);
			/* DLL_VREG Bias Trim, VREF Tuning for DLL_VREG */
			mrc_alt_write_mask(DDRPHY,
				CMDMDLLCTL + ch * DDRIOCCC_CH_OFFSET,
				(0x3 << 4) | (0x7 << 0), 0x7f);

			/* CLK-CTL */
			mrc_alt_write_mask(DDRPHY,
				CCOBSCKEBBCTL + ch * DDRIOCCC_CH_OFFSET,
				0, 1 << 24);	/* CLKEBB */
			/* Buffer Enable: CS,CKE,ODT,CLK */
			mrc_alt_write_mask(DDRPHY,
				CCCFGREG0 + ch * DDRIOCCC_CH_OFFSET,
				0x1f, 0x000ffff1);
			/* ODT RCOMP */
			mrc_alt_write_mask(DDRPHY,
				CCRCOMPODT + ch * DDRIOCCC_CH_OFFSET,
				(0x03 << 8) | (0x03 << 0), 0x00001f1f);
			/* DLL_VREG Bias Trim, VREF Tuning for DLL_VREG */
			mrc_alt_write_mask(DDRPHY,
				CCMDLLCTL + ch * DDRIOCCC_CH_OFFSET,
				(0x3 << 4) | (0x7 << 0), 0x7f);

			/*
			 * COMP (RON channel specific)
			 * - DQ/DQS/DM RON: 32 Ohm
			 * - CTRL/CMD RON: 27 Ohm
			 * - CLK RON: 26 Ohm
			 */
			/* RCOMP Vref PU/PD */
			mrc_alt_write_mask(DDRPHY,
				DQVREFCH0 +  ch * DDRCOMP_CH_OFFSET,
				(0x08 << 24) | (0x03 << 16), 0x3f3f0000);
			/* RCOMP Vref PU/PD */
			mrc_alt_write_mask(DDRPHY,
				CMDVREFCH0 + ch * DDRCOMP_CH_OFFSET,
				(0x0C << 24) | (0x03 << 16), 0x3f3f0000);
			/* RCOMP Vref PU/PD */
			mrc_alt_write_mask(DDRPHY,
				CLKVREFCH0 + ch * DDRCOMP_CH_OFFSET,
				(0x0F << 24) | (0x03 << 16), 0x3f3f0000);
			/* RCOMP Vref PU/PD */
			mrc_alt_write_mask(DDRPHY,
				DQSVREFCH0 + ch * DDRCOMP_CH_OFFSET,
				(0x08 << 24) | (0x03 << 16), 0x3f3f0000);
			/* RCOMP Vref PU/PD */
			mrc_alt_write_mask(DDRPHY,
				CTLVREFCH0 + ch * DDRCOMP_CH_OFFSET,
				(0x0C << 24) | (0x03 << 16), 0x3f3f0000);

			/* DQS Swapped Input Enable */
			mrc_alt_write_mask(DDRPHY,
				COMPEN1CH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 19) | (1 << 17), 0xc00ac000);

			/* ODT VREF = 1.5 x 274/360+274 = 0.65V (code of ~50) */
			/* ODT Vref PU/PD */
			mrc_alt_write_mask(DDRPHY,
				DQVREFCH0 + ch * DDRCOMP_CH_OFFSET,
				(0x32 << 8) | (0x03 << 0), 0x00003f3f);
			/* ODT Vref PU/PD */
			mrc_alt_write_mask(DDRPHY,
				DQSVREFCH0 + ch * DDRCOMP_CH_OFFSET,
				(0x32 << 8) | (0x03 << 0), 0x00003f3f);
			/* ODT Vref PU/PD */
			mrc_alt_write_mask(DDRPHY,
				CLKVREFCH0 + ch * DDRCOMP_CH_OFFSET,
				(0x0E << 8) | (0x05 << 0), 0x00003f3f);

			/*
			 * Slew rate settings are frequency specific,
			 * numbers below are for 800Mhz (speed == 0)
			 * - DQ/DQS/DM/CLK SR: 4V/ns,
			 * - CTRL/CMD SR: 1.5V/ns
			 */
			temp = (0x0e << 16) | (0x0e << 12) | (0x08 << 8) |
				(0x0b << 4) | (0x0b << 0);
			/* DCOMP Delay Select: CTL,CMD,CLK,DQS,DQ */
			mrc_alt_write_mask(DDRPHY,
				DLYSELCH0 + ch * DDRCOMP_CH_OFFSET,
				temp, 0x000fffff);
			/* TCO Vref CLK,DQS,DQ */
			mrc_alt_write_mask(DDRPHY,
				TCOVREFCH0 + ch * DDRCOMP_CH_OFFSET,
				(0x05 << 16) | (0x05 << 8) | (0x05 << 0),
				0x003f3f3f);
			/* ODTCOMP CMD/CTL PU/PD */
			mrc_alt_write_mask(DDRPHY,
				CCBUFODTCH0 + ch * DDRCOMP_CH_OFFSET,
				(0x03 << 8) | (0x03 << 0),
				0x00001f1f);
			/* COMP */
			mrc_alt_write_mask(DDRPHY,
				COMPEN0CH0 + ch * DDRCOMP_CH_OFFSET,
				0, 0xc0000100);

#ifdef BACKUP_COMPS
			/* DQ COMP Overrides */
			/* RCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				DQDRVPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0a << 16),
				0x801f0000);
			/* RCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				DQDRVPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0a << 16),
				0x801f0000);
			/* DCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				DQDLYPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x10 << 16),
				0x801f0000);
			/* DCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				DQDLYPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x10 << 16),
				0x801f0000);
			/* ODTCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				DQODTPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0b << 16),
				0x801f0000);
			/* ODTCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				DQODTPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0b << 16),
				0x801f0000);
			/* TCOCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				DQTCOPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				1 << 31, 1 << 31);
			/* TCOCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				DQTCOPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				1 << 31, 1 << 31);

			/* DQS COMP Overrides */
			/* RCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				DQSDRVPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0a << 16),
				0x801f0000);
			/* RCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				DQSDRVPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0a << 16),
				0x801f0000);
			/* DCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				DQSDLYPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x10 << 16),
				0x801f0000);
			/* DCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				DQSDLYPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x10 << 16),
				0x801f0000);
			/* ODTCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				DQSODTPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0b << 16),
				0x801f0000);
			/* ODTCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				DQSODTPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0b << 16),
				0x801f0000);
			/* TCOCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				DQSTCOPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				1 << 31, 1 << 31);
			/* TCOCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				DQSTCOPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				1 << 31, 1 << 31);

			/* CLK COMP Overrides */
			/* RCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				CLKDRVPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0c << 16),
				0x801f0000);
			/* RCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				CLKDRVPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0c << 16),
				0x801f0000);
			/* DCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				CLKDLYPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x07 << 16),
				0x801f0000);
			/* DCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				CLKDLYPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x07 << 16),
				0x801f0000);
			/* ODTCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				CLKODTPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0b << 16),
				0x801f0000);
			/* ODTCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				CLKODTPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0b << 16),
				0x801f0000);
			/* TCOCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				CLKTCOPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				1 << 31, 1 << 31);
			/* TCOCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				CLKTCOPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				1 << 31, 1 << 31);

			/* CMD COMP Overrides */
			/* RCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				CMDDRVPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0d << 16),
				0x803f0000);
			/* RCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				CMDDRVPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0d << 16),
				0x803f0000);
			/* DCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				CMDDLYPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0a << 16),
				0x801f0000);
			/* DCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				CMDDLYPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0a << 16),
				0x801f0000);

			/* CTL COMP Overrides */
			/* RCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				CTLDRVPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0d << 16),
				0x803f0000);
			/* RCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				CTLDRVPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0d << 16),
				0x803f0000);
			/* DCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				CTLDLYPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0a << 16),
				0x801f0000);
			/* DCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				CTLDLYPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x0a << 16),
				0x801f0000);
#else
			/* DQ TCOCOMP Overrides */
			/* TCOCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				DQTCOPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x1f << 16),
				0x801f0000);
			/* TCOCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				DQTCOPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x1f << 16),
				0x801f0000);

			/* DQS TCOCOMP Overrides */
			/* TCOCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				DQSTCOPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x1f << 16),
				0x801f0000);
			/* TCOCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				DQSTCOPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x1f << 16),
				0x801f0000);

			/* CLK TCOCOMP Overrides */
			/* TCOCOMP PU */
			mrc_alt_write_mask(DDRPHY,
				CLKTCOPUCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x1f << 16),
				0x801f0000);
			/* TCOCOMP PD */
			mrc_alt_write_mask(DDRPHY,
				CLKTCOPDCTLCH0 + ch * DDRCOMP_CH_OFFSET,
				(1 << 31) | (0x1f << 16),
				0x801f0000);
#endif

			/* program STATIC delays */
#ifdef BACKUP_WCMD
			set_wcmd(ch, ddr_wcmd[PLATFORM_ID]);
#else
			set_wcmd(ch, ddr_wclk[PLATFORM_ID] + HALF_CLK);
#endif

			for (rk = 0; rk < NUM_RANKS; rk++) {
				if (mrc_params->rank_enables & (1 << rk)) {
					set_wclk(ch, rk, ddr_wclk[PLATFORM_ID]);
#ifdef BACKUP_WCTL
					set_wctl(ch, rk, ddr_wctl[PLATFORM_ID]);
#else
					set_wctl(ch, rk, ddr_wclk[PLATFORM_ID] + HALF_CLK);
#endif
				}
			}
		}
	}

	/* COMP (non channel specific) */
	/* RCOMP: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, DQANADRVPUCTL, 1 << 30, 1 << 30);
	/* RCOMP: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, DQANADRVPDCTL, 1 << 30, 1 << 30);
	/* RCOMP: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, CMDANADRVPUCTL, 1 << 30, 1 << 30);
	/* RCOMP: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, CMDANADRVPDCTL, 1 << 30, 1 << 30);
	/* RCOMP: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, CLKANADRVPUCTL, 1 << 30, 1 << 30);
	/* RCOMP: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, CLKANADRVPDCTL, 1 << 30, 1 << 30);
	/* RCOMP: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, DQSANADRVPUCTL, 1 << 30, 1 << 30);
	/* RCOMP: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, DQSANADRVPDCTL, 1 << 30, 1 << 30);
	/* RCOMP: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, CTLANADRVPUCTL, 1 << 30, 1 << 30);
	/* RCOMP: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, CTLANADRVPDCTL, 1 << 30, 1 << 30);
	/* ODT: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, DQANAODTPUCTL, 1 << 30, 1 << 30);
	/* ODT: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, DQANAODTPDCTL, 1 << 30, 1 << 30);
	/* ODT: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, CLKANAODTPUCTL, 1 << 30, 1 << 30);
	/* ODT: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, CLKANAODTPDCTL, 1 << 30, 1 << 30);
	/* ODT: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, DQSANAODTPUCTL, 1 << 30, 1 << 30);
	/* ODT: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, DQSANAODTPDCTL, 1 << 30, 1 << 30);
	/* DCOMP: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, DQANADLYPUCTL, 1 << 30, 1 << 30);
	/* DCOMP: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, DQANADLYPDCTL, 1 << 30, 1 << 30);
	/* DCOMP: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, CMDANADLYPUCTL, 1 << 30, 1 << 30);
	/* DCOMP: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, CMDANADLYPDCTL, 1 << 30, 1 << 30);
	/* DCOMP: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, CLKANADLYPUCTL, 1 << 30, 1 << 30);
	/* DCOMP: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, CLKANADLYPDCTL, 1 << 30, 1 << 30);
	/* DCOMP: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, DQSANADLYPUCTL, 1 << 30, 1 << 30);
	/* DCOMP: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, DQSANADLYPDCTL, 1 << 30, 1 << 30);
	/* DCOMP: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, CTLANADLYPUCTL, 1 << 30, 1 << 30);
	/* DCOMP: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, CTLANADLYPDCTL, 1 << 30, 1 << 30);
	/* TCO: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, DQANATCOPUCTL, 1 << 30, 1 << 30);
	/* TCO: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, DQANATCOPDCTL, 1 << 30, 1 << 30);
	/* TCO: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, CLKANATCOPUCTL, 1 << 30, 1 << 30);
	/* TCO: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, CLKANATCOPDCTL, 1 << 30, 1 << 30);
	/* TCO: Dither PU Enable */
	mrc_alt_write_mask(DDRPHY, DQSANATCOPUCTL, 1 << 30, 1 << 30);
	/* TCO: Dither PD Enable */
	mrc_alt_write_mask(DDRPHY, DQSANATCOPDCTL, 1 << 30, 1 << 30);
	/* TCOCOMP: Pulse Count */
	mrc_alt_write_mask(DDRPHY, TCOCNTCTRL, 1, 3);
	/* ODT: CMD/CTL PD/PU */
	mrc_alt_write_mask(DDRPHY, CHNLBUFSTATIC,
		(0x03 << 24) | (0x03 << 16), 0x1f1f0000);
	/* Set 1us counter */
	mrc_alt_write_mask(DDRPHY, MSCNTR, 0x64, 0xff);
	mrc_alt_write_mask(DDRPHY, LATCH1CTL, 0x1 << 28, 0x70000000);

	/* Release PHY from reset */
	mrc_alt_write_mask(DDRPHY, MASTERRSTN, 1, 1);

	/* STEP1 */
	mrc_post_code(0x03, 0x11);

	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			/* DQ01-DQ23 */
			for (bl_grp = 0;
			     bl_grp < (NUM_BYTE_LANES / bl_divisor) / 2;
			     bl_grp++) {
				mrc_alt_write_mask(DDRPHY,
					DQMDLLCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					1 << 13,
					1 << 13);	/* Enable VREG */
				delay_n(3);
			}

			/* ECC */
			mrc_alt_write_mask(DDRPHY, ECCMDLLCTL,
				1 << 13, 1 << 13);	/* Enable VREG */
			delay_n(3);
			/* CMD */
			mrc_alt_write_mask(DDRPHY,
				CMDMDLLCTL + ch * DDRIOCCC_CH_OFFSET,
				1 << 13, 1 << 13);	/* Enable VREG */
			delay_n(3);
			/* CLK-CTL */
			mrc_alt_write_mask(DDRPHY,
				CCMDLLCTL + ch * DDRIOCCC_CH_OFFSET,
				1 << 13, 1 << 13);	/* Enable VREG */
			delay_n(3);
		}
	}

	/* STEP2 */
	mrc_post_code(0x03, 0x12);
	delay_n(200);

	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			/* DQ01-DQ23 */
			for (bl_grp = 0;
			     bl_grp < (NUM_BYTE_LANES / bl_divisor) / 2;
			     bl_grp++) {
				mrc_alt_write_mask(DDRPHY,
					DQMDLLCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					1 << 17,
					1 << 17);	/* Enable MCDLL */
				delay_n(50);
			}

		/* ECC */
		mrc_alt_write_mask(DDRPHY, ECCMDLLCTL,
			1 << 17, 1 << 17);	/* Enable MCDLL */
		delay_n(50);
		/* CMD */
		mrc_alt_write_mask(DDRPHY,
			CMDMDLLCTL + ch * DDRIOCCC_CH_OFFSET,
			1 << 18, 1 << 18);	/* Enable MCDLL */
		delay_n(50);
		/* CLK-CTL */
		mrc_alt_write_mask(DDRPHY,
			CCMDLLCTL + ch * DDRIOCCC_CH_OFFSET,
			1 << 18, 1 << 18);	/* Enable MCDLL */
		delay_n(50);
		}
	}

	/* STEP3: */
	mrc_post_code(0x03, 0x13);
	delay_n(100);

	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			/* DQ01-DQ23 */
			for (bl_grp = 0;
			     bl_grp < (NUM_BYTE_LANES / bl_divisor) / 2;
			     bl_grp++) {
#ifdef FORCE_16BIT_DDRIO
				temp = (bl_grp &&
					(mrc_params->channel_width == X16)) ?
					0x11ff : 0xffff;
#else
				temp = 0xffff;
#endif
				/* Enable TXDLL */
				mrc_alt_write_mask(DDRPHY,
					DQDLLTXCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					temp, 0xffff);
				delay_n(3);
				/* Enable RXDLL */
				mrc_alt_write_mask(DDRPHY,
					DQDLLRXCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					0xf, 0xf);
				delay_n(3);
				/* Enable RXDLL Overrides BL0 */
				mrc_alt_write_mask(DDRPHY,
					B0OVRCTL +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					0xf, 0xf);
			}

			/* ECC */
			temp = 0xffff;
			mrc_alt_write_mask(DDRPHY, ECCDLLTXCTL,
				temp, 0xffff);
			delay_n(3);

			/* CMD (PO) */
			mrc_alt_write_mask(DDRPHY,
				CMDDLLTXCTL + ch * DDRIOCCC_CH_OFFSET,
				temp, 0xffff);
			delay_n(3);
		}
	}

	/* STEP4 */
	mrc_post_code(0x03, 0x14);

	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			/* Host To Memory Clock Alignment (HMC) for 800/1066 */
			for (bl_grp = 0;
			     bl_grp < (NUM_BYTE_LANES / bl_divisor) / 2;
			     bl_grp++) {
				/* CLK_ALIGN_MOD_ID */
				mrc_alt_write_mask(DDRPHY,
					DQCLKALIGNREG2 +
					bl_grp * DDRIODQ_BL_OFFSET +
					ch * DDRIODQ_CH_OFFSET,
					bl_grp ? 3 : 1,
					0xf);
			}

			mrc_alt_write_mask(DDRPHY,
				ECCCLKALIGNREG2 + ch * DDRIODQ_CH_OFFSET,
				0x2, 0xf);
			mrc_alt_write_mask(DDRPHY,
				CMDCLKALIGNREG2 + ch * DDRIODQ_CH_OFFSET,
				0x0, 0xf);
			mrc_alt_write_mask(DDRPHY,
				CCCLKALIGNREG2 + ch * DDRIODQ_CH_OFFSET,
				0x2, 0xf);
			mrc_alt_write_mask(DDRPHY,
				CMDCLKALIGNREG0 + ch * DDRIOCCC_CH_OFFSET,
				0x20, 0x30);
			/*
			 * NUM_SAMPLES, MAX_SAMPLES,
			 * MACRO_PI_STEP, MICRO_PI_STEP
			 */
			mrc_alt_write_mask(DDRPHY,
				CMDCLKALIGNREG1 + ch * DDRIOCCC_CH_OFFSET,
				(0x18 << 16) | (0x10 << 8) |
				(0x8 << 2) | (0x1 << 0),
				0x007f7fff);
			/* TOTAL_NUM_MODULES, FIRST_U_PARTITION */
			mrc_alt_write_mask(DDRPHY,
				CMDCLKALIGNREG2 + ch * DDRIOCCC_CH_OFFSET,
				(0x10 << 16) | (0x4 << 8) | (0x2 << 4),
				0x001f0ff0);
#ifdef HMC_TEST
			/* START_CLK_ALIGN=1 */
			mrc_alt_write_mask(DDRPHY,
				CMDCLKALIGNREG0 + ch * DDRIOCCC_CH_OFFSET,
				1 << 24, 1 << 24);
			while (msg_port_alt_read(DDRPHY,
				CMDCLKALIGNREG0 + ch * DDRIOCCC_CH_OFFSET) &
				(1 << 24))
				;	/* wait for START_CLK_ALIGN=0 */
#endif

			/* Set RD/WR Pointer Seperation & COUNTEN & FIFOPTREN */
			mrc_alt_write_mask(DDRPHY,
				CMDPTRREG + ch * DDRIOCCC_CH_OFFSET,
				1, 1);	/* WRPTRENABLE=1 */

			/* COMP initial */
			/* enable bypass for CLK buffer (PO) */
			mrc_alt_write_mask(DDRPHY,
				COMPEN0CH0 + ch * DDRCOMP_CH_OFFSET,
				1 << 5, 1 << 5);
			/* Initial COMP Enable */
			mrc_alt_write_mask(DDRPHY, CMPCTRL, 1, 1);
			/* wait for Initial COMP Enable = 0 */
			while (msg_port_alt_read(DDRPHY, CMPCTRL) & 1)
				;
			/* disable bypass for CLK buffer (PO) */
			mrc_alt_write_mask(DDRPHY,
				COMPEN0CH0 + ch * DDRCOMP_CH_OFFSET,
				~(1 << 5), 1 << 5);

			/* IOBUFACT */

			/* STEP4a */
			mrc_alt_write_mask(DDRPHY,
				CMDCFGREG0 + ch * DDRIOCCC_CH_OFFSET,
				1 << 2, 1 << 2);	/* IOBUFACTRST_N=1 */

			/* DDRPHY initialization complete */
			mrc_alt_write_mask(DDRPHY,
				CMDPMCONFIG0 + ch * DDRIOCCC_CH_OFFSET,
				1 << 20, 1 << 20);	/* SPID_INIT_COMPLETE=1 */
		}
	}

	LEAVEFN();
}

/* This function performs JEDEC initialization on all enabled channels */
void perform_jedec_init(struct mrc_params *mrc_params)
{
	uint8_t twr, wl, rank;
	uint32_t tck;
	u32 dtr0;
	u32 drp;
	u32 drmc;
	u32 mrs0_cmd = 0;
	u32 emrs1_cmd = 0;
	u32 emrs2_cmd = 0;
	u32 emrs3_cmd = 0;

	ENTERFN();

	/* jedec_init starts */
	mrc_post_code(0x04, 0x00);

	/* DDR3_RESET_SET=0, DDR3_RESET_RESET=1 */
	mrc_alt_write_mask(DDRPHY, CCDDR3RESETCTL, 2, 0x102);

	/* Assert RESET# for 200us */
	delay_u(200);

	/* DDR3_RESET_SET=1, DDR3_RESET_RESET=0 */
	mrc_alt_write_mask(DDRPHY, CCDDR3RESETCTL, 0x100, 0x102);

	dtr0 = msg_port_read(MEM_CTLR, DTR0);

	/*
	 * Set CKEVAL for populated ranks
	 * then send NOP to each rank (#4550197)
	 */

	drp = msg_port_read(MEM_CTLR, DRP);
	drp &= 0x3;

	drmc = msg_port_read(MEM_CTLR, DRMC);
	drmc &= 0xfffffffc;
	drmc |= (DRMC_CKEMODE | drp);

	msg_port_write(MEM_CTLR, DRMC, drmc);

	for (rank = 0; rank < NUM_RANKS; rank++) {
		/* Skip to next populated rank */
		if ((mrc_params->rank_enables & (1 << rank)) == 0)
			continue;

		dram_init_command(DCMD_NOP(rank));
	}

	msg_port_write(MEM_CTLR, DRMC,
		(mrc_params->rd_odt_value == 0 ? DRMC_ODTMODE : 0));

	/*
	 * setup for emrs 2
	 * BIT[15:11] --> Always "0"
	 * BIT[10:09] --> Rtt_WR: want "Dynamic ODT Off" (0)
	 * BIT[08]    --> Always "0"
	 * BIT[07]    --> SRT: use sr_temp_range
	 * BIT[06]    --> ASR: want "Manual SR Reference" (0)
	 * BIT[05:03] --> CWL: use oem_tCWL
	 * BIT[02:00] --> PASR: want "Full Array" (0)
	 */
	emrs2_cmd |= (2 << 3);
	wl = 5 + mrc_params->ddr_speed;
	emrs2_cmd |= ((wl - 5) << 9);
	emrs2_cmd |= (mrc_params->sr_temp_range << 13);

	/*
	 * setup for emrs 3
	 * BIT[15:03] --> Always "0"
	 * BIT[02]    --> MPR: want "Normal Operation" (0)
	 * BIT[01:00] --> MPR_Loc: want "Predefined Pattern" (0)
	 */
	emrs3_cmd |= (3 << 3);

	/*
	 * setup for emrs 1
	 * BIT[15:13]     --> Always "0"
	 * BIT[12:12]     --> Qoff: want "Output Buffer Enabled" (0)
	 * BIT[11:11]     --> TDQS: want "Disabled" (0)
	 * BIT[10:10]     --> Always "0"
	 * BIT[09,06,02]  --> Rtt_nom: use rtt_nom_value
	 * BIT[08]        --> Always "0"
	 * BIT[07]        --> WR_LVL: want "Disabled" (0)
	 * BIT[05,01]     --> DIC: use ron_value
	 * BIT[04:03]     --> AL: additive latency want "0" (0)
	 * BIT[00]        --> DLL: want "Enable" (0)
	 *
	 * (BIT5|BIT1) set Ron value
	 * 00 --> RZQ/6 (40ohm)
	 * 01 --> RZQ/7 (34ohm)
	 * 1* --> RESERVED
	 *
	 * (BIT9|BIT6|BIT2) set Rtt_nom value
	 * 000 --> Disabled
	 * 001 --> RZQ/4 ( 60ohm)
	 * 010 --> RZQ/2 (120ohm)
	 * 011 --> RZQ/6 ( 40ohm)
	 * 1** --> RESERVED
	 */
	emrs1_cmd |= (1 << 3);
	emrs1_cmd &= ~(1 << 6);

	if (mrc_params->ron_value == 0)
		emrs1_cmd |= (1 << 7);
	else
		emrs1_cmd &= ~(1 << 7);

	if (mrc_params->rtt_nom_value == 0)
		emrs1_cmd |= (DDR3_EMRS1_RTTNOM_40 << 6);
	else if (mrc_params->rtt_nom_value == 1)
		emrs1_cmd |= (DDR3_EMRS1_RTTNOM_60 << 6);
	else if (mrc_params->rtt_nom_value == 2)
		emrs1_cmd |= (DDR3_EMRS1_RTTNOM_120 << 6);

	/* save MRS1 value (excluding control fields) */
	mrc_params->mrs1 = emrs1_cmd >> 6;

	/*
	 * setup for mrs 0
	 * BIT[15:13]     --> Always "0"
	 * BIT[12]        --> PPD: for Quark (1)
	 * BIT[11:09]     --> WR: use oem_tWR
	 * BIT[08]        --> DLL: want "Reset" (1, self clearing)
	 * BIT[07]        --> MODE: want "Normal" (0)
	 * BIT[06:04,02]  --> CL: use oem_tCAS
	 * BIT[03]        --> RD_BURST_TYPE: want "Interleave" (1)
	 * BIT[01:00]     --> BL: want "8 Fixed" (0)
	 * WR:
	 * 0 --> 16
	 * 1 --> 5
	 * 2 --> 6
	 * 3 --> 7
	 * 4 --> 8
	 * 5 --> 10
	 * 6 --> 12
	 * 7 --> 14
	 * CL:
	 * BIT[02:02] "0" if oem_tCAS <= 11 (1866?)
	 * BIT[06:04] use oem_tCAS-4
	 */
	mrs0_cmd |= (1 << 14);
	mrs0_cmd |= (1 << 18);
	mrs0_cmd |= ((((dtr0 >> 12) & 7) + 1) << 10);

	tck = t_ck[mrc_params->ddr_speed];
	/* Per JEDEC: tWR=15000ps DDR2/3 from 800-1600 */
	twr = MCEIL(15000, tck);
	mrs0_cmd |= ((twr - 4) << 15);

	for (rank = 0; rank < NUM_RANKS; rank++) {
		/* Skip to next populated rank */
		if ((mrc_params->rank_enables & (1 << rank)) == 0)
			continue;

		emrs2_cmd |= (rank << 22);
		dram_init_command(emrs2_cmd);

		emrs3_cmd |= (rank << 22);
		dram_init_command(emrs3_cmd);

		emrs1_cmd |= (rank << 22);
		dram_init_command(emrs1_cmd);

		mrs0_cmd |= (rank << 22);
		dram_init_command(mrs0_cmd);

		dram_init_command(DCMD_ZQCL(rank));
	}

	LEAVEFN();
}

/*
 * Dunit Initialization Complete
 *
 * Indicates that initialization of the Dunit has completed.
 *
 * Memory accesses are permitted and maintenance operation begins.
 * Until this bit is set to a 1, the memory controller will not accept
 * DRAM requests from the MEMORY_MANAGER or HTE.
 */
void set_ddr_init_complete(struct mrc_params *mrc_params)
{
	u32 dco;

	ENTERFN();

	dco = msg_port_read(MEM_CTLR, DCO);
	dco &= ~DCO_PMICTL;
	dco |= DCO_IC;
	msg_port_write(MEM_CTLR, DCO, dco);

	LEAVEFN();
}

/*
 * This function will retrieve relevant timing data
 *
 * This data will be used on subsequent boots to speed up boot times
 * and is required for Suspend To RAM capabilities.
 */
void restore_timings(struct mrc_params *mrc_params)
{
	uint8_t ch, rk, bl;
	const struct mrc_timings *mt = &mrc_params->timings;

	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		for (rk = 0; rk < NUM_RANKS; rk++) {
			for (bl = 0; bl < NUM_BYTE_LANES; bl++) {
				set_rcvn(ch, rk, bl, mt->rcvn[ch][rk][bl]);
				set_rdqs(ch, rk, bl, mt->rdqs[ch][rk][bl]);
				set_wdqs(ch, rk, bl, mt->wdqs[ch][rk][bl]);
				set_wdq(ch, rk, bl, mt->wdq[ch][rk][bl]);
				if (rk == 0) {
					/* VREF (RANK0 only) */
					set_vref(ch, bl, mt->vref[ch][bl]);
				}
			}
			set_wctl(ch, rk, mt->wctl[ch][rk]);
		}
		set_wcmd(ch, mt->wcmd[ch]);
	}
}

/*
 * Configure default settings normally set as part of read training
 *
 * Some defaults have to be set earlier as they may affect earlier
 * training steps.
 */
void default_timings(struct mrc_params *mrc_params)
{
	uint8_t ch, rk, bl;

	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		for (rk = 0; rk < NUM_RANKS; rk++) {
			for (bl = 0; bl < NUM_BYTE_LANES; bl++) {
				set_rdqs(ch, rk, bl, 24);
				if (rk == 0) {
					/* VREF (RANK0 only) */
					set_vref(ch, bl, 32);
				}
			}
		}
	}
}

/*
 * This function will perform our RCVEN Calibration Algorithm.
 * We will only use the 2xCLK domain timings to perform RCVEN Calibration.
 * All byte lanes will be calibrated "simultaneously" per channel per rank.
 */
void rcvn_cal(struct mrc_params *mrc_params)
{
	uint8_t ch;	/* channel counter */
	uint8_t rk;	/* rank counter */
	uint8_t bl;	/* byte lane counter */
	uint8_t bl_divisor = (mrc_params->channel_width == X16) ? 2 : 1;

#ifdef R2R_SHARING
	/* used to find placement for rank2rank sharing configs */
	uint32_t final_delay[NUM_CHANNELS][NUM_BYTE_LANES];
#ifndef BACKUP_RCVN
	/* used to find placement for rank2rank sharing configs */
	uint32_t num_ranks_enabled = 0;
#endif
#endif

#ifdef BACKUP_RCVN
#else
	uint32_t temp;
	/* absolute PI value to be programmed on the byte lane */
	uint32_t delay[NUM_BYTE_LANES];
	u32 dtr1, dtr1_save;
#endif

	ENTERFN();

	/* rcvn_cal starts */
	mrc_post_code(0x05, 0x00);

#ifndef BACKUP_RCVN
	/* need separate burst to sample DQS preamble */
	dtr1 = msg_port_read(MEM_CTLR, DTR1);
	dtr1_save = dtr1;
	dtr1 |= DTR1_TCCD_12CLK;
	msg_port_write(MEM_CTLR, DTR1, dtr1);
#endif

#ifdef R2R_SHARING
	/* need to set "final_delay[][]" elements to "0" */
	memset((void *)(final_delay), 0x00, (size_t)sizeof(final_delay));
#endif

	/* loop through each enabled channel */
	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			/* perform RCVEN Calibration on a per rank basis */
			for (rk = 0; rk < NUM_RANKS; rk++) {
				if (mrc_params->rank_enables & (1 << rk)) {
					/*
					 * POST_CODE here indicates the current
					 * channel and rank being calibrated
					 */
					mrc_post_code(0x05, 0x10 + ((ch << 4) | rk));

#ifdef BACKUP_RCVN
					/* et hard-coded timing values */
					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl++)
						set_rcvn(ch, rk, bl, ddr_rcvn[PLATFORM_ID]);
#else
					/* enable FIFORST */
					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl += 2) {
						mrc_alt_write_mask(DDRPHY,
							B01PTRCTL1 +
							(bl >> 1) * DDRIODQ_BL_OFFSET +
							ch * DDRIODQ_CH_OFFSET,
							0, 1 << 8);
					}
					/* initialize the starting delay to 128 PI (cas +1 CLK) */
					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl++) {
						/* 1x CLK domain timing is cas-4 */
						delay[bl] = (4 + 1) * FULL_CLK;

						set_rcvn(ch, rk, bl, delay[bl]);
					}

					/* now find the rising edge */
					find_rising_edge(mrc_params, delay, ch, rk, true);

					/* Now increase delay by 32 PI (1/4 CLK) to place in center of high pulse */
					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl++) {
						delay[bl] += QRTR_CLK;
						set_rcvn(ch, rk, bl, delay[bl]);
					}
					/* Now decrement delay by 128 PI (1 CLK) until we sample a "0" */
					do {
						temp = sample_dqs(mrc_params, ch, rk, true);
						for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl++) {
							if (temp & (1 << bl)) {
								if (delay[bl] >= FULL_CLK) {
									delay[bl] -= FULL_CLK;
									set_rcvn(ch, rk, bl, delay[bl]);
								} else {
									/* not enough delay */
									training_message(ch, rk, bl);
									mrc_post_code(0xee, 0x50);
								}
							}
						}
					} while (temp & 0xff);

#ifdef R2R_SHARING
					/* increment "num_ranks_enabled" */
					num_ranks_enabled++;
					/* Finally increment delay by 32 PI (1/4 CLK) to place in center of preamble */
					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl++) {
						delay[bl] += QRTR_CLK;
						/* add "delay[]" values to "final_delay[][]" for rolling average */
						final_delay[ch][bl] += delay[bl];
						/* set timing based on rolling average values */
						set_rcvn(ch, rk, bl, final_delay[ch][bl] / num_ranks_enabled);
					}
#else
					/* Finally increment delay by 32 PI (1/4 CLK) to place in center of preamble */
					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl++) {
						delay[bl] += QRTR_CLK;
						set_rcvn(ch, rk, bl, delay[bl]);
					}
#endif

					/* disable FIFORST */
					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl += 2) {
						mrc_alt_write_mask(DDRPHY,
							B01PTRCTL1 +
							(bl >> 1) * DDRIODQ_BL_OFFSET +
							ch * DDRIODQ_CH_OFFSET,
							1 << 8, 1 << 8);
					}
#endif
				}
			}
		}
	}

#ifndef BACKUP_RCVN
	/* restore original */
	msg_port_write(MEM_CTLR, DTR1, dtr1_save);
#endif

	LEAVEFN();
}

/*
 * This function will perform the Write Levelling algorithm
 * (align WCLK and WDQS).
 *
 * This algorithm will act on each rank in each channel separately.
 */
void wr_level(struct mrc_params *mrc_params)
{
	uint8_t ch;	/* channel counter */
	uint8_t rk;	/* rank counter */
	uint8_t bl;	/* byte lane counter */
	uint8_t bl_divisor = (mrc_params->channel_width == X16) ? 2 : 1;

#ifdef R2R_SHARING
	/* used to find placement for rank2rank sharing configs */
	uint32_t final_delay[NUM_CHANNELS][NUM_BYTE_LANES];
#ifndef BACKUP_WDQS
	/* used to find placement for rank2rank sharing configs */
	uint32_t num_ranks_enabled = 0;
#endif
#endif

#ifdef BACKUP_WDQS
#else
	/* determines stop condition for CRS_WR_LVL */
	bool all_edges_found;
	/* absolute PI value to be programmed on the byte lane */
	uint32_t delay[NUM_BYTE_LANES];
	/*
	 * static makes it so the data is loaded in the heap once by shadow(),
	 * where non-static copies the data onto the stack every time this
	 * function is called
	 */
	uint32_t address;	/* address to be checked during COARSE_WR_LVL */
	u32 dtr4, dtr4_save;
#endif

	ENTERFN();

	/* wr_level starts */
	mrc_post_code(0x06, 0x00);

#ifdef R2R_SHARING
	/* need to set "final_delay[][]" elements to "0" */
	memset((void *)(final_delay), 0x00, (size_t)sizeof(final_delay));
#endif

	/* loop through each enabled channel */
	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			/* perform WRITE LEVELING algorithm on a per rank basis */
			for (rk = 0; rk < NUM_RANKS; rk++) {
				if (mrc_params->rank_enables & (1 << rk)) {
					/*
					 * POST_CODE here indicates the current
					 * rank and channel being calibrated
					 */
					mrc_post_code(0x06, 0x10 + ((ch << 4) | rk));

#ifdef BACKUP_WDQS
					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl++) {
						set_wdqs(ch, rk, bl, ddr_wdqs[PLATFORM_ID]);
						set_wdq(ch, rk, bl, ddr_wdqs[PLATFORM_ID] - QRTR_CLK);
					}
#else
					/*
					 * perform a single PRECHARGE_ALL command to
					 * make DRAM state machine go to IDLE state
					 */
					dram_init_command(DCMD_PREA(rk));

					/*
					 * enable Write Levelling Mode
					 * (EMRS1 w/ Write Levelling Mode Enable)
					 */
					dram_init_command(DCMD_MRS1(rk, 0x82));

					/*
					 * set ODT DRAM Full Time Termination
					 * disable in MCU
					 */

					dtr4 = msg_port_read(MEM_CTLR, DTR4);
					dtr4_save = dtr4;
					dtr4 |= DTR4_ODTDIS;
					msg_port_write(MEM_CTLR, DTR4, dtr4);

					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor) / 2; bl++) {
						/*
						 * Enable Sandy Bridge Mode (WDQ Tri-State) &
						 * Ensure 5 WDQS pulses during Write Leveling
						 */
						mrc_alt_write_mask(DDRPHY,
							DQCTL + DDRIODQ_BL_OFFSET * bl + DDRIODQ_CH_OFFSET * ch,
							0x10000154,
							0x100003fc);
					}

					/* Write Leveling Mode enabled in IO */
					mrc_alt_write_mask(DDRPHY,
						CCDDR3RESETCTL + DDRIOCCC_CH_OFFSET * ch,
						1 << 16, 1 << 16);

					/* Initialize the starting delay to WCLK */
					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl++) {
						/*
						 * CLK0 --> RK0
						 * CLK1 --> RK1
						 */
						delay[bl] = get_wclk(ch, rk);

						set_wdqs(ch, rk, bl, delay[bl]);
					}

					/* now find the rising edge */
					find_rising_edge(mrc_params, delay, ch, rk, false);

					/* disable Write Levelling Mode */
					mrc_alt_write_mask(DDRPHY,
						CCDDR3RESETCTL + DDRIOCCC_CH_OFFSET * ch,
						0, 1 << 16);

					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor) / 2; bl++) {
						/* Disable Sandy Bridge Mode & Ensure 4 WDQS pulses during normal operation */
						mrc_alt_write_mask(DDRPHY,
							DQCTL + DDRIODQ_BL_OFFSET * bl + DDRIODQ_CH_OFFSET * ch,
							0x00000154,
							0x100003fc);
					}

					/* restore original DTR4 */
					msg_port_write(MEM_CTLR, DTR4, dtr4_save);

					/*
					 * restore original value
					 * (Write Levelling Mode Disable)
					 */
					dram_init_command(DCMD_MRS1(rk, mrc_params->mrs1));

					/*
					 * perform a single PRECHARGE_ALL command to
					 * make DRAM state machine go to IDLE state
					 */
					dram_init_command(DCMD_PREA(rk));

					mrc_post_code(0x06, 0x30 + ((ch << 4) | rk));

					/*
					 * COARSE WRITE LEVEL:
					 * check that we're on the correct clock edge
					 */

					/* hte reconfiguration request */
					mrc_params->hte_setup = 1;

					/* start CRS_WR_LVL with WDQS = WDQS + 128 PI */
					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl++) {
						delay[bl] = get_wdqs(ch, rk, bl) + FULL_CLK;
						set_wdqs(ch, rk, bl, delay[bl]);
						/*
						 * program WDQ timings based on WDQS
						 * (WDQ = WDQS - 32 PI)
						 */
						set_wdq(ch, rk, bl, (delay[bl] - QRTR_CLK));
					}

					/* get an address in the targeted channel/rank */
					address = get_addr(ch, rk);
					do {
						uint32_t coarse_result = 0x00;
						uint32_t coarse_result_mask = byte_lane_mask(mrc_params);
						/* assume pass */
						all_edges_found = true;

						mrc_params->hte_setup = 1;
						coarse_result = check_rw_coarse(mrc_params, address);

						/* check for failures and margin the byte lane back 128 PI (1 CLK) */
						for (bl = 0; bl < NUM_BYTE_LANES / bl_divisor; bl++) {
							if (coarse_result & (coarse_result_mask << bl)) {
								all_edges_found = false;
								delay[bl] -= FULL_CLK;
								set_wdqs(ch, rk, bl, delay[bl]);
								/* program WDQ timings based on WDQS (WDQ = WDQS - 32 PI) */
								set_wdq(ch, rk, bl, delay[bl] - QRTR_CLK);
							}
						}
					} while (!all_edges_found);

#ifdef R2R_SHARING
					/* increment "num_ranks_enabled" */
					 num_ranks_enabled++;
					/* accumulate "final_delay[][]" values from "delay[]" values for rolling average */
					for (bl = 0; bl < NUM_BYTE_LANES / bl_divisor; bl++) {
						final_delay[ch][bl] += delay[bl];
						set_wdqs(ch, rk, bl, final_delay[ch][bl] / num_ranks_enabled);
						/* program WDQ timings based on WDQS (WDQ = WDQS - 32 PI) */
						set_wdq(ch, rk, bl, final_delay[ch][bl] / num_ranks_enabled - QRTR_CLK);
					}
#endif
#endif
				}
			}
		}
	}

	LEAVEFN();
}

void prog_page_ctrl(struct mrc_params *mrc_params)
{
	u32 dpmc0;

	ENTERFN();

	dpmc0 = msg_port_read(MEM_CTLR, DPMC0);
	dpmc0 &= ~DPMC0_PCLSTO_MASK;
	dpmc0 |= (4 << 16);
	dpmc0 |= DPMC0_PREAPWDEN;
	msg_port_write(MEM_CTLR, DPMC0, dpmc0);
}

/*
 * This function will perform the READ TRAINING Algorithm on all
 * channels/ranks/byte_lanes simultaneously to minimize execution time.
 *
 * The idea here is to train the VREF and RDQS (and eventually RDQ) values
 * to achieve maximum READ margins. The algorithm will first determine the
 * X coordinate (RDQS setting). This is done by collapsing the VREF eye
 * until we find a minimum required RDQS eye for VREF_MIN and VREF_MAX.
 * Then we take the averages of the RDQS eye at VREF_MIN and VREF_MAX,
 * then average those; this will be the final X coordinate. The algorithm
 * will then determine the Y coordinate (VREF setting). This is done by
 * collapsing the RDQS eye until we find a minimum required VREF eye for
 * RDQS_MIN and RDQS_MAX. Then we take the averages of the VREF eye at
 * RDQS_MIN and RDQS_MAX, then average those; this will be the final Y
 * coordinate.
 *
 * NOTE: this algorithm assumes the eye curves have a one-to-one relationship,
 * meaning for each X the curve has only one Y and vice-a-versa.
 */
void rd_train(struct mrc_params *mrc_params)
{
	uint8_t ch;	/* channel counter */
	uint8_t rk;	/* rank counter */
	uint8_t bl;	/* byte lane counter */
	uint8_t bl_divisor = (mrc_params->channel_width == X16) ? 2 : 1;
#ifdef BACKUP_RDQS
#else
	uint8_t side_x;	/* tracks LEFT/RIGHT approach vectors */
	uint8_t side_y;	/* tracks BOTTOM/TOP approach vectors */
	/* X coordinate data (passing RDQS values) for approach vectors */
	uint8_t x_coordinate[2][2][NUM_CHANNELS][NUM_RANKS][NUM_BYTE_LANES];
	/* Y coordinate data (passing VREF values) for approach vectors */
	uint8_t y_coordinate[2][2][NUM_CHANNELS][NUM_BYTE_LANES];
	/* centered X (RDQS) */
	uint8_t x_center[NUM_CHANNELS][NUM_RANKS][NUM_BYTE_LANES];
	/* centered Y (VREF) */
	uint8_t y_center[NUM_CHANNELS][NUM_BYTE_LANES];
	uint32_t address;	/* target address for check_bls_ex() */
	uint32_t result;	/* result of check_bls_ex() */
	uint32_t bl_mask;	/* byte lane mask for result checking */
#ifdef R2R_SHARING
	/* used to find placement for rank2rank sharing configs */
	uint32_t final_delay[NUM_CHANNELS][NUM_BYTE_LANES];
	/* used to find placement for rank2rank sharing configs */
	uint32_t num_ranks_enabled = 0;
#endif
#endif

	/* rd_train starts */
	mrc_post_code(0x07, 0x00);

	ENTERFN();

#ifdef BACKUP_RDQS
	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			for (rk = 0; rk < NUM_RANKS; rk++) {
				if (mrc_params->rank_enables & (1 << rk)) {
					for (bl = 0;
					     bl < NUM_BYTE_LANES / bl_divisor;
					     bl++) {
						set_rdqs(ch, rk, bl, ddr_rdqs[PLATFORM_ID]);
					}
				}
			}
		}
	}
#else
	/* initialize x/y_coordinate arrays */
	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			for (rk = 0; rk < NUM_RANKS; rk++) {
				if (mrc_params->rank_enables & (1 << rk)) {
					for (bl = 0;
					     bl < NUM_BYTE_LANES / bl_divisor;
					     bl++) {
						/* x_coordinate */
						x_coordinate[L][B][ch][rk][bl] = RDQS_MIN;
						x_coordinate[R][B][ch][rk][bl] = RDQS_MAX;
						x_coordinate[L][T][ch][rk][bl] = RDQS_MIN;
						x_coordinate[R][T][ch][rk][bl] = RDQS_MAX;
						/* y_coordinate */
						y_coordinate[L][B][ch][bl] = VREF_MIN;
						y_coordinate[R][B][ch][bl] = VREF_MIN;
						y_coordinate[L][T][ch][bl] = VREF_MAX;
						y_coordinate[R][T][ch][bl] = VREF_MAX;
					}
				}
			}
		}
	}

	/* initialize other variables */
	bl_mask = byte_lane_mask(mrc_params);
	address = get_addr(0, 0);

#ifdef R2R_SHARING
	/* need to set "final_delay[][]" elements to "0" */
	memset((void *)(final_delay), 0x00, (size_t)sizeof(final_delay));
#endif

	/* look for passing coordinates */
	for (side_y = B; side_y <= T; side_y++) {
		for (side_x = L; side_x <= R; side_x++) {
			mrc_post_code(0x07, 0x10 + side_y * 2 + side_x);

			/* find passing values */
			for (ch = 0; ch < NUM_CHANNELS; ch++) {
				if (mrc_params->channel_enables & (0x1 << ch)) {
					for (rk = 0; rk < NUM_RANKS; rk++) {
						if (mrc_params->rank_enables &
							(0x1 << rk)) {
							/* set x/y_coordinate search starting settings */
							for (bl = 0;
							     bl < NUM_BYTE_LANES / bl_divisor;
							     bl++) {
								set_rdqs(ch, rk, bl,
									 x_coordinate[side_x][side_y][ch][rk][bl]);
								set_vref(ch, bl,
									 y_coordinate[side_x][side_y][ch][bl]);
							}

							/* get an address in the target channel/rank */
							address = get_addr(ch, rk);

							/* request HTE reconfiguration */
							mrc_params->hte_setup = 1;

							/* test the settings */
							do {
								/* result[07:00] == failing byte lane (MAX 8) */
								result = check_bls_ex(mrc_params, address);

								/* check for failures */
								if (result & 0xff) {
									/* at least 1 byte lane failed */
									for (bl = 0; bl < NUM_BYTE_LANES / bl_divisor; bl++) {
										if (result &
											(bl_mask << bl)) {
											/* adjust the RDQS values accordingly */
											if (side_x == L)
												x_coordinate[L][side_y][ch][rk][bl] += RDQS_STEP;
											else
												x_coordinate[R][side_y][ch][rk][bl] -= RDQS_STEP;

											/* check that we haven't closed the RDQS_EYE too much */
											if ((x_coordinate[L][side_y][ch][rk][bl] > (RDQS_MAX - MIN_RDQS_EYE)) ||
												(x_coordinate[R][side_y][ch][rk][bl] < (RDQS_MIN + MIN_RDQS_EYE)) ||
												(x_coordinate[L][side_y][ch][rk][bl] ==
												x_coordinate[R][side_y][ch][rk][bl])) {
												/*
												 * not enough RDQS margin available at this VREF
												 * update VREF values accordingly
												 */
												if (side_y == B)
													y_coordinate[side_x][B][ch][bl] += VREF_STEP;
												else
													y_coordinate[side_x][T][ch][bl] -= VREF_STEP;

												/* check that we haven't closed the VREF_EYE too much */
												if ((y_coordinate[side_x][B][ch][bl] > (VREF_MAX - MIN_VREF_EYE)) ||
													(y_coordinate[side_x][T][ch][bl] < (VREF_MIN + MIN_VREF_EYE)) ||
													(y_coordinate[side_x][B][ch][bl] == y_coordinate[side_x][T][ch][bl])) {
													/* VREF_EYE collapsed below MIN_VREF_EYE */
													training_message(ch, rk, bl);
													mrc_post_code(0xEE, 0x70 + side_y * 2 + side_x);
												} else {
													/* update the VREF setting */
													set_vref(ch, bl, y_coordinate[side_x][side_y][ch][bl]);
													/* reset the X coordinate to begin the search at the new VREF */
													x_coordinate[side_x][side_y][ch][rk][bl] =
														(side_x == L) ? RDQS_MIN : RDQS_MAX;
												}
											}

											/* update the RDQS setting */
											set_rdqs(ch, rk, bl, x_coordinate[side_x][side_y][ch][rk][bl]);
										}
									}
								}
							} while (result & 0xff);
						}
					}
				}
			}
		}
	}

	mrc_post_code(0x07, 0x20);

	/* find final RDQS (X coordinate) & final VREF (Y coordinate) */
	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			for (rk = 0; rk < NUM_RANKS; rk++) {
				if (mrc_params->rank_enables & (1 << rk)) {
					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl++) {
						uint32_t temp1;
						uint32_t temp2;

						/* x_coordinate */
						DPF(D_INFO,
						    "RDQS T/B eye rank%d lane%d : %d-%d %d-%d\n",
						    rk, bl,
						    x_coordinate[L][T][ch][rk][bl],
						    x_coordinate[R][T][ch][rk][bl],
						    x_coordinate[L][B][ch][rk][bl],
						    x_coordinate[R][B][ch][rk][bl]);

						/* average the TOP side LEFT & RIGHT values */
						temp1 = (x_coordinate[R][T][ch][rk][bl] + x_coordinate[L][T][ch][rk][bl]) / 2;
						/* average the BOTTOM side LEFT & RIGHT values */
						temp2 = (x_coordinate[R][B][ch][rk][bl] + x_coordinate[L][B][ch][rk][bl]) / 2;
						/* average the above averages */
						x_center[ch][rk][bl] = (uint8_t) ((temp1 + temp2) / 2);

						/* y_coordinate */
						DPF(D_INFO,
						    "VREF R/L eye lane%d : %d-%d %d-%d\n",
						    bl,
						    y_coordinate[R][B][ch][bl],
						    y_coordinate[R][T][ch][bl],
						    y_coordinate[L][B][ch][bl],
						    y_coordinate[L][T][ch][bl]);

						/* average the RIGHT side TOP & BOTTOM values */
						temp1 = (y_coordinate[R][T][ch][bl] + y_coordinate[R][B][ch][bl]) / 2;
						/* average the LEFT side TOP & BOTTOM values */
						temp2 = (y_coordinate[L][T][ch][bl] + y_coordinate[L][B][ch][bl]) / 2;
						/* average the above averages */
						y_center[ch][bl] = (uint8_t) ((temp1 + temp2) / 2);
					}
				}
			}
		}
	}

#ifdef RX_EYE_CHECK
	/* perform an eye check */
	for (side_y = B; side_y <= T; side_y++) {
		for (side_x = L; side_x <= R; side_x++) {
			mrc_post_code(0x07, 0x30 + side_y * 2 + side_x);

			/* update the settings for the eye check */
			for (ch = 0; ch < NUM_CHANNELS; ch++) {
				if (mrc_params->channel_enables & (1 << ch)) {
					for (rk = 0; rk < NUM_RANKS; rk++) {
						if (mrc_params->rank_enables & (1 << rk)) {
							for (bl = 0; bl < NUM_BYTE_LANES / bl_divisor; bl++) {
								if (side_x == L)
									set_rdqs(ch, rk, bl, x_center[ch][rk][bl] - (MIN_RDQS_EYE / 2));
								else
									set_rdqs(ch, rk, bl, x_center[ch][rk][bl] + (MIN_RDQS_EYE / 2));

								if (side_y == B)
									set_vref(ch, bl, y_center[ch][bl] - (MIN_VREF_EYE / 2));
								else
									set_vref(ch, bl, y_center[ch][bl] + (MIN_VREF_EYE / 2));
							}
						}
					}
				}
			}

			/* request HTE reconfiguration */
			mrc_params->hte_setup = 1;

			/* check the eye */
			if (check_bls_ex(mrc_params, address) & 0xff) {
				/* one or more byte lanes failed */
				mrc_post_code(0xee, 0x74 + side_x * 2 + side_y);
			}
		}
	}
#endif

	mrc_post_code(0x07, 0x40);

	/* set final placements */
	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			for (rk = 0; rk < NUM_RANKS; rk++) {
				if (mrc_params->rank_enables & (1 << rk)) {
#ifdef R2R_SHARING
					/* increment "num_ranks_enabled" */
					num_ranks_enabled++;
#endif
					for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl++) {
						/* x_coordinate */
#ifdef R2R_SHARING
						final_delay[ch][bl] += x_center[ch][rk][bl];
						set_rdqs(ch, rk, bl, final_delay[ch][bl] / num_ranks_enabled);
#else
						set_rdqs(ch, rk, bl, x_center[ch][rk][bl]);
#endif
						/* y_coordinate */
						set_vref(ch, bl, y_center[ch][bl]);
					}
				}
			}
		}
	}
#endif

	LEAVEFN();
}

/*
 * This function will perform the WRITE TRAINING Algorithm on all
 * channels/ranks/byte_lanes simultaneously to minimize execution time.
 *
 * The idea here is to train the WDQ timings to achieve maximum WRITE margins.
 * The algorithm will start with WDQ at the current WDQ setting (tracks WDQS
 * in WR_LVL) +/- 32 PIs (+/- 1/4 CLK) and collapse the eye until all data
 * patterns pass. This is because WDQS will be aligned to WCLK by the
 * Write Leveling algorithm and WDQ will only ever have a 1/2 CLK window
 * of validity.
 */
void wr_train(struct mrc_params *mrc_params)
{
	uint8_t ch;	/* channel counter */
	uint8_t rk;	/* rank counter */
	uint8_t bl;	/* byte lane counter */
	uint8_t bl_divisor = (mrc_params->channel_width == X16) ? 2 : 1;
#ifdef BACKUP_WDQ
#else
	uint8_t side;		/* LEFT/RIGHT side indicator (0=L, 1=R) */
	uint32_t temp;		/* temporary DWORD */
	/* 2 arrays, for L & R side passing delays */
	uint32_t delay[2][NUM_CHANNELS][NUM_RANKS][NUM_BYTE_LANES];
	uint32_t address;	/* target address for check_bls_ex() */
	uint32_t result;	/* result of check_bls_ex() */
	uint32_t bl_mask;	/* byte lane mask for result checking */
#ifdef R2R_SHARING
	/* used to find placement for rank2rank sharing configs */
	uint32_t final_delay[NUM_CHANNELS][NUM_BYTE_LANES];
	/* used to find placement for rank2rank sharing configs */
	uint32_t num_ranks_enabled = 0;
#endif
#endif

	/* wr_train starts */
	mrc_post_code(0x08, 0x00);

	ENTERFN();

#ifdef BACKUP_WDQ
	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			for (rk = 0; rk < NUM_RANKS; rk++) {
				if (mrc_params->rank_enables & (1 << rk)) {
					for (bl = 0;
					     bl < NUM_BYTE_LANES / bl_divisor;
					     bl++) {
						set_wdq(ch, rk, bl, ddr_wdq[PLATFORM_ID]);
					}
				}
			}
		}
	}
#else
	/* initialize "delay" */
	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			for (rk = 0; rk < NUM_RANKS; rk++) {
				if (mrc_params->rank_enables & (1 << rk)) {
					for (bl = 0;
					     bl < NUM_BYTE_LANES / bl_divisor;
					     bl++) {
						/*
						 * want to start with
						 * WDQ = (WDQS - QRTR_CLK)
						 * +/- QRTR_CLK
						 */
						temp = get_wdqs(ch, rk, bl) - QRTR_CLK;
						delay[L][ch][rk][bl] = temp - QRTR_CLK;
						delay[R][ch][rk][bl] = temp + QRTR_CLK;
					}
				}
			}
		}
	}

	/* initialize other variables */
	bl_mask = byte_lane_mask(mrc_params);
	address = get_addr(0, 0);

#ifdef R2R_SHARING
	/* need to set "final_delay[][]" elements to "0" */
	memset((void *)(final_delay), 0x00, (size_t)sizeof(final_delay));
#endif

	/*
	 * start algorithm on the LEFT side and train each channel/bl
	 * until no failures are observed, then repeat for the RIGHT side.
	 */
	for (side = L; side <= R; side++) {
		mrc_post_code(0x08, 0x10 + side);

		/* set starting values */
		for (ch = 0; ch < NUM_CHANNELS; ch++) {
			if (mrc_params->channel_enables & (1 << ch)) {
				for (rk = 0; rk < NUM_RANKS; rk++) {
					if (mrc_params->rank_enables &
						(1 << rk)) {
						for (bl = 0;
						     bl < NUM_BYTE_LANES / bl_divisor;
						     bl++) {
							set_wdq(ch, rk, bl, delay[side][ch][rk][bl]);
						}
					}
				}
			}
		}

		/* find passing values */
		for (ch = 0; ch < NUM_CHANNELS; ch++) {
			if (mrc_params->channel_enables & (1 << ch)) {
				for (rk = 0; rk < NUM_RANKS; rk++) {
					if (mrc_params->rank_enables &
						(1 << rk)) {
						/* get an address in the target channel/rank */
						address = get_addr(ch, rk);

						/* request HTE reconfiguration */
						mrc_params->hte_setup = 1;

						/* check the settings */
						do {
							/* result[07:00] == failing byte lane (MAX 8) */
							result = check_bls_ex(mrc_params, address);
							/* check for failures */
							if (result & 0xff) {
								/* at least 1 byte lane failed */
								for (bl = 0; bl < NUM_BYTE_LANES / bl_divisor; bl++) {
									if (result &
										(bl_mask << bl)) {
										if (side == L)
											delay[L][ch][rk][bl] += WDQ_STEP;
										else
											delay[R][ch][rk][bl] -= WDQ_STEP;

										/* check for algorithm failure */
										if (delay[L][ch][rk][bl] != delay[R][ch][rk][bl]) {
											/*
											 * margin available
											 * update delay setting
											 */
											set_wdq(ch, rk, bl,
												delay[side][ch][rk][bl]);
										} else {
											/*
											 * no margin available
											 * notify the user and halt
											 */
											training_message(ch, rk, bl);
											mrc_post_code(0xee, 0x80 + side);
										}
									}
								}
							}
						/* stop when all byte lanes pass */
						} while (result & 0xff);
					}
				}
			}
		}
	}

	/* program WDQ to the middle of passing window */
	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		if (mrc_params->channel_enables & (1 << ch)) {
			for (rk = 0; rk < NUM_RANKS; rk++) {
				if (mrc_params->rank_enables & (1 << rk)) {
#ifdef R2R_SHARING
					/* increment "num_ranks_enabled" */
					num_ranks_enabled++;
#endif
					for (bl = 0; bl < NUM_BYTE_LANES / bl_divisor; bl++) {
						DPF(D_INFO,
						    "WDQ eye rank%d lane%d : %d-%d\n",
						    rk, bl,
						    delay[L][ch][rk][bl],
						    delay[R][ch][rk][bl]);

						temp = (delay[R][ch][rk][bl] + delay[L][ch][rk][bl]) / 2;

#ifdef R2R_SHARING
						final_delay[ch][bl] += temp;
						set_wdq(ch, rk, bl,
							final_delay[ch][bl] / num_ranks_enabled);
#else
						set_wdq(ch, rk, bl, temp);
#endif
					}
				}
			}
		}
	}
#endif

	LEAVEFN();
}

/*
 * This function will store relevant timing data
 *
 * This data will be used on subsequent boots to speed up boot times
 * and is required for Suspend To RAM capabilities.
 */
void store_timings(struct mrc_params *mrc_params)
{
	uint8_t ch, rk, bl;
	struct mrc_timings *mt = &mrc_params->timings;

	for (ch = 0; ch < NUM_CHANNELS; ch++) {
		for (rk = 0; rk < NUM_RANKS; rk++) {
			for (bl = 0; bl < NUM_BYTE_LANES; bl++) {
				mt->rcvn[ch][rk][bl] = get_rcvn(ch, rk, bl);
				mt->rdqs[ch][rk][bl] = get_rdqs(ch, rk, bl);
				mt->wdqs[ch][rk][bl] = get_wdqs(ch, rk, bl);
				mt->wdq[ch][rk][bl] = get_wdq(ch, rk, bl);

				if (rk == 0)
					mt->vref[ch][bl] = get_vref(ch, bl);
			}

			mt->wctl[ch][rk] = get_wctl(ch, rk);
		}

		mt->wcmd[ch] = get_wcmd(ch);
	}

	/* need to save for a case of changing frequency after warm reset */
	mt->ddr_speed = mrc_params->ddr_speed;
}

/*
 * The purpose of this function is to ensure the SEC comes out of reset
 * and IA initiates the SEC enabling Memory Scrambling.
 */
void enable_scrambling(struct mrc_params *mrc_params)
{
	uint32_t lfsr = 0;
	uint8_t i;

	if (mrc_params->scrambling_enables == 0)
		return;

	ENTERFN();

	/* 32 bit seed is always stored in BIOS NVM */
	lfsr = mrc_params->timings.scrambler_seed;

	if (mrc_params->boot_mode == BM_COLD) {
		/*
		 * factory value is 0 and in first boot,
		 * a clock based seed is loaded.
		 */
		if (lfsr == 0) {
			/*
			 * get seed from system clock
			 * and make sure it is not all 1's
			 */
			lfsr = rdtsc() & 0x0fffffff;
		} else {
			/*
			 * Need to replace scrambler
			 *
			 * get next 32bit LFSR 16 times which is the last
			 * part of the previous scrambler vector
			 */
			for (i = 0; i < 16; i++)
				lfsr32(&lfsr);
		}

		/* save new seed */
		mrc_params->timings.scrambler_seed = lfsr;
	}

	/*
	 * In warm boot or S3 exit, we have the previous seed.
	 * In cold boot, we have the last 32bit LFSR which is the new seed.
	 */
	lfsr32(&lfsr);	/* shift to next value */
	msg_port_write(MEM_CTLR, SCRMSEED, (lfsr & 0x0003ffff));

	for (i = 0; i < 2; i++)
		msg_port_write(MEM_CTLR, SCRMLO + i, (lfsr & 0xaaaaaaaa));

	LEAVEFN();
}

/*
 * Configure MCU Power Management Control Register
 * and Scheduler Control Register
 */
void prog_ddr_control(struct mrc_params *mrc_params)
{
	u32 dsch;
	u32 dpmc0;

	ENTERFN();

	dsch = msg_port_read(MEM_CTLR, DSCH);
	dsch &= ~(DSCH_OOODIS | DSCH_OOOST3DIS | DSCH_NEWBYPDIS);
	msg_port_write(MEM_CTLR, DSCH, dsch);

	dpmc0 = msg_port_read(MEM_CTLR, DPMC0);
	dpmc0 &= ~DPMC0_DISPWRDN;
	dpmc0 |= (mrc_params->power_down_disable << 25);
	dpmc0 &= ~DPMC0_CLKGTDIS;
	dpmc0 &= ~DPMC0_PCLSTO_MASK;
	dpmc0 |= (4 << 16);
	dpmc0 |= DPMC0_PREAPWDEN;
	msg_port_write(MEM_CTLR, DPMC0, dpmc0);

	/* CMDTRIST = 2h - CMD/ADDR are tristated when no valid command */
	mrc_write_mask(MEM_CTLR, DPMC1, 0x20, 0x30);

	LEAVEFN();
}

/*
 * After training complete configure MCU Rank Population Register
 * specifying: ranks enabled, device width, density, address mode
 */
void prog_dra_drb(struct mrc_params *mrc_params)
{
	u32 drp;
	u32 dco;
	u8 density = mrc_params->params.density;

	ENTERFN();

	dco = msg_port_read(MEM_CTLR, DCO);
	dco &= ~DCO_IC;
	msg_port_write(MEM_CTLR, DCO, dco);

	drp = 0;
	if (mrc_params->rank_enables & 1)
		drp |= DRP_RKEN0;
	if (mrc_params->rank_enables & 2)
		drp |= DRP_RKEN1;
	if (mrc_params->dram_width == X16) {
		drp |= (1 << 4);
		drp |= (1 << 9);
	}

	/*
	 * Density encoding in struct dram_params: 0=512Mb, 1=Gb, 2=2Gb, 3=4Gb
	 * has to be mapped RANKDENSx encoding (0=1Gb)
	 */
	if (density == 0)
		density = 4;

	drp |= ((density - 1) << 6);
	drp |= ((density - 1) << 11);

	/* Address mode can be overwritten if ECC enabled */
	drp |= (mrc_params->address_mode << 14);

	msg_port_write(MEM_CTLR, DRP, drp);

	dco &= ~DCO_PMICTL;
	dco |= DCO_IC;
	msg_port_write(MEM_CTLR, DCO, dco);

	LEAVEFN();
}

/* Send DRAM wake command */
void perform_wake(struct mrc_params *mrc_params)
{
	ENTERFN();

	dram_wake_command();

	LEAVEFN();
}

/*
 * Configure refresh rate and short ZQ calibration interval
 * Activate dynamic self refresh
 */
void change_refresh_period(struct mrc_params *mrc_params)
{
	u32 drfc;
	u32 dcal;
	u32 dpmc0;

	ENTERFN();

	drfc = msg_port_read(MEM_CTLR, DRFC);
	drfc &= ~DRFC_TREFI_MASK;
	drfc |= (mrc_params->refresh_rate << 12);
	drfc |= DRFC_REFDBTCLR;
	msg_port_write(MEM_CTLR, DRFC, drfc);

	dcal = msg_port_read(MEM_CTLR, DCAL);
	dcal &= ~DCAL_ZQCINT_MASK;
	dcal |= (3 << 8);	/* 63ms */
	msg_port_write(MEM_CTLR, DCAL, dcal);

	dpmc0 = msg_port_read(MEM_CTLR, DPMC0);
	dpmc0 |= (DPMC0_DYNSREN | DPMC0_ENPHYCLKGATE);
	msg_port_write(MEM_CTLR, DPMC0, dpmc0);

	LEAVEFN();
}

/*
 * Configure DDRPHY for Auto-Refresh, Periodic Compensations,
 * Dynamic Diff-Amp, ZQSPERIOD, Auto-Precharge, CKE Power-Down
 */
void set_auto_refresh(struct mrc_params *mrc_params)
{
	uint32_t channel;
	uint32_t rank;
	uint32_t bl;
	uint32_t bl_divisor = 1;
	uint32_t temp;

	ENTERFN();

	/*
	 * Enable Auto-Refresh, Periodic Compensations, Dynamic Diff-Amp,
	 * ZQSPERIOD, Auto-Precharge, CKE Power-Down
	 */
	for (channel = 0; channel < NUM_CHANNELS; channel++) {
		if (mrc_params->channel_enables & (1 << channel)) {
			/* Enable Periodic RCOMPS */
			mrc_alt_write_mask(DDRPHY, CMPCTRL, 2, 2);

			/* Enable Dynamic DiffAmp & Set Read ODT Value */
			switch (mrc_params->rd_odt_value) {
			case 0:
				temp = 0x3f;	/* OFF */
				break;
			default:
				temp = 0x00;	/* Auto */
				break;
			}

			for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor) / 2; bl++) {
				/* Override: DIFFAMP, ODT */
				mrc_alt_write_mask(DDRPHY,
					B0OVRCTL + bl * DDRIODQ_BL_OFFSET +
					channel * DDRIODQ_CH_OFFSET,
					temp << 10,
					0x003ffc00);

				/* Override: DIFFAMP, ODT */
				mrc_alt_write_mask(DDRPHY,
					B1OVRCTL + bl * DDRIODQ_BL_OFFSET +
					channel * DDRIODQ_CH_OFFSET,
					temp << 10,
					0x003ffc00);
			}

			/* Issue ZQCS command */
			for (rank = 0; rank < NUM_RANKS; rank++) {
				if (mrc_params->rank_enables & (1 << rank))
					dram_init_command(DCMD_ZQCS(rank));
			}
		}
	}

	clear_pointers();

	LEAVEFN();
}

/*
 * Depending on configuration enables ECC support
 *
 * Available memory size is decreased, and updated with 0s
 * in order to clear error status. Address mode 2 forced.
 */
void ecc_enable(struct mrc_params *mrc_params)
{
	u32 drp;
	u32 dsch;
	u32 ecc_ctrl;

	if (mrc_params->ecc_enables == 0)
		return;

	ENTERFN();

	/* Configuration required in ECC mode */
	drp = msg_port_read(MEM_CTLR, DRP);
	drp &= ~DRP_ADDRMAP_MASK;
	drp |= DRP_ADDRMAP_MAP1;
	drp |= DRP_PRI64BSPLITEN;
	msg_port_write(MEM_CTLR, DRP, drp);

	/* Disable new request bypass */
	dsch = msg_port_read(MEM_CTLR, DSCH);
	dsch |= DSCH_NEWBYPDIS;
	msg_port_write(MEM_CTLR, DSCH, dsch);

	/* Enable ECC */
	ecc_ctrl = (DECCCTRL_SBEEN | DECCCTRL_DBEEN | DECCCTRL_ENCBGEN);
	msg_port_write(MEM_CTLR, DECCCTRL, ecc_ctrl);

	/* Assume 8 bank memory, one bank is gone for ECC */
	mrc_params->mem_size -= mrc_params->mem_size / 8;

	/* For S3 resume memory content has to be preserved */
	if (mrc_params->boot_mode != BM_S3) {
		select_hte();
		hte_mem_init(mrc_params, MRC_MEM_INIT);
		select_mem_mgr();
	}

	LEAVEFN();
}

/*
 * Execute memory test
 * if error detected it is indicated in mrc_params->status
 */
void memory_test(struct mrc_params *mrc_params)
{
	uint32_t result = 0;

	ENTERFN();

	select_hte();
	result = hte_mem_init(mrc_params, MRC_MEM_TEST);
	select_mem_mgr();

	DPF(D_INFO, "Memory test result %x\n", result);
	mrc_params->status = ((result == 0) ? MRC_SUCCESS : MRC_E_MEMTEST);
	LEAVEFN();
}

/* Lock MCU registers at the end of initialization sequence */
void lock_registers(struct mrc_params *mrc_params)
{
	u32 dco;

	ENTERFN();

	dco = msg_port_read(MEM_CTLR, DCO);
	dco &= ~(DCO_PMICTL | DCO_PMIDIS);
	dco |= (DCO_DRPLOCK | DCO_CPGCLOCK);
	msg_port_write(MEM_CTLR, DCO, dco);

	LEAVEFN();
}
