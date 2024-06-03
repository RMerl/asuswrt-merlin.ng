// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009
 * Guennadi Liakhovetski, DENX Software Engineering, <lg@denx.de>
 * Copyright (C) 2011
 * HALE electronic GmbH, <helmut.raiger@hale.at>
 */
#include <common.h>
#include <malloc.h>
#include <video_fb.h>

#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <linux/errno.h>
#include <asm/io.h>

#include "videomodes.h"

/* this might need panel specific set-up as-well */
#define IF_CONF		0

/* -------------- controller specific stuff -------------- */

/* IPU DMA Controller channel definitions. */
enum ipu_channel {
	IDMAC_IC_0 = 0,		/* IC (encoding task) to memory */
	IDMAC_IC_1 = 1,		/* IC (viewfinder task) to memory */
	IDMAC_ADC_0 = 1,
	IDMAC_IC_2 = 2,
	IDMAC_ADC_1 = 2,
	IDMAC_IC_3 = 3,
	IDMAC_IC_4 = 4,
	IDMAC_IC_5 = 5,
	IDMAC_IC_6 = 6,
	IDMAC_IC_7 = 7,		/* IC (sensor data) to memory */
	IDMAC_IC_8 = 8,
	IDMAC_IC_9 = 9,
	IDMAC_IC_10 = 10,
	IDMAC_IC_11 = 11,
	IDMAC_IC_12 = 12,
	IDMAC_IC_13 = 13,
	IDMAC_SDC_0 = 14,	/* Background synchronous display data */
	IDMAC_SDC_1 = 15,	/* Foreground data (overlay) */
	IDMAC_SDC_2 = 16,
	IDMAC_SDC_3 = 17,
	IDMAC_ADC_2 = 18,
	IDMAC_ADC_3 = 19,
	IDMAC_ADC_4 = 20,
	IDMAC_ADC_5 = 21,
	IDMAC_ADC_6 = 22,
	IDMAC_ADC_7 = 23,
	IDMAC_PF_0 = 24,
	IDMAC_PF_1 = 25,
	IDMAC_PF_2 = 26,
	IDMAC_PF_3 = 27,
	IDMAC_PF_4 = 28,
	IDMAC_PF_5 = 29,
	IDMAC_PF_6 = 30,
	IDMAC_PF_7 = 31,
};

/* More formats can be copied from the Linux driver if needed */
enum pixel_fmt {
	/* 2 bytes */
	IPU_PIX_FMT_RGB565,
	IPU_PIX_FMT_RGB666,
	IPU_PIX_FMT_BGR666,
	/* 3 bytes */
	IPU_PIX_FMT_RGB24,
};

struct pixel_fmt_cfg {
	u32	b0;
	u32	b1;
	u32	b2;
	u32	acc;
};

static struct pixel_fmt_cfg fmt_cfg[] = {
	[IPU_PIX_FMT_RGB24] = {
		0x1600AAAA, 0x00E05555, 0x00070000, 3,
	},
	[IPU_PIX_FMT_RGB666] = {
		0x0005000F, 0x000B000F, 0x0011000F, 1,
	},
	[IPU_PIX_FMT_BGR666] = {
		0x0011000F, 0x000B000F, 0x0005000F, 1,
	},
	[IPU_PIX_FMT_RGB565] = {
		0x0004003F, 0x000A000F, 0x000F003F, 1,
	}
};

enum ipu_panel {
	IPU_PANEL_SHARP_TFT,
	IPU_PANEL_TFT,
};

