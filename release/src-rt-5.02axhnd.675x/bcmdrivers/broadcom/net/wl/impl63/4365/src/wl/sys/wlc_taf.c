/*      wlc_taf.c
 *
 *	This file implements the WL driver infrastructure for the TAF module.
 *
 *      Copyright 2020 Broadcom
 *
 *      This program is the proprietary software of Broadcom and/or
 *      its licensors, and may only be used, duplicated, modified or distributed
 *      pursuant to the terms and conditions of a separate, written license
 *      agreement executed between you and Broadcom (an "Authorized License").
 *      Except as set forth in an Authorized License, Broadcom grants no license
 *      (express or implied), right to use, or waiver of any kind with respect to
 *      the Software, and Broadcom expressly reserves all rights in and to the
 *      Software and all intellectual property rights therein.  IF YOU HAVE NO
 *      AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *      WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *      THE SOFTWARE.
 *
 *      Except as expressly set forth in the Authorized License,
 *
 *      1. This program, including its structure, sequence and organization,
 *      constitutes the valuable trade secrets of Broadcom, and you shall use
 *      all reasonable efforts to protect the confidentiality thereof, and to
 *      use this information only in connection with your use of Broadcom
 *      integrated circuit products.
 *
 *      2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *      "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *      REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *      OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *      DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *      NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *      ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *      CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *      OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *      3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *      BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *      SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *      IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *      IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *      ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *      OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *      NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *	$Id$
 */
/*
 * Include files.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#ifdef BCM_HOST_MEM_SCB
#include <wlc_scb_alloc.h>
#endif // endif

#include <wlc_taf.h>
#include <wlc_scb_ratesel.h>

#include <wlc_ampdu.h>
#include <wlc_nar.h>
#ifdef WLATF
#include <wlc_airtime.h>
#endif // endif
#ifdef AP
#include <wlc_apps.h>
#endif // endif

#if defined(WLTAF) && !defined(WLC_LOW)
#error WLTAF can only be enabled with WLC_LOW
#endif // endif

#if defined(WLTAF) && !defined(WLATF)
#error WLTAF needs WLATF
#endif // endif

/* NAR isn't done yet - TODO */
#define TAF_ENABLE_NAR		0

/* Max count of total TAF scb cubbies. When reached, reuse the oldest one. Must be >= MAXSCB */
#define TAF_SCB_CNT_MAX		MAXSCB

#define TAF_SCORE_MAX		0xFFFFFFFF
#define TAF_COEFF_ATOS		4016
#define TAF_COEFF_ATOS_MAX	4096 /* coefficient for normalisation */
#define TAF_SCORE_ATOS_MAX	(TAF_SCORE_MAX / TAF_COEFF_ATOS_MAX) /* max val prior norm */
#define TAF_COEFF_EBOS		4
#define TAF_COEFF_EBOS_MAX	64

#define TAF_SCHED_NAME(type)	(((type) < NUM_TAF_SCHEDULERS) ?\
					taf_scheduler_definitions[(type)].name : "undefined")
#define TAF_DUMP_NAME(type)	(((type) < NUM_TAF_SCHEDULERS) ?\
					taf_scheduler_definitions[(type)].dump_name : "")

#define TAF_TIME_FORCE_DEFAULT	500
#define TAF_TIME_HIGH_MAX	16000
#define TAF_TIME_LOW_MAX	16000
#define TAF_TIME_HIGH_DEFAULT	6000
#define TAF_TIME_LOW_DEFAULT	2000
#define TAF_TIME_ATOS_HIGH_DEFAULT	6000
#define TAF_TIME_ATOS_LOW_DEFAULT	2000
#define TAF_TIME_ATOS2_HIGH_DEFAULT	1500
#define TAF_TIME_ATOS2_LOW_DEFAULT	900
#define TAF_TIME_ADAPT_DEFAULT	0

#define TAF_COMMONQ_FULL_PCT	75
#define TAF_COMMONQ_EMPTY_PCT	45

#define TAF_ACTIVE_CHK_DEFAULT	1000000 /* 1 second */
#define TAF_MU_EXTEND_COEFF_DEFAULT	0	/* default mu extend units coefficient is 0 */
#define TAF_MU_EXTEND_COEFF_MAX		10	/* max mu extend units coefficient 1 */
#define TAF_MU_EXTEND_COEFF_MIN		0	/* set 0 to disable extend units for mu */

#if defined(WL_MU_TX)
#define TAF_MU_SCB_LOOP	0
#define TAF_SU_SCB_LOOP	1
#endif // endif

/* order of scheduler priority */
typedef enum {
	TAF_SCHEDULER_START = 0,
	TAF_EBOS = TAF_SCHEDULER_START,
	FIRST_EBOS_SCHEDULER = TAF_EBOS,
	TAF_PSEUDO_RR,
	TAF_ATOS,
	TAF_ATOS2,
	LAST_EBOS_SCHEDULER = TAF_ATOS2,
	NUM_EBOS_SCHEDULERS = LAST_EBOS_SCHEDULER - FIRST_EBOS_SCHEDULER + 1,
	NUM_TAF_SCHEDULERS = NUM_EBOS_SCHEDULERS,
	TAF_UNDEFINED = NUM_TAF_SCHEDULERS
} taf_scheduler_kind;

typedef struct taf_scb_cubby taf_scb_cubby_t;
typedef struct taf_release_context taf_release_context_t;

typedef bool  (*taf_scheduler_fn)(wlc_taf_info_t*, taf_release_context_t*, void *);
typedef int   (*taf_watchdog_fn)(wlc_taf_info_t*, void*);

typedef struct {
	uint32 prev_release_time;
	uint32 released_units;
	uint32 released_bytes;
	uint32 reschedule_units;
	uint32 high;
	uint32 low;
	uint32 adapt_cnt;
	uint16 last_release_pkttag;
	uint8  reschedule_index;
	bool   was_reset;
} taf_schedule_tid_state_t;

typedef enum {
	TAF_ORDER_TID_SCB,
	TAF_ORDER_TID_PARALLEL,
	TAF_ORDER_NUM_OPTIONS
} taf_schedule_order_t;

typedef struct {
	uint32 released_bytes;
} taf_scheduler_usage_t;

/* TAF per interface context */
struct wlc_taf_info {
	wlc_info_t        *wlc;      /* Back link to wlc */
	bool              enabled;   /* On/Off switch */
	bool              bypass;    /* taf bypass (soft taf disable) */
	bool              fallback;  /* Fallback to more conservative settings */
	uint8             index;     /* used to separate scheduler periods */
	int               scb_handle;	/* Offset for scb cubby */
	bool              rawfb;
	taf_scheduler_kind	default_scheduler;
	taf_scb_cubby_t   *head;		/* Ordered list of associated STAs */
	taf_scb_cubby_t   *pool;		/* Backup for disassociated STAs cubbies */
	uint32            scb_cnt;	/* Total count of allocated cubbies */
	taf_scheduler_fn  scheduler_fn[NUM_TAF_SCHEDULERS];
	taf_watchdog_fn	  watchdog_fn[NUM_TAF_SCHEDULERS];
	taf_scheduler_usage_t usage[NUM_TAF_SCHEDULERS];
	void*             scheduler_context[NUM_TAF_SCHEDULERS];
	uint32            force_time;
	taf_schedule_tid_state_t tid_state[NUMPRIO];
	taf_schedule_tid_state_t unified_tid_state;
	uint32            high;
	uint32            low;
	uint32            high_max;
	uint32            low_max;
	uint32            adapt;
	uint32            atos_high;
	uint32            atos_low;
	uint32            atos2_high;
	uint32            atos2_low;

	taf_schedule_order_t ordering;
	taf_schedule_order_t pending_ordering;
	uint32            ebos_last_active_time; /* make sure no ebos traffic before bypass */
	uint32            atos2_last_active_time; /* timestamp of last atos2 traffic */
	uint32            mu_extend_coeff;	/* mu scb extend units coefficient */
};

typedef struct {
	uint32 skip_ps;
	uint32 mov;
	uint32 mov_sav;
	uint32 mov_plen;
} taf_scheduler_scb_stats_t;

typedef struct {
	taf_scheduler_tid_stats_t    tid_stats;
} taf_scheduler_tid_info_t;

typedef struct {
	taf_scheduler_kind type;
	taf_scheduler_tid_info_t tid_info[NUMPRIO];
	taf_scheduler_scb_stats_t scb_stats;
} taf_scheduler_info_t;

/*
 * Per SCB data, malloced, to which a pointer is stored in the SCB cubby.
 *
 * This struct is malloc'ed when initializing the scb, but it is not free with the scb!
 * It will be kept in the "pool" list to save the preferences assigned to the STA, and
 * reused if the STA associates again to the AP.
 */
struct taf_scb_cubby {
	struct taf_scb_cubby *next; /* Next cubby for TAF list. Must be first */
	struct taf_list**   taf_list_head;
	struct scb *scb;   /* Back pointer */
	uint32 score; /* score for scheduler ordering: constant for EBOS and dynamic otherwise */
	uint32 force_time;   /* Force sending traffic */
	taf_scheduler_info_t info; /* Some other stats */
	uint32 timestamp;       /* When cubby is moved to backup pool */
	struct ether_addr  ea; /* Copy of MAC to find back reassociated STA */
};

typedef struct taf_sched_data {
	uint32 decay_time;
	ratespec_t rspec;
	uint32 byte_rate;
	uint32 pkt_rate;
	uint16 idle_periods[NUMPRIO];
	uint16 pktq_plen;
	bool enough_pend;
} taf_list_sched_data_t;

typedef struct taf_list {
	struct taf_list *next;
	taf_scb_cubby_t *scb_taf;
	taf_list_sched_data_t sched_data;
} taf_list_t;

typedef struct {
	taf_scheduler_kind type;
	bool   release_ampdu;
	bool   release_nar;
	uint8  tid;
	void*  scb_nar;
	void*  scb_ampdu;
	void*  scb_tid_ampdu;
	taf_list_t *item;
#ifdef BCMDBG
	taf_scheduler_tid_stats_t *tidstats;
#endif // endif
} taf_release_params_t;

typedef struct {
	taf_scheduler_kind type;
	uint32 counter;
	uint32 list_sort_time;
	uint32 coeff;
	struct wlc_taf_info* taf_info;
	taf_list_t*  list;
#if defined(WL_MU_TX)
	uint8 mu_counter;
	uint32 mu_scb_extend_units;
	uint32 mu_scb_extend_units_base;
#endif // endif
} taf_method_info_t;

typedef struct {
	uint32 counter;
	struct wlc_taf_info* taf_info;
} taf_default_info_t;

struct taf_release_context {
	taf_scheduler_kind       type;
	taf_release_params_t     release_params;
	taf_scheduler_public_t   public;
	taf_schedule_tid_state_t *tid_state;
	int     tid;
	bool    exit_early;
	bool    is_forced;
	uint32  now_time;
	uint32  actual_release;
};

static void* BCMATTACHFN(wlc_taf_method_attach)(wlc_taf_info_t*, taf_scheduler_kind);
static int   BCMATTACHFN(wlc_taf_method_detach)(void*);

static int wlc_taf_method_dump(void*, struct bcmstrbuf*);

static int wlc_taf_method_watchdog(wlc_taf_info_t*, void*);
static void wlc_taf_watchdog(void *handle);

static taf_method_info_t* taf_get_method_info(wlc_taf_info_t*, taf_scheduler_kind);
static taf_list_t* taf_list_ea_find(taf_list_t** head, const struct ether_addr*  ea);

