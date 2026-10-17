// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#include <config.h>
#include <common.h>
#include <log.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dp_info.h>
#include <asm/arch/dp.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include "exynos_dp_lowlevel.h"

/* Declare global data pointer */
static void exynos_dp_enable_video_input(struct exynos_dp *dp_regs,
					 unsigned int enable)
{
	unsigned int reg;

	reg = readl(&dp_regs->video_ctl1);
	reg &= ~VIDEO_EN_MASK;

	/* enable video input */
	if (enable)
		reg |= VIDEO_EN_MASK;

	writel(reg, &dp_regs->video_ctl1);

	return;
}

void exynos_dp_enable_video_bist(struct exynos_dp *dp_regs, unsigned int enable)
{
	/* enable video bist */
	unsigned int reg;

	reg = readl(&dp_regs->video_ctl4);
	reg &= ~VIDEO_BIST_MASK;

	/* enable video bist */
	if (enable)
		reg |= VIDEO_BIST_MASK;

	writel(reg, &dp_regs->video_ctl4);

	return;
}

void exynos_dp_enable_video_mute(struct exynos_dp *dp_regs, unsigned int enable)
{
	unsigned int reg;

	reg = readl(&dp_regs->video_ctl1);
	reg &= ~(VIDEO_MUTE_MASK);
	if (enable)
		reg |= VIDEO_MUTE_MASK;

	writel(reg, &dp_regs->video_ctl1);

	return;
}


static void exynos_dp_init_analog_param(struct exynos_dp *dp_regs)
{
	unsigned int reg;

	/*
	 * Set termination
	 * Normal bandgap, Normal swing, Tx terminal registor 61 ohm
	 * 24M Phy clock, TX digital logic power is 100:1.0625V
	 */
	reg = SEL_BG_NEW_BANDGAP | TX_TERMINAL_CTRL_61_OHM |
		SWING_A_30PER_G_NORMAL;
	writel(reg, &dp_regs->analog_ctl1);

	reg = SEL_24M | TX_DVDD_BIT_1_0625V;
	writel(reg, &dp_regs->analog_ctl2);

	/*
	 * Set power source for internal clk driver to 1.0625v.
	 * Select current reference of TX driver current to 00:Ipp/2+Ic/2.
	 * Set VCO range of PLL +- 0uA
	 */
	reg = DRIVE_DVDD_BIT_1_0625V | SEL_CURRENT_DEFAULT | VCO_BIT_000_MICRO;
	writel(reg, &dp_regs->analog_ctl3);

	/*
	 * Set AUX TX terminal resistor to 102 ohm
	 * Set AUX channel amplitude control
	 */
	reg = PD_RING_OSC | AUX_TERMINAL_CTRL_52_OHM | TX_CUR1_2X | TX_CUR_4_MA;
	writel(reg, &dp_regs->pll_filter_ctl1);

	/*
	 * PLL loop filter bandwidth
	 * For 2.7Gbps: 175KHz, For 1.62Gbps: 234KHz
	 * PLL digital power select: 1.2500V
	 */
	reg = CH3_AMP_0_MV | CH2_AMP_0_MV | CH1_AMP_0_MV | CH0_AMP_0_MV;

	writel(reg, &dp_regs->amp_tuning_ctl);

	/*
	 * PLL loop filter bandwidth
	 * For 2.7Gbps: 175KHz, For 1.62Gbps: 234KHz
	 * PLL digital power select: 1.1250V
	 */
	reg = DP_PLL_LOOP_BIT_DEFAULT | DP_PLL_REF_BIT_1_1250V;
	writel(reg, &dp_regs->pll_ctl);
}

static void exynos_dp_init_interrupt(struct exynos_dp *dp_regs)
{
	/* Set interrupt registers to initial states */

	/*
	 * Disable interrupt
	 * INT pin assertion polarity. It must be configured
	 * correctly according to ICU setting.
	 * 1 = assert high, 0 = assert low
	 */
	writel(INT_POL, &dp_regs->int_ctl);

	/* Clear pending registers */
	writel(0xff, &dp_regs->common_int_sta1);
	writel(0xff, &dp_regs->common_int_sta2);
	writel(0xff, &dp_regs->common_int_sta3);
	writel(0xff, &dp_regs->common_int_sta4);
	writel(0xff, &dp_regs->int_sta);

	/* 0:mask,1: unmask */
	writel(0x00, &dp_regs->int_sta_mask1);
	writel(0x00, &dp_regs->int_sta_mask2);
	writel(0x00, &dp_regs->int_sta_mask3);
	writel(0x00, &dp_regs->int_sta_mask4);
	writel(0x00, &dp_regs->int_sta_mask);
}

void exynos_dp_reset(struct exynos_dp *dp_regs)
{
	unsigned int reg_func_1;

	/* dp tx sw reset */
	writel(RESET_DP_TX, &dp_regs->tx_sw_reset);

	exynos_dp_enable_video_input(dp_regs, DP_DISABLE);
	exynos_dp_enable_video_bist(dp_regs, DP_DISABLE);
	exynos_dp_enable_video_mute(dp_regs, DP_DISABLE);

	/* software reset */
	reg_func_1 = MASTER_VID_FUNC_EN_N | SLAVE_VID_FUNC_EN_N |
		AUD_FIFO_FUNC_EN_N | AUD_FUNC_EN_N |
		HDCP_FUNC_EN_N | SW_FUNC_EN_N;

	writel(reg_func_1, &dp_regs->func_en1);
	writel(reg_func_1, &dp_regs->func_en2);

	mdelay(1);

	exynos_dp_init_analog_param(dp_regs);
	exynos_dp_init_interrupt(dp_regs);

	return;
}

