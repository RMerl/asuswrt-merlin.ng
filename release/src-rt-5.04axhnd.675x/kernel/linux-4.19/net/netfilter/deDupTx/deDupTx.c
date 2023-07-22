#include <linux/version.h>
#include <linux/init.h>
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
#include <linux/list.h>

#include <linux/netdevice.h>
#include <linux/proc_fs.h>
#include <asm/atomic.h>
#include <linux/time.h>

#include <linux/skbuff.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>

#include <linux/netfilter.h>
#include <linux/netfilter_bridge.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>

#include <net/netfilter/nf_conntrack.h>
#include <linux/hashtable.h>
#include <linux/jiffies.h>

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

#ifdef CONFIG_NF_CONNTRACK_CHAIN_EVENTS
#include <linux/notifier.h>
#endif
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/x_tables.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_ecache.h>

#include <linux/netdevice.h>

#ifdef HAVE_BRCM_FC
#include <linux/blog.h>
#include <linux/nbuff.h>
#include <fcache.h>
#endif


#include <linux/proc_fs.h>
#include <linux/inet.h>


#define ERR(fmt, args...) printk(KERN_ERR " *** ERROR: [%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##args)
#define INFO(fmt, args...) printk(KERN_INFO FWMOD_NAME ": " fmt "\n", ##args)


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#define DEFINE_HOOK_FUNC(__func) \
uint32_t forward_##__func(   \
	const void *priv, struct sk_buff *skb, \
        const struct nf_hook_state *state) \
{          \
         return (__func(state->hook, state->sk, skb, state->in, state->out, state->okfn)); \
}
#else
#define DEFINE_HOOK_FUNC(__func) \
uint32_t forward_##__func(   \
	const struct nf_hook_ops *ops, struct sk_buff *skb, \
	const struct nf_hook_state *state) \
{          \
	 return (__func(ops->hooknum, state->sk, skb, state->in, state->out, state->okfn)); \
}
#endif

#define DEFINE_HOOK(__func)		\
	static uint32_t __func(				\
		uint32_t hooknum, struct sock *sk, struct sk_buff *skb,	\
		const struct net_device *in_dev, const struct net_device *out_dev,	\
		void* okfn);	\
	DEFINE_HOOK_FUNC(__func);

#define SKB_IP(skb)			( (struct iphdr*)   (skb->head + skb->network_header) )
#define SKB_IP_FRAG_OFF(skb)	( SKB_IP(skb)->frag_off )

#define GET_TUPLE_SIP(__tuple)	(__tuple)->src.u3.ip
#define GET_TUPLE_DIP(__tuple)	(__tuple)->dst.u3.ip
#define GET_TUPLE_SPORT(__tuple)	(__tuple)->src.u.all
#define GET_TUPLE_DPORT(__tuple)	(__tuple)->dst.u.all



#define DEBUG_DUMP 0

#ifdef CONFIG_NF_CONNTRACK_CHAIN_EVENTS
struct notifier_block ct_event_notifier;
#else
struct nf_ct_event_notifier ct_event_notifier;
#endif
int ct_event_notifier_registered = 0;



#define DIR_DOWNLINK 0
#define DIR_UPLINK 1

#define UP_TOS_BYTE 0xC8 /* Detect OPPO TOS , v1->0xC8, v2->0xB8 */
#define DN_TOS_BYTE 0xB8 /* 0XB8 meet rfc 8325 TOS <-> DSCP map VO */

#define CLONE_MARK 	6    /* DNLINK to clone wlan0 through wlan1 */
#define TC_QOS_MARK 10   /* UPLINK to go through with tc rule */

#define DUP_EN_TRESHOLD 2   /* accumlumated dup_en deteced count */
#define QOS_EN_TRESHOLD 2	/* accumlumated qos_en deteced count */

#define DUP_EN_OFF_LATENCY 3000 /* routerboost ct last_dup_jiffies lapsed ms time*/
#define QOS_EN_OFF_LATENCY 3000 /* routerboost ct last_qos_jiffies lapsed ms time */

#define TIMER_DETECT_EN_MS 2000  /* how long detect qos and dup off worker will be invoked */
#define TIMEOUT_MS 5000 		/* max aged time for tx_pkt hash node */

