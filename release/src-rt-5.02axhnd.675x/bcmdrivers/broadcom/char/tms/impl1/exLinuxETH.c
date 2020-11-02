/* 
 * <:label-3PIP:2019:NONE:standard
 * 
 * :>
 */
/* 
 * 
 * Copyright 1997-2017 NComm, Inc.  All rights reserved.
 * 
 * 
 *                     *** Important Notice ***
 *            This notice may not be removed from this file.
 * 
 *  * valid license agreement between your company and NComm, Inc. The license 
 * agreement includes the definition of a PROJECT.
 * 
 *  * and within the project definition in the license. Any use beyond this 
 * scope is prohibited without executing an additional agreement with 
 * NComm, Inc. Please refer to your license agreement for the definition of 
 * the PROJECT.
 * 
 * This software may be modified for use within the above scope. All 
 * modifications must be clearly marked as non-NComm changes.
 * 
 * If you are in doubt of any of these terms, please contact NComm, Inc. 
 * at sales@ncomm.com. Verification of your company's license agreement 
 * and copies of that agreement also may be obtained from:
 *  
 * NComm, Inc.
 * 81 Main Street  
 * Suite 201
 * Kingston, NH 03848
 * 603-329-5221 
 * sales@ncomm.com
 * 
 */

/*
 * TUNABLE paramters.  You probably don't need/want to touch these.
 * If you do need to change them, suggest you do that on the gcc command
 * line (e.g. -DMAX_FILTERS=64)
 */

#ifndef MAX_HOOKS
	#define MAX_HOOKS 32
#endif

#ifndef MAX_FILTERS
	#define MAX_FILTERS 32
#endif


/*
 * Pull in the necessary kernel header files
 * You many need to adjust these as necessary
 */

#include "linux/version.h"	/* Kernel version information */

#include <generated/autoconf.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 4)
        #include "linux/kconfig.h"
#endif

#include "linux/version.h"	/* Kernel Version Information */
#include "linux/kthread.h"	/* Kernel threads */
#include "linux/sched.h"	/* HZ, jiffies definition, task_struct */
#include "linux/list.h"		/* linked list declaration */
#include "linux/slab.h"		/* kmalloc declaration */

#include "linux/mutex.h"

#include "linux/interrupt.h"	/* IRQ_RETVAL stuff */
#include "linux/errno.h"	/* error codes */
#include "linux/module.h"	/* module-related stuff */
#include "linux/poll.h"		/* gets the user-access macros */

#include <linux/module.h>	/* Module header files */
#include <linux/init.h>
#include <linux/kernel.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0))
#include "linux/sched/signal.h" 
#endif

#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/rtnetlink.h>

#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* the following may not be in the Linux if_ether.h header-file.
 *	http://standards.ieee.org/regauth/ethertype/eth.txt
 */
#ifndef	ETH_P_8021AG
	#define	ETH_P_8021AG	0x8902
#endif

#ifndef	ETH_P_8021AD
	#define	ETH_P_8021AD	0x88A8
#endif

#ifndef ETH_P_8021Q
	#define ETH_P_8021Q	0x8100
#endif

#ifndef ETH_P_MPLS_UC
	#define ETH_P_MPLS_UC 0x8847
#endif

#ifndef ETH_P_MPLS_MC
	#define ETH_P_MPLS_MC 0x8848
#endif


/*--------------------------------------------------------------------------*/
/*
 * This accounts for different versions of Linux
 */

/*--------------------------------------------------------------------------*/
#ifndef GET_NS_BY_DEVNAME
	#define GET_NS_BY_DEVNAME(name)		(&init_net)
#endif

/*--------------------------------------------------------------------------*/
#ifndef PUT_DEV_BY_NAME
	#define PUT_DEV_BY_NAME(dev)		dev_put(dev)
#endif

/*--------------------------------------------------------------------------*/
#ifndef NF_REGISTER_NET_HOOK
   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0))
	#define NF_REGISTER_NET_HOOK(nameSpace, hook) \
				nf_register_net_hook(nameSpace, hook)

   #else

	#define NF_REGISTER_NET_HOOK(nameSpace, hook) \
			nf_register_hook(hook)

   #endif
#endif

/*--------------------------------------------------------------------------*/
#ifndef NF_UNREGISTER_NET_HOOK
   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0))

	#define NF_UNREGISTER_NET_HOOK(nameSpace, hook) \
				nf_unregister_net_hook(nameSpace, hook)
   #else

	#define NF_UNREGISTER_NET_HOOK(nameSpace, hook) \
			nf_unregister_hook(hook)
   #endif
#endif


/*--------------------------------------------------------------------------*/
#ifndef NCOMM_TXPKT
   #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)

	#define NCOMM_TXPKT(mytype, mywhere, nameSpace, myskb, mydev) \
		NF_HOOK_COND(mytype, mywhere, \
			nameSpace, NULL,\
			myskb, NULL, mydev, my_dev_xmit, 1)

   #elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)

	#define NCOMM_TXPKT(mytype, mywhere, nameSpace, myskb, mydev) \
			NF_HOOK_THRESH(mytype, mywhere, \
				nameSpace, NULL,\
				myskb, NULL, mydev, my_dev_xmit, \
				INT_MAX)

   #elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)

	#define NCOMM_TXPKT(mytype, mywhere, nameSpace, myskb, mydev) \
			NF_HOOK_THRESH(mytype, mywhere, \
				NULL,\
				myskb, NULL, mydev, dev_queue_xmit_sk, \
				INT_MAX)

   #else

	#define NCOMM_TXPKT(mytype, mywhere, nameSpace, myskb, mydev) \
			NF_HOOK_THRESH(mytype, mywhere, \
				myskb, NULL, mydev, dev_queue_xmit, \
				INT_MAX)

   #endif
#endif

/*--------------------------------------------------------------------------*/
#ifndef NCOMM_TXPKT_FUNCTION
   #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)

	#define NCOMM_TXPKT_FUNCTION \
		static int my_dev_xmit(struct net *net, struct sock *sk, \
			struct sk_buff *skb) \
		{ \
			return(dev_queue_xmit(skb)); \
		}

   #elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)

	#define NCOMM_TXPKT_FUNCTION \
		static int my_dev_xmit(struct net *net, struct sock *sk, \
			struct sk_buff *skb) \
		{ \
			return(dev_queue_xmit(skb)); \
		}

   #elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)

	#define NCOMM_TXPKT_FUNCTION	/* Don't need a TXPKT_FUNCTION */
   #else

	#define NCOMM_TXPKT_FUNCTION	/* Don't need a TXPKT_FUNCTION */
   #endif
#endif



/*--------------------------------------------------------------------------*/
#ifndef GET_DEV_BY_SKB
   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
	#define GET_DEV_BY_SKB(nameSpace, skb)	\
		dev_get_by_index((nameSpace == NULL) ? &init_net : nameSpace, \
			skb->skb_iif)

   #elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))

	#define GET_DEV_BY_SKB(nameSpace, skb) \
		dev_get_by_index((nameSpace == NULL) ? &init_net : nameSpace, \
			skb->iif)

   #elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))

	#define GET_DEV_BY_SKB(nameSpace, skb)	dev_get_by_index(skb->iif)

   #else

	#define GET_DEV_BY_SKB(nameSpace, skb)	skb->input_dev

   #endif
#endif


