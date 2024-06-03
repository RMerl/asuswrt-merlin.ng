// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011-2012 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <linux/errno.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>
#include <hwconfig.h>

#include "../common/qixis.h"
#include "../common/vsc3316_3308.h"
#include "../common/idt8t49n222a_serdes_clk.h"
#include "../common/zm7300.h"
#include "b4860qds.h"
#include "b4860qds_qixis.h"
#include "b4860qds_crossbar_con.h"

#define CLK_MUX_SEL_MASK	0x4
#define ETH_PHY_CLK_OUT		0x4

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	char buf[64];
	u8 sw;
	struct cpu_type *cpu = gd->arch.cpu;
	static const char *const freq[] = {"100", "125", "156.25", "161.13",
						"122.88", "122.88", "122.88"};
	int clock;

	printf("Board: %sQDS, ", cpu->name);
	printf("Sys ID: 0x%02x, Sys Ver: 0x%02x, ",
		QIXIS_READ(id), QIXIS_READ(arch));

	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank: %d\n", sw);
	else if (sw >= 0x8 && sw <= 0xE)
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
	clock = (sw >> 5) & 7;
	printf("Bank1=%sMHz ", freq[clock]);
	sw = QIXIS_READ(brdcfg[4]);
	clock = (sw >> 6) & 3;
	printf("Bank2=%sMHz\n", freq[clock]);

	return 0;
}

int select_i2c_ch_pca(u8 ch)
{
	int ret;

	/* Selecting proper channel via PCA*/
	ret = i2c_write(I2C_MUX_PCA_ADDR, 0x0, 1, &ch, 1);
	if (ret) {
		printf("PCA: failed to select proper channel.\n");
		return ret;
	}

	return 0;
}

/*
 * read_voltage from sensor on I2C bus
 * We use average of 4 readings, waiting for 532us befor another reading
 */
#define WAIT_FOR_ADC	532	/* wait for 532 microseconds for ADC */
#define NUM_READINGS	4	/* prefer to be power of 2 for efficiency */

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

