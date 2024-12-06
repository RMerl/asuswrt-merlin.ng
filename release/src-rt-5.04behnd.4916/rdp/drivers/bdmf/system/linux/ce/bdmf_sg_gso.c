/*
 * <:copyright-BRCM:2022:DUAL/GPL:standard
 * 
 *    Copyright (c) 2022 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/ip6_checksum.h>
#include <linux/ppp_defs.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/nbuff.h>
#include "bdmf_system.h"
#include "access_macros.h"
#include "bcm_mm.h"
#include <rdpa_cpu_basic.h> 
#include <net/bcm_gso.h>

#define BCM_SG_GSO_MAX_SEGS     64

#if defined(CONFIG_RUNNER_CPU_TX_FRAG_GATHER)
enum {
    BCM_SG_GSO_SEG_TYPE_SINGLE = 0,
    BCM_SG_GSO_SEG_TYPE_FIRST,
    BCM_SG_GSO_SEG_TYPE_MIDDLE,
    BCM_SG_GSO_SEG_TYPE_LAST,
    BCM_SG_GSO_SEG_TYPE_MAX
};

typedef struct rnr_gso_hdrs {
    struct {
      GSO_COMMON_HDR
    };

    /* RNR_GSO specific field */
    rnr_cpu_tx_func rnr_xmit_fn;
    pbuf_t *pbuf;
    const rdpa_cpu_tx_info_t *info;
    uint64_t skb_shadow_data[RUNNER_MAX_GSO_FRAGS];
    uint16_t skb_shadow_len[RUNNER_MAX_GSO_FRAGS];
    uint16_t pseudo_hdr_csum;
    uint16_t data_payload_len;
    uint8_t nr_frags;
    uint8_t padding;
    uint8_t l2hdrlen;
    uint8_t l3hdrlen;
        
} rnr_gso_hrds_t;

static uint32_t g_cnt_total_skb = 0;
static uint32_t g_cnt_tcp_skb = 0;
static uint32_t g_cnt_udp_skb = 0;

#undef OFFSETOF
#define OFFSETOF(stype, member) ((size_t)&((struct stype *)0)->member)

static void bdmf_sg_calc_hdr_len(struct rnr_gso_hdrs *hdrs, struct sk_buff *skb)
{
    if (hdrs->l3proto == BCM_L3_PROTO_IPV4)
    {
        hdrs->l2hdrlen = hdrs->totlen - hdrs->ipv4hdrlen - hdrs->l4hdrlen;
        hdrs->l3hdrlen = hdrs->ipv4hdrlen;
        hdrs->data_payload_len = hdrs->ip_len - hdrs->ipv4hdrlen - hdrs->l4hdrlen;
        hdrs->padding = skb->len - hdrs->l2hdrlen - hdrs->ip_len;
    }
    else if (hdrs->l3proto == BCM_L3_PROTO_IPV6)
    {
        hdrs->l2hdrlen = hdrs->totlen - hdrs->ipv6hdrlen - hdrs->l4hdrlen;
        hdrs->l3hdrlen = hdrs->ipv6hdrlen;
        hdrs->data_payload_len = hdrs->ip_len - hdrs->l4hdrlen;
        hdrs->padding = skb->len - hdrs->l2hdrlen - hdrs->ip_len - hdrs->ipv6hdrlen;
    }
    BDMF_TRACE_DBG("csum_only skb.l2hdrlen %d, l3hdrlen %d, l4hdrlen %d, data_payload_len %d, padding %d, ip_len %d, skb.len %d\n",
                hdrs->l2hdrlen, hdrs->l3hdrlen, hdrs->l4hdrlen, hdrs->data_payload_len, hdrs->padding, hdrs->ip_len, skb->len);
}

/** Function to handle single packet csum_only request
 * \param[in]   skb  system buffer
 * \param[in]   sg_desc_p  pre-allocated SG descriptor
 * \returns 0 success; negative int for error cases
 */
