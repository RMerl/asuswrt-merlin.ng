/*
 * Broadcom IEEE1905 MultiAP-R3 AES-SIV utils
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#if defined(MULTIAP)
#include "ieee1905_aessiv.h"
#include "ieee1905_trace.h"
#include <ethernet.h>
#include <time.h>

#define I5_TRACE_MODULE i5TraceTlv

int
ieee1905_encrypt_packet(unsigned char *data, int data_len, unsigned char *siv,
	unsigned char *cmdu, unsigned char *enc_tx_counter, unsigned char *src_mac,
	i5_dm_device_type *dst_dev)
{
	int ret;
	aessiv_ctx_t ctx;

	/* Initialize */
	ret = aessiv_init(&ctx, SIV_ENCRYPT, dst_dev->ptk_len, dst_dev->ptk, NULL);
	if (ret != 0) {
		i5TraceError("aessiv_init returned error %d\n", ret);
		return ret;
	}

	/* AD1 first six bytes of cmdu */
	ret = aessiv_update(&ctx, cmdu, I5_SEC_CMDU_LEN);
	if (ret != 0) {
		i5TraceError("aessiv_update for AD1 returned error %d\n", ret);
		return ret;
	}

	/* AD2 counter */
	ret = aessiv_update(&ctx, enc_tx_counter, I5_SEC_COUNTER_LEN);
	if (ret != 0) {
		i5TraceError("aessiv_update for AD2 returned error %d\n", ret);
		return ret;
	}

	/* AD3 Src almac address */
	ret = aessiv_update(&ctx, src_mac, ETHER_ADDR_LEN);
	if (ret != 0) {
		i5TraceError("aessiv_update for AD3 returned error %d\n", ret);
		return ret;
	}

	/* AD4 Dst almac address */
	ret = aessiv_update(&ctx, dst_dev->DeviceId, ETHER_ADDR_LEN);
	if (ret != 0) {
		i5TraceError("aessiv_update for AD4 returned error %d\n", ret);
		return ret;
	}

	/* Encrypt using aessiv */
	ret = aessiv_final(&ctx, siv, data, data_len);
	if (ret != 0) {
		i5TraceError("aessiv_final returned error %d\n", ret);
		return ret;
	}

	return ret;
}

int
ieee1905_decrypt_packet(unsigned char *data, int data_len, unsigned char *siv,
	unsigned char *cmdu, unsigned char *enc_rx_counter, unsigned char *dst_mac,
	i5_dm_device_type *src_dev)
{
	int ret;
	aessiv_ctx_t ctx;

	/* Initialize */
	ret = aessiv_init(&ctx, SIV_DECRYPT, src_dev->ptk_len, src_dev->ptk, NULL);
	if (ret != 0) {
		i5TraceError("aessiv_init returned error %d\n", ret);
	}

	/* AD1 first six bytes of data */
	ret = aessiv_update(&ctx, cmdu, I5_SEC_CMDU_LEN);
	if (ret != 0) {
		i5TraceError("aessiv_update for AD1 returned error %d\n", ret);
	}

	/* AD2 counter */
	ret = aessiv_update(&ctx, enc_rx_counter, I5_SEC_COUNTER_LEN);
	if (ret != 0) {
		i5TraceError("aessiv_update for AD2 returned error %d\n", ret);
	}

	/* AD3 Src almac address */
	ret = aessiv_update(&ctx, src_dev->DeviceId, ETHER_ADDR_LEN);
	if (ret != 0) {
		i5TraceError("aessiv_update for AD3 returned error %d\n", ret);
	}

	/* AD4 Dst almac address */
	ret = aessiv_update(&ctx, dst_mac, ETHER_ADDR_LEN);
	if (ret != 0) {
		i5TraceError("aessiv_update for AD4 returned error %d\n", ret);
	}

	/* Decrypt */
	ret = aessiv_final(&ctx, siv, data, data_len);
	if (ret != 0) {
		i5TraceError("aessiv_final returned error %d\n", ret);
	}

	return ret;
}

#endif /* MULTIAP */