static void taf_list_demote_item(taf_method_info_t* method, uint32 prio);
static void taf_move_list_item(taf_scb_cubby_t* scb_taf, taf_method_info_t* method);
static void BCMFASTPATH taf_sort_list(taf_method_info_t* method);

static bool BCMFASTPATH
wlc_taf_method_schedule_send(wlc_taf_info_t* taf_info, taf_release_context_t* context,
                       taf_method_info_t* method, taf_release_params_t* release_params);

static void BCMFASTPATH
wlc_taf_rate_to_taf_units(wlc_info_t *wlc, struct scb *scb, ratespec_t* rspec,
                          uint32* byte_rate, uint32* pkt_rate);

static void
taf_set_cubby_method(taf_method_info_t *method_to, taf_scb_cubby_t *scb_taf);

typedef struct {
	taf_scheduler_kind  type;
	const char*             name;
	void *  (*attach_fn) (wlc_taf_info_t *, taf_scheduler_kind);
	int     (*detach_fn) (void *);
	const char*             dump_name;
	int     (*dump_fn)   (void *, struct bcmstrbuf *);
} taf_scheduler_def_t;

static const taf_scheduler_def_t  taf_scheduler_definitions[NUM_TAF_SCHEDULERS] = {
	{ TAF_EBOS,           "ebos",    wlc_taf_method_attach,    wlc_taf_method_detach,
	"taf_ebos",    wlc_taf_method_dump},
	{ TAF_PSEUDO_RR, "prr",     wlc_taf_method_attach,    wlc_taf_method_detach,
	"taf_prr",     wlc_taf_method_dump},
	{ TAF_ATOS, "atos",     wlc_taf_method_attach,    wlc_taf_method_detach,
	"taf_atos",     wlc_taf_method_dump},
	{ TAF_ATOS2, "atos2",     wlc_taf_method_attach,    wlc_taf_method_detach,
	"taf_atos2",     wlc_taf_method_dump},
};

const char* taf_ordering[TAF_ORDER_NUM_OPTIONS] = {"TID order then SCB",
                                                   "SCB order with TID parallel"};

#define SCB_TAF_CUBBY_PTR(info, scb) ((taf_scb_cubby_t **)(SCB_CUBBY((scb), (info)->scb_handle)))
#define SCB_TAF_CUBBY(taf_info, scb) (*SCB_TAF_CUBBY_PTR(taf_info, scb))

/*
 * Module iovar handling.
 */
enum {
	IOV_TAF_DEFINE		/* universal configuration */
};

static const bcm_iovar_t taf_iovars[] = {
	{"taf", IOV_TAF_DEFINE, 0,
	IOVT_BUFFER, sizeof(wl_taf_define_t)},
	{NULL, 0, 0, 0, 0}
};

static const uint8 taf_tid_service_order[NUMPRIO] =
	{PRIO_8021D_NC, PRIO_8021D_VO, PRIO_8021D_VI,
	PRIO_8021D_CL, PRIO_8021D_EE, PRIO_8021D_BE,
	PRIO_8021D_BK, PRIO_8021D_NONE};

static uint8 taf_tid_service_reverse[NUMPRIO];

static INLINE uint32 BCMFASTPATH taf_timestamp(wlc_info_t *wlc)
{
	return R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
}

static void wlc_taf_tid_times_sync(wlc_taf_info_t *taf_info, taf_schedule_tid_state_t *tid_state,
	taf_scheduler_kind type)
{
	if (type == TAF_ATOS2) {
		tid_state->low = taf_info->atos2_low;
		tid_state->high = taf_info->atos2_high;
	}
	else if (type == TAF_ATOS) {
		tid_state->low = taf_info->atos_low;
		tid_state->high = taf_info->atos_high;
	}
	else {
		tid_state->low = taf_info->low;
		tid_state->high = taf_info->high;
	}

	tid_state->adapt_cnt = 0;

	WL_TAF(("Type:%d high:%d low:%d\n", type, tid_state->high, tid_state->low));
}

static void wlc_taf_times_sync(wlc_taf_info_t *taf_info)
{
	int i;

	for (i = 0; i < ARRAYSIZE(taf_info->tid_state); ++i) {
		wlc_taf_tid_times_sync(taf_info, &taf_info->tid_state[i], TAF_EBOS);
	}

	wlc_taf_tid_times_sync(taf_info, &taf_info->unified_tid_state, TAF_EBOS);

	taf_info->high_max = MAX(taf_info->high_max, taf_info->high);
	taf_info->low_max = MAX(taf_info->low_max, taf_info->low);
}

static void
wlc_taf_upd_ts2_scb_flag(struct scb *scb, int type)
{

	switch (type) {
		case TAF_EBOS:
			scb->flags3 &= ~(SCB3_TS_MASK);
			scb->flags3 |= SCB3_TS_EBOS;
			break;
		case TAF_ATOS:
			scb->flags3 &= ~(SCB3_TS_MASK);
			scb->flags3 |= SCB3_TS_ATOS;
			break;
		case TAF_ATOS2:
			scb->flags3 &= ~(SCB3_TS_MASK);
			scb->flags3 |= SCB3_TS_ATOS2;
			break;
		default:
			scb->flags3 &= ~(SCB3_TS_MASK);
			break;
	}
}

static taf_schedule_tid_state_t* BCMFASTPATH taf_get_tid_state(wlc_taf_info_t *taf_info, int tid)
{
	if (taf_info->ordering == TAF_ORDER_TID_PARALLEL) {
		/* all access classes are running in parallel, so maintain separated TID
		 * context
		 */
		return &taf_info->tid_state[tid];
	} else {
		/* the scheduling is across access class in some way (either SCB then
		 * TID, or else TID then SCB). So the context is unified.
		 */
		return &taf_info->unified_tid_state;
	}
}

static int wlc_taf_dump_list(taf_method_info_t* method, struct bcmstrbuf* b)
{
	taf_list_t* iter = method ? method->list : NULL;
	uint32 list_index = 0;

	if (iter) {
		bcm_bprintf(b, "Assigned %s entries:\n", TAF_SCHED_NAME(method->type));
	}
	else {
		bcm_bprintf(b, "No assigned entries\n");
	}

	while (iter) {
		bcm_bprintf(b, "%3d: "MACF, ++list_index,
		            ETHER_TO_MACF(iter->scb_taf->ea));

		if (method->type == TAF_EBOS) {
			bcm_bprintf(b, " : %u\n", iter->scb_taf->score);
		}
		else {
			bcm_bprintf(b, "\n");
		}
		iter = iter->next;
	}
	return BCME_OK;
}

static int wlc_taf_param(const char** cmd, uint32* param, uint32 min, uint32 max,
                         struct bcmstrbuf* b)
{
	*cmd += strlen(*cmd) + 1;

	if (**cmd) {
		/* an extra parameter was supplied, treat it as a 'set' operation */
		uint32 value;

		value = bcm_strtoul(*cmd, NULL, 0);

		if (value > max || value < min) {
			return BCME_RANGE;
		}
		*param = value;
		if (b) {
			bcm_bprintf(b, "%c", '\0');
		}
	}
	else if (b) {
		/* no more parameter was given, treat as a 'get' operation */
		if (*param > 16) {
			bcm_bprintf(b, "%u (0x%x)\n", *param, *param);
		}
		else {
			bcm_bprintf(b, "%u\n", *param);
		}
	}
	return BCME_OK;
}

static int wlc_taf_define(wlc_taf_info_t* taf_info, const struct ether_addr* ea,
                          wl_taf_define_t* result, wl_taf_define_t* input,
                          struct bcmstrbuf* b, bool set)
{
	taf_scheduler_kind type;
	taf_method_info_t* method;
	const struct ether_addr undef_ea = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

