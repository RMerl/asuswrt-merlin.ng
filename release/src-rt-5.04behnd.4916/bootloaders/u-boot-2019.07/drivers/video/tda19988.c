// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Liviu Dudau <liviu@dudau.co.uk>
 *
 * Based on the Linux driver, (C) 2012 Texas Instruments
 */

#include <common.h>
#include <dm.h>
#include <display.h>
#include <i2c.h>

/*
 * TDA19988 uses paged registers. We encode the page# in the upper
 * bits of the register#. It also means that reads/writes to a register
 * have to ensure that the register's page is selected as the current
 * page.
 */
#define REG(page, addr)		(((page) << 8) | (addr))
#define REG2ADDR(reg)		((reg) & 0xff)
#define REG2PAGE(reg)		(((reg) >> 8) & 0xff)

/* register for setting current page */
#define REG_CURRENT_PAGE		0xff

/* Page 00h: General Control */
#define REG_VERSION_LSB		REG(0x00, 0x00)     /* read */
#define REG_MAIN_CNTRL0		REG(0x00, 0x01)     /* read/write */
#define  MAIN_CNTRL0_SR		BIT(0)
#define  MAIN_CNTRL0_DECS	BIT(1)
#define  MAIN_CNTRL0_DEHS	BIT(2)
#define  MAIN_CNTRL0_CECS	BIT(3)
#define  MAIN_CNTRL0_CEHS	BIT(4)
#define  MAIN_CNTRL0_SCALER	BIT(7)
#define REG_VERSION_MSB		REG(0x00, 0x02)     /* read */
#define REG_SOFTRESET		REG(0x00, 0x0a)     /* write */
#define  SOFTRESET_AUDIO	BIT(0)
#define  SOFTRESET_I2C_MASTER	BIT(1)
#define REG_DDC_DISABLE		REG(0x00, 0x0b)     /* read/write */
#define REG_I2C_MASTER		REG(0x00, 0x0d)     /* read/write */
#define  I2C_MASTER_DIS_MM	BIT(0)
#define  I2C_MASTER_DIS_FILT	BIT(1)
#define  I2C_MASTER_APP_STRT_LAT BIT(2)
#define REG_FEAT_POWERDOWN	REG(0x00, 0x0e)     /* read/write */
#define  FEAT_POWERDOWN_PREFILT	BIT(0)
#define  FEAT_POWERDOWN_CSC	BIT(1)
#define  FEAT_POWERDOWN_SPDIF	BIT(3)
#define REG_INT_FLAGS_0		REG(0x00, 0x0f)     /* read/write */
#define REG_INT_FLAGS_1		REG(0x00, 0x10)     /* read/write */
#define REG_INT_FLAGS_2		REG(0x00, 0x11)     /* read/write */
#define  INT_FLAGS_2_EDID_BLK_RD  BIT(1)
#define REG_ENA_VP_0		REG(0x00, 0x18)     /* read/write */
#define REG_ENA_VP_1		REG(0x00, 0x19)     /* read/write */
#define REG_ENA_VP_2		REG(0x00, 0x1a)     /* read/write */
#define REG_ENA_AP		REG(0x00, 0x1e)     /* read/write */
#define REG_VIP_CNTRL_0		REG(0x00, 0x20)     /* write */
#define  VIP_CNTRL_0_MIRR_A	BIT(7)
#define  VIP_CNTRL_0_SWAP_A(x)	(((x) & 7) << 4)
#define  VIP_CNTRL_0_MIRR_B	BIT(3)
#define  VIP_CNTRL_0_SWAP_B(x)	(((x) & 7) << 0)
#define REG_VIP_CNTRL_1		REG(0x00, 0x21)     /* write */
#define  VIP_CNTRL_1_MIRR_C	BIT(7)
#define  VIP_CNTRL_1_SWAP_C(x)	(((x) & 7) << 4)
#define  VIP_CNTRL_1_MIRR_D	BIT(3)
#define  VIP_CNTRL_1_SWAP_D(x)	(((x) & 7) << 0)
#define REG_VIP_CNTRL_2		REG(0x00, 0x22)     /* write */
#define  VIP_CNTRL_2_MIRR_E	BIT(7)
#define  VIP_CNTRL_2_SWAP_E(x)	(((x) & 7) << 4)
#define  VIP_CNTRL_2_MIRR_F	BIT(3)
#define  VIP_CNTRL_2_SWAP_F(x)	(((x) & 7) << 0)
#define REG_VIP_CNTRL_3		REG(0x00, 0x23)     /* write */
#define  VIP_CNTRL_3_X_TGL	BIT(0)
#define  VIP_CNTRL_3_H_TGL	BIT(1)
#define  VIP_CNTRL_3_V_TGL	BIT(2)
#define  VIP_CNTRL_3_EMB	BIT(3)
#define  VIP_CNTRL_3_SYNC_DE	BIT(4)
#define  VIP_CNTRL_3_SYNC_HS	BIT(5)
#define  VIP_CNTRL_3_DE_INT	BIT(6)
#define  VIP_CNTRL_3_EDGE	BIT(7)
#define REG_VIP_CNTRL_4		REG(0x00, 0x24)     /* write */
#define  VIP_CNTRL_4_BLC(x)	(((x) & 3) << 0)
#define  VIP_CNTRL_4_BLANKIT(x)	(((x) & 3) << 2)
#define  VIP_CNTRL_4_CCIR656	BIT(4)
#define  VIP_CNTRL_4_656_ALT	BIT(5)
#define  VIP_CNTRL_4_TST_656	BIT(6)
#define  VIP_CNTRL_4_TST_PAT	BIT(7)
#define REG_VIP_CNTRL_5		REG(0x00, 0x25)     /* write */
#define  VIP_CNTRL_5_CKCASE	BIT(0)
#define  VIP_CNTRL_5_SP_CNT(x)	(((x) & 3) << 1)
#define REG_MUX_VP_VIP_OUT	REG(0x00, 0x27)     /* read/write */
#define REG_MAT_CONTRL		REG(0x00, 0x80)     /* write */
#define  MAT_CONTRL_MAT_SC(x)	(((x) & 3) << 0)
#define  MAT_CONTRL_MAT_BP	BIT(2)
#define REG_VIDFORMAT		REG(0x00, 0xa0)     /* write */
#define REG_REFPIX_MSB		REG(0x00, 0xa1)     /* write */
#define REG_REFPIX_LSB		REG(0x00, 0xa2)     /* write */
#define REG_REFLINE_MSB		REG(0x00, 0xa3)     /* write */
#define REG_REFLINE_LSB		REG(0x00, 0xa4)     /* write */
#define REG_NPIX_MSB		REG(0x00, 0xa5)     /* write */
#define REG_NPIX_LSB		REG(0x00, 0xa6)     /* write */
#define REG_NLINE_MSB		REG(0x00, 0xa7)     /* write */
#define REG_NLINE_LSB		REG(0x00, 0xa8)     /* write */
#define REG_VS_LINE_STRT_1_MSB	REG(0x00, 0xa9)     /* write */
#define REG_VS_LINE_STRT_1_LSB	REG(0x00, 0xaa)     /* write */
#define REG_VS_PIX_STRT_1_MSB	REG(0x00, 0xab)     /* write */
#define REG_VS_PIX_STRT_1_LSB	REG(0x00, 0xac)     /* write */
#define REG_VS_LINE_END_1_MSB	REG(0x00, 0xad)     /* write */
#define REG_VS_LINE_END_1_LSB	REG(0x00, 0xae)     /* write */
#define REG_VS_PIX_END_1_MSB	REG(0x00, 0xaf)     /* write */
#define REG_VS_PIX_END_1_LSB	REG(0x00, 0xb0)     /* write */
#define REG_VS_LINE_STRT_2_MSB	REG(0x00, 0xb1)     /* write */
#define REG_VS_LINE_STRT_2_LSB	REG(0x00, 0xb2)     /* write */
#define REG_VS_PIX_STRT_2_MSB	REG(0x00, 0xb3)     /* write */
#define REG_VS_PIX_STRT_2_LSB	REG(0x00, 0xb4)     /* write */
#define REG_VS_LINE_END_2_MSB	REG(0x00, 0xb5)     /* write */
#define REG_VS_LINE_END_2_LSB	REG(0x00, 0xb6)     /* write */
#define REG_VS_PIX_END_2_MSB	REG(0x00, 0xb7)     /* write */
#define REG_VS_PIX_END_2_LSB	REG(0x00, 0xb8)     /* write */
#define REG_HS_PIX_START_MSB	REG(0x00, 0xb9)     /* write */
#define REG_HS_PIX_START_LSB	REG(0x00, 0xba)     /* write */
#define REG_HS_PIX_STOP_MSB	REG(0x00, 0xbb)     /* write */
#define REG_HS_PIX_STOP_LSB	REG(0x00, 0xbc)     /* write */
#define REG_VWIN_START_1_MSB	REG(0x00, 0xbd)     /* write */
#define REG_VWIN_START_1_LSB	REG(0x00, 0xbe)     /* write */
#define REG_VWIN_END_1_MSB	REG(0x00, 0xbf)     /* write */
#define REG_VWIN_END_1_LSB	REG(0x00, 0xc0)     /* write */
#define REG_VWIN_START_2_MSB	REG(0x00, 0xc1)     /* write */
#define REG_VWIN_START_2_LSB	REG(0x00, 0xc2)     /* write */
#define REG_VWIN_END_2_MSB	REG(0x00, 0xc3)     /* write */
#define REG_VWIN_END_2_LSB	REG(0x00, 0xc4)     /* write */
#define REG_DE_START_MSB	REG(0x00, 0xc5)     /* write */
#define REG_DE_START_LSB	REG(0x00, 0xc6)     /* write */
#define REG_DE_STOP_MSB		REG(0x00, 0xc7)     /* write */
#define REG_DE_STOP_LSB		REG(0x00, 0xc8)     /* write */
#define REG_TBG_CNTRL_0		REG(0x00, 0xca)     /* write */
#define  TBG_CNTRL_0_TOP_TGL	BIT(0)
#define  TBG_CNTRL_0_TOP_SEL	BIT(1)
#define  TBG_CNTRL_0_DE_EXT	BIT(2)
#define  TBG_CNTRL_0_TOP_EXT	BIT(3)
#define  TBG_CNTRL_0_FRAME_DIS	BIT(5)
#define  TBG_CNTRL_0_SYNC_MTHD	BIT(6)
#define  TBG_CNTRL_0_SYNC_ONCE	BIT(7)
#define REG_TBG_CNTRL_1		REG(0x00, 0xcb)     /* write */
#define  TBG_CNTRL_1_H_TGL	BIT(0)
#define  TBG_CNTRL_1_V_TGL	BIT(1)
#define  TBG_CNTRL_1_TGL_EN	BIT(2)
#define  TBG_CNTRL_1_X_EXT	BIT(3)
#define  TBG_CNTRL_1_H_EXT	BIT(4)
#define  TBG_CNTRL_1_V_EXT	BIT(5)
#define  TBG_CNTRL_1_DWIN_DIS	BIT(6)
#define REG_ENABLE_SPACE	REG(0x00, 0xd6)     /* write */
#define REG_HVF_CNTRL_0		REG(0x00, 0xe4)     /* write */
#define  HVF_CNTRL_0_SM		BIT(7)
#define  HVF_CNTRL_0_RWB	BIT(6)
#define  HVF_CNTRL_0_PREFIL(x)	(((x) & 3) << 2)
#define  HVF_CNTRL_0_INTPOL(x)	(((x) & 3) << 0)
#define REG_HVF_CNTRL_1		REG(0x00, 0xe5)     /* write */
#define  HVF_CNTRL_1_FOR	BIT(0)
#define  HVF_CNTRL_1_YUVBLK	BIT(1)
#define  HVF_CNTRL_1_VQR(x)	(((x) & 3) << 2)
#define  HVF_CNTRL_1_PAD(x)	(((x) & 3) << 4)
#define REG_RPT_CNTRL		REG(0x00, 0xf0)     /* write */
#define REG_AIP_CLKSEL		REG(0x00, 0xfd)     /* write */
#define  AIP_CLKSEL_AIP_SPDIF	(0 << 3)
#define  AIP_CLKSEL_AIP_I2S	BIT(3)
#define  AIP_CLKSEL_FS_ACLK	(0 << 0)
#define  AIP_CLKSEL_FS_MCLK	BIT(0)