/*--------------------------------------------------------------------------*/
#ifndef PUT_DEV_BY_SKB
   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))

	#define PUT_DEV_BY_SKB(dev)		dev_put(dev)

   #else

	#define PUT_DEV_BY_SKB(dev)

   #endif
#endif

/*--------------------------------------------------------------------------*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)

	#define CHECK_DEV_PORT (dev->rx_handler_data)

#else

	#define CHECK_DEV_PORT (dev->br_port)

#endif

/*--------------------------------------------------------------------------*/

#ifndef SKB_RESET_MAC_HEADER
   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))

	#define SKB_RESET_MAC_HEADER(skb)	skb_reset_mac_header(skb)

   #else

	#define SKB_RESET_MAC_HEADER(skb)	skb->mac.raw = skb->data

   #endif
#endif

/*--------------------------------------------------------------------------*/
#ifndef SKB_RESET_NETWORK_HEADER
   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))

	#define SKB_RESET_NETWORK_HEADER(skb)	skb_reset_network_header(skb)

   #else

	#define SKB_RESET_NETWORK_HEADER(skb)	skb->nh.raw = skb->data

   #endif
#endif

/*--------------------------------------------------------------------------*/
#ifndef FOR_EACH_NETDEV

   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))

	#define FOR_EACH_NETDEV(nameSpace, dev)	\
			for_each_netdev((struct net *)nameSpace, dev)

   #elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))

	#define FOR_EACH_NETDEV(nameSpace, dev)		for_each_netdev(dev)

   #else

	#define FOR_EACH_NETDEV(nameSpace, dev)			\
			for (dev = dev_base; dev; dev = dev->next)

   #endif
#endif


/*--------------------------------------------------------------------------*/
#ifndef FLT_PSKB_DECLARATION

   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))

	#define FLT_PSKB_DECLARATION		struct sk_buff *

   #else
	#define FLT_PSKB_DECLARATION		struct sk_buff **

   #endif
#endif

/*--------------------------------------------------------------------------*/
#ifndef FLT_PSKB_DEREFERENCE

   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))

	#define FLT_PSKB_DEREFERENCE(ptr)	ptr

   #else

	#define FLT_PSKB_DEREFERENCE(ptr)	*ptr

   #endif
#endif

/*--------------------------------------------------------------------------*/
#ifndef GET_DEV_BY_NAME
   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))

	#define GET_DEV_BY_NAME(nameSpace, devname) \
					dev_get_by_name(nameSpace, devname)
   #else

	#define GET_DEV_BY_NAME(nameSpace, name)	dev_get_by_name(name)

   #endif
#endif


/*--------------------------------------------------------------------------*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
	#define _PRE_ROUTING			NF_INET_PRE_ROUTING
	#define _LOCAL_IN			NF_INET_LOCAL_IN
	#define _FORWARD			NF_INET_FORWARD
	#define _LOCAL_OUT			NF_INET_LOCAL_OUT
	#define _POST_ROUTING			NF_INET_POST_ROUTING

#else

	#define _PRE_ROUTING			NF_IP_PRE_ROUTING
	#define _LOCAL_IN			NF_IP_LOCAL_IN
	#define _FORWARD			NF_IP_FORWARD
	#define _LOCAL_OUT			NF_IP_LOCAL_OUT
	#define _POST_ROUTING			NF_IP_POST_ROUTING

#endif

/*--------------------------------------------------------------------------*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28))

	#define _PKT_UNSPEC			NFPROTO_UNSPEC
	#define _PKT_IPV4			NFPROTO_IPV4
	#define _PKT_ARP			NFPROTO_ARP
	#define _PKT_IPV6			NFPROTO_IPV6
	#define _PKT_BRIDGE			NFPROTO_BRIDGE

#else
	#define _PKT_UNSPEC			PF_UNSPEC
	#define _PKT_IPV4			PF_INET
	#define _PKT_ARP			PF_LOCAL
	#define _PKT_IPV6			PF_INET6
	#define _PKT_BRIDGE			PF_BRIDGE
#endif

/*--------------------------------------------------------------------------*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28))

	#define _PKT_BRIDGE		NFPROTO_BRIDGE

#else

	#define _PKT_BRIDGE		PF_BRIDGE

#endif

/*--------------------------------------------------------------------------*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)

	#define KRNLSTUFF_DECLARE_STATS struct rtnl_link_stats64

#else

	#define KRNLSTUFF_DECLARE_STATS struct rtnl_link_stats

#endif

/*--------------------------------------------------------------------------*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)

	#define DECLARE_FILTER_FUNCTION(filter_name) \
		unsigned int filter_name(void *priv, \
			struct sk_buff *pskb, \
			const struct nf_hook_state *state)

	#define DECLARE_FILTER_LOCALS \
			struct net_device *in = state->in; \
			struct net_device *out = state->out; \
			struct sk_buff *skb = FLT_PSKB_DEREFERENCE(pskb);\
			in = in;out = out

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)

	#define DECLARE_FILTER_FUNCTION(filter_name) \
		uint32_t filter_name(const struct nf_hook_ops *ops, \
		FLT_PSKB_DECLARATION pskb,  \
		const struct nf_hook_state *state)

	#define DECLARE_FILTER_LOCALS \
			struct net_device *in = state->in; \
			struct net_device *out = state->out; \
			struct sk_buff *skb = FLT_PSKB_DEREFERENCE(pskb);\
			in = in;out = out

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)

	#define DECLARE_FILTER_FUNCTION(filter_name) \
		uint32_t filter_name(const struct nf_hook_ops *ops, \
		FLT_PSKB_DECLARATION pskb,  \
		const struct net_device *in, \
		const struct net_device *out,  \
		int (*okfn)(struct sk_buff *))

	#define DECLARE_FILTER_LOCALS \
			struct sk_buff *skb = FLT_PSKB_DEREFERENCE(pskb)

#else

	#define DECLARE_FILTER_FUNCTION(filter_name) \
		uint32_t filter_name(uint32_t hooknum, \
		FLT_PSKB_DECLARATION pskb,  \
		const struct net_device *in, \
		const struct net_device *out,  \
		int (*okfn)(struct sk_buff *))

	#define DECLARE_FILTER_LOCALS \
			struct sk_buff *skb = FLT_PSKB_DEREFERENCE(pskb)

#endif

/*--------------------------------------------------------------------------*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)

	#define NF_HOOK_OPS_INITIALIZER {{{0}}}

#else

	#define NF_HOOK_OPS_INITIALIZER {{{{0}}}}

#endif

/*--------------------------------------------------------------------------*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)

	#define HOOK_OWNER_DECLARE(myowner)
	#define HOOK_OWNER_ASSIGN(myowner)

#else

	#define HOOK_OWNER_DECLARE(myowner) myowner = THIS_MODULE,
	#define HOOK_OWNER_ASSIGN(myowner) myowner = THIS_MODULE

#endif

/*--------------------------------------------------------------------------*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)

	#define ASSIGN_PRIV(mypriv, x) mypriv	= x

#else

	#define ASSIGN_PRIV(mypriv, x) 

#endif

/*--------------------------------------------------------------------------*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)

	#define GET_DEV_MASTER(dev_master, dev_source) \
		rtnl_lock(); \
			dev_master = netdev_master_upper_dev_get(dev_source); \
		rtnl_unlock()
#else

	#define GET_DEV_MASTER(dev_master, dev_source) \
		dev_master = (dev_source)->master

#endif


/*--------------------------------------------------------------------------*/
static int family[] = {
	_PKT_UNSPEC,
	_PKT_IPV4,
	_PKT_ARP,
	_PKT_IPV6,
	_PKT_BRIDGE};

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*
 * Define the module entery and exit routines 
 */

