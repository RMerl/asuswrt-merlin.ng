/*
 * Wake-on-wireless related source file
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_wowlpf.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * This file is used in firmware builds, not in NIC builds. Note that there is also a 'ucode'
 * alternative instead of a 'packet filter'.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlFullDongleWowlPF]
 */

#ifndef WOWLPF
#error "WOWLPF is not defined"
#endif // endif

#ifdef WLC_LOW_ONLY
#error "WOWLPF Cannot be in WLC_LOW_ONLY!"
#endif // endif

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <proto/bcmevent.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wlc_hw_priv.h>
#include <wl_export.h>
#include <wlc_scb.h>
#include <wlc_scan.h>
#include <wlc_wowlpf.h>

/*
 * A WOL (wake-on-lan) magic packet is a packet containing anywhere within its payload
 * 6 bytes of 0xFF followed by 16 contiguous copies of the receiving NIC's Ethernet address
 */
#define	WOL_MAGIC_PATTERN_SIZE	(ETHER_ADDR_LEN*17)
#define	WOL_MAGIC_MASK_SIZE		(ETHER_ADDR_LEN*17)

#define WLC_WOWL_PKT_FILTER_MIN_ID 128
#define WLC_WOWL_PKT_FILTER_MAX_ID (WLC_WOWL_PKT_FILTER_MIN_ID + 128)

#ifdef SECURE_WOWL

/* when turn following debug open on, you may need reduce the features
 * in build target to gurantee to FW can be loaded and dongle be attached
 * correctly, this option will bring lots prints and will increase FW size
 */
#define SECWOWL_DEBUG 0

#if SECWOWL_DEBUG
#define wowl_prhex(x, y, z) do { \
	printf("%s:%d\n", __FUNCTION__, __LINE__); \
	prhex(x, y, z); \
} while (0)
#define WL_SECWOWL_ERR(fmt, ...) printf("%s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define WL_WOWL_ERR(fmt, ...) printf("%s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define wowl_prhex(x)
#define WL_SECWOWL_ERR(fmt, ...)
#define WL_WOWL_ERR(fmt, ...)
#endif /* SECWOWL_DEBUG */

#define WOWL_PARSE_D11_PKT       0

#define TCP_PSEUDO_HEADER_LEN	12
#define SNAP_HDR_LEN			6	/* 802.3 SNAP header length */
static const uint8 llc_snap_hdr[SNAP_HDR_LEN] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};

static uint32 TLS_packet_encode(wowlpf_info_t *wowl,
	uint8 *in, int in_len, uint8 *out, int out_len);
static uint8* TLS_packet_parse(wowlpf_info_t *wowl, uint8 *data, int data_len, int *pkt_len);
static uint8* TCP_packet_parse(wowlpf_info_t *wowl, uint8 *data, int data_len);
static uint8* IP_packet_parse(wowlpf_info_t *wowl, uint8 *data, int data_len);
static uint8* ETH_packet_parse(wowlpf_info_t *wowl, uint8 *data, int data_len);
#if WOWL_PARSE_D11_PKT
static uint8* DOT11_packet_parse(wowlpf_info_t *wowl, uint8 *data, int data_len);
#endif /* WOWL_PARSE_D11_PKT */
static uint8* wlc_wowlpf_secpkt_parse(void *ctx, const void *sdu, int sending);
static tls_info_t *wlc_secwowl_alloc(wowlpf_info_t *wowl);

#endif /* SECURE_WOWL */

#if defined(SS_WOWL)
static void wlc_wowlpf_tmout(void *arg);
static int wlc_secwowl_activate_get(wowlpf_info_t *wowl, void *pdata, int datalen);
static int wlc_secwowl_activate_set(wowlpf_info_t *wowl, void *pdata, int datalen);
static int wlc_wowlpf_wakeup(wowlpf_info_t *wowl, int action);
#endif /* defined(SS_WOWL) */

/* iovar table */
enum {
	IOV_WOWL,
	IOV_WOWL_PATTERN,
	IOV_WOWL_WAKEIND,
	IOV_WOWL_STATUS,
	IOV_WOWL_ACTIVATE,
	IOV_WOWL_ACTVSEC,
	IOV_WOWL_WAKEUP,
	IOV_WOWL_PATTMODE,
	IOV_WOWL_CLEAR,
	IOV_WOWL_GPIO,
	IOV_WOWL_GPIOPOL,
	IOV_WOWL_DNGLDOWN,
	IOV_WOWL_KEY_ROT
};

static const bcm_iovar_t wowl_iovars[] = {
	{"wowl", IOV_WOWL, (IOVF_WHL), IOVT_UINT32, 0},
	{"wowl_pattern", IOV_WOWL_PATTERN, (0), IOVT_BUFFER, 0},
	{"wowl_wakeind", IOV_WOWL_WAKEIND, (0), IOVT_BUFFER, sizeof(wl_wowl_wakeind_t)},
	{"wowl_status", IOV_WOWL_STATUS, (0), IOVT_UINT16, 0},
	{"wowl_activate", IOV_WOWL_ACTIVATE, (0), IOVT_BOOL, 0},
#if defined(SS_WOWL)
	{"wowl_activate_secure", IOV_WOWL_ACTVSEC, (0), IOVT_BUFFER, sizeof(tls_param_info_t)},
	{"wowl_wakeup", IOV_WOWL_WAKEUP, (0), IOVT_UINT32, 0},
	{"wowl_pattmode", IOV_WOWL_PATTMODE, (0), IOVT_UINT32, 0},
#endif /* SS_WOWL */
	{"wowl_clear", IOV_WOWL_CLEAR, (0), IOVT_BOOL, 0},
	{"wowl_gpio", IOV_WOWL_GPIO, (0), IOVT_UINT8, 0},
	{"wowl_gpiopol", IOV_WOWL_GPIOPOL, (0), IOVT_UINT8, 0},
	{"wowl_dngldown", IOV_WOWL_DNGLDOWN, (0), IOVT_BOOL, 0},
	{"wowl_keyrot", IOV_WOWL_KEY_ROT, (0), IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0}
};

#define WOWLPF_TIMEOUT 2 /* 1 ms */

struct wowl_pattern;
typedef struct wowl_pattern {
	struct wowl_pattern *next;
	wl_wowl_pattern_t *pattern;
} wowl_pattern_t;

#define WOWL_SENDINGPKT_MAX_LEN        1024
typedef struct wowl_net_session {
	uint8 local_macaddr[ETHER_ADDR_LEN];
	uint16 local_port;
	uint8 local_ip[IPV4_ADDR_LEN];

	uint8 remote_macaddr[ETHER_ADDR_LEN];
	uint16 remote_port;
	uint8 remote_ip[IPV4_ADDR_LEN];

	uint32 lseq; /* local tcp seq number */
	uint32 rseq; /* tcp ack number, equals to tcp seq number at the other end */

	uint32 keepalive_interval; /* keepalive interval, in seconds */
	uint8 terminated;
	uint8 ack_required;
	uint8 fin_sent;
	uint8 fin_recv;
	uint8 wakeup_reason;
	uint8 resend_times;
	uint32 bytecnt_sending;
	uint8 message_sending[WOWL_SENDINGPKT_MAX_LEN];
} wowl_net_session_t;

/* WOWL module specific state */
struct wowlpf_info {
	wlc_info_t *wlc;                /* pointer to main wlc structure */
	wowl_pattern_t *pattern_list;   /* Netpattern list */
	uint32	flags_user;             /* Separate User setting from OS setting */
	uint32	flags_current;
	uint32  wakeind;                /* Last wakeup -indication from ucode */
	uint8	pattern_count;          /* count of patterns for easy access */
	bool	cap;                    /* hardware is wowl-capable */
	uint8	gpio;
	bool	gpio_polarity;
	bool	dngldown;
	uint8	wakepatt_mode;
	uint8	tm_type;
	uint8	flt_cnt;                /* count of filter in list */
	uint32	*flt_lst;               /* filter IDs was added to pkt_filter */
	struct	wl_timer *wowl_timer;
	wowl_net_session_t 	*netsession;
	tls_info_t *tls;
};

#ifdef BCMDBG
static void wlc_print_wowlpattern(wl_wowl_pattern_t *pattern);
static int wlc_wowl_dump(wowlpf_info_t *wowl, struct bcmstrbuf *b);
#endif // endif
static void wlc_wowlpf_free(wlc_info_t *wlc);
static int wlc_wowl_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
                            void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static int wlc_wowl_upd_pattern_list(wowlpf_info_t *wowl, wl_wowl_pattern_t *wl_pattern,
	uint size, char *arg);
static bool wlc_wowlpf_init_gpio(wlc_hw_info_t *wlc_hw, uint8 wowl_gpio, bool polarity);
static bool wlc_wowlpf_set_gpio(wlc_hw_info_t *wlc_hw, uint8 wowl_gpio, bool polarity);
bool wlc_wowlpf_enable(wowlpf_info_t *wowl);
uint32 wlc_wowlpf_clear(wowlpf_info_t *wowl);
static void wlc_wowlpf_dngldown_tmout(void *arg);
static void wlc_secwowl_free(wowlpf_info_t *wowl);

static wl_pkt_decrypter_t secwowl_decrypt_ctx = {0, 0};

static const char BCMATTACHDATA(rstr_wowl_gpio)[] = "wowl_gpio";
static const char BCMATTACHDATA(rstr_wowl_gpiopol)[] = "wowl_gpiopol";

#define MS_PER_SECOND                    (1000)

#if defined(SS_WOWL)

#define SS_WOWL_PING_HEADER_SIZE          4
#define SS_WOWL_MAX_ASSYNC_ID             32767
#define SS_WOWL_MAX_RESEND_TIMES          4

#define SS_WOWL_HDR_RESERVED              0
#define SS_WOWL_HDR_PING_REQUEST          6
#define SS_WOWL_HDR_PING_REPLY            7
#define SS_WOWL_PING_INTERVAL_MIN         (15) /* min 15 seconds */
#define SS_WOWL_PING_INTERVAL_MAX         (4 * 60) /* max 4 minutes */

#define GPIO_PULSE_LOW	                  0
#define GPIO_PULSE_HIGH                   1
#define GPIO_PULSE_LVL(ptr)               ((*ptr) >> 7)
#define GPIO_PULSE_MS(ptr)                ((*ptr) & 0x7F)
#define GPIO_PULSE_NODE(lvl, ms)          (lvl << 7) | (ms & 0x7F)

#define SS_WOWL_WAKEUP_ID_LEN              8 /* length of "SECWOWIN" */
#define SS_WOWL_WAKEE_ADDR_LEN             6
#define SS_WOWL_WAKEUP_INDOOR_APP_INVALID  0
#define SS_WOWL_WAKEUP_INDOOR_APP_MAX      2
#define SS_WOWL_WAKEUP_APP_OUTDOOR         4
#define SS_WOWL_WAKEUP_AP_LINK_LOST        5
#define SS_WOWL_WAKEUP_SERVER_LINK_FAIL    6
#define SS_WOWL_WAKEUP_REASON_CNT          8
#define SS_WOWL_WAKEUP_REASON_MIN          1
#define SS_WOWL_WAKEUP_REASON_MAX          SS_WOWL_WAKEUP_REASON_CNT

#define SS_WOWL_WAKEUP_PATTMODE_0          0
#define SS_WOWL_WAKEUP_PATTMODE_1          1

#define SS_WOWL_WAKEUP_PATTMODE_MIN        SS_WOWL_WAKEUP_PATTMODE_0
#define SS_WOWL_WAKEUP_PATTMODE_MAX        SS_WOWL_WAKEUP_PATTMODE_1

/* default: wakeup pattern mode 0 */
/* wakeup pattern mode 0: trl 4ms trh 1ms */
static uint8 ss_wowl_wakeup_wave_0[(SS_WOWL_WAKEUP_REASON_CNT + 1) * 2] = {
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 24),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 1),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 1),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 1),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 1),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 1),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 1),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 1),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 1),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 1)
};
/* wakeup pattern mode 1: trl 6ms trh 4ms */
static uint8 ss_wowl_wakeup_wave_1[(SS_WOWL_WAKEUP_REASON_CNT + 1) * 2] = {
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 24),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 1),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 6),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 6),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 6),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 6),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 6),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 6),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 6),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 4),
	GPIO_PULSE_NODE(GPIO_PULSE_LOW, 6),
	GPIO_PULSE_NODE(GPIO_PULSE_HIGH, 4)
};

uint8 *ss_wowl_wakeup_wave[] = {
	ss_wowl_wakeup_wave_0,
	ss_wowl_wakeup_wave_1
};

