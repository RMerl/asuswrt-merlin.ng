// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "ddr3_init.h"
#include "mv_ddr_common.h"
#include "xor_regs.h"

/* defines  */
#ifdef MV_DEBUG
#define DB(x)	x
#else
#define DB(x)
#endif

static u32 ui_xor_regs_ctrl_backup;
static u32 ui_xor_regs_base_backup[MAX_CS_NUM + 1];
static u32 ui_xor_regs_mask_backup[MAX_CS_NUM + 1];

void mv_sys_xor_init(u32 num_of_cs, u32 cs_ena, uint64_t cs_size, u32 base_delta)
{
	u32 reg, ui, cs_count;
	uint64_t base, size_mask;

	ui_xor_regs_ctrl_backup = reg_read(XOR_WINDOW_CTRL_REG(0, 0));
	for (ui = 0; ui < MAX_CS_NUM + 1; ui++)
		ui_xor_regs_base_backup[ui] =
			reg_read(XOR_BASE_ADDR_REG(0, ui));
	for (ui = 0; ui < MAX_CS_NUM + 1; ui++)
		ui_xor_regs_mask_backup[ui] =
			reg_read(XOR_SIZE_MASK_REG(0, ui));

	reg = 0;
	for (ui = 0, cs_count = 0;
	     (cs_count < num_of_cs) && (ui < 8);
	     ui++, cs_count++) {
		if (cs_ena & (1 << ui)) {
			/* Enable Window x for each CS */
			reg |= (0x1 << (ui));
			/* Enable Window x for each CS */
			reg |= (0x3 << ((ui * 2) + 16));
		}
	}

	reg_write(XOR_WINDOW_CTRL_REG(0, 0), reg);

	cs_count = 0;
	for (ui = 0, cs_count = 0;
	     (cs_count < num_of_cs) && (ui < 8);
	     ui++, cs_count++) {
		if (cs_ena & (1 << ui)) {
			/*
			 * window x - Base - 0x00000000,
			 * Attribute 0x0e - DRAM
			 */
			base = cs_size * ui + base_delta;
			/* fixed size 2GB for each CS */
			size_mask = 0x7FFF0000;
			switch (ui) {
			case 0:
				base |= 0xe00;
				break;
			case 1:
				base |= 0xd00;
				break;
			case 2:
				base |= 0xb00;
				break;
			case 3:
				base |= 0x700;
				break;
			case 4: /* SRAM */
				base = 0x40000000;
				/* configure as shared transaction */
				base |= 0x1F00;
				size_mask = 0xF0000;
				break;
			}

			reg_write(XOR_BASE_ADDR_REG(0, ui), (u32)base);
			size_mask = (cs_size / _64K) - 1;
			size_mask = (size_mask << XESMRX_SIZE_MASK_OFFS) & XESMRX_SIZE_MASK_MASK;
			/* window x - Size */
			reg_write(XOR_SIZE_MASK_REG(0, ui), (u32)size_mask);
		}
	}

	mv_xor_hal_init(1);

	return;
}

void mv_sys_xor_finish(void)
{
	u32 ui;

	reg_write(XOR_WINDOW_CTRL_REG(0, 0), ui_xor_regs_ctrl_backup);
	for (ui = 0; ui < MAX_CS_NUM + 1; ui++)
		reg_write(XOR_BASE_ADDR_REG(0, ui),
			  ui_xor_regs_base_backup[ui]);
	for (ui = 0; ui < MAX_CS_NUM + 1; ui++)
		reg_write(XOR_SIZE_MASK_REG(0, ui),
			  ui_xor_regs_mask_backup[ui]);

	reg_write(XOR_ADDR_OVRD_REG(0, 0), 0);
}

/*
 * mv_xor_hal_init - Initialize XOR engine
 *
 * DESCRIPTION:
 *               This function initialize XOR unit.
 * INPUT:
 *       None.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
 */