/* IPU Common registers */
/* IPU_CONF and its bits already defined in imx-regs.h */
#define IPU_CHA_BUF0_RDY	(0x04 + IPU_BASE)
#define IPU_CHA_BUF1_RDY	(0x08 + IPU_BASE)
#define IPU_CHA_DB_MODE_SEL	(0x0C + IPU_BASE)
#define IPU_CHA_CUR_BUF		(0x10 + IPU_BASE)
#define IPU_FS_PROC_FLOW	(0x14 + IPU_BASE)
#define IPU_FS_DISP_FLOW	(0x18 + IPU_BASE)
#define IPU_TASKS_STAT		(0x1C + IPU_BASE)
#define IPU_IMA_ADDR		(0x20 + IPU_BASE)
#define IPU_IMA_DATA		(0x24 + IPU_BASE)
#define IPU_INT_CTRL_1		(0x28 + IPU_BASE)
#define IPU_INT_CTRL_2		(0x2C + IPU_BASE)
#define IPU_INT_CTRL_3		(0x30 + IPU_BASE)
#define IPU_INT_CTRL_4		(0x34 + IPU_BASE)
#define IPU_INT_CTRL_5		(0x38 + IPU_BASE)
#define IPU_INT_STAT_1		(0x3C + IPU_BASE)
#define IPU_INT_STAT_2		(0x40 + IPU_BASE)
#define IPU_INT_STAT_3		(0x44 + IPU_BASE)
#define IPU_INT_STAT_4		(0x48 + IPU_BASE)
#define IPU_INT_STAT_5		(0x4C + IPU_BASE)
#define IPU_BRK_CTRL_1		(0x50 + IPU_BASE)
#define IPU_BRK_CTRL_2		(0x54 + IPU_BASE)
#define IPU_BRK_STAT		(0x58 + IPU_BASE)
#define IPU_DIAGB_CTRL		(0x5C + IPU_BASE)

/* Image Converter Registers */
#define IC_CONF			(0x88 + IPU_BASE)
#define IC_PRP_ENC_RSC		(0x8C + IPU_BASE)
#define IC_PRP_VF_RSC		(0x90 + IPU_BASE)
#define IC_PP_RSC		(0x94 + IPU_BASE)
#define IC_CMBP_1		(0x98 + IPU_BASE)
#define IC_CMBP_2		(0x9C + IPU_BASE)
#define PF_CONF			(0xA0 + IPU_BASE)
#define IDMAC_CONF		(0xA4 + IPU_BASE)
#define IDMAC_CHA_EN		(0xA8 + IPU_BASE)
#define IDMAC_CHA_PRI		(0xAC + IPU_BASE)
#define IDMAC_CHA_BUSY		(0xB0 + IPU_BASE)

/* Image Converter Register bits */
#define IC_CONF_PRPENC_EN	0x00000001
#define IC_CONF_PRPENC_CSC1	0x00000002
#define IC_CONF_PRPENC_ROT_EN	0x00000004
#define IC_CONF_PRPVF_EN	0x00000100
#define IC_CONF_PRPVF_CSC1	0x00000200
#define IC_CONF_PRPVF_CSC2	0x00000400
#define IC_CONF_PRPVF_CMB	0x00000800
#define IC_CONF_PRPVF_ROT_EN	0x00001000
#define IC_CONF_PP_EN		0x00010000
#define IC_CONF_PP_CSC1		0x00020000
#define IC_CONF_PP_CSC2		0x00040000
#define IC_CONF_PP_CMB		0x00080000
#define IC_CONF_PP_ROT_EN	0x00100000
#define IC_CONF_IC_GLB_LOC_A	0x10000000
#define IC_CONF_KEY_COLOR_EN	0x20000000
#define IC_CONF_RWS_EN		0x40000000
#define IC_CONF_CSI_MEM_WR_EN	0x80000000

/* SDC Registers */
#define SDC_COM_CONF		(0xB4 + IPU_BASE)
#define SDC_GW_CTRL		(0xB8 + IPU_BASE)
#define SDC_FG_POS		(0xBC + IPU_BASE)
#define SDC_BG_POS		(0xC0 + IPU_BASE)
#define SDC_CUR_POS		(0xC4 + IPU_BASE)
#define SDC_PWM_CTRL		(0xC8 + IPU_BASE)
#define SDC_CUR_MAP		(0xCC + IPU_BASE)
#define SDC_HOR_CONF		(0xD0 + IPU_BASE)
#define SDC_VER_CONF		(0xD4 + IPU_BASE)
#define SDC_SHARP_CONF_1	(0xD8 + IPU_BASE)
#define SDC_SHARP_CONF_2	(0xDC + IPU_BASE)

/* Register bits */
#define SDC_COM_TFT_COLOR	0x00000001UL
#define SDC_COM_FG_EN		0x00000010UL
#define SDC_COM_GWSEL		0x00000020UL
#define SDC_COM_GLB_A		0x00000040UL
#define SDC_COM_KEY_COLOR_G	0x00000080UL
#define SDC_COM_BG_EN		0x00000200UL
#define SDC_COM_SHARP		0x00001000UL

#define SDC_V_SYNC_WIDTH_L	0x00000001UL

