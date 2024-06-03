// SPDX-License-Identifier: GPL-2.0+
/*
 * Porting to u-boot:
 *
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * Linux IPU driver for MX51:
 *
 * (C) Copyright 2005-2010 Freescale Semiconductor, Inc.
 */

/* #define DEBUG */
#include <common.h>
#include <linux/types.h>
#include <linux/err.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <div64.h>
#include "ipu.h"
#include "ipu_regs.h"

extern struct mxc_ccm_reg *mxc_ccm;
extern u32 *ipu_cpmem_base;

struct ipu_ch_param_word {
	uint32_t data[5];
	uint32_t res[3];
};

struct ipu_ch_param {
	struct ipu_ch_param_word word[2];
};

#define ipu_ch_param_addr(ch) (((struct ipu_ch_param *)ipu_cpmem_base) + (ch))

#define _param_word(base, w) \
	(((struct ipu_ch_param *)(base))->word[(w)].data)

#define ipu_ch_param_set_field(base, w, bit, size, v) { \
	int i = (bit) / 32; \
	int off = (bit) % 32; \
	_param_word(base, w)[i] |= (v) << off; \
	if (((bit) + (size) - 1) / 32 > i) { \
		_param_word(base, w)[i + 1] |= (v) >> (off ? (32 - off) : 0); \
	} \
}

#define ipu_ch_param_mod_field(base, w, bit, size, v) { \
	int i = (bit) / 32; \
	int off = (bit) % 32; \
	u32 mask = (1UL << size) - 1; \
	u32 temp = _param_word(base, w)[i]; \
	temp &= ~(mask << off); \
	_param_word(base, w)[i] = temp | (v) << off; \
	if (((bit) + (size) - 1) / 32 > i) { \
		temp = _param_word(base, w)[i + 1]; \
		temp &= ~(mask >> (32 - off)); \
		_param_word(base, w)[i + 1] = \
			temp | ((v) >> (off ? (32 - off) : 0)); \
	} \
}

#define ipu_ch_param_read_field(base, w, bit, size) ({ \
	u32 temp2; \
	int i = (bit) / 32; \
	int off = (bit) % 32; \
	u32 mask = (1UL << size) - 1; \
	u32 temp1 = _param_word(base, w)[i]; \
	temp1 = mask & (temp1 >> off); \
	if (((bit)+(size) - 1) / 32 > i) { \
		temp2 = _param_word(base, w)[i + 1]; \
		temp2 &= mask >> (off ? (32 - off) : 0); \
		temp1 |= temp2 << (off ? (32 - off) : 0); \
	} \
	temp1; \
})

#define IPU_SW_RST_TOUT_USEC	(10000)

#define IPUV3_CLK_MX51		133000000
#define IPUV3_CLK_MX53		200000000
#define IPUV3_CLK_MX6Q		264000000
#define IPUV3_CLK_MX6DL		198000000

void clk_enable(struct clk *clk)
{
	if (clk) {
		if (clk->usecount++ == 0) {
			clk->enable(clk);
		}
	}
}

void clk_disable(struct clk *clk)
{
	if (clk) {
		if (!(--clk->usecount)) {
			if (clk->disable)
				clk->disable(clk);
		}
	}
}

int clk_get_usecount(struct clk *clk)
{
	if (clk == NULL)
		return 0;

	return clk->usecount;
}

u32 clk_get_rate(struct clk *clk)
{
	if (!clk)
		return 0;

	return clk->rate;
}

struct clk *clk_get_parent(struct clk *clk)
{
	if (!clk)
		return 0;

	return clk->parent;
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	if (!clk)
		return 0;

	if (clk->set_rate)
		clk->set_rate(clk, rate);

	return clk->rate;
}

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	if (clk == NULL || !clk->round_rate)
		return 0;

	return clk->round_rate(clk, rate);
}

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	clk->parent = parent;
	if (clk->set_parent)
		return clk->set_parent(clk, parent);
	return 0;
}

static int clk_ipu_enable(struct clk *clk)
{
	u32 reg;

	reg = __raw_readl(clk->enable_reg);
	reg |= MXC_CCM_CCGR_CG_MASK << clk->enable_shift;
	__raw_writel(reg, clk->enable_reg);

#if defined(CONFIG_MX51) || defined(CONFIG_MX53)
	/* Handshake with IPU when certain clock rates are changed. */
	reg = __raw_readl(&mxc_ccm->ccdr);
	reg &= ~MXC_CCM_CCDR_IPU_HS_MASK;
	__raw_writel(reg, &mxc_ccm->ccdr);

	/* Handshake with IPU when LPM is entered as its enabled. */
	reg = __raw_readl(&mxc_ccm->clpcr);
	reg &= ~MXC_CCM_CLPCR_BYPASS_IPU_LPM_HS;
	__raw_writel(reg, &mxc_ccm->clpcr);
#endif
	return 0;
}

static void clk_ipu_disable(struct clk *clk)
{
	u32 reg;

	reg = __raw_readl(clk->enable_reg);
	reg &= ~(MXC_CCM_CCGR_CG_MASK << clk->enable_shift);
	__raw_writel(reg, clk->enable_reg);

#if defined(CONFIG_MX51) || defined(CONFIG_MX53)
	/*
	 * No handshake with IPU whe dividers are changed
	 * as its not enabled.
	 */
	reg = __raw_readl(&mxc_ccm->ccdr);
	reg |= MXC_CCM_CCDR_IPU_HS_MASK;
	__raw_writel(reg, &mxc_ccm->ccdr);

	/* No handshake with IPU when LPM is entered as its not enabled. */
	reg = __raw_readl(&mxc_ccm->clpcr);
	reg |= MXC_CCM_CLPCR_BYPASS_IPU_LPM_HS;
	__raw_writel(reg, &mxc_ccm->clpcr);
#endif
}


