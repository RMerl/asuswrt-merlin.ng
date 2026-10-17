/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright (C) 2018 Rockchip Electronics Co., Ltd
 */

#ifndef _ASM_ARCH_SDRAM_COMMON_H
#define _ASM_ARCH_SDRAM_COMMON_H

#ifndef MHZ
#define MHZ		(1000 * 1000)
#endif

#define PATTERN		(0x5aa5f00f)

#define MIN(a, b)	(((a) > (b)) ? (b) : (a))
#define MAX(a, b)	(((a) > (b)) ? (a) : (b))

/* get head info for initial */
#define DDR_FREQ_F0_SHIFT		(0)
#define DDR_FREQ_F1_SHIFT		(12)
#define DDR_FREQ_F2_SHIFT		(0)
#define DDR_FREQ_F3_SHIFT		(12)
#define DDR_FREQ_F4_SHIFT		(0)
#define DDR_FREQ_F5_SHIFT		(12)
#define DDR_FREQ_MASK			(0xfff)

#define UART_INFO_ID_SHIFT		(28)
#define UART_INFO_IOMUX_SHIFT		(24)
#define UART_INFO_BAUD_SHIFT		(0)
#define UART_INFO_ID(n)			(((n) >> 28) & 0xf)
#define UART_INFO_IOMUX(n)		(((n) >> 24) & 0xf)
#define UART_INFO_BAUD(n)		((n) & 0xffffff)

/* g_ch_info[15:0]: g_stdby_idle */
#define STANDBY_IDLE(n)			((n) & 0xffff)

#define SR_INFO(n)			(((n) >> 16) & 0xffff)
#define PD_INFO(n)			((n) & 0xffff)

#define FIRST_SCAN_CH(n)		(((n) >> 28) & 0xf)
#define CHANNEL_MASK(n)			(((n) >> 24) & 0xf)
#define STRIDE_TYPE(n)			(((n) >> 16) & 0xff)

#define DDR_2T_INFO(n)			((n) & 1)
#define PLL_SSMOD_SPREAD(n)		(((n) >> 1) & 0xff)
#define PLL_SSMOD_DIV(n)		(((n) >> 9) & 0xff)
#define PLL_SSMOD_DOWNSPREAD(n)		(((n) >> 17) & 0x3)

/* sdram_head_info_v2 define */
/* for *_drv_odten and *_drv_odtoff */
#define PHY_DQ_DRV_SHIFT		0
#define PHY_CA_DRV_SHIFT		8
#define PHY_CLK_DRV_SHIFT		16
#define DRAM_DQ_DRV_SHIFT		24
#define DRV_INFO_PHY_DQ_DRV(n)		((n) & 0xff)
#define DRV_INFO_PHY_CA_DRV(n)		(((n) >> PHY_CA_DRV_SHIFT) & 0xff)
#define DRV_INFO_PHY_CLK_DRV(n)		(((n) >> PHY_CLK_DRV_SHIFT) & 0xff)
#define DRV_INFO_DRAM_DQ_DRV(n)		(((n) >> DRAM_DQ_DRV_SHIFT) & 0xff)

/* for *_odt_info */
#define DRAM_ODT_SHIFT			0
#define PHY_ODT_SHIFT			8
#define PHY_ODT_PUUP_EN_SHIFT		18
#define PHY_ODT_PUDN_EN_SHIFT		19
#define ODT_INFO_DRAM_ODT(n)		(((n) >> DRAM_ODT_SHIFT) & 0xff)
#define ODT_INFO_PHY_ODT(n)		(((n) >> PHY_ODT_SHIFT) & 0x3ff)
#define ODT_INFO_PULLUP_EN(n)		(((n) >> PHY_ODT_PUUP_EN_SHIFT) & 1)
#define ODT_INFO_PULLDOWN_EN(n)		(((n) >> PHY_ODT_PUDN_EN_SHIFT) & 1)

/* for *odt_en_freq; */
#define DRAM_ODT_EN_FREQ_SHIFT		0
#define PHY_ODT_EN_FREQ_SHIFT		12
#define DRAMODT_EN_FREQ(n)		(((n) >> DRAM_ODT_EN_FREQ_SHIFT) & \
					 0xfff)
#define PHYODT_EN_FREQ(n)		(((n) >> PHY_ODT_EN_FREQ_SHIFT) & 0xfff)

#define PHY_DQ_SR_SHIFT			0
#define PHY_CA_SR_SHIFT			8
#define PHY_CLK_SR_SHIFT		16
#define DQ_SR_INFO(n)			(((n) >> PHY_DQ_SR_SHIFT) & 0xff)
#define CA_SR_INFO(n)			(((n) >> PHY_CA_SR_SHIFT) & 0xff)
#define CLK_SR_INFO(n)			(((n) >> PHY_CLK_SR_SHIFT) & 0xff)

