/*
 * Device config
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
 * $Id: dev_config.c 766179 2018-07-26 07:49:15Z $
 */

#include <string.h>
#include <stdlib.h>

#include <wpsheaders.h>
#include <wpscommon.h>
#include <wpserror.h>
#include <info.h>
#include <portability.h>
#include <tutrace.h>
#include <wps_enrapi.h>
#include <reg_protomsg.h>

/* Functions for devinfo */
DevInfo *
devinfo_new()
{
	DevInfo *dev_info;

	dev_info = (DevInfo *)alloc_init(sizeof(DevInfo));
	if (!dev_info) {
		TUTRACE((TUTRACE_INFO, "Could not create deviceInfo\n"));
		return NULL;
	}

	/* Initialize other member variables */
	dev_info->assocState = WPS_ASSOC_NOT_ASSOCIATED;
	dev_info->configError = 0; /* No error */
	dev_info->devPwdId = WPS_DEVICEPWDID_DEFAULT;

	return dev_info;
}

void
devinfo_delete(DevInfo *dev_info)
{
	if (dev_info) {
		if (dev_info->DHSecret)
			DH_free(dev_info->DHSecret);
		if (dev_info->mp_tlvEsM7Ap)
			reg_msg_es_del(dev_info->mp_tlvEsM7Ap, 0);
		if (dev_info->mp_tlvEsM7Sta)
			reg_msg_es_del(dev_info->mp_tlvEsM7Sta, 0);
		if (dev_info->mp_tlvEsM8Ap)
			reg_msg_es_del(dev_info->mp_tlvEsM8Ap, 0);
		if (dev_info->mp_tlvEsM8Sta)
			reg_msg_es_del(dev_info->mp_tlvEsM8Sta, 0);
#if defined(MULTIAP)
		if (dev_info->mp_tlvEsM8BhSta) {
			reg_msg_es_del(dev_info->mp_tlvEsM8BhSta, 0);
		}
#endif	/* MULTIAP */

		free(dev_info);
	}
	return;
}

uint16
devinfo_getKeyMgmtType(DevInfo *dev_info)
{
	char *pmgmt = dev_info->keyMgmt;

	if (!strcmp(pmgmt, "WPA-PSK"))
		return WPS_WL_AKM_PSK;
	else if (!strcmp(pmgmt, "WPA2-PSK"))
		return WPS_WL_AKM_PSK2;
	else if (!strcmp(pmgmt, "WPA-PSK WPA2-PSK"))
		return WPS_WL_AKM_BOTH;

	return WPS_WL_AKM_NONE;
}

#if defined(MULTIAP)
uint16
devinfo_getBackhaulKeyMgmtType(DevInfo *dev_info)
{
	char *pmgmt = dev_info->backhaul_keyMgmt;

	if (!strcmp(pmgmt, "WPA-PSK"))
		return WPS_WL_AKM_PSK;
	else if (!strcmp(pmgmt, "WPA2-PSK"))
		return WPS_WL_AKM_PSK2;
	else if (!strcmp(pmgmt, "WPA-PSK WPA2-PSK"))
		return WPS_WL_AKM_BOTH;

	return WPS_WL_AKM_NONE;
}
#endif	/* MULTIAP */
