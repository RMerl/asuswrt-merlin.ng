/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _DDR3_LOGGING_CONFIG_H
#define _DDR3_LOGGING_CONFIG_H

#ifdef SILENT_LIB
#define DEBUG_TRAINING_BIST_ENGINE(level, s)
#define DEBUG_TRAINING_IP(level, s)
#define DEBUG_CENTRALIZATION_ENGINE(level, s)
#define DEBUG_TRAINING_HW_ALG(level, s)
#define DEBUG_TRAINING_IP_ENGINE(level, s)
#define DEBUG_LEVELING(level, s)
#define DEBUG_PBS_ENGINE(level, s)
#define DEBUG_TRAINING_STATIC_IP(level, s)
#define DEBUG_TRAINING_ACCESS(level, s)
#else
#ifdef LIB_FUNCTIONAL_DEBUG_ONLY
#define DEBUG_TRAINING_BIST_ENGINE(level, s)
#define DEBUG_TRAINING_IP_ENGINE(level, s)
#define DEBUG_TRAINING_IP(level, s)		\
	if (level >= debug_training)		\
		printf s
#define DEBUG_CENTRALIZATION_ENGINE(level, s)	\
	if (level >= debug_centralization)	\
		printf s
#define DEBUG_TRAINING_HW_ALG(level, s)		\
	if (level >= debug_training_hw_alg)	\
		printf s
#define DEBUG_LEVELING(level, s)		\
	if (level >= debug_leveling)		\
		printf s
#define DEBUG_PBS_ENGINE(level, s)		\
	if (level >= debug_pbs)			\
		printf s
#define DEBUG_TRAINING_STATIC_IP(level, s)	\
	if (level >= debug_training_static)	\
		printf s
#define DEBUG_TRAINING_ACCESS(level, s)		\
	if (level >= debug_training_access)	\
		printf s
#else
#define DEBUG_TRAINING_BIST_ENGINE(level, s)	\
	if (level >= debug_training_bist)	\
		printf s

#define DEBUG_TRAINING_IP_ENGINE(level, s)	\
	if (level >= debug_training_ip)		\
		printf s
#define DEBUG_TRAINING_IP(level, s)		\
	if (level >= debug_training)		\
		printf s
#define DEBUG_CENTRALIZATION_ENGINE(level, s)	\
	if (level >= debug_centralization)	\
		printf s
#define DEBUG_TRAINING_HW_ALG(level, s)		\
	if (level >= debug_training_hw_alg)	\
		printf s
#define DEBUG_LEVELING(level, s)		\
	if (level >= debug_leveling)		\
		printf s
#define DEBUG_PBS_ENGINE(level, s)		\
	if (level >= debug_pbs)			\
		printf s
#define DEBUG_TRAINING_STATIC_IP(level, s)	\
	if (level >= debug_training_static)	\
		printf s
#define DEBUG_TRAINING_ACCESS(level, s)		\
	if (level >= debug_training_access)	\
		printf s
#endif
#endif


/* Logging defines */
enum mv_ddr_debug_level {
	DEBUG_LEVEL_TRACE = 1,
	DEBUG_LEVEL_INFO = 2,
	DEBUG_LEVEL_ERROR = 3,
	DEBUG_LEVEL_LAST
};

enum ddr_lib_debug_block {
	DEBUG_BLOCK_STATIC,
	DEBUG_BLOCK_TRAINING_MAIN,
	DEBUG_BLOCK_LEVELING,
	DEBUG_BLOCK_CENTRALIZATION,
	DEBUG_BLOCK_PBS,
	DEBUG_BLOCK_IP,
	DEBUG_BLOCK_BIST,
	DEBUG_BLOCK_ALG,
	DEBUG_BLOCK_DEVICE,
	DEBUG_BLOCK_ACCESS,
	DEBUG_STAGES_REG_DUMP,
	/* All excluding IP and REG_DUMP, should be enabled separatelly */
	DEBUG_BLOCK_ALL
};

int ddr3_tip_print_log(u32 dev_num, u32 mem_addr);
int ddr3_tip_print_stability_log(u32 dev_num);

#endif /* _DDR3_LOGGING_CONFIG_H */
