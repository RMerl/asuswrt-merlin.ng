/*
 * Copyright © 2008 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef _DRM_DP_HELPER_H_
#define _DRM_DP_HELPER_H_

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/delay.h>

/*
 * Unless otherwise noted, all values are from the DP 1.1a spec.  Note that
 * DP and DPCD versions are independent.  Differences from 1.0 are not noted,
 * 1.0 devices basically don't exist in the wild.
 *
 * Abbreviations, in chronological order:
 *
 * eDP: Embedded DisplayPort version 1
 * DPI: DisplayPort Interoperability Guideline v1.1a
 * 1.2: DisplayPort 1.2
 * MST: Multistream Transport - part of DP 1.2a
 *
 * 1.2 formally includes both eDP and DPI definitions.
 */

#define DP_AUX_MAX_PAYLOAD_BYTES	16

#define DP_AUX_I2C_WRITE		0x0
#define DP_AUX_I2C_READ			0x1
#define DP_AUX_I2C_STATUS		0x2
#define DP_AUX_I2C_MOT			0x4
#define DP_AUX_NATIVE_WRITE		0x8
#define DP_AUX_NATIVE_READ		0x9

#define DP_AUX_NATIVE_REPLY_ACK		(0x0 << 0)
#define DP_AUX_NATIVE_REPLY_NACK	(0x1 << 0)
#define DP_AUX_NATIVE_REPLY_DEFER	(0x2 << 0)
#define DP_AUX_NATIVE_REPLY_MASK	(0x3 << 0)

#define DP_AUX_I2C_REPLY_ACK		(0x0 << 2)
#define DP_AUX_I2C_REPLY_NACK		(0x1 << 2)
#define DP_AUX_I2C_REPLY_DEFER		(0x2 << 2)
#define DP_AUX_I2C_REPLY_MASK		(0x3 << 2)

/* AUX CH addresses */
/* DPCD */
#define DP_DPCD_REV                         0x000

#define DP_MAX_LINK_RATE                    0x001

#define DP_MAX_LANE_COUNT                   0x002
# define DP_MAX_LANE_COUNT_MASK		    0x1f
# define DP_TPS3_SUPPORTED		    (1 << 6) /* 1.2 */
# define DP_ENHANCED_FRAME_CAP		    (1 << 7)

#define DP_MAX_DOWNSPREAD                   0x003
# define DP_NO_AUX_HANDSHAKE_LINK_TRAINING  (1 << 6)

#define DP_NORP                             0x004

#define DP_DOWNSTREAMPORT_PRESENT           0x005
# define DP_DWN_STRM_PORT_PRESENT           (1 << 0)
# define DP_DWN_STRM_PORT_TYPE_MASK         0x06
# define DP_DWN_STRM_PORT_TYPE_DP           (0 << 1)
# define DP_DWN_STRM_PORT_TYPE_ANALOG       (1 << 1)
# define DP_DWN_STRM_PORT_TYPE_TMDS         (2 << 1)
# define DP_DWN_STRM_PORT_TYPE_OTHER        (3 << 1)
# define DP_FORMAT_CONVERSION               (1 << 3)
# define DP_DETAILED_CAP_INFO_AVAILABLE	    (1 << 4) /* DPI */

#define DP_MAIN_LINK_CHANNEL_CODING         0x006

#define DP_DOWN_STREAM_PORT_COUNT	    0x007
# define DP_PORT_COUNT_MASK		    0x0f
# define DP_MSA_TIMING_PAR_IGNORED	    (1 << 6) /* eDP */
# define DP_OUI_SUPPORT			    (1 << 7)

#define DP_RECEIVE_PORT_0_CAP_0		    0x008
# define DP_LOCAL_EDID_PRESENT		    (1 << 1)
# define DP_ASSOCIATED_TO_PRECEDING_PORT    (1 << 2)

#define DP_RECEIVE_PORT_0_BUFFER_SIZE	    0x009

#define DP_RECEIVE_PORT_1_CAP_0		    0x00a
#define DP_RECEIVE_PORT_1_BUFFER_SIZE       0x00b

#define DP_I2C_SPEED_CAP		    0x00c    /* DPI */
# define DP_I2C_SPEED_1K		    0x01
# define DP_I2C_SPEED_5K		    0x02
# define DP_I2C_SPEED_10K		    0x04
# define DP_I2C_SPEED_100K		    0x08
# define DP_I2C_SPEED_400K		    0x10
# define DP_I2C_SPEED_1M		    0x20

#define DP_EDP_CONFIGURATION_CAP            0x00d   /* XXX 1.2? */
# define DP_ALTERNATE_SCRAMBLER_RESET_CAP   (1 << 0)
# define DP_FRAMING_CHANGE_CAP		    (1 << 1)
# define DP_DPCD_DISPLAY_CONTROL_CAPABLE     (1 << 3) /* edp v1.2 or higher */

#define DP_TRAINING_AUX_RD_INTERVAL         0x00e   /* XXX 1.2? */