static int __init exLinuxETH_init(void)
{
	printk(KERN_ERR "exLinuxETH Loaded - NComm, Inc.\n");

	return 0;
}

static void __exit exLinuxETH_cleanup(void)
{
	printk(KERN_ERR "exLinuxETH Exit - NComm, Inc.\n");
}

/*--------------------------------------------------------------------------*/
/*
 * Define interface for retriving network stats
 */

/*
 * This returns the current count of errored frames
 * Customization of this routine may be required.
 */

uint64_t exLinuxETH_dev_get_errored_frames(void *devPtr, 
	uint64_t *rx_errors,
	uint64_t *rx_length_errors,
	uint64_t *rx_crc_errors)
{
struct net_device *dev;
uint64_t currentSample;


	dev = devPtr;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	{
	struct rtnl_link_stats64 stats;

		memset(&stats, 0, sizeof(stats));

		dev_get_stats(dev, &stats);

		*rx_errors = (uint64_t)(stats.rx_errors);
		*rx_length_errors = (uint64_t)(stats.rx_length_errors);
		*rx_crc_errors = (uint64_t)(stats.rx_crc_errors);

		currentSample = (uint64_t)(stats.rx_errors) +
			(uint64_t)(stats.rx_length_errors) +
			(uint64_t)(stats.rx_crc_errors);
	}

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)

	{
		*rx_errors = (uint64_t)(stats.rx_errors);
		*rx_length_errors = 0;
		*rx_crc_errors = 0;

		currentSample = (_uint64_t)dev->stats.rx_errors;
	}

#else
	{
		struct net_device_stats *stats;

		stats = dev->get_stats(dev);

		*rx_errors = (uint64_t)(stats->rx_errors);
		*rx_length_errors = (uint64_t)(stats->rx_length_errors);
		*rx_crc_errors = (uint64_t)(stats->rx_crc_errors);

		currentSample = (uint64_t)(stats->rx_errors) +
				(uint64_t)(stats->rx_length_errors) +
				(uint64_t)(stats->rx_crc_errors);
	}
#endif

	return(currentSample);
}
EXPORT_SYMBOL(exLinuxETH_dev_get_errored_frames);


/*--------------------------------------------------------------------------*/
/*
 * This returns the current count of errored Symbols
 * Customization of this routine may be required.
 */

uint64_t exLinuxETH_dev_get_errored_symbols(void *devPtr, 
	uint64_t *rx_symbols)
{
struct net_device *dev;
uint64_t currentSample;


	dev = devPtr;

	currentSample = 0;

	/*
	 * NOTE: Linux does not have the ability to return this information.
	 * So, you can customize the return value if you have hardware
	 * that provides this information.
	 */

	return(currentSample);
}
EXPORT_SYMBOL(exLinuxETH_dev_get_errored_symbols);


/*--------------------------------------------------------------------------*/
/*
 * This returns the current count of Symbols per second
 * Customization of this routine may be required.
 */

uint64_t exLinuxETH_dev_get_SymbolsPerSec(void *devPtr)
{
struct net_device *dev;
uint64_t retval = 0;


	dev = devPtr;

        if (dev->ethtool_ops && dev->ethtool_ops->get_settings) {

                struct ethtool_cmd cmd = {ETHTOOL_GSET};

                if (!(*dev->ethtool_ops->get_settings)(dev, &cmd)) {

                        retval = ((uint64_t)cmd.speed * 1000000) / 8;
                }
        }

	return(retval);
}
EXPORT_SYMBOL(exLinuxETH_dev_get_SymbolsPerSec);


/*--------------------------------------------------------------------------*/
/*
 * This is a linux version independant transmit function.
 */

NCOMM_TXPKT_FUNCTION;


/*
 * Transmit a packet
 */

int exLinuxETH_TxPkt(void *devPtr, 
	void *nameSpace,
	unsigned short protocol,
	int hookLocation,
	int priority,
	unsigned char *macPtr, int macLen,	/* Mac  value and length */
	unsigned char *datPtr, int datLen,	/* Data value and length */
	void *pktItem, int pktItemSize)
{
struct net_device *dev;
struct sk_buff *skb;
unsigned int hooknum;

	switch(hookLocation) {
	case 0:
		hooknum = NF_BR_PRE_ROUTING;
		break;

	case 1:
		hooknum = NF_BR_POST_ROUTING;
		break;

	case 2:
		hooknum = NF_BR_LOCAL_IN;
		break;

	case 3:
		hooknum = NF_BR_LOCAL_OUT;
		break;

	case 4:
		hooknum = NF_BR_FORWARD;
		break;

	default:
		return(0);
	}

	dev = devPtr;

	if ((skb = alloc_skb(macLen + datLen, GFP_ATOMIC)) == NULL) {

		printk(KERN_WARNING "exLinuxEth : "
				"TxPkt dropped - out of skbuffs.\n");
		return(0);
	}

	skb->protocol = htons(protocol);
	skb->priority = priority;
	skb->dev      = dev;
	skb->mac_len  = macLen;

	if((pktItem != NULL) && (pktItemSize <= sizeof(skb->cb))) {
		memcpy(((void *)(&skb->cb[0])), pktItem, pktItemSize);
	}

	skb_reserve(skb, macLen);

	memcpy(skb_put(skb, datLen), datPtr, datLen);
	memcpy(skb_push(skb, macLen), macPtr, macLen);

        SKB_RESET_MAC_HEADER(skb);
        SKB_RESET_NETWORK_HEADER(skb);
#if 1 /* brcm: mark as the highest priority frame for transmission. */
	skb->mark |= 0x7;
#endif
        NCOMM_TXPKT(_PKT_BRIDGE, hooknum, nameSpace,
                        skb, skb->dev);
	return(1);
}
EXPORT_SYMBOL(exLinuxETH_TxPkt);


/*--------------------------------------------------------------------------*/
/*
 * This routine gets the name space by the device name
 */

void *exLinuxETH_get_ns_by_devname(char *devName)
{

	return(GET_NS_BY_DEVNAME(devName));
}
EXPORT_SYMBOL(exLinuxETH_get_ns_by_devname);


/*--------------------------------------------------------------------------*/
/*
 * This routine gets the device by the name of device in the Name Space
 */

void *exLinuxETH_dev_get_by_name(void *nameSpace, char *devName)
{

	return(dev_get_by_name(nameSpace, devName));
}
EXPORT_SYMBOL(exLinuxETH_dev_get_by_name);

/*--------------------------------------------------------------------------*/
/*
 * This routine gets the MAC address of a device
 */

unsigned char *exLinuxETH_dev_get_macAdr(void *devPtr)
{
struct net_device *dev;

	dev = devPtr;

	if(dev == NULL) {
		return(NULL);
	}

	return(dev->dev_addr);
}
EXPORT_SYMBOL(exLinuxETH_dev_get_macAdr);

/*--------------------------------------------------------------------------*/
/*
 * This routine gets the Name of a device
 */