/* LP4 */
#define LP4_CA_ODT_SHIFT			(18)
#define LP4_DRV_PU_CAL_ODTEN_SHIFT		(26)
#define LP4_DRV_PU_CAL_ODTOFF_SHIFT		(27)
#define PHY_LP4_DRV_PULLDOWN_EN_ODTEN_SHIFT	(28)
#define PHY_LP4_DRV_PULLDOWN_EN_ODTOFF_SHIFT	(29)
#define ODT_INFO_LP4_CA_ODT(n)			(((n) >> LP4_CA_ODT_SHIFT) & \
						 0xff)
#define LP4_DRV_PU_CAL_ODTEN(n)		\
	(((n) >> LP4_DRV_PU_CAL_ODTEN_SHIFT) & 1)
#define LP4_DRV_PU_CAL_ODTOFF(n)	\
	(((n) >> LP4_DRV_PU_CAL_ODTOFF_SHIFT) & 1)
#define PHY_LP4_DRV_PULLDOWN_EN_ODTEN(n)	\
	(((n) >> PHY_LP4_DRV_PULLDOWN_EN_ODTEN_SHIFT) & 1)
#define PHY_LP4_DRV_PULLDOWN_EN_ODTOFF(n)	\
	(((n) >> PHY_LP4_DRV_PULLDOWN_EN_ODTOFF_SHIFT) & 1)

#define PHY_LP4_CS_DRV_ODTEN_SHIFT	(0)
#define PHY_LP4_CS_DRV_ODTOFF_SHIFT	(8)
#define LP4_ODTE_CK_SHIFT		(16)
#define LP4_ODTE_CS_EN_SHIFT		(17)
#define LP4_ODTD_CA_EN_SHIFT		(18)
#define PHY_LP4_CS_DRV_ODTEN(n)		\
	(((n) >> PHY_LP4_CS_DRV_ODTEN_SHIFT) & 0xff)
#define PHY_LP4_CS_DRV_ODTOFF(n)	\
	(((n) >> PHY_LP4_CS_DRV_ODTOFF_SHIFT) & 0xff)
#define LP4_ODTE_CK_EN(n)		(((n) >> LP4_ODTE_CK_SHIFT) & 1)
#define LP4_ODTE_CS_EN(n)		(((n) >> LP4_ODTE_CS_EN_SHIFT) & 1)
#define LP4_ODTD_CA_EN(n)		(((n) >> LP4_ODTD_CA_EN_SHIFT) & 1)

#define PHY_LP4_DQ_VREF_SHIFT		(0)
#define LP4_DQ_VREF_SHIFT		(10)
#define LP4_CA_VREF_SHIFT		(20)

#define PHY_LP4_DQ_VREF(n)		\
	(((n) >> PHY_LP4_DQ_VREF_SHIFT) & 0x3ff)
#define LP4_DQ_VREF(n)			(((n) >> LP4_DQ_VREF_SHIFT) & 0x3ff)
#define LP4_CA_VREF(n)			(((n) >> LP4_CA_VREF_SHIFT) & 0x3ff)

#define LP4_DQ_ODT_EN_FREQ_SHIFT	(0)
#define PHY_LP4_ODT_EN_FREQ_SHIFT	(12)
#define LP4_CA_ODT_EN_FREQ_SHIFT	(0)
#define PHY_LP4_ODT_EN_FREQ(n)		\
	(((n) >> PHY_LP4_ODT_EN_FREQ_SHIFT) & 0xfff)
#define LP4_DQ_ODT_EN_FREQ(n)		\
	(((n) >> LP4_DQ_ODT_EN_FREQ_SHIFT) & 0xfff)
#define LP4_CA_ODT_EN_FREQ(n)		\
	(((n) >> LP4_CA_ODT_EN_FREQ_SHIFT) & 0xfff)

struct sdram_head_info_v0 {
	u32 start_tag;
	u32 version_info;
	u32 gcpu_gen_freq;
	u32 g_d2_lp2_freq;
	u32 g_d3_lp3_freq;
	u32 g_d4_lp4_freq;
	u32 g_uart_info;
	u32 g_sr_pd_idle;
	u32 g_ch_info;
	u32 g_2t_info;
	u32 reserved11;
	u32 reserved12;
	u32 reserved13;
};

struct index_info {
	u8 offset;
	u8 size;
};

struct sdram_head_info_index_v2 {
	u32 start_tag;
	u32 version_info;
	struct index_info cpu_gen_index;
	struct index_info global_index;