void mv_xor_hal_init(u32 xor_chan_num)
{
	u32 i;

	/* Abort any XOR activity & set default configuration */
	for (i = 0; i < xor_chan_num; i++) {
		mv_xor_command_set(i, MV_STOP);
		mv_xor_ctrl_set(i, (1 << XEXCR_REG_ACC_PROTECT_OFFS) |
				(4 << XEXCR_DST_BURST_LIMIT_OFFS) |
				(4 << XEXCR_SRC_BURST_LIMIT_OFFS));
	}
}

/*
 * mv_xor_ctrl_set - Set XOR channel control registers
 *
 * DESCRIPTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
 * NOTE:
 *  This function does not modify the Operation_mode field of control register.
 */
int mv_xor_ctrl_set(u32 chan, u32 xor_ctrl)
{
	u32 old_value;

	/* update the XOR Engine [0..1] Configuration Registers (XEx_c_r) */
	old_value = reg_read(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan))) &
		XEXCR_OPERATION_MODE_MASK;
	xor_ctrl &= ~XEXCR_OPERATION_MODE_MASK;
	xor_ctrl |= old_value;
	reg_write(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)), xor_ctrl);

	return MV_OK;
}

int mv_xor_mem_init(u32 chan, u32 start_ptr, unsigned long long block_size,
		    u32 init_val_high, u32 init_val_low)
{
	u32 temp;

	if (block_size == _4G)
		block_size -= 1;

	/* Parameter checking */
	if (chan >= MV_XOR_MAX_CHAN)
		return MV_BAD_PARAM;

	if (MV_ACTIVE == mv_xor_state_get(chan))
		return MV_BUSY;

	if ((block_size < XEXBSR_BLOCK_SIZE_MIN_VALUE) ||
	    (block_size > XEXBSR_BLOCK_SIZE_MAX_VALUE))
		return MV_BAD_PARAM;

	/* set the operation mode to Memory Init */
	temp = reg_read(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)));
	temp &= ~XEXCR_OPERATION_MODE_MASK;
	temp |= XEXCR_OPERATION_MODE_MEM_INIT;
	reg_write(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)), temp);

	/*
	 * update the start_ptr field in XOR Engine [0..1] Destination Pointer
	 * Register
	 */
	reg_write(XOR_DST_PTR_REG(XOR_UNIT(chan), XOR_CHAN(chan)), start_ptr);

	/*
	 * update the Block_size field in the XOR Engine[0..1] Block Size
	 * Registers
	 */
	reg_write(XOR_BLOCK_SIZE_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
		  block_size);

	/*
	 * update the field Init_val_l in the XOR Engine Initial Value Register
	 * Low (XEIVRL)
	 */
	reg_write(XOR_INIT_VAL_LOW_REG(XOR_UNIT(chan)), init_val_low);

	/*
	 * update the field Init_val_h in the XOR Engine Initial Value Register
	 * High (XEIVRH)
	 */
	reg_write(XOR_INIT_VAL_HIGH_REG(XOR_UNIT(chan)), init_val_high);

	/* start transfer */
	reg_bit_set(XOR_ACTIVATION_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
		    XEXACTR_XESTART_MASK);

	return MV_OK;
}

/*
 * mv_xor_state_get - Get XOR channel state.
 *
 * DESCRIPTION:
 *       XOR channel activity state can be active, idle, paused.
 *       This function retrunes the channel activity state.
 *
 * INPUT:
 *       chan     - the channel number
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       XOR_CHANNEL_IDLE    - If the engine is idle.
 *       XOR_CHANNEL_ACTIVE  - If the engine is busy.
 *       XOR_CHANNEL_PAUSED  - If the engine is paused.
 *       MV_UNDEFINED_STATE  - If the engine state is undefind or there is no
 *                             such engine
 */
enum mv_state mv_xor_state_get(u32 chan)
{
	u32 state;

	/* Parameter checking   */
	if (chan >= MV_XOR_MAX_CHAN) {
		DB(printf("%s: ERR. Invalid chan num %d\n", __func__, chan));
		return MV_UNDEFINED_STATE;
	}

