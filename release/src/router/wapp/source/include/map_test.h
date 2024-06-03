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

int cli_assoc_cntrl_req(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int discovery(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int topoquery(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int toponotify(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wps_connect( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int steering_completed(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int btm_report(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int steer_mand(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int steer_oppo(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int steer_policy(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int steer_policy_rssi(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int assoc_cntrl(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int backhaul_steer(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int metrics_all_neighbor(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int metrics_specific_neighbor(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int metrics_query(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int metric_report_policy(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int sta_link_metric_query(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int sta_unlink_metric_query(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int map_policy_config_req(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int ch_prefer_query(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int ch_select_req(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int cli_steer_req(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int operbss(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int high_layer_data(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);

int wapp_high_layer_data_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac);

int wapp_send_toponotify_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *sta_mac, unsigned char *bssid,
	unsigned char assoc_evt, unsigned char req_len);
int wapp_send_policy_config_req_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac);
int wapp_send_ch_prefer_query_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac);
int wapp_send_cli_steer_req_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac);
int wapp_send_ch_select_req_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac);
int wapp_send_topoquery_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac);
int wapp_send_discovery_msg(struct wifi_app *wapp, char *buf, int max_len);
int wapp_send_cli_assoc_cntrl_req_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *almac);