static struct clk ipu_clk = {
	.name = "ipu_clk",
#if defined(CONFIG_MX51) || defined(CONFIG_MX53)
	.enable_reg = (u32 *)(CCM_BASE_ADDR +
		offsetof(struct mxc_ccm_reg, CCGR5)),
	.enable_shift = MXC_CCM_CCGR5_IPU_OFFSET,
#else
	.enable_reg = (u32 *)(CCM_BASE_ADDR +
		offsetof(struct mxc_ccm_reg, CCGR3)),
	.enable_shift = MXC_CCM_CCGR3_IPU1_IPU_DI0_OFFSET,
#endif
	.enable = clk_ipu_enable,
	.disable = clk_ipu_disable,
	.usecount = 0,
};

#if !defined CONFIG_SYS_LDB_CLOCK
#define CONFIG_SYS_LDB_CLOCK 65000000
#endif

static struct clk ldb_clk = {
	.name = "ldb_clk",
	.rate = CONFIG_SYS_LDB_CLOCK,
	.usecount = 0,
};

/* Globals */
struct clk *g_ipu_clk;
struct clk *g_ldb_clk;
unsigned char g_ipu_clk_enabled;
struct clk *g_di_clk[2];
struct clk *g_pixel_clk[2];
unsigned char g_dc_di_assignment[10];
uint32_t g_channel_init_mask;
uint32_t g_channel_enable_mask;

static int ipu_dc_use_count;
static int ipu_dp_use_count;
static int ipu_dmfc_use_count;
static int ipu_di_use_count[2];

u32 *ipu_cpmem_base;
u32 *ipu_dc_tmpl_reg;

/* Static functions */

static inline void ipu_ch_param_set_high_priority(uint32_t ch)
{
	ipu_ch_param_mod_field(ipu_ch_param_addr(ch), 1, 93, 2, 1);
};

static inline uint32_t channel_2_dma(ipu_channel_t ch, ipu_buffer_t type)
{
	return ((uint32_t) ch >> (6 * type)) & 0x3F;
};

/* Either DP BG or DP FG can be graphic window */
static inline int ipu_is_dp_graphic_chan(uint32_t dma_chan)
{
	return (dma_chan == 23 || dma_chan == 27);
}

static inline int ipu_is_dmfc_chan(uint32_t dma_chan)
{
	return ((dma_chan >= 23) && (dma_chan <= 29));
}


static inline void ipu_ch_param_set_buffer(uint32_t ch, int bufNum,
					    dma_addr_t phyaddr)
{
	ipu_ch_param_mod_field(ipu_ch_param_addr(ch), 1, 29 * bufNum, 29,
			       phyaddr / 8);
};

#define idma_is_valid(ch)	(ch != NO_DMA)
#define idma_mask(ch)		(idma_is_valid(ch) ? (1UL << (ch & 0x1F)) : 0)
#define idma_is_set(reg, dma)	(__raw_readl(reg(dma)) & idma_mask(dma))

static void ipu_pixel_clk_recalc(struct clk *clk)
{
	u32 div;
	u64 final_rate = (unsigned long long)clk->parent->rate * 16;

	div = __raw_readl(DI_BS_CLKGEN0(clk->id));
	debug("read BS_CLKGEN0 div:%d, final_rate:%lld, prate:%ld\n",
	      div, final_rate, clk->parent->rate);

	clk->rate = 0;
	if (div != 0) {
		do_div(final_rate, div);
		clk->rate = final_rate;
	}
}

static unsigned long ipu_pixel_clk_round_rate(struct clk *clk,
	unsigned long rate)
{
	u64 div, final_rate;
	u32 remainder;
	u64 parent_rate = (unsigned long long)clk->parent->rate * 16;

	/*
	 * Calculate divider
	 * Fractional part is 4 bits,
	 * so simply multiply by 2^4 to get fractional part.
	 */
	div = parent_rate;
	remainder = do_div(div, rate);
	/* Round the divider value */
	if (remainder > (rate / 2))
		div++;
	if (div < 0x10)            /* Min DI disp clock divider is 1 */
		div = 0x10;
	if (div & ~0xFEF)
		div &= 0xFF8;
	else {
		/* Round up divider if it gets us closer to desired pix clk */
		if ((div & 0xC) == 0xC) {
			div += 0x10;
			div &= ~0xF;
		}
	}
	final_rate = parent_rate;
	do_div(final_rate, div);

	return final_rate;
}

static int ipu_pixel_clk_set_rate(struct clk *clk, unsigned long rate)
{
	u64 div, parent_rate;
	u32 remainder;

	parent_rate = (unsigned long long)clk->parent->rate * 16;
	div = parent_rate;
	remainder = do_div(div, rate);
	/* Round the divider value */
	if (remainder > (rate / 2))
		div++;

	/* Round up divider if it gets us closer to desired pix clk */
	if ((div & 0xC) == 0xC) {
		div += 0x10;
		div &= ~0xF;
	}
	if (div > 0x1000)
		debug("Overflow, DI_BS_CLKGEN0 div:0x%x\n", (u32)div);

	__raw_writel(div, DI_BS_CLKGEN0(clk->id));

	/*
	 * Setup pixel clock timing
	 * Down time is half of period
	 */
	__raw_writel((div / 16) << 16, DI_BS_CLKGEN1(clk->id));

	do_div(parent_rate, div);

	clk->rate = parent_rate;

	return 0;
}

static int ipu_pixel_clk_enable(struct clk *clk)
{
	u32 disp_gen = __raw_readl(IPU_DISP_GEN);
	disp_gen |= clk->id ? DI1_COUNTER_RELEASE : DI0_COUNTER_RELEASE;
	__raw_writel(disp_gen, IPU_DISP_GEN);

	return 0;
}

static void ipu_pixel_clk_disable(struct clk *clk)
{
	u32 disp_gen = __raw_readl(IPU_DISP_GEN);
	disp_gen &= clk->id ? ~DI1_COUNTER_RELEASE : ~DI0_COUNTER_RELEASE;
	__raw_writel(disp_gen, IPU_DISP_GEN);

}