void exynos_dp_enable_sw_func(struct exynos_dp *dp_regs, unsigned int enable)
{
	unsigned int reg;

	reg = readl(&dp_regs->func_en1);
	reg &= ~(SW_FUNC_EN_N);

	if (!enable)
		reg |= SW_FUNC_EN_N;

	writel(reg, &dp_regs->func_en1);

	return;
}

unsigned int exynos_dp_set_analog_power_down(struct exynos_dp *dp_regs,
					     unsigned int block, u32 enable)
{
	unsigned int reg;

	reg = readl(&dp_regs->phy_pd);
	switch (block) {
	case AUX_BLOCK:
		reg &= ~(AUX_PD);
		if (enable)
			reg |= AUX_PD;
		break;
	case CH0_BLOCK:
		reg &= ~(CH0_PD);
		if (enable)
			reg |= CH0_PD;
		break;
	case CH1_BLOCK:
		reg &= ~(CH1_PD);
		if (enable)
			reg |= CH1_PD;
		break;
	case CH2_BLOCK:
		reg &= ~(CH2_PD);
		if (enable)
			reg |= CH2_PD;
		break;
	case CH3_BLOCK:
		reg &= ~(CH3_PD);
		if (enable)
			reg |= CH3_PD;
		break;
	case ANALOG_TOTAL:
		reg &= ~PHY_PD;
		if (enable)
			reg |= PHY_PD;
		break;
	case POWER_ALL:
		reg &= ~(PHY_PD | AUX_PD | CH0_PD | CH1_PD | CH2_PD |
			CH3_PD);
		if (enable)
			reg |= (PHY_PD | AUX_PD | CH0_PD | CH1_PD |
				CH2_PD | CH3_PD);
		break;
	default:
		printf("DP undefined block number : %d\n",  block);
		return -1;
	}

	writel(reg, &dp_regs->phy_pd);

	return 0;
}

unsigned int exynos_dp_get_pll_lock_status(struct exynos_dp *dp_regs)
{
	unsigned int reg;

	reg = readl(&dp_regs->debug_ctl);

	if (reg & PLL_LOCK)
		return PLL_LOCKED;
	else
		return PLL_UNLOCKED;
}

static void exynos_dp_set_pll_power(struct exynos_dp *dp_regs,
				    unsigned int enable)
{
	unsigned int reg;

	reg = readl(&dp_regs->pll_ctl);
	reg &= ~(DP_PLL_PD);

	if (!enable)
		reg |= DP_PLL_PD;

	writel(reg, &dp_regs->pll_ctl);
}

int exynos_dp_init_analog_func(struct exynos_dp *dp_regs)
{
	int ret = EXYNOS_DP_SUCCESS;
	unsigned int retry_cnt = 10;
	unsigned int reg;

	/* Power On All Analog block */
	exynos_dp_set_analog_power_down(dp_regs, POWER_ALL, DP_DISABLE);

	reg = PLL_LOCK_CHG;
	writel(reg, &dp_regs->common_int_sta1);

	reg = readl(&dp_regs->debug_ctl);
	reg &= ~(F_PLL_LOCK | PLL_LOCK_CTRL);
	writel(reg, &dp_regs->debug_ctl);

	/* Assert DP PLL Reset */
	reg = readl(&dp_regs->pll_ctl);
	reg |= DP_PLL_RESET;
	writel(reg, &dp_regs->pll_ctl);

	mdelay(1);

	/* Deassert DP PLL Reset */
	reg = readl(&dp_regs->pll_ctl);
	reg &= ~(DP_PLL_RESET);
	writel(reg, &dp_regs->pll_ctl);

	exynos_dp_set_pll_power(dp_regs, DP_ENABLE);

	while (exynos_dp_get_pll_lock_status(dp_regs) == PLL_UNLOCKED) {
		mdelay(1);
		retry_cnt--;
		if (retry_cnt == 0) {
			printf("DP dp's pll lock failed : retry : %d\n",
					retry_cnt);
			return -EINVAL;
		}
	}

	debug("dp's pll lock success(%d)\n", retry_cnt);

	/* Enable Serdes FIFO function and Link symbol clock domain module */
	reg = readl(&dp_regs->func_en2);
	reg &= ~(SERDES_FIFO_FUNC_EN_N | LS_CLK_DOMAIN_FUNC_EN_N
		| AUX_FUNC_EN_N);
	writel(reg, &dp_regs->func_en2);

	return ret;
}

void exynos_dp_init_hpd(struct exynos_dp *dp_regs)
{
	unsigned int reg;

	/* Clear interrupts related to Hot Plug Detect */
	reg = HOTPLUG_CHG | HPD_LOST | PLUG;
	writel(reg, &dp_regs->common_int_sta4);

	reg = INT_HPD;
	writel(reg, &dp_regs->int_sta);

	reg = readl(&dp_regs->sys_ctl3);
	reg &= ~(F_HPD | HPD_CTRL);
	writel(reg, &dp_regs->sys_ctl3);

	return;
}

static inline void exynos_dp_reset_aux(struct exynos_dp *dp_regs)
{
	unsigned int reg;

	/* Disable AUX channel module */
	reg = readl(&dp_regs->func_en2);
	reg |= AUX_FUNC_EN_N;
	writel(reg, &dp_regs->func_en2);

	return;
}

