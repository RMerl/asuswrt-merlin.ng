/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#ifndef _ITC_CHANNEL_STRUCTS_H_
#define _ITC_CHANNEL_STRUCTS_H_

#include "itc_rpc.h"
#include "itc_msg_q.h"
#include "dqm_lite.h"

/* service and FIFO tunnel structures */

/* services */
typedef struct
{
	const char		*name;
	rpc_function		*func_tab;
	int			func_tab_sz;
	rpc_queue		*orphan_queue;
	bool			active;
	bool			registered;
	uint32_t		req_count;
} rpc_service;

/* FIFO tunnels */
typedef struct
{
	char			name[16];
	bool			initialized;
	int			idx;
	char			*tx_fifo;
	char			*rx_fifo;
	enum dqm_dev_idx	fifo_dev_idx;
	int			tx_fifo_idx;
	int			rx_fifo_idx;
	int			tx_fifo_h;
	int			rx_fifo_h;
	bool			link_up;
	void			*priv;
	int 			(*read)(int h, unsigned char *buf, int cnt);
	int 			(*write)(int h, unsigned char *buf, int cnt);
} fifo_tunnel;

#endif
