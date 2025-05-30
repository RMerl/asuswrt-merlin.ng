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

#include <linux/types.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <net/ip.h>
#include <net/dsfield.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#if defined(CONFIG_IPV6)
#include <linux/ipv6.h>
#include <net/ipv6.h>
#include <linux/netfilter_ipv6.h>
#endif
#include <linux/bcm_skb_defines.h>
#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif

#if 0

#define DEBUG_DSCP(args) printk args
#define DEBUG_DSCP1(args) printk args

#else

#define DEBUG_DSCP(args)   
#define DEBUG_DSCP1(args)  

#endif

#define DUMP_TUPLE_IPV4(tp)						\
	 DEBUG_DSCP(("tuple %px: %u %pI4:%hu  %pI4:%hu \n", \
				 (tp), (tp)->dst.protonum,				\
				 &(tp)->src.u3.ip, ntohs((tp)->src.u.all),		\
				 &(tp)->dst.u3.ip, ntohs((tp)->dst.u.all))) 


#define DUMP_TUPLE_IPV6(tp)						\
	 DEBUG_DSCP(("tuple %px: %u %pI6 %hu -> %pI6 %hu \n", \
				 (tp), (tp)->dst.protonum,				\
				 (tp)->src.u3.all, ntohs((tp)->src.u.all),		\
				 (tp)->dst.u3.all, ntohs((tp)->dst.u.all)))

#define DYNDSCP_DSCP_MASK 0xfc  /* 11111100 */
#define DYNDSCP_DSCP_SHIFT   2
#define DYNDSCP_DSCP_MAX  0x3f  /* 00111111 */

#define DYNDSCP_INITIALIZING   0
#define DYNDSCP_INHERITED 		1
#define DYNDSCP_SKIP 				2

#define DYNDSCP_LAN2WAN_DEFAULT_DSCP 0
#define DYNDSCP_WAN2LAN_DEFAULT_DSCP 0

#define DYNDSCP_PROC_TRANSTBL_FILENAME "nf_dyndscp_w2ldscp_transtbl"
#define DSCP_MAPPINGTABLE_MAX_SIZE 64
#define DYNDSCP_MAX_PROC_WRITE_BUFLEN 64

static DEFINE_SPINLOCK(nf_dyndscp_lock);

static char dyndscp_proc_buffer[DYNDSCP_MAX_PROC_WRITE_BUFLEN];
static struct proc_dir_entry * dyndscp_proc_file = NULL;

/* structure used to maintain dscp transmarking table entries*/

struct dscpMapping {
	 uint8_t orig;
	 uint8_t new;
};

/* dscp transmarking table entries*/
struct transMarkTable {
	 uint16_t size;
	 uint16_t used;
	 struct dscpMapping *dscp;
};

static struct transMarkTable transMarkTbl;

/*finds the dscp mapping and returns new dscp value
 * returns DYNDSCP_WAN2LAN_DEFAULT_DSCP if no match */ 

uint8_t getDscpfromTransTbl(uint8_t orig)
{
	int i;
	spin_lock_bh(&nf_dyndscp_lock);
	for(i=0; i < transMarkTbl.size; i++)
	{
		if(transMarkTbl.dscp[i].orig == orig)
		{
			spin_unlock_bh(&nf_dyndscp_lock);
			return transMarkTbl.dscp[i].new;
		}

	}

	spin_unlock_bh(&nf_dyndscp_lock);
	return DYNDSCP_WAN2LAN_DEFAULT_DSCP;
}

/* Adds a new DSCP mapping,(over writes the existing mapping for 
 * origDscp value if present)
 * an entry is free if both orig and new are 0 */
