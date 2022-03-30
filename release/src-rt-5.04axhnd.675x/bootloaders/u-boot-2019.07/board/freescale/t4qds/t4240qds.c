// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2012 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>

#include "../common/qixis.h"
#include "../common/vsc3316_3308.h"
#include "t4qds.h"
#include "t4240qds_qixis.h"

DECLARE_GLOBAL_DATA_PTR;

static int8_t vsc3316_fsm1_tx[8][2] = { {0, 0}, {1, 1}, {6, 6}, {7, 7},
				{8, 8}, {9, 9}, {14, 14}, {15, 15} };

static int8_t vsc3316_fsm2_tx[8][2] = { {2, 2}, {3, 3}, {4, 4}, {5, 5},
				{10, 10}, {11, 11}, {12, 12}, {13, 13} };

static int8_t vsc3316_fsm1_rx[8][2] = { {2, 12}, {3, 13}, {4, 5}, {5, 4},
				{10, 11}, {11, 10}, {12, 2}, {13, 3} };

static int8_t vsc3316_fsm2_rx[8][2] = { {0, 15}, {1, 14}, {6, 7}, {7, 6},
				{8, 9}, {9, 8}, {14, 1}, {15, 0} };

int checkboard(void)
{
	char buf[64];
	u8 sw;
	struct cpu_type *cpu = gd->arch.cpu;
	unsigned int i;

	printf("Board: %sQDS, ", cpu->name);
	printf("Sys ID: 0x%02x, Sys Ver: 0x%02x, ",
	       QIXIS_READ(id), QIXIS_READ(arch));

	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank: %d\n", sw);
	else if (sw == 0x8)
		puts("Promjet\n");
	else if (sw == 0x9)
		puts("NAND\n");
	else
		printf("invalid setting of SW%u\n", QIXIS_LBMAP_SWITCH);

	printf("FPGA: v%d (%s), build %d",
	       (int)QIXIS_READ(scver), qixis_read_tag(buf),
	       (int)qixis_read_minor());
	/* the timestamp string contains "\n" at the end */
	printf(" on %s", qixis_read_time(buf));

	/*
	 * Display the actual SERDES reference clocks as configured by the
	 * dip switches on the board.  Note that the SWx registers could
	 * technically be set to force the reference clocks to match the
	 * values that the SERDES expects (or vice versa).  For now, however,
	 * we just display both values and hope the user notices when they
	 * don't match.
	 */
	puts("SERDES Reference Clocks: ");
	sw = QIXIS_READ(brdcfg[2]);
	for (i = 0; i < MAX_SERDES; i++) {
		static const char * const freq[] = {
			"100", "125", "156.25", "161.1328125"};
		unsigned int clock = (sw >> (6 - 2 * i)) & 3;

		printf("SERDES%u=%sMHz ", i+1, freq[clock]);
	}
	puts("\n");

	return 0;
}

int select_i2c_ch_pca9547(u8 ch)
{
	int ret;

	ret = i2c_write(I2C_MUX_PCA_ADDR_PRI, 0, 1, &ch, 1);
	if (ret) {
		puts("PCA: failed to select proper channel\n");
		return ret;
	}

	return 0;
}

/*
 * read_voltage from sensor on I2C bus
 * We use average of 4 readings, waiting for 532us befor another reading
 */
#define NUM_READINGS	4	/* prefer to be power of 2 for efficiency */
#define WAIT_FOR_ADC	532	/* wait for 532 microseconds for ADC */

static inline int read_voltage(void)
{
	int i, ret, voltage_read = 0;
	u16 vol_mon;

	for (i = 0; i < NUM_READINGS; i++) {
		ret = i2c_read(I2C_VOL_MONITOR_ADDR,
			I2C_VOL_MONITOR_BUS_V_OFFSET, 1, (void *)&vol_mon, 2);
		if (ret) {
			printf("VID: failed to read core voltage\n");
			return ret;
		}
		if (vol_mon & I2C_VOL_MONITOR_BUS_V_OVF) {
			printf("VID: Core voltage sensor error\n");
			return -1;
		}
		debug("VID: bus voltage reads 0x%04x\n", vol_mon);
		/* LSB = 4mv */
		voltage_read += (vol_mon >> I2C_VOL_MONITOR_BUS_V_SHIFT) * 4;
		udelay(WAIT_FOR_ADC);
	}
	/* calculate the average */
	voltage_read /= NUM_READINGS;

	return voltage_read;
}

