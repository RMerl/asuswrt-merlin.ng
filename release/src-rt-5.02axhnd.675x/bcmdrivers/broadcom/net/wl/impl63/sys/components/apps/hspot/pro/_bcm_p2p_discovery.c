/*
 * Test harness for WiFi-Direct discovery state machine.
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
#include <unistd.h>
#include "trace.h"
#include "test.h"
#include "dsp.h"
#include "wlu_api.h"
#include "bcm_p2p_discovery.h"

TEST_DECLARE();

/* --------------------------------------------------------------- */

void testP2PDiscovery(void)
{
	bcm_p2p_discovery_t *disc;

	TEST(bcm_p2p_discovery_initialize(), "bcm_p2p_discovery_initialize failed");
	disc = bcm_p2p_discovery_create(0, 11);
	TEST(disc != 0, "bcm_p2p_discovery_create failed");

	/* discovery */
	TEST(bcm_p2p_discovery_start_discovery(disc), "bcm_p2p_discovery_start_discovery failed");
	sleep(15);
	TEST(bcm_p2p_discovery_reset(disc), "bcm_p2p_discovery_reset failed");

	/* extended listen */
	TEST(bcm_p2p_discovery_start_ext_listen(disc, 500, 4500),
		"bcm_p2p_discovery_start_ext_listen failed");
	sleep(15);
	TEST(bcm_p2p_discovery_reset(disc), "bcm_p2p_discovery_reset failed");

	/* listen */
	TEST(bcm_p2p_discovery_start_ext_listen(disc, 5000, 0),
		"bcm_p2p_discovery_start_ext_listen failed");
	sleep(15);
	TEST(bcm_p2p_discovery_reset(disc), "bcm_p2p_discovery_reset failed");

	TEST(bcm_p2p_discovery_destroy(disc), "bcm_p2p_discovery_destroy failed");
	TEST(bcm_p2p_discovery_deinitialize(), "bcm_p2p_discovery_deinitialize failed");
}

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	TRACE_LEVEL_SET(TRACE_ERROR | TRACE_DEBUG);
	TEST_INITIALIZE();

	testP2PDiscovery();

	/* disable wlan */
	wlFree();

	/* terminate dispatcher */
	dspFree();

	TEST_FINALIZE();
	return 0;
}