int  addDscpinTransTbl(uint8_t origDscp, uint8_t newDscp)
{
	 int i;

	 spin_lock_bh(&nf_dyndscp_lock);
	 /*replace entry */
	 for(i=0; i < transMarkTbl.size; i++)
	 {
			if(transMarkTbl.dscp[i].orig == origDscp)
			{

				 if((transMarkTbl.dscp[i].orig == 0) && (transMarkTbl.dscp[i].new == 0 ) &&(newDscp != 0 )) 
						transMarkTbl.used++;/* new entry special case as intially entries are set to 0*/

				 if((transMarkTbl.dscp[i].orig == 0) && (transMarkTbl.dscp[i].new != 0 ) &&(newDscp == 0 )) 
						transMarkTbl.used--;/*  remove entry special case as intially entries are set to 0*/

				 transMarkTbl.dscp[i].new = newDscp;


				 spin_unlock_bh(&nf_dyndscp_lock);
				 return 0; 
			}
	 }

	 /*new entry */
	 for(i=0; i < transMarkTbl.size; i++)
	 {
			if((transMarkTbl.dscp[i].orig == 0) && (transMarkTbl.dscp[i].new == 0 ))
			{
				 transMarkTbl.dscp[i].orig = origDscp;
				 transMarkTbl.dscp[i].new = newDscp;
				 transMarkTbl.used++;
				 spin_unlock_bh(&nf_dyndscp_lock);
				 return 0; 
			}
	 }

	 spin_unlock_bh(&nf_dyndscp_lock);
	 /*table full */
	 printk(KERN_ERR "%s:Transmark Table is Full\n",__FUNCTION__);
	 return -1; 
}

/* delete a DSCP mapping from trans table */
int  delDscpinTransTbl(uint8_t origDscp)
{
	 int i;

	 spin_lock_bh(&nf_dyndscp_lock);
	 for(i=0; i < transMarkTbl.size; i++)
	 {
			if((transMarkTbl.dscp[i].orig == origDscp) )
			{
				 transMarkTbl.dscp[i].orig = 0;
				 transMarkTbl.dscp[i].new = 0;
				 transMarkTbl.used--;
				 spin_unlock_bh(&nf_dyndscp_lock);
				 return 0; 
			}
	 }

	 printk(KERN_ERR "%s: Entry not found in Transmark Table\n",__FUNCTION__);
	 spin_unlock_bh(&nf_dyndscp_lock);
	 return -1; 
}

/*for marking interface as WAN ;LAB testing purpose */
int setWanIfFlag(char *name)
{
	 struct net_device *dev = NULL;         

	 dev = dev_get_by_name(&init_net, name);  
	 if(dev){
			printk(KERN_INFO "setting %s as WAN device\n",name);
			netdev_wan_set(dev);     
			return 0;
	 } else {
			printk(KERN_ERR "interface %s not found\n",name);
			return -1;
	 }
}

/*
 * updating tos field before pulling ipv6 header leads checksum error
 * Fix: pull header before tos update and push after tos update
 * TODO: need to check why UDP csum is considering entire IPV6 header
 */
static void set_ipv6_dsfield(struct sk_buff *skb, struct ipv6hdr *ipv6h, __u8 mask, __u8 ipv6_tclass)
{
	skb_postpull_rcsum(skb, ipv6h, 4);

	ipv6_change_dsfield(ipv6h, mask, ipv6_tclass);

	skb_postpush_rcsum(skb, ipv6h, 4);
}

/* Entry point into dyndscp module at pre-routing
 * this  function is the core engine of this module
 * */
