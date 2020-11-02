/*
 * MAC Offload Module Interface
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
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
 * $Id: wlc_macol.h 708017 2017-06-29 14:11:45Z $
 */
#ifndef _WLC_MACOL_H_
#define _WLC_MACOL_H_

#define DEFAULT_KEYS			4

/* security */
#define RC4_STATE_NBYTES		256
#define WLC_AES_EXTENDED_PACKET		(1 << 5)
#define WLC_AES_OCB_IV_MAX		((1 << 28) - 3)
typedef struct tx_info tx_info_t;
typedef struct sec_info sec_info_t;
typedef struct macol_info macol_info_t;

typedef struct macol_tkip_info {
	uint16      phase1[TKHASH_P1_KEY_SIZE/sizeof(uint16)];  /* tkhash phase1 result */
	uint8       phase2[TKHASH_P2_KEY_SIZE];         /* tkhash phase2 result */
	uint8       PAD[2];
	uint32      micl;
	uint32      micr;
} macol_tkip_info_t;

typedef struct tkip_iv {
    uint32      hi; /* upper 32 bits of IV */
    uint16      lo; /* lower 16 bits of IV */
    uint16      PAD;
} macol_iv_t;

struct sec_info {
	uint8		idx;		/* key index in wsec_keys array */
	uint8		id;		/* key ID [0-3] */
	uint8		algo;		/* CRYPTO_ALGO_AES_CCM, CRYPTO_ALGO_WEP128, etc */
	uint8 		algo_hw;	/* cache for hw register */
	int8		iv_len;		/* IV length */
	int8 		icv_len;
	ol_iv_t		txiv;		/* Tx IV */
	uint8		data[DOT11_MAX_KEY_SIZE];	/* key data */
	macol_iv_t		rxiv[WLC_KEY_NUM_RX_SEQ];	/* Rx IV (one per TID) */
	macol_tkip_info_t	tkip_tx;
	macol_tkip_info_t	tkip_rx;	/* tkip receive state */
};

struct tx_info {
	uint		TX;
	sec_info_t	key;
	sec_info_t	defaultkeys[DEFAULT_KEYS];
	uint8		qos;
	uint8		hwmic;
	uint16		rate;
	uint16      	seqnum;
	uint16		PhyTxControlWord_0;
	uint16		PhyTxControlWord_1;
	uint16		PhyTxControlWord_2;
	struct ether_addr	BSSID;
};

#define MAX_FRAME_PENDING	30

struct macol_info {
	wlc_hw_info_t 	*hw;
	wlc_pub_t       *pub;
	osl_t		*osh;		/* pointer to os handle */
	uint		unit;		/* device instance number */

	tx_info_t	txinfo;
	sec_info_t	secinfo;
	uint32		counter;

	uint16		frameid[MAX_FRAME_PENDING];
	uint16		frame_pend;

	uint8		txchain;
	uint8		rxchain;

	uint32		bcn_count;
	bool		bcn_filter_enable;

	uint8		wakeinterval;

	bool		rssi_low_indicated;
	uint32		rxbcn;
	uint32		mode;

	uint		nmulticast;		/* # enabled multicast addresses */

	uint32		tbtt_thresh;

	bool		bss_htcapable;
	ht_cap_ie_t	bss_htcap;
	ht_add_ie_t	bss_htinfo;

	/* rxdrops */
	uint32		rxdrops;

	wlc_bsscfg_t	*bsscfg;
	struct ether_addr	*multicast; 	/* ptr to list of multicast addresses */
	struct ether_addr	cur_etheraddr;
};

extern void BCMATTACHFN(wlc_macol_attach)(wlc_hw_info_t *wlc_hw, int *err);
extern void BCMATTACHFN(wlc_macol_detach)(wlc_hw_info_t *wlc_hw);
extern uint16 wlc_macol_d11hdrs(macol_info_t *macol, void *p, ratespec_t rspec, uint16 fifo);
extern void wlc_macol_frameid_add(macol_info_t *macol, uint16 frameid);
extern int wlc_macol_chain_set(macol_info_t *macol, uint8 txchain, uint8 rxchain);
extern void wlc_macol_intr_enable(wlc_hw_info_t *wlc_hw, uint bit);
extern void wlc_macol_intr_disable(wlc_hw_info_t *wlc_hw, uint bit);
extern void macol_print_txstatus(tx_status_t* txs);
extern void wlc_macol_set_shmem_coremask(wlc_hw_info_t *wlc_hw);
extern int wlc_macol_pso_shmem_upd(wlc_hw_info_t *wlc_hw);
#endif /* _WLC_MACOL_H_ */