/* Display Interface registers */
#define DI_DISP_IF_CONF		(0x0124 + IPU_BASE)
#define DI_DISP_SIG_POL		(0x0128 + IPU_BASE)
#define DI_SER_DISP1_CONF	(0x012C + IPU_BASE)
#define DI_SER_DISP2_CONF	(0x0130 + IPU_BASE)
#define DI_HSP_CLK_PER		(0x0134 + IPU_BASE)
#define DI_DISP0_TIME_CONF_1	(0x0138 + IPU_BASE)
#define DI_DISP0_TIME_CONF_2	(0x013C + IPU_BASE)
#define DI_DISP0_TIME_CONF_3	(0x0140 + IPU_BASE)
#define DI_DISP1_TIME_CONF_1	(0x0144 + IPU_BASE)
#define DI_DISP1_TIME_CONF_2	(0x0148 + IPU_BASE)
#define DI_DISP1_TIME_CONF_3	(0x014C + IPU_BASE)
#define DI_DISP2_TIME_CONF_1	(0x0150 + IPU_BASE)
#define DI_DISP2_TIME_CONF_2	(0x0154 + IPU_BASE)
#define DI_DISP2_TIME_CONF_3	(0x0158 + IPU_BASE)
#define DI_DISP3_TIME_CONF	(0x015C + IPU_BASE)
#define DI_DISP0_DB0_MAP	(0x0160 + IPU_BASE)
#define DI_DISP0_DB1_MAP	(0x0164 + IPU_BASE)
#define DI_DISP0_DB2_MAP	(0x0168 + IPU_BASE)
#define DI_DISP0_CB0_MAP	(0x016C + IPU_BASE)
#define DI_DISP0_CB1_MAP	(0x0170 + IPU_BASE)
#define DI_DISP0_CB2_MAP	(0x0174 + IPU_BASE)
#define DI_DISP1_DB0_MAP	(0x0178 + IPU_BASE)
#define DI_DISP1_DB1_MAP	(0x017C + IPU_BASE)
#define DI_DISP1_DB2_MAP	(0x0180 + IPU_BASE)
#define DI_DISP1_CB0_MAP	(0x0184 + IPU_BASE)
#define DI_DISP1_CB1_MAP	(0x0188 + IPU_BASE)
#define DI_DISP1_CB2_MAP	(0x018C + IPU_BASE)
#define DI_DISP2_DB0_MAP	(0x0190 + IPU_BASE)
#define DI_DISP2_DB1_MAP	(0x0194 + IPU_BASE)
#define DI_DISP2_DB2_MAP	(0x0198 + IPU_BASE)
#define DI_DISP2_CB0_MAP	(0x019C + IPU_BASE)
#define DI_DISP2_CB1_MAP	(0x01A0 + IPU_BASE)
#define DI_DISP2_CB2_MAP	(0x01A4 + IPU_BASE)
#define DI_DISP3_B0_MAP		(0x01A8 + IPU_BASE)
#define DI_DISP3_B1_MAP		(0x01AC + IPU_BASE)
#define DI_DISP3_B2_MAP		(0x01B0 + IPU_BASE)
#define DI_DISP_ACC_CC		(0x01B4 + IPU_BASE)
#define DI_DISP_LLA_CONF	(0x01B8 + IPU_BASE)
#define DI_DISP_LLA_DATA	(0x01BC + IPU_BASE)

/* DI_DISP_SIG_POL bits */
#define DI_D3_VSYNC_POL		(1 << 28)
#define DI_D3_HSYNC_POL		(1 << 27)
#define DI_D3_DRDY_SHARP_POL	(1 << 26)
#define DI_D3_CLK_POL		(1 << 25)
#define DI_D3_DATA_POL		(1 << 24)

/* DI_DISP_IF_CONF bits */
#define DI_D3_CLK_IDLE		(1 << 26)
#define DI_D3_CLK_SEL		(1 << 25)
#define DI_D3_DATAMSK		(1 << 24)

#define IOMUX_PADNUM_MASK	0x1ff
#define IOMUX_GPIONUM_SHIFT	9
#define IOMUX_GPIONUM_MASK	(0xff << IOMUX_GPIONUM_SHIFT)

#define IOMUX_PIN(gpionum, padnum) ((padnum) & IOMUX_PADNUM_MASK)

#define IOMUX_MODE_L(pin, mode) IOMUX_MODE(((pin) + 0xc) ^ 3, mode)

struct chan_param_mem_planar {
	/* Word 0 */
	u32	xv:10;
	u32	yv:10;
	u32	xb:12;