static int adjust_vdd(ulong vdd_override)
{
	int re_enable = disable_interrupts();
	ccsr_gur_t __iomem *gur =
		(void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 fusesr;
	u8 vid;
	int vdd_target, vdd_last;
	int existing_voltage, temp_voltage, voltage; /* all in 1/10 mV */
	int ret;
	unsigned int orig_i2c_speed;
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

	ret = select_i2c_ch_pca(I2C_MUX_CH_VOL_MONITOR);
	if (ret) {
		printf("VID: I2c failed to switch channel\n");
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
	debug("VID:Reading from from fuse,vid=%x vdd is %dmV\n",
	      vid, vdd_target/10);

	/* check override variable for overriding VDD */
	vdd_string = env_get("b4qds_vdd_mv");
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
		printf("VID: VID not used\n");
		ret = 0;
		goto exit;
	}

	/*
	 * Read voltage monitor to check real voltage.
	 * Voltage monitor LSB is 4mv.
	 */
	vdd_last = read_voltage();
	if (vdd_last < 0) {
		printf("VID: abort VID adjustment\n");
		ret = -1;
		goto exit;
	}

	debug("VID: Core voltage is at %d mV\n", vdd_last);
	ret = select_i2c_ch_pca(I2C_MUX_CH_DPM);
	if (ret) {
		printf("VID: I2c failed to switch channel to DPM\n");
		ret = -1;
		goto exit;
	}

	/* Round up to the value of step of Voltage regulator */
	voltage = roundup(vdd_target, ZM_STEP);
	debug("VID: rounded up voltage = %d\n", voltage);

	/* lower the speed to 100kHz to access ZM7300 device */
	debug("VID: Setting bus speed to 100KHz if not already set\n");
	orig_i2c_speed = i2c_get_bus_speed();
	if (orig_i2c_speed != 100000)
		i2c_set_bus_speed(100000);

	/* Read the existing level on board, if equal to requsted one,
	   no need to re-set */
	existing_voltage = zm_read_voltage();

	/* allowing the voltage difference of one step 0.0125V acceptable */
	if ((existing_voltage >= voltage) &&
	    (existing_voltage < (voltage + ZM_STEP))) {
		debug("VID: voltage already set as requested,returning\n");
		ret = existing_voltage;
		goto out;
	}
	debug("VID: Changing voltage for board from %dmV to %dmV\n",
	      existing_voltage/10, voltage/10);

	if (zm_disable_wp() < 0) {
		ret = -1;
		goto out;
	}
	/* Change Voltage: the change is done through all the steps in the
	   way, to avoid reset to the board due to power good signal fail
	   in big voltage change gap jump.
	*/
	if (existing_voltage > voltage) {
		temp_voltage = existing_voltage - ZM_STEP;
			while (temp_voltage >= voltage) {
				ret = zm_write_voltage(temp_voltage);
				if (ret == temp_voltage) {
					temp_voltage -= ZM_STEP;
				} else {
					/* ZM7300 device failed to set
					 * the voltage */
					printf
					("VID:Stepping down vol failed:%dmV\n",
					 temp_voltage/10);
				     ret = -1;
				     goto out;
				}
			}
	} else {
		temp_voltage = existing_voltage + ZM_STEP;
			while (temp_voltage < (voltage + ZM_STEP)) {
				ret = zm_write_voltage(temp_voltage);
				if (ret == temp_voltage) {
					temp_voltage += ZM_STEP;
				} else {
					/* ZM7300 device failed to set
					 * the voltage */
					printf
					("VID:Stepping up vol failed:%dmV\n",
					 temp_voltage/10);
				     ret = -1;
				     goto out;
				}
			}
	}

	if (zm_enable_wp() < 0)
		ret = -1;

	/* restore the speed to 400kHz */
out:	debug("VID: Restore the I2C bus speed to %dKHz\n",
				orig_i2c_speed/1000);
	i2c_set_bus_speed(orig_i2c_speed);
	if (ret < 0)
		goto exit;

	ret = select_i2c_ch_pca(I2C_MUX_CH_VOL_MONITOR);
	if (ret) {
		printf("VID: I2c failed to switch channel\n");
		ret = -1;
		goto exit;
	}
	vdd_last = read_voltage();
	select_i2c_ch_pca(I2C_CH_DEFAULT);

	if (vdd_last > 0)
		printf("VID: Core voltage %d mV\n", vdd_last);
	else
		ret = -1;

exit:
	if (re_enable)
		enable_interrupts();
	return ret;
}

int configure_vsc3316_3308(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	unsigned int num_vsc16_con, num_vsc08_con;
	u32 serdes1_prtcl, serdes2_prtcl;
	int ret;
	char buffer[HWCONFIG_BUFFER_SIZE];
	char *buf = NULL;

	serdes1_prtcl = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	if (!serdes1_prtcl) {
		printf("SERDES1 is not enabled\n");
		return 0;
	}
	serdes1_prtcl >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;
	debug("Using SERDES1 Protocol: 0x%x:\n", serdes1_prtcl);

	serdes2_prtcl = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS2_PRTCL;
	if (!serdes2_prtcl) {
		printf("SERDES2 is not enabled\n");
		return 0;
	}
	serdes2_prtcl >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;
	debug("Using SERDES2 Protocol: 0x%x:\n", serdes2_prtcl);

	switch (serdes1_prtcl) {
	case 0x29:
	case 0x2a:
	case 0x2C:
	case 0x2D:
	case 0x2E:
			/*
			 * Configuration:
			 * SERDES: 1
			 * Lanes: A,B: SGMII
			 * Lanes: C,D,E,F,G,H: CPRI
			 */
		debug("Configuring crossbar to use onboard SGMII PHYs:"
				"srds_prctl:%x\n", serdes1_prtcl);
		num_vsc16_con = NUM_CON_VSC3316;
		/* Configure VSC3316 crossbar switch */
		ret = select_i2c_ch_pca(I2C_CH_VSC3316);
		if (!ret) {
			ret = vsc3316_config(VSC3316_TX_ADDRESS,
					vsc16_tx_4sfp_sgmii_12_56,
					num_vsc16_con);
			if (ret)
				return ret;
			ret = vsc3316_config(VSC3316_RX_ADDRESS,
					vsc16_rx_4sfp_sgmii_12_56,
					num_vsc16_con);
			if (ret)
				return ret;
		} else {
			return ret;
		}
		break;

	case 0x01:
	case 0x02:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
	case 0x0C:
	case 0x2F:
	case 0x30:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x39:
	case 0x3A:
	case 0x3C:
	case 0x3D:
	case 0x5C:
	case 0x5D:
			/*
			 * Configuration:
			 * SERDES: 1
			 * Lanes: A,B: AURORA
			 * Lanes: C,d: SGMII
			 * Lanes: E,F,G,H: CPRI
			 */
		debug("Configuring crossbar for Aurora, SGMII 3 and 4,"
				" and CPRI. srds_prctl:%x\n", serdes1_prtcl);
		num_vsc16_con = NUM_CON_VSC3316;
		/* Configure VSC3316 crossbar switch */
		ret = select_i2c_ch_pca(I2C_CH_VSC3316);
		if (!ret) {
			ret = vsc3316_config(VSC3316_TX_ADDRESS,
					vsc16_tx_sfp_sgmii_aurora,
					num_vsc16_con);
			if (ret)
				return ret;
			ret = vsc3316_config(VSC3316_RX_ADDRESS,
					vsc16_rx_sfp_sgmii_aurora,
					num_vsc16_con);
			if (ret)
				return ret;
		} else {
			return ret;
		}
		break;

#ifdef CONFIG_ARCH_B4420
	case 0x17:
	case 0x18:
			/*
			 * Configuration:
			 * SERDES: 1
			 * Lanes: A,B,C,D: SGMII
			 * Lanes: E,F,G,H: CPRI
			 */
		debug("Configuring crossbar to use onboard SGMII PHYs:"
				"srds_prctl:%x\n", serdes1_prtcl);
		num_vsc16_con = NUM_CON_VSC3316;
		/* Configure VSC3316 crossbar switch */
		ret = select_i2c_ch_pca(I2C_CH_VSC3316);
		if (!ret) {
			ret = vsc3316_config(VSC3316_TX_ADDRESS,
					vsc16_tx_sgmii_lane_cd, num_vsc16_con);
			if (ret)
				return ret;
			ret = vsc3316_config(VSC3316_RX_ADDRESS,
					vsc16_rx_sgmii_lane_cd, num_vsc16_con);
			if (ret)
				return ret;
		} else {
			return ret;
		}
		break;
#endif

	case 0x3E:
	case 0x0D:
	case 0x0E:
	case 0x12:
		num_vsc16_con = NUM_CON_VSC3316;
		/* Configure VSC3316 crossbar switch */
		ret = select_i2c_ch_pca(I2C_CH_VSC3316);
		if (!ret) {
			ret = vsc3316_config(VSC3316_TX_ADDRESS,
					vsc16_tx_sfp, num_vsc16_con);
			if (ret)
				return ret;
			ret = vsc3316_config(VSC3316_RX_ADDRESS,
					vsc16_rx_sfp, num_vsc16_con);
			if (ret)
				return ret;
		} else {
			return ret;
		}
		break;
	default:
		printf("WARNING:VSC crossbars programming not supported for:%x"
					" SerDes1 Protocol.\n", serdes1_prtcl);
		return -1;
	}

	num_vsc08_con = NUM_CON_VSC3308;
	/* Configure VSC3308 crossbar switch */
	ret = select_i2c_ch_pca(I2C_CH_VSC3308);
	switch (serdes2_prtcl) {
#ifdef CONFIG_ARCH_B4420
	case 0x9d:
#endif
	case 0x9E:
	case 0x9A:
	case 0x98:
	case 0x48:
	case 0x49:
	case 0x4E:
	case 0x79:
	case 0x7A:
		if (!ret) {
			ret = vsc3308_config(VSC3308_TX_ADDRESS,
					vsc08_tx_amc, num_vsc08_con);
			if (ret)
				return ret;
			ret = vsc3308_config(VSC3308_RX_ADDRESS,
					vsc08_rx_amc, num_vsc08_con);
			if (ret)
				return ret;
		} else {
			return ret;
		}
		break;
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87:
	case 0x88:
	case 0x89:
	case 0x8a:
	case 0x8b:
	case 0x8c:
	case 0x8d:
	case 0x8e:
	case 0xb1:
	case 0xb2:
		if (!ret) {
			/*
			 * Extract hwconfig from environment since environment
			 * is not setup properly yet
			 */
			env_get_f("hwconfig", buffer, sizeof(buffer));
			buf = buffer;

			if (hwconfig_subarg_cmp_f("fsl_b4860_serdes2",
						  "sfp_amc", "sfp", buf)) {
#ifdef CONFIG_SYS_FSL_B4860QDS_XFI_ERR
				/* change default VSC3308 for XFI erratum */
				ret = vsc3308_config_adjust(VSC3308_TX_ADDRESS,
						vsc08_tx_sfp, num_vsc08_con);
				if (ret)
					return ret;

				ret = vsc3308_config_adjust(VSC3308_RX_ADDRESS,
						vsc08_rx_sfp, num_vsc08_con);
				if (ret)
					return ret;
#else
				ret = vsc3308_config(VSC3308_TX_ADDRESS,
						vsc08_tx_sfp, num_vsc08_con);
				if (ret)
					return ret;

				ret = vsc3308_config(VSC3308_RX_ADDRESS,
						vsc08_rx_sfp, num_vsc08_con);
				if (ret)
					return ret;
#endif
			} else {
				ret = vsc3308_config(VSC3308_TX_ADDRESS,
						vsc08_tx_amc, num_vsc08_con);
				if (ret)
					return ret;

				ret = vsc3308_config(VSC3308_RX_ADDRESS,
						vsc08_rx_amc, num_vsc08_con);
				if (ret)
					return ret;
			}

		} else {
			return ret;
		}
		break;
	default:
		printf("WARNING:VSC crossbars programming not supported for: %x"
					" SerDes2 Protocol.\n", serdes2_prtcl);
		return -1;
	}

	return 0;
}

static int calibrate_pll(serdes_corenet_t *srds_regs, int pll_num)
{
	u32 rst_err;

	/* Steps For SerDes PLLs reset and reconfiguration
	 * or PLL power-up procedure
	 */
	debug("CALIBRATE PLL:%d\n", pll_num);
	clrbits_be32(&srds_regs->bank[pll_num].rstctl,
			SRDS_RSTCTL_SDRST_B);
	udelay(10);
	clrbits_be32(&srds_regs->bank[pll_num].rstctl,
		(SRDS_RSTCTL_SDEN | SRDS_RSTCTL_PLLRST_B));
	udelay(10);
	setbits_be32(&srds_regs->bank[pll_num].rstctl,
			SRDS_RSTCTL_RST);
	setbits_be32(&srds_regs->bank[pll_num].rstctl,
		(SRDS_RSTCTL_SDEN | SRDS_RSTCTL_PLLRST_B
		| SRDS_RSTCTL_SDRST_B));

	udelay(20);

	/* Check whether PLL has been locked or not */
	rst_err = in_be32(&srds_regs->bank[pll_num].rstctl) &
				SRDS_RSTCTL_RSTERR;
	rst_err >>= SRDS_RSTCTL_RSTERR_SHIFT;
	debug("RST_ERR value for PLL %d is: 0x%x:\n", pll_num, rst_err);
	if (rst_err)
		return rst_err;

	return rst_err;
}

static int check_pll_locks(serdes_corenet_t *srds_regs, int pll_num)
{
	int ret = 0;
	u32 fcap, dcbias, bcap, pllcr1, pllcr0;

	if (calibrate_pll(srds_regs, pll_num)) {
		/* STEP 1 */
		/* Read fcap, dcbias and bcap value */
		clrbits_be32(&srds_regs->bank[pll_num].pllcr0,
				SRDS_PLLCR0_DCBIAS_OUT_EN);
		fcap = in_be32(&srds_regs->bank[pll_num].pllsr2) &
					SRDS_PLLSR2_FCAP;
		fcap >>= SRDS_PLLSR2_FCAP_SHIFT;
		bcap = in_be32(&srds_regs->bank[pll_num].pllsr2) &
					SRDS_PLLSR2_BCAP_EN;
		bcap >>= SRDS_PLLSR2_BCAP_EN_SHIFT;
		setbits_be32(&srds_regs->bank[pll_num].pllcr0,
				SRDS_PLLCR0_DCBIAS_OUT_EN);
		dcbias = in_be32(&srds_regs->bank[pll_num].pllsr2) &
					SRDS_PLLSR2_DCBIAS;
		dcbias >>= SRDS_PLLSR2_DCBIAS_SHIFT;
		debug("values of bcap:%x, fcap:%x and dcbias:%x\n",
					bcap, fcap, dcbias);
		if (fcap == 0 && bcap == 1) {
			/* Step 3 */
			clrbits_be32(&srds_regs->bank[pll_num].rstctl,
				(SRDS_RSTCTL_SDEN | SRDS_RSTCTL_PLLRST_B
				 | SRDS_RSTCTL_SDRST_B));
			clrbits_be32(&srds_regs->bank[pll_num].pllcr1,
					SRDS_PLLCR1_BCAP_EN);
			setbits_be32(&srds_regs->bank[pll_num].pllcr1,
					SRDS_PLLCR1_BCAP_OVD);
			if (calibrate_pll(srds_regs, pll_num)) {
				/*save the fcap, dcbias and bcap values*/
				clrbits_be32(&srds_regs->bank[pll_num].pllcr0,
						SRDS_PLLCR0_DCBIAS_OUT_EN);
				fcap = in_be32(&srds_regs->bank[pll_num].pllsr2)
					& SRDS_PLLSR2_FCAP;
				fcap >>= SRDS_PLLSR2_FCAP_SHIFT;
				bcap = in_be32(&srds_regs->bank[pll_num].pllsr2)
					& SRDS_PLLSR2_BCAP_EN;
				bcap >>= SRDS_PLLSR2_BCAP_EN_SHIFT;
				setbits_be32(&srds_regs->bank[pll_num].pllcr0,
						SRDS_PLLCR0_DCBIAS_OUT_EN);
				dcbias = in_be32
					(&srds_regs->bank[pll_num].pllsr2) &
							SRDS_PLLSR2_DCBIAS;
				dcbias >>= SRDS_PLLSR2_DCBIAS_SHIFT;

				/* Step 4*/
				clrbits_be32(&srds_regs->bank[pll_num].rstctl,
				(SRDS_RSTCTL_SDEN | SRDS_RSTCTL_PLLRST_B
				 | SRDS_RSTCTL_SDRST_B));
				setbits_be32(&srds_regs->bank[pll_num].pllcr1,
						SRDS_PLLCR1_BYP_CAL);
				clrbits_be32(&srds_regs->bank[pll_num].pllcr1,
						SRDS_PLLCR1_BCAP_EN);
				setbits_be32(&srds_regs->bank[pll_num].pllcr1,
						SRDS_PLLCR1_BCAP_OVD);
				/* change the fcap and dcbias to the saved
				 * values from Step 3 */
				clrbits_be32(&srds_regs->bank[pll_num].pllcr1,
							SRDS_PLLCR1_PLL_FCAP);
				pllcr1 = (in_be32
					(&srds_regs->bank[pll_num].pllcr1)|
					(fcap << SRDS_PLLCR1_PLL_FCAP_SHIFT));
				out_be32(&srds_regs->bank[pll_num].pllcr1,
							pllcr1);
				clrbits_be32(&srds_regs->bank[pll_num].pllcr0,
						SRDS_PLLCR0_DCBIAS_OVRD);
				pllcr0 = (in_be32
				(&srds_regs->bank[pll_num].pllcr0)|
				(dcbias << SRDS_PLLCR0_DCBIAS_OVRD_SHIFT));
				out_be32(&srds_regs->bank[pll_num].pllcr0,
							pllcr0);
				ret = calibrate_pll(srds_regs, pll_num);
				if (ret)
					return ret;
			} else {
				goto out;
			}
		} else { /* Step 5 */
			clrbits_be32(&srds_regs->bank[pll_num].rstctl,
				(SRDS_RSTCTL_SDEN | SRDS_RSTCTL_PLLRST_B
				 | SRDS_RSTCTL_SDRST_B));
			udelay(10);
			/* Change the fcap, dcbias, and bcap to the
			 * values from Step 1 */
			setbits_be32(&srds_regs->bank[pll_num].pllcr1,
					SRDS_PLLCR1_BYP_CAL);
			clrbits_be32(&srds_regs->bank[pll_num].pllcr1,
						SRDS_PLLCR1_PLL_FCAP);
			pllcr1 = (in_be32(&srds_regs->bank[pll_num].pllcr1)|
				(fcap << SRDS_PLLCR1_PLL_FCAP_SHIFT));
			out_be32(&srds_regs->bank[pll_num].pllcr1,
						pllcr1);
			clrbits_be32(&srds_regs->bank[pll_num].pllcr0,
						SRDS_PLLCR0_DCBIAS_OVRD);
			pllcr0 = (in_be32(&srds_regs->bank[pll_num].pllcr0)|
				(dcbias << SRDS_PLLCR0_DCBIAS_OVRD_SHIFT));
			out_be32(&srds_regs->bank[pll_num].pllcr0,
						pllcr0);
			clrbits_be32(&srds_regs->bank[pll_num].pllcr1,
					SRDS_PLLCR1_BCAP_EN);
			setbits_be32(&srds_regs->bank[pll_num].pllcr1,
					SRDS_PLLCR1_BCAP_OVD);
			ret = calibrate_pll(srds_regs, pll_num);
			if (ret)
				return ret;
		}
	}
out:
	return 0;
}

static int check_serdes_pll_locks(void)
{
	serdes_corenet_t *srds1_regs =
		(void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	serdes_corenet_t *srds2_regs =
		(void *)CONFIG_SYS_FSL_CORENET_SERDES2_ADDR;
	int i, ret1, ret2;

	debug("\nSerDes1 Lock check\n");
	for (i = 0; i < CONFIG_SYS_FSL_SRDS_NUM_PLLS; i++) {
		ret1 = check_pll_locks(srds1_regs, i);
		if (ret1) {
			printf("SerDes1, PLL:%d didnt lock\n", i);
			return ret1;
		}
	}
	debug("\nSerDes2 Lock check\n");
	for (i = 0; i < CONFIG_SYS_FSL_SRDS_NUM_PLLS; i++) {
		ret2 = check_pll_locks(srds2_regs, i);
		if (ret2) {
			printf("SerDes2, PLL:%d didnt lock\n", i);
			return ret2;
		}
	}

	return 0;
}

int config_serdes1_refclks(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	serdes_corenet_t *srds_regs =
		(void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	u32 serdes1_prtcl, lane;
	unsigned int flag_sgmii_aurora_prtcl = 0;
	int i;
	int ret = 0;

	serdes1_prtcl = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	if (!serdes1_prtcl) {
		printf("SERDES1 is not enabled\n");
		return -1;
	}
	serdes1_prtcl >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;
	debug("Using SERDES1 Protocol: 0x%x:\n", serdes1_prtcl);

	/* To prevent generation of reset request from SerDes
	 * while changing the refclks, By setting SRDS_RST_MSK bit,
	 * SerDes reset event cannot cause a reset request
	 */
	setbits_be32(&gur->rstrqmr1, FSL_CORENET_RSTRQMR1_SRDS_RST_MSK);

	/* Reconfigure IDT idt8t49n222a device for CPRI to work
	 * For this SerDes1's Refclk1 and refclk2 need to be set
	 * to 122.88MHz
	 */
	switch (serdes1_prtcl) {
	case 0x29:
	case 0x2A:
	case 0x2C:
	case 0x2D:
	case 0x2E:
	case 0x01:
	case 0x02:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
	case 0x0C:
	case 0x2F:
	case 0x30:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x39:
	case 0x3A:
	case 0x3C:
	case 0x3D:
	case 0x5C:
	case 0x5D:
		debug("Configuring idt8t49n222a for CPRI SerDes clks:"
			" for srds_prctl:%x\n", serdes1_prtcl);
		ret = select_i2c_ch_pca(I2C_CH_IDT);
		if (!ret) {
			ret = set_serdes_refclk(IDT_SERDES1_ADDRESS, 1,
					SERDES_REFCLK_122_88,
					SERDES_REFCLK_122_88, 0);
			if (ret) {
				printf("IDT8T49N222A configuration failed.\n");
				goto out;
			} else
				debug("IDT8T49N222A configured.\n");
		} else {
			goto out;
		}
		select_i2c_ch_pca(I2C_CH_DEFAULT);

		/* Change SerDes1's Refclk1 to 125MHz for on board
		 * SGMIIs or Aurora to work
		 */
		for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
			enum srds_prtcl lane_prtcl = serdes_get_prtcl
						(0, serdes1_prtcl, lane);
			switch (lane_prtcl) {
			case SGMII_FM1_DTSEC1:
			case SGMII_FM1_DTSEC2:
			case SGMII_FM1_DTSEC3:
			case SGMII_FM1_DTSEC4:
			case SGMII_FM1_DTSEC5:
			case SGMII_FM1_DTSEC6:
			case AURORA:
				flag_sgmii_aurora_prtcl++;
				break;
			default:
				break;
			}
		}

		if (flag_sgmii_aurora_prtcl)
			QIXIS_WRITE(brdcfg[4], QIXIS_SRDS1CLK_125);

		/* Steps For SerDes PLLs reset and reconfiguration after
		 * changing SerDes's refclks
		 */
		for (i = 0; i < CONFIG_SYS_FSL_SRDS_NUM_PLLS; i++) {
			debug("For PLL%d reset and reconfiguration after"
			       " changing refclks\n", i+1);
			clrbits_be32(&srds_regs->bank[i].rstctl,
					SRDS_RSTCTL_SDRST_B);
			udelay(10);
			clrbits_be32(&srds_regs->bank[i].rstctl,
				(SRDS_RSTCTL_SDEN | SRDS_RSTCTL_PLLRST_B));
			udelay(10);
			setbits_be32(&srds_regs->bank[i].rstctl,
					SRDS_RSTCTL_RST);
			setbits_be32(&srds_regs->bank[i].rstctl,
				(SRDS_RSTCTL_SDEN | SRDS_RSTCTL_PLLRST_B
				| SRDS_RSTCTL_SDRST_B));
		}
		break;
	default:
		printf("WARNING:IDT8T49N222A configuration not"
			" supported for:%x SerDes1 Protocol.\n",
			serdes1_prtcl);
	}

out:
	/* Clearing SRDS_RST_MSK bit as now
	 * SerDes reset event can cause a reset request
	 */
	clrbits_be32(&gur->rstrqmr1, FSL_CORENET_RSTRQMR1_SRDS_RST_MSK);
	return ret;
}

int config_serdes2_refclks(void)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	serdes_corenet_t *srds2_regs =
		(void *)CONFIG_SYS_FSL_CORENET_SERDES2_ADDR;
	u32 serdes2_prtcl;
	int ret = 0;
	int i;

	serdes2_prtcl = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS2_PRTCL;
	if (!serdes2_prtcl) {
		debug("SERDES2 is not enabled\n");
		return -ENODEV;
	}
	serdes2_prtcl >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;
	debug("Using SERDES2 Protocol: 0x%x:\n", serdes2_prtcl);

	/* To prevent generation of reset request from SerDes
	 * while changing the refclks, By setting SRDS_RST_MSK bit,
	 * SerDes reset event cannot cause a reset request
	 */
	setbits_be32(&gur->rstrqmr1, FSL_CORENET_RSTRQMR1_SRDS_RST_MSK);

	/* Reconfigure IDT idt8t49n222a device for PCIe SATA to work
	 * For this SerDes2's Refclk1 need to be set to 100MHz
	 */
	switch (serdes2_prtcl) {
#ifdef CONFIG_ARCH_B4420
	case 0x9d:
#endif
	case 0x9E:
	case 0x9A:
		/* fallthrough */
	case 0xb1:
	case 0xb2:
		debug("Configuring IDT for PCIe SATA for srds_prctl:%x\n",
			serdes2_prtcl);
		ret = select_i2c_ch_pca(I2C_CH_IDT);
		if (!ret) {
			ret = set_serdes_refclk(IDT_SERDES2_ADDRESS, 2,
					SERDES_REFCLK_100,
					SERDES_REFCLK_156_25, 0);
			if (ret) {
				printf("IDT8T49N222A configuration failed.\n");
				goto out;
			} else
				debug("IDT8T49N222A configured.\n");
		} else {
			goto out;
		}
		select_i2c_ch_pca(I2C_CH_DEFAULT);

		/* Steps For SerDes PLLs reset and reconfiguration after
		 * changing SerDes's refclks
		 */
		for (i = 0; i < CONFIG_SYS_FSL_SRDS_NUM_PLLS; i++) {
			clrbits_be32(&srds2_regs->bank[i].rstctl,
					SRDS_RSTCTL_SDRST_B);
			udelay(10);
			clrbits_be32(&srds2_regs->bank[i].rstctl,
				(SRDS_RSTCTL_SDEN | SRDS_RSTCTL_PLLRST_B));
			udelay(10);
			setbits_be32(&srds2_regs->bank[i].rstctl,
					SRDS_RSTCTL_RST);
			setbits_be32(&srds2_regs->bank[i].rstctl,
				(SRDS_RSTCTL_SDEN | SRDS_RSTCTL_PLLRST_B
				| SRDS_RSTCTL_SDRST_B));

			udelay(10);
		}
		break;
	default:
		printf("IDT configuration not supported for:%x S2 Protocol.\n",
			serdes2_prtcl);
	}

out:
	/* Clearing SRDS_RST_MSK bit as now
	 * SerDes reset event can cause a reset request
	 */
	clrbits_be32(&gur->rstrqmr1, FSL_CORENET_RSTRQMR1_SRDS_RST_MSK);
	return ret;
}

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);
	int ret;
	u32 svr = SVR_SOC_VER(get_svr());

	/* Create law for MAPLE only for personalities having MAPLE */
	if ((svr == SVR_B4860) || (svr == SVR_B4440) ||
	    (svr == SVR_B4420) || (svr == SVR_B4220)) {
		set_next_law(CONFIG_SYS_MAPLE_MEM_PHYS, LAW_SIZE_16M,
			     LAW_TRGT_IF_MAPLE);
	}

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

	/*
	 * Adjust core voltage according to voltage ID
	 * This function changes I2C mux to channel 2.
	 */
	if (adjust_vdd(0) < 0)
		printf("Warning: Adjusting core voltage failed\n");

	/* SerDes1 refclks need to be set again, as default clks
	 * are not suitable for CPRI and onboard SGMIIs to work
	 * simultaneously.
	 * This function will set SerDes1's Refclk1 and refclk2
	 * as per SerDes1 protocols
	 */
	if (config_serdes1_refclks())
		printf("SerDes1 Refclks couldn't set properly.\n");
	else
		printf("SerDes1 Refclks have been set.\n");

	/* SerDes2 refclks need to be set again, as default clks
	 * are not suitable for PCIe SATA to work
	 * This function will set SerDes2's Refclk1 and refclk2
	 * for SerDes2 protocols having PCIe in them
	 * for PCIe SATA to work
	 */
	ret = config_serdes2_refclks();
	if (!ret)
		printf("SerDes2 Refclks have been set.\n");
	else if (ret == -ENODEV)
		printf("SerDes disable, Refclks couldn't change.\n");
	else
		printf("SerDes2 Refclk reconfiguring failed.\n");