#define DP_ADAPTER_CAP			    0x00f   /* 1.2 */
# define DP_FORCE_LOAD_SENSE_CAP	    (1 << 0)
# define DP_ALTERNATE_I2C_PATTERN_CAP	    (1 << 1)

#define DP_SUPPORTED_LINK_RATES		    0x010 /* eDP 1.4 */
# define DP_MAX_SUPPORTED_RATES		     8	    /* 16-bit little-endian */

/* Multiple stream transport */
#define DP_FAUX_CAP			    0x020   /* 1.2 */
# define DP_FAUX_CAP_1			    (1 << 0)

#define DP_MSTM_CAP			    0x021   /* 1.2 */
# define DP_MST_CAP			    (1 << 0)

#define DP_NUMBER_OF_AUDIO_ENDPOINTS	    0x022   /* 1.2 */

/* AV_SYNC_DATA_BLOCK                                  1.2 */
#define DP_AV_GRANULARITY		    0x023
# define DP_AG_FACTOR_MASK		    (0xf << 0)
# define DP_AG_FACTOR_3MS		    (0 << 0)
# define DP_AG_FACTOR_2MS		    (1 << 0)
# define DP_AG_FACTOR_1MS		    (2 << 0)
# define DP_AG_FACTOR_500US		    (3 << 0)
# define DP_AG_FACTOR_200US		    (4 << 0)
# define DP_AG_FACTOR_100US		    (5 << 0)
# define DP_AG_FACTOR_10US		    (6 << 0)
# define DP_AG_FACTOR_1US		    (7 << 0)
# define DP_VG_FACTOR_MASK		    (0xf << 4)
# define DP_VG_FACTOR_3MS		    (0 << 4)
# define DP_VG_FACTOR_2MS		    (1 << 4)
# define DP_VG_FACTOR_1MS		    (2 << 4)
# define DP_VG_FACTOR_500US		    (3 << 4)
# define DP_VG_FACTOR_200US		    (4 << 4)
# define DP_VG_FACTOR_100US		    (5 << 4)

#define DP_AUD_DEC_LAT0			    0x024
#define DP_AUD_DEC_LAT1			    0x025

#define DP_AUD_PP_LAT0			    0x026
#define DP_AUD_PP_LAT1			    0x027

#define DP_VID_INTER_LAT		    0x028

#define DP_VID_PROG_LAT			    0x029

#define DP_REP_LAT			    0x02a

#define DP_AUD_DEL_INS0			    0x02b
#define DP_AUD_DEL_INS1			    0x02c
#define DP_AUD_DEL_INS2			    0x02d
/* End of AV_SYNC_DATA_BLOCK */

#define DP_RECEIVER_ALPM_CAP		    0x02e   /* eDP 1.4 */
# define DP_ALPM_CAP			    (1 << 0)

#define DP_SINK_DEVICE_AUX_FRAME_SYNC_CAP   0x02f   /* eDP 1.4 */
# define DP_AUX_FRAME_SYNC_CAP		    (1 << 0)

#define DP_GUID				    0x030   /* 1.2 */

#define DP_PSR_SUPPORT                      0x070   /* XXX 1.2? */
# define DP_PSR_IS_SUPPORTED                1
# define DP_PSR2_IS_SUPPORTED		    2	    /* eDP 1.4 */

#define DP_PSR_CAPS                         0x071   /* XXX 1.2? */
# define DP_PSR_NO_TRAIN_ON_EXIT            1
# define DP_PSR_SETUP_TIME_330              (0 << 1)
# define DP_PSR_SETUP_TIME_275              (1 << 1)
# define DP_PSR_SETUP_TIME_220              (2 << 1)
# define DP_PSR_SETUP_TIME_165              (3 << 1)
# define DP_PSR_SETUP_TIME_110              (4 << 1)
# define DP_PSR_SETUP_TIME_55               (5 << 1)
# define DP_PSR_SETUP_TIME_0                (6 << 1)
# define DP_PSR_SETUP_TIME_MASK             (7 << 1)
# define DP_PSR_SETUP_TIME_SHIFT            1

/*
 * 0x80-0x8f describe downstream port capabilities, but there are two layouts
 * based on whether DP_DETAILED_CAP_INFO_AVAILABLE was set.  If it was not,
 * each port's descriptor is one byte wide.  If it was set, each port's is
 * four bytes wide, starting with the one byte from the base info.  As of
 * DP interop v1.1a only VGA defines additional detail.
 */

/* offset 0 */
#define DP_DOWNSTREAM_PORT_0		    0x80
# define DP_DS_PORT_TYPE_MASK		    (7 << 0)
# define DP_DS_PORT_TYPE_DP		    0
# define DP_DS_PORT_TYPE_VGA		    1
# define DP_DS_PORT_TYPE_DVI		    2
# define DP_DS_PORT_TYPE_HDMI		    3
# define DP_DS_PORT_TYPE_NON_EDID	    4
# define DP_DS_PORT_HPD			    (1 << 3)
/* offset 1 for VGA is maximum megapixels per second / 8 */
/* offset 2 */
# define DP_DS_VGA_MAX_BPC_MASK		    (3 << 0)
# define DP_DS_VGA_8BPC			    0
# define DP_DS_VGA_10BPC		    1
# define DP_DS_VGA_12BPC		    2
# define DP_DS_VGA_16BPC		    3

