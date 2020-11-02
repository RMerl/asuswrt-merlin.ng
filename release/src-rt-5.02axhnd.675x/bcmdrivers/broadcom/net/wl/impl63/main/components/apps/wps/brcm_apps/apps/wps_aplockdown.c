/*
 * WPS aplockdown
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
 * $Id: wps_aplockdown.c 772297 2019-02-20 06:50:21Z $
 */

#include <stdio.h>
#include <time.h>
#include <tutrace.h>
#include <wps_wps.h>
#include <wps_wl.h>
#include <wps_aplockdown.h>
#include <wps_ui.h>
#include <wps_ie.h>

typedef struct {
	unsigned int force_on;
	unsigned int locked;
	unsigned int time;
	unsigned int start_cnt;
	unsigned int forever_cnt;
	int failed_cnt;
} WPS_APLOCKDOWN;

#define WPS_APLOCKDOWN_START_CNT	3
#define	WPS_APLOCKDOWN_DEF_FOREVER_CNT	4
#define	WPS_APLOCKDOWN_FOREVER_CNT	10

static WPS_APLOCKDOWN wps_aplockdown;
wps_aplockdown_custom_callback_t wps_aplockdown_custom;

int
wps_aplockdown_init()
{
	char *p;
	int start_cnt = 0;
	int forever_cnt = 0;

	memset(&wps_aplockdown, 0, sizeof(wps_aplockdown));

	if (!strcmp(wps_safe_get_conf("wps_aplockdown_forceon"), "1")) {
		wps_aplockdown.force_on = 1;

		wps_ui_set_env("wps_aplockdown", "1");
		wps_set_conf("wps_aplockdown", "1");
	}
	else {
		wps_ui_set_env("wps_aplockdown", "0");
		wps_set_conf("wps_aplockdown", "0");
	}

	/* check lock start count */
	if ((p = wps_get_conf("wps_lock_start_cnt")))
		start_cnt = atoi(p);

	if (start_cnt < WPS_APLOCKDOWN_START_CNT ||
	    start_cnt > WPS_APLOCKDOWN_FOREVER_CNT) {
		/* Default start count */
		start_cnt = WPS_APLOCKDOWN_START_CNT;
	}

	/* check lock forever count */
	if ((p = wps_get_conf("wps_lock_forever_cnt")))
		forever_cnt = atoi(p);

	if (forever_cnt < WPS_APLOCKDOWN_START_CNT ||
	    forever_cnt > WPS_APLOCKDOWN_FOREVER_CNT) {
		/* Default forever lock count */
		forever_cnt = WPS_APLOCKDOWN_DEF_FOREVER_CNT;
	}

	if (start_cnt > forever_cnt)
		start_cnt = forever_cnt;

	/* Save to structure */
	wps_aplockdown.start_cnt = start_cnt;
	wps_aplockdown.forever_cnt = forever_cnt;

	if (wps_aplockdown_custom.init != NULL) {
		wps_aplockdown_custom.init(&wps_aplockdown.start_cnt,
			&wps_aplockdown.forever_cnt, &wps_aplockdown.failed_cnt);
		if (wps_aplockdown.failed_cnt >= wps_aplockdown.start_cnt) {
			wps_aplockdown.locked = 1;

			wps_ui_set_env("wps_aplockdown", "1");
			wps_set_conf("wps_aplockdown", "1");

			/* reset the IE */
			wps_ie_set(NULL, NULL);

			TUTRACE((TUTRACE_ERR, "AP is initially lock down\n"));
		}
	}
	TUTRACE((TUTRACE_INFO,
		"WPS aplockdown init: force_on = %d, start_cnt = %d,"
		"forever_cnt = %d failed_cnt = %d!\n",
		wps_aplockdown.force_on,
		wps_aplockdown.start_cnt,
		wps_aplockdown.forever_cnt,
		wps_aplockdown.failed_cnt));

	return 0;
}

int
wps_aplockdown_add(void)
{
	unsigned long now;

	time((time_t *)&now);

	TUTRACE((TUTRACE_INFO, "Error of config AP pin fail in %d!\n", now));

	/*
	 * Add PIN failed count
	 */
	if (wps_aplockdown.failed_cnt < wps_aplockdown.forever_cnt)
		wps_aplockdown.failed_cnt++;

	if (wps_aplockdown_custom.wps_pinfail_added != NULL) {
		wps_aplockdown_custom.wps_pinfail_added(wps_aplockdown.start_cnt,
			wps_aplockdown.forever_cnt, &wps_aplockdown.failed_cnt);
		TUTRACE((TUTRACE_INFO,
			"WPS pin faulire: force_on = %d, start_cnt = %d,"
			"forever_cnt = %d failed_cnt = %d!\n",
			wps_aplockdown.force_on, wps_aplockdown.start_cnt,
			wps_aplockdown.forever_cnt, wps_aplockdown.failed_cnt));
	}
	/*
	 * Lock it if reach start count.
	 */
	if (wps_aplockdown.failed_cnt >= wps_aplockdown.start_cnt) {
		wps_aplockdown.locked = 1;
		wps_aplockdown.time = now;

		wps_ui_set_env("wps_aplockdown", "1");
		wps_set_conf("wps_aplockdown", "1");

		/* reset the IE */
		wps_ie_set(NULL, NULL);

		TUTRACE((TUTRACE_ERR, "AP is lock down now\n"));
	}

	TUTRACE((TUTRACE_INFO, "Fail AP pin trial count = %d!\n", wps_aplockdown.failed_cnt));

	return wps_aplockdown.locked;
}

int
wps_aplockdown_check(void)
{
	unsigned long now;
	int ageout;

	if (wps_aplockdown.locked == 0)
		return 0;

	/* check lock forever */
	if (wps_aplockdown.force_on ||
	    wps_aplockdown.failed_cnt >= wps_aplockdown.forever_cnt)
		return 1;

	/* wps_aplockdown.failed_cnt will always >= wps_aplockdown.start_cnt,
	 * so, ageout start from 1 minutes.
	 */
	ageout = (1 << (wps_aplockdown.failed_cnt - wps_aplockdown.start_cnt)) * 60;

	time((time_t *)&now);

	/* Lock release check */
	if ((unsigned long)(now - wps_aplockdown.time) > ageout) {
		/* unset apLockDown indicator */
		wps_aplockdown.locked = 0;

		wps_ui_set_env("wps_aplockdown", "0");
		wps_set_conf("wps_aplockdown", "0");

		/* reset the IE */
		wps_ie_set(NULL, NULL);

		TUTRACE((TUTRACE_INFO, "Unlock AP lock down\n"));
	}

	return wps_aplockdown.locked;
}

int
wps_aplockdown_islocked()
{
	return wps_aplockdown.locked | wps_aplockdown.force_on;
}

int
wps_aplockdown_cleanup()
{
	/* Cleanup dynamic variables */
	wps_aplockdown.locked = 0;
	wps_aplockdown.time = 0;
	wps_aplockdown.failed_cnt = 0;

	return 0;
}
