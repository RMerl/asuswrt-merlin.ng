/*
 * TOF based proximity detection implementation for Broadcom 802.11 Networking Driver
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
 * $Id: wlc_tof.c 491282 2014-07-15 19:26:17Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_cfg.h>
#include <wlc_pub.h>
#include <wlc_hrt.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scan.h>
#include <wl_export.h>
#include <wlc_assoc.h>
#include <wlc_bmac.h>
#include <wlc_pcb.h>

#include <wlc_pdsvc.h>
#include <wlc_pddefs.h>
#include <wlc_pdmthd.h>
#include <wlc_fft.h>

#undef TOF_DBG

#define MAX_CORDIC32_ITER		17 /* maximum number of cordic32 iterations */
#define MAX_CORDIC32_NFRAC		17 /* Number of fractional bits in atan table */
#define CORDIC32_LOG2_PI_OVER_TWO	18 /* LOG2 PI over 2 */
#define CORDIC32_PI_OVER_TWO \
	(1<<CORDIC32_LOG2_PI_OVER_TWO) /* PI/2 as a fixed point number */
#define CORDIC32_PI			 \
	(CORDIC32_PI_OVER_TWO << 1)    /* PI as a fixed point number */
#define CORDIC32_NUM_FRAC_BITS_INTERNAL	29

#define k_log2_tof_rtd_adj_window_len	5
#define k_tof_rtd_adj_window_len	(1<<k_log2_tof_rtd_adj_window_len)