static bool
wlc_wowlpf_sswakeup(wowlpf_info_t *wowl, int reason)
{
	uint8 *wakeup_pulse_cur = ss_wowl_wakeup_wave[wowl->wakepatt_mode];

	WL_ERROR(("wl%d:%s up reason %d \n", wowl->wlc->pub->unit, __FUNCTION__, reason));

	if ((reason < SS_WOWL_WAKEUP_REASON_MIN) || (reason > SS_WOWL_WAKEUP_REASON_MAX))
		return FALSE;

	if (wowl->netsession)
		wowl->netsession->terminated = 1;

	reason += 1; /* add Wake-Up indication */
	reason *= 2; /* each indication and reason pulse has low and high */
	while (reason--) {
		wlc_wowlpf_set_gpio(wowl->wlc->hw, wowl->gpio, GPIO_PULSE_LVL(wakeup_pulse_cur));
		OSL_DELAY(GPIO_PULSE_MS(wakeup_pulse_cur) * 1000);
		wakeup_pulse_cur++;
	}

	return TRUE;
}
#endif /* SS_WOWL */

static void
wlc_wowlpf_free(wlc_info_t *wlc)
{
	wowlpf_info_t *wowl = wlc->wowlpf;
	if (wowl) {
		if (wowl->flt_lst)
			MFREE(wlc->osh, wowl->flt_lst, sizeof(uint32)
				* (WLC_WOWL_PKT_FILTER_MAX_ID - WLC_WOWL_PKT_FILTER_MIN_ID + 1));
		MFREE(wlc->osh, wowl, sizeof(wowlpf_info_t));
		wlc->wowlpf = NULL;
	}
}

static void
wlc_wowlpf_rm_timer(wowlpf_info_t *wowl)
{
	if (wowl && wowl->wowl_timer) {
		wl_del_timer(wowl->wlc->wl, wowl->wowl_timer);
		wl_free_timer(wowl->wlc->wl, wowl->wowl_timer);
		wowl->wowl_timer = NULL;
	}
}

static bool
wlc_wowlpf_add_timer(wowlpf_info_t *wowl, void (*fn)(void* arg),
	uint32 msdelay, bool repeat, uint32 tm_type)
{
	bool rtn = TRUE;
	wlc_wowlpf_rm_timer(wowl);
	wowl->wowl_timer = wl_init_timer(wowl->wlc->wl, fn, wowl, "wowl");
	if (!wowl->wowl_timer) {
		rtn = FALSE;
		WL_ERROR(("wl%d: wowl wl_init_timer() failed\n", wowl->wlc->pub->unit));
	} else {
		wowl->tm_type = tm_type;
		wl_add_timer(wowl->wlc->wl, wowl->wowl_timer, msdelay, repeat);
	}
	WL_INFORM(("add time wowl->tm_type %d\n", wowl->tm_type));
	return rtn;
}