	u32	yb:12;
	u32	res1:2;
	u32	nsb:1;
	u32	lnpb:6;
	u32	ubo_l:11;

	u32	ubo_h:15;
	u32	vbo_l:17;

	u32	vbo_h:9;
	u32	res2:3;
	u32	fw:12;
	u32	fh_l:8;

	u32	fh_h:4;
	u32	res3:28;

	/* Word 1 */
	u32	eba0;

	u32	eba1;

	u32	bpp:3;
	u32	sl:14;
	u32	pfs:3;
	u32	bam:3;
	u32	res4:2;
	u32	npb:6;
	u32	res5:1;

	u32	sat:2;
	u32	res6:30;
} __attribute__ ((packed));

struct chan_param_mem_interleaved {
	/* Word 0 */
	u32	xv:10;
	u32	yv:10;
	u32	xb:12;

	u32	yb:12;
	u32	sce:1;
	u32	res1:1;
	u32	nsb:1;
	u32	lnpb:6;
	u32	sx:10;
	u32	sy_l:1;

	u32	sy_h:9;
	u32	ns:10;
	u32	sm:10;
	u32	sdx_l:3;

	u32	sdx_h:2;
	u32	sdy:5;
	u32	sdrx:1;
	u32	sdry:1;
	u32	sdr1:1;
	u32	res2:2;
	u32	fw:12;
	u32	fh_l:8;

	u32	fh_h:4;
	u32	res3:28;

	/* Word 1 */
	u32	eba0;

	u32	eba1;

	u32	bpp:3;
	u32	sl:14;
	u32	pfs:3;
	u32	bam:3;
	u32	res4:2;
	u32	npb:6;
	u32	res5:1;

	u32	sat:2;
	u32	scc:1;
	u32	ofs0:5;
	u32	ofs1:5;
	u32	ofs2:5;
	u32	ofs3:5;
	u32	wid0:3;
	u32	wid1:3;
	u32	wid2:3;

	u32	wid3:3;
	u32	dec_sel:1;
	u32	res6:28;
} __attribute__ ((packed));

union chan_param_mem {
	struct chan_param_mem_planar		pp;
	struct chan_param_mem_interleaved	ip;
};

/* graphics setup */
static GraphicDevice panel;
static struct ctfb_res_modes *mode;
static struct ctfb_res_modes var_mode;

/*
 * sdc_init_panel() - initialize a synchronous LCD panel.
 * @width:		width of panel in pixels.
 * @height:		height of panel in pixels.
 * @di_setup:	pixel format of the frame buffer
 * @di_panel:	either SHARP or normal TFT
 * @return:		0 on success or negative error code on failure.
 */
static int sdc_init_panel(u16 width, u16 height,
		enum pixel_fmt di_setup, enum ipu_panel di_panel)
{
	u32 reg, div;
	uint32_t old_conf;
	int clock;

	debug("%s(width=%d, height=%d)\n", __func__, width, height);

	/* Init clocking, the IPU receives its clock from the hsp divder */
	clock = mxc_get_clock(MXC_IPU_CLK);
	if (clock < 0)
		return -EACCES;

	/* Init panel size and blanking periods */
	reg = width + mode->left_margin + mode->right_margin - 1;
	if (reg > 1023) {
		printf("mx3fb: Display width too large, coerced to 1023!");
		reg = 1023;
	}
	reg = ((mode->hsync_len - 1) << 26) | (reg << 16);
	writel(reg, SDC_HOR_CONF);

	reg = height + mode->upper_margin + mode->lower_margin - 1;
	if (reg > 1023) {
		printf("mx3fb: Display height too large, coerced to 1023!");
		reg = 1023;
	}
	reg = ((mode->vsync_len - 1) << 26) | SDC_V_SYNC_WIDTH_L | (reg << 16);
	writel(reg, SDC_VER_CONF);

	switch (di_panel) {
	case IPU_PANEL_SHARP_TFT:
		writel(0x00FD0102L, SDC_SHARP_CONF_1);
		writel(0x00F500F4L, SDC_SHARP_CONF_2);
		writel(SDC_COM_SHARP | SDC_COM_TFT_COLOR, SDC_COM_CONF);
		/* TODO: probably IF_CONF must be adapted (see below)! */
		break;
	case IPU_PANEL_TFT:
		writel(SDC_COM_TFT_COLOR, SDC_COM_CONF);
		break;
	default:
		return -EINVAL;
	}

