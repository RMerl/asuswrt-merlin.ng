/*
 * Broadcom Home Gateway Reference Design
 * Broadcom QoS Map IE related shared definitions & functions
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 *
 *
 * <<Broadcom-WL-IPTag/Dual:>>
 *
 * $Id: qosmap_utils.c 764858 2018-06-06 13:27:56Z $
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <net/if.h>
#include <sys/types.h>

#include <typedefs.h>

#include "common_utils.h"
#include "qosmap_utils.h"

/* 11u QoS map set */
#define DOT11_MNG_QOS_MAP_ID				110
#define MAP_DSCP_PCP_MAP_SIZE				64
#define BCM_DECODE_QOS_MAP_MAX_EXCEPT_COUNT		21
#define BCM_DECODE_QOS_MAP_MAX_UP			8
#define BCM_DECODE_QOS_MAP_MAX_EXCEPT_HEXSTR_LEN	\
		(BCM_DECODE_QOS_MAP_MAX_EXCEPT_COUNT * 2)
#define BCM_DECODE_QOS_MAP_MAX_EXCEPT_STR_LEN		\
		(BCM_DECODE_QOS_MAP_MAX_EXCEPT_HEXSTR_LEN * 2) + 1

typedef struct
{
	uint8 exceptCount;
	struct {
		uint8 dscp;
		uint8 up;
	} except[BCM_DECODE_QOS_MAP_MAX_EXCEPT_COUNT];
	struct {
		uint8 low;
		uint8 high;
	} up[BCM_DECODE_QOS_MAP_MAX_UP];
} bcm_decode_qos_map_t;

/* Create qosmapie IOVAR Buffer from qosmapie NVRAM String Value */
void
qosmapie_create_iovbuf_from_nvram(char * nvval_str,
	uint8 *iov_buf, uint16 *iov_buf_size)
{
	char var[16], len = 0;
	bool exception_list_parsed = FALSE;
	int i = 0, j = 0, k = 0;

	/* reset */
	i = j = k = len = 0;
	exception_list_parsed = FALSE;
	if (strchr(nvval_str, '+') == NULL) {
		/* no entry found in exception list */
		exception_list_parsed = TRUE;
	}

	iov_buf[j++] = DOT11_MNG_QOS_MAP_ID; /* QosMap id */
	iov_buf[j++] = 0; /* len */

	for (i = 0; i <= strlen(nvval_str); i++) {
		if ((exception_list_parsed == FALSE) && (nvval_str[i] == '+')) {
			exception_list_parsed = TRUE;
		}
		if (exception_list_parsed == FALSE) {
			var[0] = nvval_str[i++];
			var[1] = nvval_str[i];
			var[2] = '\0';

			iov_buf[j++] = (char)strtol(var, NULL, 16);
			len++;
			continue;
		}
		if ((nvval_str[i] != '\0') && (nvval_str[i] != ',') && (nvval_str[i] != ';')) {
			var[k++] = nvval_str[i];
			continue;
		}

		var[k] = '\0';
		iov_buf[j++] = (char)strtol(var, NULL, 10);
		len++;
		k = 0;
	}

	iov_buf[1] = len;
	*iov_buf_size = len + 2;
}

/* Mark Execptions for a tid in QoS Map IE Structure Object using User Priority Table */
static void
qosmapie_mark_except_for_tid(unsigned char *up_table,
	bcm_decode_qos_map_t *qos_map_ie, int tid)
{
	int pos = 0;

	for (pos = qos_map_ie->up[tid].low; ((pos <= qos_map_ie->up[tid].high) &&
		(qos_map_ie->exceptCount < BCM_DECODE_QOS_MAP_MAX_EXCEPT_COUNT)); pos++) {
		if (up_table[pos] != tid) {
			qos_map_ie->except[qos_map_ie->exceptCount].dscp = pos;
			qos_map_ie->except[qos_map_ie->exceptCount].up = up_table[pos];
			qos_map_ie->exceptCount++;
		}
	}
}

/* Create QoS Map IE Structure Object from User Priority Table */
static void
qosmapie_create_ie_from_up_table(unsigned char *up_table,
	bcm_decode_qos_map_t *qos_map_ie)
{
	int tid = 0;

	/* Initialize execpt & up arrays */
	bzero(qos_map_ie, sizeof(*qos_map_ie));
	memset(qos_map_ie->up, 0xFF, sizeof(qos_map_ie->up));

	for (tid = 0; tid < BCM_DECODE_QOS_MAP_MAX_UP; tid++) {

		qos_map_ie->up[tid].low = (tid * BCM_DECODE_QOS_MAP_MAX_UP);
		qos_map_ie->up[tid].high = (((tid + 1) * BCM_DECODE_QOS_MAP_MAX_UP) - 1);

		/* Mark Execptions for this tid */
		qosmapie_mark_except_for_tid(up_table, qos_map_ie, tid);
	}
}

