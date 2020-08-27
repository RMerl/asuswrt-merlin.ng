//#include <config/modversions.h>

#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif

#if !CONFIG_NETFILTER
#error "Plz activate netfilter in kernel."
#endif

#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/netdevice.h>
#include <linux/proc_fs.h>
#include <asm/atomic.h>
#include <linux/time.h>

#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>

#include <linux/netfilter.h>
#include <linux/netfilter_bridge.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>

#include <net/netfilter/nf_conntrack.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25)
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_nat_helper.h>

#ifdef CONFIG_COMPAT
#include <asm/compat.h>
#endif

/* Mapping Kernel 2.6 Netfilter API to Kernel 2.4 */
#define ip_conntrack		nf_conn
#define ip_conntrack_get	nf_ct_get
#define ip_conntrack_tuple  nf_conntrack_tuple

#else // older kernel
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_conntrack_tuple.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#endif

#include <linux/netdevice.h>

#ifdef HAVE_BRCM_FC
#include <linux/blog.h>
#include <linux/nbuff.h>
#include <fcache.h>
#endif

#include "internal.h"
#include "skb_access.h"
//#include <shared/bcmenet.h>

#define CONFIG_DEBUG (0) //!< Say 1 to debug
#define CONFIG_DEBUG_SKB (0) //!< Say 1 to dump skb.

#define FWMOD_NAME "fltr" //!< Module name to display in kmesg.

#define ERR(fmt, args...) printk(KERN_ERR " *** ERROR: [%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##args)
#define INFO(fmt, args...) printk(KERN_INFO FWMOD_NAME ": " fmt "\n", ##args)

#if CONFIG_DEBUG
#define DBG(fmt, args...) printk(KERN_DEBUG "[%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##args)
#define assert(_condition) \
	do \
	{ \
		if (unlikely(!(_condition))) \
		{ \
			printk(KERN_ERR "\n" FWMOD_NAME  ": Assertion failed at %s:%d\n", __FUNCTION__, __LINE__); \
			BUG(); \
		} \
	} while(0)
#else
#define DBG(fmt, args...) do { } while (0)
#define assert(_condition) do { } while (0)
#endif

#define MEM_DBG(fmt, args...) printk(KERN_DEBUG "[%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##args)

////////////////////////////////////////////////////////////////////////////////
#define is_skb_upload(_skb, _dev_wan, _dev_lan)		\
	((strcmp((_skb)->dev->name, _dev_wan) != 0) || 	\
		(strcmp((_skb)->dev->name, _dev_lan) == 0))

DEFINE_SPINLOCK(fltr_lock);

#ifdef HAVE_BRCM_FC
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,49)
#define SKIP_SKB(_p) blog_skip(_p, blog_skip_reason_dpi)
#else
#define SKIP_SKB(_p) blog_skip(_p)
#endif

extern void ip_conntrack_fc_resume(struct sk_buff *skb, struct nf_conn *ct, u_int8_t isv4);
//extern int fc_flush(void);
#endif

flt_stat_t flt_stat;

static int dbg_packet = 0;
module_param(dbg_packet, int, 0660);

static int dbg_connt = 0;
module_param(dbg_connt, int, 0660);