#define IP_FLAG_MF 0x01
#define IP_OFFMASK 0x1fff

struct tx_udp_packet {
     uint16_t id;
     unsigned int src_ip;        /* from origin_tuple src */
     unsigned short src_port;
	 unsigned int dst_ip;		 /* from origin_tuple dst */
	 unsigned short dst_port;
     unsigned long jiffies;
     struct hlist_node node;
};

struct router_boost_ct {
	struct net *net;
	struct nf_conntrack_zone *zone;
	struct nf_conntrack_tuple origin_tuple;
	int is_qos_en;
	int is_dup_en;
	unsigned long accum_dup_count;
	unsigned long accum_qos_count;
	unsigned long last_dup_jiffies;
	unsigned long last_qos_jiffies;
	int destroyed;
	struct list_head list;
};

LIST_HEAD(router_boost_ct_list);

static void detect_en_worker(struct work_struct *work);
static struct workqueue_struct *det_en_wq __read_mostly = NULL;
static DECLARE_DELAYED_WORK(detect_en_wk, detect_en_worker);
static int detect_en_worker_alive = 0;


struct ipv4_node {
     unsigned int ip;
	 struct hlist_node node;
};
#define PACKET_FILTER_IP_BITS 5 
static DEFINE_HASHTABLE(ipv4_table, PACKET_FILTER_IP_BITS);

#define PACKET_ID_BITS 8  
static DEFINE_HASHTABLE(packets_table, PACKET_ID_BITS);

int  size = 0;
static DEFINE_SPINLOCK(size_lock);


struct proc_dir_entry *de_dir;

unsigned long filter_ip_count = 0;

unsigned int dbg_flag = 0;
unsigned int force_qos = 0;
unsigned int force_dup = 0;
unsigned int qos_en_off_latency = QOS_EN_OFF_LATENCY;
unsigned int dup_en_off_latency = DUP_EN_OFF_LATENCY;
unsigned int dup_det_timeout = TIMEOUT_MS;
unsigned int dump_qos_en = 1;
unsigned int dump_dup_en = 1;

void dump_packet(unsigned char *buff, int len);
void dump_tuple(struct nf_conntrack_tuple *ct_tuple, char *buf);

static int de_ip_seq_show(struct seq_file *s, void *v)
{
	int i = 0; 
	struct ipv4_node *p_tmp = NULL;
	for(i = 0; i < HASH_SIZE(ipv4_table); i++) {
		if(!hlist_empty(&ipv4_table[i])) {
			hlist_for_each_entry(p_tmp, &ipv4_table[i], node) {
				seq_printf(s, "deDupTx ip:%x ip_str:%pI4\n",p_tmp->ip, &p_tmp->ip);
			}
		}
    }
       
	return 0;
}

static int de_ip_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, de_ip_seq_show, NULL);
}

/* 
#echo "a 192.168.51.5" > /proc/dedup/ip
#echo "d 192.168.51.5" > /proc/dedup/ip 
*/
static ssize_t de_ip_seq_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *pos)
{
	char buf[INET_ADDRSTRLEN + 10] = {0};
	size_t len = min(sizeof(buf) - 1, count);
	u_int32_t ip, key;
	struct ipv4_node *p_new = NULL, *p_old = NULL;

	if (copy_from_user(buf, buffer, len))
		return count;
	buf[len] = 0;

#if 0	
	dump_packet((unsigned char *)buf, len);
#endif	
	if(!(buf[0] == 'd' || buf[0] == 'a'))
		goto out;

	ip = in_aton(buf+2);
	key = ip;

	if(buf[0] == 'a')
	{   hash_for_each_possible(ipv4_table, p_old, node, key) {
			if(p_old->ip == ip)
				goto out;
		}
		p_new = kmalloc(sizeof(struct ipv4_node), GFP_ATOMIC);
       	if(!p_new)
          goto out;		
		p_new->ip = ip;
		hash_add(ipv4_table, &p_new->node, key);
		filter_ip_count++;
	} 
	else if(buf[0] == 'd')
	{
		hash_for_each_possible(ipv4_table, p_old, node, key) {
			if(p_old->ip == ip)
			{	
				hash_del(&p_old->node);
				kfree(p_old);
				filter_ip_count--;
				goto out;
			}
		}		
	}
	
out:	
	return strnlen(buf, len);
}


