/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __ASM_ARCH_TEGRA_DC_H
#define __ASM_ARCH_TEGRA_DC_H

/* Register definitions for the Tegra display controller */

/* CMD register 0x000 ~ 0x43 */
struct dc_cmd_reg {
	/* Address 0x000 ~ 0x002 */
	uint gen_incr_syncpt;		/* _CMD_GENERAL_INCR_SYNCPT_0 */
	uint gen_incr_syncpt_ctrl;	/* _CMD_GENERAL_INCR_SYNCPT_CNTRL_0 */
	uint gen_incr_syncpt_err;	/* _CMD_GENERAL_INCR_SYNCPT_ERROR_0 */

	uint reserved0[5];		/* reserved_0[5] */

	/* Address 0x008 ~ 0x00a */
	uint win_a_incr_syncpt;		/* _CMD_WIN_A_INCR_SYNCPT_0 */
	uint win_a_incr_syncpt_ctrl;	/* _CMD_WIN_A_INCR_SYNCPT_CNTRL_0 */
	uint win_a_incr_syncpt_err;	/* _CMD_WIN_A_INCR_SYNCPT_ERROR_0 */

	uint reserved1[5];		/* reserved_1[5] */

	/* Address 0x010 ~ 0x012 */
	uint win_b_incr_syncpt;		/* _CMD_WIN_B_INCR_SYNCPT_0 */
	uint win_b_incr_syncpt_ctrl;	/* _CMD_WIN_B_INCR_SYNCPT_CNTRL_0 */
	uint win_b_incr_syncpt_err;	/* _CMD_WIN_B_INCR_SYNCPT_ERROR_0 */

	uint reserved2[5];		/* reserved_2[5] */

	/* Address 0x018 ~ 0x01a */
	uint win_c_incr_syncpt;		/* _CMD_WIN_C_INCR_SYNCPT_0 */
	uint win_c_incr_syncpt_ctrl;	/* _CMD_WIN_C_INCR_SYNCPT_CNTRL_0 */
	uint win_c_incr_syncpt_err;	/* _CMD_WIN_C_INCR_SYNCPT_ERROR_0 */

	uint reserved3[13];		/* reserved_3[13] */

	/* Address 0x028 */
	uint cont_syncpt_vsync;		/* _CMD_CONT_SYNCPT_VSYNC_0 */

	uint reserved4[7];		/* reserved_4[7] */

	/* Address 0x030 ~ 0x033 */
	uint ctxsw;			/* _CMD_CTXSW_0 */
	uint disp_cmd_opt0;		/* _CMD_DISPLAY_COMMAND_OPTION0_0 */
	uint disp_cmd;			/* _CMD_DISPLAY_COMMAND_0 */
	uint sig_raise;			/* _CMD_SIGNAL_RAISE_0 */

	uint reserved5[2];		/* reserved_0[2] */

	/* Address 0x036 ~ 0x03e */
	uint disp_pow_ctrl;		/* _CMD_DISPLAY_POWER_CONTROL_0 */
	uint int_stat;			/* _CMD_INT_STATUS_0 */
	uint int_mask;			/* _CMD_INT_MASK_0 */
	uint int_enb;			/* _CMD_INT_ENABLE_0 */
	uint int_type;			/* _CMD_INT_TYPE_0 */
	uint int_polarity;		/* _CMD_INT_POLARITY_0 */
	uint sig_raise1;		/* _CMD_SIGNAL_RAISE1_0 */
	uint sig_raise2;		/* _CMD_SIGNAL_RAISE2_0 */
	uint sig_raise3;		/* _CMD_SIGNAL_RAISE3_0 */

	uint reserved6;			/* reserved_6 */

	/* Address 0x040 ~ 0x043 */
	uint state_access;		/* _CMD_STATE_ACCESS_0 */
	uint state_ctrl;		/* _CMD_STATE_CONTROL_0 */
	uint disp_win_header;		/* _CMD_DISPLAY_WINDOW_HEADER_0 */
	uint reg_act_ctrl;		/* _CMD_REG_ACT_CONTROL_0 */
};