	/*
	 * Calculate divider: The fractional part is 4 bits so simply
	 * multiple by 2^4 to get it.
	 *
	 * Opposed to the kernel driver mode->pixclock is the time of one
	 * pixel in pico seconds, so:
	 *		pixel_clk = 1e12 / mode->pixclock
	 *		div = ipu_clk * 16 / pixel_clk
	 * leads to:
	 *		div = ipu_clk * 16 / (1e12 / mode->pixclock)
	 * or:
	 *		div = ipu_clk * 16 * mode->pixclock / 1e12
	 *
	 * To avoid integer overflows this is split into 2 shifts and
	 * one divide with sufficient accuracy:
	 *		16*1024*128*476837 =  0.9999996682e12
	 */
	div = ((clock/1024) * (mode->pixclock/128)) / 476837;
	debug("hsp_clk is %d, div=%d\n", clock, div);
	/* coerce to not less than 4.0, not more than 255.9375 */
	if (div < 0x40)
		div = 0x40;
	else if (div > 0xFFF)
		div = 0xFFF;
	/* DISP3_IF_CLK_DOWN_WR is half the divider value and 2 less
	 * fraction bits. Subtract 1 extra from DISP3_IF_CLK_DOWN_WR
	 * based on timing debug DISP3_IF_CLK_UP_WR is 0
	 */
	writel((((div / 8) - 1) << 22) | div, DI_DISP3_TIME_CONF);

	/* DI settings for display 3: clock idle (bit 26) during vsync */
	old_conf = readl(DI_DISP_IF_CONF) & 0x78FFFFFF;
	writel(old_conf | IF_CONF, DI_DISP_IF_CONF);

	/* only set display 3 polarity bits */
	old_conf = readl(DI_DISP_SIG_POL) & 0xE0FFFFFF;
	writel(old_conf | mode->sync, DI_DISP_SIG_POL);

	writel(fmt_cfg[di_setup].b0, DI_DISP3_B0_MAP);
	writel(fmt_cfg[di_setup].b1, DI_DISP3_B1_MAP);
	writel(fmt_cfg[di_setup].b2, DI_DISP3_B2_MAP);
	writel(readl(DI_DISP_ACC_CC) |
		  ((fmt_cfg[di_setup].acc - 1) << 12), DI_DISP_ACC_CC);

	debug("DI_DISP_IF_CONF = 0x%08X\n",	readl(DI_DISP_IF_CONF));
	debug("DI_DISP_SIG_POL = 0x%08X\n", readl(DI_DISP_SIG_POL));
	debug("DI_DISP3_TIME_CONF = 0x%08X\n", readl(DI_DISP3_TIME_CONF));
	debug("SDC_HOR_CONF = 0x%08X\n", readl(SDC_HOR_CONF));
	debug("SDC_VER_CONF = 0x%08X\n", readl(SDC_VER_CONF));

	return 0;
}

static void ipu_ch_param_set_size(union chan_param_mem *params,
				  uint pixelfmt, uint16_t width,
				  uint16_t height, uint16_t stride)
{
	debug("%s(pixelfmt=%d, width=%d, height=%d, stride=%d)\n",
			__func__, pixelfmt, width, height, stride);

	params->pp.fw		= width - 1;
	params->pp.fh_l		= height - 1;
	params->pp.fh_h		= (height - 1) >> 8;
	params->pp.sl		= stride - 1;

	/* See above, for further formats see the Linux driver */
	switch (pixelfmt) {
	case GDF_16BIT_565RGB:
		params->ip.bpp	= 2;
		params->ip.pfs	= 4;
		params->ip.npb	= 7;
		params->ip.sat	= 2;		/* SAT = 32-bit access */
		params->ip.ofs0	= 0;		/* Red bit offset */
		params->ip.ofs1	= 5;		/* Green bit offset */
		params->ip.ofs2	= 11;		/* Blue bit offset */
		params->ip.ofs3	= 16;		/* Alpha bit offset */
		params->ip.wid0	= 4;		/* Red bit width - 1 */
		params->ip.wid1	= 5;		/* Green bit width - 1 */
		params->ip.wid2	= 4;		/* Blue bit width - 1 */
		break;
	case GDF_32BIT_X888RGB:
		params->ip.bpp	= 1;		/* 24 BPP & RGB PFS */
		params->ip.pfs	= 4;
		params->ip.npb	= 7;
		params->ip.sat	= 2;		/* SAT = 32-bit access */
		params->ip.ofs0	= 16;		/* Red bit offset */
		params->ip.ofs1	= 8;		/* Green bit offset */
		params->ip.ofs2	= 0;		/* Blue bit offset */
		params->ip.ofs3	= 24;		/* Alpha bit offset */
		params->ip.wid0	= 7;		/* Red bit width - 1 */
		params->ip.wid1	= 7;		/* Green bit width - 1 */
		params->ip.wid2	= 7;		/* Blue bit width - 1 */
		break;
	default:
		printf("mx3fb: Pixel format not supported!\n");
		break;
	}

	params->pp.nsb = 1;
}

