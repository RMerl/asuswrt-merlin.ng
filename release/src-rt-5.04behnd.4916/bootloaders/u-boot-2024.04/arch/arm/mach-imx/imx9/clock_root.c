// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <command.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/ccm_regs.h>
#include <asm/global_data.h>
#include <linux/iopoll.h>

DECLARE_GLOBAL_DATA_PTR;

static struct ccm_reg *ccm_reg = (struct ccm_reg *)CCM_BASE_ADDR;

static enum ccm_clk_src clk_root_mux[][4] = {
	{ OSC_24M_CLK, SYS_PLL_PFD0, SYS_PLL_PFD1, SYS_PLL_PFD2 }, /* bus */
	{ OSC_24M_CLK, SYS_PLL_PFD0_DIV2, SYS_PLL_PFD1_DIV2, SYS_PLL_PFD2_DIV2 }, /* non-IO */
	{ OSC_24M_CLK, SYS_PLL_PFD0_DIV2, SYS_PLL_PFD1_DIV2, VIDEO_PLL_CLK }, /* IO*/
	{ OSC_24M_CLK, SYS_PLL_PFD0, AUDIO_PLL_CLK, EXT_CLK  }, /* TPM */
	{ OSC_24M_CLK, AUDIO_PLL_CLK, VIDEO_PLL_CLK, EXT_CLK }, /* Audio */
	{ OSC_24M_CLK, AUDIO_PLL_CLK, VIDEO_PLL_CLK, SYS_PLL_PFD0 }, /* Video */
	{ OSC_24M_CLK, SYS_PLL_PFD0, SYS_PLL_PFD1, AUDIO_PLL_CLK }, /* CKO1 */
	{ OSC_24M_CLK, SYS_PLL_PFD0, SYS_PLL_PFD1, VIDEO_PLL_CLK }, /* CKO2 */
	{ OSC_24M_CLK, AUDIO_PLL_CLK, VIDEO_PLL_CLK, SYS_PLL_PFD2 }, /* CAMSCAN */
};

