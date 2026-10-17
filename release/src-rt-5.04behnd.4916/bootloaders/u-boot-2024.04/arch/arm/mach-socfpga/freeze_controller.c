// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 */


#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/freeze_controller.h>
#include <linux/delay.h>
#include <linux/errno.h>

static const struct socfpga_freeze_controller *freeze_controller_base =
		(void *)(SOCFPGA_SYSMGR_ADDRESS + SYSMGR_FRZCTRL_ADDRESS);

/*
 * Default state from cold reset is FREEZE_ALL; the global
 * flag is set to TRUE to indicate the IO banks are frozen
 */
static uint32_t frzctrl_channel_freeze[FREEZE_CHANNEL_NUM]
	= { FREEZE_CTRL_FROZEN, FREEZE_CTRL_FROZEN,
	FREEZE_CTRL_FROZEN, FREEZE_CTRL_FROZEN};

/* Freeze HPS IOs */
void sys_mgr_frzctrl_freeze_req(void)
{
	u32 ioctrl_reg_offset;
	u32 reg_value;
	u32 reg_cfg_mask;
	u32 channel_id;

	/* select software FSM */
	writel(SYSMGR_FRZCTRL_SRC_VIO1_ENUM_SW,	&freeze_controller_base->src);

	/* Freeze channel 0 to 2 */
	for (channel_id = 0; channel_id <= 2; channel_id++) {
		ioctrl_reg_offset = (u32)(
			&freeze_controller_base->vioctrl + channel_id);

		/*
		 * Assert active low enrnsl, plniotri
		 * and niotri signals
		 */
		reg_cfg_mask =
			SYSMGR_FRZCTRL_VIOCTRL_SLEW_MASK
			| SYSMGR_FRZCTRL_VIOCTRL_WKPULLUP_MASK
			| SYSMGR_FRZCTRL_VIOCTRL_TRISTATE_MASK;
		clrbits_le32(ioctrl_reg_offset,	reg_cfg_mask);

		/*
		 * Note: Delay for 20ns at min
		 * Assert active low bhniotri signal and de-assert
		 * active high csrdone
		 */
		reg_cfg_mask
			= SYSMGR_FRZCTRL_VIOCTRL_BUSHOLD_MASK
			| SYSMGR_FRZCTRL_VIOCTRL_CFG_MASK;
		clrbits_le32(ioctrl_reg_offset,	reg_cfg_mask);

		/* Set global flag to indicate channel is frozen */
		frzctrl_channel_freeze[channel_id] = FREEZE_CTRL_FROZEN;
	}

	/* Freeze channel 3 */
	/*
	 * Assert active low enrnsl, plniotri and
	 * niotri signals
	 */
	reg_cfg_mask
		= SYSMGR_FRZCTRL_HIOCTRL_SLEW_MASK
		| SYSMGR_FRZCTRL_HIOCTRL_WKPULLUP_MASK
		| SYSMGR_FRZCTRL_HIOCTRL_TRISTATE_MASK;
	clrbits_le32(&freeze_controller_base->hioctrl, reg_cfg_mask);

	/*
	 * assert active low bhniotri & nfrzdrv signals,
	 * de-assert active high csrdone and assert
	 * active high frzreg and nfrzdrv signals
	 */
	reg_value = readl(&freeze_controller_base->hioctrl);
	reg_cfg_mask
		= SYSMGR_FRZCTRL_HIOCTRL_BUSHOLD_MASK
		| SYSMGR_FRZCTRL_HIOCTRL_CFG_MASK;
	reg_value
		= (reg_value & ~reg_cfg_mask)
		| SYSMGR_FRZCTRL_HIOCTRL_REGRST_MASK
		| SYSMGR_FRZCTRL_HIOCTRL_OCTRST_MASK;
	writel(reg_value, &freeze_controller_base->hioctrl);

	/*
	 * assert active high reinit signal and de-assert
	 * active high pllbiasen signals
	 */
	reg_value = readl(&freeze_controller_base->hioctrl);
	reg_value
		= (reg_value &
		~SYSMGR_FRZCTRL_HIOCTRL_OCT_CFGEN_CALSTART_MASK)
		| SYSMGR_FRZCTRL_HIOCTRL_DLLRST_MASK;
	writel(reg_value, &freeze_controller_base->hioctrl);

	/* Set global flag to indicate channel is frozen */
	frzctrl_channel_freeze[channel_id] = FREEZE_CTRL_FROZEN;
}