unsigned char *exLinuxETH_dev_get_name(void *devPtr)
{
struct net_device *dev;

	dev = devPtr;

	if(dev == NULL) {
		return(NULL);
	}

	return(dev->name);
}
EXPORT_SYMBOL(exLinuxETH_dev_get_name);


/*--------------------------------------------------------------------------*/
/*
 * This routine puts the device by the device
 */

void exLinuxETH_dev_put(void *devPtr)
{
struct net_device *dev;

	dev = devPtr;

	dev_put(dev);
	return;
}
EXPORT_SYMBOL(exLinuxETH_dev_put);


/*--------------------------------------------------------------------------*/
/*
 * This routine gets a devices in the namespace by MAC address
 */

void *exLinuxETH_GetDevByMAC(void *nameSpace, char *macPtr)
{
struct net_device *dev = NULL;
struct net_device *altDev = NULL;
struct net_device *macDev = NULL;

        rtnl_lock();

	FOR_EACH_NETDEV(nameSpace, dev) {

		if (!memcmp(dev->dev_addr, macPtr, dev->addr_len)) {

			if (CHECK_DEV_PORT) {
				macDev = dev;
				break;
			}
			else {
				altDev = dev;
			}
		}
	}

	rtnl_unlock();

	return((!macDev) ? altDev : macDev);
}
EXPORT_SYMBOL(exLinuxETH_GetDevByMAC);


/*--------------------------------------------------------------------------*/
/*
 * This routine returns the link state of a device
 */

int exLinuxETH_linkState(void *devPtr)
{
struct net_device *dev;

	dev = devPtr;

	return(netif_carrier_ok(dev));
}
EXPORT_SYMBOL(exLinuxETH_linkState);



/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*
 * FilterProcess is where packets that are received will be processed
 * before handing off to higher levels
 */
extern int FilterProcess(int index, 
		int filterType,
		struct sk_buff *skb,
		struct net_device *in,
		struct net_device *out,
		void *kptr);

#define HOOKFUNCTION(x) \
static DECLARE_FILTER_FUNCTION(hookFunction_##x) \
{ \
int rc; \
DECLARE_FILTER_LOCALS; \
 \
	rc = FilterProcess(x, 0, skb, in, out, NULL); \
	return(rc); \
}

extern int filterFunction(int index, 
		struct sk_buff *skb,
		struct net_device *in,
		struct net_device *out);

#define FILTERFUNCTION(x) \
static int filterFunction_##x(struct sk_buff *skb, struct net_device *dev, \
 	struct packet_type *pt, struct net_device *orig_dev) \
{ \
int rc; \
 \
	rc = FilterProcess(x, 1, skb, dev, orig_dev, \
		((void *)(pt->af_packet_priv))); \
	return(rc); \
}

/*
 * Declare the HOOK functions.
 */

#if MAX_HOOKS > 0
HOOKFUNCTION(0)
#endif
#if MAX_HOOKS > 1
HOOKFUNCTION(1)
#endif
#if MAX_HOOKS > 2
HOOKFUNCTION(2)
#endif
#if MAX_HOOKS > 3
HOOKFUNCTION(3)
#endif
#if MAX_HOOKS > 4
HOOKFUNCTION(4)
#endif
#if MAX_HOOKS > 5
HOOKFUNCTION(5)
#endif
#if MAX_HOOKS > 6
HOOKFUNCTION(6)
#endif
#if MAX_HOOKS > 7
HOOKFUNCTION(7)
#endif
#if MAX_HOOKS > 8
HOOKFUNCTION(8)
#endif
#if MAX_HOOKS > 9
HOOKFUNCTION(9)
#endif
#if MAX_HOOKS > 10
HOOKFUNCTION(10)
#endif
#if MAX_HOOKS > 11
HOOKFUNCTION(11)
#endif
#if MAX_HOOKS > 12
HOOKFUNCTION(12)
#endif
#if MAX_HOOKS > 13
HOOKFUNCTION(13)
#endif
#if MAX_HOOKS > 14
HOOKFUNCTION(14)
#endif
#if MAX_HOOKS > 15
HOOKFUNCTION(15)
#endif
#if MAX_HOOKS > 16
HOOKFUNCTION(16)
#endif
#if MAX_HOOKS > 17
HOOKFUNCTION(17)
#endif
#if MAX_HOOKS > 18
HOOKFUNCTION(18)
#endif
#if MAX_HOOKS > 19
HOOKFUNCTION(19)
#endif
#if MAX_HOOKS > 20
HOOKFUNCTION(20)
#endif
#if MAX_HOOKS > 21
HOOKFUNCTION(21)
#endif
#if MAX_HOOKS > 22
HOOKFUNCTION(22)
#endif
#if MAX_HOOKS > 23
HOOKFUNCTION(23)
#endif
#if MAX_HOOKS > 24
HOOKFUNCTION(24)
#endif
#if MAX_HOOKS > 25
HOOKFUNCTION(25)
#endif
#if MAX_HOOKS > 26
HOOKFUNCTION(26)
#endif
#if MAX_HOOKS > 27
HOOKFUNCTION(27)
#endif
#if MAX_HOOKS > 28
HOOKFUNCTION(28)
#endif
#if MAX_HOOKS > 29
HOOKFUNCTION(29)
#endif
#if MAX_HOOKS > 30
HOOKFUNCTION(30)
#endif
#if MAX_HOOKS > 31
HOOKFUNCTION(31)
#endif

#if (MAX_HOOKS > 32)
	#error Need to fix the HOOKFUNCTION declarations
#endif


/*
 * Declare the FILTERFUNCTONs.
 */

#if MAX_FILTERS > 0
FILTERFUNCTION(0)
#endif
#if MAX_FILTERS > 1
FILTERFUNCTION(1)
#endif
#if MAX_FILTERS > 2
FILTERFUNCTION(2)
#endif
#if MAX_FILTERS > 3
FILTERFUNCTION(3)
#endif
#if MAX_FILTERS > 4
FILTERFUNCTION(4)
#endif
#if MAX_FILTERS > 5
FILTERFUNCTION(5)
#endif
#if MAX_FILTERS > 6
FILTERFUNCTION(6)
#endif
#if MAX_FILTERS > 7
FILTERFUNCTION(7)
#endif
#if MAX_FILTERS > 8
FILTERFUNCTION(8)
#endif
#if MAX_FILTERS > 9
FILTERFUNCTION(9)
#endif
#if MAX_FILTERS > 10
FILTERFUNCTION(10)
#endif
#if MAX_FILTERS > 11
FILTERFUNCTION(11)
#endif
#if MAX_FILTERS > 12
FILTERFUNCTION(12)
#endif
#if MAX_FILTERS > 13
FILTERFUNCTION(13)
#endif
#if MAX_FILTERS > 14
FILTERFUNCTION(14)
#endif
#if MAX_FILTERS > 15
FILTERFUNCTION(15)
#endif
#if MAX_FILTERS > 16
FILTERFUNCTION(16)
#endif
#if MAX_FILTERS > 17
FILTERFUNCTION(17)
#endif
#if MAX_FILTERS > 18
FILTERFUNCTION(18)
#endif
#if MAX_FILTERS > 19
FILTERFUNCTION(19)
#endif
#if MAX_FILTERS > 20
FILTERFUNCTION(20)
#endif
#if MAX_FILTERS > 21
FILTERFUNCTION(21)
#endif
#if MAX_FILTERS > 22
FILTERFUNCTION(22)
#endif
#if MAX_FILTERS > 23
FILTERFUNCTION(23)
#endif
#if MAX_FILTERS > 24
FILTERFUNCTION(24)
#endif
#if MAX_FILTERS > 25
FILTERFUNCTION(25)
#endif
#if MAX_FILTERS > 26
FILTERFUNCTION(26)
#endif
#if MAX_FILTERS > 27
FILTERFUNCTION(27)
#endif
#if MAX_FILTERS > 28
FILTERFUNCTION(28)
#endif
#if MAX_FILTERS > 29
FILTERFUNCTION(29)
#endif
#if MAX_FILTERS > 30
FILTERFUNCTION(30)
#endif
#if MAX_FILTERS > 31
FILTERFUNCTION(31)
#endif


