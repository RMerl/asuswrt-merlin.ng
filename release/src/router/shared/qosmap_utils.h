/*
 * Broadcom Home Gateway Reference Design
 * Broadcom QoS Map IE related shared definitions & functions
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * <<Broadcom-WL-IPTag/Open:>>
 * $Id: qosmap_utils.h 764230 2018-05-24 08:40:37Z $
 */

#ifndef _QOSMAP_UTILS_H_
#define _QOSMAP_UTILS_H_

/* sample qosmapie nvram -
 * nvram set wl1.1_qosmapie="exception_list+low,high;...;low,high;"
 * nvram set wl1.1_qosmapie="3005+0,7;8,15;16,23;24,31;32,39;40,47;48,55;56,63"
 */
#define MAX_EXCEPTION_LIST			21	/* as per 9.4.2.94 QoS Map element */
#define EXCEPTION_LIST_STRLEN			4	/* ex: "3005" */
#define DSCP_RANGE_NUM				8	/* number of up entries */
#define DSCP_RANGE_STRLEN			8	/* ex: "48,55;" or "255,255;" */

#define MAX_QOSMAPIE_STRLEN ((MAX_EXCEPTION_LIST * EXCEPTION_LIST_STRLEN) + \
	1 + (DSCP_RANGE_NUM * DSCP_RANGE_STRLEN))

/* Create qosmapie IOVAR Buffer from qosmapie NVRAM String Value */
void qosmapie_create_iovbuf_from_nvram(char * nvval_str,
	uint8 *iov_buf, uint16 *iov_buf_size);

/* Create qosmapie NVRAM String Value from User Priority Table */
void qosmapie_create_nvram_from_up_table(unsigned char *up_table,
	char *out_str, int out_str_sz);

/* Create User Priority Table from qosmapie NVRAM String Value */
void qosmapie_create_up_table_from_nvram(unsigned char *out_up_table,
	char *nvval_str);

#endif /* _QOSMAP_UTILS_H_ */
