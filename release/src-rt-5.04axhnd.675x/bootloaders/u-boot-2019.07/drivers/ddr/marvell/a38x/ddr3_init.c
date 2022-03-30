// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "ddr3_init.h"
#include "mv_ddr_common.h"

static char *ddr_type = "DDR3";

/*
 * generic_init_controller controls D-unit configuration:
 * '1' - dynamic D-unit configuration,
 */
u8 generic_init_controller = 1;

static int mv_ddr_training_params_set(u8 dev_num);

/*
 * Name:     ddr3_init - Main DDR3 Init function
 * Desc:     This routine initialize the DDR3 MC and runs HW training.
 * Args:     None.
 * Notes:
 * Returns:  None.
 */
int ddr3_init(void)
{
	int status;
	int is_manual_cal_done;

	/* Print mv_ddr version */
	mv_ddr_ver_print();

	mv_ddr_pre_training_fixup();

	/* SoC/Board special initializations */
	mv_ddr_pre_training_soc_config(ddr_type);

	/* Set log level for training library */
	mv_ddr_user_log_level_set(DEBUG_BLOCK_ALL);

	mv_ddr_early_init();

	if (mv_ddr_topology_map_update()) {
		printf("mv_ddr: failed to update topology\n");
		return MV_FAIL;
	}

	if (mv_ddr_early_init2() != MV_OK)
		return MV_FAIL;

	/* Set training algorithm's parameters */
	status = mv_ddr_training_params_set(0);
	if (MV_OK != status)
		return status;

	mv_ddr_mc_config();

	is_manual_cal_done = mv_ddr_manual_cal_do();

	mv_ddr_mc_init();

	if (!is_manual_cal_done) {
	}


	status = ddr3_silicon_post_init();
	if (MV_OK != status) {
		printf("DDR3 Post Init - FAILED 0x%x\n", status);
		return status;
	}

	/* PHY initialization (Training) */
	status = hws_ddr3_tip_run_alg(0, ALGO_TYPE_DYNAMIC);
	if (MV_OK != status) {
		printf("%s Training Sequence - FAILED\n", ddr_type);
		return status;
	}

#if defined(CONFIG_PHY_STATIC_PRINT)
	mv_ddr_phy_static_print();
#endif

	/* Post MC/PHY initializations */
	mv_ddr_post_training_soc_config(ddr_type);

	mv_ddr_post_training_fixup();

	if (mv_ddr_is_ecc_ena())
		mv_ddr_mem_scrubbing();

	printf("mv_ddr: completed successfully\n");

	return MV_OK;
}

/*
 * Name:	mv_ddr_training_params_set
 * Desc:
 * Args:
 * Notes:	sets internal training params
 * Returns:
 */
static int mv_ddr_training_params_set(u8 dev_num)
{
	struct tune_train_params params;
	int status;
	u32 cs_num;

	cs_num = mv_ddr_cs_num_get();

	/* NOTE: do not remove any field initilization */
	params.ck_delay = TUNE_TRAINING_PARAMS_CK_DELAY;
	params.phy_reg3_val = TUNE_TRAINING_PARAMS_PHYREG3VAL;
	params.g_zpri_data = TUNE_TRAINING_PARAMS_PRI_DATA;
	params.g_znri_data = TUNE_TRAINING_PARAMS_NRI_DATA;
	params.g_zpri_ctrl = TUNE_TRAINING_PARAMS_PRI_CTRL;
	params.g_znri_ctrl = TUNE_TRAINING_PARAMS_NRI_CTRL;
	params.g_znodt_data = TUNE_TRAINING_PARAMS_N_ODT_DATA;
	params.g_zpodt_ctrl = TUNE_TRAINING_PARAMS_P_ODT_CTRL;
	params.g_znodt_ctrl = TUNE_TRAINING_PARAMS_N_ODT_CTRL;

	params.g_zpodt_data = TUNE_TRAINING_PARAMS_P_ODT_DATA;
	params.g_dic = TUNE_TRAINING_PARAMS_DIC;
	params.g_rtt_nom = TUNE_TRAINING_PARAMS_RTT_NOM;
	if (cs_num == 1) {
		params.g_rtt_wr = TUNE_TRAINING_PARAMS_RTT_WR_1CS;
		params.g_odt_config = TUNE_TRAINING_PARAMS_ODT_CONFIG_1CS;
	} else {
		params.g_rtt_wr = TUNE_TRAINING_PARAMS_RTT_WR_2CS;
		params.g_odt_config = TUNE_TRAINING_PARAMS_ODT_CONFIG_2CS;
	}

	status = ddr3_tip_tune_training_params(dev_num, &params);
	if (MV_OK != status) {
		printf("%s Training Sequence - FAILED\n", ddr_type);
		return status;
	}

	return MV_OK;
}