void exynos_dp_init_aux(struct exynos_dp *dp_regs)
{
	unsigned int reg;

	/* Clear interrupts related to AUX channel */
	reg = RPLY_RECEIV | AUX_ERR;
	writel(reg, &dp_regs->int_sta);

	exynos_dp_reset_aux(dp_regs);

	/* Disable AUX transaction H/W retry */
	reg = AUX_BIT_PERIOD_EXPECTED_DELAY(3) | AUX_HW_RETRY_COUNT_SEL(3)|
		AUX_HW_RETRY_INTERVAL_600_MICROSECONDS;
	writel(reg, &dp_regs->aux_hw_retry_ctl);

	/* Receive AUX Channel DEFER commands equal to DEFER_COUNT*64 */
	reg = DEFER_CTRL_EN | DEFER_COUNT(1);
	writel(reg, &dp_regs->aux_ch_defer_ctl);

	/* Enable AUX channel module */
	reg = readl(&dp_regs->func_en2);
	reg &= ~AUX_FUNC_EN_N;
	writel(reg, &dp_regs->func_en2);

	return;
}

void exynos_dp_config_interrupt(struct exynos_dp *dp_regs)
{
	unsigned int reg;

	/* 0: mask, 1: unmask */
	reg = COMMON_INT_MASK_1;
	writel(reg, &dp_regs->common_int_mask1);

	reg = COMMON_INT_MASK_2;
	writel(reg, &dp_regs->common_int_mask2);

	reg = COMMON_INT_MASK_3;
	writel(reg, &dp_regs->common_int_mask3);

	reg = COMMON_INT_MASK_4;
	writel(reg, &dp_regs->common_int_mask4);

	reg = INT_STA_MASK;
	writel(reg, &dp_regs->int_sta_mask);

	return;
}

unsigned int exynos_dp_get_plug_in_status(struct exynos_dp *dp_regs)
{
	unsigned int reg;

	reg = readl(&dp_regs->sys_ctl3);
	if (reg & HPD_STATUS)
		return 0;

	return -1;
}

unsigned int exynos_dp_detect_hpd(struct exynos_dp *dp_regs)
{
	int timeout_loop = DP_TIMEOUT_LOOP_COUNT;

	mdelay(2);

	while (exynos_dp_get_plug_in_status(dp_regs) != 0) {
		if (timeout_loop == 0)
			return -EINVAL;
		mdelay(10);
		timeout_loop--;
	}

	return EXYNOS_DP_SUCCESS;
}

unsigned int exynos_dp_start_aux_transaction(struct exynos_dp *dp_regs)
{
	unsigned int reg;
	unsigned int ret = 0;
	unsigned int retry_cnt;

	/* Enable AUX CH operation */
	reg = readl(&dp_regs->aux_ch_ctl2);
	reg |= AUX_EN;
	writel(reg, &dp_regs->aux_ch_ctl2);

	retry_cnt = 10;
	while (retry_cnt) {
		reg = readl(&dp_regs->int_sta);
		if (!(reg & RPLY_RECEIV)) {
			if (retry_cnt == 0) {
				printf("DP Reply Timeout!!\n");
				ret = -EAGAIN;
				return ret;
			}
			mdelay(1);
			retry_cnt--;
		} else
			break;
	}

	/* Clear interrupt source for AUX CH command reply */
	writel(reg, &dp_regs->int_sta);

	/* Clear interrupt source for AUX CH access error */
	reg = readl(&dp_regs->int_sta);
	if (reg & AUX_ERR) {
		printf("DP Aux Access Error\n");
		writel(AUX_ERR, &dp_regs->int_sta);
		ret = -EAGAIN;
		return ret;
	}

	/* Check AUX CH error access status */
	reg = readl(&dp_regs->aux_ch_sta);
	if ((reg & AUX_STATUS_MASK) != 0) {
		debug("DP AUX CH error happens: %x\n", reg & AUX_STATUS_MASK);
		ret = -EAGAIN;
		return ret;
	}

	return EXYNOS_DP_SUCCESS;
}

unsigned int exynos_dp_write_byte_to_dpcd(struct exynos_dp *dp_regs,
					  unsigned int reg_addr,
					  unsigned char data)
{
	unsigned int reg, ret;

	/* Clear AUX CH data buffer */
	reg = BUF_CLR;
	writel(reg, &dp_regs->buffer_data_ctl);

	/* Select DPCD device address */
	reg = AUX_ADDR_7_0(reg_addr);
	writel(reg, &dp_regs->aux_addr_7_0);
	reg = AUX_ADDR_15_8(reg_addr);
	writel(reg, &dp_regs->aux_addr_15_8);
	reg = AUX_ADDR_19_16(reg_addr);
	writel(reg, &dp_regs->aux_addr_19_16);

	/* Write data buffer */
	reg = (unsigned int)data;
	writel(reg, &dp_regs->buf_data0);

	/*
	 * Set DisplayPort transaction and write 1 byte
	 * If bit 3 is 1, DisplayPort transaction.
	 * If Bit 3 is 0, I2C transaction.
	 */
	reg = AUX_TX_COMM_DP_TRANSACTION | AUX_TX_COMM_WRITE;
	writel(reg, &dp_regs->aux_ch_ctl1);

	/* Start AUX transaction */
	ret = exynos_dp_start_aux_transaction(dp_regs);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP Aux transaction failed\n");
		return ret;
	}

	return ret;
}

