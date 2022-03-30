/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef __DDR3_TRAINING_H
#define __DDR3_TRAINING_H

#include "ddr3_init.h"

#ifdef MV88F78X60
#include "ddr3_axp.h"
#elif defined(MV88F67XX)
#include "ddr3_a370.h"
#elif defined(MV88F672X)
#include "ddr3_a375.h"
#endif

/* The following is a list of Marvell status    */
#define MV_ERROR	(-1)
#define MV_OK		(0x00)	/* Operation succeeded                   */
#define MV_FAIL		(0x01)	/* Operation failed                      */
#define MV_BAD_VALUE	(0x02)	/* Illegal value (general)               */
#define MV_OUT_OF_RANGE	(0x03)	/* The value is out of range             */
#define MV_BAD_PARAM	(0x04)	/* Illegal parameter in function called  */
#define MV_BAD_PTR	(0x05)	/* Illegal pointer value                 */
#define MV_BAD_SIZE	(0x06)	/* Illegal size                          */
#define MV_BAD_STATE	(0x07)	/* Illegal state of state machine        */
#define MV_SET_ERROR	(0x08)	/* Set operation failed                  */
#define MV_GET_ERROR	(0x09)	/* Get operation failed                  */
#define MV_CREATE_ERROR	(0x0A)	/* Fail while creating an item           */
#define MV_NOT_FOUND	(0x0B)	/* Item not found                        */
#define MV_NO_MORE	(0x0C)	/* No more items found                   */
#define MV_NO_SUCH	(0x0D)	/* No such item                          */
#define MV_TIMEOUT	(0x0E)	/* Time Out                              */
#define MV_NO_CHANGE	(0x0F)	/* Parameter(s) is already in this value */
#define MV_NOT_SUPPORTED (0x10)	/* This request is not support           */
#define MV_NOT_IMPLEMENTED (0x11) /* Request supported but not implemented*/
#define MV_NOT_INITIALIZED (0x12) /* The item is not initialized          */
#define MV_NO_RESOURCE	(0x13)	/* Resource not available (memory ...)   */
#define MV_FULL		(0x14)	/* Item is full (Queue or table etc...)  */
#define MV_EMPTY	(0x15)	/* Item is empty (Queue or table etc...) */
#define MV_INIT_ERROR	(0x16)	/* Error occurred while INIT process      */
#define MV_HW_ERROR	(0x17)	/* Hardware error                        */
#define MV_TX_ERROR	(0x18)	/* Transmit operation not succeeded      */
#define MV_RX_ERROR	(0x19)	/* Recieve operation not succeeded       */
#define MV_NOT_READY	(0x1A)	/* The other side is not ready yet       */
#define MV_ALREADY_EXIST (0x1B)	/* Tried to create existing item         */
#define MV_OUT_OF_CPU_MEM   (0x1C) /* Cpu memory allocation failed.      */
#define MV_NOT_STARTED	(0x1D)	/* Not started yet                       */
#define MV_BUSY		(0x1E)	/* Item is busy.                         */
#define MV_TERMINATE	(0x1F)	/* Item terminates it's work.            */
#define MV_NOT_ALIGNED	(0x20)	/* Wrong alignment                       */
#define MV_NOT_ALLOWED	(0x21)	/* Operation NOT allowed                 */
#define MV_WRITE_PROTECT (0x22)	/* Write protected                       */

#define MV_INVALID	(int)(-1)

/*
 * Debug (Enable/Disable modules) and Error report
 */

#ifdef BASIC_DEBUG
#define MV_DEBUG_WL
#define MV_DEBUG_RL
#define MV_DEBUG_DQS_RESULTS
#endif

#ifdef FULL_DEBUG
#define MV_DEBUG_WL
#define MV_DEBUG_RL
#define MV_DEBUG_DQS

#define MV_DEBUG_PBS
#define MV_DEBUG_DFS
#define MV_DEBUG_MAIN_FULL
#define MV_DEBUG_DFS_FULL
#define MV_DEBUG_DQS_FULL
#define MV_DEBUG_RL_FULL
#define MV_DEBUG_WL_FULL
#endif

/*
 * General Consts
 */

#define SDRAM_READ_WRITE_LEN_IN_WORDS           16
#define SDRAM_READ_WRITE_LEN_IN_DOUBLE_WORDS    8
#define CACHE_LINE_SIZE                         0x20

#define SDRAM_CS_BASE                           0x0

#define SRAM_BASE                               0x40000000
#define SRAM_SIZE                               0xFFF