static unsigned int nf_dyndscp_in(void *priv,
			struct sk_buff *skb,
			const struct nf_hook_state *state)
{
	 struct nf_conn *ct;
	 enum ip_conntrack_info ctinfo;
	 u_int8_t pktDscp; 

	 ct = nf_ct_get(skb, &ctinfo);

	 DEBUG_DSCP1((" %s: seen packet \n",__FUNCTION__));

	 if(!ct) {
			DEBUG_DSCP1((KERN_INFO " %s: seen packet with out flow\n",__FUNCTION__));
			return NF_ACCEPT;
	 }

	 if(ct->bcm_ext.dyndscp.status == DYNDSCP_INHERITED) {
			DEBUG_DSCP1((KERN_INFO "%s: changing tos in pkt to %x \n",__FUNCTION__,
                ct->bcm_ext.dyndscp.dscp[CTINFO2DIR(ctinfo)]));

			if (skb_ensure_writable(skb, sizeof(struct iphdr)))
				 return NF_DROP;

			ipv4_change_dsfield(ip_hdr(skb), (__u8)(~DYNDSCP_DSCP_MASK),
						ct->bcm_ext.dyndscp.dscp[CTINFO2DIR(ctinfo)] << DYNDSCP_DSCP_SHIFT);

	 } else if(ct->bcm_ext.dyndscp.status == DYNDSCP_INITIALIZING) {

			/*TODO do we need to check and skip nf_ct_is_template(), most likley no */

			/*for now we change DSCP only for TCP/UDP */
			if(!((ip_hdr(skb)->protocol == IPPROTO_UDP) || (ip_hdr(skb)->protocol == IPPROTO_TCP))){
				 ct->bcm_ext.dyndscp.status = DYNDSCP_SKIP;
				 return NF_ACCEPT;
			}

         /*TODO: should we skip broadcast packets ?? */

			pktDscp = ipv4_get_dsfield(ip_hdr(skb)) >> DYNDSCP_DSCP_SHIFT;

			if(!SKBMARK_GET_IFFWAN_MARK(skb->mark)) {
				 /* LAN -> WAN packet */

				 DEBUG_DSCP1((" %s: initializing case lan->wan packet \n",__FUNCTION__));

				 if(pktDscp != DYNDSCP_LAN2WAN_DEFAULT_DSCP) {

						if (skb_ensure_writable(skb, sizeof(struct iphdr)))
							 return NF_DROP;

						ipv4_change_dsfield(ip_hdr(skb), (__u8)(~DYNDSCP_DSCP_MASK),
									DYNDSCP_LAN2WAN_DEFAULT_DSCP << DYNDSCP_DSCP_SHIFT);
				 }

			} else {
				 /* WAN -> LAN packet */

				 DEBUG_DSCP1(("%s: initializing case wan->lan packet \n",__FUNCTION__));
				 if (skb_ensure_writable(skb, sizeof(struct iphdr)))
						return NF_DROP;

				 /* inherit tos from packet */
				 if (CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL) {
						/*connection intiated from WAN */
						ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_REPLY] = pktDscp;
						ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_ORIGINAL] = getDscpfromTransTbl(pktDscp);

						ipv4_change_dsfield(ip_hdr(skb), (__u8)(~DYNDSCP_DSCP_MASK),
									ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_ORIGINAL] << DYNDSCP_DSCP_SHIFT);
				 } else {
						/*connection intiated from LAN or LOCAL*/
						ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_ORIGINAL] = pktDscp;
						ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_REPLY] = getDscpfromTransTbl(pktDscp);

						ipv4_change_dsfield(ip_hdr(skb), (__u8)(~DYNDSCP_DSCP_MASK),
									ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_REPLY] << DYNDSCP_DSCP_SHIFT);
#if defined(CONFIG_BLOG)
						/* Notify associated flows in flow cache, so they will relearn with
						 *  new tos values this is needed only for UDP, as TCP flows 
						 *  are created only when packet are seen from both directions 	 
						 */

						if(ip_hdr(skb)->protocol == IPPROTO_UDP){
							blog_lock();
							if ((ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] != BLOG_KEY_FC_INVALID)
								|| (ct->bcm_ext.blog_key[IP_CT_DIR_REPLY] != BLOG_KEY_FC_INVALID)) {
								blog_notify(DESTROY_FLOWTRACK, (void*)ct,
									(uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL],
									(uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_REPLY]);
							}
							set_bit(IPS_BLOG_BIT, &ct->status);  /* re-enable conntrack blogging */
							blog_unlock();

							 DEBUG_DSCP(("%s:blog_notify:DYNAMIC_DSCP_EVENT for\n",__FUNCTION__));
							 DUMP_TUPLE_IPV4(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
						}
#endif
				 }

				 /*update tos status in nf_conn */
				 ct->bcm_ext.dyndscp.status = DYNDSCP_INHERITED;

				 DEBUG_DSCP((KERN_INFO "dynamic tos values(%X, %X) inherited forflow\n",
									ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_ORIGINAL],
									ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_REPLY]));
				 DUMP_TUPLE_IPV4(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
			}

	 } else if(ct->bcm_ext.dyndscp.status == DYNDSCP_SKIP){
			/*handle untracked connections */

	 } else {

			printk(KERN_WARNING " %s :dyndscp unknown status(%d) for flow\n",
						__FUNCTION__, ct->bcm_ext.dyndscp.status);
			DUMP_TUPLE_IPV4(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	 }

	 return NF_ACCEPT;
}