int bdmf_sg_csum(struct sk_buff *skb, runner_sg_desc_t *sg_desc_p)
{
    struct rnr_gso_hdrs hdrs;
    int rc = BDMF_ERR_PARM;

    /*only csum is needed */
    memset(&hdrs,0, sizeof(struct rnr_gso_hdrs));
    if ( bcm_parse_gso_hdrs(skb, (struct gso_hdrs *)&hdrs) < 0) {
        rc = BDMF_ERR_PARSE;
        goto done;
    };

    bdmf_sg_calc_hdr_len(&hdrs, skb);

    switch(hdrs.l4proto)
    {
        case IPPROTO_TCP:
        {
            struct tcphdr *th= hdrs.l4hdr;

            /* pseudo hdr fix portion (w/o length) */
            th->check = bcm_gso_l4_csum((struct gso_hdrs *)&hdrs, 0, IPPROTO_TCP, 0);
            sg_desc_p->pseudo_hdr_csum = th->check ^ 0xFFFF;
            BDMF_TRACE_DBG("Psuedo_hdr_csum: 0x%02x, in le", sg_desc_p->pseudo_hdr_csum);
            sg_desc_p->csum_sop = hdrs.l4hdr - hdrs.ethhdr;
            sg_desc_p->csum_offset = OFFSETOF(tcphdr, check);
            sg_desc_p->is_csum_offload = 1;

            /* insert len at check filed as it was not included in pseudo_hdr calculation */
            th->check = htons(hdrs.data_payload_len + hdrs.l4hdrlen);
            break;
        }

        case IPPROTO_UDP:
        {
            struct udphdr *uh= hdrs.l4hdr;

            /* pseudo hdr fix portion (w/o length) */
            uh->check = bcm_gso_l4_csum((struct gso_hdrs *)&hdrs, 0, IPPROTO_UDP, 0);
            sg_desc_p->pseudo_hdr_csum = uh->check ^ 0xFFFF;
            BDMF_TRACE_DBG("Psuedo_hdr_csum: 0x%02x, in le", sg_desc_p->pseudo_hdr_csum);
            sg_desc_p->csum_sop = hdrs.l4hdr - hdrs.ethhdr;
            sg_desc_p->csum_offset = OFFSETOF(udphdr, check);
            sg_desc_p->is_csum_offload = 1;
            sg_desc_p->is_udp = 1;

            /* insert len at check filed as it was not included in pseudo_hdr calculation */
            uh->check = htons(hdrs.data_payload_len + hdrs.l4hdrlen);
            break;
        }

        default:
            rc = BDMF_ERR_NOT_SUPPORTED;
            BDMF_TRACE_ERR("rc=%d, SW GSO not supported for protocol=%d \n", rc, hdrs.l4proto);
            goto done;
    }

    /* csum from csum_start to the end of the packet, and fill in the result at (csum_start + csum_offset) */
    /* csum_start = l4_offset, csum_offset = tcp/udp_csum offset relate to l4_offset */
    BDMF_TRACE_DBG("sg_desc: csum_offset %u, csum_sop %u, is_csum_offload %d, pseudo_hdr_csum 0x%2x\n",
            sg_desc_p->csum_offset, sg_desc_p->csum_sop, sg_desc_p->is_csum_offload, sg_desc_p->pseudo_hdr_csum);

    /* only linear header and data. Remove padding, let runner to pad */
    sg_desc_p->frag_data[0] = __swap4bytes64(VIRT_TO_PHYS(skb->data));
    sg_desc_p->frag_len[0]  = swap2bytes(skb->len - hdrs.padding);
    
    sg_desc_p->total_len = sg_desc_p->frag_len[0];
    sg_desc_p->nr_frags = 1;
    BDMF_TRACE_DBG("sg_desc: total_len %d, sg_frag %d, is_allocated %d\n", 
            swap2bytes(sg_desc_p->total_len), sg_desc_p->nr_frags, sg_desc_p->is_allocated);

    skb->ip_summed = CHECKSUM_NONE;
    return 0;

done:
    /* NOTE: can't free the original skb here, data still in skb, leave to recycle */
    return rc;
}