static struct clk_root_map clk_root_array[] = {
	{ ARM_A55_PERIPH_CLK_ROOT,	0 },
	{ ARM_A55_MTR_BUS_CLK_ROOT,	2 },
	{ ARM_A55_CLK_ROOT,		0 },
	{ M33_CLK_ROOT,			2 },
	{ ELE_CLK_ROOT,			2 },
	{ BUS_WAKEUP_CLK_ROOT,		2 },
	{ BUS_AON_CLK_ROOT,		2 },
	{ WAKEUP_AXI_CLK_ROOT,		0 },
	{ SWO_TRACE_CLK_ROOT,		2 },
	{ M33_SYSTICK_CLK_ROOT,		2 },
	{ FLEXIO1_CLK_ROOT,		2 },
	{ FLEXIO2_CLK_ROOT,		2 },
	{ LPIT1_CLK_ROOT,		2 },
	{ LPIT2_CLK_ROOT,		2 },
	{ LPTMR1_CLK_ROOT,		2 },
	{ LPTMR2_CLK_ROOT,		2 },
	{ TPM1_CLK_ROOT,		3 },
	{ TPM2_CLK_ROOT,		3 },
	{ TPM3_CLK_ROOT,		3 },
	{ TPM4_CLK_ROOT,		3 },
	{ TPM5_CLK_ROOT,		3 },
	{ TPM6_CLK_ROOT,		3 },
	{ FLEXSPI1_CLK_ROOT,		0 },
	{ CAN1_CLK_ROOT,		2 },
	{ CAN2_CLK_ROOT,		2 },
	{ LPUART1_CLK_ROOT,		2 },
	{ LPUART2_CLK_ROOT,		2 },
	{ LPUART3_CLK_ROOT,		2 },
	{ LPUART4_CLK_ROOT,		2 },
	{ LPUART5_CLK_ROOT,		2 },
	{ LPUART6_CLK_ROOT,		2 },
	{ LPUART7_CLK_ROOT,		2 },
	{ LPUART8_CLK_ROOT,		2 },
	{ LPI2C1_CLK_ROOT,		2 },
	{ LPI2C2_CLK_ROOT,		2 },
	{ LPI2C3_CLK_ROOT,		2 },
	{ LPI2C4_CLK_ROOT,		2 },
	{ LPI2C5_CLK_ROOT,		2 },
	{ LPI2C6_CLK_ROOT,		2 },
	{ LPI2C7_CLK_ROOT,		2 },
	{ LPI2C8_CLK_ROOT,		2 },
	{ LPSPI1_CLK_ROOT,		2 },
	{ LPSPI2_CLK_ROOT,		2 },
	{ LPSPI3_CLK_ROOT,		2 },
	{ LPSPI4_CLK_ROOT,		2 },
	{ LPSPI5_CLK_ROOT,		2 },
	{ LPSPI6_CLK_ROOT,		2 },
	{ LPSPI7_CLK_ROOT,		2 },
	{ LPSPI8_CLK_ROOT,		2 },
	{ I3C1_CLK_ROOT,		2 },
	{ I3C2_CLK_ROOT,		2 },
	{ USDHC1_CLK_ROOT,		0 },
	{ USDHC2_CLK_ROOT,		0 },
	{ USDHC3_CLK_ROOT,		0 },
	{ SAI1_CLK_ROOT,		4 },
	{ SAI2_CLK_ROOT,		4 },
	{ SAI3_CLK_ROOT,		4 },
	{ CCM_CKO1_CLK_ROOT,		6 },
	{ CCM_CKO2_CLK_ROOT,		7 },
	{ CCM_CKO3_CLK_ROOT,		6 },
	{ CCM_CKO4_CLK_ROOT,		7 },
	{ HSIO_CLK_ROOT,		2 },
	{ HSIO_USB_TEST_60M_CLK_ROOT,	2 },
	{ HSIO_ACSCAN_80M_CLK_ROOT,	2 },
	{ HSIO_ACSCAN_480M_CLK_ROOT,	0 },
	{ NIC_CLK_ROOT,			0 },
	{ NIC_APB_CLK_ROOT,		2 },
	{ ML_APB_CLK_ROOT,		2 },
	{ ML_CLK_ROOT,			0 },
	{ MEDIA_AXI_CLK_ROOT,		0 },
	{ MEDIA_APB_CLK_ROOT,		2 },
	{ MEDIA_LDB_CLK_ROOT,		5 },
	{ MEDIA_DISP_PIX_CLK_ROOT,	5 },
	{ CAM_PIX_CLK_ROOT,		5 },
	{ MIPI_TEST_BYTE_CLK_ROOT,	5 },
	{ MIPI_PHY_CFG_CLK_ROOT,	5 },
	{ DRAM_ALT_CLK_ROOT,		0 },
	{ DRAM_APB_CLK_ROOT,		1 },
	{ ADC_CLK_ROOT,			2 },
	{ PDM_CLK_ROOT,			4 },
	{ TSTMR1_CLK_ROOT,		2 },
	{ TSTMR2_CLK_ROOT,		2 },
	{ MQS1_CLK_ROOT,		4 },
	{ MQS2_CLK_ROOT,		4 },
	{ AUDIO_XCVR_CLK_ROOT,		1 },
	{ SPDIF_CLK_ROOT,		4 },
	{ ENET_CLK_ROOT,		1 },
	{ ENET_TIMER1_CLK_ROOT,		2 },
	{ ENET_TIMER2_CLK_ROOT,		2 },
	{ ENET_REF_CLK_ROOT,		1 },
	{ ENET_REF_PHY_CLK_ROOT,	2 },
	{ I3C1_SLOW_CLK_ROOT,		2 },
	{ I3C2_SLOW_CLK_ROOT,		2 },
	{ USB_PHY_BURUNIN_CLK_ROOT,	2 },
	{ PAL_CAME_SCAN_CLK_ROOT,	8 },
};

int ccm_clk_src_on(enum ccm_clk_src oscpll, bool enable)
{
	u32 authen;

	if (oscpll >= OSCPLL_END)
		return -EINVAL;

	authen = readl(&ccm_reg->clk_oscplls[oscpll].authen);

	/* If using cpulpm, need disable it first */
	if (authen & CCM_AUTHEN_CPULPM_MODE)
		return -EPERM;

	if (enable)
		writel(1, &ccm_reg->clk_oscplls[oscpll].direct);
	else
		writel(0, &ccm_reg->clk_oscplls[oscpll].direct);

	return 0;
}

/* auto mode, enable =  DIRECT[ON] | STATUS0[IN_USE] */
int ccm_clk_src_auto(enum ccm_clk_src oscpll, bool enable)
{
	u32 authen;

	if (oscpll >= OSCPLL_END)
		return -EINVAL;

	authen = readl(&ccm_reg->clk_oscplls[oscpll].authen);

	/* AUTO CTRL and CPULPM are mutual exclusion, need disable CPULPM first */
	if (authen & CCM_AUTHEN_CPULPM_MODE)
		return -EPERM;

	if (enable)
		writel(authen | CCM_AUTHEN_AUTO_CTRL, &ccm_reg->clk_oscplls[oscpll].authen);
	else
		writel((authen & ~CCM_AUTHEN_AUTO_CTRL), &ccm_reg->clk_oscplls[oscpll].authen);

	return 0;
}

