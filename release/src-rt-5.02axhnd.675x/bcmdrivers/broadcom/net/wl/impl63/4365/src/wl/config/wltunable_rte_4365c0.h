/*
 * Broadcom 802.11 Networking Device Driver Configuration file for 4365c0
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
 * $Id: wltunable_rte_4365c0.h $:
 *
 * wl driver tunables for 4365c0 RTE dev
 *
 */

#define	D11CONF		0
#define	D11CONF2	0
#define	D11CONF3	0x00000002	/* D11 Core Rev 65 */
#define ACCONF		0
#define ACCONF2		(1<<1)		/* AC-Phy rev 33 */

#define NTXD		512
#define NRXD		256
#ifndef NRXBUFPOST
#define NRXBUFPOST	64
#endif // endif
#define WLC_DATAHIWAT	50		/* NIC: 50 */
#define WLC_AMPDUDATAHIWAT	128	/* NIC: 128 */
#define RXBND		48
#define WLC_MAX_UCODE_BSS	8	/* Max BSS supported */
#define WLC_MAXBSSCFG	8
#define WLC_MAXDPT	1
#define WLC_MAXTDLS	5
#ifdef MINPKTPOOL
#define MAXSCB		64
#else
#define MAXSCB		64 /* (WLC_MAXBSSCFG + WLC_MAXDPT + WLC_MAXTDLS), NIC:128 */
#endif // endif
#define AIDMAPSZ	32

#ifndef AMPDU_RX_BA_DEF_WSIZE
#define AMPDU_RX_BA_DEF_WSIZE	64 /* Default value to be overridden for dongle */
#endif // endif

#define PKTCBND			RXBND
#define PKTCBND_AC3X3		RXBND
#define NTXD_LARGE_AC3X3	NTXD
#define NRXD_LARGE_AC3X3	NRXD
#define RXBND_LARGE_AC3X3	RXBND
#define NRXBUFPOST_LARGE_AC3X3	NRXBUFPOST

#define NTXD_LFRAG		1024

/* IE MGMT tunables */
#define MAXIEREGS		8
#define MAXVSIEBUILDCBS		96
#define MAXIEPARSECBS		98
#define MAXVSIEPARSECBS		64

/* Module and cubby tunables */
#define MAXBSSCFGCUBBIES	36	/* max number of cubbies in bsscfg container */
#define WLC_MAXMODULES		78	/* max #  wlc_module_register() calls */
