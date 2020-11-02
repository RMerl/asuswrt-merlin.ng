/*
 * Test harness for encoding and decoding base functions.
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
 * $Id:$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "trace.h"
#include "bcm_encode.h"
#include "bcm_decode.h"

TEST_DECLARE();

#define BUFFER_SIZE		256
static uint8 buffer[BUFFER_SIZE];
static bcm_encode_t enc;

/* --------------------------------------------------------------- */

static void testEncode(void)
{
	uint8 data[BUFFER_SIZE];

	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");

	TEST(bcm_encode_be16(&enc, 0x1122) == 2, "bcm_encode_be16 failed");
	TEST(bcm_encode_be32(&enc, 0x11223344) == 4, "bcm_encode_be32 failed");
	TEST(bcm_encode_le16(&enc, 0xaabb) == 2, "bcm_encode_le16 failed");
	TEST(bcm_encode_le32(&enc, 0xaabbccdd) == 4, "bcm_encode_le32 failed");

	/* packet full */
	TEST(bcm_encode_bytes(&enc, BUFFER_SIZE, data) == 0, "bcm_encode_bytes failed");
	TEST(bcm_encode_length(&enc) == 12, "bcm_encode_length failed");
}

static void testDecode(void)
{
	bcm_decode_t dec;
	uint16 data16;
	uint32 data32;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");
	WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

	data16 = 0;
	TEST(bcm_decode_be16(&dec, &data16) == 2, "bcm_decode_be16 failed");
	TEST(data16 == 0x1122, "invalid data");
	data32 = 0;
	TEST(bcm_decode_be32(&dec, &data32) == 4, "bcm_decode_be32 failed");
	TEST(data32 == 0x11223344, "invalid data");
	data16 = 0;
	TEST(bcm_decode_le16(&dec, &data16) == 2, "bcm_decode_le16 failed");
	TEST(data16 == 0xaabb, "invalid data");
	data32 = 0;
	TEST(bcm_decode_le32(&dec, &data32) == 4, "bcm_decode_le32 failed");
	TEST(data32 == 0xaabbccdd, "invalid data");

	/* decode beyond buffer */
	TEST(bcm_decode_be16(&dec, &data16) == 0, "bcm_decode_be16 failed");
	TEST(bcm_decode_be32(&dec, &data32) == 0, "bcm_decode_be32 failed");
	TEST(bcm_decode_le16(&dec, &data16) == 0, "bcm_decode_le16 failed");
	TEST(bcm_decode_le32(&dec, &data32) == 0, "bcm_decode_le32 failed");
}

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	TRACE_LEVEL_SET(TRACE_ALL);
	TEST_INITIALIZE();

	testEncode();
	testDecode();

	TEST_FINALIZE();
	return 0;
}