#define LEN_64BIT_STD_PATTERN                   16
#define LEN_64BIT_KILLER_PATTERN                128
#define LEN_64BIT_SPECIAL_PATTERN               128
#define LEN_64BIT_PBS_PATTERN                   16
#define LEN_WL_SUP_PATTERN		                32

#define LEN_16BIT_STD_PATTERN                   4
#define LEN_16BIT_KILLER_PATTERN                128
#define LEN_16BIT_SPECIAL_PATTERN               128
#define LEN_16BIT_PBS_PATTERN                   4

#define CMP_BYTE_SHIFT                          8
#define CMP_BYTE_MASK                           0xFF
#define PUP_SIZE                                8

#define S 0
#define C 1
#define P 2
#define D 3
#define DQS 6
#define PS 2
#define DS 3
#define PE 4
#define DE 5

#define CS0                                     0
#define MAX_DIMM_NUM                            2
#define MAX_DELAY                               0x1F

/*
 * Invertion limit and phase1 limit are WA for the RL @ 1:1 design bug -
 * Armada 370 & AXP Z1
 */
#define MAX_DELAY_INV_LIMIT                     0x5
#define MIN_DELAY_PHASE_1_LIMIT                 0x10

#define MAX_DELAY_INV                           (0x3F - MAX_DELAY_INV_LIMIT)
#define MIN_DELAY                               0
#define MAX_PUP_NUM                             9
#define ECC_PUP                                 8
#define DQ_NUM                                  8
#define DQS_DQ_NUM                              8
#define INIT_WL_DELAY                           13
#define INIT_RL_DELAY                           15
#define TWLMRD_DELAY                            20
#define TCLK_3_DELAY                            3
#define ECC_BIT                                 8
#define DMA_SIZE                                64
#define MV_DMA_0                                0
#define MAX_TRAINING_RETRY                      10

#define PUP_RL_MODE                             0x2
#define PUP_WL_MODE                             0
#define PUP_PBS_TX                              0x10
#define PUP_PBS_TX_DM                           0x1A
#define PUP_PBS_RX                              0x30
#define PUP_DQS_WR                              0x1
#define PUP_DQS_RD                              0x3
#define PUP_BC                                  10
#define PUP_DELAY_MASK                          0x1F
#define PUP_PHASE_MASK                          0x7
#define PUP_NUM_64BIT                           8
#define PUP_NUM_32BIT                           4
#define PUP_NUM_16BIT                           2

/* control PHY registers */
#define CNTRL_PUP_DESKEW                        0x10

/* WL */
#define COUNT_WL_HI_FREQ                        2
#define COUNT_WL                                2
#define COUNT_WL_RFRS                           9
#define WL_HI_FREQ_SHIFT                        2
#define WL_HI_FREQ_STATE                        1
#define COUNT_HW_WL                             2

/* RL */
/*
 * RL_MODE - this define uses the RL mode SW RL instead of the functional
 * window SW RL
 */
#define RL_MODE
#define RL_WINDOW_WA
#define MAX_PHASE_1TO1                          2
#define MAX_PHASE_2TO1                          4

#define MAX_PHASE_RL_UL_1TO1                    0
#define MAX_PHASE_RL_L_1TO1                     4
#define MAX_PHASE_RL_UL_2TO1                    3
#define MAX_PHASE_RL_L_2TO1                     7

#define RL_UNLOCK_STATE                         0
#define RL_WINDOW_STATE                         1
#define RL_FINAL_STATE                          2
#define RL_RETRY_COUNT                          2
#define COUNT_HW_RL                             2

/* PBS */
#define MAX_PBS                                 31
#define MIN_PBS                                 0
#define COUNT_PBS_PATTERN                       2
#define COUNT_PBS_STARTOVER                     2
#define COUNT_PBS_REPEAT                        3
#define COUNT_PBS_COMP_RETRY_NUM                2
#define PBS_DIFF_LIMIT                          31
#define PATTERN_PBS_TX_A                        0x55555555
#define PATTERN_PBS_TX_B                        0xAAAAAAAA

/* DQS */
#define ADLL_ERROR                              0x55
#define ADLL_MAX                                31
#define ADLL_MIN                                0
#define MIN_WIN_SIZE                            4
#define VALID_WIN_THRS                          MIN_WIN_SIZE

#define MODE_2TO1                               1
#define MODE_1TO1                               0

/*
 * Macros
 */
#define IS_PUP_ACTIVE(_data_, _pup_)        (((_data_) >> (_pup_)) & 0x1)

/*
 * Internal ERROR codes
 */
