/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Texas Instruments' K3 Secure proxy
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 *
 */

#ifndef K3_SEC_PROXY_H
#define K3_SEC_PROXY_H

/**
 * struct k3_sec_proxy_msg - Secure proxy message structure
 * @len: Length of data in the Buffer
 * @buf: Buffer pointer
 *
 * This is the structure for data used in mbox_send() and mbox_recv().
 */
struct k3_sec_proxy_msg {
	size_t len;
	u32 *buf;
};

#endif /* K3_SEC_PROXY_H */
