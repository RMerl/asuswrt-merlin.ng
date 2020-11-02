/*
 * L2 Filter handling functions
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
 * $Id: l2_filter.h 473138 2014-04-30 14:01:06Z $
 *
 */
#ifndef _l2_filter_h_
#define _l2_filter_h_

/* Proxy ARP processing return values */
#define PARP_DROP			0
#define PARP_NOP			1
#define PARP_TAKEN			2
/* Adjust for ETHER_HDR_LEN pull in linux
 * which makes pkt nonaligned
 */
#define ALIGN_ADJ_BUFLEN		2

#define	BCM_PARP_TABLE_SIZE		32		/* proxyarp hash table bucket size */
#define	BCM_PARP_TABLE_MASK		0x1f	/* proxyarp hash table index mask */
#define	BCM_PARP_TABLE_INDEX(val)	(val & BCM_PARP_TABLE_MASK)
#define	BCM_PARP_TIMEOUT		600	/* proxyarp cache entry timerout duration(10 min) */

#define BCM_PARP_IS_TIMEOUT(pub_tick, entry)	\
			(pub_tick - entry->used > BCM_PARP_TIMEOUT)

#define	BCM_PARP_ANNOUNCE_WAIT		2	/* proxyarp announce wait duration(2 sec) */

#define BCM_PARP_ANNOUNCE_WAIT_REACH(pub_tick, entry) \
	(pub_tick - entry->used > BCM_PARP_ANNOUNCE_WAIT)

#define BCM_ARP_TABLE_UPDATE_TIMEOUT	100

typedef struct parp_entry {
	struct parp_entry	*next;
	uint32			used;		/* time stamp */
	struct ether_addr	ea;
	bcm_tlv_t		ip;
} parp_entry_t;

typedef struct arp_table arp_table_t;

extern int bcm_l2_filter_gratuitous_arp(osl_t *osh, void *pktbuf);
extern int bcm_l2_filter_block_ping(osl_t *osh, void *pktbuf);
extern int bcm_l2_filter_get_mac_addr_dhcp_pkt(osl_t *osh, void *pktbuf,
	int ifidx, uint8** addr);

arp_table_t* init_l2_filter_arp_table(osl_t* osh);
void deinit_l2_filter_arp_table(osl_t* osh, arp_table_t* ptable);
int get_pkt_ether_type(osl_t *osh, void *skb, uint8 **data_ptr,
	int *len_ptr, uint16 *et_ptr, bool *snap_ptr);
int get_pkt_ip_type(osl_t *osh, void *pktbuf,
	uint8 **data_ptr, int *len_ptr, uint8 *prot_ptr);
int bcm_l2_filter_parp_addentry(osl_t *osh, arp_table_t* arp_tbl, struct ether_addr *ea,
	uint8 *ip, uint8 ip_ver, bool cached, unsigned int entry_tickcnt);
int bcm_l2_filter_parp_delentry(osl_t *osh, arp_table_t* arp_tbl, struct ether_addr *ea,
	uint8 *ip, uint8 ip_ver, bool cached);
parp_entry_t *bcm_l2_filter_parp_findentry(arp_table_t* arp_tbl, uint8 *ip,
	uint8 ip_ver, bool cached, unsigned int entry_tickcnt);

int bcm_l2_filter_parp_modifyentry(arp_table_t* arp_tbl, struct ether_addr *ea,
	uint8 *ip, uint8 ip_ver, bool cached, unsigned int entry_tickcnt);
extern void bcm_l2_filter_arp_table_update(osl_t *osh, arp_table_t* arp_tbl, bool all,
	uint8 *del_ea, bool periodic, unsigned int tickcnt);

void *bcm_l2_filter_proxyarp_alloc_reply(osl_t* osh, uint16 pktlen, struct ether_addr *src_ea,
	struct ether_addr *dst_ea, uint16 ea_type, bool snap, void **p);
void bcm_l2_filter_parp_get_smac(arp_table_t* ptable, void* smac);
void bcm_l2_filter_parp_get_cmac(arp_table_t* ptable, void* cmac);
void bcm_l2_filter_parp_set_smac(arp_table_t* ptable, void* smac);
void bcm_l2_filter_parp_set_cmac(arp_table_t* ptable, void* cmac);
bcm_tlv_t* parse_nd_options(void *buf, int buflen, uint key);
uint16 calc_checksum(uint8 *src_ipa, uint8 *dst_ipa, uint32 ul_len, uint8 prot, uint8 *ul_data);
#endif /* _l2_filter_h */