static void print_ct_info(struct sk_buff *skb)
{
	unsigned int ct_sip, ct_dip, ct_sip_r, ct_dip_r;
	unsigned short ct_sport, ct_dport, ct_sport_r, ct_dport_r;
	unsigned char ctinfo;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
	struct nf_conn *ct = (struct nf_conn*)skb->nfct;
	struct nf_conntrack_tuple *ct_tuple, *ct_tuple_r;
#else
	struct ip_conntrack *ct = (struct ip_conntrack*)skb->nfct;
	struct ip_conntrack_tuple *ct_tuple, *ct_tuple_r;
#endif

	if (ct == NULL) {
		return ;
	}

	ctinfo = skb->nfctinfo;

	ct_tuple = &(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	ct_sip = GET_TUPLE_SIP(ct_tuple);
	ct_dip = GET_TUPLE_DIP(ct_tuple);
	ct_sport = ntohs(GET_TUPLE_SPORT(ct_tuple));
	ct_dport = ntohs(GET_TUPLE_DPORT(ct_tuple));

	ct_tuple_r = &(ct->tuplehash[IP_CT_DIR_REPLY].tuple);
	ct_sip_r = GET_TUPLE_SIP(ct_tuple_r);
	ct_dip_r = GET_TUPLE_DIP(ct_tuple_r);
	ct_sport_r = ntohs(GET_TUPLE_SPORT(ct_tuple_r));
	ct_dport_r = ntohs(GET_TUPLE_DPORT(ct_tuple_r));

	if (SKB_IP_PRO(skb) == IPPROTO_ICMP) {
		printk("S#%d, CT_SRC=%s, CT_DST=%s, %d/%d\n", 
			   ctinfo, 
			   ip_str((unsigned char*)&ct_sip, 4), ip_str((unsigned char*)&ct_dip, 4), 
			   GET_TUPLE_ICMP_TYPE(ct_tuple), GET_TUPLE_ICMP_CODE(ct_tuple));
//		printk("S#%d, CT_SRC_R=%s, CT_DST_R=%s, %d/%d\n", 
//			   ctinfo, ip_str((unsigned char*)&ct_sip_r, 4), ip_str((unsigned char*)&ct_dip_r, 4), 
//			   GET_TUPLE_ICMP_TYPE(ct_tuple_r), GET_TUPLE_ICMP_CODE(ct_tuple_r));
	} else {
		printk("S#%d, CT_SRC=%s:%d, CT_DST=%s:%d\n", 
			   ctinfo, 
			   ip_str((unsigned char*)&ct_sip, 4), ct_sport, 
			   ip_str((unsigned char*)&ct_dip, 4), ct_dport);
//		printk("S#%d, CT_SRC_R=%s:%d, CT_DST_R=%s:%d\n", 
//			   ctinfo, 
//			   ip_str((unsigned char*)&ct_sip_r, 4), ct_sport_r, 
//			   ip_str((unsigned char*)&ct_dip_r, 4), ct_dport_r);
	}
}

void dump_packet(unsigned char *buff, int len)
{
	int i=0, j=0, maxlen;

	if (len <= 0 || !buff)
		return;

	maxlen = (len > 64) ? 64 : len;

	for (i=0;i<maxlen;i++) {
		printk("%02x", buff[i]&0xFF);
		j++;
		if (j >= 16) {
			printk("\n");
			j=0;
		}
	}
	printk("\n");

}

static void print_pkt_info(struct sk_buff *skb, struct ip_conntrack *conn)
{
	unsigned char *proto = "NA";
	unsigned int sip_i, dip_i;
	unsigned char ip_proto, ip_ihl;

	if (SKB_IP_VER(skb) != 4) {
		printk("Unsupported IP ver=%d, bypass the pkt ...\n", SKB_IP_VER(skb));
	}

	ip_proto = SKB_IP_PRO(skb);
	ip_ihl = SKB_IP_IHL(skb) << 2;

	sip_i = SKB_IP_SIP(skb);
	dip_i = SKB_IP_DIP(skb);

	if (sip_i == 0x0100007F) //127.0.0.1
		return;

	/* TCP/UDP/ICMP */
	if ((ip_proto == IPPROTO_TCP) ||
		(ip_proto == IPPROTO_UDP) ||
		(ip_proto == IPPROTO_ICMP))
	{
		unsigned short sport, dport;
		sport = dport = 0;

		if (ip_proto == IPPROTO_TCP) {
			if ((skb->len - ip_ihl) >= sizeof(struct tcphdr)) {
				struct tcphdr *tcph = (struct tcphdr*)(skb->data + ip_ihl);

				unsigned char *dptr = (unsigned char *)tcph + (SKB_TCP_HLEN(skb)<<2);
				int ip_len = ntohs(SKB_IP_TOT_LEN(skb));
				int tcp_len = ntohs(SKB_IP_TOT_LEN(skb)) - (SKB_IP_IHL(skb)<<2) - (SKB_TCP_HLEN(skb)<<2);

				sport = ntohs(tcph->source);
				dport = ntohs(tcph->dest);
				proto = "TCP";

				printk("%s: mark:%u, SRC=%s:%d, DST=%s:%d, ihl:%d thl:%d, iplen:%d tcplen:%d, seq:%08x ack:%08x, %s%s%s%s%s%s\n", 
					   proto, conn->mark,
					   ip_str((unsigned char*)&sip_i, 4), sport,
					   ip_str((unsigned char*)&dip_i, 4), dport,
					   SKB_IP_IHL(skb), SKB_TCP_HLEN(skb), 
					   ip_len, tcp_len,
					   ntohl(SKB_TCP_SEQ(skb)),ntohl(SKB_TCP_ACK(skb)),
					   SKB_TCP_FLAGS_SYN(skb) ? "SYN," : "",
					   SKB_TCP_FLAGS_PSH(skb) ? "PSH," : "",
					   SKB_TCP_FLAGS_FIN(skb) ? "FIN," : "",
					   SKB_TCP_FLAGS_ACK(skb) ? "ACK," : "",
					   SKB_TCP_FLAGS_RST(skb) ? "RST," : "",
					   SKB_TCP_FLAGS_URG(skb) ? "URG," : "");
				dump_packet(dptr, tcp_len);
			}
		} else if (ip_proto == IPPROTO_UDP) {
			if ((skb->len - ip_ihl) >= sizeof(struct udphdr)) {
				struct udphdr *udph = (struct udphdr*)(skb->data + ip_ihl);

				sport = ntohs(udph->source);
				dport = ntohs(udph->dest);
				proto = "UDP";
				printk("%s: mark:%u, SRC=%s:%d, DST=%s:%d\n", 
					   proto, conn->mark,
					   ip_str((unsigned char*)&sip_i, 4), sport,
					   ip_str((unsigned char*)&dip_i, 4), dport);
			}
		} else {
			if ((skb->len - ip_ihl) >= sizeof(struct icmphdr)) {
				struct icmphdr *icmph = (struct icmphdr*)(skb->data + ip_ihl);

				sport = icmph->type;
				dport = icmph->code;
				proto = "ICMP";
				printk("%s: SRC=%s:%d, DST=%s:%d\n", proto,
					   ip_str((unsigned char*)&sip_i, 4), sport,
					   ip_str((unsigned char*)&dip_i, 4), dport);
			}
		}
	} else {
		printk("SRC=%s, DST=%s\n", 
			   ip_str((unsigned char*)&sip_i, 4), 
			   ip_str((unsigned char*)&dip_i, 4));
	}

	if (dbg_connt && strcmp(proto, "NA") != 0) {
		print_ct_info(skb);
	}
}

/*!
 * \brief Hook point in FORWARD chain of NF filter table.
 * \details This is created to identify application of each connection outgoing connection.
 *
 * \return NF_ACCEPT in most cases. We do not intend to drop anything.
 * \return Otherwise, there's problem here. Please fix this.
 */
static uint32_t hookfn_preroute_filter(uint32_t hooknum, 
									   struct sock *sk, 
									   struct sk_buff *skb, 
									   const struct net_device *in_dev, 
									   const struct net_device *out_dev, 
									   void* okfn)
{
	struct ip_conntrack *conntrack;
	enum ip_conntrack_info ctinfo;
	uint32_t verdict = NF_ACCEPT;

	conntrack = ip_conntrack_get(skb, &ctinfo);
	if (!conntrack) {
		DBG(" * WARNING: empty conntrack in skb"); // Possibly already closed by peer.
		return verdict;
	}

	if(dbg_packet)
		print_pkt_info(skb, conntrack);
	
	if (conntrack->mark < 77) { //we can pre-catch the first 77 packets to perform our task
		conntrack->mark++;
#ifdef HAVE_BRCM_FC
		SKIP_SKB(skb);
#endif
	} else { // else pass it to brcm fc, and fc will not re-send back to pre-routing anymore
		conntrack->mark = 0x99;
		if (SKB_ETH_PRO(skb) == htons(ETH_P_IP)) {
#ifdef HAVE_BRCM_FC
			ip_conntrack_fc_resume(skb, conntrack, 1);
#endif
			//printk("IPV4 resume, fkbmark=0x%lx\n", skb->fkb_mark);
		} else {
			//printk("IPV6 resume\n");
#ifdef HAVE_BRCM_FC
			ip_conntrack_fc_resume(skb, conntrack, 0);
#endif
		}
	}
   	
	return verdict;
}

static struct nf_hook_ops hookfn_ops_preroute_filter;

DEFINE_HOOK(hookfn_preroute_filter);

static int __register_hook(struct nf_hook_ops *ops, unsigned int hooknum, 
						   u_int8_t pf, int prio, void *cb)

{
	memset(ops, 0x00, sizeof(*ops));

	ops->hooknum  = hooknum;
	ops->pf	      = pf;
	ops->priority = prio;
	ops->owner	  = THIS_MODULE;
	ops->hook	  = (nf_hookfn *) cb;

	if (nf_register_hook(ops) != 0) {
		return -1;
	}

	return 0;
}

#if 0
int forward_filter_hook_cb(void *pNBuff, struct net_device *dev)
{
#define IS_TX_DEV_WAN(dev) (dev->priv_flags & IFF_WANDEV)

	uint32_t ctmark = 0;
	uint32_t pktlen = 0;
	//uint32_t *skb_mark = NULL;
	//tdts_res_t ret = TDTS_RES_ACCEPT;
	//tdts_udb_param_t fw_param;

	if (pNBuff == NULL) {
		printk("pNBuff NULL\n");
		return PKT_DONE;
	}

	CNT_INC(fastpath);
	//memset(&fw_param, 0, sizeof(tdts_udb_param_t));
	
	if(IS_FKBUFF_PTR(pNBuff)) {
		FkBuff_t *pfkb = PNBUFF_2_FKBUFF(pNBuff);
		ctmark = B_IQOS_GET_CTMARK(pfkb->mark);
		pktlen = pfkb->len;
		//skb_mark = &pfkb->mark; // FIXME! pfkb->mark is unsigned long!! --mit@20170303
	} else {
		struct sk_buff *pskb = PNBUFF_2_SKBUFF(pNBuff);
		ctmark = B_IQOS_GET_CTMARK(pskb->fkb_mark);
		pktlen = pskb->len;
		//skb_mark = &pskb->mark;
	}

	/*
	SET_SKB_UPLOAD(fw_param.skb_flag, IS_TX_DEV_WAN(dev));
	fw_param.skb_len = pktlen;
	fw_param.ct_mark = &ctmark;
	fw_param.skb_mark = skb_mark;
	fw_param.hook = TDTS_HOOK_FAST_PATH;

	ret = udb_shell_do_fastpath_action(&fw_param);

	switch (ret) {
	case TDTS_RES_DROP:
		return PKT_DROP;
	case TDTS_RES_CONTINUE:
	case TDTS_RES_ACCEPT:
	default:
		return PKT_DONE;
	}
	*/
	return PKT_DONE;
}
#endif

static int forward_filter_init(void)
{
	//BROADSTREAM_IQOS_SET_ENABLE(1);
	//br_fwdcb_register(forward_filter_hook_cb);
	//enet_fwdcb_register(forward_filter_hook_cb);

	DBG("Register forward_hookfn_preroute_filter");
	if (__register_hook(&hookfn_ops_preroute_filter, NF_INET_PRE_ROUTING, PF_INET, 
						NF_IP_PRI_MANGLE - 1, forward_hookfn_preroute_filter) < 0)
	{
		ERR("Cannot register forward_hookfn_preroute_filter");
		return -1;
	}

	return 0;
}

static void forward_filter_exit(void)
{
	DBG("Unregister forward_hook_ops_forward_filter");
	nf_unregister_hook(&hookfn_ops_preroute_filter);

	//enet_fwdcb_register(NULL);
	//br_fwdcb_register(NULL);
	//BROADSTREAM_IQOS_SET_ENABLE(0);

}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief k mod init
 */
static int fltr_init(void)
{
//	INFO("%s is ready\n", FWMOD_NAME);
//	fc_flush();
//	INFO("flush fc\n");

	if(forward_filter_init() < 0) {
		goto error_exit;
	}

	return 0;

error_exit:
	forward_filter_exit();

	return -1;
}

/*!
 * \brief k mod exit
 */
static void fltr_exit(void)
{
	forward_filter_exit();
}

MODULE_DESCRIPTION("ASUS network conntrack filter");
MODULE_AUTHOR("Andrew Hung <andrew_hung@asus.com>");
MODULE_LICENSE("Proprietary");

module_init(fltr_init);
module_exit(fltr_exit);