/* link configuration */
#define	DP_LINK_BW_SET		            0x100
# define DP_LINK_RATE_TABLE		    0x00    /* eDP 1.4 */
# define DP_LINK_BW_1_62		    0x06
# define DP_LINK_BW_2_7			    0x0a
# define DP_LINK_BW_5_4			    0x14    /* 1.2 */

#define DP_LANE_COUNT_SET	            0x101
# define DP_LANE_COUNT_MASK		    0x0f
# define DP_LANE_COUNT_ENHANCED_FRAME_EN    (1 << 7)

#define DP_TRAINING_PATTERN_SET	            0x102
# define DP_TRAINING_PATTERN_DISABLE	    0
# define DP_TRAINING_PATTERN_1		    1
# define DP_TRAINING_PATTERN_2		    2
# define DP_TRAINING_PATTERN_3		    3	    /* 1.2 */
# define DP_TRAINING_PATTERN_MASK	    0x3

/* DPCD 1.1 only. For DPCD >= 1.2 see per-lane DP_LINK_QUAL_LANEn_SET */
# define DP_LINK_QUAL_PATTERN_11_DISABLE    (0 << 2)
# define DP_LINK_QUAL_PATTERN_11_D10_2	    (1 << 2)
# define DP_LINK_QUAL_PATTERN_11_ERROR_RATE (2 << 2)
# define DP_LINK_QUAL_PATTERN_11_PRBS7	    (3 << 2)
# define DP_LINK_QUAL_PATTERN_11_MASK	    (3 << 2)

# define DP_RECOVERED_CLOCK_OUT_EN	    (1 << 4)
# define DP_LINK_SCRAMBLING_DISABLE	    (1 << 5)

# define DP_SYMBOL_ERROR_COUNT_BOTH	    (0 << 6)
# define DP_SYMBOL_ERROR_COUNT_DISPARITY    (1 << 6)
# define DP_SYMBOL_ERROR_COUNT_SYMBOL	    (2 << 6)
# define DP_SYMBOL_ERROR_COUNT_MASK	    (3 << 6)

#define DP_TRAINING_LANE0_SET		    0x103
#define DP_TRAINING_LANE1_SET		    0x104
#define DP_TRAINING_LANE2_SET		    0x105
#define DP_TRAINING_LANE3_SET		    0x106

# define DP_TRAIN_VOLTAGE_SWING_MASK	    0x3
# define DP_TRAIN_VOLTAGE_SWING_SHIFT	    0
# define DP_TRAIN_MAX_SWING_REACHED	    (1 << 2)
# define DP_TRAIN_VOLTAGE_SWING_LEVEL_0 (0 << 0)
# define DP_TRAIN_VOLTAGE_SWING_LEVEL_1 (1 << 0)
# define DP_TRAIN_VOLTAGE_SWING_LEVEL_2 (2 << 0)
# define DP_TRAIN_VOLTAGE_SWING_LEVEL_3 (3 << 0)

# define DP_TRAIN_PRE_EMPHASIS_MASK	    (3 << 3)
# define DP_TRAIN_PRE_EMPH_LEVEL_0		(0 << 3)
# define DP_TRAIN_PRE_EMPH_LEVEL_1		(1 << 3)
# define DP_TRAIN_PRE_EMPH_LEVEL_2		(2 << 3)
# define DP_TRAIN_PRE_EMPH_LEVEL_3		(3 << 3)

# define DP_TRAIN_PRE_EMPHASIS_SHIFT	    3
# define DP_TRAIN_MAX_PRE_EMPHASIS_REACHED  (1 << 5)

#define DP_DOWNSPREAD_CTRL		    0x107
# define DP_SPREAD_AMP_0_5		    (1 << 4)
# define DP_MSA_TIMING_PAR_IGNORE_EN	    (1 << 7) /* eDP */

#define DP_MAIN_LINK_CHANNEL_CODING_SET	    0x108
# define DP_SET_ANSI_8B10B		    (1 << 0)

#define DP_I2C_SPEED_CONTROL_STATUS	    0x109   /* DPI */
/* bitmask as for DP_I2C_SPEED_CAP */

#define DP_EDP_CONFIGURATION_SET            0x10a   /* XXX 1.2? */
# define DP_ALTERNATE_SCRAMBLER_RESET_ENABLE (1 << 0)
# define DP_FRAMING_CHANGE_ENABLE	    (1 << 1)
# define DP_PANEL_SELF_TEST_ENABLE	    (1 << 7)

