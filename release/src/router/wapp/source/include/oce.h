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

#ifndef _OCE_H_
#define _OCE_H_

#include "wapp_cmm.h"
struct wapp_sta;
struct wapp_conf;


#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */


typedef enum {
	OCE_SUCCESS = 0,
	OCE_INVALID_ARG,
	OCE_LOOKUP_ENTRY_NOT_FOUND,
	OCE_UNEXP,
} OCE_ERR_CODE;

typedef enum {
	OCE_CAP_UPDATE, 		/* Oce Capability */
	OCE_DELASSRSSI_UPDATE,	/* DeltaAssocRSSI */
} OCE_STA_UPDATE_CODE;

typedef enum {
	OCE_MSG_INFO_UPDATE = 0,
} OCE_MSG_TYPE;

struct oce_info {
	u8	mac_addr[MAC_ADDR_LEN];
	u8	bssid[MAC_ADDR_LEN];
	u8	OceCapIndication;
	s8	DeltaAssocRSSI;
};

typedef union GNU_PACKED _oce_msg_body{
	struct oce_info OceEvtStaInfo;
} OCE_MSG_BODY;


struct oce_msg {
	u32 ifindex;
	u8  len;
	u8 	type;
	OCE_MSG_BODY body;
};

struct oce_cfg {
	/* call back function of oce events */
	const struct oce_event_ops *event_ops;
	/* driver interface operation */
	//const struct oce_drv_ops *drv_ops;
};

int oce_init(struct oce_cfg *oce);


#endif /* #ifndef _OCE_H_ */