#if (MAX_FILTERS > 32)
	#error Need to fix the FILTERFUNCTION declarations
#endif

#define HOOK_TABLE_ENTRY(x) 						\
	{NULL, 				/* hook */			\
	 NULL,				/* nameSpace */			\
	 NULL,				/* device */			\
	"HookEntry_" #x, 		/* debug hook string */		\
	0, 				/* protocol */			\
	hookFunction_##x, 		/* hook function */		\
	NULL, 				/* hook call back function */	\
	}

#define FILTER_TABLE_ENTRY(x) 						\
	{NULL,				/* pt */			\
	"FilterEntry_" #x, 		/* debug filter string */	\
	0, 				/* protocol */			\
	filterFunction_##x, 		/* Packet Type function */	\
	NULL, 				/* pt call back function */	\
	}
	
	
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* These next set of routines deal with adding Net filters to a device      */

typedef struct _hookTable {
	struct nf_hook_ops *hook;		/* Hook */
						/* This item gets malloced */
	void *nameSpace;			/* Name space */
	void *dev;				/* Device */
	char *hookName;				/* Name of hook (for debug) */
	int pf;					/* protocol family */
	void *hookFunction;			/* Pointer to filter hook */
						/* This is what linux calls */
	int (*hookCallBack)(			/* This is the user function */
		void *in,			/* for the nf_hook_ops */
		void *out,
		void *dev,
		unsigned short protocol,
		uint32_t priority,
		int packet_type,
		uint32_t vlanTag,
		void *macPtr,
		int macLen,
		void *datPtr,
		int datLen,
		void *private,
		void *kptr,
		void *indev);

} HOOK_TABLE;

typedef struct _filterTable {
	struct packet_type *pt;			/* Pointer to a hook */
						/* This item gets malloced */
	char *filterName;			/* Name of filter (for debug) */
	unsigned short protocol;		/* protocol of filter */
	void *filterFunction;			/* Pointer to Packet type func*/
						/* This is what linux calls */
	int (*filterCallBack)(			/* This is the user function */
		void *in,
		void *out,
		void *dev,
		unsigned short protocol,
		uint32_t priority,
		int packet_type,
		uint32_t vlanTag,
		void *macPtr,
		int macLen,
		void *datPtr,
		int datLen,
		void *private,
		void *kptr,
		void *indev);
						/* Packet data */
} FILTER_TABLE;


static HOOK_TABLE hookTable[MAX_HOOKS] = 
{
	
#if MAX_HOOKS > 0
	HOOK_TABLE_ENTRY(0),
#endif
#if MAX_HOOKS > 1
	HOOK_TABLE_ENTRY(1),
#endif
#if MAX_HOOKS > 2
	HOOK_TABLE_ENTRY(2),
#endif
#if MAX_HOOKS > 3
	HOOK_TABLE_ENTRY(3),
#endif
#if MAX_HOOKS > 4
	HOOK_TABLE_ENTRY(4),
#endif
#if MAX_HOOKS > 5
	HOOK_TABLE_ENTRY(5),
#endif
#if MAX_HOOKS > 6
	HOOK_TABLE_ENTRY(6),
#endif
#if MAX_HOOKS > 7
	HOOK_TABLE_ENTRY(7),
#endif
#if MAX_HOOKS > 8
	HOOK_TABLE_ENTRY(8),
#endif
#if MAX_HOOKS > 9
	HOOK_TABLE_ENTRY(9),
#endif
#if MAX_HOOKS > 10
	HOOK_TABLE_ENTRY(10),
#endif
#if MAX_HOOKS > 11
	HOOK_TABLE_ENTRY(11),
#endif
#if MAX_HOOKS > 12
	HOOK_TABLE_ENTRY(12),
#endif
#if MAX_HOOKS > 13
	HOOK_TABLE_ENTRY(13),
#endif
#if MAX_HOOKS > 14
	HOOK_TABLE_ENTRY(14),
#endif
#if MAX_HOOKS > 15
	HOOK_TABLE_ENTRY(15),
#endif
#if MAX_HOOKS > 16
	HOOK_TABLE_ENTRY(16),
#endif
#if MAX_HOOKS > 17
	HOOK_TABLE_ENTRY(17),
#endif
#if MAX_HOOKS > 18
	HOOK_TABLE_ENTRY(18),
#endif
#if MAX_HOOKS > 19
	HOOK_TABLE_ENTRY(19),
#endif
#if MAX_HOOKS > 20
	HOOK_TABLE_ENTRY(20),
#endif
#if MAX_HOOKS > 21
	HOOK_TABLE_ENTRY(21),
#endif
#if MAX_HOOKS > 22
	HOOK_TABLE_ENTRY(22),
#endif
#if MAX_HOOKS > 23
	HOOK_TABLE_ENTRY(23),
#endif
#if MAX_HOOKS > 24
	HOOK_TABLE_ENTRY(24),
#endif
#if MAX_HOOKS > 25
	HOOK_TABLE_ENTRY(25),
#endif
#if MAX_HOOKS > 26
	HOOK_TABLE_ENTRY(26),
#endif
#if MAX_HOOKS > 27
	HOOK_TABLE_ENTRY(27),
#endif
#if MAX_HOOKS > 28
	HOOK_TABLE_ENTRY(28),
#endif
#if MAX_HOOKS > 29
	HOOK_TABLE_ENTRY(29),
#endif
#if MAX_HOOKS > 30
	HOOK_TABLE_ENTRY(30),
#endif
#if MAX_HOOKS > 31
	HOOK_TABLE_ENTRY(31),
#endif
#if MAX_HOOKS > 32
	#error Need to add more HOOK_TABLE_ENTRYs
#endif
};