unsigned int exynos_dp_read_byte_from_dpcd(struct exynos_dp *dp_regs,
					   unsigned int reg_addr,
					   unsigned char *data)
{
	unsigned int reg;
	int retval;

	/* Clear AUX CH data buffer */
	reg = BUF_CLR;
	writel(reg, &dp_regs->buffer_data_ctl);

	/* Select DPCD device address */
	reg = AUX_ADDR_7_0(reg_addr);
	writel(reg, &dp_regs->aux_addr_7_0);
	reg = AUX_ADDR_15_8(reg_addr);
	writel(reg, &dp_regs->aux_addr_15_8);
	reg = AUX_ADDR_19_16(reg_addr);
	writel(reg, &dp_regs->aux_addr_19_16);

	/*
	 * Set DisplayPort transaction and read 1 byte
	 * If bit 3 is 1, DisplayPort transaction.
	 * If Bit 3 is 0, I2C transaction.
	 */
	reg = AUX_TX_COMM_DP_TRANSACTION | AUX_TX_COMM_READ;
	writel(reg, &dp_regs->aux_ch_ctl1);

	/* Start AUX transaction */
	retval = exynos_dp_start_aux_transaction(dp_regs);
	if (!retval)
		debug("DP Aux Transaction fail!\n");

	/* Read data buffer */
	reg = readl(&dp_regs->buf_data0);
	*data = (unsigned char)(reg & 0xff);

	return retval;
}

unsigned int exynos_dp_write_bytes_to_dpcd(struct exynos_dp *dp_regs,
					   unsigned int reg_addr,
					   unsigned int count,
					   unsigned char data[])
{
	unsigned int reg;
	unsigned int start_offset;
	unsigned int cur_data_count;
	unsigned int cur_data_idx;
	unsigned int retry_cnt;
	unsigned int ret = 0;

	/* Clear AUX CH data buffer */
	reg = BUF_CLR;
	writel(reg, &dp_regs->buffer_data_ctl);

	start_offset = 0;
	while (start_offset < count) {
		/* Buffer size of AUX CH is 16 * 4bytes */
		if ((count - start_offset) > 16)
			cur_data_count = 16;
		else
			cur_data_count = count - start_offset;

		retry_cnt = 5;
		while (retry_cnt) {
			/* Select DPCD device address */
			reg = AUX_ADDR_7_0(reg_addr + start_offset);
			writel(reg, &dp_regs->aux_addr_7_0);
			reg = AUX_ADDR_15_8(reg_addr + start_offset);
			writel(reg, &dp_regs->aux_addr_15_8);
			reg = AUX_ADDR_19_16(reg_addr + start_offset);
			writel(reg, &dp_regs->aux_addr_19_16);

			for (cur_data_idx = 0; cur_data_idx < cur_data_count;
					cur_data_idx++) {
				reg = data[start_offset + cur_data_idx];
				writel(reg, (unsigned int)&dp_regs->buf_data0 +
						(4 * cur_data_idx));
			}
			/*
			* Set DisplayPort transaction and write
			* If bit 3 is 1, DisplayPort transaction.
			* If Bit 3 is 0, I2C transaction.
			*/
			reg = AUX_LENGTH(cur_data_count) |
				AUX_TX_COMM_DP_TRANSACTION | AUX_TX_COMM_WRITE;
			writel(reg, &dp_regs->aux_ch_ctl1);

			/* Start AUX transaction */
			ret = exynos_dp_start_aux_transaction(dp_regs);
			if (ret != EXYNOS_DP_SUCCESS) {
				if (retry_cnt == 0) {
					printf("DP Aux Transaction failed\n");
					return ret;
				}
				retry_cnt--;
			} else
				break;
		}
		start_offset += cur_data_count;
	}

	return ret;
}

unsigned int exynos_dp_read_bytes_from_dpcd(struct exynos_dp *dp_regs,
					    unsigned int reg_addr,
					    unsigned int count,
					    unsigned char data[])
{
	unsigned int reg;
	unsigned int start_offset;
	unsigned int cur_data_count;
	unsigned int cur_data_idx;
	unsigned int retry_cnt;
	unsigned int ret = 0;

	/* Clear AUX CH data buffer */
	reg = BUF_CLR;
	writel(reg, &dp_regs->buffer_data_ctl);

	start_offset = 0;
	while (start_offset < count) {
		/* Buffer size of AUX CH is 16 * 4bytes */
		if ((count - start_offset) > 16)
			cur_data_count = 16;
		else
			cur_data_count = count - start_offset;

		retry_cnt = 5;
		while (retry_cnt) {
			/* Select DPCD device address */
			reg = AUX_ADDR_7_0(reg_addr + start_offset);
			writel(reg, &dp_regs->aux_addr_7_0);
			reg = AUX_ADDR_15_8(reg_addr + start_offset);
			writel(reg, &dp_regs->aux_addr_15_8);
			reg = AUX_ADDR_19_16(reg_addr + start_offset);
			writel(reg, &dp_regs->aux_addr_19_16);
			/*
			 * Set DisplayPort transaction and read
			 * If bit 3 is 1, DisplayPort transaction.
			 * If Bit 3 is 0, I2C transaction.
			 */
			reg = AUX_LENGTH(cur_data_count) |
				AUX_TX_COMM_DP_TRANSACTION | AUX_TX_COMM_READ;
			writel(reg, &dp_regs->aux_ch_ctl1);

			/* Start AUX transaction */
			ret = exynos_dp_start_aux_transaction(dp_regs);
			if (ret != EXYNOS_DP_SUCCESS) {
				if (retry_cnt == 0) {
					printf("DP Aux Transaction failed\n");
					return ret;
				}
				retry_cnt--;
			} else
				break;
		}

		for (cur_data_idx = 0; cur_data_idx < cur_data_count;
				cur_data_idx++) {
			reg = readl((unsigned int)&dp_regs->buf_data0 +
					4 * cur_data_idx);
			data[start_offset + cur_data_idx] = (unsigned char)reg;
		}

		start_offset += cur_data_count;
	}

	return ret;
}