#define DP_LINK_QUAL_LANE0_SET		    0x10b   /* DPCD >= 1.2 */
#define DP_LINK_QUAL_LANE1_SET		    0x10c
#define DP_LINK_QUAL_LANE2_SET		    0x10d
#define DP_LINK_QUAL_LANE3_SET		    0x10e
# define DP_LINK_QUAL_PATTERN_DISABLE	    0
# define DP_LINK_QUAL_PATTERN_D10_2	    1
# define DP_LINK_QUAL_PATTERN_ERROR_RATE    2
# define DP_LINK_QUAL_PATTERN_PRBS7	    3
# define DP_LINK_QUAL_PATTERN_80BIT_CUSTOM  4
# define DP_LINK_QUAL_PATTERN_HBR2_EYE      5
# define DP_LINK_QUAL_PATTERN_MASK	    7

#define DP_TRAINING_LANE0_1_SET2	    0x10f
#define DP_TRAINING_LANE2_3_SET2	    0x110
# define DP_LANE02_POST_CURSOR2_SET_MASK    (3 << 0)
# define DP_LANE02_MAX_POST_CURSOR2_REACHED (1 << 2)
# define DP_LANE13_POST_CURSOR2_SET_MASK    (3 << 4)
# define DP_LANE13_MAX_POST_CURSOR2_REACHED (1 << 6)

#define DP_MSTM_CTRL			    0x111   /* 1.2 */
# define DP_MST_EN			    (1 << 0)
# define DP_UP_REQ_EN			    (1 << 1)
# define DP_UPSTREAM_IS_SRC		    (1 << 2)

#define DP_AUDIO_DELAY0			    0x112   /* 1.2 */
#define DP_AUDIO_DELAY1			    0x113
#define DP_AUDIO_DELAY2			    0x114

#define DP_LINK_RATE_SET		    0x115   /* eDP 1.4 */
# define DP_LINK_RATE_SET_SHIFT		    0
# define DP_LINK_RATE_SET_MASK		    (7 << 0)

#define DP_RECEIVER_ALPM_CONFIG		    0x116   /* eDP 1.4 */
# define DP_ALPM_ENABLE			    (1 << 0)
# define DP_ALPM_LOCK_ERROR_IRQ_HPD_ENABLE  (1 << 1)

#define DP_SINK_DEVICE_AUX_FRAME_SYNC_CONF  0x117   /* eDP 1.4 */
# define DP_AUX_FRAME_SYNC_ENABLE	    (1 << 0)
# define DP_IRQ_HPD_ENABLE		    (1 << 1)

#define DP_UPSTREAM_DEVICE_DP_PWR_NEED	    0x118   /* 1.2 */
# define DP_PWR_NOT_NEEDED		    (1 << 0)

#define DP_AUX_FRAME_SYNC_VALUE		    0x15c   /* eDP 1.4 */
# define DP_AUX_FRAME_SYNC_VALID	    (1 << 0)

#define DP_PSR_EN_CFG			    0x170   /* XXX 1.2? */
# define DP_PSR_ENABLE			    (1 << 0)
# define DP_PSR_MAIN_LINK_ACTIVE	    (1 << 1)
# define DP_PSR_CRC_VERIFICATION	    (1 << 2)
# define DP_PSR_FRAME_CAPTURE		    (1 << 3)
# define DP_PSR_SELECTIVE_UPDATE	    (1 << 4)
# define DP_PSR_IRQ_HPD_WITH_CRC_ERRORS     (1 << 5)

#define DP_ADAPTER_CTRL			    0x1a0
# define DP_ADAPTER_CTRL_FORCE_LOAD_SENSE   (1 << 0)

#define DP_BRANCH_DEVICE_CTRL		    0x1a1
# define DP_BRANCH_DEVICE_IRQ_HPD	    (1 << 0)

#define DP_PAYLOAD_ALLOCATE_SET		    0x1c0
#define DP_PAYLOAD_ALLOCATE_START_TIME_SLOT 0x1c1
#define DP_PAYLOAD_ALLOCATE_TIME_SLOT_COUNT 0x1c2

#define DP_SINK_COUNT			    0x200
/* prior to 1.2 bit 7 was reserved mbz */
# define DP_GET_SINK_COUNT(x)		    ((((x) & 0x80) >> 1) | ((x) & 0x3f))
# define DP_SINK_CP_READY		    (1 << 6)

#define DP_DEVICE_SERVICE_IRQ_VECTOR	    0x201
# define DP_REMOTE_CONTROL_COMMAND_PENDING  (1 << 0)
# define DP_AUTOMATED_TEST_REQUEST	    (1 << 1)
# define DP_CP_IRQ			    (1 << 2)
# define DP_MCCS_IRQ			    (1 << 3)
# define DP_DOWN_REP_MSG_RDY		    (1 << 4) /* 1.2 MST */
# define DP_UP_REQ_MSG_RDY		    (1 << 5) /* 1.2 MST */
# define DP_SINK_SPECIFIC_IRQ		    (1 << 6)