/** Helper functions to re-assemble segment packet header
 *  Including L2, L3, L4 (tcp) header
 */
static inline int bdmf_add_l2hdr(runner_sg_desc_t *sg, struct rnr_gso_hdrs *hdrs)
{
    bcm_gso_add_l2hdr(&sg->gso_template_hdr, &sg->len, (struct gso_hdrs *)hdrs);
    return 0;
}

static inline int bdmf_add_l3hdr(runner_sg_desc_t *sg, struct rnr_gso_hdrs *hdrs)
{
    int ret =0;

    ret = bcm_gso_add_l3hdr(&sg->gso_template_hdr, &sg->len, (struct gso_hdrs *)hdrs);
    return ret;
}

static inline void bdmf_add_tcphdr(runner_sg_desc_t *sg, struct rnr_gso_hdrs *hdrs)
{
    struct tcphdr *th;

    bcm_gso_add_tcphdr(&sg->gso_template_hdr, &sg->len, (struct gso_hdrs *)hdrs);

    th = (struct tcphdr *)sg->gso_template_hdr;

    /* Set tcp.checksum field to initial value = l4_payload_len, this is a dynamic value for each segment */
    /* after tcphdr push and payloadlen, sg.len = tcp_l4_payload_len here */
    BDMF_TRACE_DBG("Init val of tcp_csum: tcp_hdr+payload=%d, data %d, padding %d\n", sg->len, sg->len - hdrs->l4hdrlen, hdrs->padding);
    th->check = htons(sg->len);
}

static inline int bdmf_add_gre_hdr(runner_sg_desc_t *sg, struct rnr_gso_hdrs *hdrs)
{
    bcm_gso_add_gre_hdr(&sg->gso_template_hdr, &sg->len, (struct gso_hdrs *)hdrs);
    return 0;
}

static inline int bdmf_add_outer_l2hdr(runner_sg_desc_t *sg, struct rnr_gso_hdrs *hdrs)
{
    bcm_gso_add_outer_l2hdr(&sg->gso_template_hdr, &sg->len, (struct gso_hdrs *)hdrs);
    return 0;
}

static inline int bdmf_add_outer_l3hdr(runner_sg_desc_t *sg, struct rnr_gso_hdrs *hdrs)
{
    int ret =0;

    ret = bcm_gso_add_outer_l3hdr(&sg->gso_template_hdr, &sg->len, (struct gso_hdrs *)hdrs);
    return ret;
}


/** Debug dump functions
 *  Dump SG descriptor, SKB, Hdrs Parsing output
 */
static inline void dump_sg(runner_sg_desc_t *sg)
{
    int i;

    BDMF_TRACE_DBG("dump SG %p, word0: 0x%08x, word1: 0x%08x\n", sg, sg->word0, sg->word1);
    BDMF_TRACE_DBG("is_allocated %d, nr_frags %d, total_len %d\n", sg->is_allocated, sg->nr_frags, sg->total_len);
    BDMF_TRACE_DBG("is_csum_offload %d, pseudo_hdr_csum 0%x, csum_sop %d, csum_offset %d\n",
                sg->is_csum_offload, sg->pseudo_hdr_csum, sg->csum_sop, sg->csum_offset);
    BDMF_TRACE_DBG("*data_p %p, len %d\n", sg->gso_template_hdr, sg->len);

    BDMF_TRACE_DBG("Host view: sg_desc: total_len %d, sg_frag %d, csum_sop %d\n", 
                swap2bytes(sg->total_len), sg->nr_frags, sg->csum_sop);
        
    for (i = 0; i < sg->nr_frags; i++)
    {
        BDMF_TRACE_DBG("Host view: frag_data[%d]=%llx, frag_len=%d \n", i,(unsigned long long) __swap4bytes64(sg->frag_data[i]), swap2bytes(sg->frag_len[i]));
        BDMF_TRACE_DBG("frag_data[%d]=%llx, frag_len=%d \n", i, sg->frag_data[i], sg->frag_len[i]);
    }

    BDMF_TRACE_DBG("\n");
}

