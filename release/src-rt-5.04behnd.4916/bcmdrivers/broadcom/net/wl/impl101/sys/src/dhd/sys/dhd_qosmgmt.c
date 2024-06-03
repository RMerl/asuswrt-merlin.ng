/*
 * Broadcom Dongle Host Driver (DHD), Linux-specific network interface
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_qosmgmt.c 821234 2023-02-06 14:16:52Z $
 */

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#include <dhd.h>
#include <dhd_linux.h>

#include <dhd_dbg.h>

/* function to process WLC_QOS_MAP_SET event */
int
dhd_qosmap_set_event_process(dhd_pub_t *dhdp, uint8 ifidx, uint8 *event_data, uint32 datalen)
{
	bcm_tlv_t *qosmap_ie = (bcm_tlv_t *)event_data;
	int ret = BCME_OK;

	if (qosmap_ie->id != DOT11_MNG_QOS_MAP_ID) {
		DHD_ERROR(("%s: invalid qosmap ie, id:%d, len:%d\n",
			__FUNCTION__, qosmap_ie->id, qosmap_ie->len));
		ret = BCME_ERROR;
		goto done;
	}

#if defined(BCM_ROUTER) && defined(QOS_MAP_SET)
	ret = dhd_set_qosmap_up_table(dhdp, ifidx, qosmap_ie);
	if (ret != BCME_OK) {
		DHD_ERROR(("%s: failed to update table\n", __FUNCTION__));
		goto done;
	}
#endif /* BCM_ROUTER && QOS_MAP_SET */

done:
	return ret;
}