wowlpf_info_t *
BCMATTACHFN(wlc_wowlpf_attach)(wlc_info_t *wlc)
{
	wowlpf_info_t *wowl;

	ASSERT(wlc_wowlpf_cap(wlc));
	ASSERT(wlc->wowlpf == NULL);

	if (!(wowl = (wowlpf_info_t *)MALLOC(wlc->osh, sizeof(wowlpf_info_t)))) {
		WL_ERROR(("wl%d: wlc_wowl_attachpf: out of mem, malloced %d bytes\n",
			wlc->pub->unit, MALLOCED(wlc->osh)));
		goto fail;
	}

	bzero((char *)wowl, sizeof(wowlpf_info_t));
	wowl->wlc = wlc;

	/* register module */
	if (wlc_module_register(wlc->pub, wowl_iovars, "wowl",
		wowl, wlc_wowl_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: wowl wlc_module_register() failed\n", wlc->pub->unit));
		goto fail;
	}

#ifdef BCMDBG
	wlc_dump_register(wlc->pub, "wowl", (dump_fn_t)wlc_wowl_dump, (void *)wowl);
#endif // endif

#if (defined(WOWL_GPIO) && defined(WOWL_GPIO_POLARITY))
	wowl->gpio = WOWL_GPIO;
	wowl->gpio_polarity = WOWL_GPIO_POLARITY;
#else
	wowl->gpio = WOWL_GPIO_INVALID_VALUE;
	wowl->gpio_polarity = 1;
#endif // endif
	{
		/* override wowl gpio if defined in nvram */
		char *var;
		if ((var = getvar(wlc->pub->vars, rstr_wowl_gpio)) != NULL)
			wowl->gpio =  (uint8)bcm_strtoul(var, NULL, 0);
		if ((var = getvar(wlc->pub->vars, rstr_wowl_gpiopol)) != NULL)
			wowl->gpio_polarity =  (bool)bcm_strtoul(var, NULL, 0);
	}

	return wowl;

fail:
	wlc_wowlpf_free(wlc);
	return NULL;
}

#ifdef BCMDBG
static int
wlc_wowl_dump(wowlpf_info_t *wowl, struct bcmstrbuf *b)
{
	uint32 wakeind = wowl->wakeind;
	uint32 flags_current = wowl->flags_curent;

	bcm_bprintf(b, "Status of last wakeup:\n");
	bcm_bprintf(b, "\tflags:0x%x\n", flags_current);

	if (flags_current & WL_WOWL_BCN)
		bcm_bprintf(b, "\t\tWake-on-Loss-of-Beacons enabled\n");

	if (flags_current & WL_WOWL_MAGIC)
		bcm_bprintf(b, "\t\tWake-on-Magic frame enabled\n");
	if (flags_current & WL_WOWL_NET)
		bcm_bprintf(b, "\t\tWake-on-Net pattern enabled\n");
	if (flags_current & WL_WOWL_DIS)
		bcm_bprintf(b, "\t\tWake-on-Deauth enabled\n");
	if (flags_current & WL_WOWL_GTK_FAILURE)
		bcm_bprintf(b, "\t\tWake-on-Key Rotation (GTK) Failure enabled\n");

	bcm_bprintf(b, "\n");

	if ((wakeind & WL_WOWL_MAGIC) == WL_WOWL_MAGIC)
		bcm_bprintf(b, "\t\tMAGIC packet received\n");
	if ((wakeind & WL_WOWL_NET) == WL_WOWL_NET)
		bcm_bprintf(b, "\t\tPacket received with Netpattern\n");
	if ((wakeind & WL_WOWL_DIS) == WL_WOWL_DIS)
		bcm_bprintf(b, "\t\tDisassociation/Deauth received\n");
	if ((wakeind & WL_WOWL_BCN) == WL_WOWL_BCN)
		bcm_bprintf(b, "\t\tBeacons Lost\n");
	if ((wakeind & WL_WOWL_GTK_FAILURE) == WL_WOWL_GTK_FAILURE)
		bcm_bprintf(b, "\t\tKey Rotation (GTK) Failed\n");
	if ((wakeind & (WL_WOWL_NET | WL_WOWL_MAGIC))) {
		if ((wakeind & WL_WOWL_BCAST) == WL_WOWL_BCAST)
			bcm_bprintf(b, "\t\t\tBroadcast/Mcast frame received\n");
		else
			bcm_bprintf(b, "\t\t\tUnicast frame received\n");
	}
	if (wakeind == 0)
		bcm_bprintf(b, "\tNo wakeup indication set\n");

	return 0;
}
#endif /* BCMDBG */

void
BCMATTACHFN(wlc_wowlpf_detach)(wowlpf_info_t *wowl)
{
	if (!wowl)
		return;

	wlc_wowlpf_rm_timer(wowl);

	if (wowl->tls)
		wlc_secwowl_free(wowl);

	/* Free the WOWL net pattern list */
	while (wowl->pattern_list) {
		wowl_pattern_t *node = wowl->pattern_list;
		wowl->pattern_list = node->next;
		MFREE(wowl->wlc->osh, node->pattern,
		      sizeof(wl_wowl_pattern_t) +
		      node->pattern->masksize +
		      node->pattern->patternsize);
		MFREE(wowl->wlc->osh, node, sizeof(wowl_pattern_t));
		wowl->pattern_count--;
	}
	ASSERT(wowl->pattern_count == 0);

	wlc_module_unregister(wowl->wlc->pub, "wowl", wowl);
	wlc_wowlpf_free(wowl->wlc);
}

/* handle WOWL related iovars */
static int
wlc_wowl_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
                 void *p, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{
	wowlpf_info_t *wowl = (wowlpf_info_t *)hdl;
	int32 int_val = 0;
	int err = 0;
	wlc_info_t *wlc;
	int32 *ret_int_ptr = (int32 *)arg;

	if ((err = wlc_iovar_check(wowl->wlc, vi, arg, alen, IOV_ISSET(actionid), wlcif)) != 0)
		return err;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	wlc = wowl->wlc;
	ASSERT(wowl == wlc->wowlpf);

	switch (actionid) {
	case IOV_GVAL(IOV_WOWL):
	        *ret_int_ptr = wowl->flags_user;
	        break;
	case IOV_SVAL(IOV_WOWL):
	        if ((int_val & ~(WL_WOWL_MAGIC | WL_WOWL_NET | WL_WOWL_DIS | WL_WOWL_SECURE
				| WL_WOWL_BCN | WL_WOWL_RETR | WL_WOWL_UNASSOC
				| WL_WOWL_GTK_FAILURE)) != 0) {
			err = BCME_BADARG;
			break;
		}
		/* Clear all the flags, else just add the current one
		 * These are cleared across sleep/wakeup by common driver
		 */
		wowl->flags_user = int_val;
		break;

	case IOV_GVAL(IOV_WOWL_STATUS):
	        *ret_int_ptr = wowl->flags_current;
	        break;

	case IOV_GVAL(IOV_WOWL_WAKEIND): {
		wl_wowl_wakeind_t *wake = (wl_wowl_wakeind_t *)arg;
		wake->pci_wakeind = wowl->wakeind;
		wake->ucode_wakeind = wowl->wakeind;
		break;
	}

	case IOV_SVAL(IOV_WOWL_WAKEIND): {
		if (strncmp(arg, "clear", strlen("clear")) == 0) {
			wowl->wakeind = 0;
		}
		break;
	}

	case IOV_GVAL(IOV_WOWL_PATTERN): {
		wl_wowl_pattern_list_t *list;
		wowl_pattern_t *src;
		uint8 *dst;
		uint src_len, rem;

		/* Check if the memory provided is too low - check for pattern/mask len below  */
		if (alen < (int)(sizeof(wl_wowl_pattern_list_t))) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		list = (wl_wowl_pattern_list_t *)arg;
		list->count = wowl->pattern_count;
		dst = (uint8*)list->pattern;
		rem = alen - sizeof(list->count);
		for (src = wowl->pattern_list;
		     src; src = src->next) {
			/* Check if there is dest has enough space */
			src_len = (src->pattern->masksize +
			           src->pattern->patternsize +
			           sizeof(wl_wowl_pattern_t));

			if (src_len > rem) {
				list->count = 0;
				err = BCME_BUFTOOSHORT;
				break;
			}

			/* Copy the pattern */
			bcopy(src->pattern, dst, src_len);

			/* Move the pointer */
			dst += src_len;
			rem -= src_len;
		}
		break;
	}
	case IOV_SVAL(IOV_WOWL_PATTERN): {
		uint size;
		uint8 *buf;
		int bufsize = MAXPATTERNSIZE + MAXMASKSIZE + sizeof(wl_wowl_pattern_t);
		wl_wowl_pattern_t *wl_pattern;

		/* validate the action */
		if (strncmp(arg, "add", 3) != 0 &&
		    strncmp(arg, "del", 3) != 0 &&
		    strncmp(arg, "clr", 3) != 0) {
			err = BCME_BADARG;
			break;
		}

		if ((alen - strlen("add") - 1) > bufsize) {
			err = BCME_BUFTOOLONG;
			break;
		}

		if ((buf = MALLOC(wlc->pub->osh, bufsize)) == NULL) {
			err = BCME_NOMEM;
			break;
		}
		bcopy((uint8 *)arg + (strlen("add") + 1), buf, (alen - strlen("add") - 1));
		wl_pattern = (wl_wowl_pattern_t *) buf;

		if (strncmp(arg, "clr", 3) == 0)
			size = 0;
		else
			size = wl_pattern->masksize + wl_pattern->patternsize +
			        sizeof(wl_wowl_pattern_t);

		ASSERT(alen == (strlen("add") + 1 + size));

		/* Validate input */
		if (strncmp(arg, "clr", 3) &&
		    (wl_pattern->masksize > (MAXPATTERNSIZE / 8) ||
		     wl_pattern->patternsize > MAXPATTERNSIZE)) {
			MFREE(wlc->osh, buf, bufsize);
			err = BCME_RANGE;
			break;
		}

#ifdef BCMDBG
		/* Prepare the pattern */
		if (WL_INFORM_ON()) {
			WL_WOWL(("wl%d: %s %d action:%s \n", wlc->pub->unit, __FUNCTION__, __LINE__,
			         (char*)arg));
			if (strncmp(arg, "clr", 3))
				wlc_print_wowlpattern(wl_pattern);
		}
#endif // endif

		/* update data pattern list */
		err = wlc_wowl_upd_pattern_list(wowl, wl_pattern, size, arg);
		MFREE(wlc->osh, buf, bufsize);
		break;
	}

#if defined(SS_WOWL)
	case IOV_GVAL(IOV_WOWL_ACTVSEC): {
		err = wlc_secwowl_activate_get(wowl, arg, alen);
		break;
	}
	case IOV_SVAL(IOV_WOWL_ACTVSEC): {
		err = wlc_secwowl_activate_set(wowl, arg, alen);
		break;
	}

	case IOV_SVAL(IOV_WOWL_WAKEUP):
		err = wlc_wowlpf_wakeup(wowl, int_val);
		break;

	case IOV_GVAL(IOV_WOWL_PATTMODE):
		*ret_int_ptr = htol32(wowl->wakepatt_mode);
		break;

	case IOV_SVAL(IOV_WOWL_PATTMODE):
		if (int_val < SS_WOWL_WAKEUP_PATTMODE_MIN ||
			int_val > SS_WOWL_WAKEUP_PATTMODE_MAX) {
			err = BCME_RANGE;
			break;
		}
		wowl->wakepatt_mode = ltoh32(int_val);
		wlc_wowlpf_init_gpio(wowl->wlc->hw, wowl->gpio, wowl->gpio_polarity);
		break;
#endif /* SS_WOWL */

	case IOV_SVAL(IOV_WOWL_ACTIVATE):
	case IOV_GVAL(IOV_WOWL_ACTIVATE):
		*ret_int_ptr = wlc_wowlpf_enable(wowl);
		break;

	case IOV_SVAL(IOV_WOWL_CLEAR):
	case IOV_GVAL(IOV_WOWL_CLEAR):
		*ret_int_ptr = wlc_wowlpf_clear(wowl);
		break;

	case IOV_GVAL(IOV_WOWL_GPIO):
		*ret_int_ptr = htol32(wowl->gpio);
		break;

	case IOV_SVAL(IOV_WOWL_GPIO):
		wowl->gpio = ltoh32(int_val);
		break;

	case IOV_GVAL(IOV_WOWL_GPIOPOL):
		*ret_int_ptr = htol32(wowl->gpio_polarity);
		break;

	case IOV_SVAL(IOV_WOWL_GPIOPOL):
		wowl->gpio_polarity = ltoh32(int_val);
		break;

	case IOV_GVAL(IOV_WOWL_DNGLDOWN):
		*ret_int_ptr = htol32(wowl->dngldown);
		break;

	case IOV_SVAL(IOV_WOWL_DNGLDOWN):
		wowl->dngldown = ltoh32(int_val);
		break;

	case IOV_GVAL(IOV_WOWL_KEY_ROT):
		/* Always return one, since it is enabled by default */
		int_val = (int32)0x1;
		bcopy(&int_val, arg, vsize);
		break;

	case IOV_SVAL(IOV_WOWL_KEY_ROT):
		/* Do nothing, since Key rotation is enabled by default in FD */
		break;

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

/* The device Wake-On-Wireless capable only if hardware allows it to */
bool
wlc_wowlpf_cap(wlc_info_t *wlc)
{
	return TRUE;
}

/* add or remove data pattern from pattern list */
static int
wlc_wowl_upd_pattern_list(wowlpf_info_t *wowl, wl_wowl_pattern_t *wl_pattern,
	uint size, char *arg)
{
	wlc_info_t *wlc = wowl->wlc;

	/* If add, then add to the front of list, else search for it to remove it
	 * Note: for deletion all the content should match
	 */
	if (!strncmp(arg, "clr", 3)) {
		while (wowl->pattern_list) {
			wowl_pattern_t *node = wowl->pattern_list;
			wowl->pattern_list = node->next;
			MFREE(wlc->osh, node->pattern,
			      sizeof(wl_wowl_pattern_t) +
			      node->pattern->masksize +
			      node->pattern->patternsize);
			MFREE(wlc->osh, node, sizeof(wowl_pattern_t));
			wowl->pattern_count--;
		}
		ASSERT(wowl->pattern_count == 0);
	} else if (!strncmp(arg, "add", 3)) {
		wowl_pattern_t *new;

		if (wowl->pattern_count == MAXPATTERNS)
			return BCME_NORESOURCE;

		if ((new = MALLOC(wlc->pub->osh, sizeof(wowl_pattern_t))) == NULL)
			return BCME_NOMEM;

		if ((new->pattern = MALLOC(wlc->pub->osh, size)) == NULL) {
			MFREE(wlc->pub->osh, new, sizeof(wowl_pattern_t));
			return BCME_NOMEM;
		}

		/* Just copy over input to the new pattern */
		bcopy(wl_pattern, new->pattern, size);

		new->next = wowl->pattern_list;
		wowl->pattern_list = new;
		wowl->pattern_count++;
	} else {	/* "del" */
		uint node_sz;
		bool matched;
		wowl_pattern_t *prev = NULL;
		wowl_pattern_t *node = wowl->pattern_list;

		while (node) {
			node_sz = node->pattern->masksize +
				node->pattern->patternsize + sizeof(wl_wowl_pattern_t);

			matched = (size == node_sz &&
				!bcmp(node->pattern, wl_pattern, size));

			if (matched) {
				if (!prev)
					wowl->pattern_list = node->next;
				else
					prev->next = node->next;
				MFREE(wlc->pub->osh, node->pattern,
				      sizeof(wl_wowl_pattern_t) +
				      node->pattern->masksize +
				      node->pattern->patternsize);
				MFREE(wlc->pub->osh, node, sizeof(wowl_pattern_t));
				wowl->pattern_count--;
				break;
			}

			prev = node;
			node = node->next;
		}

		if (!matched)
			return BCME_NOTFOUND;
	}

	return BCME_OK;
}

static bool
wlc_wowl_add_magic_filter(wowlpf_info_t *wowl)
{
	wlc_info_t *wlc = wowl->wlc;
	wl_pkt_filter_t *pkt_filterp;
	wl_pkt_filter_enable_t pkt_flt_en;
	uint8 buf[sizeof(wl_pkt_filter_t) + WOL_MAGIC_MASK_SIZE + WOL_MAGIC_PATTERN_SIZE];
	uint8 *mask, *pattern;
	int32 i, err = BCME_OK;

	pkt_filterp = (wl_pkt_filter_t *) buf;
	bzero(buf, sizeof(buf));

	pkt_filterp->id = wowl->flt_cnt + WLC_WOWL_PKT_FILTER_MIN_ID;
	pkt_filterp->negate_match = 0;
	pkt_filterp->type = WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH;
	pkt_filterp->u.pattern.offset = ETHER_HDR_LEN;
	if (wowl->flags_user & WL_WOWL_UNASSOC)
		pkt_filterp->u.pattern.offset = DOT11_A3_HDR_LEN;
	pkt_filterp->u.pattern.size_bytes = WOL_MAGIC_PATTERN_SIZE;

	mask = pkt_filterp->u.pattern.mask_and_pattern;
	memset(mask, 0xFF, WOL_MAGIC_MASK_SIZE);
	pattern = mask + WOL_MAGIC_MASK_SIZE;
	memset(pattern, 0xFF, ETHER_ADDR_LEN);
	for (i = 0; i < 16; i++)
		memcpy(&pattern[ETHER_ADDR_LEN + i * ETHER_ADDR_LEN],
			(uint8*)&wlc->pub->cur_etheraddr, ETHER_ADDR_LEN);

	while (pkt_filterp->id < WLC_WOWL_PKT_FILTER_MAX_ID) {
		err = wlc_iovar_op(wlc, "pkt_filter_add", NULL, 0, buf,
			WL_PKT_FILTER_FIXED_LEN + WL_PKT_FILTER_PATTERN_FIXED_LEN +
			WOL_MAGIC_MASK_SIZE + WOL_MAGIC_PATTERN_SIZE,
			IOV_SET, NULL);
		if (err)
			pkt_filterp->id++;
		else
			break;
	}

	if (err) {
		WL_ERROR(("wl%d: ERROR %d calling wlc_iovar_op \"pkt_filter_add\"\n",
			wlc->pub->unit, err));
		return FALSE;
	}

	pkt_flt_en.id = pkt_filterp->id;
	pkt_flt_en.enable = 1;
	err = wlc_iovar_op(wlc, "pkt_filter_enable", NULL, 0, &pkt_flt_en,
		sizeof(pkt_flt_en), IOV_SET, NULL);
	ASSERT(err == BCME_OK);

	wowl->flt_lst[wowl->flt_cnt++] = pkt_filterp->id;

	return TRUE;
}
static bool
wlc_wowl_add_net_filter(wowlpf_info_t *wowl)
{
	uint i, j;
	wowl_pattern_t *node;
	uint8 pattern_count;
	wlc_info_t *wlc;
	wlc = wowl->wlc;
	wl_pkt_filter_t *pkt_filterp;
	wl_pkt_filter_enable_t pkt_flt_en;
	uint32 buf[sizeof(unsigned long) + (3 * MAXPATTERNSIZE) / sizeof(uint32)];
	uint32 maskoffset = 0;
	unsigned long *psecwowldec = NULL;
	int32 err = 1;
	int32 idbase = wowl->flt_cnt + WLC_WOWL_PKT_FILTER_MIN_ID;
	uint8  *ptr;
	int32 sizebytes;

	node = wowl->pattern_list;
	pattern_count = wowl->pattern_count;
	ASSERT(pattern_count <= MAXPATTERNS);

	for (i = 0; i < pattern_count; i++, node = node->next) {
		wl_wowl_pattern_t *pattern;

		ASSERT(node);
		pattern = node->pattern;

#ifdef BCMDBG
		if (WL_INFORM_ON())
			wlc_print_wowlpattern(pattern);
#endif // endif
				/* update the patterns to wlc_pktfilter_xxx */

		pkt_filterp = (wl_pkt_filter_t *) buf;
		bzero(buf, sizeof(buf));

		pkt_filterp->id = idbase;
		pkt_filterp->negate_match = 0;

		if (wowl->flags_user & WL_WOWL_SECURE) {
			pkt_filterp->type = WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH;
			psecwowldec = (unsigned long *)(pkt_filterp->u.pattern.mask_and_pattern);
			*psecwowldec = (unsigned long)&secwowl_decrypt_ctx;
			maskoffset = sizeof(unsigned long);
		} else {
			pkt_filterp->type = WL_PKT_FILTER_TYPE_PATTERN_MATCH;
			pkt_filterp->u.pattern.offset = pattern->offset;
		}
		if (wowl->flags_user & WL_WOWL_UNASSOC) {
			pkt_filterp->u.pattern.offset += (DOT11_A3_HDR_LEN +
				DOT11_LLC_SNAP_HDR_LEN - ETHER_HDR_LEN);
		}

		/* set the size_bytes equal to minimum of pattern->masksize * 8
		* and pattern size i.e. the pktfilter pattern is trimmed when
		* masksize and patternsize of wowl filter do not match
		* (masksize * 8 != patternsize)
		*/
		sizebytes = MIN((pattern->masksize * 8), pattern->patternsize);
		pkt_filterp->u.pattern.size_bytes = sizebytes;

		ptr = ((uint8*) pattern + sizeof(wl_wowl_pattern_t));

		/* expand mask - if a bit is set correspoding pattern byte is compared */
		for (j = 0; j < sizebytes; j++) {
			pkt_filterp->u.pattern.mask_and_pattern[maskoffset + j]
			 = (isset(ptr, j) ? 0xff:0);
		}

		/* copy the pattern */
		ptr = ((uint8*) pattern + pattern->patternoffset);
		bcopy((uint8 *)ptr,
			(uint8 *)&pkt_filterp->u.pattern.mask_and_pattern[maskoffset + sizebytes],
			sizebytes);

		while (pkt_filterp->id < WLC_WOWL_PKT_FILTER_MAX_ID) {
			err = wlc_iovar_op(wlc, "pkt_filter_add", NULL, 0, buf,
				WL_PKT_FILTER_FIXED_LEN + WL_PKT_FILTER_PATTERN_FIXED_LEN
				+ (maskoffset + 2 * sizebytes),
				IOV_SET, NULL);
			if (err)
				pkt_filterp->id++;
			else
				break;
		}

		if (err) {
			WL_ERROR(("wl%d: ERROR %d calling wlc_iovar_op  \"pkt_filter_add\"\n",
				wlc->pub->unit, err));
			return FALSE;
		}

		pkt_flt_en.id = pkt_filterp->id;
		pkt_flt_en.enable = 1;
		err = wlc_iovar_op(wlc, "pkt_filter_enable", NULL, 0, &pkt_flt_en,
			sizeof(pkt_flt_en), IOV_SET, NULL);
		ASSERT(err == BCME_OK);

		wowl->flt_lst[wowl->flt_cnt++] = pkt_filterp->id;
		idbase = pkt_filterp->id + 1;
	}
	return TRUE;
}

/* Enable WOWL after downloading new ucode and setting */
bool
wlc_wowlpf_enable(wowlpf_info_t *wowl)
{
	uint32 wowl_flags;
	wlc_info_t *wlc;
	struct scb *scb = NULL;
	int pm = PM_MAX;
	uint32 pkt_filter_mode = 0;
#ifdef BCMDBG_ERR
	int err = 0; /* User Error code to designate which step failed */
#endif // endif
	wlc_key_t *key;
	wlc_key_info_t key_info;

	BCM_REFERENCE(key);
	ASSERT(wowl);

	wlc = wowl->wlc;

	if (WOWLPF_ACTIVE(wlc->pub))
		return TRUE;

	/* Make sure that there is something to do */
	if (!wlc_wowlpf_cap(wlc) ||
		(!(wowl->tls && wowl->tls->tlsparam) &&
		(wowl->flags_user & WL_WOWL_SECURE)) ||
		(!(wlc->cfg->BSS && wlc_bss_connected(wlc->cfg)) &&
		!(wowl->flags_user & WL_WOWL_UNASSOC)) ||
		(wowl->flags_user == 0)) {
		WL_ERROR(("wl%d:Wowl not enabled: because\n", wlc->pub->unit));
		WL_ERROR(("wl%d:\tcap: %d associated: %d\n"
			"\tflags_user: 0x%x\n", wlc->pub->unit,
			wlc_wowlpf_cap(wlc), wlc_bss_connected(wlc->cfg),
			wowl->flags_user));
#ifdef BCMDBG_ERR
		err = 1;
#endif // endif

		goto end;
	}

	wowl_flags = wowl->flags_user;

	/* If Magic packet is not set then validate the rest of the flags as each
	* one requires at least one more valid paramater. Set other shared flags
	* or net/TSF based on flags
	*/
	if (wowl_flags & WL_WOWL_NET) {
		if (wowl->pattern_count == 0)
		wowl_flags &= ~WL_WOWL_NET;
	}

	/* enable main WOWL feature only if successful */
	if (wowl_flags == 0) {
#ifdef BCMDBG_ERR
		err = 2;
#endif // endif
		goto end;
	}

	if (!(wowl->flags_user & WL_WOWL_UNASSOC)) {
		/* Find the security algorithm for our AP */
		if (BSSCFG_STA(wlc->cfg) && !(scb = wlc_scbfind(wlc, wlc->cfg, &wlc->cfg->BSSID))) {
			WL_WOWL(("wlc_wowlpf_enable: scan_in_progress %d home_channel 0x%x"
				" current_channel 0x%x. Attempting to abort scan..\n",
				SCAN_IN_PROGRESS(wlc->scan), wlc->cfg->current_bss->chanspec,
				wlc->chanspec));
			if (SCAN_IN_PROGRESS(wlc->scan))
				wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);
			/* Second attempt */
			if (!(scb = wlc_scbfind(wlc, wlc->cfg, &wlc->cfg->BSSID))) {
#ifdef BCMDBG_ERR
				err = 3;
#endif // endif
				goto end;
			}
		}

		/* Make sure that a key has been plumbed till now */
		key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
				WLC_KEY_FLAG_NONE, &key_info);
		ASSERT(key != NULL);
		if (key_info.algo == CRYPTO_ALGO_OFF)
			key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, SCB_BSSCFG(scb),
				FALSE, &key_info);
		if (key_info.algo == CRYPTO_ALGO_OFF) {
				if (((SCB_BSSCFG(scb)->WPA_auth != WPA_AUTH_DISABLED) ||
					(WSEC_WEP_ENABLED(SCB_BSSCFG(scb)->wsec)))) {
#ifdef BCMDBG_ERR
			                     err = 4;
#endif // endif
			                     goto end;
		       }
		}
		ASSERT(scb);
	}

	wlc->pub->_wowlpf_active = TRUE;

	if (!(wowl->flt_lst = (uint32 *)MALLOC(wlc->osh, sizeof(uint32)
		* (WLC_WOWL_PKT_FILTER_MAX_ID - WLC_WOWL_PKT_FILTER_MIN_ID + 1)))) {
		WL_ERROR(("wl%d: wlc_wowl_attachpf: out of mem, malloced %d bytes\n",
			wlc->pub->unit, MALLOCED(wlc->osh)));
		goto end;
	}

	/* If Magic packet is not set then validate the rest of the flags as each one requires at
	 * least one more valid paramater. Set other shared flags for net/TSF based on flags
	 */
	if (wowl_flags & WL_WOWL_MAGIC) {
		/* update wlc_pkt_filter with magic pattern filter */
		wlc_wowl_add_magic_filter(wowl);
	}

	if (wowl_flags & WL_WOWL_NET) {
		wlc_wowl_add_net_filter(wowl);
	}

	/* Enabling Wowl */
	WL_ERROR(("wl%d:%s enabling wowl 0x%x \n", wlc->pub->unit, __FUNCTION__, wowl_flags));
	if (!(wlc_wowlpf_init_gpio(wlc->hw, wowl->gpio, wowl->gpio_polarity))) {
#ifdef BCMDBG_ERR
		err = 5;
#endif // endif
		goto end;
	}
	wowl->flags_current = wowl_flags;
	wlc_ioctl(wlc, WLC_SET_PM, &pm, sizeof(pm), NULL);

	if (wowl->dngldown) {
		if (!wlc_wowlpf_add_timer(wowl, wlc_wowlpf_dngldown_tmout,
			WOWLPF_TIMEOUT, FALSE, WOWL_TM_DONGLE_DOWN)) {
			goto end;
		}
	}

	if (wowl->flags_user & WL_WOWL_UNASSOC) {
		/* Enable the monitor mode */
		int monitor = 1;
		wlc_ioctl(wlc, WLC_SET_MONITOR, &monitor, sizeof(monitor), NULL);
	}

	/* Set operation_mode of packet filter to PKT_FILTER_MODE_PKT_FORWARD_OFF_DEFAULT
	 * to restrict forwarding received packets to the host
	 */
	wlc_iovar_op(wowl->wlc,
		"pkt_filter_mode",
		NULL,
		0,
		&pkt_filter_mode,
		sizeof(pkt_filter_mode),
		IOV_GET,
		NULL);
	pkt_filter_mode |= PKT_FILTER_MODE_PKT_FORWARD_OFF_DEFAULT;
	wlc_iovar_op(wowl->wlc,
		"pkt_filter_mode",
		NULL,
		0,
		&pkt_filter_mode,
		sizeof(pkt_filter_mode),
		IOV_SET,
		NULL);
	return TRUE;

end:
	WL_ERROR(("wl%d:Wowl not enabled err = %d\n", wlc->pub->unit, err));
	wowl->wakeind = FALSE;
	wowl->flags_current = 0;

	return FALSE;
}

