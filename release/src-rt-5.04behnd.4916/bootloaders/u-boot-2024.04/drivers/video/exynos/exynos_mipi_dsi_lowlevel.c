// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#include <common.h>
#include <asm/arch/dsim.h>
#include <asm/arch/mipi_dsim.h>
#include <asm/arch/power.h>
#include <asm/arch/cpu.h>
#include <linux/delay.h>

#include "exynos_mipi_dsi_lowlevel.h"
#include "exynos_mipi_dsi_common.h"

void exynos_mipi_dsi_func_reset(struct mipi_dsim_device *dsim)
{
	unsigned int reg;

	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	reg = readl(&mipi_dsim->swrst);

	reg |= DSIM_FUNCRST;

	writel(reg, &mipi_dsim->swrst);
}

void exynos_mipi_dsi_sw_reset(struct mipi_dsim_device *dsim)
{
	unsigned int reg = 0;

	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	reg = readl(&mipi_dsim->swrst);

	reg |= DSIM_SWRST;
	reg |= DSIM_FUNCRST;

	writel(reg, &mipi_dsim->swrst);
}

void exynos_mipi_dsi_sw_release(struct mipi_dsim_device *dsim)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = readl(&mipi_dsim->intsrc);

	reg |= INTSRC_SWRST_RELEASE;

	writel(reg, &mipi_dsim->intsrc);
}

void exynos_mipi_dsi_set_interrupt_mask(struct mipi_dsim_device *dsim,
		unsigned int mode, unsigned int mask)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = readl(&mipi_dsim->intmsk);

	if (mask)
		reg |= mode;
	else
		reg &= ~mode;

	writel(reg, &mipi_dsim->intmsk);
}

void exynos_mipi_dsi_init_fifo_pointer(struct mipi_dsim_device *dsim,
		unsigned int cfg)
{
	unsigned int reg;
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	reg = readl(&mipi_dsim->fifoctrl);

	writel(reg & ~(cfg), &mipi_dsim->fifoctrl);
	udelay(10 * 1000);
	reg |= cfg;

	writel(reg, &mipi_dsim->fifoctrl);
}

/*
 * this function set PLL P, M and S value in D-PHY
 */
void exynos_mipi_dsi_set_phy_tunning(struct mipi_dsim_device *dsim,
		unsigned int value)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	writel(DSIM_AFC_CTL(value), &mipi_dsim->phyacchr);
}

void exynos_mipi_dsi_set_main_disp_resol(struct mipi_dsim_device *dsim,
	unsigned int width_resol, unsigned int height_resol)
{
	unsigned int reg;
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	/* standby should be set after configuration so set to not ready*/
	reg = (readl(&mipi_dsim->mdresol)) & ~(DSIM_MAIN_STAND_BY);
	writel(reg, &mipi_dsim->mdresol);

	/* reset resolution */
	reg &= ~(DSIM_MAIN_VRESOL(0x7ff) | DSIM_MAIN_HRESOL(0x7ff));
	reg |= DSIM_MAIN_VRESOL(height_resol) | DSIM_MAIN_HRESOL(width_resol);

	reg |= DSIM_MAIN_STAND_BY;
	writel(reg, &mipi_dsim->mdresol);
}

void exynos_mipi_dsi_set_main_disp_vporch(struct mipi_dsim_device *dsim,
	unsigned int cmd_allow, unsigned int vfront, unsigned int vback)
{
	unsigned int reg;
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	reg = (readl(&mipi_dsim->mvporch)) &
		~((DSIM_CMD_ALLOW_MASK) | (DSIM_STABLE_VFP_MASK) |
		(DSIM_MAIN_VBP_MASK));

	reg |= ((cmd_allow & 0xf) << DSIM_CMD_ALLOW_SHIFT) |
		((vfront & 0x7ff) << DSIM_STABLE_VFP_SHIFT) |
		((vback & 0x7ff) << DSIM_MAIN_VBP_SHIFT);

	writel(reg, &mipi_dsim->mvporch);
}

