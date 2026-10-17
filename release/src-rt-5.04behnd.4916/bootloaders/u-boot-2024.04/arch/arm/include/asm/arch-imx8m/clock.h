/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 *
 * Peng Fan <peng.fan at nxp.com>
 */

#include <linux/bitops.h>

#ifdef CONFIG_IMX8MQ
#include <asm/arch/clock_imx8mq.h>
#elif defined(CONFIG_IMX8MM) || defined(CONFIG_IMX8MN) || \
	defined(CONFIG_IMX8MP)
#include <asm/arch/clock_imx8mm.h>
#else
#error "Error no clock.h"
#endif

#define MHZ(X)	((X) * 1000000UL)

/* Mainly for compatible to imx common code. */
enum mxc_clock {
	MXC_ARM_CLK = 0,
	MXC_IPG_CLK,
	MXC_CSPI_CLK,
	MXC_ESDHC_CLK,
	MXC_ESDHC2_CLK,
	MXC_ESDHC3_CLK,
	MXC_I2C_CLK,
	MXC_UART_CLK,
	MXC_QSPI_CLK,
};

enum clk_slice_type {
	CORE_CLOCK_SLICE,
	BUS_CLOCK_SLICE,
	IP_CLOCK_SLICE,
	AHB_CLOCK_SLICE,
	IPG_CLOCK_SLICE,
	CORE_SEL_CLOCK_SLICE,
	DRAM_SEL_CLOCK_SLICE,
};

enum root_pre_div {
	CLK_ROOT_PRE_DIV1 = 0,
	CLK_ROOT_PRE_DIV2,
	CLK_ROOT_PRE_DIV3,
	CLK_ROOT_PRE_DIV4,
	CLK_ROOT_PRE_DIV5,
	CLK_ROOT_PRE_DIV6,
	CLK_ROOT_PRE_DIV7,
	CLK_ROOT_PRE_DIV8,
};

enum root_post_div {
	CLK_ROOT_POST_DIV1 = 0,
	CLK_ROOT_POST_DIV2,
	CLK_ROOT_POST_DIV3,
	CLK_ROOT_POST_DIV4,
	CLK_ROOT_POST_DIV5,
	CLK_ROOT_POST_DIV6,
	CLK_ROOT_POST_DIV7,
	CLK_ROOT_POST_DIV8,
	CLK_ROOT_POST_DIV9,
	CLK_ROOT_POST_DIV10,
	CLK_ROOT_POST_DIV11,
	CLK_ROOT_POST_DIV12,
	CLK_ROOT_POST_DIV13,
	CLK_ROOT_POST_DIV14,
	CLK_ROOT_POST_DIV15,
	CLK_ROOT_POST_DIV16,
	CLK_ROOT_POST_DIV17,
	CLK_ROOT_POST_DIV18,
	CLK_ROOT_POST_DIV19,
	CLK_ROOT_POST_DIV20,
	CLK_ROOT_POST_DIV21,
	CLK_ROOT_POST_DIV22,
	CLK_ROOT_POST_DIV23,
	CLK_ROOT_POST_DIV24,
	CLK_ROOT_POST_DIV25,
	CLK_ROOT_POST_DIV26,
	CLK_ROOT_POST_DIV27,
	CLK_ROOT_POST_DIV28,
	CLK_ROOT_POST_DIV29,
	CLK_ROOT_POST_DIV30,
	CLK_ROOT_POST_DIV31,
	CLK_ROOT_POST_DIV32,
	CLK_ROOT_POST_DIV33,
	CLK_ROOT_POST_DIV34,
	CLK_ROOT_POST_DIV35,
	CLK_ROOT_POST_DIV36,
	CLK_ROOT_POST_DIV37,
	CLK_ROOT_POST_DIV38,
	CLK_ROOT_POST_DIV39,
	CLK_ROOT_POST_DIV40,
	CLK_ROOT_POST_DIV41,
	CLK_ROOT_POST_DIV42,
	CLK_ROOT_POST_DIV43,
	CLK_ROOT_POST_DIV44,
	CLK_ROOT_POST_DIV45,
	CLK_ROOT_POST_DIV46,
	CLK_ROOT_POST_DIV47,
	CLK_ROOT_POST_DIV48,
	CLK_ROOT_POST_DIV49,
	CLK_ROOT_POST_DIV50,
	CLK_ROOT_POST_DIV51,
	CLK_ROOT_POST_DIV52,
	CLK_ROOT_POST_DIV53,
	CLK_ROOT_POST_DIV54,
	CLK_ROOT_POST_DIV55,
	CLK_ROOT_POST_DIV56,
	CLK_ROOT_POST_DIV57,
	CLK_ROOT_POST_DIV58,
	CLK_ROOT_POST_DIV59,
	CLK_ROOT_POST_DIV60,
	CLK_ROOT_POST_DIV61,
	CLK_ROOT_POST_DIV62,
	CLK_ROOT_POST_DIV63,
	CLK_ROOT_POST_DIV64,
};