static int de_dbg_seq_show(struct seq_file *s, void *v)
{
	seq_printf(s, "deDupTx: dbg_flag=0x%x\n", dbg_flag);
	seq_printf(s, "deDupTx: force_qos=0x%x force_dup=0x%x dump_qos=0x%x dump_dup=0x%x\n"
	"qos_en_off_latency=%d dup_en_off_latency=%d dup_det_timeout=%d\n",
	force_qos, force_dup, dump_qos_en, dump_dup_en, qos_en_off_latency, 
	dup_en_off_latency, dup_det_timeout);
	   
	return 0;
}

static int de_dbg_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, de_dbg_seq_show, NULL);
}

/* 
#echo 0x3 > /proc/dedup/dbg  (force qos , force dup, and all rb_ct_list)
#echo 0xF > /proc/dedup/dbg  (force qos, force dup, dump_qos_en , dump_qos_en )
#echo 0x02010103 > /proc/dedup/dbg  
(force qos and dup, qos_en_off 1000, dup_en_off 1000, timeout 2000
*/
static ssize_t de_dbg_seq_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *pos)
{
	char buf[20] = {0};
	size_t len = min(sizeof(buf) - 1, count);
	unsigned long new_flag = 0;
	
	if (copy_from_user(buf, buffer, len))
		return count;
	buf[len] = 0;
#if 0	
	dump_packet((unsigned char *)buf, len);
#endif	
	if(len > 0 && kstrtoul(buf, 0, &new_flag)==0) {
		dbg_flag = new_flag;
		force_qos = ( dbg_flag>>0)&0x1;
		force_dup = (dbg_flag>>1)&0x1;
		dump_qos_en = (dbg_flag>>2)&0x1;
		dump_dup_en = (dbg_flag>>3)&0x1;
		qos_en_off_latency = (dbg_flag>>8&0xFF)*1000;
		dup_en_off_latency = (dbg_flag>>16&0xFF)*1000;
		dup_det_timeout = (dbg_flag>>24&0xFF)*1000;
	}
out:	
	return strnlen(buf, len);
}


static int de_rblist_seq_show(struct seq_file *s, void *v)
{
	struct router_boost_ct *node = NULL;	
	struct nf_conntrack_tuple *ct_tuple_origin;
	struct list_head *iter, *tmp;
	struct tuple_list *item;
	char buf[200] = {0};
	int idx = 0;
 
    list_for_each_safe(iter, tmp, &router_boost_ct_list) {
      node = list_entry(iter, struct router_boost_ct, list);
	  ct_tuple_origin = &node->origin_tuple;
	  if(ct_tuple_origin) {
		if((dump_qos_en == 0 && dump_dup_en == 0) ||
		   (dump_qos_en == 1&& node->is_qos_en) ||
			(dump_dup_en == 1&& node->is_dup_en) ) 
		{
			idx ++; 
			dump_tuple(ct_tuple_origin, buf);
			seq_printf(s, "---------------------idx=%d---------------------\n", idx);
			seq_printf(s, "%s", buf);
			seq_printf(s, "is_dup_en=%d is_qos_en=%d D=%d\n",
				node->is_dup_en, node->is_qos_en, node->destroyed);
			seq_printf(s, "accum_dup_cnt=%lu accum_qos_cnt=%lu\n",
				node->accum_dup_count, node->accum_qos_count);
			seq_printf(s, "last_dup=%lu last_qos=%lu\n", 
				node->last_dup_jiffies, node->last_qos_jiffies);
		}
	  }
    }	   
	return 0;
}

static int de_rblist_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, de_rblist_seq_show, NULL);
}


static const struct file_operations de_ip_fops = {
	.owner		= THIS_MODULE,
	.open		= de_ip_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= de_ip_seq_write,
};


static const struct file_operations de_dbg_fops = {
	.owner		= THIS_MODULE,
	.open		= de_dbg_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= de_dbg_seq_write,
};