	/* if undef_ea is set, process as text arguments */
	if (eacmp(ea, &undef_ea) == 0) {
		const char* cmd = &input->text[0];

		result->ea = undef_ea;

		/* no argument given */
		if (cmd[0] == 0) {
			bcm_bprintf(b, "missing parameter(s)\n");
			return BCME_OK;
		}

		if (!strcmp(cmd, "enable")) {
			uint32 state = taf_info->enabled ? 1 : 0;
			int err = wlc_taf_param(&cmd, &state, 0, 1, b);

			/* was not in down state and there was attempt to change it */
			if ((set || *cmd) && taf_info->wlc->pub->up) {
				return BCME_NOTDOWN;
			}
			taf_info->enabled = state ? 1 : 0;
			result->misc = taf_info->enabled;
			return err;
		}

		if (!taf_info->enabled) {
			return BCME_NOTREADY;
		}
		if (!taf_info->wlc->pub->up) {
			return BCME_NOTUP;
		}

		if (!strcmp(cmd, "order")) {
			int err = wlc_taf_param(&cmd, &taf_info->pending_ordering,
			                        TAF_ORDER_TID_SCB, TAF_ORDER_TID_PARALLEL, b);

			if (taf_info->ordering != taf_info->pending_ordering) {
				result->misc = taf_info->pending_ordering;
				bcm_bprintf(b, "taf order will change to be");
			}
			else {
				result->misc = taf_info->ordering;
				bcm_bprintf(b, "taf order is");
			}
			bcm_bprintf(b, " %s\n", taf_ordering[result->misc]);
			return err;
		}
		if (!strcmp(cmd, "bypass")) {
			uint32 state = taf_info->bypass;
			int err = wlc_taf_param(&cmd, &state, FALSE, TRUE, b);

			taf_info->bypass = state;
			result->misc = taf_info->bypass;

			/* reset state info for when bypass is removed */
			if (taf_info->bypass) {
				int index;

				for (index = 0; index < NUMPRIO; index++) {
					taf_info->tid_state[index].released_units = 0;
				}
				taf_info->unified_tid_state.released_units = 0;
			}
			return err;
		}
		if (!strcmp(cmd, "rawfb")) {
			uint32 state = taf_info->rawfb;
			int err = wlc_taf_param(&cmd, &state, FALSE, TRUE, b);

			taf_info->rawfb = state;
			result->misc = taf_info->rawfb;

			return err;
		}
		if (!strcmp(cmd, "fallback")) {
			uint32 state = taf_info->fallback;
			int err = wlc_taf_param(&cmd, &state, FALSE, TRUE, b);

			taf_info->fallback = state;
			result->misc = taf_info->fallback;

			return err;
		}
		if (!strcmp(cmd, "adapt")) {
			int err = wlc_taf_param(&cmd, &taf_info->adapt, 0, ~0, b);
			if (err == BCME_OK) {
				wlc_taf_times_sync(taf_info);
			}
			result->misc = taf_info->adapt;
			return err;
		}
		if (!strcmp(cmd, "high")) {
			int err = wlc_taf_param(&cmd, &taf_info->high, taf_info->low,
				TAF_MICROSEC_MAX, b);
			if (err == BCME_OK) {
				wlc_taf_times_sync(taf_info);
			}
			result->misc = taf_info->high;
			return err;
		}
		if (!strcmp(cmd, "low")) {
			int err = wlc_taf_param(&cmd, &taf_info->low, 0,
				taf_info->high, b);
			if (err == BCME_OK) {
				wlc_taf_times_sync(taf_info);
			}
			result->misc = taf_info->low;
			return err;
		}
		if (!strcmp(cmd, "high_max")) {
			int err = wlc_taf_param(&cmd, &taf_info->high_max, taf_info->high,
				TAF_MICROSEC_MAX, b);
			if (err == BCME_OK) {
				wlc_taf_times_sync(taf_info);
			}
			result->misc = taf_info->high_max;
			return err;
		}
		if (!strcmp(cmd, "low_max")) {
			int err = wlc_taf_param(&cmd, &taf_info->low_max, taf_info->low,
				TAF_MICROSEC_MAX, b);
			if (err == BCME_OK) {
				wlc_taf_times_sync(taf_info);
			}
			result->misc = taf_info->low_max;
			return err;
		}
		if (!strcmp(cmd, "atos_high")) {
			int err = wlc_taf_param(&cmd, &taf_info->atos_high, taf_info->atos_low,
				TAF_MICROSEC_MAX, b);
			if (err == BCME_OK) {
				wlc_taf_times_sync(taf_info);
			}
			result->misc = taf_info->atos_high;
			return err;
		}
		if (!strcmp(cmd, "atos_low")) {
			int err = wlc_taf_param(&cmd, &taf_info->atos_low, 0,
				taf_info->atos_high, b);
			if (err == BCME_OK) {
				wlc_taf_times_sync(taf_info);
			}
			result->misc = taf_info->atos_low;
			return err;
		}
		if (!strcmp(cmd, "atos2_high")) {
			int err = wlc_taf_param(&cmd, &taf_info->atos2_high, taf_info->atos2_low,
				TAF_MICROSEC_MAX, b);
			if (err == BCME_OK) {
				wlc_taf_times_sync(taf_info);
			}
			result->misc = taf_info->atos2_high;
			return err;
		}
		if (!strcmp(cmd, "atos2_low")) {
			int err = wlc_taf_param(&cmd, &taf_info->atos2_low, 0,
				taf_info->atos2_high, b);
			if (err == BCME_OK) {
				wlc_taf_times_sync(taf_info);
			}
			result->misc = taf_info->atos2_low;
			return err;
		}
		if (!strcmp(cmd, "force")) {
			int err = wlc_taf_param(&cmd, &taf_info->force_time, 0,
				taf_info->high, b);
			result->misc = taf_info->force_time;
			return err;
		}
		if (!strcmp(cmd, "list")) {
			for (type = FIRST_EBOS_SCHEDULER; type <= LAST_EBOS_SCHEDULER; type++) {
				bcm_bprintf(b, "%u: %s\n", type, TAF_SCHED_NAME(type));
			}
			return BCME_OK;
		}
		if (!strcmp(cmd, "mu_extend_coeff")) {
			int err = wlc_taf_param(&cmd, &taf_info->mu_extend_coeff,
				TAF_MU_EXTEND_COEFF_MIN, TAF_MU_EXTEND_COEFF_MAX, b);
			result->misc = taf_info->mu_extend_coeff;
			return err;
		}

		for (type = FIRST_EBOS_SCHEDULER; type <= LAST_EBOS_SCHEDULER; type++) {
			if (!strcmp(cmd, TAF_SCHED_NAME(type)) ||
			    (cmd[0] >= '0' && cmd[0] <= '9' && bcm_strtoul(cmd, NULL, 0) == type)) {

				taf_method_info_t* method = taf_get_method_info(taf_info, type);
				cmd += strlen(cmd) + 1;

				if (*cmd) {
					if (!strcmp(cmd, "coeff")) {
						uint32 max_val = (method->type == TAF_EBOS) ?
							TAF_COEFF_EBOS_MAX : TAF_COEFF_ATOS_MAX;
						int err = wlc_taf_param(&cmd, &method->coeff,
							0, max_val, b);
						result->misc = method->coeff;
						return err;
					}
					if (!strcmp(cmd, "dump")) {
						return wlc_taf_method_dump(method, b);
					}
					if (!strcmp(cmd, "list")) {
						return wlc_taf_dump_list(method, b);
					}
					return BCME_UNSUPPORTED;
				}
				return wlc_taf_dump_list(method, b);
			}
		}
		return BCME_UNSUPPORTED;
	}

	if (!taf_info->enabled) {
		return BCME_NOTREADY;
	}
	if (!taf_info->wlc->pub->up) {
		return BCME_NOTUP;
	}
	/* at this point, a MAC address was supplied so it is not a text command. */

	/* find entry by MAC address operation */
	for (type = FIRST_EBOS_SCHEDULER; type <= LAST_EBOS_SCHEDULER; type++) {
		taf_list_t* found;
		taf_method_info_t* dst = NULL;

		method = taf_get_method_info(taf_info, type);
		found = taf_list_ea_find(&method->list, ea);

		if (!found) {
			continue;
		}

		if (!set) {
			/* this is 'get' */
			result->ea = *ea;
			result->sch = type;
			result->prio = found->scb_taf->score;
			result->misc = 0;
			bcm_bprintf(b, "%s", TAF_SCHED_NAME(type));
			return BCME_OK;
		}

		/* this is a 'set' */

		/* was a valid (numeric) scheduler given? */
		dst = (input->sch != (uint32)(~0)) ?
		      taf_get_method_info(taf_info, input->sch) : NULL;

		/* scheduler wasn't numeric, try to find by name */
		if (!dst && input->text[0]) {
			taf_scheduler_kind type = FIRST_EBOS_SCHEDULER;

			for (; type <= LAST_EBOS_SCHEDULER; type++) {
				if (!strcmp(input->text, TAF_SCHED_NAME(type))) {
					dst = taf_get_method_info(taf_info, type);
					break;
				}
			}
		}

		if (!dst) {
			return BCME_NOTFOUND;
		}

		/* check priority correctly configured */
		if ((dst->type == TAF_EBOS && !input->prio) ||
		       (dst->type != TAF_EBOS && input->prio)) {
			return BCME_BADARG;
		}

		if (dst->type == TAF_EBOS) {
			/* demote existing entry at this priority (if it exists) */
			taf_list_demote_item(dst, input->prio);

			/* set the priority */
			found->scb_taf->score = input->prio;
		}
		else {
			found->scb_taf->score = 0;
		}
		wlc_taf_upd_ts2_scb_flag(found->scb_taf->scb, dst->type);
		taf_set_cubby_method(dst, found->scb_taf);
		return BCME_OK;
	}
	return BCME_NOTFOUND;
}

static int
wlc_taf_doiovar(void *handle, const bcm_iovar_t * vi, uint32 actionid,
                const char *name, void *params, uint plen, void *arg,
                int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_taf_info_t *taf_info = handle;
	int status = BCME_OK;

	if (D11REV_LT(taf_info->wlc->pub->corerev, 40)) {
		/* only support on latest chipsets */
		return BCME_UNSUPPORTED;
	}

	switch (actionid) {
		/* there is overlap in Set and Get for this iovar */
		case IOV_GVAL(IOV_TAF_DEFINE):
		case IOV_SVAL(IOV_TAF_DEFINE):
		{
			wl_taf_define_t* taf_def_return = (wl_taf_define_t*) arg;
			wl_taf_define_t* taf_def_input = (wl_taf_define_t*) params;
			const struct ether_addr ea = taf_def_input->ea;

			struct bcmstrbuf b;
			int32  avail_len;

			/* only version 1 is currently supported */
			if (taf_def_input->version != 1) {
				WL_ERROR(("taf iovar version incorrect (%u/%u)\n",
				          taf_def_input->version, 1));
				return BCME_VERSION;
			}

			avail_len = alen - OFFSETOF(wl_taf_define_t, text);
			avail_len = (avail_len > 0 ? avail_len : 0);
			bcm_binit(&b, (char*)(&taf_def_return->text[0]), avail_len);

			return wlc_taf_define(taf_info, &ea, taf_def_return,
			                        taf_def_input, &b,
			                        actionid == IOV_SVAL(IOV_TAF_DEFINE));
		}
		break;

		default:
			status = BCME_UNSUPPORTED;
			break;

	}
	return status;
}

static void BCMFASTPATH taf_list_add(taf_list_t** head, taf_list_t* item)
{
	if (head && *head == NULL) {
		*head = item;
	}
	else {
		taf_list_t* iter = head ? *head : NULL;

		while (iter && iter->next) {
			iter = iter->next;
		}

		if (iter) {
			iter->next = item;
			item->next = NULL;
		}
	}
}

static taf_list_t* taf_list_ea_find(taf_list_t** head, const struct ether_addr*  ea)
{
	taf_list_t* iter = head ? *head : NULL;
	taf_scb_cubby_t* scb_taf;

	while (iter) {
		scb_taf = iter->scb_taf;
		if (eacmp(ea, (const char*)&(scb_taf->ea)) == 0) {
			return iter;
		}
		iter = iter->next;
	}
	return iter;
}

static taf_list_t* BCMFASTPATH taf_list_find(taf_list_t** head, taf_scb_cubby_t *scb_taf)
{
	taf_list_t* iter = head ? *head : NULL;

	while (iter && iter->scb_taf != scb_taf) {
		iter = iter->next;
	}
	return iter;
}

static void BCMFASTPATH taf_list_remove(taf_list_t** head, taf_list_t* item)
{
	if (head && item == *head) {
		*head = item->next;
	}
	else {
		taf_list_t* iter = head ? *head : NULL;

		while (iter && iter->next != item) {
			iter = iter->next;
		}

		if (!iter) {
			/* not in list */
			return;
		}
		iter->next = item->next;
	}
	item->next = NULL;
}

static void taf_list_demote_item(taf_method_info_t* method, uint32 prio)
{
	taf_list_t* found;

	do {
		taf_list_t* iter = method->list;
		taf_scb_cubby_t* scb_taf;
		uint32 highest = 0;
		found = NULL;

		while (iter) {
			scb_taf = iter->scb_taf;

			if (scb_taf->score > highest) {
				highest = scb_taf->score;
			}

			if (scb_taf->score == prio) {
				found = iter;
			}
			iter = iter->next;
		}

		if (found) {
			found->scb_taf->score = highest + 1;
			WL_TAF(("demoted item "MACF" with prio %u to prio %u\n",
			        ETHER_TO_MACF(found->scb_taf->ea),
			        prio, found->scb_taf->score));
		}
	} while (found);
}

static void taf_list_delete(wlc_taf_info_t* taf_info, taf_scb_cubby_t *scb_taf)
{
	taf_list_t* item = taf_list_find(scb_taf->taf_list_head, scb_taf);

	if (item) {
		taf_list_remove(scb_taf->taf_list_head, item);
		scb_taf->taf_list_head = NULL;
		MFREE(taf_info->wlc->pub->osh, item, sizeof(*item));
	}
}

static void BCMFASTPATH taf_decay_atos_item(taf_method_info_t *method,
	taf_list_t *item, uint32 nowtime)
{
	/* The decay coeff, is how much fractional reduction to occur per 2 milliseconds
	 * of elapsed time. This is an exponential model.
	 */
	uint32 decay_coeff = method->coeff;
	uint32 value = item->scb_taf->score;
	uint32 elapsed = nowtime - item->sched_data.decay_time;

	/* convert microseconds to units */
	elapsed = (elapsed + 1024) / 2048;

	if (elapsed > 500) {
		/* if it is a long time (1 second or more), just reset the score */
		value = 0;
	} else {
		while (value && elapsed--) {
			value = value * decay_coeff;
			value /= TAF_COEFF_ATOS_MAX; /* normalise coeff */
		}
	}

	item->scb_taf->score = value;
	item->sched_data.decay_time = nowtime;
}

static uint32 BCMFASTPATH taf_item_adj_par(taf_method_info_t *method, uint32 cnt, uint32 par)
{
	if (method->type == TAF_EBOS) {
		return cnt < method->coeff ? par : 0;
	} else {
		return (par >> cnt);
	}
}