/* Unfreeze/Thaw HPS IOs */
void sys_mgr_frzctrl_thaw_req(void)
{
	u32 ioctrl_reg_offset;
	u32 reg_cfg_mask;
	u32 reg_value;
	u32 channel_id;
	unsigned long eosc1_freq;

	/* select software FSM */
	writel(SYSMGR_FRZCTRL_SRC_VIO1_ENUM_SW,	&freeze_controller_base->src);

	/* Thaw channel 0 to 2 */
	for (channel_id = 0; channel_id <= 2; channel_id++) {
		ioctrl_reg_offset
			= (u32)(&freeze_controller_base->vioctrl + channel_id);

		/*
		 * Assert active low bhniotri signal and
		 * de-assert active high csrdone
		 */
		reg_cfg_mask
			= SYSMGR_FRZCTRL_VIOCTRL_BUSHOLD_MASK
			| SYSMGR_FRZCTRL_VIOCTRL_CFG_MASK;
		setbits_le32(ioctrl_reg_offset,	reg_cfg_mask);

		/*
		 * Note: Delay for 20ns at min
		 * de-assert active low plniotri and niotri signals
		 */
		reg_cfg_mask
			= SYSMGR_FRZCTRL_VIOCTRL_WKPULLUP_MASK
			| SYSMGR_FRZCTRL_VIOCTRL_TRISTATE_MASK;
		setbits_le32(ioctrl_reg_offset,	reg_cfg_mask);

		/*
		 * Note: Delay for 20ns at min
		 * de-assert active low enrnsl signal
		 */
		setbits_le32(ioctrl_reg_offset,
			SYSMGR_FRZCTRL_VIOCTRL_SLEW_MASK);

		/* Set global flag to indicate channel is thawed */
		frzctrl_channel_freeze[channel_id] = FREEZE_CTRL_THAWED;
	}

	/* Thaw channel 3 */
	/* de-assert active high reinit signal */
	clrbits_le32(&freeze_controller_base->hioctrl,
		SYSMGR_FRZCTRL_HIOCTRL_DLLRST_MASK);

	/*
	 * Note: Delay for 40ns at min
	 * assert active high pllbiasen signals
	 */
	setbits_le32(&freeze_controller_base->hioctrl,
		SYSMGR_FRZCTRL_HIOCTRL_OCT_CFGEN_CALSTART_MASK);

	/* Delay 1000 intosc cycles. The intosc is based on eosc1. */
	eosc1_freq = cm_get_osc_clk_hz(1) / 1000;	/* kHz */
	udelay(DIV_ROUND_UP(1000000, eosc1_freq));

	/*
	 * de-assert active low bhniotri signals,
	 * assert active high csrdone and nfrzdrv signal
	 */
	reg_value = readl(&freeze_controller_base->hioctrl);
	reg_value = (reg_value
		| SYSMGR_FRZCTRL_HIOCTRL_BUSHOLD_MASK
		| SYSMGR_FRZCTRL_HIOCTRL_CFG_MASK)
		& ~SYSMGR_FRZCTRL_HIOCTRL_OCTRST_MASK;
	writel(reg_value, &freeze_controller_base->hioctrl);

	/*
	 * Delay 33 intosc
	 * Use worst case which is fatest eosc1=50MHz, delay required
	 * is 1/50MHz * 33 = 660ns ~= 1us
	 */
	udelay(1);

	/* de-assert active low plniotri and niotri signals */
	reg_cfg_mask
		= SYSMGR_FRZCTRL_HIOCTRL_WKPULLUP_MASK
		| SYSMGR_FRZCTRL_HIOCTRL_TRISTATE_MASK;

	setbits_le32(&freeze_controller_base->hioctrl, reg_cfg_mask);

	/*
	 * Note: Delay for 40ns at min
	 * de-assert active high frzreg signal
	 */
	clrbits_le32(&freeze_controller_base->hioctrl,
		SYSMGR_FRZCTRL_HIOCTRL_REGRST_MASK);

	/*
	 * Note: Delay for 40ns at min
	 * de-assert active low enrnsl signal
	 */
	setbits_le32(&freeze_controller_base->hioctrl,
		SYSMGR_FRZCTRL_HIOCTRL_SLEW_MASK);

	/* Set global flag to indicate channel is thawed */
	frzctrl_channel_freeze[channel_id] = FREEZE_CTRL_THAWED;
}