static const struct file_operations de_rblist_fops = {
	.owner		= THIS_MODULE,
	.open		= de_rblist_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static inline
char* ip_str(unsigned char *ip, int ip_ver)
{
	static int iter = 0;
	static unsigned char ipstr_buf[4][25];

	unsigned char *ipstr = ipstr_buf[iter++ & 3];
	*ipstr = 0;

	if (ip_ver == 4) {
		snprintf(ipstr, 25,
				 "%d.%d.%d.%d", 
				 ip[0], ip[1], ip[2], ip[3]);
	} 
	return ipstr;
}


void dump_packet(unsigned char *buff, int len)
{
	int i=0, j=0, maxlen;
	if (len <= 0 || !buff)
		return;
	maxlen = (len > 64) ? 64 : len;
	for (i=0;i<maxlen;i++) {
		printk("%02x ", buff[i]&0xFF);
		j++;
		if (j >= 16) {
			printk("\n");
			j=0;
		}
	}
	printk("\n");

}

struct router_boost_ct* allocate_router_boost_ct(struct nf_conn *ct)
{
	struct router_boost_ct *p_new = NULL;
	struct nf_conntrack_tuple *ct_tuple_origin;
	struct nf_conntrack_zone *zone;
	struct net *net;
	
	net = nf_ct_net(ct);
	zone = nf_ct_zone(ct);

	ct_tuple_origin = &(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	p_new = kmalloc(sizeof(struct router_boost_ct), GFP_ATOMIC);
	if (p_new == NULL) {
		printk("deDupTx: ERR new rb_ct ailed.\n");
		return NULL;
	}
	p_new->is_qos_en = 0;
	p_new->is_dup_en = 0;
	p_new->accum_dup_count = 0;
	p_new->accum_qos_count = 0;
	p_new->last_dup_jiffies = 0;
	p_new->last_qos_jiffies = 0;
	p_new->destroyed = 0;
	memcpy(&p_new->origin_tuple, ct_tuple_origin, sizeof(struct nf_conntrack_tuple));
	
	p_new->net = net;
	p_new->zone = zone;
	
	list_add(&p_new->list, &router_boost_ct_list);
 
	return p_new;
}

struct router_boost_ct* find_router_boost_ct(struct nf_conn *ct)
{
	struct router_boost_ct *node = NULL;	
	struct nf_conntrack_tuple *ct_tuple_origin;
	struct list_head *iter, *tmp;
	struct tuple_list *item;
 
	if (ct == NULL) {
		goto out;
	}

	ct_tuple_origin = &(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	
	/* look for router_boost by origin tuple */
    list_for_each_safe(iter, tmp, &router_boost_ct_list) {
      node = list_entry(iter, struct router_boost_ct, list);
      if (nf_ct_tuple_equal(ct_tuple_origin, &node->origin_tuple)) {
			return node;
      }
    }	
out:	
	return NULL;
}

/* conntrack destroy event callback function */
#ifdef CONFIG_NF_CONNTRACK_CHAIN_EVENTS
static int ct_event_cb(struct notifier_block *this, unsigned long events, void *ptr) {
  struct nf_ct_event *item = ptr;
#else
static int ct_event_cb(unsigned int events, struct nf_ct_event *item) {
#endif
	struct nf_conn *ct;
	struct nf_conntrack_tuple *ct_tuple_reply, *ct_tuple_original;
	struct nf_conntrack_tuple *ct_tuple, *ct_tuple_r;
	struct router_boost_ct *node = NULL;
	uint8_t protonum;
	unsigned int ct_sip, ct_dip, ct_sip_r, ct_dip_r;
	unsigned short ct_sport, ct_dport, ct_sport_r, ct_dport_r;
	
	ct = item->ct;
	if (ct == NULL || !(events & (1 << IPCT_DESTROY))) {
		return 0;
	}

#if 0	
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
	  
	protonum = (ct_tuple->dst).protonum;
	if (protonum != IPPROTO_UDP) {
		return 0;
	}
	printk("=============\n");	
	printk("Destroy CT_SRC=%s:%d, CT_DST=%s:%d\n",
			   ip_str((unsigned char*)&ct_sip, 4), ct_sport, 
			   ip_str((unsigned char*)&ct_dip, 4), ct_dport);
	printk("Destroy CT_SRC_R=%s:%d, CT_DST=%s:%d\n",
			   ip_str((unsigned char*)&ct_sip_r, 4), ct_sport_r, 
			   ip_str((unsigned char*)&ct_dip_r, 4), ct_dport_r);
#endif	
	
	node = find_router_boost_ct(ct);
	if(node)
	{
		node->destroyed = 1;
	}
	return 0;
}

int skb_in_filter_list(struct sk_buff *skb, struct nf_conn *ct)
{
	int dir = -1;
	unsigned int ct_sip, ct_dip, ct_sip_r, ct_dip_r;
	unsigned short ct_sport, ct_dport, ct_sport_r, ct_dport_r;
	unsigned int sip, dip;
	unsigned short sport, dport;
	struct iphdr *ip;
    struct udphdr *udp;
	unsigned int compare_ip = 0; 
	struct nf_conntrack_tuple *ct_tuple, *ct_tuple_r;
	int i = 0; 
	struct ipv4_node *p_tmp = NULL;
	
	if (ct == NULL) {
 		return dir;
	}
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
	
	ip = ip_hdr(skb);  	
	udp = udp_hdr(skb);
	
	if(!ip || !udp || ct_sport_r == 53 || ct_dport == 53 ) /* dns ct come/leave too many */
	{
		return dir;
	}
	if(ip->saddr == ct_sip )
	{	
		//uplink, compare to skb saddr
		compare_ip = ip->saddr; 
		dir = DIR_UPLINK;
	}
	else if (ip->saddr == ct_sip_r )
	{
		//dnlink, compare to reply tuple daddr
		compare_ip = ct_sip;
		dir = DIR_DOWNLINK;
	}
	
	for(i = 0; i < HASH_SIZE(ipv4_table); i++) {
		if(!hlist_empty(&ipv4_table[i])) {
			hlist_for_each_entry(p_tmp, &ipv4_table[i], node) {
				if( compare_ip == p_tmp->ip)
					return dir;
			}
		}
    }
	return -1;
}

void dump_tuple(struct nf_conntrack_tuple *ct_tuple, char *buf)
{
	unsigned int ct_sip, ct_dip, ct_sip_r, ct_dip_r;
	unsigned short ct_sport, ct_dport, ct_sport_r, ct_dport_r;
	char buffer[100]={0};
	ct_sip = GET_TUPLE_SIP(ct_tuple);
	ct_dip = GET_TUPLE_DIP(ct_tuple);
	ct_sport = ntohs(GET_TUPLE_SPORT(ct_tuple));
	ct_dport = ntohs(GET_TUPLE_DPORT(ct_tuple));
	sprintf(buffer, "CT_SRC=%s:%d, CT_DST=%s:%d\n", 
			   ip_str((unsigned char*)&ct_sip, 4), ct_sport, 
			   ip_str((unsigned char*)&ct_dip, 4), ct_dport);
	if(buf == NULL)
		printk("%s", buffer);
	else
		strcpy(buf, buffer);	
}

int check_ct_valid(struct net *net, struct nf_conntrack_zone *zone, struct nf_conntrack_tuple *tuple)
{
	int ret = 0;
	struct nf_conntrack_tuple_hash *tuple_hash;
  
    tuple_hash = nf_conntrack_find_get(net, zone, tuple);
	if(tuple_hash == NULL)
		return 0;
	else
		return 1;
}
static void detect_en_worker(struct work_struct *work)
{
	struct router_boost_ct *node = NULL;	
	struct nf_conntrack_tuple *ct_tuple_origin;
	struct list_head *iter, *tmp;
	
	/* look for router_boost by origin tuple */
    list_for_each_safe(iter, tmp, &router_boost_ct_list) {
      node = list_entry(iter, struct router_boost_ct, list);
	  if(node->is_qos_en && 
			jiffies - node->last_qos_jiffies > msecs_to_jiffies(qos_en_off_latency))
	  {
		  node->is_qos_en = 0;
		  //node->accum_qos_count = 0;
	  }
	  if(node->is_dup_en &&
		jiffies - node->last_dup_jiffies > msecs_to_jiffies(dup_en_off_latency))
	  {
		  node->is_dup_en = 0;
		  //node->accum_dup_count = 0;
	  }
	  
	  // check ct is valid?
	  if(node->destroyed == 1 ||
		check_ct_valid(node->net, node->zone, &node->origin_tuple)== 0)
	  {
#if 0		  
		printk("d rb_node\n");
		dump_tuple(&node->origin_tuple, NULL);
#endif
		list_del(&node->list);
		kfree(node);
		  
	  }		
    }
	
	// fire next check
	if (det_en_wq != NULL && detect_en_worker_alive)
		queue_delayed_work(det_en_wq, &detect_en_wk, msecs_to_jiffies(TIMER_DETECT_EN_MS));
	
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
	uint32_t verdict = NF_ACCEPT;
	unsigned char *proto = "NA";
    unsigned int sip_i, dip_i;
	unsigned char ip_proto, ip_ihl;
	unsigned short sport, dport;
	uint16_t id;       
	struct tx_udp_packet new_tx, *p_new_tx, *p_old_tx; 	
	struct iphdr *ip;
	struct udphdr *udp;
	int hit = 0;
	int dn = 0;
	struct router_boost_ct *rb_ct = NULL;
	struct nf_conntrack_tuple *ct_tuple;
	unsigned long old_hit_jiffies = 0;
	struct nf_conn *ct;
    enum ip_conntrack_info ctinfo;
	unsigned char flags = 0;
	unsigned int fragment = 0;
	
	
	sport = dport = 0;
	ip = ip_hdr(skb);

	if (ip->version != 4 || ip->protocol != IPPROTO_UDP)
	{
		goto err;
    } 
	
	ct = nf_ct_get(skb, &ctinfo);     
    if(!ct)
	{
		goto err;
	}

    if( filter_ip_count <= 0 || (dn = skb_in_filter_list(skb, ct)) < 0) 
	{	
	 	goto err;
	}
	

/*   PREROUTING ingress skb dst is still wan pulbic ip instead local lan ip, 
	 can't use either "-p udp -d addr_src -j SKIPLOG" or "-s addr_dst" to skip blog,
	 we need to give tc mark and ingress clone mark, have to be different value with 1 for ioslaton
	 conntrack match might be alternative, but we use blog_skip to make thing sample
	 To prevent iptables -t mangle -A PREROUTING -s ip -jSKIPLOG or iptables -j MARK 0x01 mess unexpecedly
	 make fc not enable by every skb from filter_ips
*/
#if defined(CONFIG_BLOG)
    blog_skip(skb, blog_skip_reason_nf_xt_skiplog);
#endif
	/* bypass fragment pkts */
	flags = (unsigned char)((ntohs(SKB_IP_FRAG_OFF(skb)) & 0xe000) >>  13);
	fragment = (unsigned int)(ntohs(SKB_IP_FRAG_OFF(skb)) & IP_OFFMASK);
	if ((flags & IP_FLAG_MF) || fragment)
	{
		goto err;
	}

	if (dn == DIR_DOWNLINK) {
		
#if 0
	   printk("DOWNLINK\n");
#endif
	   rb_ct = find_router_boost_ct(ct);
	   if(!rb_ct)
	   {
		  goto err;
		   
	   }
	   /* both of 0xa0 and 0xb8 will map to rfc8325 VO */
	   if((rb_ct->is_qos_en || force_qos) && !rb_ct->is_dup_en)
	   {
		// set dnlink TOS (0xa0) to go back to dev with WMM (VO)
			if (!skb_make_writable(skb, sizeof(struct iphdr)))
				return NF_DROP;
            ip = ip_hdr(skb);
            ipv4_change_dsfield(ip, 0, 0xa0);
	   }
	   else if(rb_ct->is_dup_en || force_dup)
	   {
		   // mark only
		   skb->mark = CLONE_MARK;
		   // set dnlink TOS (0xB8) to go back to dev with WMM (VO)
		   if (!skb_make_writable(skb, sizeof(struct iphdr)))
               return NF_DROP;
            ip = ip_hdr(skb);
            ipv4_change_dsfield(ip, 0, DN_TOS_BYTE);
	   }	  			
	} /* end of DIR_DOWNLINK */ 
	else if (dn == DIR_UPLINK)
	{
	   rb_ct = find_router_boost_ct(ct);
	   if(!rb_ct)
	   {
		  // handle add a new router_boost_ct
		   rb_ct = allocate_router_boost_ct(ct);
		   if(!rb_ct)
				goto err;
	   }
	   ct_tuple = &rb_ct->origin_tuple;
	   
		new_tx.id = ntohs(ip->id);
		new_tx.src_ip =   GET_TUPLE_SIP(ct_tuple);
		new_tx.src_port = ntohs(GET_TUPLE_SPORT(ct_tuple));
		//new_tx.dst_ip =    GET_TUPLE_DIP(ct_tuple);
		//new_tx.dst_port = ntohs(GET_TUPLE_DPORT(ct_tuple));
		new_tx.jiffies = jiffies;
#if 0       
       	printk("src:%s:%d  dst:%s:%d id:%d \n", 
		ip_str((unsigned char *)&new_tx.src_ip, 4), new_tx.src_port,
        ip_str((unsigned char *)new_tx.dst_ip, 4), new_tx.dst_port, ntohs(ip->id));
#endif            

       	spin_lock_bh(&size_lock);
       	hash_for_each_possible(packets_table, p_old_tx, node, new_tx.id) {
            if(p_old_tx->id == new_tx.id && 
			   p_old_tx->src_ip == new_tx.src_ip 
			   && p_old_tx->src_port == new_tx.src_port
			//   && p_old_tx->dst_ip == new_tx.dst_ip 
			//   && p_old_tx->dst_port == new_tx.dst_port
			   ) {
          
				old_hit_jiffies = p_old_tx->jiffies;
				hash_del(&p_old_tx->node);
                kfree(p_old_tx);
                verdict = NF_DROP;
				hit = 1;
                goto out;
             }
            else if(new_tx.jiffies - p_old_tx->jiffies > msecs_to_jiffies(dup_det_timeout)) {
                hash_del(&p_old_tx->node);
                kfree(p_old_tx);
            }
       
        }
out: 
       spin_unlock_bh(&size_lock);
	   /* check hit */
	   if(hit)
	   {
		   rb_ct->last_dup_jiffies = new_tx.jiffies;
		   if(rb_ct->accum_dup_count++ >= DUP_EN_TRESHOLD)
		   {
			   rb_ct->is_dup_en = 1;
		   }
#if DEBUG_HASHMAP_HIT     
       	printk("hit----src:%s:%d dst:%s:%d id:%d j=%lu\n", 
		ip_str((unsigned char *)&new_tx.src_ip, 4), new_tx.src_port,
        ip_str((unsigned char *)&new_tx.dst_ip, 4), new_tx.dst_port, 
		ntohs(ip->id), old_hit_jiffies);
#endif		   
	   }
	   else
	   {
		    p_new_tx = kmalloc(sizeof(struct tx_udp_packet), GFP_ATOMIC);
			if(!p_new_tx)
				goto err;
			p_new_tx->id = new_tx.id;
			p_new_tx->src_ip =  new_tx.src_ip;
			p_new_tx->src_port = new_tx.src_port;
			p_new_tx->dst_ip =	new_tx.dst_ip;
			p_new_tx->dst_port = new_tx.dst_port;
			p_new_tx->jiffies = new_tx.jiffies;

			hash_add(packets_table, &p_new_tx->node, p_new_tx->id);
	   }
	   	   
	   /* check QOS */
	   if(ip->tos == UP_TOS_BYTE )
	   {
			rb_ct->last_qos_jiffies = new_tx.jiffies;
			if( !hit && rb_ct->accum_qos_count++ >= QOS_EN_TRESHOLD)
			{
			   rb_ct->is_qos_en = 1;
			}
			if(!hit)
                        {	
			  if (!skb_make_writable(skb, sizeof(struct iphdr)))
			    return NF_DROP;
			  ip = ip_hdr(skb);
			  ipv4_change_dsfield(ip, 0, 0x00); 
			  // reset upload tos to 0x00   
			}
	   }
	   
	   if(rb_ct->is_qos_en)
	   {
		   skb->mark = TC_QOS_MARK; // 10 to make tc rule matched	  
	   }
	   
	} /* end of DIR_UPLINK */
err:
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
	ops->hook	  = (nf_hookfn *) cb;

	if (nf_register_net_hook(&init_net, ops) != 0) {
		return -1;
	}

	return 0;
}

static int forward_filter_init(void)
{
	if (__register_hook(&hookfn_ops_preroute_filter, 
						NF_INET_PRE_ROUTING,
						PF_INET, 
					//	NF_IP_PRI_FIRST,
						NF_IP_PRI_MANGLE - 1,
						forward_hookfn_preroute_filter) < 0)
	{
		ERR("Cannot register forward_hookfn_preroute_filter");
		return -1;
	}
	
#ifdef CONFIG_NF_CONNTRACK_CHAIN_EVENTS
    ct_event_notifier.notifier_call = ct_event_cb;
#else
    ct_event_notifier.fcn = ct_event_cb;
#endif

    nf_ct_netns_get(&init_net, NFPROTO_IPV4);
    if (nf_conntrack_register_notifier(&init_net, &ct_event_notifier) == 0) {
      ct_event_notifier_registered = 1;
      printk("deDupTx: ct_event_notifier registered\n");
    } else {
      printk("deDupTx: failed to register a conntrack notifier.\n");
    }

	det_en_wq = create_singlethread_workqueue("deDupTx");
	if (det_en_wq == NULL) {
		printk("deDupTx: warning: failed to create workqueue\n");
	}
	
	if (det_en_wq != NULL)
    {
		detect_en_worker_alive = 1;
		queue_delayed_work(det_en_wq, &detect_en_wk, msecs_to_jiffies(TIMER_DETECT_EN_MS));
	}

	return 0;
}

static void forward_filter_exit(void)
{
	struct tx_udp_packet *p_tmp; 	
	int i = 0; 

	detect_en_worker_alive = 0;
	spin_lock_bh(&size_lock);
	for(i = 0; i < HASH_SIZE(packets_table); i++) {
	if(!hlist_empty(&packets_table[i])) {
			hlist_for_each_entry(p_tmp, &packets_table[i], node) {
		hash_del(&p_tmp->node);
				kfree(p_tmp);
			}
		}
	}	
	spin_unlock_bh(&size_lock);

	printk("Unregister forward_hook_ops_forward_filter");
	nf_unregister_net_hook(&init_net, &hookfn_ops_preroute_filter);

	if (ct_event_notifier_registered) {
      nf_conntrack_unregister_notifier(&init_net, &ct_event_notifier);
      ct_event_notifier_registered = 0;
      printk("deDupTx: ct_event_notifier unregistered\n");
    }

	if (det_en_wq) {
		cancel_delayed_work_sync(&detect_en_wk);
		flush_workqueue(det_en_wq);
		destroy_workqueue(det_en_wq);
	}
	
}

static int deDupTx_init(void)
{
    struct proc_dir_entry *pde;
	if(forward_filter_init() < 0) {
		goto err_exit;
	}
	de_dir = proc_mkdir("dedup", NULL);
	if (!de_dir) {
		goto err_exit;
	}   
	pde = proc_create("ip", 0644, de_dir, &de_ip_fops);
	if (!pde) {
		goto err_free_dir;
	}
	pde = proc_create("dbg", 0644, de_dir, &de_dbg_fops);
	if (!pde) {
		goto err_free_proc_ip;
	}
	pde = proc_create("rblist", 0644, de_dir, &de_rblist_fops);
	if (!pde) {
		goto err_free_proc_ip;
	}

	return 0;

err_free_proc_dbg:
	remove_proc_entry("dbg", de_dir);
err_free_proc_ip:
	remove_proc_entry("ip", de_dir);
err_free_dir:
	proc_remove(de_dir);
err_exit:
	forward_filter_exit();
	
	return -1;
}

static void deDupTx_exit(void)
{
	forward_filter_exit();
	
	remove_proc_entry("rblist", de_dir);
	remove_proc_entry("dbg", de_dir);
	remove_proc_entry("ip", de_dir);
	
	proc_remove(de_dir);

}

MODULE_DESCRIPTION("ASUS RouterBoost enable module");
MODULE_AUTHOR("ASUS-WL");
MODULE_LICENSE("GPL");

module_init(deDupTx_init);
module_exit(deDupTx_exit);
