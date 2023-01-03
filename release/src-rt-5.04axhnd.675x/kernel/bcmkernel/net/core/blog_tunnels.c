/*
* <:copyright-BRCM:2012:DUAL/GPL:standard
* 
*    Copyright (c) 2012 Broadcom 
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

#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/blog.h>
#include <linux/blog_net.h>
#include <linux/nbuff.h>
#include <linux/skbuff.h>

#include <net/rtnetlink.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <net/ip_tunnels.h>
#include <net/gre.h>
#include <net/ip6_tunnel.h>

#include "blog_inline.h"

#if IS_ENABLED(CONFIG_NET_IPGRE)

extern unsigned int ipgre_net_id __read_mostly;
extern unsigned int gre_tap_net_id __read_mostly;

static inline int __bcm_gre_rcv_check(struct ip_tunnel *tunnel, struct iphdr *iph, 
	uint16_t len, uint32_t *pkt_seqno)
{
	int ret = BLOG_GRE_RCV_NO_SEQNO;
	int grehlen = 4;
	int iph_len = iph->ihl<<2;
	__be16 *p = (__be16*)((uint8_t *)iph+iph_len);
	__be16 flags;

	flags = p[0];

	if (tunnel->parms.i_flags & TUNNEL_CSUM) {
		uint16_t csum;

		grehlen += 4;
		csum = *(((__be16 *)p) + 2);

		if (!csum)
			goto no_csum;

		csum = ip_compute_csum((void*)(iph+1), len - iph_len);

		if (csum) {
			tunnel->dev->stats.rx_crc_errors++;
			tunnel->dev->stats.rx_errors++;
			ret = BLOG_GRE_RCV_CHKSUM_ERR;
			goto rcv_done;
		}
	}

no_csum:
	if ((tunnel->parms.i_flags & TUNNEL_KEY) && (flags&GRE_KEY))
		grehlen += 4;

	if (tunnel->parms.i_flags & TUNNEL_SEQ) {
		uint32_t seqno = *(((__be32 *)p) + (grehlen / 4));
		*pkt_seqno = seqno;
		if (tunnel->i_seqno && (s32)(seqno - tunnel->i_seqno) == 0) {
			tunnel->i_seqno = seqno + 1;
			ret = BLOG_GRE_RCV_IN_SEQ;
		} else if (tunnel->i_seqno && (s32)(seqno - tunnel->i_seqno) < 0) {
			tunnel->dev->stats.rx_fifo_errors++;
			tunnel->dev->stats.rx_errors++;
			ret = BLOG_GRE_RCV_OOS_LT;
		} else {
			tunnel->i_seqno = seqno + 1;
			ret = BLOG_GRE_RCV_OOS_GT;
		}
	}

rcv_done:
	return ret;
}

int bcm_gre_rcv_check(struct net_device *dev, struct iphdr *iph, 
	uint16_t len, void **tunl, uint32_t *pkt_seqno)
{
	int ret = BLOG_GRE_RCV_NO_TUNNEL;

	struct net *net = dev_net(dev);
	struct ip_tunnel_net *itn;
	struct ip_tunnel *tunnel;
    struct tnl_ptk_info tpi;

	int iph_len = iph->ihl<<2;
	struct gre_base_hdr *greh = (struct gre_base_hdr *)((uint8_t *)iph+iph_len);
    __be32 *options =(__be32*)(__be32 *)(greh + 1);

	tpi.flags = gre_flags_to_tnl_flags(greh->flags);
    tpi.proto = greh->protocol;

	if (greh->flags & GRE_CSUM) {
		options++;
	}

	if (greh->flags & GRE_KEY) {
		tpi.key = *options;
		options++;
	} else
		tpi.key = 0;

	if (tpi.proto == htons(ETH_P_TEB))
		itn = net_generic(net, gre_tap_net_id);
	else
		itn = net_generic(net, ipgre_net_id);

	tunnel = ip_tunnel_lookup(itn, dev->ifindex, tpi.flags,
				  iph->saddr, iph->daddr, tpi.key);

    if (tunnel) {
        ret =  __bcm_gre_rcv_check(tunnel, iph, len, pkt_seqno);
    }

	*tunl = (void *) tunnel;
	return ret;
}

/* Adds the TX seqno, Key and updates the GRE checksum */
static inline 
void __bcm_gre_xmit_update(struct ip_tunnel *tunnel, struct iphdr *iph, 
	uint16_t len)
{
	/* IPV4 Tunnel doesn't use GRE flags, uses TUNNEL flags */
	if (tunnel->parms.o_flags&(TUNNEL_KEY|TUNNEL_CSUM|TUNNEL_SEQ)) {
		int iph_len = iph->ihl<<2;
		/* tunnel->encap_hlen is only used for FOU, GUE. Otherwise it's 0 */
		__be32 *ptr = (__be32*)(((u8*)iph) + iph_len + tunnel->hlen - 4);

		if (tunnel->parms.o_flags&TUNNEL_SEQ) {
			++tunnel->o_seqno;
			*ptr = htonl(tunnel->o_seqno);
			ptr--;
		}

		if (tunnel->parms.o_flags&TUNNEL_KEY) {
			*ptr = tunnel->parms.o_key;
			ptr--;
		}

		if (tunnel->parms.o_flags&TUNNEL_CSUM) {
			*ptr = 0;
			*(__sum16*)ptr = ip_compute_csum((void*)(iph+1), len - iph_len);
		}
		cache_flush_len(ptr, tunnel->hlen);
	}
}