#define DP_LANE0_1_STATUS		    0x202
#define DP_LANE2_3_STATUS		    0x203
# define DP_LANE_CR_DONE		    (1 << 0)
# define DP_LANE_CHANNEL_EQ_DONE	    (1 << 1)
# define DP_LANE_SYMBOL_LOCKED		    (1 << 2)

#define DP_CHANNEL_EQ_BITS (DP_LANE_CR_DONE |		\
			    DP_LANE_CHANNEL_EQ_DONE |	\
			    DP_LANE_SYMBOL_LOCKED)

#define DP_LANE_ALIGN_STATUS_UPDATED	    0x204

#define DP_INTERLANE_ALIGN_DONE		    (1 << 0)
#define DP_DOWNSTREAM_PORT_STATUS_CHANGED   (1 << 6)
#define DP_LINK_STATUS_UPDATED		    (1 << 7)

#define DP_SINK_STATUS			    0x205

#define DP_RECEIVE_PORT_0_STATUS	    (1 << 0)
#define DP_RECEIVE_PORT_1_STATUS	    (1 << 1)

#define DP_ADJUST_REQUEST_LANE0_1	    0x206
#define DP_ADJUST_REQUEST_LANE2_3	    0x207
# define DP_ADJUST_VOLTAGE_SWING_LANE0_MASK  0x03
# define DP_ADJUST_VOLTAGE_SWING_LANE0_SHIFT 0
# define DP_ADJUST_PRE_EMPHASIS_LANE0_MASK   0x0c
# define DP_ADJUST_PRE_EMPHASIS_LANE0_SHIFT  2
# define DP_ADJUST_VOLTAGE_SWING_LANE1_MASK  0x30
# define DP_ADJUST_VOLTAGE_SWING_LANE1_SHIFT 4
# define DP_ADJUST_PRE_EMPHASIS_LANE1_MASK   0xc0
# define DP_ADJUST_PRE_EMPHASIS_LANE1_SHIFT  6

#define DP_TEST_REQUEST			    0x218
# define DP_TEST_LINK_TRAINING		    (1 << 0)
# define DP_TEST_LINK_VIDEO_PATTERN	    (1 << 1)
# define DP_TEST_LINK_EDID_READ		    (1 << 2)
# define DP_TEST_LINK_PHY_TEST_PATTERN	    (1 << 3) /* DPCD >= 1.1 */
# define DP_TEST_LINK_FAUX_PATTERN	    (1 << 4) /* DPCD >= 1.2 */

#define DP_TEST_LINK_RATE		    0x219
# define DP_LINK_RATE_162		    (0x6)
# define DP_LINK_RATE_27		    (0xa)

#define DP_TEST_LANE_COUNT		    0x220

#define DP_TEST_PATTERN			    0x221

#define DP_TEST_CRC_R_CR		    0x240
#define DP_TEST_CRC_G_Y			    0x242
#define DP_TEST_CRC_B_CB		    0x244

#define DP_TEST_SINK_MISC		    0x246
# define DP_TEST_CRC_SUPPORTED		    (1 << 5)
# define DP_TEST_COUNT_MASK		    0x7

#define DP_TEST_RESPONSE		    0x260
# define DP_TEST_ACK			    (1 << 0)
# define DP_TEST_NAK			    (1 << 1)
# define DP_TEST_EDID_CHECKSUM_WRITE	    (1 << 2)

#define DP_TEST_EDID_CHECKSUM		    0x261

#define DP_TEST_SINK			    0x270
# define DP_TEST_SINK_START		    (1 << 0)

#define DP_PAYLOAD_TABLE_UPDATE_STATUS      0x2c0   /* 1.2 MST */
# define DP_PAYLOAD_TABLE_UPDATED           (1 << 0)
# define DP_PAYLOAD_ACT_HANDLED             (1 << 1)

#define DP_VC_PAYLOAD_ID_SLOT_1             0x2c1   /* 1.2 MST */
/* up to ID_SLOT_63 at 0x2ff */

#define DP_SOURCE_OUI			    0x300
#define DP_SINK_OUI			    0x400
#define DP_BRANCH_OUI			    0x500

#define DP_SET_POWER                        0x600
# define DP_SET_POWER_D0                    0x1
# define DP_SET_POWER_D3                    0x2
# define DP_SET_POWER_MASK                  0x3

#define DP_EDP_DPCD_REV			    0x700    /* eDP 1.2 */
# define DP_EDP_11			    0x00
# define DP_EDP_12			    0x01
# define DP_EDP_13			    0x02
# define DP_EDP_14			    0x03

#define DP_EDP_GENERAL_CAP_1		    0x701

#define DP_EDP_BACKLIGHT_ADJUSTMENT_CAP     0x702

#define DP_EDP_GENERAL_CAP_2		    0x703

#define DP_EDP_GENERAL_CAP_3		    0x704    /* eDP 1.4 */

#define DP_EDP_DISPLAY_CONTROL_REGISTER     0x720

