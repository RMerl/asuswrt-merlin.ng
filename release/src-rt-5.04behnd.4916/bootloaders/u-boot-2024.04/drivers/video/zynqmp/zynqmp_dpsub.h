/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, Advanced Micro Devices, Inc.
 *
 */

#ifndef _VIDEO_ZYNQMP_DPSUB_H
#define _VIDEO_ZYNQMP_DPSUB_H

enum video_mode {
	VIDC_VM_640x480_60_P = 0,
	VIDC_VM_1024x768_60_P = 1,
};

enum {
	LANE_COUNT_1 = 1,
	LANE_COUNT_2 = 2,
};

enum {
	LINK_RATE_162GBPS = 0x06,
	LINK_RATE_270GBPS = 0x0A,
	LINK_RATE_540GBPS = 0x14,
};

enum video_color_depth {
	VIDC_BPC_6 = 6,
	VIDC_BPC_8 = 8,
	VIDC_BPC_10 = 10,
	VIDC_BPC_12 = 12,
	VIDC_BPC_14 = 14,
	VIDC_BPC_16 = 16,
	VIDC_BPC_NUM_SUPPORTED = 6,
	VIDC_BPC_UNKNOWN
};

enum video_color_encoding {
	DP_CENC_RGB = 0,
	DP_CENC_YONLY,
};

enum dp_dma_channel_type {
	VIDEO_CHAN,
	GRAPHICS_CHAN,
};

enum dp_dma_channel_state {
	DPDMA_DISABLE,
	DPDMA_ENABLE,
	DPDMA_IDLE,
	DPDMA_PAUSE
};

enum link_training_states {
	TS_CLOCK_RECOVERY,
	TS_CHANNEL_EQUALIZATION,
	TS_ADJUST_LINK_RATE,
	TS_ADJUST_LANE_COUNT,
	TS_FAILURE,
	TS_SUCCESS
};

enum video_frame_rate {
	VIDC_FR_60HZ = 60,
	VIDC_FR_NUM_SUPPORTED = 2,
	VIDC_FR_UNKNOWN
};

enum av_buf_video_modes {
	INTERLEAVED,
	SEMIPLANAR
};

enum av_buf_video_format {
	RGBA8888 = 1,
};

enum av_buf_video_stream {
	AVBUF_VIDSTREAM1_LIVE,
	AVBUF_VIDSTREAM1_NONLIVE,
	AVBUF_VIDSTREAM1_TPG,
	AVBUF_VIDSTREAM1_NONE,
};

enum av_buf_gfx_stream {
	AVBUF_VIDSTREAM2_DISABLEGFX = 0x0,
	AVBUF_VIDSTREAM2_NONLIVE_GFX = 0x4,
	AVBUF_VIDSTREAM2_LIVE_GFX = 0x8,
	AVBUF_VIDSTREAM2_NONE = 0xC0,
};

/**
 * struct aux_transaction - Description of an AUX channel transaction
 * @cmd_code:  Command code of the transaction
 * @num_bytes: The number of bytes in the transaction's payload data
 * @address:   The DPCD address of the transaction
 * @data:      Payload data of the AUX channel transaction
 */
struct aux_transaction {
	u16 cmd_code;
	u8 num_bytes;
	u32 address;
	u8 *data;
};

/**
 * struct link_config - Description of link configuration
 * @lane_count:                    Currently selected lane count for this link
 * @link_rate:                     Currently selected link rate for this link
 * @scrambler_en:                  Flag to determine whether the scrambler is
 *                                 enabled for this link
 * @enhanced_framing_mode:         Flag to determine whether enhanced framing
 *                                 mode is active for this link
 * @max_lane_count:                Maximum lane count for this link
 * @max_link_rate:                 Maximum link rate for this link
 * @support_enhanced_framing_mode: Flag to indicate whether the link supports
 *                                 enhanced framing mode
 * @vs_level:                      Voltage swing for each lane
 * @pe_level:                      Pre-emphasis/cursor level for each lane
 * @pattern:                      The current pattern currently in use over the main link
 */
struct link_config {
	u8 lane_count;
	u8 link_rate;
	u8 scrambler_en;
	u8 enhanced_framing_mode;
	u8 max_lane_count;
	u8 max_link_rate;
	u8 support_enhanced_framing_mode;
	u8 support_downspread_control;
	u8 vs_level;
	u8 pe_level;
	u8 pattern;
};