/* Page 02h: PLL settings */
#define REG_PLL_SERIAL_1	REG(0x02, 0x00)     /* read/write */
#define  PLL_SERIAL_1_SRL_FDN	   BIT(0)
#define  PLL_SERIAL_1_SRL_IZ(x)	   (((x) & 3) << 1)
#define  PLL_SERIAL_1_SRL_MAN_IZ   BIT(6)
#define REG_PLL_SERIAL_2	REG(0x02, 0x01)     /* read/write */
#define  PLL_SERIAL_2_SRL_NOSC(x)  ((x) << 0)
#define  PLL_SERIAL_2_SRL_PR(x)	   (((x) & 0xf) << 4)
#define REG_PLL_SERIAL_3	REG(0x02, 0x02)     /* read/write */
#define  PLL_SERIAL_3_SRL_CCIR	   BIT(0)
#define  PLL_SERIAL_3_SRL_DE	   BIT(2)
#define  PLL_SERIAL_3_SRL_PXIN_SEL BIT(4)
#define REG_SERIALIZER		REG(0x02, 0x03)     /* read/write */
#define REG_BUFFER_OUT		REG(0x02, 0x04)     /* read/write */
#define REG_PLL_SCG1		REG(0x02, 0x05)     /* read/write */
#define REG_PLL_SCG2		REG(0x02, 0x06)     /* read/write */
#define REG_PLL_SCGN1		REG(0x02, 0x07)     /* read/write */
#define REG_PLL_SCGN2		REG(0x02, 0x08)     /* read/write */
#define REG_PLL_SCGR1		REG(0x02, 0x09)     /* read/write */
#define REG_PLL_SCGR2		REG(0x02, 0x0a)     /* read/write */
#define REG_AUDIO_DIV		REG(0x02, 0x0e)     /* read/write */
#define  AUDIO_DIV_SERCLK_1	0
#define  AUDIO_DIV_SERCLK_2	1
#define  AUDIO_DIV_SERCLK_4	2
#define  AUDIO_DIV_SERCLK_8	3
#define  AUDIO_DIV_SERCLK_16	4
#define  AUDIO_DIV_SERCLK_32	5
#define REG_SEL_CLK		REG(0x02, 0x11)     /* read/write */
#define  SEL_CLK_SEL_CLK1	BIT(0)
#define  SEL_CLK_SEL_VRF_CLK(x)	(((x) & 3) << 1)
#define  SEL_CLK_ENA_SC_CLK	BIT(3)
#define REG_ANA_GENERAL		REG(0x02, 0x12)     /* read/write */

