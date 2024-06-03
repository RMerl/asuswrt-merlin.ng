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

#ifndef WAPP_USR_INTF_CTRL_H
#define WAPP_USR_INTF_CTRL_H

/**
 * struct wapp_usr_intf_ctrl - Internal structure for control interface library
 *
 * This structure is used by the daemon control interface
 * library to store internal data. Programs using the library should not touch
 * this data directly. They can only use the pointer to the data structure as
 * an identifier for the control interface connection and use this as one of
 * the arguments for most of the control interface library functions.
 */

struct wapp_usr_intf_ctrl
{
	char* name;
	int s;
	struct sockaddr_un local;
	struct sockaddr_un dest;
};

/**
 * wapp_usr_intf_ctrl_open - Open a control interface to wapp
 * @daemon: current daemon name
 * @local_path: Path for UNIX domain sockets;
 * Returns: Pointer to abstract control interface data or %NULL on failure
 *
 * This function is used to open a control interface to wapp..
 */

struct wapp_usr_intf_ctrl * wapp_usr_intf_ctrl_open(const char* daemon, const char *local_path);

/**
 * wapp_usr_intf_ctrl_close - Close a control interface to wapp
 * @ctrl: Control interface data from wapp_usr_intf_ctrl_open()
 *
 * This function is used to close a control interface
 */

void wapp_usr_intf_ctrl_close(struct wapp_usr_intf_ctrl *ctrl);

/**
 * wpa_ctrl_request - Send a command to wapp
 * @ctrl: Control interface data from wapp_usr_intf_ctrl_open()
 * @cmd: the cmd structure pointer
 * @cmd_len: Length of the cmd in bytes
 * Returns: -1 on error (send), 0 on success
 *
 * This function is used to send commands to wapp
 */
int wapp_usr_intf_ctrl_request(struct wapp_usr_intf_ctrl *ctrl, const char *cmd, size_t cmd_len);

/**
 * wapp_usr_intf_ctrl_recv - Receive a pending control interface message
 * @ctrl: Control interface data from wapp_usr_intf_ctrl_open()
 * @reply: Buffer for the message data
 * @reply_len: Length of the reply buffer
 * Returns: 0 on success, -1 on failure
 *
 * This function will receive a pending control interface message. The received
 * response will be written to reply and reply_len is set to the actual length
 * of the reply.

 * wapp_usr_intf_ctrl_recv() is only used for event messages, i.e., wapp_usr_intf_ctrl_attach()
 * must have been used to register the control interface as an event monitor.
 */
int wapp_usr_intf_ctrl_recv(struct wapp_usr_intf_ctrl *ctrl, char *reply, size_t *reply_len);

/**
 * wapp_usr_intf_ctrl_pending - Check whether there are pending event messages
 * @ctrl: Control interface data from wapp_usr_intf_ctrl_open()
 * @tv: pending time, if tv equal to NULL, this function will block until any message received
 * Returns: 1 if there are pending messages, 0 if timeout, or -1 on error
 *
 * This function will check whether there are any pending control interface
 * message available to be received with wapp_usr_intf_ctrl_recv(). wapp_usr_intf_ctrl_pending() is
 * only used for event messages, i.e., wapp_usr_intf_ctrl_attach() must have been used to
 * register the control interface as an event monitor.
 */
int wapp_usr_intf_ctrl_pending(struct wapp_usr_intf_ctrl *ctrl, struct timeval *tv);

/**
 * wapp_usr_intf_ctrl_get_fd - Get file descriptor used by the control interface
 * @ctrl: Control interface data from wapp_usr_intf_ctrl_open()
 * Returns: File descriptor used for the connection
 *
 * This function can be used to get the file descriptor that is used for the
 * control interface connection. The returned value can be used, e.g., with
 * select() while waiting for multiple events.
 *
 * The returned file descriptor must not be used directly for sending or
 * receiving packets; instead, the library functions wapp_usr_intf_ctrl_request() and
 * wapp_usr_intf_ctrl_recv() must be used for this.
 */
int wapp_usr_intf_ctrl_get_fd(struct wapp_usr_intf_ctrl *ctrl);

#endif