static inline void dump_hdrs(struct rnr_gso_hdrs *hdrs)
{
    int i;

    BDMF_TRACE_DBG("ethhdr %p, ppphdr %p, ipv4hdr %p, ipv6hdr %p, l4hdr %p\n",
            hdrs->ethhdr, hdrs->ppphdr, hdrs->ipv4hdr, hdrs->ipv6hdr, hdrs->l4hdr);
    BDMF_TRACE_DBG("ethhdrlen %d, ppphdrlen %d, ipv4hdrlen %d, ipv6hdrlen %d, l4hdrlen %d\n",
            hdrs->ethhdrlen, hdrs->ppphdrlen, hdrs->ipv4hdrlen, hdrs->ipv6hdrlen, hdrs->l4hdrlen);
    BDMF_TRACE_DBG("totlen=l2_l3_l4_hdr_len %d, l3proto %d, l4proto %d, ip_len %d, padding %d\n", 
            hdrs->totlen, hdrs->l3proto, hdrs->l4proto, hdrs->ip_len, hdrs->padding);

    for (i = 0; i < hdrs->nr_frags; i++)
        BDMF_TRACE_DBG("skb_shadow_data_array[%d].data 0x%llx, len %d\n", i, hdrs->skb_shadow_data[i], hdrs->skb_shadow_len[i]);
}

static inline void dump_skb(struct sk_buff *skb)
{
    BDMF_TRACE_DBG("skb->ip_summed = %d\n", skb->ip_summed);
    BDMF_TRACE_DBG("skb_is_gso(skb) = %d\n", skb_is_gso(skb) ? 1 : 0);
    BDMF_TRACE_DBG("skb.gso_size, mss = %d\n", skb_shinfo(skb)->gso_size);
    BDMF_TRACE_DBG("skb.nr_frags = %d\n", skb_shinfo(skb)->nr_frags);
    BDMF_TRACE_DBG("skb.len %d, skb.data_len %d, mac_len %d, skb.hdr_len %d\n",
                    skb->len, skb->data_len, skb->mac_len, skb->hdr_len);
    BDMF_TRACE_DBG("skb: hdr_len linear_in_q %d, page_len %d, page_hdr_len %d\n",
                    skb_headlen(skb), __skb_pagelen(skb), skb_pagelen(skb));

}


/** Traverse frag data list and locate next data payload
 * \param[in]   hdrs  struture with skb frag list shadow copy
 * \param[in]   request_len  number of data needs to locate
 * \param[out]  *phy_addr  physical address of located data payload
 */
uint16_t bdmf_sg_seek_data(struct rnr_gso_hdrs *hdrs, uint64_t *phy_addr, uint16_t request_len)
{
    uint16_t frag_len, i;

    for (i = 0; i < hdrs->nr_frags; i++ )
    {
        frag_len = hdrs->skb_shadow_len[i];
        if (frag_len)
        {
            request_len = min(request_len, frag_len);
            *phy_addr = hdrs->skb_shadow_data[i];
            BDMF_TRACE_DBG("Seek_data: frag %d, 0x%08llx, len %d, remain_len %d", i, hdrs->skb_shadow_data[i], request_len, hdrs->skb_shadow_len[i] - request_len);
            hdrs->skb_shadow_data[i] += request_len;
            hdrs->skb_shadow_len[i] -= request_len;
            return request_len;
        }
    }
    BDMF_TRACE_DBG("frag data underflow %d\n", request_len);
    return 0;
}

/** Create a shadow copy of SKB frag data list
 *  This copy is used for segment slicing (avoid changing on original SKB)
 * \param[in]   skb  system buffer
 * \param[out]   hdrs  struture with skb frag list shadow copy
 */
