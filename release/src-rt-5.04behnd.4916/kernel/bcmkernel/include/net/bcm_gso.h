/*
* <:copyright-BRCM:2023:DUAL/GPL:standard
* 
*    Copyright (c) 2023 Broadcom 
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
:>
*/
#ifndef _BCM_GSO_H
#define _BCM_GSO_H

#include <linux/nbuff.h>
#include <net/ip6_checksum.h>
#include <linux/ppp_defs.h>
#include <net/ip_tunnels.h>
#include <net/gre.h>
#include <linux/version.h>

#define BCM_SW_GSO_MAX_HEADERS  16
#define BCM_SW_GSO_MAX_SEGS     64
#define BCM_L3_PROTO_IPV4       1
#define BCM_L3_PROTO_IPV6       2

enum {
    BCM_SW_GSO_SEG_TYPE_SINGLE=0,
    BCM_SW_GSO_SEG_TYPE_FIRST,
    BCM_SW_GSO_SEG_TYPE_MIDDLE,
    BCM_SW_GSO_SEG_TYPE_LAST,
    BCM_SW_GSO_SEG_TYPE_MAX
};

#define GSO_COMMON_HDR \
    void *ethhdr; /*including VLAN's */ \
    void *ppphdr; \
    void *ipv4hdr; \
    void *ipv6hdr; \
    void *l4hdr; \
    union { \
        struct { \
            uint8_t ethhdrlen;/*including VLAN's */ \
            uint8_t ppphdrlen; \
            uint8_t ipv4hdrlen; \
            uint8_t ipv6hdrlen; \
            uint8_t l4hdrlen; \
            uint8_t totlen;   /* total header len, including L2, ppp, L3, L4 */ \
            uint8_t l3proto; \
            uint8_t l4proto; \
        }; \
        uint64_t lenproto; \
    }; \
    uint32_t ipv6_flowlbl; \
    uint32_t tcpseq; \
    uint32_t ipv6_fragid; \
    uint16_t ipv4_id; \
    uint16_t ip_len; \
    uint16_t ip_fragoff; \
    uint8_t ip_fragneeded; \
    uint8_t ip_lastfrag; \
    uint8_t tcp_segtype; \
     \
    __wsum payload_csum; \
     \
    void *outer_ethhdr; /*including VLAN's */ \
    uint8_t outer_ethhdrlen;/*including VLAN's */ \
     \
    uint8_t outer_l3proto; \
    void *outer_ipv4hdr; \
    uint8_t outer_ipv4hdrlen; \
    uint16_t outer_ipv4_id; \
    uint16_t outer_ip_fragoff; \
    uint8_t outer_ip_fragneeded; \
    uint8_t outer_ip_lastfrag; \
     \
    void *grehdr; \
    uint8_t grehdrlen; \
    \
    void *txdev; \
    HardStartXmitFuncP xmit_fn;

typedef struct gso_hdrs {
       GSO_COMMON_HDR

} gso_hdrs_t;

extern int bcm_parse_gso_hdrs(struct sk_buff *skb, struct gso_hdrs *hdrs);

static inline __sum16 bcm_gso_l4_csum(struct gso_hdrs *hdrs, uint32_t len, uint16_t proto, __wsum base)
{
    if(hdrs->l3proto == BCM_L3_PROTO_IPV4 ){
        struct iphdr *ipv4= hdrs->ipv4hdr;
        return csum_tcpudp_magic(ipv4->saddr, ipv4->daddr, len, proto, base);
    } else {
        struct ipv6hdr *ipv6 = hdrs->ipv6hdr;
        return csum_ipv6_magic(&ipv6->saddr, &ipv6->daddr, len, proto, base);
    }
}

static inline void bcm_gso_push_hdr(uint8_t **data_pp, unsigned int *len_p, void *src, unsigned int len)
{
    *data_pp -= len;
    *len_p += len;
    memcpy(*data_pp, src, len);
}

static inline int bcm_gso_add_l2hdr(uint8_t **data_pp, unsigned int *len_p, struct gso_hdrs *hdrs)
{
    if(hdrs->ppphdrlen)
    {
        uint16_t *ppp_len = NULL;
        bcm_gso_push_hdr(data_pp, len_p, hdrs->ppphdr, hdrs->ppphdrlen);

        if(hdrs->l3proto == BCM_L3_PROTO_IPV4 || hdrs->l3proto == BCM_L3_PROTO_IPV6){
            /* update len   */
            ppp_len = (uint16_t *)(*data_pp + PPP_HDRLEN);
            *ppp_len = htons(*len_p + 2 - hdrs->ppphdrlen);
        }
    }
    bcm_gso_push_hdr(data_pp, len_p, hdrs->ethhdr, hdrs->ethhdrlen);
    return 0;
}