static uint32 BCMFASTPATH taf_est_airtime(taf_list_sched_data_t *sched_data, uint16 pktq_plen)
{
	return TAF_PKTBYTES_TO_TIME(ETHER_MAX_DATA * pktq_plen,
		sched_data->pkt_rate, sched_data->byte_rate);
}

static void BCMFASTPATH taf_update_item_stat(taf_method_info_t *method, taf_list_t *item)
{
#ifdef BCMDBG
	taf_scb_cubby_t *scb_taf = item->scb_taf;
	taf_list_sched_data_t *sched_data = &item->sched_data;
	taf_scheduler_scb_stats_t *scb_stats;

	if (sched_data->enough_pend || !sched_data->pktq_plen) {
		return;
	}

	scb_stats = &scb_taf->info.scb_stats;
	scb_stats->mov++;
	if (!scb_stats->mov_plen) {
		scb_stats->mov_plen = sched_data->pktq_plen;
	}
#endif /* BCMDBG */
}

static void BCMFASTPATH taf_update_item_times(taf_method_info_t *method,
	taf_release_context_t *context, uint8 fullness)
{
	wlc_taf_info_t *taf_info = method->taf_info;
	taf_schedule_tid_state_t *tid_state = context->tid_state;

	ASSERT(tid_state->high >= taf_info->high);
	ASSERT(tid_state->low >= taf_info->low);
	ASSERT(taf_info->high_max >= tid_state->high);
	ASSERT(taf_info->low_max >= tid_state->low);

	/*
	 * TID's high/low floats/adapts between globally configurable
	 * high/low and high_max/low_max.
	 */
	if (fullness > TAF_COMMONQ_FULL_PCT) {
		/*
		 * Common queue is nearly full, let's decrease scheduling periods
		 * towards minimum settings.
		 */
		tid_state->high -= ((tid_state->high - taf_info->high) >> 2);
		tid_state->low -= ((tid_state->low - taf_info->low) >> 2);
	} else if (fullness < TAF_COMMONQ_EMPTY_PCT) {
		/*
		 * Common queue is nearly empty, let's increase scheduling periods
		 * towards maximum values.
		 */
		tid_state->high += ((taf_info->high_max - tid_state->high) >> 1);
		tid_state->low += ((taf_info->low_max - tid_state->low) >> 1);
	}

	tid_state->adapt_cnt = 0;
}

static bool INLINE taf_check_fallback(taf_method_info_t *method, uint16 plen)
{
	/*
	 * Bandwidth-delay product and TCP RWIN limits TCP throughput.
	 * Transmission cannot exceeds RWIN / latency value.
	 * So to make sure that video TCP is not degrading in presence of
	 * other (data) links, we have to make sure that no latency is added
	 * for video link by data links. Practically this is hard to achieve.
	 * Except for special case of TCP stream occupy whole bandwidth,
	 * there are moments of time when no video TCP data is present.
	 * So data link can be served, and if TCP video data comes while MAC
	 * transmitting data then this creates latency and limits TCP throughput.
	 *
	 * So following approach proposed.
	 * Latency over system is under control and configurable through high/low parameters.
	 * Can we just set high/low to very small number and enjoy low latency and so
	 * high level of preserving video TCP? No, we can't - low latency damages throughput
	 * because of aggregation and because of existing latency between MAC and driver.
	 * Video TCP stream has to use large enough RWIN to cope with this fixed latency
	 * and deliver required throughput (we no need highest possible, we need just enough
	 * to deliver video stream).
	 *
	 * So high/low parameters has to be set to high enough value to not harm throughput much
	 * (low value harms throughput) and at the same time deliver just enough TCP traffic
	 * (high value harms ability to save TCP traffic).
	 * When no TCP video traffic scheduler can grow these parameters automatically.
	 * When TCP video traffic detected then parameters are fallback to statically defined
	 * parameters. Unfortunately deep packet inspection is not currently available, so
	 * any video traffic triggers falling back.
	 */
	wlc_taf_info_t *taf_info = method->taf_info;
	return (taf_info->fallback && (method->type == TAF_EBOS) && plen);
}

static void BCMFASTPATH taf_update_item(taf_method_info_t *method, taf_list_t *item,
	taf_release_context_t *context)
{
	struct scb *scb = item->scb_taf->scb;
	wlc_taf_info_t *taf_info = method->taf_info;
	wlc_info_t *wlc = taf_info->wlc;
	void *scb_ampdu = wlc_ampdu_get_taf_scb_info(wlc->ampdu_tx, scb);
	taf_list_sched_data_t *sched_data = &item->sched_data;
	taf_schedule_tid_state_t *tid_state = context->tid_state;
	const uint8 tid = context->tid;
	void *scb_tid = wlc_ampdu_get_taf_scb_tid_info(scb_ampdu, tid);
	const uint16 idle_periods = sched_data->idle_periods[tid];
	const uint16 plen = wlc_ampdu_get_taf_scb_tid_pktlen(scb_ampdu, scb_tid);
	const bool fallback = taf_check_fallback(method, plen);

	/* Assume rate info is the same throughout all scheduling interval. */
	wlc_taf_rate_to_taf_units(wlc, scb,
		&sched_data->rspec,
		&sched_data->byte_rate,
		&sched_data->pkt_rate);

	/* Update pending packets related scheduling variables. */
	sched_data->pktq_plen = plen;
	if (plen == 0) {
		sched_data->enough_pend = FALSE;
	} else if (fallback) {
		sched_data->enough_pend = TRUE;
	} else if (plen >= taf_item_adj_par(method,
		idle_periods, wlc_ampdu_get_taf_scb_tid_rel(scb_ampdu))) {
		sched_data->enough_pend = TRUE;
	} else if (taf_est_airtime(sched_data, plen) >= taf_item_adj_par(method,
		idle_periods, tid_state->high)) {
		sched_data->enough_pend = TRUE;
	} else {
		sched_data->enough_pend = FALSE;
	}

	/* Update counter which tells how many periods item is not serviced. */
	sched_data->idle_periods[tid] = plen ? (sched_data->idle_periods[tid] + 1) : 0;

	/* Update times. */
	if (!(taf_info->adapt)) {
		wlc_taf_tid_times_sync(taf_info, tid_state, method->type);
	}
	else if (fallback) {
		wlc_taf_tid_times_sync(taf_info, tid_state, method->type);
	} else if (taf_info->adapt && (tid_state->adapt_cnt >= taf_info->adapt)) {
		taf_update_item_times(method, context,
		wlc_ampdu_get_taf_txq_fullness_pct(scb_ampdu, scb_tid));
	}

	taf_update_item_stat(method, item);
}

static void BCMFASTPATH taf_update_list(taf_method_info_t *method, taf_release_context_t *context)
{
	taf_list_t *item = method->list;
	taf_schedule_tid_state_t *tid_state = context->tid_state;

	context->now_time = taf_timestamp(method->taf_info->wlc);
	method->counter++;
	tid_state->adapt_cnt++;
#if defined(WL_MU_TX)
	method->mu_counter = 0;
	method->mu_scb_extend_units = 0;
	method->mu_scb_extend_units_base = 0;
#endif // endif

	while (item) {
		switch (method->type) {
			case TAF_EBOS:
				break;

			case TAF_PSEUDO_RR:
			case TAF_ATOS:
			case TAF_ATOS2:
				taf_decay_atos_item(method, item, context->now_time);
				break;

			default:
				WL_ERROR(("%s invalid method type (%d)\n", __FUNCTION__,
					method->type));
				ASSERT(0);
				break;
		}

#if defined(WL_MU_TX)
		/* count total number of MU scb for this TAF schedule period */
		if (SCB_MU(item->scb_taf->scb))
			method->mu_counter++;
#endif // endif
		taf_update_item(method, item, context);

		item = item->next;
	}
}

static void BCMFASTPATH
taf_prepare_list(taf_method_info_t *method, taf_release_context_t *ctx)
{
	taf_update_list(method, ctx);
	taf_sort_list(method);
}

static uint32 BCMFASTPATH taf_item_score(taf_list_t* item, bool *min_score)
{
	/*
	 * Put the 'force' links first, followed by the links with minimum scoring
	 * (most unrepresented).
	 * Push back links which have no traffic ready to send.
	 */
	*min_score = item->scb_taf->force_time ? TRUE : FALSE;
	if (item->sched_data.enough_pend) {
		return item->scb_taf->score;
	} else {
		return (TAF_SCORE_MAX - item->sched_data.pktq_plen);
	}
}

static taf_list_t* BCMFASTPATH taf_list_minimum(taf_list_t *head)
{
	uint32 minimum = TAF_SCORE_MAX;
	taf_list_t *result = head;

	while (head) {
		bool min_score;
		uint32 score = taf_item_score(head, &min_score);

		if (min_score) {
			result = head;
			break;
		} else if (score < minimum) {
			minimum = score;
			result = head;
		}

		head = head->next;
	}

	return result;
}

static taf_list_t*
taf_list_new(wlc_taf_info_t* taf_info, taf_scb_cubby_t *scb_taf)
{
	wlc_info_t *wlc = taf_info->wlc;
	taf_list_t* new = MALLOCZ(wlc->osh, sizeof(*new));

	if (new) {
		new->scb_taf = scb_taf;
		new->sched_data.decay_time = taf_timestamp(wlc);
	}
	else {
		WL_ERROR(("%s:%d unable to alloc memory\n", __FUNCTION__, __LINE__));
	}
	return new;
}

static void taf_move_list_item(taf_scb_cubby_t* scb_taf, taf_method_info_t* method)
{
	taf_list_t** head_src = scb_taf->taf_list_head;
	taf_list_t** head_dst = &method->list;
	taf_list_t*  item;

	if (head_src) {
		item = taf_list_find(head_src, scb_taf);
	} else {
		item = taf_list_new(method->taf_info, scb_taf);
	}

	WL_TAF(("removing "MACF" from %s and add to %s prio %d\n",
	        ETHER_TO_MACF(scb_taf->ea),
	        TAF_SCHED_NAME(scb_taf->info.type),
	        TAF_SCHED_NAME(method->type),
	        scb_taf->score));

	taf_list_remove(head_src, item);
	scb_taf->taf_list_head = head_dst;
	taf_list_add(head_dst, item);
	scb_taf->info.type = method->type;
}

static void BCMFASTPATH taf_sort_list(taf_method_info_t *method)
{
	taf_list_t *local_list_head = NULL;
	taf_list_t *item;

	while ((item = taf_list_minimum(method->list)) != NULL) {
		taf_list_remove(&method->list, item);
		taf_list_add(&local_list_head, item);
	}

	method->list = local_list_head;
}

static int
wlc_taf_method_watchdog(wlc_taf_info_t* taf_info, void* handle)
{
	taf_method_info_t* method = handle;

	if (!method || !taf_info) {
		return BCME_ERROR;
	}
	return BCME_OK;
}

/*
 * Watchdog timer. Called approximatively once a second.
 */
