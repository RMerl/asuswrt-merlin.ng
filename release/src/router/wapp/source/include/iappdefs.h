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

/****************************************************************************

	Abstract:

	All related IEEE802.11f IAPP + IEEE802.11r IAPP extension.

***************************************************************************/

#ifndef __IAPP_DEF_H__
#define __IAPP_DEF_H__


/* Some Notes */

#define IAPP_DS_LINK_LOST
/*
	When an AP continues to accept associations without a link to the DS,
	it is a black hole in the WLAN, where STAs associate and cannot
	communicate with anything beyond the AP¡¦s BSS. When an AP loses its link
	to the DS, it should cease transmitting Beacons, disassociate all
	associated stations, and cease responding to Probe Request,
	Authentication, and Association Request frames.
*/


/* Local use file */
#define PROC_NET_ARP_PATH				"/proc/net/arp"
#ifdef OPENWRT_SUPPORT
#define IAPP_PID_BACKUP_FILE			"/tmp/pid_iapp"
#else
#define IAPP_PID_BACKUP_FILE			"/etc/Wireless/pid_iapp"
#endif

/* Definition of IAPP */
#define IAPP_DAEMON_VERSION			"v1.1.0"
#define IAPP_VERSION					0
//#define IAPP_MULTICAST_ADDR				"224.0.1.178" /* used in ADD-req frame*/
//#define IAPP_MULTICAST_ADDR				"192.168.15.255" /* used in ADD-req frame*/

/*
	The UDP Port parameter is the UDP port number to be opened for the IAPP
	for transmission and reception of IAPP packets.
*/
#define IAPP_UDP_PORT					3517 /* used in UDP connection */

/*
	The TCP Port parameter is the TCP port number that the IAPP entity opens
	to listen for new IAPP TCP connections from the IAPP entities of other APs.
*/
#define IAPP_TCP_PORT					3517 /* used in TCP connection */


/* Message type */
#define IAPP_ETH_PRO					0xEEEE
#define IAPPMSGQUEID					1
#define RADIUSMSGQUEID					2
#define RRB_ETH_PRO						0x890D


/* Message subtype */
#define IAPP_OPEN_SERVICE_REQ			0
#define IAPP_OPEN_SERVICE_RSP			1
#define IAPP_CLOSE_SERVICE_REQ			2
#define IAPP_SIGNAL_REQ					3
#define IAPP_SET_OID_REQ				4
#define IAPP_QUERY_OID_REQ				5
#define IAPP_QUERY_OID_RSP				6


/* Definition of signal */
#define IAPP_SIG_NONE					0
#define IAPP_SIG_ASSOCIATION			1
#define IAPP_SIG_REASSOCIATION			2
#define IAPP_SIG_TERMINATE				3


/* Definition of IAPP command */ /* reference to IEEE802.11f page 50 */
#define IAPP_CMD_ADD_NOTIFY				0	/* ADD-notify */
#define IAPP_CMD_MOVE_NOTIFY			1	/* MOVE-notify */
#define IAPP_CMD_MOVE_RESPONSE			2	/* MOVE-response */
#define IAPP_CMD_SEND_SECURITY_BLOCK	3	/* Send-Security-Block */
#define IAPP_CMD_ACK_SECURITY_BLOCK		4	/* ACK-Security-Block */
#define IAPP_CMD_CACHE_NOTIFY			5	/* CACHE-notify */
#define IAPP_CMD_CACHE_RESPONSE			6	/* CACHE-response */

#define IAPP_CMD_FT_SEND_SECURITY_BLOCK	50	/* proprietary FT Send-Security-Block */
#define IAPP_CMD_FT_ACK_SECURITY_BLOCK	51	/* proprietary FT ACK-Security-Block */

#define IAPP_CMD_INFO_BROADCAST			60	/* proprietary INFO broadcast */
#define IAPP_CMD_INFO_REQUEST			61	/* proprietary INFO request */
#define IAPP_CMD_INFO_RESPONSE			62	/* proprietary INFO response */

#define IAPP_CMD_SECURITY_MONITOR		128 /* proprietary, for SM broadcast */


/* Definition of event log */
#define IAPP_MAX_SIZE_OF_EVENT_LOG		1024


/* Definition of general */
#define IAPP_SELECT_TIMEOUT				5 /* unit: second */

#define IAPP_MAX_RCV_PKT_SIZE			1600
#define IAPP_MAX_RCV_PKT_SIZE_SAFE		100

#define IAPP_IP_HEADER_OFFSET			14 /* skip MAC header */
#define IAPP_IP_PROTO_OFFSET			9
#define IAPP_IP_PROTO_UDP				17
#define IAPP_UDP_DST_PORT_OFFSET		(20+2)

#define IAPP_MAC_IP_UDP_LEN				(14+20+8)

/* 16 for extra 8B encryption & 8B-align, other 16B for safe */
#define IAPP_SECURITY_EXTEND_LEN		32

#define IAPP_IN
#define IAPP_OUT
#define IAPP_INOUT

#endif /* __IAPP_DEF_H__ */

/* End of iapp_def.h */