static inline int bcm_gso_add_l3hdr(uint8_t **data_pp, unsigned int *len_p, struct gso_hdrs *hdrs)
{
    if(hdrs->l3proto == BCM_L3_PROTO_IPV4){
        struct iphdr *ipv4;
        bcm_gso_push_hdr(data_pp, len_p, hdrs->ipv4hdr, hdrs->ipv4hdrlen);
        ipv4 = (struct iphdr *)*data_pp;
        /* update len   */
        ipv4->tot_len = htons(*len_p);

        if(hdrs->ip_fragneeded){
             /* update offset */
             ipv4->frag_off = htons((hdrs->ip_fragoff >> 3));
             /* update flags */
             if( !hdrs->ip_lastfrag)
                 ipv4->frag_off |= htons(IP_MF);
        }

        /* update IPID  */

        /* update csum  */
        ipv4->check = 0;
        ipv4->check = ip_fast_csum((unsigned char *)ipv4, ipv4->ihl);
    }
    else if(hdrs->l3proto == BCM_L3_PROTO_IPV6){
        struct ipv6hdr *ipv6;

        if(hdrs->ip_fragneeded){
            struct frag_hdr fh;

            fh.nexthdr = ((struct ipv6hdr *)hdrs->ipv6hdr)->nexthdr;
            fh.reserved = 0;
            fh.identification = hdrs->ipv6_fragid;
            fh.frag_off = htons(hdrs->ip_fragoff);
            bcm_gso_push_hdr(data_pp, len_p, &fh, sizeof(struct frag_hdr));
        }

        bcm_gso_push_hdr(data_pp, len_p, hdrs->ipv6hdr, hdrs->ipv6hdrlen);
        ipv6 = (struct ipv6hdr *)*data_pp;

        /* update len   */
        ipv6->payload_len = htons(*len_p - hdrs->ipv6hdrlen);

        if(hdrs->ip_fragneeded){
            ipv6->nexthdr =NEXTHDR_FRAGMENT;
        }

        /* update IPID  */
        /* update flags */
    }
    else{
        printk(KERN_ERR "%s:Unsuppoerted L3 protocol %d\n", __func__, hdrs->l3proto);
        return -1;
    }
    return 0;
}

static inline void bcm_gso_add_tcphdr(uint8_t **data_pp, unsigned int *len_p, struct gso_hdrs *hdrs)
{
    struct tcphdr *th;

    bcm_gso_push_hdr(data_pp, len_p, hdrs->l4hdr, hdrs->l4hdrlen);
    /* update seq */
    th = (struct tcphdr *)*data_pp;
    th->seq = htonl(hdrs->tcpseq);

    if(hdrs->tcp_segtype != BCM_SW_GSO_SEG_TYPE_SINGLE){

        /* clear cwr in all packets except first */
        if(hdrs->tcp_segtype != BCM_SW_GSO_SEG_TYPE_FIRST)
            th->cwr = 0;

        /* clear fin, psh in all packets except last */
        if((hdrs->tcp_segtype != BCM_SW_GSO_SEG_TYPE_LAST))
            th->fin = th->psh = 0;
    }
}


static inline int bcm_gso_add_gre_hdr(uint8_t **data_pp, unsigned int *len_p, struct gso_hdrs *hdrs)
{
    //TODO: GRE Seq 
    bcm_gso_push_hdr(data_pp, len_p, hdrs->grehdr, hdrs->grehdrlen);
    return 0;
}


static inline int bcm_gso_add_outer_l2hdr(uint8_t **data_pp, unsigned int *len_p, struct gso_hdrs *hdrs)
{
    bcm_gso_push_hdr(data_pp, len_p, hdrs->outer_ethhdr, hdrs->outer_ethhdrlen);
    return 0;
}


static inline int bcm_gso_add_outer_l3hdr(uint8_t **data_pp, unsigned int *len_p, struct gso_hdrs *hdrs)
{
    if(hdrs->outer_l3proto == BCM_L3_PROTO_IPV4){
        struct iphdr *ipv4;
        bcm_gso_push_hdr(data_pp, len_p, hdrs->outer_ipv4hdr, hdrs->outer_ipv4hdrlen);
        ipv4 = (struct iphdr *)*data_pp;
        /* update len   */
        ipv4->tot_len = htons(*len_p);

        if(hdrs->outer_ip_fragneeded){
            /* update offset */
            ipv4->frag_off = htons((hdrs->outer_ip_fragoff >> 3));
            /* update flags */
            if( !hdrs->outer_ip_lastfrag)
                ipv4->frag_off |= htons(IP_MF);
        }

        /* update IPID  */

        /* update csum  */
        ipv4->check = 0;
        ipv4->check = ip_fast_csum((unsigned char *)ipv4, ipv4->ihl);
    }
#if 0 //Not support outer ipv6 yet
    else if(hdrs->l3proto == BCM_L3_PROTO_IPV6){
        struct ipv6hdr *ipv6;

        if(hdrs->ip_fragneeded){
            struct frag_hdr fh;

            fh.nexthdr = ((struct ipv6hdr *)hdrs->ipv6hdr)->nexthdr;
            fh.reserved = 0;
            fh.identification = hdrs->ipv6_fragid;
            fh.frag_off = htons(hdrs->ip_fragoff);
            bcm_gso_push_hdr(data_pp, len_p, &fh, sizeof(struct frag_hdr));
        }

        bcm_gso_push_hdr(data_pp, len_p, hdrs->ipv6hdr, hdrs->ipv6hdrlen);
        ipv6 = (struct ipv6hdr *)*data_pp;

        /* update len   */
        ipv6->payload_len = htons(*len_p - hdrs->ipv6hdrlen);

        if(hdrs->ip_fragneeded){
            ipv6->nexthdr =NEXTHDR_FRAGMENT;
        }

        /* update IPID  */
        /* update flags */
    }
#endif    
    else{
        printk(KERN_ERR "%s:Unsuppoerted L3 protocol %d\n", __func__, hdrs->l3proto);
        return -1;
    }
    return 0;
}

#endif  /* _BCM_GSO_H */