#define DP_EDP_BACKLIGHT_MODE_SET_REGISTER  0x721

#define DP_EDP_BACKLIGHT_BRIGHTNESS_MSB     0x722
#define DP_EDP_BACKLIGHT_BRIGHTNESS_LSB     0x723

#define DP_EDP_PWMGEN_BIT_COUNT             0x724
#define DP_EDP_PWMGEN_BIT_COUNT_CAP_MIN     0x725
#define DP_EDP_PWMGEN_BIT_COUNT_CAP_MAX     0x726

#define DP_EDP_BACKLIGHT_CONTROL_STATUS     0x727

#define DP_EDP_BACKLIGHT_FREQ_SET           0x728

#define DP_EDP_BACKLIGHT_FREQ_CAP_MIN_MSB   0x72a
#define DP_EDP_BACKLIGHT_FREQ_CAP_MIN_MID   0x72b
#define DP_EDP_BACKLIGHT_FREQ_CAP_MIN_LSB   0x72c

#define DP_EDP_BACKLIGHT_FREQ_CAP_MAX_MSB   0x72d
#define DP_EDP_BACKLIGHT_FREQ_CAP_MAX_MID   0x72e
#define DP_EDP_BACKLIGHT_FREQ_CAP_MAX_LSB   0x72f

#define DP_EDP_DBC_MINIMUM_BRIGHTNESS_SET   0x732
#define DP_EDP_DBC_MAXIMUM_BRIGHTNESS_SET   0x733

#define DP_EDP_REGIONAL_BACKLIGHT_BASE      0x740    /* eDP 1.4 */
#define DP_EDP_REGIONAL_BACKLIGHT_0	    0x741    /* eDP 1.4 */

#define DP_SIDEBAND_MSG_DOWN_REQ_BASE	    0x1000   /* 1.2 MST */
#define DP_SIDEBAND_MSG_UP_REP_BASE	    0x1200   /* 1.2 MST */
#define DP_SIDEBAND_MSG_DOWN_REP_BASE	    0x1400   /* 1.2 MST */
#define DP_SIDEBAND_MSG_UP_REQ_BASE	    0x1600   /* 1.2 MST */

#define DP_SINK_COUNT_ESI		    0x2002   /* 1.2 */
/* 0-5 sink count */
# define DP_SINK_COUNT_CP_READY             (1 << 6)

#define DP_DEVICE_SERVICE_IRQ_VECTOR_ESI0   0x2003   /* 1.2 */

#define DP_DEVICE_SERVICE_IRQ_VECTOR_ESI1   0x2004   /* 1.2 */

#define DP_LINK_SERVICE_IRQ_VECTOR_ESI0     0x2005   /* 1.2 */

#define DP_PSR_ERROR_STATUS                 0x2006  /* XXX 1.2? */
# define DP_PSR_LINK_CRC_ERROR              (1 << 0)
# define DP_PSR_RFB_STORAGE_ERROR           (1 << 1)
# define DP_PSR_VSC_SDP_UNCORRECTABLE_ERROR (1 << 2) /* eDP 1.4 */

#define DP_PSR_ESI                          0x2007  /* XXX 1.2? */
# define DP_PSR_CAPS_CHANGE                 (1 << 0)

#define DP_PSR_STATUS                       0x2008  /* XXX 1.2? */
# define DP_PSR_SINK_INACTIVE               0
# define DP_PSR_SINK_ACTIVE_SRC_SYNCED      1
# define DP_PSR_SINK_ACTIVE_RFB             2
# define DP_PSR_SINK_ACTIVE_SINK_SYNCED     3
# define DP_PSR_SINK_ACTIVE_RESYNC          4
# define DP_PSR_SINK_INTERNAL_ERROR         7
# define DP_PSR_SINK_STATE_MASK             0x07

#define DP_RECEIVER_ALPM_STATUS		    0x200b  /* eDP 1.4 */
# define DP_ALPM_LOCK_TIMEOUT_ERROR	    (1 << 0)

/* DP 1.2 Sideband message defines */
/* peer device type - DP 1.2a Table 2-92 */
#define DP_PEER_DEVICE_NONE		0x0
#define DP_PEER_DEVICE_SOURCE_OR_SST	0x1
#define DP_PEER_DEVICE_MST_BRANCHING	0x2
#define DP_PEER_DEVICE_SST_SINK		0x3
#define DP_PEER_DEVICE_DP_LEGACY_CONV	0x4

/* DP 1.2 MST sideband request names DP 1.2a Table 2-80 */
#define DP_LINK_ADDRESS			0x01
#define DP_CONNECTION_STATUS_NOTIFY	0x02
#define DP_ENUM_PATH_RESOURCES		0x10
#define DP_ALLOCATE_PAYLOAD		0x11
#define DP_QUERY_PAYLOAD		0x12
#define DP_RESOURCE_STATUS_NOTIFY	0x13
#define DP_CLEAR_PAYLOAD_ID_TABLE	0x14
#define DP_REMOTE_DPCD_READ		0x20
#define DP_REMOTE_DPCD_WRITE		0x21
#define DP_REMOTE_I2C_READ		0x22
#define DP_REMOTE_I2C_WRITE		0x23
#define DP_POWER_UP_PHY			0x24
#define DP_POWER_DOWN_PHY		0x25
#define DP_SINK_EVENT_NOTIFY		0x30
#define DP_QUERY_STREAM_ENC_STATUS	0x38

