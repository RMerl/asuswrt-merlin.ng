/**
* @file
* @brief
* WLOTA feature.
* On a high level there are two modes of operation
* 1. Non Tethered Mode
* 2. Tethered Mode
*
* IN non tethered mode, a cmd flow file which contains encoded test
*	information is downloaded to device.
*	Format of the cmd flow file is pre defined. Host interprest the cmd flow file
*	and passes down a test "structure" to dongle.
*	Reading and parsing can be done by brcm wl utility or any host software which can do
*	the same operation.
*
*	Once cmd flow file is downloaded, a "trigger" cmd is
*	called to put the device into testing mode. It will wait for a sync packet from
* 	tester as a part of handshake mechanism. if device successfully decodes sync packet
*	from an expected mac address, device is good to start with the test sequece.
*	Right now only two kinds of test are downloaded to device.
*		ota_tx
*		ota_rx
*
*	ota_tx/ota_rx takes in arguments as
*	test chan bandwidth contrlchan rates stf txant rxant tx_ifs tx_len num_pkt pwrctrl
*		start:delta:end
*
*	Cmd flow file should have a test setup information like various mac address, sycn timeout.
*	Format is:  synchtimeoout(seconds) synchbreak/loop synchmac txmac rxmac
*
* In tethered mode, test flow is passed down in form of wl iovars through batching mode
*	Sequence of operation is
*	test_stream start	[start batching mode operation]
*	test_stream test_setup  [where test_setup is of the same format in cmd flow file]
*	test_stream test_cmd	[should be of same format of ota_tx /ota_rx in cmd_flow file]
*	test_stream stop	[stops batching mode operation and downloads the file to dongle]
*$Id: wlc_ota_test.c 708017 2017-06-29 14:11:45Z $
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
*/

#ifdef WLOTA_EN

/* ---------------------Include files -------------------------- */
#include <wlc_cfg.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc_pio.h>
#include <wlc.h>
#ifdef DONGLE_BUILD
#include <wl_oid.h>
#endif // endif
#include <wlc_types.h>

#include <wl_export.h>
#include <wlc_ota_test.h>

static void wlc_schedule_ota_test(void *arg);
static void wlc_schedule_ota_tx_test(void *arg);
static void wlc_ota_test_wait_for_sync(void *arg);
static void wlc_ota_test_arbitrate(wlc_info_t * wlc);
static void wlc_ota_test_engine_reset(wlc_info_t * wlc);
static void wlc_ota_trigger_test_engine(wlc_info_t * wlc);
static void wlc_ota_test_cleanup(ota_test_info_t * ota_info);
static void wlc_ota_test_exit_engine(wlc_info_t *wlc);

/* Main OTA test structure */
struct ota_test_info {
	/* Pointer back to wlc structure */
	wlc_info_t *wlc;

	wl_ota_test_vector_t * test_vctr;	/* test vector */

	struct wl_timer *test_timer ;  /* timer to track ota test */
	struct wl_timer *tx_test_timer; /* timer to track lp tx test */
	struct wl_timer *sync_timer;	/* timer to track sync operation */

	int16 test_phase;		/* cur test cnt. Shows the index of test being run */
	uint16 tx_test_phase;		/* cur tx test phase */
	uint8 test_stage;		/* Test stages 1. idle 2. active 3. success 4. fail */
	uint8 download_stage;		/* download stage */
	int8 test_skip_reason;		/* test fail reason */
	uint8 test_loop_cnt;		/* Looping cnt in dbg mode */
	uint8 ota_sync_status;		/* OTA sync status */
	uint8 ota_sync_wait_cnt;	/* Cntr maintained to check no of iteration for sync */
};