uint32
wlc_wowlpf_clear(wowlpf_info_t *wowl)
{
	wlc_info_t *wlc;
	uint32 id;
	uint32 pkt_filter_mode = 0;

	ASSERT(wowl);
	wlc = wowl->wlc;

	if (WOWLPF_ACTIVE(wlc->pub)) {
		WL_INFORM(("wl%d: wlc_wowlpf_clear: clearing wake mode 0x%x\n", wlc->pub->unit,
		          wowl->flags_current));
		wlc->pub->_wowlpf_active = FALSE;

		/* clear the packet filters */
		while (wowl->flt_cnt) {
			id = wowl->flt_lst[--wowl->flt_cnt];
			wlc_iovar_op(wlc, "pkt_filter_delete", NULL, 0, &id,
				sizeof(uint32), IOV_SET, NULL);
		}

		wlc_wowlpf_set_gpio(wlc->hw, wowl->gpio, wowl->gpio_polarity);
		wowl->flags_current = 0;

		/* Restore operation_mode of packet filter */
		wlc_iovar_op(wowl->wlc,
                        "pkt_filter_mode",
			NULL,
			0,
			&pkt_filter_mode,
			sizeof(pkt_filter_mode),
			IOV_GET,
			NULL);
		pkt_filter_mode &= ~PKT_FILTER_MODE_PKT_FORWARD_OFF_DEFAULT;
		wlc_iovar_op(wowl->wlc,
			"pkt_filter_mode",
			NULL,
			0,
			&pkt_filter_mode,
			sizeof(pkt_filter_mode),
			IOV_SET,
			NULL);
	}

	wowl->wakeind = 0;
	return wowl->wakeind;
}

#ifdef BCMDBG
static void
wlc_print_wowlpattern(wl_wowl_pattern_t *wl_pattern)
{
	uint8 *pattern;
	uint i;
	WL_ERROR(("masksize:%d offset:%d patternsize:%d\nMask:0x",
	          wl_pattern->masksize, wl_pattern->offset, wl_pattern->patternsize));
	pattern = ((uint8 *)wl_pattern + sizeof(wl_wowl_pattern_t));
	for (i = 0; i < wl_pattern->masksize; i++)
		WL_ERROR(("%02x", pattern[i]));
	WL_ERROR(("\nPattern:0x"));
	/* Go to end to find pattern */
	pattern = ((uint8*)wl_pattern + wl_pattern->patternoffset);
	for (i = 0; i < wl_pattern->patternsize; i++)
		WL_ERROR(("%02x", pattern[i]));
	WL_ERROR(("\n"));
}
#endif /* BCMDBG */

bool
wlc_wowlpf_pktfilter_cb(wlc_info_t *wlc,
	uint32 type, uint32 id, const void *patt, const void *sdu)
{
	bool ret = TRUE;
	wowlpf_info_t *wowl = wlc->wowlpf;
	uint8 *pkt = PKTDATA(wlc->osh, sdu);

	if (WOWLPF_ACTIVE(wlc->pub)) {
		if (type == WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH)
			wowl->wakeind = WL_WOWL_MAGIC;
		else
			wowl->wakeind = WL_WOWL_NET;
		if (ETHER_ISBCAST(pkt))
			wowl->wakeind |= WL_WOWL_BCAST;

#if (defined(SECURE_WOWL) && defined(SS_WOWL))
		if (type == WL_PKT_FILTER_TYPE_PATTERN_MATCH) {
			uint8 appid = SS_WOWL_WAKEUP_INDOOR_APP_INVALID;
			wl_pkt_filter_pattern_t *pattern = (wl_pkt_filter_pattern_t *)patt;
			/* normal wake-up by indoor app #0 ~ #2 */
			if (pattern->offset + SS_WOWL_WAKEUP_ID_LEN +
				SS_WOWL_WAKEE_ADDR_LEN < PKTLEN(wlc->osh, sdu))
				appid = pkt[pattern->offset +
					SS_WOWL_WAKEUP_ID_LEN + SS_WOWL_WAKEE_ADDR_LEN];
			if (appid > SS_WOWL_WAKEUP_INDOOR_APP_MAX) {
				WL_SECWOWL_ERR("wl%d: wrong appid %d\n",
					wlc->pub->unit, appid);
				appid = SS_WOWL_WAKEUP_INDOOR_APP_INVALID;
			} else {
				appid += 1; /* change app # 0 ~ 2 to reason # 1 ~ 3 */
			}
			ret = wlc_wowlpf_sswakeup(wowl, appid);
		} else if (type == WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH) {
			/* wakeup by outdoor wowl waker, will do it later in the timer */
			wowl->netsession->wakeup_reason = SS_WOWL_WAKEUP_APP_OUTDOOR;
		} else {
			WL_SECWOWL_ERR("filter type %d not supported\n", type);
		}
#else
		ret = wlc_wowlpf_set_gpio(wlc->hw, wowl->gpio, !wowl->gpio_polarity);
		if (wowl->dngldown)
			wl_wowl_dngldown(wowl->wlc->wl);
#endif /* (defined(SECURE_WOWL) && defined(SS_WOWL)) */
	}

	return ret;
}