/*
 * We need to calculate how long before the voltage starts to drop or increase
 * It returns with the loop count. Each loop takes several readings (532us)
 */
static inline int wait_for_voltage_change(int vdd_last)
{
	int timeout, vdd_current;

	vdd_current = read_voltage();
	/* wait until voltage starts to drop */
	for (timeout = 0; abs(vdd_last - vdd_current) <= 4 &&
		timeout < 100; timeout++) {
		vdd_current = read_voltage();
	}
	if (timeout >= 100) {
		printf("VID: Voltage adjustment timeout\n");
		return -1;
	}
	return timeout;
}

/*
 * argument 'wait' is the time we know the voltage difference can be measured
 * this function keeps reading the voltage until it is stable
 */
static inline int wait_for_voltage_stable(int wait)
{
	int timeout, vdd_current, vdd_last;

	vdd_last = read_voltage();
	udelay(wait * NUM_READINGS * WAIT_FOR_ADC);
	/* wait until voltage is stable */
	vdd_current = read_voltage();
	for (timeout = 0; abs(vdd_last - vdd_current) >= 4 &&
		timeout < 100; timeout++) {
		vdd_last = vdd_current;
		udelay(wait * NUM_READINGS * WAIT_FOR_ADC);
		vdd_current = read_voltage();
	}
	if (timeout >= 100) {
		printf("VID: Voltage adjustment timeout\n");
		return -1;
	}

	return vdd_current;
}

static inline int set_voltage(u8 vid)
{
	int wait, vdd_last;

	vdd_last = read_voltage();
	QIXIS_WRITE(brdcfg[6], vid);
	wait = wait_for_voltage_change(vdd_last);
	if (wait < 0)
		return -1;
	debug("VID: Waited %d us\n", wait * NUM_READINGS * WAIT_FOR_ADC);
	wait = wait ? wait : 1;

	vdd_last = wait_for_voltage_stable(wait);
	if (vdd_last < 0)
		return -1;
	debug("VID: Current voltage is %d mV\n", vdd_last);

	return vdd_last;
}


