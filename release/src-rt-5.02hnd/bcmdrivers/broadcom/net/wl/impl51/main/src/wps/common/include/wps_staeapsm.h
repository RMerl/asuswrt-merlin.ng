/*
 * WPS sta eap step machine
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wps_staeapsm.h 241182 2011-02-17 21:50:03Z $
 */

/* type of wireless STA : enrollee or registrar  */
#define WPS_MSGTYPE_OFFSET  9

/* internal API  */
int wps_eap_sta_init(char *bssid, void *handle, int e_mode);
int wps_eap_enr_create_response(char * dataBuffer, int dataLen, uint8 eapCode);
int wps_process_ap_msg(char *eapol_msg, int len);