/* Page 09h: EDID Control */
#define REG_EDID_DATA_0		REG(0x09, 0x00)     /* read */
/* next 127 successive registers are the EDID block */
#define REG_EDID_CTRL		REG(0x09, 0xfa)     /* read/write */
#define REG_DDC_ADDR		REG(0x09, 0xfb)     /* read/write */
#define REG_DDC_OFFS		REG(0x09, 0xfc)     /* read/write */
#define REG_DDC_SEGM_ADDR	REG(0x09, 0xfd)     /* read/write */
#define REG_DDC_SEGM		REG(0x09, 0xfe)     /* read/write */

/* Page 11h: audio settings and content info packets */
#define REG_AIP_CNTRL_0		REG(0x11, 0x00)     /* read/write */
#define  AIP_CNTRL_0_RST_FIFO	BIT(0)
#define REG_ENC_CNTRL		REG(0x11, 0x0d)     /* read/write */
#define  ENC_CNTRL_RST_ENC	BIT(0)
#define  ENC_CNTRL_RST_SEL	BIT(1)
#define  ENC_CNTRL_CTL_CODE(x)	(((x) & 3) << 2)

/* Page 12h: HDCP and OTP */
#define REG_TX3			REG(0x12, 0x9a)     /* read/write */
#define REG_TX4			REG(0x12, 0x9b)     /* read/write */
#define  TX4_PD_RAM		BIT(1)
#define REG_TX33		REG(0x12, 0xb8)     /* read/write */
#define  TX33_HDMI		BIT(1)

