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

#include <linux/nbuff.h>
#include <net/ip6_checksum.h>
#include <linux/ppp_defs.h>
#include <net/ip_tunnels.h>
#include <net/gre.h>
#include <linux/version.h>
#include <net/bcm_gso.h>

int bcm_parse_gso_hdrs(struct sk_buff *skb, struct gso_hdrs *hdrs)
{
    struct iphdr *ipv4;
    struct ipv6hdr *ipv6;
    struct tcphdr *tcp_hdr;
    int header = -1;
    void *data = skb->data;
    unsigned int skbheadlen = skb_headlen(skb);
    /*assumes first header is ETH always */
    uint16_t proto = ETH_P_802_3;
    int is_ppp = 0;
    int is_gre = 0;
    int ipv4_hdr_cnt = 0;
    int v6_next_hdr_tot_sz = 0;

    while(1) {
        header++;

        if(hdrs->totlen > skbheadlen){
            printk(KERN_ERR "%s: headers not present in linear data hdrslen=%u skbheadlen=%d \n",
                 __func__, hdrs->totlen, skbheadlen);
            return -1;
        }

        if (header > BCM_SW_GSO_MAX_HEADERS){
            printk(KERN_ERR "%s:Too many headers <%d>\n", __func__, header);
            return -1;
        }

        switch(proto)
        {
            case ETH_P_802_3:  /* first encap: XYZoE */

                if(header != 0 && is_gre == 0) 
                {
                        return -1;
                }

                if(is_gre) 
                {
                     /* That's second Header (Inner)
                       Backup first Header info to Outer and reserve GSO ethhdr field for Inner
                    */
                    hdrs->outer_ethhdr = hdrs->ethhdr;
                    hdrs->outer_ethhdrlen = hdrs->ethhdrlen;
                }

                hdrs->ethhdr = data;

                proto = ntohs(((struct ethhdr *)data)->h_proto);
                hdrs->ethhdrlen = ETH_HLEN;
                hdrs->totlen += ETH_HLEN;
                data += ETH_HLEN;
                break;

            case ETH_P_8021Q:
            case ETH_P_8021AD:

                proto = ntohs(*((uint16_t *)(data+2)));
                hdrs->ethhdrlen += 4;
                hdrs->totlen += 4;
                data += 4;
                break;

            case ETH_P_PPP_SES:   /* PPPoE session */
                proto = ntohs(*((uint16_t *)(data+PPP_HDRLEN+2)));
                hdrs->ppphdr = data;
                hdrs->ppphdrlen = 8;
                hdrs->totlen += 8;
                data += 8;
                is_ppp = 1;
                break;

            case PPP_IP:   /* IPv4 over PPPoE session */
                if (!is_ppp)
                    break;
                is_ppp = 0;
                __attribute__((__fallthrough__));

            case ETH_P_IP:

                ipv4 = data;
                ipv4_hdr_cnt++;
                hdrs->ip_len = ntohs(ipv4->tot_len);
                hdrs->l3proto = BCM_L3_PROTO_IPV4;
                proto = ipv4->protocol;

                if ((proto == IPPROTO_TCP) || (proto == IPPROTO_UDP) || (proto == IPPROTO_GRE))
                {
                    hdrs->ipv4hdr = data;
                    hdrs->ipv4_id = ntohs(ipv4->id);
                    hdrs->ipv4hdrlen = ipv4->ihl<<2;
                    hdrs->totlen += ipv4->ihl<<2;
                    data += ipv4->ihl<<2;

                } else {
                    printk(KERN_ERR "%s Unsupported L4 type=%u \n", __func__, proto);
                    return -1;
                }
                break;

            case PPP_IPV6:   /* IPv6 over PPPoE session */
                if (!is_ppp)
                    break;
                is_ppp = 0;
                __attribute__((__fallthrough__));
            case ETH_P_IPV6:

                ipv6 = data;
                hdrs->ip_len = ntohs(ipv6->payload_len);
                hdrs->l3proto = BCM_L3_PROTO_IPV6;
                proto = ipv6->nexthdr;
                v6_next_hdr_tot_sz = 0 ;

                if(proto == NEXTHDR_FRAGMENT)
                {
                    /* for 4.1 kernel may enable UFO
                       [udp6_ufo_fragment()] may add fragmentation header on GSO pkt
                       Skip fragmentation header and found next header for proto
                    */

                    struct frag_hdr *fh = NULL;
                    fh = data + sizeof(struct ipv6hdr);
                    proto = fh->nexthdr; //For L4 proto 
                    v6_next_hdr_tot_sz+= sizeof(struct frag_hdr);
                }

                if ((proto == IPPROTO_TCP) || (proto == IPPROTO_UDP)){

                    hdrs->ipv6hdr = (void*) ipv6;
                    hdrs->ipv6_flowlbl = ntohl(ip6_flowlabel(ipv6));
                    hdrs->ipv6hdrlen = sizeof(struct ipv6hdr);
                    hdrs->totlen += sizeof(struct ipv6hdr);
                    data += sizeof(struct ipv6hdr) + v6_next_hdr_tot_sz;

                } else {
                    printk(KERN_ERR "%s Unsupported L4 type=%u \n", __func__, proto);
                    return -1;
                }
                break;

            case IPPROTO_TCP :
                tcp_hdr = data;

                hdrs->l4hdr = data;
                hdrs->l4proto = IPPROTO_TCP;
                hdrs->l4hdrlen = tcp_hdr->doff <<2;
                hdrs->totlen += tcp_hdr->doff <<2;
                hdrs->tcpseq = ntohl(tcp_hdr->seq);
                return  0 ;

            case IPPROTO_UDP :
                hdrs->l4proto = IPPROTO_UDP;
                hdrs->l4hdr = data;
                hdrs->l4hdrlen = sizeof(struct udphdr) ;
                hdrs->totlen += sizeof(struct udphdr);
                return  0 ;

            case IPPROTO_GRE :
                { //Save GRE header info to GSO hdr
                    struct gre_base_hdr *greh = NULL;
                    hdrs->grehdr = data;
                    greh = (struct gre_base_hdr *) hdrs->grehdr ;

                    hdrs->grehdrlen = gre_calc_hlen(greh->flags);
                    hdrs->totlen += hdrs->grehdrlen;

                    data += hdrs->grehdrlen;
                    is_gre = 1;
  
                    if((greh->flags & (GRE_CSUM | GRE_KEY | GRE_SEQ)) != 0x00 )
                    {
                        //Not support GRE header with SUM/SEQ/KEY yet.
                        printk(KERN_ERR "%s:UNSUPPORTED GRE flags 0x%X ", __func__, greh->flags);
                        return -1;                                
                    }

                    switch(ntohs(greh->protocol))
                    {
                        case ETH_P_TEB: 
                        proto = ETH_P_802_3; //Next header will be 802.3
                        break;

                        case ETH_P_ERSPAN:
                        case ETH_P_ERSPAN2:
                        default:
                        //Not Support yet ...
                        printk(KERN_ERR "%s:UNSUPPORTED protocol %u ", __func__, proto);
                        return -1;
                        break;
                    }

                    if(ipv4_hdr_cnt==1)
                    {
                        /* There may have second IPv4 Header (Inner)
                           Backup first ipv4 Header info to outer field
                           and reserve original GSO hdr field for inner ipv4 header
                        */
                        hdrs->outer_l3proto = hdrs->l3proto;
                        hdrs->outer_ipv4hdr = hdrs->ipv4hdr;
                        hdrs->outer_ipv4_id = hdrs->ipv4_id;
                        hdrs->outer_ipv4hdrlen = hdrs->ipv4hdrlen;
                        
                        //Clean original GSO hdr field 
                        hdrs->l3proto = 0;
                        hdrs->ipv4hdr = 0;
                        hdrs->ipv4_id = 0;
                        hdrs->ipv4hdrlen = 0;
                    }
                }
                break;

            /*TODO add PPP */

            default :
                printk(KERN_ERR "%s:UNSUPPORTED protocol %u ", __func__, proto);
                return -1;
        } /* switch (headerType) */
    }
    return -1;
}
EXPORT_SYMBOL(bcm_parse_gso_hdrs);

