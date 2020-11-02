/*
 * rc4.c
 * RC4 stream cipher
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: rc4.c 451682 2014-01-27 20:30:17Z $
 */

#include <typedefs.h>
#include <bcmcrypto/rc4.h>

void
prepare_key(uint8 *key_data, int key_data_len, rc4_ks_t *ks)
{
	unsigned int counter, index1 = 0, index2 = 0;
	uint8 key_byte, temp;
	uint8 *key_state = ks->state;

	for (counter = 0; counter < RC4_STATE_NBYTES; counter++) {
		key_state[counter] = (uint8) counter;
	}

	for (counter = 0; counter < RC4_STATE_NBYTES; counter++) {
		key_byte = key_data[index1];
		index2 = (key_byte + key_state[counter] + index2) % RC4_STATE_NBYTES;
		temp = key_state[counter];
		key_state[counter] = key_state[index2];
		key_state[index2] = temp;
		index1 = (index1 + 1) % key_data_len;
	}

	ks->x = 0;
	ks->y = 0;
}

/* encrypt or decrypt using RC4 */
void
rc4(uint8 *buf, int data_len, rc4_ks_t *ks)
{
	uint8 tmp;
	uint8 xor_ind, x = ks->x, y = ks->y, *key_state = ks->state;
	int i;

	for (i = 0; i < data_len; i++) {
		y += key_state[++x]; /* mod RC4_STATE_NBYTES */
		tmp = key_state[x];
		key_state[x] = key_state[y];
		key_state[y] = tmp;
		xor_ind = key_state[x] + key_state[y];
		if (buf)
			buf[i] ^= key_state[xor_ind];
	}

	ks->x = x;
	ks->y = y;
}

#ifdef BCMRC4_TEST
#include <stdio.h>
#include <string.h>

#include "rc4_vectors.h"

#define NUM_VECTORS  (sizeof(rc4_vec) / sizeof(rc4_vec[0]))

int
main(int argc, char **argv)
{
	int k, fail = 0;
	uint8 data[RC4_STATE_NBYTES];
	rc4_ks_t ks;
	for (k = 0; k < NUM_VECTORS; k++) {
		memset(data, 0, RC4_STATE_NBYTES);
		memcpy(data, rc4_vec[k].input, rc4_vec[k].il);

		prepare_key(rc4_vec[k].key, rc4_vec[k].kl, &ks);
		rc4(data, rc4_vec[k].il, &ks);
		if (memcmp(data, rc4_vec[k].ref, rc4_vec[k].il) != 0) {
			printf("%s: rc4 encrypt failed\n", *argv);
			fail++;
		} else {
			printf("%s: rc4 encrypt %d passed\n", *argv, k);
		}

		memset(data, 0, RC4_STATE_NBYTES);
		memcpy(data, rc4_vec[k].ref, rc4_vec[k].il);

		prepare_key(rc4_vec[k].key, rc4_vec[k].kl, &ks);
		rc4(data, rc4_vec[k].il, &ks);
		if (memcmp(data, rc4_vec[k].input, rc4_vec[k].il) != 0) {
			printf("%s: rc4 decrypt failed\n", *argv);
			fail++;
		} else {
			printf("%s: rc4 decrypt %d passed\n", *argv, k);
		}
	}

	printf("%s: %s\n", *argv, fail?"FAILED":"PASSED");
	return (fail);
}
#endif /* BCMRC4_TEST */