enum {
	PIN_REG_COUNT		= 4,
	PIN_OUTPUT_SEL_COUNT	= 7,
};

/* COM register 0x300 ~ 0x329 */
struct dc_com_reg {
	/* Address 0x300 ~ 0x301 */
	uint crc_ctrl;			/* _COM_CRC_CONTROL_0 */
	uint crc_checksum;		/* _COM_CRC_CHECKSUM_0 */

	/* _COM_PIN_OUTPUT_ENABLE0/1/2/3_0: Address 0x302 ~ 0x305 */
	uint pin_output_enb[PIN_REG_COUNT];

	/* _COM_PIN_OUTPUT_POLARITY0/1/2/3_0: Address 0x306 ~ 0x309 */
	uint pin_output_polarity[PIN_REG_COUNT];

	/* _COM_PIN_OUTPUT_DATA0/1/2/3_0: Address 0x30a ~ 0x30d */
	uint pin_output_data[PIN_REG_COUNT];

	/* _COM_PIN_INPUT_ENABLE0_0: Address 0x30e ~ 0x311 */
	uint pin_input_enb[PIN_REG_COUNT];

	/* Address 0x312 ~ 0x313 */
	uint pin_input_data0;		/* _COM_PIN_INPUT_DATA0_0 */
	uint pin_input_data1;		/* _COM_PIN_INPUT_DATA1_0 */

	/* _COM_PIN_OUTPUT_SELECT0/1/2/3/4/5/6_0: Address 0x314 ~ 0x31a */
	uint pin_output_sel[PIN_OUTPUT_SEL_COUNT];

	/* Address 0x31b ~ 0x329 */
	uint pin_misc_ctrl;		/* _COM_PIN_MISC_CONTROL_0 */
	uint pm0_ctrl;			/* _COM_PM0_CONTROL_0 */
	uint pm0_duty_cycle;		/* _COM_PM0_DUTY_CYCLE_0 */
	uint pm1_ctrl;			/* _COM_PM1_CONTROL_0 */
	uint pm1_duty_cycle;		/* _COM_PM1_DUTY_CYCLE_0 */
	uint spi_ctrl;			/* _COM_SPI_CONTROL_0 */
	uint spi_start_byte;		/* _COM_SPI_START_BYTE_0 */
	uint hspi_wr_data_ab;		/* _COM_HSPI_WRITE_DATA_AB_0 */
	uint hspi_wr_data_cd;		/* _COM_HSPI_WRITE_DATA_CD */
	uint hspi_cs_dc;		/* _COM_HSPI_CS_DC_0 */
	uint scratch_reg_a;		/* _COM_SCRATCH_REGISTER_A_0 */
	uint scratch_reg_b;		/* _COM_SCRATCH_REGISTER_B_0 */
	uint gpio_ctrl;			/* _COM_GPIO_CTRL_0 */
	uint gpio_debounce_cnt;		/* _COM_GPIO_DEBOUNCE_COUNTER_0 */
	uint crc_checksum_latched;	/* _COM_CRC_CHECKSUM_LATCHED_0 */
};

enum dc_disp_h_pulse_pos {
	H_PULSE0_POSITION_A,
	H_PULSE0_POSITION_B,
	H_PULSE0_POSITION_C,
	H_PULSE0_POSITION_D,
	H_PULSE0_POSITION_COUNT,
};

struct _disp_h_pulse {
	/* _DISP_H_PULSE0/1/2_CONTROL_0 */
	uint h_pulse_ctrl;
	/* _DISP_H_PULSE0/1/2_POSITION_A/B/C/D_0 */
	uint h_pulse_pos[H_PULSE0_POSITION_COUNT];
};

