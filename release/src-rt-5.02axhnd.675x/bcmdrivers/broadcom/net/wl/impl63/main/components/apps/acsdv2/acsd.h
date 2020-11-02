/*
 * ACSD shared include file
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: acsd.h 785200 2020-03-17 05:25:59Z $
 */

#ifndef _acsd_h_
#define _acsd_h_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmtimer.h>
#include <bcmendian.h>

#include <shutils.h>
#include <bcmendian.h>
#include <bcmwifi_channels.h>
#include <wlioctl.h>
#include <wlioctl_utils.h>
#include <wlutils.h>

#include <security_ipc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <shared.h>

typedef struct acs_bgdfs_info acs_bgdfs_info_t;

#include "acs_dfsr.h"

typedef enum {
	ACS_SCAN_TYPE_CS,
	ACS_SCAN_TYPE_CI
} acs_scan_type;

#define SW_NUM_SLOTS		3

#define ACSD_DEBUG_ERROR	0x0001
#define ACSD_DEBUG_WARNING	0x0002
#define ACSD_DEBUG_INFO		0x0004
#define ACSD_DEBUG_DETAIL	0x0008
#define ACSD_DEBUG_CHANIM	0x0010
#define ACSD_DEBUG_5G		0x0020
#define ACSD_DEBUG_DFSR		0x0040
#define ACSD_DEBUG_SYSLOG	0x0080

#define ACSD_VERSION		2
#define ACSD_DFLT_FD			-1
#define ACSD_DFLT_CLI_PORT	  ACSD_DEFAULT_CLI_PORT

extern bool acsd_swap;
#define htod32(i) (acsd_swap?bcmswap32(i):(uint32)(i))
#define htod16(i) (acsd_swap?bcmswap16(i):(uint16)(i))
#define dtoh64(i) (acsd_swap?bcmswap64(i):(uint64)(i))
#define dtoh32(i) (acsd_swap?bcmswap32(i):(uint32)(i))
#define dtoh16(i) (acsd_swap?bcmswap16(i):(uint16)(i))
#define htodchanspec(i) (acsd_swap?htod16(i):i)
#define dtohchanspec(i) (acsd_swap?dtoh16(i):i)
#define htodenum(i) (acsd_swap?((sizeof(i) == 4) ? \
			htod32(i) : ((sizeof(i) == 2) ? htod16(i) : i)):i)
#define dtohenum(i) (acsd_swap?((sizeof(i) == 4) ? \
			dtoh32(i) : ((sizeof(i) == 2) ? htod16(i) : i)):i)

#define acsddbg(fmt, args...) do { FILE *fp = fopen("/dev/console", "w"); if (fp) { fprintf(fp, fmt, ## args); fclose(fp); } else fprintf(stderr, fmt, ## args); } while (0)

