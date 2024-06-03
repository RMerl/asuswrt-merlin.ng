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
#include "wdev.h"
#include "driver_wext.h"

int wdev_sta_del(struct wifi_app	*wapp, struct wapp_dev	*wdev);

struct wdev_ops wdev_sta_ops = {
	.wdev_del = wdev_sta_del,
};


int wdev_sta_create(
	struct wifi_app *wapp,
	wapp_dev_info *dev_info)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_sta *sta = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !dev_info) {
		return WAPP_INVALID_ARG;
	}

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, dev_info->ifindex);	

	if (wdev == NULL)
		goto new_wdev;
	else if (wdev->p_dev == NULL)
		goto new_sta;
	else if (wdev->dev_type != WAPP_DEV_TYPE_STA/* remove APCLI*/) {
		DBGPRINT_RAW(RT_DEBUG_OFF, "Error! dev_type = %u\n", wdev->dev_type);
		/* check duplicate mac_address? */
		return WAPP_UNEXP;
	} else {
		/* already exsist */
		return WAPP_SUCCESS;
	}

new_wdev:
	if (wapp_dev_create(wapp, (char *) dev_info->ifname, dev_info->ifindex, dev_info->mac_addr) != WAPP_SUCCESS)
		DBGPRINT_RAW(RT_DEBUG_OFF, "Warning! wdev create failed.\n");
	return WAPP_SUCCESS;

new_sta:
	wdev->p_dev = os_zalloc(sizeof(struct wapp_sta));

	if (!wdev->p_dev)
		return WAPP_RESOURCE_ALLOC_FAIL;

	sta = wdev->p_dev;
	os_memset(sta, 0, sizeof(struct wapp_sta));
	dl_list_init(&sta->non_pref_ch_list); 
	wdev->ops = &wdev_sta_ops;

	wdev->dev_type = WAPP_DEV_TYPE_STA;
	wdev->radio = wapp_radio_update_or_create(wapp,
						dev_info->adpt_id,
						dev_info->radio_id);

	if (!wdev->radio) {
		DBGPRINT_RAW(RT_DEBUG_OFF, "Warning! radio not found.\n");
	}

	wdev->wireless_mode = dev_info->wireless_mode;
	wdev->valid = 1;
	return WAPP_SUCCESS;
}

int wdev_sta_del(
struct wifi_app	*wapp,
struct wapp_dev	*wdev)
{
	struct wapp_sta *sta = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}

	sta = (struct wapp_sta *) wdev->p_dev;

	/* free sta */
	os_free(sta);
	wdev->p_dev = NULL;

	return WAPP_SUCCESS;
}