struct clk_root_map {
	enum clk_root_index entry;
	enum clk_slice_type slice_type;
	u32 slice_index;
	u8 src_mux[8];
};

struct ccm_ccgr {
	u32 ccgr;
	u32 ccgr_set;
	u32 ccgr_clr;
	u32 ccgr_tog;
};

struct ccm_root {
	u32 target_root;
	u32 target_root_set;
	u32 target_root_clr;
	u32 target_root_tog;
	u32 misc;
	u32 misc_set;
	u32 misc_clr;
	u32 misc_tog;
	u32 nm_post;
	u32 nm_post_root_set;
	u32 nm_post_root_clr;
	u32 nm_post_root_tog;
	u32 nm_pre;
	u32 nm_pre_root_set;
	u32 nm_pre_root_clr;
	u32 nm_pre_root_tog;
	u32 db_post;
	u32 db_post_root_set;
	u32 db_post_root_clr;
	u32 db_post_root_tog;
	u32 db_pre;
	u32 db_pre_root_set;
	u32 db_pre_root_clr;
	u32 db_pre_root_tog;
	u32 reserved[4];
	u32 access_ctrl;
	u32 access_ctrl_root_set;
	u32 access_ctrl_root_clr;
	u32 access_ctrl_root_tog;
};

struct ccm_reg {
	u32 reserved_0[4096];
	struct ccm_ccgr ccgr_array[192];
	u32 reserved_1[3328];
	struct ccm_root core_root[5];
	u32 reserved_2[352];
	struct ccm_root bus_root[12];
	u32 reserved_3[128];
	struct ccm_root ahb_ipg_root[4];
	u32 reserved_4[384];
	struct ccm_root dram_sel;
	struct ccm_root core_sel;
	u32 reserved_5[448];
	struct ccm_root ip_root[78];
};

enum enet_freq {
	ENET_25MHZ = 0,
	ENET_50MHZ,
	ENET_125MHZ,
};

#define DRAM_BYPASS_ROOT_CONFIG(_rate, _m, _p, _s, _k)			\
	{								\
		.clk		=	(_rate),			\
		.alt_root_sel	=	(_m),				\
		.alt_pre_div	=	(_p),				\
		.apb_root_sel	=	(_s),				\
		.apb_pre_div	=	(_k),				\
	}

struct dram_bypass_clk_setting {
	ulong clk;
	int alt_root_sel;
	enum root_pre_div alt_pre_div;
	int apb_root_sel;
	enum root_pre_div apb_pre_div;
};

#define CCGR_CLK_ON_MASK	0x03
#define CLK_SRC_ON_MASK		0x03

#define CLK_ROOT_ON		BIT(28)
#define CLK_ROOT_OFF		(0 << 28)
#define CLK_ROOT_ENABLE_MASK	BIT(28)
#define CLK_ROOT_ENABLE_SHIFT	28
#define CLK_ROOT_SOURCE_SEL(n)	(((n) & 0x7) << 24)

