/*
 * NSA generic application API
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
 * $Id: $
 */
#include <stdio.h>
#include <stdlib.h>

#include "nsa_api.h"
#include "gki_int.h"
#include "uipc.h"

#include "nsa_dm_api.h"
#include "app_generic.h"
#include "app_nsa_utils.h"
#include "app_ndef_data_test.h"
#include "nsa_ndef_utils.h"
#include "ndef_utils.h"

/*
* Types
*/

typedef enum
{
	APP_NSA_NO_ACTION,
	APP_NSA_CHO
} tAPP_NSA_STOP_ACTION;

typedef enum
{
	NSA_CHO_NONE,
	NSA_CHO_BT,
	NSA_CHO_WIFI
} tNSA_CHO_SEL_OPTION;

typedef struct
{
	/* NDEF handler */
	tNFA_HANDLE          ndef_default_handle;
	tAPP_NSA_STOP_ACTION pending_stop_action;

	/* CHO */
	BOOLEAN cho_server_started;
} tAPP_NSA_CB;

/*
* Global variables
*/
tAPP_NSA_CB app_nsa_cb;

/*
* Local variables
*/
static char *p_wsc_type = "application/vnd.wfa.wsc";
static char *p_p2p_type = "application/vnd.wfa.p2p";
#define WFA_WSC_TYPE_LEN 23
#define WFA_P2P_TYPE_LEN 23

static const tNFA_INTF_TYPE prot_to_rf_if[] =
{
	NFA_INTERFACE_FRAME,    /* UNKNOWN */
	NFA_INTERFACE_FRAME,    /* NFA_PROTOCOL_T1T */
	NFA_INTERFACE_FRAME,    /* NFA_PROTOCOL_T2T */
	NFA_INTERFACE_FRAME,    /* NFA_PROTOCOL_T3T */
	NFA_INTERFACE_ISO_DEP,  /* NFA_PROTOCOL_ISO_DEP */
	NFA_INTERFACE_NFC_DEP   /* NFA_PROTOCOL_NFC_DEP */
};

/*
 * Local functions
 */
int app_nsa_dm_continue_stop_cho(void);

/* definition of app callbacks */
extern void wps_nfc_dm_cback(tNSA_CONN_EVT event, int status);
extern void wps_nfc_done_cback(tNSA_CONN_EVT event, int status);
extern void wps_nfc_conn_cback(tNSA_CONN_EVT event, tNSA_STATUS status);
extern void wps_nfc_ndef_cback(UINT8 *ndef, UINT32 ndef_len);

BOOLEAN nsa_parse_cho_wifi_req(tNSA_CHO_REQUEST * p_req)
{
	UINT8 *p_rec;
	int i;

	/* parse and display received request */
	for (i = 0; i < p_req->num_ac_rec; i++) {
		APP_INFO1("\t\t Parsing AC = %d ", i);
		APP_INFO1("\t\t   cps = % d ", p_req->ac_rec[i].cps);
		APP_INFO1("\t\t   aux_data_ref_count = % d ", p_req->ac_rec[i].aux_data_ref_count);
	}

	APP_DUMP("\t\tNDEF CHO req", p_req->ref_ndef, p_req->ref_ndef_len);
	APP_DEBUG0("\t\t Start to parse NDEF content of CHO req.");
	ParseNDefData(p_req->ref_ndef, p_req->ref_ndef_len);

	/* look for WIFI records */
	p_rec = NDEF_MsgGetFirstRecByType(p_req->ref_ndef, NDEF_TNF_MEDIA,
		(UINT8*) p_wsc_type, (UINT8)strlen((char *) p_wsc_type));
	if (p_rec) {
		APP_INFO1("\t\t Wifi Config present at 0x%x ", p_rec);
		return TRUE;
	}

	return FALSE;
}

void nsa_parse_cho_sel(tNSA_CHO_SELECT *p_sel)
{
	UINT8 *p_rec;
	int i;

	/* parse and display received request */
	for (i = 0; i < p_sel->num_ac_rec; i++) {
		APP_INFO1("\t\t Parsing AC = %d ", i);
		APP_INFO1("\t\t   cps = % d ", p_sel->ac_rec[i].cps);
		APP_INFO1("\t\t   aux_data_ref_count = % d ", p_sel->ac_rec[i].aux_data_ref_count);
	}

	APP_DUMP("\t\tNDEF CHO sel", p_sel->ref_ndef, p_sel->ref_ndef_len);
	APP_DEBUG0("\t\t Start to parse NDEF content of CHO sel.");
	ParseNDefData(p_sel->ref_ndef, p_sel->ref_ndef_len);

	/* look for WIFI records */
	p_rec = NDEF_MsgGetFirstRecByType(p_sel->ref_ndef, NDEF_TNF_MEDIA,
		(UINT8*) p_wsc_type, (UINT8)strlen((char *) p_wsc_type));
	if (p_rec) {
		APP_INFO1("\t\t Wifi Config select present at 0x%x ", p_rec);
	}
}