int exynos_dp_select_i2c_device(struct exynos_dp *dp_regs,
				unsigned int device_addr, unsigned int reg_addr)
{
	unsigned int reg;
	int retval;

	/* Set EDID device address */
	reg = device_addr;
	writel(reg, &dp_regs->aux_addr_7_0);
	writel(0x0, &dp_regs->aux_addr_15_8);
	writel(0x0, &dp_regs->aux_addr_19_16);

	/* Set offset from base address of EDID device */
	writel(reg_addr, &dp_regs->buf_data0);

	/*
	 * Set I2C transaction and write address
	 * If bit 3 is 1, DisplayPort transaction.
	 * If Bit 3 is 0, I2C transaction.
	 */
	reg = AUX_TX_COMM_I2C_TRANSACTION | AUX_TX_COMM_MOT |
		AUX_TX_COMM_WRITE;
	writel(reg, &dp_regs->aux_ch_ctl1);

	/* Start AUX transaction */
	retval = exynos_dp_start_aux_transaction(dp_regs);
	if (retval != 0)
		printf("%s: DP Aux Transaction fail!\n", __func__);

	return retval;
}

int exynos_dp_read_byte_from_i2c(struct exynos_dp *dp_regs,
				 unsigned int device_addr,
				 unsigned int reg_addr, unsigned int *data)
{
	unsigned int reg;
	int i;
	int retval;

	for (i = 0; i < 10; i++) {
		/* Clear AUX CH data buffer */
		reg = BUF_CLR;
		writel(reg, &dp_regs->buffer_data_ctl);

		/* Select EDID device */
		retval = exynos_dp_select_i2c_device(dp_regs, device_addr,
						     reg_addr);
		if (retval != 0) {
			printf("DP Select EDID device fail. retry !\n");
			continue;
		}

		/*
		 * Set I2C transaction and read data
		 * If bit 3 is 1, DisplayPort transaction.
		 * If Bit 3 is 0, I2C transaction.
		 */
		reg = AUX_TX_COMM_I2C_TRANSACTION |
			AUX_TX_COMM_READ;
		writel(reg, &dp_regs->aux_ch_ctl1);

		/* Start AUX transaction */
		retval = exynos_dp_start_aux_transaction(dp_regs);
		if (retval != EXYNOS_DP_SUCCESS)
			printf("%s: DP Aux Transaction fail!\n", __func__);
	}

	/* Read data */
	if (retval == 0)
		*data = readl(&dp_regs->buf_data0);

	return retval;
}

int exynos_dp_read_bytes_from_i2c(struct exynos_dp *dp_regs,
				  unsigned int device_addr,
				  unsigned int reg_addr, unsigned int count,
				  unsigned char edid[])
{
	unsigned int reg;
	unsigned int i, j;
	unsigned int cur_data_idx;
	unsigned int defer = 0;
	int retval = 0;

	for (i = 0; i < count; i += 16) { /* use 16 burst */
		for (j = 0; j < 100; j++) {
			/* Clear AUX CH data buffer */
			reg = BUF_CLR;
			writel(reg, &dp_regs->buffer_data_ctl);

			/* Set normal AUX CH command */
			reg = readl(&dp_regs->aux_ch_ctl2);
			reg &= ~ADDR_ONLY;
			writel(reg, &dp_regs->aux_ch_ctl2);

			/*
			 * If Rx sends defer, Tx sends only reads
			 * request without sending addres
			 */
			if (!defer)
				retval = exynos_dp_select_i2c_device(
					dp_regs, device_addr, reg_addr + i);
			else
				defer = 0;

			if (retval == EXYNOS_DP_SUCCESS) {
				/*
				 * Set I2C transaction and write data
				 * If bit 3 is 1, DisplayPort transaction.
				 * If Bit 3 is 0, I2C transaction.
				 */
				reg = AUX_LENGTH(16) |
					AUX_TX_COMM_I2C_TRANSACTION |
					AUX_TX_COMM_READ;
				writel(reg, &dp_regs->aux_ch_ctl1);

				/* Start AUX transaction */
				retval = exynos_dp_start_aux_transaction(
							dp_regs);
				if (retval == 0)
					break;
				else
					printf("DP Aux Transaction fail!\n");
			}
			/* Check if Rx sends defer */
			reg = readl(&dp_regs->aux_rx_comm);
			if (reg == AUX_RX_COMM_AUX_DEFER ||
				reg == AUX_RX_COMM_I2C_DEFER) {
				printf("DP Defer: %d\n", reg);
				defer = 1;
			}
		}

		for (cur_data_idx = 0; cur_data_idx < 16; cur_data_idx++) {
			reg = readl((unsigned int)&dp_regs->buf_data0
						 + 4 * cur_data_idx);
			edid[i + cur_data_idx] = (unsigned char)reg;
		}
	}

	return retval;
}