static int32 atan_tbl[MAX_CORDIC32_ITER] = {
	131072, 77376, 40884, 20753, 10417,
	5213, 2607, 1304, 652, 326, 163, 81, 41, 20, 10, 5, 3
};
/* Cordic calculation */
static int32
cordic(cint32 phasor)
{
	int32 x, y, z, prev_x, prev_y, prev_z, mu;
	int iter, signx, signy;

	x = phasor.i;
	y = phasor.q;
	z = 0;

	/* Figure out in which quadrant we are */
	signx = (x < 0)? -1 : 1;
	signy = (y <= 0)? -1 : 1;

	/* If x < 0, negate x and y (rotate 180 degrees) */
	/* This rotates into the 1st or 4th quadrant */
	/* CORDIC only works in 1st and 4th quadrant */
	x = signx * x;
	y = signx * y;

	/* CORDIC iteration */
	iter = 0;
	while (iter < MAX_CORDIC32_ITER)
	{
		prev_x = x;
		prev_y = y;
		prev_z = z;
		mu = (y < 0)? 1 : -1;
		z = prev_z - mu * atan_tbl[iter];
		x = prev_x - mu * ROUND(prev_y, iter);
		y = prev_y + mu * ROUND(prev_x, iter);
		x = LIMIT(x, -(1<<CORDIC32_NUM_FRAC_BITS_INTERNAL),
			(1<<CORDIC32_NUM_FRAC_BITS_INTERNAL)-1);
		y = LIMIT(y, -(1<<CORDIC32_NUM_FRAC_BITS_INTERNAL),
			(1<<CORDIC32_NUM_FRAC_BITS_INTERNAL)-1);
		iter++;
	}

	/* If in 2nd quadrant, output angle is (z + pi). */
	/* If in 3rd quadrant, output angle is (z - pi). */
	/* Otherwise, output angle is z. */
	if (signx < 0)
		z = z + CORDIC32_PI * signy;

	/* Limit angle to [-pi, pi) */
	z = LIMIT(z, -CORDIC32_PI, CORDIC32_PI);

	return z;
}
/* channel smoothing threshold adj calculation */
int
tof_rtd_adj(wlc_info_t *wlc, struct tof_rtd_adj_params *params)
{
	int32  tmp, theta, gd, gd_adj_ns, h_ts;
	cint32 acc, *H;
	int i, i_l, n, nfft, log2_nfft, nfft_over_2, rshift, ret = 0, adj_ns;

	if (params->bw == TOF_BW_20MHZ) {
		rshift = TOF_BW_20MHZ_INDEX;
		h_ts = (Q1_NS << 2);
		log2_nfft = 6;
		i_l = 28;       /* for legacy this is 26, for vht-20 this is 28 */
	} else if (params->bw == TOF_BW_40MHZ) {
		rshift = TOF_BW_40MHZ_INDEX;
		h_ts = (Q1_NS << 1);
		log2_nfft = 7;
		i_l = 58;
	} else if (params->bw == TOF_BW_80MHZ) {
		rshift = TOF_BW_80MHZ_INDEX;
		h_ts = Q1_NS;
		log2_nfft = 8;
		i_l = 122;
	} else {
		return BCME_ERROR;
	}
	nfft = (1 << log2_nfft);
	H = NULL;

	if (params->w_ext) {
		/* HW */
		gd_adj_ns = params->gd_ns; /* Q1 */
		params->gd_ns = (gd_adj_ns + 1) >> 1;
		params->gd_shift = FALSE;
	} else {
		  /* SW */
		nfft_over_2 = (nfft >> 1);
		H = (cint32*)params->H;

		/* undo rotation */
		if (params->bw == TOF_BW_40MHZ) {
			for (i = 0; i < TOF_NFFT_40MHZ; i++) {
				tmp = H[i].i;
				H[i].i = H[i].q;
				H[i].q = -tmp;
			}
		} else if (params->bw == TOF_BW_80MHZ) {
			for (i = 0; i < NFFT_BASE; i++) {
				H[i].i = -H[i].i;
				H[i].q = -H[i].q;
			}
		}

		/* group delay */
		acc.i = 0; acc.q = 0;
		n = (i_l << 1);
		i = nfft_over_2 - i_l;
		while (n-- > 0) {
			acc.i += ((H[i].i * H[i+1].i) + (H[i].q * H[i+1].q)) >> rshift;
			acc.q += ((H[i].q * H[i+1].i) - (H[i].i * H[i+1].q)) >> rshift;
			i++;
		}
		tmp = ((acc.i >> 28) ^ (acc.q >> 28)) & 0xf;
		if ((tmp != 0) && (tmp != 0xf)) {
			acc.i = acc.i >> 4;
			acc.q = acc.q >> 4;
		}
		theta = cordic(acc);
		gd = theta * nfft;
		gd_adj_ns = gd;
		gd = (gd >> (CORDIC32_LOG2_PI_OVER_TWO+2));
		params->gd = gd;
		tmp = (gd_adj_ns >> 28) & 0xf;
		if ((tmp != 0) && (tmp != 0xf)) {
			tmp = (gd_adj_ns >> 4) * h_ts;
			gd_adj_ns = ROUND(tmp, CORDIC32_LOG2_PI_OVER_TWO+2+4+1);
		} else {
			tmp = gd_adj_ns * h_ts;
			gd_adj_ns = ROUND(tmp, CORDIC32_LOG2_PI_OVER_TWO+2+1);
		}
		params->gd_ns = gd_adj_ns;
		gd_adj_ns = gd * h_ts;  /* Q1 */
		params->gd_shift = TRUE;
		if (params->bw == TOF_BW_80MHZ) {
			/* output is time reversed impulse response */
			ret = FFT256(wlc->osh, H, H);
		} else if (params->bw == TOF_BW_40MHZ) {
			/* output is time reversed impulse response */
			ret = FFT128(wlc->osh, H, H);
		} else {
			/* output is time reversed impulse response */
			ret = FFT64(wlc->osh, H, H);
		}

		if (ret == BCME_NOMEM)
		{
			return ret;
		}

#ifdef RSSI_REFINE
		for (i = 0; i < nfft && params->p_A; i++) {
			tmp1 = H[i].i;
			tmp2 = H[i].q;
			tmp = tmp1 * tmp1 + tmp2 * tmp2;
			params->p_A[i] = tmp;
		}
#endif // endif

		}

	ret = wlapi_tof_pdp_ts(log2_nfft, (void*)H, params->bw, 1,
		(void*)params, &tmp, &adj_ns);
	if (ret != BCME_OK)
		return ret;
	adj_ns = (adj_ns / 5); /* return is in 0.1ns, need to make Q1 */
	gd_adj_ns += adj_ns;
	gd_adj_ns = (gd_adj_ns + 1) >> 1;
	params->adj_ns = gd_adj_ns;

	return BCME_OK;
	}

#define k_tof_w_Q 6
#define k_tof_w_ts_mult 10000

static int32 tof_pdp_thresh_crossing(int start, int end, uint32* pW, uint32 thresh,
	uint32 overflow_mask, int check)
{
	int k, check_start = -1, check_end;
	uint32 crossing = 0;

	if (end < 0)
		end = 0;

	k = start;
	while (k > end) {
		if ((pW[k] >= thresh) && (pW[k-1] < thresh)) {
			uint32 d1, d0;
			int shft = 0;

			/* Scaling to avoid overflow */
			d1 = pW[k] - pW[k-1];
			d0 = thresh - pW[k-1];
			while ((shft < 31) && (d1 & overflow_mask)) {
				shft++;
				d0 >>= 1;
				d1 >>= 1;
			};
			check_start = k-1;
			crossing = (((k - 1)*d1 + d0) << k_tof_w_Q)/d1;
		}
		k--;
	}

	if (check > 0) {
		if ((check_start > 0)) {
			check_end = check_start - check;
			if (check_end < 0)
				check_end = 0;
			while (check_start >= check_end) {
				if (pW[check_start] > (1*thresh >> 0))
					crossing = 0;
				check_start--;
			};
		} else {
			crossing = 0;
		}
	}

	return crossing;
}