void app_nsa_cho_cback(tNSA_CHO_EVT event, tNSA_CHO_EVT_DATA *p_data)
{
	tNSA_CHO_SEND_SEL nsa_cho_send_sel;
	tNSA_CHO_SEND_SEL_ERR nsa_cho_send_sel_err;
	tNSA_STATUS nsa_status;
	UINT8 *p_ndef_header;
	tNSA_CHO_SELECT *p_sel;
	tNSA_CHO_REQUEST *p_req;
	int i;

	switch (event) {
	case NSA_CHO_STARTED_EVT:
		APP_INFO1("NSA_CHO_STARTED_EVT Status:%d", p_data->status);
		break;
	case NSA_CHO_REQ_EVT:
		APP_INFO1("NSA_CHO_REQ_EVT Status:%d", p_data->request.status);
		if (p_data->request.status == NFA_STATUS_OK) {
			APP_INFO1("\t received %d Alternative Carrier", p_data->request.num_ac_rec);
		}
		else {
			app_nsa_cb.cho_server_started = FALSE;
			break;
		}

		p_req = &p_data->request;

		if (nsa_parse_cho_wifi_req(p_req) == FALSE) {
			NSA_ChoSendSelectErrorInit(&nsa_cho_send_sel_err);

			nsa_cho_send_sel_err.error_data = 0;
			nsa_cho_send_sel_err.error_reason = NFA_CHO_ERROR_CARRIER;

			nsa_status = NSA_ChoSendSelectError(&nsa_cho_send_sel_err);
			if (nsa_status != NSA_SUCCESS) {
				APP_ERROR1("NSA_ChoSendSelectError failed:%d", nsa_status);
			}

			break;
		}

		/* parse wifi req successful */
		NSA_ChoSendSelectInit(&nsa_cho_send_sel);
		NDEF_MsgInit(nsa_cho_send_sel.ndef_data, MAX_NDEF_LENGTH,
			&nsa_cho_send_sel.ndef_len);

#ifdef WFA_WPS_20_NFC_TESTBED
		/* Just in case we need put p2p before wsc in plugfest. */
		if (strcmp(wps_safe_get_conf("wps_nfc_fake_p2p"), "1") == 0) {
			/* add a fake p2p hard-coded WIFI_OOB and ac info value,
			 * restore NDEF header which might be updated during
			 * previous appending record
			 */
			memcpy(&wifi_hs_rec[NDEF_PAYLOAD_TYPE_OFFSET], p_p2p_type,
				WFA_P2P_TYPE_LEN);
			p_ndef_header = (UINT8*)wifi_hs_rec;
			*p_ndef_header |= (NDEF_ME_MASK|NDEF_MB_MASK);

			NDEF_MsgAppendRec(nsa_cho_send_sel.ndef_data, MAX_NDEF_LENGTH,
			&nsa_cho_send_sel.ndef_len, wifi_hs_rec, wifi_hs_rec_len);

			nsa_cho_send_sel.ac_info[nsa_cho_send_sel.num_ac_info].cps =
				NFA_CHO_CPS_ACTIVE;
			nsa_cho_send_sel.ac_info[nsa_cho_send_sel.num_ac_info].num_aux_data = 0;
			nsa_cho_send_sel.num_ac_info++;
		}
#endif /* WFA_WPS_20_NFC_TESTBED */

		/* add hard-coded WIFI_OOB and ac info value,
		 * restore NDEF header which might be updated during
		 * previous appending record
		 */
		memcpy(&wifi_hs_rec[NDEF_PAYLOAD_TYPE_OFFSET], p_wsc_type, WFA_WSC_TYPE_LEN);
		p_ndef_header = (UINT8*)wifi_hs_rec;
		*p_ndef_header |= (NDEF_ME_MASK|NDEF_MB_MASK);

		NDEF_MsgAppendRec(nsa_cho_send_sel.ndef_data, MAX_NDEF_LENGTH,
			&nsa_cho_send_sel.ndef_len, wifi_hs_rec, wifi_hs_rec_len);

		nsa_cho_send_sel.ac_info[nsa_cho_send_sel.num_ac_info].cps = NFA_CHO_CPS_ACTIVE;
		nsa_cho_send_sel.ac_info[nsa_cho_send_sel.num_ac_info].num_aux_data = 0;
		nsa_cho_send_sel.num_ac_info++;

#ifdef WFA_WPS_20_NFC_TESTBED
		/* For AP device we should put wsc before fake p2p and the
		 * STA should pick the first wsc record.
		*/
		if (strcmp(wps_safe_get_conf("wps_nfc_fake_p2p"), "2") == 0) {
			/* add a fake p2p hard-coded WIFI_OOB and ac info value,
			 * restore NDEF header which might be updated during
			 * previous appending record
			 */
			memcpy(&wifi_hs_rec[NDEF_PAYLOAD_TYPE_OFFSET], p_p2p_type,
				WFA_P2P_TYPE_LEN);
			p_ndef_header = (UINT8*)wifi_hs_rec;
			*p_ndef_header |= (NDEF_ME_MASK|NDEF_MB_MASK);

			NDEF_MsgAppendRec(nsa_cho_send_sel.ndef_data, MAX_NDEF_LENGTH,
			&nsa_cho_send_sel.ndef_len, wifi_hs_rec, wifi_hs_rec_len);

			nsa_cho_send_sel.ac_info[nsa_cho_send_sel.num_ac_info].cps =
				NFA_CHO_CPS_ACTIVE;
			nsa_cho_send_sel.ac_info[nsa_cho_send_sel.num_ac_info].num_aux_data = 0;
			nsa_cho_send_sel.num_ac_info++;
		}
#endif /* WFA_WPS_20_NFC_TESTBED */

		if (NSA_ChoSendSelect(&nsa_cho_send_sel) == NSA_SUCCESS)
			wps_nfc_ndef_cback(p_req->ref_ndef, p_req->ref_ndef_len);

		break;

	case NSA_CHO_SEL_EVT:
		APP_INFO1("NSA_CHO_SEL_EVT Status:%d", p_data->status);
		if (p_data->status == NFA_STATUS_OK) {
			APP_INFO1("\t received %d Alternative Carrier", p_data->request.num_ac_rec);
		}
		else
			break;

		/* Just for WSC NDEF */
		p_sel = &p_data->select;

		/* parse and display received request */
		for (i = 0; i < p_sel->num_ac_rec; i++) {
			APP_INFO1("\t\t Parsing AC = %d ", i);
			APP_INFO1("\t\t   cps = % d ", p_sel->ac_rec[i].cps);
			APP_INFO1("\t\t   aux_data_ref_count = % d ",
				p_sel->ac_rec[i].aux_data_ref_count);
		}

		APP_DUMP("\t\tNDEF CHO sel", p_sel->ref_ndef, p_sel->ref_ndef_len);
		APP_DEBUG0("\t\t Start to parse NDEF content of CHO sel.");

		wps_nfc_ndef_cback(p_sel->ref_ndef, p_sel->ref_ndef_len);
		break;

	case NSA_CHO_SEL_ERR_EVT:
		APP_INFO0("NSA_CHO_SEL_ERR_EVT");
		APP_INFO1("\terror_data = 0x%x", p_data->sel_err.error_data);
		APP_INFO1("\terror_reason = 0x%x", p_data->sel_err.error_reason);
		break;

	case NSA_CHO_TX_FAIL_ERR_EVT:
		APP_INFO1("NSA_CHO_TX_FAIL_ERR_EVT Status:%d", p_data->status);
		break;

	default:
		APP_ERROR1("Bad CHO Event", event);
		break;
	}
}

