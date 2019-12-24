/*
 * NAS WorkSpace - NAS application common code
 *
 * Copyright 2019 Broadcom
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
 * $Id: nas_wksp.h 687235 2017-02-28 07:07:14Z $
 */

#ifndef __NAS_WKSP_H__
#define __NAS_WKSP_H__

/* debug stuff */
#ifdef BCMDBG
#include <stdio.h>
#include <string.h>
extern int debug_nwksp;
#define NASDBG(fmt, arg...)	(\
{ \
	if (debug_nwksp) { \
		fprintf(stderr, "%s: "fmt, __FUNCTION__ , ##arg); \
	} \
} \
)
#define NASHEX(mem, size)	(\
{ \
	if (debug_nwksp) { \
		char buf[80]; \
		int i, j, k; \
		for (i = 0; i < size; ) { \
			j = sprintf(buf, "%04X: ", i); \
			for (k = 0; k < 16 && i < size; k++, i++) \
				j += sprintf(&buf[j], " %02X", mem[i]); \
			printf("%s\n", buf); \
		} \
	} \
} \
)
#else	/* #if BCMDBG */
#define NASDBG(fmt, arg...)
#define NASHEX(mem, size)
#endif	/* #if BCMDBG */
#define NASMSG(fmt, arg...)	printf(fmt , ##arg)

#include <sys/types.h>
#include <net/if.h>
#include "bcmtimer.h"
#include "nas.h"
#include "nas_wpa.h"

/*
 * The same source code can be built into either NAS server or
 * NAS supplicant.
 */
#if !defined(NAS_WKSP_BUILD_NAS_AUTH) && !defined(NAS_WKSP_BUILD_NAS_SUPPL)
#error must defined NAS_WKSP_BUILD_NAS_AUTH and/or NAS_WKSP_BUILD_NAS_SUPPL
#endif // endif

#define NAS_WKSP_PSK_LEN				64
#define NAS_WKSP_PASSPHRASE_MIN		8
#define NAS_WKSP_PASSPHRASE_MAX		63

#define NAS_WKSP_MAX_USER_KEY_LEN	80
#define NAS_WKSP_MAX_NUM_INTERFACES	66

#define NAS_WKSP_UNK_FILE_DESC		-1

#define NAS_WPA_CB_FLAG_SUPPL		WLIFU_WSEC_SUPPL	/* use nas as supplicant */
#define NAS_WPA_CB_FLAG_AUTH			WLIFU_WSEC_AUTH	/* use nas as authenticator */
#define NAS_WPA_CB_FLAG_WDS			WLIFU_WSEC_WDS	/* nas in WDS mode */
#define NAS_WPA_CB_FLAG_ERROR		0x80000000		/* error to init nas_t struct */

#define NAS_WKSP_FLAG_SHUTDOWN		1	/* Shutdown flag */
#define NAS_WKSP_FLAG_REKEY		2	/* Rekey flag */

/* nas/wpa combo and lots of goodies, one per NAS instance */
typedef struct nas_wpa_cb
{
	nas_t nas;
	wpa_t wpa;

	/* user-supplied psk passphrase */
	uint8 psk[NAS_WKSP_MAX_USER_KEY_LEN + 1];
	/* user-supplied radius secret */
	uint8 secret[NAS_WKSP_MAX_USER_KEY_LEN + 1];
	/* user-supplied wep key */
	uint8 wep[NAS_WKSP_MAX_USER_KEY_LEN + 1];
	/* wep key index */
	int index;
	/* run time flags */
	uint32 flags;
	/* wl unit # */
	int unit;
	/* back pointer */
	struct nas_wksp *nwksp;
} nas_wpa_cb_t;

/* nas work space */
typedef struct nas_wksp
{
	/* packet buffer for reading socket */
	uint8 packet[RADIUS_MAX_LEN];
	/* file dscc set for select() */
	fd_set fdset;
	int fdmax;
	/* receive/send eapol/bcmevent/preauth type packets to eapd */
	int eapd;
	/* timer module id */
	bcm_timer_module_id timer;
	/* run time flags */
	uint32 flags;
	/* # of i/f (in/out) */
	int nwcbs;
	/* nas_wpa_cb_t list */
	nas_wpa_cb_t *nwcb[NAS_WKSP_MAX_NUM_INTERFACES];
	/* Should we daemonize? */
	int foreground;
#ifdef WLHOSTFBT
	struct l2_packet_data * l2_rrb;
	/* receive/send rrb packets */
	uint32 l2_rrb_fd;
#endif /* WLHOSTFBT */
} nas_wksp_t;

typedef enum {
	NAS_WKSP_NWCB_SEARCH_ONLY = 0,
	NAS_WKSP_NWCB_SEARCH_ENTER
} nwcb_lookup_mode_t;

#ifdef NAS_WKSP_ON_DEMAND
#define	NAS_WKSP_NWCB_AUTO	NAS_WKSP_NWCB_SEARCH_ENTER
#define NAS_WKSP_ADD_NWCB(nwksp, mac, osifname, nwcb_reuse) \
		nas_wksp_add_nwcb(nwksp, mac, osifname, nwcb_reuse)
#else
#define	NAS_WKSP_NWCB_AUTO	NAS_WKSP_NWCB_SEARCH_ONLY
#define NAS_WKSP_ADD_NWCB(nwksp, mac, osifname)	(NULL)
#endif /* #ifdef NAS_WKSP_ON_DEMAND */

#ifdef __cplusplus
extern "C" {
#endif // endif

/* Common */
void nas_wksp_display_usage(void);
int nas_wksp_parse_cmd(int argc, char *argv[], nas_wksp_t *nwksp);
nas_wksp_t *nas_wksp_alloc_workspace(void);
void nas_wksp_free_workspace(nas_wksp_t *nwksp);
nas_wpa_cb_t *nas_wksp_find_nwcb(nas_wksp_t *nwksp, uint8 *mac, char *osifname, int mode);
int nas_wksp_init(nas_wksp_t *nwksp);
void nas_wksp_cleanup(nas_wksp_t *nwksp);
int nas_wksp_main_loop(nas_wksp_t *nwksp);
void nas_wksp_dispatch_packet(nas_wksp_t *rootnwksp);
void nas_wksp_clear_inited(void);
int nas_wksp_is_inited(void);

/* Socket specific */
int nas_wksp_open_eapd(nas_wksp_t *rootnwksp);
void nas_wksp_close_eapd(nas_wksp_t *rootnwksp);

int nas_validate_wlpvt_message(int bytes, uint8 *pkt);
int nas_handle_wlpvt_messages(nas_wpa_cb_t *nwcb, void *dpkt, int bytes);
void nas_eapol_message_dispatch(nas_wpa_cb_t *nwcb, void *eapol, int bytes);

/* OS dependent */
int nas_safe_get_conf(char *outval, int outval_size, char *name);

#ifdef __cplusplus
}
#endif // endif

#endif	/* ifndef __NAS_WKSP_H__ */