void exynos_dp_reset_macro(struct exynos_dp *dp_regs)
{
	unsigned int reg;

	reg = readl(&dp_regs->phy_test);
	reg |= MACRO_RST;
	writel(reg, &dp_regs->phy_test);

	/* 10 us is the minimum Macro reset time. */
	mdelay(1);

	reg &= ~MACRO_RST;
	writel(reg, &dp_regs->phy_test);
}

void exynos_dp_set_link_bandwidth(struct exynos_dp *dp_regs,
				  unsigned char bwtype)
{
	unsigned int reg;

	reg = (unsigned int)bwtype;

	 /* Set bandwidth to 2.7G or 1.62G */
	if ((bwtype == DP_LANE_BW_1_62) || (bwtype == DP_LANE_BW_2_70))
		writel(reg, &dp_regs->link_bw_set);
}

unsigned char exynos_dp_get_link_bandwidth(struct exynos_dp *dp_regs)
{
	unsigned char ret;
	unsigned int reg;

	reg = readl(&dp_regs->link_bw_set);
	ret = (unsigned char)reg;

	return ret;
}

void exynos_dp_set_lane_count(struct exynos_dp *dp_regs, unsigned char count)
{
	unsigned int reg;

	reg = (unsigned int)count;

	if ((count == DP_LANE_CNT_1) || (count == DP_LANE_CNT_2) ||
			(count == DP_LANE_CNT_4))
		writel(reg, &dp_regs->lane_count_set);
}

unsigned int exynos_dp_get_lane_count(struct exynos_dp *dp_regs)
{
	return readl(&dp_regs->lane_count_set);
}

unsigned char exynos_dp_get_lanex_pre_emphasis(struct exynos_dp *dp_regs,
					       unsigned char lanecnt)
{
	unsigned int reg_list[DP_LANE_CNT_4] = {
		(unsigned int)&dp_regs->ln0_link_training_ctl,
		(unsigned int)&dp_regs->ln1_link_training_ctl,
		(unsigned int)&dp_regs->ln2_link_training_ctl,
		(unsigned int)&dp_regs->ln3_link_training_ctl,
	};

	return readl(reg_list[lanecnt]);
}

void exynos_dp_set_lanex_pre_emphasis(struct exynos_dp *dp_regs,
				      unsigned char request_val,
				      unsigned char lanecnt)
{
	unsigned int reg_list[DP_LANE_CNT_4] = {
		(unsigned int)&dp_regs->ln0_link_training_ctl,
		(unsigned int)&dp_regs->ln1_link_training_ctl,
		(unsigned int)&dp_regs->ln2_link_training_ctl,
		(unsigned int)&dp_regs->ln3_link_training_ctl,
	};

	writel(request_val, reg_list[lanecnt]);
}

void exynos_dp_set_lane_pre_emphasis(struct exynos_dp *dp_regs,
				     unsigned int level, unsigned char lanecnt)
{
	unsigned char i;
	unsigned int reg;
	unsigned int reg_list[DP_LANE_CNT_4] = {
		(unsigned int)&dp_regs->ln0_link_training_ctl,
		(unsigned int)&dp_regs->ln1_link_training_ctl,
		(unsigned int)&dp_regs->ln2_link_training_ctl,
		(unsigned int)&dp_regs->ln3_link_training_ctl,
	};
	unsigned int reg_shift[DP_LANE_CNT_4] = {
		PRE_EMPHASIS_SET_0_SHIFT,
		PRE_EMPHASIS_SET_1_SHIFT,
		PRE_EMPHASIS_SET_2_SHIFT,
		PRE_EMPHASIS_SET_3_SHIFT
	};

	for (i = 0; i < lanecnt; i++) {
		reg = level << reg_shift[i];
		writel(reg, reg_list[i]);
	}
}

void exynos_dp_set_training_pattern(struct exynos_dp *dp_regs,
				    unsigned int pattern)
{
	unsigned int reg = 0;

	switch (pattern) {
	case PRBS7:
		reg = SCRAMBLING_ENABLE | LINK_QUAL_PATTERN_SET_PRBS7;
		break;
	case D10_2:
		reg = SCRAMBLING_ENABLE | LINK_QUAL_PATTERN_SET_D10_2;
		break;
	case TRAINING_PTN1:
		reg = SCRAMBLING_DISABLE | SW_TRAINING_PATTERN_SET_PTN1;
		break;
	case TRAINING_PTN2:
		reg = SCRAMBLING_DISABLE | SW_TRAINING_PATTERN_SET_PTN2;
		break;
	case DP_NONE:
		reg = SCRAMBLING_ENABLE | LINK_QUAL_PATTERN_SET_DISABLE |
			SW_TRAINING_PATTERN_SET_NORMAL;
		break;
	default:
		break;
	}

	writel(reg, &dp_regs->training_ptn_set);
}

void exynos_dp_enable_enhanced_mode(struct exynos_dp *dp_regs,
				    unsigned char enable)
{
	unsigned int reg;

	reg = readl(&dp_regs->sys_ctl4);
	reg &= ~ENHANCED;

	if (enable)
		reg |= ENHANCED;

	writel(reg, &dp_regs->sys_ctl4);
}

void exynos_dp_enable_scrambling(struct exynos_dp *dp_regs, unsigned int enable)
{
	unsigned int reg;

	reg = readl(&dp_regs->training_ptn_set);
	reg &= ~(SCRAMBLING_DISABLE);

	if (!enable)
		reg |= SCRAMBLING_DISABLE;

	writel(reg, &dp_regs->training_ptn_set);
}