void app_nsa_rw_cback(tNSA_RW_EVT event, tNSA_RW_EVT_DATA *p_data)
{
	switch (event) {
	case NSA_RW_STOP_EVT:
		/* complete RW action */
		APP_INFO1("NSA_RW_STOP_EVT Status:%d", p_data->status);
		if (p_data->status == NFA_STATUS_OK) {
			APP_INFO0("Ready to receive a new RW command.");
		}

		wps_nfc_done_cback(NSA_RW_STOP_EVT, p_data->status == NFA_STATUS_OK ? 0 : -1);
		break;

	default:
		APP_ERROR1("Bad RW Event", event);
		break;
	}
}

void app_nsa_dm_cback(tNSA_DM_EVT event, tNSA_DM_MSG *p_data)
{
	switch (event) {
	case NSA_DM_ENABLED_EVT:
		/* Result of NFA_Enable */
		APP_INFO1("NSA_DM_ENABLED_EVT Status:%d", p_data->status);
		wps_nfc_dm_cback(NSA_DM_ENABLED_EVT, p_data->status);
		break;
	case NSA_DM_DISABLED_EVT:
		/* Result of NFA_Disable */
		APP_INFO0("NSA_DM_DISABLED_EVT");
		break;
	case NSA_DM_SET_CONFIG_EVT:
		/* Result of NFA_SetConfig */
		APP_INFO0("NSA_DM_SET_CONFIG_EVT");
		break;
	case NSA_DM_GET_CONFIG_EVT:
		/* Result of NFA_GetConfig */
		APP_INFO0("NSA_DM_GET_CONFIG_EVT");
		break;
	case NSA_DM_PWR_MODE_CHANGE_EVT:
		/* Result of NFA_PowerOffSleepMode */
		APP_INFO0("NSA_DM_PWR_MODE_CHANGE_EVT");
		break;
	case NSA_DM_RF_FIELD_EVT:
		/* Status of RF Field */
		APP_INFO1("NSA_DM_RF_FIELD_EVT Status:%d RfFieldStatus:%d",
			p_data->rf_field.status, p_data->rf_field.rf_field_status);
		break;
	case NSA_DM_NFCC_TIMEOUT_EVT:
		/* NFCC is not responding */
		APP_INFO0("NSA_DM_ENABLED_EVT");
		break;
	case NSA_DM_NFCC_TRANSPORT_ERR_EVT:
		/* NCI Tranport error */
		APP_INFO0("NSA_DM_ENABLED_EVT");
		break;
	default:
		APP_ERROR1("Bad Event", event);
		break;
	}
}