bool
wlc_wowlpf_event_cb(wlc_info_t *wlc, uint32 event_type, uint32 reason)
{
	bool ret = TRUE;
	wowlpf_info_t *wowl;

	wowl = wlc->wowlpf;

	if (wowl->wakeind || !WOWLPF_ACTIVE(wlc->pub))
		return ret;

	switch (event_type) {

	case WLC_E_DEAUTH_IND:
	case WLC_E_DISASSOC_IND:

		if (wowl->flags_current & WL_WOWL_DIS)
			wowl->wakeind = WL_WOWL_DIS;
		break;

	case WLC_E_RETROGRADE_TSF:

		if (wowl->flags_current & WL_WOWL_RETR)
			wowl->wakeind = WL_WOWL_RETR;
		break;

	case WLC_E_ROAM:
		if ((reason == WLC_E_REASON_BCNS_LOST) && (wowl->flags_current & WL_WOWL_BCN))
			wowl->wakeind = WL_WOWL_BCN;
		break;

	case WLC_E_LINK:
		if ((reason == WLC_E_LINK_BCN_LOSS) && (wowl->flags_current & WL_WOWL_BCN))
			wowl->wakeind = WL_WOWL_BCN;
		break;

	case WLC_E_PSK_SUP:
		if ((reason == WLC_E_SUP_GTK_DECRYPT_FAIL) ||
			(reason == WLC_E_SUP_GRP_MSG1_NO_GTK)) {
			if (wowl->flags_current & WL_WOWL_GTK_FAILURE)
				wowl->wakeind = WL_WOWL_GTK_FAILURE;
		}
		break;
	}

	if (wowl->wakeind) {
#if defined(SS_WOWL)
		ret = wlc_wowlpf_sswakeup(wowl, SS_WOWL_WAKEUP_AP_LINK_LOST);
#else
		ret = wlc_wowlpf_set_gpio(wlc->hw, wowl->gpio, !wowl->gpio_polarity);
#endif /* SS_WOWL */
	}

	return ret;
}

static void
wlc_wowlpf_dngldown_tmout(void *arg)
{
	wowlpf_info_t *wowl = (wowlpf_info_t *) arg;

	if (wowl->dngldown) {
		/* bring the usb interface down */
		wl_wowl_dngldown(wowl->wlc->wl);

#if defined(SS_WOWL)
		if (wowl->netsession &&
			wowl->netsession->keepalive_interval) {
			wlc_wowlpf_add_timer(wowl, wlc_wowlpf_tmout,
				WOWLPF_TIMEOUT, TRUE, WOWL_TM_PACKET_KEEPALIVE);
		}
#endif // endif
	}
}

static bool
wlc_wowlpf_init_gpio(wlc_hw_info_t *wlc_hw, uint8 wowl_gpio, bool polarity)
{
	if (!wlc_hw->clk) {
		WL_ERROR(("wl: %s: No hw clk \n",  __FUNCTION__));
		return FALSE;
	}

	if (wowl_gpio == WOWL_GPIO_INVALID_VALUE) {
		WL_ERROR(("wl: %s: invalid GPIO\n",  __FUNCTION__));
		return FALSE;
	}

	si_gpiocontrol(wlc_hw->sih,  1 << wowl_gpio, 0, GPIO_DRV_PRIORITY);
	si_gpioout(wlc_hw->sih, 1 <<  wowl_gpio, polarity << wowl_gpio, GPIO_DRV_PRIORITY);
	si_gpioouten(wlc_hw->sih, 1 << wowl_gpio, 1 << wowl_gpio, GPIO_DRV_PRIORITY);

	return TRUE;
}

static bool
wlc_wowlpf_set_gpio(wlc_hw_info_t *wlc_hw, uint8 wowl_gpio, bool polarity)
{
	if (!wlc_hw->clk) {
		WL_ERROR(("wl: %s: No hw clk \n",  __FUNCTION__));
		return FALSE;
	}

	if (wowl_gpio == WOWL_GPIO_INVALID_VALUE) {
		WL_ERROR(("wl: %s: invalid GPIO\n",  __FUNCTION__));
		return FALSE;
	}

	si_gpioout(wlc_hw->sih, 1 <<  wowl_gpio, polarity << wowl_gpio, GPIO_DRV_PRIORITY);

	return TRUE;
}

#ifdef SECURE_WOWL
static uint32 TLS_packet_encode(wowlpf_info_t *wowl,
	uint8 *in, int in_len, uint8 *out, int out_len)
{
	tls_info_t *tls = wowl->tls;
	tls_param_info_t *tlsparam = tls->tlsparam;
	uint32 sequence_len = tlsparam->write_sequence_len;
	uint8 digest_length = tls->digest_length;
	uint8 digest[TLS_MAX_DEGIST_LENGTH];
	uint8 pad_length = ((in_len + digest_length) % tls->block_length) ?
		(tls->block_length - ((in_len + digest_length) % tls->block_length)) :
		tls->block_length;
	uint8 *implicit_iv = NULL;
	uint8 *p = NULL;
	int i, msg_len = tls->explicit_iv_length + in_len + digest_length + pad_length;
	int enc_data_len = 0;

	if (in_len <= 0) {
		WL_SECWOWL_ERR("no valid data, len %d\n", in_len);
		goto ERR_EXIT;
	}

	if (out_len < msg_len + TLS_RECORD_HEADER_LENGTH) {
		WL_SECWOWL_ERR("out put buff %d too short, at least need %d\n", out_len, msg_len);
		goto ERR_EXIT;
	}

	/* record header */
	out[TLS_OFFSET_CONTENTTYPE] = CONTENTTYPE_APPLICATION_DATA;
	out[TLS_OFFSET_VERSION_MAJOR] = tlsparam->version.major;
	out[TLS_OFFSET_VERSION_MINOR] = tlsparam->version.minor;
	/* use real length value to degist */
	out[TLS_OFFSET_LENGTH_HIGH] = in_len >> 8;
	out[TLS_OFFSET_LENGTH_LOW] = in_len & 0xFF;

	p = out + TLS_RECORD_HEADER_LENGTH;
	/* construct the data for digest */
	memcpy(p, tlsparam->write_sequence, sequence_len);
	memcpy(p + sequence_len, out, TLS_RECORD_HEADER_LENGTH);
	memcpy(p + sequence_len + TLS_RECORD_HEADER_LENGTH, in, in_len);

#if SECWOWL_DEBUG
	wowl_prhex("original plain text to be hashed", in, in_len);
	wowl_prhex("write_mac_key", tlsparam->write_mac_key, tlsparam->write_mac_key_len);
	wowl_prhex("write_sequence", tlsparam->write_sequence, sequence_len);
	wowl_prhex("data for digest", p, sequence_len + TLS_RECORD_HEADER_LENGTH + in_len);
#endif // endif

	hmac_sha1(p, sequence_len + TLS_RECORD_HEADER_LENGTH + in_len,
		tlsparam->write_mac_key, tlsparam->write_mac_key_len, digest);

#if SECWOWL_DEBUG
	wowl_prhex("digest output", digest, digest_length);
#endif // endif

	/* add padding bytes to tail part if need */
	memset(p, pad_length - 1, msg_len);
	if (tls->explicit_iv_length)
		wlc_getrand(wowl->wlc, p, tls->explicit_iv_length);
	memcpy(p + tls->explicit_iv_length, in, in_len);
	memcpy(p + tls->explicit_iv_length + in_len, digest, digest_length);

#if SECWOWL_DEBUG
	wowl_prhex("data (original data + digest + padding bytes) to be encrypted", p, msg_len);
	wowl_prhex("write_master_key", tlsparam->write_master_key, tlsparam->write_master_key_len);
	/* wowl_prhex("write_ks", (unsigned char*)tls->write_ks, (int)sizeof(tls->write_ks)); */
	wowl_prhex("write_iv", tlsparam->write_iv, tlsparam->write_iv_len);
#endif // endif

	if (msg_len != aes_cbc_encrypt(tls->write_ks, tlsparam->write_master_key_len,
		tlsparam->write_iv, msg_len, p, out + TLS_RECORD_HEADER_LENGTH)) {
		WL_SECWOWL_ERR("aes_cbc_encrypt failed\n");
		goto ERR_EXIT;
	}
	enc_data_len = msg_len;
	implicit_iv = out + TLS_RECORD_HEADER_LENGTH + msg_len - tlsparam->write_iv_len;

	memcpy(tlsparam->write_iv, implicit_iv, tlsparam->write_iv_len);
	/* increase write sequence number */
	for (i = 7; i >= 0; i--)
		if (++tlsparam->write_sequence[i])
			break;

#if SECWOWL_DEBUG
	wowl_prhex("encrypted data", out + TLS_RECORD_HEADER_LENGTH, msg_len);
	wowl_prhex("next write_iv", tlsparam->write_iv, tlsparam->write_iv_len);
	wowl_prhex("next write_sequence", tlsparam->write_sequence, 8);
#endif // endif

ERR_EXIT:
	if (enc_data_len) {
		/* set encrypted data len to send */
		out[TLS_OFFSET_LENGTH_HIGH] = enc_data_len >> 8;
		out[TLS_OFFSET_LENGTH_LOW] = enc_data_len & 0xFF;
#if SECWOWL_DEBUG
		wowl_prhex("final encrypted SSL/TLS packet", out,
			TLS_RECORD_HEADER_LENGTH + enc_data_len);
#endif // endif
		return TLS_RECORD_HEADER_LENGTH + enc_data_len;
	} else {
		return 0;
	}
}