enum dc_disp_v_pulse_pos {
	V_PULSE0_POSITION_A,
	V_PULSE0_POSITION_B,
	V_PULSE0_POSITION_C,
	V_PULSE0_POSITION_COUNT,
};

struct _disp_v_pulse0 {
	/* _DISP_H_PULSE0/1_CONTROL_0 */
	uint v_pulse_ctrl;
	/* _DISP_H_PULSE0/1_POSITION_A/B/C_0 */
	uint v_pulse_pos[V_PULSE0_POSITION_COUNT];
};

struct _disp_v_pulse2 {
	/* _DISP_H_PULSE2/3_CONTROL_0 */
	uint v_pulse_ctrl;
	/* _DISP_H_PULSE2/3_POSITION_A_0 */
	uint v_pulse_pos_a;
};

enum dc_disp_h_pulse_reg {
	H_PULSE0,
	H_PULSE1,
	H_PULSE2,
	H_PULSE_COUNT,
};

enum dc_disp_pp_select {
	PP_SELECT_A,
	PP_SELECT_B,
	PP_SELECT_C,
	PP_SELECT_D,
	PP_SELECT_COUNT,
};

/* DISP register 0x400 ~ 0x4c1 */
struct dc_disp_reg {
	/* Address 0x400 ~ 0x40a */
	uint disp_signal_opt0;		/* _DISP_DISP_SIGNAL_OPTIONS0_0 */
	uint disp_signal_opt1;		/* _DISP_DISP_SIGNAL_OPTIONS1_0 */
	uint disp_win_opt;		/* _DISP_DISP_WIN_OPTIONS_0 */
	uint mem_high_pri;		/* _DISP_MEM_HIGH_PRIORITY_0 */
	uint mem_high_pri_timer;	/* _DISP_MEM_HIGH_PRIORITY_TIMER_0 */
	uint disp_timing_opt;		/* _DISP_DISP_TIMING_OPTIONS_0 */
	uint ref_to_sync;		/* _DISP_REF_TO_SYNC_0 */
	uint sync_width;		/* _DISP_SYNC_WIDTH_0 */
	uint back_porch;		/* _DISP_BACK_PORCH_0 */
	uint disp_active;		/* _DISP_DISP_ACTIVE_0 */
	uint front_porch;		/* _DISP_FRONT_PORCH_0 */

	/* Address 0x40b ~ 0x419: _DISP_H_PULSE0/1/2_  */
	struct _disp_h_pulse h_pulse[H_PULSE_COUNT];

	/* Address 0x41a ~ 0x421 */
	struct _disp_v_pulse0 v_pulse0;	/* _DISP_V_PULSE0_ */
	struct _disp_v_pulse0 v_pulse1;	/* _DISP_V_PULSE1_ */

	/* Address 0x422 ~ 0x425 */
	struct _disp_v_pulse2 v_pulse3;	/* _DISP_V_PULSE2_ */
	struct _disp_v_pulse2 v_pulse4;	/* _DISP_V_PULSE3_ */

	/* Address 0x426 ~ 0x429 */
	uint m0_ctrl;			/* _DISP_M0_CONTROL_0 */
	uint m1_ctrl;			/* _DISP_M1_CONTROL_0 */
	uint di_ctrl;			/* _DISP_DI_CONTROL_0 */
	uint pp_ctrl;			/* _DISP_PP_CONTROL_0 */

	/* Address 0x42a ~ 0x42d: _DISP_PP_SELECT_A/B/C/D_0 */
	uint pp_select[PP_SELECT_COUNT];