struct video_timing {
	u16 h_active;
	u16 h_front_porch;
	u16 h_sync_width;
	u16 h_back_porch;
	u16 h_total;
	bool h_sync_polarity;
	u16 v_active;
	u16 f0_pv_front_porch;
	u16 f0_pv_sync_width;
	u16 f0_pv_back_porch;
	u16 f0_pv_total;
	u16 f1_v_front_porch;
	u16 f1_v_sync_width;
	u16 f1_v_back_porch;
	u16 f1_v_total;
	bool v_sync_polarity;
};

struct video_timing_mode {
	enum video_mode  vid_mode;
	char name[21];
	enum video_frame_rate   frame_rate;
	struct video_timing     video_timing;
};

/*
 * struct main_stream_attributes - Main Stream Attributes (MSA)
 * @pixel_clock_hz:            The pixel clock of the stream (in Hz)
 * @h_start:                   Horizontal blank start (in pixels)
 * @v_start:                   Vertical blank start (in lines).
 * @misc0:                    Miscellaneous stream attributes 0
 * @misc1:                    Miscellaneous stream attributes 1
 * @n_vid                     N value for the video stream
 * @user_pixel_width:          The width of the user data input port.
 * @data_per_plane:           Used to translate the number of pixels per
 *                            line to the native internal 16-bit datapath.
 * @avg_bytes_per_tu:         Average number of bytes per transfer unit,
 *                            scaled up by a factor of 1000.
 * @transfer_unit_size:               Size of the transfer unit in the
 *                            framing logic.
 * @init_wait:                Number of initial wait cycles at the start
 *                            of a new line by the framing logic.
 * @bits_per_color:           Number of bits per color component.
 * @component_format:         The component format currently in
 *                            use by the video stream.
 * @dynamic_range:            The dynamic range currently in use
 *                            by the video stream.
 * @y_cb_cr_colorimetry:       The YCbCr colorimetry currently in
 *                            use by the video stream.
 * @synchronous_clock_mode:    Synchronous clock mode is currently
 *                            in use by the video stream.
 */
struct main_stream_attributes {
	struct video_timing_mode vid_timing_mode;
	u32 pixel_clock_hz;
	u32 h_start;
	u32 v_start;
	u32 misc0;
	u32 misc1;
	u32 n_vid;
	u32 user_pixel_width;
	u32 data_per_lane;
	u32 avg_bytes_per_tu;
	u32 transfer_unit_size;
	u32 init_wait;
	u32 bits_per_color;
	u8 component_format;
	u8 dynamic_range;
	u8 y_cb_cr_colorimetry;
	u8 synchronous_clock_mode;
};

struct av_buf_vid_attribute {
	enum av_buf_video_format video_format;
	u8 value;
	enum av_buf_video_modes mode;
	u32 sf[3];
	u8 sampling_en;
	u8 is_rgb;
	u8 swap;
	u8 bpp;
};

struct av_buf_mode {
	enum av_buf_video_stream video_src;
	enum av_buf_gfx_stream gfx_src;
	u8 video_clk;
};

struct dp_dma_descriptor {
	u32 control;
	u32 dscr_id;
	u32 xfer_size;
	u32 line_size_stride;
	u32 lsb_timestamp;
	u32 msb_timestamp;
	u32 addr_ext;
	u32 next_desr;
	u32 src_addr;
	u32 addr_ext_23;
	u32 addr_ext_45;
	u32 src_addr2;
	u32 src_addr3;
	u32 src_addr4;
	u32 src_addr5;
	u32 crc;
};

struct dp_dma_channel {
	struct dp_dma_descriptor *cur;
};

struct dp_dma_frame_buffer {
	u64 address;
	u32 size;
	u32 stride;
	u32 line_size;
};

struct dp_dma_gfx_channel {
	struct dp_dma_channel channel;
	u8 trigger_status;
	u8 av_buf_en;
	struct dp_dma_frame_buffer *frame_buffer;
};

struct dp_dma {
	phys_addr_t base_addr;
	struct dp_dma_gfx_channel gfx;
};

/**
 * struct zynqmp_dpsub_priv - Private structure
 * @dev: Device uclass for video_ops
 */
struct zynqmp_dpsub_priv {
	phys_addr_t base_addr;
	u32 clock;
	struct av_buf_vid_attribute *non_live_graphics;
	struct av_buf_mode av_mode;
	struct dp_dma_frame_buffer frame_buffer;