void exynos_mipi_dsi_set_main_disp_hporch(struct mipi_dsim_device *dsim,
	unsigned int front, unsigned int back)
{
	unsigned int reg;
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	reg = (readl(&mipi_dsim->mhporch)) &
		~((DSIM_MAIN_HFP_MASK) | (DSIM_MAIN_HBP_MASK));

	reg |= (front << DSIM_MAIN_HFP_SHIFT) | (back << DSIM_MAIN_HBP_SHIFT);

	writel(reg, &mipi_dsim->mhporch);
}

void exynos_mipi_dsi_set_main_disp_sync_area(struct mipi_dsim_device *dsim,
	unsigned int vert, unsigned int hori)
{
	unsigned int reg;
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	reg = (readl(&mipi_dsim->msync)) &
		~((DSIM_MAIN_VSA_MASK) | (DSIM_MAIN_HSA_MASK));

	reg |= ((vert & 0x3ff) << DSIM_MAIN_VSA_SHIFT) |
		(hori << DSIM_MAIN_HSA_SHIFT);

	writel(reg, &mipi_dsim->msync);
}

void exynos_mipi_dsi_set_sub_disp_resol(struct mipi_dsim_device *dsim,
	unsigned int vert, unsigned int hori)
{
	unsigned int reg;
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	reg = (readl(&mipi_dsim->sdresol)) &
		~(DSIM_SUB_STANDY_MASK);

	writel(reg, &mipi_dsim->sdresol);

	reg &= ~(DSIM_SUB_VRESOL_MASK) | ~(DSIM_SUB_HRESOL_MASK);
	reg |= ((vert & 0x7ff) << DSIM_SUB_VRESOL_SHIFT) |
		((hori & 0x7ff) << DSIM_SUB_HRESOL_SHIFT);
	writel(reg, &mipi_dsim->sdresol);

	/* DSIM STANDBY */
	reg |= (1 << DSIM_SUB_STANDY_SHIFT);
	writel(reg, &mipi_dsim->sdresol);
}

void exynos_mipi_dsi_init_config(struct mipi_dsim_device *dsim)
{
	struct mipi_dsim_config *dsim_config = dsim->dsim_config;
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int cfg = (readl(&mipi_dsim->config)) &
		~((1 << DSIM_EOT_PACKET_SHIFT) |
		(0x1f << DSIM_HSA_MODE_SHIFT) |
		(0x3 << DSIM_NUM_OF_DATALANE_SHIFT));

	cfg |=	(dsim_config->auto_flush << DSIM_AUTO_FLUSH_SHIFT) |
		(dsim_config->eot_disable << DSIM_EOT_PACKET_SHIFT) |
		(dsim_config->auto_vertical_cnt << DSIM_AUTO_MODE_SHIFT) |
		(dsim_config->hse << DSIM_HSE_MODE_SHIFT) |
		(dsim_config->hfp << DSIM_HFP_MODE_SHIFT) |
		(dsim_config->hbp << DSIM_HBP_MODE_SHIFT) |
		(dsim_config->hsa << DSIM_HSA_MODE_SHIFT) |
		(dsim_config->e_no_data_lane << DSIM_NUM_OF_DATALANE_SHIFT);

	writel(cfg, &mipi_dsim->config);
}

void exynos_mipi_dsi_display_config(struct mipi_dsim_device *dsim,
				struct mipi_dsim_config *dsim_config)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	u32 reg = (readl(&mipi_dsim->config)) &
		~((0x3 << DSIM_BURST_MODE_SHIFT) | (1 << DSIM_VIDEO_MODE_SHIFT)
		| (0x3 << DSIM_MAINVC_SHIFT) | (0x7 << DSIM_MAINPIX_SHIFT)
		| (0x3 << DSIM_SUBVC_SHIFT) | (0x7 << DSIM_SUBPIX_SHIFT));

	if (dsim_config->e_interface == DSIM_VIDEO)
		reg |= (1 << DSIM_VIDEO_MODE_SHIFT);
	else if (dsim_config->e_interface == DSIM_COMMAND)
		reg &= ~(1 << DSIM_VIDEO_MODE_SHIFT);
	else {
		printf("unknown lcd type.\n");
		return;
	}

	/* main lcd */
	reg |= ((u8) (dsim_config->e_burst_mode) & 0x3) << DSIM_BURST_MODE_SHIFT
	| ((u8) (dsim_config->e_virtual_ch) & 0x3) << DSIM_MAINVC_SHIFT
	| ((u8) (dsim_config->e_pixel_format) & 0x7) << DSIM_MAINPIX_SHIFT;

	writel(reg, &mipi_dsim->config);
}

