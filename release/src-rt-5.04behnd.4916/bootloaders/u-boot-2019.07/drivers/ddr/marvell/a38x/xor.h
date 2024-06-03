/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _XOR_H
#define _XOR_H

#define SRAM_BASE		0x40000000

#define MV_XOR_MAX_UNIT		2	/* XOR unit == XOR engine */
#define MV_XOR_MAX_CHAN		4	/* total channels for all units */
#define MV_XOR_MAX_CHAN_PER_UNIT 2	/* channels for units */

#define MV_IS_POWER_OF_2(num)	(((num) != 0) && (((num) & ((num) - 1)) == 0))

/*
 * This structure describes address space window. Window base can be
 * 64 bit, window size up to 4GB
 */
struct addr_win {
	u32 base_low;		/* 32bit base low       */
	u32 base_high;		/* 32bit base high      */
	u32 size;		/* 32bit size           */
};

/* This structure describes SoC units address decode window	*/
struct unit_win_info {
	struct addr_win addr_win;	/* An address window */
	int enable;		/* Address decode window is enabled/disabled  */
	u8 attrib;		/* chip select attributes */
	u8 target_id;		/* Target Id of this MV_TARGET */
};

/*
 * This enumerator describes the type of functionality the XOR channel
 * can have while using the same data structures.
 */
enum xor_type {
	MV_XOR,			/* XOR channel functions as XOR accelerator   */
	MV_DMA,			/* XOR channel functions as IDMA channel      */
	MV_CRC32		/* XOR channel functions as CRC 32 calculator */
};

enum mv_state {
	MV_IDLE,
	MV_ACTIVE,
	MV_PAUSED,
	MV_UNDEFINED_STATE
};

/*
 * This enumerator describes the set of commands that can be applied on
 * an engine (e.g. IDMA, XOR). Appling a comman depends on the current
 * status (see MV_STATE enumerator)
 *
 * Start can be applied only when status is IDLE
 * Stop can be applied only when status is IDLE, ACTIVE or PAUSED
 * Pause can be applied only when status is ACTIVE
 * Restart can be applied only when status is PAUSED
 */
enum mv_command {
	MV_START,		/* Start     */
	MV_STOP,		/* Stop     */
	MV_PAUSE,		/* Pause    */
	MV_RESTART		/* Restart  */
};

enum xor_override_target {
	SRC_ADDR0,		/* Source Address #0 Control */
	SRC_ADDR1,		/* Source Address #1 Control */
	SRC_ADDR2,		/* Source Address #2 Control */
	SRC_ADDR3,		/* Source Address #3 Control */
	SRC_ADDR4,		/* Source Address #4 Control */
	SRC_ADDR5,		/* Source Address #5 Control */
	SRC_ADDR6,		/* Source Address #6 Control */
	SRC_ADDR7,		/* Source Address #7 Control */
	XOR_DST_ADDR,		/* Destination Address Control */
	XOR_NEXT_DESC		/* Next Descriptor Address Control */
};

enum mv_state mv_xor_state_get(u32 chan);
void mv_xor_hal_init(u32 xor_chan_num);
int mv_xor_ctrl_set(u32 chan, u32 xor_ctrl);
int mv_xor_command_set(u32 chan, enum mv_command command);
int mv_xor_override_set(u32 chan, enum xor_override_target target, u32 win_num,
			int enable);
int mv_xor_transfer(u32 chan, enum xor_type type, u32 xor_chain_ptr);

#endif
