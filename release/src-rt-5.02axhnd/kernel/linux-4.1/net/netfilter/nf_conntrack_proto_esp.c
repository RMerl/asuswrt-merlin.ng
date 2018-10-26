#if defined(CONFIG_BCM_KF_PROTO_ESP)
/*
<:copyright-BRCM:2012:GPL/GPL:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:> 
*/
/******************************************************************************
 Filename:       nf_conntrack_proto_esp.c
 Author:         Pavan Kumar
 Creation Date:  05/27/04

 Description:
  Implements the ESP ALG connection tracking.
  Migrated to kernel 2.6.21.5 on April 16, 2008 by Dan-Han Tsai.
  Migrated to kernel 3.4.11 on Jan 21, 2013 by Kirill Tsym
*****************************************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/seq_file.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/ip.h>

#include <net/netfilter/nf_log.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <linux/netfilter/nf_conntrack_proto_esp.h>

enum grep_conntrack {
	ESP_CT_UNREPLIED,
	ESP_CT_REPLIED,
	ESP_CT_MAX
};

static unsigned int esp_timeouts[ESP_CT_MAX] = {
	[ESP_CT_UNREPLIED]	= 30*HZ,
	[ESP_CT_REPLIED]	= 60*HZ,
};

#define IPSEC_INUSE    1
#define MAX_PORTS      8 			/* KT: Changed to match MAX_VPN_CONNECTION */
#define TEMP_SPI_START 1500

struct _esp_table 
{
   u_int32_t l_spi;
   u_int32_t r_spi;
   u_int32_t l_ip;
   u_int32_t r_ip;
   u_int32_t timeout;
   u_int16_t tspi;
   struct nf_conn *ct;
   int    pkt_rcvd;
   int    inuse;
};

static struct _esp_table esp_table[MAX_PORTS];

/*
 * Allocate a free IPSEC table entry.
 */
struct _esp_table *alloc_esp_entry( void )
{
   int idx = 0;

   for( ; idx < MAX_PORTS; idx++ )
   {
      if( esp_table[idx].inuse == IPSEC_INUSE )
         continue;

      memset(&esp_table[idx], 0, sizeof(struct _esp_table));
      esp_table[idx].tspi  = idx + TEMP_SPI_START;
      esp_table[idx].inuse = IPSEC_INUSE;

      pr_debug("[%d] alloc_entry() tspi(%u)\n", idx, esp_table[idx].tspi);

      return (&esp_table[idx]);
   }
   return NULL;
}

/*
 * Search an ESP table entry by ct.
 */
struct _esp_table *search_esp_entry_by_ct(struct nf_conn *ct)
{
   int idx = 0;

   for( ; idx < MAX_PORTS; idx++)
   {
	  if(esp_table[idx].inuse != IPSEC_INUSE )
		 continue;

      pr_debug("Searching entry->ct(%p) <--> ct(%p)\n",
         esp_table[idx].ct, ct);

      /* checking ct */
      if(esp_table[idx].ct == ct)
      {
         pr_debug("Found entry with ct(%p)\n", ct);

         return &esp_table[idx];
      }
   }

   pr_debug("No Entry for ct(%p)\n", ct);
   return NULL;
}

/*
 * Search an ESP table entry by source IP.
 * If found one, update the spi value
 */
struct _esp_table *search_esp_entry_by_ip( const struct nf_conntrack_tuple *tuple, const __u32 spi )
{
   int idx = 0;
   __u32 srcIP = tuple->src.u3.ip;
   __u32 dstIP = tuple->dst.u3.ip;
   struct _esp_table *esp_entry = esp_table;

   for( ; idx < MAX_PORTS; idx++, esp_entry++ )
   {
      pr_debug("   Searching IP %pI4 <-> %pI4,  %pI4\n",
          &srcIP, &esp_entry->l_ip,
          &esp_entry->r_ip);
      
      /* make sure l_ip is LAN IP */
      if( (srcIP == esp_entry->l_ip) && (((unsigned char *)&(srcIP))[0] == 192) )
      {
         pr_debug("   found entry with l_ip\n");
         esp_entry->l_spi = spi;

         /* This is a new connection of the same LAN host */
         if( dstIP != esp_entry->r_ip )
         {
            esp_entry->r_ip = dstIP;
            esp_entry->r_spi = 0;
         }
         return esp_entry;
      }
      else if( srcIP == esp_entry->r_ip )
      {
         pr_debug("   found entry with r_ip\n");
         /* FIXME */
         if( esp_entry->r_spi == 0 )
         {
            pr_debug("   found entry with r_ip and r_spi == 0\n");
            esp_entry->r_spi = spi;
            return esp_entry;
         }
	 /* We cannot handle spi changed at WAN side */
         pr_debug("   found entry with r_ip but r_spi != 0\n");
      }
   }
   pr_debug("No Entry for spi(0x%x)\n", spi);
   return NULL;
}