static int adjust_vdd(ulong vdd_override)
{
	int re_enable = disable_interrupts();
	ccsr_gur_t __iomem *gur =
		(void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 fusesr;
	u8 vid, vid_current;
	int vdd_target, vdd_current, vdd_last;
	int ret;
	unsigned long vdd_string_override;
	char *vdd_string;
	static const uint16_t vdd[32] = {
		0,	/* unused */
		9875,	/* 0.9875V */
		9750,
		9625,
		9500,
		9375,
		9250,
		9125,
		9000,
		8875,
		8750,
		8625,
		8500,
		8375,
		8250,
		8125,
		10000,	/* 1.0000V */
		10125,
		10250,
		10375,
		10500,
		10625,
		10750,
		10875,
		11000,
		0,	/* reserved */
	};
	struct vdd_drive {
		u8 vid;
		unsigned voltage;
	};

	ret = select_i2c_ch_pca9547(I2C_MUX_CH_VOL_MONITOR);
	if (ret) {
		debug("VID: I2c failed to switch channel\n");
		ret = -1;
		goto exit;
	}

	/* get the voltage ID from fuse status register */
	fusesr = in_be32(&gur->dcfg_fusesr);
	vid = (fusesr >> FSL_CORENET_DCFG_FUSESR_VID_SHIFT) &
		FSL_CORENET_DCFG_FUSESR_VID_MASK;
	if (vid == FSL_CORENET_DCFG_FUSESR_VID_MASK) {
		vid = (fusesr >> FSL_CORENET_DCFG_FUSESR_ALTVID_SHIFT) &
			FSL_CORENET_DCFG_FUSESR_ALTVID_MASK;
	}
	vdd_target = vdd[vid];

	/* check override variable for overriding VDD */
	vdd_string = env_get("t4240qds_vdd_mv");
	if (vdd_override == 0 && vdd_string &&
	    !strict_strtoul(vdd_string, 10, &vdd_string_override))
		vdd_override = vdd_string_override;
	if (vdd_override >= 819 && vdd_override <= 1212) {
		vdd_target = vdd_override * 10; /* convert to 1/10 mV */
		debug("VDD override is %lu\n", vdd_override);
	} else if (vdd_override != 0) {
		printf("Invalid value.\n");
	}

	if (vdd_target == 0) {
		debug("VID: VID not used\n");
		ret = 0;
		goto exit;
	} else {
		/* round up and divice by 10 to get a value in mV */
		vdd_target = DIV_ROUND_UP(vdd_target, 10);
		debug("VID: vid = %d mV\n", vdd_target);
	}

	/*
	 * Check current board VID setting
	 * Voltage regulator support output to 6.250mv step
	 * The highes voltage allowed for this board is (vid=0x40) 1.21250V
	 * the lowest is (vid=0x7f) 0.81875V
	 */
	vid_current =  QIXIS_READ(brdcfg[6]);
	vdd_current = 121250 - (vid_current - 0x40) * 625;
	debug("VID: Current vid setting is (0x%x) %d mV\n",
	      vid_current, vdd_current/100);

	/*
	 * Read voltage monitor to check real voltage.
	 * Voltage monitor LSB is 4mv.
	 */
	vdd_last = read_voltage();
	if (vdd_last < 0) {
		printf("VID: Could not read voltage sensor abort VID adjustment\n");
		ret = -1;
		goto exit;
	}
	debug("VID: Core voltage is at %d mV\n", vdd_last);
	/*
	 * Adjust voltage to at or 8mV above target.
	 * Each step of adjustment is 6.25mV.
	 * Stepping down too fast may cause over current.
	 */
	while (vdd_last > 0 && vid_current < 0x80 &&
		vdd_last > (vdd_target + 8)) {
		vid_current++;
		vdd_last = set_voltage(vid_current);
	}
	/*
	 * Check if we need to step up
	 * This happens when board voltage switch was set too low
	 */
	while (vdd_last > 0 && vid_current >= 0x40 &&
		vdd_last < vdd_target + 2) {
		vid_current--;
		vdd_last = set_voltage(vid_current);
	}
	if (vdd_last > 0)
		printf("VID: Core voltage %d mV\n", vdd_last);
	else
		ret = -1;

exit:
	if (re_enable)
		enable_interrupts();
	return ret;
}

/* Configure Crossbar switches for Front-Side SerDes Ports */
int config_frontside_crossbar_vsc3316(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_prtcl_s1, srds_prtcl_s2;
	int ret;

	ret = select_i2c_ch_pca9547(I2C_MUX_CH_VSC3316_FS);
	if (ret)
		return ret;

	srds_prtcl_s1 = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	srds_prtcl_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;
	switch (srds_prtcl_s1) {
	case 37:
	case 38:
		/* swap first lane and third lane on slot1 */
		vsc3316_fsm1_tx[0][1] = 14;
		vsc3316_fsm1_tx[6][1] = 0;
		vsc3316_fsm1_rx[1][1] = 2;
		vsc3316_fsm1_rx[6][1] = 13;
	case 39:
	case 40:
	case 45:
	case 46:
	case 47:
	case 48:
		/* swap first lane and third lane on slot2 */
		vsc3316_fsm1_tx[2][1] = 8;
		vsc3316_fsm1_tx[4][1] = 6;
		vsc3316_fsm1_rx[2][1] = 10;
		vsc3316_fsm1_rx[5][1] = 5;
	default:
		ret = vsc3316_config(VSC3316_FSM_TX_ADDR, vsc3316_fsm1_tx, 8);
		if (ret)
			return ret;
		ret = vsc3316_config(VSC3316_FSM_RX_ADDR, vsc3316_fsm1_rx, 8);
		if (ret)
			return ret;
		break;
	}

	srds_prtcl_s2 = in_be32(&gur->rcwsr[4]) &
				FSL_CORENET2_RCWSR4_SRDS2_PRTCL;
	srds_prtcl_s2 >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;
	switch (srds_prtcl_s2) {
	case 37:
	case 38:
		/* swap first lane and third lane on slot3 */
		vsc3316_fsm2_tx[2][1] = 11;
		vsc3316_fsm2_tx[5][1] = 4;
		vsc3316_fsm2_rx[2][1] = 9;
		vsc3316_fsm2_rx[4][1] = 7;
	case 39:
	case 40:
	case 45:
	case 46:
	case 47:
	case 48:
	case 49:
	case 50:
	case 51:
	case 52:
	case 53:
	case 54:
		/* swap first lane and third lane on slot4 */
		vsc3316_fsm2_tx[6][1] = 3;
		vsc3316_fsm2_tx[1][1] = 12;
		vsc3316_fsm2_rx[0][1] = 1;
		vsc3316_fsm2_rx[6][1] = 15;
	default:
		ret = vsc3316_config(VSC3316_FSM_TX_ADDR, vsc3316_fsm2_tx, 8);
		if (ret)
			return ret;
		ret = vsc3316_config(VSC3316_FSM_RX_ADDR, vsc3316_fsm2_rx, 8);
		if (ret)
			return ret;
		break;
	}

	return 0;
}

int config_backside_crossbar_mux(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_prtcl_s3, srds_prtcl_s4;
	u8 brdcfg;

	srds_prtcl_s3 = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS3_PRTCL;
	srds_prtcl_s3 >>= FSL_CORENET2_RCWSR4_SRDS3_PRTCL_SHIFT;
	switch (srds_prtcl_s3) {
	case 0:
		/* SerDes3 is not enabled */
		break;
	case 1:
	case 2:
	case 9:
	case 10:
		/* SD3(0:7) => SLOT5(0:7) */
		brdcfg = QIXIS_READ(brdcfg[12]);
		brdcfg &= ~BRDCFG12_SD3MX_MASK;
		brdcfg |= BRDCFG12_SD3MX_SLOT5;
		QIXIS_WRITE(brdcfg[12], brdcfg);
		break;
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
		/* SD3(4:7) => SLOT6(0:3) */
		brdcfg = QIXIS_READ(brdcfg[12]);
		brdcfg &= ~BRDCFG12_SD3MX_MASK;
		brdcfg |= BRDCFG12_SD3MX_SLOT6;
		QIXIS_WRITE(brdcfg[12], brdcfg);
		break;
	default:
		printf("WARNING: unsupported for SerDes3 Protocol %d\n",
		       srds_prtcl_s3);
		return -1;
	}

	srds_prtcl_s4 = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS4_PRTCL;
	srds_prtcl_s4 >>= FSL_CORENET2_RCWSR4_SRDS4_PRTCL_SHIFT;
	switch (srds_prtcl_s4) {
	case 0:
		/* SerDes4 is not enabled */
		break;
	case 1:
	case 2:
		/* 10b, SD4(0:7) => SLOT7(0:7) */
		brdcfg = QIXIS_READ(brdcfg[12]);
		brdcfg &= ~BRDCFG12_SD4MX_MASK;
		brdcfg |= BRDCFG12_SD4MX_SLOT7;
		QIXIS_WRITE(brdcfg[12], brdcfg);
		break;
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		/* x1b, SD4(4:7) => SLOT8(0:3) */
		brdcfg = QIXIS_READ(brdcfg[12]);
		brdcfg &= ~BRDCFG12_SD4MX_MASK;
		brdcfg |= BRDCFG12_SD4MX_SLOT8;
		QIXIS_WRITE(brdcfg[12], brdcfg);
		break;
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 18:
		/* 00b, SD4(4:5) => AURORA, SD4(6:7) => SATA */
		brdcfg = QIXIS_READ(brdcfg[12]);
		brdcfg &= ~BRDCFG12_SD4MX_MASK;
		brdcfg |= BRDCFG12_SD4MX_AURO_SATA;
		QIXIS_WRITE(brdcfg[12], brdcfg);
		break;
	default:
		printf("WARNING: unsupported for SerDes4 Protocol %d\n",
		       srds_prtcl_s4);
		return -1;
	}

	return 0;
}

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash + PROMJET region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	if (flash_esel == -1) {
		/* very unlikely unless something is messed up */
		puts("Error: Could not find TLB for FLASH BASE\n");
		flash_esel = 2;	/* give our best effort to continue */
	} else {
		/* invalidate existing TLB entry for flash + promjet */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		0, flash_esel, BOOKE_PAGESZ_256M, 1);

	/* Disable remote I2C connection to qixis fpga */
	QIXIS_WRITE(brdcfg[5], QIXIS_READ(brdcfg[5]) & ~BRDCFG5_IRE);

	/*
	 * Adjust core voltage according to voltage ID
	 * This function changes I2C mux to channel 2.
	 */
	if (adjust_vdd(0))
		printf("Warning: Adjusting core voltage failed.\n");

	/* Configure board SERDES ports crossbar */
	config_frontside_crossbar_vsc3316();
	config_backside_crossbar_mux();
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);

	return 0;
}