void
app_nsa_conn_cback(tNSA_CONN_EVT event, tNSA_CONN_MSG *p_data)
{
	tNSA_DM_SELECT nsa_dm_select;
	tNSA_STATUS nsa_status;

	switch (event) {
	case NSA_POLL_ENABLED_EVT:
		APP_INFO1("NSA_POLL_ENABLED_EVT status:%d", p_data->status);
		break;
	case NSA_POLL_DISABLED_EVT:
		APP_INFO1("NSA_POLL_DISABLED_EVT status:%d", p_data->status);
		break;
	case NSA_RF_DISCOVERY_STARTED_EVT:
		APP_INFO0("NSA_RF_DISCOVERY_STARTED_EVT");
		break;
	case NSA_ACTIVATED_EVT:
		APP_INFO0("NSA_ACTIVATED_EVT");
		break;
	case NSA_DEACTIVATED_EVT:
		APP_INFO0("NSA_DEACTIVATED_EVT");
		wps_nfc_conn_cback(event, p_data->status);
		break;
	case NSA_NDEF_DETECT_EVT:
		APP_INFO1("NSA_NDEF_DETECT_EVT status:%d", p_data->ndef_detect.status);
		break;
	case NSA_RF_DISCOVERY_STOPPED_EVT:
		APP_DEBUG0("RF discovery has been stopped.");

		switch (app_nsa_cb.pending_stop_action) {
		case APP_NSA_CHO:
			app_nsa_dm_continue_stop_cho();
			break;
		default:
			APP_DEBUG1("nothing to be stopped (%d)", app_nsa_cb.pending_stop_action);
		}

		app_nsa_cb.pending_stop_action = APP_NSA_NO_ACTION;
		break;
	case NSA_DATA_EVT:
		APP_INFO0("NSA_DATA_EVT");
		break;
	case NSA_LLCP_ACTIVATED_EVT:
		APP_INFO0("NSA_LLCP_ACTIVATED_EVT");
		break;
	case NSA_LLCP_DEACTIVATED_EVT:
		APP_INFO0("NSA_LLCP_DEACTIVATED_EVT");
		break;
	case NSA_SET_P2P_LISTEN_TECH_EVT:
		APP_INFO0("NSA_SET_P2P_LISTEN_TECH_EVT");
		break;
	case NSA_SELECT_RESULT_EVT:
		APP_INFO0("NSA_SELECT_RESULT_EVT");
		if (p_data->status == NFA_STATUS_FAILED) {
			APP_ERROR0("selection failed!!!");
		}
		break;
	case NSA_READ_CPLT_EVT:
		APP_INFO1("NSA_READ_CPLT_EVT: %d", p_data->status);
		wps_nfc_conn_cback(event, p_data->status);
		break;
	case NSA_WRITE_CPLT_EVT:
		APP_INFO1("NSA_WRITE_CPLT_EVT: %d", p_data->status);
		wps_nfc_conn_cback(event, p_data->status);
		break;
	case NSA_FORMAT_CPLT_EVT:
		APP_INFO1("NSA_FORMAT_CPLT_EVT: %d", p_data->status);
		wps_nfc_conn_cback(event, p_data->status);
		break;
	case NSA_SET_TAG_RO_EVT:
		APP_INFO1("NSA_SET_TAG_RO_EVT: %d", p_data->status);
		break;
	case NSA_DISC_RESULT_EVT:
		APP_INFO0("NSA_DISC_RESULT_EVT");
		APP_INFO1("rf_disc_id = %d", p_data->disc_result.discovery_ntf.rf_disc_id);
		APP_INFO1("protocol = %d", p_data->disc_result.discovery_ntf.protocol);
		APP_INFO1("more = %d", p_data->disc_result.discovery_ntf.more);
		if (p_data->disc_result.discovery_ntf.more == 0) {
			/* just choose latest disc result in this demo code */
			NSA_DmSelectInit(&nsa_dm_select);

			nsa_dm_select.protocol = p_data->disc_result.discovery_ntf.protocol;
			nsa_dm_select.rf_disc_id = p_data->disc_result.discovery_ntf.rf_disc_id;
			if (nsa_dm_select.protocol > NFC_PROTOCOL_NFC_DEP)
				nsa_dm_select.rf_interface = NCI_INTERFACE_FRAME;
			else
				nsa_dm_select.rf_interface = prot_to_rf_if[nsa_dm_select.protocol];

			/* add dummy delay */
			GKI_delay(100);

			nsa_status = NSA_DmSelect(&nsa_dm_select);
			if (nsa_status != NSA_SUCCESS) {
				APP_ERROR1("NSA_DmSelect failed:%d", nsa_status);
			}
		}
		break;

	default:
		APP_ERROR1("Unknown event:%d", event);
	}
}