static uint8* TLS_packet_parse(wowlpf_info_t *wowl, uint8 *data, int data_len, int *pkt_len)
{
	tls_info_t *tls = wowl->tls;
	tls_param_info_t *tlsparam = tls->tlsparam;
	uint32 sequence_len = tlsparam->read_sequence_len;
	uint8 digest_length = tls->digest_length;
	uint8 digest[TLS_MAX_DEGIST_LENGTH];
	uint8 pad_length = 0;
	uint8 *p, *buf = NULL, *bufdec = NULL, *pbufvmac = NULL, *bufrtn = NULL;
	uint8 *implicit_iv = NULL;
	int i, msg_len;

#if SECWOWL_DEBUG
	wowl_prhex("all data to be parsed", data, data_len);
#endif // endif

	if (data[TLS_OFFSET_CONTENTTYPE] != CONTENTTYPE_APPLICATION_DATA) {
		WL_SECWOWL_ERR("not app data, type : %d\n", data[TLS_OFFSET_CONTENTTYPE]);
		*pkt_len = data_len;
		goto ERR_EXIT;
	}
	if (data[TLS_OFFSET_VERSION_MAJOR] != tlsparam->version.major ||
		data[TLS_OFFSET_VERSION_MINOR] != tlsparam->version.minor) {
		WL_SECWOWL_ERR("version mismatch: want %d:%d, get %d:%d\n",
			tlsparam->version.major, tlsparam->version.minor,
			data[TLS_OFFSET_VERSION_MAJOR], data[TLS_OFFSET_VERSION_MINOR]);
		*pkt_len = data_len;
		goto ERR_EXIT;
	}

	*pkt_len = TLS_RECORD_HEADER_LENGTH +
		((data[TLS_OFFSET_LENGTH_HIGH] << 8) | data[TLS_OFFSET_LENGTH_LOW]);
	if (*pkt_len > data_len) {
		WL_SECWOWL_ERR("data length wrong: want %u, get %u\n", *pkt_len, data_len);
		*pkt_len = data_len;
		goto ERR_EXIT;
	}

	msg_len = data_len = *pkt_len;

	buf = MALLOC(tls->wlc->osh, data_len);
	bufdec = MALLOC(tls->wlc->osh, data_len);
	pbufvmac = MALLOC(tls->wlc->osh,
		sequence_len + TLS_RECORD_HEADER_LENGTH + data_len);
	if (!buf || !pbufvmac) {
		WL_SECWOWL_ERR("alloc buff failed\n");
		goto ERR_EXIT;
	}

	p = buf;
	memcpy(p, data, data_len);
	p += TLS_RECORD_HEADER_LENGTH;
	msg_len -= TLS_RECORD_HEADER_LENGTH;

#if SECWOWL_DEBUG
	wowl_prhex("original encrypted SSL/TLS packet", data, data_len);
	wowl_prhex("data to be decrypted", p, msg_len);
	wowl_prhex("read_master_key", tlsparam->read_master_key, tlsparam->read_master_key_len);
	/* wowl_prhex("read_ks", (unsigned char*)tls->read_ks, (int)sizeof(tls->read_ks)); */
	wowl_prhex("read_iv", tlsparam->read_iv, tlsparam->read_iv_len);
#endif // endif

	implicit_iv = p + msg_len - tlsparam->read_iv_len;
	if (aes_cbc_decrypt(tls->read_ks, tlsparam->read_master_key_len,
		tlsparam->read_iv, msg_len, p, bufdec) != msg_len) {
		WL_SECWOWL_ERR("decrypt failed");
		goto ERR_EXIT;
	}
	p = bufdec;

#if SECWOWL_DEBUG
	wowl_prhex("decrypted data", p, msg_len);
#endif // endif

	/* skip the explicit IV, if there is any */
	if (tls->explicit_iv_length) {
		p += tls->explicit_iv_length;
		msg_len -= tls->explicit_iv_length;
	}
	/* remove padding bytes, if there is any */
	pad_length = p[msg_len - 1];
	/* verify padding bytes are correct */
	for (i = 0; i <= pad_length; ++i) {
		if (p[msg_len - 1 - i] != pad_length) {
			WL_SECWOWL_ERR("padding_length 0x%02x, wrong padding byte 0x%02x\n",
				p[msg_len - 1], p[msg_len - 1 - i]);
			goto ERR_EXIT;
		}
	}
	msg_len -= (pad_length + 1);
	/* remove degist comes with the packet */
	msg_len -= digest_length;
	if (msg_len < 0) {
		WL_SECWOWL_ERR("invalid msg_len (%d) < 0\n", msg_len);
		goto ERR_EXIT;
	}

	/* construct the original plain text to regenerate the degist */
	memcpy(pbufvmac, tlsparam->read_sequence, sequence_len);
	memcpy(pbufvmac + sequence_len, data, TLS_RECORD_HEADER_LENGTH);
	/* use real length value to degist */
	pbufvmac[sequence_len + TLS_OFFSET_LENGTH_HIGH] = msg_len >> 8;
	pbufvmac[sequence_len + TLS_OFFSET_LENGTH_LOW] = msg_len & 0xFF;
	memcpy(pbufvmac + sequence_len + TLS_RECORD_HEADER_LENGTH, p, msg_len);

#if SECWOWL_DEBUG
	wowl_prhex("original plain text to be hashed", p, msg_len);
	wowl_prhex("read_mac_key", tlsparam->read_mac_key, tlsparam->read_mac_key_len);
	wowl_prhex("read_sequence", tlsparam->read_sequence, sequence_len);
	wowl_prhex("data for digest", pbufvmac, sequence_len + TLS_RECORD_HEADER_LENGTH + msg_len);
#endif // endif

	hmac_sha1(pbufvmac, sequence_len + TLS_RECORD_HEADER_LENGTH + msg_len,
		tlsparam->read_mac_key, tlsparam->read_mac_key_len, digest);

#if SECWOWL_DEBUG
	wowl_prhex("digest output", digest, digest_length);
	wowl_prhex("digest come with in packet", &p[msg_len], digest_length);
#endif // endif

	/* verify digest */
	if (memcmp(digest, &p[msg_len], digest_length)) {
		WL_SECWOWL_ERR("degist not match\n");
		goto ERR_EXIT;
	}

	memcpy(tlsparam->read_iv, implicit_iv, tlsparam->read_iv_len);
	/* increase read sequence number if everything match */
	for (i = 7; i >= 0; i--)
		if (++tlsparam->read_sequence[i])
			break;

#if SECWOWL_DEBUG
	wowl_prhex("next read_iv", tlsparam->read_iv, tlsparam->read_iv_len);
	wowl_prhex("next read_sequence", tlsparam->read_sequence, 8);
#endif // endif

	if (msg_len) {
		if (tls->mask_and_pattern)
			MFREE(tls->wlc->osh, tls->mask_and_pattern, tls->size_bytes);
		tls->size_bytes = msg_len;
		tls->mask_and_pattern = MALLOC(tls->wlc->osh, msg_len);
		if (tls->mask_and_pattern) {
			memcpy(tls->mask_and_pattern, p, msg_len);
			bufrtn = tls->mask_and_pattern;
		}
	}

ERR_EXIT:
	if (buf)
		MFREE(tls->wlc->osh, buf, data_len);
	if (bufdec)
		MFREE(tls->wlc->osh, bufdec, data_len);
	if (pbufvmac)
		MFREE(tls->wlc->osh, pbufvmac,
			sequence_len + TLS_RECORD_HEADER_LENGTH + data_len);

#if SECWOWL_DEBUG
	if (bufrtn) {
		wowl_prhex("final plain text", bufrtn, msg_len);
	}
#endif // endif

	return bufrtn;
}

static uint8* TLS_packet_merge(wowlpf_info_t *wowl, uint8 **pbuf, int *buf_len)
{
	uint8 *pbuftmp = MALLOC(wowl->tls->wlc->osh, (*buf_len) + wowl->tls->size_bytes);
	if (!pbuftmp) {
		WL_SECWOWL_ERR("exit: MALLOC failed\n");
		return NULL;
	}
	if (*pbuf) {
		memcpy(pbuftmp, *pbuf, *buf_len);
		MFREE(wowl->tls->wlc->osh, *pbuf, *buf_len);
	}
	memcpy(pbuftmp + (*buf_len), wowl->tls->mask_and_pattern, wowl->tls->size_bytes);
	(*buf_len) += wowl->tls->size_bytes;
	return *pbuf = pbuftmp;
}

static uint8* TCP_packet_parse(wowlpf_info_t *wowl, uint8 *data, int data_len)
{
	wowl_net_session_t *netsession = wowl->netsession;
	struct bcmtcp_hdr *tcp_hdr = (struct bcmtcp_hdr*)data;
	uint16 tcp_flags = TCP_FLAGS(ntoh16(tcp_hdr->hdrlen_rsvd_flags));
	uint8 tcp_ack = tcp_flags & TCP_FLAG_ACK;
	uint8 tcp_push = tcp_flags & TCP_FLAG_PSH;
	int tcp_hdr_len = 4 *
		((ntoh16(tcp_hdr->hdrlen_rsvd_flags) & TCP_HLEN_MASK) >> TCP_HLEN_SHIFT);
	int data_remain = data_len - tcp_hdr_len;
	int tlspkt_len = 0;
	uint8 *pdata = NULL;
	uint8 *ptlspkt = data + tcp_hdr_len;
	uint8 *pbuf = NULL;
	int buf_len = 0;
	uint32 ack_num, seq_num;

	if (data_len < tcp_hdr_len) {
		WL_WOWL_ERR("exit: tcp packet data not completed\n");
		goto EXIT;
	}

	if ((tcp_hdr->dst_port != netsession->local_port) ||
		(tcp_hdr->src_port != netsession->remote_port)) {
		WL_WOWL_ERR("exit: recriving, port not as expected\n");
		goto EXIT;
	}

#if defined(SS_WOWL)
	if (tcp_flags & (TCP_FLAG_FIN | TCP_FLAG_SYN | TCP_FLAG_RST)) {
		WL_WOWL_ERR("wake up host: connection changed 0x%x\n",
			(tcp_flags & (TCP_FLAG_FIN | TCP_FLAG_SYN | TCP_FLAG_RST)));
		if (!netsession->wakeup_reason)
			netsession->wakeup_reason = SS_WOWL_WAKEUP_SERVER_LINK_FAIL;
		if (!netsession->fin_sent)
			netsession->fin_recv = 1;
		else
			wlc_wowlpf_sswakeup(wowl, netsession->wakeup_reason);
	}
#endif /* SS_WOWL */

	seq_num = ntoh32(tcp_hdr->seq_num);
	ack_num = ntoh32(tcp_hdr->ack_num);
	if ((!tcp_ack) || (netsession->lseq &&
		(netsession->lseq + netsession->bytecnt_sending != ack_num))) {
		WL_WOWL_ERR("exit: no ack or ack number wrong? lseq(0x%x)+len(%d)!=ack(0x%x)\n",
			netsession->lseq, netsession->bytecnt_sending, ack_num);
		goto EXIT;
	}
	if ((tcp_push) && netsession->rseq &&
		(netsession->rseq != seq_num)) {
		WL_WOWL_ERR("exit: wrong rseq number? known 0x%x get 0x%x\n",
			netsession->rseq, seq_num);
		if (netsession->rseq == seq_num + data_remain)
			netsession->ack_required = 1; /* prior ack lost */
		goto EXIT;
	}

	WL_INFORM(("rseq was 0x%x, changed to 0x%x\n", netsession->rseq, seq_num + data_remain));
	netsession->rseq = seq_num + data_remain;
	WL_INFORM(("lseq was 0x%x, changed to 0x%x\n", netsession->lseq, ack_num));
	netsession->lseq = ack_num;
	WL_INFORM(("bytecnt_sending was %d, reset to 0\n", netsession->bytecnt_sending));
	netsession->bytecnt_sending = 0;

	if ((!tcp_push) || (data_len < tcp_hdr_len + TLS_RECORD_HEADER_LENGTH)) {
		WL_WOWL_ERR("exit: no valid ssl/tls packet data\n");
		goto EXIT;
	}

#if SECWOWL_DEBUG
	wowl_prhex("original encrypted SSL/TLS packet", ptlspkt, data_remain);
#endif // endif

	while (data_remain > 0) {
		if (pdata && !TLS_packet_merge(wowl, &pbuf, &buf_len)) {
			WL_WOWL_ERR("exit: failed to merge ssl/tls packet data\n");
			pdata = NULL;
			goto EXIT;
		}
		pdata = TLS_packet_parse(wowl, ptlspkt, data_remain, &tlspkt_len);
		ptlspkt += tlspkt_len;
		data_remain -= tlspkt_len;
	}
	if (pdata && pbuf && !TLS_packet_merge(wowl, &pbuf, &buf_len)) {
		WL_WOWL_ERR("exit: failed to merge ssl/tls packet data\n");
		pdata = NULL;
		goto EXIT;
	}
	if (pbuf) {
		MFREE(wowl->tls->wlc->osh, wowl->tls->mask_and_pattern, wowl->tls->size_bytes);
		wowl->tls->size_bytes = buf_len;
		pdata = wowl->tls->mask_and_pattern = pbuf;
	}

EXIT:
	return pdata;
}

static uint8* IP_packet_parse(wowlpf_info_t *wowl, uint8 *data, int data_len)
{
	uint8* pdata = NULL;
	wowl_net_session_t *netsession = wowl->netsession;
	struct ipv4_hdr *ip_hdr = (struct ipv4_hdr *)data;
	uint32 ip_hdr_len = IPV4_HLEN(ip_hdr);

	if (((ip_hdr->version_ihl & IPV4_VER_MASK) >> IPV4_VER_SHIFT) != IP_VER_4)  {
		WL_WOWL_ERR("exit: Not IPv4\n");
		goto EXIT;
	}
	if (ip_hdr_len < IPV4_MIN_HEADER_LEN)  {
		WL_WOWL_ERR("exit: ip packet header not correct\n");
		goto EXIT;
	}

	if (data_len < ntoh16(ip_hdr->tot_len))  {
		WL_WOWL_ERR("exit: packet data imcomplete, wanted %d bytes, get %d bytes\n",
			ntoh16(ip_hdr->tot_len), data_len);
		goto EXIT;
	}
	/* ignore extra bytes, if any */
	if (data_len > ntoh16(ip_hdr->tot_len))  {
		WL_WOWL_ERR("info: ignore %d bytes\n", data_len - ntoh16(ip_hdr->tot_len));
		goto EXIT;
	}
	data_len = ntoh16(ip_hdr->tot_len);

	if ((ntoh16(ip_hdr->frag) & IPV4_FRAG_MORE) ||
		(ntoh16(ip_hdr->frag) & IPV4_FRAG_OFFSET_MASK)) {
		WL_WOWL_ERR("exit: fragmented packet\n");
		goto EXIT;
	}

	if (ip_hdr->prot != IP_PROT_TCP)  {
		/* WL_WOWL_ERR("exit: Not TCP\n"); */
		goto EXIT;
	}

	if (memcmp(netsession->remote_ip, ip_hdr->src_ip, IPV4_ADDR_LEN) ||
		memcmp(netsession->local_ip, ip_hdr->dst_ip, IPV4_ADDR_LEN)) {
		WL_WOWL_ERR("exit: ip addr wrong\n");
		goto EXIT;
	}

	pdata = TCP_packet_parse(wowl, data + ip_hdr_len, data_len - ip_hdr_len);

EXIT:
	return pdata;
}