static void ipu_ch_param_set_buffer(union chan_param_mem *params,
				    void *buf0, void *buf1)
{
	params->pp.eba0 = (u32)buf0;
	params->pp.eba1 = (u32)buf1;
}

static void ipu_write_param_mem(uint32_t addr, uint32_t *data,
				uint32_t num_words)
{
	for (; num_words > 0; num_words--) {
		writel(addr, IPU_IMA_ADDR);
		writel(*data++, IPU_IMA_DATA);
		addr++;
		if ((addr & 0x7) == 5) {
			addr &= ~0x7;	/* set to word 0 */
			addr += 8;	/* increment to next row */
		}
	}
}

static uint32_t dma_param_addr(enum ipu_channel channel)
{
	/* Channel Parameter Memory */
	return 0x10000 | (channel << 4);
}

static void ipu_init_channel_buffer(enum ipu_channel channel, void *fbmem)
{
	union chan_param_mem params = {};
	uint32_t reg;
	uint32_t stride_bytes;

	stride_bytes = (panel.plnSizeX * panel.gdfBytesPP + 3) & ~3;

	debug("%s(channel=%d, fbmem=%p)\n", __func__, channel, fbmem);

	/* Build parameter memory data for DMA channel */
	ipu_ch_param_set_size(&params, panel.gdfIndex,
			      panel.plnSizeX, panel.plnSizeY, stride_bytes);
	ipu_ch_param_set_buffer(&params, fbmem, NULL);
	params.pp.bam = 0;
	/* Some channels (rotation) have restriction on burst length */

	switch (channel) {
	case IDMAC_SDC_0:
		/* In original code only IPU_PIX_FMT_RGB565 was setting burst */
		params.pp.npb = 16 - 1;
		break;
	default:
		break;
	}

	ipu_write_param_mem(dma_param_addr(channel), (uint32_t *)&params, 10);

	/* Disable double-buffering */
	reg = readl(IPU_CHA_DB_MODE_SEL);
	reg &= ~(1UL << channel);
	writel(reg, IPU_CHA_DB_MODE_SEL);
}

static void ipu_channel_set_priority(enum ipu_channel channel,
				     int prio)
{
	u32 reg = readl(IDMAC_CHA_PRI);

	if (prio)
		reg |= 1UL << channel;
	else
		reg &= ~(1UL << channel);

	writel(reg, IDMAC_CHA_PRI);
}

/*
 * ipu_enable_channel() - enable an IPU channel.
 * @channel:	channel ID.
 * @return:	0 on success or negative error code on failure.
 */
static int ipu_enable_channel(enum ipu_channel channel)
{
	uint32_t reg;

	/* Reset to buffer 0 */
	writel(1UL << channel, IPU_CHA_CUR_BUF);

	switch (channel) {
	case IDMAC_SDC_0:
		ipu_channel_set_priority(channel, 1);
		break;
	default:
		break;
	}

	reg = readl(IDMAC_CHA_EN);
	writel(reg | (1UL << channel), IDMAC_CHA_EN);

	return 0;
}

static int ipu_update_channel_buffer(enum ipu_channel channel, void *buf)
{
	uint32_t reg;

	reg = readl(IPU_CHA_BUF0_RDY);
	if (reg & (1UL << channel))
		return -EACCES;

	/* 44.3.3.1.9 - Row Number 1 (WORD1, offset 0) */
	writel(dma_param_addr(channel) + 0x0008UL, IPU_IMA_ADDR);
	writel((u32)buf, IPU_IMA_DATA);

	return 0;
}