/* Adds the oseqno and updates the GRE checksum */
void bcm_gre_xmit_update(struct ip_tunnel *tunnel, struct iphdr *iph, 
	uint16_t len)
{
	rcu_read_lock();
	__bcm_gre_xmit_update(tunnel, iph, len);
	rcu_read_unlock();
}

blog_gre_rcv_check_t blog_gre_rcv_check_fn = (blog_gre_rcv_check_t) bcm_gre_rcv_check;
blog_gre_xmit_upd_t blog_gre_xmit_update_fn = (blog_gre_xmit_upd_t) bcm_gre_xmit_update;

#endif


#if IS_ENABLED(CONFIG_IPV6_GRE)

extern struct ip6_tnl *ip6gre_tunnel_lookup(struct net_device *dev,
		const struct in6_addr *remote, const struct in6_addr *local,
		__be32 key, __be16 gre_proto);

static inline int __bcm_gre6_rcv_check(struct ip6_tnl *tunnel, struct ipv6hdr *ipv6h, 
	uint16_t len, uint32_t *pkt_seqno)
{
	int ret = BLOG_GRE_RCV_NO_SEQNO;
	int grehlen = 4;
	u8 dst_option_len = (ipv6h->nexthdr == NEXTHDR_DEST) ? BLOG_IPV6EXT_HDR_LEN : 0;

	/* Not supporting extension headers except dest_opton. If this changes, need to calculate. */
	__be16 *p = (__be16*)((uint8_t *)ipv6h+BLOG_IPV6_HDR_LEN+dst_option_len);
	__be16 flags;

	flags = p[0];

	if (tunnel->parms.i_flags & TUNNEL_CSUM) {
		uint16_t csum;

		grehlen += 4;		csum = *(((__be16 *)p) + 2);

		if (!csum)
			goto no_csum;

		csum = ip_compute_csum((void*)((u8 *)(ipv6h+1)+dst_option_len), 
					len - BLOG_IPV6_HDR_LEN - dst_option_len);
		if (csum) {
			tunnel->dev->stats.rx_crc_errors++;
			tunnel->dev->stats.rx_errors++;
			ret = BLOG_GRE_RCV_CHKSUM_ERR;
			goto rcv_done;
		}
	}

no_csum:
	if ((tunnel->parms.i_flags & TUNNEL_KEY) && (flags&GRE_KEY))
		grehlen += 4;

	if (tunnel->parms.i_flags & TUNNEL_SEQ) {
		__be32 seqno = *(((__be32 *)p) + (grehlen / 4));
		*pkt_seqno = seqno;
		if (tunnel->i_seqno && (s32)(ntohl(seqno) - tunnel->i_seqno) < 0) {
			tunnel->dev->stats.rx_fifo_errors++;
			tunnel->dev->stats.rx_errors++;
			ret = BLOG_GRE_RCV_OOS_LT;
		} else {
			tunnel->i_seqno = ntohl(seqno) + 1;
			ret = BLOG_GRE_RCV_IN_SEQ;
		}
	}

rcv_done:
	return ret;
}

int bcm_gre6_rcv_check(struct net_device *dev, struct ipv6hdr *ipv6h,
	uint16_t len, void **tunl, uint32_t *pkt_seqno)
{
	int ret = BLOG_GRE_RCV_NO_TUNNEL;
	struct ip6_tnl *tunnel;
	u8 dst_option_len = (ipv6h->nexthdr == NEXTHDR_DEST) ? BLOG_IPV6EXT_HDR_LEN : 0;
	u8 *h = (uint8_t *)ipv6h+BLOG_IPV6_HDR_LEN+dst_option_len;
	__be16 flags = *(__be16 *)h;
	__be16 protocol = *((__be16 *)h + 1);
	struct gre_base_hdr *greh = (struct gre_base_hdr *)((uint8_t *)h);
    __be32 *options =(__be32*)(__be32 *)(greh + 1);
	__be32 key = 0;

	if (flags & GRE_CSUM) {
		options++;
	}

	if (flags & GRE_KEY) {
		key = *options;
	}

	tunnel = ip6gre_tunnel_lookup(dev, &ipv6h->saddr, &ipv6h->daddr,
				key, protocol);

	if (tunnel) {
		rcu_read_lock();
		ret = __bcm_gre6_rcv_check(tunnel, ipv6h, len, pkt_seqno);
		rcu_read_unlock();
	}

	*tunl = (void *) tunnel;
	return ret;
}