/* DP 1.2 MST sideband nak reasons - table 2.84 */
#define DP_NAK_WRITE_FAILURE		0x01
#define DP_NAK_INVALID_READ		0x02
#define DP_NAK_CRC_FAILURE		0x03
#define DP_NAK_BAD_PARAM		0x04
#define DP_NAK_DEFER			0x05
#define DP_NAK_LINK_FAILURE		0x06
#define DP_NAK_NO_RESOURCES		0x07
#define DP_NAK_DPCD_FAIL		0x08
#define DP_NAK_I2C_NAK			0x09
#define DP_NAK_ALLOCATE_FAIL		0x0a

#define MODE_I2C_START	1
#define MODE_I2C_WRITE	2
#define MODE_I2C_READ	4
#define MODE_I2C_STOP	8

/* DP 1.2 MST PORTs - Section 2.5.1 v1.2a spec */
#define DP_MST_PHYSICAL_PORT_0 0
#define DP_MST_LOGICAL_PORT_0 8

#define DP_LINK_STATUS_SIZE	   6
bool drm_dp_channel_eq_ok(const u8 link_status[DP_LINK_STATUS_SIZE],
			  int lane_count);
bool drm_dp_clock_recovery_ok(const u8 link_status[DP_LINK_STATUS_SIZE],
			      int lane_count);
u8 drm_dp_get_adjust_request_voltage(const u8 link_status[DP_LINK_STATUS_SIZE],
				     int lane);
u8 drm_dp_get_adjust_request_pre_emphasis(const u8 link_status[DP_LINK_STATUS_SIZE],
					  int lane);

#define DP_RECEIVER_CAP_SIZE		0xf
#define EDP_PSR_RECEIVER_CAP_SIZE	2

void drm_dp_link_train_clock_recovery_delay(const u8 dpcd[DP_RECEIVER_CAP_SIZE]);
void drm_dp_link_train_channel_eq_delay(const u8 dpcd[DP_RECEIVER_CAP_SIZE]);

u8 drm_dp_link_rate_to_bw_code(int link_rate);
int drm_dp_bw_code_to_link_rate(u8 link_bw);

struct edp_sdp_header {
	u8 HB0; /* Secondary Data Packet ID */
	u8 HB1; /* Secondary Data Packet Type */
	u8 HB2; /* 7:5 reserved, 4:0 revision number */
	u8 HB3; /* 7:5 reserved, 4:0 number of valid data bytes */
} __packed;

#define EDP_SDP_HEADER_REVISION_MASK		0x1F
#define EDP_SDP_HEADER_VALID_PAYLOAD_BYTES	0x1F

struct edp_vsc_psr {
	struct edp_sdp_header sdp_header;
	u8 DB0; /* Stereo Interface */
	u8 DB1; /* 0 - PSR State; 1 - Update RFB; 2 - CRC Valid */
	u8 DB2; /* CRC value bits 7:0 of the R or Cr component */
	u8 DB3; /* CRC value bits 15:8 of the R or Cr component */
	u8 DB4; /* CRC value bits 7:0 of the G or Y component */
	u8 DB5; /* CRC value bits 15:8 of the G or Y component */
	u8 DB6; /* CRC value bits 7:0 of the B or Cb component */
	u8 DB7; /* CRC value bits 15:8 of the B or Cb component */
	u8 DB8_31[24]; /* Reserved */
} __packed;

#define EDP_VSC_PSR_STATE_ACTIVE	(1<<0)
#define EDP_VSC_PSR_UPDATE_RFB		(1<<1)
#define EDP_VSC_PSR_CRC_VALUES_VALID	(1<<2)

static inline int
drm_dp_max_link_rate(const u8 dpcd[DP_RECEIVER_CAP_SIZE])
{
	return drm_dp_bw_code_to_link_rate(dpcd[DP_MAX_LINK_RATE]);
}

static inline u8
drm_dp_max_lane_count(const u8 dpcd[DP_RECEIVER_CAP_SIZE])
{
	return dpcd[DP_MAX_LANE_COUNT] & DP_MAX_LANE_COUNT_MASK;
}

static inline bool
drm_dp_enhanced_frame_cap(const u8 dpcd[DP_RECEIVER_CAP_SIZE])
{
	return dpcd[DP_DPCD_REV] >= 0x11 &&
		(dpcd[DP_MAX_LANE_COUNT] & DP_ENHANCED_FRAME_CAP);
}

/*
 * DisplayPort AUX channel
 */