static int idmac_tx_submit(enum ipu_channel channel, void *buf)
{
	int ret;

	ipu_init_channel_buffer(channel, buf);


	/* ipu_idmac.c::ipu_submit_channel_buffers() */
	ret = ipu_update_channel_buffer(channel, buf);
	if (ret < 0)
		return ret;

	/* ipu_idmac.c::ipu_select_buffer() */
	/* Mark buffer 0 as ready. */
	writel(1UL << channel, IPU_CHA_BUF0_RDY);


	ret = ipu_enable_channel(channel);
	return ret;
}

static void sdc_enable_channel(void *fbmem)
{
	int ret;
	u32 reg;

	ret = idmac_tx_submit(IDMAC_SDC_0, fbmem);

	/* mx3fb.c::sdc_fb_init() */
	if (ret >= 0) {
		reg = readl(SDC_COM_CONF);
		writel(reg | SDC_COM_BG_EN, SDC_COM_CONF);
	}

	/*
	 * Attention! Without this msleep the channel keeps generating
	 * interrupts. Next sdc_set_brightness() is going to be called
	 * from mx3fb_blank().
	 */
	udelay(2000);
}

/*
 * mx3fb_set_par() - set framebuffer parameters and change the operating mode.
 * @return:	0 on success or negative error code on failure.
 *  TODO: currently only 666 and TFT as DI setup supported
 */
static int mx3fb_set_par(void)
{
	int ret;

	ret = sdc_init_panel(panel.plnSizeX, panel.plnSizeY,
			IPU_PIX_FMT_RGB666, IPU_PANEL_TFT);
	if (ret < 0)
		return ret;

	writel((mode->left_margin << 16) | mode->upper_margin, SDC_BG_POS);

	return 0;
}

static void ll_disp3_enable(void *base)
{
	u32 reg;

	debug("%s(base=0x%x)\n", __func__, (u32) base);
	/* pcm037.c::mxc_board_init() */

	/* Display Interface #3 */
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD0, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD1, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD2, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD3, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD4, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD5, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD6, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD7, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD8, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD9, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD10, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD11, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD12, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD13, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD14, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD15, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD16, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD17, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_VSYNC3, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_HSYNC, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_FPSHIFT, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_DRDY0, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_D3_REV, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_CONTRAST, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_D3_SPL, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_D3_CLS, MUX_CTL_FUNC));


	/* ipu_idmac.c::ipu_probe() */

	/* Start the clock */
	__REG(CCM_CGR1) = __REG(CCM_CGR1) | (3 << 22);


	/* ipu_idmac.c::ipu_idmac_init() */

	/* Service request counter to maximum - shouldn't be needed */
	writel(0x00000070, IDMAC_CONF);


	/* ipu_idmac.c::ipu_init_channel() */

	/* Enable IPU sub modules */
	reg = readl(IPU_CONF) | IPU_CONF_SDC_EN | IPU_CONF_DI_EN;
	writel(reg, IPU_CONF);


	/* mx3fb.c::init_fb_chan() */

	/* set Display Interface clock period */
	writel(0x00100010L, DI_HSP_CLK_PER);
	/* Might need to trigger HSP clock change - see 44.3.3.8.5 */


	/* mx3fb.c::sdc_set_brightness() */

	/* This might be board-specific */
	writel(0x03000000UL | 255 << 16, SDC_PWM_CTRL);


	/* mx3fb.c::sdc_set_global_alpha() */

	/* Use global - not per-pixel - Alpha-blending */
	reg = readl(SDC_GW_CTRL) & 0x00FFFFFFL;
	writel(reg | ((uint32_t) 0xff << 24), SDC_GW_CTRL);

	reg = readl(SDC_COM_CONF);
	writel(reg | SDC_COM_GLB_A, SDC_COM_CONF);


	/* mx3fb.c::sdc_set_color_key() */

	/* Disable colour-keying for background */
	reg = readl(SDC_COM_CONF) &
		~(SDC_COM_GWSEL | SDC_COM_KEY_COLOR_G);
	writel(reg, SDC_COM_CONF);


	mx3fb_set_par();

	sdc_enable_channel(base);

	/*
	 * Linux driver calls sdc_set_brightness() here again,
	 * once is enough for us
	 */
	debug("%s() done\n", __func__);
}

/* ------------------------ public part ------------------- */
ulong calc_fbsize(void)
{
	return panel.plnSizeX * panel.plnSizeY * panel.gdfBytesPP;
}