static void
wlc_taf_watchdog(void *handle)
{
	wlc_taf_info_t *taf_info = handle;
	int status = BCME_OK;
	taf_scheduler_kind type;
	uint32 index;
	uint32 pending;

	if (taf_info == NULL || !taf_info->enabled || taf_info->bypass) {
		return; // status;
	}
	pending = TXPKTPENDTOT(taf_info->wlc);

	if (pending == 0) {
		WL_TAF(("%s pending is 0\n", __FUNCTION__));
	}

	if (taf_info->ordering == TAF_ORDER_TID_SCB) {
		WL_TAF(("unified high %uus low %uus released %uus\n",
			taf_info->unified_tid_state.high,
			taf_info->unified_tid_state.low,
			TAF_UNITS_TO_MICROSEC(taf_info->unified_tid_state.released_units)));
		if (pending == 0) {
			wlc_taf_reset_scheduling(taf_info, 0);
		}
	}
	else if (taf_info->ordering == TAF_ORDER_TID_PARALLEL) {
		for (index = 0; index < NUMPRIO; index++) {
			WL_TAF(("TID %u high %uus low %uus released %uus\n",
				index,
				taf_info->tid_state[index].high,
				taf_info->tid_state[index].low,
				TAF_UNITS_TO_MICROSEC(taf_info->tid_state[index].released_units)));
			if (pending == 0) {
				wlc_taf_reset_scheduling(taf_info, index);
			}
		}
	}
	else {
		ASSERT(0);
	}

	/* process watchdog for scheduler methods */
	for (type = TAF_SCHEDULER_START; status == BCME_OK && type < NUM_TAF_SCHEDULERS; type++) {
		if (taf_info->watchdog_fn[type]) {
			status = taf_info->watchdog_fn[type]
					 (taf_info, taf_info->scheduler_context[type]);
		}
	}

//	return status;
}

static int
wlc_taf_method_dump(void *handle, struct bcmstrbuf *b)
{
	taf_method_info_t* method = handle;
	wlc_taf_info_t *taf_info = method ? method->taf_info : NULL;
	int _tid_index = 0;
	const char* scan_method;
	taf_list_t *list;

	if (method == NULL || !taf_info || !taf_info->enabled) {
		bcm_bprintf(b, "taf must be enabled first\n");
		return BCME_OK;
	}
	list = method->list;

	scan_method = taf_ordering[taf_info->ordering];

	bcm_bprintf(b, "%s count %d\n",
	               TAF_SCHED_NAME(method->type), method->counter);

	method->counter = 0;

	/* Dump all SCBs in our special order. */

	bcm_bprintf(b, "Stations list (scheduling as %s)\n", scan_method);
	bcm_bprintf(b, "Idx %17s %4s "
		"%8s %8s %8s %8s "
		"%8s %8s %8s %8s "
		"%8s %8s %8s\n",
		"Mac Address", "TID",
		"Ready", "TimeRel",
		"RelFrCnt", "RelPCnt", "AveFrSze",
		"AveDelay", "MaxDelay", "Emptied",
		"SkipPS", "Mov", "MovSav");

	for (; _tid_index < NUMPRIO; _tid_index++) {
		int tid_start = _tid_index;
		int tid_end = _tid_index + 1;
		bool first_tid = TRUE;
		bool did_output = FALSE;
		int idx = 0;

		list = method->list;

		while (list) {
			taf_scb_cubby_t *scb_taf = list->scb_taf;
			taf_scheduler_scb_stats_t* scb_stats = &scb_taf->info.scb_stats;
			int tid_index;

			if (scb_taf->info.type != method->type) {
				idx++;
				list = list->next;
				continue;
			}

			for (tid_index = tid_start; tid_index < tid_end; tid_index++) {
				uint32 tid = taf_tid_service_order[tid_index];
				taf_scheduler_tid_stats_t* tstats =
				              &scb_taf->info.tid_info[tid].tid_stats;

				if (tstats->ready) {
					uint32 rel_frc = tstats->release_frcount ?
					                 tstats->release_frcount : 1;
					uint32 ave_delay = tstats->did_rel_delta/rel_frc;
					uint32 max_delay = tstats->max_did_rel_delta;

					did_output = TRUE;

					if (first_tid) {
						bcm_bprintf(b, "%3d "MACF" %4u ", idx,
						            ETHER_TO_MACF(scb_taf->scb->ea), tid);
						first_tid = FALSE;
					}
					else {
						bcm_bprintf(b, "                      %4u ", tid);
					}

					if (ave_delay > 999999) {
						ave_delay = 999999;
					}
					if (max_delay > 999999) {
						max_delay = 999999;
					}

					bcm_bprintf(b,
								"%8u %8u %8u %8u "
								"%8u %8u %8u %8u "
								"%8u %8u %8u\n",
								tstats->ready,
								tstats->release_time/1000,
								tstats->release_frcount,
								tstats->release_pcount,
								(tstats->release_pcount +
								          (rel_frc/2))/ rel_frc,
								ave_delay, max_delay,
								tstats->emptied,
								scb_stats->skip_ps,
								scb_stats->mov,
								scb_stats->mov_sav);
				}
				memset(tstats, 0, OFFSETOF(taf_scheduler_tid_stats_t,
				                           did_rel_time));
			}
			list = list->next;
			idx++;
		}

		if (did_output && ((_tid_index + 1) < NUMPRIO)) {
			bcm_bprintf(b, "\n");
		}
	}

	for (list = method->list; list; list = list->next) {
		taf_scheduler_scb_stats_t* scb_stats =  &list->scb_taf->info.scb_stats;
		memset(scb_stats, 0, sizeof(*scb_stats));
	}

	return BCME_OK;
}

static taf_method_info_t* taf_get_method_info(wlc_taf_info_t* taf_info,
                                              taf_scheduler_kind sched)
{
	if (taf_info->enabled && (sched >= FIRST_EBOS_SCHEDULER) &&
	                         (sched <= LAST_EBOS_SCHEDULER)) {
		return (taf_method_info_t*)(taf_info->scheduler_context[sched]);
	}
	return NULL;
}

bool BCMFASTPATH wlc_taf_enabled(wlc_taf_info_t* taf_info)
{
	return (taf_info != NULL) ? taf_info->enabled : FALSE;
}

bool BCMFASTPATH wlc_taf_rawfb(wlc_taf_info_t* taf_info)
{
	return (taf_info != NULL) ? taf_info->rawfb : FALSE;
}

uint32 BCMFASTPATH wlc_taf_schedule_period(wlc_taf_info_t* taf_info, int tid)
{
	taf_schedule_tid_state_t *tid_state = taf_info ? taf_get_tid_state(taf_info, tid) : NULL;
	return tid_state ? tid_state->high : 0;
}

static void BCMFASTPATH
wlc_taf_set_force(wlc_taf_info_t* taf_info, struct scb* scb)
{
	/* Search out the scb and set the force parameter */
	taf_scb_cubby_t *scb_taf = taf_info->head;

	while (scb_taf) {
		if (scb == scb_taf->scb) {
			scb_taf->force_time = taf_info->force_time;
			WL_TAF(("Setting force option to "MACF"\n",
			        ETHER_TO_MACF(scb_taf->scb->ea)));
			return;
		}
		scb_taf = scb_taf->next;
	}
}

uint16
wlc_taf_traffic_active(wlc_taf_info_t* taf_info, struct scb* scb)
{
	wlc_info_t *wlc = taf_info->wlc;
	void *scb_ampdu = wlc_ampdu_get_taf_scb_info(wlc->ampdu_tx, scb);
	uint8 tid;
	void *scb_tid;
	uint16 traffic_active = 0;

	if (!scb_ampdu)
		return traffic_active;

	for (tid = 0; tid < NUMPRIO; tid++) {
		if ((scb_tid = wlc_ampdu_get_taf_scb_tid_info(scb_ampdu, tid)) != NULL)
			if (wlc_ampdu_get_taf_scb_tid_pktlen(scb_ampdu, scb_tid))
				traffic_active |= (1 << tid);
	}

	return traffic_active;
}

static bool
wlc_taf_is_bypass(wlc_taf_info_t *taf_info, uint32 now_time)
{
#if defined(WL_MU_TX)
	taf_scheduler_kind type = TAF_EBOS;
	uint16	ebos_traffic_active = 0;
	uint16	atos2_traffic_active = 0;
	uint32	diff = 0;
	taf_method_info_t* method = NULL;
	taf_list_t* list = NULL;
#endif	/* WL_MU_TX */
	bool is_bypass = FALSE;

	if (taf_info->bypass)
		return TRUE;

#if defined(WL_MU_TX)
	while (type <= TAF_ATOS2) {
		method = taf_get_method_info(taf_info, type);
		list = method ? method->list : NULL;
		switch (type) {
			case TAF_EBOS:
				/* bypass if ebos list is NULL */
				if (!list) {
					is_bypass = TRUE;
					break;
				}

				/* only check activity every second after ebos is avtived */
				diff = (uint32)((int32)now_time -
						(int32)taf_info->ebos_last_active_time);

				if (diff <= TAF_ACTIVE_CHK_DEFAULT) {
					ebos_traffic_active = TRUE;
					break;
				}
				for (list = method->list; list; list = list->next) {
					ebos_traffic_active = wlc_taf_traffic_active(taf_info,
						list->scb_taf->scb);
					if (ebos_traffic_active) {
						taf_info->ebos_last_active_time = now_time;
						break;
					}
				}

				/* bypass if no traffic for ebos scb */
				if (diff > (TAF_ACTIVE_CHK_DEFAULT*2))
					is_bypass = TRUE;
				else
					return is_bypass;

				break;

			case TAF_PSEUDO_RR:
			case TAF_ATOS:
				break;

			case TAF_ATOS2:
				if (!list)
					break;

				/* only check activity every second after atos2 is avtived */
				diff = (uint32)((int32)now_time -
						(int32)taf_info->atos2_last_active_time);

				if (diff <= TAF_ACTIVE_CHK_DEFAULT) {
					atos2_traffic_active = TRUE;
					break;
				}
				for (list = method->list; list; list = list->next) {
					atos2_traffic_active = wlc_taf_traffic_active(taf_info,
						list->scb_taf->scb);
					if (atos2_traffic_active) {
						taf_info->atos2_last_active_time = now_time;
						break;
					}
				}
				break;

			default:
				break;
		}
		type++;
	}
	/* Still enable TAF for following user case:
	 * EBOS(active) only
	 * EBOS(active) + ATOS
	 * ATOS + ATOS2(active)
	 * EBOS(active) + ATOS + ATOS2(active)
	 */
	if (ebos_traffic_active || atos2_traffic_active)
		is_bypass = FALSE;
#endif	/* WL_MU_TX */

	return is_bypass;
}