	/* Address 0x42e ~ 0x435 */
	uint disp_clk_ctrl;		/* _DISP_DISP_CLOCK_CONTROL_0 */
	uint disp_interface_ctrl;	/* _DISP_DISP_INTERFACE_CONTROL_0 */
	uint disp_color_ctrl;		/* _DISP_DISP_COLOR_CONTROL_0 */
	uint shift_clk_opt;		/* _DISP_SHIFT_CLOCK_OPTIONS_0 */
	uint data_enable_opt;		/* _DISP_DATA_ENABLE_OPTIONS_0 */
	uint serial_interface_opt;	/* _DISP_SERIAL_INTERFACE_OPTIONS_0 */
	uint lcd_spi_opt;		/* _DISP_LCD_SPI_OPTIONS_0 */
	uint border_color;		/* _DISP_BORDER_COLOR_0 */

	/* Address 0x436 ~ 0x439 */
	uint color_key0_lower;		/* _DISP_COLOR_KEY0_LOWER_0 */
	uint color_key0_upper;		/* _DISP_COLOR_KEY0_UPPER_0 */
	uint color_key1_lower;		/* _DISP_COLOR_KEY1_LOWER_0 */
	uint color_key1_upper;		/* _DISP_COLOR_KEY1_UPPER_0 */

	uint reserved0[2];		/* reserved_0[2] */

	/* Address 0x43c ~ 0x442 */
	uint cursor_foreground;		/* _DISP_CURSOR_FOREGROUND_0 */
	uint cursor_background;		/* _DISP_CURSOR_BACKGROUND_0 */
	uint cursor_start_addr;		/* _DISP_CURSOR_START_ADDR_0 */
	uint cursor_start_addr_ns;	/* _DISP_CURSOR_START_ADDR_NS_0 */
	uint cursor_pos;		/* _DISP_CURSOR_POSITION_0 */
	uint cursor_pos_ns;		/* _DISP_CURSOR_POSITION_NS_0 */
	uint seq_ctrl;			/* _DISP_INIT_SEQ_CONTROL_0 */

	/* Address 0x443 ~ 0x446 */
	uint spi_init_seq_data_a;	/* _DISP_SPI_INIT_SEQ_DATA_A_0 */
	uint spi_init_seq_data_b;	/* _DISP_SPI_INIT_SEQ_DATA_B_0 */
	uint spi_init_seq_data_c;	/* _DISP_SPI_INIT_SEQ_DATA_C_0 */
	uint spi_init_seq_data_d;	/* _DISP_SPI_INIT_SEQ_DATA_D_0 */

	uint reserved1[0x39];		/* reserved1[0x39], */

	/* Address 0x480 ~ 0x484 */
	uint dc_mccif_fifoctrl;		/* _DISP_DC_MCCIF_FIFOCTRL_0 */
	uint mccif_disp0a_hyst;		/* _DISP_MCCIF_DISPLAY0A_HYST_0 */
	uint mccif_disp0b_hyst;		/* _DISP_MCCIF_DISPLAY0B_HYST_0 */
	uint mccif_disp0c_hyst;		/* _DISP_MCCIF_DISPLAY0C_HYST_0 */
	uint mccif_disp1b_hyst;		/* _DISP_MCCIF_DISPLAY1B_HYST_0 */

	uint reserved2[0x3b];		/* reserved2[0x3b] */

	/* Address 0x4c0 ~ 0x4c1 */
	uint dac_crt_ctrl;		/* _DISP_DAC_CRT_CTRL_0 */
	uint disp_misc_ctrl;		/* _DISP_DISP_MISC_CONTROL_0 */

	u32 rsvd_4c2[34];		/* 4c2 - 4e3 */

	/* Address 0x4e4 */
	u32 blend_background_color;	/* _DISP_BLEND_BACKGROUND_COLOR_0 */
};

enum dc_winc_filter_p {
	WINC_FILTER_COUNT	= 0x10,
};

/* Window A/B/C register 0x500 ~ 0x628 */
struct dc_winc_reg {

	/* Address 0x500 */
	uint color_palette;		/* _WINC_COLOR_PALETTE_0 */

	uint reserved0[0xff];		/* reserved_0[0xff] */

	/* Address 0x600 */
	uint palette_color_ext;		/* _WINC_PALETTE_COLOR_EXT_0 */