void bdmf_sg_create_skb_shadow(struct sk_buff *skb, struct rnr_gso_hdrs *hdrs)
{
    unsigned short linear_len, i, frag_idx;
    skb_frag_t *frag;
    uint8_t *vaddr;
    unsigned int bv_offset=0, bv_len=0;

    hdrs->nr_frags = 0;
    frag_idx = 0;

    /* save linear data in skb_shadow.frag[0] */
    /* Deduct out gso_template_hdr_len from skb.len at first segment */
    /* don't include padding, let runner to add padding */
    /* UDP header is part of payload and only appears once in first segment, not part of gso template header */
    /* TCP header is part of gso template header, and repeats in each segment */
    if (hdrs->l4proto == IPPROTO_UDP)
        linear_len = skb->len - hdrs->totlen + hdrs->l4hdrlen - skb->data_len - hdrs->padding;
    else
        linear_len = skb->len - hdrs->totlen - skb->data_len - hdrs->padding;

    if (linear_len)
    {
        cache_flush_len(skb->data, skb->len - skb->data_len);
        if (hdrs->l4proto == IPPROTO_UDP)
            hdrs->skb_shadow_data[frag_idx] = VIRT_TO_PHYS(skb->data + hdrs->totlen - hdrs->l4hdrlen);
        else
            hdrs->skb_shadow_data[frag_idx] = VIRT_TO_PHYS(skb->data + hdrs->totlen);
        hdrs->skb_shadow_len[frag_idx] = linear_len;
        hdrs->nr_frags += 1;
        frag_idx += 1;
    }

    /* skb frag data next */
    for (i = 0; i < skb_shinfo(skb)->nr_frags; i++ )
    {
        frag = &skb_shinfo(skb)->frags[i];
        vaddr = bdmf_kmap_skb_frag(frag);
#if defined(__KERNEL__) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
        bv_offset = frag->bv_offset;
        bv_len = frag->bv_len;
#else
        bv_offset = frag->page_offset;
        bv_len = frag->size;
#endif
        cache_flush_len((vaddr + bv_offset), bv_len);
        hdrs->skb_shadow_data[i + frag_idx]= VIRT_TO_PHYS(vaddr + bv_offset);
        hdrs->skb_shadow_len[i + frag_idx] = (uint16_t)(bv_len);
        bdmf_kunmap_skb_frag(vaddr);
        hdrs->nr_frags += 1;
    }
}

/** TCP GSO processing
 *  Main function to slice SKB into MSS segment packets and update gso header
 *  SG descriptor is allocated during processing, filled with segment data
 *  and sent out via rdpa cpu_tx ring
 * \param[in]   skb  system buffer
 * \returns 0 success; negative int for error cases
 */