static int ipu_pixel_clk_set_parent(struct clk *clk, struct clk *parent)
{
	u32 di_gen = __raw_readl(DI_GENERAL(clk->id));

	if (parent == g_ipu_clk)
		di_gen &= ~DI_GEN_DI_CLK_EXT;
	else if (!IS_ERR(g_di_clk[clk->id]) && parent == g_ldb_clk)
		di_gen |= DI_GEN_DI_CLK_EXT;
	else
		return -EINVAL;

	__raw_writel(di_gen, DI_GENERAL(clk->id));
	ipu_pixel_clk_recalc(clk);
	return 0;
}

static struct clk pixel_clk[] = {
	{
	.name = "pixel_clk",
	.id = 0,
	.recalc = ipu_pixel_clk_recalc,
	.set_rate = ipu_pixel_clk_set_rate,
	.round_rate = ipu_pixel_clk_round_rate,
	.set_parent = ipu_pixel_clk_set_parent,
	.enable = ipu_pixel_clk_enable,
	.disable = ipu_pixel_clk_disable,
	.usecount = 0,
	},
	{
	.name = "pixel_clk",
	.id = 1,
	.recalc = ipu_pixel_clk_recalc,
	.set_rate = ipu_pixel_clk_set_rate,
	.round_rate = ipu_pixel_clk_round_rate,
	.set_parent = ipu_pixel_clk_set_parent,
	.enable = ipu_pixel_clk_enable,
	.disable = ipu_pixel_clk_disable,
	.usecount = 0,
	},
};

/*
 * This function resets IPU
 */
static void ipu_reset(void)
{
	u32 *reg;
	u32 value;
	int timeout = IPU_SW_RST_TOUT_USEC;

	reg = (u32 *)SRC_BASE_ADDR;
	value = __raw_readl(reg);
	value = value | SW_IPU_RST;
	__raw_writel(value, reg);

	while (__raw_readl(reg) & SW_IPU_RST) {
		udelay(1);
		if (!(timeout--)) {
			printf("ipu software reset timeout\n");
			break;
		}
	};
}

/*
 * This function is called by the driver framework to initialize the IPU
 * hardware.
 *
 * @param	dev	The device structure for the IPU passed in by the
 *			driver framework.
 *
 * @return      Returns 0 on success or negative error code on error
 */
int ipu_probe(void)
{
	unsigned long ipu_base;
#if defined CONFIG_MX51
	u32 temp;

	u32 *reg_hsc_mcd = (u32 *)MIPI_HSC_BASE_ADDR;
	u32 *reg_hsc_mxt_conf = (u32 *)(MIPI_HSC_BASE_ADDR + 0x800);

	 __raw_writel(0xF00, reg_hsc_mcd);

	/* CSI mode reserved*/
	temp = __raw_readl(reg_hsc_mxt_conf);
	 __raw_writel(temp | 0x0FF, reg_hsc_mxt_conf);

	temp = __raw_readl(reg_hsc_mxt_conf);
	__raw_writel(temp | 0x10000, reg_hsc_mxt_conf);
#endif

	ipu_base = IPU_CTRL_BASE_ADDR;
	ipu_cpmem_base = (u32 *)(ipu_base + IPU_CPMEM_REG_BASE);
	ipu_dc_tmpl_reg = (u32 *)(ipu_base + IPU_DC_TMPL_REG_BASE);

	g_pixel_clk[0] = &pixel_clk[0];
	g_pixel_clk[1] = &pixel_clk[1];

	g_ipu_clk = &ipu_clk;
#if defined(CONFIG_MX51)
	g_ipu_clk->rate = IPUV3_CLK_MX51;
#elif defined(CONFIG_MX53)
	g_ipu_clk->rate = IPUV3_CLK_MX53;
#else
	g_ipu_clk->rate = is_mx6sdl() ? IPUV3_CLK_MX6DL : IPUV3_CLK_MX6Q;
#endif
	debug("ipu_clk = %u\n", clk_get_rate(g_ipu_clk));
	g_ldb_clk = &ldb_clk;
	debug("ldb_clk = %u\n", clk_get_rate(g_ldb_clk));
	ipu_reset();

	clk_set_parent(g_pixel_clk[0], g_ipu_clk);
	clk_set_parent(g_pixel_clk[1], g_ipu_clk);
	clk_enable(g_ipu_clk);

	g_di_clk[0] = NULL;
	g_di_clk[1] = NULL;

	__raw_writel(0x807FFFFF, IPU_MEM_RST);
	while (__raw_readl(IPU_MEM_RST) & 0x80000000)
		;

	ipu_init_dc_mappings();

	__raw_writel(0, IPU_INT_CTRL(5));
	__raw_writel(0, IPU_INT_CTRL(6));
	__raw_writel(0, IPU_INT_CTRL(9));
	__raw_writel(0, IPU_INT_CTRL(10));

	/* DMFC Init */
	ipu_dmfc_init(DMFC_NORMAL, 1);

	/* Set sync refresh channels as high priority */
	__raw_writel(0x18800000L, IDMAC_CHA_PRI(0));

	/* Set MCU_T to divide MCU access window into 2 */
	__raw_writel(0x00400000L | (IPU_MCU_T_DEFAULT << 18), IPU_DISP_GEN);

	clk_disable(g_ipu_clk);

	return 0;
}