int tof_pdp_ts(int log2n, void* pBuf, int FsMHz, int rx, void* pparams,
	int32* p_ts_thresh, int32* p_thresh_adj)
{
	int i, k;
	int n, mask = 0, pmax = 0;
	uint32 *pW, *pWr, max = 0, thresh;
	cint32* pIn = (cint32*)pBuf;
	int wpos;
	int wlen = ((struct tof_rtd_adj_params *)pparams)->w_len;
	int wzero = ((struct tof_rtd_adj_params *)pparams)->w_offset;
	int thresh_scale = ((struct tof_rtd_adj_params *)pparams)->thresh_scale[rx];
	int thresh_log2 = ((struct tof_rtd_adj_params *)pparams)->thresh_log2[rx];
	int wshift;
	uint32 overflow_mask, tmp;
	uint32 th0, th1;

	n = (1 << log2n);
	if (((struct tof_rtd_adj_params *)pparams)->w_ext) {
		pW = (uint32*)((struct tof_rtd_adj_params *)pparams)->w_ext;
		pWr = NULL;
	} else if (pIn) {
		/* Mag sqrd */
		mask = (n - 1);
		pWr = (uint32*)pIn;
		for (i = 0; i < n; i++) {
			int32 di, dq;
			uint32 d;

			di = pIn->i;
			dq = pIn->q;
			d = (uint32)(di * di + dq * dq);
			if (d >= max) {
				pmax = i;
				max = d;
			}
			*pWr++ = d;
			pIn++;
		}
		pW = pWr;
		pWr -= n;
	} else {
		return BCME_ERROR;
	}

	if (((struct tof_rtd_adj_params *)pparams)->gd_shift) {
		wshift = -(((struct tof_rtd_adj_params *)pparams)->gd);
	} else {
		wshift = pmax;
	}
	wpos = (-(wshift + wzero)) & mask;

	/* Position window to include leading edge of impulse response */
	do {
		max = 0;
		for (i = wshift + wzero, k = 0; k < wlen; i--, k++) {
			if (pWr)
				pW[k] = pWr[(i & mask)];
			if (pW[k] >= max) {
				max = pW[k];
				pmax = k;
			}
		}
		thresh = ((uint32)thresh_scale*max) >> thresh_log2;
		if (pW[0] < (thresh - (thresh >> 1)))
			break;
		wzero++;
		wpos--;
	} while (wzero < wlen);
	if (pW[0] >= thresh)
		return BCME_ERROR; /* Window likely doesnt include leading edge */

	/* Threshold crossing in window */
	wzero = (wzero - 1) << k_tof_w_Q;
	wpos  <<= k_tof_w_Q;
	overflow_mask = 0xffffffff;
	overflow_mask <<= (32 - log2n - k_tof_w_Q);
	while (n > wlen) {
		overflow_mask <<= 1;
		wlen <<= 1;
	}

	th0 = th1 = 0;
	if (rx && (FsMHz == 160) && 1) {
		int w_u, w_l;

		th0 = tof_pdp_thresh_crossing(pmax, 0, pW, (thresh<<1), overflow_mask, 0);

		w_u = (th0 >> k_tof_w_Q) + 1;
		w_l = w_u - 4;
		th1 = tof_pdp_thresh_crossing(w_u, w_l, pW, (thresh), overflow_mask, 0);

		tmp = th1;
	} else {
		tmp = tof_pdp_thresh_crossing(pmax, 0, pW, (thresh), overflow_mask, 0);
	}

	*p_ts_thresh  = (k_tof_w_ts_mult*((int32)tmp + (int32)wpos))/(FsMHz << k_tof_w_Q);
	if (p_thresh_adj)
		*p_thresh_adj = (k_tof_w_ts_mult*((int32)tmp - (int32)wzero))/(FsMHz << k_tof_w_Q);

	return BCME_OK;
}

#ifdef RSSI_REFINE
#define RSSI_VHTSCALE	100
#define RSSI_NFFT_RATIO	(RSSI_VHTSCALE*256/8)
/* Find crossing point */
int32 find_crossing(int32* T, int max, int nfft, uint32 threshold)
{
	int i;
	uint32 vhigh, vth, z, rt;
	int32 delta;

	threshold = (1 << (TOF_MAXSCALE - threshold));
	for (i = 0; i < nfft; i++) {
		if (T[i] > threshold)
			break;
	}
	if (i == nfft || i == 0) i = 1;

	vhigh = T[i] - T[i-1];
	vth = threshold - T[i-1];
	z = vhigh / 10000000;
	if (z > 1) {
		vhigh /= z;
		vth /= z;
	}

	rt = (vth *RSSI_VHTSCALE) / vhigh;
	rt += RSSI_VHTSCALE * (i-1);
	delta = (nfft * RSSI_VHTSCALE / 2) - rt;
	/* rounding up to the nearest integer */
	delta = (delta * RSSI_NFFT_RATIO)/nfft;

	return delta;
}
#endif /* RSSI_REFINE */