/* For SEL, only use 1 bit */
#define CLK_ROOT_SRC_MUX_MASK	0x07000000
#define CLK_ROOT_SRC_MUX_SHIFT	24
#define CLK_ROOT_SRC_0		0x00000000
#define CLK_ROOT_SRC_1		0x01000000
#define CLK_ROOT_SRC_2		0x02000000
#define CLK_ROOT_SRC_3		0x03000000
#define CLK_ROOT_SRC_4		0x04000000
#define CLK_ROOT_SRC_5		0x05000000
#define CLK_ROOT_SRC_6		0x06000000
#define CLK_ROOT_SRC_7		0x07000000

#define CLK_ROOT_PRE_DIV_MASK	(0x00070000)
#define CLK_ROOT_PRE_DIV_SHIFT	16
#define CLK_ROOT_PRE_DIV(n)	(((n) << 16) & 0x00070000)

#define CLK_ROOT_AUDO_SLOW_EN	0x1000

#define CLK_ROOT_AUDO_DIV_MASK	0x700
#define CLK_ROOT_AUDO_DIV_SHIFT	0x8
#define CLK_ROOT_AUDO_DIV(n)	(((n) << 8) & 0x700)

/* For CORE: mask is 0x7; For IPG: mask is 0x3 */
#define CLK_ROOT_POST_DIV_MASK		0x3f
#define CLK_ROOT_CORE_POST_DIV_MASK	0x7
#define CLK_ROOT_IPG_POST_DIV_MASK	0x3
#define CLK_ROOT_POST_DIV_SHIFT		0
#define CLK_ROOT_POST_DIV(n)		((n) & 0x3f)
#define ENET1_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_125M_CLK		0x01000000
#define ENET1_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_50M_CLK		0x02000000
#define ENET1_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_25M_CLK		0x03000000
#define ENET_AXI_CLK_ROOT_FROM_PLL_SYS_PFD4_CLK			0x07000000
#define ENET_AXI_CLK_ROOT_FROM_SYS1_PLL_266M			0x01000000
#define ENET1_TIME_CLK_ROOT_FROM_PLL_ENET_MAIN_100M_CLK		0x01000000
#define ENET_PHY_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_25M_CLK	0x01000000

void dram_pll_init(ulong pll_val);
void dram_enable_bypass(ulong clk_val);
void dram_disable_bypass(void);
u32 imx_get_fecclk(void);
u32 imx_get_uartclk(void);
int clock_init(void);
void init_clk_usdhc(u32 index);
void init_nand_clk(void);
void init_uart_clk(u32 index);
void init_usb_clk(void);
void init_wdog_clk(void);
unsigned int mxc_get_clock(enum mxc_clock clk);
int clock_enable(enum clk_ccgr_index index, bool enable);
int clock_root_enabled(enum clk_root_index clock_id);
int clock_root_cfg(enum clk_root_index clock_id, enum root_pre_div pre_div,
		   enum root_post_div post_div, enum clk_root_src clock_src);
int clock_set_target_val(enum clk_root_index clock_id, u32 val);
int clock_get_target_val(enum clk_root_index clock_id, u32 *val);
int clock_get_prediv(enum clk_root_index clock_id, enum root_pre_div *pre_div);
int clock_get_postdiv(enum clk_root_index clock_id,
		      enum root_post_div *post_div);
int clock_get_src(enum clk_root_index clock_id, enum clk_root_src *p_clock_src);
void mxs_set_lcdclk(u32 base_addr, u32 freq);
int set_clk_qspi(void);
void enable_ocotp_clk(unsigned char enable);
int enable_i2c_clk(unsigned char enable, unsigned int i2c_num);
int set_clk_enet(enum enet_freq type);
void hab_caam_clock_enable(unsigned char enable);
