// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <i2c.h>
#include <log.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "xor.h"
#include "xor_regs.h"

static u32 xor_regs_ctrl_backup;
static u32 xor_regs_base_backup[MAX_CS];
static u32 xor_regs_mask_backup[MAX_CS];

static int mv_xor_cmd_set(u32 chan, int command);
static int mv_xor_ctrl_set(u32 chan, u32 xor_ctrl);

void mv_sys_xor_init(MV_DRAM_INFO *dram_info)
{
	u32 reg, ui, base, cs_count;

	xor_regs_ctrl_backup = reg_read(XOR_WINDOW_CTRL_REG(0, 0));
	for (ui = 0; ui < MAX_CS; ui++)
		xor_regs_base_backup[ui] = reg_read(XOR_BASE_ADDR_REG(0, ui));
	for (ui = 0; ui < MAX_CS; ui++)
		xor_regs_mask_backup[ui] = reg_read(XOR_SIZE_MASK_REG(0, ui));

	reg = 0;
	for (ui = 0; ui < (dram_info->num_cs + 1); ui++) {
		/* Enable Window x for each CS */
		reg |= (0x1 << (ui));
		/* Enable Window x for each CS */
		reg |= (0x3 << ((ui * 2) + 16));
	}

	reg_write(XOR_WINDOW_CTRL_REG(0, 0), reg);

	/* Last window - Base - 0x40000000, Attribute 0x1E - SRAM */
	base = (SRAM_BASE & 0xFFFF0000) | 0x1E00;
	reg_write(XOR_BASE_ADDR_REG(0, dram_info->num_cs), base);
	/* Last window - Size - 64 MB */
	reg_write(XOR_SIZE_MASK_REG(0, dram_info->num_cs), 0x03FF0000);

	cs_count = 0;
	for (ui = 0; ui < MAX_CS; ui++) {
		if (dram_info->cs_ena & (1 << ui)) {
			/*
			 * Window x - Base - 0x00000000, Attribute 0x0E - DRAM
			 */
			base = 0;
			switch (ui) {
			case 0:
				base |= 0xE00;
				break;
			case 1:
				base |= 0xD00;
				break;
			case 2:
				base |= 0xB00;
				break;
			case 3:
				base |= 0x700;
				break;
			}

			reg_write(XOR_BASE_ADDR_REG(0, cs_count), base);

			/* Window x - Size - 256 MB */
			reg_write(XOR_SIZE_MASK_REG(0, cs_count), 0x0FFF0000);
			cs_count++;
		}
	}

	mv_xor_hal_init(1);

	return;
}

void mv_sys_xor_finish(void)
{
	u32 ui;

	reg_write(XOR_WINDOW_CTRL_REG(0, 0), xor_regs_ctrl_backup);
	for (ui = 0; ui < MAX_CS; ui++)
		reg_write(XOR_BASE_ADDR_REG(0, ui), xor_regs_base_backup[ui]);
	for (ui = 0; ui < MAX_CS; ui++)
		reg_write(XOR_SIZE_MASK_REG(0, ui), xor_regs_mask_backup[ui]);

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
void mv_xor_hal_init(u32 chan_num)
{
	u32 i;

	/* Abort any XOR activity & set default configuration */
	for (i = 0; i < chan_num; i++) {
		mv_xor_cmd_set(i, MV_STOP);
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
 *    This function does not modify the OperationMode field of control register.
 *
 */
static int mv_xor_ctrl_set(u32 chan, u32 xor_ctrl)
{
	u32 val;

	/* Update the XOR Engine [0..1] Configuration Registers (XExCR) */
	val = reg_read(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)))
	    & XEXCR_OPERATION_MODE_MASK;
	xor_ctrl &= ~XEXCR_OPERATION_MODE_MASK;
	xor_ctrl |= val;
	reg_write(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)), xor_ctrl);

	return MV_OK;
}

