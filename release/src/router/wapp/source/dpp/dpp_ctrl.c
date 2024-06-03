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

#include "os.h"
#include "util.h"
#include "eloop.h"
#include "wapp_cmm.h"
#include "dpp.h"
#include "dpp_wdev.h"

int wapp_ctrl_iface_cmd_dpp(struct wifi_app *wapp, char *iface,
                                              char *buf, char *reply,
                                              int reply_size)
{
	struct wapp_dev *wdev = NULL;
	int reply_len = 0, res;

	os_memcpy(reply, "OK\n", 3);
	reply_len = 3;
	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);
	if (!wdev) {
		printf("failed to get wdev");
		return -1;
	}

	if (os_strncmp(buf, "DPP_QR_CODE ", 12) == 0) {
                res = wapp_dpp_qr_code(wapp, buf + 12);
                if (res < 0) {
                        reply_len = -1;
                } else {
                        reply_len = os_snprintf(reply, reply_size, "%d", res);
                        if (os_snprintf_error(reply_size, reply_len))
                                reply_len = -1;
                }
        } else if (os_strncmp(buf, "DPP_BOOTSTRAP_GEN ", 18) == 0) {
                res = dpp_bootstrap_gen(wapp->dpp, buf + 18);
                if (res < 0) {
                        reply_len = -1;
                } else {
                        reply_len = os_snprintf(reply, reply_size, "%d", res);
                        if (os_snprintf_error(reply_size, reply_len))
                                reply_len = -1;
                }
        } else if (os_strncmp(buf, "DPP_BOOTSTRAP_REMOVE ", 21) == 0) {
                if (dpp_bootstrap_remove(wapp->dpp, buf + 21) < 0)
                        reply_len = -1;
        } else if (os_strncmp(buf, "DPP_BOOTSTRAP_GET_URI ", 22) == 0) {
                const char *uri;

                uri = dpp_bootstrap_get_uri(wapp->dpp, atoi(buf + 22));
                if (!uri) {
                        reply_len = -1;
                } else {
                        reply_len = os_snprintf(reply, reply_size, "%s", uri);
                        if (os_snprintf_error(reply_size, reply_len))
                                reply_len = -1;
                }
        } else if (os_strncmp(buf, "DPP_BOOTSTRAP_INFO ", 19) == 0) {
                reply_len = dpp_bootstrap_info(wapp->dpp, atoi(buf + 19),
                                                       reply, reply_size);
        } else if (os_strncmp(buf, "DPP_AUTH_INIT ", 14) == 0) {
                if (wapp_dpp_auth_init(wapp, wdev, buf + 13) < 0)
                        reply_len = -1;
        } else if (os_strncmp(buf, "DPP_LISTEN ", 11) == 0) {
                if (wapp_dpp_listen(wapp, wdev, buf + 11) < 0)
                        reply_len = -1;
        } else if (os_strcmp(buf, "DPP_STOP_LISTEN ") == 0) {
                wapp_dpp_stop(wapp);
                wapp_dpp_listen_stop(wapp, wdev);
        } else if (os_strncmp(buf, "DPP_CONFIGURATOR_ADD", 20) == 0) {
                res = dpp_configurator_add(wapp->dpp, buf + 20);
                if (res < 0) {
                        reply_len = -1;
                } else {
                        reply_len = os_snprintf(reply, reply_size, "%d", res);
                        if (os_snprintf_error(reply_size, reply_len))
                                reply_len = -1;
                }
        } else if (os_strncmp(buf, "DPP_CONFIGURATOR_REMOVE ", 24) == 0) {
                if (dpp_configurator_remove(wapp->dpp, buf + 24) < 0)
                        reply_len = -1;
        } else if (os_strncmp(buf, "DPP_CONFIGURATOR_SIGN ", 22) == 0) {
                if (wapp_dpp_configurator_sign(wapp, wdev, buf + 22) < 0)
                        reply_len = -1;
        } else if (os_strncmp(buf, "DPP_CONFIGURATOR_GET_KEY ", 25) == 0) {
                reply_len = dpp_configurator_get_key_id(wapp->dpp,
                                                             atoi(buf + 25),
                                                             reply, reply_size);
        } else if (os_strncmp(buf, "DPP_PKEX_ADD ", 13) == 0) {
                res = wapp_dpp_pkex_add(wapp, wdev, buf + 12);
                if (res < 0) {
                        reply_len = -1;
                } else {
                        reply_len = os_snprintf(reply, reply_size, "%d", res);
                        if (os_snprintf_error(reply_size, reply_len))
                                reply_len = -1;
                }
	} else if (os_strncmp(buf, "DPP_PKEX_REMOVE ", 16) == 0) {
		if (wapp_dpp_pkex_remove(wapp, buf + 16) < 0)
			reply_len = -1;
#ifdef CONFIG_DPP2
	} else if (os_strncmp(buf, "DPP_CONTROLLER_START", 20) == 0) {
		if (wapp_dpp_controller_start(wapp, NULL) < 0)
			reply_len = -1;
	} else if (os_strcmp(buf, "DPP_CONTROLLER_STOP") == 0) {
		dpp_controller_stop(wapp->dpp);
#endif
        } else {
                os_memcpy(reply, "UNKNOWN COMMAND\n", 16);
                reply_len = 16;
        }

        if (reply_len < 0) {
                os_memcpy(reply, "FAIL\n", 5);
                reply_len = 5;
        }

        return reply_len;
}