/* ota test iovar function */
STATIC int
ota_test_doiovar
(
	void                                    *hdl,
	const bcm_iovar_t       *vi,
	uint32                          actionid,
	const char                      *name,
	void                                    *p,
	uint                                    plen,
	void                                    *a,
	int                                     alen,
	int                                     vsize,
	struct wlc_if           *wlcif
);
/* OTA test iovar enums */
enum {
	IOV_OTA_TRIGGER,
	IOV_OTA_LOADTEST,
	IOV_OTA_TESTSTATUS,
	IOV_OTA_TESTSTOP
};
static const bcm_iovar_t ota_test_iovars[] = {
	{"ota_trigger", IOV_OTA_TRIGGER,
	(IOVF_OPEN_ALLOW), IOVT_BOOL, 0
	},
	{"ota_loadtest", IOV_OTA_LOADTEST,
	(0), IOVT_BUFFER, WL_OTA_ARG_PARSE_BLK_SIZE,
	},
	{"ota_teststatus", IOV_OTA_TESTSTATUS,
	(0), IOVT_BUFFER, sizeof(wl_ota_test_status_t),
	},
	{"ota_teststop", IOV_OTA_TESTSTOP,
	(0), IOVT_BOOL, 0,
	},
	{NULL, 0, 0, 0, 0 }

};
ota_test_info_t *
BCMATTACHFN(wlc_ota_test_attach)(wlc_info_t *wlc)
{
	ota_test_info_t * ota_test_info = NULL;

	/* ota test info */
	ota_test_info = (ota_test_info_t *)MALLOCZ(wlc->osh, sizeof(ota_test_info_t));
	if (ota_test_info == NULL) {
		WL_ERROR(("wl%d: %s: MALLOCZ failed, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* attach ota test vector */
	ota_test_info->test_vctr = (wl_ota_test_vector_t *)MALLOCZ(wlc->osh,
		sizeof(wl_ota_test_vector_t));
	if (ota_test_info->test_vctr == NULL) {
		WL_ERROR(("wl%d: %s: MALLOCZ failed, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	/* Timers for wl ota test */
	if (!(ota_test_info->test_timer = wl_init_timer(wlc->wl, wlc_schedule_ota_test, wlc,
		"test_timer"))) {
		WL_ERROR(("wl%d:  wl_init_timer for test_timer failed\n", wlc->pub->unit));
		goto fail;
	}

	/* Split up the full ota test cases so that tx buffer issues are not there */
	if (!(ota_test_info->tx_test_timer = wl_init_timer(wlc->wl, wlc_schedule_ota_tx_test, wlc,
		"tx_test_timer"))) {
		WL_ERROR(("wl%d:  wl_init_timer for tx_test_timer failed\n", wlc->pub->unit));
		goto fail;
	}
	if (!(ota_test_info->sync_timer = wl_init_timer(wlc->wl, wlc_ota_test_wait_for_sync, wlc,
		"sync_timer"))) {
		WL_ERROR(("wl%d:  wl_init_timer for sync_timer failed\n", wlc->pub->unit));
		goto fail;
	}
	ota_test_info->wlc = wlc;
	ota_test_info->test_skip_reason = 0;
	ota_test_info->tx_test_phase = 0;
	ota_test_info->test_phase = -1;
	ota_test_info->test_stage =  WL_OTA_TEST_IDLE;
	wlc->iov_block = &ota_test_info->test_stage;

	/* Register this module. */
	wlc_module_register(wlc->pub,
		ota_test_iovars,
		"ota_test",
		ota_test_info,
		ota_test_doiovar,
		NULL, NULL,
		NULL);

	return ota_test_info;

fail:
	/* Free up the memory */
	wlc_ota_test_cleanup(ota_test_info);

	return NULL;
}
static void
BCMATTACHFN(wlc_ota_test_cleanup)(ota_test_info_t * ota_info)
{

	/* Kill the test timers */
	if (ota_info->test_timer) {
		wl_free_timer(ota_info->wlc->wl, ota_info->test_timer);
		ota_info->test_timer = NULL;
	}
	if (ota_info->tx_test_timer) {
		wl_free_timer(ota_info->wlc->wl, ota_info->tx_test_timer);
		ota_info->tx_test_timer = NULL;
	}
	if (ota_info->sync_timer) {
		wl_free_timer(ota_info->wlc->wl, ota_info->sync_timer);
		ota_info->sync_timer = NULL;
	}

	/* release test vector */
	if (ota_info->test_vctr)
		MFREE(ota_info->wlc->osh, ota_info->test_vctr, sizeof(wl_ota_test_vector_t));

	/* Free test info */
	if (ota_info)
		MFREE(ota_info->wlc->osh, ota_info, sizeof(ota_test_info_t));
}
void
BCMATTACHFN(wlc_ota_test_detach)(ota_test_info_t * ota_info)
{

	/* Unregister this module */
	wlc_module_unregister(ota_info->wlc->pub, "ota_test", ota_info->wlc);

	/* Free up the memory */
	wlc_ota_test_cleanup(ota_info);
}
STATIC int
ota_test_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	int err = 0;
	ota_test_info_t * ota_info;
	int32 *ret_int_ptr = NULL;
	bool bool_val  = FALSE;
	int32 int_val;

	ota_info = hdl;
	wlc_info_t *wlc =  ota_info->wlc;
	wl_ota_test_vector_t * test_vctr = ota_info->test_vctr;

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	/* convenience int and bool vals for first 4 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
	case IOV_GVAL(IOV_OTA_TESTSTATUS): {
		uint16 cnt;
		wl_ota_test_status_t * test_status = (wl_ota_test_status_t *)MALLOC(wlc->osh,
			sizeof(wl_ota_test_status_t));

		if (test_status == NULL) {
			/* Malloc failures; Dont proceed */
			err = BCME_NOMEM;
			break;
		}

		/* Copy test details */
		bcopy(params, &cnt, sizeof(uint16));

		test_status->test_cnt = test_vctr->test_cnt;
		test_status->loop_test = test_vctr->loop_test;
		test_status->cur_test_cnt = wlc->ota_info->test_phase;
		test_status->skip_test_reason = wlc->ota_info->test_skip_reason;
		test_status->file_dwnld_valid = test_vctr->file_dwnld_valid;
		test_status->sync_timeout = test_vctr->sync_timeout;
		test_status->sync_fail_action = test_vctr->sync_fail_action;
		test_status->test_stage = wlc->ota_info->test_stage;
		test_status->sync_status = wlc->ota_info->ota_sync_status;

		bcopy(&(test_vctr->sync_mac), &(test_status->sync_mac), sizeof(struct ether_addr));
		bcopy(&(test_vctr->tx_mac), &(test_status->tx_mac), sizeof(struct ether_addr));
		bcopy(&(test_vctr->rx_mac), &(test_status->rx_mac), sizeof(struct ether_addr));

		if ((cnt  > 0) && (cnt <= test_vctr->test_cnt)) {
			bcopy(&(test_vctr->test_arg[cnt - 1]), &(test_status->test_arg),
				sizeof(wl_ota_test_args_t));
		}

		bcopy(test_status, (wl_ota_test_status_t *)arg, sizeof(wl_ota_test_status_t));

		/* Free test pointers */
		MFREE(wlc->osh, test_status, sizeof(wl_ota_test_status_t));
		break;
		}
	case IOV_SVAL(IOV_OTA_LOADTEST): {
		void * ptr1;
		uint8 num_loop;
		uint32 rem;
		uint32 size;

		num_loop = sizeof(wl_ota_test_vector_t) / WL_OTA_ARG_PARSE_BLK_SIZE;
		rem = sizeof(wl_ota_test_vector_t) % WL_OTA_ARG_PARSE_BLK_SIZE;

		/* use new uint * pnt so that we can do byte wide operation on it */
		ptr1 = (uint8 *)test_vctr;

		if (wlc->ota_info->download_stage == 0) {
			/* reset all test flags */
			wlc_ota_test_engine_reset(wlc);

			bzero(test_vctr, sizeof(wl_ota_test_vector_t));
		}

		/* decide whether to copy full WL_OTA_ARG_PARSE_BLK_SIZE bytes of pending bytes */
		size = (wlc->ota_info->download_stage == num_loop) ?
			rem : WL_OTA_ARG_PARSE_BLK_SIZE;

		/* Copy WL_OTA_ARG_PARSE_BLK_SIZE bytes ata atime */
		bcopy(params, (ptr1 + wlc->ota_info->download_stage * WL_OTA_ARG_PARSE_BLK_SIZE),
			size);

		wlc->ota_info->download_stage++;

		if (wlc->ota_info->download_stage == (num_loop + 1)) {
			/* Last stage of copying */
			test_vctr->file_dwnld_valid = TRUE;
			wlc->ota_info->download_stage = 0;
		}
		break;
	}
	case IOV_SVAL(IOV_OTA_TRIGGER):
		if (bool_val) {
			wlc_ota_trigger_test_engine(wlc);
		}
		break;
	case IOV_SVAL(IOV_OTA_TESTSTOP):

		/* reset all test flags */
		wlc_ota_test_engine_reset(wlc);

		wlc_statsupd(wlc);
		/* wl down */
		wlc_set(wlc, WLC_DOWN, 1);
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}
static void
wlc_ota_test_engine_reset(wlc_info_t * wlc)
{
	/*  delete timers */
	wl_del_timer(wlc->wl, wlc->ota_info->test_timer);
	wl_del_timer(wlc->wl, wlc->ota_info->tx_test_timer);
	wl_del_timer(wlc->wl, wlc->ota_info->sync_timer);

	/* reset test variables */
	wlc->ota_info->test_stage = WL_OTA_TEST_IDLE;
	wlc->ota_info->test_skip_reason = 0;
	wlc->ota_info->test_phase = -1;
	wlc->ota_info->tx_test_phase = 0;
	wlc->ota_info->test_loop_cnt = 0;
	wlc->ota_info->ota_sync_wait_cnt = 0;
	wlc->ota_info->ota_sync_status = WL_OTA_SYNC_IDLE;

}
static void
wlc_ota_trigger_test_engine(wlc_info_t * wlc)
{
	/*  delete timers */
	wl_del_timer(wlc->wl, wlc->ota_info->test_timer);
	wl_del_timer(wlc->wl, wlc->ota_info->tx_test_timer);
	wl_del_timer(wlc->wl, wlc->ota_info->sync_timer);

	/* reset test variables */
	wlc->ota_info->test_skip_reason = 0;
	wlc->ota_info->tx_test_phase = 0;
	wlc->ota_info->test_stage = WL_OTA_TEST_ACTIVE;
	wlc->ota_info->test_phase = 0;
	wlc->ota_info->ota_sync_wait_cnt = 0;

	/* call with a 50ms delay timer */
	wl_add_timer(wlc->wl, wlc->ota_info->test_timer, 50, 0);
}
static void
wlc_ota_test_start_pkteng(wlc_info_t * wlc, uint8 mask, wl_ota_test_args_t *test_arg,
	uint8 sync)
{
	uint8 dst_addr[6];
	uint8 src_addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	wl_pkteng_t pkteng;
	wl_ota_test_vector_t * test_vctr = wlc->ota_info->test_vctr;

	/* copy pkteng information from flow file */
	pkteng.delay = test_arg->pkteng.delay;
	pkteng.nframes = test_arg->pkteng.nframes;
	pkteng.length = test_arg->pkteng.length;
	pkteng.seqno = 0;

	if (sync)
		pkteng.flags = WL_PKTENG_SYNCHRONOUS | mask;
	else
		pkteng.flags = mask;

	/* copy destination address differently for rx and tx testing */
	if (mask == WL_PKTENG_PER_TX_START) {
		bcopy(&(test_vctr->tx_mac), dst_addr,  sizeof(dst_addr));
	} else if (mask == WL_PKTENG_PER_RX_WITH_ACK_START) {
		bcopy(&(test_vctr->rx_mac), dst_addr,  sizeof(dst_addr));
	}

	bcopy(dst_addr, pkteng.dest.octet, sizeof(dst_addr));
	bcopy(src_addr, pkteng.src.octet, sizeof(src_addr));

	/* trigger  pkteng */
	wlc_iovar_op(wlc, "pkteng", NULL, 0, &pkteng, sizeof(wl_pkteng_t), IOV_SET, NULL);
}

/* parse rate string to rspec */
static void
wlc_ota_test_set_rate(wlc_info_t * wlc, wl_ota_test_args_t *test_arg, uint8 i)
{
	int32 int_val = 0;

	int_val = test_arg->rt_info.rate_val_mbps[i];

	int_val |= (test_arg->stf_mode << OLD_NRATE_STF_SHIFT) & OLD_NRATE_STF_MASK;

	wlc_iovar_op(wlc, "nrate", NULL, 0, &int_val, sizeof(int_val), IOV_SET, NULL);
}
/* wl txpwr1 -o -q dbm iovar */
static void
wlc_ota_test_set_txpwr(wlc_info_t * wlc, int8 target_pwr)
{
	int32 int_val;

	if (target_pwr != -1)
		int_val = (WL_TXPWR_OVERRIDE | target_pwr);
	else
		int_val = 127;

	/* wl txpwr1 -o -q 60 */
	wlc_iovar_op(wlc, "qtxpower", NULL, 0, &int_val, sizeof(int_val), IOV_SET, NULL);
}
/* Force phy calibrations */
static int8
wlc_ota_test_force_phy_cal(wlc_info_t * wlc)
{

	uint8 wait_ctr = 0;
	int val2;

#ifdef BCM4334_CHIP_ID
	if (CHIPID(wlc->pub->sih->chip) == BCM4334_CHIP_ID) {
		return BCME_OK;
	}
#endif /* BCM4334_CHIP_ID */

	wlc_iovar_setint(wlc, "phy_forcecal", PHY_FULLCAL_SPHASE);

	OSL_DELAY(1000 * 100);
	wait_ctr = 0;

	while (wait_ctr < 5) {
		wlc_iovar_getint(wlc, "phy_activecal", &val2);
		if (val2 == 0)
			break;
		else
			OSL_DELAY(1000 * 10);
		wait_ctr++;
	}
	if (wait_ctr == 5) {
		WL_ERROR(("Force cal failure \n"));
		return WL_OTA_SKIP_TEST_CAL_FAIL;
	}

	return BCME_OK;
}
/* Test init sequence */
static int8
wlc_ota_test_init_seq(wlc_info_t *wlc)
{
	int8 skip_test_reason;
	int isup;
	char cntry[WLC_CNTRY_BUF_SZ] = "ALL";

	/* wl down */
	wlc_set(wlc, WLC_DOWN, 1);

	/* wl country ALL */
	wlc_iovar_op(wlc, "country", NULL, 0, cntry, sizeof(cntry), IOV_SET, NULL);

	/* wl band b */
	wlc_set(wlc, WLC_SET_BAND, WLC_BAND_2G);

	/* wl mpc 0 */
	wlc_iovar_setint(wlc, "mpc", 0);

	/* wl mimo_bw_cap 1 */
	wlc_iovar_setint(wlc, "mimo_bw_cap", 1);

	/* wl up */
	wlc_set(wlc, WLC_UP, 1);

	/* wl phy_forcecal 1 */
	if ((skip_test_reason = wlc_ota_test_force_phy_cal(wlc)) != BCME_OK) {
		return skip_test_reason;
	}

	if (!wlc_get(wlc, WLC_GET_UP, &isup)) {
		if (!isup) {
			skip_test_reason = WL_OTA_SKIP_TEST_WL_NOT_UP;
			return skip_test_reason;
		}
	}

	/* wl txant 0 */
	wlc_set(wlc, WLC_SET_TXANT, 0);

	/* wl antdiv 0 */
	wlc_set(wlc, WLC_SET_ANTDIV, 0);

	/* wl rtsthresh 1764 */
	wlc_iovar_setint(wlc, "rtsthresh", 1764);

	wlc_iovar_setint(wlc, "fast_timer", 15000);	/* fast timer */
	wlc_iovar_setint(wlc, "slow_timer", 15000);	/* slow timer */
	wlc_iovar_setint(wlc, "glacial_timer", 15000);	/* glacial timer */
	wlc_iovar_setint(wlc, "phy_watchdog", 0);	/* disbale watchdog */
	wlc_iovar_setint(wlc, "phy_forcecal", 0);	/* disable forcecal */
	wlc_iovar_setint(wlc, "phy_percal", 0);		/* disable percal */

	wlc_set(wlc, WLC_SET_SCANSUPPRESS, 1);
	wlc_set(wlc, WLC_SET_PLCPHDR, WLC_PLCP_LONG);

	return BCME_OK;
}
/* Wait for sync packet */
static int
wlc_ota_test_wait_for_sync_pkt(wlc_info_t *wlc)
{

	uint8 dst_addr[6];
	uint8 src_addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint32 pktengrxducast_start, pktengrxducast_end;

	wl_pkteng_t pkteng;
	wl_ota_test_vector_t * test_vctr = wlc->ota_info->test_vctr;

	/* Ppulate pkteng structure */
	bcopy(&(test_vctr->sync_mac), dst_addr, sizeof(dst_addr));
	pkteng.flags = WL_PKTENG_SYNCHRONOUS | WL_PKTENG_PER_RX_WITH_ACK_START;
	pkteng.delay = 1 * 1000;	/* Wait for 1 second in sync loop */
	pkteng.nframes = 0x1;
	pkteng.length = 0x0;
	pkteng.seqno = 0;
	bcopy(dst_addr, pkteng.dest.octet, sizeof(dst_addr));
	bcopy(src_addr, pkteng.src.octet, sizeof(src_addr));

	/* get counter value before start of pkt engine */
	wlc_ctrupd(wlc, MCSTOFF_RXGOODUCAST);
	pktengrxducast_start = MCSTVAR(wlc->pub, pktengrxducast);

	/* Trigger pkteng */
	wlc_iovar_op(wlc, "pkteng", NULL, 0, &pkteng, sizeof(wl_pkteng_t), IOV_SET, NULL);

	/* get counter update after sync packet */
	wlc_ctrupd(wlc, MCSTOFF_RXGOODUCAST);

	pktengrxducast_end = MCSTVAR(wlc->pub, pktengrxducast);

	/* return if failed to recieve the sycn packet */
	if ((pktengrxducast_end - pktengrxducast_start) < 1) {
		return (WL_OTA_SKIP_TEST_SYNCH_FAIL);
	}
	return BCME_OK;
}
/* Set the channel */
static int
wlc_ota_test_set_chan(wlc_info_t *wlc, wl_ota_test_args_t *test_arg)
{
	uint8 band;
	uint32 chanspec = 0;
	uint8 skip_test_reason;
	int32 int_val;

	/* Populate channel number */
	chanspec = chanspec | test_arg->chan;

	/* Populate the band */
	band = (test_arg->chan <= CH_MAX_2G_CHANNEL) ? WLC_BAND_2G : WLC_BAND_5G;

	if (band == WLC_BAND_2G)
		chanspec = chanspec | WL_CHANSPEC_BAND_2G;
	else
		chanspec = chanspec | WL_CHANSPEC_BAND_5G;

	/* Populate Bandwidth */
	if (test_arg->bw == WL_OTA_TEST_BW_20MHZ)
		chanspec = chanspec | WL_CHANSPEC_BW_20;
	else
		chanspec = chanspec | WL_CHANSPEC_BW_40;

	/* Populate sideband */
	if (test_arg->bw == WL_OTA_TEST_BW_20MHZ)
		chanspec = chanspec | WL_CHANSPEC_CTL_SB_NONE;
	else if (test_arg->control_band == 'l')
		chanspec = chanspec | WL_CHANSPEC_CTL_SB_LOWER;
	else if (test_arg->control_band == 'u')
		chanspec = chanspec | WL_CHANSPEC_CTL_SB_UPPER;

	/* wl band b/a */
	wlc_set(wlc, WLC_SET_BAND, band);

	/* invoke wl chanspec iovar */
	wlc_iovar_op(wlc, "chanspec", NULL, 0, &chanspec, sizeof(uint32), IOV_SET, NULL);

	if (test_arg->bw == WL_OTA_TEST_BW_40MHZ) {
		int_val = PHY_TXC1_BW_40MHZ;
	} else {
		int_val = -1;
	}

	/* wl mimo txbw */
	wlc_iovar_op(wlc, "mimo_txbw", NULL, 0, &int_val, sizeof(int_val), IOV_SET, NULL);

	/* Do a force cal */
	if ((skip_test_reason = wlc_ota_test_force_phy_cal(wlc)) != BCME_OK) {
		return skip_test_reason;
	}

	/* Set tx and rx ant */
	wlc_set(wlc, WLC_SET_TXANT, test_arg->txant);
	wlc_set(wlc, WLC_SET_ANTDIV, test_arg->rxant);

	return BCME_OK;
}
/* Packet engine Rx */
static void
wlc_ota_start_rx_test(wlc_info_t *wlc, wl_ota_test_args_t *test_arg)
{
	wl_ota_test_vector_t * test_vctr = wlc->ota_info->test_vctr;

	/* invoke pkteng iovar */
	wlc_ota_test_start_pkteng(wlc, WL_PKTENG_PER_RX_WITH_ACK_START, test_arg, 1);

	/* Increment test phase */
	wlc->ota_info->test_phase++;

	if (wlc->ota_info->test_phase == test_vctr->test_cnt) {
		/* Exit out of test gracefully */
		wlc_ota_test_exit_engine(wlc);
	} else {
		/* Call next phase */
		wl_add_timer(wlc->wl, wlc->ota_info->test_timer, 50, 0);
	}
}
/* Tx Analysis */
static void
wlc_schedule_ota_tx_test(void *arg)
{

	wlc_info_t *wlc = (wlc_info_t *)arg;
	wl_ota_test_args_t *test_arg;
	wl_ota_test_vector_t * test_vctr = wlc->ota_info->test_vctr;

	uint8 i;
	int8 j;
	int32 txidx[2];

	test_arg = &(test_vctr->test_arg[wlc->ota_info->test_phase]);

	/* save the current rate id */
	i = wlc->ota_info->tx_test_phase;

	/* stop any packetengine running */
	wlc_ota_test_start_pkteng(wlc, WL_PKTENG_PER_TX_STOP, test_arg, 0);

	/* Set rate */
	wlc_ota_test_set_rate(wlc, test_arg, i);

	/* Loop through the target power/index */
	for (j = test_arg->pwr_info.start_pwr; j <= test_arg->pwr_info.end_pwr;
		j = j + test_arg->pwr_info.delta_pwr) {

		/* stop any packetengine running */
		wlc_ota_test_start_pkteng(wlc, WL_PKTENG_PER_TX_STOP, test_arg, 0);

		if (test_arg->pwr_info.pwr_ctrl_on == 1) {
			/* Sweep target power */
			wlc_iovar_setint(wlc, "phy_txpwrctrl", 1);
			wlc_ota_test_set_txpwr(wlc, j);
		} else if (test_arg->pwr_info.pwr_ctrl_on == 0) {
			/* Sweep target index */
			txidx[0] = j;
			txidx[1] = j;
			wlc_iovar_setint(wlc, "phy_txpwrctrl", 0);
			/* tx idx */
			wlc_iovar_op(wlc, "phy_txpwrindex", NULL, 0, txidx,
				sizeof(txidx), IOV_SET, NULL);
		} else if (test_arg->pwr_info.pwr_ctrl_on == -1) {
			/* default power */
			wlc_iovar_setint(wlc, "phy_txpwrctrl", 1);
			wlc_ota_test_set_txpwr(wlc, -1);
		}

		/* packetengine tx */
		wlc_ota_test_start_pkteng(wlc, WL_PKTENG_PER_TX_START, test_arg, 1);
		if (test_arg->pwr_info.pwr_ctrl_on == -1)
			break;
	}

	/* increment rate phase id */
	wlc->ota_info->tx_test_phase++;

	if (wlc->ota_info->tx_test_phase == test_arg->rt_info.rate_cnt) {
		/* increment test phase id */
		wlc->ota_info->test_phase++;
		wlc->ota_info->tx_test_phase = 0;
		if (wlc->ota_info->test_phase == test_vctr->test_cnt) {
			/* Exit out of test gracefully */
			wlc_ota_test_exit_engine(wlc);
		} else {
			/* Call back for next test phase */
			wl_add_timer(wlc->wl, wlc->ota_info->test_timer, 50, 0);
		}
	} else {
		/* call back for next rate phase id */
		wl_add_timer(wlc->wl, wlc->ota_info->tx_test_timer, 50, 0);
	}

}
static void wlc_schedule_ota_test(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t *)arg;
	int16 cnt = 0;
	int8 skip_test_reason = 0;
	uint8 sync_retrial;
	wl_ota_test_vector_t * test_vctr = wlc->ota_info->test_vctr;

	/* Initialize sync counters */
	wlc->ota_info->ota_sync_status = WL_OTA_SYNC_IDLE;
	wlc->ota_info->ota_sync_wait_cnt = 0;

	if (wlc->ota_info->test_phase == -1) {
		/* We shouldnt be here */
		skip_test_reason = WL_OTA_SKIP_TEST_UNKNOWN_CALL;
		goto skip_test;
	}

	/* Check if download has happened. Exit if not */
	if (test_vctr->file_dwnld_valid == FALSE) {
		skip_test_reason = WL_OTA_SKIP_TEST_FILE_DWNLD_FAIL;
		goto skip_test;
	}

	if (wlc->ota_info->test_phase == 0) {
		/* Do the init only once */
		if ((skip_test_reason = wlc_ota_test_init_seq(wlc)) != BCME_OK) {
			WL_ERROR(("Init seq failed. \n"));
			goto skip_test;
		}
	}

	sync_retrial = 0;

	/* Current test phase */
	cnt = wlc->ota_info->test_phase;

	if (test_vctr->test_cnt == 0) {
		skip_test_reason = WL_OTA_SKIP_TEST_NO_TEST_FOUND;
		goto skip_test;
	}

	/* Set the channel */
	if ((skip_test_reason = wlc_ota_test_set_chan(wlc,
		&(test_vctr->test_arg[cnt]))) != BCME_OK) {
		goto skip_test;
	}

	if (test_vctr->test_arg[cnt].wait_for_sync) {
		/* Schedule a ota sync and return */
		wl_add_timer(wlc->wl, wlc->ota_info->sync_timer, 50, 0);
		return;
	}
	/* Main test engine */
	/* Right now only two test cases either TX or RX */
	/* schedule a test and return */
	wlc_ota_test_arbitrate(wlc);
	return;
skip_test:
	if (skip_test_reason != 0) {
		/* reset all test flags */
		wlc_ota_test_engine_reset(wlc);
		/* wl down */
		wlc_set(wlc, WLC_DOWN, 1);
		WL_ERROR(("Test skipped due to reason %d \n", skip_test_reason));
	}

	/* Save the reason for skipping the test */
	wlc->ota_info->test_stage = WL_OTA_TEST_FAIL;
	wlc->ota_info->test_skip_reason = skip_test_reason;
}
static void
wlc_ota_test_arbitrate(wlc_info_t * wlc)
{
	wl_ota_test_vector_t * test_vctr = wlc->ota_info->test_vctr;
	uint8 cnt;

	/* Current test phase */
	cnt = wlc->ota_info->test_phase;

	switch (test_vctr->test_arg[cnt].cur_test) {
		case WL_OTA_TEST_TX:
			/* scheduling tx test case */
			wl_add_timer(wlc->wl, wlc->ota_info->tx_test_timer, 50, 0);
			break;
		case WL_OTA_TEST_RX:
			/* start rx test case */
			wlc_ota_start_rx_test(wlc, &(test_vctr->test_arg[cnt]));
			break;
	}
}
static void
wlc_ota_test_wait_for_sync(void *arg)
{

	wlc_info_t *wlc = (wlc_info_t *)arg;
	wl_ota_test_vector_t * test_vctr = wlc->ota_info->test_vctr;
	int8 skip_test_reason;

	wlc->ota_info->ota_sync_status = WL_OTA_SYNC_ACTIVE;
	/* Wait for sync packet */
	if ((skip_test_reason = wlc_ota_test_wait_for_sync_pkt(wlc)) != BCME_OK) {
		if (test_vctr->sync_fail_action == 1) {
			/* sync retry */
			if (wlc->ota_info->ota_sync_wait_cnt < test_vctr->sync_timeout) {
				wlc->ota_info->ota_sync_wait_cnt++;
				wl_add_timer(wlc->wl, wlc->ota_info->sync_timer, 10, 0);
			} else {
				goto sync_fail;
			}
		} else if (test_vctr->sync_fail_action == 0) {
			/* skip test */
			goto sync_fail;
		} else {
			/* continue with the test */
			wlc_ota_test_arbitrate(wlc);
		}
	} else {
		wlc_ota_test_arbitrate(wlc);
	}
	return;
sync_fail:
	/* reset all test flags */
	wlc_ota_test_engine_reset(wlc);
	/* wl down */
	wlc_set(wlc, WLC_DOWN, 1);
	WL_ERROR(("Test skipped due to reason %d \n", skip_test_reason));

	wlc->ota_info->ota_sync_status = WL_OTA_SYNC_FAIL;
	wlc->ota_info->test_stage = WL_OTA_TEST_FAIL;
	wlc->ota_info->test_skip_reason = skip_test_reason;
}
static void
wlc_ota_test_exit_engine(wlc_info_t *wlc)
{
	wl_ota_test_vector_t * test_vctr = wlc->ota_info->test_vctr;

	/* Increment loop cntr: for litepoint regression */
	wlc->ota_info->test_loop_cnt++;

	/* Reset test states */
	wlc->ota_info->test_phase = 0;
	wlc->ota_info->test_stage = WL_OTA_TEST_SUCCESS;

	/* wl down */
	wlc_statsupd(wlc);
	wlc_set(wlc, WLC_DOWN, 1);

	/* Debug feature */
	/* If loop test enabled, regress the test infinite times */
	if ((test_vctr->loop_test == -1) || (wlc->ota_info->test_loop_cnt < test_vctr->loop_test))
		wlc_ota_trigger_test_engine(wlc);
	else
		wlc->ota_info->test_loop_cnt = 0;
}

#endif /* WLOTA_EN */