int exynos_dp_init_video(struct exynos_dp *dp_regs)
{
	unsigned int reg;

	/* Clear VID_CLK_CHG[1] and VID_FORMAT_CHG[3] and VSYNC_DET[7] */
	reg = VSYNC_DET | VID_FORMAT_CHG | VID_CLK_CHG;
	writel(reg, &dp_regs->common_int_sta1);

	/* I_STRM__CLK detect : DE_CTL : Auto detect */
	reg &= ~DET_CTRL;
	writel(reg, &dp_regs->sys_ctl1);

	return 0;
}

void exynos_dp_config_video_slave_mode(struct exynos_dp *dp_regs,
				       struct edp_video_info *video_info)
{
	unsigned int reg;

	/* Video Slave mode setting */
	reg = readl(&dp_regs->func_en1);
	reg &= ~(MASTER_VID_FUNC_EN_N|SLAVE_VID_FUNC_EN_N);
	reg |= MASTER_VID_FUNC_EN_N;
	writel(reg, &dp_regs->func_en1);

	/* Configure Interlaced for slave mode video */
	reg = readl(&dp_regs->video_ctl10);
	reg &= ~INTERACE_SCAN_CFG;
	reg |= (video_info->interlaced << INTERACE_SCAN_CFG_SHIFT);
	writel(reg, &dp_regs->video_ctl10);

	/* Configure V sync polarity for slave mode video */
	reg = readl(&dp_regs->video_ctl10);
	reg &= ~VSYNC_POLARITY_CFG;
	reg |= (video_info->v_sync_polarity << V_S_POLARITY_CFG_SHIFT);
	writel(reg, &dp_regs->video_ctl10);

	/* Configure H sync polarity for slave mode video */
	reg = readl(&dp_regs->video_ctl10);
	reg &= ~HSYNC_POLARITY_CFG;
	reg |= (video_info->h_sync_polarity << H_S_POLARITY_CFG_SHIFT);
	writel(reg, &dp_regs->video_ctl10);

	/* Set video mode to slave mode */
	reg = AUDIO_MODE_SPDIF_MODE | VIDEO_MODE_SLAVE_MODE;
	writel(reg, &dp_regs->soc_general_ctl);
}

void exynos_dp_set_video_color_format(struct exynos_dp *dp_regs,
				      struct edp_video_info *video_info)
{
	unsigned int reg;

	/* Configure the input color depth, color space, dynamic range */
	reg = (video_info->dynamic_range << IN_D_RANGE_SHIFT) |
		(video_info->color_depth << IN_BPC_SHIFT) |
		(video_info->color_space << IN_COLOR_F_SHIFT);
	writel(reg, &dp_regs->video_ctl2);

	/* Set Input Color YCbCr Coefficients to ITU601 or ITU709 */
	reg = readl(&dp_regs->video_ctl3);
	reg &= ~IN_YC_COEFFI_MASK;
	if (video_info->ycbcr_coeff)
		reg |= IN_YC_COEFFI_ITU709;
	else
		reg |= IN_YC_COEFFI_ITU601;
	writel(reg, &dp_regs->video_ctl3);
}

int exynos_dp_config_video_bist(struct exynos_dp *dp_regs,
				struct exynos_dp_priv *priv)
{
	unsigned int reg;
	unsigned int bist_type = 0;
	struct edp_video_info video_info = priv->video_info;

	/* For master mode, you don't need to set the video format */
	if (video_info.master_mode == 0) {
		writel(TOTAL_LINE_CFG_L(priv->disp_info.v_total),
		       &dp_regs->total_ln_cfg_l);
		writel(TOTAL_LINE_CFG_H(priv->disp_info.v_total),
		       &dp_regs->total_ln_cfg_h);
		writel(ACTIVE_LINE_CFG_L(priv->disp_info.v_res),
		       &dp_regs->active_ln_cfg_l);
		writel(ACTIVE_LINE_CFG_H(priv->disp_info.v_res),
		       &dp_regs->active_ln_cfg_h);
		writel(priv->disp_info.v_sync_width, &dp_regs->vsw_cfg);
		writel(priv->disp_info.v_back_porch, &dp_regs->vbp_cfg);
		writel(priv->disp_info.v_front_porch, &dp_regs->vfp_cfg);

		writel(TOTAL_PIXEL_CFG_L(priv->disp_info.h_total),
		       &dp_regs->total_pix_cfg_l);
		writel(TOTAL_PIXEL_CFG_H(priv->disp_info.h_total),
		       &dp_regs->total_pix_cfg_h);
		writel(ACTIVE_PIXEL_CFG_L(priv->disp_info.h_res),
		       &dp_regs->active_pix_cfg_l);
		writel(ACTIVE_PIXEL_CFG_H(priv->disp_info.h_res),
		       &dp_regs->active_pix_cfg_h);
		writel(H_F_PORCH_CFG_L(priv->disp_info.h_front_porch),
		       &dp_regs->hfp_cfg_l);
		writel(H_F_PORCH_CFG_H(priv->disp_info.h_front_porch),
		       &dp_regs->hfp_cfg_h);
		writel(H_SYNC_PORCH_CFG_L(priv->disp_info.h_sync_width),
		       &dp_regs->hsw_cfg_l);
		writel(H_SYNC_PORCH_CFG_H(priv->disp_info.h_sync_width),
		       &dp_regs->hsw_cfg_h);
		writel(H_B_PORCH_CFG_L(priv->disp_info.h_back_porch),
		       &dp_regs->hbp_cfg_l);
		writel(H_B_PORCH_CFG_H(priv->disp_info.h_back_porch),
		       &dp_regs->hbp_cfg_h);

		/*
		 * Set SLAVE_I_SCAN_CFG[2], VSYNC_P_CFG[1],
		 * HSYNC_P_CFG[0] properly
		 */
		reg = (video_info.interlaced << INTERACE_SCAN_CFG_SHIFT |
			video_info.v_sync_polarity << V_S_POLARITY_CFG_SHIFT |
			video_info.h_sync_polarity << H_S_POLARITY_CFG_SHIFT);
		writel(reg, &dp_regs->video_ctl10);
	}