/* Adds the TX seqno, Key and updates the GRE checksum */
static inline
void __bcm_gre6_xmit_update(struct ip6_tnl *tunnel, struct ipv6hdr *ipv6h,
	uint16_t len)
{
	if (tunnel->parms.o_flags&(TUNNEL_KEY|TUNNEL_CSUM|TUNNEL_SEQ)) {
		/* ip6 tunnel hlen is precalculated GRE header length (includes encap). */
		u8 dst_option_len = (ipv6h->nexthdr == NEXTHDR_DEST) ? BLOG_IPV6EXT_HDR_LEN : 0;
		__be32 *ptr = (__be32*)(((u8*)(ipv6h+1)) + dst_option_len + tunnel->hlen - 4);

		if (tunnel->parms.o_flags&TUNNEL_SEQ) {
			++tunnel->o_seqno;
			*ptr = htonl(tunnel->o_seqno);
			ptr--;
		}

		if (tunnel->parms.o_flags&TUNNEL_KEY) {
			*ptr = tunnel->parms.o_key;
			ptr--;
		}

		if (tunnel->parms.o_flags&TUNNEL_CSUM) {
			*ptr = 0;
			*(__sum16*)ptr = ip_compute_csum((void*)((u8 *)(ipv6h+1)+dst_option_len), 
							len - BLOG_IPV6_HDR_LEN - dst_option_len);
		}
		cache_flush_len(ptr, tunnel->hlen);
	}
}

/* Adds the oseqno and updates the GRE checksum */
void bcm_gre6_xmit_update(struct ip6_tnl *tunnel, struct ipv6hdr *ipv6h,
	uint16_t len)
{
	rcu_read_lock();
	__bcm_gre6_xmit_update(tunnel, ipv6h, len);
	rcu_read_unlock();
}

blog_gre6_rcv_check_t blog_gre6_rcv_check_fn = (blog_gre6_rcv_check_t) bcm_gre6_rcv_check;
blog_gre6_xmit_upd_t blog_gre6_xmit_update_fn = (blog_gre6_xmit_upd_t) bcm_gre6_xmit_update;

#endif


#if IS_ENABLED(CONFIG_NET_IPGRE) || IS_ENABLED(CONFIG_ACCEL_PPTP)
/*
 * Macro specific to parsing: Used in blog_gre_rcv().
 * - Fetch the next encapsulation
 * - set the hdr_p to point to next next header start
 */