#define MV_DDR3_TRAINING_ERR_WR_LVL_HW              0xDD302001
#define MV_DDR3_TRAINING_ERR_LOAD_PATTERNS          0xDD302002
#define MV_DDR3_TRAINING_ERR_WR_LVL_HI_FREQ         0xDD302003
#define MV_DDR3_TRAINING_ERR_DFS_H2L                0xDD302004
#define MV_DDR3_TRAINING_ERR_DRAM_COMPARE           0xDD302005
#define MV_DDR3_TRAINING_ERR_WIN_LIMITS             0xDD302006
#define MV_DDR3_TRAINING_ERR_PUP_RANGE              0xDD302025
#define MV_DDR3_TRAINING_ERR_DQS_LOW_LIMIT_SEARCH   0xDD302007
#define MV_DDR3_TRAINING_ERR_DQS_HIGH_LIMIT_SEARCH  0xDD302008
#define MV_DDR3_TRAINING_ERR_DQS_PATTERN            0xDD302009
#define MV_DDR3_TRAINING_ERR_PBS_ADLL_SHR_1PHASE    0xDD302010
#define MV_DDR3_TRAINING_ERR_PBS_TX_MAX_VAL         0xDD302011
#define MV_DDR3_TRAINING_ERR_PBS_RX_PER_BIT         0xDD302012
#define MV_DDR3_TRAINING_ERR_PBS_TX_PER_BIT         0xDD302013
#define MV_DDR3_TRAINING_ERR_PBS_RX_MAX_VAL         0xDD302014
#define MV_DDR3_TRAINING_ERR_PBS_SHIFT_QDS_SRAM_CMP 0xDD302015
#define MV_DDR3_TRAINING_ERR_PBS_SHIFT_QDS_MAX_VAL  0xDD302016
#define MV_DDR3_TRAINING_ERR_RD_LVL_RL_PATTERN      0xDD302017
#define MV_DDR3_TRAINING_ERR_RD_LVL_RL_PUP_UNLOCK   0xDD302018
#define MV_DDR3_TRAINING_ERR_RD_LVL_PUP_UNLOCK      0xDD302019
#define MV_DDR3_TRAINING_ERR_WR_LVL_SW              0xDD302020
#define MV_DDR3_TRAINING_ERR_PRBS_RX                0xDD302021
#define MV_DDR3_TRAINING_ERR_DQS_RX                 0xDD302022
#define MV_DDR3_TRAINING_ERR_PRBS_TX                0xDD302023
#define MV_DDR3_TRAINING_ERR_DQS_TX                 0xDD302024

/*
 * DRAM information structure
 */
typedef struct dram_info {
	u32 num_cs;
	u32 cs_ena;
	u32 num_of_std_pups;	/* Q value = ddrWidth/8 - Without ECC!! */
	u32 num_of_total_pups;	/* numOfStdPups + eccEna */
	u32 target_frequency;	/* DDR Frequency */
	u32 ddr_width;		/* 32/64 Bit or 16/32 Bit */
	u32 ecc_ena;		/* 0/1 */
	u32 wl_val[MAX_CS][MAX_PUP_NUM][7];
	u32 rl_val[MAX_CS][MAX_PUP_NUM][7];
	u32 rl_max_phase;
	u32 rl_min_phase;
	u32 wl_max_phase;
	u32 wl_min_phase;
	u32 rd_smpl_dly;
	u32 rd_rdy_dly;
	u32 cl;
	u32 cwl;
	u32 mode_2t;
	int rl400_bug;
	int multi_cs_mr_support;
	int reg_dimm;
} MV_DRAM_INFO;

enum training_modes  {
	DQS_WR_MODE,
	WL_MODE_,
	RL_MODE_,
	DQS_RD_MODE,
	PBS_TX_DM_MODE,
	PBS_TX_MODE,
	PBS_RX_MODE,
	MAX_TRAINING_MODE,
};

typedef struct dram_training_init {
	u32 reg_addr;
	u32 reg_value;
} MV_DRAM_TRAINING_INIT;

typedef struct dram_mv_init {
	u32 reg_addr;
	u32 reg_value;
} MV_DRAM_MC_INIT;

/* Board/Soc revisions define */
enum board_rev {
	Z1,
	Z1_PCAC,
	Z1_RD_SLED,
	A0,
	A0_AMC
};

typedef struct dram_modes {
	char *mode_name;
	u8 cpu_freq;
	u8 fab_freq;
	u8 chip_id;
	int chip_board_rev;
	MV_DRAM_MC_INIT *regs;
	MV_DRAM_TRAINING_INIT *vals;
} MV_DRAM_MODES;