static FILTER_TABLE filterTable[MAX_FILTERS] = 
{
	
#if MAX_FILTERS > 0
	FILTER_TABLE_ENTRY(0),
#endif
#if MAX_FILTERS > 1
	FILTER_TABLE_ENTRY(1),
#endif
#if MAX_FILTERS > 2
	FILTER_TABLE_ENTRY(2),
#endif
#if MAX_FILTERS > 3
	FILTER_TABLE_ENTRY(3),
#endif
#if MAX_FILTERS > 4
	FILTER_TABLE_ENTRY(4),
#endif
#if MAX_FILTERS > 5
	FILTER_TABLE_ENTRY(5),
#endif
#if MAX_FILTERS > 6
	FILTER_TABLE_ENTRY(6),
#endif
#if MAX_FILTERS > 7
	FILTER_TABLE_ENTRY(7),
#endif
#if MAX_FILTERS > 8
	FILTER_TABLE_ENTRY(8),
#endif
#if MAX_FILTERS > 9
	FILTER_TABLE_ENTRY(9),
#endif
#if MAX_FILTERS > 10
	FILTER_TABLE_ENTRY(10),
#endif
#if MAX_FILTERS > 11
	FILTER_TABLE_ENTRY(11),
#endif
#if MAX_FILTERS > 12
	FILTER_TABLE_ENTRY(12),
#endif
#if MAX_FILTERS > 13
	FILTER_TABLE_ENTRY(13),
#endif
#if MAX_FILTERS > 14
	FILTER_TABLE_ENTRY(14),
#endif
#if MAX_FILTERS > 15
	FILTER_TABLE_ENTRY(15),
#endif
#if MAX_FILTERS > 16
	FILTER_TABLE_ENTRY(16),
#endif
#if MAX_FILTERS > 17
	FILTER_TABLE_ENTRY(17),
#endif
#if MAX_FILTERS > 18
	FILTER_TABLE_ENTRY(18),
#endif
#if MAX_FILTERS > 19
	FILTER_TABLE_ENTRY(19),
#endif
#if MAX_FILTERS > 20
	FILTER_TABLE_ENTRY(20),
#endif
#if MAX_FILTERS > 21
	FILTER_TABLE_ENTRY(21),
#endif
#if MAX_FILTERS > 22
	FILTER_TABLE_ENTRY(22),
#endif
#if MAX_FILTERS > 23
	FILTER_TABLE_ENTRY(23),
#endif
#if MAX_FILTERS > 24
	FILTER_TABLE_ENTRY(24),
#endif
#if MAX_FILTERS > 25
	FILTER_TABLE_ENTRY(25),
#endif
#if MAX_FILTERS > 26
	FILTER_TABLE_ENTRY(26),
#endif
#if MAX_FILTERS > 27
	FILTER_TABLE_ENTRY(27),
#endif
#if MAX_FILTERS > 28
	FILTER_TABLE_ENTRY(28),
#endif
#if MAX_FILTERS > 29
	FILTER_TABLE_ENTRY(29),
#endif
#if MAX_FILTERS > 30
	FILTER_TABLE_ENTRY(30),
#endif
#if MAX_FILTERS > 31
	FILTER_TABLE_ENTRY(31),
#endif
#if MAX_FILTERS > 32
	#error Need to add more FILTER_TABLE_ENTRYs
#endif
};


/*
 * Linux sometimes passes the mac address within the packet
 * this routine looks at the packet to figure that out.
 */

static int mac_in_packet(unsigned char *pkt, unsigned short proto)
{
unsigned short current_proto;
int index;

	index = 12;	/* this should be the protocol but could be vlan */
	current_proto = (pkt[index] << 8) | pkt[index + 1];

	if(current_proto != proto) {
		return(0);
	}

	while(1) {

		/*
		 * The protocol provided by linux should match the protocol
		 * in the packet if the mac address is in the data.
		 */

		switch(current_proto) {
		case ETH_P_SLOW:
		case ETH_P_8021AG:
		case ETH_P_MPLS_UC:
		case ETH_P_MPLS_MC:
			/* 
			 * We found the protocol in the packet so
			 * the mac address must be in the packet data
			 */
			return(1);

		case ETH_P_8021AD:
		case ETH_P_8021Q:
			/* 
			 * We have a vlan tag, adjust and check again
			 */
			index += 4;
			current_proto = (pkt[index] << 8) | pkt[index + 1];
			break;

		default:
			/*
			 * Did not find the protocol, the mac must not be
			 * in the packet.
			 */
			return(0);
		}
	}
}

/*
 * This is the routine that does most of the work of a filter.
 * This does a batch of error checking and call a function
 * that has been registered.
 */

int FilterProcess(int index, 
		int filterType,
		struct sk_buff *skb,
		struct net_device *in,
		struct net_device *out,
		void *kptr)
{
struct ethhdr *eth;
unsigned short protocol;
struct net_device *dev, *indev;
unsigned char *macPtr;		/* Pointer to the mac address data */
int macLen;
unsigned char *datPtr;		/* Pointer to the packet data */
int datLen;
uint32_t vlanTag;		/* Tag of vlan (if present) */
unsigned short vlanProto;	/* protocol of vlan */
int vlanTagPresent;		/* Flag if vlan tag is present */
int action;
int packet_type;		/* type of packet */
				/* 0 = default */
				/* 1 = multicast */
				/* 2 = OUT GOING */

	/*
	 * validate information before handling the packet
	 * If something is wrong with what is passed, just accept the
	 * packet.
	 */
	if(skb == NULL) {
		goto packet_accept;
	}

	if (skb_linearize(skb)) {
		goto packet_accept;
	}

	if((skb->mac_len != 14) && (skb->mac_len != 0)) {
		return(0);
	}

	if ((eth = eth_hdr(skb)) == NULL) {
		goto packet_accept;
	}

	if((in == NULL) && (out == NULL)) {
		goto packet_accept;
	}

	if((index < 0) || (index >= MAX_FILTERS)) {
		goto packet_accept;
	}

	dev = skb->dev;

	if(dev == NULL) {
		goto packet_accept;
	}

	/*
	 * get the protocol
	 */
	protocol = ntohs(skb->protocol);

	/*
	 * get the vlan protocol
	 */
	vlanTag = 0;

	vlanTagPresent = 0;

	vlanProto = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
	vlanProto = skb->vlan_proto;
#else
	vlanProto = ETH_P_8021Q;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
	if(skb->vlan_tci & VLAN_TAG_PRESENT) {
		vlanTagPresent = 1;
		vlanTag = 
			(((((skb->vlan_tci & (~(VLAN_TAG_PRESENT))) >> 8) & 
				0xff) << 8) | 
			((((skb->vlan_tci & (~(VLAN_TAG_PRESENT))) >> 0) & 
				0xff) << 0)) |
			((((ntohs(vlanProto) >> 8) & 0xff) << 24) |
			 (((ntohs(vlanProto) >> 0) & 0xff) << 16));
	}
#endif

	switch(skb->pkt_type) {
	default:
		packet_type = 0;
		break;

	case PACKET_MULTICAST:
		packet_type = 1;
		break;

	case PACKET_OUTGOING:
		packet_type = 2;
		break;
	}

	macPtr = NULL;

	indev = GET_DEV_BY_SKB(hookTable[index].nameSpace, skb);

	if (indev != NULL) {
		PUT_DEV_BY_SKB(indev);
		}
	else {
		indev = dev;
	}

	/*
	 * Get the contents of the packet
	 */

	if(skb->mac_len == 14) {
		macPtr = eth_hdr(skb)->h_dest;
		macLen = skb->mac_len;
		datPtr = &skb->data[0];
		datLen = skb->len;
	}
	else {
		if(mac_in_packet(&skb->data[0], protocol)) {
			/* need to find the data */
			macPtr = &skb->data[0];
			macLen = 14;
			datPtr = &skb->data[14];
			datLen = skb->len - 14;
		}
		else {
			/* need to find the data */
			macPtr = &(eth_hdr(skb)->h_dest[0]);
			macLen = 14;
			datPtr = &skb->data[0];
			datLen = skb->len;
		}
	}

	if(filterType == 0) {
	   action = (*hookTable[index].hookCallBack) ((void *)in, 
		(void *)out, 
		(void *)dev,
		protocol,
		skb->priority,
		packet_type,
		vlanTag,
		(void *)macPtr,
		macLen,
		(void *)datPtr,
		datLen,
		(void *)(&skb->cb[0]),	 /* Private data area */
		kptr,
		indev);		
	}
	else {
	   action = (*filterTable[index].filterCallBack) ((void *)in, 
		(void *)out, 
		(void *)dev,
		protocol,
		skb->priority,
		packet_type,
		vlanTag,
		(void *)macPtr,
		macLen,
		(void *)datPtr,
		datLen,
		(void *)(&skb->cb[0]),	 /* Private data area */
		kptr,
		indev);		
	}
		
	switch(action) {
	case 0:		/* For HOOK - PACKET_HOOK_DROP */
		goto packet_drop;

	case 1:		/* For HOOK - PACKET_HOOK_ACCEPT */
		goto packet_accept;

	case 2:		/* For filters - PACKET_FILTER_FREE */
		kfree_skb(skb);
		return(0);

	case 3:		/* For filters - PACKET_FILTER_PASS */
		return(0);

	default:
		goto packet_accept;
	}

	goto packet_accept;

	/* 
	 * Define consequential actions of receiving the packet
	 * All exits come through here (helps with debugging) 
	 */

packet_drop:
	return(NF_DROP);

packet_accept:
	return(NF_ACCEPT);
}