#define BLOG_PARSE(tag, length, proto)  h_proto = (proto);  \
                                        hdr_p += (length);  \
                                        ix++;               \
    blog_print( "BLOG_PARSE %s: length<%d> proto<0x%04x>", \
                          #tag, length, ntohs(h_proto) );

/*
 *------------------------------------------------------------------------------
 * Function     : blog_parse_l2hdr
 * Description  : Given a packet quickly parse the L2 header
 * Parameters   :
 *  fkb_p       : Pointer to a fast kernel buffer<data,len>
 *  h_proto     : First encapsulation type
                : NULL : if the parsing failed or not an IPv4 or IPv6 Hdr
                : ip_p : pointer to first IPv4 or IPv6 Hdr if the
                : parsing was successful up to it
 * Return values:
 *              : Pointer to first IPv4 or IPv6 header
 *------------------------------------------------------------------------------
 */
static inline 
BlogIpv4Hdr_t * _blog_parse_l2hdr( struct fkbuff *fkb_p, uint32_t h_proto )
{
    int          ix;
    char         * hdr_p;
    BlogIpv4Hdr_t *ip_p;

    BLOG_DBG(
          if ((fkb_p!=FKB_NULL) &&
              ((h_proto==TYPE_ETH)||(h_proto==TYPE_PPP)||(h_proto==TYPE_IP)))
          {
            blog_assertr(((fkb_p!=FKB_NULL) 
                         && ((h_proto==TYPE_ETH)||(h_proto==TYPE_PPP)
                              ||(h_proto==TYPE_IP))), NULL );
          } );
    blog_print( "fkb<%p> data<%p> len<%d> h_proto<%u>",
                fkb_p, fkb_p->data, (int)fkb_p->len, h_proto );

    /* PACKET PARSE PHASE */

    /* initialize locals */
    hdr_p           = fkb_p->data;
    ix              = 0;
    ip_p          = (BlogIpv4Hdr_t *)NULL;
    h_proto         = htons(h_proto);

    switch ( h_proto )  /* First Encap */
    {
        case htons(TYPE_ETH):  /* first encap: XYZoE */
            /* Check whether multicast logging support is enabled */
            if (((BlogEthHdr_t*)hdr_p)->macDa.u8[0] & 0x1) /* mcast or bcast */
            {
                blog_print( "ABORT multicast MAC" );
                goto done;
            }
            /* PS. Multicast over PPPoE would not have multicast MacDA */
            BLOG_PARSE( ETH, (int)BLOG_ETH_HDR_LEN, *((uint16_t*)hdr_p+6) ); 
            break;

        case htons(TYPE_PPP):  /* first encap: PPPoA */
            if ( unlikely(ix != 0) )
                goto done;
            BLOG_PARSE( PPP, (int)BLOG_PPP_HDR_LEN, *(uint16_t*)hdr_p ); 
            break;

        case htons(TYPE_IP):   /* first encap: IPoA */
            ip_p = (BlogIpv4Hdr_t *)hdr_p;
            goto done;

        default:
            break;
    }

    if ( unlikely(ix > BLOG_ENCAP_MAX)) goto done;
    switch ( h_proto ) /* parse Broadcom Tags */
    {
        case htons(BLOG_ETH_P_BRCM6TAG):
            BLOG_PARSE( BRCM6, BLOG_BRCM6_HDR_LEN, *((uint16_t*)hdr_p+2) );
            break;

        case htons(BLOG_ETH_P_BRCM4TAG):
            BLOG_PARSE( BRCM4, BLOG_BRCM4_HDR_LEN, *((uint16_t*)hdr_p+1) );
            break;

        default:
            break;
    }

    do /* parse VLAN tags */
    {
        if ( unlikely(ix > BLOG_ENCAP_MAX)) goto done;
        switch ( h_proto )
        {
            case htons(BLOG_ETH_P_8021Q): 
            case htons(BLOG_ETH_P_8021AD):
                BLOG_PARSE( VLAN, BLOG_VLAN_HDR_LEN, *((uint16_t*)hdr_p+1) ); 
                break;

            default:
                goto _blog_parse_l2hdr_eth_type;
        }
    } while(1);

_blog_parse_l2hdr_eth_type:
    if ( unlikely(ix > BLOG_ENCAP_MAX)) goto done;
    switch ( h_proto )
    {
        case htons(BLOG_ETH_P_PPP_SES):
            BLOG_PARSE( PPPOE, BLOG_PPPOE_HDR_LEN, *((uint16_t*)hdr_p+3) );
            goto _blog_parse_l2_ppp_ip;

        case htons(BLOG_PPP_IPV6):
        case htons(BLOG_ETH_P_IPV6):
        case htons(BLOG_PPP_IPV4):
        case htons(BLOG_ETH_P_IPV4):
            ip_p = (BlogIpv4Hdr_t *)hdr_p;
            goto done;

        default :
            blog_print( "ABORT UNKNOWN Rx h_proto 0x%04x", 
                (uint16_t) ntohs(h_proto) );
            goto done;
    } /* switch ( h_proto ) */

_blog_parse_l2_ppp_ip:
    {
        switch ( h_proto )
        {
            case htons(BLOG_PPP_IPV6):
            case htons(BLOG_PPP_IPV4):
                ip_p = (BlogIpv4Hdr_t *)hdr_p;
                goto done;

            default :
                blog_print( "ABORT UNKNOWN Rx h_proto 0x%04x", 
                    (uint16_t) ntohs(h_proto) );
                goto done;
        } /* switch ( h_proto ) */
    }

done:
    return ip_p;
}

BlogIpv4Hdr_t * blog_parse_l2hdr( struct fkbuff *fkb_p, uint32_t h_proto )
{
    return _blog_parse_l2hdr(fkb_p, h_proto);
}

int blog_rcv_chk_gre(struct fkbuff *fkb_p, uint32_t h_proto, uint16_t *gflags_p) 
{
	BlogIpv4Hdr_t* ip_p;
	char * hdr_p;
    uint16_t *grehdr_p;
    BlogGreIeFlagsVer_t gre_flags = {.u16 = 0 };	

    ip_p = _blog_parse_l2hdr( fkb_p, h_proto );

    if (ip_p != NULL) 
    {
        int ver = (*(uint8_t*)ip_p) >> 4;
        uint32_t ip_proto = -BLOG_IPPROTO_GRE;

        blog_print( "Rcv Check GRE or PPTP" );

        if ( ver == 4 )
        {
            hdr_p = (char *)ip_p;
            hdr_p += BLOG_IPV4_HDR_LEN;

            if ( unlikely(*(uint8_t*)ip_p != 0x45) )
            {
                blog_print( "ABORT IP ver<%d> len<%d>", ip_p->ver, ip_p->ihl );
                return 0;
            }
            ip_proto = ip_p->proto;
        }
        else if ( ver == 6 )
        {
            hdr_p = (char *)ip_p;
            hdr_p += BLOG_IPV6_HDR_LEN;

            /*  Support extension headers? */
            ip_proto = ((BlogIpv6Hdr_t*)ip_p)->nextHdr;
            if (ip_proto == BLOG_IPPROTO_DSTOPTS) 
            {
                ip_proto = ((BlogIpv6ExtHdr_t*)hdr_p)->nextHdr;
                hdr_p += BLOG_IPV6EXT_HDR_LEN;
            }
        }

        if ( ip_proto == BLOG_IPPROTO_GRE ) 
        {
            grehdr_p = (uint16_t*)hdr_p;
            *gflags_p = gre_flags.u16 = ntohs(*(uint16_t*)grehdr_p);

            if ( gre_flags.ver)
            {
                return PPTP_GRE_VER_1;           	
            }
            else
            {	
            	return PPTP_GRE_VER_0; 
            }		
        }
    }    

    *gflags_p = 0;
	return PPTP_GRE_NONE;
}

int blog_xmit_chk_gre(struct sk_buff *skb_p, uint32_t h_proto) 
{
    if (skb_p && blog_gre_tunnel_accelerated())
    {
        BlogIpv4Hdr_t* ip_p;
        struct fkbuff * fkb_p;
        char * hdr_p;
        uint16_t *grehdr_p;
        BlogGreIeFlagsVer_t gre_flags = {.u16 = 0 };
   
        blog_print( "Xmit Check GRE or PPTP" );

        fkb_p = (struct fkbuff*) ((uintptr_t)skb_p + BLOG_OFFSETOF(sk_buff,fkbInSkb));
        ip_p = _blog_parse_l2hdr( fkb_p, h_proto );
        
        if (ip_p != NULL)
        {
            int ver = (*(uint8_t*)ip_p) >> 4;
            uint32_t ip_proto = ~BLOG_IPPROTO_GRE;

            if ( ver == 4 )
            {
                ip_proto = ip_p->proto;
            }
            else if ( ver == 6 )
            {
                /* Support extension headers? */
                ip_proto = ((BlogIpv6Hdr_t*)ip_p)->nextHdr;
            }

            if ( ip_proto == BLOG_IPPROTO_GRE ) 
            {
                hdr_p = (char *)ip_p;
                hdr_p += (ver == 4)? BLOG_IPV4_HDR_LEN : BLOG_IPV6_HDR_LEN;
                grehdr_p = (uint16_t*)hdr_p;
                gre_flags.u16 = ntohs(*(uint16_t*)grehdr_p);
                if (gre_flags.ver)
                {	   
                    //printk("Xmit PPTP_GRE_VER_1 !!!\n");
                    return PPTP_GRE_VER_1;
                }    
                else
                {	
                    //printk("Xmit PPTP_GRE_VER_0 !!!\n");
                    return PPTP_GRE_VER_0; 
                }
            }	    			   
        }
         
    }
    return PPTP_GRE_NONE;
}

#endif

#if IS_ENABLED(CONFIG_NET_IPGRE) || IS_ENABLED(CONFIG_IPV6_GRE)
/*
 *------------------------------------------------------------------------------
 * Function     : blog_gre_rcv
 * Description  : Given a packet quickly detect whether it is a GRE packet.
 *                If yes then do the other processing based on the GRE flags.
 * Parameters   :
 *  fkb_p       : Pointer to a fast kernel buffer<data,len>
 *  dev_p       : Pointer to the net_device on which the packet arrived.
 *  h_proto     : First encapsulation type
 *  tunl_pp     : Pointer to pointer to GRE tunnel
 *  pkt_seqno_p : Pointer to received packet seqno
 * Return values:
 *  BLOG_GRE_RCV_NO_GRE: 
 *              : Either the packet is not GRE or it cannot be 
 *                accelerated.
 *  BLOG_GRE_RCV_NO_SEQNO: 
 *              : Received packet does not have seqno.
 *  BLOG_GRE_RCV_IN_SEQ: 
 *              : GRE tunnel is configured with seqno and the received packet
 *              : seqno is in sync with the tunnel seqno.
 *  BLOG_GRE_RCV_NO_TUNNEL: 
 *              : Could not find the GRE tunnel matching with packet. 
 *  BLOG_GRE_RCV_FLAGS_MISSMATCH: 
 *              : GRE flags in the received packet does not match the flags 
 *              : in the configured GRE tunnel.
 *  BLOG_GRE_RCV_CHKSUM_ERR: 
 *              : Received packet has bad GRE checksum.
 *  BLOG_GRE_RCV_OOS_LT: 
 *              : GRE tunnel is configured with seqno and the received packet
 *              : seqno is out-of-seq (OOS) and less than the next seqno
 *              : expected by the tunnel seqno.
 *  BLOG_GRE_RCV_OOS_GT: 
 *              : GRE tunnel is configured with seqno and the received packet
 *              : seqno is out-of-seq and greater than the next seqno 
 *              : expected by the tunnel.
 * Note         : The *tunl_pp pointer makes all the tunnel fields available
 *                (including seqno). The tunnel seqno and pkt_seqno can
 *                be used to implement functions to put received packets 
 *                in sequence before giving the packets to flow cache 
 *                (i.e. invoking the blog_rx_hook_g()).
 *------------------------------------------------------------------------------
 */
int blog_gre_rcv( struct fkbuff *fkb_p, void * dev_p, uint32_t h_proto,
    void **tunl_pp, uint32_t *pkt_seqno_p)
{
    BlogIpv4Hdr_t* ip_p;
    int ret = BLOG_GRE_RCV_NOT_GRE;

    ip_p = _blog_parse_l2hdr( fkb_p, h_proto );

    if (ip_p != NULL) 
    {
        int ver = (*(uint8_t*)ip_p) >> 4;

        if ( ver == 4 )
        {
            blog_print( "BLOG PARSE IPv4:" );

            /* 
             * Abort parse
             * - If not IPv4 or with options.
             * - If this is a unicast and fragmented IP Pkt, let it pass through the
             *   network stack, as intermediate fragments do not carry a
             *   full upper layer protocol to determine the port numbers.
             */
            if ( unlikely(*(uint8_t*)ip_p != 0x45) )
            {
                blog_print( "ABORT IP ver<%d> len<%d>", ip_p->ver, ip_p->ihl );
                goto pkt_not_gre;
            }
            if ( ip_p->proto == BLOG_IPPROTO_GRE ) 
            {
                blog_print( "BLOG PARSE GRE:" );
                if (blog_gre_rcv_check_fn != NULL)
                    ret = blog_gre_rcv_check_fn( dev_p, ip_p, 
                        fkb_p->len - ((uintptr_t)ip_p - (uintptr_t)fkb_p->data), 
                        tunl_pp, pkt_seqno_p );
            }
        }
#if IS_ENABLED(CONFIG_IPV6_GRE)
        else if ( ver == 6 )
        {
            blog_print( "BLOG PARSE IPv6:" );

            /* Support extension headers? */
            switch( ((BlogIpv6Hdr_t*)ip_p)->nextHdr) 
            {
                case BLOG_IPPROTO_GRE:
                case BLOG_IPPROTO_DSTOPTS:
                {
                    blog_print( "BLOG PARSE GRE:" );
                    if (blog_gre6_rcv_check_fn != NULL)
                        ret = blog_gre6_rcv_check_fn( dev_p, (BlogIpv6Hdr_t *)ip_p, 
                            fkb_p->len - ((uintptr_t)ip_p - (uintptr_t)fkb_p->data), 
                            tunl_pp, pkt_seqno_p );
                }
                break;

                default:
                    break;
            }
        }
#endif /* CONFIG_IPV6_GRE */
    }

pkt_not_gre:
    return ret;
}

void blog_gre_xmit(struct sk_buff *skb_p, uint32_t h_proto)
{
    if (skb_p && skb_p->bcm_ext.tunl && blog_gre_tunnel_accelerated())
    {
        BlogIpv4Hdr_t* ip_p;
        struct fkbuff * fkb_p;

        /* non-accelerated GRE tunnel US case we need to sync seqno */
        blog_print( "non-XL GRE Tunnel" );

        fkb_p = (struct fkbuff*) ((uintptr_t)skb_p + 
                                        BLOG_OFFSETOF(sk_buff,fkbInSkb));
        ip_p = _blog_parse_l2hdr( fkb_p, h_proto );

        if (ip_p != NULL)
        {
            int ver = (*(uint8_t*)ip_p) >> 4;

            blog_print( "tunl<%p> skb<%p> data<%p> len<%u> ip_p<%p> "
                        "l2_data_len<%u>",
                skb_p->bcm_ext.tunl, skb_p, skb_p->data, skb_p->len, ip_p, 
                skb_p->len - (uint32_t)((uintptr_t) ip_p - (uintptr_t) skb_p->data)); 

            if ( ver == 4 )
            {
                if (blog_gre_xmit_update_fn != NULL)
                    blog_gre_xmit_update_fn((struct ip_tunnel *)skb_p->bcm_ext.tunl, ip_p, 
                        skb_p->len - ((uintptr_t) ip_p - (uintptr_t) skb_p->data));
            }
#if IS_ENABLED(CONFIG_IPV6_GRE)
            else if ( ver == 6 )
            {
                if (blog_gre6_xmit_update_fn != NULL)
                    blog_gre6_xmit_update_fn((struct ip6_tnl *)skb_p->bcm_ext.tunl, (BlogIpv6Hdr_t *)ip_p, 
                        skb_p->len - ((uintptr_t) ip_p - (uintptr_t) skb_p->data));
            }
#endif /* CONFIG_IPV6_GRE */
        }
    }
}

EXPORT_SYMBOL(blog_gre_rcv);
EXPORT_SYMBOL(blog_gre_xmit);

EXPORT_SYMBOL(blog_gre_rcv_check_fn);
EXPORT_SYMBOL(blog_gre_xmit_update_fn);
#if IS_ENABLED(CONFIG_IPV6_GRE)
EXPORT_SYMBOL(blog_gre6_rcv_check_fn);
EXPORT_SYMBOL(blog_gre6_xmit_update_fn);
#endif /* CONFIG_IPV6_GRE */
#endif

#if IS_ENABLED(CONFIG_ACCEL_PPTP)

#include <linux/if_pppox.h>

#ifndef MAX_CALLID
#define MAX_CALLID 65535
#endif
extern DECLARE_BITMAP(callid_bitmap, MAX_CALLID + 1);
extern struct pppox_sock __rcu **callid_sock;
extern spinlock_t chan_lock;

extern int bcm_ppp_rcv_decomp_run(struct ppp_channel *);

int __bcm_pptp_rcv_check(uint16_t call_id, uint32_t *rcv_pktSeq, uint32_t rcv_pktAck, uint32_t saddr)
{
    struct pppox_sock *sock;
    struct pptp_opt *opt;
    int ret = BLOG_PPTP_RCV_NO_TUNNEL;
    
    sock = rcu_dereference(callid_sock[call_id]);
    if (sock) 
    {
        opt=&sock->proto.pptp;
        if (opt->dst_addr.sin_addr.s_addr!=saddr) 
		{
            sock=NULL;
		}
        else 
        {   
            sock_hold(sk_pppox(sock));
            //printk(KERN_INFO "PPTP: bcm_pptp_rcv_check() current seq_recv is %d \n", opt->seq_recv);
            if (bcm_ppp_rcv_decomp_run(&sock->chan)) {
                ret = BLOG_PPTP_ENCRYPTED;
                rcu_read_unlock();
                return ret;
            } else if (opt->seq_recv && ((*rcv_pktSeq) > opt->seq_recv)) 
            {
                opt->seq_recv = (*rcv_pktSeq);
                ret = BLOG_PPTP_RCV_IN_SEQ;
            } else if (opt->seq_recv && ((*rcv_pktSeq) - opt->seq_recv) <= 0) {
                printk(KERN_INFO "bcm_pptp_rcv_check():[BLOG_PPTP_RCV_OOS_LT] current seq_recv is %d pkt_seq=%d \n", opt->seq_recv, *rcv_pktSeq);
                ret = BLOG_PPTP_RCV_OOS_LT;
            } else {
                printk(KERN_INFO "bcm_pptp_rcv_check():[BLOG_PPTP_RCV_OOS_GT] current seq_recv is %d pkt_seq=%d \n", opt->seq_recv, *rcv_pktSeq);
                opt->seq_recv = (*rcv_pktSeq);
                ret = BLOG_PPTP_RCV_OOS_GT;
            }       
            if (rcv_pktAck > opt->ack_recv) opt->ack_recv = rcv_pktAck;    
        }
            
    }          
    return ret;
}

int bcm_pptp_rcv_check(uint16_t call_id, uint32_t *rcv_pktSeq, uint32_t rcv_pktAck, uint32_t saddr)
{
    int ret = BLOG_PPTP_RCV_NO_TUNNEL;
	rcu_read_lock();
    ret = __bcm_pptp_rcv_check(call_id, rcv_pktSeq, rcv_pktAck, saddr);
	rcu_read_unlock();
    return ret;
}

int bcm_pptp_xmit_update(uint16_t call_id, uint32_t* seqNum, uint32_t* ackNum, uint32_t daddr)
{
    struct pppox_sock *sock;
    struct pptp_opt *opt;
    int i, ack_flag = PPTP_NOT_ACK;
    
    rcu_read_lock();

    for(i = find_next_bit(callid_bitmap,MAX_CALLID,1); i < MAX_CALLID; i = find_next_bit(callid_bitmap, MAX_CALLID, i + 1))
    {
        sock = rcu_dereference(callid_sock[i]);
        if (!sock)
            continue;
            
        opt = &sock->proto.pptp;
        if (opt->dst_addr.call_id == call_id && opt->dst_addr.sin_addr.s_addr == daddr) 
        {   
            //printk(KERN_INFO "PPTP: seq_sent = %d, ack_sent = %d \n", opt->seq_sent, opt->ack_sent);
            opt->seq_sent += 1;
            *seqNum = opt->seq_sent;
            
            if (opt->ack_sent != opt->seq_recv)
            {   
                ack_flag = PPTP_WITH_ACK;
                opt->ack_sent = opt->seq_recv;            
            }   
            *ackNum = opt->ack_sent;
            break;
        }
    }
    
    rcu_read_unlock();

    return ack_flag;
}


blog_pptp_rcv_check_t blog_pptp_rcv_check_fn = (blog_pptp_rcv_check_t) bcm_pptp_rcv_check;
blog_pptp_xmit_upd_t blog_pptp_xmit_update_fn = (blog_pptp_xmit_upd_t) bcm_pptp_xmit_update;

int blog_pptp_rcv( struct fkbuff *fkb_p, uint32_t h_proto, uint32_t *rcv_pktSeq) 
{
	BlogIpv4Hdr_t* ip_p;
	char * hdr_p;
    uint16_t *grehdr_p;
    BlogGreIeFlagsVer_t gre_flags = {.u16 = 0 };
	uint16_t call_id = 0;
	uint32_t saddr, rcv_pktAck = 0;
	
    int ret = BLOG_PPTP_RCV_NOT_PPTP;

    ip_p = _blog_parse_l2hdr( fkb_p, h_proto );

    if (ip_p != NULL) 
    {
        blog_print( "BLOG PARSE IPv4:" );

        /* 
         * Abort parse
         * - If not IPv4 or with options.
         * - If this is a unicast and fragmented IP Pkt, let it pass through the
         *   network stack, as intermediate fragments do not carry a
         *   full upper layer protocol to determine the port numbers.
         */
        if ( unlikely(*(uint8_t*)ip_p != 0x45) )
        {
            blog_print( "ABORT IP ver<%d> len<%d>", ip_p->ver, ip_p->ihl );
            goto pkt_not_pptp;
        }

        if ( ip_p->proto == BLOG_IPPROTO_GRE ) 
        {
            hdr_p = (char *)ip_p;
            hdr_p += BLOG_IPV4_HDR_LEN;
            grehdr_p = (uint16_t*)hdr_p;
            gre_flags.u16 = ntohs(*(uint16_t*)grehdr_p);
            
            /* the pkt is PPTP with seq number */
            if (gre_flags.seqIe && gre_flags.keyIe && gre_flags.ver) 
            {
            	blog_print( "BLOG PARSE PPTP:" );
            	call_id = ntohs(*(uint16_t*) (grehdr_p + 3));
            	*rcv_pktSeq = ntohl(*(uint32_t*) (grehdr_p + 4));
            	saddr  = blog_read32_align16( (uint16_t *)&ip_p->sAddr );

            	blog_print( "\nincoming pptp pkt's seq = %d, callid= %d\n", *rcv_pktSeq , call_id);
            	if(gre_flags.ackIe) /* the pkt is PPTP with ack number */
                {	
                	rcv_pktAck = ntohl(*(uint32_t*) (grehdr_p + 6));
                	blog_print( "rcv_pktAck = %d \n", rcv_pktAck );
                }
                
            	if (blog_pptp_rcv_check_fn != NULL)
            	   ret = blog_pptp_rcv_check_fn(call_id, rcv_pktSeq, 
            	                             rcv_pktAck, saddr );
            	
            }
        }
    }

pkt_not_pptp:
    return ret;
}

void blog_pptp_xmit(struct sk_buff *skb_p, uint32_t h_proto) 
{
    if (skb_p && blog_gre_tunnel_accelerated())
    {
        BlogIpv4Hdr_t* ip_p;
        struct fkbuff * fkb_p;
        char * hdr_p;
        uint16_t *grehdr_p;
        BlogGreIeFlagsVer_t gre_flags = {.u16 = 0 };
        uint16_t call_id = 0;
        uint32_t seqNum = 0, ackNum = 0;
        uint32_t        saddr;        
        uint32_t        daddr;
    
        /* non-accelerated PPTP tunnel US case we need to sync seqno */
        blog_print( "non-XL PPTP Tunnel" );

        fkb_p = (struct fkbuff*) ((uintptr_t)skb_p + BLOG_OFFSETOF(sk_buff,fkbInSkb));
        ip_p = _blog_parse_l2hdr( fkb_p, h_proto );
        
        if (ip_p != NULL && (*(uint8_t*)ip_p) >> 4 == 4
            && ip_p->proto == BLOG_IPPROTO_GRE )
        {
            hdr_p = (char *)ip_p;
            hdr_p += BLOG_IPV4_HDR_LEN;
            grehdr_p = (uint16_t*)hdr_p;
            gre_flags.u16 = ntohs(*(uint16_t*)grehdr_p);
            
            /* the pkt is PPTP with seq number */
            if ((blog_pptp_xmit_update_fn != NULL) && gre_flags.seqIe
                    && gre_flags.keyIe && gre_flags.ver) 
            {	
            	call_id = ntohs(*(uint16_t*) (grehdr_p + 3));
            	daddr  = blog_read32_align16( (uint16_t *)&ip_p->dAddr );

                /* Save the outgoing pkt's seq/ack number */
                seqNum = ntohl(*(uint32_t*) (grehdr_p + 4));
                if(gre_flags.ackIe)
                    ackNum = ntohl(*(uint32_t*) (grehdr_p + 6));

                blog_lock(); /*TODO remove this lock and add PPTP specific lock inside pptp driver
                             to protect seq & ack */

                /* -----------------------------------------------------------
                 * [pptp tunnel mode local out pkt case] 
                 *  Find the corresponding pptp tunnel via call_id/daddr 
                 *  and then update seq/ack number
                 * -----------------------------------------------------------
                 * [pptp passthrough but enable fc gre case]
                 *  pptp tunnel not found in the local machine,
                 *  so seq/ack number was not updated, keep the original number 
                 * -----------------------------------------------------------
                */
                blog_pptp_xmit_update_fn(call_id, &seqNum, &ackNum, daddr);

            	*(uint32_t*) (grehdr_p + 4) = htonl(seqNum);

                if(gre_flags.ackIe) /* the pkt is PPTP with ack number */
                {	
                    *(uint32_t*) (grehdr_p + 6) = htonl(ackNum);
                }
				blog_unlock();

            	blog_print( "call id = %d, seqNum = %d, daddr = %X\n", 
            	             call_id, seqNum, daddr );
            } 
        }
    }

}

EXPORT_SYMBOL(blog_pptp_rcv);
EXPORT_SYMBOL(blog_pptp_xmit);
#else
blog_pptp_xmit_upd_t blog_pptp_xmit_update_fn = NULL;
#endif

EXPORT_SYMBOL(blog_pptp_xmit_update_fn);

#if IS_ENABLED(CONFIG_L2TP)
#include<linux/l2tp.h>
#include "l2tp_core.h"
int l2tp_rcv_check(struct net_device *dev, uint16_t tunnel_id, uint16_t session_id)
{
    struct net *net = dev_net(dev);
    struct l2tp_tunnel *tunnel;
    struct l2tp_session *session = NULL;
    int ret = BLOG_L2TP_RCV_NO_TUNNEL;
    
    tunnel = l2tp_tunnel_get(net, tunnel_id);
    if (tunnel)
	{   //printk("*** l2tp tunnel found!!!\n"); 
		session = l2tp_tunnel_get_session( tunnel, session_id);
		if (session)
		{   
			//printk("*** l2tp session found!!!\n");    
			ret = BLOG_L2TP_RCV_TUNNEL_FOUND;
			l2tp_session_dec_refcount(session);
		}
		l2tp_tunnel_dec_refcount(tunnel);
	}   
return ret; 
}
blog_l2tp_rcv_check_t blog_l2tp_rcv_check_fn =  (blog_l2tp_rcv_check_t)l2tp_rcv_check;
EXPORT_SYMBOL(l2tp_rcv_check);
#else
blog_l2tp_rcv_check_t blog_l2tp_rcv_check_fn = NULL;
#endif
EXPORT_SYMBOL(blog_l2tp_rcv_check_fn);