/*
 * The current implementation is only tested for GDF_16BIT_565RGB!
 * It was switched from the original CONFIG_LCD setup to CONFIG_VIDEO,
 * because the lcd code seemed loaded with color table stuff, that
 * does not relate to most modern TFTs. cfb_console.c looks more
 * straight forward.
 * This is the environment setting for the original setup
 *	"unknown=video=ctfb:x:240,y:320,depth:16,mode:0,pclk:185925,le:9,ri:17,
 *		up:7,lo:10,hs:1,vs:1,sync:100663296,vmode:0"
 *	"videomode=unknown"
 *
 * Settings for VBEST VGG322403 display:
 *	"videomode=video=ctfb:x:320,y:240,depth:16,mode:0,pclk:156000,
 *		"le:20,ri:68,up:7,lo:29,hs:30,vs:3,sync:100663296,vmode:0"
 *
 * Settings for COM57H5M10XRC display:
 *	"videomode=video=ctfb:x:640,y:480,depth:16,mode:0,pclk:40000,
 *		"le:120,ri:40,up:35,lo:10,hs:30,vs:3,sync:100663296,vmode:0"
 */
void *video_hw_init(void)
{
	char *penv;
	u32 memsize;
	unsigned long t1, hsynch, vsynch;
	int bits_per_pixel, i, tmp, videomode;

	tmp = 0;

	puts("Video: ");

	videomode = CONFIG_SYS_DEFAULT_VIDEO_MODE;
	/* get video mode via environment */
	penv = env_get("videomode");
	if (penv) {
		/* decide if it is a string */
		if (penv[0] <= '9') {
			videomode = (int) simple_strtoul(penv, NULL, 16);
			tmp = 1;
		}
	} else {
		tmp = 1;
	}
	if (tmp) {
		/* parameter are vesa modes */
		/* search params */
		for (i = 0; i < VESA_MODES_COUNT; i++) {
			if (vesa_modes[i].vesanr == videomode)
				break;
		}
		if (i == VESA_MODES_COUNT) {
			printf("No VESA Mode found, switching to mode 0x%x ",
					CONFIG_SYS_DEFAULT_VIDEO_MODE);
			i = 0;
		}
		mode = (struct ctfb_res_modes *)
				&res_mode_init[vesa_modes[i].resindex];
		bits_per_pixel = vesa_modes[i].bits_per_pixel;
	} else {
		mode = (struct ctfb_res_modes *) &var_mode;
		bits_per_pixel = video_get_params(mode, penv);
	}

	/* calculate hsynch and vsynch freq (info only) */
	t1 = (mode->left_margin + mode->xres +
	      mode->right_margin + mode->hsync_len) / 8;
	t1 *= 8;
	t1 *= mode->pixclock;
	t1 /= 1000;
	hsynch = 1000000000L / t1;
	t1 *= (mode->upper_margin + mode->yres +
	       mode->lower_margin + mode->vsync_len);
	t1 /= 1000;
	vsynch = 1000000000L / t1;

	/* fill in Graphic device struct */
	sprintf(panel.modeIdent, "%dx%dx%d %ldkHz %ldHz",
			mode->xres, mode->yres,
			bits_per_pixel, (hsynch / 1000), (vsynch / 1000));
	printf("%s\n", panel.modeIdent);
	panel.winSizeX = mode->xres;
	panel.winSizeY = mode->yres;
	panel.plnSizeX = mode->xres;
	panel.plnSizeY = mode->yres;

	switch (bits_per_pixel) {
	case 24:
		panel.gdfBytesPP = 4;
		panel.gdfIndex = GDF_32BIT_X888RGB;
		break;
	case 16:
		panel.gdfBytesPP = 2;
		panel.gdfIndex = GDF_16BIT_565RGB;
		break;
	default:
		panel.gdfBytesPP = 1;
		panel.gdfIndex = GDF__8BIT_INDEX;
		break;
	}

	/* set up Hardware */
	memsize = calc_fbsize();

	debug("%s() allocating %d bytes\n", __func__, memsize);

	/* fill in missing Graphic device struct */
	panel.frameAdrs = (u32) malloc(memsize);
	if (panel.frameAdrs == 0) {
		printf("%s() malloc(%d) failed\n", __func__, memsize);
		return 0;
	}
	panel.memSize = memsize;

	ll_disp3_enable((void *) panel.frameAdrs);
	memset((void *) panel.frameAdrs, 0, memsize);

	debug("%s() done, framebuffer at 0x%x, size=%d cleared\n",
			__func__, panel.frameAdrs, memsize);

	return (void *) &panel;
}