int mv_xor_mem_init(u32 chan, u32 start_ptr, unsigned long long block_size,
		    u32 init_val_high, u32 init_val_low)
{
	u32 tmp;

	/* Parameter checking */
	if (chan >= MV_XOR_MAX_CHAN)
		return MV_BAD_PARAM;

	if (MV_ACTIVE == mv_xor_state_get(chan))
		return MV_BUSY;

	if ((block_size < XEXBSR_BLOCK_SIZE_MIN_VALUE) ||
	    (block_size > XEXBSR_BLOCK_SIZE_MAX_VALUE))
		return MV_BAD_PARAM;

	/* Set the operation mode to Memory Init */
	tmp = reg_read(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)));
	tmp &= ~XEXCR_OPERATION_MODE_MASK;
	tmp |= XEXCR_OPERATION_MODE_MEM_INIT;
	reg_write(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)), tmp);

	/*
	 * Update the start_ptr field in XOR Engine [0..1] Destination Pointer
	 * Register (XExDPR0)
	 */
	reg_write(XOR_DST_PTR_REG(XOR_UNIT(chan), XOR_CHAN(chan)), start_ptr);

	/*
	 * Update the BlockSize field in the XOR Engine[0..1] Block Size
	 * Registers (XExBSR)
	 */
	reg_write(XOR_BLOCK_SIZE_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
		  block_size);

	/*
	 * Update the field InitValL in the XOR Engine Initial Value Register
	 * Low (XEIVRL)
	 */
	reg_write(XOR_INIT_VAL_LOW_REG(XOR_UNIT(chan)), init_val_low);

	/*
	 * Update the field InitValH in the XOR Engine Initial Value Register
	 * High (XEIVRH)
	 */
	reg_write(XOR_INIT_VAL_HIGH_REG(XOR_UNIT(chan)), init_val_high);

	/* Start transfer */
	reg_bit_set(XOR_ACTIVATION_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
		    XEXACTR_XESTART_MASK);

	return MV_OK;
}

/*
 * mv_xor_transfer - Transfer data from source to destination on one of
 *                 three modes (XOR,CRC32,DMA)
 *
 * DESCRIPTION:
 *       This function initiates XOR channel, according to function parameters,
 *       in order to perform XOR or CRC32 or DMA transaction.
 *       To gain maximum performance the user is asked to keep the following
 *       restrictions:
 *       1) Selected engine is available (not busy).
 *       1) This module does not take into consideration CPU MMU issues.
 *          In order for the XOR engine to access the appropreate source
 *          and destination, address parameters must be given in system
 *          physical mode.
 *       2) This API does not take care of cache coherency issues. The source,
 *          destination and in case of chain the descriptor list are assumed
 *          to be cache coherent.
 *       4) Parameters validity. For example, does size parameter exceeds
 *          maximum byte count of descriptor mode (16M or 64K).
 *
 * INPUT:
 *       chan          - XOR channel number. See MV_XOR_CHANNEL enumerator.
 *       xor_type      - One of three: XOR, CRC32 and DMA operations.
 *       xor_chain_ptr - address of chain pointer
 *
 * OUTPUT:
 *       None.
 *
 * RETURS:
 *       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
 *
 */
