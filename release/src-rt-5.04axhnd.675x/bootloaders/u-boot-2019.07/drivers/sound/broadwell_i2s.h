/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Intel Broadwell I2S driver
 *
 * Copyright 2019 Google LLC
 *
 * Modified from dc i2s/broadwell/broadwell.h
 */

#ifndef __BROADWELL_I2S_H__
#define __BROADWELL_I2S_H__

enum {
	SSP_FIFO_SIZE = 7,
};

enum frame_sync_rel_timing_t {
	NEXT_FRMS_AFTER_END_OF_T4 =  0,
	NEXT_FRMS_WITH_LSB_PREVIOUS_FRM,
};

enum frame_sync_pol_t {
	SSP_FRMS_ACTIVE_LOW = 0,
	SSP_FRMS_ACTIVE_HIGH,
};

enum end_transfer_state_t {
	SSP_END_TRANSFER_STATE_LOW = 0,
	SSP_END_TRANSFER_STATE_PEVIOUS_BIT,
};

enum clock_mode_t {
	/* Data driven (falling), data sampled (rising), idle state (low) */
	SCLK_MODE_DDF_DSR_ISL,
	/* Data driven (rising), data sampled (falling), idle state (low) */
	SCLK_MODE_DDR_DSF_ISL,
	/* Data driven (rising), data sampled (falling), idle state (high) */
	SCLK_MODE_DDR_DSF_ISH,
	/* Data driven (falling), data sampled (rising), idle state (high) */
	SCLK_MODE_DDF_DSR_ISH,
};

struct i2s_shim_regs {
	u32 csr;		/* 0x00 */
	u32 reserved0[29];	/* 0x14 - 0x77 */
	u32 clkctl;		/* 0x78 */
	u32 reserved1;		/* 0x7c */
	u32 cs2;		/* 0x80 */
};

struct broadwell_i2s_regs {
	u32 sscr0;		/* 0x00 */
	u32 sscr1;		/* 0x04 */
	u32 sssr;		/* 0x08 */
	u32 ssitr;		/* 0x0c */
	u32 ssdr;		/* 0x10 */
	u32 reserved0[5];	/* 0x14 - 0x27 */
	u32 ssto;		/* 0x28 */
	u32 sspsp;		/* 0x2c */
	u32 sstsa;		/* 0x30 */
	u32 ssrsa;		/* 0x34 */
	u32 sstss;		/* 0x38 */
	u32 sscr2;		/* 0x40 */
	u32 sspsp2;		/* 0x44 */
};

/* SHIM Configuration & Status */
enum {
	/* Low Power Clock Select */
	SHIM_CS_LPCS = 1 << 31,
	/* SSP Force Clock Running */
	SHIM_CS_SFCR_SSP1 = 1 << 28,
	SHIM_CS_SFCR_SSP0 = 1 << 27,
	/* SSP1 IO Clock Select */
	SHIM_CS_S1IOCS = 1 << 23,
	/* SSP0 IO Clock Select */
	SHIM_CS_S0IOCS = 1 << 21,
	/* Parity Check Enable */
	SHIM_CS_PCE = 1 << 15,
	/* SSP DMA or PIO Mode */
	SHIM_CS_SDPM_PIO_SSP1 = 1 << 12,
	SHIM_CS_SDPM_DMA_SSP1 = 0 << 12,
	SHIM_CS_SDPM_PIO_SSP0 = 1 << 11,
	SHIM_CS_SDPM_DMA_SSP0 = 0 << 11,
	/* Run / Stall */
	SHIM_CS_STALL = 1 << 10,
	/* DSP Clock Select */
	SHIM_CS_DCS_DSP320_AF80 = 0 << 4,
	SHIM_CS_DCS_DSP160_AF80 = 1 << 4,
	SHIM_CS_DCS_DSP80_AF80 = 2 << 4,
	SHIM_CS_DCS_DSP320_AF160 = 4 << 4,
	SHIM_CS_DCS_DSP160_AF160 = 5 << 4,
	SHIM_CS_DCS_DSP32_AF32 = 6 << 4,
	SHIM_CS_DCS_MASK =  7 << 4,
	/* SSP Base Clock Select */
	SHIM_CS_SBCS_SSP0_24MHZ = 1 << 3,
	SHIM_CS_SBCS_SSP0_32MHZ = 0 << 3,
	SHIM_CS_SBCS_SSP1_24MHZ = 1 << 2,
	SHIM_CS_SBCS_SSP1_32MHZ = 0 << 2,
	/* DSP Core Reset */
	SHIM_CS_RST = 1 << 1,
};