static void
app_nsa_default_ndef_hdlr(tNSA_NDEF_EVT event, tNSA_NDEF_EVT_DATA *p_data)
{
	switch (event) {
	case NSA_NDEF_DATA_EVT:
		APP_DUMP("NDEF", p_data->ndef_data.data, p_data->ndef_data.len);
		APP_DEBUG0("--->>> Start to parse NDEF content.");
		wps_nfc_ndef_cback(p_data->ndef_data.data, p_data->ndef_data.len);
		break;

	case NSA_NDEF_REGISTER_EVT:
		if (p_data->ndef_reg.status == NFA_STATUS_OK)
		{
			APP_DEBUG1("Ndef handler 0x%x has been correctly registered.",
				p_data->ndef_reg.ndef_type_handle);
			app_nsa_cb.ndef_default_handle = p_data->ndef_reg.ndef_type_handle;
		}
		else
		{
			APP_DEBUG1("Ndef handler 0x%x has NOT been correctly registered.",
				p_data->ndef_reg.ndef_type_handle);
		}
		break;

	default:
		APP_ERROR1("Unknown event:%d", event);
	}
}

int app_nsa_gen_init(void)
{
	memset(&app_nsa_cb, 0, sizeof(app_nsa_cb));
	return 0;
}

void app_nsa_end(void)
{
	app_nsa_dm_disable();
}

int app_nsa_dm_enable(void)
{
	tNSA_DM_ENABLE nsa_req;
	tNSA_STATUS nsa_status;
	int port;

	NSA_DmEnableInit(&nsa_req);

	nsa_req.dm_cback = app_nsa_dm_cback;
	nsa_req.conn_cback = app_nsa_conn_cback;

	/* Select Port: 0 for local, other for BSA IPC */
	port = 0;

	nsa_req.port = (UINT8)port;

	nsa_status = NSA_DmEnable(&nsa_req);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_DmEnable failed:%d", nsa_status);
		return -1;
	}

	return 0;
}

int app_nsa_dm_disable(void)
{
	tNSA_DM_DISABLE nsa_req;
	tNSA_STATUS nsa_status;

	NSA_DmDisableInit(&nsa_req);

	nsa_req.graceful = TRUE;

	nsa_status = NSA_DmDisable(&nsa_req);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_DmDisable failed:%d", nsa_status);
		return -1;
	}
	return 0;
}