/*
 * Function Declarations
 */

u32 cache_inv(u32 addr);
void flush_l1_v7(u32 line);
void flush_l1_v6(u32 line);

u32 ddr3_cl_to_valid_cl(u32 cl);
u32 ddr3_valid_cl_to_cl(u32 ui_valid_cl);

void ddr3_write_pup_reg(u32 mode, u32 cs, u32 pup, u32 phase, u32 delay);
u32 ddr3_read_pup_reg(u32 mode, u32 cs, u32 pup);

int ddr3_sdram_pbs_compare(MV_DRAM_INFO *dram_info, u32 pup_locked, int is_tx,
			   u32 pbs_pattern_idx, u32 pbs_curr_val,
			   u32 pbs_lock_val, u32 *skew_array,
			   u8 *unlock_pup_dq_array, u32 ecc);

int ddr3_sdram_dqs_compare(MV_DRAM_INFO *dram_info, u32 unlock_pup,
			   u32 *new_locked_pup, u32 *pattern,
			   u32 pattern_len, u32 sdram_offset, int write,
			   int mask, u32 *mask_pattern, int b_special_compare);

int ddr3_sdram_compare(MV_DRAM_INFO *dram_info, u32 unlock_pup,
		       u32 *new_locked_pup, u32 *pattern, u32 pattern_len,
		       u32 sdram_offset, int write, int mask,
		       u32 *mask_pattern, int b_special_compare);

int ddr3_sdram_direct_compare(MV_DRAM_INFO *dram_info, u32 unlock_pup,
			      u32 *new_locked_pup, u32 *pattern,
			      u32 pattern_len, u32 sdram_offset, int write,
			      int mask, u32 *mask_pattern);

int ddr3_sdram_dm_compare(MV_DRAM_INFO *dram_info, u32 unlock_pup,
			  u32 *new_locked_pup, u32 *pattern,
			  u32 sdram_offset);
int ddr3_dram_sram_read(u32 src, u32 dst, u32 len);
int ddr3_load_patterns(MV_DRAM_INFO *dram_info, int resume);

int ddr3_read_leveling_hw(u32 freq, MV_DRAM_INFO *dram_info);
int ddr3_read_leveling_sw(u32 freq, int ratio_2to1, MV_DRAM_INFO *dram_info);

int ddr3_write_leveling_hw(u32 freq, MV_DRAM_INFO *dram_info);
int ddr3_write_leveling_sw(u32 freq, int ratio_2to1, MV_DRAM_INFO *dram_info);
int ddr3_write_leveling_hw_reg_dimm(u32 freq, MV_DRAM_INFO *dram_info);
int ddr3_wl_supplement(MV_DRAM_INFO *dram_info);

int ddr3_dfs_high_2_low(u32 freq, MV_DRAM_INFO *dram_info);
int ddr3_dfs_low_2_high(u32 freq, int ratio_2to1, MV_DRAM_INFO *dram_info);

int ddr3_pbs_tx(MV_DRAM_INFO *dram_info);
int ddr3_pbs_rx(MV_DRAM_INFO *dram_info);
int ddr3_load_pbs_patterns(MV_DRAM_INFO *dram_info);

int ddr3_dqs_centralization_rx(MV_DRAM_INFO *dram_info);
int ddr3_dqs_centralization_tx(MV_DRAM_INFO *dram_info);
int ddr3_load_dqs_patterns(MV_DRAM_INFO *dram_info);

void ddr3_static_training_init(void);

u8 ddr3_get_eprom_fabric(void);
void ddr3_set_performance_params(MV_DRAM_INFO *dram_info);
int ddr3_dram_sram_burst(u32 src, u32 dst, u32 len);
void ddr3_save_training(MV_DRAM_INFO *dram_info);
int ddr3_read_training_results(void);
int ddr3_training_suspend_resume(MV_DRAM_INFO *dram_info);
int ddr3_get_min_max_read_sample_delay(u32 cs_enable, u32 reg, u32 *min,
				       u32 *max, u32 *cs_max);
int ddr3_get_min_max_rl_phase(MV_DRAM_INFO *dram_info, u32 *min, u32 *max,
			      u32 cs);
int ddr3_odt_activate(int activate);
int ddr3_odt_read_dynamic_config(MV_DRAM_INFO *dram_info);
void ddr3_print_freq(u32 freq);
void ddr3_reset_phy_read_fifo(void);

#endif /* __DDR3_TRAINING_H */
