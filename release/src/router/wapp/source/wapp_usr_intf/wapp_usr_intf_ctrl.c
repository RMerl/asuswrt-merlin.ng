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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stddef.h>
#include "../include/os.h"
#include "wapp_usr_intf_ctrl.h"

#define CMD_SUC_SYNC_EVENT 0
#define CMD_SUC_ASYNC_EVENT 1
#define CMD_PROCESS_FAIL 2

#define WAPP_SERVER_PATH "/tmp/wapp_server"
int wapp_usr_intf_ctrl_attach(struct wapp_usr_intf_ctrl *ctrl, const char* daemon);
int wapp_usr_intf_ctrl_detach(struct wapp_usr_intf_ctrl *ctrl);
struct wapp_usr_intf_ctrl * wapp_usr_intf_ctrl_open(const char* daemon, const char *local_path)
{
	struct wapp_usr_intf_ctrl *ctrl;
	static int counter = 0;
	int nameLen = 0;
	int ret;
	size_t res;
	int tries = 0;
	int flags;
	int path_len = 0;

	ctrl = (struct wapp_usr_intf_ctrl *)os_malloc(sizeof(*ctrl));
	if (ctrl == NULL)
	{
		printf("%s, alloc memory fail\n", __func__);
		return NULL;
	}
	os_memset(ctrl, 0, sizeof(*ctrl));

	ctrl->s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (ctrl->s < 0) {
		os_free(ctrl);
		return NULL;
	}

	ctrl->local.sun_family = AF_UNIX;
	counter++;
try_again:
	nameLen = strlen(local_path);
    if (nameLen >= (int) sizeof(ctrl->local.sun_path) -1) {
		close(ctrl->s);
		os_free(ctrl);
        return NULL;
    }
    ctrl->local.sun_path[0] = '\0';  /* abstract namespace */
    /* in kernel #define UNIX_PATH_MAX	108 */
    snprintf(ctrl->local.sun_path + 1, 108 - 1, "%s", local_path);
    path_len = 1 + nameLen + offsetof(struct sockaddr_un, sun_path);
	tries++;
	if (bind(ctrl->s, (struct sockaddr *) &ctrl->local,
		    path_len) < 0) {
		if (errno == EADDRINUSE && tries < 2) {
			/*
			 * getpid() returns unique identifier for this instance
			 * of wpa_ctrl, so the existing socket file must have
			 * been left by unclean termination of an earlier run.
			 * Remove the file and try again.
			 */
			/*for abstract socket path, no need to unlink it*/
//			unlink(ctrl->local.sun_path);
			goto try_again;
		}
		close(ctrl->s);
		os_free(ctrl);
		printf("%s, bind fail\n", __func__);
		return NULL;
	}

	ctrl->dest.sun_family = AF_UNIX;
	nameLen = strlen(WAPP_SERVER_PATH);
    if (nameLen >= (int) sizeof(ctrl->dest.sun_path) -1) {
		close(ctrl->s);
		os_free(ctrl);
		return NULL;
    }
	snprintf(ctrl->dest.sun_path, sizeof(ctrl->dest.sun_path), "%s", WAPP_SERVER_PATH);
	path_len = nameLen + offsetof(struct sockaddr_un, sun_path);
	if (connect(ctrl->s, (struct sockaddr *) &ctrl->dest,
		    path_len) < 0) {
		close(ctrl->s);
		/*for abstract socket path, no need to unlink it*/
//		unlink(ctrl->local.sun_path);
		os_free(ctrl);
		printf("%s, connect fail, %s\n", __func__, strerror(errno));
		return NULL;
	}

	/*
	 * Make socket non-blocking so that we don't hang forever if
	 * target dies unexpectedly.
	 */
	flags = fcntl(ctrl->s, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(ctrl->s, F_SETFL, flags) < 0) {
			perror("fcntl(ctrl->s, O_NONBLOCK)");
			/* Not fatal, continue on.*/
		}
	}

	if(wapp_usr_intf_ctrl_attach(ctrl, daemon))
	{
		printf("attach failed\n");
		close(ctrl->s);
		os_free(ctrl);
		return NULL;
	}
	return ctrl;
}