#if defined(CONFIG_SYS_FSL_ERRATUM_A006384) || \
			defined(CONFIG_SYS_FSL_ERRATUM_A006475)
	/* Rechecking the SerDes locks after all SerDes configurations
	 * are done, As SerDes PLLs may not lock reliably at 5 G VCO
	 * and at cold temperatures.
	 * Following sequence ensure the proper locking of SerDes PLLs.
	 */
	if (SVR_MAJ(get_svr()) == 1) {
		if (check_serdes_pll_locks())
			printf("SerDes plls still not locked properly.\n");
		else
			printf("SerDes plls have been locked well.\n");
	}
#endif

	/* Configure VSC3316 and VSC3308 crossbar switches */
	if (configure_vsc3316_3308())
		printf("VSC:failed to configure VSC3316/3308.\n");
	else
		printf("VSC:VSC3316/3308 successfully configured.\n");

	select_i2c_ch_pca(I2C_CH_DEFAULT);

	return 0;
}

unsigned long get_board_sys_clk(void)
{
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);

	switch ((sysclk_conf & 0x0C) >> 2) {
	case QIXIS_CLK_100:
		return 100000000;
	case QIXIS_CLK_125:
		return 125000000;
	case QIXIS_CLK_133:
		return 133333333;
	}
	return 66666666;
}

