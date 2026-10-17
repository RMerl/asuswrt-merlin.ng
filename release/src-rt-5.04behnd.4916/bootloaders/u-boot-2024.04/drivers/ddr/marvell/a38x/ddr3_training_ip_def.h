/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _DDR3_TRAINING_IP_DEF_H
#define _DDR3_TRAINING_IP_DEF_H

#define PATTERN_55			0x55555555
#define PATTERN_AA			0xaaaaaaaa
#define PATTERN_80			0x80808080
#define PATTERN_20			0x20202020
#define PATTERN_01			0x01010101
#define PATTERN_FF			0xffffffff
#define PATTERN_00			0x00000000

/* 16bit bus width patterns */
#define PATTERN_55AA			0x5555aaaa
#define PATTERN_00FF			0x0000ffff
#define PATTERN_0080			0x00008080

#define INVALID_VALUE			0xffffffff
#define MAX_NUM_OF_DUNITS		32
/*
 * length *2 = length in words of pattern, first low address,
 * second high address
 */
#define TEST_PATTERN_LENGTH		4
#define KILLER_PATTERN_DQ_NUMBER	8
#define SSO_DQ_NUMBER			4
#define PATTERN_MAXIMUM_LENGTH		64
#define ADLL_TX_LENGTH			64
#define ADLL_RX_LENGTH			32

#define PARAM_NOT_CARE			0
#define PARAM_UNDEFINED			0xffffffff

#define READ_LEVELING_PHY_OFFSET	2
#define WRITE_LEVELING_PHY_OFFSET	0

#define MASK_ALL_BITS			0xffffffff

#define CS_BIT_MASK			0xf

/* DFX access */
#define BROADCAST_ID			28
#define MULTICAST_ID			29

#define XSB_BASE_ADDR			0x00004000
#define XSB_CTRL_0_REG			0x00000000
#define XSB_CTRL_1_REG			0x00000004
#define XSB_CMD_REG			0x00000008
#define XSB_ADDRESS_REG			0x0000000c
#define XSB_DATA_REG			0x00000010
#define PIPE_ENABLE_ADDR		0x000f8000
#define ENABLE_DDR_TUNING_ADDR		0x000f829c

#define CLIENT_BASE_ADDR		0x00002000
#define CLIENT_CTRL_REG			0x00000000

#define TARGET_INT			0x1801
#define TARGET_EXT			0x180e
#define BYTE_EN				0
#define CMD_READ			0
#define CMD_WRITE			1

#define INTERNAL_ACCESS_PORT		1
#define EXECUTING			1
#define ACCESS_EXT			1
#define CS2_EXIST_BIT			2
#define TRAINING_ID			0xf
#define EXT_TRAINING_ID			1
#define EXT_MODE			0x4

#define GET_RESULT_STATE(res)		(res)
#define SET_RESULT_STATE(res, state)	(res = state)

#define ADDR_SIZE_512MB			0x04000000
#define ADDR_SIZE_1GB			0x08000000
#define ADDR_SIZE_2GB			0x10000000
#define ADDR_SIZE_4GB			0x20000000
#define ADDR_SIZE_8GB			0x40000000
#define ADDR_SIZE_16GB			0x80000000


enum hws_edge_compare {
	EDGE_PF,
	EDGE_FP,
	EDGE_FPF,
	EDGE_PFP
};

enum hws_control_element {
	HWS_CONTROL_ELEMENT_ADLL,		/* per bit 1 edge */
	HWS_CONTROL_ELEMENT_DQ_SKEW,
	HWS_CONTROL_ELEMENT_DQS_SKEW
};

enum hws_search_dir {
	HWS_LOW2HIGH,
	HWS_HIGH2LOW,
	HWS_SEARCH_DIR_LIMIT
};

enum hws_operation {
	OPERATION_READ = 0,
	OPERATION_WRITE = 1
};

enum hws_training_ip_stat {
	HWS_TRAINING_IP_STATUS_FAIL,
	HWS_TRAINING_IP_STATUS_SUCCESS,
	HWS_TRAINING_IP_STATUS_TIMEOUT
};

enum hws_ddr_cs {
	CS_SINGLE,
	CS_NON_SINGLE
};

enum hws_ddr_phy {
	DDR_PHY_DATA = 0,
	DDR_PHY_CONTROL = 1
};

enum hws_dir {
	OPER_WRITE,
	OPER_READ,
	OPER_WRITE_AND_READ
};

enum hws_wl_supp {
	PHASE_SHIFT,
	CLOCK_SHIFT,
	ALIGN_SHIFT
};

enum  mv_ddr_tip_bit_state {
	BIT_LOW_UI,
	BIT_HIGH_UI,
	BIT_SPLIT_IN,
	BIT_SPLIT_OUT,
	BIT_STATE_LAST
};

enum  mv_ddr_tip_byte_state{
	BYTE_NOT_DEFINED,
	BYTE_HOMOGENEOUS_LOW = 0x1,
	BYTE_HOMOGENEOUS_HIGH = 0x2,
	BYTE_HOMOGENEOUS_SPLIT_IN = 0x4,
	BYTE_HOMOGENEOUS_SPLIT_OUT = 0x8,
	BYTE_SPLIT_OUT_MIX = 0x10,
	BYTE_STATE_LAST
};

struct reg_data {
	unsigned int reg_addr;
	unsigned int reg_data;
	unsigned int reg_mask;
};

enum dm_direction {
	DM_DIR_INVERSE,
	DM_DIR_DIRECT
};

#endif /* _DDR3_TRAINING_IP_DEF_H */