static inline int bdmf_sg_gso_tcp_segment(struct sk_buff *skb, struct rnr_gso_hdrs *hdrs)
{
    int npkts, i;
    unsigned short bytesleft;   /* total payload in one SKB */
    unsigned short offset;      /* starting of payload */
    unsigned short mss;
    unsigned short payloadlen, fill_len, chunk_len;
    int cur_pkt=0;
    uint64_t phy_addr;
    runner_sg_desc_t *sg_desc_p;

    dump_hdrs(hdrs);

    /* remove padding from payload, let runner to pad */
    bytesleft = skb->len - hdrs->totlen - hdrs->padding;
    offset = hdrs->totlen;

    /* calculate the number of packets needed to transmit this skb */
    mss = skb_shinfo(skb)->gso_size;
    if (mss == 0)
    {
        mss = bytesleft;
        npkts =1;
    }
    else
        npkts = DIV_ROUND_UP(bytesleft, mss);

    if (npkts > BCM_SG_GSO_MAX_SEGS)
    {
        BDMF_TRACE_RET(BDMF_ERR_RANGE, "npkts=%d greater than max segs(%d)\n", 
                            npkts, BCM_SG_GSO_MAX_SEGS);
    }

    /* create skb frag shadow copy */
    bdmf_sg_create_skb_shadow(skb, hdrs);

    hdrs->tcp_segtype = BCM_SG_GSO_SEG_TYPE_FIRST;

    do {

        if (cur_pkt > npkts)
        {
            /* we shoud never be here,if we are then most likely there is
             * some thing wrong with mss, allocated packets are already xmitted,
             * no need to free them here
             */ 
            BDMF_TRACE_RET(BDMF_ERR_RANGE, "error pktcount =%d > allocated buffers=%d mss=%d\n",
                                    cur_pkt, npkts, mss);
        }

        sg_desc_p = bdmf_runner_sg_desc_alloc();
        if (sg_desc_p == NULL)
        {
            BDMF_TRACE_RET(BDMF_ERR_NORES, "couldn't allocate SG_Desc, cur_pkt =%d\n", cur_pkt);
        }
        if (sg_desc_p->gso_template_hdr == NULL)
        {
            bdmf_runner_sg_desc_free(sg_desc_p);
            BDMF_TRACE_RET(BDMF_ERR_INVALID_OP, "gso_template_hdr invalid, cur_pkt =%d\n", cur_pkt);
        }

        /* Fill data from original skb to new SG desc */
        sg_desc_p->pseudo_hdr_csum = hdrs->pseudo_hdr_csum;
        sg_desc_p->csum_sop = hdrs->l4hdr - hdrs->ethhdr;
        sg_desc_p->csum_offset = OFFSETOF(tcphdr, check);
        sg_desc_p->is_csum_offload = 1;

        /* First sg_frag[0] is always gso template header */
        sg_desc_p->frag_len[0]  = swap2bytes(hdrs->totlen);
        sg_desc_p->nr_frags = 1;
        sg_desc_p->total_len = hdrs->totlen;

        /* Fill data from original skb to new SG desc, budget up to mss/payloadlen */
        payloadlen = min(mss, bytesleft);                   /* Each segment payload length, = mss, except last one */
        fill_len = payloadlen;

        for (i = 1; i < RUNNER_MAX_SG_FRAGS && fill_len; i++)
        {
            chunk_len = bdmf_sg_seek_data(hdrs, &phy_addr, fill_len);
            if (chunk_len == 0)
                break;

            sg_desc_p->frag_data[i]  = __swap4bytes64(phy_addr);
            sg_desc_p->frag_len[i]  = swap2bytes(chunk_len);

            sg_desc_p->nr_frags += 1;
            sg_desc_p->total_len += chunk_len;
            fill_len -= chunk_len;
            if (fill_len == 0)
                break;
        }

        /* Recalculate real data payload len based on bytes filled into MAX_SG_FRAG arrays */
        payloadlen = payloadlen - fill_len; 

        BDMF_TRACE_DBG("hdrs totlen %d, payloadlen %d, sg_totlen %d\n",
                        hdrs->totlen, payloadlen, sg_desc_p->total_len);

        sg_desc_p->total_len = swap2bytes(sg_desc_p->total_len);

        /* start with data payload (len) and push L4/L3/L2 header */
        sg_desc_p->len = payloadlen;                        

        offset += payloadlen;
        bytesleft -= payloadlen;

        (hdrs->pbuf)->is_sg_desc = 1;
        (hdrs->pbuf)->do_not_recycle = 1;
        if (bytesleft == 0)
        {
            if (likely(hdrs->tcp_segtype != BCM_SG_GSO_SEG_TYPE_FIRST))
                hdrs->tcp_segtype = BCM_SG_GSO_SEG_TYPE_LAST;
            else
                hdrs->tcp_segtype = BCM_SG_GSO_SEG_TYPE_SINGLE;

            /* Last segment triggers SKB recycle */
            (hdrs->pbuf)->do_not_recycle = (hdrs->info)->bits.do_not_recycle;
        }

        /* copy tcp hdr & update fields */
        bdmf_add_tcphdr(sg_desc_p, hdrs);

        /* reset the seq_number and type for next segment (if any) */
        hdrs->tcpseq += payloadlen;
        hdrs->tcp_segtype = BCM_SG_GSO_SEG_TYPE_MIDDLE;

        /* copy l3 hdr  */
        bdmf_add_l3hdr(sg_desc_p, hdrs);

        /* copy l2 hdr  */
        bdmf_add_l2hdr(sg_desc_p, hdrs);

        if (hdrs->grehdr)
        {
            /* add gre hdr  */
            bdmf_add_gre_hdr(sg_desc_p, hdrs);
            sg_desc_p->csum_sop += hdrs->grehdrlen;
            
            /* copy outer l3 hdr  */
            if(hdrs->outer_ipv4hdr) {
                bdmf_add_outer_l3hdr(sg_desc_p, hdrs);
                sg_desc_p->csum_sop += hdrs->outer_ipv4hdrlen;
            }

            /* copy outer l2 hdr  */
            if(hdrs->outer_ethhdr) {
                bdmf_add_outer_l2hdr(sg_desc_p,hdrs);
                sg_desc_p->csum_sop += hdrs->outer_ethhdrlen;
            }
        }

        /* Assign SG frag[0] after building gso header */
        sg_desc_p->frag_data[0]  = __swap4bytes64(VIRT_TO_PHYS(sg_desc_p->gso_template_hdr));

        dump_sg(sg_desc_p);
        cache_flush_len(sg_desc_p, sizeof(runner_sg_desc_t));
        cache_flush_len(sg_desc_p->gso_template_hdr, RUNNER_SG_HDR_SIZE);

        /* xmit packet */
        bdmf_sg_send_desc(sg_desc_p, hdrs->rnr_xmit_fn, hdrs->pbuf, hdrs->info);

    } while (bytesleft);

    return 0;
}