int ccm_clk_src_lpm(enum ccm_clk_src oscpll, bool enable)
{
	u32 authen;

	if (oscpll >= OSCPLL_END)
		return -EINVAL;

	authen = readl(&ccm_reg->clk_oscplls[oscpll].authen);

	/* AUTO CTRL and CPULPM are mutual exclusion, need disable AUTO CTRL first */
	if (authen & CCM_AUTHEN_AUTO_CTRL)
		return -EPERM;

	if (enable)
		writel(authen | CCM_AUTHEN_CPULPM_MODE, &ccm_reg->clk_oscplls[oscpll].authen);
	else
		writel((authen & ~CCM_AUTHEN_CPULPM_MODE), &ccm_reg->clk_oscplls[oscpll].authen);

	return 0;
}

int ccm_clk_src_config_lpm(enum ccm_clk_src oscpll, u32 domain, u32 lpm_val)
{
	u32 lpm, authen;

	if (oscpll >= OSCPLL_END || domain >= 16)
		return -EINVAL;

	authen = readl(&ccm_reg->clk_oscplls[oscpll].authen);
	if (!(authen & CCM_AUTHEN_CPULPM_MODE))
		return -EPERM;

	if (domain > 7) {
		lpm = readl(&ccm_reg->clk_oscplls[oscpll].lpm1);
		lpm &= ~(0x3 << ((domain - 8) * 4));
		lpm |= (lpm_val & 0x3) << ((domain - 8) * 4);
		writel(lpm, &ccm_reg->clk_oscplls[oscpll].lpm1);
	} else {
		lpm = readl(&ccm_reg->clk_oscplls[oscpll].lpm0);
		lpm &= ~(0x3 << (domain * 4));
		lpm |= (lpm_val & 0x3) << (domain * 4);
		writel(lpm, &ccm_reg->clk_oscplls[oscpll].lpm0);
	}

	return 0;
}

bool ccm_clk_src_is_clk_on(enum ccm_clk_src oscpll)
{
	return !!(readl(&ccm_reg->clk_oscplls[oscpll].status0) & 0x1);
}

int ccm_clk_src_tz_access(enum ccm_clk_src oscpll, bool non_secure, bool user_mode, bool lock_tz)
{
	u32 authen;

	if (oscpll >= OSCPLL_END)
		return -EINVAL;

	authen = readl(&ccm_reg->clk_oscplls[oscpll].authen);

	authen |= non_secure ? CCM_AUTHEN_TZ_NS : 0;
	authen |= user_mode ? CCM_AUTHEN_TZ_USER : 0;
	authen |= lock_tz ? CCM_AUTHEN_LOCK_TZ : 0;

	writel(authen, &ccm_reg->clk_oscplls[oscpll].authen);

	return 0;
}

int ccm_clk_root_cfg(u32 clk_root_id, enum ccm_clk_src src, u32 div)
{
	int i;
	int ret;
	u32 mux, status;

	if (clk_root_id >= CLK_ROOT_NUM || div > 256 || div == 0)
		return -EINVAL;

	mux = clk_root_array[clk_root_id].mux_type;

	for (i = 0; i < 4; i++) {
		if (src == clk_root_mux[mux][i])
			break;
	}

	if (i == 4) {
		printf("Invalid source [%u] for this clk root\n", src);
		return -EINVAL;
	}

	writel((i << 8) | (div - 1), &ccm_reg->clk_roots[clk_root_id].control);

	ret = readl_poll_timeout(&ccm_reg->clk_roots[clk_root_id].status0, status,
				 !(status & CLK_ROOT_STATUS_CHANGING), 200000);
	if (ret)
		printf("%s: failed, status: 0x%x\n", __func__,
		       readl(&ccm_reg->clk_roots[clk_root_id].status0));

	return ret;
};

u32 ccm_clk_root_get_rate(u32 clk_root_id)
{
	u32 mux, status, div, rate;
	enum ccm_clk_src src;

	if (clk_root_id >= CLK_ROOT_NUM)
		return 0;

	status = readl(&ccm_reg->clk_roots[clk_root_id].control);

	if (status & CLK_ROOT_STATUS_OFF)
		return 0; /* clock is off */

	mux = (status & CLK_ROOT_MUX_MASK) >> CLK_ROOT_MUX_SHIFT;
	div = status & CLK_ROOT_DIV_MASK;
	src = clk_root_mux[clk_root_array[clk_root_id].mux_type][mux];

	rate = get_clk_src_rate(src) * 1000;

	return rate / (div + 1); /* return in hz */
}