	struct index_info ddr2_index;
	struct index_info ddr3_index;

	struct index_info ddr4_index;
	struct index_info ddr5_index;

	struct index_info lp2_index;
	struct index_info lp3_index;

	struct index_info lp4_index;
	struct index_info lp5_index;

	struct index_info skew_index;
	struct index_info dq_map_index;

	struct index_info lp4x_index;
	struct index_info reserved;
};

struct global_info {
	u32 uart_info;
	u32 sr_pd_info;
	u32 ch_info;
	u32 info_2t;
	u32 reserved[4];
};

struct ddr2_3_4_lp2_3_info {
	u32 ddr_freq0_1;
	u32 ddr_freq2_3;
	u32 ddr_freq4_5;
	u32 drv_when_odten;
	u32 drv_when_odtoff;
	u32 odt_info;
	u32 odten_freq;
	u32 sr_when_odten;
	u32 sr_when_odtoff;
};

struct lp4_info {
	u32 ddr_freq0_1;
	u32 ddr_freq2_3;
	u32 ddr_freq4_5;
	u32 drv_when_odten;
	u32 drv_when_odtoff;
	u32 odt_info;
	u32 dq_odten_freq;
	u32 sr_when_odten;
	u32 sr_when_odtoff;
	u32 ca_odten_freq;
	u32 cs_drv_ca_odt_info;
	u32 vref_when_odten;
	u32 vref_when_odtoff;
};

struct dq_map_info {
	u32 byte_map[2];
	u32 lp3_dq0_7_map;
	u32 lp2_dq0_7_map;
	u32 ddr4_dq_map[4];
};

struct sdram_cap_info {
	unsigned int rank;
	/* dram column number, 0 means this channel is invalid */
	unsigned int col;
	/* dram bank number, 3:8bank, 2:4bank */
	unsigned int bk;
	/* channel buswidth, 2:32bit, 1:16bit, 0:8bit */
	unsigned int bw;
	/* die buswidth, 2:32bit, 1:16bit, 0:8bit */
	unsigned int dbw;
	/*
	 * row_3_4 = 1: 6Gb or 12Gb die
	 * row_3_4 = 0: normal die, power of 2
	 */
	unsigned int row_3_4;
	unsigned int cs0_row;
	unsigned int cs1_row;
	unsigned int cs0_high16bit_row;
	unsigned int cs1_high16bit_row;
	unsigned int ddrconfig;
};

struct sdram_base_params {
	unsigned int ddr_freq;
	unsigned int dramtype;
	unsigned int num_channels;
	unsigned int stride;
	unsigned int odt;
};

#define DDR_SYS_REG_VERSION		(0x2)
/* for modify tRFC and related timing */
#define DIE_CAP_512MBIT	64
#define DIE_CAP_1GBIT	128
#define DIE_CAP_2GBIT	256
#define DIE_CAP_4GBIT	512
#define DIE_CAP_8GBIT	1024
#define DIE_CAP_16GBIT	2048
#define DIE_CAP_32GBIT	4096
/*
 * sys_reg2 bitfield struct
 * [31]		row_3_4_ch1
 * [30]		row_3_4_ch0
 * [29:28]	chinfo
 * [27]		rank_ch1
 * [26:25]	col_ch1
 * [24]		bk_ch1
 * [23:22]	cs0_row_ch1
 * [21:20]	cs1_row_ch1
 * [19:18]	bw_ch1
 * [17:16]	dbw_ch1;
 * [15:13]	ddrtype
 * [12]		channelnum
 * [11]		rank_ch0
 * [10:9]	col_ch0
 * [8]		bk_ch0
 * [7:6]	cs0_row_ch0
 * [5:4]	cs1_row_ch0
 * [3:2]	bw_ch0
 * [1:0]	dbw_ch0
 */
#define SYS_REG_ENC_ROW_3_4(n, ch)	((n) << (30 + (ch)))
#define SYS_REG_DEC_ROW_3_4(n, ch)	(((n) >> (30 + (ch))) & 0x1)
#define SYS_REG_ENC_CHINFO(ch)		(1 << (28 + (ch)))
#define SYS_REG_ENC_DDRTYPE(n)		((n) << 13)
#define SYS_REG_DEC_DDRTYPE(n)		(((n) >> 13) & 0x7)
#define SYS_REG_ENC_NUM_CH(n)		(((n) - 1) << 12)
#define SYS_REG_DEC_NUM_CH(n)		(1 + (((n) >> 12) & 0x1))
#define SYS_REG_ENC_RANK(n, ch)		(((n) - 1) << (11 + ((ch) * 16)))
#define SYS_REG_DEC_RANK(n, ch)		(1 + (((n) >> (11 + 16 * (ch))) & 0x1))
#define SYS_REG_ENC_COL(n, ch)		(((n) - 9) << (9 + ((ch) * 16)))
#define SYS_REG_DEC_COL(n, ch)		(9 + (((n) >> (9 + 16 * (ch))) & 0x3))
#define SYS_REG_ENC_BK(n, ch)		(((n) == 3 ? 0 : 1) << \
						(8 + ((ch) * 16)))