	struct link_config link_config;
	struct main_stream_attributes msa_config;
	struct dp_dma  *dp_dma;
	enum video_mode   video_mode;
	enum video_color_depth  bpc;
	enum video_color_encoding color_encode;
	u32 pix_clk;
	u8 dpcd_rx_caps[16];
	u8 lane_status_ajd_reqs[6];
	u8 sink_count;
	u8 use_max_lane_count;
	u8 use_max_link_rate;
	u8 lane_count;
	u8 link_rate;
	u8 use_max_cfg_caps;
	u8 en_sync_clk_mode;
};

/**************************** Variable Definitions ****************************/
#define TRAINING_PATTERN_SET						0x000C
#define TRAINING_PATTERN_SET_OFF					0x0
#define SCRAMBLING_DISABLE						0x0014
#define TRAINING_PATTERN_SET_TP1					0x1
#define TRAINING_PATTERN_SET_TP2					0x2
#define TRAINING_PATTERN_SET_TP3					0x3

#define AVBUF_BUF_4BIT_SF						0x11111
#define AVBUF_BUF_5BIT_SF						0x10842
#define AVBUF_BUF_6BIT_SF						0x10410
#define AVBUF_BUF_8BIT_SF						0x10101
#define AVBUF_BUF_10BIT_SF						0x10040
#define AVBUF_BUF_12BIT_SF						0x10000
#define AVBUF_BUF_6BPC							0x000
#define AVBUF_BUF_8BPC							0x001
#define AVBUF_BUF_10BPC							0x010
#define AVBUF_BUF_12BPC							0x011
#define AVBUF_CHBUF3							0x0000B01C
#define AVBUF_CHBUF3_BURST_LEN_SHIFT					2
#define AVBUF_CHBUF3_FLUSH_MASK						0x00000002
#define AVBUF_CHBUF0_EN_MASK						0x00000001
#define AVBUF_BUF_OUTPUT_AUD_VID_SELECT					0x0000B070
#define AVBUF_BUF_OUTPUT_AUD_VID_SELECT_VID_STREAM2_SEL_MASK		0x0000000C
#define AVBUF_BUF_OUTPUT_AUD_VID_SELECT_VID_STREAM1_SEL_MASK		0x00000003
#define AVBUF_BUF_OUTPUT_AUD_VID_SELECT					0x0000B070
#define AVBUF_BUF_GRAPHICS_COMP0_SCALE_FACTOR				0x0000B200
#define AVBUF_V_BLEND_LAYER1_CONTROL					0x0000A01C
#define AVBUF_V_BLEND_IN2CSC_COEFF0					0x0000A080
#define AVBUF_BUF_FORMAT						0x0000B000
#define AVBUF_BUF_FORMAT_NL_VID_FORMAT_MASK				0x0000001F
#define AVBUF_BUF_FORMAT_NL_GRAPHX_FORMAT_MASK				0x00000F00
#define AVBUF_BUF_FORMAT_NL_GRAPHX_FORMAT_SHIFT				8
#define AVBUF_V_BLEND_LAYER0_CONTROL_RGB_MODE_SHIFT			1
#define AVBUF_V_BLEND_OUTPUT_VID_FORMAT_EN_DOWNSAMPLE_SHIFT		4
#define AVBUF_V_BLEND_OUTPUT_VID_FORMAT					0x0000A014
#define AVBUF_V_BLEND_RGB2YCBCR_COEFF0					0x0000A020
#define AVBUF_V_BLEND_LUMA_OUTCSC_OFFSET				0x0000A074
#define AVBUF_V_BLEND_LUMA_IN1CSC_OFFSET_POST_OFFSET_SHIFT		16
#define AVBUF_V_BLEND_SET_GLOBAL_ALPHA_REG_VALUE_SHIFT			1
#define AVBUF_V_BLEND_SET_GLOBAL_ALPHA_REG				0x0000A00C
#define DP_MAIN_STREAM_MISC0_COMPONENT_FORMAT_SHIFT			1
#define DP_MAIN_STREAM_MISC0_DYNAMIC_RANGE_SHIFT			3
#define DP_MAIN_STREAM_MISC0_YCBCR_COLORIMETRY_SHIFT			4
#define DP_MAIN_STREAM_MISC1_Y_ONLY_EN_MASK				0x00000080
#define DP_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR422			0x1
#define AVBUF_PL_CLK							0x0
#define AVBUF_PS_CLK							0x1
#define AVBUF_BUF_AUD_VID_CLK_SOURCE_VID_TIMING_SRC_SHIFT		2
#define AVBUF_BUF_AUD_VID_CLK_SOURCE_VID_CLK_SRC_SHIFT			0
#define AVBUF_BUF_AUD_VID_CLK_SOURCE_AUD_CLK_SRC_SHIFT			1
#define AVBUF_BUF_AUD_VID_CLK_SOURCE					0x0000B120
#define AVBUF_BUF_SRST_REG						0x0000B124
#define AVBUF_BUF_SRST_REG_VID_RST_MASK					0x00000002
#define AVBUF_CLK_FPD_BASEADDR						0xFD1A0000
#define AVBUF_CLK_LPD_BASEADDR						0xFF5E0000
#define AVBUF_LPD_CTRL_OFFSET						16
#define AVBUF_FPD_CTRL_OFFSET						12
#define AVBUF_EXTERNAL_DIVIDER						2
#define AVBUF_VIDEO_REF_CTRL						0x00000070
#define AVBUF_VIDEO_REF_CTRL_SRCSEL_MASK				0x00000007
#define AVBUF_VPLL_SRC_SEL						0
#define AVBUF_DPLL_SRC_SEL						2
#define AVBUF_RPLL_TO_FPD_SRC_SEL					3
#define AVBUF_INPUT_REF_CLK						3333333333
#define AVBUF_PLL_OUT_FREQ						1450000000
#define AVBUF_INPUT_FREQ_PRECISION					100
#define AVBUF_PRECISION							16
#define AVBUF_SHIFT_DECIMAL						BIT(16)
#define AVBUF_DECIMAL							(AVBUF_SHIFT_DECIMAL - 1)
#define AVBUF_ENABLE_BIT						1
#define AVBUF_DISABLE_BIT						0
#define AVBUF_PLL_CTRL_BYPASS_SHIFT					3
#define AVBUF_PLL_CTRL_FBDIV_SHIFT					8
#define AVBUF_PLL_CTRL_DIV2_SHIFT					16
#define AVBUF_PLL_CTRL_PRE_SRC_SHIFT					20
#define AVBUF_PLL_CTRL							0x00000020
#define AVBUF_PLL_CFG_CP_SHIFT						5
#define AVBUF_PLL_CFG_RES_SHIFT						0
#define AVBUF_PLL_CFG_LFHF_SHIFT					10
#define AVBUF_PLL_CFG_LOCK_DLY_SHIFT					25
#define AVBUF_PLL_CFG_LOCK_CNT_SHIFT					13
#define AVBUF_PLL_FRAC_CFG						0x00000028
#define AVBUF_PLL_FRAC_CFG_ENABLED_SHIFT				31
#define AVBUF_PLL_FRAC_CFG_DATA_SHIFT					0
#define AVBUF_PLL_CTRL_RESET_MASK					0x00000001
#define AVBUF_PLL_CTRL_RESET_SHIFT					0
#define AVBUF_PLL_STATUS						0x00000044
#define AVBUF_REG_OFFSET						4
#define AVBUF_PLL_CTRL_BYPASS_MASK					0x00000008
#define AVBUF_PLL_CTRL_BYPASS_SHIFT					3
#define AVBUF_DOMAIN_SWITCH_CTRL					0x00000044
#define AVBUF_DOMAIN_SWITCH_DIVISOR0_MASK				0x00003F00
#define AVBUF_DOMAIN_SWITCH_DIVISOR0_SHIFT				8
#define AVBUF_PLL_CFG							0x00000024
#define AVBUF_BUF_AUD_VID_CLK_SOURCE_VID_CLK_SRC_SHIFT			0
#define AVBUF_VIDEO_REF_CTRL_CLKACT_MASK				0x01000000
#define AVBUF_VIDEO_REF_CTRL_CLKACT_SHIFT				24
#define AVBUF_VIDEO_REF_CTRL_DIVISOR1_MASK				0x003F0000
#define AVBUF_VIDEO_REF_CTRL_DIVISOR1_SHIFT				16
#define AVBUF_VIDEO_REF_CTRL_DIVISOR0_MASK				0x00003F00
#define AVBUF_VIDEO_REF_CTRL_DIVISOR0_SHIFT				8
#define AVBUF_VIDEO_REF_CTRL_CLKACT_MASK				0x01000000
#define AVBUF_VIDEO_REF_CTRL_CLKACT_SHIFT				24