/* CEC registers, not paged */
#define REG_CEC_FRO_IM_CLK_CTRL		0xfb	    /* read/write */
#define  CEC_FRO_IM_CLK_CTRL_GHOST_DIS	BIT(7)
#define  CEC_FRO_IM_CLK_CTRL_ENA_OTP	BIT(6)
#define  CEC_FRO_IM_CLK_CTRL_IMCLK_SEL	BIT(1)
#define  CEC_FRO_IM_CLK_CTRL_FRO_DIV	BIT(0)
#define REG_CEC_RXSHPDINTENA		0xfc	    /* read/write */
#define REG_CEC_RXSHPDINT		0xfd	    /* read */
#define  CEC_RXSHPDINT_RXSENS		BIT(0)
#define  CEC_RXSHPDINT_HPD		BIT(1)
#define TDA19988_CEC_ENAMODS		0xff	    /* read/write */
#define  CEC_ENAMODS_EN_RXSENS		BIT(2)
#define  CEC_ENAMODS_EN_HDMI		BIT(1)
#define  CEC_ENAMODS_EN_CEC		BIT(0)

/* Device versions */
#define TDA9989N2	0x0101
#define TDA19989	0x0201
#define TDA19989N2	0x0202
#define TDA19988	0x0301

struct tda19988_priv {
	struct udevice *chip;
	struct udevice *cec_chip;
	u16 revision;
	u8 current_page;
};