	/* read the current state */
	state = reg_read(XOR_ACTIVATION_REG(XOR_UNIT(chan), XOR_CHAN(chan)));
	state &= XEXACTR_XESTATUS_MASK;

	/* return the state */
	switch (state) {
	case XEXACTR_XESTATUS_IDLE:
		return MV_IDLE;
	case XEXACTR_XESTATUS_ACTIVE:
		return MV_ACTIVE;
	case XEXACTR_XESTATUS_PAUSED:
		return MV_PAUSED;
	}

	return MV_UNDEFINED_STATE;
}

/*
 * mv_xor_command_set - Set command of XOR channel
 *
 * DESCRIPTION:
 *       XOR channel can be started, idle, paused and restarted.
 *       Paused can be set only if channel is active.
 *       Start can be set only if channel is idle or paused.
 *       Restart can be set only if channel is paused.
 *       Stop can be set only if channel is active.
 *
 * INPUT:
 *       chan     - The channel number
 *       command  - The command type (start, stop, restart, pause)
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       MV_OK on success , MV_BAD_PARAM on erroneous parameter, MV_ERROR on
 *       undefind XOR engine mode
 */
int mv_xor_command_set(u32 chan, enum mv_command command)
{
	enum mv_state state;

	/* Parameter checking */
	if (chan >= MV_XOR_MAX_CHAN) {
		DB(printf("%s: ERR. Invalid chan num %d\n", __func__, chan));
		return MV_BAD_PARAM;
	}

	/* get the current state */
	state = mv_xor_state_get(chan);

	if ((command == MV_START) && (state == MV_IDLE)) {
		/* command is start and current state is idle */
		reg_bit_set(XOR_ACTIVATION_REG
			    (XOR_UNIT(chan), XOR_CHAN(chan)),
			    XEXACTR_XESTART_MASK);
		return MV_OK;
	} else if ((command == MV_STOP) && (state == MV_ACTIVE)) {
		/* command is stop and current state is active */
		reg_bit_set(XOR_ACTIVATION_REG
			    (XOR_UNIT(chan), XOR_CHAN(chan)),
			    XEXACTR_XESTOP_MASK);
		return MV_OK;
	} else if (((enum mv_state)command == MV_PAUSED) &&
		   (state == MV_ACTIVE)) {
		/* command is paused and current state is active */
		reg_bit_set(XOR_ACTIVATION_REG
			    (XOR_UNIT(chan), XOR_CHAN(chan)),
			    XEXACTR_XEPAUSE_MASK);
		return MV_OK;
	} else if ((command == MV_RESTART) && (state == MV_PAUSED)) {
		/* command is restart and current state is paused */
		reg_bit_set(XOR_ACTIVATION_REG
			    (XOR_UNIT(chan), XOR_CHAN(chan)),
			    XEXACTR_XERESTART_MASK);
		return MV_OK;
	} else if ((command == MV_STOP) && (state == MV_IDLE)) {
		/* command is stop and current state is active */
		return MV_OK;
	}

	/* illegal command */
	DB(printf("%s: ERR. Illegal command\n", __func__));

	return MV_BAD_PARAM;
}

void ddr3_new_tip_ecc_scrub(void)
{
	u32 cs_c, max_cs;
	u32 cs_ena = 0;
	uint64_t total_mem_size, cs_mem_size = 0;

	printf("DDR Training Sequence - Start scrubbing\n");
	max_cs = mv_ddr_cs_num_get();
	for (cs_c = 0; cs_c < max_cs; cs_c++)
		cs_ena |= 1 << cs_c;

#if defined(CONFIG_ARMADA_38X) || defined(CONFIG_ARMADA_39X)
	/* all chip-selects are of same size */
	ddr3_calc_mem_cs_size(0, &cs_mem_size);
#endif

	mv_sys_xor_init(max_cs, cs_ena, cs_mem_size, 0);
	total_mem_size = max_cs * cs_mem_size;
	mv_xor_mem_init(0, 0, total_mem_size, 0xdeadbeef, 0xdeadbeef);
	/* wait for previous transfer completion */
	while (mv_xor_state_get(0) != MV_IDLE)
		;
	/* Return XOR State */
	mv_sys_xor_finish();

	printf("DDR3 Training Sequence - End scrubbing\n");
}