void exynos_mipi_dsi_enable_lane(struct mipi_dsim_device *dsim,
			unsigned int lane, unsigned int enable)
{
	unsigned int reg;
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	reg = readl(&mipi_dsim->config);

	if (enable)
		reg |= DSIM_LANE_ENx(lane);
	else
		reg &= ~DSIM_LANE_ENx(lane);

	writel(reg, &mipi_dsim->config);
}

void exynos_mipi_dsi_set_data_lane_number(struct mipi_dsim_device *dsim,
	unsigned int count)
{
	unsigned int cfg;
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	/* get the data lane number. */
	cfg = DSIM_NUM_OF_DATA_LANE(count);

	writel(cfg, &mipi_dsim->config);
}

void exynos_mipi_dsi_enable_afc(struct mipi_dsim_device *dsim,
			unsigned int enable, unsigned int afc_code)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = readl(&mipi_dsim->phyacchr);

	reg = 0;

	if (enable) {
		reg |= DSIM_AFC_EN;
		reg &= ~(0x7 << DSIM_AFC_CTL_SHIFT);
		reg |= DSIM_AFC_CTL(afc_code);
	} else
		reg &= ~DSIM_AFC_EN;

	writel(reg, &mipi_dsim->phyacchr);
}

void exynos_mipi_dsi_enable_pll_bypass(struct mipi_dsim_device *dsim,
	unsigned int enable)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->clkctrl)) &
		~(DSIM_PLL_BYPASS_EXTERNAL);

	reg |= enable << DSIM_PLL_BYPASS_SHIFT;

	writel(reg, &mipi_dsim->clkctrl);
}

void exynos_mipi_dsi_pll_freq_band(struct mipi_dsim_device *dsim,
		unsigned int freq_band)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->pllctrl)) &
		~(0x1f << DSIM_FREQ_BAND_SHIFT);

	reg |= ((freq_band & 0x1f) << DSIM_FREQ_BAND_SHIFT);

	writel(reg, &mipi_dsim->pllctrl);
}

void exynos_mipi_dsi_pll_freq(struct mipi_dsim_device *dsim,
		unsigned int pre_divider, unsigned int main_divider,
		unsigned int scaler)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->pllctrl)) &
		~(0x7ffff << 1);

	reg |= ((pre_divider & 0x3f) << DSIM_PREDIV_SHIFT) |
		((main_divider & 0x1ff) << DSIM_MAIN_SHIFT) |
		((scaler & 0x7) << DSIM_SCALER_SHIFT);

	writel(reg, &mipi_dsim->pllctrl);
}

void exynos_mipi_dsi_pll_stable_time(struct mipi_dsim_device *dsim,
	unsigned int lock_time)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	writel(lock_time, &mipi_dsim->plltmr);
}

void exynos_mipi_dsi_enable_pll(struct mipi_dsim_device *dsim,
				unsigned int enable)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->pllctrl)) &
		~(0x1 << DSIM_PLL_EN_SHIFT);

	reg |= ((enable & 0x1) << DSIM_PLL_EN_SHIFT);

	writel(reg, &mipi_dsim->pllctrl);
}

void exynos_mipi_dsi_set_byte_clock_src(struct mipi_dsim_device *dsim,
		unsigned int src)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->clkctrl)) &
		~(0x3 << DSIM_BYTE_CLK_SRC_SHIFT);

	reg |= ((unsigned int) src) << DSIM_BYTE_CLK_SRC_SHIFT;

	writel(reg, &mipi_dsim->clkctrl);
}