/**
 * struct drm_dp_aux_msg - DisplayPort AUX channel transaction
 * @address: address of the (first) register to access
 * @request: contains the type of transaction (see DP_AUX_* macros)
 * @reply: upon completion, contains the reply type of the transaction
 * @buffer: pointer to a transmission or reception buffer
 * @size: size of @buffer
 */
struct drm_dp_aux_msg {
	unsigned int address;
	u8 request;
	u8 reply;
	void *buffer;
	size_t size;
};

/**
 * struct drm_dp_aux - DisplayPort AUX channel
 * @name: user-visible name of this AUX channel and the I2C-over-AUX adapter
 * @ddc: I2C adapter that can be used for I2C-over-AUX communication
 * @dev: pointer to struct device that is the parent for this AUX channel
 * @hw_mutex: internal mutex used for locking transfers
 * @transfer: transfers a message representing a single AUX transaction
 *
 * The .dev field should be set to a pointer to the device that implements
 * the AUX channel.
 *
 * The .name field may be used to specify the name of the I2C adapter. If set to
 * NULL, dev_name() of .dev will be used.
 *
 * Drivers provide a hardware-specific implementation of how transactions
 * are executed via the .transfer() function. A pointer to a drm_dp_aux_msg
 * structure describing the transaction is passed into this function. Upon
 * success, the implementation should return the number of payload bytes
 * that were transferred, or a negative error-code on failure. Helpers
 * propagate errors from the .transfer() function, with the exception of
 * the -EBUSY error, which causes a transaction to be retried. On a short,
 * helpers will return -EPROTO to make it simpler to check for failure.
 *
 * An AUX channel can also be used to transport I2C messages to a sink. A
 * typical application of that is to access an EDID that's present in the
 * sink device. The .transfer() function can also be used to execute such
 * transactions. The drm_dp_aux_register_i2c_bus() function registers an
 * I2C adapter that can be passed to drm_probe_ddc(). Upon removal, drivers
 * should call drm_dp_aux_unregister_i2c_bus() to remove the I2C adapter.
 * The I2C adapter uses long transfers by default; if a partial response is
 * received, the adapter will drop down to the size given by the partial
 * response for this transaction only.
 *
 * Note that the aux helper code assumes that the .transfer() function
 * only modifies the reply field of the drm_dp_aux_msg structure.  The
 * retry logic and i2c helpers assume this is the case.
 */
struct drm_dp_aux {
	const char *name;
	struct i2c_adapter ddc;
	struct device *dev;
	struct mutex hw_mutex;
	ssize_t (*transfer)(struct drm_dp_aux *aux,
			    struct drm_dp_aux_msg *msg);
	unsigned i2c_nack_count, i2c_defer_count;
};

ssize_t drm_dp_dpcd_read(struct drm_dp_aux *aux, unsigned int offset,
			 void *buffer, size_t size);
ssize_t drm_dp_dpcd_write(struct drm_dp_aux *aux, unsigned int offset,
			  void *buffer, size_t size);

/**
 * drm_dp_dpcd_readb() - read a single byte from the DPCD
 * @aux: DisplayPort AUX channel
 * @offset: address of the register to read
 * @valuep: location where the value of the register will be stored
 *
 * Returns the number of bytes transferred (1) on success, or a negative
 * error code on failure.
 */
static inline ssize_t drm_dp_dpcd_readb(struct drm_dp_aux *aux,
					unsigned int offset, u8 *valuep)
{
	return drm_dp_dpcd_read(aux, offset, valuep, 1);
}

/**
 * drm_dp_dpcd_writeb() - write a single byte to the DPCD
 * @aux: DisplayPort AUX channel
 * @offset: address of the register to write
 * @value: value to write to the register
 *
 * Returns the number of bytes transferred (1) on success, or a negative
 * error code on failure.
 */
static inline ssize_t drm_dp_dpcd_writeb(struct drm_dp_aux *aux,
					 unsigned int offset, u8 value)
{
	return drm_dp_dpcd_write(aux, offset, &value, 1);
}

int drm_dp_dpcd_read_link_status(struct drm_dp_aux *aux,
				 u8 status[DP_LINK_STATUS_SIZE]);

/*
 * DisplayPort link
 */
#define DP_LINK_CAP_ENHANCED_FRAMING (1 << 0)

struct drm_dp_link {
	unsigned char revision;
	unsigned int rate;
	unsigned int num_lanes;
	unsigned long capabilities;
};

int drm_dp_link_probe(struct drm_dp_aux *aux, struct drm_dp_link *link);
int drm_dp_link_power_up(struct drm_dp_aux *aux, struct drm_dp_link *link);
int drm_dp_link_power_down(struct drm_dp_aux *aux, struct drm_dp_link *link);
int drm_dp_link_configure(struct drm_dp_aux *aux, struct drm_dp_link *link);

int drm_dp_aux_register(struct drm_dp_aux *aux);
void drm_dp_aux_unregister(struct drm_dp_aux *aux);

#endif /* _DRM_DP_HELPER_H_ */