/*
* mv_xor_transfer - Transfer data from source to destination in one of
*		    three modes: XOR, CRC32 or DMA
*
* DESCRIPTION:
*	This function initiates XOR channel, according to function parameters,
*	in order to perform XOR, CRC32 or DMA transaction.
*	To gain maximum performance the user is asked to keep the following
*	restrictions:
*	1) Selected engine is available (not busy).
*	2) This module does not take into consideration CPU MMU issues.
*	   In order for the XOR engine to access the appropriate source
*	   and destination, address parameters must be given in system
*	   physical mode.
*	3) This API does not take care of cache coherency issues. The source,
*	   destination and, in case of chain, the descriptor list are assumed
*	   to be cache coherent.
*	4) Parameters validity.
*
* INPUT:
*	chan		- XOR channel number.
*	type	- One of three: XOR, CRC32 and DMA operations.
*	xor_chain_ptr	- address of chain pointer
*
* OUTPUT:
*	None.
*
* RETURN:
*       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
*
*******************************************************************************/
int mv_xor_transfer(u32 chan, enum xor_type type, u32 xor_chain_ptr)
{
	u32 temp;

	/* Parameter checking */
	if (chan >= MV_XOR_MAX_CHAN) {
		DB(printf("%s: ERR. Invalid chan num %d\n", __func__, chan));
		return MV_BAD_PARAM;
	}
	if (mv_xor_state_get(chan) == MV_ACTIVE) {
		DB(printf("%s: ERR. Channel is already active\n", __func__));
		return MV_BUSY;
	}
	if (xor_chain_ptr == 0x0) {
		DB(printf("%s: ERR. xor_chain_ptr is NULL pointer\n", __func__));
		return MV_BAD_PARAM;
	}

	/* read configuration register and mask the operation mode field */
	temp = reg_read(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)));
	temp &= ~XEXCR_OPERATION_MODE_MASK;

	switch (type) {
	case MV_XOR:
		if ((xor_chain_ptr & XEXDPR_DST_PTR_XOR_MASK) != 0) {
			DB(printf("%s: ERR. Invalid chain pointer (bits [5:0] must be cleared)\n",
				  __func__));
			return MV_BAD_PARAM;
		}
		/* set the operation mode to XOR */
		temp |= XEXCR_OPERATION_MODE_XOR;
		break;
	case MV_DMA:
		if ((xor_chain_ptr & XEXDPR_DST_PTR_DMA_MASK) != 0) {
			DB(printf("%s: ERR. Invalid chain pointer (bits [4:0] must be cleared)\n",
				  __func__));
			return MV_BAD_PARAM;
		}
		/* set the operation mode to DMA */
		temp |= XEXCR_OPERATION_MODE_DMA;
		break;
	case MV_CRC32:
		if ((xor_chain_ptr & XEXDPR_DST_PTR_CRC_MASK) != 0) {
			DB(printf("%s: ERR. Invalid chain pointer (bits [4:0] must be cleared)\n",
				  __func__));
			return MV_BAD_PARAM;
		}
		/* set the operation mode to CRC32 */
		temp |= XEXCR_OPERATION_MODE_CRC;
		break;
	default:
		return MV_BAD_PARAM;
	}

	/* write the operation mode to the register */
	reg_write(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)), temp);
	/*
	 * update the NextDescPtr field in the XOR Engine [0..1] Next Descriptor
	 * Pointer Register (XExNDPR)
	 */
	reg_write(XOR_NEXT_DESC_PTR_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
		  xor_chain_ptr);

	/* start transfer */
	reg_bit_set(XOR_ACTIVATION_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
		    XEXACTR_XESTART_MASK);

	return MV_OK;
}