void exynos_mipi_dsi_enable_byte_clock(struct mipi_dsim_device *dsim,
		unsigned int enable)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->clkctrl)) &
		~(1 << DSIM_BYTE_CLKEN_SHIFT);

	reg |= enable << DSIM_BYTE_CLKEN_SHIFT;

	writel(reg, &mipi_dsim->clkctrl);
}

void exynos_mipi_dsi_set_esc_clk_prs(struct mipi_dsim_device *dsim,
		unsigned int enable, unsigned int prs_val)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->clkctrl)) &
		~((1 << DSIM_ESC_CLKEN_SHIFT) | (0xffff));

	reg |= enable << DSIM_ESC_CLKEN_SHIFT;
	if (enable)
		reg |= prs_val;

	writel(reg, &mipi_dsim->clkctrl);
}

void exynos_mipi_dsi_enable_esc_clk_on_lane(struct mipi_dsim_device *dsim,
		unsigned int lane_sel, unsigned int enable)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = readl(&mipi_dsim->clkctrl);

	if (enable)
		reg |= DSIM_LANE_ESC_CLKEN(lane_sel);
	else
		reg &= ~DSIM_LANE_ESC_CLKEN(lane_sel);

	writel(reg, &mipi_dsim->clkctrl);
}

void exynos_mipi_dsi_force_dphy_stop_state(struct mipi_dsim_device *dsim,
	unsigned int enable)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->escmode)) &
		~(0x1 << DSIM_FORCE_STOP_STATE_SHIFT);

	reg |= ((enable & 0x1) << DSIM_FORCE_STOP_STATE_SHIFT);

	writel(reg, &mipi_dsim->escmode);
}

unsigned int exynos_mipi_dsi_is_lane_state(struct mipi_dsim_device *dsim)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = readl(&mipi_dsim->status);

	/**
	 * check clock and data lane states.
	 * if MIPI-DSI controller was enabled at bootloader then
	 * TX_READY_HS_CLK is enabled otherwise STOP_STATE_CLK.
	 * so it should be checked for two case.
	 */
	if ((reg & DSIM_STOP_STATE_DAT(0xf)) &&
			((reg & DSIM_STOP_STATE_CLK) ||
			 (reg & DSIM_TX_READY_HS_CLK)))
		return 1;
	else
		return 0;
}

void exynos_mipi_dsi_set_stop_state_counter(struct mipi_dsim_device *dsim,
		unsigned int cnt_val)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->escmode)) &
		~(0x7ff << DSIM_STOP_STATE_CNT_SHIFT);

	reg |= ((cnt_val & 0x7ff) << DSIM_STOP_STATE_CNT_SHIFT);

	writel(reg, &mipi_dsim->escmode);
}

void exynos_mipi_dsi_set_bta_timeout(struct mipi_dsim_device *dsim,
		unsigned int timeout)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->timeout)) &
		~(0xff << DSIM_BTA_TOUT_SHIFT);

	reg |= (timeout << DSIM_BTA_TOUT_SHIFT);

	writel(reg, &mipi_dsim->timeout);
}

void exynos_mipi_dsi_set_lpdr_timeout(struct mipi_dsim_device *dsim,
		unsigned int timeout)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->timeout)) &
		~(0xffff << DSIM_LPDR_TOUT_SHIFT);

	reg |= (timeout << DSIM_LPDR_TOUT_SHIFT);

	writel(reg, &mipi_dsim->timeout);
}

void exynos_mipi_dsi_set_cpu_transfer_mode(struct mipi_dsim_device *dsim,
		unsigned int lp)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = readl(&mipi_dsim->escmode);

	reg &= ~DSIM_CMD_LPDT_LP;

	if (lp)
		reg |= DSIM_CMD_LPDT_LP;

	writel(reg, &mipi_dsim->escmode);
}