void ipu_dump_registers(void)
{
	debug("IPU_CONF = \t0x%08X\n", __raw_readl(IPU_CONF));
	debug("IDMAC_CONF = \t0x%08X\n", __raw_readl(IDMAC_CONF));
	debug("IDMAC_CHA_EN1 = \t0x%08X\n",
	       __raw_readl(IDMAC_CHA_EN(0)));
	debug("IDMAC_CHA_EN2 = \t0x%08X\n",
	       __raw_readl(IDMAC_CHA_EN(32)));
	debug("IDMAC_CHA_PRI1 = \t0x%08X\n",
	       __raw_readl(IDMAC_CHA_PRI(0)));
	debug("IDMAC_CHA_PRI2 = \t0x%08X\n",
	       __raw_readl(IDMAC_CHA_PRI(32)));
	debug("IPU_CHA_DB_MODE_SEL0 = \t0x%08X\n",
	       __raw_readl(IPU_CHA_DB_MODE_SEL(0)));
	debug("IPU_CHA_DB_MODE_SEL1 = \t0x%08X\n",
	       __raw_readl(IPU_CHA_DB_MODE_SEL(32)));
	debug("DMFC_WR_CHAN = \t0x%08X\n",
	       __raw_readl(DMFC_WR_CHAN));
	debug("DMFC_WR_CHAN_DEF = \t0x%08X\n",
	       __raw_readl(DMFC_WR_CHAN_DEF));
	debug("DMFC_DP_CHAN = \t0x%08X\n",
	       __raw_readl(DMFC_DP_CHAN));
	debug("DMFC_DP_CHAN_DEF = \t0x%08X\n",
	       __raw_readl(DMFC_DP_CHAN_DEF));
	debug("DMFC_IC_CTRL = \t0x%08X\n",
	       __raw_readl(DMFC_IC_CTRL));
	debug("IPU_FS_PROC_FLOW1 = \t0x%08X\n",
	       __raw_readl(IPU_FS_PROC_FLOW1));
	debug("IPU_FS_PROC_FLOW2 = \t0x%08X\n",
	       __raw_readl(IPU_FS_PROC_FLOW2));
	debug("IPU_FS_PROC_FLOW3 = \t0x%08X\n",
	       __raw_readl(IPU_FS_PROC_FLOW3));
	debug("IPU_FS_DISP_FLOW1 = \t0x%08X\n",
	       __raw_readl(IPU_FS_DISP_FLOW1));
}

/*
 * This function is called to initialize a logical IPU channel.
 *
 * @param       channel Input parameter for the logical channel ID to init.
 *
 * @param       params  Input parameter containing union of channel
 *                      initialization parameters.
 *
 * @return      Returns 0 on success or negative error code on fail
 */
int32_t ipu_init_channel(ipu_channel_t channel, ipu_channel_params_t *params)
{
	int ret = 0;
	uint32_t ipu_conf;

	debug("init channel = %d\n", IPU_CHAN_ID(channel));

	if (g_ipu_clk_enabled == 0) {
		g_ipu_clk_enabled = 1;
		clk_enable(g_ipu_clk);
	}


	if (g_channel_init_mask & (1L << IPU_CHAN_ID(channel))) {
		printf("Warning: channel already initialized %d\n",
			IPU_CHAN_ID(channel));
	}

	ipu_conf = __raw_readl(IPU_CONF);

	switch (channel) {
	case MEM_DC_SYNC:
		if (params->mem_dc_sync.di > 1) {
			ret = -EINVAL;
			goto err;
		}

		g_dc_di_assignment[1] = params->mem_dc_sync.di;
		ipu_dc_init(1, params->mem_dc_sync.di,
			     params->mem_dc_sync.interlaced);
		ipu_di_use_count[params->mem_dc_sync.di]++;
		ipu_dc_use_count++;
		ipu_dmfc_use_count++;
		break;
	case MEM_BG_SYNC:
		if (params->mem_dp_bg_sync.di > 1) {
			ret = -EINVAL;
			goto err;
		}

		g_dc_di_assignment[5] = params->mem_dp_bg_sync.di;
		ipu_dp_init(channel, params->mem_dp_bg_sync.in_pixel_fmt,
			     params->mem_dp_bg_sync.out_pixel_fmt);
		ipu_dc_init(5, params->mem_dp_bg_sync.di,
			     params->mem_dp_bg_sync.interlaced);
		ipu_di_use_count[params->mem_dp_bg_sync.di]++;
		ipu_dc_use_count++;
		ipu_dp_use_count++;
		ipu_dmfc_use_count++;
		break;
	case MEM_FG_SYNC:
		ipu_dp_init(channel, params->mem_dp_fg_sync.in_pixel_fmt,
			     params->mem_dp_fg_sync.out_pixel_fmt);

		ipu_dc_use_count++;
		ipu_dp_use_count++;
		ipu_dmfc_use_count++;
		break;
	default:
		printf("Missing channel initialization\n");
		break;
	}

	/* Enable IPU sub module */
	g_channel_init_mask |= 1L << IPU_CHAN_ID(channel);
	if (ipu_dc_use_count == 1)
		ipu_conf |= IPU_CONF_DC_EN;
	if (ipu_dp_use_count == 1)
		ipu_conf |= IPU_CONF_DP_EN;
	if (ipu_dmfc_use_count == 1)
		ipu_conf |= IPU_CONF_DMFC_EN;
	if (ipu_di_use_count[0] == 1) {
		ipu_conf |= IPU_CONF_DI0_EN;
	}
	if (ipu_di_use_count[1] == 1) {
		ipu_conf |= IPU_CONF_DI1_EN;
	}

	__raw_writel(ipu_conf, IPU_CONF);

err:
	return ret;
}

/*
 * This function is called to uninitialize a logical IPU channel.
 *
 * @param       channel Input parameter for the logical channel ID to uninit.
 */
