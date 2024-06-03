// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "mv_ddr_regs.h"
#include "mv_ddr_sys_env_lib.h"

static u32 mv_ddr_board_id_get(void)
{
#if defined(CONFIG_TARGET_DB_88F6820_GP)
	return DB_GP_68XX_ID;
#else
	/*
	 * Return 0 here for custom board as this should not be used
	 * for custom boards.
	 */
	return 0;
#endif
}

static u32 mv_ddr_board_id_index_get(u32 board_id)
{
	/*
	 * Marvell Boards use 0x10 as base for Board ID:
	 * mask MSB to receive index for board ID
	 */
	return board_id & (MARVELL_BOARD_ID_MASK - 1);
}

/*
 * read gpio input for suspend-wakeup indication
 * return indicating suspend wakeup status:
 * 0 - not supported,
 * 1 - supported: read magic word detect wakeup,
 * 2 - detected wakeup from gpio
 */
enum suspend_wakeup_status mv_ddr_sys_env_suspend_wakeup_check(void)
{
	u32 reg, board_id_index, gpio;
	struct board_wakeup_gpio board_gpio[] = MV_BOARD_WAKEUP_GPIO_INFO;

	board_id_index = mv_ddr_board_id_index_get(mv_ddr_board_id_get());
	if (!(sizeof(board_gpio) / sizeof(struct board_wakeup_gpio) >
	      board_id_index)) {
		printf("\n_failed loading Suspend-Wakeup information (invalid board ID)\n");
		return SUSPEND_WAKEUP_DISABLED;
	}

	/*
	 * - Detect if Suspend-Wakeup is supported on current board
	 * - Fetch the GPIO number for wakeup status input indication
	 */
	if (board_gpio[board_id_index].gpio_num == -1) {
		/* Suspend to RAM is not supported */
		return SUSPEND_WAKEUP_DISABLED;
	} else if (board_gpio[board_id_index].gpio_num == -2) {
		/*
		 * Suspend to RAM is supported but GPIO indication is
		 * not implemented - Skip
		 */
		return SUSPEND_WAKEUP_ENABLED;
	} else {
		gpio = board_gpio[board_id_index].gpio_num;
	}

	/* Initialize MPP for GPIO (set MPP = 0x0) */
	reg = reg_read(MPP_CONTROL_REG(MPP_REG_NUM(gpio)));
	/* reset MPP21 to 0x0, keep rest of MPP settings*/
	reg &= ~MPP_MASK(gpio);
	reg_write(MPP_CONTROL_REG(MPP_REG_NUM(gpio)), reg);

	/* Initialize GPIO as input */
	reg = reg_read(GPP_DATA_OUT_EN_REG(GPP_REG_NUM(gpio)));
	reg |= GPP_MASK(gpio);
	reg_write(GPP_DATA_OUT_EN_REG(GPP_REG_NUM(gpio)), reg);

	/*
	 * Check GPP for input status from PIC: 0 - regular init,
	 * 1 - suspend wakeup
	 */
	reg = reg_read(GPP_DATA_IN_REG(GPP_REG_NUM(gpio)));

	/* if GPIO is ON: wakeup from S2RAM indication detected */
	return (reg & GPP_MASK(gpio)) ? SUSPEND_WAKEUP_ENABLED_GPIO_DETECTED :
		SUSPEND_WAKEUP_DISABLED;
}

/*
 * get bit mask of enabled cs
 * return bit mask of enabled cs:
 * 1 - only cs0 enabled,
 * 3 - both cs0 and cs1 enabled
 */
u32 mv_ddr_sys_env_get_cs_ena_from_reg(void)
{
	return reg_read(DDR3_RANK_CTRL_REG) &
		((CS_EXIST_MASK << CS_EXIST_OFFS(0)) |
		 (CS_EXIST_MASK << CS_EXIST_OFFS(1)) |
		 (CS_EXIST_MASK << CS_EXIST_OFFS(2)) |
		 (CS_EXIST_MASK << CS_EXIST_OFFS(3)));
}