static unsigned int nf_dyndscp_local(void *priv,
			struct sk_buff *skb,
			const struct nf_hook_state *state)
{
	 struct nf_conn *ct;
	 enum ip_conntrack_info ctinfo;

	 /* root is playing with raw sockets. */
	 if (skb->len < sizeof(struct iphdr)
				 || ip_hdr(skb)->ihl * 4 < sizeof(struct iphdr)) {
			if (net_ratelimit())
				 DEBUG_DSCP((KERN_INFO "nf_dyndscp_local: happy cracking.\n"));
			return NF_ACCEPT;
	 }

	 ct = nf_ct_get(skb, &ctinfo);

	 if(!ct){
			DEBUG_DSCP((KERN_INFO "%s: seen packet with out flow\n",__FUNCTION__));
			return NF_ACCEPT;
	 }

	 if(ct->bcm_ext.dyndscp.status == DYNDSCP_INHERITED) {

			if (skb_ensure_writable(skb, sizeof(struct iphdr)))
				 return NF_DROP;

			/* LOCAL -> WAN packet */
			ipv4_change_dsfield(ip_hdr(skb), (__u8)(~DYNDSCP_DSCP_MASK),
						ct->bcm_ext.dyndscp.dscp[CTINFO2DIR(ctinfo)] << DYNDSCP_DSCP_SHIFT);

	 } else if(ct->bcm_ext.dyndscp.status == DYNDSCP_INITIALIZING) {

			if (test_bit(IPS_SEEN_REPLY_BIT, &ct->status)) {
				 /*this happens only with LAN <-> LOCAL so just skip */ 
				 ct->bcm_ext.dyndscp.status = DYNDSCP_SKIP;
			}

	 }

	 return NF_ACCEPT;
}

#if defined(CONFIG_IPV6)
/* Entry point into dyndscp module at pre-routing
 * this  function is the core engine of this module
 * */
static unsigned int nf_dyndscp_in6(void *priv,
			struct sk_buff *skb,
			const struct nf_hook_state *state)
{
	 struct nf_conn *ct;
	 enum ip_conntrack_info ctinfo;
	 u_int8_t pktDscp; 

	 ct = nf_ct_get(skb, &ctinfo);

	 DEBUG_DSCP1((" %s: seen packet \n",__FUNCTION__));

	 if(!ct) {
			DEBUG_DSCP1((KERN_INFO " %s: seen packet with out flow\n",__FUNCTION__));
			return NF_ACCEPT;
	 }

	 if(ct->bcm_ext.dyndscp.status == DYNDSCP_INHERITED) {
			DEBUG_DSCP1((KERN_INFO "%s: changing tos in pkt to %x \n",__FUNCTION__,
                ct->bcm_ext.dyndscp.dscp[CTINFO2DIR(ctinfo)]));

			if (skb_ensure_writable(skb, sizeof(struct ipv6hdr)))
				 return NF_DROP;

			set_ipv6_dsfield(skb, ipv6_hdr(skb), (__u8)(~DYNDSCP_DSCP_MASK),
						ct->bcm_ext.dyndscp.dscp[CTINFO2DIR(ctinfo)] << DYNDSCP_DSCP_SHIFT);

	 } else if(ct->bcm_ext.dyndscp.status == DYNDSCP_INITIALIZING) {


			/*TODO do we need to check and skip nf_ct_is_template(), most likley no */

			/*for now we change DSCP only for TCP/UDP */
			if(!((ipv6_hdr(skb)->nexthdr == IPPROTO_UDP) || (ipv6_hdr(skb)->nexthdr == IPPROTO_TCP))){
				 ct->bcm_ext.dyndscp.status = DYNDSCP_SKIP;
				 return NF_ACCEPT;
			}

         	/*TODO: should we skip broadcast packets ?? */


			pktDscp = ipv6_get_dsfield(ipv6_hdr(skb)) >> DYNDSCP_DSCP_SHIFT;

			if(!SKBMARK_GET_IFFWAN_MARK(skb->mark)) {
				 /* LAN -> WAN packet */

				 DEBUG_DSCP1((" %s: initializing case lan->wan packet \n",__FUNCTION__));

				 if(pktDscp != DYNDSCP_LAN2WAN_DEFAULT_DSCP) {

						if (skb_ensure_writable(skb, sizeof(struct ipv6hdr)))
							 return NF_DROP;

						set_ipv6_dsfield(skb, ipv6_hdr(skb), (__u8)(~DYNDSCP_DSCP_MASK),
									DYNDSCP_LAN2WAN_DEFAULT_DSCP << DYNDSCP_DSCP_SHIFT);
				 }

			} else {
				 /* WAN -> LAN packet */

				 DEBUG_DSCP1(("%s: initializing case wan->lan packet \n",__FUNCTION__));
				 if (skb_ensure_writable(skb, sizeof(struct ipv6hdr)))
						return NF_DROP;

				 /* inherit tos from packet */
				 if (CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL) {
						/*connection intiated from WAN */
						ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_REPLY] = pktDscp;
						ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_ORIGINAL] = getDscpfromTransTbl(pktDscp);

						set_ipv6_dsfield(skb, ipv6_hdr(skb), (__u8)(~DYNDSCP_DSCP_MASK),
									ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_ORIGINAL] << DYNDSCP_DSCP_SHIFT);
				 } else {
						/*connection intiated from LAN or LOCAL*/
						ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_ORIGINAL] = pktDscp;
						ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_REPLY] = getDscpfromTransTbl(pktDscp);

						set_ipv6_dsfield(skb, ipv6_hdr(skb), (__u8)(~DYNDSCP_DSCP_MASK),
									ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_REPLY] << DYNDSCP_DSCP_SHIFT);