/* NSA RW command */
int app_nsa_rw_write_wps(UINT8 *payload, UINT32 payload_size)
{
	tNSA_RW_WRITE            nsa_rw_write;
	tNSA_STATUS              nsa_status;
	UINT32			 cp_size;

	NSA_RwWriteInit(&nsa_rw_write);

	nsa_rw_write.for_ever = 0; /* just one tag */

	/* Update new layload */
	cp_size = payload_size;
	if (cp_size > (MAX_NDEF_LENGTH - WIFI_NDEF_REC_WPS_OFFSET)) {
		APP_ERROR1("Read/Write NDEF payload size %d, too big trim off!\n", cp_size);
		cp_size = MAX_NDEF_LENGTH - WIFI_NDEF_REC_WPS_OFFSET;
	}

	memcpy(&wifi_wps[WIFI_NDEF_REC_WPS_OFFSET], payload, cp_size);
	wifi_wps[NDEF_PAYLOAD_LEN_OFFSET] = cp_size;
	wifi_wps_len = WIFI_NDEF_REC_WPS_OFFSET + cp_size;

	nsa_rw_write.ndef_length = wifi_wps_len;
	memcpy(nsa_rw_write.ndef_data, wifi_wps, nsa_rw_write.ndef_length);
	nsa_rw_write.rw_cback = app_nsa_rw_cback;

	nsa_status = NSA_RwWrite(&nsa_rw_write);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_RwWrite failed:%d", nsa_status);
		return nsa_status;
	}

	return 0;
}

int app_nsa_rw_read(void)
{
	tNSA_RW_READ nsa_rw_read;
	tNSA_STATUS nsa_status;
	tNSA_DM_REG_NDEF_TYPE_HDLR nsa_ndef_reg_req;

	/* first register a default NDEF handler */
	NSA_DmRegNdefTypeHdlrInit(&nsa_ndef_reg_req);

	nsa_ndef_reg_req.handle_whole_msg = TRUE;
	nsa_ndef_reg_req.tnf = NSA_TNF_DEFAULT;
	nsa_ndef_reg_req.p_ndef_cback = app_nsa_default_ndef_hdlr;
	memcpy(nsa_ndef_reg_req.type_name, "", 0);
	nsa_ndef_reg_req.type_name_len = 0;

	nsa_status = NSA_DmRegNdefTypeHdlr(&nsa_ndef_reg_req);
	if (nsa_status != NSA_SUCCESS) {
	APP_ERROR1("NSA_DmRegNdefTypeHdlr failed:%d", nsa_status);
	return nsa_status;
	}

	NSA_RwReadInit(&nsa_rw_read);

	/* 1: Continue Read or 0: just one tag */
	nsa_rw_read.for_ever = 0;
	nsa_rw_read.rw_cback = app_nsa_rw_cback;

	nsa_status = NSA_RwRead(&nsa_rw_read);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_RwRead failed:%d", nsa_status);
		return nsa_status;
	}

	return 0;
}

int app_nsa_rw_format(void)
{
	tNSA_RW_FORMAT nsa_rw_format;
	tNSA_STATUS nsa_status;

	NSA_RwFormatInit(&nsa_rw_format);

	/* 1: Continue format or 0: just one tag */
	nsa_rw_format.for_ever = 0;
	nsa_rw_format.rw_cback = app_nsa_rw_cback;

	nsa_status = NSA_RwFormat(&nsa_rw_format);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_RwFormat failed:%d", nsa_status);
		return nsa_status;
	}

	return 0;
}

int app_nsa_rw_stop(void)
{
	tNSA_RW_STOP	nsa_rw_stop;
	tNSA_STATUS     nsa_status;

	NSA_RwStopInit(&nsa_rw_stop);

	nsa_status = NSA_RwStop(&nsa_rw_stop);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_RwStop failed:%d", nsa_status);
		return -1;
	}
	return 0;
}