/*
 * Add a Net Filter to manage packets
 */

void *exLinuxETH_addHook(void *nameSpace,
	void *devPtr,
	int packetFamily,		/* Family of packet */
	int hookLocation,		/* location of filter */
					/* 0 = Pre Routing */
					/* 1 = Post routing */
					/* 2 = Local In */
					/* 3 = Local Out */

	int hookPriority,		/* 0 = First */
					/* 1 = last */
					
	int (*hookCallBack)(
		void *in,
		void *out,
		void *dev,
		unsigned short protocol,
		uint32_t pkt_priority,
		int packet_type,
		uint32_t vlanTag,
		void *macPtr,
		int macLen,
		void *datPtr,
		int datLen,
		void *private,
		void *kptr,
		void *indev),
	void *hookPrivate,

	void *currentHook
)
{
struct net_device *dev;
struct nf_hook_ops *hookPtr = NULL;
int i, index;
int priority;
int pf;
unsigned int hooknum;

	dev = devPtr;

	index = (-1);

	/*
	 * Validate arguments
	 */

	if(hookCallBack == NULL) {
		goto bagout;
	}

	switch(hookPriority) {
	case 0:
		priority = NF_BR_PRI_FIRST;
		break;

	case 1:
		priority = NF_BR_PRI_LAST;
		break;

	default:
		goto bagout;
	}

	switch(hookLocation) {
	case 0:
		hooknum = NF_BR_PRE_ROUTING;
		break;

	case 1:
		hooknum = NF_BR_POST_ROUTING;
		break;

	case 2:
		hooknum = NF_BR_LOCAL_IN;
		break;

	case 3:
		hooknum = NF_BR_LOCAL_OUT;
		break;

	case 4:
		hooknum = NF_BR_FORWARD;
		break;

	default:
		goto bagout;
	}

	if((packetFamily < 0) || 
			(packetFamily >= (sizeof(family)/sizeof(family[0])))) {
		goto bagout;
	}

	pf = family[packetFamily];

	if(nameSpace == NULL) {
		nameSpace = (&init_net);
	}

	/*
	 * See if we have the filter already 
	 * If we do, three is nothing to do.
	 * Handle changing the PF.
	 * Also, find an empty record if needed.
	 */

	for(i = 0;i < MAX_HOOKS;i++) {
		if((currentHook != NULL) && 
			((&hookTable[i]) == currentHook)) {

			/*
			 * See if hook is already set properly
			 */
			if (hookTable[i].pf == pf) {
				hookPtr = hookTable[i].hook;
				goto bagout;
			}

			/*
			 * we have a hook set up already but want to
			 * change the protocol family.
			 */

			index = i;

			NF_UNREGISTER_NET_HOOK(hookTable[index].nameSpace, 
						hookTable[index].hook);
			goto change_hook;
		}
		if((hookTable[i].hook == NULL) && (index == (-1))) {
			index = i;
		}
	}

	/*
	 * ok, we need to add the hook if we have an opening.
	 */

	if(index == (-1)) {
		/* 
		 * We did not find an open record - return an error
		 */
		goto bagout;
	}

	/*
	 * Now create and add the hook
	 */

	if (!(hookPtr = kmalloc(sizeof(struct nf_hook_ops), GFP_KERNEL))) {
		goto bagout;
	}

	memset(hookPtr, 0, sizeof(struct nf_hook_ops));

	/*
	 * create the nf hook here.
	 */

change_hook:

	hookPtr->hook = hookTable[index].hookFunction;
	hookPtr->priority = priority;
	hookPtr->hooknum = hooknum;
	hookPtr->pf = pf;
	HOOK_OWNER_ASSIGN(hookPtr->owner);
	ASSIGN_PRIV(hookPtr->priv, hookPrivate);

	hookTable[index].hookCallBack = hookCallBack;
	hookTable[index].hook = hookPtr;
	hookTable[index].nameSpace = nameSpace;
	hookTable[index].dev = dev;

	if (NF_REGISTER_NET_HOOK(nameSpace, hookPtr) < 0) {
		hookTable[index].hook = NULL;
		hookTable[index].nameSpace = NULL;
		kfree(hookPtr);
		return(NULL);
	}
bagout:
	return((index == (-1)) ? NULL : (&hookTable[index]));
}
EXPORT_SYMBOL(exLinuxETH_addHook);

/*
 * Routine to add a filter
 */

void *exLinuxETH_addFilter(
	void *devPtr,
	unsigned short protocol,

	int (*filterCallBack)(
		void *in,
		void *out,
		void *dev,
		unsigned short protocol,
		uint32_t pkt_priority,
		int packet_type,
		uint32_t vlanTag,
		void *macPtr,
		int macLen,
		void *datPtr,
		int datLen,
		void *private,
		void *kptr,
		void *indev),		/* in Mac Ptr is NULL */
	void *filterPrivate,
	void *currentFilter
)
{
struct net_device *dev;
struct packet_type *ptPtr = NULL;
int i, index;

	dev = devPtr;

	index = (-1);

	/*
	 * Validate arguments
	 */

	if(filterCallBack == NULL) {
		goto bagout;
	}

	/*
	 * See if we have the filter already 
	 * If we do, three is nothing to do.
	 * Handle changing the protocol.
	 * Also, find an empty record if needed.
	 */

	for(i = 0;i < MAX_FILTERS;i++) {
		if((currentFilter != NULL) && 
			((&filterTable[i]) == currentFilter)) {

			/*
			 * See if filter is already set properly
			 */
			if (filterTable[i].protocol == protocol) {

				ptPtr = filterTable[i].pt;
				goto bagout;
			}

			/*
			 * we have a filter set up already but want to
			 * change the protocol.
			 */
			index = i;
			goto change_protocol;
		}

		/*
		 * Found and empty index, record it for later
		 */

		if((filterTable[i].pt == NULL) && (index == (-1))) {
			index = i;
		}
	}

	/*
	 * ok, we need to add the filter if we have an opening.
	 */

	if(index == (-1)) {
		/* 
		 * We did not find an open record - return an error
		 */
		goto bagout;
	}

	/*
	 * Now create and add the filter
	 */


	if (!(ptPtr = kmalloc(sizeof(struct packet_type), GFP_KERNEL))) {
		goto bagout;
	}

	memset(ptPtr, 0, sizeof(struct packet_type));

	/*
	 * create the packet_type here.
	 */

add_protocol:

	if(ptPtr == NULL) {
		goto bagout;
	}

	ptPtr->type = __constant_htons(protocol);
	ptPtr->dev  = dev;
	ptPtr->func = filterTable[index].filterFunction;
	ptPtr->af_packet_priv = filterPrivate;

	filterTable[index].filterCallBack = filterCallBack;
	filterTable[index].pt = ptPtr;

	dev_add_pack(ptPtr);

	goto bagout;

change_protocol:
	/*
	 * This is where we change the protocol on the filter
	 */
	ptPtr = filterTable[index].pt;

	if(ptPtr != NULL) {
		dev_remove_pack(ptPtr);
		memset(ptPtr, 0, sizeof(struct packet_type));
	}
	else {
		index = (-1);
		goto bagout;
	}
		
	goto add_protocol;

bagout:
	return((index == (-1)) ? NULL : (&filterTable[index]));
}
EXPORT_SYMBOL(exLinuxETH_addFilter);