static uint8* ETH_packet_parse(wowlpf_info_t *wowl, uint8 *data, int data_len)
{
	uint8* pdata = NULL;

	/* Ethernet II frames
	-------------------------------------------------
	| DA (6) | SA (6) | type (2) | data (46 - 1500) |
	-------------------------------------------------
	*/
	if (data_len < ETHER_HDR_LEN) {
		WL_WOWL_ERR("exit: ethernet packet header not complete\n");
		goto EXIT;
	}

	if (memcmp((uint8*)&ether_bcast, data, ETHER_ADDR_LEN) &&
		memcmp(wowl->netsession->local_macaddr, data, ETHER_ADDR_LEN)) {
		WL_WOWL_ERR("exit: MAC addr wrong\n");
		goto EXIT;
	}

	if (((data[ETHER_TYPE_OFFSET] << 8) | data[ETHER_TYPE_OFFSET+1])
		!= ETHER_TYPE_IP)  {
		/* WL_WOWL_ERR("exit: not IPv4 protocol\n"); */
		goto EXIT;
	}

	if (data_len < ETHER_HDR_LEN + IPV4_MIN_HEADER_LEN) {
		WL_WOWL_ERR("exit: ip header not complete\n");
		goto EXIT;
	}

	pdata = IP_packet_parse(wowl, data + ETHER_HDR_LEN, data_len - ETHER_HDR_LEN);

EXIT:
	if (pdata)
		memcpy(wowl->netsession->remote_macaddr, data + ETHER_ADDR_LEN, ETHER_ADDR_LEN);

	return pdata;
}

