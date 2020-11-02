/*
 * ESCAND shared include file
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
 * $Id: escand.h 766308 2018-07-30 09:35:53Z $
 */

#ifndef _escand_h_
#define _escand_h_

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

typedef enum {
	ESCAND_SCAN_TYPE_CS,
	ESCAND_SCAN_TYPE_CI
} escand_scan_type;

#define SW_NUM_SLOTS		10

#define ESCAND_DEBUG_ERROR	0x0001
#define ESCAND_DEBUG_WARNING	0x0002
#define ESCAND_DEBUG_INFO		0x0004
#define ESCAND_DEBUG_DETAIL	0x0008
#define ESCAND_DEBUG_5G		0x0020
#define ESCAND_DEBUG_DFSR		0x0040

#define ESCAND_VERSION 1
#define ESCAND_DFLT_FD			-1
#define ESCAND_DFLT_CLI_PORT	  ESCAND_DEFAULT_CLI_PORT

extern bool escand_swap;
#define htod32(i) (escand_swap?bcmswap32(i):(uint32)(i))
#define htod16(i) (escand_swap?bcmswap16(i):(uint16)(i))
#define dtoh64(i) (escand_swap?bcmswap64(i):(uint64)(i))
#define dtoh32(i) (escand_swap?bcmswap32(i):(uint32)(i))
#define dtoh16(i) (escand_swap?bcmswap16(i):(uint16)(i))
#define htodchanspec(i) (escand_swap?htod16(i):i)
#define dtohchanspec(i) (escand_swap?dtoh16(i):i)
#define htodenum(i) (escand_swap?((sizeof(i) == 4) ? \
			htod32(i) : ((sizeof(i) == 2) ? htod16(i) : i)):i)
#define dtohenum(i) (escand_swap?((sizeof(i) == 4) ? \
			dtoh32(i) : ((sizeof(i) == 2) ? htod16(i) : i)):i)

extern int escand_debug_level;
#define ESCAND_ERROR(fmt, arg...) \
		do { if (escand_debug_level & ESCAND_DEBUG_ERROR) \
			printf("ESCAND >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); } while (0)

#define ESCAND_WARNING(fmt, arg...) \
		do { if (escand_debug_level & ESCAND_DEBUG_WARNING) \
			printf("ESCAND >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); } while (0)

#define ESCAND_INFO(fmt, arg...) \
		do { if (escand_debug_level & ESCAND_DEBUG_INFO) \
			printf("ESCAND >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); } while (0)

#define ESCAND_DEBUG(fmt, arg...) \
		do { if (escand_debug_level & ESCAND_DEBUG_DETAIL) \
			printf("ESCAND >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); } while (0)

#define ESCAND_5G(fmt, arg...) \
		do { if (escand_debug_level & ESCAND_DEBUG_5G) \
			printf(fmt, ##arg); } while (0)

#define ESCAND_PRINT(fmt, arg...) \
		do { printf("escand: "fmt, ##arg); } while (0)

#define HERE ESCAND_ERROR("trace\n")

#define ESCAND_FREE(data_ptr)	\
	do { 					\
		if (data_ptr) 		\
			free(data_ptr); \
		data_ptr = NULL; 	\
	} while (0)

#define ESCAND_ERR(ret, string) \
	do {	\
		if (ret < 0) {	\
			ESCAND_ERROR(string "ret code: %d\n", ret); \
			return ret;	\
		} \
	} while (0)

#define SSID_FMT_BUF_LEN 4*32+1	/* Length for SSID format string */

/* other bss info derived from scan result */
typedef struct escand_chan_bssinfo {
	uint8 channel;
	uint8 nCtrl;	/* # of BSS' using this as their ctl channel */
	uint8 nExt20;	/* # of 40/80/160 MHZBSS' using this as their ext20 channel */
	uint8 nExt40;   /* # of 80/160 MHZ BSS' using this as one of their ext40 channels */
	uint8 nExt80;	/* # of 160MHZ BSS' using this as one of their ext80 channels */
} escand_chan_bssinfo_t;

typedef struct escand_channel {
	uint8 control;
	uint8 ext20;
	uint8 ext40[2];
	uint8 ext80[4];
} escand_channel_t;

#define ESCAND_INVALID_COEX	0x1
#define ESCAND_INVALID_INTF_CCA	0x2
#define ESCAND_INVALID_INTF_BGN	0x4
#define ESCAND_INVALID_OVLP	0x8
#define ESCAND_INVALID_NOISE	0x10
#define ESCAND_INVALID_ALIGN	0x20
#define ESCAND_INVALID_144		0x40
#define ESCAND_INVALID_DFS		0x80
#define ESCAND_INVALID_CHAN_FLOP_PERIOD	0x100
#define ESCAND_INVALID_EXCL		0x200
#define ESCAND_INVALID_MISMATCH_SB		0x400
#define ESCAND_INVALID_SAMECHAN		0x800
#define ESCAND_INVALID_DFS_NO_11H		0x1000	/* Cannot use DFS channels if 802.11h is off */
#define ESCAND_INVALID_LPCHAN              0x2000
#define ESCAND_INVALID_AVOID_PREV		0x4000	/* avoid entering prev chan before 240sec */
#define ESCAND_INVALID_NONDFS		0x8000	/* avoid non-dfs channel during bootup with dfs
						   enabled or during dfs re-entry
						 */

#define ESCAND_BUFSIZE_4K	4096

#define ESCAND_WL_CNTBUF_SIZE	2048

/* all the policy related configuration goes here */
extern int escand_safe_get_conf(char *outval, int outval_size, char *name);
extern void dump_networks(char *network_buf);
extern char * escand_malloc(int bufsize);
extern char * escand_realloc(char *buf, int bufsize);
extern void sleep_ms(const unsigned int ms);
extern int swrite(int fd, char *buf, unsigned int size);
extern int sread(int fd, char *buf, unsigned int size);
extern int wl_format_ssid(char* ssid_buf, uint8* ssid, int ssid_len);
extern char *wl_ether_etoa(const struct ether_addr *n);
extern int escand_snprintf(char *str, size_t size, const char *format, ...);
#endif /*  _escand_h_ */