#if defined(CONFIG_BLOG)
						/* Notify associated flows in flow cache, so they will relearn with
						 *  new tos values this is needed only for UDP, as TCP flows 
						 *  are created only when packet are seen from both directions 	 
						 */

						if(ipv6_hdr(skb)->nexthdr == IPPROTO_UDP){

							blog_lock();
							if ((ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] != BLOG_KEY_FC_INVALID)
								|| (ct->bcm_ext.blog_key[IP_CT_DIR_REPLY] != BLOG_KEY_FC_INVALID)) {
								blog_notify(DESTROY_FLOWTRACK, (void*)ct,
										(uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL],
										(uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_REPLY]);

							}
							set_bit(IPS_BLOG_BIT, &ct->status);  /* re-enable conntrack blogging */
							blog_unlock();

							 DEBUG_DSCP(("%s:blog_notify:DYNAMIC_DSCP_EVENT for\n",__FUNCTION__));
							 DUMP_TUPLE_IPV6(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
						}
#endif
				 }

				 /*update tos status in nf_conn */
				 ct->bcm_ext.dyndscp.status = DYNDSCP_INHERITED;

				 DEBUG_DSCP((KERN_INFO "dynamic tos values(%X, %X) inherited forflow\n",
									ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_ORIGINAL],
									ct->bcm_ext.dyndscp.dscp[IP_CT_DIR_REPLY]));
				 DUMP_TUPLE_IPV6(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
			}

	 } else if(ct->bcm_ext.dyndscp.status == DYNDSCP_SKIP){
			/*handle untracked connections */

	 } else {

			printk(KERN_WARNING " %s :dyndscp unknown status(%d) for flow\n",
						__FUNCTION__, ct->bcm_ext.dyndscp.status);
			DUMP_TUPLE_IPV6(&ct->bcm_ext.tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	 }

	 return NF_ACCEPT;
}

static unsigned int nf_dyndscp_local6(void *priv,
			struct sk_buff *skb,
			const struct nf_hook_state *state)
{
	 struct nf_conn *ct;
	 enum ip_conntrack_info ctinfo;

	 /* root is playing with raw sockets. */
	 if (skb->len < sizeof(struct ipv6hdr)){
			if (net_ratelimit())
				 DEBUG_DSCP((KERN_INFO "nf_dyndscp_local6: happy cracking.\n"));
			return NF_ACCEPT;
	 }

	 ct = nf_ct_get(skb, &ctinfo);

	 if(!ct){
			DEBUG_DSCP((KERN_INFO "%s: seen packet with out flow\n",__FUNCTION__));
			return NF_ACCEPT;
	 }

	 if(ct->bcm_ext.dyndscp.status == DYNDSCP_INHERITED) {

			if (skb_ensure_writable(skb, sizeof(struct ipv6hdr)))
				 return NF_DROP;

			/* LOCAL -> WAN packet */
			set_ipv6_dsfield(skb, ipv6_hdr(skb), (__u8)(~DYNDSCP_DSCP_MASK),
						ct->bcm_ext.dyndscp.dscp[CTINFO2DIR(ctinfo)] << DYNDSCP_DSCP_SHIFT);

	 } else if(ct->bcm_ext.dyndscp.status == DYNDSCP_INITIALIZING) {

			if (test_bit(IPS_SEEN_REPLY_BIT, &ct->status)) {
				 /*this happens only with LAN <-> LOCAL so just skip */ 
				 ct->bcm_ext.dyndscp.status = DYNDSCP_SKIP;
			}

	 }

	 return NF_ACCEPT;
}
#endif

