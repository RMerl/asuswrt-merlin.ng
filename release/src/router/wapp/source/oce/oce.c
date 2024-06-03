/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdlib.h>
#include <stdio.h>
#include "wapp_cmm.h"
#include "oce.h"
#include "driver_wext.h"
#ifdef OCE_SUPPORT

struct oce_event_ops;
extern struct oce_event_ops oce_evnt_ops;


int oce_init(struct oce_cfg *oce)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	oce->event_ops = &oce_evnt_ops;
	return OCE_SUCCESS;
}

int oce_sta_update(
	struct wifi_app *wapp,
	struct wapp_sta *sta,
	u8				action,
	u32				len,
	void			*data)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp || !sta) {
		return OCE_INVALID_ARG;
	}

	switch(action)
	{
		case OCE_CAP_UPDATE:
		{
			if (len == sizeof(u8))
				sta->oce_cap= *((u8 *) data);
			break;
		}

		case OCE_DELASSRSSI_UPDATE:
		{
			if (len == sizeof(u32))
				sta->deltaassocrssi = *((u32 *) data);
			break;
		}

		default:
			break;
	}
	return OCE_SUCCESS;
}

int oce_event_sta_update(struct wifi_app *wapp, char *msg, u8 msg_type)
{
	struct wapp_sta *sta = NULL;
	struct oce_msg *pmsg = (struct oce_msg *) msg;
	struct oce_info *info = &pmsg->body.OceEvtStaInfo;
	struct wapp_dev *wdev = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, pmsg->ifindex);

	if (!wdev)
		return OCE_LOOKUP_ENTRY_NOT_FOUND;

	sta = wdev_ap_client_list_lookup(wapp, (struct ap_dev *) wdev->p_dev, info->mac_addr);

	if (!sta)
		return OCE_LOOKUP_ENTRY_NOT_FOUND;

	switch(msg_type)
	{
		case OCE_MSG_INFO_UPDATE:
			printf("\033[1;32m %s, %u info->OceCapIndication 0x%x\033[0m\n"
				, __FUNCTION__, __LINE__,info->OceCapIndication);
			oce_sta_update(	wapp,
							sta,
							OCE_CAP_UPDATE,
							sizeof(sta->oce_cap),
							(void *) &info->OceCapIndication);
			oce_sta_update(	wapp,
							sta,
							OCE_DELASSRSSI_UPDATE,
							sizeof(sta->deltaassocrssi),
							(void *) &info->DeltaAssocRSSI);
			break;
		default:
			break;
	}

	return OCE_SUCCESS;
}

struct oce_event_ops oce_evnt_ops = {
	.sta_update = oce_event_sta_update,
};

#endif /* OCE_SUPPORT */

