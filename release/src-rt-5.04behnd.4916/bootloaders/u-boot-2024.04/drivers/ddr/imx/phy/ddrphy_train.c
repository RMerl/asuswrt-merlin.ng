// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <log.h>
#include <linux/kernel.h>
#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>

int ddr_cfg_phy(struct dram_timing_info *dram_timing)
{
	struct dram_cfg_param *dram_cfg;
	struct dram_fsp_msg *fsp_msg;
	unsigned int num;
	int i = 0;
	int j = 0;
	int ret;

	/* initialize PHY configuration */
	dram_cfg = dram_timing->ddrphy_cfg;
	num  = dram_timing->ddrphy_cfg_num;
	for (i = 0; i < num; i++) {
		/* config phy reg */
		dwc_ddrphy_apb_wr(dram_cfg->reg, dram_cfg->val);
		dram_cfg++;
	}

	/* load the frequency setpoint message block config */
	fsp_msg = dram_timing->fsp_msg;
	for (i = 0; i < dram_timing->fsp_msg_num; i++) {
		debug("DRAM PHY training for %dMTS\n", fsp_msg->drate);
		/* set dram PHY input clocks to desired frequency */
		ddrphy_init_set_dfi_clk(fsp_msg->drate);

		/* load the dram training firmware image */
		dwc_ddrphy_apb_wr(0xd0000, 0x0);
		ddr_load_train_firmware(fsp_msg->fw_type);

		/* load the frequency set point message block parameter */
		dram_cfg = fsp_msg->fsp_cfg;
		num = fsp_msg->fsp_cfg_num;
		for (j = 0; j < num; j++) {
			dwc_ddrphy_apb_wr(dram_cfg->reg, dram_cfg->val);
			dram_cfg++;
		}

		/*
		 * -------------------- excute the firmware --------------------
		 * Running the firmware is a simply process to taking the
		 * PMU out of reset and stall, then the firwmare will be run
		 * 1. reset the PMU;
		 * 2. begin the excution;
		 * 3. wait for the training done;
		 * 4. read the message block result.
		 * -------------------------------------------------------------
		 */
		dwc_ddrphy_apb_wr(0xd0000, 0x1);
		dwc_ddrphy_apb_wr(0xd0099, 0x9);
		dwc_ddrphy_apb_wr(0xd0099, 0x1);
		dwc_ddrphy_apb_wr(0xd0099, 0x0);

		/* Wait for the training firmware to complete */
		ret = wait_ddrphy_training_complete();
		if (ret)
			return ret;

		/* Halt the microcontroller. */
		dwc_ddrphy_apb_wr(0xd0099, 0x1);

		/* Read the Message Block results */
		dwc_ddrphy_apb_wr(0xd0000, 0x0);

		ddrphy_init_read_msg_block(fsp_msg->fw_type);

		if(fsp_msg->fw_type != FW_2D_IMAGE)
			get_trained_CDD(i);

		dwc_ddrphy_apb_wr(0xd0000, 0x1);


		fsp_msg++;
	}

	/* Load PHY Init Engine Image */
	dram_cfg = dram_timing->ddrphy_pie;
	num = dram_timing->ddrphy_pie_num;
	for (i = 0; i < num; i++) {
		dwc_ddrphy_apb_wr(dram_cfg->reg, dram_cfg->val);
		dram_cfg++;
	}

	/* save the ddr PHY trained CSR in memory for low power use */
	ddrphy_trained_csr_save(ddrphy_trained_csr, ddrphy_trained_csr_num);

	return 0;
}
