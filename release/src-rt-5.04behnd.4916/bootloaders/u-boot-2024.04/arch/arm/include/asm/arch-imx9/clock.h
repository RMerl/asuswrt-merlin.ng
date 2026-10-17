/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 NXP
 *
 * Peng Fan <peng.fan at nxp.com>
 */

#ifndef __CLOCK_IMX9__
#define __CLOCK_IMX9__

#include <linux/bitops.h>

#define MHZ(x)	((x) * 1000000UL)

enum enet_freq {
	ENET_25MHZ = 0,
	ENET_50MHZ,
	ENET_125MHZ,
};

enum ccm_clk_src {
	OSC_24M_CLK,
	ARM_PLL,
	ARM_PLL_CLK,
	SYS_PLL_PG,
	SYS_PLL_PFD0_PG,
	SYS_PLL_PFD0,
	SYS_PLL_PFD0_DIV2,
	SYS_PLL_PFD1_PG,
	SYS_PLL_PFD1,
	SYS_PLL_PFD1_DIV2,
	SYS_PLL_PFD2_PG,
	SYS_PLL_PFD2,
	SYS_PLL_PFD2_DIV2,
	AUDIO_PLL,
	AUDIO_PLL_CLK,
	DRAM_PLL,
	DRAM_PLL_CLK,
	VIDEO_PLL,
	VIDEO_PLL_CLK,
	OSCPLL_END,
	EXT_CLK,
};

/* Mainly for compatible to imx common code. */
enum mxc_clock {
	MXC_ARM_CLK = 0,
	MXC_IPG_CLK,
	MXC_FLEXSPI_CLK,
	MXC_CSPI_CLK,
	MXC_ESDHC_CLK,
	MXC_ESDHC2_CLK,
	MXC_ESDHC3_CLK,
	MXC_UART_CLK,
	MXC_I2C_CLK,
	MXC_FEC_CLK,
};

struct ccm_obs {
	u32 direct;
	u32 reserved[31];
};

struct ccm_gpr {
	u32 gpr;
	u32 gpr_set;
	u32 gpr_clr;
	u32 gpr_tog;
	u32 authen;
	u32 authen_set;
	u32 authen_clr;
	u32 authen_tog;
};

struct ccm_lpcg_oscpll {
	u32 direct;
	u32 lpm_status0;
	u32 lpm_status1;
	u32 reserved0;
	u32 lpm0;
	u32 lpm1;
	u32 reserved1;
	u32 lpm_cur;
	u32 status0;
	u32 status1;
	u32 reserved2[2];
	u32 authen;
	u32 reserved3[3];
};

struct ccm_root {
	u32 control;
	u32 control_set;
	u32 control_clr;
	u32 control_tog;
	u32 reserved[4];
	u32 status0;
	u32 reserved1[3];
	u32 authen;
	u32 reserved2[19];
};

struct ccm_reg {
	struct ccm_root clk_roots[95]; /* 0x0 */
	u32 reserved_0[1312];
	struct ccm_obs clk_obs[6]; /* 0x4400 */
	u32 reserved_1[64];
	struct ccm_gpr clk_shared_gpr[8]; /* 0x4800 */
	u32 reserved_2[192];
	struct ccm_gpr clk_private_gpr[8]; /* 0x4C00 */
	u32 reserved_3[192];
	struct ccm_lpcg_oscpll clk_oscplls[19]; /* 0x5000 */
	u32 reserved_4[2768];
	struct ccm_lpcg_oscpll clk_lpcgs[122]; /* 0x8000 */
};

struct ana_pll_reg_elem {
	u32 reg;
	u32 reg_set;
	u32 reg_clr;
	u32 reg_tog;
};

struct ana_pll_dfs {
	struct ana_pll_reg_elem dfs_ctrl;
	struct ana_pll_reg_elem dfs_div;
};

struct ana_pll_reg {
	struct ana_pll_reg_elem ctrl;
	struct ana_pll_reg_elem ana_prg;
	struct ana_pll_reg_elem test;
	struct ana_pll_reg_elem ss; /* Spread spectrum */
	struct ana_pll_reg_elem num; /* numerator */
	struct ana_pll_reg_elem denom; /* demoninator */
	struct ana_pll_reg_elem div;
	struct ana_pll_dfs dfs[4];
	u32 pll_status;
	u32 dfs_status;
	u32 reserved[2];
};

