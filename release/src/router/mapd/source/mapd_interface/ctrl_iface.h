/*
 * WPA Supplicant / Control interface (shared code for all backends)
 * Copyright (c) 2004-2015, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */
/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2018, Mediatek, Inc.
 * *
 * * All rights reserved. Mediatek's source code is an unpublished work and the
 * * use of a copyright notice does not imply otherwise. This source code
 * * contains confidential trade secret material of Ralink Tech. Any attemp
 * * or participation in deciphering, decoding, reverse engineering or in any
 * * way altering the source code is stricitly prohibited, unless the prior
 * * written consent of Mediatek, Inc. is obtained.
 * ***************************************************************************
 *
 *  Module Name:
 *  Control interface
 *
 *  Abstract:
 *  Control interface
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Neelansh.M   2018/05/02     Derived from WPA Supplicant / Control interface
 * */

#ifndef CTRL_IFACE_H
#define CTRL_IFACE_H

struct mapd_ctrl_dst {
	struct dl_list list;
	struct sockaddr_un addr;
	socklen_t addrlen;
	int debug_level;
	int errors;
};

struct ctrl_iface_global_priv {
	struct mapd_global *global;
	int sock;
	struct dl_list ctrl_dst;
	struct dl_list msg_queue;
	unsigned int throttle_count;
};

/* Shared functions from ctrl_iface.c; to be called by ctrl_iface backends */


/**
 * mapd_global_ctrl_iface_process - Process global ctrl_iface commapd
 * @global: Pointer to global data from mapd_init()
 * @buf: Received commapd buffer (nul terminated string)
 * @resp_len: Variable to be set to the response length
 * Returns: Response (*resp_len bytes) or %NULL on failure
 *
 * Control interface backends call this function when receiving a message from
 * the global ctrl_iface connection. The return response value is then sent to
 * the external program that sent the commapd. Caller is responsible for
 * freeing the buffer after this. If %NULL is returned, *resp_len can be set to
 * two special values: 1 = send "FAIL\n" response, 2 = send "OK\n" response. If
 * *resp_len has any other value, no response is sent.
 */
char * mapd_global_ctrl_iface_process(struct mapd_global *global,
						char *buf, size_t *resp_len);


/* Functions that each ctrl_iface backend must implement */

/**
 * mapd_global_ctrl_iface_init - Initialize global control interface
 * @global: Pointer to global data from mapd_init()
 * Returns: Pointer to private data on success, %NULL on failure
 *
 * Initialize the global control interface and start receiving commapds from
 * external programs.
 *
 * Required to be implemented in each control interface backend.
 */
struct ctrl_iface_global_priv *
mapd_global_ctrl_iface_init(struct mapd_global *global);

/**
 * mapd_global_ctrl_iface_deinit - Deinitialize global ctrl interface
 * @priv: Pointer to private data from mapd_global_ctrl_iface_init()
 *
 * Deinitialize the global control interface that was initialized with
 * mapd_global_ctrl_iface_init().
 *
 * Required to be implemented in each control interface backend.
 */
void mapd_global_ctrl_iface_deinit(
	struct ctrl_iface_global_priv *priv);


typedef void (*mapd_msg_cb_func)(void *ctx,
				const char *txt, size_t len);

void mapd_msg_ctrl(void *ctx, const char *fmt, ...);
void mapd_ctrl_iface_send(struct mapd_global *global,
					   int sock,
					   struct dl_list *ctrl_dst,
					   const char *buf,
					   size_t len,
					   struct ctrl_iface_global_priv *gp);

#endif /* CTRL_IFACE_H */