unsigned long get_board_sys_clk(void)
{
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);
#ifdef CONFIG_FSL_QIXIS_CLOCK_MEASUREMENT
	/* use accurate clock measurement */
	int freq = QIXIS_READ(clk_freq[0]) << 8 | QIXIS_READ(clk_freq[1]);
	int base = QIXIS_READ(clk_base[0]) << 8 | QIXIS_READ(clk_base[1]);
	u32 val;

	val =  freq * base;
	if (val) {
		debug("SYS Clock measurement is: %d\n", val);
		return val;
	} else {
		printf("Warning: SYS clock measurement is invalid, using value from brdcfg1.\n");
	}
#endif

	switch (sysclk_conf & 0x0F) {
	case QIXIS_SYSCLK_83:
		return 83333333;
	case QIXIS_SYSCLK_100:
		return 100000000;
	case QIXIS_SYSCLK_125:
		return 125000000;
	case QIXIS_SYSCLK_133:
		return 133333333;
	case QIXIS_SYSCLK_150:
		return 150000000;
	case QIXIS_SYSCLK_160:
		return 160000000;
	case QIXIS_SYSCLK_166:
		return 166666666;
	}
	return 66666666;
}

unsigned long get_board_ddr_clk(void)
{
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);
#ifdef CONFIG_FSL_QIXIS_CLOCK_MEASUREMENT
	/* use accurate clock measurement */
	int freq = QIXIS_READ(clk_freq[2]) << 8 | QIXIS_READ(clk_freq[3]);
	int base = QIXIS_READ(clk_base[0]) << 8 | QIXIS_READ(clk_base[1]);
	u32 val;

	val =  freq * base;
	if (val) {
		debug("DDR Clock measurement is: %d\n", val);
		return val;
	} else {
		printf("Warning: DDR clock measurement is invalid, using value from brdcfg1.\n");
	}