int ccm_clk_root_tz_access(u32 clk_root_id, bool non_secure, bool user_mode, bool lock_tz)
{
	u32 authen;

	if (clk_root_id >= CLK_ROOT_NUM)
		return -EINVAL;

	authen = readl(&ccm_reg->clk_roots[clk_root_id].authen);

	authen |= non_secure ? CCM_AUTHEN_TZ_NS : 0;
	authen |= user_mode ? CCM_AUTHEN_TZ_USER : 0;
	authen |= lock_tz ? CCM_AUTHEN_LOCK_TZ : 0;

	writel(authen, &ccm_reg->clk_roots[clk_root_id].authen);

	return 0;
}

int ccm_lpcg_on(u32 lpcg, bool enable)
{
	u32 authen;

	if (lpcg >= CCGR_NUM)
		return -EINVAL;

	authen = readl(&ccm_reg->clk_lpcgs[lpcg].authen);

	/* If using cpulpm, need disable it first */
	if (authen & CCM_AUTHEN_CPULPM_MODE)
		return -EPERM;

	if (enable)
		writel(1, &ccm_reg->clk_lpcgs[lpcg].direct);
	else
		writel(0, &ccm_reg->clk_lpcgs[lpcg].direct);

	return 0;
}

int ccm_lpcg_lpm(u32 lpcg, bool enable)
{
	u32 authen;

	if (lpcg >= CCGR_NUM)
		return -EINVAL;

	authen = readl(&ccm_reg->clk_lpcgs[lpcg].authen);

	if (enable)
		writel(authen | CCM_AUTHEN_CPULPM_MODE, &ccm_reg->clk_lpcgs[lpcg].authen);
	else
		writel((authen & ~CCM_AUTHEN_CPULPM_MODE), &ccm_reg->clk_lpcgs[lpcg].authen);

	return 0;
}

int ccm_lpcg_config_lpm(u32 lpcg, u32 domain, u32 lpm_val)
{
	u32 lpm, authen;

	if (lpcg >= CCGR_NUM || domain >= 16)
		return -EINVAL;

	authen = readl(&ccm_reg->clk_lpcgs[lpcg].authen);
	if (!(authen & CCM_AUTHEN_CPULPM_MODE))
		return -EPERM;

	if (domain > 7) {
		lpm = readl(&ccm_reg->clk_lpcgs[lpcg].lpm1);
		lpm &= ~(0x3 << ((domain - 8) * 4));
		lpm |= (lpm_val & 0x3) << ((domain - 8) * 4);
		writel(lpm, &ccm_reg->clk_lpcgs[lpcg].lpm1);
	} else {
		lpm = readl(&ccm_reg->clk_lpcgs[lpcg].lpm0);
		lpm &= ~(0x3 << (domain * 4));
		lpm |= (lpm_val & 0x3) << (domain * 4);
		writel(lpm, &ccm_reg->clk_lpcgs[lpcg].lpm0);
	}

	return 0;
}

bool ccm_lpcg_is_clk_on(u32 lpcg)
{
	return !!(readl(&ccm_reg->clk_lpcgs[lpcg].status0) & 0x1);
}

int ccm_lpcg_tz_access(u32 lpcg, bool non_secure, bool user_mode, bool lock_tz)
{
	u32 authen;

	if (lpcg >= CCGR_NUM)
		return -EINVAL;

	authen = readl(&ccm_reg->clk_lpcgs[lpcg].authen);

	authen |= non_secure ? CCM_AUTHEN_TZ_NS : 0;
	authen |= user_mode ? CCM_AUTHEN_TZ_USER : 0;
	authen |= lock_tz ? CCM_AUTHEN_LOCK_TZ : 0;

	writel(authen, &ccm_reg->clk_lpcgs[lpcg].authen);

	return 0;
}

int ccm_shared_gpr_set(u32 gpr, u32 val)
{
	if (gpr >= SHARED_GPR_NUM)
		return -EINVAL;

	writel(val, &ccm_reg->clk_shared_gpr[gpr].gpr);

	return 0;
}

int ccm_shared_gpr_get(u32 gpr, u32 *val)
{
	if (gpr >= SHARED_GPR_NUM || !val)
		return -EINVAL;

	*val = readl(&ccm_reg->clk_shared_gpr[gpr].gpr);

	return 0;
}

int ccm_shared_gpr_tz_access(u32 gpr, bool non_secure, bool user_mode, bool lock_tz)
{
	u32 authen;

	if (gpr >= SHARED_GPR_NUM)
		return -EINVAL;

	authen = readl(&ccm_reg->clk_shared_gpr[gpr].authen);

	authen |= non_secure ? CCM_AUTHEN_TZ_NS : 0;
	authen |= user_mode ? CCM_AUTHEN_TZ_USER : 0;
	authen |= lock_tz ? CCM_AUTHEN_LOCK_TZ : 0;

	writel(authen, &ccm_reg->clk_shared_gpr[gpr].authen);

	return 0;
}