bool BCMFASTPATH
wlc_taf_schedule(wlc_taf_info_t *taf_info, int tid, struct scb *scb, bool force)
{
	bool finished = FALSE;
	bool skip_this = FALSE;
	taf_release_context_t context = {};
	taf_schedule_tid_state_t *tid_state = NULL;
	uint32 now_time = taf_timestamp(taf_info->wlc);

	if (wlc_taf_is_bypass(taf_info, now_time))
		return FALSE;

	ASSERT(tid < NUMPRIO);

	if (force && scb) {
		/* set the force flag */
		wlc_taf_set_force(taf_info, scb);
	}

	tid_state = taf_get_tid_state(taf_info, tid);

	if (tid_state->released_units) {
		/* still waiting for previous scheduled traffic to go to air */
		finished = TRUE;
		skip_this = TRUE;
	}
	else {
		/* did we change TAF ordering ? */
		if (taf_info->ordering != taf_info->pending_ordering) {
			WL_TAF(("Changing TAF ordering from %u to %u\n",
			        taf_info->ordering, taf_info->pending_ordering));
			taf_info->ordering = taf_info->pending_ordering;
			tid_state = taf_get_tid_state(taf_info, tid);
		}

		context.type = TAF_SCHEDULER_START;
		context.tid = tid;
		context.public.index = taf_info->index;
		context.tid_state = tid_state;
	}

	while (!finished && (context.type < NUM_TAF_SCHEDULERS)) {

		if (taf_info->scheduler_fn[context.type] != NULL) {
			uint32 prev_released_bytes = tid_state->released_bytes;

			finished = taf_info->scheduler_fn[context.type](taf_info,
			           &context,
			           taf_info->scheduler_context[context.type]);

			taf_info->usage[context.type].released_bytes += tid_state->released_bytes -
			                                                prev_released_bytes;
		}

		if (!finished) {
			context.type++;
		}
	}

	if (!skip_this) {
#ifdef BCMDBG
		uint32 time_delta = now_time - tid_state->prev_release_time;
#endif // endif
		tid_state->prev_release_time = now_time;
		tid_state->was_reset = FALSE;

		if (context.actual_release) {
			uint32 low_units = TAF_MICROSEC_TO_UNITS(tid_state->low);
			uint32 pkt_units = TAF_PKTTAG_TO_UNITS(tid_state->last_release_pkttag);

			if (tid_state->released_units > low_units) {
				tid_state->reschedule_units = tid_state->released_units - low_units;

				if (tid_state->reschedule_units > pkt_units) {
					tid_state->reschedule_units = pkt_units;
				}
			}
			else {
				tid_state->reschedule_units = 0;
			}
			tid_state->reschedule_index = context.public.index;

			WL_TAF(("schedule exit tid %u: (%u) %u us scheduled (%u pkts), "
			        "will trigger %uus (delta since previous is %u, "
			        "entry pending %u)\n", tid, context.public.index,
			        TAF_UNITS_TO_MICROSEC(tid_state->released_units),
			        context.actual_release,
			        TAF_UNITS_TO_MICROSEC(tid_state->reschedule_units),
			        time_delta, TXPKTPENDTOT(taf_info->wlc)));

			/* mask index with '3' because the pkt tag only has 2 bits to hold this */
			taf_info->index = (taf_info->index + 1) & 3;
		}
		else {
			WL_TAF(("schedule exit tid %u: no pkts scheduled "
					"(delta since previous is %d)\n", tid, time_delta));
		}
	}

	return finished;
}

static void
taf_set_cubby_method(taf_method_info_t *method_to, taf_scb_cubby_t *scb_taf)
{
	taf_move_list_item(scb_taf, method_to);
	taf_sort_list(method_to);
}

/*
 * SCB cubby functions, called when an SCB is created or destroyed.
 */
static int
wlc_taf_scbcubby_init(void *handle, struct scb *scb)
{
	wlc_taf_info_t *taf_info = handle;
	taf_scb_cubby_t *scb_taf = NULL;
	taf_method_info_t *method;

	/* Init function always called after SCB reset */
	ASSERT(*SCB_TAF_CUBBY_PTR(taf_info, scb) == NULL);

	if (taf_info->enabled && !SCB_INTERNAL(scb)) {
		taf_scb_cubby_t *prev;
		/* Remember the oldest cubby as we may reuse it */
		taf_scb_cubby_t *oldest = NULL, *prev_oldest = NULL;
		uint32 ts_oldest = -1U;

		/* Search pool for same MAC. This casting allows to modify the list seamlessly */
		prev = (taf_scb_cubby_t *) &taf_info->pool;

		for (scb_taf = taf_info->pool; scb_taf; prev = scb_taf, scb_taf = scb_taf->next) {
			if (memcmp(&scb_taf->ea, &scb->ea, ETHER_ADDR_LEN) == 0) {
				/* Found! remove it from the pool list */
				prev->next = scb_taf->next;
				WL_TAF(("TAF cby "MACF" found in pool\n", ETHER_TO_MACF(scb->ea)));
				break;
			}

			/* Oldest one? then save it */
			if (scb_taf->timestamp < ts_oldest) {
				ts_oldest = scb_taf->timestamp;
				prev_oldest = prev;
				oldest = scb_taf;
			}
		}

		if (scb_taf == NULL) {
			if (taf_info->scb_cnt <= TAF_SCB_CNT_MAX) {
				wlc_info_t *wlc = taf_info->wlc;

				/* Not found, initialize a new one */
#ifdef BCM_HOST_MEM_SCB
				if (SCB_ALLOC_ENAB(wlc->pub) && SCB_HOST(scb)) {
					scb_taf = (taf_scb_cubby_t *)wlc_scb_alloc_mem_get(wlc,
						SCB_CUBBY_ID_TAF, sizeof(taf_scb_cubby_t), 1);
				}

				if (!scb_taf)
#endif // endif
				scb_taf = MALLOCZ(wlc->osh, sizeof(taf_scb_cubby_t));
				if (scb_taf == NULL)
					return BCME_NOMEM;
				taf_info->scb_cnt++;
				WL_TAF(("TAF cby "MACF" allocated\n", ETHER_TO_MACF(scb->ea)));
			} else {
				ASSERT(oldest && prev_oldest);
				/* Not found and max count reached, reuse the oldest one */
				/* check if there is space available? */
				if (!oldest || !prev_oldest)
					return BCME_NOMEM;

				prev_oldest->next = oldest->next;
				scb_taf = oldest;
				WL_TAF(("TAF cby "MACF" reused\n", ETHER_TO_MACF(scb->ea)));
			}
			memset(scb_taf, 0, sizeof(*scb_taf));
		}

		scb_taf->scb = scb;
		scb_taf->ea = scb->ea;
		scb_taf->info.type = TAF_UNDEFINED;

		method = taf_get_method_info(taf_info, taf_info->default_scheduler);

		ASSERT(method);

		taf_set_cubby_method(method, scb_taf);

		/* Just put the cubby at the begining of the list as bsscfg is unknown */
		scb_taf->next = taf_info->head;
		taf_info->head = scb_taf;

		*SCB_TAF_CUBBY_PTR(taf_info, scb) = scb_taf;
	}

	return BCME_OK;
}

/*
 * The SCB cubby is saved to be reused later if the station re-associate, or for another STA.
 */
static void
wlc_taf_scbcubby_exit(void *handle, struct scb *scb)
{
	wlc_taf_info_t *taf_info = handle;
	taf_scb_cubby_t* scb_taf = *SCB_TAF_CUBBY_PTR(taf_info, scb);

	/* If we do have a cubby, clean up. */
	if (scb_taf) {
		taf_scb_cubby_t *prev, *curr;

		taf_list_delete(taf_info, scb_taf);

		/* Trick to modify head seamlessly */
		prev = (taf_scb_cubby_t *) &taf_info->head;

		for (curr = taf_info->head; curr; prev = curr, curr = curr->next) {
			if (scb_taf == curr) {
				/* Found! remove it from the head list */
				prev->next = curr->next;
				break;
			}
		}
		ASSERT(curr);

		/* Save the cubby in the pool list */
		scb_taf->next = taf_info->pool;
		taf_info->pool = scb_taf;

		/* Clear the backlink which will become invalid */
		scb_taf->scb = NULL;

		/* Save time of last scb use for potential discard */
		scb_taf->timestamp = taf_info->wlc->pub->now;

		*SCB_TAF_CUBBY_PTR(taf_info, scb) = NULL;

		WL_TAF(("TAF cby "MACF" moved to pool list\n", ETHER_TO_MACF(scb->ea)));
	}
}

bool BCMFASTPATH wlc_taf_reset_scheduling(wlc_taf_info_t *taf_info, int tid)
{
	taf_schedule_tid_state_t *tid_state;

	if (!taf_info || !taf_info->enabled || taf_info->bypass) {
		WL_NONE(("TAF not enabled\n"));
		return FALSE;
	}

	tid_state = taf_get_tid_state(taf_info, tid);
	tid_state->was_reset = TRUE;

	if (tid_state->released_units) {
		tid_state->released_units = 0;
		WL_TAF(("%s tid %u (ordering %d)\n", __FUNCTION__, tid, taf_info->ordering));
	}
	return TRUE;
}

bool BCMFASTPATH wlc_taf_handle_star(wlc_taf_info_t *taf_info, int tid, uint16 pkttag, uint8 index)
{
	taf_schedule_tid_state_t *tid_state;

	if (!taf_info || !taf_info->enabled || taf_info->bypass) {
		WL_NONE(("TAF not enabled\n"));
		return FALSE;
	}

	tid_state = taf_get_tid_state(taf_info, tid);

	if (index != tid_state->reschedule_index) {
		WL_NONE(("%s ignoring tid %d tag=%d:%d (ordering %d)\n",
		         __FUNCTION__, tid, index, pkttag, taf_info->ordering));
		return TRUE;
	}
	if (tid_state->released_units &&
	               (TAF_PKTTAG_TO_UNITS(pkttag) >= tid_state->reschedule_units)) {
		tid_state->released_units = 0;
		WL_TAF(("%s tid %d tag=%d:%d  reschedule=%d, (ordering %d)\n",
		         __FUNCTION__, tid, index, pkttag,
		         TAF_UNITS_TO_MICROSEC(tid_state->reschedule_units), taf_info->ordering));
	}
	else if (tid_state->released_units == 0) {
		WL_NONE(("%s FREE tid %d tag=%d:%d  reschedule=%d, (ordering %d)\n",
		         __FUNCTION__, tid, index, pkttag,
		         TAF_UNITS_TO_MICROSEC(tid_state->reschedule_units), taf_info->ordering));
	}
	else {
		WL_NONE(("%s NOT YET tid %d tag=%d:%d  reschedule=%d, (ordering %d) "
		         "released_units %d\n", __FUNCTION__, tid, index, pkttag,
		         TAF_UNITS_TO_MICROSEC(tid_state->reschedule_units), taf_info->ordering,
		         tid_state->released_units));
	}
	return TRUE;
}