void wapp_usr_intf_ctrl_close(struct wapp_usr_intf_ctrl *ctrl)
{
	if (ctrl == NULL)
		return;
	if(wapp_usr_intf_ctrl_detach(ctrl))
	{
		printf("detach failed\n");
	}
//	unlink(ctrl->local.sun_path);
	if (ctrl->s >= 0)
		close(ctrl->s);
	os_free(ctrl);
}

int wapp_usr_intf_ctrl_request(struct wapp_usr_intf_ctrl *ctrl, const char *cmd, size_t cmd_len)
{
	struct timeval tv;
	struct os_time started_at;
	int res;
	fd_set rfds;
	const char *_cmd;
	size_t _cmd_len;

	_cmd = cmd;
	_cmd_len = cmd_len;
	started_at.sec = 0;
	started_at.usec = 0;
retry_send:
	if (send(ctrl->s, _cmd, _cmd_len, 0) < 0) {
		if (errno == EAGAIN || errno == EBUSY || errno == EWOULDBLOCK) {
			/*
			 * Must be a non-blocking socket... Try for a bit
			 * longer before giving up.
			 */
			if (started_at.sec == 0) {
				os_get_time(&started_at);
			} else {
				struct os_time n;
				os_get_time(&n);
				/* Try for a few seconds. */
				if (os_reltime_expired(&n, &started_at, 5)) {
					printf("%s: send failed: %d - %s\n", __func__, errno, strerror(errno));
					return -2;
				}
			}
			os_sleep(0, 10000);
			goto retry_send;
		}
		printf("%s: send failed: %d - %s\n", __func__, errno, strerror(errno));
		return -1;
	}
	return 0;
}

int wapp_usr_intf_ctrl_pending(struct wapp_usr_intf_ctrl *ctrl, struct timeval *tv);

int wapp_usr_intf_ctrl_attach_helper(struct wapp_usr_intf_ctrl *ctrl, int attach, const char* daemon)
{
	char buf[10];
	int ret;
	size_t len = 10;
	struct timeval tv;
	char request_buf[64];

	os_memset(request_buf, 0, sizeof(request_buf));
	os_memset(buf, 0, sizeof(buf));
	os_snprintf(request_buf, sizeof(request_buf), attach ? "ATTACH:%s" : "DETACH:%s", daemon);
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	ret = wapp_usr_intf_ctrl_request(ctrl, request_buf, os_strlen(request_buf));
	if (ret < 0)
		return ret;
	if(wapp_usr_intf_ctrl_pending(ctrl, &tv))
	{
		if(wapp_usr_intf_ctrl_recv(ctrl, buf, &len) < 0)
		{
			return -1;
		}
	}
	if (len == 3 && os_memcmp(buf, "OK\n", 3) == 0)
		return 0;
	buf[9] = '\0';
	printf("buf : %s\n", buf);
	return -1;
}


int wapp_usr_intf_ctrl_attach(struct wapp_usr_intf_ctrl *ctrl, const char* daemon)
{
	return wapp_usr_intf_ctrl_attach_helper(ctrl, 1, daemon);
}


int wapp_usr_intf_ctrl_detach(struct wapp_usr_intf_ctrl *ctrl)
{
	return wapp_usr_intf_ctrl_attach_helper(ctrl, 0, "");
}

int wapp_usr_intf_ctrl_recv(struct wapp_usr_intf_ctrl *ctrl, char *reply, size_t *reply_len)
{
	int res;

	res = recv(ctrl->s, reply, *reply_len, 0);
	if (res < 0)
		return res;
	*reply_len = res;
	return 0;
}


int wapp_usr_intf_ctrl_pending(struct wapp_usr_intf_ctrl *ctrl, struct timeval *tv)
{
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(ctrl->s, &rfds);
	select(ctrl->s + 1, &rfds, NULL, NULL, tv);
	return FD_ISSET(ctrl->s, &rfds);
}


int wapp_usr_intf_ctrl_get_fd(struct wapp_usr_intf_ctrl *ctrl)
{
	return ctrl->s;
}


