// SPDX-License-Identifier: GPL-2.0
/*
 * Amlogic Meson Video Processing Unit driver
 *
 * Copyright (c) 2018 BayLibre, SAS.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#define DEBUG

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <linux/bitops.h>

#include "meson_vpu.h"

/* HHI Registers */
#define HHI_VDAC_CNTL0		0x2F4 /* 0xbd offset in data sheet */
#define HHI_VDAC_CNTL0_G12A	0x2EC /* 0xbd offset in data sheet */
#define HHI_VDAC_CNTL1		0x2F8 /* 0xbe offset in data sheet */
#define HHI_VDAC_CNTL1_G12A	0x2F0 /* 0xbe offset in data sheet */
#define HHI_HDMI_PHY_CNTL0	0x3a0 /* 0xe8 offset in data sheet */

/* OSDx_CTRL_STAT2 */
#define OSD_REPLACE_EN		BIT(14)
#define OSD_REPLACE_SHIFT	6

void meson_vpp_setup_mux(struct meson_vpu_priv *priv, unsigned int mux)
{
	writel(mux, priv->io_base + _REG(VPU_VIU_VENC_MUX_CTRL));
}

static unsigned int vpp_filter_coefs_4point_bspline[] = {
	0x15561500, 0x14561600, 0x13561700, 0x12561800,
	0x11551a00, 0x11541b00, 0x10541c00, 0x0f541d00,
	0x0f531e00, 0x0e531f00, 0x0d522100, 0x0c522200,
	0x0b522300, 0x0b512400, 0x0a502600, 0x0a4f2700,
	0x094e2900, 0x084e2a00, 0x084d2b00, 0x074c2c01,
	0x074b2d01, 0x064a2f01, 0x06493001, 0x05483201,
	0x05473301, 0x05463401, 0x04453601, 0x04433702,
	0x04423802, 0x03413a02, 0x03403b02, 0x033f3c02,
	0x033d3d03
};

static void meson_vpp_write_scaling_filter_coefs(struct meson_vpu_priv *priv,
						 const unsigned int *coefs,
						 bool is_horizontal)
{
	int i;

	writel(is_horizontal ? VPP_SCALE_HORIZONTAL_COEF : 0,
	       priv->io_base + _REG(VPP_OSD_SCALE_COEF_IDX));
	for (i = 0; i < 33; i++)
		writel(coefs[i],
		       priv->io_base + _REG(VPP_OSD_SCALE_COEF));
}

static const u32 vpp_filter_coefs_bicubic[] = {
	0x00800000, 0x007f0100, 0xff7f0200, 0xfe7f0300,
	0xfd7e0500, 0xfc7e0600, 0xfb7d0800, 0xfb7c0900,
	0xfa7b0b00, 0xfa7a0dff, 0xf9790fff, 0xf97711ff,
	0xf87613ff, 0xf87416fe, 0xf87218fe, 0xf8701afe,
	0xf76f1dfd, 0xf76d1ffd, 0xf76b21fd, 0xf76824fd,
	0xf76627fc, 0xf76429fc, 0xf7612cfc, 0xf75f2ffb,
	0xf75d31fb, 0xf75a34fb, 0xf75837fa, 0xf7553afa,
	0xf8523cfa, 0xf8503ff9, 0xf84d42f9, 0xf84a45f9,
	0xf84848f8
};

static void meson_vpp_write_vd_scaling_filter_coefs(struct meson_vpu_priv *priv,
						    const unsigned int *coefs,
						    bool is_horizontal)
{
	int i;

	writel(is_horizontal ? VPP_SCALE_HORIZONTAL_COEF : 0,
	       priv->io_base + _REG(VPP_SCALE_COEF_IDX));
	for (i = 0; i < 33; i++)
		writel(coefs[i],
		       priv->io_base + _REG(VPP_SCALE_COEF));
}

/* OSD csc defines */

enum viu_matrix_sel_e {
	VIU_MATRIX_OSD_EOTF = 0,
	VIU_MATRIX_OSD,
};

enum viu_lut_sel_e {
	VIU_LUT_OSD_EOTF = 0,
	VIU_LUT_OSD_OETF,
};