/*
 * Routine to delete a hook
 */

int exLinuxETH_deleteHook(void *current_table_entry)
{
HOOK_TABLE *ht = current_table_entry;

	if(current_table_entry == NULL) {
		return(0);
	}

	if(ht->hook != NULL) {
		NF_UNREGISTER_NET_HOOK(ht->nameSpace, ht->hook);
		kfree(ht->hook);
		ht->hook = NULL;
	}
		
	ht->nameSpace = NULL;
	ht->hook = NULL;
	return(1);
}
EXPORT_SYMBOL(exLinuxETH_deleteHook);

/*
 * Routine to delete a filter
 */

int exLinuxETH_deleteFilter(void *current_table_entry)
{
FILTER_TABLE *ft = current_table_entry;

	if(current_table_entry == NULL) {
		return(0);
	}

	if(ft->pt != NULL) {
		dev_remove_pack(ft->pt);
		kfree(ft->pt);
		ft->pt = NULL;
	}
		
	ft->protocol = 0;
	return(1);
}
EXPORT_SYMBOL(exLinuxETH_deleteFilter);

/*
 * Turn on/off Promiscuous mode.
 * 1  = turn on
 * -1 = turn off
 */
void exLinuxETH_set_Promiscuous(void *devPtr, int setclear)
{
struct net_device *dev = devPtr;

	if(dev == NULL) {
		return;
	}

	switch(setclear) {
	case 1:
	case (-1):
		break;

	default:
		return;
	}

	rtnl_lock();

	dev_set_promiscuity(dev, setclear);

	rtnl_unlock();
}
EXPORT_SYMBOL(exLinuxETH_set_Promiscuous);

/*
 * Routine to get time stamp information.
 */

void exLinuxETH_GetTimeStamp(uint32_t *seconds, uint32_t *nanoSeconds, 
	int adjust)
{
struct timespec tsample;

	if((seconds == NULL) || (nanoSeconds == NULL)) {
		return;
	}

	getnstimeofday(&tsample);

	if(adjust) {
		*seconds = htonl(tsample.tv_sec);
		*nanoSeconds = htonl(tsample.tv_nsec);
	}
	else {
		*seconds = tsample.tv_sec;
		*nanoSeconds = tsample.tv_nsec;
	}
}
EXPORT_SYMBOL(exLinuxETH_GetTimeStamp);

/*
 * Routine to get the first device on the bridge.
 */

void *exLinuxETH_GetFirstDevice(void *nameSpace)
{

	if(nameSpace == NULL) {
		return(NULL);
	}

	return(first_net_device(nameSpace));
}
EXPORT_SYMBOL(exLinuxETH_GetFirstDevice);

void *exLinuxETH_GetNextDevice(void *devPtr)
{
struct net_device *dev = devPtr;

	if(devPtr == NULL) {
		return(NULL);
	}

	return(next_net_device(dev));
}
EXPORT_SYMBOL(exLinuxETH_GetNextDevice);

/*
 * Routine to get the Master device on the bridge.
 */

void *exLinuxETH_GetMaster(void *devPtr)
{
struct net_device *dev = devPtr;
struct net_device *master;

	if(devPtr == NULL) {
		return(NULL);
	}

	GET_DEV_MASTER(master, dev);

	return(master);
}
EXPORT_SYMBOL(exLinuxETH_GetMaster);

/*
 * Routine to validate the master device
 */

int exLinuxETH_ValidateMaster(void *masterPtr, void *devPtr)
{
struct net_device *dev = devPtr;
struct net_device *master = masterPtr;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)

		if((master == NULL) || (master != dev)) {
			return(0);
		}

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)

		/*
		 * NOTE: IN kernels 2.6.37 - 2.6.38
		 * the members of the bridge cannot be determined.
		 * The bridge code prevents access the information to
		 * determine which ports are members of the bridge.
		 * The code below will find if a port is a member of a bridge.
		 * However, if there are more than one bridge, then
		 * this code will not operate correct and filters
		 * will be applied to ports that should not have them.
		 * Contact NComm support for assistance if you are in
		 * this situation.
		 */
		if (!(dev->priv_flags & IFF_BRIDGE_PORT)) {
			if((master == NULL) || (master != dev)) {
				return(0);
			}
		}

#else

		if(master == NULL) {
			if(dev->br_port == NULL) {
				return(0);
			}
		}
#endif

	return(1);
}
EXPORT_SYMBOL(exLinuxETH_ValidateMaster);

#if 1 /* brcm */
/*--------------------------------------------------------------------------*/
/* Check whether the packet should be handled or not. This function is
 * registered to kernel when 802.1ag module is loaded.
 */
int exLinuxETH_bcm1agHandleFrameCheck(struct sk_buff *skb)
{
	if(skb)
	{
		if (skb->protocol == htons(ETH_P_8021AG))
		{
			/* No vlan tagged 1ag */
			return 1;
		}
		else if (skb->protocol == htons(ETH_P_8021Q))
		{
			struct vlan_hdr *vh = (struct vlan_hdr *)skb->data;
			if (vh->h_vlan_encapsulated_proto == htons(ETH_P_8021AG))
			{
				/* Single vlan tagged 1ag */
				return 1;
			}
			else if (vh->h_vlan_encapsulated_proto == htons(ETH_P_8021Q))
			{
				vh = (struct vlan_hdr *)(skb->data + 4);
				if (vh->h_vlan_encapsulated_proto == htons(ETH_P_8021AG))
				{
					/* Dobule vlan tagged 1ag */
					return 1;
				}
			}
		}
	}
	return(0);
}
EXPORT_SYMBOL(exLinuxETH_bcm1agHandleFrameCheck);
#endif


module_init(exLinuxETH_init);
module_exit(exLinuxETH_cleanup);

MODULE_LICENSE("Proprietary: exLinuxETH - NComm, Inc. Copyright (C) 2018");
MODULE_AUTHOR("NComm, Inc.");
MODULE_DESCRIPTION("exLinuxETH");

