/*
 * Global ASSERT Logging
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
 * $Id: bcm_assert_log.c 708017 2017-06-29 14:11:45Z $
 */

#include <epivers.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <osl.h>
#include <bcmutils.h>

#include <bcm_assert_log.h>

#ifdef WLC_HIGH
#if !defined(NDIS) && !defined(linux)
#error "ASSERT LOG only supports NDIS and LINUX in HIGH driver"
#endif // endif
#endif /* WLC_HIGH */

#define MAX_ASSERT_NUM		32

struct bcm_assert_info {
	uint8 ref_cnt;
	uint8 cur_idx;
	uint32 seq_num;
#if defined(linux)
	spinlock_t lock;
#elif defined(NDIS)
	NDIS_SPIN_LOCK	lock;
	bool		gotspinlocks;
#endif // endif
	assert_record_t assert_table[MAX_ASSERT_NUM];
};

#if defined(linux)
#define ASSERT_INIT_LOCK(context)	spin_lock_init(&(context)->lock)
#define ASSERT_FREE_LOCK(context)
#define ASSERT_LOCK(context) 		spin_lock(&(context)->lock)
#define ASSERT_UNLOCK(context) 		spin_unlock(&(context)->lock)

#elif defined(NDIS)
#define ASSERT_INIT_LOCK(context) do { \
	NdisAllocateSpinLock(&(context)->lock); \
	(context)->gotspinlocks = TRUE; \
	} while (0)
#define ASSERT_FREE_LOCK(context) do { \
		if ((context)->gotspinlocks) { \
			NdisFreeSpinLock(&(context)->lock); \
			(context)->gotspinlocks = FALSE; \
		} \
	} while (0)
#define ASSERT_LOCK(context)		NdisAcquireSpinLock(&(context)->lock)
#define ASSERT_UNLOCK(context)		NdisReleaseSpinLock(&(context)->lock)
#endif /* defined(linux) */

/* Global assert info table: This table cannot be dynamically allocated, since ndis
 * requires allocations to be bound to a driver instance, so this table will have to
 * be bound to a specific driver instance. This causes memory leaks in multiple
 * instance scenario depending on order of load/unload. Hence, using a static allocation.
 */
bcm_assert_info_t g_assert_info;

void
BCMATTACHFN(bcm_assertlog_init)(void)
{
	bcm_assert_info_t *g_assert_hdl = &g_assert_info;

	if (g_assert_hdl->ref_cnt == 0)
		ASSERT_INIT_LOCK(g_assert_hdl);

	ASSERT_LOCK(g_assert_hdl);
	g_assert_hdl->ref_cnt++;
	ASSERT_UNLOCK(g_assert_hdl);

	return;
}

void
BCMATTACHFN(bcm_assertlog_deinit)(void)
{
	bcm_assert_info_t *g_assert_hdl = &g_assert_info;

	if (g_assert_hdl->ref_cnt) {
		ASSERT_LOCK(g_assert_hdl);
		g_assert_hdl->ref_cnt--;
		ASSERT_UNLOCK(g_assert_hdl);
	}

	if (g_assert_hdl->ref_cnt == 0) {
		ASSERT_FREE_LOCK(g_assert_hdl);
		memset(g_assert_hdl, 0, sizeof(bcm_assert_info_t));
	}

	return;
}

void
bcm_assert_log(char *str)
{
	bcm_assert_info_t *g_assert_hdl = &g_assert_info;
	assert_record_t *cur_record;

	if (g_assert_hdl->ref_cnt == 0)
		return;

	ASSERT_LOCK(g_assert_hdl);

	cur_record = &g_assert_hdl->assert_table[g_assert_hdl->cur_idx];
	cur_record->time = OSL_SYSUPTIME();
	cur_record->seq_num = g_assert_hdl->seq_num++;
	if (str) {
		memset(cur_record->str, 0, MAX_ASSRTSTR_LEN);
		memcpy(cur_record->str, str, MAX_ASSRTSTR_LEN - 1);
	}

	g_assert_hdl->cur_idx = MODINC(g_assert_hdl->cur_idx, MAX_ASSERT_NUM);

	ASSERT_UNLOCK(g_assert_hdl);

	return;
}

int
bcm_assertlog_get(void *outbuf, int iobuf_len)
{
	bcm_assert_info_t *g_assert_hdl = &g_assert_info;
	assertlog_results_t *results = (assertlog_results_t *)outbuf;
	int iobuf_allowed_num = 0;
	uint8 num = 0;
	uint8 last_idx;
	uint8 cur_idx;
	assert_record_t *log_record = &results->logs[0];
	int i;

	if (g_assert_hdl->ref_cnt == 0)
		return -1;

	ASSERT_LOCK(g_assert_hdl);

	iobuf_allowed_num = IOBUF_ALLOWED_NUM_OF_LOGREC(assert_record_t, iobuf_len);

	cur_idx = g_assert_hdl->cur_idx;
	if (g_assert_hdl->seq_num < MAX_ASSERT_NUM) {
		last_idx = 0;
		num = cur_idx;
	}
	else {
		last_idx = MODINC(cur_idx, MAX_ASSERT_NUM);
		num = MAX_ASSERT_NUM - 1;
	}

	num = MIN(iobuf_allowed_num, num);

	results->num = num;
	results->version = ASSERTLOG_CUR_VER;
	results->record_len = sizeof(assert_record_t);

	for (i = 0; i < num; i++) {
		memcpy(&log_record[i], &g_assert_hdl->assert_table[last_idx],
			sizeof(assert_record_t));
		last_idx = MODINC(last_idx, MAX_ASSERT_NUM);
	}

	ASSERT_UNLOCK(g_assert_hdl);

	return 0;
}
