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

#ifndef _MBO_H_
#define _MBO_H_

#include "wapp_cmm.h"
struct wapp_sta;
struct wapp_conf;


#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

#define MAC_ADDR_EQUAL(pAddr1,pAddr2)           !os_memcmp((PVOID)(pAddr1), (PVOID)(pAddr2), MAC_ADDR_LEN)
#define COPY_MAC_ADDR(d, s)             		os_memcpy((d), (s), MAC_ADDR_LEN)

#define MAC_ADDR_LEN 				6

#define OCE_MAX_RNR_NUM_TO_BCN 9

#define	MBO_NEIGHBOR_REPORT_MAX_LEN 128
#define IE_RRM_NEIGHBOR_REP			52
#define IE_MBO_ELEMENT_ID 			221 /* 0xDD */
#define MBO_ATTR_MAX_LEN			252 /* spec 0.0.23 - IE LEN is 256 = OUI 4 + ATTR 252 */

/* MBO Attribute Id List */
#define MBO_ATTR_AP_CAP_INDCATION				1
#define MBO_ATTR_STA_NOT_PREFER_CH_REP			2
#define MBO_ATTR_STA_CDC						3 		/* Cellular Data Capability */
#define MBO_ATTR_AP_ASSOC_DISALLOW				4
#define MBO_ATTR_AP_CDCP						5 		/* Cellular Data Connection Preference */
#define MBO_ATTR_AP_TRANS_REASON				6
#define MBO_ATTR_STA_TRANS_REJ_REASON			7
#define MBO_ATTR_AP_ASSOC_RETRY_DELAY			8
#define MBO_WDEV_ATTR_MAX_NUM					8 		/* Should be updated according to ID list */

typedef enum {
	MBO_MSG_NEIGHBOR_REPORT = 0,
	MBO_MSG_STA_PREF_UPDATE,
	MBO_MSG_CDC_UPDATE,
	MBO_MSG_STA_STEERING,
	MBO_MSG_DISASSOC_STA,
	MBO_MSG_AP_TERMINATION,
	MBO_MSG_BSSID_UPDATE,
	MBO_MSG_REMOVE_STA,
	MBO_MSG_STA_SEC_INFO_UPDATE,
} MBO_MSG_TYPE;

typedef enum {
	MBO_SUCCESS = 0,
	MBO_INVALID_ARG,
	MBO_RESOURCE_ALLOC_FAIL,
	MBO_NOT_INITIALIZED,
	MBO_LOOKUP_ENTRY_NOT_FOUND,
	MBO_UNEXP,
} MBO_ERR_CODE;

enum mbo_transition_reason_code {
    MBO_UNSPECIFIED,
    MBO_EXCESSIVE_FRAME_LOSS_RATE,
    MBO_EXCESSIVE_DELAY,
    MBO_INSUFFICIENT_BW_CAPACITY,
    MBO_LOAD_BALANCING,
    MBO_LOW_RSSI,
    MBO_EXCEEDED_MAX_NUM_OF_RETRANS,
    MBO_HIGH_INTERFERENCE,
    MBO_GRAY_ZONE,
    MBO_TRANSITION_TO_A_PREMIUM_AP,
};

typedef enum {
	MBO_CDC_UPDATE, 		/* Cellular Data Capability */
	MBO_NPC_APPEND,			/* None Prefer Ch */
	MBO_BSSID_UPDATE,		/* Linked BSS */
	MBO_AKM_UPDATE,			/* STA AKM */
	MBO_CIPHER_UPDATE,		/* STA CIPHER */
} MBO_STA_UPDATE_CODE;

typedef enum {
	MBO_FRAME_ANQP = 0,
	MBO_FRAME_BTM,
} MBO_FRAME_TYPE;

typedef enum {
	PARAM_MBO_AP_ASSOC_DISALLOW=0,
	PARAM_MBO_AP_CAP,
	PARAM_MBO_AP_CDCP,
	PARAM_MBO_AP_BSS_TERM,
}MBO_AP_PARAMETER;

struct non_pref_ch {
	u8 ch;
	u8 pref;
	u8 reason_code;
};

typedef struct GNU_PACKED _MBO_STA_CH_PREF_CDC_INFO
{
	u8 mac_addr[MAC_ADDR_LEN];
	u8 bssid[MAC_ADDR_LEN];
	u8	cdc; /* cellular data capability */
	u8	npc_num;
	u32 akm;
	u32 cipher;
	struct non_pref_ch npc[];
} MBO_STA_CH_PREF_CDC_INFO, *P_MBO_STA_CH_PREF_CDC_INFO;