void ipu_uninit_channel(ipu_channel_t channel)
{
	uint32_t reg;
	uint32_t in_dma, out_dma = 0;
	uint32_t ipu_conf;

	if ((g_channel_init_mask & (1L << IPU_CHAN_ID(channel))) == 0) {
		debug("Channel already uninitialized %d\n",
			IPU_CHAN_ID(channel));
		return;
	}

	/*
	 * Make sure channel is disabled
	 * Get input and output dma channels
	 */
	in_dma = channel_2_dma(channel, IPU_OUTPUT_BUFFER);
	out_dma = channel_2_dma(channel, IPU_VIDEO_IN_BUFFER);

	if (idma_is_set(IDMAC_CHA_EN, in_dma) ||
	    idma_is_set(IDMAC_CHA_EN, out_dma)) {
		printf(
			"Channel %d is not disabled, disable first\n",
			IPU_CHAN_ID(channel));
		return;
	}

	ipu_conf = __raw_readl(IPU_CONF);

	/* Reset the double buffer */
	reg = __raw_readl(IPU_CHA_DB_MODE_SEL(in_dma));
	__raw_writel(reg & ~idma_mask(in_dma), IPU_CHA_DB_MODE_SEL(in_dma));
	reg = __raw_readl(IPU_CHA_DB_MODE_SEL(out_dma));
	__raw_writel(reg & ~idma_mask(out_dma), IPU_CHA_DB_MODE_SEL(out_dma));

	switch (channel) {
	case MEM_DC_SYNC:
		ipu_dc_uninit(1);
		ipu_di_use_count[g_dc_di_assignment[1]]--;
		ipu_dc_use_count--;
		ipu_dmfc_use_count--;
		break;
	case MEM_BG_SYNC:
		ipu_dp_uninit(channel);
		ipu_dc_uninit(5);
		ipu_di_use_count[g_dc_di_assignment[5]]--;
		ipu_dc_use_count--;
		ipu_dp_use_count--;
		ipu_dmfc_use_count--;
		break;
	case MEM_FG_SYNC:
		ipu_dp_uninit(channel);
		ipu_dc_use_count--;
		ipu_dp_use_count--;
		ipu_dmfc_use_count--;
		break;
	default:
		break;
	}

	g_channel_init_mask &= ~(1L << IPU_CHAN_ID(channel));

	if (ipu_dc_use_count == 0)
		ipu_conf &= ~IPU_CONF_DC_EN;
	if (ipu_dp_use_count == 0)
		ipu_conf &= ~IPU_CONF_DP_EN;
	if (ipu_dmfc_use_count == 0)
		ipu_conf &= ~IPU_CONF_DMFC_EN;
	if (ipu_di_use_count[0] == 0) {
		ipu_conf &= ~IPU_CONF_DI0_EN;
	}
	if (ipu_di_use_count[1] == 0) {
		ipu_conf &= ~IPU_CONF_DI1_EN;
	}

	__raw_writel(ipu_conf, IPU_CONF);

	if (ipu_conf == 0) {
		clk_disable(g_ipu_clk);
		g_ipu_clk_enabled = 0;
	}

}

static inline void ipu_ch_param_dump(int ch)
{
#ifdef DEBUG
	struct ipu_ch_param *p = ipu_ch_param_addr(ch);
	debug("ch %d word 0 - %08X %08X %08X %08X %08X\n", ch,
		 p->word[0].data[0], p->word[0].data[1], p->word[0].data[2],
		 p->word[0].data[3], p->word[0].data[4]);
	debug("ch %d word 1 - %08X %08X %08X %08X %08X\n", ch,
		 p->word[1].data[0], p->word[1].data[1], p->word[1].data[2],
		 p->word[1].data[3], p->word[1].data[4]);
	debug("PFS 0x%x, ",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 1, 85, 4));
	debug("BPP 0x%x, ",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 0, 107, 3));
	debug("NPB 0x%x\n",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 1, 78, 7));

	debug("FW %d, ",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 0, 125, 13));
	debug("FH %d, ",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 0, 138, 12));
	debug("Stride %d\n",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 1, 102, 14));

	debug("Width0 %d+1, ",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 1, 116, 3));
	debug("Width1 %d+1, ",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 1, 119, 3));
	debug("Width2 %d+1, ",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 1, 122, 3));
	debug("Width3 %d+1, ",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 1, 125, 3));
	debug("Offset0 %d, ",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 1, 128, 5));
	debug("Offset1 %d, ",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 1, 133, 5));
	debug("Offset2 %d, ",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 1, 138, 5));
	debug("Offset3 %d\n",
		 ipu_ch_param_read_field(ipu_ch_param_addr(ch), 1, 143, 5));
#endif
}

static inline void ipu_ch_params_set_packing(struct ipu_ch_param *p,
					      int red_width, int red_offset,
					      int green_width, int green_offset,
					      int blue_width, int blue_offset,
					      int alpha_width, int alpha_offset)
{
	/* Setup red width and offset */
	ipu_ch_param_set_field(p, 1, 116, 3, red_width - 1);
	ipu_ch_param_set_field(p, 1, 128, 5, red_offset);
	/* Setup green width and offset */
	ipu_ch_param_set_field(p, 1, 119, 3, green_width - 1);
	ipu_ch_param_set_field(p, 1, 133, 5, green_offset);
	/* Setup blue width and offset */
	ipu_ch_param_set_field(p, 1, 122, 3, blue_width - 1);
	ipu_ch_param_set_field(p, 1, 138, 5, blue_offset);
	/* Setup alpha width and offset */
	ipu_ch_param_set_field(p, 1, 125, 3, alpha_width - 1);
	ipu_ch_param_set_field(p, 1, 143, 5, alpha_offset);
}