/* Create qosmapie NVRAM String Value from User Priority Table */
void
qosmapie_create_nvram_from_up_table(unsigned char *up_table,
	char *out_str, int out_str_sz)
{
	bcm_decode_qos_map_t qos_map_ie;
	int iter = 0;

	/* Create QoS Map IE Structure Object from User Priority Table */
	qosmapie_create_ie_from_up_table(up_table, &qos_map_ie);

	for (iter = 0; iter < qos_map_ie.exceptCount; iter++) {
		snprintf(out_str + strlen(out_str), out_str_sz - strlen(out_str),
			"%02X", qos_map_ie.except[iter].dscp);
		snprintf(out_str + strlen(out_str), out_str_sz - strlen(out_str),
			"%02X", qos_map_ie.except[iter].up);
	}

	snprintf(out_str + strlen(out_str), out_str_sz - strlen(out_str), "+");

	for (iter = 0; iter < BCM_DECODE_QOS_MAP_MAX_UP; iter++) {
		snprintf(out_str + strlen(out_str), out_str_sz - strlen(out_str),
			"%d", qos_map_ie.up[iter].low);
		snprintf(out_str + strlen(out_str), out_str_sz - strlen(out_str), ",");
		snprintf(out_str + strlen(out_str), out_str_sz - strlen(out_str),
			"%d", qos_map_ie.up[iter].high);
		if (iter != (BCM_DECODE_QOS_MAP_MAX_UP - 1)) {
			snprintf(out_str + strlen(out_str), out_str_sz - strlen(out_str), ";");
		}
	}
}

/* Fill Exceptions in User Priority Table using Exception String of NVRAM Value */
static void
qosmapie_fill_up_table_exceptions(unsigned char *out_up_table, char *except_str)
{
	unsigned char iter_str_hex = 0, except_str_hex_len = strlen(except_str)/2;
	unsigned char except_str_hex[BCM_DECODE_QOS_MAP_MAX_EXCEPT_HEXSTR_LEN];
	unsigned char pos = 0, val = 0;

	bzero(except_str_hex, sizeof(except_str_hex));

	get_hex_data((unsigned char *)except_str,
		&except_str_hex[0], except_str_hex_len);

	/* Fill Exceptions in User Priority Table */
	for (iter_str_hex = 0; iter_str_hex < except_str_hex_len; iter_str_hex += 2) {
		pos = except_str_hex[iter_str_hex];
		val = except_str_hex[iter_str_hex + 1];
		*(out_up_table + pos) = val;
	}
}

/* Fill TID values in User Priority Table using Range String of NVRAM Value */
static int
qosmapie_fill_up_table_range(unsigned char *out_up_table, char *range_str)
{
	unsigned char iter_tid = 0, dscp_low = 0, dscp_high = 0, iter_dscp = 0;
	int num = 0, range_str_eol = 0, range_str_len = strlen(range_str);
	char tid_range[16];
	char *remaining_str = range_str, *next_range = NULL;

	while (remaining_str && !range_str_eol) {

		next_range = strchr(remaining_str, ';');
		if (!next_range) {
			next_range = &range_str[range_str_len];
			range_str_eol = 1;
		}
		memcpy(tid_range, remaining_str, (next_range - remaining_str));
		tid_range[(next_range - remaining_str)] = '\0';

		num = sscanf(tid_range, "%hhu,%hhu", &dscp_low, &dscp_high);
		if ((num != 2) || (dscp_low > dscp_high)) {
			return -1;
		}
		if ((dscp_low == dscp_high) &&
			(dscp_low == 0xFF) && (dscp_high == 0xFF)) {
			continue;
		}

		for (iter_dscp = dscp_low; iter_dscp <= dscp_high; iter_dscp++) {
			*(out_up_table + iter_dscp) = iter_tid;
		}

		remaining_str += strlen(tid_range) + 1;
		iter_tid++;
	}
	return 0;
}

/* Create User Priority Table from qosmapie NVRAM String Value */
void
qosmapie_create_up_table_from_nvram(unsigned char *out_up_table, char *nvval_str)
{
	char *range_str = NULL, except_str[BCM_DECODE_QOS_MAP_MAX_EXCEPT_STR_LEN];

	bzero(out_up_table, MAP_DSCP_PCP_MAP_SIZE);
	bzero(except_str, sizeof(except_str));

	range_str = strchr(nvval_str, '+');
	if (!range_str) {
		range_str = nvval_str;
	} else {
		memcpy(except_str, nvval_str, (range_str - nvval_str));
		range_str++;
	}

	/* Fill TID values in User Priority Table using Range String */
	qosmapie_fill_up_table_range(out_up_table, range_str);

	/* Fill Exceptions in User Priority Table using Exception String */
	qosmapie_fill_up_table_exceptions(out_up_table, except_str);
}