/** GSO UDP is not supported (NETIF_F_GSO_UDP_L4)
 */
static inline int bdmf_sg_gso_ip_fragment(struct sk_buff *skb, struct rnr_gso_hdrs *hdrs)
{
    return BDMF_ERR_NOT_SUPPORTED;
}


/** Main entry of GSO handling
 *  Parse incoming SKB and call proper gso processing
 * \param[in]   skb  system buffer
 * \param[in]   rnr_xmit_fn  function to put SG descriptor into cpu_tx ring queue
 * \param[in]   *info  rdpa_cpu_tx info struture
 * \returns 0 success; negative int for error cases
 */
int bdmf_sg_gso(struct sk_buff *skb, rnr_cpu_tx_func rnr_xmit_fn, pbuf_t *pbuf, const rdpa_cpu_tx_info_t *info)
{
    struct rnr_gso_hdrs hdrs;
    int rc = 0;

    memset(&hdrs,0, sizeof(struct rnr_gso_hdrs));

    rc = bcm_parse_gso_hdrs(skb, (struct gso_hdrs *)&hdrs);
    if (rc)
    {
        BDMF_TRACE_RET(rc, "GSO packet parsing failed \n");
    };

    hdrs.rnr_xmit_fn = rnr_xmit_fn;
    hdrs.pbuf = pbuf;
    hdrs.info = info;

    g_cnt_total_skb++;
    BDMF_TRACE_DBG("skb.len %d, skb.data_len %d, page_len %d, skb.nr_frags %d\n",
                    skb->len, skb->data_len, __skb_pagelen(skb), skb_shinfo(skb)->nr_frags);

    bdmf_sg_calc_hdr_len(&hdrs, skb);

    switch(hdrs.l4proto)
    {
        case IPPROTO_TCP:
            /* Only need to calculate pseudo_hdr_csum once for all segments */
            hdrs.pseudo_hdr_csum = bcm_gso_l4_csum((struct gso_hdrs *)&hdrs, 0, IPPROTO_TCP, 0) ^ 0xFFFF;
            BDMF_TRACE_DBG("Psuedo_hdr_csum: 0x%02x, in le", hdrs.pseudo_hdr_csum);
            g_cnt_tcp_skb++;
            rc = bdmf_sg_gso_tcp_segment(skb, &hdrs);
            break;

        case IPPROTO_UDP:
            g_cnt_udp_skb++;
            rc = bdmf_sg_gso_ip_fragment(skb, &hdrs);
            break;

        default:
            BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "HW GSO not supported for protocol=%d \n", hdrs.l4proto); 
    }

    return rc;
}

#endif /* #if defined(CONFIG_RUNNER_CPU_TX_FRAG_GATHER) */