/*
 * Search an ESP table entry by spi
 */
struct _esp_table *search_esp_entry_by_spi( const __u32 spi, const __u32 srcIP )
{
	int idx = 0;
	struct _esp_table *esp_entry = esp_table;

	for( ; idx < MAX_PORTS; idx++, esp_entry++ )
	{
		pr_debug("   Searching spi 0x%x <-> 0x%x, 0x%x\n",
		spi, esp_entry->l_spi, esp_entry->r_spi);

		if( (spi == esp_entry->l_spi) || (spi == esp_entry->r_spi) )
		{
			pr_debug("   In %s, found entry %d with tspi %u\n",
			  __FUNCTION__, idx, esp_entry->tspi);

			/* l_spi and r_spi may be the same */
			if( (spi == esp_entry->l_spi) && (srcIP == esp_entry->r_ip) )
			{
				pr_debug("l_spi(0x%x)==r_spi\n", spi);
				esp_entry->r_spi = spi;
			}

			return esp_entry;
		}
	}
	pr_debug("No Entry for spi(0x%x)\n", spi);

	return NULL;
}

/* invert esp part of tuple */
static bool esp_invert_tuple(struct nf_conntrack_tuple *tuple,
			    const struct nf_conntrack_tuple *orig)
{
   pr_debug("with spi = %u\n", orig->src.u.esp.spi);

   tuple->dst.u.esp.spi = orig->dst.u.esp.spi;
   tuple->src.u.esp.spi = orig->src.u.esp.spi;
   return true;
}

/* esp hdr info to tuple */
static bool esp_pkt_to_tuple(const struct sk_buff *skb, unsigned int dataoff,
                            struct nf_conntrack_tuple *tuple)
{
   struct esphdr _esphdr, *esphdr;
   struct _esp_table *esp_entry = NULL;

   esphdr = skb_header_pointer(skb, dataoff, sizeof(_esphdr), &_esphdr);
   if( !esphdr ) 
   {
      /* try to behave like "nf_conntrack_proto_generic" */
      tuple->src.u.all = 0;
      tuple->dst.u.all = 0;
      return true;
   }

   pr_debug("Enter pkt_to_tuple() with spi 0x%x\n", esphdr->spi);
   /* check if esphdr has a new SPI:
    *   if no, update tuple with correct tspi and increment pkt count;
    *   if yes, check if we have seen the source IP:
    *             if yes, do the tspi and pkt count update
    *             if no, create a new entry
    */

   if( ((esp_entry = search_esp_entry_by_spi(esphdr->spi, tuple->src.u3.ip)) == NULL) )
   {
      if( (esp_entry = 
           search_esp_entry_by_ip(tuple, esphdr->spi)) == NULL )
      {
#if 0
      /* Because SA is simplex, it's possible that WAN starts connection first.
	  * We need to make sure that the connection starts from LAN.
	  */
         if( ((unsigned char *)&(tuple->src.u3.ip))[0] != 192 )
	 {
 	      pr_debug("srcIP %pI4 is WAN IP, DROP packet\n", &tuple->src.u3.ip);
	      return false;
	 }
#endif
         esp_entry = alloc_esp_entry();
         if( esp_entry == NULL ) 
         {
            pr_debug("Too many entries. New spi(0x%x)\n", esphdr->spi);
            return false;
         }

         esp_entry->l_spi = esphdr->spi;
         esp_entry->l_ip = tuple->src.u3.ip;
         esp_entry->r_ip = tuple->dst.u3.ip;
      }

   }

   pr_debug("esp_entry: tspi(%u) l_ip[%pI4]-->r_ip[%pI4] tuple: srcIP[%pI4]-->dstIP[%pI4]\n",
         esp_entry->tspi,
         &esp_entry->l_ip, &esp_entry->r_ip,
         &tuple->src.u3.ip, &tuple->dst.u3.ip);

   tuple->dst.u.esp.spi = tuple->src.u.esp.spi = esp_entry->tspi;
   esp_entry->pkt_rcvd++;

   return true;
}

/* print esp part of tuple */
static void esp_print_tuple(struct seq_file *s,
                           const struct nf_conntrack_tuple *tuple)
{
   seq_printf(s, "srcspi=0x%x dstspi=0x%x ",
              tuple->src.u.esp.spi, tuple->dst.u.esp.spi);
}

/* print private data for conntrack */
static void esp_print_conntrack(struct seq_file *s, struct nf_conn *ct)
{
   seq_printf(s, "timeout=%u, stream_timeout=%u ",
              (ct->proto.esp.timeout / HZ),
              (ct->proto.esp.stream_timeout / HZ));
}

static unsigned int *esp_get_timeouts(struct net *net)
{
	return esp_timeouts;
}