#define SYS_REG_DEC_BK(n, ch)		(3 - (((n) >> (8 + 16 * (ch))) & 0x1))
#define SYS_REG_ENC_BW(n, ch)		((2 >> (n)) << (2 + ((ch) * 16)))
#define SYS_REG_DEC_BW(n, ch)		(2 >> (((n) >> (2 + 16 * (ch))) & 0x3))
#define SYS_REG_ENC_DBW(n, ch)		((2 >> (n)) << (0 + ((ch) * 16)))
#define SYS_REG_DEC_DBW(n, ch)		(2 >> (((n) >> (0 + 16 * (ch))) & 0x3))
/* sys reg 3 */
#define SYS_REG_ENC_VERSION(n)		((n) << 28)
#define SYS_REG_DEC_VERSION(n)		(((n) >> 28) & 0xf)
#define SYS_REG_ENC_CS0_ROW(n, os_reg2, os_reg3, ch) do { \
			(os_reg2) |= (((n) - 13) & 0x3) << (6 + 16 * (ch)); \
			(os_reg3) |= ((((n) - 13) & 0x4) >> 2) << \
				     (5 + 2 * (ch)); \
		} while (0)

#define SYS_REG_DEC_CS0_ROW(os_reg2, os_reg3, ch)	\
		((((((os_reg2) >> (6 + 16 * (ch)) & 0x3) | \
		 ((((os_reg3) >> (5 + 2 * (ch))) & 0x1) << 2)) + 1) & 0x7) + 12)

#define SYS_REG_ENC_CS1_ROW(n, os_reg2, os_reg3, ch) do { \
			(os_reg2) &= (~(0x3 << (4 + 16 * (ch)))); \
			(os_reg3) &= (~(0x1 << (4 + 2 * (ch)))); \
			(os_reg2) |= (((n) - 13) & 0x3) << (4 + 16 * (ch)); \
			(os_reg3) |= ((((n) - 13) & 0x4) >> 2) << \
				     (4 + 2 * (ch)); \
		} while (0)

#define SYS_REG_DEC_CS1_ROW(os_reg2, os_reg3, ch) \
		((((((os_reg2) >> (4 + 16 * (ch)) & 0x3) | \
		 ((((os_reg3) >> (4 + 2 * (ch))) & 0x1) << 2)) + 1) & 0x7) + 12)

#define SYS_REG_ENC_CS1_COL(n, ch)	(((n) - 9) << (0 + 2 * (ch)))
#define SYS_REG_DEC_CS1_COL(n, ch)	(9 + (((n) >> (0 + 2 * (ch))) & 0x3))

void sdram_print_dram_type(unsigned char dramtype);
void sdram_print_ddr_info(struct sdram_cap_info *cap_info,
			  struct sdram_base_params *base, u32 split);
void sdram_print_stride(unsigned int stride);

void sdram_org_config(struct sdram_cap_info *cap_info,
		      struct sdram_base_params *base,
		      u32 *p_os_reg2, u32 *p_os_reg3, u32 channel);

int sdram_detect_bw(struct sdram_cap_info *cap_info);
int sdram_detect_cs(struct sdram_cap_info *cap_info);
int sdram_detect_col(struct sdram_cap_info *cap_info,
		     u32 coltmp);
int sdram_detect_bank(struct sdram_cap_info *cap_info,
		      u32 coltmp, u32 bktmp);
int sdram_detect_bg(struct sdram_cap_info *cap_info,
		    u32 coltmp);
int sdram_detect_dbw(struct sdram_cap_info *cap_info, u32 dram_type);
int sdram_detect_row(struct sdram_cap_info *cap_info,
		     u32 coltmp, u32 bktmp, u32 rowtmp);
int sdram_detect_row_3_4(struct sdram_cap_info *cap_info,
			 u32 coltmp, u32 bktmp);
int sdram_detect_high_row(struct sdram_cap_info *cap_info);
int sdram_detect_cs1_row(struct sdram_cap_info *cap_info, u32 dram_type);
u64 sdram_get_cs_cap(struct sdram_cap_info *cap_info, u32 cs, u32 dram_type);
void sdram_copy_to_reg(u32 *dest, const u32 *src, u32 n);

#endif