#define COEFF_NORM(a) ((int)((((a) * 2048.0) + 1) / 2))
#define MATRIX_5X3_COEF_SIZE 24

#define EOTF_COEFF_NORM(a) ((int)((((a) * 4096.0) + 1) / 2))
#define EOTF_COEFF_SIZE 10
#define EOTF_COEFF_RIGHTSHIFT 1

static int RGB709_to_YUV709l_coeff[MATRIX_5X3_COEF_SIZE] = {
	0, 0, 0, /* pre offset */
	COEFF_NORM(0.181873),	COEFF_NORM(0.611831),	COEFF_NORM(0.061765),
	COEFF_NORM(-0.100251),	COEFF_NORM(-0.337249),	COEFF_NORM(0.437500),
	COEFF_NORM(0.437500),	COEFF_NORM(-0.397384),	COEFF_NORM(-0.040116),
	0, 0, 0, /* 10'/11'/12' */
	0, 0, 0, /* 20'/21'/22' */
	64, 512, 512, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

/*  eotf matrix: bypass */
static int eotf_bypass_coeff[EOTF_COEFF_SIZE] = {
	EOTF_COEFF_NORM(1.0),	EOTF_COEFF_NORM(0.0),	EOTF_COEFF_NORM(0.0),
	EOTF_COEFF_NORM(0.0),	EOTF_COEFF_NORM(1.0),	EOTF_COEFF_NORM(0.0),
	EOTF_COEFF_NORM(0.0),	EOTF_COEFF_NORM(0.0),	EOTF_COEFF_NORM(1.0),
	EOTF_COEFF_RIGHTSHIFT /* right shift */
};

static void meson_viu_set_g12a_osd1_matrix(struct meson_vpu_priv *priv,
					   int *m, bool csc_on)
{
	/* VPP WRAP OSD1 matrix */
	writel(((m[0] & 0xfff) << 16) | (m[1] & 0xfff),
	       priv->io_base + _REG(VPP_WRAP_OSD1_MATRIX_PRE_OFFSET0_1));
	writel(m[2] & 0xfff,
	       priv->io_base + _REG(VPP_WRAP_OSD1_MATRIX_PRE_OFFSET2));
	writel(((m[3] & 0x1fff) << 16) | (m[4] & 0x1fff),
	       priv->io_base + _REG(VPP_WRAP_OSD1_MATRIX_COEF00_01));
	writel(((m[5] & 0x1fff) << 16) | (m[6] & 0x1fff),
	       priv->io_base + _REG(VPP_WRAP_OSD1_MATRIX_COEF02_10));
	writel(((m[7] & 0x1fff) << 16) | (m[8] & 0x1fff),
	       priv->io_base + _REG(VPP_WRAP_OSD1_MATRIX_COEF11_12));
	writel(((m[9] & 0x1fff) << 16) | (m[10] & 0x1fff),
	       priv->io_base + _REG(VPP_WRAP_OSD1_MATRIX_COEF20_21));
	writel((m[11] & 0x1fff) << 16,
	       priv->io_base +	_REG(VPP_WRAP_OSD1_MATRIX_COEF22));

	writel(((m[18] & 0xfff) << 16) | (m[19] & 0xfff),
	       priv->io_base + _REG(VPP_WRAP_OSD1_MATRIX_OFFSET0_1));
	writel(m[20] & 0xfff,
	       priv->io_base + _REG(VPP_WRAP_OSD1_MATRIX_OFFSET2));

	writel_bits(BIT(0), csc_on ? BIT(0) : 0,
		    priv->io_base + _REG(VPP_WRAP_OSD1_MATRIX_EN_CTRL));
}

static void meson_viu_set_osd_matrix(struct meson_vpu_priv *priv,
				     enum viu_matrix_sel_e m_select,
				     int *m, bool csc_on)
{
	if (m_select == VIU_MATRIX_OSD) {
		/* osd matrix, VIU_MATRIX_0 */
		writel(((m[0] & 0xfff) << 16) | (m[1] & 0xfff),
		       priv->io_base + _REG(VIU_OSD1_MATRIX_PRE_OFFSET0_1));
		writel(m[2] & 0xfff,
		       priv->io_base + _REG(VIU_OSD1_MATRIX_PRE_OFFSET2));
		writel(((m[3] & 0x1fff) << 16) | (m[4] & 0x1fff),
		       priv->io_base + _REG(VIU_OSD1_MATRIX_COEF00_01));
		writel(((m[5] & 0x1fff) << 16) | (m[6] & 0x1fff),
		       priv->io_base + _REG(VIU_OSD1_MATRIX_COEF02_10));
		writel(((m[7] & 0x1fff) << 16) | (m[8] & 0x1fff),
		       priv->io_base + _REG(VIU_OSD1_MATRIX_COEF11_12));
		writel(((m[9] & 0x1fff) << 16) | (m[10] & 0x1fff),
		       priv->io_base + _REG(VIU_OSD1_MATRIX_COEF20_21));

		if (m[21]) {
			writel(((m[11] & 0x1fff) << 16) | (m[12] & 0x1fff),
			       priv->io_base +
					_REG(VIU_OSD1_MATRIX_COEF22_30));
			writel(((m[13] & 0x1fff) << 16) | (m[14] & 0x1fff),
			       priv->io_base +
					_REG(VIU_OSD1_MATRIX_COEF31_32));
			writel(((m[15] & 0x1fff) << 16) | (m[16] & 0x1fff),
			       priv->io_base +
					_REG(VIU_OSD1_MATRIX_COEF40_41));
			writel(m[17] & 0x1fff, priv->io_base +
			       _REG(VIU_OSD1_MATRIX_COLMOD_COEF42));
		} else {
			writel((m[11] & 0x1fff) << 16, priv->io_base +
			       _REG(VIU_OSD1_MATRIX_COEF22_30));
		}

		writel(((m[18] & 0xfff) << 16) | (m[19] & 0xfff),
		       priv->io_base + _REG(VIU_OSD1_MATRIX_OFFSET0_1));
		writel(m[20] & 0xfff,
		       priv->io_base + _REG(VIU_OSD1_MATRIX_OFFSET2));

		writel_bits(3 << 30, m[21] << 30,
			    priv->io_base +
			    _REG(VIU_OSD1_MATRIX_COLMOD_COEF42));
		writel_bits(7 << 16, m[22] << 16,
			    priv->io_base +
			    _REG(VIU_OSD1_MATRIX_COLMOD_COEF42));

		/* 23 reserved for clipping control */
		writel_bits(BIT(0), csc_on ? BIT(0) : 0,
			    priv->io_base + _REG(VIU_OSD1_MATRIX_CTRL));
		writel_bits(BIT(1), 0,
			    priv->io_base + _REG(VIU_OSD1_MATRIX_CTRL));
	} else if (m_select == VIU_MATRIX_OSD_EOTF) {
		int i;

		/* osd eotf matrix, VIU_MATRIX_OSD_EOTF */
		for (i = 0; i < 5; i++)
			writel(((m[i * 2] & 0x1fff) << 16) |
				(m[i * 2 + 1] & 0x1fff), priv->io_base +
				_REG(VIU_OSD1_EOTF_CTL + i + 1));

		writel_bits(BIT(30), csc_on ? BIT(30) : 0,
			    priv->io_base + _REG(VIU_OSD1_EOTF_CTL));
		writel_bits(BIT(31), csc_on ? BIT(31) : 0,
			    priv->io_base + _REG(VIU_OSD1_EOTF_CTL));
	}
}

#define OSD_EOTF_LUT_SIZE 33
#define OSD_OETF_LUT_SIZE 41

static void meson_viu_set_osd_lut(struct meson_vpu_priv *priv,
				  enum viu_lut_sel_e lut_sel,
				  unsigned int *r_map, unsigned int *g_map,
				  unsigned int *b_map,
				  bool csc_on)
{
	unsigned int addr_port;
	unsigned int data_port;
	unsigned int ctrl_port;
	int i;

	if (lut_sel == VIU_LUT_OSD_EOTF) {
		addr_port = VIU_OSD1_EOTF_LUT_ADDR_PORT;
		data_port = VIU_OSD1_EOTF_LUT_DATA_PORT;
		ctrl_port = VIU_OSD1_EOTF_CTL;
	} else if (lut_sel == VIU_LUT_OSD_OETF) {
		addr_port = VIU_OSD1_OETF_LUT_ADDR_PORT;
		data_port = VIU_OSD1_OETF_LUT_DATA_PORT;
		ctrl_port = VIU_OSD1_OETF_CTL;
	} else {
		return;
	}

	if (lut_sel == VIU_LUT_OSD_OETF) {
		writel(0, priv->io_base + _REG(addr_port));

		for (i = 0; i < 20; i++)
			writel(r_map[i * 2] | (r_map[i * 2 + 1] << 16),
			       priv->io_base + _REG(data_port));

		writel(r_map[OSD_OETF_LUT_SIZE - 1] | (g_map[0] << 16),
		       priv->io_base + _REG(data_port));

		for (i = 0; i < 20; i++)
			writel(g_map[i * 2 + 1] | (g_map[i * 2 + 2] << 16),
			       priv->io_base + _REG(data_port));

		for (i = 0; i < 20; i++)
			writel(b_map[i * 2] | (b_map[i * 2 + 1] << 16),
			       priv->io_base + _REG(data_port));

		writel(b_map[OSD_OETF_LUT_SIZE - 1],
		       priv->io_base + _REG(data_port));

		if (csc_on)
			writel_bits(0x7 << 29, 7 << 29,
				    priv->io_base + _REG(ctrl_port));
		else
			writel_bits(0x7 << 29, 0,
				    priv->io_base + _REG(ctrl_port));
	} else if (lut_sel == VIU_LUT_OSD_EOTF) {
		writel(0, priv->io_base + _REG(addr_port));

		for (i = 0; i < 20; i++)
			writel(r_map[i * 2] | (r_map[i * 2 + 1] << 16),
			       priv->io_base + _REG(data_port));

		writel(r_map[OSD_EOTF_LUT_SIZE - 1] | (g_map[0] << 16),
		       priv->io_base + _REG(data_port));

		for (i = 0; i < 20; i++)
			writel(g_map[i * 2 + 1] | (g_map[i * 2 + 2] << 16),
			       priv->io_base + _REG(data_port));

		for (i = 0; i < 20; i++)
			writel(b_map[i * 2] | (b_map[i * 2 + 1] << 16),
			       priv->io_base + _REG(data_port));

		writel(b_map[OSD_EOTF_LUT_SIZE - 1],
		       priv->io_base + _REG(data_port));

		if (csc_on)
			writel_bits(7 << 27, 7 << 27,
				    priv->io_base + _REG(ctrl_port));
		else
			writel_bits(7 << 27, 0,
				    priv->io_base + _REG(ctrl_port));

		writel_bits(BIT(31), BIT(31),
			    priv->io_base + _REG(ctrl_port));
	}
}

/* eotf lut: linear */
static unsigned int eotf_33_linear_mapping[OSD_EOTF_LUT_SIZE] = {
	0x0000,	0x0200,	0x0400, 0x0600,
	0x0800, 0x0a00, 0x0c00, 0x0e00,
	0x1000, 0x1200, 0x1400, 0x1600,
	0x1800, 0x1a00, 0x1c00, 0x1e00,
	0x2000, 0x2200, 0x2400, 0x2600,
	0x2800, 0x2a00, 0x2c00, 0x2e00,
	0x3000, 0x3200, 0x3400, 0x3600,
	0x3800, 0x3a00, 0x3c00, 0x3e00,
	0x4000
};

/* osd oetf lut: linear */
static unsigned int oetf_41_linear_mapping[OSD_OETF_LUT_SIZE] = {
	0, 0, 0, 0,
	0, 32, 64, 96,
	128, 160, 196, 224,
	256, 288, 320, 352,
	384, 416, 448, 480,
	512, 544, 576, 608,
	640, 672, 704, 736,
	768, 800, 832, 864,
	896, 928, 960, 992,
	1023, 1023, 1023, 1023,
	1023
};

static void meson_viu_load_matrix(struct meson_vpu_priv *priv)
{
	/* eotf lut bypass */
	meson_viu_set_osd_lut(priv, VIU_LUT_OSD_EOTF,
			      eotf_33_linear_mapping, /* R */
			      eotf_33_linear_mapping, /* G */
			      eotf_33_linear_mapping, /* B */
			      false);

	/* eotf matrix bypass */
	meson_viu_set_osd_matrix(priv, VIU_MATRIX_OSD_EOTF,
				 eotf_bypass_coeff,
				 false);

	/* oetf lut bypass */
	meson_viu_set_osd_lut(priv, VIU_LUT_OSD_OETF,
			      oetf_41_linear_mapping, /* R */
			      oetf_41_linear_mapping, /* G */
			      oetf_41_linear_mapping, /* B */
			      false);

	/* osd matrix RGB709 to YUV709 limit */
	meson_viu_set_osd_matrix(priv, VIU_MATRIX_OSD,
				 RGB709_to_YUV709l_coeff,
				 true);
}

static inline uint32_t meson_viu_osd_burst_length_reg(uint32_t length)
{
	u32 val = (((length & 0x80) % 24) / 12);

	return (((val & 0x3) << 10) | (((val & 0x4) >> 2) << 31));
}

void meson_vpu_init(struct udevice *dev)
{
	struct meson_vpu_priv *priv = dev_get_priv(dev);
	u32 reg;

	/*
	 * Slave dc0 and dc5 connected to master port 1.
	 * By default other slaves are connected to master port 0.
	 */
	reg = VPU_RDARB_SLAVE_TO_MASTER_PORT(0, 1) |
		VPU_RDARB_SLAVE_TO_MASTER_PORT(5, 1);
	writel(reg, priv->io_base + _REG(VPU_RDARB_MODE_L1C1));

	/* Slave dc0 connected to master port 1 */
	reg = VPU_RDARB_SLAVE_TO_MASTER_PORT(0, 1);
	writel(reg, priv->io_base + _REG(VPU_RDARB_MODE_L1C2));

	/* Slave dc4 and dc7 connected to master port 1 */
	reg = VPU_RDARB_SLAVE_TO_MASTER_PORT(4, 1) |
		VPU_RDARB_SLAVE_TO_MASTER_PORT(7, 1);
	writel(reg, priv->io_base + _REG(VPU_RDARB_MODE_L2C1));

	/* Slave dc1 connected to master port 1 */
	reg = VPU_RDARB_SLAVE_TO_MASTER_PORT(1, 1);
	writel(reg, priv->io_base + _REG(VPU_WRARB_MODE_L2C1));

	/* Disable CVBS VDAC */
	if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_G12A)) {
		hhi_write(HHI_VDAC_CNTL0_G12A, 0);
		hhi_write(HHI_VDAC_CNTL1_G12A, 8);
	} else {
		hhi_write(HHI_VDAC_CNTL0, 0);
		hhi_write(HHI_VDAC_CNTL1, 8);
	}

	/* Power Down Dacs */
	writel(0xff, priv->io_base + _REG(VENC_VDAC_SETTING));

	/* Disable HDMI PHY */
	hhi_write(HHI_HDMI_PHY_CNTL0, 0);

	/* Disable HDMI */
	writel_bits(VPU_HDMI_ENCI_DATA_TO_HDMI |
		    VPU_HDMI_ENCP_DATA_TO_HDMI, 0,
		    priv->io_base + _REG(VPU_HDMI_SETTING));

	/* Disable all encoders */
	writel(0, priv->io_base + _REG(ENCI_VIDEO_EN));
	writel(0, priv->io_base + _REG(ENCP_VIDEO_EN));
	writel(0, priv->io_base + _REG(ENCL_VIDEO_EN));

	/* Disable VSync IRQ */
	writel(0, priv->io_base + _REG(VENC_INTCTRL));

	/* set dummy data default YUV black */
	if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_GXL)) {
		writel(0x108080, priv->io_base + _REG(VPP_DUMMY_DATA1));
	} else if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_GXM)) {
		writel_bits(0xff << 16, 0xff << 16,
			    priv->io_base + _REG(VIU_MISC_CTRL1));
		writel(VPP_PPS_DUMMY_DATA_MODE,
		       priv->io_base + _REG(VPP_DOLBY_CTRL));
		writel(0x1020080,
		       priv->io_base + _REG(VPP_DUMMY_DATA1));
	} else if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_G12A))
		writel(0xf, priv->io_base + _REG(DOLBY_PATH_CTRL));

	/* Initialize vpu fifo control registers */
	if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_G12A))
		writel(VPP_OFIFO_SIZE_DEFAULT,
		       priv->io_base + _REG(VPP_OFIFO_SIZE));
	else
		writel_bits(VPP_OFIFO_SIZE_MASK, 0x77f,
			    priv->io_base + _REG(VPP_OFIFO_SIZE));
	writel(VPP_POSTBLEND_HOLD_LINES(4) | VPP_PREBLEND_HOLD_LINES(4),
	       priv->io_base + _REG(VPP_HOLD_LINES));

	if (!meson_vpu_is_compatible(priv, VPU_COMPATIBLE_G12A)) {
		/* Turn off preblend */
		writel_bits(VPP_PREBLEND_ENABLE, 0,
			    priv->io_base + _REG(VPP_MISC));

		/* Turn off POSTBLEND */
		writel_bits(VPP_POSTBLEND_ENABLE, 0,
			    priv->io_base + _REG(VPP_MISC));

		/* Force all planes off */
		writel_bits(VPP_OSD1_POSTBLEND | VPP_OSD2_POSTBLEND |
			    VPP_VD1_POSTBLEND | VPP_VD2_POSTBLEND |
			    VPP_VD1_PREBLEND | VPP_VD2_PREBLEND, 0,
			    priv->io_base + _REG(VPP_MISC));

		/* Setup default VD settings */
		writel(4096,
		       priv->io_base + _REG(VPP_PREBLEND_VD1_H_START_END));
		writel(4096,
		       priv->io_base + _REG(VPP_BLEND_VD2_H_START_END));
	}

	/* Disable Scalers */
	writel(0, priv->io_base + _REG(VPP_OSD_SC_CTRL0));
	writel(0, priv->io_base + _REG(VPP_OSD_VSC_CTRL0));
	writel(0, priv->io_base + _REG(VPP_OSD_HSC_CTRL0));

	writel(VPP_VSC_BANK_LENGTH(4) | VPP_HSC_BANK_LENGTH(4) |
	       VPP_SC_VD_EN_ENABLE,
	       priv->io_base + _REG(VPP_SC_MISC));

	/* Enable minus black level for vadj1 */
	writel(VPP_MINUS_BLACK_LVL_VADJ1_ENABLE,
	       priv->io_base + _REG(VPP_VADJ_CTRL));

	/* Write in the proper filter coefficients. */
	meson_vpp_write_scaling_filter_coefs(priv,
				vpp_filter_coefs_4point_bspline, false);
	meson_vpp_write_scaling_filter_coefs(priv,
				vpp_filter_coefs_4point_bspline, true);

	/* Write the VD proper filter coefficients. */
	meson_vpp_write_vd_scaling_filter_coefs(priv, vpp_filter_coefs_bicubic,
						false);
	meson_vpp_write_vd_scaling_filter_coefs(priv, vpp_filter_coefs_bicubic,
						true);

	/* Disable OSDs */
	writel_bits(VIU_OSD1_OSD_BLK_ENABLE | VIU_OSD1_OSD_ENABLE, 0,
		    priv->io_base + _REG(VIU_OSD1_CTRL_STAT));
	writel_bits(VIU_OSD1_OSD_BLK_ENABLE | VIU_OSD1_OSD_ENABLE, 0,
		    priv->io_base + _REG(VIU_OSD2_CTRL_STAT));

	/* On GXL/GXM, Use the 10bit HDR conversion matrix */
	if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_GXM) ||
	    meson_vpu_is_compatible(priv, VPU_COMPATIBLE_GXL))
		meson_viu_load_matrix(priv);
	else if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_G12A))
		meson_viu_set_g12a_osd1_matrix(priv, RGB709_to_YUV709l_coeff,
					       true);

	/* Initialize OSD1 fifo control register */
	reg = VIU_OSD_DDR_PRIORITY_URGENT |
		VIU_OSD_HOLD_FIFO_LINES(4) |
		VIU_OSD_FIFO_DEPTH_VAL(32) | /* fifo_depth_val: 32*8=256 */
		VIU_OSD_WORDS_PER_BURST(4) | /* 4 words in 1 burst */
		VIU_OSD_FIFO_LIMITS(2);      /* fifo_lim: 2*16=32 */

	if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_G12A))
		reg |= meson_viu_osd_burst_length_reg(32);
	else
		reg |= meson_viu_osd_burst_length_reg(64);

	writel(reg, priv->io_base + _REG(VIU_OSD1_FIFO_CTRL_STAT));
	writel(reg, priv->io_base + _REG(VIU_OSD2_FIFO_CTRL_STAT));

	/* Set OSD alpha replace value */
	writel_bits(0xff << OSD_REPLACE_SHIFT,
		    0xff << OSD_REPLACE_SHIFT,
		    priv->io_base + _REG(VIU_OSD1_CTRL_STAT2));
	writel_bits(0xff << OSD_REPLACE_SHIFT,
		    0xff << OSD_REPLACE_SHIFT,
		    priv->io_base + _REG(VIU_OSD2_CTRL_STAT2));

	/* Disable VD1 AFBC */
	/* di_mif0_en=0 mif0_to_vpp_en=0 di_mad_en=0 and afbc vd1 set=0*/
	writel_bits(VIU_CTRL0_VD1_AFBC_MASK, 0,
		    priv->io_base + _REG(VIU_MISC_CTRL0));
	writel(0, priv->io_base + _REG(AFBC_ENABLE));

	writel(0x00FF00C0,
	       priv->io_base + _REG(VD1_IF0_LUMA_FIFO_SIZE));
	writel(0x00FF00C0,
	       priv->io_base + _REG(VD2_IF0_LUMA_FIFO_SIZE));

	if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_G12A)) {
		writel(VIU_OSD_BLEND_REORDER(0, 1) |
		       VIU_OSD_BLEND_REORDER(1, 0) |
		       VIU_OSD_BLEND_REORDER(2, 0) |
		       VIU_OSD_BLEND_REORDER(3, 0) |
		       VIU_OSD_BLEND_DIN_EN(1) |
		       VIU_OSD_BLEND1_DIN3_BYPASS_TO_DOUT1 |
		       VIU_OSD_BLEND1_DOUT_BYPASS_TO_BLEND2 |
		       VIU_OSD_BLEND_DIN0_BYPASS_TO_DOUT0 |
		       VIU_OSD_BLEND_BLEN2_PREMULT_EN(1) |
		       VIU_OSD_BLEND_HOLD_LINES(4),
		       priv->io_base + _REG(VIU_OSD_BLEND_CTRL));
		writel(OSD_BLEND_PATH_SEL_ENABLE,
		       priv->io_base + _REG(OSD1_BLEND_SRC_CTRL));
		writel(OSD_BLEND_PATH_SEL_ENABLE,
		       priv->io_base + _REG(OSD2_BLEND_SRC_CTRL));
		writel(0, priv->io_base + _REG(VD1_BLEND_SRC_CTRL));
		writel(0, priv->io_base + _REG(VD2_BLEND_SRC_CTRL));
		writel(0, priv->io_base + _REG(VIU_OSD_BLEND_DUMMY_DATA0));
		writel(0, priv->io_base + _REG(VIU_OSD_BLEND_DUMMY_ALPHA));
		writel_bits(DOLBY_BYPASS_EN(0xc), DOLBY_BYPASS_EN(0xc),
			    priv->io_base + _REG(DOLBY_PATH_CTRL));
	}
}