static void tda19988_register_set(struct tda19988_priv *priv, u16 reg, u8 val)
{
	u8 old_val, page = REG2PAGE(reg);

	if (priv->current_page != page) {
		dm_i2c_reg_write(priv->chip, REG_CURRENT_PAGE, page);
		priv->current_page = page;
	}
	old_val = dm_i2c_reg_read(priv->chip, REG2ADDR(reg));
	old_val |= val;
	dm_i2c_reg_write(priv->chip, REG2ADDR(reg), old_val);
}

static void tda19988_register_clear(struct tda19988_priv *priv, u16 reg, u8 val)
{
	u8 old_val, page = REG2PAGE(reg);

	if (priv->current_page != page) {
		dm_i2c_reg_write(priv->chip, REG_CURRENT_PAGE, page);
		priv->current_page = page;
	}
	old_val = dm_i2c_reg_read(priv->chip, REG2ADDR(reg));
	old_val &= ~val;
	dm_i2c_reg_write(priv->chip, REG2ADDR(reg), old_val);
}

static void tda19988_register_write(struct tda19988_priv *priv, u16 reg, u8 val)
{
	u8 page = REG2PAGE(reg);

	if (priv->current_page != page) {
		dm_i2c_reg_write(priv->chip, REG_CURRENT_PAGE, page);
		priv->current_page = page;
	}
	dm_i2c_reg_write(priv->chip, REG2ADDR(reg), val);
}

static int tda19988_register_read(struct tda19988_priv *priv, u16 reg)
{
	u8 page = REG2PAGE(reg);

	if (priv->current_page != page) {
		dm_i2c_reg_write(priv->chip, REG_CURRENT_PAGE, page);
		priv->current_page = page;
	}
	return dm_i2c_reg_read(priv->chip, REG2ADDR(reg));
}

static void tda19988_register_write16(struct tda19988_priv *priv,
				      u16 reg, u16 val)
{
	u8 buf[] = { val >> 8, val }, page = REG2PAGE(reg);

	if (priv->current_page != page) {
		dm_i2c_reg_write(priv->chip, REG_CURRENT_PAGE, page);
		priv->current_page = page;
	}
	dm_i2c_write(priv->chip, REG2ADDR(reg), buf, 2);
}

static int tda19988_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
	struct tda19988_priv *priv = dev_get_priv(dev);
	int i, val = 0, offset = 0;

	/*
	 * The TDA998x has a problem when trying to read the EDID close to a
	 * HPD assertion: it needs a delay of 100ms to avoid timing out while
	 * trying to read EDID data.
	 */
	mdelay(120);

	if (priv->revision == TDA19988)
		tda19988_register_clear(priv, REG_TX4, TX4_PD_RAM);

	while (offset < buf_size) {
		tda19988_register_write(priv, REG_DDC_ADDR, 0xa0);
		tda19988_register_write(priv, REG_DDC_OFFS, offset);
		tda19988_register_write(priv, REG_DDC_SEGM_ADDR, 0x60);
		tda19988_register_write(priv, REG_DDC_SEGM, 0);

		/* enable reading EDID */
		tda19988_register_write(priv, REG_EDID_CTRL, 1);

		/* flags must be cleared by software */
		tda19988_register_write(priv, REG_EDID_CTRL, 0);

		/* wait for block read to complete */
		for (i = 300; i > 0; i--) {
			mdelay(1);
			val = tda19988_register_read(priv, REG_INT_FLAGS_2);
			if (val < 0)
				return val;
			if (val & INT_FLAGS_2_EDID_BLK_RD)
				break;
		}

		if (i == 0)
			return -ETIMEDOUT;

		priv->current_page = REG2PAGE(REG_EDID_DATA_0);
		dm_i2c_reg_write(priv->chip,
				 REG_CURRENT_PAGE, REG2PAGE(REG_EDID_DATA_0));
		val = dm_i2c_read(priv->chip,
				  REG2ADDR(REG_EDID_DATA_0), buf + offset, 128);
		offset += 128;
	}

	if (priv->revision == TDA19988)
		tda19988_register_set(priv, REG_TX4, TX4_PD_RAM);

	return offset;
}