	/* _WINC_H_FILTER_P00~0F_0 */
	/* Address 0x601 ~ 0x610 */
	uint h_filter_p[WINC_FILTER_COUNT];

	/* Address 0x611 ~ 0x618 */
	uint csc_yof;			/* _WINC_CSC_YOF_0 */
	uint csc_kyrgb;			/* _WINC_CSC_KYRGB_0 */
	uint csc_kur;			/* _WINC_CSC_KUR_0 */
	uint csc_kvr;			/* _WINC_CSC_KVR_0 */
	uint csc_kug;			/* _WINC_CSC_KUG_0 */
	uint csc_kvg;			/* _WINC_CSC_KVG_0 */
	uint csc_kub;			/* _WINC_CSC_KUB_0 */
	uint csc_kvb;			/* _WINC_CSC_KVB_0 */

	/* Address 0x619 ~ 0x628: _WINC_V_FILTER_P00~0F_0 */
	uint v_filter_p[WINC_FILTER_COUNT];
};

/* WIN A/B/C Register 0x700 ~ 0x719*/
struct dc_win_reg {
	/* Address 0x700 ~ 0x719 */
	uint win_opt;			/* _WIN_WIN_OPTIONS_0 */
	uint byte_swap;			/* _WIN_BYTE_SWAP_0 */
	uint buffer_ctrl;		/* _WIN_BUFFER_CONTROL_0 */
	uint color_depth;		/* _WIN_COLOR_DEPTH_0 */
	uint pos;			/* _WIN_POSITION_0 */
	uint size;			/* _WIN_SIZE_0 */
	uint prescaled_size;		/* _WIN_PRESCALED_SIZE_0 */
	uint h_initial_dda;		/* _WIN_H_INITIAL_DDA_0 */
	uint v_initial_dda;		/* _WIN_V_INITIAL_DDA_0 */
	uint dda_increment;		/* _WIN_DDA_INCREMENT_0 */
	uint line_stride;		/* _WIN_LINE_STRIDE_0 */
	uint buf_stride;		/* _WIN_BUF_STRIDE_0 */
	uint uv_buf_stride;		/* _WIN_UV_BUF_STRIDE_0 */
	uint buffer_addr_mode;		/* _WIN_BUFFER_ADDR_MODE_0 */
	uint dv_ctrl;			/* _WIN_DV_CONTROL_0 */
	uint blend_nokey;		/* _WIN_BLEND_NOKEY_0 */
	uint blend_1win;		/* _WIN_BLEND_1WIN_0 */
	uint blend_2win_x;		/* _WIN_BLEND_2WIN_X_0 */
	uint blend_2win_y;		/* _WIN_BLEND_2WIN_Y_0 */
	uint blend_3win_xy;		/* _WIN_BLEND_3WIN_XY_0 */
	uint hp_fetch_ctrl;		/* _WIN_HP_FETCH_CONTROL_0 */
	uint global_alpha;		/* _WIN_GLOBAL_ALPHA */
	uint blend_layer_ctrl;		/* _WINBUF_BLEND_LAYER_CONTROL_0 */
	uint blend_match_select;	/* _WINBUF_BLEND_MATCH_SELECT_0 */
	uint blend_nomatch_select;	/* _WINBUF_BLEND_NOMATCH_SELECT_0 */
	uint blend_alpha_1bit;		/* _WINBUF_BLEND_ALPHA_1BIT_0 */
};

