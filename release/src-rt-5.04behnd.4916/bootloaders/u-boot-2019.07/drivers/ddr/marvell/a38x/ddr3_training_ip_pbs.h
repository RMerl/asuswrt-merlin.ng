/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _DDR3_TRAINING_IP_PBS_H_
#define _DDR3_TRAINING_IP_PBS_H_

enum {
	EBA_CONFIG,
	EEBA_CONFIG,
	SBA_CONFIG
};

enum hws_training_load_op {
	TRAINING_LOAD_OPERATION_UNLOAD,
	TRAINING_LOAD_OPERATION_LOAD
};

enum hws_edge {
	TRAINING_EDGE_1,
	TRAINING_EDGE_2
};

enum hws_edge_search {
	TRAINING_EDGE_MAX,
	TRAINING_EDGE_MIN
};

enum pbs_dir {
	PBS_TX_MODE = 0,
	PBS_RX_MODE,
	NUM_OF_PBS_MODES
};

int ddr3_tip_pbs_rx(u32 dev_num);
int ddr3_tip_print_all_pbs_result(u32 dev_num);
int ddr3_tip_pbs_tx(u32 dev_num);

#endif /* _DDR3_TRAINING_IP_PBS_H_ */
