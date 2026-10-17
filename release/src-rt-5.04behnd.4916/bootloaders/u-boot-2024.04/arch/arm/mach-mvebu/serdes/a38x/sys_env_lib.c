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
#include "sys_env_lib.h"

enum unit_id sys_env_soc_unit_nums[MAX_UNITS_ID][MAX_DEV_ID_NUM] = {
/*                     6820    6810     6811     6828     */
/* PEX_UNIT_ID      */ { 4,     3,       3,       4},
/* ETH_GIG_UNIT_ID  */ { 3,	2,       3,       3},
/* USB3H_UNIT_ID    */ { 2,     2,       2,       2},
/* USB3D_UNIT_ID    */ { 1,     1,       1,       1},
/* SATA_UNIT_ID     */ { 2,     2,       2,       4},
/* QSGMII_UNIT_ID   */ { 1,     0,       0,       1},
/* XAUI_UNIT_ID     */ { 0,     0,       0,       0},
/* RXAUI_UNIT_ID    */ { 0,     0,       0,       0}
};

u32 g_dev_id = -1;

u32 mv_board_id_get(void)
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

u32 mv_board_tclk_get(void)
{
	u32 value;

	value = (reg_read(DEVICE_SAMPLE_AT_RESET1_REG) >> 15) & 0x1;

	switch (value) {
	case (0x0):
		return 250000000;
	case (0x1):
		return 200000000;
	default:
		return 0xffffffff;
	}
}

u32 mv_board_id_index_get(u32 board_id)
{
	/*
	 * Marvell Boards use 0x10 as base for Board ID:
	 * mask MSB to receive index for board ID
	 */
	return board_id & (MARVELL_BOARD_ID_MASK - 1);
}

/*
 * sys_env_suspend_wakeup_check
 * DESCRIPTION:		Reads GPIO input for suspend-wakeup indication.
 * INPUT:		None.
 * OUTPUT:
 * RETURNS:		u32 indicating suspend wakeup status:
 * 0 - Not supported,
 * 1 - supported: read magic word detect wakeup,
 * 2 - detected wakeup from GPIO.
 */
enum suspend_wakeup_status sys_env_suspend_wakeup_check(void)
{
	u32 reg, board_id_index, gpio;
	struct board_wakeup_gpio board_gpio[] = MV_BOARD_WAKEUP_GPIO_INFO;

	board_id_index = mv_board_id_index_get(mv_board_id_get());
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
 * mv_ctrl_dev_id_index_get
 *
 * DESCRIPTION: return SOC device index
 * INPUT: None
 * OUTPUT: None
 * RETURN:
 *        return SOC device index
 */
u32 sys_env_id_index_get(u32 ctrl_model)
{
	switch (ctrl_model) {
	case MV_6820_DEV_ID:
		return MV_6820_INDEX;
	case MV_6810_DEV_ID:
		return MV_6810_INDEX;
	case MV_6811_DEV_ID:
		return MV_6811_INDEX;
	case MV_6828_DEV_ID:
		return MV_6828_INDEX;
	default:
		return MV_6820_INDEX;
	}
}

u32 sys_env_unit_max_num_get(enum unit_id unit)
{
	u32 dev_id_index;

	if (unit >= MAX_UNITS_ID) {
		printf("%s: Error: Wrong unit type (%u)\n", __func__, unit);
		return 0;
	}

	dev_id_index = sys_env_id_index_get(sys_env_model_get());
	return sys_env_soc_unit_nums[unit][dev_id_index];
}

/*
 * sys_env_model_get
 * DESCRIPTION:	Returns 16bit describing the device model (ID) as defined
 *		in Vendor ID configuration register
 */
u16 sys_env_model_get(void)
{
	u32 default_ctrl_id, ctrl_id = reg_read(DEV_ID_REG);
	ctrl_id = (ctrl_id & (DEV_ID_REG_DEVICE_ID_MASK)) >>
		DEV_ID_REG_DEVICE_ID_OFFS;

	switch (ctrl_id) {
	case MV_6820_DEV_ID:
	case MV_6810_DEV_ID:
	case MV_6811_DEV_ID:
	case MV_6828_DEV_ID:
		return ctrl_id;
	default:
		/* Device ID Default for A38x: 6820 */
		default_ctrl_id =  MV_6820_DEV_ID;
		printf("%s: Error retrieving device ID (%x), using default ID = %x\n",
		       __func__, ctrl_id, default_ctrl_id);
		return default_ctrl_id;
	}
}

/*
 * sys_env_device_id_get
 * DESCRIPTION:	Returns enum (0..7) index of the device model (ID)
 */
u32 sys_env_device_id_get(void)
{
	char *device_id_str[4] = {
		"6810", "6820", "6811", "6828",
	};

	if (g_dev_id != -1)
		return g_dev_id;

	g_dev_id = reg_read(DEVICE_SAMPLE_AT_RESET1_REG);
	g_dev_id = g_dev_id >> SAR_DEV_ID_OFFS & SAR_DEV_ID_MASK;
	printf("Detected Device ID %s\n", g_dev_id < 4 ? device_id_str[g_dev_id] : "NONE");

	return g_dev_id;
}

/*
 * sys_env_device_rev_get - Get Marvell controller device revision number
 *
 * DESCRIPTION:
 *       This function returns 8bit describing the device revision as defined
 *       Revision ID Register.
 *
 * INPUT:
 *       None.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       8bit desscribing Marvell controller revision number
 */
u8 sys_env_device_rev_get(void)
{
	u32 value;

	value = reg_read(DEV_VERSION_ID_REG);
	return (value & (REVISON_ID_MASK)) >> REVISON_ID_OFFS;
}

void mv_rtc_config(void)
{
	u32 i, val;

	/* Activate pipe0 for read/write transaction, and set XBAR client number #1 */
	val = 0x1 << DFX_PIPE_SELECT_PIPE0_ACTIVE_OFFS |
	      0x1 << DFX_PIPE_SELECT_XBAR_CLIENT_SEL_OFFS;
	writel(val, MVEBU_DFX_BASE);

	/* Set new RTC value for all memory wrappers */
	for (i = 0; i < RTC_MEMORY_WRAPPER_COUNT; i++)
		reg_write(RTC_MEMORY_WRAPPER_REG(i), RTC_MEMORY_WRAPPER_CTRL_VAL);
}

void mv_avs_init(void)
{
	u32 sar_freq;

	reg_write(AVS_DEBUG_CNTR_REG, AVS_DEBUG_CNTR_DEFAULT_VALUE);
	reg_write(AVS_DEBUG_CNTR_REG, AVS_DEBUG_CNTR_DEFAULT_VALUE);

	sar_freq = reg_read(DEVICE_SAMPLE_AT_RESET1_REG);
	sar_freq = sar_freq >> SAR_FREQ_OFFSET & SAR_FREQ_MASK;

	/* Set AVS value only for core frequency of 1600MHz or less.
	 * For higher frequency leave the default value.
	 */
	if (sar_freq <= 0xd) {
		u32 avs_reg_data = reg_read(AVS_ENABLED_CONTROL);

		avs_reg_data &= ~(AVS_LOW_VDD_LIMIT_MASK
				| AVS_HIGH_VDD_LIMIT_MASK);
		avs_reg_data |= AVS_LOW_VDD_SLOW_VAL | AVS_HIGH_VDD_SLOW_VAL;
		reg_write(AVS_ENABLED_CONTROL, avs_reg_data);
	}
}