#define DP_INTERRUPT_SIG_STATE						0x0130
#define DP_INTR_STATUS							0x03A0
#define DP_INTERRUPT_SIG_STATE_HPD_STATE_MASK				0x00000001
#define DP_INTR_HPD_EVENT_MASK						0x00000002
#define DP_INTR_HPD_PULSE_DETECTED_MASK					0x00000010
#define DP_HPD_DURATION							0x0150
#define DP_FORCE_SCRAMBLER_RESET					0x00C0
#define DP_ENABLE_MAIN_STREAM						0x0084
#define DP_IS_CONNECTED_MAX_TIMEOUT_COUNT				50
#define DP_0_LINK_RATE							20
#define DP_0_LANE_COUNT							1
#define DP_ENHANCED_FRAME_EN						0x0008
#define DP_LANE_COUNT_SET						0x0004
#define DP_LINK_BW_SET_162GBPS						0x06
#define DP_LINK_BW_SET_270GBPS						0x0A
#define DP_LINK_BW_SET_540GBPS						0x14
#define DP_LINK_BW_SET							0x0000
#define DP_DOWNSPREAD_CTRL						0x0018
#define DP_SCRAMBLING_DISABLE						0x0014
#define DP_AUX_CMD_READ							0x9
#define DP_AUX_CMD_WRITE						0x8
#define DP_AUX_CMD_I2C_READ						0x1
#define DP_AUX_CMD_I2C_READ_MOT						0x5
#define DP_AUX_CMD_I2C_WRITE						0x0
#define DP_AUX_CMD_I2C_WRITE_MOT					0x4
#define DP_REPLY_STATUS_REPLY_IN_PROGRESS_MASK				0x00000002
#define DP_REPLY_STATUS_REQUEST_IN_PROGRESS_MASK			0x00000004
#define DP_REPLY_STATUS							0x014C
#define DP_AUX_MAX_TIMEOUT_COUNT					50
#define DP_AUX_MAX_DEFER_COUNT						50
#define DP_AUX_ADDRESS							0x0108
#define DP_AUX_WRITE_FIFO						0x0104
#define DP_AUX_CMD							0x0100
#define DP_AUX_CMD_SHIFT						8
#define DP_AUX_CMD_NBYTES_TRANSFER_MASK					0x0000000F
#define DP_AUX_REPLY_CODE						0x0138
#define DP_AUX_REPLY_CODE_DEFER						0x2
#define DP_AUX_REPLY_CODE_I2C_DEFER					0x8
#define DP_AUX_REPLY_CODE_NACK						0x1
#define DP_AUX_REPLY_CODE_I2C_NACK					0x4
#define DP_REPLY_DATA_COUNT						0x0148
#define DP_AUX_REPLY_DATA						0x0134
#define DP_LANE_COUNT_SET_1						0x01
#define DP_LANE_COUNT_SET_2						0x02
#define DP_MAXIMUM_PE_LEVEL						2
#define DP_MAXIMUM_VS_LEVEL						3
#define DP_MAIN_STREAM_MISC0_COMPONENT_FORMAT_RGB			0x0
#define DP_MAIN_STREAM_MISC0_BDC_6BPC					0x0
#define DP_MAIN_STREAM_MISC0_BDC_8BPC					0x1
#define DP_MAIN_STREAM_MISC0_BDC_10BPC					0x2
#define DP_MAIN_STREAM_MISC0_BDC_12BPC					0x3
#define DP_MAIN_STREAM_MISC0_BDC_16BPC					0x4
#define DP_MAIN_STREAM_MISC0_BDC_SHIFT					5
#define DP_PHY_CONFIG_TX_PHY_8B10BEN_MASK				0x0010000
#define DP_PHY_CONFIG_PHY_RESET_MASK					0x0000001
#define DP_ENABLE_MAIN_STREAM						0x0084
#define DP_SOFT_RESET							0x001C
#define DP_MAIN_STREAM_HTOTAL						0x0180
#define DP_MAIN_STREAM_VTOTAL						0x0184
#define DP_MAIN_STREAM_POLARITY						0x0188
#define DP_MAIN_STREAM_POLARITY_VSYNC_POL_SHIFT				1
#define DP_MAIN_STREAM_HSWIDTH						0x018C
#define DP_MAIN_STREAM_VSWIDTH						0x0190
#define DP_MAIN_STREAM_HRES						0x0194
#define DP_MAIN_STREAM_VRES						0x0198
#define DP_MAIN_STREAM_HSTART						0x019C
#define DP_MAIN_STREAM_VSTART						0x01A0
#define DP_MAIN_STREAM_MISC0						0x01A4
#define DP_MAIN_STREAM_MISC1						0x01A8
#define DP_M_VID							0x01AC
#define DP_N_VID							0x01B4
#define DP_USER_PIXEL_WIDTH						0x01B8
#define DP_USER_DATA_COUNT_PER_LANE					0x01BC
#define DP_TU_SIZE							0x01B0
#define DP_MIN_BYTES_PER_TU						0x01C4
#define DP_FRAC_BYTES_PER_TU						0x01C8
#define DP_INIT_WAIT							0x01CC
#define DP_PHY_CLOCK_SELECT_162GBPS					0x1
#define DP_PHY_CLOCK_SELECT_270GBPS					0x3
#define DP_PHY_CLOCK_SELECT_540GBPS					0x5
#define DP_PHY_STATUS							0x0280
#define DP_PHY_STATUS_ALL_LANES_READY_MASK				0x00000013
#define DP_PHY_STATUS_GT_PLL_LOCK_MASK					0x00000010
#define DP_PHY_STATUS_RESET_LANE_0_DONE_MASK				0x00000001
#define DP_INTR_HPD_IRQ_MASK						0x00000001
#define DP_INTR_MASK							0x03A4
#define DP_DP_ENABLE							0x1
#define DP_PHY_CONFIG_GT_ALL_RESET_MASK					0x0000003
#define DP_PHY_CLOCK_SELECT						0x0234
#define DP_AUX_CLK_DIVIDER_VAL_MASK					0x000000FF
#define DP_AUX_CLK_DIVIDER						0x010C
#define DP_DISABLE							0x0
#define DP_ENABLE							0x0080
#define DP_SOFT_RESET_EN						0x1
#define DP_PHY_CONFIG							0x0200
#define DP_REPLY_STATUS_REPLY_RECEIVED_MASK				0x00000001
#define DP_REPLY_STATUS_REPLY_IN_PROGRESS_MASK				0x00000002
#define DP_REPLY_STATUS_REPLY_ERROR_MASK				0x00000008
#define DP_AUX_MAX_WAIT							20000