void exynos_mipi_dsi_set_lcdc_transfer_mode(struct mipi_dsim_device *dsim,
		unsigned int lp)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = readl(&mipi_dsim->escmode);

	reg &= ~DSIM_TX_LPDT_LP;

	if (lp)
		reg |= DSIM_TX_LPDT_LP;

	writel(reg, &mipi_dsim->escmode);
}

void exynos_mipi_dsi_enable_hs_clock(struct mipi_dsim_device *dsim,
		unsigned int enable)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->clkctrl)) &
		~(1 << DSIM_TX_REQUEST_HSCLK_SHIFT);

	reg |= enable << DSIM_TX_REQUEST_HSCLK_SHIFT;

	writel(reg, &mipi_dsim->clkctrl);
}

void exynos_mipi_dsi_dp_dn_swap(struct mipi_dsim_device *dsim,
		unsigned int swap_en)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = readl(&mipi_dsim->phyacchr1);

	reg &= ~(0x3 << DSIM_DPDN_SWAP_DATA_SHIFT);
	reg |= (swap_en & 0x3) << DSIM_DPDN_SWAP_DATA_SHIFT;

	writel(reg, &mipi_dsim->phyacchr1);
}

void exynos_mipi_dsi_hs_zero_ctrl(struct mipi_dsim_device *dsim,
		unsigned int hs_zero)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->pllctrl)) &
		~(0xf << DSIM_ZEROCTRL_SHIFT);

	reg |= ((hs_zero & 0xf) << DSIM_ZEROCTRL_SHIFT);

	writel(reg, &mipi_dsim->pllctrl);
}

void exynos_mipi_dsi_prep_ctrl(struct mipi_dsim_device *dsim, unsigned int prep)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (readl(&mipi_dsim->pllctrl)) &
		~(0x7 << DSIM_PRECTRL_SHIFT);

	reg |= ((prep & 0x7) << DSIM_PRECTRL_SHIFT);

	writel(reg, &mipi_dsim->pllctrl);
}

void exynos_mipi_dsi_clear_interrupt(struct mipi_dsim_device *dsim)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = readl(&mipi_dsim->intsrc);

	reg |= INTSRC_PLL_STABLE;

	writel(reg, &mipi_dsim->intsrc);
}

void exynos_mipi_dsi_clear_all_interrupt(struct mipi_dsim_device *dsim)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	writel(0xffffffff, &mipi_dsim->intsrc);
}

unsigned int exynos_mipi_dsi_is_pll_stable(struct mipi_dsim_device *dsim)
{
	unsigned int reg;
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	reg = readl(&mipi_dsim->status);

	return reg & DSIM_PLL_STABLE ? 1 : 0;
}

unsigned int exynos_mipi_dsi_get_fifo_state(struct mipi_dsim_device *dsim)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	return readl(&mipi_dsim->fifoctrl) & ~(0x1f);
}

void exynos_mipi_dsi_wr_tx_header(struct mipi_dsim_device *dsim,
	unsigned int di, const unsigned char data0, const unsigned char data1)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = (DSIM_PKTHDR_DAT1(data1) | DSIM_PKTHDR_DAT0(data0) |
			DSIM_PKTHDR_DI(di));

	writel(reg, &mipi_dsim->pkthdr);
}

unsigned int _exynos_mipi_dsi_get_frame_done_status(struct mipi_dsim_device
						*dsim)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = readl(&mipi_dsim->intsrc);

	return (reg & INTSRC_FRAME_DONE) ? 1 : 0;
}

void _exynos_mipi_dsi_clear_frame_done(struct mipi_dsim_device *dsim)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();
	unsigned int reg = readl(&mipi_dsim->intsrc);

	writel(reg | INTSRC_FRAME_DONE, &mipi_dsim->intsrc);
}

void exynos_mipi_dsi_wr_tx_data(struct mipi_dsim_device *dsim,
		unsigned int tx_data)
{
	struct exynos_mipi_dsim *mipi_dsim =
		(struct exynos_mipi_dsim *)samsung_get_base_mipi_dsim();

	writel(tx_data, &mipi_dsim->payload);
}
