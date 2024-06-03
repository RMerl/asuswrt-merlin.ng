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
 
#ifndef __RT_NL_H__
#define __RT_NL_H__

/*
 * RANL_CMD_TEST: Test netlink interface
 *
 * RANL_CMD_IE : Set/Receive information element
 *
 * RANL_CMD_ANQP_REQ: Send/Receive ANQP request to/from driver
 *
 * RANL_CMD_ANQP_RSP: Send/Receive ANQP response to/from driver
 *
 * RANL_CMD_HOTSPOT_ONOFF: Enable/Disable Hotspot2.0 feature
 *
 * RANL_CMD_JOIN: Join to workspace
 *
 * RANL_CMD_LEAVE: Leave the workspace
 *
 */
enum ranl_commands {
	RANL_CMD_UNSPEC,
	RANL_CMD_TEST,
	RANL_CMD_IE,
	RANL_CMD_ANQP_REQ,
	RANL_CMD_ANQP_RSP,
	RANL_CMD_HOTSPOT_ONOFF,
	RANL_CMD_JOIN,
	RANL_CMD_LEAVE,
	__RANL_CMD_AFTER_LAST,
	RANL_CMD_MAX = __RANL_CMD_AFTER_LAST - 1
};


enum ranl_attrs {
	RANL_ATTR_UNSPEC,
	RANL_ATTR_TEST,
	RANL_ATTR_IFINDEX,
	RANL_ATTR_OPMODE,
	RANL_ATTR_PID,
	RANL_ATTR_IE,
	RANL_ATTR_ANQP_DATA,
	RANL_ATTR_HOTSPOT_ONOFF,
	RANL_ATTR_PEER_MAC_ADDR,
	RANL_ATTR_STATUS,
	RANL_ATTR_WORKSPACE_ID,
	RANL_ATTR_HOTSPOT_IPCTYPE,
	__RANL_ATTR_AFTER_LAST,
	RANL_ATTR_MAX = __RANL_ATTR_AFTER_LAST - 1
};


#define RANL_FLAG_NEED_NETDEV 0x01 
#define HS_DAEMON_ID	0x00

int ranl_init(void);
void ranl_exit(void);
void ranl_hotspot_test_event(void);
void ranl_send_anqp_req_event(void *net_dev, const char *peer_mac_addr, 
				              const char *anqp_req, u16 anqp_req_len);
void ranl_send_anqp_rsp_event(void *net_dev, const char *peer_mac_addr,
							  u16 status,
					          const char *anqp_rsp, u16 anqp_rsp_len);
void ranl_hotspot_onff_event(void *net_dev, int onoff);
#endif /* __RT_NL_H__ */