static bool BCMFASTPATH
wlc_taf_method_schedule_scb(wlc_taf_info_t *taf_info, taf_method_info_t *method, int tid_index,
                      taf_release_context_t *context)
{
	wlc_info_t *wlc = taf_info->wlc;
	taf_release_params_t *release_params = &context->release_params;
	taf_schedule_tid_state_t *tid_state = context->tid_state;
	uint32 high_units = TAF_MICROSEC_TO_UNITS(tid_state->high);
	taf_list_t *list;
#if defined(WL_MU_TX)
	uint8 list_loop_cnt, local_mu_counter = 0;
#endif // endif

	context->tid = taf_tid_service_order[tid_index];

	taf_prepare_list(method, context);

#if defined(WL_MU_TX)
	/*
	 * TAF + MU solution:
	 * 1) TAF algo keeps schedule priority no change: Scheduling EBOS
	 *    first, then ATOS and ATOS2.
	 * 2) Treat MU SCB and SU SCB differently,first loop handle MU SCB
	 *    list followed by SU SCB list.
	 * 3) All the MU SCB are serviced in one scheduling period.
	 * 4) Dynamically increase release units based on the number of
	 *    active MU clients to improve MU gain.
	 */
	for (list_loop_cnt = TAF_MU_SCB_LOOP; list_loop_cnt <= TAF_SU_SCB_LOOP; list_loop_cnt++) {
#endif // endif
	  for (list = method->list; list; list = list->next) {
		taf_scb_cubby_t *scb_taf = list->scb_taf;
		struct scb *scb = scb_taf->scb;
		taf_scheduler_info_t *sinfo = &scb_taf->info;
		uint16	traffic_active;

		void* loop_scb_ampdu = NULL;
		void* loop_scb_nar = NULL;
		void* scb_tid = NULL;
#ifdef BCMDBG
		taf_scheduler_tid_stats_t* tidstats = NULL;
#endif // endif

		ASSERT(sinfo->type == method->type);

		if (!scb) {
			continue;
		}

#if defined(WL_MU_TX)
		if (list_loop_cnt == TAF_MU_SCB_LOOP) {
			/* first loop handle MU scbs */
			if (SCB_MU(scb))
				local_mu_counter++;
			else
				continue;
		} else {
			/* second loop handle SU scbs */
			if (SCB_MU(scb))
				continue;
		}
#endif // endif
		/* update scb->traffic_active */
		traffic_active = wlc_taf_traffic_active(taf_info, scb);

		if (!traffic_active) {
			/* no traffic - clear the force status */
			scb_taf->force_time = 0;
			WL_TAF((MACF" no active traffic\n", ETHER_TO_MACF(scb->ea)));
			continue;
		}
		/* basic traffic pending check */
		if (!(traffic_active & (1 << context->tid))) {
			WL_TAF((MACF" no active traffic for tid %u\n", ETHER_TO_MACF(scb->ea),
			        context->tid));
			continue;
		}

#ifdef AP
		/* If station is in PS (or pretend) mode, then check if there is queued traffic in
		 * ps queue. If so, we don't send any more. If it is empty, do a release
		 * so there will be pending traffic to wake the scb.
		 */
		if (SCB_PS(scb)) {
			int ps_queued = wlc_apps_psq_len(wlc, scb);

			if (ps_queued) {
				taf_scheduler_scb_stats_t* scb_stats = &sinfo->scb_stats;
				scb_stats->skip_ps++;

				/* cannot force so clear the force status */
				scb_taf->force_time = 0;

				WL_TAF(("skipping "MACF" which is in ps mode with %u queued\n",
				        ETHER_TO_MACF(scb->ea), ps_queued));
				continue;
			}
		}
#endif /* AP */

#if TAF_ENABLE_NAR
		loop_scb_nar = wlc_nar_get_scb_handle(wlc->nar_handle, scb);
#endif // endif
		loop_scb_ampdu = wlc_ampdu_get_taf_scb_info(wlc->ampdu_tx, scb);

		if (!loop_scb_ampdu && !loop_scb_nar) {
			/* cannot force so clear the force status */
			scb_taf->force_time = 0;
			continue;
		}

		/* context->is_forced is initialised FALSE; this is TRUE if the previous
		 * scb was forced (and all previous to that as well as the intention is
		 * to have all forced at once)
		 */
		if (context->is_forced) {
			/* Process forced and non-forced separately. Hence, if the next scb
			 * is not forced but the previous one was, stop.
			 */
			if (!scb_taf->force_time) {
				context->exit_early = TRUE;
				return FALSE;
			}
		}
		context->is_forced = scb_taf->force_time ? TRUE : FALSE;
#ifdef BCMDBG
		tidstats = &sinfo->tid_info[context->tid].tid_stats;
		context->public.tidstats = tidstats;
#endif // endif

		/* First, do NAR */
		if (loop_scb_nar) {
			release_params->release_nar  = TRUE;
			release_params->scb_nar = loop_scb_nar;
		}

		/* Now AMPDU */
		scb_tid = wlc_ampdu_get_taf_scb_tid_info(loop_scb_ampdu, context->tid);

		if (scb_tid) {
			release_params->release_ampdu = TRUE;
			release_params->scb_ampdu = loop_scb_ampdu;
			release_params->scb_tid_ampdu = scb_tid;
		}

		/* either NAR, AMPDU or both ready to send, then send! */
		if (release_params->release_nar || release_params->release_ampdu) {
			bool emptied;

			release_params->tid = context->tid;
			release_params->item = list;

#ifdef BCMDBG
			if (tidstats) {
				release_params->tidstats = tidstats;
				tidstats->ready++;
			}
#endif /* BCMDBG */

			emptied = wlc_taf_method_schedule_send(taf_info, context, method,
			                                 release_params);

			if (emptied)
				wlc_apps_pvb_update(wlc, scb);

			/* clear the force status, because some traffic was sent */
			scb_taf->force_time = 0;

#if defined(WL_MU_TX)
			/* MU scbs are serviced in one scheduling period, only return
			 * when it is last MU scb
			 */
			if (((!emptied && !SCB_PS(scb)) ||
				(tid_state->released_units >= high_units)) &&
				(((list_loop_cnt == TAF_MU_SCB_LOOP) &&
				(local_mu_counter == method->mu_counter)) ||
				(list_loop_cnt == TAF_SU_SCB_LOOP))) {
					return TRUE;
			}
#else
			if (!emptied && !SCB_PS(scb)) {
				return TRUE;
			}

			if (tid_state->released_units >= high_units) {
				return TRUE;
			}
#endif	/* WL_MU_TX */
		}
		else {
			/* clear the force status, because we can't send
			 * when there is nothing pending
			 */
			scb_taf->force_time = 0;
		}
	  }
#if defined(WL_MU_TX)
	}
#endif // endif

	return FALSE;
}

static bool BCMFASTPATH
wlc_taf_method_schedule_tid(wlc_taf_info_t* taf_info, taf_method_info_t* method,
                           int tid_index_start, int tid_index_end, taf_release_context_t* context)
{
	bool finished;
	int tid_index = tid_index_start;

	do {
		finished = wlc_taf_method_schedule_scb(taf_info, method, tid_index, context);

		if (!finished) {
			if (context->exit_early) {
				return TRUE;
			}

			tid_index++;
		}
	} while (!finished && tid_index < tid_index_end);

	return finished;
}

static bool BCMFASTPATH
wlc_taf_method_schedule_send(wlc_taf_info_t *taf_info, taf_release_context_t *context,
                  taf_method_info_t *method, taf_release_params_t *release_params)
{
	taf_schedule_tid_state_t *tid_state = context->tid_state;
	wlc_info_t* wlc = taf_info->wlc;
	taf_list_t *item = release_params->item;
	taf_scb_cubby_t* scb_taf = item->scb_taf;
	uint32 actual_release = 0;
	uint32 released_units = 0;
	uint32 released_bytes = 0;
	uint16 last_release_pkttag = 0;
	uint32 time_limit_units = TAF_MICROSEC_TO_UNITS(tid_state->high);
#ifdef AP
	bool psmode = SCB_PS(scb_taf->scb) || SCB_PS_PRETEND_BLOCKED(scb_taf->scb);
#else
	const bool psmode = FALSE;
#endif // endif
#ifdef BCMDBG
	taf_scheduler_scb_stats_t* scb_stats = &scb_taf->info.scb_stats;
	taf_scheduler_tid_stats_t* tidstats = release_params->tidstats;
	context->public.tidstats = release_params->tidstats;
#endif // endif

	context->public.is_ps_mode = psmode;
	context->public.released_units = 0;
	context->public.actual_release = 0;
	context->public.released_bytes = 0;
	context->public.total_released_units = tid_state->released_units;
	context->public.byte_rate = item->sched_data.byte_rate;
	context->public.pkt_rate = item->sched_data.pkt_rate;

	if (tid_state->was_reset) {
		/* due to previous malfunction the taf scheduler was reset, so proceed
		 * cautiously with a release of just 1ms......
		 */
		if (time_limit_units > TAF_MICROSEC_TO_UNITS(1000)) {
			time_limit_units = TAF_MICROSEC_TO_UNITS(1000);
		}
	}

#if defined(WL_MU_TX)
	/* Increase the time_limit_units for MU case:
	 * The MU gain is not N*X for N-MU STAs when test with OTA, N = 2..4.
	 * There is sounding overhead. Basically, 1.6X for 2-MU, 2X for 3-MU
	 * and 2.4 for 4-MU is the best we can get so far. Use mu_extend_coeff
	 * to fine tune the extend units, default is 0.6 for each mu scb.
	 */
	if ((taf_info->mu_extend_coeff) && SCB_MU(scb_taf->scb) &&
			(method->mu_counter > 1)) {
		if (!method->mu_scb_extend_units_base) {
			/* calculate mu_scb_extend_units_base if this is first MU scb */
			method->mu_scb_extend_units_base =
				((time_limit_units - tid_state->released_units) *
				(taf_info->mu_extend_coeff))/TAF_MU_EXTEND_COEFF_MAX;
			time_limit_units = tid_state->released_units +
				method->mu_scb_extend_units_base;
		}
		else {
			/* extend time_limit_units for next MU scb */
			method->mu_scb_extend_units += method->mu_scb_extend_units_base;
			time_limit_units += method->mu_scb_extend_units;
		}
	}
#endif	/* WL_MU_TX */

	context->public.time_limit_units = time_limit_units;

#if TAF_ENABLE_NAR
	/* do NAR first */
	while (release_params->release_nar &&
	       (tid_state->released_units + released_units) < time_limit_units &&
	       wlc_nar_release_from_queue_action(release_params->scb_nar,
	                             release_params->tid, 1 /* TO DO */, &context->public)) {
		actual_release += context->public.actual_release;
		released_units += context->public.released_units;
		released_bytes += context->public.released_bytes;
		last_release_pkttag = context->public.last_release_pkttag;

		context->public.released_units = 0;
		context->public.actual_release = 0;
		context->public.released_bytes = 0;
		context->public.total_released_units = tid_state->released_units + released_units;

		if (context->is_forced &&
		        released_units >=
		        TAF_MICROSEC_TO_UNITS(scb_taf->force_time)) {
			WL_TAF(("force time %u reached for "MACF"\n", scb_taf->force_time,
			        ETHER_TO_MACF(scb_taf->scb->ea)));
			break;
		}
		if (psmode) {
			WL_TAF(("stopping NAR release to "MACF" in ps mode\n",
			        ETHER_TO_MACF(scb_taf->scb->ea)));
			break;
		}
	};
#else
	/* NAR part isn't ready.... */
	ASSERT(!release_params->release_nar);
#endif /* 0 */

	/* then do AMPDU */
	while (release_params->release_ampdu &&
	       (tid_state->released_units + released_units) < time_limit_units &&
	       wlc_ampdu_txeval_action(wlc->ampdu_tx, release_params->scb_ampdu,
	                              release_params->scb_tid_ampdu,
	                              scb_taf->force_time, &context->public)) {

		actual_release += context->public.actual_release;
		released_units += context->public.released_units;
		released_bytes += context->public.released_bytes;
		last_release_pkttag = context->public.last_release_pkttag;

		context->public.released_units = 0;
		context->public.actual_release = 0;
		context->public.released_bytes = 0;
		context->public.total_released_units = tid_state->released_units + released_units;

		if (context->is_forced &&
		        released_units >=
		        TAF_MICROSEC_TO_UNITS(scb_taf->force_time)) {
				WL_TAF(("force time %u reached for "MACF"\n", scb_taf->force_time,
				        ETHER_TO_MACF(scb_taf->scb->ea)));
			break;
		}
		if (psmode) {
			WL_TAF(("stopping AMPDU release to "MACF" in ps mode\n",
			        ETHER_TO_MACF(scb_taf->scb->ea)));
			break;
		}
		if (wlc->block_datafifo) {
			WL_TAF(("stopping AMPDU release to "MACF". dfifo blocked\n",
			        ETHER_TO_MACF(scb_taf->scb->ea)));
			break;
		}
	}

	/* don't count data released to a scb in ps mode towards our scheduling interval */
	if (!psmode) {
		tid_state->released_units += released_units;
		tid_state->last_release_pkttag = last_release_pkttag;

		context->actual_release += actual_release;
	}

	switch (method->type) {
		case TAF_EBOS:
			break;

		case TAF_PSEUDO_RR:
			/* this is packet number scoring (or could use released_bytes instead) */
			scb_taf->score = MIN(scb_taf->score + actual_release, TAF_SCORE_ATOS_MAX);
			break;

		case TAF_ATOS:
		case TAF_ATOS2:
			/* this is scheduled airtime scoring */
			scb_taf->score = MIN(scb_taf->score + released_units, TAF_SCORE_ATOS_MAX);
			break;

		default:
			break;
	}

	/* this item serviced */
	item->sched_data.idle_periods[context->tid] = 0;

#ifdef BCMDBG
	if (scb_stats->mov_plen && (actual_release > scb_stats->mov_plen)) {
		scb_stats->mov_sav += actual_release - scb_stats->mov_plen;
	}
	scb_stats->mov_plen = 0;

	if (released_units && tidstats) {
		uint32 rel_delta;

		rel_delta = context->now_time - tidstats->did_rel_time;

		if (rel_delta > tidstats->max_did_rel_delta) {
			tidstats->max_did_rel_delta = rel_delta;
		}
		tidstats->did_rel_delta += rel_delta;
		tidstats->did_rel_time = context->now_time;

		tidstats->release_frcount ++;
		tidstats->release_pcount += actual_release;
		tidstats->release_time += TAF_UNITS_TO_MICROSEC(released_units);

		if (context->public.was_emptied) {
			tidstats->emptied++;
		}
		WL_TAF((MACF"%s rate %d, rel %d pkts is %d us, total %d us %s\n",
		        ETHER_TO_MACF(scb_taf->scb->ea), psmode ? "PS" : "",
		        wlc_rate_rspec2rate(item->sched_data.rspec),
		        actual_release,
		        TAF_UNITS_TO_MICROSEC(released_units),
		        TAF_UNITS_TO_MICROSEC(tid_state->released_units),
		        context->public.was_emptied ? "EMPTIED":""));
	}
#endif /* BCMDBG */

	return context->public.was_emptied;
}