/* NSA CHO command */
int app_nsa_cho_start(BOOLEAN cho_as_server, UINT8 *payload, UINT32 payload_size)
{
	tNSA_CHO_START nsa_cho_start;
	tNSA_DM_SET_P2P_TECH nsa_set_p2p_tech_req;
	tNSA_DM_ENABLEPOLL nsa_enable_poll_req;
	tNSA_DM_START_RF_DISCOV nsa_start_rf_req;
	tNSA_STATUS nsa_status;
	UINT32 cp_size;

	/* sanity check */
	if (!payload || !payload_size) {
		APP_ERROR0("NSA_ChoStart in server mode must have payload argument\n");
		return -1;
	}

	NSA_ChoStartInit(&nsa_cho_start);

	app_nsa_cb.cho_server_started = cho_as_server;
	nsa_cho_start.reg_server = app_nsa_cb.cho_server_started;

	/* Update new layload */
	cp_size = payload_size;
	if (cp_size > (MAX_NDEF_LENGTH - WIFI_CHO_REC_WPS_OFFSET)) {
		APP_ERROR1("Hand over Selector payload size %d, too big trim off!\n", cp_size);
		cp_size = MAX_NDEF_LENGTH - WIFI_CHO_REC_WPS_OFFSET;
	}

	memcpy(&wifi_hs_rec[NDEF_PAYLOAD_TYPE_OFFSET], p_wsc_type, WFA_WSC_TYPE_LEN);
	if (cho_as_server) {
		memcpy(&wifi_hs_rec[WIFI_CHO_REC_WPS_OFFSET], payload, cp_size);
		wifi_hs_rec[NDEF_PAYLOAD_LEN_OFFSET] = cp_size;
		wifi_hs_rec_len = WIFI_CHO_REC_WPS_OFFSET + cp_size;
	} else {
		memcpy(&wifi_hr_rec[WIFI_CHO_REC_WPS_OFFSET], payload, cp_size);
		wifi_hr_rec[NDEF_PAYLOAD_LEN_OFFSET] = cp_size;
		wifi_hr_rec_len = WIFI_CHO_REC_WPS_OFFSET + cp_size;
	}

	nsa_cho_start.p_callback = app_nsa_cho_cback;

	nsa_status = NSA_ChoStart(&nsa_cho_start);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_ChoStart failed:%d", nsa_status);
		return nsa_status;
	}

	if (nsa_cho_start.reg_server) {
		/* set P2P tech listen mask for CHO, needed only if server is
		  * started to receive CHO req
		  */
		NSA_DmSetP2pListenTechInit(&nsa_set_p2p_tech_req);

		nsa_set_p2p_tech_req.poll_mask = NFA_TECHNOLOGY_MASK_ALL;

		nsa_status = NSA_DmSetP2pListenTech(&nsa_set_p2p_tech_req);
		if (nsa_status != NSA_SUCCESS) {
			APP_ERROR1("NSA_DmSetP2pListenTech failed:%d", nsa_status);
			return nsa_status;
		}
	}

	/* set polling mask */
	NSA_DmEnablePollingInit(&nsa_enable_poll_req);
	nsa_enable_poll_req.poll_mask = NFA_TECHNOLOGY_MASK_ALL;

	nsa_status = NSA_DmEnablePolling(&nsa_enable_poll_req);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_DmEnablePolling failed:%d", nsa_status);
		return nsa_status;
	}

	/* start RF discovery */
	NSA_DmStartRfDiscovInit(&nsa_start_rf_req);
	/* no param to be passed */
	nsa_status = NSA_DmStartRfDiscov(&nsa_start_rf_req);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_DmStartRfDiscov failed:%d", nsa_status);
		return nsa_status;
	}

	return 0;
}

int app_nsa_cho_stop(void)
{
	tNSA_DM_STOP_RF_DISCOV nsa_stop_rf_req;
	tNSA_STATUS nsa_status;

	/* first stop RF discovery */
	NSA_DmStopRfDiscovInit(&nsa_stop_rf_req);
	/* no param to be passed */
	nsa_status = NSA_DmStopRfDiscov(&nsa_stop_rf_req);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_DmStopRfDiscov failed:%d", nsa_status);
		return -1;
	}

	/* it will be continued once RF discovery stopped event is received */
	app_nsa_cb.pending_stop_action = APP_NSA_CHO;

	return 0;
}

int app_nsa_dm_continue_stop_cho(void)
{
	tNSA_DM_SET_P2P_TECH nsa_set_p2p_tech_req;
	tNSA_CHO_STOP nsa_cho_stop;
	tNSA_STATUS nsa_status;
	tNSA_DM_DISABLEPOLL nsa_disable_poll_req;

	NSA_DmDisablePollingInit(&nsa_disable_poll_req);
	nsa_status = NSA_DmDisablePolling(&nsa_disable_poll_req);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_DmDisablePolling failed:%d", nsa_status);
		return -1;
	}

	if (app_nsa_cb.cho_server_started) {
		/* disable P2P tech mask; neede only if started */
		NSA_DmSetP2pListenTechInit(&nsa_set_p2p_tech_req);

		nsa_set_p2p_tech_req.poll_mask = 0x00;
		nsa_status = NSA_DmSetP2pListenTech(&nsa_set_p2p_tech_req);
		if (nsa_status != NSA_SUCCESS) {
			APP_ERROR1("NSA_DmSetP2pListenTech failed:%d", nsa_status);
			return -1;
		}
	}

	NSA_ChoStopInit(&nsa_cho_stop);

	nsa_status = NSA_ChoStop(&nsa_cho_stop);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_ChoStop failed:%d", nsa_status);
		return -1;
	}

	app_nsa_cb.cho_server_started = FALSE;

	return 0;
}