/* WINBUF A/B/C Register 0x800 ~ 0x80d */
struct dc_winbuf_reg {
	/* Address 0x800 ~ 0x80d */
	uint start_addr;		/* _WINBUF_START_ADDR_0 */
	uint start_addr_ns;		/* _WINBUF_START_ADDR_NS_0 */
	uint start_addr_u;		/* _WINBUF_START_ADDR_U_0 */
	uint start_addr_u_ns;		/* _WINBUF_START_ADDR_U_NS_0 */
	uint start_addr_v;		/* _WINBUF_START_ADDR_V_0 */
	uint start_addr_v_ns;		/* _WINBUF_START_ADDR_V_NS_0 */
	uint addr_h_offset;		/* _WINBUF_ADDR_H_OFFSET_0 */
	uint addr_h_offset_ns;		/* _WINBUF_ADDR_H_OFFSET_NS_0 */
	uint addr_v_offset;		/* _WINBUF_ADDR_V_OFFSET_0 */
	uint addr_v_offset_ns;		/* _WINBUF_ADDR_V_OFFSET_NS_0 */
	uint uflow_status;		/* _WINBUF_UFLOW_STATUS_0 */
	uint buffer_surface_kind;	/* DC_WIN_BUFFER_SURFACE_KIND */
	uint rsvd_80c;
	uint start_addr_hi;		/* DC_WINBUF_START_ADDR_HI_0 */
};

/* Display Controller (DC_) regs */
struct dc_ctlr {
	struct dc_cmd_reg cmd;		/* CMD register 0x000 ~ 0x43 */
	uint reserved0[0x2bc];

	struct dc_com_reg com;		/* COM register 0x300 ~ 0x329 */
	uint reserved1[0xd6];

	struct dc_disp_reg disp;	/* DISP register 0x400 ~ 0x4e4 */
	uint reserved2[0x1b];

	struct dc_winc_reg winc;	/* Window A/B/C 0x500 ~ 0x628 */
	uint reserved3[0xd7];

	struct dc_win_reg win;		/* WIN A/B/C 0x700 ~ 0x719*/
	uint reserved4[0xe6];

	struct dc_winbuf_reg winbuf;	/* WINBUF A/B/C 0x800 ~ 0x80d */
};

/* DC_CMD_DISPLAY_COMMAND 0x032 */
#define CTRL_MODE_SHIFT		5
#define CTRL_MODE_MASK		(0x3 << CTRL_MODE_SHIFT)
enum {
	CTRL_MODE_STOP,
	CTRL_MODE_C_DISPLAY,
	CTRL_MODE_NC_DISPLAY,
};

/* _WIN_COLOR_DEPTH_0 */
enum win_color_depth_id {
	COLOR_DEPTH_P1,
	COLOR_DEPTH_P2,
	COLOR_DEPTH_P4,
	COLOR_DEPTH_P8,
	COLOR_DEPTH_B4G4R4A4,
	COLOR_DEPTH_B5G5R5A,
	COLOR_DEPTH_B5G6R5,
	COLOR_DEPTH_AB5G5R5,
	COLOR_DEPTH_B8G8R8A8 = 12,
	COLOR_DEPTH_R8G8B8A8,
	COLOR_DEPTH_B6x2G6x2R6x2A8,
	COLOR_DEPTH_R6x2G6x2B6x2A8,
	COLOR_DEPTH_YCbCr422,
	COLOR_DEPTH_YUV422,
	COLOR_DEPTH_YCbCr420P,
	COLOR_DEPTH_YUV420P,
	COLOR_DEPTH_YCbCr422P,
	COLOR_DEPTH_YUV422P,
	COLOR_DEPTH_YCbCr422R,
	COLOR_DEPTH_YUV422R,
	COLOR_DEPTH_YCbCr422RA,
	COLOR_DEPTH_YUV422RA,
};

/* DC_CMD_DISPLAY_POWER_CONTROL 0x036 */
#define PW0_ENABLE		BIT(0)
#define PW1_ENABLE		BIT(2)
#define PW2_ENABLE		BIT(4)
#define PW3_ENABLE		BIT(6)
#define PW4_ENABLE		BIT(8)
#define PM0_ENABLE		BIT(16)
#define PM1_ENABLE		BIT(18)
#define SPI_ENABLE		BIT(24)
#define HSPI_ENABLE		BIT(25)

