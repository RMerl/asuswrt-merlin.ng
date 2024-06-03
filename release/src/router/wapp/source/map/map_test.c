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
#include "map_1905.h"
#include "interface.h"


int wapp_send_cli_assoc_cntrl_req_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac)
{
	struct evt *wapp_event;
	int send_pkt_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_CLI_ASSOC_CNTRL_REQUEST;
	wapp_event->length = 6;
	memcpy(wapp_event->buffer, almac, 6);
	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;

	if(0 > map_1905_send(wapp, buf, send_pkt_len)) {
		printf("send cli_assoc_cntrl_req msg fail\n");
		return -1;
	}

	memset(buf, 0, send_pkt_len);
	return 0;
}

int cli_assoc_cntrl_req(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	char buf[256];
	unsigned char almac[6] = {0};
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}


	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_send_cli_assoc_cntrl_req_msg(wapp, buf, 256, almac);

	return WAPP_SUCCESS;
}

int discovery(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 1) {
		printf("%s: cmd parameter error! need 1 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x0000;
	pcmdu_req->len = 0;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req);
	if(0 > map_1905_send(wapp, (char *)buf, send_pkt_len)) {
		printf("send discovery msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int topoquery(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x0002;
	pcmdu_req->len = 0;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req);
	if(0 > map_1905_send(wapp, (char *)buf, send_pkt_len)) {
		printf("send topology query msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int toponotify(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 1) {
		printf("%s: cmd parameter error! need 1 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x0001;
	pcmdu_req->len = 0;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req);
	if(0 > map_1905_send(wapp, (char *)buf, send_pkt_len)) {
		printf("send notification msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int wps_connect( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	//char buf[256];
	//wapp_send_wps_connect_msg(NULL, buf, 256);
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	return WAPP_SUCCESS;
}

int steering_completed(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
//	char buf[256];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 1) {
		printf("%s: cmd parameter error! need 1 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}

//	wapp_send_steering_completed_msg(wapp, buf, 256);

	return WAPP_SUCCESS;
}

int btm_report(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	char buf[256];
	struct cli_steer_btm_event btm_evt;
	unsigned char bssid[6] = {0x00, 0x0c, 0x43, 0x26, 0x60, 0x98};
	unsigned char tbssid[6] = {0};
	unsigned char sta[6] = {0x38, 0xbc, 0x1a, 0xc1, 0xd3, 0x40};
	unsigned char status = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 1) {
		printf("%s: cmd parameter error! need 1 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}

	memset(&btm_evt, 0, sizeof(struct cli_steer_btm_event));
	memcpy(btm_evt.bssid, bssid, 6);
	memcpy(btm_evt.tbssid, tbssid, 6);
	memcpy(btm_evt.sta_mac, sta, 6);
	btm_evt.status = status;

	wapp_send_cli_steer_btm_report_msg(wapp, buf, 256, &btm_evt);

	return WAPP_SUCCESS;
}

int steer_mand(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};
	unsigned char *pos = NULL, *ptlvlen = NULL;	
	unsigned char bssid[6] = {0x00, 0x00, 0x43, 0x26, 0x60, 0x98};
	unsigned char tbssid[6] = {0x00, 0x00, 0x43, 0x26, 0x60, 0x99};
	unsigned char sta[6] = {0x38, 0xbc, 0x1a, 0xc1, 0xd3, 0x40};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x8014;
	pos = pcmdu_req->body;
	/*tlv type*/
	*pos = 0x9B;
	pos += 1;
	/*tlv length*/
	ptlvlen = pos;
	pos += 2;
	/*bssid*/
	memcpy(pos, bssid, 6);
	pos += 6;
	len += 6;
	/**/
	*pos = 0xe0;
	pos += 1;
	len += 1;
	/*disassociation timer*/
	*pos = 0x00;
	*(pos+1) = 0x05;
	pos += 2;
	len += 2;
	/*sta count*/
	*pos = 0x01;
	pos += 1;
	len += 1;
	/*sta mac*/
	memcpy(pos, sta, 6);
	pos += 6;
	len += 6;
	/*target bssid cnt*/
	*pos = 0x01;
	pos += 1;
	len += 1;
	/*target bssid*/
	memcpy(pos, tbssid, 6);
	pos += 6;
	len += 6;
	/*oper class*/
	*pos = 115;
	pos += 1;
	len += 1;
	/*channel*/
	*pos = 36;
	pos += 1;
	len += 1;

	*ptlvlen = (len >> 8) & 0xff;
	*(ptlvlen+1) = len & 0xff;
	
	pcmdu_req->len = len + 3;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req) + pcmdu_req->len;
	if(0 > map_1905_send(wapp, (char *)buf, send_pkt_len)) {
		printf("send steer mand msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int steer_oppo(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};
	unsigned char *pos = NULL, *ptlvlen = NULL;	
	unsigned char bssid[6] = {0x00, 0x00, 0x43, 0x26, 0x60, 0x98};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x8014;
	pos = pcmdu_req->body;
	/*tlv type*/
	*pos = 0x9B;
	pos += 1;
	/*tlv length*/
	ptlvlen = pos;
	pos += 2;
	/*bssid*/
	memcpy(pos, bssid, 6);
	pos += 6;
	len += 6;
	/**/
	*pos = 0x00;
	pos += 1;
	len += 1;
	/*steering opportunity window*/
	*pos = 0x00;
	*(pos+1) = 0x0A;
	pos += 2;
	len += 2;
	/*sta count*/
	*pos = 0x00;
	pos += 1;
	len += 1;

	*ptlvlen = (len >> 8) & 0xff;
	*(ptlvlen+1) = len & 0xff;
	
	pcmdu_req->len = len + 3;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req) + pcmdu_req->len;
	if(0 > map_1905_send(wapp, (char *)buf, send_pkt_len)) {
		printf("send steer oppo msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int steer_policy(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};
	unsigned char *pos = NULL, *ptlvlen = NULL;	
	unsigned char rid[6] = {0};
	unsigned char sta[6] = {0x38, 0xbc, 0x1a, 0xc1, 0xd3, 0x40};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x8003;
	pos = pcmdu_req->body;
	/*tlv type*/
	*pos = 0x89;
	pos += 1;
	/*tlv length*/
	ptlvlen = pos;
	pos += 2;
	/*local steering disallowed sta count*/
	*pos = 0x00;
	pos += 1;
	len += 1;
	/*btm steering disallowed sta count.*/
	*pos = 0x01;
	pos += 1;
	len += 1;
	/*sta mac*/
	memcpy(pos, sta, 6);
	pos += 6;
	len += 6;
	/*number of radios*/
	*pos = 0x01;
	pos += 1;
	len += 1;
	/*radio unique identifier*/
	memcpy(pos, rid, 6);
	pos += 6;
	len += 6;
	/*policy*/
	*pos = 0x00;
	pos += 1;
	len += 1;
	/*channel utilization threshold*/
	*pos = 0xff;
	pos += 1;
	len += 1;
	/*rssi steering threshold*/
	*pos = 0x20;
	pos += 1;
	len += 1;

	*ptlvlen = (len >> 8) & 0xff;
	*(ptlvlen+1) = len & 0xff;
	
	pcmdu_req->len = len + 3;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req) + pcmdu_req->len;
	if(0 > map_1905_send(wapp, (char *)buf, send_pkt_len)) {
		printf("send steer mand msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int steer_policy_rssi(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};
	unsigned char *pos = NULL, *ptlvlen = NULL;	
	unsigned char rid[6] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x8003;
	pos = pcmdu_req->body;
	/*tlv type*/
	*pos = 0x89;
	pos += 1;
	/*tlv length*/
	ptlvlen = pos;
	pos += 2;
	/*local steering disallowed sta count*/
	*pos = 0x00;
	pos += 1;
	len += 1;
	/*btm steering disallowed sta count.*/
	*pos = 0x00;
	pos += 1;
	len += 1;
	/*number of radios*/
	*pos = 0x01;
	pos += 1;
	len += 1;
	/*radio unique identifier*/
	memcpy(pos, rid, 6);
	pos += 6;
	len += 6;
	/*policy*/
	*pos = 0x01;
	pos += 1;
	len += 1;
	/*channel utilization threshold*/
	*pos = 0xff;
	pos += 1;
	len += 1;
	/*rssi steering threshold*/
	*pos = 0x20;
	pos += 1;
	len += 1;

	*ptlvlen = (len >> 8) & 0xff;
	*(ptlvlen+1) = len & 0xff;
	
	pcmdu_req->len = len + 3;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req) + pcmdu_req->len;
	if(0 > map_1905_send(wapp, (char *)buf, send_pkt_len)) {
		printf("send steer mand msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int assoc_cntrl(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};
	unsigned char *pos = NULL, *ptlvlen = NULL;	
	unsigned char bssid[6] = {0x00, 0x00, 0x43, 0x26, 0x60, 0x98};
	unsigned char sta[6] = {0x38, 0xbc, 0x1a, 0xc1, 0xd3, 0x40};
	unsigned char control = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 3 || !argv[1] || !argv[2]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	control = atoi(argv[2]);
	printf("%s\n",control ? "block the association" : "unblock the association");

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x8016;
	pos = pcmdu_req->body;
	/*tlv type*/
	*pos = 0x9D;
	pos += 1;
	/*tlv length*/
	ptlvlen = pos;
	pos += 2;
	/*bssid*/
	memcpy(pos, bssid, 6);
	pos += 6;
	len += 6;
	/*association control*/
	*pos = control;
	pos += 1;
	len += 1;
	/*validity period*/
	*pos = 0x00;
	*(pos+1) = 0x30;
	pos += 2;
	len += 2;
	/*sta count*/
	*pos = 0x01;
	pos += 1;
	len += 1;
	/*sta mac*/
	memcpy(pos, sta, 6);
	pos += 6;
	len += 6;
	
	*ptlvlen = (len >> 8) & 0xff;
	*(ptlvlen+1) = len & 0xff;
	
	pcmdu_req->len = len + 3;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req) + pcmdu_req->len;
	if(0 > map_1905_send(wapp, (char *)buf, send_pkt_len)) {
		printf("send assoc cntrl block msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int backhaul_steer(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};
	unsigned char *pos = NULL, *ptlvlen = NULL;	
	unsigned char bssid[6] = {0x00, 0x00, 0x43, 0x26, 0x60, 0x98};
	unsigned char bsta[6] = {0x38, 0xbc, 0x1a, 0xc1, 0xd3, 0x40};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x8019;
	pos = pcmdu_req->body;
	/*tlv type*/
	*pos = 0x9E;
	pos += 1;
	/*tlv length*/
	ptlvlen = pos;
	pos += 2;
	/*bssid*/
	memcpy(pos, bsta, 6);
	pos += 6;
	len += 6;
	/*sta mac*/
	memcpy(pos, bssid, 6);
	pos += 6;
	len += 6;
	/*oper class*/
	*pos = 115;
	pos += 1;
	len += 1;
	/*channel*/
	*pos = 36;
	pos += 1;
	len += 1;
	
	*ptlvlen = 0x00;
	*(ptlvlen+1) = 0x0D;
	
	pcmdu_req->len = len + 3;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req) + pcmdu_req->len;
	if(0 > map_1905_send(wapp, (char *)buf, send_pkt_len)) {
		printf("send backhaul steer msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int metrics_all_neighbor(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};
	unsigned char *pos = NULL, *ptlvlen = NULL;	

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x0005;
	pos = pcmdu_req->body;
	/*tlv type*/
	*pos = 0x08;
	pos += 1;
	/*tlv length*/
	ptlvlen = pos;
	pos += 2;
	/*neighbor*/
	*pos = 0x00;
	pos += 1;
	len += 1;
	/*direction*/
	*pos = 0x02;
	pos += 1;
	len += 1;
	
	*ptlvlen = 0x00;
	*(ptlvlen+1) = 0x02;
	
	pcmdu_req->len = len + 3;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req) + pcmdu_req->len;
	if(0 > map_1905_send_controller(wapp, (char *)buf, send_pkt_len)) {
		printf("send metrics all neighbor msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int metrics_specific_neighbor(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};
	unsigned char *pos = NULL, *ptlvlen = NULL;	
	unsigned char neighbor[6] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 3 || !argv[1] || !argv[2]) {
		printf("%s: cmd parameter error! need 3 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	/*parse parameters*/
	ret = hwaddr_aton(argv[2],neighbor);
	if(ret) {
		printf("%s: incorrect neighbor almac address %s\n", __func__, argv[2]);
		return WAPP_INVALID_ARG;
	}
	printf("neighbor almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(neighbor));

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x0005;
	pos = pcmdu_req->body;
	/*tlv type*/
	*pos = 0x08;
	pos += 1;
	/*tlv length*/
	ptlvlen = pos;
	pos += 2;
	/*neighbor*/
	*pos = 0x01;
	pos += 1;
	len += 1;
	/*neighbor almac*/
	memcpy(pos, neighbor, 6);
	pos += 6;
	len += 6;
	/*direction*/
	*pos = 0x02;
	pos += 1;
	len += 1;
	
	*ptlvlen = 0x00;
	*(ptlvlen+1) = 0x08;
	
	pcmdu_req->len = len + 3;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req) + pcmdu_req->len;
	if(0 > map_1905_send_controller(wapp, (char *)buf, send_pkt_len)) {
		printf("send metrics specific neighbor msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int metrics_query(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};
	unsigned char *pos = NULL, *ptlvlen = NULL;	
	unsigned char bssid[6] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 3 || !argv[1] || !argv[2]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	/*parse parameters*/
	ret = hwaddr_aton(argv[2],bssid);
	if(ret) {
		printf("%s: incorrect bssid %s\n", __func__, argv[2]);
		return WAPP_INVALID_ARG;
	}
	printf("bssid %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(bssid));

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x800B;
	pos = pcmdu_req->body;
	/*tlv type*/
	*pos = 0x93;
	pos += 1;
	/*tlv length*/
	ptlvlen = pos;
	pos += 2;
	/*neighbor almac*/
	memcpy(pos, bssid, 6);
	pos += 6;
	len += 6;
	
	*ptlvlen = 0x00;
	*(ptlvlen+1) = 0x06;
	
	pcmdu_req->len = len + 3;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req) + pcmdu_req->len;
	if(0 > map_1905_send_controller(wapp, (char *)buf, send_pkt_len)) {
		printf("send metrics query msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int metric_report_policy(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};
	unsigned char *pos = NULL, *ptlvlen = NULL;
	unsigned char interval = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 3 || !argv[1] || !argv[2]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	interval = atoi(argv[2]);	
	printf("ap metrics reporting interval=%d\n",interval);
	
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x8003;
	pos = pcmdu_req->body;
	/*tlv type*/
	*pos = 0x8A;
	pos += 1;
	/*tlv length*/
	ptlvlen = pos;
	pos += 2;
	/*reporting interval*/
	*pos = interval;
	pos += 1;
	len += 1;
	/*number of radios*/
	*pos = 0x00;
	pos += 1;
	len += 1;

	*ptlvlen = 0x00;
	*(ptlvlen+1) = 0x03;
	
	pcmdu_req->len = len + 3;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req) + pcmdu_req->len;
	if(0 > map_1905_send_controller(wapp, (char *)buf, send_pkt_len)) {
		printf("send metric policy msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int sta_link_metric_query(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};
	unsigned char *pos = NULL, *ptlvlen = NULL;
	unsigned char sta[6] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 3 || !argv[1] || !argv[2]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	/*parse parameters*/
	ret = hwaddr_aton(argv[2],sta);
	if(ret) {
		printf("%s: incorrect sta mac address %s\n", __func__, argv[2]);
		return WAPP_INVALID_ARG;
	}
	printf("sta mac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(sta));
	
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x800D;
	pos = pcmdu_req->body;
	/*tlv type*/
	*pos = 0x95;
	pos += 1;
	/*tlv length*/
	ptlvlen = pos;
	pos += 2;
	/*sta mac*/
	memcpy(pos, sta, 6);
	pos += 6;
	len += 6;

	*ptlvlen = 0x00;
	*(ptlvlen+1) = 0x06;
	
	pcmdu_req->len = len + 3;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req) + pcmdu_req->len;
	if(0 > map_1905_send_controller(wapp, (char *)buf, send_pkt_len)) {
		printf("send sta link metric query msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int sta_unlink_metric_query(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	unsigned char buf[128] = {0};
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0, len = 0, ret = 0;
	struct _1905_cmdu_request* pcmdu_req = NULL;	
	unsigned char almac[6] = {0};
	unsigned char *pos = NULL, *ptlvlen = NULL;
	unsigned char sta[6] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 3 || !argv[1] || !argv[2]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	/*parse parameters*/
	ret = hwaddr_aton(argv[2],sta);
	if(ret) {
		printf("%s: incorrect sta mac address %s\n", __func__, argv[2]);
		return WAPP_INVALID_ARG;
	}
	printf("sta mac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(sta));
	
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	pcmdu_req->type = 0x800f;
	pos = pcmdu_req->body;
	/*tlv type*/
	*pos = 0x97;
	pos += 1;
	/*tlv length*/
	ptlvlen = pos;
	pos += 2;
	/*opclass*/
	*pos = 115;
	pos += 1;
	len += 1;
	/*number of channel*/
	*pos = 1;
	pos += 1;
	len += 1;
	/*channel list*/
	*pos = 36;
	pos += 1;
	len += 1;
	/*number of sta*/
	*pos = 1;
	pos += 1;
	len += 1;
	/*sta list*/
	memcpy(pos, sta, 6);
	pos += 6;
	len += 6;

	*ptlvlen = (len >> 8) & 0xff;
	*(ptlvlen+1) = len & 0xff;
	
	pcmdu_req->len = len + 3;
	send_pkt_len = sizeof(*wapp_event) + sizeof(*pcmdu_req) + pcmdu_req->len;
	if(0 > map_1905_send_controller(wapp, (char *)buf, send_pkt_len)) {
		printf("send sta unlink metric query msg fail\n");
		return -1;
	}

	return WAPP_SUCCESS;
}

int wapp_send_discovery_msg(struct wifi_app *wapp, char *buf, int max_len)
{
	struct evt *wapp_event;
	int send_pkt_len = 0;
	unsigned char almac[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x13};
	struct _1905_cmdu_request* pcmdu_req;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	

	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	pcmdu_req->type = 0x0000;   //discovery
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);

	pcmdu_req->len = 0;
	wapp_event->length = sizeof(struct _1905_cmdu_request) + pcmdu_req->len;
		
	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;

	if(0 > map_1905_send(wapp, buf, send_pkt_len)) {
		printf("send wapp_send_discovery_msg fail\n");
		return -1;
	}
	memset(buf, 0, send_pkt_len);
	return 0;
}

int wapp_send_topoquery_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac)
{
	struct evt *wapp_event;
	int send_pkt_len = 0;
	struct _1905_cmdu_request* pcmdu_req;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;

	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	pcmdu_req->type = 0x0002;   //query
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);

	pcmdu_req->len = 0;
	wapp_event->length = sizeof(struct _1905_cmdu_request) + pcmdu_req->len;

	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;

	if(0 > map_1905_send(wapp, buf, send_pkt_len)) {
		printf("send wapp_send_topoquery_msg fail\n");
		return -1;
	}
	memset(buf, 0, send_pkt_len);
	return 0;
}

int wapp_send_ch_select_req_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac)
{
	struct evt *wapp_event;
	int send_pkt_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_CH_SELECTION_REQUEST;
	wapp_event->length = 6;
	memcpy(wapp_event->buffer, almac, 6);
	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;

	if(0 > map_1905_send(wapp, buf, send_pkt_len)) {
		printf("send ch_select_req msg fail\n");
		return -1;
	}

	memset(buf, 0, send_pkt_len);
	return 0;
}

int wapp_send_cli_steer_req_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac)
{
	struct evt *wapp_event;
	int send_pkt_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_CLI_STEER_REQUEST;
	wapp_event->length = 6;
	memcpy(wapp_event->buffer, almac, 6);
	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;

	if(0 > map_1905_send(wapp, buf, send_pkt_len)) {
		printf("send wapp_send_cli_steer_req_msg fail\n");
		return -1;
	}

	memset(buf, 0, send_pkt_len);
	return 0;
}

int wapp_send_ch_prefer_query_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac)
{
	struct evt *wapp_event;
	int send_pkt_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_CH_PREFER_QUERY;
	wapp_event->length = 6;
	memcpy(wapp_event->buffer, almac, 6);
	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;

	if(0 > map_1905_send(wapp, buf, send_pkt_len)) {
		printf("send wapp_send_ch_prefer_query_msg msg fail\n");
		return -1;
	}

	memset(buf, 0, send_pkt_len);
	return 0;
}

int wapp_send_policy_config_req_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac)
{
	struct evt *wapp_event;
	int send_pkt_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_POLICY_CONFIG_REQUEST;
	wapp_event->length = 6;
	memcpy(wapp_event->buffer, almac, 6);
	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;

	if(0 > map_1905_send(wapp, buf, send_pkt_len)) {
		printf("send policy_config_req msg fail\n");
		return -1;
	}

	memset(buf, 0, send_pkt_len);
	return 0;
}

int wapp_send_toponotify_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *sta_mac, unsigned char *bssid,
	unsigned char assoc_evt, unsigned char req_len)
{
	struct evt *wapp_event;	
	struct client_association_event *cli_assoc_evt = NULL;
	int send_pkt_len = 0;
	unsigned char i = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_CLIENT_NOTIFICATION;
	wapp_event->length = sizeof(struct client_association_event) + req_len;

	cli_assoc_evt = (struct client_association_event *)wapp_event->buffer;
	memcpy(cli_assoc_evt->map_assoc_evt.sta_mac, sta_mac, 6);
	memcpy(cli_assoc_evt->map_assoc_evt.bssid, bssid, 6);
	cli_assoc_evt->map_assoc_evt.assoc_evt = assoc_evt;
	cli_assoc_evt->map_assoc_evt.assoc_time = 0;
	cli_assoc_evt->map_assoc_evt.assoc_req_len = req_len;
	
	for (i = 0; i < req_len; i++) {
		memcpy(cli_assoc_evt->map_assoc_evt.assoc_req + i, &i, 1);
	}

	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;

	hex_dump("wapp_send_toponotify_msg", wapp_event->buffer, wapp_event->length);

	if(0 > map_1905_send(wapp, buf, send_pkt_len)) {
		printf("send topology notification msg fail\n");
		return -1;
	}

	memset(buf, 0, send_pkt_len);
	return 0;
}

int wapp_high_layer_data_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac)
{
	struct evt *wapp_event;
	int send_pkt_len = 0;
	struct _1905_cmdu_request* pcmdu_req;
	int i = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_1905_CMDU_REQUEST;
	

	pcmdu_req = (struct _1905_cmdu_request*)wapp_event->buffer;
	pcmdu_req->type = 0x8018;   //high layer data
	memcpy(pcmdu_req->dest_al_mac, almac, ETH_ALEN);
	*pcmdu_req->body = 0x00;
	pcmdu_req->len += 1;
	for(i = 0; i < 200; i++)
	{
		memcpy(pcmdu_req->body + 1 + i * ETH_ALEN, almac, ETH_ALEN);
		pcmdu_req->len += 6;
	}

	wapp_event->length = sizeof(struct _1905_cmdu_request) + pcmdu_req->len;
	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;
	if(0 > map_1905_send(wapp, buf, send_pkt_len)) {
		printf("send wapp_high_layer_data_msg fail\n");
		return -1;
	}
	memset(buf, 0, send_pkt_len);
	return 0;
}

int map_policy_config_req(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	char buf[256];
	unsigned char almac[6] = {0};
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}


	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_send_policy_config_req_msg(wapp, buf, 256, almac);

	return WAPP_SUCCESS;
}

int ch_prefer_query(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	char buf[256];
	unsigned char almac[6] = {0};
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}


	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_send_ch_prefer_query_msg(wapp, buf, 256, almac);

	return WAPP_SUCCESS;
}

int ch_select_req(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	char buf[256];
	unsigned char almac[6] = {0};
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}

	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_send_ch_select_req_msg(wapp, buf, 256, almac);

	return WAPP_SUCCESS;
}

int cli_steer_req(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	char buf[256];
	unsigned char almac[6] = {0};
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}

	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_send_cli_steer_req_msg(wapp, buf, 256, almac);

	return WAPP_SUCCESS;
}

int operbss(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	char buf[256];
	unsigned char identifier[6] = {0};
	unsigned char bssid[16][6];
	unsigned char *ssid[16];
	int ret = 0, i = 0, j = 0;
	char *this_char = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 4 || !argv[1] || !argv[2] || !argv[3]) {
		printf("%s: cmd parameter error! need 4 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}

	/*parse parameters*/
	ret = hwaddr_aton(argv[1],identifier);
	if(ret) {
		printf("%s: incorrect identifier %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("identifier %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(identifier));

	printf("bssid: \n");
	while ((this_char = strsep((char **)&argv[2], ",")) != NULL) {
		ret = hwaddr_aton(this_char, bssid[i]);
		if(ret) {
			printf("incorrect current bssid address %s\n", this_char);
			return WAPP_INVALID_ARG;
		}
		printf("index=%d, bssid %02x:%02x:%02x:%02x:%02x:%02x\n", i, PRINT_MAC(bssid[i]));
		i++;
	}

	printf("ssid: \n");
	while ((this_char = strsep((char **)&argv[3], ",")) != NULL) {
		ssid[j] = (unsigned char *)this_char;
		printf("index=%d, ssid(%s)\n", j, ssid[j]);
		j++;
	}

	if (i != j) {
		printf("bssid&ssid mismatch\n");
		return WAPP_INVALID_ARG;
	}

	wapp_send_operbss_msg(wapp, buf, 256, identifier, bssid, ssid, i);

	return WAPP_SUCCESS;
}

int high_layer_data(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	char buf[MAX_EVT_BUF_LEN];
	unsigned char almac[6] = {0};
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (argc != 2 || !argv[1]) {
		printf("%s: cmd parameter error! need 2 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}

	/*parse parameters*/
	ret = hwaddr_aton(argv[1],almac);
	if(ret) {
		printf("%s: incorrect almac address %s\n", __func__, argv[1]);
		return WAPP_INVALID_ARG;
	}
	printf("almac address %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(almac));

	wapp_high_layer_data_msg(wapp, buf, MAX_EVT_BUF_LEN, almac);

	return WAPP_SUCCESS;
}


