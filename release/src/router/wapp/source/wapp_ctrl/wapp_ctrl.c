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

#include "wapp_ctrl.h"

struct wapp_ctrl *wapp_ctrl_open(const char *ctrl_path)
{
	struct wapp_ctrl *ctrl;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ctrl = os_zalloc(sizeof(*ctrl));

	if (!ctrl) {
		DBGPRINT(RT_DEBUG_ERROR, "memory is not available\n");
		return NULL;
	}

	ctrl->s = socket(PF_UNIX, SOCK_DGRAM, 0);

	if (ctrl->s < 0) {
		os_free(ctrl);
		DBGPRINT(RT_DEBUG_ERROR, "create socket for ctrl interface fail\n");
		return NULL;
	}

	ctrl->local.sun_family = AF_UNIX;
	os_snprintf(ctrl->local.sun_path, sizeof(ctrl->local.sun_path), "/tmp/hsctrl_%d",
					getpid());

	if (bind(ctrl->s, (struct sockaddr *)&ctrl->local, sizeof(ctrl->local)) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Bind local domain address fail\n");
		close(ctrl->s);
		os_free(ctrl);
		return NULL;
	}

	ctrl->dest.sun_family = AF_UNIX;
	os_strncpy(ctrl->dest.sun_path, ctrl_path, sizeof(ctrl->dest.sun_path) - 1);
	ctrl->dest.sun_path[sizeof(ctrl->dest.sun_path) - 1] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\n\nctrl_path: %s\n",ctrl_path);

	if (connect(ctrl->s, (struct sockaddr *)&ctrl->dest, sizeof(ctrl->dest)) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Connect to server address fail\n");	
		unlink(ctrl->local.sun_path);
		close(ctrl->s);
		os_free(ctrl);
		return NULL;
	}
	
	return ctrl;
}

void wapp_ctrl_close(struct wapp_ctrl *ctrl)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	unlink(ctrl->local.sun_path);
	close(ctrl->s);
	os_free(ctrl);
}

int wapp_ctrl_command(struct wapp_ctrl *ctrl, const char *cmd, size_t cmd_len,
							char *reply, size_t *reply_len)
{
	struct timeval tv;
	int ret;
	fd_set rfds;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (send(ctrl->s, cmd, cmd_len, 0) < 0) {
		printf("send command to ctrol socket fail\n");
		return -1;
	}

	for(;;) {
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		FD_ZERO(&rfds);
		FD_SET(ctrl->s, &rfds);
		ret = select(ctrl->s + 1, &rfds, NULL, NULL, &tv);

		if (FD_ISSET(ctrl->s, &rfds)) {
			ret = recv(ctrl->s, reply, *reply_len, 0);
				if (ret < 0)
					return ret;
		
			*reply_len = ret;
			break;

		} else
			return -1;
	}

	return 0;
}

int wapp_ctrl_event_regiser(struct wapp_ctrl *ctrl)
{
	char buf[10];
	int ret;
	size_t len = 10;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(buf, 0, 10);

	ret = wapp_ctrl_command(ctrl, "EVENT_REGISTER", 14, buf, &len);

	if (ret < 0)
		return ret;

	if (len == 3 && os_memcmp(buf, "OK\n", 3) == 0)
		return 0;

	return -1;
}

int wapp_ctrl_event_unregister(struct wapp_ctrl *ctrl)
{
	char buf[10];
	int ret;
	size_t len = 10;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(buf, 0, 10);

	ret = wapp_ctrl_command(ctrl, "EVENT_UNREGISTER", 16, buf, &len);

	if (ret < 0)
		return ret;

	if (len == 3 && os_memcmp(buf, "OK\n", 3) == 0)
		return 0;

	return -1; 
}