#define DP_DPCD_SINK_COUNT						0x00200
#define DP_DPCD_TP_SET_SCRAMB_DIS_MASK					0x20
#define DP_DPCD_STATUS_LANE_1_CR_DONE_MASK				0x10
#define DP_DPCD_STATUS_LANE_0_CR_DONE_MASK				0x01
#define DP_DPCD_STATUS_LANE_1_CE_DONE_MASK				0x20
#define DP_DPCD_STATUS_LANE_0_CE_DONE_MASK				0x02
#define DP_DPCD_STATUS_LANE_1_SL_DONE_MASK				0x40
#define DP_DPCD_STATUS_LANE_0_SL_DONE_MASK				0x04
#define DP_DPCD_LANE_ALIGN_STATUS_UPDATED_IA_DONE_MASK			0x01
#define DP_DPCD_ADJ_REQ_LANE_0_2_VS_MASK				0x03
#define DP_DPCD_ADJ_REQ_LANE_1_3_VS_MASK				0x30
#define DP_DPCD_ADJ_REQ_LANE_1_3_VS_SHIFT				4
#define DP_DPCD_ADJ_REQ_LANE_0_2_PE_MASK				0x0C
#define DP_DPCD_ADJ_REQ_LANE_0_2_PE_SHIFT				2
#define DP_DPCD_ADJ_REQ_LANE_1_3_PE_MASK				0xC0
#define DP_DPCD_ADJ_REQ_LANE_1_3_PE_SHIFT				6
#define DP_DPCD_TRAINING_LANE0_SET					0x00103
#define DP_DPCD_TRAINING_LANEX_SET_MAX_VS_MASK				0x04
#define DP_DPCD_TRAINING_LANEX_SET_MAX_PE_MASK				0x20
#define DP_DPCD_TRAINING_LANEX_SET_PE_SHIFT				3
#define DP_DPCD_SET_POWER_DP_PWR_VOLTAGE				0x00600
#define DP_DPCD_RECEIVER_CAP_FIELD_START				0x00000
#define DP_DPCD_MAX_LINK_RATE						0x00001
#define DP_DPCD_MAX_LANE_COUNT						0x00002
#define DP_DPCD_MAX_LANE_COUNT_MASK					0x1F
#define DP_DPCD_ENHANCED_FRAME_SUPPORT_MASK				0x80
#define DP_DPCD_MAX_DOWNSPREAD						0x00003
#define DP_DPCD_MAX_DOWNSPREAD_MASK					0x01
#define DP_DPCD_LANE_COUNT_SET						0x00101
#define DP_DPCD_ENHANCED_FRAME_EN_MASK					0x80
#define DP_DPCD_LINK_BW_SET						0x00100
#define DP_DPCD_DOWNSPREAD_CTRL						0x00107
#define DP_DPCD_SPREAD_AMP_MASK						0x10
#define DP_DPCD_LANE_COUNT_SET_MASK					0x1F
#define DP_DPCD_TPS3_SUPPORT_MASK					0x40
#define DP_DPCD_TRAIN_AUX_RD_INTERVAL					0x0000E
#define DP_DPCD_SINK_COUNT_HIGH_MASK					0x80
#define DP_DPCD_SINK_COUNT_HIGH_LOW_SHIFT				1
#define DP_DPCD_SINK_COUNT_LOW_MASK					0x3F
#define DP_DPCD_TP_SET							0x00102