int app_nsa_cho_send(void)
{
	tNSA_CHO_SEND_REQ nsa_cho_send_req;
	tNSA_STATUS nsa_status;
	tNDEF_STATUS ndef_status;
	UINT8 *p_ndef_header;

	NSA_ChoSendRequestInit(&nsa_cho_send_req);

	NDEF_MsgInit(nsa_cho_send_req.ndef_data, MAX_NDEF_LENGTH, &(nsa_cho_send_req.ndef_len));

#ifdef WFA_WPS_20_NFC_TESTBED
	/* For APSTA as a simulated test device, we should put fake p2p brfore wsc and
	 * the AP should pick the second wsc record.
	*/
	if (strcmp(wps_safe_get_conf("wps_nfc_fake_p2p"), "1") == 0) {
		/* A fake p2p hard-coded WiFi req with Hc as TNF and ac info value */
		/* restore NDEF header which might be updated during previous appending record */
		memcpy(&wifi_hr_rec[NDEF_PAYLOAD_TYPE_OFFSET], p_p2p_type, WFA_P2P_TYPE_LEN);
		p_ndef_header = (UINT8*)wifi_hr_rec;
		*p_ndef_header |= (NDEF_ME_MASK|NDEF_MB_MASK);
		ndef_status = NDEF_MsgAppendRec(nsa_cho_send_req.ndef_data, MAX_NDEF_LENGTH,
			&nsa_cho_send_req.ndef_len, wifi_hr_rec, wifi_hr_rec_len);
		nsa_cho_send_req.ac_info[nsa_cho_send_req.num_ac_info].cps = NFA_CHO_CPS_ACTIVE;
		nsa_cho_send_req.ac_info[nsa_cho_send_req.num_ac_info].num_aux_data = 0;
		nsa_cho_send_req.num_ac_info++;
	}
#endif /* WFA_WPS_20_NFC_TESTBED */

	/* add hard-coded WiFi req with Hc as TNF and ac info value */
	/* restore NDEF header which might be updated during previous appending record */
	memcpy(&wifi_hr_rec[NDEF_PAYLOAD_TYPE_OFFSET], p_wsc_type, WFA_WSC_TYPE_LEN);
	p_ndef_header = (UINT8*)wifi_hr_rec;
	*p_ndef_header |= (NDEF_ME_MASK|NDEF_MB_MASK);
	ndef_status = NDEF_MsgAppendRec(nsa_cho_send_req.ndef_data, MAX_NDEF_LENGTH,
		&nsa_cho_send_req.ndef_len, wifi_hr_rec, wifi_hr_rec_len);
	nsa_cho_send_req.ac_info[nsa_cho_send_req.num_ac_info].cps = NFA_CHO_CPS_ACTIVE;
	nsa_cho_send_req.ac_info[nsa_cho_send_req.num_ac_info].num_aux_data = 0;
	nsa_cho_send_req.num_ac_info++;

#ifdef WFA_WPS_20_NFC_TESTBED
	/* Just in case we need put p2p after wsc in plugfest. */
	if (strcmp(wps_safe_get_conf("wps_nfc_fake_p2p"), "2") == 0) {
		/* A fake p2p hard-coded WiFi req with Hc as TNF and ac info value */
		/* restore NDEF header which might be updated during previous appending record */
		memcpy(&wifi_hr_rec[NDEF_PAYLOAD_TYPE_OFFSET], p_p2p_type, WFA_P2P_TYPE_LEN);
		p_ndef_header = (UINT8*)wifi_hr_rec;
		*p_ndef_header |= (NDEF_ME_MASK|NDEF_MB_MASK);
		ndef_status = NDEF_MsgAppendRec(nsa_cho_send_req.ndef_data, MAX_NDEF_LENGTH,
			&nsa_cho_send_req.ndef_len, wifi_hr_rec, wifi_hr_rec_len);
		nsa_cho_send_req.ac_info[nsa_cho_send_req.num_ac_info].cps = NFA_CHO_CPS_ACTIVE;
		nsa_cho_send_req.ac_info[nsa_cho_send_req.num_ac_info].num_aux_data = 0;
		nsa_cho_send_req.num_ac_info++;
	}
#endif /* WFA_WPS_20_NFC_TESTBED */

	nsa_status = NSA_ChoSendRequest(&nsa_cho_send_req);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_ChoSendRequest failed:%d", nsa_status);
		return nsa_status;
	}

	return 0;
}