struct anatop_reg {
	u32 osc_ctrl;
	u32 osc_state;
	u32 reserved_0[510];
	u32 chip_version;
	u32 reserved_1[511];
	struct ana_pll_reg arm_pll;
	struct ana_pll_reg sys_pll;
	struct ana_pll_reg audio_pll;
	struct ana_pll_reg dram_pll;
	struct ana_pll_reg video_pll;
};

#define PLL_CTRL_HW_CTRL_SEL BIT(16)
#define PLL_CTRL_CLKMUX_BYPASS BIT(2)
#define PLL_CTRL_CLKMUX_EN BIT(1)
#define PLL_CTRL_POWERUP BIT(0)

#define PLL_STATUS_PLL_LOCK BIT(0)
#define PLL_DFS_CTRL_ENABLE BIT(31)
#define PLL_DFS_CTRL_CLKOUT BIT(30)
#define PLL_DFS_CTRL_CLKOUT_DIV2 BIT(29)
#define PLL_DFS_CTRL_BYPASS BIT(23)

#define PLL_SS_EN BIT(15)

struct imx_intpll_rate_table {
	u32 rate; /*khz*/
	int rdiv;
	int mfi;
	int odiv;
};

struct imx_fracpll_rate_table {
	u32 rate; /*khz*/
	int rdiv;
	int mfi;
	int odiv;
	int mfn;
	int mfd;
};

#define INT_PLL_RATE(_rate, _r, _m, _o)			\
	{							\
		.rate	=	(_rate),			\
		.rdiv	=	(_r),				\
		.mfi	=	(_m),				\
		.odiv	=	(_o),				\
	}

#define FRAC_PLL_RATE(_rate, _r, _m, _o, _n, _d)			\
	{							\
		.rate	=	(_rate),			\
		.rdiv	=	(_r),				\
		.mfi	=	(_m),				\
		.odiv	=	(_o),				\
		.mfn	=	(_n),				\
		.mfd	=	(_d),				\
	}

struct clk_root_map {
	u32 clk_root_id;
	u32 mux_type;
};

struct imx_clk_setting {
	u32 clk_root;
	enum ccm_clk_src src;
	u32 div;
};

int clock_init(void);
u32 get_clk_src_rate(enum ccm_clk_src source);
u32 get_lpuart_clk(void);
void init_uart_clk(u32 index);
void init_clk_usdhc(u32 index);
int enable_i2c_clk(unsigned char enable, u32 i2c_num);
u32 imx_get_i2cclk(u32 i2c_num);
u32 mxc_get_clock(enum mxc_clock clk);
void dram_pll_init(ulong pll_val);
void dram_enable_bypass(ulong clk_val);
void dram_disable_bypass(void);
void set_arm_core_max_clk(void);

int configure_intpll(enum ccm_clk_src pll, u32 freq);

int ccm_clk_src_on(enum ccm_clk_src oscpll, bool enable);
int ccm_clk_src_auto(enum ccm_clk_src oscpll, bool enable);
int ccm_clk_src_lpm(enum ccm_clk_src oscpll, bool enable);
int ccm_clk_src_config_lpm(enum ccm_clk_src oscpll, u32 domain, u32 lpm_val);
bool ccm_clk_src_is_clk_on(enum ccm_clk_src oscpll);
int ccm_clk_src_tz_access(enum ccm_clk_src oscpll, bool non_secure, bool user_mode, bool lock_tz);
int ccm_clk_root_cfg(u32 clk_root_id, enum ccm_clk_src src, u32 div);
u32 ccm_clk_root_get_rate(u32 clk_root_id);
int ccm_clk_root_tz_access(u32 clk_root_id, bool non_secure, bool user_mode, bool lock_tz);
int ccm_lpcg_on(u32 lpcg, bool enable);
int ccm_lpcg_lpm(u32 lpcg, bool enable);
int ccm_lpcg_config_lpm(u32 lpcg, u32 domain, u32 lpm_val);
bool ccm_lpcg_is_clk_on(u32 lpcg);
int ccm_lpcg_tz_access(u32 lpcg, bool non_secure, bool user_mode, bool lock_tz);
int ccm_shared_gpr_set(u32 gpr, u32 val);
int ccm_shared_gpr_get(u32 gpr, u32 *val);
int ccm_shared_gpr_tz_access(u32 gpr, bool non_secure, bool user_mode, bool lock_tz);

void enable_usboh3_clk(unsigned char enable);
int set_clk_enet(enum enet_freq type);
int set_clk_eqos(enum enet_freq type);
void set_arm_clk(ulong freq);
#endif