#define SERDES_BASEADDR							0xFD400000
#define SERDES_L0_TX_MARGININGF						0x0CC0
#define SERDES_L0_TX_DEEMPHASIS						0x0048
#define SERDES_LANE_OFFSET						0x4000

#define DPDMA_TRIGGER_EN						1U
#define DPDMA_RETRIGGER_EN						2U
#define DPDMA_DESC_PREAMBLE						0xA5U
#define DPDMA_DESC_IGNR_DONE						0x400U
#define DPDMA_DESC_LAST_FRAME						0x200000U
#define DPDMA_DESCRIPTOR_LINE_SIZE_STRIDE_SHIFT				18
#define DPDMA_DESCRIPTOR_SRC_ADDR_WIDTH					32U
#define DPDMA_DESCRIPTOR_ADDR_EXT_SRC_ADDR_EXT_SHIFT			16U
#define DPDMA_CH0_DSCR_STRT_ADDR					0X0204U
#define DPDMA_CH_OFFSET							0x100U
#define DPDMA_CH0_CNTL							0x0218U
#define DPDMA_CH3_CNTL							0x0518U
#define DPDMA_CH0_DSCR_STRT_ADDRE					0x0200U
#define DPDMA_CH3_DSCR_STRT_ADDR					0x0504
#define DPDMA_CH3_DSCR_STRT_ADDRE					0x0500
#define DPDMA_CH_CNTL_EN_MASK						0x1U
#define DPDMA_CH_CNTL_PAUSE_MASK					0x2U
#define DPDMA_GBL							0x0104U
#define DPDMA_GBL_TRG_CH3_MASK						0x8
#define DPDMA_TRIGGER_DONE						0U
#define DPDMA_CH_CNTL_EN_MASK						0x1U
#define DPDMA_CH_CNTL_PAUSE_MASK					0x2U
#define DPDMA_CH_CNTL_QOS_DATA_RD_SHIFT					10U
#define DPDMA_CH_CNTL_QOS_DATA_RD_MASK					0x3C00U
#define DPDMA_CH_CNTL_QOS_DSCR_RD_SHIFT					6U
#define DPDMA_CH_CNTL_QOS_DSCR_RD_MASK					0x03C0U
#define DPDMA_CH_CNTL_QOS_DSCR_WR_SHIFT					2U
#define DPDMA_CH_CNTL_QOS_DSCR_WR_MASK					0x3CU
#define DPDMA_CH_OFFSET							0x100U
#define DPDMA_WAIT_TIMEOUT						10000U
#define DPDMA_AUDIO_ALIGNMENT						128U
#define DPDMA_VIDEO_CHANNEL0						0U
#define DPDMA_VIDEO_CHANNEL1						1U
#define DPDMA_VIDEO_CHANNEL2						2U
#define DPDMA_GRAPHICS_CHANNEL						3U
#define DPDMA_AUDIO_CHANNEL0						4U
#define DPDMA_AUDIO_CHANNEL1						5U
#define DPDMA_DESC_PREAMBLE						0xA5U
#define DPDMA_DESC_IGNR_DONE						0x400U
#define DPDMA_DESC_UPDATE						0x200U
#define DPDMA_DESC_COMP_INTR						0x100U
#define DPDMA_DESC_LAST_FRAME						0x200000U
#define DPDMA_DESC_DONE_SHIFT						31U
#define DPDMA_QOS_MIN							4U
#define DPDMA_QOS_MAX							11U
#define DPDMA_BASE_ADDRESS						0xFD4C0000
#define DPDMA_ISR							0x0004U
#define DPDMA_IEN							0x000CU
#define DPDMA_ISR_VSYNC_INT_MASK					0x08000000

