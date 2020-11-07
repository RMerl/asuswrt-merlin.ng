/*
 * Test harness utility.
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

#ifndef _TEST_H_
#define _TEST_H_

/* --------------------------------------------------------------- */
typedef struct
{
	int count;		/* number of tests run */
	int passed;		/* number of tests passed */
	int failed;		/* number of tests failed */
} testLogT;

#define TEST_DECLARE() static testLogT gTestLog;

#define TEST_INITIALIZE()									\
{															\
	memset(&gTestLog, 0, sizeof(gTestLog));					\
}

#define TEST(condition, error)								\
	do {													\
		gTestLog.count++;									\
		if ((condition)) {									\
			gTestLog.passed++;								\
		}													\
		else {												\
			gTestLog.failed++;								\
			printf("\n*** FAIL *** - %s %s():%d - %s\n\n",	\
				__FILE__, __FUNCTION__, __LINE__, error);	\
		}													\
	} while (0)

#define TEST_FATAL(condition, error)						\
	do {													\
		gTestLog.count++;									\
		if ((condition)) {									\
			gTestLog.passed++;								\
		}													\
		else {												\
			gTestLog.failed++;								\
			printf("\n*** FAIL *** - %s():%d - %s\n\n",		\
				__FUNCTION__, __LINE__, error);				\
			exit(-1);										\
		}													\
	} while (0)

#define TEST_FINALIZE()										\
{															\
	int percent = gTestLog.count ?							\
		gTestLog.passed * 100 / gTestLog.count : 0; 		\
	printf("\n");											\
	printf("Test Results (%s):\n\n", __FILE__);				\
	printf("Tests    %d\n", gTestLog.count);				\
	printf("Pass     %d\n", gTestLog.passed);				\
	printf("Fail     %d\n\n", gTestLog.failed);				\
	printf("%d%%\n\n", percent);							\
}

#endif /* _TEST_H_ */