static struct nf_hook_ops nf_dyndscp_ops[] = {
	 {
			.hook		= nf_dyndscp_in,
			.pf			= PF_INET,
			.hooknum	= NF_INET_PRE_ROUTING,
			.priority	= NF_IP_PRI_MANGLE - 10,/*pre routing do it before mangle table */
	 },
	 {
			.hook		= nf_dyndscp_local,
			.pf			= PF_INET,
			.hooknum	= NF_INET_LOCAL_OUT,
			.priority	= NF_IP_PRI_MANGLE + 10,/*local out do it after mangle table */

	 },
#if defined(CONFIG_IPV6)
	 {
			.hook		= nf_dyndscp_in6,
			.pf			= PF_INET6,
			.hooknum	= NF_INET_PRE_ROUTING,
			.priority	= NF_IP6_PRI_MANGLE - 10,/*pre routing do it before mangle table */
	 },
	 {
			.hook		= nf_dyndscp_local6,
			.pf			= PF_INET6,
			.hooknum	= NF_INET_LOCAL_OUT,
			.priority	= NF_IP6_PRI_MANGLE + 10,/*local out do it after mangle table */

	 },
#endif
};

/* proc interface functions for configuring/reading transmark table 
 * from userspace
 * */

static void *dyndscp_seq_start(struct seq_file *seq, loff_t *pos)
{
	 if(*pos > transMarkTbl.size)
			return NULL;

	 return *pos ? pos : SEQ_START_TOKEN;
}

static void *dyndscp_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	 ++(*pos);
	 if(*pos > transMarkTbl.size)
			return NULL;

	 return &transMarkTbl.dscp[(*pos)-1];
}

static void dyndscp_seq_stop(struct seq_file *seq, void *v)
{
	 return;
}

static int dyndscp_seq_show(struct seq_file *seq, void *v)
{
	 if (v == SEQ_START_TOKEN){
			seq_printf(seq,"WANDSCP\t-->\tLANDSCP Max num entries:%d,"
						"Current num Entries:%d\n",
						transMarkTbl.size, transMarkTbl.used);
	 } else {
			struct dscpMapping *tos = (struct dscpMapping *)v;
			if((tos->orig !=0) && (tos->new !=0))/*show only used entries*/
				 seq_printf(seq, "%02x\t   \t%02x\n",tos->orig,tos->new);
	 }
	 return 0;
}

static struct seq_operations dyndscp_seq_ops = {
	 .start   =  dyndscp_seq_start,
	 .next =  dyndscp_seq_next,
	 .stop =  dyndscp_seq_stop,
	 .show =  dyndscp_seq_show,
};

int nf_dyndscp_proc_open(struct inode *inode, struct file *file)
{
	 return seq_open(file, &dyndscp_seq_ops);
}