#define CLK_FPD_BASEADDR						0xFD1A0000
#define VIDEO_REF_CTRL							0x00000070
#define VIDEO_REF_CTRL_SRCSEL_MASK					0x00000007
#define PLL_OUT_FREQ							1450000000
#define INPUT_FREQ_PRECISION						100
#define PRECISION							16
#define SHIFT_DECIMAL							BIT(16)
#define ENABLE_BIT							1
#define DISABLE_BIT							0
#define PLL_CTRL_BYPASS_SHIFT						3
#define PLL_CTRL_FBDIV_SHIFT						8
#define PLL_CTRL_DIV2_SHIFT						16
#define PLL_CTRL_PRE_SRC_SHIFT						20
#define PLL_CTRL							0x00000020
#define VPLL_CTRL							0x00000038
#define PLL_CFG								0x00000024
#define VPLL								2
#define VPLL_CFG							0x0000003C
#define VPLL_CFG_CP							4
#define VPLL_CFG_RES							6
#define VPLL_CFG_LFHF							3
#define VPLL_CFG_LOCK_DLY						63
#define VPLL_CFG_LOCK_CNT						600
#define PLL_STATUS_VPLL_LOCK						2
#define PLL_CFG_CP_SHIFT						5
#define PLL_CFG_RES_SHIFT						0
#define PLL_CFG_LFHF_SHIFT						10
#define PLL_CFG_LOCK_DLY_SHIFT						25
#define PLL_CFG_LOCK_CNT_SHIFT						13
#define PLL_FRAC_CFG							0x00000028
#define VPLL_FRAC_CFG							0x00000040
#define PLL_FRAC_CFG_ENABLED_SHIFT					31
#define PLL_FRAC_CFG_DATA_SHIFT						0
#define PLL_CTRL_RESET_MASK						0x00000001
#define PLL_CTRL_RESET_SHIFT						0
#define PLL_STATUS							0x00000044
#define REG_OFFSET							4
#define PLL_CTRL_BYPASS_MASK						0x00000008
#define PLL_CTRL_BYPASS_SHIFT						3
#define DOMAIN_SWITCH_CTRL						0x00000044
#define DOMAIN_SWITCH_DIVISOR0_MASK					0x00003F00
#define DOMAIN_SWITCH_DIVISOR0_SHIFT					8
#define VIDEO_REF_CTRL_CLKACT_MASK					0x01000000
#define VIDEO_REF_CTRL_CLKACT_SHIFT					24
#define VIDEO_REF_CTRL_DIVISOR1_MASK					0x003F0000
#define VIDEO_REF_CTRL_DIVISOR1_SHIFT					16
#define VIDEO_REF_CTRL_DIVISOR0_MASK					0x00003F00
#define VIDEO_REF_CTRL_DIVISOR0_SHIFT					8
#define PSS_REF_CLK							0
#define FPD_CTRL_OFFSET							12
#define VIDC_VM_NUM_SUPPORTED						2