static void ipu_ch_param_init(int ch,
			      uint32_t pixel_fmt, uint32_t width,
			      uint32_t height, uint32_t stride,
			      uint32_t u, uint32_t v,
			      uint32_t uv_stride, dma_addr_t addr0,
			      dma_addr_t addr1)
{
	uint32_t u_offset = 0;
	uint32_t v_offset = 0;
	struct ipu_ch_param params;

	memset(&params, 0, sizeof(params));

	ipu_ch_param_set_field(&params, 0, 125, 13, width - 1);

	if ((ch == 8) || (ch == 9) || (ch == 10)) {
		ipu_ch_param_set_field(&params, 0, 138, 12, (height / 2) - 1);
		ipu_ch_param_set_field(&params, 1, 102, 14, (stride * 2) - 1);
	} else {
		ipu_ch_param_set_field(&params, 0, 138, 12, height - 1);
		ipu_ch_param_set_field(&params, 1, 102, 14, stride - 1);
	}

	ipu_ch_param_set_field(&params, 1, 0, 29, addr0 >> 3);
	ipu_ch_param_set_field(&params, 1, 29, 29, addr1 >> 3);

	switch (pixel_fmt) {
	case IPU_PIX_FMT_GENERIC:
		/*Represents 8-bit Generic data */
		ipu_ch_param_set_field(&params, 0, 107, 3, 5);	/* bits/pixel */
		ipu_ch_param_set_field(&params, 1, 85, 4, 6);	/* pix format */
		ipu_ch_param_set_field(&params, 1, 78, 7, 63);	/* burst size */

		break;
	case IPU_PIX_FMT_GENERIC_32:
		/*Represents 32-bit Generic data */
		break;
	case IPU_PIX_FMT_RGB565:
		ipu_ch_param_set_field(&params, 0, 107, 3, 3);	/* bits/pixel */
		ipu_ch_param_set_field(&params, 1, 85, 4, 7);	/* pix format */
		ipu_ch_param_set_field(&params, 1, 78, 7, 15);	/* burst size */

		ipu_ch_params_set_packing(&params, 5, 0, 6, 5, 5, 11, 8, 16);
		break;
	case IPU_PIX_FMT_BGR24:
		ipu_ch_param_set_field(&params, 0, 107, 3, 1);	/* bits/pixel */
		ipu_ch_param_set_field(&params, 1, 85, 4, 7);	/* pix format */
		ipu_ch_param_set_field(&params, 1, 78, 7, 19);	/* burst size */

		ipu_ch_params_set_packing(&params, 8, 0, 8, 8, 8, 16, 8, 24);
		break;
	case IPU_PIX_FMT_RGB24:
	case IPU_PIX_FMT_YUV444:
		ipu_ch_param_set_field(&params, 0, 107, 3, 1);	/* bits/pixel */
		ipu_ch_param_set_field(&params, 1, 85, 4, 7);	/* pix format */
		ipu_ch_param_set_field(&params, 1, 78, 7, 19);	/* burst size */

		ipu_ch_params_set_packing(&params, 8, 16, 8, 8, 8, 0, 8, 24);
		break;
	case IPU_PIX_FMT_BGRA32:
	case IPU_PIX_FMT_BGR32:
		ipu_ch_param_set_field(&params, 0, 107, 3, 0);	/* bits/pixel */
		ipu_ch_param_set_field(&params, 1, 85, 4, 7);	/* pix format */
		ipu_ch_param_set_field(&params, 1, 78, 7, 15);	/* burst size */

		ipu_ch_params_set_packing(&params, 8, 8, 8, 16, 8, 24, 8, 0);
		break;
	case IPU_PIX_FMT_RGBA32:
	case IPU_PIX_FMT_RGB32:
		ipu_ch_param_set_field(&params, 0, 107, 3, 0);	/* bits/pixel */
		ipu_ch_param_set_field(&params, 1, 85, 4, 7);	/* pix format */
		ipu_ch_param_set_field(&params, 1, 78, 7, 15);	/* burst size */

		ipu_ch_params_set_packing(&params, 8, 24, 8, 16, 8, 8, 8, 0);
		break;
	case IPU_PIX_FMT_ABGR32:
		ipu_ch_param_set_field(&params, 0, 107, 3, 0);	/* bits/pixel */
		ipu_ch_param_set_field(&params, 1, 85, 4, 7);	/* pix format */

		ipu_ch_params_set_packing(&params, 8, 0, 8, 8, 8, 16, 8, 24);
		break;
	case IPU_PIX_FMT_UYVY:
		ipu_ch_param_set_field(&params, 0, 107, 3, 3);	/* bits/pixel */
		ipu_ch_param_set_field(&params, 1, 85, 4, 0xA);	/* pix format */
		ipu_ch_param_set_field(&params, 1, 78, 7, 15);	/* burst size */
		break;
	case IPU_PIX_FMT_YUYV:
		ipu_ch_param_set_field(&params, 0, 107, 3, 3);	/* bits/pixel */
		ipu_ch_param_set_field(&params, 1, 85, 4, 0x8);	/* pix format */
		ipu_ch_param_set_field(&params, 1, 78, 7, 31);	/* burst size */
		break;
	case IPU_PIX_FMT_YUV420P2:
	case IPU_PIX_FMT_YUV420P:
		ipu_ch_param_set_field(&params, 1, 85, 4, 2);	/* pix format */

		if (uv_stride < stride / 2)
			uv_stride = stride / 2;

		u_offset = stride * height;
		v_offset = u_offset + (uv_stride * height / 2);
		/* burst size */
		if ((ch == 8) || (ch == 9) || (ch == 10)) {
			ipu_ch_param_set_field(&params, 1, 78, 7, 15);
			uv_stride = uv_stride*2;
		} else {
			ipu_ch_param_set_field(&params, 1, 78, 7, 31);
		}
		break;
	case IPU_PIX_FMT_YVU422P:
		/* BPP & pixel format */
		ipu_ch_param_set_field(&params, 1, 85, 4, 1);	/* pix format */
		ipu_ch_param_set_field(&params, 1, 78, 7, 31);	/* burst size */

		if (uv_stride < stride / 2)
			uv_stride = stride / 2;

		v_offset = (v == 0) ? stride * height : v;
		u_offset = (u == 0) ? v_offset + v_offset / 2 : u;
		break;
	case IPU_PIX_FMT_YUV422P:
		/* BPP & pixel format */
		ipu_ch_param_set_field(&params, 1, 85, 4, 1);	/* pix format */
		ipu_ch_param_set_field(&params, 1, 78, 7, 31);	/* burst size */

		if (uv_stride < stride / 2)
			uv_stride = stride / 2;

		u_offset = (u == 0) ? stride * height : u;
		v_offset = (v == 0) ? u_offset + u_offset / 2 : v;
		break;
	case IPU_PIX_FMT_NV12:
		/* BPP & pixel format */
		ipu_ch_param_set_field(&params, 1, 85, 4, 4);	/* pix format */
		ipu_ch_param_set_field(&params, 1, 78, 7, 31);	/* burst size */
		uv_stride = stride;
		u_offset = (u == 0) ? stride * height : u;
		break;
	default:
		puts("mxc ipu: unimplemented pixel format\n");
		break;
	}


	if (uv_stride)
		ipu_ch_param_set_field(&params, 1, 128, 14, uv_stride - 1);

	/* Get the uv offset from user when need cropping */
	if (u || v) {
		u_offset = u;
		v_offset = v;
	}

	/* UBO and VBO are 22-bit */
	if (u_offset/8 > 0x3fffff)
		puts("The value of U offset exceeds IPU limitation\n");
	if (v_offset/8 > 0x3fffff)
		puts("The value of V offset exceeds IPU limitation\n");

	ipu_ch_param_set_field(&params, 0, 46, 22, u_offset / 8);
	ipu_ch_param_set_field(&params, 0, 68, 22, v_offset / 8);

	debug("initializing idma ch %d @ %p\n", ch, ipu_ch_param_addr(ch));
	memcpy(ipu_ch_param_addr(ch), &params, sizeof(params));
};