	/* BIST color bar width set--set to each bar is 32 pixel width */
	switch (video_info.bist_pattern) {
	case COLORBAR_32:
		bist_type = BIST_WIDTH_BAR_32_PIXEL |
			  BIST_TYPE_COLOR_BAR;
		break;
	case COLORBAR_64:
		bist_type = BIST_WIDTH_BAR_64_PIXEL |
			  BIST_TYPE_COLOR_BAR;
		break;
	case WHITE_GRAY_BALCKBAR_32:
		bist_type = BIST_WIDTH_BAR_32_PIXEL |
			  BIST_TYPE_WHITE_GRAY_BLACK_BAR;
		break;
	case WHITE_GRAY_BALCKBAR_64:
		bist_type = BIST_WIDTH_BAR_64_PIXEL |
			  BIST_TYPE_WHITE_GRAY_BLACK_BAR;
		break;
	case MOBILE_WHITEBAR_32:
		bist_type = BIST_WIDTH_BAR_32_PIXEL |
			  BIST_TYPE_MOBILE_WHITE_BAR;
		break;
	case MOBILE_WHITEBAR_64:
		bist_type = BIST_WIDTH_BAR_64_PIXEL |
			  BIST_TYPE_MOBILE_WHITE_BAR;
		break;
	default:
		return -1;
	}

	reg = bist_type;
	writel(reg, &dp_regs->video_ctl4);

	return 0;
}

unsigned int exynos_dp_is_slave_video_stream_clock_on(struct exynos_dp *dp_regs)
{
	unsigned int reg;

	/* Update Video stream clk detect status */
	reg = readl(&dp_regs->sys_ctl1);
	writel(reg, &dp_regs->sys_ctl1);

	reg = readl(&dp_regs->sys_ctl1);

	if (!(reg & DET_STA)) {
		debug("DP Input stream clock not detected.\n");
		return -EIO;
	}

	return EXYNOS_DP_SUCCESS;
}

void exynos_dp_set_video_cr_mn(struct exynos_dp *dp_regs, unsigned int type,
			       unsigned int m_value, unsigned int n_value)
{
	unsigned int reg;

	if (type == REGISTER_M) {
		reg = readl(&dp_regs->sys_ctl4);
		reg |= FIX_M_VID;
		writel(reg, &dp_regs->sys_ctl4);
		reg = M_VID0_CFG(m_value);
		writel(reg, &dp_regs->m_vid0);
		reg = M_VID1_CFG(m_value);
		writel(reg, &dp_regs->m_vid1);
		reg = M_VID2_CFG(m_value);
		writel(reg, &dp_regs->m_vid2);

		reg = N_VID0_CFG(n_value);
		writel(reg, &dp_regs->n_vid0);
		reg = N_VID1_CFG(n_value);
		writel(reg, &dp_regs->n_vid1);
		reg = N_VID2_CFG(n_value);
		writel(reg, &dp_regs->n_vid2);
	} else  {
		reg = readl(&dp_regs->sys_ctl4);
		reg &= ~FIX_M_VID;
		writel(reg, &dp_regs->sys_ctl4);
	}
}

void exynos_dp_set_video_timing_mode(struct exynos_dp *dp_regs,
				     unsigned int type)
{
	unsigned int reg;

	reg = readl(&dp_regs->video_ctl10);
	reg &= ~FORMAT_SEL;

	if (type != VIDEO_TIMING_FROM_CAPTURE)
		reg |= FORMAT_SEL;

	writel(reg, &dp_regs->video_ctl10);
}

void exynos_dp_enable_video_master(struct exynos_dp *dp_regs,
				   unsigned int enable)
{
	unsigned int reg;

	reg = readl(&dp_regs->soc_general_ctl);
	if (enable) {
		reg &= ~VIDEO_MODE_MASK;
		reg |= VIDEO_MASTER_MODE_EN | VIDEO_MODE_MASTER_MODE;
	} else {
		reg &= ~VIDEO_MODE_MASK;
		reg |= VIDEO_MODE_SLAVE_MODE;
	}

	writel(reg, &dp_regs->soc_general_ctl);
}

void exynos_dp_start_video(struct exynos_dp *dp_regs)
{
	unsigned int reg;

	/* Enable Video input and disable Mute */
	reg = readl(&dp_regs->video_ctl1);
	reg |= VIDEO_EN;
	writel(reg, &dp_regs->video_ctl1);
}

unsigned int exynos_dp_is_video_stream_on(struct exynos_dp *dp_regs)
{
	unsigned int reg;

	/* Update STRM_VALID */
	reg = readl(&dp_regs->sys_ctl3);
	writel(reg, &dp_regs->sys_ctl3);

	reg = readl(&dp_regs->sys_ctl3);
	if (!(reg & STRM_VALID))
		return -EIO;

	return EXYNOS_DP_SUCCESS;
}
