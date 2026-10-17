
/*
 * The Linux nl80211 WIFI HAL device driver
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#ifndef _rdk_nl80211_priv_h_
#define _rdk_nl80211_priv_h_

#define CSI_DELAY_PERIOD		1000	/* CSI collection period: 100ms */
#define CSI_MAX_STA				5   /* Max Number of CSI STA */

/* CSI report config format: 8 32bits words, to decode the csi rec */
/* pkt_bw: word:0, bit 14, 15 */
/* 0:20 MHz, 1: 40 MHz, 2:80 MHz, 4:160 MHz */
#define CSI_CONFIG_PKTBW_IDX 0
#define CSI_CONFIG_PKTBW_LSB 14
#define CSI_CONFIG_PKTBW_MSK 0x3

/* phy_bw: word:0, bit 16, 17 */
/* 0:20 MHz, 1: 40 MHz, 2:80 MHz, 4:160 MHz */
#define CSI_CONFIG_PHYBW_IDX 0
#define CSI_CONFIG_PHYBW_LSB 16
#define CSI_CONFIG_PHYBW_MSK 0x3

/* ntx: word:0, bit 21, 22 */
#define CSI_CONFIG_NTX_IDX 0
#define CSI_CONFIG_NTX_LSB 21
#define CSI_CONFIG_NTX_MSK 0x3

/* num_tones: word:6, bit 30, 31 (as least 2 bits) and word:7, bit 0~8 (as high 9 bits) */
#define CSI_CONFIG_NUM_TONES_IDX 6
#define CSI_CONFIG_NUM_TONES_LSB 30
#define CSI_CONFIG_NUM_TONES_MSK 0x3
#define CSI_CONFIG_NUM_TONES_EXT_IDX 7
#define CSI_CONFIG_NUM_TONES_EXT_SHIFT 2
#define CSI_CONFIG_NUM_TONES_EXT_MSK 0x1FF

/* Macro to fetch the config field */
#define CSI_EXTRACT_CONFIG_WORD(a, b) \
	(((a)[CSI_CONFIG_##b##_IDX] >> CSI_CONFIG_##b##_LSB) & CSI_CONFIG_##b##_MSK)

#define CSI_EXTRACT_CONFIG_WORD_SPILL(a, b) \
	((((a)[CSI_CONFIG_##b##_IDX] >> CSI_CONFIG_##b##_LSB) & CSI_CONFIG_##b##_MSK) + \
	(((a)[CSI_CONFIG_##b##_EXT_IDX] & CSI_CONFIG_##b##_EXT_MSK) << CSI_CONFIG_##b##_EXT_SHIFT))

#define RDK_CSI_LOG_ERROR	0x0001
#define RDK_CSI_LOG_WARN	0x0002
#define RDK_CSI_LOG_INFO	0x0004
#define RDK_CSI_LOG_DBG		0x0008
#define RDK_CSI_LOG_VERBOSE	0x0010

extern u32 rdk_msglevel;
#define CFG80211_RDK_CSI		"CFG80211-RDK-CSI) "

#define RDK_WL_CSI_PRT(args) \
	do { \
		printk args; \
	} while (0)

#define RDK_WL_CSI_ERR(args) \
	do { \
		if (rdk_msglevel & RDK_CSI_LOG_ERROR) { \
			printk(KERN_INFO CFG80211_RDK_CSI "Err: %s : ", __func__);	\
			printk args; \
		} \
	}	while (0)

#define RDK_WL_CSI_WARN(args) \
	do { \
		if (rdk_msglevel & RDK_CSI_LOG_WARN) { \
			printk args; \
		} \
	}	while (0)

#define RDK_WL_CSI_INFO(args) \
	do { \
		if (rdk_msglevel & RDK_CSI_LOG_INFO) { \
			printk args; \
		} \
	}	while (0)

#define RDK_WL_CSI_DBG(args) \
	do { \
		if (rdk_msglevel & RDK_CSI_LOG_DBG) { \
			printk args; \
		} \
	}	while (0)

#define RDK_WL_CSI_VERBOSE(args) \
	do { \
		if (rdk_msglevel & RDK_CSI_LOG_VERBOSE) { \
			printk args; \
		} \
	}	while (0)

#define RDK_WL_CSI_VERBOSE_ENABLED	(rdk_msglevel & RDK_CSI_LOG_VERBOSE)

typedef struct _wifi_frame_info
{
	u8 bw_mode;            /* Bit 0-3: 0:20MHz; 1:40MHz; 2:80MHz; 3:160MHz */
				/* Bit 4: 80+80MHz */
				/* Bit 4-7: 0:11n; 1:11ac */
	u8 mcs;                /* Encoded as 11ac numbering */
	u8 Nr;                 /* Number of antennas used to receive the frame */
	u8 Nc;                 /* Number of streams used to transmit the frame */
	int32 nr_rssi[4];      /* RSSI on each of Nr */
	u16 valid_mask;        /* Bit mask */
	u16 phy_bw;            /* VAP BW at the time of capture */
	u16 cap_bw;            /* Frame BW at the time of capture */
	u32 num_sc;            /* Number of subcarriers */
	u8 decimation;         /* Value to indicate degree to which CSI matrix is decimated */
	u32 channel;           /* Primary Channel of received frame */
	int32 cfo;             /* center frequency offset */
	u64 time_stamp;        /* PHY timestamp of CSI capture with at minimum millisecond */
} wifi_frame_info_t;

typedef union bcm_csimon_hdr {
	unsigned char u8[64];
	struct {
		unsigned int format_id;			/*  Id/version */
		unsigned char client_ea[ETH_ALEN];	/*  client MAC address */
		unsigned char bss_ea[ETH_ALEN];		/*  BSS MAC address */
		unsigned short chanspec;		/*  band, channel, bandwidth */
		unsigned char txstreams;		/*  number of tx spatial streams */
		unsigned char rxstreams;		/*  number of rx spatial streams */
		unsigned int report_ts;			/*  CSI Rx TSF timer timestamp */
		unsigned int assoc_ts;			/*  TSF at association time */
		char rssi[4];				/*  RSSI for each rx chain */
		unsigned int cfo;			/* Carrier Freqency Offset */
	} data;
} bcm_csimon_hdr_t;

typedef union bcm_csi_rpt_config {
	unsigned int config[8];
} bcm_csi_rpt_config_t;

#ifdef WL_CSIMON_CUST_SENDUP
extern void dhd_linux_csimon_cb_register(dhd_linux_csimon_func_t cb);
#endif

#endif /* _rdk_nl80211_priv_h_ */