/*
 * This function is called to initialize a buffer for logical IPU channel.
 *
 * @param       channel         Input parameter for the logical channel ID.
 *
 * @param       type            Input parameter which buffer to initialize.
 *
 * @param       pixel_fmt       Input parameter for pixel format of buffer.
 *                              Pixel format is a FOURCC ASCII code.
 *
 * @param       width           Input parameter for width of buffer in pixels.
 *
 * @param       height          Input parameter for height of buffer in pixels.
 *
 * @param       stride          Input parameter for stride length of buffer
 *                              in pixels.
 *
 * @param       phyaddr_0       Input parameter buffer 0 physical address.
 *
 * @param       phyaddr_1       Input parameter buffer 1 physical address.
 *                              Setting this to a value other than NULL enables
 *                              double buffering mode.
 *
 * @param       u		private u offset for additional cropping,
 *				zero if not used.
 *
 * @param       v		private v offset for additional cropping,
 *				zero if not used.
 *
 * @return      Returns 0 on success or negative error code on fail
 */
int32_t ipu_init_channel_buffer(ipu_channel_t channel, ipu_buffer_t type,
				uint32_t pixel_fmt,
				uint16_t width, uint16_t height,
				uint32_t stride,
				dma_addr_t phyaddr_0, dma_addr_t phyaddr_1,
				uint32_t u, uint32_t v)
{
	uint32_t reg;
	uint32_t dma_chan;

	dma_chan = channel_2_dma(channel, type);
	if (!idma_is_valid(dma_chan))
		return -EINVAL;

	if (stride < width * bytes_per_pixel(pixel_fmt))
		stride = width * bytes_per_pixel(pixel_fmt);

	if (stride % 4) {
		printf(
			"Stride not 32-bit aligned, stride = %d\n", stride);
		return -EINVAL;
	}
	/* Build parameter memory data for DMA channel */
	ipu_ch_param_init(dma_chan, pixel_fmt, width, height, stride, u, v, 0,
			   phyaddr_0, phyaddr_1);

	if (ipu_is_dmfc_chan(dma_chan)) {
		ipu_dmfc_set_wait4eot(dma_chan, width);
	}

	if (idma_is_set(IDMAC_CHA_PRI, dma_chan))
		ipu_ch_param_set_high_priority(dma_chan);

	ipu_ch_param_dump(dma_chan);

	reg = __raw_readl(IPU_CHA_DB_MODE_SEL(dma_chan));
	if (phyaddr_1)
		reg |= idma_mask(dma_chan);
	else
		reg &= ~idma_mask(dma_chan);
	__raw_writel(reg, IPU_CHA_DB_MODE_SEL(dma_chan));

	/* Reset to buffer 0 */
	__raw_writel(idma_mask(dma_chan), IPU_CHA_CUR_BUF(dma_chan));

	return 0;
}

/*
 * This function enables a logical channel.
 *
 * @param       channel         Input parameter for the logical channel ID.
 *
 * @return      This function returns 0 on success or negative error code on
 *              fail.
 */
int32_t ipu_enable_channel(ipu_channel_t channel)
{
	uint32_t reg;
	uint32_t in_dma;
	uint32_t out_dma;

	if (g_channel_enable_mask & (1L << IPU_CHAN_ID(channel))) {
		printf("Warning: channel already enabled %d\n",
			IPU_CHAN_ID(channel));
	}

	/* Get input and output dma channels */
	out_dma = channel_2_dma(channel, IPU_OUTPUT_BUFFER);
	in_dma = channel_2_dma(channel, IPU_VIDEO_IN_BUFFER);

	if (idma_is_valid(in_dma)) {
		reg = __raw_readl(IDMAC_CHA_EN(in_dma));
		__raw_writel(reg | idma_mask(in_dma), IDMAC_CHA_EN(in_dma));
	}
	if (idma_is_valid(out_dma)) {
		reg = __raw_readl(IDMAC_CHA_EN(out_dma));
		__raw_writel(reg | idma_mask(out_dma), IDMAC_CHA_EN(out_dma));
	}

	if ((channel == MEM_DC_SYNC) || (channel == MEM_BG_SYNC) ||
	    (channel == MEM_FG_SYNC))
		ipu_dp_dc_enable(channel);

	g_channel_enable_mask |= 1L << IPU_CHAN_ID(channel);

	return 0;
}