/* Returns verdict for packet, and may modify conntrack */
static int esp_packet(struct nf_conn *ct,
				const struct sk_buff *skb,
                unsigned int dataoff,
                enum ip_conntrack_info ctinfo,
                u_int8_t pf,
                unsigned int hooknum,
  		        unsigned int *timeouts)
{
   struct esphdr _esphdr, *esphdr;
   struct iphdr *iph = ip_hdr(skb);

   esphdr = skb_header_pointer(skb, dataoff, sizeof(_esphdr), &_esphdr);

   pr_debug("(0x%x) %pI4 <-> %pI4 status %s info %d %s\n",
	  esphdr->spi, &iph->saddr, &iph->daddr, (ct->status & IPS_SEEN_REPLY) ? "SEEN" : "NOT_SEEN",
	  ctinfo, (ctinfo == IP_CT_NEW ) ? "CT_NEW" : "SEEN_REPLY" );

   /* If we've seen traffic both ways, this is some kind of ESP
      stream.  Extend timeout. */
   if( test_bit(IPS_SEEN_REPLY_BIT, &ct->status) ) 
   {
      nf_ct_refresh_acct(ct, ctinfo, skb, ct->proto.esp.stream_timeout);
      /* Also, more likely to be important, and not a probe */
      if( !test_and_set_bit(IPS_ASSURED_BIT, &ct->status) )
         nf_conntrack_event_cache(IPCT_ASSURED, ct);
   } 
   else
      nf_ct_refresh_acct(ct, ctinfo, skb, ct->proto.esp.timeout);

   return NF_ACCEPT;
}

/* Called when a new connection for this protocol found. */
static bool esp_new(struct nf_conn *ct, const struct sk_buff *skb,
                   unsigned int dataoff, unsigned int *timeouts)
{
   struct iphdr *iph = ip_hdr(skb);
   struct _esp_table *esp_entry;
   struct esphdr _esphdr, *esphdr;

   ct->proto.esp.stream_timeout = timeouts[ESP_CT_UNREPLIED];
   ct->proto.esp.timeout = timeouts[ESP_CT_UNREPLIED];

   esphdr = skb_header_pointer(skb, dataoff, sizeof(_esphdr), &_esphdr);

   pr_debug("NEW SPI(0x%x) %pI4 <-> %pI4 ct(%p)\n",
     esphdr->spi, &iph->saddr, &iph->daddr, ct);

   if( (esp_entry = search_esp_entry_by_spi(esphdr->spi, 0)) != NULL ) {
      esp_entry->ct = ct;
   } else {
	  pr_debug("ERR: In esp_new(), cannot find an entry with SPI %x\n", esphdr->spi);
      return false;
   }

   return true;
}

/* Protect conntrack agaist broken packets. Code referenced from nf_conntrack_proto_tcp.c. */
static int esp_error(struct net *net, struct nf_conn *tmpl,
                     struct sk_buff *skb,
                     unsigned int dataoff,
                     enum ip_conntrack_info *ctinfo,
                     u_int8_t pf,
                     unsigned int hooknum)
{
   const struct esphdr *esphdr;
   struct esphdr _esphdr;

   /* smaller than minimal ESP header? */
   esphdr = skb_header_pointer(skb, dataoff, sizeof(_esphdr), &_esphdr);
   if (esphdr == NULL) {
      if (LOG_INVALID(net, IPPROTO_ESP))
         nf_log_packet(net, pf, 0, skb, NULL, NULL, NULL,
                       "nf_ct_esp: short packet ");
      return -NF_ACCEPT;
   }
   return NF_ACCEPT;
}

/* Called when the connection is deleted. */
static void esp_destroy(struct nf_conn *ct)
{
	struct _esp_table *esp_entry = NULL;

	pr_debug("DEL ESP entry ct(%p)\n", ct);
	if ((esp_entry = search_esp_entry_by_ct(ct))) {
		memset(esp_entry, 0, sizeof(struct _esp_table));
	} else {
		pr_debug("ERR: DEL ESP Failed for ct(%p): no such entry\n", ct);
	}
}

/* protocol helper struct */
struct nf_conntrack_l4proto nf_conntrack_l4proto_esp4 = {
   .l3proto = PF_INET,
   .l4proto = IPPROTO_ESP,
   .name = "esp",
   .pkt_to_tuple = esp_pkt_to_tuple,
   .invert_tuple = esp_invert_tuple,
   .print_tuple = esp_print_tuple,
   .print_conntrack = esp_print_conntrack,
   .get_timeouts    = esp_get_timeouts,
   .packet = esp_packet,
   .new = esp_new,
   .error = esp_error,
   .destroy = esp_destroy,
   .me = THIS_MODULE,
};

int __init nf_ct_proto_esp_init(void)
{
   return nf_ct_l4proto_register(&nf_conntrack_l4proto_esp4);
}

void __exit nf_ct_proto_esp_fini(void)
{
   nf_ct_l4proto_unregister(&nf_conntrack_l4proto_esp4);
}
module_init(nf_ct_proto_esp_init);
module_exit(nf_ct_proto_esp_fini);

MODULE_LICENSE("GPL");
#endif