static const u32 vs[4][4] = {
	{ 0x2a, 0x27, 0x24, 0x20 },
	{ 0x27, 0x23, 0x20, 0xff },
	{ 0x24, 0x20, 0xff, 0xff },
	{ 0xff, 0xff, 0xff, 0xff },
};

static const u32 pe[4][4] = {
	{ 0x02, 0x02, 0x02, 0x02 },
	{ 0x01, 0x01, 0x01, 0xff },
	{ 0x00, 0x00, 0xff, 0xff },
	{ 0xff, 0xff, 0xff, 0xff },
};

const struct video_timing_mode vidc_video_timing_modes[VIDC_VM_NUM_SUPPORTED] = {
	{ VIDC_VM_640x480_60_P, "640x480@60Hz", VIDC_FR_60HZ,
	{640, 16, 96, 48, 800, 0,
	 480, 10, 2, 33, 525, 0, 0, 0, 0, 0} },
	{ VIDC_VM_1024x768_60_P, "1024x768@60Hz", VIDC_FR_60HZ,
	{1024, 24, 136, 160, 1344, 0,
	 768, 3, 6, 29, 806, 0, 0, 0, 0, 0} },
};

const struct av_buf_vid_attribute avbuf_supported_formats[] = {
	/* Non-Live Graphics formats */
	{ RGBA8888, 0, INTERLEAVED,
	{AVBUF_BUF_8BIT_SF, AVBUF_BUF_8BIT_SF, AVBUF_BUF_8BIT_SF},
	 0, 1, 0, 32},
};

#endif