static ssize_t nf_dyndscp_proc_write(struct file *file, const char *buffer,
			size_t len, loff_t *offset)
{
	 uint8_t origDscp, newDscp;
	 char wanIfname[32];

	 if(len > DYNDSCP_MAX_PROC_WRITE_BUFLEN)
	 {
			printk(KERN_ALERT "%s: User datalen > max kernel buffer len=%zu\n",
						__FUNCTION__, len);
			return -EFAULT;
	 }

	 if ( copy_from_user(dyndscp_proc_buffer, buffer, len) )
	 {
			printk(KERN_ALERT "%s copy_from_user failure.\n", __FUNCTION__ );
			//kfree( kbuffer );
			return -EFAULT;
	 }

	 DEBUG_DSCP((KERN_INFO "Applying %u bytes configuration\n", len));

	 if(sscanf(dyndscp_proc_buffer,"add %hhi %hhi",&origDscp, &newDscp)) {
			if(addDscpinTransTbl(origDscp,newDscp) < 0)
				 return -EFAULT;
	 } else if(sscanf(dyndscp_proc_buffer,"delete %hhi",&origDscp)) {
			if(delDscpinTransTbl(origDscp) < 0)
				 return -EFAULT;
	 } 
	 else if(sscanf(dyndscp_proc_buffer,"setwanif %s", wanIfname)) {
			if(setWanIfFlag(wanIfname) < 0)
				 return -EFAULT;
	 } 
	 else {
			printk(KERN_ALERT " unknown command/syntax in %s .\n", __FUNCTION__ );
			printk(KERN_ALERT "use 'add' or 'delete' commands Ex: \n");
			printk(KERN_ALERT "add origDscp newDscp >/proc/.../.. \n");
			printk(KERN_ALERT "delete origDscp >/proc/.../.. \n");
			return -EFAULT;
	 }	

	 return len;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static struct proc_ops dyndscp_proc_fops = {
	 .proc_open    = nf_dyndscp_proc_open,
	 .proc_read    = seq_read,
	 .proc_write   = nf_dyndscp_proc_write,
	 .proc_lseek  = seq_lseek,
	 .proc_release = seq_release,
};
#else
static struct file_operations dyndscp_proc_fops = {
	 .owner	 = THIS_MODULE,
	 .open    = nf_dyndscp_proc_open,
	 .read    = seq_read,
	 .write   = nf_dyndscp_proc_write,
	 .llseek  = seq_lseek,
	 .release = seq_release,
};
#endif

static int __net_init nf_dyndscp_proc_init(struct net *net)
{
	 dyndscp_proc_file = proc_create(DYNDSCP_PROC_TRANSTBL_FILENAME, S_IFREG|S_IRUGO|S_IWUSR, net->nf.proc_netfilter, &dyndscp_proc_fops);
	 if ( dyndscp_proc_file == (struct proc_dir_entry *)NULL )
	 {
			printk(KERN_ALERT "Error: Could not initialize /proc/net/netfilter/%s\n",
						DYNDSCP_PROC_TRANSTBL_FILENAME);
			return -ENOMEM;
	 }
	 proc_set_size(dyndscp_proc_file, 80);
	 proc_set_user(dyndscp_proc_file, KUIDT_INIT(0), KGIDT_INIT(0));

	 printk(KERN_DEBUG "/proc/net/netfilter/%s created\n", DYNDSCP_PROC_TRANSTBL_FILENAME);

	 return 0; /* success */
}

static void __net_exit nf_dyndscp_proc_fini(struct net *net)
{
	 remove_proc_entry(DYNDSCP_PROC_TRANSTBL_FILENAME, net->nf.proc_netfilter);
	 printk(KERN_DEBUG "/proc/net/netfilter/%s removed\n", DYNDSCP_PROC_TRANSTBL_FILENAME);
}

static struct pernet_operations nf_dyndscp_net_ops = {
	.init = nf_dyndscp_proc_init,
	.exit = nf_dyndscp_proc_fini,
};

static int __init nf_dyndscp_init(void)
{
	 int ret = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,20,0))
	 need_conntrack();
#endif

	 transMarkTbl.size = DSCP_MAPPINGTABLE_MAX_SIZE;
	 transMarkTbl.dscp = kmalloc((transMarkTbl.size * sizeof(struct dscpMapping)),
																		 GFP_KERNEL);
	 memset(transMarkTbl.dscp, 0, (transMarkTbl.size * sizeof(struct dscpMapping)));

	 /*TODO: need to carfeul with name spaces, do we need to register
            per namespace */
	
	 ret = nf_register_net_hooks(&init_net, nf_dyndscp_ops,
				 ARRAY_SIZE(nf_dyndscp_ops));
	 if (ret < 0) {
			printk("nf_dyndscp: can't register hooks.\n");
			goto cleanup_tbl;
	 }
#if defined(CONFIG_PROC_FS)
	ret = register_pernet_subsys(&nf_dyndscp_net_ops);
	if (ret < 0)
		goto cleanup_hooks;

	 return ret;

cleanup_hooks:
	 nf_unregister_net_hooks(&init_net, nf_dyndscp_ops, ARRAY_SIZE(nf_dyndscp_ops));
#endif
cleanup_tbl:
	 return ret;
}

static void __exit nf_dyndscp_fini(void)
{
#if defined(CONFIG_PROC_FS)
	 unregister_pernet_subsys(&nf_dyndscp_net_ops);
#endif
	 nf_unregister_net_hooks(&init_net, nf_dyndscp_ops, ARRAY_SIZE(nf_dyndscp_ops));
}

MODULE_AUTHOR("broadcom.com");
MODULE_DESCRIPTION("DSCP Inheritance from WAN");
MODULE_LICENSE("GPL");
MODULE_ALIAS("nf_dyndscp-" __stringify(AF_INET));
MODULE_ALIAS("nf_dyndscp");

module_init(nf_dyndscp_init);
module_exit(nf_dyndscp_fini);
