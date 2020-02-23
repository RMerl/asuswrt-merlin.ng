/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

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

#if defined(BCM_NBUFF)

#include <linux/kernel.h>
#include <linux/nbuff.h>
#include <osl_decl.h>
#include <bcmutils.h>

#include <nbuff_pkt.h>

#ifndef PRIO_LOC_NFMARK
#define PRIO_LOC_NFMARK 16
#endif

/* GENERIC NBUFF */
uint
nbuff_pktprio(void *pkt)
{
	uint32 prio = 0;
	if (IS_SKBUFF_PTR(pkt)) {
		prio = ((struct sk_buff*)pkt)->mark>>PRIO_LOC_NFMARK & 0x7;
	} else { /* manuplated in dhd, 3bit prio + 10bit flowid */
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		prio = fkb_p->wl.ucast.dhd.wl_prio; /* Same bit pos used for mcast */
	}

	if (prio > 7) {
		prio = 0;
	}

	return prio;
}

void
nbuff_pktsetprio(void *pkt, uint x)
{
	if (IS_SKBUFF_PTR(pkt)) {
		((struct sk_buff*)pkt)->mark &= ~(0x7 << PRIO_LOC_NFMARK);
		((struct sk_buff*)pkt)->mark |= (x & 0x7) << PRIO_LOC_NFMARK;
	} else {
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		fkb_p->wl.ucast.dhd.wl_prio = x; /* Same bit pos used for mcast */
	}
}

uchar*
nbuff_pktdata(osl_t *osh, void *pkt)
{
	BCM_REFERENCE(osh);
	return nbuff_get_data((pNBuff_t)pkt);
}

uint
nbuff_pktlen(osl_t *osh, void *pkt)
{
	BCM_REFERENCE(osh);
	return nbuff_get_len((pNBuff_t)pkt);
}

void
nbuff_pktsetlen(osl_t *osh, void *pkt, uint len)
{
	BCM_REFERENCE(osh);
	if (IS_SKBUFF_PTR((pNBuff_t)pkt))
		__pskb_trim((struct sk_buff*)pkt, len);
	/* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
	else
		nbuff_set_len((pNBuff_t)pkt, len);
}

uint
nbuff_pktheadroom(osl_t *osh, void *pkt)
{
	BCM_REFERENCE(osh);

	if (IS_FKBUFF_PTR(pkt))
		return (uint) fkb_headroom((FkBuff_t *)PNBUFF_2_PBUF(pkt));
	else
		return (uint) skb_headroom((struct sk_buff *) pkt);
}

uint
nbuff_pkttailroom(osl_t *osh, void *pkt)
{
	BCM_REFERENCE(osh);

	if (IS_FKBUFF_PTR(pkt)) {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		return 0;
	} else {
		return (uint) skb_tailroom((struct sk_buff *) pkt);
	}
}

uchar*
nbuff_pktpush(osl_t *osh, void *pkt, int bytes)
{
	BCM_REFERENCE(osh);
	return (nbuff_push(pkt, bytes));
}

uchar*
nbuff_pktpull(osl_t *osh, void *pkt, int bytes)
{
	BCM_REFERENCE(osh);
	return (nbuff_pull(pkt, bytes));
}

bool
nbuff_pktshared(void *pkt)
{
	if (IS_SKBUFF_PTR(pkt)) {
		return (((struct sk_buff*)pkt)->cloned);
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		return FALSE;
	}
}

void *
nbuff_pktlink(void *pkt)
{
	if (IS_FKBUFF_PTR(pkt))
		return nbuff_get_queue(pkt);
	else
		return (((struct sk_buff*)(pkt))->prev);
}

void
nbuff_pktsetlink(void *pkt, void *x)
{
	if (IS_FKBUFF_PTR(pkt))
		nbuff_set_queue(pkt, x);
	else
		(((struct sk_buff*)(pkt))->prev = (struct sk_buff*)(x));
}

void
nbuff_pktsetnext(osl_t *osh, void *pkt, void *x)
{
	BCM_REFERENCE(osh);
	if (IS_FKBUFF_PTR(pkt))
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
	else
		(((struct sk_buff*)(pkt))->next = (struct sk_buff*)(x));
}

void *
nbuff_pktnext(osl_t *osh, void *pkt)
{
	BCM_REFERENCE(osh);
	if (IS_FKBUFF_PTR(pkt)) {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);

		return NULL;
	} else {
		return (((struct sk_buff*)(pkt))->next);
	}
}

void *
nbuff_pktdup(osl_t *osh, void *pkt)
{
	void *p = NULL;

	if (IS_SKBUFF_PTR(pkt)) {
        skb_bpm_tainted((struct sk_buff*)pkt); // clear SKB_BPM_PRISTINE, dirty
		p = osl_pktdup(osh, pkt);
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
	}
	return p;
}

void *
nbuff_pktdup_cpy(osl_t *osh, void *pkt)
{
	void *p = NULL;

	if (IS_SKBUFF_PTR(pkt)) {
        skb_bpm_tainted((struct sk_buff*)pkt); // clear SKB_BPM_PRISTINE, dirty
		p = osl_pktdup_cpy(osh, pkt);
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
	}
	return p;
}

void BCMFASTPATH
nbuff_pktfree(osl_t *osh, void *pkt, bool send)
{
	if (IS_SKBUFF_PTR(pkt)) {
#ifdef BCM_NBUFF_PKT
		linux_pktfree(osh, pkt, send);
#else
		osl_pktfree(osh, pkt, send);
#endif
	} else {
		nbuff_free((pNBuff_t)pkt);
	}
	return;
}

struct sk_buff *
nbuff_pkt_tonative(osl_t *osh, void *pkt)
{
	struct sk_buff *p;

	if (IS_SKBUFF_PTR(pkt))
		p = osl_pkt_tonative(osh, pkt);
	else
		p = (struct sk_buff *)pkt;

	return p;
}

void *
nbuff_pkt_frmnative(osl_t *osh, void *pkt)
{
	struct sk_buff *p;

	if (IS_SKBUFF_PTR(pkt))
		p = osl_pkt_frmnative(osh, pkt);
	else
		p = (struct sk_buff *)pkt;

	return p;
}

void inline *nbuff_pkt_get_tag(void *pkt)
{
	if (IS_SKBUFF_PTR(pkt)) {
		return  ((void *)(((struct sk_buff*)(pkt))->cb));
	} else {
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		return ((void *)(PFKBUFF_TO_PHEAD(fkb_p)));
	}
}

void inline nbuff_pkt_clear_tag(void *pkt)
{
        void *tag = nbuff_pkt_get_tag(pkt);

        *(uint32 *)(tag) = 0;
        *(uint32 *)(tag+4) = 0;
        *(uint32 *)(tag+8) = 0;
        *(uint32 *)(tag+12) = 0;
        *(uint32 *)(tag+16) = 0;
        *(uint32 *)(tag+20) = 0;
        *(uint32 *)(tag+24) = 0;
        *(uint32 *)(tag+28) = 0;
}

#endif /* BCM_NBUFF */