extern int acsd_debug_level;
#define ACSD_ERROR(fmt, arg...) \
		do { \
			if (acsd_debug_level & ACSD_DEBUG_ERROR) { \
				acsddbg("ACSD >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
				if (acsd_debug_level & ACSD_DEBUG_SYSLOG) \
					logmessage("acsd", "%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
			} \
		} while (0)

#define ACSD_WARNING(fmt, arg...) \
		do { \
			if (acsd_debug_level & ACSD_DEBUG_WARNING) { \
				acsddbg("ACSD >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
				if (acsd_debug_level & ACSD_DEBUG_SYSLOG) \
					logmessage("acsd", "%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
			} \
		} while (0)

#define ACSD_INFO(fmt, arg...) \
		do { \
			if (acsd_debug_level & ACSD_DEBUG_INFO) { \
				acsddbg("ACSD >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
				if (acsd_debug_level & ACSD_DEBUG_SYSLOG) \
					logmessage("acsd", "%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
			} \
		} while (0)

#define ACSD_DEBUG(fmt, arg...) \
		do { \
			if (acsd_debug_level & ACSD_DEBUG_DETAIL) { \
				acsddbg("ACSD >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
				if (acsd_debug_level & ACSD_DEBUG_SYSLOG) \
					logmessage("acsd", "%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
			} \
		} while (0)

#define ACSD_CHANIM(fmt, arg...) \
		do { \
			if (acsd_debug_level & ACSD_DEBUG_CHANIM) { \
				acsddbg("ACSD CHANIM >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
				if (acsd_debug_level & ACSD_DEBUG_SYSLOG) \
					logmessage("acsd chanim", "%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
			} \
		} while (0)

#define ACSD_5G(fmt, arg...) \
		do { \
			if (acsd_debug_level & ACSD_DEBUG_5G) { \
				acsddbg("ACSD 5G >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
				if (acsd_debug_level & ACSD_DEBUG_SYSLOG) \
					logmessage("acsd 5g", "%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
			} \
		} while (0)

#define ACSD_DFSR(fmt, arg...) \
		do { \
			if (acsd_debug_level & ACSD_DEBUG_DFSR) { \
				acsddbg("ACSD DFSR >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
				if (acsd_debug_level & ACSD_DEBUG_SYSLOG) \
					logmessage("acsd dfsr", "%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
			} \
		} while (0)

#define ACSD_PRINT(fmt, arg...) \
		do { \
			acsddbg("acsd: "fmt, ##arg); \
			if (acsd_debug_level & ACSD_DEBUG_SYSLOG) \
				logmessage("acsd", fmt, ##arg); \
		} while (0)

#define ACSD_PRINT2(fmt, arg...) \
		do { \
			acsddbg("acsd: "fmt, ##arg); \
			if (acsd_debug_level & ACSD_DEBUG_DETAIL) { \
				if (acsd_debug_level & ACSD_DEBUG_SYSLOG) \
					logmessage("acsd", fmt, ##arg); \
			} \
		} while (0)

#define HERE ACSD_ERROR("trace\n")

#define ACS_FREE(data_ptr)	\
	do { 					\
		if (data_ptr) 		\
			free(data_ptr); \
		data_ptr = NULL; 	\
	} while (0)

#define ACS_ERR(ret, string) \
	do {	\
		if (ret < 0) {	\
			ACSD_ERROR(string "ret code: %d\n", ret); \
			return ret;	\
		} \
	} while (0)

#define SSID_FMT_BUF_LEN 4*32+1	/* Length for SSID format string */

/* other bss info derived from scan result */
typedef struct acs_chan_bssinfo {
	uint8 channel;
	uint8 nCtrl;	/* # of BSS' using this as their ctl channel */
	uint8 nExt20;	/* # of 40/80/160 MHZBSS' using this as their ext20 channel */
	uint8 nExt40;   /* # of 80/160 MHZ BSS' using this as one of their ext40 channels */
	uint8 nExt80;	/* # of 160MHZ BSS' using this as one of their ext80 channels */
} acs_chan_bssinfo_t;

typedef struct acs_channel {
	uint8 control;
	uint8 ext20;
	uint8 ext40[2];
	uint8 ext80[4];
} acs_channel_t;

typedef struct ch_score {
	int score;
	int weight;
} ch_score_t;

typedef struct cns_score {
	int highest_score;
	int lowest_score;
} cns_score_t;

#define CH_SCORE_BSS		0	/* number of bss */
#define CH_SCORE_BUSY		1	/* channel occupancy */
#define CH_SCORE_INTF		2	/* interference */
#define CH_SCORE_INTFADJ	3	/* interference adjustment, include neighboring channels */
#define CH_SCORE_FCS		4	/* FCS */
#define CH_SCORE_TXPWR		5	/* TX pwr consideration */
#define CH_SCORE_BGNOISE	6
#define CH_SCORE_TOTAL		7
#define CH_SCORE_CNS		8
/* adjacent channel score(number of bss's using the adjacent channel spec) */
#define CH_SCORE_ADJ		9
#define CH_SCORE_TXOP		10
#define CH_SCORE_DFS		11	/* DFS consideration */
#define CH_SCORE_MAX		12

#define ACS_INVALID_COEX	0x1
#define ACS_INVALID_INTF_CCA	0x2
#define ACS_INVALID_INTF_BGN	0x4
#define ACS_INVALID_OVLP	0x8
#define ACS_INVALID_NOISE	0x10
#define ACS_INVALID_ALIGN	0x20
#define ACS_INVALID_144		0x40
#define ACS_INVALID_DFS		0x80
#define ACS_INVALID_CHAN_FLOP_PERIOD	0x100
#define ACS_INVALID_EXCL		0x200
#define ACS_INVALID_MISMATCH_SB		0x400
#define ACS_INVALID_SAMECHAN		0x800
#define ACS_INVALID_DFS_NO_11H		0x1000	/* Cannot use DFS channels if 802.11h is off */
#define ACS_INVALID_LPCHAN              0x2000
#define ACS_INVALID_AVOID_PREV		0x4000	/* avoid entering prev chan before 240sec */
#define ACS_INVALID_NONDFS		0x8000	/* avoid non-dfs channel during bootup with dfs
						   enabled or during dfs re-entry
						 */

#define ACSD_BUFSIZE_4K	4096

#define ACSD_WL_CNTBUF_SIZE	2048

typedef struct ch_candidate {
	chanspec_t chspec;
	bool valid;
	bool is_dfs;
	uint16 reason;
	ch_score_t chscore[CH_SCORE_MAX];
} ch_candidate_t;

/* all the policy related configuration goes here */
extern int acs_safe_get_conf(char *outval, int outval_size, char *name);
extern void dump_networks(char *network_buf);
extern char * acsd_malloc(int bufsize);
extern char * acsd_realloc(char *buf, int bufsize);
extern void sleep_ms(const unsigned int ms);
extern int swrite(int fd, char *buf, unsigned int size);
extern int sread(int fd, char *buf, unsigned int size);
extern const char * acs_ch_score_name(const int ch_score_index);
extern void acs_dump_score(ch_score_t* score_p);
extern void acs_dump_score_csv(chanspec_t chspec, ch_score_t * score_p);
extern int wl_format_ssid(char* ssid_buf, uint8* ssid, int ssid_len);
extern char *wl_ether_etoa(const struct ether_addr *n);
extern int acs_snprintf(char *str, size_t size, const char *format, ...);
extern long uptime(void);
#endif /*  _acsd_h_ */
