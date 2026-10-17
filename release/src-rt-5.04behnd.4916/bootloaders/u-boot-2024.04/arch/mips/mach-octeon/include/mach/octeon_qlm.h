/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __OCTEON_QLM_H__
#define __OCTEON_QLM_H__

/* Reference clock selector values for ref_clk_sel */
#define OCTEON_QLM_REF_CLK_100MHZ 0 /** 100 MHz */
#define OCTEON_QLM_REF_CLK_125MHZ 1 /** 125 MHz */
#define OCTEON_QLM_REF_CLK_156MHZ 2 /** 156.25 MHz */
#define OCTEON_QLM_REF_CLK_161MHZ 3 /** 161.1328125 MHz */

/**
 * Configure qlm/dlm speed and mode.
 * @param qlm     The QLM or DLM to configure
 * @param speed   The speed the QLM needs to be configured in Mhz.
 * @param mode    The QLM to be configured as SGMII/XAUI/PCIe.
 * @param rc      Only used for PCIe, rc = 1 for root complex mode, 0 for EP
 *		  mode.
 * @param pcie_mode Only used when qlm/dlm are in pcie mode.
 * @param ref_clk_sel Reference clock to use for 70XX where:
 *			0: 100MHz
 *			1: 125MHz
 *			2: 156.25MHz
 *			3: 161.1328125MHz (CN73XX and CN78XX only)
 * @param ref_clk_input	This selects which reference clock input to use.  For
 *			cn70xx:
 *				0: DLMC_REF_CLK0
 *				1: DLMC_REF_CLK1
 *				2: DLM0_REF_CLK
 *			cn61xx: (not used)
 *			cn78xx/cn76xx/cn73xx:
 *				0: Internal clock (QLM[0-7]_REF_CLK)
 *				1: QLMC_REF_CLK0
 *				2: QLMC_REF_CLK1
 *
 * Return:	Return 0 on success or -1.
 *
 * @note	When the 161MHz clock is used it can only be used for
 *		XLAUI mode with a 6316 speed or XFI mode with a 103125 speed.
 *		This rate is also only supported for CN73XX and CN78XX.
 */
int octeon_configure_qlm(int qlm, int speed, int mode, int rc, int pcie_mode, int ref_clk_sel,
			 int ref_clk_input);

int octeon_configure_qlm_cn78xx(int node, int qlm, int speed, int mode, int rc, int pcie_mode,
				int ref_clk_sel, int ref_clk_input);

/**
 * Some QLM speeds need to override the default tuning parameters
 *
 * @param node     Node to configure
 * @param qlm      QLM to configure
 * @param baud_mhz Desired speed in MHz
 * @param lane     Lane the apply the tuning parameters
 * @param tx_swing Voltage swing.  The higher the value the lower the voltage,
 *		   the default value is 7.
 * @param tx_pre   pre-cursor pre-emphasis
 * @param tx_post  post-cursor pre-emphasis.
 * @param tx_gain   Transmit gain. Range 0-7
 * @param tx_vboost Transmit voltage boost. Range 0-1
 */
void octeon_qlm_tune_per_lane_v3(int node, int qlm, int baud_mhz, int lane, int tx_swing,
				 int tx_pre, int tx_post, int tx_gain, int tx_vboost);

/**
 * Some QLM speeds need to override the default tuning parameters
 *
 * @param node     Node to configure
 * @param qlm      QLM to configure
 * @param baud_mhz Desired speed in MHz
 * @param tx_swing Voltage swing.  The higher the value the lower the voltage,
 *		   the default value is 7.
 * @param tx_premptap bits [0:3] pre-cursor pre-emphasis, bits[4:8] post-cursor
 *		      pre-emphasis.
 * @param tx_gain   Transmit gain. Range 0-7
 * @param tx_vboost Transmit voltage boost. Range 0-1
 */
void octeon_qlm_tune_v3(int node, int qlm, int baud_mhz, int tx_swing, int tx_premptap, int tx_gain,
			int tx_vboost);

/**
 * Disables DFE for the specified QLM lane(s).
 * This function should only be called for low-loss channels.
 *
 * @param node     Node to configure
 * @param qlm      QLM to configure
 * @param lane     Lane to configure, or -1 all lanes
 * @param baud_mhz The speed the QLM needs to be configured in Mhz.
 * @param mode     The QLM to be configured as SGMII/XAUI/PCIe.
 */
void octeon_qlm_dfe_disable(int node, int qlm, int lane, int baud_mhz, int mode);

/**
 * Some QLMs need to override the default pre-ctle for low loss channels.
 *
 * @param node     Node to configure
 * @param qlm      QLM to configure
 * @param pre_ctle pre-ctle settings for low loss channels
 */
void octeon_qlm_set_channel_v3(int node, int qlm, int pre_ctle);

void octeon_init_qlm(int node);

int octeon_mcu_probe(int node);

#endif /* __OCTEON_QLM_H__ */