int mv_xor_transfer(u32 chan, int xor_type, u32 xor_chain_ptr)
{
	u32 tmp;

	/* Parameter checking */
	if (chan >= MV_XOR_MAX_CHAN) {
		debug("%s: ERR. Invalid chan num %d\n", __func__, chan);
		return MV_BAD_PARAM;
	}

	if (MV_ACTIVE == mv_xor_state_get(chan)) {
		debug("%s: ERR. Channel is already active\n", __func__);
		return MV_BUSY;
	}

	if (0x0 == xor_chain_ptr) {
		debug("%s: ERR. xor_chain_ptr is NULL pointer\n", __func__);
		return MV_BAD_PARAM;
	}

	/* Read configuration register and mask the operation mode field */
	tmp = reg_read(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)));
	tmp &= ~XEXCR_OPERATION_MODE_MASK;

	switch (xor_type) {
	case MV_XOR:
		if (0 != (xor_chain_ptr & XEXDPR_DST_PTR_XOR_MASK)) {
			debug("%s: ERR. Invalid chain pointer (bits [5:0] must be cleared)\n",
			      __func__);
			return MV_BAD_PARAM;
		}

		/* Set the operation mode to XOR */
		tmp |= XEXCR_OPERATION_MODE_XOR;
		break;

	case MV_DMA:
		if (0 != (xor_chain_ptr & XEXDPR_DST_PTR_DMA_MASK)) {
			debug("%s: ERR. Invalid chain pointer (bits [4:0] must be cleared)\n",
			      __func__);
			return MV_BAD_PARAM;
		}

		/* Set the operation mode to DMA */
		tmp |= XEXCR_OPERATION_MODE_DMA;
		break;

	case MV_CRC32:
		if (0 != (xor_chain_ptr & XEXDPR_DST_PTR_CRC_MASK)) {
			debug("%s: ERR. Invalid chain pointer (bits [4:0] must be cleared)\n",
			      __func__);
			return MV_BAD_PARAM;
		}

		/* Set the operation mode to CRC32 */
		tmp |= XEXCR_OPERATION_MODE_CRC;
		break;

	default:
		return MV_BAD_PARAM;
	}

	/* Write the operation mode to the register */
	reg_write(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)), tmp);

	/*
	 * Update the NextDescPtr field in the XOR Engine [0..1] Next Descriptor
	 * Pointer Register (XExNDPR)
	 */
	reg_write(XOR_NEXT_DESC_PTR_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
		  xor_chain_ptr);

	/* Start transfer */
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
 *
 */
int mv_xor_state_get(u32 chan)
{
	u32 state;

	/* Parameter checking */
	if (chan >= MV_XOR_MAX_CHAN) {
		debug("%s: ERR. Invalid chan num %d\n", __func__, chan);
		return MV_UNDEFINED_STATE;
	}

	/* Read the current state */
	state = reg_read(XOR_ACTIVATION_REG(XOR_UNIT(chan), XOR_CHAN(chan)));
	state &= XEXACTR_XESTATUS_MASK;

	/* Return the state */
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
 * mv_xor_cmd_set - Set command of XOR channel
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
 *
 */
static int mv_xor_cmd_set(u32 chan, int command)
{
	int state;

	/* Parameter checking */
	if (chan >= MV_XOR_MAX_CHAN) {
		debug("%s: ERR. Invalid chan num %d\n", __func__, chan);
		return MV_BAD_PARAM;
	}

	/* Get the current state */
	state = mv_xor_state_get(chan);

	/* Command is start and current state is idle */
	if ((command == MV_START) && (state == MV_IDLE)) {
		reg_bit_set(XOR_ACTIVATION_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
			    XEXACTR_XESTART_MASK);
		return MV_OK;
	}
	/* Command is stop and current state is active */
	else if ((command == MV_STOP) && (state == MV_ACTIVE)) {
		reg_bit_set(XOR_ACTIVATION_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
			    XEXACTR_XESTOP_MASK);
		return MV_OK;
	}
	/* Command is paused and current state is active */
	else if ((command == MV_PAUSED) && (state == MV_ACTIVE)) {
		reg_bit_set(XOR_ACTIVATION_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
			    XEXACTR_XEPAUSE_MASK);
		return MV_OK;
	}
	/* Command is restart and current state is paused */
	else if ((command == MV_RESTART) && (state == MV_PAUSED)) {
		reg_bit_set(XOR_ACTIVATION_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
			    XEXACTR_XERESTART_MASK);
		return MV_OK;
	}
	/* Command is stop and current state is active */
	else if ((command == MV_STOP) && (state == MV_IDLE))
		return MV_OK;

	/* Illegal command */
	debug("%s: ERR. Illegal command\n", __func__);

	return MV_BAD_PARAM;
}
