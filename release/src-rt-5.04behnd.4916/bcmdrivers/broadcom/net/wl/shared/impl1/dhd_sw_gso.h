/*
    Copyright (c) 2021 Broadcom
    All Rights Reserved

    <:label-BRCM:2021:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>
*/


/**
 * =============================================================================
 *
 * DHD GSO Packet Datapath
 *
 * =============================================================================
 */

#if defined(BCM_CPE_DHD_GSO)

#if defined(BCM_PKTFWD)
#define SW_GSO_PKTLIST //Depend on PKTFWD
#endif /* BCM_AWL && BCM_PKTFWD */

#if defined(SW_GSO_PKTLIST)
typedef enum
{
	FLUSH_NONE = 0,
	FLUSH_DIFF_FLOW,
	FLUSH_ENOUGH_PKT,
	FLUSH_ALWAYS,
	FLUSH_FORCE,
	FLUSH_IDLETIME_LIMIT
} FLUSH_CAUSE_CODE;

/* GSO Priv buff API */
void *dhd_gso_priv_buf_get(void);
void dhd_gso_priv_buf_free(void *ptr);
uint16 dhd_gsopriv_get_flowid(void *ptr);
void dhd_gsopriv_set_flowid(void *ptr, uint16 flowid);
unsigned long dhd_gsopriv_get_mark(void *ptr);
void dhd_gsopriv_set_mark(void *ptr, unsigned long mark);

/* GSO PKTLIST API */
int dhd_gsopktlist_init(uint radio_idx);
int dhd_gso_enq_pktlist(struct net_device *net_device, void *dhd_gso_priv, int radio_idx);
void dhd_wake_gsolist_task(int radio_idx);
int dhd_bcmgsolist_out(struct sk_buff *nbuff, struct net_device *net);
#endif /* SW_GSO_PKTLIST */

int dhd_bcmgso_out(struct sk_buff *nbuff, struct net_device *net);

/* bcm_sw_gso API */
extern int bcm_sw_gso_xmit(struct sk_buff *skb, struct net_device *dev, HardStartXmitFuncP xmit_fn);
extern int
bcm_gso_enqueue(struct sk_buff *skb, struct net_device *dev, HardStartXmitFuncP skb_handle_fn);
extern int
bcm_sw_gso_xmit_classic(struct sk_buff *skb, struct net_device *dev, HardStartXmitFuncP xmit_fn);
#define bcm_sw_gso_xmit bcm_sw_gso_xmit_classic

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
#define AWL_LOCAL_IN_BYPASS_FILTER
extern unsigned int local_in_bypass_filter(void *pkt);
extern int awl_localin_filter_proc_init(void);
extern int awl_localin_filter_proc_fini(void);
#endif

#endif /* BCM_CPE_DHD_GSO */