static int tda19988_enable(struct udevice *dev, int panel_bpp,
			   const struct display_timing *timing)
{
	struct tda19988_priv *priv = dev_get_priv(dev);
	u8 div = 148500000 / timing->pixelclock.typ, reg;
	u16 line_clocks, lines;

	if (dev != 0) {
		div--;
		if (div > 3)
			div = 3;
	}
	/* first disable the video ports */
	tda19988_register_write(priv, REG_ENA_VP_0, 0);
	tda19988_register_write(priv, REG_ENA_VP_1, 0);
	tda19988_register_write(priv, REG_ENA_VP_2, 0);

	/* shutdown audio */
	tda19988_register_write(priv, REG_ENA_AP, 0);

	line_clocks = timing->hsync_len.typ + timing->hback_porch.typ +
		timing->hactive.typ + timing->hfront_porch.typ;
	lines = timing->vsync_len.typ + timing->vback_porch.typ +
		timing->vactive.typ + timing->vfront_porch.typ;

	/* mute the audio FIFO */
	tda19988_register_set(priv, REG_AIP_CNTRL_0, AIP_CNTRL_0_RST_FIFO);
	/* HDMI HDCP: off */
	tda19988_register_write(priv, REG_TBG_CNTRL_1, TBG_CNTRL_1_DWIN_DIS);
	tda19988_register_clear(priv, REG_TX33, TX33_HDMI);
	tda19988_register_write(priv, REG_ENC_CNTRL, ENC_CNTRL_CTL_CODE(0));

	/* no pre-filter or interpolator */
	tda19988_register_write(priv, REG_HVF_CNTRL_0, HVF_CNTRL_0_PREFIL(0) |
				HVF_CNTRL_0_INTPOL(0));
	tda19988_register_set(priv, REG_FEAT_POWERDOWN,
			      FEAT_POWERDOWN_PREFILT);
	tda19988_register_write(priv, REG_VIP_CNTRL_5, VIP_CNTRL_5_SP_CNT(0));
	tda19988_register_write(priv, REG_VIP_CNTRL_4,
				VIP_CNTRL_4_BLANKIT(0) | VIP_CNTRL_4_BLC(0) |
				VIP_CNTRL_4_TST_PAT);

	tda19988_register_clear(priv, REG_PLL_SERIAL_1,
				PLL_SERIAL_1_SRL_MAN_IZ);
	tda19988_register_clear(priv, REG_PLL_SERIAL_3, PLL_SERIAL_3_SRL_CCIR |
				PLL_SERIAL_3_SRL_DE);

	tda19988_register_write(priv, REG_SERIALIZER, 0);
	tda19988_register_write(priv, REG_HVF_CNTRL_1, HVF_CNTRL_1_VQR(0));

	tda19988_register_write(priv, REG_RPT_CNTRL, 0);
	tda19988_register_write(priv, REG_SEL_CLK, SEL_CLK_SEL_VRF_CLK(0) |
				SEL_CLK_SEL_CLK1 | SEL_CLK_ENA_SC_CLK);
	tda19988_register_write(priv, REG_PLL_SERIAL_2,
				PLL_SERIAL_2_SRL_NOSC(div) |
				PLL_SERIAL_2_SRL_PR(0));

	/* set color matrix bypass flag: */
	tda19988_register_write(priv, REG_MAT_CONTRL, MAT_CONTRL_MAT_BP |
				MAT_CONTRL_MAT_SC(1));
	tda19988_register_set(priv, REG_FEAT_POWERDOWN, FEAT_POWERDOWN_CSC);

	/* set BIAS tmds value: */
	tda19988_register_write(priv, REG_ANA_GENERAL, 0x09);

	/*
	 * Sync on rising HSYNC/VSYNC
	 */
	reg = VIP_CNTRL_3_SYNC_HS;

	/*
	 * TDA19988 requires high-active sync at input stage,
	 * so invert low-active sync provided by master encoder here
	 */
	if (timing->flags & DISPLAY_FLAGS_HSYNC_LOW)
		reg |= VIP_CNTRL_3_H_TGL;
	if (timing->flags & DISPLAY_FLAGS_VSYNC_LOW)
		reg |= VIP_CNTRL_3_V_TGL;
	tda19988_register_write(priv, REG_VIP_CNTRL_3, reg);

	tda19988_register_write(priv, REG_VIDFORMAT, 0x00);
	tda19988_register_write16(priv, REG_REFPIX_MSB,
				  timing->hfront_porch.typ + 3);
	tda19988_register_write16(priv, REG_REFLINE_MSB,
				  timing->vfront_porch.typ + 1);
	tda19988_register_write16(priv, REG_NPIX_MSB, line_clocks);
	tda19988_register_write16(priv, REG_NLINE_MSB, lines);
	tda19988_register_write16(priv, REG_VS_LINE_STRT_1_MSB,
				  timing->vfront_porch.typ);
	tda19988_register_write16(priv, REG_VS_PIX_STRT_1_MSB,
				  timing->hfront_porch.typ);
	tda19988_register_write16(priv, REG_VS_LINE_END_1_MSB,
				  timing->vfront_porch.typ +
				  timing->vsync_len.typ);
	tda19988_register_write16(priv, REG_VS_PIX_END_1_MSB,
				  timing->hfront_porch.typ);
	tda19988_register_write16(priv, REG_VS_LINE_STRT_2_MSB, 0);
	tda19988_register_write16(priv, REG_VS_PIX_STRT_2_MSB, 0);
	tda19988_register_write16(priv, REG_VS_LINE_END_2_MSB, 0);
	tda19988_register_write16(priv, REG_VS_PIX_END_2_MSB, 0);
	tda19988_register_write16(priv, REG_HS_PIX_START_MSB,
				  timing->hfront_porch.typ);
	tda19988_register_write16(priv, REG_HS_PIX_STOP_MSB,
				  timing->hfront_porch.typ +
				  timing->hsync_len.typ);
	tda19988_register_write16(priv, REG_VWIN_START_1_MSB,
				  lines - timing->vactive.typ - 1);
	tda19988_register_write16(priv, REG_VWIN_END_1_MSB, lines - 1);
	tda19988_register_write16(priv, REG_VWIN_START_2_MSB, 0);
	tda19988_register_write16(priv, REG_VWIN_END_2_MSB, 0);
	tda19988_register_write16(priv, REG_DE_START_MSB,
				  line_clocks - timing->hactive.typ);
	tda19988_register_write16(priv, REG_DE_STOP_MSB, line_clocks);

	if (priv->revision == TDA19988) {
		/* let incoming pixels fill the active space (if any) */
		tda19988_register_write(priv, REG_ENABLE_SPACE, 0x00);
	}

	/*
	 * Always generate sync polarity relative to input sync and
	 * revert input stage toggled sync at output stage
	 */
	reg = TBG_CNTRL_1_DWIN_DIS | TBG_CNTRL_1_TGL_EN;
	if (timing->flags & DISPLAY_FLAGS_HSYNC_LOW)
		reg |= TBG_CNTRL_1_H_TGL;
	if (timing->flags & DISPLAY_FLAGS_VSYNC_LOW)
		reg |= TBG_CNTRL_1_V_TGL;
	tda19988_register_write(priv, REG_TBG_CNTRL_1, reg);

	/* must be last register set: */
	tda19988_register_write(priv, REG_TBG_CNTRL_0, 0);

	/* turn on HDMI HDCP */
	reg &= ~TBG_CNTRL_1_DWIN_DIS;
	tda19988_register_write(priv, REG_TBG_CNTRL_1, reg);
	tda19988_register_write(priv, REG_ENC_CNTRL, ENC_CNTRL_CTL_CODE(1));
	tda19988_register_set(priv, REG_TX33, TX33_HDMI);

	mdelay(400);

	/* enable video ports */
	tda19988_register_write(priv, REG_ENA_VP_0, 0xff);
	tda19988_register_write(priv, REG_ENA_VP_1, 0xff);
	tda19988_register_write(priv, REG_ENA_VP_2, 0xff);
	/* set muxing after enabling ports: */
	tda19988_register_write(priv, REG_VIP_CNTRL_0,
				VIP_CNTRL_0_SWAP_A(2) | VIP_CNTRL_0_SWAP_B(3));
	tda19988_register_write(priv, REG_VIP_CNTRL_1,
				VIP_CNTRL_1_SWAP_C(4) | VIP_CNTRL_1_SWAP_D(5));
	tda19988_register_write(priv, REG_VIP_CNTRL_2,
				VIP_CNTRL_2_SWAP_E(0) | VIP_CNTRL_2_SWAP_F(1));

	return 0;
}