typedef struct GNU_PACKED _MBO_EVENT_STA_DISASSOC
{
	u8 mac_addr[MAC_ADDR_LEN];	
} MBO_EVENT_STA_DISASSOC, *P_MBO_EVENT_STA_DISASSOC;

typedef struct GNU_PACKED _MBO_EVENT_STA_AKM_CIPHER
{
	u8 mac_addr[MAC_ADDR_LEN];
	u32 akm;
	u32 cipher;
} MBO_EVENT_STA_AKM_CIPHER, *P_MBO_EVENT_STA_AKM_CIPHER;

typedef struct GNU_PACKED _MBO_EVENT_BSS_TERM
{
	UINT32 TsfLowPart;
	UINT32 TsfHighPart;
} MBO_EVENT_BSS_TERM, *P_MBO_EVENT_BSS_TERM;

typedef union GNU_PACKED _msg_body{	
	MBO_STA_CH_PREF_CDC_INFO MboEvtStaInfo;
	MBO_EVENT_STA_DISASSOC MboEvtStaDisassoc;
	MBO_EVENT_BSS_TERM MboEvtBssTermTsf;
	//MBO_EVENT_STA_AKM_CIPHER MboEvtStaSecInfo;
} MBO_MSG_BODY;

struct mbo_msg {
	u32 ifindex;
	u8  len;
	u8 	type;
	MBO_MSG_BODY body;
};

struct non_pref_ch_entry {
	struct dl_list list;
	struct non_pref_ch npc;
};

struct mbo_cfg {
	u8  cdcp; 						/* AP's cellular data connection preference */
	u8  assoc_disallow_reason;
	u8  ap_capability;
	u16 assoc_retry_delay;			/* mbo_assoc_retry_delay  unit: 1 second */
	u8  dft_trans_reason; 			/* mbo_default_trans_reason */	

	/* call back function of mbo events */
	const struct mbo_event_ops *event_ops;

	/* driver interface operation */
	const struct mbo_drv_ops *drv_ops;
};

typedef struct {
    u8   AttrID;
    u8   AttrLen;
    //CHAR    AttrBody[1];
    char AttrBody[MBO_ATTR_MAX_LEN];
} MBO_ATTR_STRUCT,*P_MBO_ATTR_STRUCT;

struct mbo_ctrl_cmd {
	char *cmd;	
	int (*cmd_proc)( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
	char *help;
};

int mbo_dl_list_init(struct mbo_cfg *mbo);
int mbo_dl_list_deinit(struct mbo_cfg *mbo);
int mbo_init(struct mbo_cfg *mbo);
struct mbo_sta* mbo_sta_list_lookup(struct mbo_cfg *mbo, const u8 *mac_addr);

int mbo_sta_entry_create(struct mbo_sta **entry);
int mbo_sta_entry_add(struct dl_list *list, struct mbo_sta **entry);

int mbo_check_sta_preference_and_append_nr_list(
	struct wifi_app *wapp,
	struct wapp_sta *sta,
	char *frame_pos, 
	u16 *frame_len, 
	u8 frame_type,
	u8 disassoc_imnt,
	u8 bss_term,
	u8 is_steer_to_cell);

int mbo_append_reduced_nr_list(
	struct wifi_app *wapp,
	char *frame_pos, 
	u16 *frame_len);

void mbo_make_mbo_ie_for_btm(
	struct wifi_app *wapp,
	char *frame_pos, 
	u16 *frame_len, 
	u8 b_insert_cdcp,
	u8 b_insert_tran_reason, 
	u8 tran_reason,
	u8 b_insert_retry_delay);

int mbo_ctrl_interface_cmd_handle(
	struct wifi_app *wapp,
	const char *iface,
	u8 argc,
	char **argv);

inline int mbo_param_setting(
	struct wifi_app *wapp,
	const char *iface, 
	u32 param,
	u32 value);

int mbo_cmd_show_help( 
	struct wifi_app *wapp,
	const char *iface, 
	u8 argc, 
	char **argv);

size_t wapp_calc_mbo_anqp_rsp_len(struct wifi_app *wapp,
									struct wapp_conf *conf,
									size_t mbo_anqp_req_len,
									const char *curpos);

int wapp_collect_mbo_anqp_rsp(struct wifi_app *wapp,
							struct wapp_conf *conf, char *buf);

#endif /* #ifndef _MBO_H_ */