/* DC_CMD_STATE_ACCESS 0x040 */
#define  READ_MUX_ASSEMBLY	(0 << 0)
#define  READ_MUX_ACTIVE	(1 << 0)
#define  WRITE_MUX_ASSEMBLY	(0 << 2)
#define  WRITE_MUX_ACTIVE	(1 << 2)

/* DC_CMD_STATE_CONTROL 0x041 */
#define GENERAL_ACT_REQ		BIT(0)
#define WIN_A_ACT_REQ		BIT(1)
#define WIN_B_ACT_REQ		BIT(2)
#define WIN_C_ACT_REQ		BIT(3)
#define WIN_D_ACT_REQ		BIT(4)
#define WIN_H_ACT_REQ		BIT(5)
#define CURSOR_ACT_REQ		BIT(7)
#define GENERAL_UPDATE		BIT(8)
#define WIN_A_UPDATE		BIT(9)
#define WIN_B_UPDATE		BIT(10)
#define WIN_C_UPDATE		BIT(11)
#define WIN_D_UPDATE		BIT(12)
#define WIN_H_UPDATE		BIT(13)
#define CURSOR_UPDATE		BIT(15)
#define NC_HOST_TRIG		BIT(24)

/* DC_CMD_DISPLAY_WINDOW_HEADER 0x042 */
#define WINDOW_A_SELECT		BIT(4)
#define WINDOW_B_SELECT		BIT(5)
#define WINDOW_C_SELECT		BIT(6)
#define	WINDOW_D_SELECT		BIT(7)
#define	WINDOW_H_SELECT		BIT(8)

/* DC_DISP_DISP_WIN_OPTIONS 0x402 */
#define	CURSOR_ENABLE		BIT(16)
#define	SOR_ENABLE		BIT(25)
#define	TVO_ENABLE		BIT(28)
#define	DSI_ENABLE		BIT(29)
#define	HDMI_ENABLE		BIT(30)

/* DC_DISP_DISP_TIMING_OPTIONS 0x405 */
#define	VSYNC_H_POSITION(x)	((x) & 0xfff)

/* DC_DISP_DISP_CLOCK_CONTROL 0x42e */
#define SHIFT_CLK_DIVIDER_SHIFT	0
#define SHIFT_CLK_DIVIDER_MASK	(0xff << SHIFT_CLK_DIVIDER_SHIFT)
#define	PIXEL_CLK_DIVIDER_SHIFT	8
#define	PIXEL_CLK_DIVIDER_MSK	(0xf << PIXEL_CLK_DIVIDER_SHIFT)
enum {
	PIXEL_CLK_DIVIDER_PCD1,
	PIXEL_CLK_DIVIDER_PCD1H,
	PIXEL_CLK_DIVIDER_PCD2,
	PIXEL_CLK_DIVIDER_PCD3,
	PIXEL_CLK_DIVIDER_PCD4,
	PIXEL_CLK_DIVIDER_PCD6,
	PIXEL_CLK_DIVIDER_PCD8,
	PIXEL_CLK_DIVIDER_PCD9,
	PIXEL_CLK_DIVIDER_PCD12,
	PIXEL_CLK_DIVIDER_PCD16,
	PIXEL_CLK_DIVIDER_PCD18,
	PIXEL_CLK_DIVIDER_PCD24,
	PIXEL_CLK_DIVIDER_PCD13,
};

/* DC_DISP_DISP_INTERFACE_CONTROL 0x42f */
#define DATA_FORMAT_SHIFT	0
#define DATA_FORMAT_MASK	(0xf << DATA_FORMAT_SHIFT)
enum {
	DATA_FORMAT_DF1P1C,
	DATA_FORMAT_DF1P2C24B,
	DATA_FORMAT_DF1P2C18B,
	DATA_FORMAT_DF1P2C16B,
	DATA_FORMAT_DF2S,
	DATA_FORMAT_DF3S,
	DATA_FORMAT_DFSPI,
	DATA_FORMAT_DF1P3C24B,
	DATA_FORMAT_DF1P3C18B,
};
#define DATA_ALIGNMENT_SHIFT	8
enum {
	DATA_ALIGNMENT_MSB,
	DATA_ALIGNMENT_LSB,
};
#define DATA_ORDER_SHIFT	9
enum {
	DATA_ORDER_RED_BLUE,
	DATA_ORDER_BLUE_RED,
};