static bool BCMFASTPATH
wlc_taf_method_schedule(wlc_taf_info_t *taf_info, taf_release_context_t *context,
	void *scheduler_context)
{
	taf_method_info_t *method = scheduler_context;
	bool finished;

	if (taf_info->ordering == TAF_ORDER_TID_SCB) {
		finished = wlc_taf_method_schedule_tid(taf_info, method, 0, NUMPRIO - 1, context);
	} else {
		int tid_index = taf_tid_service_reverse[context->tid];
		finished = wlc_taf_method_schedule_scb(taf_info, method, tid_index, context);
	}

	if ((method->type == LAST_EBOS_SCHEDULER) || context->exit_early) {
		finished = TRUE;
	}

	return finished;
}

static void*
BCMATTACHFN(wlc_taf_method_attach)(wlc_taf_info_t *taf_info, taf_scheduler_kind type)
{
	taf_method_info_t* method = (taf_method_info_t*) MALLOCZ(taf_info->wlc->pub->osh,
	                            sizeof(*method));
	int index;

	if (method == NULL) {
		return NULL;
	}

	method->taf_info = taf_info;
	method->type = type;

	switch (type) {
		case TAF_EBOS :
			method->coeff = TAF_COEFF_EBOS;
			for (index = 0; index < NUMPRIO; index++) {
				taf_tid_service_reverse[taf_tid_service_order[index]] = index;
			}
			break;

		case TAF_PSEUDO_RR :
		case TAF_ATOS:
		case TAF_ATOS2:
			method->coeff = TAF_COEFF_ATOS;
			break;

		default:
			MFREE(taf_info->wlc->pub->osh, method, sizeof(*method));
			return NULL;
	}

	taf_info->scheduler_fn[type] = wlc_taf_method_schedule;
	taf_info->watchdog_fn[type] = wlc_taf_method_watchdog;

	return method;
}

static int
BCMATTACHFN(wlc_taf_method_detach)(void* context)
{
	taf_method_info_t* method = context;

	if (method) {
		MFREE(method->taf_info->wlc->pub->osh, method, sizeof(*method));
	}
	return BCME_OK;
}

static void BCMFASTPATH
wlc_taf_rate_to_taf_units(wlc_info_t *wlc, struct scb *scb, ratespec_t* rspec, uint32* byte_rate,
                          uint32* pkt_rate)
{
	*rspec = wlc_scb_ratesel_get_primary(wlc, scb, NULL);
	*byte_rate = wlc_airtime_payload_time_us(0, *rspec, TAF_PKTBYTES_COEFF);
	*pkt_rate = *byte_rate * wlc_airtime_dot11hdrsize(scb->wsec);
}

#if defined(BCMDBG)
static int
wlc_taf_dump(void *handle, struct bcmstrbuf *b)
{
	wlc_taf_info_t	*taf_info = handle;
	taf_scheduler_kind type = TAF_SCHEDULER_START;

	bcm_bprintf(b, "taf is %sabled%s\n", taf_info->enabled ? "en" : "dis",
	            taf_info->bypass ? " BUT BYPASSED (not in use)" : "");

	if (!taf_info->enabled) {
		return BCME_OK;
	}

	while (type < NUM_TAF_SCHEDULERS) {
		bcm_bprintf(b, "\nMethod '%s' is available (scheduler index %u); "
					"for detailed info, please do 'dump %s'\n",
					TAF_SCHED_NAME(type), type, TAF_DUMP_NAME(type));

		switch (type) {
			case TAF_EBOS:
			case TAF_PSEUDO_RR:
			case TAF_ATOS:
			case TAF_ATOS2:
			{
				taf_method_info_t* method = taf_get_method_info(taf_info, type);

				wlc_taf_dump_list(method, b);
			}
			break;
			default :
				break;
		}
		type++;
	}
	return BCME_OK;
}
#endif /* BCMDBG */

/*
 * wlc_taf_up() - interface coming up.
 */
static int
wlc_taf_up(void *handle)
{
	/* wlc_taf_info_t *taf_info = handle; */

	return BCME_OK;
}
/*
 * wlc_taf_down() - interface going down.
 */
static int
wlc_taf_down(void *handle)
{
	/* wlc_taf_info_t *taf_info = handle; */

	return BCME_OK;
}

/*
 * wlc_taf_attach() - attach function, called from wlc_attach_module().
 *
 * Allocate and initialise our context structure, register the module, register the txmod.
 *
 */
wlc_taf_info_t *
BCMATTACHFN(wlc_taf_attach)(wlc_info_t * wlc)
{
	wlc_taf_info_t *taf_info;
	int status;
	taf_scheduler_kind type;

	/* Allocate and initialise our main structure. */
	taf_info = MALLOCZ(wlc->pub->osh, sizeof(*taf_info));
	if (!taf_info) {
		return NULL;
	}

	/* Save backlink to wlc */
	taf_info->wlc = wlc;

	status = wlc_module_register(wlc->pub, taf_iovars, "taf", taf_info,
	                             wlc_taf_doiovar, wlc_taf_watchdog,
	                             wlc_taf_up, wlc_taf_down);

	if (status != BCME_OK) {
		MFREE(taf_info->wlc->pub->osh, taf_info, sizeof(*taf_info));
		return NULL;
	}

	taf_info->enabled = FALSE;

	taf_info->ordering = TAF_ORDER_TID_PARALLEL;
	taf_info->pending_ordering = taf_info->ordering;

#if defined(BCMDBG)
	wlc_dump_register(wlc->pub, "taf", wlc_taf_dump, taf_info);
#endif /* BCMDBG */

	if (D11REV_LT(wlc->pub->corerev, 40)) {
		/* only support on latest chipsets, proceed no more */
		return taf_info;
	}

	/*
	* set up an scb cubby - this returns an offset, or -1 on failure.
	*/
	if ((taf_info->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(taf_scb_cubby_t *),
	                 wlc_taf_scbcubby_init, wlc_taf_scbcubby_exit, NULL, taf_info)) < 0)
	{
		goto exitfail;
	}

	for (type = TAF_SCHEDULER_START; type < NUM_TAF_SCHEDULERS; type++) {
		taf_info->scheduler_context[type] = taf_info;

		if (taf_scheduler_definitions[type].attach_fn) {
			taf_info->scheduler_context[type] =
				taf_scheduler_definitions[type].attach_fn(taf_info, type);
		}

		if (taf_info->scheduler_context[type] == NULL) {
			WL_ERROR(("%s: failed to attach (%d)\n", __FUNCTION__, type));
			goto exitfail;
		}

#if defined(BCMDBG)
		if (taf_scheduler_definitions[type].dump_fn) {
			wlc_dump_register(wlc->pub, taf_scheduler_definitions[type].dump_name,
			                  taf_scheduler_definitions[type].dump_fn,
			                  taf_info->scheduler_context[type]);
		}
#endif /* BCMDBG */
	}

	taf_info->default_scheduler = TAF_ATOS;

	/* the default amount of traffic (in microsecs) to send when forced,
	 * this helps to prevent a link from stalling completely
	 */
	taf_info->force_time = TAF_TIME_FORCE_DEFAULT;

	taf_info->high_max = TAF_TIME_HIGH_MAX;
	taf_info->low_max = TAF_TIME_LOW_MAX;
	taf_info->high = TAF_TIME_HIGH_DEFAULT;
	taf_info->low = TAF_TIME_LOW_DEFAULT;

	taf_info->atos_high = TAF_TIME_ATOS_HIGH_DEFAULT;
	taf_info->atos_low = TAF_TIME_ATOS_LOW_DEFAULT;

	taf_info->atos2_high = TAF_TIME_ATOS2_HIGH_DEFAULT;
	taf_info->atos2_low = TAF_TIME_ATOS2_LOW_DEFAULT;

	wlc_taf_times_sync(taf_info);

	taf_info->fallback = FALSE;

	taf_info->adapt = TAF_TIME_ADAPT_DEFAULT;

	taf_info->mu_extend_coeff = TAF_MU_EXTEND_COEFF_DEFAULT;
	taf_info->ebos_last_active_time = 0;

	/* All is fine, return handle */
	return taf_info;
exitfail:
	wlc_taf_detach(taf_info);
	return NULL;
}

/*
 * wlc_taf_detach() - wlc module detach function, called from wlc_detach_module().
 *
 */
int
BCMATTACHFN(wlc_taf_detach) (wlc_taf_info_t *taf_info)
{
	if (taf_info) {
		taf_scheduler_kind type;
		/* Free the pool list */
		taf_scb_cubby_t *next;
		taf_scb_cubby_t *scb_taf = taf_info->pool;

		while (scb_taf) {
			next = scb_taf->next;
			MFREE(taf_info->wlc->osh, scb_taf, sizeof(*scb_taf));
			scb_taf = next;
		}

		for (type = TAF_SCHEDULER_START; type < NUM_TAF_SCHEDULERS; type++) {

			if (taf_scheduler_definitions[type].detach_fn) {
				void* sch_context = taf_info->scheduler_context[type];

				taf_scheduler_definitions[type].detach_fn(sch_context);
			}
			taf_info->scheduler_context[type] = NULL;
		}

		wlc_module_unregister(taf_info->wlc->pub, "taf", taf_info);

		MFREE(taf_info->wlc->pub->osh, taf_info, sizeof(*taf_info));
	}

	return BCME_OK;
}