/* SHIM Clock Control */
enum {
	/* Clock Frequency Change In Progress */
	SHIM_CLKCTL_CFCIP = 1 << 31,
	/* SSP MCLK Output Select */
	SHIM_CLKCTL_MCLK_MASK = 0x3,
	SHIM_CLKCTL_MCLK_SHIFT = 24,
	SHIM_CLKCTL_MCLK_DISABLED = 0 << 24,
	SHIM_CLKCTL_MCLK_6MHZ = 1 << 24,
	SHIM_CLKCTL_MCLK_12MHZ = 2 << 24,
	SHIM_CLKCTL_MCLK_24MHZ = 3 << 24,
	/* DSP Core Prevent Local Clock Gating */
	SHIM_CLKCTL_DCPLCG = 1 << 18,
	/* SSP Clock Output Enable */
	SHIM_CLKCTL_SCOE_SSP1 = 1 << 17,
	SHIM_CLKCTL_SCOE_SSP0 = 1 << 16,
	/* DMA Engine Force Local Clock Gating */
	SHIM_CLKCTL_DEFLCGB_DMA1_CGE = 0 << 6,
	SHIM_CLKCTL_DEFLCGB_DMA1_CGD = 1 << 6,
	SHIM_CLKCTL_DEFLCGB_DMA0_CGE = 0 << 5,
	SHIM_CLKCTL_DEFLCGB_DMA0_CGD = 1 << 5,
	/* SSP Force Local Clock Gating */
	SHIM_CLKCTL_SFLCGB_SSP1_CGE = 0 << 1,
	SHIM_CLKCTL_SFLCGB_SSP1_CGD = 1 << 1,
	SHIM_CLKCTL_SFLCGB_SSP0_CGE = 0 << 0,
	SHIM_CLKCTL_SFLCGB_SSP0_CGD = 1 << 0,

	/* Reserved bits: 30:26, 23:19, 15:7, 4:2 */
	SHIM_CLKCTL_RESERVED = 0x1f << 26 | 0x1f << 19 | 0x1ff << 7 | 0x7 << 2,
};

/* SSP Status */
enum {
	/* Bit Count Error */
	SSP_SSS_BCE = 1 << 23,
	/* Clock Sync Statu s*/
	SSP_SSS_CSS = 1 << 22,
	/* Transmit FIFO Underrun */
	SSP_SSS_TUR = 1 << 21,
	/* End Of Chain */
	SSP_SSS_EOC = 1 << 20,
	/* Receiver Time-out Interrupt */
	SSP_SSS_TINT = 1 << 19,
	/* Peripheral Trailing Byte Interrupt */
	SSP_SSS_PINT = 1 << 18,
	/* Received FIFO Level */
	SSP_RFL_MASK = 0xf,
	SSP_RFL_SHIFT = 12,
	/* Transmit FIFO Level */
	SSP_TFL_MASK = 0xf,
	SSP_TFL_SHIFT = 8,
	/* Receive FIFO Overrun */
	SSP_SSS_ROR = 1 << 7,
	/* Receive FIFO Service Request */
	SSP_SSS_RFS = 1 << 6,
	/* Transmit FIFO Service Request */
	SSP_SSS_TFS = 1 << 5,
	/* SSP Busy */
	SSP_SSS_BSY = 1 << 4,
	/* Receive FIFO Not Empty */
	SSP_SSS_RNE = 1 << 3,
	/* Transmit FIFO Not Full */
	SSP_SSS_TNF = 1 << 2,
};

/* SSP Control 0 */
enum {
	/* Mode */
	SSP_SSC0_MODE_NORMAL = 0 << 31,
	SSP_SSC0_MODE_NETWORK = 1 << 31,
	/* Audio Clock Select */
	SSP_SSC0_ACS_PCH = 0 << 30,
	/* Frame Rate Divider Control (0-7) */
	SSP_SSC0_FRDC_MASK = 0x7,
	SSP_SSC0_FRDC_SHIFT = 24,
	SSP_SSC0_FRDC_STEREO = 1 << 24,
	/* Transmit FIFO Underrun Interrupt Mask */
	SSP_SSC0_TIM = 1 << 23,
	/* Receive FIFO Underrun Interrupt Mask */
	SSP_SSC0_RIM = 1 << 22,
	/* Network Clock Select */
	SSP_SSC0_NCS_PCH = 0 << 21,
	/* Extended Data Size Select */
	SSP_SSC0_EDSS = 1 << 20,
	/* Serial Clock Rate (0-4095) */
	SSP_SSC0_SCR_SHIFT = 8,
	SSP_SSC0_SCR_MASK = 0xfff << SSP_SSC0_SCR_SHIFT,
	/* Synchronous Serial Port Enable */
	SSP_SSC0_SSE = 1 << 7,
	/* External Clock Select */
	SSP_SSC0_ECS_PCH = 0 << 6,
	/* Frame Format */
	SSP_SSC0_FRF_MOTOROLA_SPI = 0 << 4,
	SSP_SSC0_FRF_TI_SSP = 1 << 4,
	SSP_SSC0_FRF_NS_MICROWIRE = 2 << 4,
	SSP_SSC0_FRF_PSP = 3 << 4,
	/* Data Size Select */
	SSP_SSC0_DSS_SHIFT = 0,
	SSP_SSC0_DSS_MASK = 0xf << SSP_SSC0_DSS_SHIFT,
};