#endif

	switch ((ddrclk_conf & 0x30) >> 4) {
	case QIXIS_DDRCLK_100:
		return 100000000;
	case QIXIS_DDRCLK_125:
		return 125000000;
	case QIXIS_DDRCLK_133:
		return 133333333;
	}
	return 66666666;
}

int misc_init_r(void)
{
	u8 sw;
	void *srds_base = (void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	serdes_corenet_t *srds_regs;
	u32 actual[MAX_SERDES];
	u32 pllcr0, expected;
	unsigned int i;

	sw = QIXIS_READ(brdcfg[2]);
	for (i = 0; i < MAX_SERDES; i++) {
		unsigned int clock = (sw >> (6 - 2 * i)) & 3;
		switch (clock) {
		case 0:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_100;
			break;
		case 1:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_125;
			break;
		case 2:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_156_25;
			break;
		case 3:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_161_13;
			break;
		}
	}

	for (i = 0; i < MAX_SERDES; i++) {
		srds_regs = srds_base + i * 0x1000;
		pllcr0 = srds_regs->bank[0].pllcr0;
		expected = pllcr0 & SRDS_PLLCR0_RFCK_SEL_MASK;
		if (expected != actual[i]) {
			printf("Warning: SERDES%u expects reference clock %sMHz, but actual is %sMHz\n",
			       i + 1, serdes_clock_to_string(expected),
			       serdes_clock_to_string(actual[i]));
		}
	}

	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_PCI
	pci_of_setup(blob, bd);
#endif

	fdt_fixup_liodn(blob);
	fsl_fdt_fixup_dr_usb(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
	fdt_fixup_board_enet(blob);
#endif

	return 0;
}

/*
 * This function is called by bdinfo to print detail board information.
 * As an exmaple for future board, we organize the messages into
 * several sections. If applicable, the message is in the format of
 * <name>      = <value>
 * It should aligned with normal output of bdinfo command.
 *
 * Voltage: Core, DDR and another configurable voltages
 * Clock  : Critical clocks which are not printed already
 * RCW    : RCW source if not printed already
 * Misc   : Other important information not in above catagories
 */
void board_detail(void)
{
	int i;
	u8 brdcfg[16], dutcfg[16], rst_ctl;
	int vdd, rcwsrc;
	static const char * const clk[] = {"66.67", "100", "125", "133.33"};

	for (i = 0; i < 16; i++) {
		brdcfg[i] = qixis_read(offsetof(struct qixis, brdcfg[0]) + i);
		dutcfg[i] = qixis_read(offsetof(struct qixis, dutcfg[0]) + i);
	}

	/* Voltage secion */
	if (!select_i2c_ch_pca9547(I2C_MUX_CH_VOL_MONITOR)) {
		vdd = read_voltage();
		if (vdd > 0)
			printf("Core voltage= %d mV\n", vdd);
		select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);
	}

	printf("XVDD        = 1.%d V\n", ((brdcfg[8] & 0xf) - 4) * 5 + 25);

	/* clock section */
	printf("SYSCLK      = %s MHz\nDDRCLK      = %s MHz\n",
	       clk[(brdcfg[11] >> 2) & 0x3], clk[brdcfg[11] & 3]);

	/* RCW section */
	rcwsrc = (dutcfg[0] << 1) + (dutcfg[1] & 1);
	puts("RCW source  = ");
	switch (rcwsrc) {
	case 0x017:
	case 0x01f:
		puts("8-bit NOR\n");
		break;
	case 0x027:
	case 0x02F:
		puts("16-bit NOR\n");
		break;
	case 0x040:
		puts("SDHC/eMMC\n");
		break;
	case 0x044:
		puts("SPI 16-bit addressing\n");
		break;
	case 0x045:
		puts("SPI 24-bit addressing\n");
		break;
	case 0x048:
		puts("I2C normal addressing\n");
		break;
	case 0x049:
		puts("I2C extended addressing\n");
		break;
	case 0x108:
	case 0x109:
	case 0x10a:
	case 0x10b:
		puts("8-bit NAND, 2KB\n");
		break;
	default:
		if ((rcwsrc >= 0x080) && (rcwsrc <= 0x09f))
			puts("Hard-coded RCW\n");
		else if ((rcwsrc >= 0x110) && (rcwsrc <= 0x11f))
			puts("8-bit NAND, 4KB\n");
		else
			puts("unknown\n");
		break;
	}

	/* Misc section */
	rst_ctl = QIXIS_READ(rst_ctl);
	puts("HRESET_REQ  = ");
	switch (rst_ctl & 0x30) {
	case 0x00:
		puts("Ignored\n");
		break;
	case 0x10:
		puts("Assert HRESET\n");
		break;
	case 0x30:
		puts("Reset system\n");
		break;
	default:
		puts("N/A\n");
		break;
	}
}

/*
 * Reverse engineering switch settings.
 * Some bits cannot be figured out. They will be displayed as
 * underscore in binary format. mask[] has those bits.
 * Some bits are calculated differently than the actual switches
 * if booting with overriding by FPGA.
 */
void qixis_dump_switch(void)
{
	int i;
	u8 sw[9];

	/*
	 * Any bit with 1 means that bit cannot be reverse engineered.
	 * It will be displayed as _ in binary format.
	 */
	static const u8 mask[] = {0, 0, 0, 0, 0, 0x1, 0xcf, 0x3f, 0x1f};
	char buf[10];
	u8 brdcfg[16], dutcfg[16];

	for (i = 0; i < 16; i++) {
		brdcfg[i] = qixis_read(offsetof(struct qixis, brdcfg[0]) + i);
		dutcfg[i] = qixis_read(offsetof(struct qixis, dutcfg[0]) + i);
	}

	sw[0] = dutcfg[0];
	sw[1] = (dutcfg[1] << 0x07)		|
		((dutcfg[12] & 0xC0) >> 1)	|
		((dutcfg[11] & 0xE0) >> 3)	|
		((dutcfg[6] & 0x80) >> 6)	|
		((dutcfg[1] & 0x80) >> 7);
	sw[2] = ((brdcfg[1] & 0x0f) << 4)	|
		((brdcfg[1] & 0x30) >> 2)	|
		((brdcfg[1] & 0x40) >> 5)	|
		((brdcfg[1] & 0x80) >> 7);
	sw[3] = brdcfg[2];
	sw[4] = ((dutcfg[2] & 0x01) << 7)	|
		((dutcfg[2] & 0x06) << 4)	|
		((~QIXIS_READ(present)) & 0x10)	|
		((brdcfg[3] & 0x80) >> 4)	|
		((brdcfg[3] & 0x01) << 2)	|
		((brdcfg[6] == 0x62) ? 3 :
		((brdcfg[6] == 0x5a) ? 2 :
		((brdcfg[6] == 0x5e) ? 1 : 0)));
	sw[5] = ((brdcfg[0] & 0x0f) << 4)	|
		((QIXIS_READ(rst_ctl) & 0x30) >> 2) |
		((brdcfg[0] & 0x40) >> 5);
	sw[6] = (brdcfg[11] & 0x20)		|
		((brdcfg[5] & 0x02) << 3);
	sw[7] = (((~QIXIS_READ(rst_ctl)) & 0x40) << 1) |
		((brdcfg[5] & 0x10) << 2);
	sw[8] = ((brdcfg[12] & 0x08) << 4)	|
		((brdcfg[12] & 0x03) << 5);

	puts("DIP switch (reverse-engineering)\n");
	for (i = 0; i < 9; i++) {
		printf("SW%d         = 0b%s (0x%02x)\n",
		       i + 1, byte_to_binary_mask(sw[i], mask[i], buf), sw[i]);
	}
}

static int do_vdd_adjust(cmd_tbl_t *cmdtp,
			 int flag, int argc,
			 char * const argv[])
{
	ulong override;

	if (argc < 2)
		return CMD_RET_USAGE;
	if (!strict_strtoul(argv[1], 10, &override))
		adjust_vdd(override);	/* the value is checked by callee */
	else
		return CMD_RET_USAGE;

	return 0;
}

U_BOOT_CMD(
	vdd_override, 2, 0, do_vdd_adjust,
	"Override VDD",
	"- override with the voltage specified in mV, eg. 1050"
);