#if WOWL_PARSE_D11_PKT
static uint8* DOT11_packet_parse(wowlpf_info_t *wowl, uint8 *data, int data_len)
{
	uint8* pdata = NULL;
	uint32 length = 0;
	uint16 fc;

	if (data_len < (length + SNAP_HDR_LEN + ETHER_TYPE_LEN))  {
		WL_WOWL_ERR("exit: too short\n");
		goto EXIT;
	}

	if (!memcmp(&data[length], llc_snap_hdr, SNAP_HDR_LEN)) {
		WL_WOWL_ERR("start with llc_snap_hdr, skip 802.11 header parse\n");
		goto LLC_PDU;
	}

	/* 802.11 frames
	------------------------------------------------------------------------------------------
	| FC (2) | DID (2) |A1 (6) |A2 (6)|A3 (6) |SID (2) |SNAP (6) |type (2) |data (46 - 1500) |
	------------------------------------------------------------------------------------------
	*/
	fc = ((data[length] << 8) | data[length + 1]);

	length = 2 + 2 + ETHER_ADDR_LEN; /* fc[2] + did[2] + addr1[6], cts frame size */
	if (data_len < length)  {
		WL_WOWL_ERR("exit: length not enough for mininum 802.11 frame\n");
		goto EXIT;
	}
	if ((fc & FC_PVER_MASK) >>  FC_PVER_SHIFT) {
		WL_WOWL_ERR("exit: not 802.11 ver 2\n");
		goto EXIT;
	}
	if (((fc & FC_TYPE_MASK) >> FC_TYPE_SHIFT) != FC_TYPE_DATA) {
		WL_WOWL_ERR("exit: not data frame\n");
		goto EXIT;
	}
	/* data frame should include addr2[6] + addr3[6] */
	length += ETHER_ADDR_LEN + ETHER_ADDR_LEN;
	/* secquence control field exists in all data frame subtypes */
	if (((data[length] << 8) | data[length + 1]) & FRAGNUM_MASK) {
		WL_WOWL_ERR("exit: more than one fragment\n");
		goto EXIT;
	}
	length += 2; /* seq[2] */
	if (FC_SUBTYPE_ANY_NULL((fc & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT)) {
		WL_WOWL_ERR("exit: no frame body field\n");
		goto EXIT;
	}
	if (FC_SUBTYPE_ANY_QOS((fc & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT)) {
		WL_WOWL_ERR("exit: qos present\n");
		length += DOT11_QOS_LEN; /* qos[2] */
		goto EXIT;
	}
	if ((fc & (FC_TODS|FC_FROMDS)) == (FC_TODS|FC_FROMDS))  {
		WL_WOWL_ERR("address 4 present\n");
		length += ETHER_ADDR_LEN; /*  addr4[6] */
	}
	if (fc & FC_MOREFRAG) {
		WL_WOWL_ERR("exit: fragmented frame\n");
		goto EXIT;
	}
	if (fc & FC_WEP) {
		WL_WOWL_ERR("exit: protedted frame\n");
		goto EXIT;
	}
	if ((fc & FC_ORDER) &&
		(FC_SUBTYPE_ANY_QOS((fc & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT))) {
		WL_WOWL_ERR("qos data frame with order field set contains HT Control field\n");
		length += DOT11_HTC_LEN; /* HT Control field[4] */
	}
	if (data_len < (length + SNAP_HDR_LEN + ETHER_TYPE_LEN)) { /* llc_hdr[6] + type[2] */
		WL_WOWL_ERR("exit: frame data not complete, want %d, got %d\n", length, data_len);
		goto EXIT;
	}

	if (memcmp(&data[length], llc_snap_hdr, SNAP_HDR_LEN)) {
		WL_WOWL_ERR("exit: not llc_snap_hdr or ip packet\n");
		goto EXIT;
	}

LLC_PDU:
	length += SNAP_HDR_LEN; /* llc_hdr[6] + type[2] */
	if (((data[length] << 8) | data[length + 1]) != ETHER_TYPE_IP) {
		WL_WOWL_ERR("exit: not IPv4 protocol\n");
		goto EXIT;
	}
	length += ETHER_TYPE_LEN;
	if (data_len < length + IPV4_MIN_HEADER_LEN) {
		WL_WOWL_ERR("exit: ip header not complete\n");
		goto EXIT;
	}

	pdata = IP_packet_parse(wowl, data + length, data_len - length);
	if (pdata) {
		ASSERT(0); /* need set the local / remote mac address */
		/* MAC address possition different from ethernet packet */
	}

EXIT:
	return pdata;
}
#endif /* WOWL_PARSE_D11_PKT */

static uint8* wlc_wowlpf_secpkt_parse(void *ctx, const void *sdu, int sending)
{
	wowlpf_info_t *wowl = (wowlpf_info_t*)ctx;
	uint8 *pdata = NULL;
	uint8 *pkt = PKTDATA(wowl->wlc->osh, sdu);
	int   data_len = PKTLEN(wowl->wlc->osh, sdu);

	pdata = ETH_packet_parse(wowl, pkt, data_len);
	if (pdata) {
		/* WL_INFORM(("encrypted packet is Ethernet II frame\n")); */
		goto EXIT;
	}

	#if WOWL_PARSE_D11_PKT
	pdata = DOT11_packet_parse(wowl, pkt, data_len);
	if (pdata) {
		/* WL_INFORM(("encrypted packet is 802.11 frames\n")); */
		goto EXIT;
	}
	#endif /* WOWL_PARSE_D11_PKT */

EXIT:
	if (pdata || wowl->netsession->ack_required || wowl->netsession->fin_recv) {
		if (!wlc_wowlpf_add_timer(wowl, wlc_wowlpf_tmout,
			10, FALSE, WOWL_TM_PACKET_ACK)) {
			pdata = NULL;
		}
	}

	return pdata;
}

static tls_info_t *
wlc_secwowl_alloc(wowlpf_info_t *wowl)
{
	tls_info_t *tls;

	if (!(tls = (tls_info_t *)MALLOC(wowl->wlc->osh, sizeof(tls_info_t)))) {
		WL_SECWOWL_ERR("wl%d: wlc_secwowl_alloc: out of mem, malloced %d bytes\n",
			wowl->wlc->pub->unit, MALLOCED(wowl->wlc->osh));
		return NULL;
	}

	bzero((char *)tls, sizeof(tls_info_t));
	tls->wlc = wowl->wlc;

	return tls;
}

static void wlc_wowlpf_tmout(void *arg)
{
	wowlpf_info_t *wowl = (wowlpf_info_t*)arg;
	wlc_info_t *wlc = wowl->wlc;
	wowl_net_session_t *netsession = wowl->netsession;
	uint8 *pkt;
	struct bcmtcp_hdr *tcp_hdr;
	struct ipv4_hdr *ip_hdr;
	struct ether_header *eth_hdr;
	uint8 *pseudo_hdr, *data;
	uint16 hdrlen_rsvd_flags = ((TCP_MIN_HEADER_LEN / 4) << TCP_HLEN_OFFSET) | TCP_FLAG_ACK;
	int    pkt_len = ETHER_HDR_LEN + IPV4_MIN_HEADER_LEN + TCP_MIN_HEADER_LEN;
	int    data_len = 0;

#if defined(SS_WOWL)
	if ((netsession->fin_recv) ||
		(netsession->wakeup_reason == SS_WOWL_WAKEUP_APP_OUTDOOR)) {
		netsession->fin_sent = 1;
		hdrlen_rsvd_flags |= TCP_FLAG_FIN;
	}
	if (wowl->tm_type == WOWL_TM_PACKET_KEEPALIVE) {
		if ((!netsession->lseq) || (!netsession->rseq)) {
			goto PUSH_EMPTY_PKT;
		}

		if (!netsession->bytecnt_sending) {
			tls_param_info_t *tlsparam = wowl->tls->tlsparam;
			uint32 app_syncid;
			uint8 ping_req[10] = {0x00, 0x06, 0x00, 0x00, 0x08, 0x00};
			uint8 *p = &ping_req[5];
			uint8 msg_len = 1;
			tlsparam->app_syncid++;
			tlsparam->app_syncid %= SS_WOWL_MAX_ASSYNC_ID;
			app_syncid = tlsparam->app_syncid;
			do {
				*p = (uint8)(app_syncid & 0x7F);
				*p++ |= (app_syncid & 0xFFFFFF80) ? 0x80 : 0x00;
				app_syncid = app_syncid >> 7;
				msg_len++;
			} while (app_syncid);
			ping_req[3] = msg_len;
			netsession->bytecnt_sending = TLS_packet_encode(wowl,
				ping_req, SS_WOWL_PING_HEADER_SIZE + msg_len,
				netsession->message_sending, WOWL_SENDINGPKT_MAX_LEN);
			netsession->resend_times = 0;
			WL_SECWOWL_ERR("lseq was 0x%x, rseq 0x%x, send new pkt %d bytes\n",
				netsession->lseq,
				netsession->rseq,
				netsession->bytecnt_sending);
		} else {
			if (netsession->resend_times >= SS_WOWL_MAX_RESEND_TIMES) {
				WL_SECWOWL_ERR("wake up host: %d times failed to send\n",
					netsession->resend_times);
				wlc_wowlpf_sswakeup(wowl, SS_WOWL_WAKEUP_SERVER_LINK_FAIL);
			}
			WL_SECWOWL_ERR("lseq was 0x%x, rseq 0x%x, resend pkt %d bytes\n",
				netsession->lseq,
				netsession->rseq,
				netsession->bytecnt_sending);
			netsession->resend_times++;
		}
		data_len = netsession->bytecnt_sending;
	}
	pkt_len += data_len;

PUSH_EMPTY_PKT:
#endif /* SS_WOWL */
	if ((!netsession->lseq) || (!netsession->rseq)) {
		WL_INFORM(("seq 0x%x or ack 0x%x no valid, give an empty push\n",
			netsession->lseq, netsession->rseq));
		/* sending an empty pkt and get seq and ack number from the ACK pkt */
		hdrlen_rsvd_flags |= TCP_FLAG_PSH;
	}

	pkt = PKTALLOC(wlc->osh, pkt_len, lbuf_basic);
	if (!pkt)
		return;

	bzero(PKTDATA(wlc->osh, pkt), PKTLEN(wlc->osh, pkt));
	PKTPULL(wlc->osh, pkt, pkt_len);

	if (data_len) {
		PKTPUSH(wlc->osh, pkt, data_len);
		data = (uint8*)PKTDATA(wlc->osh, pkt);
		memcpy(data, netsession->message_sending, data_len);
		hdrlen_rsvd_flags |= TCP_FLAG_PSH;
	}

	PKTPUSH(wlc->osh, pkt, TCP_MIN_HEADER_LEN);
	tcp_hdr = (struct bcmtcp_hdr*)PKTDATA(wlc->osh, pkt);
	tcp_hdr->src_port = (netsession->local_port);
	tcp_hdr->dst_port = (netsession->remote_port);
	tcp_hdr->seq_num = hton32(netsession->lseq);
	tcp_hdr->ack_num = hton32(netsession->rseq);

	tcp_hdr->hdrlen_rsvd_flags = hton16(hdrlen_rsvd_flags);
	tcp_hdr->tcpwin = hton16(1024); /* pretend we have 1024 TCP window size */
	tcp_hdr->chksum = 0;
	tcp_hdr->urg_ptr = 0;

	PKTPUSH(wlc->osh, pkt, TCP_PSEUDO_HEADER_LEN);
	pseudo_hdr = (uint8 *)PKTDATA(wlc->osh, pkt);
	memcpy(&pseudo_hdr[0], netsession->local_ip, IPV4_ADDR_LEN);
	memcpy(&pseudo_hdr[4], netsession->remote_ip, IPV4_ADDR_LEN);
	pseudo_hdr[9] = IP_PROT_TCP;
	pseudo_hdr[10] = ((TCP_MIN_HEADER_LEN + data_len) & 0xFF00) >> 8;
	pseudo_hdr[11] = (TCP_MIN_HEADER_LEN + data_len) & 0x00FF;
	tcp_hdr->chksum = hton16(bcm_ip_cksum((uint8 *)pseudo_hdr,
		TCP_PSEUDO_HEADER_LEN + TCP_MIN_HEADER_LEN + data_len, 0));
	PKTPULL(wlc->osh, pkt, TCP_PSEUDO_HEADER_LEN);

	PKTPUSH(wlc->osh, pkt, IPV4_MIN_HEADER_LEN);
	ip_hdr = (struct ipv4_hdr *)PKTDATA(wlc->osh, pkt);
	ip_hdr->version_ihl = (IP_VER_4 << IP_VER_SHIFT) | (IPV4_MIN_HEADER_LEN / 4);
	ip_hdr->tos = 0;
	ip_hdr->tot_len = hton16(pkt_len - ETHER_HDR_LEN);
	ip_hdr->id = 0;
	ip_hdr->frag = hton16(IPV4_FRAG_DONT);
	ip_hdr->ttl = 64;
	ip_hdr->prot = IP_PROT_TCP;
	ip_hdr->hdr_chksum = 0;
	memcpy(ip_hdr->src_ip, netsession->local_ip, IPV4_ADDR_LEN);
	memcpy(ip_hdr->dst_ip, netsession->remote_ip, IPV4_ADDR_LEN);
	ip_hdr->hdr_chksum = hton16(bcm_ip_cksum((uint8 *)ip_hdr, IPV4_HLEN(ip_hdr), 0));

	PKTPUSH(wlc->osh, pkt, ETHER_HDR_LEN);
	eth_hdr = (struct ether_header *)PKTDATA(wlc->osh, pkt);
	memcpy(eth_hdr->ether_dhost, netsession->remote_macaddr, ETHER_ADDR_LEN);
	memcpy(eth_hdr->ether_shost, netsession->local_macaddr, ETHER_ADDR_LEN);
	eth_hdr->ether_type = hton16(ETHER_TYPE_IP);

	netsession->ack_required = 0;
	if (netsession->terminated)
		return;

	wlc_sendpkt(wlc, pkt, NULL);

#if defined(SS_WOWL)
	if ((netsession->fin_recv) && (netsession->fin_sent)) {
		wlc_wowlpf_sswakeup(wowl, netsession->wakeup_reason);
	} else
#endif /* SS_WOWL */
	if (netsession->keepalive_interval) {
		wlc_wowlpf_add_timer(wowl, wlc_wowlpf_tmout,
			netsession->keepalive_interval * MS_PER_SECOND, FALSE,
			WOWL_TM_PACKET_KEEPALIVE);
	}
}
#endif /* SECURE_WOWL */

static void
wlc_secwowl_free(wowlpf_info_t *wowl)
{
#ifdef SECURE_WOWL
	tls_info_t *tls = wowl->tls;

	if (!tls)
		return;

	wlc_wowlpf_clear(wowl);

	if (wowl->netsession) {
		MFREE(wowl->wlc->osh, wowl->netsession, sizeof(wowl_net_session_t));
	}

	if (tls->tlsparam) {
		MFREE(wowl->wlc->osh, tls->tlsparam, tls->tlsparam_size);
	}
	if (tls->mask_and_pattern) {
		MFREE(wowl->wlc->osh, tls->mask_and_pattern, tls->size_bytes);
	}

	MFREE(wowl->wlc->osh, tls, sizeof(tls_info_t));
	wowl->tls = NULL;
#else
	return;
#endif /* SECURE_WOWL */
}

#ifdef SS_WOWL
static int
wlc_secwowl_activate_get(wowlpf_info_t *wowl, void *pdata, int datalen)
{
#ifdef SECURE_WOWL
	if (!wowl->tls->tlsparam)
		return BCME_UNSUPPORTED;
	if (sizeof(tls_param_info_t) > datalen)
		return BCME_NOMEM;

	bcopy(wowl->tls->tlsparam, pdata, sizeof(tls_param_info_t));
	return BCME_OK;
#else
	return BCME_UNSUPPORTED;
#endif /* SECURE_WOWL */
}

static int
wlc_secwowl_activate_set(wowlpf_info_t *wowl, void *pdata, int datalen)
{
#ifdef SECURE_WOWL
	int err = BCME_OK;
	wlc_info_t *wlc = wowl->wlc;
	tls_param_info_t *tlsparam = NULL;
	wowl_net_session_t *netsession = NULL;
	ASSERT(sizeof(tls_param_info_t) == datalen);

	wlc_secwowl_free(wowl);
	if (!(wowl->tls = wlc_secwowl_alloc(wowl))) {
		err = BCME_NOMEM;
		goto exit;
	}
	if ((tlsparam = MALLOC(wlc->osh, datalen)) == NULL) {
		err = BCME_NOMEM;
		goto exit;
	}
	bcopy((uint8 *)pdata, (uint8 *)tlsparam, datalen);
	if ((netsession = MALLOC(wlc->osh, sizeof(wowl_net_session_t))) == NULL) {
		err = BCME_NOMEM;
		goto exit;
	}
	bzero(netsession, sizeof(wowl_net_session_t));

	/* rfc5246 Appendix C, page 83.  Cipher Suite Definitions */
	wowl->tls->block_length = 16;
	wowl->tls->iv_length = 16;
	wowl->tls->digest_length = 20;
	wowl->tls->mac_key_length = 20;

	if ((tlsparam->version.major >= 3) && (tlsparam->version.minor >= 2))
		wowl->tls->explicit_iv_length = wowl->tls->iv_length;
	else
		wowl->tls->explicit_iv_length = 0;

#if SECWOWL_DEBUG
	if (tlsparam->compression_algorithm != COMPRESSIONMETHOD_NULL) {
		WL_SECWOWL_ERR("wl%d: compressionmethod %d not support\n",
			wlc->pub->unit, tlsparam->compression_algorithm);
		err = BCME_BADARG;
		goto exit;
	}
	if (tlsparam->cipher_algorithm != BULKCIPHERALGORITHM_AES) {
		WL_SECWOWL_ERR("wl%d: cipher_algorithm %d not support\n",
			wlc->pub->unit, tlsparam->cipher_algorithm);
		err = BCME_BADARG;
		goto exit;
	}
	if (tlsparam->cipher_type != CIPHERTYPE_BLOCK) {
		WL_SECWOWL_ERR("wl%d: cipher_type %d not support\n",
			wlc->pub->unit, tlsparam->cipher_type);
		err = BCME_BADARG;
		goto exit;
	}
	if (tlsparam->mac_algorithm != MACALGORITHM_HMAC_SHA1) {
		WL_SECWOWL_ERR("wl%d: mac_algorithm %d not support\n",
			wlc->pub->unit, tlsparam->mac_algorithm);
		err = BCME_BADARG;
		goto exit;
	}
	if (wowl->tls->iv_length != tlsparam->read_iv_len ||
		wowl->tls->mac_key_length != tlsparam->read_mac_key_len ||
		wowl->tls->iv_length != tlsparam->write_iv_len ||
		wowl->tls->mac_key_length != tlsparam->write_mac_key_len) {
		WL_SECWOWL_ERR("wl%d: wrong length of iv (%d:%d:%d) / mac (%d:%d:%d)\n",
			wlc->pub->unit,
			wowl->tls->iv_length, tlsparam->read_iv_len, tlsparam->write_iv_len,
			wowl->tls->mac_key_length, tlsparam->read_mac_key_len,
			tlsparam->write_mac_key_len);
		err = BCME_BADARG;
		goto exit;
	}
#endif /* SECWOWL_DEBUG */

	if (tlsparam->cipher_algorithm == BULKCIPHERALGORITHM_AES) {
		if (rijndaelKeySetupDec(wowl->tls->read_ks,
			tlsparam->read_master_key, tlsparam->read_master_key_len * 8) < 0) {
			WL_SECWOWL_ERR("wl%d: rijndaelKeySetupDec failed\n", wlc->pub->unit);
			err = BCME_BADARG;
			goto exit;
		}
		if (rijndaelKeySetupEnc(wowl->tls->write_ks,
			tlsparam->write_master_key, tlsparam->write_master_key_len * 8) < 0) {
			WL_SECWOWL_ERR("wl%d: rijndaelKeySetupEnc failed\n", wlc->pub->unit);
			err = BCME_BADARG;
			goto exit;
		}
	}

	wowl->tls->tlsparam = tlsparam;
	wowl->tls->tlsparam_size = datalen;
	wowl->netsession = netsession;
	netsession->keepalive_interval = tlsparam->keepalive_interval;
	netsession->rseq = ntoh32(tlsparam->tcp_ack_num);
	netsession->lseq = ntoh32(tlsparam->tcp_seq_num);
	netsession->remote_port = tlsparam->remote_port;
	netsession->local_port = tlsparam->local_port;
	memcpy(netsession->remote_ip, tlsparam->remote_ip, IPV4_ADDR_LEN);
	memcpy(netsession->local_ip, tlsparam->local_ip, IPV4_ADDR_LEN);
	memcpy(netsession->remote_macaddr, tlsparam->remote_mac_addr, ETHER_ADDR_LEN);
	memcpy(netsession->local_macaddr, tlsparam->local_mac_addr, ETHER_ADDR_LEN);

#if SECWOWL_DEBUG
	wowl_prhex("read_master_key", tlsparam->read_master_key, tlsparam->read_master_key_len);
	wowl_prhex("read_master_key stream", (unsigned char *)wowl->tls->read_ks,
		(int)sizeof(wowl->tls->read_ks));
	wowl_prhex("write_master_key", tlsparam->write_master_key, tlsparam->write_master_key_len);
	wowl_prhex("write_master_key stream", (unsigned char *)wowl->tls->write_ks,
		(int)sizeof(wowl->tls->write_ks));
	wowl_prhex("read_iv", tlsparam->read_iv, tlsparam->read_iv_len);
	wowl_prhex("write_iv", tlsparam->write_iv, tlsparam->write_iv_len);
	wowl_prhex("read_mac_key", tlsparam->read_mac_key, tlsparam->read_mac_key_len);
	wowl_prhex("write_mac_key", tlsparam->write_mac_key, tlsparam->write_mac_key_len);
	wowl_prhex("read_sequence", tlsparam->read_sequence, tlsparam->read_sequence_len);
	wowl_prhex("write_sequence", tlsparam->write_sequence, tlsparam->write_sequence_len);
	wowl_prhex("remote_mac_addr", tlsparam->remote_mac_addr, ETHER_ADDR_LEN);
	wowl_prhex("local_mac_addr", tlsparam->local_mac_addr, ETHER_ADDR_LEN);
	WL_SECWOWL_ERR("rseq %u 0x%x\n", netsession->rseq, netsession->rseq);
	WL_SECWOWL_ERR("lseq %u 0x%x\n", netsession->lseq, netsession->lseq);
#endif /* SECWOWL_DEBUG */

	secwowl_decrypt_ctx.dec_cb = wlc_wowlpf_secpkt_parse;
	secwowl_decrypt_ctx.dec_ctx = (void*)wowl;

exit:
	if (err) {
		if (tlsparam)
			MFREE(wlc->osh, tlsparam, datalen);
		if (netsession)
			MFREE(wlc->osh, netsession, sizeof(wowl_net_session_t));
	} else if (wlc_wowlpf_enable(wowl)) {
		if (netsession->keepalive_interval && !wowl->dngldown) {
			wlc_wowlpf_add_timer(wowl, wlc_wowlpf_tmout,
				WOWLPF_TIMEOUT, FALSE, WOWL_TM_PACKET_KEEPALIVE);
		}
	}
	return err;
#else
	return BCME_UNSUPPORTED;
#endif /* SECURE_WOWL */
}

static int
wlc_wowlpf_wakeup(wowlpf_info_t *wowl, int action)
{
#ifdef SS_WOWL
	if (wlc_wowlpf_sswakeup(wowl, action))
		return BCME_OK;
	else
		return BCME_RANGE;
#else
	return BCME_UNSUPPORTED;
#endif /* SS_WOWL */
}
#endif /* SS_WOWL */