/* SSP Control 1 */
enum {
	/* TXD Tristate Enable on Last Phase */
	SSP_SSC1_TTELP = 1 << 31,
	/* TXD Tristate Enable */
	SSP_SSC1_TTE = 1 << 30,
	/* Enable Bit Count Error Interrupt */
	SSP_SSC1_EBCEI = 1 << 29,
	/* Slave Clock Running */
	SSP_SSC1_SCFR = 1 << 28,
	/* Enable Clock Request A */
	SSP_SSC1_ECRA = 1 << 27,
	/* Enable Clock Request B */
	SSP_SSC1_ECRB = 1 << 26,
	/* SSPCLK Direction */
	SSP_SSC1_SCLKDIR_SLAVE = 1 << 25,
	SSP_SSC1_SCLKDIR_MASTER = 0 << 25,
	/* SSPFRM Direction */
	SSP_SSC1_SFRMDIR_SLAVE = 1 << 24,
	SSP_SSC1_SFRMDIR_MASTER = 0 << 24,
	/* Receive without Transmit */
	SSP_SSC1_RWOT = 1 << 23,
	/* Trailing Byte */
	SSP_SSC1_TRAIL = 1 << 22,
	/* DMA Tx Service Request Enable */
	SSP_SSC1_TSRE = 1 << 21,
	/* DMA Rx Service Request Enable */
	SSP_SSC1_RSRE = 1 << 20,
	/* Receiver Timeout Interrupt Enable */
	SSP_SSC1_TINTE = 1 << 19,
	/* Periph. Trailing Byte Int. Enable */
	SSP_SSC1_PINTE = 1 << 18,
	/* Invert Frame Signal */
	SSP_SSC1_IFS = 1 << 16,
	/* Select FIFO for EFWR: test mode */
	SSP_SSC1_STRF = 1 << 15,
	/* Enable FIFO Write/Read: test mode */
	SSP_SSC1_EFWR = 1 << 14,
	/* Receive FIFO Trigger Threshold */
	SSP_SSC1_RFT_SHIFT = 10,
	SSP_SSC1_RFT_MASK = 0xf << SSP_SSC1_RFT_SHIFT,
	/* Transmit FIFO Trigger Threshold */
	SSP_SSC1_TFT_SHIFT = 6,
	SSP_SSC1_TFT_MASK = 0xf << SSP_SSC1_TFT_SHIFT,
	/* Microwire Transmit Data Size */
	SSP_SSC1_MWDS = 1 << 5,
	/* Motorola SPI SSPSCLK Phase Setting*/
	SSP_SSC1_SPH = 1 << 4,
	/* Motorola SPI SSPSCLK Polarity */
	SSP_SSC1_SPO = 1 << 3,
	/* Loopback mode: test mode */
	SSP_SSC1_LBM = 1 << 2,
	/* Transmit FIFO Interrupt Enable */
	SSP_SSC1_TIE = 1 << 1,
	/* Receive FIFO Interrupt Enable */
	SSP_SSC1_RIE = 1 << 0,

	SSP_SSC1_RESERVED = 17 << 1,
};

/* SSP Programmable Serial Protocol */
enum {
	/* Extended Dummy Stop (0-31) */
	SSP_PSP_EDYMSTOP_SHIFT = 26,
	SSP_PSP_EDMYSTOP_MASK = 0x7 << SSP_PSP_EDYMSTOP_SHIFT,
	/* Frame Sync Relative Timing */
	SSP_PSP_FSRT = 1 << 25,
	/* Dummy Stop low bits */
	SSP_PSP_DMYSTOP_SHIFT = 23,
	SSP_PSP_DMYSTOP_MASK = 0x3 << SSP_PSP_DMYSTOP_SHIFT,
	/* Serial Frame Width */
	SSP_PSP_SFRMWDTH_SHIFT = 16,
	SSP_PSP_SFRMWDTH_MASK = 0x3f << SSP_PSP_SFRMWDTH_SHIFT,
	/* Serial Frame Delay */
	SSP_PSP_SFRMDLY_MASK = 0x7f,
	SSP_PSP_SFRMDLY_SHIFT = 9,
	/* Start Delay */
	SSP_PSP_STRTDLY_MASK = 0x7,
	SSP_PSP_STRTDLY_SHIFT = 4,
	/* End of Transfer Data State */
	SSP_PSP_ETDS = 1 << 3,
	/* Serial Frame Polarity */
	SSP_PSP_SFRMP = 1 << 2,
	/* Serial Clock Mode */
	SSP_PSP_SCMODE_SHIFT = 0,
	SSP_PSP_SCMODE_MASK = 0x3 << SSP_PSP_SCMODE_SHIFT,

	SSP_PSP_RESERVED = 1 << 22,
};

/* SSP TX Time Slot Active */
enum {
	SSP_SSTSA_EN = 1 << 8,
	SSP_SSTSA_MASK = 0xff,
};

#endif /* __BROADWELL_I2S_H__ */
