// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "seq_exec.h"
#include "high_speed_env_spec.h"

#include "../../../drivers/ddr/marvell/a38x/ddr3_init.h"

#if defined(MV_DEBUG_INIT_FULL) || defined(MV_DEBUG)
#define DB(x)	x
#else
#define DB(x)
#endif

/* Array for mapping the operation (write, poll or delay) functions */
op_execute_func_ptr op_execute_func_arr[] = {
	write_op_execute,
	delay_op_execute,
	poll_op_execute
};

int write_op_execute(u32 serdes_num, struct op_params *params, u32 data_arr_idx)
{
	u32 unit_base_reg, unit_offset, data, mask, reg_data, reg_addr;

	/* Getting write op params from the input parameter */
	data = params->data[data_arr_idx];
	mask = params->mask;

	/* an empty operation */
	if (data == NO_DATA)
		return MV_OK;

	/* get updated base address since it can be different between Serdes */
	CHECK_STATUS(hws_get_ext_base_addr(serdes_num, params->unit_base_reg,
					   params->unit_offset,
					   &unit_base_reg, &unit_offset));

	/* Address calculation */
	reg_addr = unit_base_reg + unit_offset * serdes_num;

#ifdef SEQ_DEBUG
	printf("Write: 0x%x: 0x%x (mask 0x%x) - ", reg_addr, data, mask);
#endif
	/* Reading old value */
	reg_data = reg_read(reg_addr);
	reg_data &= (~mask);

	/* Writing new data */
	data &= mask;
	reg_data |= data;
	reg_write(reg_addr, reg_data);

#ifdef SEQ_DEBUG
	printf(" - 0x%x\n", reg_data);
#endif

	return MV_OK;
}

int delay_op_execute(u32 serdes_num, struct op_params *params, u32 data_arr_idx)
{
	u32 delay;

	/* Getting delay op params from the input parameter */
	delay = params->wait_time;
#ifdef SEQ_DEBUG
	printf("Delay: %d\n", delay);
#endif
	mdelay(delay);

	return MV_OK;
}

int poll_op_execute(u32 serdes_num, struct op_params *params, u32 data_arr_idx)
{
	u32 unit_base_reg, unit_offset, data, mask, num_of_loops, wait_time;
	u32 poll_counter = 0;
	u32 reg_addr, reg_data;

	/* Getting poll op params from the input parameter */
	data = params->data[data_arr_idx];
	mask = params->mask;
	num_of_loops = params->num_of_loops;
	wait_time = params->wait_time;

	/* an empty operation */
	if (data == NO_DATA)
		return MV_OK;

	/* get updated base address since it can be different between Serdes */
	CHECK_STATUS(hws_get_ext_base_addr(serdes_num, params->unit_base_reg,
					   params->unit_offset,
					   &unit_base_reg, &unit_offset));

	/* Address calculation */
	reg_addr = unit_base_reg + unit_offset * serdes_num;

	/* Polling */
#ifdef SEQ_DEBUG
	printf("Poll:  0x%x: 0x%x (mask 0x%x)\n", reg_addr, data, mask);
#endif

	do {
		reg_data = reg_read(reg_addr) & mask;
		poll_counter++;
		udelay(wait_time);
	} while ((reg_data != data) && (poll_counter < num_of_loops));

	if ((poll_counter >= num_of_loops) && (reg_data != data)) {
		DEBUG_INIT_S("poll_op_execute: TIMEOUT\n");
		return MV_TIMEOUT;
	}

	return MV_OK;
}

enum mv_op get_cfg_seq_op(struct op_params *params)
{
	if (params->wait_time == 0)
		return WRITE_OP;
	else if (params->num_of_loops == 0)
		return DELAY_OP;

	return POLL_OP;
}

int mv_seq_exec(u32 serdes_num, u32 seq_id)
{
	u32 seq_idx;
	struct op_params *seq_arr;
	u32 seq_size;
	u32 data_arr_idx;
	enum mv_op curr_op;

	DB(printf("\n### mv_seq_exec ###\n"));
	DB(printf("seq id: %d\n", seq_id));

	if (hws_is_serdes_active(serdes_num) != 1) {
		printf("mv_seq_exec_ext:Error: SerDes lane %d is not valid\n",
		       serdes_num);
		return MV_BAD_PARAM;
	}

	seq_arr = serdes_seq_db[seq_id].op_params_ptr;
	seq_size = serdes_seq_db[seq_id].cfg_seq_size;
	data_arr_idx = serdes_seq_db[seq_id].data_arr_idx;

	DB(printf("seq_size: %d\n", seq_size));
	DB(printf("data_arr_idx: %d\n", data_arr_idx));

	/* Executing the sequence operations */
	for (seq_idx = 0; seq_idx < seq_size; seq_idx++) {
		curr_op = get_cfg_seq_op(&seq_arr[seq_idx]);
		op_execute_func_arr[curr_op](serdes_num, &seq_arr[seq_idx],
					     data_arr_idx);
	}

	return MV_OK;
}