unsigned long get_board_ddr_clk(void)
{
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);

	switch (ddrclk_conf & 0x03) {
	case QIXIS_CLK_100:
		return 100000000;
	case QIXIS_CLK_125:
		return 125000000;
	case QIXIS_CLK_133:
		return 133333333;
	}
	return 66666666;
}

static int serdes_refclock(u8 sw, u8 sdclk)
{
	unsigned int clock;
	int ret = -1;
	u8 brdcfg4;

	if (sdclk == 1) {
		brdcfg4 = QIXIS_READ(brdcfg[4]);
		if ((brdcfg4 & CLK_MUX_SEL_MASK) == ETH_PHY_CLK_OUT)
			return SRDS_PLLCR0_RFCK_SEL_125;
		else
			clock = (sw >> 5) & 7;
	} else
		clock = (sw >> 6) & 3;

	switch (clock) {
	case 0:
		ret = SRDS_PLLCR0_RFCK_SEL_100;
		break;
	case 1:
		ret = SRDS_PLLCR0_RFCK_SEL_125;
		break;
	case 2:
		ret = SRDS_PLLCR0_RFCK_SEL_156_25;
		break;
	case 3:
		ret = SRDS_PLLCR0_RFCK_SEL_161_13;
		break;
	case 4:
	case 5:
	case 6:
		ret = SRDS_PLLCR0_RFCK_SEL_122_88;
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

#define NUM_SRDS_BANKS	2

int misc_init_r(void)
{
	u8 sw;
	serdes_corenet_t *srds_regs =
		(void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	u32 actual[NUM_SRDS_BANKS];
	unsigned int i;
	int clock;

	sw = QIXIS_READ(brdcfg[2]);
	clock = serdes_refclock(sw, 1);
	if (clock >= 0)
		actual[0] = clock;
	else
		printf("Warning: SDREFCLK1 switch setting is unsupported\n");

	sw = QIXIS_READ(brdcfg[4]);
	clock = serdes_refclock(sw, 2);
	if (clock >= 0)
		actual[1] = clock;
	else
		printf("Warning: SDREFCLK2 switch setting unsupported\n");

	for (i = 0; i < NUM_SRDS_BANKS; i++) {
		u32 pllcr0 = srds_regs->bank[i].pllcr0;
		u32 expected = pllcr0 & SRDS_PLLCR0_RFCK_SEL_MASK;
		if (expected != actual[i]) {
			printf("Warning: SERDES bank %u expects reference clock"
			       " %sMHz, but actual is %sMHz\n", i + 1,
			       serdes_clock_to_string(expected),
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

#ifdef CONFIG_HAS_FSL_DR_USB
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
	fdt_fixup_board_enet(blob);
#endif

	return 0;
}

/*
 * Dump board switch settings.
 * The bits that cannot be read/sampled via some FPGA or some
 * registers, they will be displayed as
 * underscore in binary format. mask[] has those bits.
 * Some bits are calculated differently than the actual switches
 * if booting with overriding by FPGA.
 */
void qixis_dump_switch(void)
{
	int i;
	u8 sw[5];

	/*
	 * Any bit with 1 means that bit cannot be reverse engineered.
	 * It will be displayed as _ in binary format.
	 */
	static const u8 mask[] = {0x07, 0, 0, 0xff, 0};
	char buf[10];
	u8 brdcfg[16], dutcfg[16];

	for (i = 0; i < 16; i++) {
		brdcfg[i] = qixis_read(offsetof(struct qixis, brdcfg[0]) + i);
		dutcfg[i] = qixis_read(offsetof(struct qixis, dutcfg[0]) + i);
	}

	sw[0] = ((brdcfg[0] & 0x0f) << 4)	| \
		(brdcfg[9] & 0x08);
	sw[1] = ((dutcfg[1] & 0x01) << 7)	| \
		((dutcfg[2] & 0x07) << 4)       | \
		((dutcfg[6] & 0x10) >> 1)       | \
		((dutcfg[6] & 0x80) >> 5)       | \
		((dutcfg[1] & 0x40) >> 5)       | \
		(dutcfg[6] & 0x01);
	sw[2] = dutcfg[0];
	sw[3] = 0;
	sw[4] = ((brdcfg[1] & 0x30) << 2)	| \
		((brdcfg[1] & 0xc0) >> 2)	| \
		(brdcfg[1] & 0x0f);

	puts("DIP switch settings:\n");
	for (i = 0; i < 5; i++) {
		printf("SW%d         = 0b%s (0x%02x)\n",
			i + 1, byte_to_binary_mask(sw[i], mask[i], buf), sw[i]);
	}
}