/*
 * This function clear buffer ready for a logical channel.
 *
 * @param       channel         Input parameter for the logical channel ID.
 *
 * @param       type            Input parameter which buffer to clear.
 *
 * @param       bufNum          Input parameter for which buffer number clear
 *				ready state.
 *
 */
void ipu_clear_buffer_ready(ipu_channel_t channel, ipu_buffer_t type,
		uint32_t bufNum)
{
	uint32_t dma_ch = channel_2_dma(channel, type);

	if (!idma_is_valid(dma_ch))
		return;

	__raw_writel(0xF0000000, IPU_GPR); /* write one to clear */
	if (bufNum == 0) {
		if (idma_is_set(IPU_CHA_BUF0_RDY, dma_ch)) {
			__raw_writel(idma_mask(dma_ch),
					IPU_CHA_BUF0_RDY(dma_ch));
		}
	} else {
		if (idma_is_set(IPU_CHA_BUF1_RDY, dma_ch)) {
			__raw_writel(idma_mask(dma_ch),
					IPU_CHA_BUF1_RDY(dma_ch));
		}
	}
	__raw_writel(0x0, IPU_GPR); /* write one to set */
}

/*
 * This function disables a logical channel.
 *
 * @param       channel         Input parameter for the logical channel ID.
 *
 * @param       wait_for_stop   Flag to set whether to wait for channel end
 *                              of frame or return immediately.
 *
 * @return      This function returns 0 on success or negative error code on
 *              fail.
 */
int32_t ipu_disable_channel(ipu_channel_t channel)
{
	uint32_t reg;
	uint32_t in_dma;
	uint32_t out_dma;

	if ((g_channel_enable_mask & (1L << IPU_CHAN_ID(channel))) == 0) {
		debug("Channel already disabled %d\n",
			IPU_CHAN_ID(channel));
		return 0;
	}

	/* Get input and output dma channels */
	out_dma = channel_2_dma(channel, IPU_OUTPUT_BUFFER);
	in_dma = channel_2_dma(channel, IPU_VIDEO_IN_BUFFER);

	if ((idma_is_valid(in_dma) &&
		!idma_is_set(IDMAC_CHA_EN, in_dma))
		&& (idma_is_valid(out_dma) &&
		!idma_is_set(IDMAC_CHA_EN, out_dma)))
		return -EINVAL;

	if ((channel == MEM_BG_SYNC) || (channel == MEM_FG_SYNC) ||
	    (channel == MEM_DC_SYNC)) {
		ipu_dp_dc_disable(channel, 0);
	}

	/* Disable DMA channel(s) */
	if (idma_is_valid(in_dma)) {
		reg = __raw_readl(IDMAC_CHA_EN(in_dma));
		__raw_writel(reg & ~idma_mask(in_dma), IDMAC_CHA_EN(in_dma));
		__raw_writel(idma_mask(in_dma), IPU_CHA_CUR_BUF(in_dma));
	}
	if (idma_is_valid(out_dma)) {
		reg = __raw_readl(IDMAC_CHA_EN(out_dma));
		__raw_writel(reg & ~idma_mask(out_dma), IDMAC_CHA_EN(out_dma));
		__raw_writel(idma_mask(out_dma), IPU_CHA_CUR_BUF(out_dma));
	}

	g_channel_enable_mask &= ~(1L << IPU_CHAN_ID(channel));

	/* Set channel buffers NOT to be ready */
	if (idma_is_valid(in_dma)) {
		ipu_clear_buffer_ready(channel, IPU_VIDEO_IN_BUFFER, 0);
		ipu_clear_buffer_ready(channel, IPU_VIDEO_IN_BUFFER, 1);
	}
	if (idma_is_valid(out_dma)) {
		ipu_clear_buffer_ready(channel, IPU_OUTPUT_BUFFER, 0);
		ipu_clear_buffer_ready(channel, IPU_OUTPUT_BUFFER, 1);
	}

	return 0;
}

uint32_t bytes_per_pixel(uint32_t fmt)
{
	switch (fmt) {
	case IPU_PIX_FMT_GENERIC:	/*generic data */
	case IPU_PIX_FMT_RGB332:
	case IPU_PIX_FMT_YUV420P:
	case IPU_PIX_FMT_YUV422P:
		return 1;
		break;
	case IPU_PIX_FMT_RGB565:
	case IPU_PIX_FMT_YUYV:
	case IPU_PIX_FMT_UYVY:
		return 2;
		break;
	case IPU_PIX_FMT_BGR24:
	case IPU_PIX_FMT_RGB24:
		return 3;
		break;
	case IPU_PIX_FMT_GENERIC_32:	/*generic data */
	case IPU_PIX_FMT_BGR32:
	case IPU_PIX_FMT_BGRA32:
	case IPU_PIX_FMT_RGB32:
	case IPU_PIX_FMT_RGBA32:
	case IPU_PIX_FMT_ABGR32:
		return 4;
		break;
	default:
		return 1;
		break;
	}
	return 0;
}

ipu_color_space_t format_to_colorspace(uint32_t fmt)
{
	switch (fmt) {
	case IPU_PIX_FMT_RGB666:
	case IPU_PIX_FMT_RGB565:
	case IPU_PIX_FMT_BGR24:
	case IPU_PIX_FMT_RGB24:
	case IPU_PIX_FMT_BGR32:
	case IPU_PIX_FMT_BGRA32:
	case IPU_PIX_FMT_RGB32:
	case IPU_PIX_FMT_RGBA32:
	case IPU_PIX_FMT_ABGR32:
	case IPU_PIX_FMT_LVDS666:
	case IPU_PIX_FMT_LVDS888:
		return RGB;
		break;

	default:
		return YCbCr;
		break;
	}
	return RGB;
}

/* should be removed when clk framework is availiable */
int ipu_set_ldb_clock(int rate)
{
	ldb_clk.rate = rate;

	return 0;
}

bool ipu_clk_enabled(void)
{
	return g_ipu_clk_enabled;
}