struct dm_display_ops tda19988_ops = {
	.read_edid = tda19988_read_edid,
	.enable = tda19988_enable,
};

static const struct udevice_id tda19988_ids[] = {
	{ .compatible = "nxp,tda998x" },
	{ }
};

static int tda19988_probe(struct udevice *dev)
{
	u8 cec_addr, chip_addr, rev_lo, rev_hi;
	int err;
	struct tda19988_priv *priv = dev_get_priv(dev);

	chip_addr = dev_read_addr(dev);
	/* CEC I2C address is using TDA19988 I2C address configuration pins */
	cec_addr = 0x34 + (chip_addr & 0x03);

	err = i2c_get_chip_for_busnum(0, cec_addr, 1, &priv->cec_chip);
	if (err) {
		printf("cec i2c_get_chip_for_busnum returned %d\n", err);
		return err;
	}

	err = i2c_get_chip_for_busnum(0, chip_addr, 1, &priv->chip);
	if (err) {
		printf("i2c_get_chip_for_busnum returned %d\n", err);
		return err;
	}

	priv->current_page = 0xff;

	/* wake up device */
	dm_i2c_reg_write(priv->cec_chip, TDA19988_CEC_ENAMODS,
			 CEC_ENAMODS_EN_RXSENS | CEC_ENAMODS_EN_HDMI);

	/* reset audio and I2C master */
	tda19988_register_write(priv, REG_SOFTRESET,
				SOFTRESET_AUDIO | SOFTRESET_I2C_MASTER);
	mdelay(50);
	tda19988_register_write(priv, REG_SOFTRESET, 0);
	mdelay(50);

	/* reset transmitter */
	tda19988_register_set(priv, REG_MAIN_CNTRL0, MAIN_CNTRL0_SR);
	tda19988_register_clear(priv, REG_MAIN_CNTRL0, MAIN_CNTRL0_SR);

	/* PLL registers common configuration */
	tda19988_register_write(priv, REG_PLL_SERIAL_1, 0x00);
	tda19988_register_write(priv, REG_PLL_SERIAL_2,
				PLL_SERIAL_2_SRL_NOSC(1));
	tda19988_register_write(priv, REG_PLL_SERIAL_3, 0x00);
	tda19988_register_write(priv, REG_SERIALIZER, 0x00);
	tda19988_register_write(priv, REG_BUFFER_OUT, 0x00);
	tda19988_register_write(priv, REG_PLL_SCG1, 0x00);
	tda19988_register_write(priv, REG_AUDIO_DIV, AUDIO_DIV_SERCLK_8);
	tda19988_register_write(priv, REG_SEL_CLK,
				SEL_CLK_SEL_CLK1 | SEL_CLK_ENA_SC_CLK);
	tda19988_register_write(priv, REG_PLL_SCGN1, 0xfa);
	tda19988_register_write(priv, REG_PLL_SCGN2, 0x00);
	tda19988_register_write(priv, REG_PLL_SCGR1, 0x5b);
	tda19988_register_write(priv, REG_PLL_SCGR2, 0x00);
	tda19988_register_write(priv, REG_PLL_SCG2, 0x10);

	/* Write the default value MUX register */
	tda19988_register_write(priv, REG_MUX_VP_VIP_OUT, 0x24);

	/* read version */
	rev_lo = dm_i2c_reg_read(priv->chip, REG_VERSION_LSB);
	rev_hi = dm_i2c_reg_read(priv->chip, REG_VERSION_MSB);

	/* mask off feature bits */
	priv->revision = ((rev_hi << 8) | rev_lo) & ~0x30;

	printf("HDMI: ");
	switch (priv->revision) {
	case TDA9989N2:
		printf("TDA9989 n2\n");
		break;
	case TDA19989:
		printf("TDA19989\n");
		break;
	case TDA19989N2:
		printf("TDA19989 n2\n");
		break;
	case TDA19988:
		printf("TDA19988\n");
		break;
	default:
		printf("unknown TDA device: 0x%04x\n", priv->revision);
		return -ENXIO;
	}

	/* after reset, enable DDC */
	tda19988_register_write(priv, REG_DDC_DISABLE, 0x00);

	/* set clock on DDC channel */
	tda19988_register_write(priv, REG_TX3, 39);

	/* if necessary, disable multi-master */
	if (priv->revision == TDA19989)
		tda19988_register_set(priv, REG_I2C_MASTER, I2C_MASTER_DIS_MM);

	dm_i2c_reg_write(priv->cec_chip, REG_CEC_FRO_IM_CLK_CTRL,
			 CEC_FRO_IM_CLK_CTRL_GHOST_DIS |
			 CEC_FRO_IM_CLK_CTRL_IMCLK_SEL);
	/* ensure interrupts are disabled */
	dm_i2c_reg_write(priv->cec_chip, REG_CEC_RXSHPDINTENA, 0);
	/* clear pending interrupts */
	dm_i2c_reg_read(priv->cec_chip, REG_CEC_RXSHPDINT);
	tda19988_register_read(priv, REG_INT_FLAGS_0);
	tda19988_register_read(priv, REG_INT_FLAGS_1);
	tda19988_register_read(priv, REG_INT_FLAGS_2);

	/* enable EDID read irq */
	tda19988_register_set(priv, REG_INT_FLAGS_2, INT_FLAGS_2_EDID_BLK_RD);

	return 0;
}

U_BOOT_DRIVER(tda19988) = {
	.name = "tda19988",
	.id = UCLASS_DISPLAY,
	.of_match = tda19988_ids,
	.ops = &tda19988_ops,
	.probe = tda19988_probe,
	.priv_auto_alloc_size = sizeof(struct tda19988_priv),
};
