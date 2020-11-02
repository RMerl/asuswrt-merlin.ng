/*
 * NSA generic application: NDEF test data
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
 * $Id: $
 */
#ifndef APP_NDEF_DATA_TEST_H
#define APP_NDEF_DATA_TEST_H

#define WPS_PSWD_NDEF_LENGTH 84
#define WIFI_HR_REC_LEN 33
#define WIFI_HS_REC_LEN 33
#define NDEF_PAYLOAD_LEN_OFFSET	2
#define NDEF_PAYLOAD_TYPE_OFFSET 4

#define WIFI_NDEF_REC_WPS_OFFSET 26
#define WIFI_CHO_REC_WPS_OFFSET 28

/* Configuration/Password Tag Read/Write */
UINT8 wifi_wps[MAX_NDEF_LENGTH] = {
	0xd2,	/* NDEF rec header */
	0x17,	/* payload type name length */
	0x3a,	/* payload length */
	/* payload type name: "application/vnd.wfa.wsc" */
	0x61, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f,
	0x6e, 0x2f, 0x76, 0x6e, 0x64, 0x2e, 0x77, 0x66, 0x61, 0x2e,
	0x77, 0x73, 0x63,
	/* payload  */
	0x10, 0x4a, 0x00, 0x01, 0x10, 0x10, 0x2c, 0x00, 0x26, 0x02,
	0x45, 0x67, 0x21, 0x23, 0x60, 0x40, 0x93, 0x84, 0xaf, 0xad,
	0x23, 0x24, 0x9a, 0x10, 0x3c, 0xdf, 0x3f, 0x66, 0x41, 0x01,
	0x0f, 0x4c, 0x3b, 0x2b, 0x20, 0x6a, 0x21, 0x2b, 0x2c, 0x56,
	0x41, 0x32, 0x51, 0x77, 0x42, 0x2b, 0x20, 0x10, 0x49, 0x00,
	0x06, 0x00, 0x37, 0x2a, 0x00, 0x01, 0x20, 0x00
};
UINT32 wifi_wps_len = WPS_PSWD_NDEF_LENGTH;

UINT8 wifi_hs_rec[MAX_NDEF_LENGTH] =
{
	0xDA,	/* NDEF rec header */
	0x17,	/* payload type name length */
	0x05,	/* payload length */
	0x01,       /* payload ID length */
	/* payload type name: "application/vnd.wfa.wsc" */
	0x61, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x74,
	0x69, 0x6f, 0x6e, 0x2f, 0x76, 0x6e, 0x64, 0x2e,
	0x77, 0x66, 0x61, 0x2e, 0x77, 0x73, 0x63,

	0x31,       /* Payload ID: "1" */
	0x10, 0x4a, 0x00, 0x01, 0x10
};
UINT32 wifi_hs_rec_len = WIFI_HS_REC_LEN;

UINT8 wifi_hr_rec[MAX_NDEF_LENGTH] =
{
	0xDA,	/* NDEF rec header */
	0x17,	/* payload type name length */
	0x05,	/* payload length */
	0x01,	/* payload ID length */
	/* payload type name: "application/vnd.wfa.wsc" */
	0x61, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x74,
	0x69, 0x6f, 0x6e, 0x2f, 0x76, 0x6e, 0x64, 0x2e,
	0x77, 0x66, 0x61, 0x2e, 0x77, 0x73, 0x63,

	0x31,       /* Payload ID: "1" */
	0x10, 0x4a, 0x00, 0x01, 0x10
};
UINT32 wifi_hr_rec_len = WIFI_HR_REC_LEN;

#endif /* APP_NDEF_DATA_TEST_H */
