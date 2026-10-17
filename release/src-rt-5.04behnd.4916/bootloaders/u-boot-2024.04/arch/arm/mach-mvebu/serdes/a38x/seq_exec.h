/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _SEQ_EXEC_H
#define _SEQ_EXEC_H

#define NA			0xff
#define DEFAULT_PARAM		0
#define MV_BOARD_TCLK_ERROR	0xffffffff

#define NO_DATA			0xffffffff
#define MAX_DATA_ARRAY		5
#define FIRST_CELL		0

/* Operation types */
enum mv_op {
	WRITE_OP,
	DELAY_OP,
	POLL_OP,
};

/* Operation parameters */
struct op_params {
	u32 unit_base_reg;
	u32 unit_offset;
	u32 mask;
	u32 data[MAX_DATA_ARRAY];	/* data array */
	u8 wait_time;			/* msec */
	u16 num_of_loops;		/* for polling only */
};

/*
 * Sequence parameters. Each sequence contains:
 * 1. Sequence id.
 * 2. Sequence size (total amount of operations during the sequence)
 * 3. a series of operations. operations can be write, poll or delay
 * 4. index in the data array (the entry where the relevant data sits)
 */
struct cfg_seq {
	struct op_params *op_params_ptr;
	u8 cfg_seq_size;
	u8 data_arr_idx;
};

extern struct cfg_seq serdes_seq_db[];

/*
 * A generic function type for executing an operation (write, poll or delay)
 */
typedef int (*op_execute_func_ptr)(u32 serdes_num, struct op_params *params,
				   u32 data_arr_idx);

/* Specific functions for executing each operation */
int write_op_execute(u32 serdes_num, struct op_params *params,
		     u32 data_arr_idx);
int delay_op_execute(u32 serdes_num, struct op_params *params,
		     u32 data_arr_idx);
int poll_op_execute(u32 serdes_num, struct op_params *params, u32 data_arr_idx);
enum mv_op get_cfg_seq_op(struct op_params *params);
int mv_seq_exec(u32 serdes_num, u32 seq_id);

#endif /*_SEQ_EXEC_H*/
