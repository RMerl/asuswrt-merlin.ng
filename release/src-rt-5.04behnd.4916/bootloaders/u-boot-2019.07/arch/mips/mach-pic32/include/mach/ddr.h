/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) 2015 Purna Chandra Mandal <purna.mandal@microchip.com>
 *
 */

#ifndef __MICROCHIP_PIC32_DDR_H
#define __MICROCHIP_PIC32_DDR_H

/* called by dram_init() function */
void ddr2_phy_init(void);
void ddr2_ctrl_init(void);
phys_size_t ddr2_calculate_size(void);

/* Maximum number of agents */
#define NUM_AGENTS		5

/* Board can provide agent specific parameters for arbitration by
 * filling struct ddr2_arbiter_params for all the agents and
 * implementing board_get_ddr_arbiter_params() to return the filled
 * structure.
 */
struct ddr2_arbiter_params {
	u32 min_limit;	/* min bursts to execute per arbitration */
	u32 req_period; /* request period threshold for accepted cmds */
	u32 min_cmd_acpt; /* min number of accepted cmds */
};

const struct ddr2_arbiter_params *board_get_ddr_arbiter_params(void);

#endif /* __MICROCHIP_PIC32_DDR_H */