/* DC_DISP_DATA_ENABLE_OPTIONS 0x432 */
#define DE_SELECT_SHIFT		0
#define DE_SELECT_MASK		(0x3 << DE_SELECT_SHIFT)
#define DE_SELECT_ACTIVE_BLANK	0x0
#define DE_SELECT_ACTIVE	0x1
#define DE_SELECT_ACTIVE_IS	0x2
#define DE_CONTROL_SHIFT	2
#define DE_CONTROL_MASK		(0x7 << DE_CONTROL_SHIFT)
enum {
	DE_CONTROL_ONECLK,
	DE_CONTROL_NORMAL,
	DE_CONTROL_EARLY_EXT,
	DE_CONTROL_EARLY,
	DE_CONTROL_ACTIVE_BLANK,
};

/* DC_WIN_WIN_OPTIONS 0x700 */
#define H_DIRECTION		BIT(0)
enum {
	H_DIRECTION_INCREMENT,
	H_DIRECTION_DECREMENT,
};
#define V_DIRECTION		BIT(2)
enum {
	V_DIRECTION_INCREMENT,
	V_DIRECTION_DECREMENT,
};
#define COLOR_EXPAND		BIT(6)
#define CP_ENABLE		BIT(16)
#define DV_ENABLE		BIT(20)
#define WIN_ENABLE		BIT(30)

/* DC_WIN_BYTE_SWAP 0x701 */
#define BYTE_SWAP_SHIFT		0
#define BYTE_SWAP_MASK		(3 << BYTE_SWAP_SHIFT)
enum {
	BYTE_SWAP_NOSWAP,
	BYTE_SWAP_SWAP2,
	BYTE_SWAP_SWAP4,
	BYTE_SWAP_SWAP4HW
};

/* DC_WIN_POSITION 0x704 */
#define H_POSITION_SHIFT	0
#define H_POSITION_MASK		(0x1FFF << H_POSITION_SHIFT)
#define V_POSITION_SHIFT	16
#define V_POSITION_MASK		(0x1FFF << V_POSITION_SHIFT)

/* DC_WIN_SIZE 0x705 */
#define H_SIZE_SHIFT		0
#define H_SIZE_MASK		(0x1FFF << H_SIZE_SHIFT)
#define V_SIZE_SHIFT		16
#define V_SIZE_MASK		(0x1FFF << V_SIZE_SHIFT)

/* DC_WIN_PRESCALED_SIZE 0x706 */
#define H_PRESCALED_SIZE_SHIFT	0
#define H_PRESCALED_SIZE_MASK	(0x7FFF << H_PRESCALED_SIZE)
#define V_PRESCALED_SIZE_SHIFT	16
#define V_PRESCALED_SIZE_MASK	(0x1FFF << V_PRESCALED_SIZE)

/* DC_WIN_DDA_INCREMENT 0x709 */
#define H_DDA_INC_SHIFT		0
#define H_DDA_INC_MASK		(0xFFFF << H_DDA_INC_SHIFT)
#define V_DDA_INC_SHIFT		16
#define V_DDA_INC_MASK		(0xFFFF << V_DDA_INC_SHIFT)

#define DC_POLL_TIMEOUT_MS		50
#define DC_N_WINDOWS			5
#define DC_REG_SAVE_SPACE		(DC_N_WINDOWS + 5)

#endif /* __ASM_ARCH_TEGRA_DC_H */
