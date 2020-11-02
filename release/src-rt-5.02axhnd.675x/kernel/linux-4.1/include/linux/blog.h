#if defined(CONFIG_BLOG)

#ifndef __BLOG_H_INCLUDED__
#define __BLOG_H_INCLUDED__

/*--------------------------------*/
/* Blog.h and Blog.c for Linux OS */
/*--------------------------------*/

/* 
* <:copyright-BRCM:2003:DUAL/GPL:standard
* 
*    Copyright (c) 2003 Broadcom 
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

/*
 *******************************************************************************
 *
 * File Name  : blog.h
 *
 * Description:
 *
 * A Blog is an extension of the native OS network stack's packet context.
 * In Linux a Blog would be an extension of the Linux socket buffer (aka skbuff)
 * or a network device driver level packet context FkBuff. The nbuff layer
 * provides a transparent access SHIM to the underlying packet context, may it
 * be a skbuff or a fkbuff. In a BSD network stack, a packet context is the
 * BSD memory buffer (aka mbuff).
 *
 * Blog layer provides Blog clients a SHIM to the native OS network stack:
 * Blog clients may be implemented to:
 *  - debug trace a packet as it passes through the network stack,
 *  - develop traffic generators (loop) at the network device driver level.
 *  - develop network driver level promiscuous mode bound applications and use
 *    the Blog SHIM to isolate themselves from the native OS network constructs
 *    or proprietery network constructs such as Ethernet bridges, VLAN network
 *    interfaces, IGMP, firewall and connection tracking systems.
 *
 * As such, Blog provides an extension of the packet context and contains the
 * received and transmitted packets data and parsed information. Parsing results
 * are saved to describe, the type of layer 1, 2, 3 and 4 headers seen, whether
 * the packet was a unicast, broadcast or multicast, a tunnel 4in6 or 6in4 etc.
 *
 * Blog views a receive or transmit end-point to be any construct that can be
 * described by a end point context and a handler op. An end-point could hence
 * be a:
 *  - a network device (Linux net_device with hard start transmit handler),
 *  - a link or queue in the network stack (e.g. a Linux Traffic Control queue
 *    or a netlink or raw socket queue),
 *  - a file system logging interface and its logging handler,
 *  - a virtual interface to some hardware block that provides some hardware
 *    assisted functionality (e.g. IPSEC acceleration or checksum offloading
 *    or GSO block),
 *  - a raw interface to an external hardware test traffic generator using say
 *    a DMA mapped packet reception or transmission.
 *
 * Blog clients are hence applications that provide value added capability by
 * binding at such end-points.
 *
 * A simple Blog client application is a loop traffic generator that simply
 * acts as a sink of packets belonging to a specific "l3 flow" and mirrors
 * them to another interface or loops them back into the stack by serving as a
 * source to a receive network device, while measuring the packet processing
 * datapath performance in the native OS network stack/proprietary constructs.
 * Such a loop traffic generator could be used to inject N cells/packets
 * that cycle through the system endlessly, serving as background traffic while
 * a few flows are studied from say a QOS perspective.
 *
 * Another example of a Blog client is a proxy accelerator (hardware / software)
 * that is capable of snooping on specific flows and accelerating them while
 * bypassing the native OS network stack and/or proprietery constructs. It is
 * however required that the native OS constructs can co-exist. E.g. it may be
 * necessary to refresh a network bridge's ARL table, or a connection/session
 * tracker, or update statistics, when individual packets bypass such network
 * constructs. A proxy accelerator may also reside between a Rx network device
 * a hardware IPSEC accelerator block and a Tx network device.
 *
 * Blog layer provides a logical composite SHIM to the network constructs
 * Linux or proprietery, allowing 3rd party network constructs to be seemlesly
 * supported in the native OS.  E.g a network stack that uses a proprietery
 * session tracker with firewalling capability would need to be transparently
 * accessed, so that a Blog client may refresh the session tracking object when
 * packets bypass the network stack.
 *
 * For each OS (eCOS, Linux, BSD) a blog.c implementation file is provided that
 * implements the OS specific SHIM. Support for 3rd-party network constructs
 * would need to be defined in the blog.c . E.g. for Linux, if a proprietery
 * session tracker replaces the Linux netfilter connection tracking framework,
 * then the void * ct_p and the corresponding query/set operations would need to
 * be implemented. The Blog clients SHOULD NOT rely on any function other than
 * those specifically defined allowing a coexistence of the Blog client and the
 * native construct. In the example of a ct_p, for all practice and purposes,
 * the void *, could have been a key or a handle to a connection tracking object
 *
 * Likewise, the Blog client may save need to save a client key with the
 * network constuct. Again a client key may be a pointer to a client object or
 * simply a hash key or some handle semantics.
 *
 * The logical SHIM is defined as follows:
 *
 * __doc_include_if_linux__
 *
 * 1. Extension of a packet context with a logging context:
 * ========================================================
 *   Explicit APIs to allocate/Free a Blog structure, and bind to the packet
 *   context, may it be a skbuff or a fkbuff. Support for transferring a
 *   Blog_t structure from one packet context to another during the course of
 *   a packet in the network stack involving a packet context clone/copy is
 *   also included. The release and recycling of Blog_t structures when a 
 *   packet context is freed are also providied.
 *   Binding is bi-directional: packet context <-- --> Blog_t
 * 
 *
 * 2. Associating native OS or 3rd-party network constructs: blog_link()
 * ==========================================================================
 *   Examples of network constructs
 *      "dev"   - Network device 
 *      "ct"    - Connection or session tracker
 *      "fdb"   - Network bridge forwarding database entity
 *
 *   Association is pseudo bi-directional, using "void *" binding in a Blog_t to
 *   a network construct. In the reverse, a network construct will link to a
 *   Blog client entity using a Key concept. Two types of keys are currently
 *   employed, a BlogFlowKey and a BlogGroupKey. 
 *
 *   A BlogFlowKey would typically refer to a single unidirectional packet
 *   stream defined by say all packets belonging to a unidirectional IPv4 flow,
 *   whereas a BlogGroupKey could be used to represent a single downstream
 *   multicast stream (IP multicast group) that results in replicated streams
 *   pertaining to multiple clients joining a the IPv4 multicast group.
 *
 *   Likewise, one may represent a single unidirectional IPv4 UDP flow using
 *   BlogFlowKey, and the reverse direction IPv4 UDP reply flow
 *   using another BlogFlowKey, and represent the mated pair using a
 *   BlogGroupKey.
 *
 *   In a Blog traffic generator client, where in several IPv4 UDP flows, each
 *   represented independently using a BlogFlowKey, allows for a set of them
 *   (background downstream stress traffic) to be managed as a group using a
 *   BlogGroupKey.
 *
 *   Designer Note:
 *   A network construct may be required to save a BlogFlowKey and/or
 *   BlogGroupKey to complete the reverse binding between a network construct
 *   and the Blog client application. An alternate approach would be to save
 *   a pointer to the Blog_t in the network construct with an additional
 *   dereference through the keys saved within the Blog_t object.
 *
 *   A BlogFlowKey and a BlogGroupKey is a 32bt sized unit and can serve either
 *   as a pointer (32bit processor) or a index or a hash key or ...
 *
 *
 * 3. Network construct and Blog client co-existence call backs:
 * =============================================================
 *
 * blog_notify():
 * ==============
 * A network construct may notify a Blog client of a change of status and may
 * be viewed as a "downcall" from specialized network construct to a Blog client
 * E.g. if a connection/session tracking system deems that a flow needs to be
 * deleted or say it itself is being destroyed, then it needs to notify the Blog
 * client. This would allow the Blog client to cleanup any association with the
 * network construct.
 * Ability for a Blog client to receive general system wide notifications of
 * changes, to include, network interfaces or link state changes, protocol stack
 * service access point changes, etc.
 * Designer Note: Linux notification list?
 *
 * blog_request():
 * ===============
 * A Blog client may request a change in state in the network construct and may
 * be viewed as a "upcall" from the Blog client into the network construct. A
 * timer refresh of the bridge fdb or connection tracking object, or a query
 * whether the session tracker has successfully established (e.g. a TCP 3-way
 * handshake has completed, or a IGMP client was permitted to join a group, or a
 * RTSP session was successful) a uni-driectional or bi-directional flow.
 *
 *
 * 4. Network end-point binding of Blog client
 * ===========================================
 *
 * blog_init(), blog_sinit(), blog_finit():
 * ========================================
 * __comment_if_linux__ : This function is invoked by a Linux network device on
 * packet reception to pass the packet to a Blog client application.
 *
 * Pass a packet context to a Blog client at a "RX" network device either using
 * a skbuff or a fkbuff packet context. Blog client MAY ONLY ACCESS fkbuff
 * fields. As per the nbuff specification, a FkBuff may be considered as a
 * base class and a skbuff is a derived class, inheriting the base class members
 * of the base class, fkbuff. The basic fields of a packet context are a pointer
 * to the received packet's data, data length, a set of reserved fields to carry
 * layer 1 information, queue priority, etc, and packet context and or packet
 * recycling. The layer 1 information is described in terms of channels and
 * and link layer phy preambles. A channel could be an ATM VCI, a DSL queue, a
 * PON Gem Port. A Phy could describe the LINK layer type and or a preamble for
 * instance a RFC2684 header in the DSL world.
 *
 * blog_[s|f]init() will setup the L1 coarse key<channel,phy> and invokes a Blog
 * client's receive hook. A Blog client may consume the packet bypassing the
 * native OS network stack, may suggest that the packet context be extended by
 * a Blog_t structure or may deem that the packet is of not interest. As such
 * the Blog client will return PKT_DONE, PKT_BLOG or PKT_NORM, respectively. In
 * case no Blog client has been registered for receiving packets (promiscuous)
 * driectly from RX network devices, then the packet will follow a normal data
 * path within the network stack (PKT_NORM).
 *
 * Designer Note: Blog clients MAY NOT use fields not defined in FkBuff.
 * 
 *
 * blog_emit():
 * ============
 * __comment_if_linux__ : This function is invoked by a Linux network device
 * prior to packet transmission to pass the packet to a Blog client application.
 *
 * Pass a packet context to a Blog client at a "TX" network device either using
 * a skbuff or a fkbuff packet context. The same restrictions on a Blog client
 * pertaining to packet field context access as defined in the blog_init()
 * variant of APIs is applicable to blog_emit(). A Blog client may also return
 * PKT_NORM or PKT_DONE, to indicate normal processing, or packet consumption.
 *
 * Designer Note: blog_emit() will ONLY pass those packets to Blog clients that
 * have a packet context extended with a Blog_t structure. Hence skbuffs or
 * fkbuffs that do not have a Blog_t extension will not be handed to the Blog
 * client. Do we need blog_semit/blog_femit variants.
 *
 *
 * 5. Binding Blog client applications: blog_bind()
 * ================================================
 * blog_bind() enables a "single" client to bind into the network stack by
 * specifying a network device packet reception handler, a network device packet
 * transmission handler, network stack to blog client notify hook.
 *
 *
 * 6. Miscellanous
 * ===============
 * - Blog_t management.
 * - Data-filling a Blog_t.
 * - Protocol Header specifications independent of OS.
 * - Debug printing.
 *
 *
 * __end_include_if_linux__
 *
 *  Version 1.0 SKB based blogging
 *  Version 2.0 NBuff/FKB based blogging (mbuf)
 *  Version 2.1 IPv6 Support
 *  Version 3.0 Restructuring Blog SHIM to support eCOS, Linux and proprietery
 *              network constructs
 *
 *******************************************************************************
 */

#define BLOG_VERSION            "v3.0"

#if defined(__KERNEL__)                 /* Kernel space compilation           */
#include <linux/types.h>                /* LINUX ISO C99 7.18 Integer types   */
#else                                   /* User space compilation             */
#include <stdint.h>                     /* C-Lib ISO C99 7.18 Integer types   */
#endif
#include <linux/blog_net.h>             /* IEEE and RFC standard definitions  */
#include <linux/nbuff_types.h>          /* for IS_SKBUFF_PTR                  */
#include <linux/brcm_dll.h>

#ifndef NULL_STMT
#define NULL_STMT                   do { /* NULL BODY */ } while (0)
#endif

#undef  BLOG_DECL
#define BLOG_DECL(x)                x,

#ifndef BLOG_OFFSETOF
#define BLOG_OFFSETOF(stype, member)     ((size_t) &((struct stype *)0)->member)
#endif

/* Forward declarations */
struct blog_t;
typedef struct blog_t Blog_t;
#define BLOG_NULL                   ((Blog_t*)NULL)
#define BLOG_KEY_NONE               0

/* __bgn_include_if_linux__ */

struct sk_buff;                         /* linux/skbuff.h                     */
struct fkbuff;                          /* linux/nbuff.h                      */

/* See RFC 4008 */


typedef struct blogCtTimeFlags {
    uint32_t        unused: 31;
    uint32_t        valid: 1;   /* BlogCtTime has valid values */
} BlogCtTimeFlags_t;

/* used to pass timer info between the stack and blog layer */
typedef struct blogCtTime {
    BlogCtTimeFlags_t flags;        /* Flags */
    uint8_t         unknown;        /* unknown proto */
    uint8_t         proto;          /* known proto TCP, UDP */
    uint8_t         intv;           /* intv in sec */
    uint8_t         idle;           /* idle time in sec */
} BlogCtTime_t;

/* used to exchange info between fcache and drivers */
typedef struct {
    uint32_t h_proto;    /* protocol */
    uint32_t key_match;  /* key */
    void     *txdev_p;
    uint8_t tx_l3_offset;
    uint8_t tx_l4_offset;
} BlogFcArgs_t;

typedef struct {
    uint32_t rx_packets;
    int32_t  rx_rtp_packets_lost; /*TODO chekc why this is defined as signed int */
    aligned_u64 rx_bytes;
    uint32_t pollTS_ms; // Poll timestamp in ms
}BlogFcStats_t;

typedef struct {
    u64 packet_count;
    u64 byte_count;
} blog_fast_stats_t;

/*
 * Linux Netfilter Conntrack registers it's conntrack refresh function which
 * will be invoked to refresh a conntrack when packets belonging to a flow
 * managed by Linux conntrack are bypassed by a Blog client.
 */
typedef void (*blog_cttime_upd_t)(void * ct_p, BlogCtTime_t *ct_time_p);
extern blog_cttime_upd_t blog_cttime_update_fn;

typedef void (*blog_ct_put_stats_t)(void * net_p, uint32_t dir, const BlogFcStats_t * stats);
extern blog_ct_put_stats_t blog_ct_put_stats_fn;

extern int blog_ct_get_stats(const void *ct, uint32_t blog_key, uint32_t dir,
        BlogFcStats_t *stats);
extern int blog_ct_push_stats(void);

#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE)
typedef int (*blog_gre_rcv_check_t)(void *dev, BlogIpv4Hdr_t *iph, uint16_t len, 
              void **tunl_pp, uint32_t *pkt_seqno_p);
extern blog_gre_rcv_check_t blog_gre_rcv_check_fn;

typedef int (*blog_gre_xmit_upd_t)(void * tunl_p, BlogIpv4Hdr_t *iph, uint16_t len);
extern blog_gre_xmit_upd_t blog_gre_xmit_update_fn;
typedef int (*blog_gre6_rcv_check_t)(void *dev, BlogIpv6Hdr_t *ipv6h, uint16_t len, 
              void **tunl_pp, uint32_t *pkt_seqno_p);
extern blog_gre6_rcv_check_t blog_gre6_rcv_check_fn;

typedef int (*blog_gre6_xmit_upd_t)(void * tunl_p, BlogIpv6Hdr_t *ipv6h, uint16_t len);
extern blog_gre6_xmit_upd_t blog_gre6_xmit_update_fn;
#endif


#define PPTP_NOT_ACK 0
#define PPTP_WITH_ACK 1
#define PPTP_GRE_VER_0 0
#define PPTP_GRE_VER_1 1
#define PPTP_GRE_NONE 2

typedef int (*blog_pptp_xmit_upd_t)(uint16_t call_id, uint32_t seqNum, 
                                    uint32_t ackNum, uint32_t daddr);
extern blog_pptp_xmit_upd_t blog_pptp_xmit_update_fn;

typedef int (*blog_pptp_xmit_get_t)(uint16_t call_id, uint32_t* seqNum, 
                                    uint32_t* ackNum, uint32_t daddr);
extern blog_pptp_xmit_get_t blog_pptp_xmit_get_fn;

typedef int (*blog_pptp_rcv_check_t)(uint16_t call_id, uint32_t *rcv_pktSeq, 
                                     uint32_t rcv_pktAck, uint32_t saddr);
extern blog_pptp_rcv_check_t blog_pptp_rcv_check_fn;
 
typedef int (*blog_l2tp_rcv_check_t)(void *dev, uint16_t tunnel_id, 
                                     uint16_t session_id);
extern blog_l2tp_rcv_check_t blog_l2tp_rcv_check_fn;

#if defined(CONFIG_BCM_OVS)
typedef int (* blog_is_ovs_internal_dev_t)(void *dev);
typedef unsigned long (* blog_mega_get_key_t)(void *mega);
typedef void (* blog_mega_set_key_t)(void *mega, unsigned long blog_key);
typedef void (* blog_mega_put_fast_stats_t)(void *net_p, 
        const blog_fast_stats_t *stats);

typedef struct {
    blog_is_ovs_internal_dev_t is_ovs_internal_dev;
    blog_mega_get_key_t        mega_get_key;
    blog_mega_set_key_t        mega_set_key;
    blog_mega_put_fast_stats_t mega_put_fast_stats;
} blog_ovs_hooks_t;
void blog_bind_ovs(blog_ovs_hooks_t *blog_ovs_hooks_p);
#endif

/* __end_include_if_linux__ */



/*
 *------------------------------------------------------------------------------
 * Denotes a Blog client,
 *------------------------------------------------------------------------------
 */
typedef enum {
        BLOG_DECL(BlogClient_fcache)
#if defined(CONFIG_BCM_KF_FAP)
        BLOG_DECL(BlogClient_fap)
#endif
#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
        BLOG_DECL(BlogClient_runner)
#endif /* CONFIG_BCM_RUNNER */
#endif /* CONFIG_BCM_KF_RUNNER */
        BLOG_DECL(BlogClient_MAX)
} BlogClient_t;

/*
 *------------------------------------------------------------------------------
 * Denotes whether a packet is consumed and freed by a Blog client application,
 * whether a packet needs to be processed normally within the network stack or
 * whether a packet context is extended with a Blog_t object.
 *------------------------------------------------------------------------------
 */
typedef enum {
        BLOG_DECL(PKT_DONE)             /* Packet consumed and freed          */
        BLOG_DECL(PKT_NORM)             /* Continue normal stack processing   */
        BLOG_DECL(PKT_BLOG)             /* Continue stack with blogging       */
        BLOG_DECL(PKT_DROP)             /* Drop Packet                        */
        BLOG_DECL(PKT_TCP4_LOCAL)       /* ipv4 tcp packet terminating locally*/
        BLOG_DECL(BLOG_ACTION_MAX)
} BlogAction_t;

/*
 *------------------------------------------------------------------------------
 * Denotes the direction in the network stack when a packet is processed by a
 * virtual network interface/network device.
 *------------------------------------------------------------------------------
 */
typedef enum {
        BLOG_DECL(DIR_RX)               /* Receive path in network stack      */
        BLOG_DECL(DIR_TX)               /* Transmit path in network stack     */
        BLOG_DECL(BLOG_DIR_MAX)
} BlogDir_t;

/*
 *------------------------------------------------------------------------------
 * Denotes the type of Network entity associated with a Blog_t.
 *
 * BlogNetEntity_t may be linked to a blog using blog_link to make the Blog_t
 * point to the BlogNetEntity_t. A reverse linking from the BlogNetEntity_t to
 * Blog_t is only possible via a key (if necessary when a one to one association
 * between the BlogNetEntity_t and a Blog exists. For instance, there is a
 * one to one association between a Flow Connection Tracker and a Blog. In fact
 * a Linux Netfilter Connection Tracking object manages a bi-directional flow
 * and thus may have 2 keys to reference the corresponding Blog_t. However, a
 * network device (physical end device or a virtual device) may have multiple
 * Flows passing through it and hence no one-to-one association exists. In this
 * can a Blog may have a link to a network device, but the reverse link (via a
 * key) is not saved in the network device.
 *
 * Linking a BlogNetEntity_t to a blog is done via blog_link() whereas saving
 * a reference key into a BlogNetEntity_t is done via blog_request() by the
 * Blog client application, if needed.
 *
 *------------------------------------------------------------------------------
 */

#define BLOG_CT_PLD             0U
#define BLOG_CT_DEL             1U
#define BLOG_CT_MAX             2U

/* FLOWTRACK: param1 is ORIG=0 or REPLY=1 direction */
#define BLOG_PARAM1_DIR_ORIG    0U
#define BLOG_PARAM1_DIR_REPLY   1U
#define BLOG_PARAM1_DIR_MAX     2U

/* FLOWTRACK: param2 is IPv4=0, IPv6=1, GRE=2, L2TP=3 */
#define BLOG_PARAM2_IPV4        0U
#define BLOG_PARAM2_IPV6        1U
#define BLOG_PARAM2_GRE_IPV4    2U
#define BLOG_PARAM2_L2TP_IPV4   3U
#define BLOG_PARAM2_MAX         4U
#define BLOG_CT_VER_MAX         2U

/* MAP_TUPLE: param1 is US=0 or DS=1 direction */
#define BLOG_PARAM1_MAP_DIR_US  BLOG_PARAM1_DIR_ORIG
#define BLOG_PARAM1_MAP_DIR_DS  BLOG_PARAM1_DIR_REPLY

/* BRIDGEFDB: param1 is src|dst */
#define BLOG_PARAM1_SRCFDB      0U
#define BLOG_PARAM1_DSTFDB      1U

/* IF_DEVICE: param1 is direction RX or TX, param 2 is minMtu */

typedef enum {
        BLOG_DECL(FLOWTRACK)            /* Flow (connection|session) tracker  */
        BLOG_DECL(BRIDGEFDB)            /* Bridge Forwarding Database entity  */
        BLOG_DECL(MCAST_FDB)            /* Multicast Client FDB entity        */
        BLOG_DECL(IF_DEVICE)            /* Virtual Interface (network device) */
        BLOG_DECL(IF_DEVICE_MCAST)      /* Virtual Interface (network device) */
        BLOG_DECL(GRE_TUNL)             /* GRE Tunnel                         */
        BLOG_DECL(TOS_MODE)             /* TOS_MODE                           */
        BLOG_DECL(MAP_TUPLE)            /* Flow (MAP-T connection) tracker    */
        BLOG_DECL(MEGA)                 /* Megaflow tracker                   */
        BLOG_DECL(BLOG_NET_ENTITY_MAX)
} BlogNetEntity_t;

/*
 *------------------------------------------------------------------------------
 * Denotes a type of notification sent from the network stack to the Blog client
 * See blog_notify(BlogNotify_t, void *, unsigned long param1, uint32_t param2);
 *------------------------------------------------------------------------------
 */

/* MCAST_CONTROL_EVT: param1 is add|del, and param2 is IPv4|IPv6 */ 
#define BLOG_PARAM1_MCAST_ADD       0U
#define BLOG_PARAM1_MCAST_DEL       1U
#define BLOG_PARAM2_MCAST_IPV4      0U
#define BLOG_PARAM2_MCAST_IPV6      1U

/* LINK_STATE_CHANGE: param1 */
#define BLOG_PARAM1_LINK_STATE_UP   0U
#define BLOG_PARAM1_LINK_STATE_DOWN 1U

typedef enum {
        BLOG_DECL(DESTROY_FLOWTRACK)    /* Session/connection is deleted      */
        BLOG_DECL(DESTROY_BRIDGEFDB)    /* Bridge FDB has aged                */
        BLOG_DECL(MCAST_CONTROL_EVT)    /* Mcast client joins a group event   */
        BLOG_DECL(MCAST_SYNC_EVT)       /* Topology change for mcast event    */
        BLOG_DECL(DESTROY_NETDEVICE)    /* Network device going down          */
        BLOG_DECL(FETCH_NETIF_STATS)    /* Fetch accumulated stats            */
        BLOG_DECL(CLEAR_NETIF_STATS)    /* Clear accumulated stats            */
        BLOG_DECL(DYNAMIC_DSCP_EVENT)   /* Dynamic DSCP change event          */
        BLOG_DECL(UPDATE_NETDEVICE)     /* Netdevice has been modified (MTU, etc) */
        BLOG_DECL(ARP_BIND_CHG)         /* ARP IP/MAC binding change event    */
        BLOG_DECL(CONFIG_CHANGE)        /* Certain configuration change event */
        BLOG_DECL(UP_NETDEVICE)         /* network device up                  */
        BLOG_DECL(DN_NETDEVICE)         /* network device down                */
        BLOG_DECL(CHANGE_ADDR)          /* network device change MAC addr     */
        BLOG_DECL(SET_DPI_PARAM)        /* Set the DPI parameters             */
        BLOG_DECL(DESTROY_MAP_TUPLE)    /* MAPT Session/connection is deleted */
        BLOG_DECL(FLUSH)                /* Flush flows based on parameters    */
        BLOG_DECL(DESTROY_MEGA)         /* Megaflow connection is deleted     */
        BLOG_DECL(FETCH_MEGA_STATS)     /* Fetch megaflow fast stats          */
        BLOG_DECL(CLEAR_MEGA_STATS)     /* Clear megaflow fast stats          */
        BLOG_DECL(BLOG_NOTIFY_MAX)
} BlogNotify_t;

typedef enum {
        BLOG_DECL(QUERY_FLOWTRACK)      /* Session/connection time is queried */
        BLOG_DECL(QUERY_BRIDGEFDB)      /* Bridge FDB time is queried         */
        BLOG_DECL(QUERY_MAP_TUPLE)      /* MAP-T connection time is queried   */
        BLOG_DECL(QUERY_FLOWTRACK_STATS)/* get stats of flows associated with NPE */
        BLOG_DECL(QUERY_GET_HW_ACCEL)
        BLOG_DECL(BLOG_QUERY_MAX)
} BlogQuery_t;

typedef struct{
    int orig_queue;                     /* Originating queue index */
    int reply_queue;                    /* Reply queue index */
    int priority;                       /* Traffic priority */
}BlogDpiParams_t;


/* Blog Notify FLUSH strucutre */
typedef int (* BlogFlushMetadataFunc_t)(void *metadata_p, const Blog_t *const blog_p);

typedef struct {
    uint32_t flush_all      :1;
    uint32_t flush_flow     :1;
    uint32_t flush_dev      :1;
    uint32_t flush_dstmac   :1;
    uint32_t flush_srcmac   :1;
    uint32_t flush_meta     :1;         /* Not available through Userspace/CLI */
    uint32_t flush_hw       :1;         /* Flush all flows from HW */
    uint32_t flush_unused   :25;
    uint8_t mac[6];
    int devid;
    int flowid;
    void *metadata_p;                   /* flush_meta = 1 ; Must set metadata_p, devid & flush_dev */
    BlogFlushMetadataFunc_t func_p;     /* flush_meta = 1 ; Must provide callback func */
}BlogFlushParams_t;

/*
 *------------------------------------------------------------------------------
 * Denotes a type of request from a Blog client to a network stack entity.
 *------------------------------------------------------------------------------
 */

typedef enum {
        BLOG_DECL(FLOWTRACK_KEY_SET)    /* Set Client key into Flowtracker    */
        BLOG_DECL(FLOWTRACK_KEY_GET)    /* Get Client key into Flowtracker    */
        BLOG_DECL(FLOWTRACK_DSCP_GET)   /* Get DSCP from Flow tracker:DYNDSCP */
        BLOG_DECL(FLOWTRACK_CONFIRMED)  /* Test whether session is confirmed  */
        BLOG_DECL(FLOWTRACK_ALG_HELPER) /* Test whether flow has an ALG       */
        BLOG_DECL(FLOWTRACK_EXCLUDE)    /* Clear flow candidacy by Client     */
        BLOG_DECL(FLOWTRACK_TIME_SET)   /* Set time in a flow tracker         */
        BLOG_DECL(FLOWTRACK_PUT_STATS)  /* Push accumulated stats to conntrack*/
        BLOG_DECL(NETIF_PUT_STATS)      /* Push accumulated stats to devices  */
        BLOG_DECL(LINK_XMIT_FN)         /* Fetch device link transmit function*/
        BLOG_DECL(LINK_NOCARRIER)       /* Fetch device link carrier          */
        BLOG_DECL(NETDEV_NAME)          /* Network device name                */
        BLOG_DECL(MCAST_DFLT_MIPS)      /* Delete action in blogRule chain    */
        BLOG_DECL(IQPRIO_SKBMARK_SET)   /* Set IQOS Prio in skb->mark         */
        BLOG_DECL(DPIQ_SKBMARK_SET)     /* Set DPIQ in skb->mark              */
        BLOG_DECL(BRIDGEFDB_KEY_SET)    /* Set Client key into bridge FDB     */
        BLOG_DECL(BRIDGEFDB_KEY_GET)    /* Get Client key into bridge FDB     */
        BLOG_DECL(BRIDGEFDB_TIME_SET)   /* Refresh bridge FDB time            */
        BLOG_DECL(BRIDGEFDB_IFIDX_GET)  /* Get bridge FDB's device ifindex    */
        BLOG_DECL(SYS_TIME_GET)         /* Get the system time in jiffies     */
        BLOG_DECL(GRE_TUNL_XMIT)        /* GRE Tunnel tx                      */
        BLOG_DECL(GRE6_TUNL_XMIT)       /* GRE6 Tunnel tx                      */
        BLOG_DECL(SKB_DST_ENTRY_SET)    /* get dst_entry from skb             */
        BLOG_DECL(SKB_DST_ENTRY_RELEASE)/* release dst_entry from blog        */
        BLOG_DECL(NETDEV_ADDR)          /* Device MAC addr                    */
        BLOG_DECL(FLOW_EVENT_ACTIVATE)  /* Flow Activation event              */
        BLOG_DECL(FLOW_EVENT_DEACTIVATE)/* Flow Deactivation event            */
        BLOG_DECL(CHK_HOST_DEV_MAC)     /* Check Dev HostMAC for addition     */
        BLOG_DECL(MAP_TUPLE_KEY_SET)    /* Set Client key into MAPT Tuple     */
        BLOG_DECL(MAP_TUPLE_KEY_GET)    /* Get Client key into MAPT Tuple     */
        BLOG_DECL(MEGA_KEY_SET)         /* Set Client key into megaflow       */
        BLOG_DECL(MEGA_KEY_GET)         /* Get Client key into megaflow       */
        BLOG_DECL(MEGA_PUT_STATS)       /* Put the stats in a megaflow        */
        BLOG_DECL(BLOG_REQUEST_MAX)
} BlogRequest_t;

/*
 *------------------------------------------------------------------------------
 * Denotes a type of update to an existing Blog flow.
 *------------------------------------------------------------------------------
 */

typedef enum {
        BLOG_DECL(BLOG_UPDATE_DPI_QUEUE)     /* DPI Queue assignment has changed */
        BLOG_DECL(BLOG_UPDATE_DPI_PRIORITY)  /* DPI Priority assignment has changed */
        BLOG_DECL(BLOG_UPDATE_MAX)
} BlogUpdate_t;

/*
 *------------------------------------------------------------------------------
 * Flow event parameters
 *------------------------------------------------------------------------------
 */

typedef enum {
        BLOG_DECL(FLOW_EVENT_TYPE_FC)        /* FCache Flow */
        BLOG_DECL(FLOW_EVENT_TYPE_HW)        /* Hardware Flow */
        BLOG_DECL(FLOW_EVENT_TYPE_MAX)
} BlogFlowEventType_t;


typedef struct {
    void *ct_pld_p;
    void *ct_del_p;
    union {
        struct {
            uint32_t is_downstream      :1;
            uint32_t flow_event_type    :2;
            uint32_t reserved           :21;
            uint32_t skb_mark_flow_id   :8;
        };
        uint32_t u32;
    };
} BlogFlowEventInfo_t;


/*
 *------------------------------------------------------------------------------
 * Clean this up.
 *------------------------------------------------------------------------------
 */

#define BLOG_ENCAP_MAX          6       /* Maximum number of L2 encaps        */
#define BLOG_HDRSZ_MAX          32      /* Maximum size of L2 encaps          */

typedef enum {
        BLOG_DECL(GRE_ETH)             /* e.g. BLOG_XTMPHY, BLOG_GPONPHY     */
        BLOG_DECL(BCM_XPHY)             /* e.g. BLOG_XTMPHY, BLOG_GPONPHY     */
        BLOG_DECL(BCM_SWC)              /* BRCM LAN Switch Tag/Header         */
        BLOG_DECL(ETH_802x)             /* Ethernet                           */
        BLOG_DECL(VLAN_8021Q)           /* Vlan 8021Q (incld stacked)         */
        BLOG_DECL(PPPoE_2516)           /* PPPoE RFC 2516                     */
        BLOG_DECL(PPP_1661)             /* PPP RFC 1661                       */
        BLOG_DECL(PLD_IPv4)             /* Payload IPv4                       */
        BLOG_DECL(PLD_IPv6)             /* Payload IPv6                       */
        BLOG_DECL(PPTP)                 /* PPTP Header                        */
        BLOG_DECL(L2TP)                 /* L2TP Header                        */
        BLOG_DECL(GRE)                  /* GRE Header                         */
        BLOG_DECL(ESP)                  /* ESP Header                         */
        BLOG_DECL(DEL_IPv4)             /* Outer IPv4                         */
        BLOG_DECL(DEL_IPv6)             /* Outer IPv6                         */
        BLOG_DECL(DEL_L2)               /* L2 DEL                             */
        BLOG_DECL(PLD_L2)               /* L2 PLD                             */
        BLOG_DECL(HDR0_IPv4)            /* IPv4 Inner Header 0                */
        BLOG_DECL(HDR0_IPv6)            /* IPv6 Inner Header 0                */
        BLOG_DECL(HDR0_L2)              /* L2 Inner Header 0                  */
        BLOG_DECL(GREoESP_type)         /* GRE over ESP type                  */
        BLOG_DECL(GREoESP_type_resvd)   /* GRE over ESP type                  */
        BLOG_DECL(GREoESP)              /* GRE over ESP                       */
        BLOG_DECL(unused1)              /* unused1                            */
        BLOG_DECL(PASS_THRU)            /* pass-through                       */
        BLOG_DECL(unused)               /* unused                             */
        BLOG_DECL(PROTO_MAX)
} BlogEncap_t;






/* CAUTION: Following macros have binary dependencies. Please do not change these
   macros without consulting with Broadcom or the subsystem owners
   Macro definition START */
#define BLOG_IS_HWACC_DISABLED_WLAN_EXTRAPHY(rxphy,txphy) ((rxphy == BLOG_EXTRA1PHY) || \
                                                           (txphy == BLOG_EXTRA1PHY))
#define BLOG_IS_TX_HWACC_ENABLED_WLAN_PHY(txphy) (txphy == BLOG_WLANPHY)
/* Macro definition END */

/*
 *------------------------------------------------------------------------------
 * Logging of a maximum 4 "virtual" network devices that a flow can traverse.
 * Virtual devices are interfaces that do not perform the actual DMA transfer.
 * E.g. an ATM interface would be referred to as a physical interface whereas
 * a ppp interface would be referred to as a Virtual interface.
 *------------------------------------------------------------------------------
 */
#define MAX_VIRT_DEV           7

#define DEV_DIR_MASK           0x3ul
#define DEV_PTR_MASK           (~DEV_DIR_MASK)
#define DEV_DIR(ptr)           ((uintptr_t)(ptr) & DEV_DIR_MASK)

#define IS_RX_DIR(ptr)         ( DEV_DIR(ptr) == DIR_RX )
#define IS_TX_DIR(ptr)         ( DEV_DIR(ptr) == DIR_TX )

/*
 *------------------------------------------------------------------------------
 * Device pointer conversion between with and without embeded direction info
 *------------------------------------------------------------------------------
 */
#define DEVP_APPEND_DIR(ptr,dir) ((void *)((uintptr_t)(ptr) | (uintptr_t)(dir)))
#define DEVP_DETACH_DIR(ptr)     ((void *)((uintptr_t)(ptr) & (uintptr_t) \
                                                              DEV_PTR_MASK))
/*
 *------------------------------------------------------------------------------
 * Denotes the tos mode.
 *------------------------------------------------------------------------------
 */
typedef enum {
    BLOG_DECL(BLOG_TOS_FIXED)
    BLOG_DECL(BLOG_TOS_INHERIT)
    BLOG_DECL(BLOG_TOS_MAX)
} BlogTos_t;

/*
 *------------------------------------------------------------------------------
 * Blog statistics structure
 *------------------------------------------------------------------------------
 */
typedef struct{
    /* NOTE : All these structure variables should be of same type/size */

    uint64_t  rx_packets;             /* total blog packets received    */
    uint64_t  tx_packets;             /* total blog packets transmitted */
    uint64_t  rx_bytes;               /* total blog bytes received      */
    uint64_t  tx_bytes;               /* total blog bytes transmitted   */
    uint64_t  multicast;              /* total blog multicast packets   */
    uint64_t  tx_multicast_packets;   /* multicast packets transmitted */
    uint64_t  rx_multicast_bytes;     /* multicast bytes recieved */ 
    uint64_t  tx_multicast_bytes;     /* multicast bytes transmitted */
} BlogStats_t;

typedef enum {
    BLOG_DECL(blog_skip_reason_unknown = 0) /* unknown or customer defined */
    BLOG_DECL(blog_skip_reason_br_flood)
    BLOG_DECL(blog_skip_reason_ct_tcp_state_not_est)
    BLOG_DECL(blog_skip_reason_ct_tcp_state_ignore)
    BLOG_DECL(blog_skip_reason_ct_status_donot_blog)
    BLOG_DECL(blog_skip_reason_nf_xt_skiplog)
    BLOG_DECL(blog_skip_reason_nf_ebt_skiplog)
    BLOG_DECL(blog_skip_reason_scrub_pkt)
    BLOG_DECL(blog_skip_reason_sch_htb)
    BLOG_DECL(blog_skip_reason_sch_dsmark)
    BLOG_DECL(blog_skip_reason_unknown_proto)
    BLOG_DECL(blog_skip_reason_unknown_proto_ah4)
    BLOG_DECL(blog_skip_reason_unknown_proto_ah6)
    BLOG_DECL(blog_skip_reason_unknown_proto_esp6)
    BLOG_DECL(blog_skip_reason_esp4_crypto_algo)
    BLOG_DECL(blog_skip_reason_esp4_spu_disabled)
    BLOG_DECL(blog_skip_reason_spudd_check_failure)
    BLOG_DECL(blog_skip_reason_dpi)
    BLOG_DECL(blog_skip_reason_bond)
    BLOG_DECL(blog_skip_reason_map_tcp)
    BLOG_DECL(blog_skip_reason_blog)
    BLOG_DECL(blog_skip_reason_l2_local_termination)
    BLOG_DECL(blog_skip_reason_mega_multi_output_ports)
    BLOG_DECL(blog_skip_reason_mega_attr_mismatch)
    BLOG_DECL(blog_skip_reason_mega_field_mismatch)
    BLOG_DECL(blog_skip_reason_max)
} blog_skip_reason_t;

typedef enum {
    BLOG_DECL(blog_free_reason_unknown = 0) /* unknown or customer defined */
    BLOG_DECL(blog_free_reason_blog_emit)
    BLOG_DECL(blog_free_reason_blog_iq_prio)
    BLOG_DECL(blog_free_reason_kfree)
    BLOG_DECL(blog_free_reason_ipmr_local)
    BLOG_DECL(blog_free_reason_max)
} blog_free_reason_t;

typedef struct {
    uint32_t blog_get;
    uint32_t blog_put;
    uint32_t blog_skip;
    uint32_t blog_free;
    uint32_t blog_xfer;
    uint32_t blog_clone;
    uint32_t blog_copy;
    uint32_t blog_min_avail;
} blog_info_stats_t;

#define BLOG_DUMP_DISABLE   0
#define BLOG_DUMP_RXBLOG    1
#define BLOG_DUMP_TXBLOG    2
#define BLOG_DUMP_RXTXBLOG  3

typedef struct blog_ctx {
    uint32_t  blog_total;
    uint32_t  blog_avail;
    uint32_t  blog_mem_fails;
    uint32_t  blog_extends;
    uint32_t  blog_extend_fails;
    blog_info_stats_t  info_stats;
    blog_skip_reason_t blog_skip_stats_table[blog_skip_reason_max];
    blog_free_reason_t blog_free_stats_table[blog_free_reason_max];
    uint32_t  blog_dump;
} blog_ctx_t;


/*
 * -----------------------------------------------------------------------------
 * Support accleration of L2, L3 packets.
 *
 * When acceleration support is enabled system wide, the default to be used may
 * be set in CC_BLOG_SUPPORT_ACCEL_MODE which gets saved in blog_support_accel_mode_g.
 * One may change the default (at runtime) by invoking blog_support_accel_mode().
 * -----------------------------------------------------------------------------
 */


/*
 * -----------------------------------------------------------------------------
 * Acceleration support:
 * All the platforms support L3 tuple based acceleatiion.
 * But FAP and PON platforns do NOT supports the fcache L2 tuple based acceleation.
 * When the acceleation mode is configured as L23, the accelerators decides
 * on per packet basis whether to use L2 or L3 tuple based acceleration.
 * -----------------------------------------------------------------------------
 */
#define BLOG_ACCEL_MODE_L3             0    /* Legacy. All platforms support*/
#define BLOG_ACCEL_MODE_L23            1    /* Not supported on FAP and PON */

#define CC_BLOG_SUPPORT_ACCEL_MODE     (BLOG_ACCEL_MODE_L3)

extern int blog_support_accel_mode_g;

typedef int (*blog_accel_mode_set_t)(uint32_t accel_mode);
extern blog_accel_mode_set_t blog_accel_mode_set_fn;

extern void blog_support_accel_mode(int accel_mode);
extern int blog_support_get_accel_mode(void);

extern int blog_support_tcp_ack_mflows_g;

typedef int (*blog_tcp_ack_mflows_set_t)(int enable);
extern blog_tcp_ack_mflows_set_t blog_tcp_ack_mflows_set_fn;

extern void blog_support_set_tcp_ack_mflows(int enable);
extern int blog_support_get_tcp_ack_mflows(void);

/*
 * -----------------------------------------------------------------------------
 * Support blogging of multicast packets.
 *
 * When Multicast support is enabled system wide, the default to be used may
 * be set in CC_BLOG_SUPPORT_MCAST which gets saved in blog_support_mcast_g.
 * One may change the default (at runtime) by invoking blog_support_mcast().
 * -----------------------------------------------------------------------------
 */

/* Multicast Support for IPv4 and IPv6 Control */
#define BLOG_MCAST_DISABLE          0
#define BLOG_MCAST_IPV4             1
#define BLOG_MCAST_IPV6             2

#ifdef CONFIG_BLOG_MCAST
#define CC_BLOG_SUPPORT_MCAST        BLOG_MCAST_IPV4 + BLOG_MCAST_IPV6
#else
#define CC_BLOG_SUPPORT_MCAST        BLOG_MCAST_DISABLE
#endif

extern int blog_support_mcast_g;
extern void blog_support_mcast(int enable);

/*
 * -----------------------------------------------------------------------------
 * Support learning of multicast packets.
 *
 * When Multicast learn support is enabled system wide, the default to be used
 * may be set in CC_BLOG_SUPPORT_MCAST_LEARN which gets saved in
 * blog_support_mcast_learn_g. One may change the default (at runtime) by
 * invoking blog_support_mcast_learn().
 * -----------------------------------------------------------------------------
 */

/* Multicast Learning Support Enable/Disable Control */
#define BLOG_MCAST_LEARN_DISABLE            0
#define BLOG_MCAST_LEARN_ENABLE             1
#define BLOG_MCAST_LEARN_ENABLE_1ST_CLIENT  2

/* If BRCM MCAST OVS Support is enabled, enable multicast learning
   by default to allow learning OVS slowpath actions */
#if defined(CONFIG_BCM_OVS_MCAST)
#define CONFIG_BLOG_MCAST_LEARN 1
#endif

#ifdef CONFIG_BLOG_MCAST_LEARN
#define CC_BLOG_SUPPORT_MCAST_LEARN        BLOG_MCAST_LEARN_ENABLE
#else
#define CC_BLOG_SUPPORT_MCAST_LEARN        BLOG_MCAST_LEARN_DISABLE
#endif

extern int blog_support_mcast_learn_g;
extern void blog_support_mcast_learn(int enable);

/*
 * -----------------------------------------------------------------------------
 * Support blogging of IPv6 traffic
 *
 * When IPv6 support is enabled system wide, the default to be used may
 * be set in CC_BLOG_SUPPORT_IPV6 which gets saved in blog_support_ipv6_g.
 * One may change the default (at runtime) by invoking blog_support_ipv6().
 * -----------------------------------------------------------------------------
 */

/* IPv6 Support Control: see blog_support_ipv6_g and blog_support_ipv6() */
#define BLOG_IPV6_DISABLE           0
#define BLOG_IPV6_ENABLE            1

#ifdef CONFIG_BLOG_IPV6
#define CC_BLOG_SUPPORT_IPV6        BLOG_IPV6_ENABLE
#else
#define CC_BLOG_SUPPORT_IPV6        BLOG_IPV6_DISABLE
#endif

extern int blog_support_ipv6_g;
extern void blog_support_ipv6(int enable);

/*
 * -----------------------------------------------------------------------------
 * Support blogging of 6rd tos
 *
 * When 6rd is configured, the default to be used may be set in
 * CC_BLOG_DEFAULT_TUNL_TOS which gets saved in blog_tunl_tos_g.
 * One may change the default (at runtime) by invoking blog_tunl_tos().
 * -----------------------------------------------------------------------------
 */

/* GRE Support: enable/disable */
#define BLOG_GRE_DISABLE          0
#define BLOG_GRE_ENABLE           1

#ifdef CONFIG_BLOG_GRE
#define CC_BLOG_SUPPORT_GRE        BLOG_GRE_ENABLE
#else
#define CC_BLOG_SUPPORT_GRE        BLOG_GRE_DISABLE
#endif

extern int blog_gre_tunnel_accelerated_g;
extern int blog_support_gre_g;
extern void blog_support_gre(int enable);

/* L2TP Support */
#define BLOG_L2TP_DISABLE             0
#define BLOG_L2TP_TUNNEL              1
#define BLOG_L2TP_TUNNEL_WITHCHKSUM   2

#ifdef CONFIG_BLOG_L2TP
#define CC_BLOG_SUPPORT_L2TP       BLOG_L2TP_TUNNEL
#else
#define CC_BLOG_SUPPORT_L2TP       BLOG_L2TP_DISABLE
#endif

extern int blog_l2tp_tunnel_accelerated_g;
extern int blog_support_l2tp_g;
extern void blog_support_l2tp(int enable);

/* ESP Support: tunnel and pass-thru modes */
#define BLOG_ESP_DISABLE          0
#define BLOG_ESP_TUNNEL           1
#define BLOG_ESP_PASS_THRU        2

#ifdef CONFIG_BLOG_ESP
#define CC_BLOG_SUPPORT_ESP        BLOG_ESP_TUNNEL
#else
#define CC_BLOG_SUPPORT_ESP        BLOG_ESP_DISABLE
#endif

/*
 * -----------------------------------------------------------------------------
 * Support 4o6 fragmentation enable/disable
 * -----------------------------------------------------------------------------
 */
#define BLOG_4O6_FRAG_DISABLE           0
#define BLOG_4O6_FRAG_ENABLE            1

extern int blog_support_4o6_frag_g;
extern void blog_support_4o6_frag(int enable);

/* blog notify processing mode */
typedef enum {
        BLOG_DECL(BLOG_NOTIFY_PROC_MODE_NOW) /* processing mode: now/sync */
        BLOG_DECL(BLOG_NOTIFY_PROC_MODE_HYBRID)/* mode: now+deferred      */
        BLOG_DECL(BLOG_NOTIFY_PROC_MODE_DFRD)/* processing mode: deferred */
        BLOG_DECL(BLOG_NOTIFY_PROC_MODE_MAX)
} blog_notify_proc_mode_t;

extern int blog_notify_proc_mode_g;
extern void blog_set_notify_proc_mode(int mode);

typedef enum {
        BLOG_DECL(BLOG_NOTIFY_API_SYNC) /* blog_notify() */
        BLOG_DECL(BLOG_NOTIFY_API_ASYNC)/* blog_notify_async() */
        BLOG_DECL(BLOG_NOTIFY_API_MAX)
} blog_notify_api_t;

typedef enum
{
    BLOG_DECL(BLOG_NOTIFY_EVT_NONE)
    BLOG_DECL(BLOG_NOTIFY_EVT_FLUSH_FDB)
    BLOG_DECL(BLOG_NOTIFY_EVT_FLUSH_NPE)
    BLOG_DECL(BLOG_NOTIFY_EVT_FLUSH_ARP)
    BLOG_DECL(BLOG_NOTIFY_EVT_FLUSH)
    BLOG_DECL(BLOG_NOTIFY_EVT_FLUSH_HW)
    BLOG_DECL(BLOG_NOTIFY_EVT_FLUSH_DEV)
    BLOG_DECL(BLOG_NOTIFY_EVT_FLUSH_PARAMS)
    BLOG_DECL(BLOG_NOTIFY_EVT_FETCH_NETIF_STATS)
    BLOG_DECL(BLOG_NOTIFY_EVT_CLEAR_NETIF_STATS)
    BLOG_DECL(BLOG_NOTIFY_EVT_MAX)
} blog_notify_evt_type_t;

/* Traffic type */
typedef enum {
    BLOG_DECL(BlogTraffic_IPV4_UCAST)
    BLOG_DECL(BlogTraffic_IPV6_UCAST)
    BLOG_DECL(BlogTraffic_IPV4_MCAST)
    BLOG_DECL(BlogTraffic_IPV6_MCAST)
    BLOG_DECL(BlogTraffic_Layer2_Flow)
    BLOG_DECL(BlogTraffic_MAX)
} BlogTraffic_t;

typedef union {
    uint32_t word;
    struct {
        BE_DECL(
            uint32_t incarn  :  3; /* Allocation instance identification */
            uint32_t self    : 29; /* Index into static allocation table */
        )
        LE_DECL(
            uint32_t self    : 29; /* Index into static allocation table */
            uint32_t incarn  :  3; /* Allocation instance identification */
        )
    };
} BlogKeyFc_t;

typedef union {
    uint32_t word;
    struct {
        BE_DECL(
            uint32_t resvd  : 16;
            uint32_t intf   :  8;
            uint32_t client :  8;
        )
        LE_DECL(
            uint32_t client :  8;
            uint32_t intf   :  8;
            uint32_t resvd  : 16;
        )
    };
} BlogKeyMc_t;

#define BLOG_FDB_KEY_INVALID        BLOG_KEY_NONE
#define BLOG_KEY_FC_INVALID         BLOG_KEY_NONE
#define BLOG_KEY_FC_TUNNEL_IPV4     0xFFFFFFFE
#define BLOG_KEY_FC_TUNNEL_IPV6     0xFFFFFFFF

#define BLOG_KEY_INVALID            BLOG_KEY_NONE
#define BLOG_KEY_MCAST_INVALID      BLOG_KEY_INVALID

typedef struct {
    BE_DECL(
        BlogKeyFc_t fc;
        BlogKeyMc_t mc;
    )
    LE_DECL(
        BlogKeyMc_t mc;
        BlogKeyFc_t fc;
    )
} BlogActivateKey_t;

typedef enum
{
    BLOG_DECL(BLOG_L2_KEYMAP_MACSA)
    BLOG_DECL(BLOG_L2_KEYMAP_MACDA)
    BLOG_DECL(BLOG_L2_KEYMAP_ETHTYPE)
    BLOG_DECL(BLOG_L2_KEYMAP_VLAN0)
    BLOG_DECL(BLOG_L2_KEYMAP_VLAN1)
    BLOG_DECL(BLOG_L2_KEYMAP_TOS)
    BLOG_DECL(BLOG_L2_KEYMAP_TCPACK)
} blog_l2_keymap_field_t;

typedef enum
{
    BLOG_DECL(BLOG_L3_KEYMAP_IPV4SA)
    BLOG_DECL(BLOG_L3_KEYMAP_IPV4DA)
    BLOG_DECL(BLOG_L3_KEYMAP_IPV6SA)
    BLOG_DECL(BLOG_L3_KEYMAP_IPV6DA)
    BLOG_DECL(BLOG_L3_KEYMAP_PROTO)
    BLOG_DECL(BLOG_L3_KEYMAP_SPORT)
    BLOG_DECL(BLOG_L3_KEYMAP_DPORT)
    BLOG_DECL(BLOG_L3_KEYMAP_TOS)
    BLOG_DECL(BLOG_L3_KEYMAP_TCPACK)
} blog_l3_keymap_field_t;


#define BLOG_SET_L2KEYMAP(b, v0, v1, t)     \
    b->l2_keymap = (1<<BLOG_L2_KEYMAP_MACSA \
               | 1<<BLOG_L2_KEYMAP_MACDA \
               | 1<<BLOG_L2_KEYMAP_ETHTYPE \
               | v0<<BLOG_L2_KEYMAP_VLAN0 \
               | v1<<BLOG_L2_KEYMAP_VLAN1 \
               | t<<BLOG_L2_KEYMAP_TOS)

#define BLOG_SET_IPV4_PT_L3KEYMAP(b)     \
    b->l3_keymap = (1<<BLOG_L3_KEYMAP_IPV4SA \
               | 1<<BLOG_L3_KEYMAP_IPV4DA \
               | 1<<BLOG_L3_KEYMAP_PROTO \
               | 1<<BLOG_L3_KEYMAP_TOS)

#define BLOG_SET_IPV4_L3KEYMAP(b)     \
    b->l3_keymap = (1<<BLOG_L3_KEYMAP_IPV4SA \
               | 1<<BLOG_L3_KEYMAP_IPV4DA \
               | 1<<BLOG_L3_KEYMAP_PROTO \
               | 1<<BLOG_L3_KEYMAP_SPORT \
               | 1<<BLOG_L3_KEYMAP_DPORT \
               | 1<<BLOG_L3_KEYMAP_TOS)

#define BLOG_SET_IPV6_PT_L3KEYMAP(b)     \
    b->l3_keymap = (1<<BLOG_L3_KEYMAP_IPV6SA \
               | 1<<BLOG_L3_KEYMAP_IPV6DA \
               | 1<<BLOG_L3_KEYMAP_PROTO \
               | 1<<BLOG_L3_KEYMAP_TOS)

#define BLOG_SET_IPV6_L3KEYMAP(b)     \
    b->l3_keymap = (1<<BLOG_L3_KEYMAP_IPV6SA \
               | 1<<BLOG_L3_KEYMAP_IPV6DA \
               | 1<<BLOG_L3_KEYMAP_PROTO \
               | 1<<BLOG_L3_KEYMAP_SPORT \
               | 1<<BLOG_L3_KEYMAP_DPORT \
               | 1<<BLOG_L3_KEYMAP_TOS)


/* Is the megaflow valid ? */
#define IS_BLOG_MEGA(b)   (((b)->mega_p != (void *)NULL))

/* Is the map tuple valid ? */
#define IS_BLOG_MAP(b)   ( MAPT(b) && ((b)->map_p != (void *)NULL) )

/* Before the flow is learnt, use CT macros */
#define IS_BLOG_CT_PLD(b)   ( (b)->ct_p[BLOG_CT_PLD] != (void *)NULL )
#define IS_BLOG_CT_DEL(b)   ( (b)->ct_p[BLOG_CT_DEL] != (void *)NULL )
#define IS_BLOG_CT(b)       ( IS_BLOG_CT_PLD((b)) || IS_BLOG_CT_DEL((b)) )
#define IS_BLOG_NOT_CT(b)   ( !IS_BLOG_CT_PLD((b)) && !IS_BLOG_CT_DEL((b)) )

#define IS_BLOG_CT4_PLD(b)                                              \
    ( IS_BLOG_CT_PLD((b)) &&                                            \
     ((b)->ct_ver[BLOG_CT_PLD] == BLOG_PARAM2_IPV4))

#define IS_BLOG_CT4_DEL(b)                                              \
    ( IS_BLOG_CT_DEL((b)) &&                                            \
     ((b)->ct_ver[BLOG_CT_DEL] == BLOG_PARAM2_IPV4))

#define IS_BLOG_CT6_PLD(b)                                              \
    ( IS_BLOG_CT_PLD((b)) &&                                            \
     ((b)->ct_ver[BLOG_CT_PLD] == BLOG_PARAM2_IPV6))

#define IS_BLOG_CT6_DEL(b)                                              \
    ( IS_BLOG_CT_DEL((b)) &&                                            \
     ((b)->ct_ver[BLOG_CT_DEL] == BLOG_PARAM2_IPV6))

#define IS_BLOG_CT4(b)      ( IS_BLOG_CT4_PLD(b) || IS_BLOG_CT4_DEL(b) )
#define IS_BLOG_CT6(b)      ( IS_BLOG_CT6_PLD(b) || IS_BLOG_CT6_DEL(b) )


#define BLOG_NPE_PLD            0U
#define BLOG_NPE_DEL            1U
#define BLOG_NPE_MAP            2U
#define BLOG_NPE_MEGA           3U
#define BLOG_NPE_MAX            4U

#define BLOG_FDB_NPE_SRCFDB     0U
#define BLOG_FDB_NPE_DSTFDB     1U
#define BLOG_FDB_NPE_MAX        2U

/* Is the map tuple valid ? */
#define IS_BLOG_NPE_MAP(b)   ( MAPT(b) && ((b)->npe_p[BLOG_NPE_MAP] != (void *)NULL) )
#define IS_BLOG_NPE_MEGA(b)  ( ((b)->npe_p[BLOG_NPE_MEGA] != (void *)NULL) )

/* After the flow is learnt, use NPE macros instead of CT macros */
#define IS_BLOG_NPE_PLD(b)   ( (b)->npe_p[BLOG_NPE_PLD] != (void *)NULL )
#define IS_BLOG_NPE_DEL(b)   ( (b)->npe_p[BLOG_NPE_DEL] != (void *)NULL )
#define IS_BLOG_NPE_CT(b)    ( IS_BLOG_NPE_PLD((b)) || IS_BLOG_NPE_DEL((b)) )
#define IS_BLOG_NOT_NPE_CT(b) ( !IS_BLOG_NPE_PLD((b)) && !IS_BLOG_NPE_DEL((b)) )

#define IS_BLOG_NPE4_PLD(b)                                              \
    ( IS_BLOG_NPE_PLD((b)) &&                                            \
     ((b)->ct_ver[BLOG_NPE_PLD] == BLOG_PARAM2_IPV4))

#define IS_BLOG_NPE4_DEL(b)                                              \
    ( IS_BLOG_NPE_DEL((b)) &&                                            \
     ((b)->ct_ver[BLOG_NPE_DEL] == BLOG_PARAM2_IPV4))

#define IS_BLOG_NPE6_PLD(b)                                              \
    ( IS_BLOG_NPE_PLD((b)) &&                                            \
     ((b)->ct_ver[BLOG_NPE_PLD] == BLOG_PARAM2_IPV6))

#define IS_BLOG_NPE6_DEL(b)                                              \
    ( IS_BLOG_NPE_DEL((b)) &&                                            \
     ((b)->ct_ver[BLOG_NPE_DEL] == BLOG_PARAM2_IPV6))

#define IS_BLOG_NPE4(b)      ( IS_BLOG_NPE4_PLD(b) || IS_BLOG_NPE4_DEL(b) )
#define IS_BLOG_NPE6(b)      ( IS_BLOG_NPE6_PLD(b) || IS_BLOG_NPE6_DEL(b) )

#define BLOG_NPE_NULL        ((blog_npe_t*)NULL)

/*
 *------------------------------------------------------------------------------
 * Flow Cache network proxiy entity (npe) Entry:
 *------------------------------------------------------------------------------
 */
struct blog_npe {
    struct dll_t    node;           /* npe Entity list                        */
    BlogKeyFc_t     key;            /* linking Linux nwe and npe entity       */
    struct blog_npe *chain_p;       /* npe Entity Hash list node              */
    uint32_t        hashix;         /* npe Entity hash index                  */

    int             type;           /* npe Entity type                        */
    void            *nwe_p;
    Dll_t           flow_list[BLOG_PARAM1_DIR_MAX];   /* flow lists       */
    uint32_t        flow_count[BLOG_PARAM1_DIR_MAX];  /* # of flows       */
} ____cacheline_aligned;
typedef struct blog_npe blog_npe_t;


/*
 * =============================================================================
 * CAUTION: OS and network stack may be built without CONFIG_BLOG defined.
 * =============================================================================
 */

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)

/*
 *------------------------------------------------------------------------------
 *
 *              Section: Blog Conditional Compiles CC_BLOG_SUPPORT_...
 *
 * These conditional compiles are not controlled by a system wide build process.
 * E.g. CONFIG_BLOG_MCAST is a system wide build configuration
 *      CC_BLOG_SUPPORT_MCAST is a blog defined build configuration
 *
 * Do not use any CONFIG_ or CC_BLOG_SUPPORT_ in Blog_t structure definitions.
 *
 *------------------------------------------------------------------------------
 */

/* LAB ONLY: Design development, uncomment to enable */
/* #define CC_BLOG_SUPPORT_COLOR */
/* #define CC_BLOG_SUPPORT_DEBUG */




/* To enable user filtering, see blog_filter(), invoked in blog_finit() */
/* #define CC_BLOG_SUPPORT_USER_FILTER */



/*
 * -----------------------------------------------------------------------------
 *                      Section: Definition of a Blog_t
 * -----------------------------------------------------------------------------
 */

#define BLOG_CHAN_INVALID   0xFF

#define HDR_BMAP_IPV4           1
#define HDR_BMAP_IPV6           2
#define HDR_BMAP_L2             3

/* GREoESP flag indicates whether it is GRE over ESP, or ESP over GRE */
#define BLOG_ESPoGRE            0
#define BLOG_GREoESP            1

#define BLOG_GREoESP_44         0
#define BLOG_GREoESP_64         1
#define BLOG_GREoESP_46         2
#define BLOG_GREoESP_66         3

#define BLOG_ESPoGRE_44         0
#define BLOG_ESPoGRE_64         1
#define BLOG_ESPoGRE_46         2
#define BLOG_ESPoGRE_66         3

typedef struct {
    union {
        struct {
            uint8_t         ivsize;     /* IPsec cipher Initialization Vector size */
        };
        uint8_t         channel;        /* e.g. port number, txchannel, ... */
    };

    union {
        struct {
            uint8_t         phyHdrLen   : 4;
            uint8_t         phyHdrType  : 4;
        };
        uint8_t         phyHdr;
    };

    uint16_t unused;

    union {
        struct {
            BE_DECL(
                uint32_t         unused      : 6;
                uint32_t         MAPE        : 1;
                uint32_t         PASS_THRU   : 1;

                uint32_t         unused1     : 1;
                uint32_t         GREoESP     : 1; 
                uint32_t         GREoESP_type: 2;
                uint32_t         HDR0_L2     : 1;
                uint32_t         HDR0_IPv6   : 1;
                uint32_t         HDR0_IPv4   : 1;
                uint32_t         PLD_L2      : 1;

                uint32_t         DEL_L2      : 1;
                uint32_t         DEL_IPv6    : 1;
                uint32_t         DEL_IPv4    : 1;
                uint32_t         ESP         : 1;
                uint32_t         GRE         : 1;
                uint32_t         L2TP        : 1; 
                uint32_t         PPTP        : 1;
                uint32_t         PLD_IPv6    : 1;

                uint32_t         PLD_IPv4    : 1;
                uint32_t         PPP_1661    : 1;
                uint32_t         PPPoE_2516  : 1;
                uint32_t         VLAN_8021Q  : 1;
                uint32_t         ETH_802x    : 1;
                uint32_t         BCM_SWC     : 1;
                uint32_t         BCM_XPHY    : 1;    /* e.g. BCM_XTM */
                uint32_t         GRE_ETH     : 1;    /* Ethernet over GRE */
            )
            LE_DECL(
                uint32_t         GRE_ETH     : 1;    /* Ethernet over GRE */
                uint32_t         BCM_XPHY    : 1;    /* e.g. BCM_XTM */
                uint32_t         BCM_SWC     : 1;
                uint32_t         ETH_802x    : 1;
                uint32_t         VLAN_8021Q  : 1;
                uint32_t         PPPoE_2516  : 1;
                uint32_t         PPP_1661    : 1;
                uint32_t         PLD_IPv4    : 1;

                uint32_t         PLD_IPv6    : 1;
                uint32_t         PPTP        : 1;
                uint32_t         L2TP        : 1;
                uint32_t         GRE         : 1;
                uint32_t         ESP         : 1;
                uint32_t         DEL_IPv4    : 1;
                uint32_t         DEL_IPv6    : 1;
                uint32_t         DEL_L2      : 1;

                uint32_t         PLD_L2      : 1;
                uint32_t         HDR0_IPv4   : 1;
                uint32_t         HDR0_IPv6   : 1;
                uint32_t         HDR0_L2     : 1;
                uint32_t         GREoESP_type: 2;
                uint32_t         GREoESP     : 1;
                uint32_t         unused1     : 1;

                uint32_t         PASS_THRU   : 1;
                uint32_t         MAPE        : 1;
                uint32_t         unused      : 6;
            )
        }               bmap;/* as per order of BlogEncap_t enums declaration */
        uint32_t        hdrs;
    }; 
} BlogInfo_t;

/*
 *------------------------------------------------------------------------------
 * Buffer to log IP Tuple.
 * Packed: 1 16byte cacheline.
 *------------------------------------------------------------------------------
 */
struct blogTuple_t {
    uint32_t        saddr;          /* IP header saddr */
    uint32_t        daddr;          /* IP header daddr */

    union {
        struct {
            uint16_t    source;     /* L4 source port */
            uint16_t    dest;       /* L4 dest port */
        }           port;
        struct {
            uint16_t    unused;
            uint16_t    gre_callid;
        };
        uint32_t    ports;
        uint32_t    esp_spi;
    };

    uint8_t         ttl;            /* IP header ttl */
    uint8_t         tos;            /* IP header tos */
    uint16_t        check;          /* checksum: rx tuple=l3, tx tuple=l4 */

}; 
typedef struct blogTuple_t BlogTuple_t;

#define NEXTHDR_IPV4 IPPROTO_IPIP
#define GRE_MAX_HDR_LEN  (sizeof(struct ipv6hdr) + sizeof(BLOG_HDRSZ_MAX) + BLOG_GRE_HDR_LEN)

#define HDRS_IPinIP     ((1<<GREoESP) | (3<<GREoESP_type) | (1<<GRE) | (1<<ESP) | \
                         (1<<PLD_IPv4) | (1<<PLD_IPv6) | (1<<PLD_L2) | \
                         (1<<HDR0_IPv4) | (1<<HDR0_IPv6) | (1<<HDR0_L2) |  \
                         (1<<DEL_IPv4) | (1<<DEL_IPv6) | (1<<DEL_L2))
#define HDRS_IP4in4     ((1<<PLD_IPv4) | (1<<DEL_IPv4))
#define HDRS_IP6in4     ((1<<PLD_IPv6) | (1<<DEL_IPv4))
#define HDRS_IP4in6     ((1<<PLD_IPv4) | (1<<DEL_IPv6))
#define HDRS_IP6in6     ((1<<PLD_IPv6) | (1<<DEL_IPv6))
#define HDRS_GIP4       ((1<<PLD_IPv4) | (1<<GRE))
#define HDRS_GIP6       ((1<<PLD_IPv6) | (1<<GRE))
#define HDRS_GL2        ((1<<PLD_L2) | (1<<GRE))
#define HDRS_EIP4       ((1<<PLD_IPv4) | (1<<ESP))
#define HDRS_IP2in4     ((1<<PLD_L2) | (1<<DEL_IPv4))
#define HDRS_IP2in6     ((1<<PLD_L2) | (1<<DEL_IPv6))

#define RX_IP4in6(b)    (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_IP4in6)
#define RX_IP6in4(b)    (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_IP6in4)
#define TX_IP4in6(b)    (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_IP4in6)
#define TX_IP6in4(b)    (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_IP6in4)

#define RX_IPV4(b)      ((b)->rx.info.bmap.PLD_IPv4)
#define TX_IPV4(b)      ((b)->tx.info.bmap.PLD_IPv4)
#define RX_IPV6(b)      ((b)->rx.info.bmap.PLD_IPv6)
#define TX_IPV6(b)      ((b)->tx.info.bmap.PLD_IPv6)
#define RX_IPV4_DEL(b)  ((b)->rx.info.bmap.DEL_IPv4)
#define TX_IPV4_DEL(b)  ((b)->tx.info.bmap.DEL_IPv4)
#define RX_IPV6_DEL(b)  ((b)->rx.info.bmap.DEL_IPv6)
#define TX_IPV6_DEL(b)  ((b)->tx.info.bmap.DEL_IPv6)
#define PT(b)           ((b)->tx.info.bmap.PASS_THRU)

#define RX_GRE(b)       ((b)->rx.info.bmap.GRE)
#define TX_GRE(b)       ((b)->tx.info.bmap.GRE)
#define RX_ESP(b)       ((b)->rx.info.bmap.ESP)
#define TX_ESP(b)       ((b)->tx.info.bmap.ESP)
#define RX_GRE_ETH(b)   ((b)->rx.info.bmap.GRE_ETH)
#define TX_GRE_ETH(b)   ((b)->tx.info.bmap.GRE_ETH)

#define RX_IPV4ONLY(b)  (((b)->rx.info.hdrs & HDRS_IPinIP)==(1 << PLD_IPv4))
#define TX_IPV4ONLY(b)  (((b)->tx.info.hdrs & HDRS_IPinIP)==(1 << PLD_IPv4))
#define RX_IPV6ONLY(b)  (((b)->rx.info.hdrs & HDRS_IPinIP)==(1 << PLD_IPv6))
#define TX_IPV6ONLY(b)  (((b)->tx.info.hdrs & HDRS_IPinIP)==(1 << PLD_IPv6))
#define RX_L2ONLY(b)    (((b)->rx.info.hdrs & HDRS_IPinIP)==(1 << PLD_L2))
#define TX_L2ONLY(b)    (((b)->tx.info.hdrs & HDRS_IPinIP)==(1 << PLD_L2))

#define CHK_RX_GIPV4(b)    (((b)->rx.info.hdrs & ((1 << DEL_IPv4) | (1 << PLD_IPv4))) && RX_GRE(b))
#define CHK_RX_GIPV6(b)    (((b)->rx.info.hdrs & ((1 << DEL_IPv6) | (1 << PLD_IPv6))) && RX_GRE(b))


#define RX_IPV4_OUTER(b) (RX_IPV4ONLY(b) || RX_IPV4_DEL(b))
#define TX_IPV4_OUTER(b) (TX_IPV4ONLY(b) || TX_IPV4_DEL(b))
#define PT4(b)          (RX_IPV4ONLY(b) && TX_IPV4ONLY(b) && PT(b))

#define RX_IPV6_OUTER(b) (RX_IPV6ONLY(b) || RX_IPV6_DEL(b))
#define TX_IPV6_OUTER(b) (TX_IPV6ONLY(b) || TX_IPV6_DEL(b))
#define PT6(b)          (RX_IPV6ONLY(b) && TX_IPV6ONLY(b) && PT(b))

#define HDRS_IPV4       ((1 << PLD_IPv4) | (1 << DEL_IPv4))
#define HDRS_IPV6       ((1 << PLD_IPv6) | (1 << DEL_IPv6))

#define MAPT_UP(b)      (RX_IPV4ONLY(b) && TX_IPV6ONLY(b))
#define MAPT_DN(b)      (RX_IPV6ONLY(b) && TX_IPV4ONLY(b))
#define MAPT(b)         (MAPT_DN(b) || MAPT_UP(b))

#define T4in6UP(b)      (RX_IPV4ONLY(b) && TX_IP4in6(b))
#define T4in6DN(b)      (RX_IP4in6(b) && TX_IPV4ONLY(b))

#define T6in4UP(b)      (RX_IPV6ONLY(b) && TX_IP6in4(b))
#define T6in4DN(b)      (RX_IP6in4(b) && TX_IPV6ONLY(b))

#define CHK4in6(b)      (T4in6UP(b) || T4in6DN(b))
#define CHK6in4(b)      (T6in4UP(b) || T6in4DN(b)) 
#define CHK4to4(b)      (RX_IPV4ONLY(b) && TX_IPV4ONLY(b))
#define CHK6to6(b)      (RX_IPV6ONLY(b) && TX_IPV6ONLY(b))

#define HDRS_GIP4in4    ((1<<GRE) | HDRS_IP4in4)
#define HDRS_GIP6in4    ((1<<GRE) | HDRS_IP6in4)
#define HDRS_GIP2in4    ((1<<GRE) | HDRS_IP2in4)

#define HDRS_GIP4in6    ((1<<GRE) | HDRS_IP4in6)
#define HDRS_GIP6in6    ((1<<GRE) | HDRS_IP6in6)
#define HDRS_GIP2in6    ((1<<GRE) | HDRS_IP2in6)

#define RX_GIPV4ONLY(b)  (((b)->rx.info.hdrs & HDRS_IPinIP)== HDRS_GIP4)
#define TX_GIPV4ONLY(b)  (((b)->tx.info.hdrs & HDRS_IPinIP)== HDRS_GIP4)
#define RX_GIPV6ONLY(b)  (((b)->rx.info.hdrs & HDRS_IPinIP)== HDRS_GIP6)
#define TX_GIPV6ONLY(b)  (((b)->tx.info.hdrs & HDRS_IPinIP)== HDRS_GIP6)
#define RX_GL2ONLY(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)== HDRS_GL2)
#define TX_GL2ONLY(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)== HDRS_GL2)

#define RX_GIP4in4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GIP4in4)
#define TX_GIP4in4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GIP4in4)
#define RX_GIP6in4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GIP6in4)
#define TX_GIP6in4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GIP6in4)
#define RX_GIP2in4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GIP2in4)
#define TX_GIP2in4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GIP2in4)
#define RX_GIP46in4(b)  (RX_GIP4in4(b) || RX_GIP6in4(b))
#define TX_GIP46in4(b)  (TX_GIP4in4(b) || TX_GIP6in4(b))

#define RX_GIP4in6(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GIP4in6)
#define TX_GIP4in6(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GIP4in6)
#define RX_GIP6in6(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GIP6in6)
#define TX_GIP6in6(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GIP6in6)
#define RX_GIP2in6(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GIP2in6)
#define TX_GIP2in6(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GIP2in6)
#define RX_GIP46in6(b)  (RX_GIP4in6(b) || RX_GIP6in6(b))
#define TX_GIP46in6(b)  (TX_GIP4in6(b) || TX_GIP6in6(b))

#define TG4in4UP(b)     (RX_IPV4ONLY(b) && TX_GIP4in4(b))
#define TG4in4DN(b)     (RX_GIP4in4(b) && TX_IPV4ONLY(b))
#define TG6in4UP(b)     (RX_IPV6ONLY(b) && TX_GIP6in4(b))
#define TG6in4DN(b)     (RX_GIP6in4(b) && TX_IPV6ONLY(b))
#define TG2in4UP(b)     (RX_L2ONLY(b) && TX_GIP2in4(b))
#define TG2in4DN(b)     (RX_GIP2in4(b) && TX_L2ONLY(b))

#define TGL3_4in4UP(b)  (RX_IPV4ONLY(b) && !TX_GRE_ETH(b) && TX_GIP4in4(b))
#define TGL3_4in4DN(b)  (!RX_GRE_ETH(b) && RX_GIP4in4(b) && TX_IPV4ONLY(b))
#define TGL2_4in4UP(b)  (RX_IPV4ONLY(b) && TX_GRE_ETH(b) && TX_GIP4in4(b))
#define TGL2_4in4DN(b)  (RX_GRE_ETH(b) && RX_GIP4in4(b) && TX_IPV4ONLY(b))
#define TGL2_2in4UP(b)  (RX_L2ONLY(b) && TX_GRE_ETH(b) && TX_GIP2in4(b))
#define TGL2_2in4DN(b)  (RX_GRE_ETH(b) && RX_GIP2in4(b) && TX_L2ONLY(b))

#define TG4in6UP(b)     (RX_IPV4ONLY(b) && TX_GIP4in6(b))
#define TG4in6DN(b)     (RX_GIP4in6(b) && TX_IPV4ONLY(b))
#define TG6in6UP(b)     (RX_IPV6ONLY(b) && TX_GIP6in6(b))
#define TG6in6DN(b)     (RX_GIP6in6(b) && TX_IPV6ONLY(b))
#define TG2in6UP(b)     (RX_L2ONLY(b) && TX_GIP2in6(b))
#define TG2in6DN(b)     (RX_GIP2in6(b) && TX_L2ONLY(b))

#define TG24in4UP(b)    (TG4in4UP(b) || TG2in4UP(b))
#define TG24in4DN(b)    (TG4in4DN(b) || TG2in4DN(b))

#define TG24in6UP(b)    (TG4in6UP(b) || TG2in6UP(b))
#define TG24in6DN(b)    (TG4in6DN(b) || TG2in6DN(b))

#define CHKG4in4(b)     (TG4in4UP(b) || TG4in4DN(b))
#define CHKG6in4(b)     (TG6in4UP(b) || TG6in4DN(b))
#define CHKG2in4(b)     (TG2in4UP(b) || TG2in4DN(b))
#define CHKG46in4UP(b)  (TG4in4UP(b) || TG6in4UP(b))
#define CHKG46in4DN(b)  (TG4in4DN(b) || TG6in4DN(b))
#define CHKG46in4(b)    (CHKG4in4(b) || CHKG6in4(b))
#define CHKG246in4UP(b) (TG4in4UP(b) || TG6in4UP(b) || TG2in4UP(b))
#define CHKG246in4DN(b) (TG4in4DN(b) || TG6in4DN(b) || TG2in4DN(b))
#define CHKG246in4(b)   (CHKG4in4(b) || CHKG6in4(b) || CHKG2in4(b))

#define CHKG4in6(b)     (TG4in6UP(b) || TG4in6DN(b))
#define CHKG6in6(b)     (TG6in6UP(b) || TG6in6DN(b))
#define CHKG2in6(b)     (TG2in6UP(b) || TG2in6DN(b))
#define CHKG46in6UP(b)  (TG4in6UP(b) || TG6in6UP(b))
#define CHKG46in6DN(b)  (TG4in6DN(b) || TG6in6DN(b))
#define CHKG46in6(b)    (CHKG4in6(b) || CHKG6in6(b))
#define CHKG246in6UP(b) (TG4in6UP(b) || TG6in6UP(b) || TG2in6UP(b))
#define CHKG246in6DN(b) (TG4in6DN(b) || TG6in6DN(b) || TG2in6DN(b))
#define CHKG246in6(b)   (CHKG4in6(b) || CHKG6in6(b) || CHKG2in6(b))

#define PTG4(b)         (RX_GIPV4ONLY(b) && TX_GIPV4ONLY(b) && PT(b))
#define PTG6(b)         (RX_GIPV6ONLY(b) && TX_GIPV6ONLY(b) && PT(b))
#define TOTG4(b)        (!PT(b) && ((RX_GIP4in4(b) && TX_GIP4in4(b)) || \
                            (RX_GIP6in4(b) && TX_GIP6in4(b))))
#define TOTG6(b)        (!PT(b) && ((RX_GIP4in6(b) && TX_GIP4in6(b)) || \
                            (RX_GIP6in6(b) && TX_GIP6in6(b))))

#define L2ACCEL_PTG(b)  (RX_GL2ONLY(b) && TX_GL2ONLY(b))

#define HDRS_EIP4in4    ((1<<ESP) | HDRS_IP4in4)
#define HDRS_EIP6in4    ((1<<ESP) | HDRS_IP6in4)

#define RX_EIPV4ONLY(b)  (((b)->rx.info.hdrs & HDRS_IPinIP)== HDRS_EIP4)
#define TX_EIPV4ONLY(b)  (((b)->tx.info.hdrs & HDRS_IPinIP)== HDRS_EIP4)

#define RX_EIP4in4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_EIP4in4)
#define TX_EIP4in4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_EIP4in4)
#define RX_EIP6in4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_EIP6in4)
#define TX_EIP6in4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_EIP6in4)

#define TE4in4UP(b)     (RX_IPV4ONLY(b) && TX_EIP4in4(b))
#define TE4in4DN(b)     (RX_EIP4in4(b) && TX_IPV4ONLY(b))
#define TE6in4UP(b)     (RX_IPV6ONLY(b) && TX_EIP6in4(b))
#define TE6in4DN(b)     (RX_EIP6in4(b) && TX_IPV6ONLY(b))

#define CHKE4in4(b)     (TE4in4UP(b) || TE4in4DN(b))
#define CHKE6in4(b)     (TE6in4UP(b) || TE6in4DN(b))
#define CHKE46in4(b)    (CHKE4in4(b) || CHKE6in4(b))

#define PTE4(b)         (RX_EIPV4ONLY(b) && TX_EIPV4ONLY(b))

#define HDRS_404        (1<<DEL_IPv4|1<<PLD_IPv4)
#define HDRS_464        (1<<DEL_IPv4|1<<HDR0_IPv6|1<<PLD_IPv4)
#define HDRS_606        (1<<DEL_IPv6|1<<PLD_IPv6)
#define HDRS_646        (1<<DEL_IPv6|1<<HDR0_IPv4|1<<PLD_IPv4)

#define RX_404(b)       (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_404)
#define RX_464(b)       (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_464)
#define RX_606(b)       (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_606)
#define RX_646(b)       (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_646)


#define HDRS_444        (1<<DEL_IPv4|1<<HDR0_IPv4|1<<PLD_IPv4)
#define HDRS_644        (1<<DEL_IPv4|1<<HDR0_IPv4|1<<PLD_IPv6)
#define HDRS_244        (1<<DEL_IPv4|1<<HDR0_IPv4|1<<PLD_L2)


/* GRE over ESP */
#define HDRS_4oG4oE4    (BLOG_GREoESP_44<<GREoESP_type|1<<GREoESP|1<<ESP|1<<GRE|HDRS_444)
#define HDRS_6oG4oE4    (BLOG_GREoESP_44<<GREoESP_type|1<<GREoESP|1<<ESP|1<<GRE|HDRS_644)
#define HDRS_2oG4oE4    (BLOG_GREoESP_44<<GREoESP_type|1<<GREoESP|1<<ESP|1<<GRE|HDRS_244)
/* HDRS_GE2 excludes IP Hdrs */
#define HDRS_GE2        (BLOG_GREoESP_44<<GREoESP_type|1<<GREoESP|1<<ESP|1<<GRE|1<<PLD_L2)

#define RX_4oG4oE4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_4oG4oE4)
#define TX_4oG4oE4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_4oG4oE4)
#define RX_6oG4oE4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_6oG4oE4)
#define TX_6oG4oE4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_6oG4oE4)
#define RX_2oG4oE4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_2oG4oE4)
#define TX_2oG4oE4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_2oG4oE4)
#define RX_GoEo2(b)     (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GE2)
#define TX_GoEo2(b)     (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GE2)

#define T4oG4oE4UP(b)    (RX_IPV4ONLY(b) && TX_4oG4oE4(b))
#define T4oG4oE4DN(b)    (RX_4oG4oE4(b) && TX_IPV4ONLY(b))
#define T6oG4oE4UP(b)    (RX_IPV6ONLY(b) && TX_6oG4oE4(b))
#define T6oG4oE4DN(b)    (RX_6oG4oE4(b) && TX_IPV6ONLY(b))
#define T2oG4oE4UP(b)    (RX_L2ONLY(b) && (TX_2oG4oE4(b) || TX_GoEo2(b)))
#define T2oG4oE4DN(b)    ((RX_2oG4oE4(b) || RX_GoEo2(b)) && TX_L2ONLY(b))

#define CHK4oG4oE4UP(b)  (T4oG4oE4UP(b))
#define CHK4oG4oE4DN(b)  (T4oG4oE4DN(b))
#define CHK4oG4oE4(b)    (CHK4oG4oE4UP(b) || CHK4oG4oE4DN(b))

#define CHK6oG4oE4UP(b)  (T6oG4oE4UP(b))
#define CHK6oG4oE4DN(b)  (T6oG4oE4DN(b))
#define CHK6oG4oE4(b)    (CHK6oG4oE4UP(b) || CHK6oG4oE4DN(b))

#define CHK46oG4oE4DN(b) (CHK4oG4oE4DN(b) || CHK6oG4oE4DN(b))
#define CHK46oG4oE4UP(b) (CHK4oG4oE4UP(b) || CHK6oG4oE4UP(b))
#define CHK46oG4oE4(b)   (CHK4oG4oE4(b) || CHK6oG4oE4(b))

#define CHK2oG4oE4UP(b)  (T2oG4oE4UP(b))
#define CHK2oG4oE4DN(b)  (T2oG4oE4DN(b))
#define CHK2oG4oE4(b)    (CHK2oG4oE4UP(b) || CHK2oG4oE4DN(b))


#define CHK246oG4oE4UP(b) (CHK4oG4oE4UP(b) || CHK6oG4oE4UP(b) || CHK2oG4oE4UP(b))
#define CHK246oG4oE4DN(b) (CHK4oG4oE4DN(b) || CHK6oG4oE4DN(b) || CHK2oG4oE4DN(b))
#define CHK246oG4oE4(b)   (CHK4oG4oE4(b) || CHK6oG4oE4(b) || CHK2oG4oE4(b))


#define RX_PPTP(b)       ((b)->rx.info.bmap.PPTP)
#define TX_PPTP(b)       ((b)->tx.info.bmap.PPTP)

#define RX_L2TP(b)       ((b)->rx.info.bmap.L2TP)
#define TX_L2TP(b)       ((b)->tx.info.bmap.L2TP)

#define TX_IPV6_MAPE(b)       ((b)->tx.info.bmap.MAPE)

#define RX_PPPOE(b)       ((b)->rx.info.bmap.PPPoE_2516)
#define TX_PPPOE(b)       ((b)->tx.info.bmap.PPPoE_2516)
#define PT_PPPOE(b)       (RX_PPPOE(b) && TX_PPPOE(b))


#define PKT_IPV6_GET_TOS_WORD(word)       \
   ((ntohl(word) & 0x0FF00000) >> 20)

#define PKT_IPV6_SET_TOS_WORD(word, tos)  \
   (word = htonl((ntohl(word) & 0xF00FFFFF) | ((tos << 20) & 0x0FF00000)))

/* BLOG_LOCK Definitions */
extern spinlock_t blog_lock_g;
#define BLOG_LOCK_BH()      spin_lock_bh( &blog_lock_g )
#define BLOG_UNLOCK_BH()    spin_unlock_bh( &blog_lock_g )

typedef struct ip6_addr {
    union {
        uint8_t     p8[16];
        uint16_t    p16[8];
        uint32_t    p32[4];
    };
} ip6_addr_t;

/*
 *------------------------------------------------------------------------------
 * Buffer to log IPv6 Tuple.
 * Packed: 3 16byte cachelines
 *------------------------------------------------------------------------------
 */
struct blogTupleV6_t {
    union {
        uint32_t    word0;
    };

    union {
        uint32_t    word1;
        struct {
            uint16_t length; 
            uint8_t next_hdr; 
            uint8_t rx_hop_limit;
        };
    };

    ip6_addr_t      saddr;
    ip6_addr_t      daddr;

    union {
        struct {
            uint16_t    source;     /* L4 source port */
            uint16_t    dest;       /* L4 dest port */
        }           port;
        uint32_t    ports;
    };

    union {
        struct {
            uint8_t     exthdrs:6;  /* Bit field of IPv6 extension headers */
            uint8_t     fragflag:1; /* 6in4 Upstream IPv4 fragmentation flag */
            uint8_t     tunnel:1;   /* Indication of IPv6 tunnel */
            uint8_t     tx_hop_limit;
            uint16_t    ipid;       /* 6in4 Upstream IPv4 identification */
        };
        uint32_t   word2;
    };
    
    union {      
        struct {
            uint8_t nextHdr; uint8_t hdrLen; uint16_t data16;
            uint32_t data32;
        };
        uint64_t ip6_ExtHdr;
    }; 

} ____cacheline_aligned;
typedef struct blogTupleV6_t BlogTupleV6_t;

typedef union blogGreFlags {
    uint16_t    u16;
    struct {
        BE_DECL(
            uint16_t csumIe : 1;
            uint16_t rtgIe  : 1;
            uint16_t keyIe  : 1;
            uint16_t seqIe  : 1;
            uint16_t srcRtIe: 1;
            uint16_t recurIe: 3;
            uint16_t ackIe  : 1;

            uint16_t flags  : 4;
            uint16_t ver    : 3;
        )
        LE_DECL(
            uint16_t ver    : 3;
            uint16_t flags  : 4;

            uint16_t ackIe  : 1;
            uint16_t recurIe: 3;
            uint16_t srcRtIe: 1;
            uint16_t seqIe  : 1;
            uint16_t keyIe  : 1;
            uint16_t rtgIe  : 1;
            uint16_t csumIe : 1;
        )
    };
} BlogGreFlags_t;

struct blogGre_t {
    uint8_t     l2hdr[ BLOG_HDRSZ_MAX ];    /* Data of all L2 headers */
    BlogGreFlags_t  gre_flags;
    union {
        uint16_t    u16;
        struct {
            BE_DECL(
                uint16_t reserved   : 10;
                uint16_t fragflag   :  1;
                uint16_t hlen       :  5;
            )
            LE_DECL(
                uint16_t hlen       :  5;
                uint16_t fragflag   :  1;
                uint16_t reserved   : 10;
            )
        };
    };
    uint16_t    ipid;
    uint16_t    l2_hlen;
	
    union { //pptp
        struct {
            uint16_t    keyLen;     
            uint16_t    keyId;      
        }; 
        uint32_t    key;
    };
    uint32_t            seqNum; 
    uint32_t            ackNum;
    uint16_t            pppInfo;
    uint16_t            pppProto;	
};

typedef struct blogGre_t BlogGre_t;

typedef union blogL2tpFlags { 
    uint16_t    u16;
    struct {
        BE_DECL(
            uint16_t type       : 1;
            uint16_t lenBit     : 3;
            uint16_t seqBit     : 2;
            uint16_t offsetBit  : 1;
            uint16_t priority   : 1;
            uint16_t reserved   : 4;
            uint16_t version    : 4;
        )
        LE_DECL(
            uint16_t version    : 4;
            uint16_t reserved   : 4;
            uint16_t priority   : 1;
            int16_t offsetBit   : 1;
            uint16_t seqBit     : 2;
            uint16_t lenBit     : 3;
            uint16_t type       : 1;            
        )
    };
} BlogL2tpFlags_t;

struct blogL2tp_t {
    BlogL2tpFlags_t  l2tp_flags;
    uint16_t    length;
    uint16_t    tunnelId;
    uint16_t    sessionId;
    uint16_t    seqNum;
    uint16_t    expSeqNum;
    uint16_t    offsetSize;
    uint16_t    offsetPad;
    union {
        uint16_t    u16;
        struct {
            BE_DECL(
                uint16_t reserved   : 10;
                uint16_t fragflag   :  1;
                uint16_t hlen       :  5;
            )
            LE_DECL(
                uint16_t hlen       :  5;
                uint16_t fragflag   :  1;
                uint16_t reserved   : 10;
            )
        };
    };
    uint16_t    ipid;
    uint16_t    unused;
    uint16_t    udpLen;
    uint16_t    udpCheck;
    uint16_t    pppInfo;
    uint16_t    pppProto;

};
typedef struct blogL2tp_t BlogL2tp_t;

#define BLOG_L2TP_PPP_LEN  4
#define BLOG_L2TP_PORT     1701

#define BLOG_PPTP_PPP_LEN  4
#define BLOG_PPTP_NOAC_PPPINFO  0X2145  /* pptp packet without ppp address control field 0xff03 */

#define BLOG_ESP_SPI_LEN         4
#define BLOG_ESP_SEQNUM_LEN      4
#define BLOG_ESP_PADLEN_LEN      1
#define BLOG_ESP_NEXT_PROTO_LEN  1
#define BLOG_ESP_ICV_LEN         12

struct blogEsp_t {
    uint32_t    u32;    
    union {
        uint16_t    u16;
        struct {
            BE_DECL(
                uint16_t reserved   :  7;
                uint16_t pmtudiscen :  1;
                uint16_t ipv6       :  1;
                uint16_t ipv4       :  1;
                uint16_t fragflag   :  1;
                uint16_t ivsize     :  5;
            )
            LE_DECL(
                uint16_t ivsize     :  5;
                uint16_t fragflag   :  1;
                uint16_t ipv4       :  1;
                uint16_t ipv6       :  1;
                uint16_t pmtudiscen :  1;
                uint16_t reserved   :  7;
            )
        };
    };
    uint16_t    ipid;
    void        *dst_p;
    void        *secPath_p;
};
typedef struct blogEsp_t BlogEsp_t;


/*
 *------------------------------------------------------------------------------
 * Buffer to log Layer 2 and IP Tuple headers.
 * Packed: 4 16byte cachelines
 *------------------------------------------------------------------------------
 */
struct blogHeader_t {

    BlogTuple_t         tuple;          /* L3+L4 IP Tuple log */

    union {
        BlogInfo_t      info;
        union {
            struct {
                uint32_t        word1;  /* channel, count, rfc2684, bmap */
                uint32_t        word;   /* channel, count, rfc2684, bmap */
            };
            uint32_t        pktlen;     /* stats info */
        };
    };

    struct {
        uint8_t             vlan_8021ad :1;     /* 8021AD stacked */
        uint8_t             wan_qdisc   :1;     /* device type */
        uint8_t             multicast   :1;     /* multicast flag */
        uint8_t             fkbInSkb    :1;     /* fkb from skb */
        uint8_t             count       :4;     /* # of L2 encapsulations */
    };
    uint8_t             length;         /* L2 header total length */
    uint8_t /*BlogEncap_t*/ encap[ BLOG_ENCAP_MAX ];/* All L2 header types */

    uint8_t             l2hdr[ BLOG_HDRSZ_MAX ];    /* Data of all L2 headers */

} ____cacheline_aligned;

typedef struct blogHeader_t BlogHeader_t;           /* L2 and L3+4 tuple */

/* Coarse hash key: L1, L3, L4 hash */
union blogHash_t {
    uint32_t        match;
    struct {
        union {
            struct {
                uint8_t  tcp_pure_ack : 1;
                uint8_t  unused       : 7;
            };
            uint8_t ext_match;
        };
        uint8_t     protocol;           /* IP protocol */

        struct {
            union {
                struct {
                    uint8_t         ivsize;  /* IPsec cipher Initialization Vector size */
                };
                uint8_t         channel;
            };

            union {
                struct {
                    uint8_t         phyLen   : 4;
                    uint8_t         phyType  : 4;
                };
                uint8_t         phy;
            };
        } l1_tuple;
    };
};

typedef union blogHash_t BlogHash_t;


/* flow priority - used for packet buffer reseravtion */
typedef enum  {
	BLOG_DECL(flow_prio_normal)
	BLOG_DECL(flow_prio_high)
	BLOG_DECL(flow_prio_exclusive)
} BlogFlowPrio_t;


/* TBD : Rearrange following bit positions for optimization. */
union blogWfd_t {
    uint32_t    u32;
    struct {
        BE_DECL(
           uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
           uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
           uint32_t            is_wfd               : 1;/* is_wfd=1 */
           uint32_t            is_chain             : 1;/* is_chain=1 */
           uint32_t            reserved1            : 4;/* unused */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            reserved0            : 1;/* unused */
           uint32_t            priority             : 4;/* Tx Priority */
           uint32_t            chain_idx            : 16;/* Tx chain index */
        )
        LE_DECL(
           uint32_t            chain_idx            : 16;/* Tx chain index */
           uint32_t            priority             : 4;/* Tx Priority */
           uint32_t            reserved0            : 1;/* unused */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            reserved1            : 4;/* unused */
           uint32_t            is_chain             : 1;/* is_chain=1 */
           uint32_t            is_wfd               : 1;/* is_wfd=1 */
           uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
           uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
        )
    } nic_ucast;

    struct {
        BE_DECL(
           uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
           uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
           uint32_t            is_wfd               : 1;/* is_wfd=1 */
           uint32_t            is_chain             : 1;/* is_chain=0 */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            ssid                 : 4;/* SSID for WLAN */
           uint32_t            reserved1            : 8;/* unused */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            priority             : 3;/* Tx Priority */
           uint32_t            flowring_idx         :10;/* Tx flowring index */
        )
        LE_DECL(
           uint32_t            flowring_idx         :10;/* Tx flowring index */
           uint32_t            priority             : 3;/* Tx Priority */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            reserved1            : 8;/* unused */
           uint32_t            ssid                 : 4;/* SSID for WLAN */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            is_chain             : 1;/* is_chain=0 */
           uint32_t            is_wfd               : 1;/* is_wfd=1 */
           uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
           uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
        )
    } dhd_ucast;

    struct {
        BE_DECL(
           uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
           uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
           uint32_t            is_wfd               : 1;/* is_wfd=1 */
           uint32_t            is_chain             : 1;/* is_chain=0 */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            reserved1            : 2;/* unused */
           uint32_t            ssid                 : 4;/* SSID */
           uint32_t            reserved0            :19;/* unused */
        )
        LE_DECL(
           uint32_t            reserved0            :19;/* unused */
           uint32_t            ssid                 : 4;/* SSID */
           uint32_t            reserved1            : 2;/* unused */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            is_chain             : 1;/* is_chain=0 */
           uint32_t            is_wfd               : 1;/* is_wfd=1 */
           uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
           uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
        )
    } mcast;
};
typedef union blogWfd_t BlogWfd_t;

struct blogRnr_t {
    BE_DECL(
       uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
       uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
       uint32_t            is_wfd               : 1;/* rnr (is_wfd=0) */
       uint32_t            radio_idx            : 2;/* Radio index */
       uint32_t            llcsnap_flag         : 1;/* llcsnap_flag */
       uint32_t            priority             : 3;/* Tx Priority */
       uint32_t            ssid                 : 4;/* SSID */
       uint32_t            reserved1            : 7;/* unused */
       uint32_t            flow_prio            : 2;/* flow priority (normal,high, exclusive) - used for packet buffer reservation */
       uint32_t            flowring_idx         :10;/* Tx flowring index */
       )
    LE_DECL(
       uint32_t            flowring_idx         :10;/* Tx flowring index */
       uint32_t            flow_prio            : 2;/* flow priority (normal,high, exclusive) - used for packet buffer reservation */
       uint32_t            reserved1            : 7;/* unused */
       uint32_t            ssid                 : 4;/* SSID */
       uint32_t            priority             : 3;/* Tx Priority */
       uint32_t            llcsnap_flag         : 1;/* llcsnap_flag */
       uint32_t            radio_idx            : 2;/* Radio index */
       uint32_t            is_wfd               : 1;/* rnr (is_wfd=0) */
       uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
       uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
       )
};

typedef struct blogRnr_t BlogRnr_t;

#define MAX_NUM_VLAN_TAG        2

/* Blog ingress priority derived from IQOS */
typedef enum  {
    BLOG_DECL(BLOG_IQ_PRIO_LOW)
    BLOG_DECL(BLOG_IQ_PRIO_HIGH)
} BlogIqPrio_t;

/*
 *------------------------------------------------------------------------------
 * Buffer log structure.
 * ARM 32: 704 bytes
 * ARM 64: 896 bytes
 * MIPS  : 704 bytes
 * Marked the cacheline boundaries in the below structure. 
 * Be cautious when adding new members to this structure.
 *------------------------------------------------------------------------------
 */
struct blog_t {

    union {
        void            * void_p;
        struct blog_t   * blog_p;       /* Free list of Blog_t */
        struct sk_buff  * skb_p;        /* Associated sk_buff */
    };
    BlogHash_t          key;            /* Coarse hash search key */
    uint32_t            hash;           /* hash */
    union {
        uint32_t        wl;
        struct {
            BE_DECL(
               uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
               uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
               uint32_t            reserved             : 30;
            )
            LE_DECL(
               uint32_t            reserved             : 30;
               uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
               uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
            )
        } wl_hw_support;
        BlogWfd_t       wfd;
        BlogRnr_t       rnr;
    };
    uint32_t            fc_context;     
    struct blog_t       * vblog_p;      /* vertical list of Blog_t */
    void                * mc_fdb;       /* physical rx network device */

    void                *map_p;
    /* --- [ARM32]32 byte cacheline boundary --- */
    BlogEthAddr_t       src_mac;        /* Flow src MAC */
    BlogEthAddr_t       dst_mac;        /* Flow dst MAC */
    /* --- [ARM64]64 byte cacheline boundary --- */

    void                * fdb[2];       /* fdb_src and fdb_dst */
    int8_t              delta[MAX_VIRT_DEV];  /* octet delta info */
    int8_t              tx_dev_delta; /* octet delta of TX dev */
    uint8_t             l2_dirty_offset;
    uint8_t             vtag_num:    4;
    uint8_t             vtag_tx_num: 4;
    uint16_t            eth_type;
    /* --- [ARM32]32 byte cacheline boundary --- */

    union {
        uint32_t        flags;
        struct {
        BE_DECL(
            uint32_t    unused:      2;
            uint32_t    is_routed:   1;
            uint32_t    fc_hybrid:   1;  /* hybrid flow accelarate in HW and SW */
            uint32_t    l2_mode:     1;
            uint32_t    is_ssm:      1;
            uint32_t    mcast_learn: 1;

            uint32_t    l2_pppoe:    1; /* L2 packet is PPPoE */
            uint32_t    l2_ipv6:     1; /* L2 packet is IPv6  */
            uint32_t    l2_ipv4:     1; /* L2 packet is IPv4  */
            uint32_t    is_mapt_us:  1; /* MAP-T Upstream flow */   
            uint32_t    is_df:       1; /* IPv4 DF flag set */
            uint32_t    ptm_us_bond: 1; /* PTM US Bonding Mode */
            uint32_t    lag_port:    2; /* LAG port when trunking is done by internal switch/runner */

            uint32_t    tos_mode_us: 1; /* ToS mode for US: fixed, inherit */
            uint32_t    tos_mode_ds: 1; /* ToS mode for DS: fixed, inherit */
            uint32_t    has_pppoe:   1;
            uint32_t    ack_done:    1; /* TCP ACK prio decision made */
            uint32_t    ack_cnt:     4; /* back to back TCP ACKs for prio */

            uint32_t    nf_dir_pld:  1;
            uint32_t    nf_dir_del:  1;
            uint32_t    nf_ct_skip_ref_dec:  1; /* when set don't decrement ct refcnt */
            uint32_t    pop_pppoa:   1;
            uint32_t    insert_eth:  1;
            uint32_t    iq_prio:     1;
            uint32_t    mc_sync:     1;
            uint32_t    rtp_seq_chk: 1; /* RTP sequence check enable       */
            uint32_t    incomplete:  1;
        )
        LE_DECL(
            uint32_t    incomplete:  1;
            uint32_t    rtp_seq_chk: 1; /* RTP sequence check enable       */
            uint32_t    mc_sync:     1;
            uint32_t    iq_prio:     1;
            uint32_t    insert_eth:  1;
            uint32_t    pop_pppoa:   1;
            uint32_t    nf_ct_skip_ref_dec:  1; /* when set don't decrement ct refcnt */
            uint32_t    nf_dir_del:  1;
            uint32_t    nf_dir_pld:  1;

            uint32_t    ack_cnt:     4; /* back to back TCP ACKs for prio */
            uint32_t    ack_done:    1; /* TCP ACK prio decision made */
            uint32_t    has_pppoe:   1;
            uint32_t    tos_mode_ds: 1; /* ToS mode for DS: fixed, inherit */
            uint32_t    tos_mode_us: 1; /* ToS mode for US: fixed, inherit */

            uint32_t    lag_port:    2; /* LAG port when trunking is done by internal switch/runner */
            uint32_t    ptm_us_bond: 1; /* PTM US Bonding Mode */
            uint32_t    is_df:       1; /* IPv4 DF flag set */
            uint32_t    is_mapt_us:  1; /* MAP-T Upstream flow */   
            uint32_t    l2_ipv4:     1; /* L2 packet is IPv4  */
            uint32_t    l2_ipv6:     1; /* L2 packet is IPv6  */
            uint32_t    l2_pppoe:    1; /* L2 packet is PPPoE */

            uint32_t    mcast_learn: 1;
            uint32_t    is_ssm:      1;
            uint32_t    l2_mode:     1;
            uint32_t    fc_hybrid:   1;  /* hybrid flow accelarate in HW and SW */
            uint32_t    is_routed:   1;
            uint32_t    unused:      2;
        )
        };
    };
    union {
        /* only the lower 32 bit in mark is used in 64 bit system
         * but we declare it as unsigned long for the ease of blog
         * to handle it in different architecture, since it part
         * of union with a dst_entry pointer */
        unsigned long       mark;           /* NF mark value on tx */
        void                *dst_entry;     /* skb dst_entry for local_in */
    };

    union { 
        uint32_t            priority;       /* Tx  priority */
        uint32_t            flowid;         /* used only for local in */
    };

    void                * blogRule_p;   /* List of Blog Rules */

    union {
        struct {
            uint32_t dosAttack : 16;
            uint32_t lenPrior  :  1;
            uint32_t vlanPrior :  1;
            uint32_t dscpMangl :  1;
            uint32_t tosMangl  :  1;
            uint32_t preMod    :  1;
            uint32_t postMod   :  1;
            uint32_t dscp2pbit :  1;
            uint32_t dscp2q    :  1;
            uint32_t reserved  :  8;
        };
        uint32_t feature;       /* Feature set for per-packet modification */
    };
    union {
        struct {
            uint8_t vlanout_offset; /* Outer VLAN header offset */
            uint8_t vlanin_offset;  /* Inner VLAN header offset */
            uint8_t vpass_tx_offset; /* Passthru VLAN headers tx offset */
            uint8_t vpass_len;      /* Passthru VLAN headers length */
            uint8_t pppoe_offset;   /* PPPoE header offset */
            uint8_t ip_offset;      /* IPv4 header offset */
            uint8_t ip6_offset;     /* IPv6 header offset */
            uint8_t l4_offset;      /* Layer 4 header offset */
            uint8_t isWan;          /* Receiving by WAN interface */
            uint8_t reserved8_3[3];
        };
        uint32_t offsets[3];
    };
    /* --- [ARM32][ARM64] cacheline boundary --- */
    int (*preHook)(Blog_t *blog_p, void *nbuff_p);  /* Pre-modify hook */
    int (*postHook)(Blog_t *blog_p, void *nbuff_p); /* Post-modify hook */
    /* vtag[] stored in network order to improve fcache performance */
    uint32_t            vtag[MAX_NUM_VLAN_TAG];
    /* pointers to the devices which the flow goes thru */
    void                * virt_dev_p[MAX_VIRT_DEV];
    /* --- [ARM32][ARM64]cacheline boundary --- */
    void                *fdb_npe_p[BLOG_FDB_NPE_MAX]; /* FDB NPEs */
    void                *npe_p[BLOG_NPE_MAX]; /* non-FDB NPEs*/
    /* --- [ARM32][ARM64]cacheline boundary --- */

    //BlogTupleV6_t and BlogHeader_t is cacheline_aligned structure, be 
    //be cautious when adding new variables around these memeber as it 
    //will create unwanted holes in the structure.
    BlogTupleV6_t       tupleV6;        /* L3+L4 IP Tuple log */
    BlogTupleV6_t       del_tupleV6;    /* Del GRE L3+L4 IPV6 Tuple log */

    BlogHeader_t        tx;             /* Transmit path headers */
    BlogHeader_t        rx;             /* Receive path headers */
    /* --- [ARM32][ARM64]cacheline boundary --- */

    void                *rx_dev_p;      /* RX physical network device */
    void                *tx_dev_p;      /* TX physical network device */

    unsigned long       dev_xmit;
    int (*dev_xmit_blog)(void * nbuff_p,const Blog_t * const blog_p); /* Xmit with blog */

    /* Flow connection/session tracker */
    void                *ct_p[BLOG_CT_MAX];
    uint32_t            ct_ver[BLOG_CT_VER_MAX];
    /* --- [ARM32]32 byte cacheline boundary --- */
    void                *rx_tunl_p;
    /* --- [ARM64]64 byte cacheline boundary --- */
    void                *tx_tunl_p;
    BlogActivateKey_t   activate_key;
    uint8_t            tx_l4_offset; /*offset to inner most L4 header*/
    uint8_t            tx_l3_offset; /*offset to inner most L3 header*/
    uint16_t            mcast_port_map;
    uint16_t            mcast_excl_udp_port;

    uint16_t            minMtu;
    uint8_t             dpi_queue;
    uint8_t             tuple_offset; /* offset of flow tuple header */
    uint8_t             hw_pathstat_idx;    /* HWACC Pathstat index */
    uint8_t             host_mac_hashix;
    uint8_t             spdtst;

    /* To make cache alignment inserting dummy_ptr */
    uint8_t             dummy_data2[3];
    /* --- [ARM32]32 byte cacheline boundary --- */
    BlogTuple_t         *grerx_tuple_p; /* gre proto RX Tuple pointer */
    BlogTuple_t         *gretx_tuple_p; /* gre proto TX Tuple pointer */
    union {
        struct {
            BlogGre_t grerx;
    /* --- [ARM32][ARM64]cacheline boundary --- */
            BlogGre_t gretx;
        };
        struct {
            BlogL2tp_t l2tptx;
        };
    };
    /* --- [ARM32]32 byte cacheline boundary (was 24 bytes ago)--- */
    BlogTuple_t         *esprx_tuple_p; /* ESP proto RX Tuple pointer */
    BlogTuple_t         *esptx_tuple_p; /* ESP proto TX Tuple pointer */
    /* --- [ARM32]32 byte cacheline boundary --- */
    struct {
        BlogEsp_t esprx;
    /* --- [ARM64]64 byte cacheline boundary --- */
        BlogEsp_t esptx;
    };
    /* --- [ARM32]32 byte cacheline boundary --- */
    BlogTuple_t         delrx_tuple;    /* Del proto RX L3+L4 IP Tuple log */
    BlogTuple_t         deltx_tuple;    /* Del proto TX L3+L4 IP Tuple log */
    /* --- [ARM32][ARM64]cacheline boundary --- */
    BlogTuple_t         rx_tuple[1];    /* RX L3+L4 IP Tuple log */
    BlogTuple_t         tx_tuple[1];    /* TX L3+L4 IP Tuple log */
    void                *mega_p;
    
    uint32_t            l2_keymap;
    uint32_t            l3_keymap;
} ____cacheline_aligned;

/*
 * -----------------------------------------------------------------------------
 * Engineering constants: Pre-allocated pool size 400 blogs Ucast+Mcast
 *
 * Extensions done in #blogs carved from a 2x4K page (external fragmentation)
 * Blog size = 240, 8192/240 = 34 extension 32bytes internal fragmentation
 *
 * Number of extensions engineered to permit approximately max # of flows
 * (assuming one blog per flow).
 * -----------------------------------------------------------------------------
 */
#define CC_BLOG_SUPPORT_EXTEND              /* Conditional compile            */
#define BLOG_POOL_SIZE_ENGG         400     /* Pre-allocated pool size        */
/* Number of Blog_t per extension */
#define BLOG_EXTEND_SIZE_ENGG      (8192/sizeof(Blog_t))
/* Maximum extensions allowed including 4K flows             */
#define BLOG_EXTEND_MAX_ENGG       (16384/BLOG_EXTEND_SIZE_ENGG)



extern const char       * strBlogAction[];
extern const char       * strBlogEncap[];
extern const char       * strRfc2684[];
extern const uint8_t    rfc2684HdrLength[];
extern const uint8_t    rfc2684HdrData[][16];


#else
struct blog_t {void * blogRule_p;};
#define BLOG_LOCK_BH()
#define BLOG_UNLOCK_BH()
#endif /* defined(CONFIG_BLOG) */

/*
 * -----------------------------------------------------------------------------
 * Blog functional interface
 * -----------------------------------------------------------------------------
 */


/*
 * -----------------------------------------------------------------------------
 * Section 1. Extension of a packet context with a logging context
 * -----------------------------------------------------------------------------
 */

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#define blog_ptr(skb_p)         skb_p->blog_p
#else
#define blog_ptr(skb_p)         BLOG_NULL
#endif

/* Allocate or deallocate a Blog_t */
Blog_t * blog_get(void);
void blog_put(Blog_t * blog_p);

/* Allocate a Blog_t and associate with sk_buff or fkbuff */
extern Blog_t * blog_skb(struct sk_buff  * skb_p);
extern Blog_t * blog_fkb(struct fkbuff  * fkb_p);

/* Clear association of Blog_t with sk_buff */
extern Blog_t * blog_snull(struct sk_buff * skb_p);
extern Blog_t * blog_fnull(struct fkbuff  * fkb_p);

/* Clear association of Blog_t with sk_buff and free Blog_t object */
extern void blog_free( struct sk_buff * skb_p, blog_skip_reason_t reason );
/* increment refcount of ct's associated with blog */
extern void blog_ct_get(Blog_t * blog_p);
/* decrement refcount of ct's associated with blog */
extern void blog_ct_put(Blog_t * blog_p);

/* Disable further logging. Dis-associate with skb and free Blog object */
extern void blog_skip(struct sk_buff * skb_p, blog_skip_reason_t reason);

/* Transfer association of a Blog_t object between two sk_buffs. */
extern void blog_xfer(struct sk_buff * skb_p, const struct sk_buff * prev_p);

/* Duplicate a Blog_t object for another skb. */
extern void blog_clone(struct sk_buff * skb_p, const struct blog_t * prev_p);

/* Copy a Blog_t object another blog object. */
extern void blog_copy(struct blog_t * new_p, const struct blog_t * prev_p);

/* get the Ingress QoS Prio from the blog */
extern int blog_iq(const struct sk_buff * skb_p);

/* get the flow cache status */
extern int blog_fc_enabled(void);

/* get the GRE tunnel accelerated status */
extern int blog_gre_tunnel_accelerated(void);

#define BLOG_PTM_US_BONDING_DISABLED                      0
#define BLOG_PTM_US_BONDING_ENABLED                       1

extern void blog_ptm_us_bonding( struct sk_buff *skb_p, int mode );

typedef int (*blog_dhd_flow_update_t)(void*, char*, char*, int);
extern blog_dhd_flow_update_t blog_dhd_flow_update_fn;
extern int blog_is_config_netdev_mac(void *dev_p, unsigned long incl_vmacs);
extern int blog_preemptible_task(void);

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
/* gets an NWE from blog based on the NPE type */
static inline void *_blog_get_nwe(Blog_t *blog_p, uint32_t npe_type)
{
    blog_npe_t *npe_p = (blog_npe_t *)blog_p->npe_p[npe_type];
    return (npe_p ? npe_p->nwe_p : (void *) NULL);
}
#endif

/*
 *------------------------------------------------------------------------------
 *  Section 2. Associating native OS or 3rd-party network constructs
 *------------------------------------------------------------------------------
 */

extern void blog_link(BlogNetEntity_t entity_type, Blog_t * blog_p,
                      void * net_p, uint32_t param1, uint32_t param2);

/*
 *------------------------------------------------------------------------------
 * Section 3. Network construct and Blog client co-existence call backs
 *------------------------------------------------------------------------------
 */

extern unsigned long blog_request(BlogRequest_t event, void * net_p,
                        unsigned long param1, unsigned long param2);

extern int blog_query(BlogQuery_t query, void * net_p,
                        uint32_t param1, uint32_t param2, unsigned long param3);

/* 
 * blog_notify():
 * blog_notify() is a synchrounous notification from an entity to blog/flow_cache
 * and will return once the notification/event has been completed. 
 * This API should be called only when it is known that the event processing time
 * is going to be short (process or interrupt context).
 *
 * Interrupt Context: It is NOT recommended to call blog_notify() from an 
 *   interrupt context (like softirq/timer) because it will block all other 
 *   processes/threads/softirq from running until the notification processing 
 *   has been completed. Instead use blog_notify_async() from an interrupt 
 *   context.
 */
extern void blog_notify(BlogNotify_t event, void *net_p,
                        unsigned long param1, unsigned long param2);

/* 
 * fc_evt task will invoke this callback function asynchronously once the 
 * event processing has been completed. It is upto each entity to decide what 
 * it wants to do within this callback but it should not hold the fc_evt task 
 * for too long. 
 */
typedef	void (*blog_notify_async_cb_fn_t)(void *notify_cb_data_p);

/* 
 * blog_notify_async():
 * It is same as blog_notify() except two new parameters (notify_cb_fn and
 * notify_cb_data_p) have been added, and it is asynchronous call. notify_cb_fn 
 * function will be called with notify_cb_data_p parameter on completion of 
 * the event processing. If an caller does not want to wait for the completion 
 * of the event it can pass NULL values for notify_cb_fn and notify_cb_data_p.
 *
 * blog_notify_async() can be called from process or interrupt context, 
 * it will not block/sleep in this call. If blocking is needed it should be
 * done outside this API.
 *
 * CAUTION: Responsibility of the calling entity:
 * - Serialization and/or locking, reference count of entry
 * - the entry (e.g. flowring, FDB, etc.) for which flows are being flused 
 *   is NOT freed or reallocated before the callback function is invoked.
 *
 * return: Caller MUST check the return value.
 *         1 : caller's notify callback function will be called
 *         0 : caller's notify callback function will NOT be called.
 *             Situations where fc_evt task is not running.
 */
extern int blog_notify_async(BlogNotify_t event, void *net_p,
        unsigned long param1, unsigned long param2,
        blog_notify_async_cb_fn_t notify_cb_fn, void *notify_cb_data_p);

/*
 *------------------------------------------------------------------------------
 * blog_notify_async_wait
 * Calls blog_notify_async() and then waits for completion of event. No callback
 * function needed from the caller, this function uses its own callback function.
 * Note : If called from NOT preempt-safe context, this function will change 
 *        blog_notify_async() to blog_notify(), which means the event is 
 *        processed synchronously.
 *        Caller should not call blog_lock()/blog_unlock() as this fucntion
 *        internally calls blog_lock before calling blog_notify/_async() APIs,
 *        and blog_unlock after calling.
 *------------------------------------------------------------------------------
 */
extern void blog_notify_async_wait(BlogNotify_t event, void *net_p,
        unsigned long param1, unsigned long param2);

/* blog notify event enqueue function type */
typedef	int (*blog_notify_evt_enqueue_hook_t)(blog_notify_evt_type_t evt_type, 
        void *net_p, unsigned long param1, unsigned long param2,
        blog_notify_async_cb_fn_t notify_cb_fn, void *notify_cb_data_p);

void blog_bind_notify_evt_enqueue( 
        blog_notify_evt_enqueue_hook_t blog_notify_evt_enqueue_fn );
/*
 *------------------------------------------------------------------------------
 * Section 4. Network end-point binding of Blog client
 *
 * If rx hook is defined,
 *  blog_sinit(): initialize a fkb from skb, and pass to hook
 *          if packet is consumed, skb is released.
 *          if packet is blogged, the blog is associated with skb.
 *  blog_finit(): pass to hook
 *          if packet is to be blogged, the blog is associated with fkb.
 *
 * If tx hook is defined, invoke tx hook, dis-associate and free Blog_t
 *------------------------------------------------------------------------------
 */
extern BlogAction_t blog_sinit(struct sk_buff *skb_p, void * dev_p,
                             uint32_t encap, uint32_t channel, uint32_t phyHdr);

extern BlogAction_t blog_finit(struct fkbuff *fkb_p, void * dev_p,
                             uint32_t encap, uint32_t channel, uint32_t phyHdr);

extern BlogAction_t blog_finit_args(struct fkbuff *fkb_p, void * dev_p,
                             uint32_t encap, uint32_t channel, uint32_t phyHdr,
                             BlogFcArgs_t *fc_args);

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
extern BlogAction_t _blog_emit(void * nbuff_p, void * dev_p,
                             uint32_t encap, uint32_t channel, uint32_t phyHdr);

static inline BlogAction_t blog_emit(void * nbuff_p, void * dev_p,
                        uint32_t encap, uint32_t channel, uint32_t phyHdr)
{
    if ( nbuff_p == NULL ) return PKT_NORM;
    if ( !IS_SKBUFF_PTR(nbuff_p) ) return PKT_NORM;
    // OK, this is something worth looking at, call real function
    return ( _blog_emit(nbuff_p, dev_p, encap, channel, phyHdr) );
}
#else
BlogAction_t blog_emit( void * nbuff_p, void * dev_p,
                        uint32_t encap, uint32_t channel, uint32_t phyHdr );
#endif

/*
 * blog_iq_prio determines the Ingress QoS priority of the packet
 */
extern int blog_iq_prio(struct sk_buff * skb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr);
/*
 *------------------------------------------------------------------------------
 *  blog_activate(): static configuration function of blog application
 *             pass a filled blog to the hook for configuration
 *------------------------------------------------------------------------------
 */
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
extern BlogActivateKey_t *blog_activate( Blog_t * blog_p, BlogTraffic_t traffic,
                               BlogClient_t client );
#else
extern uint32_t blog_activate( Blog_t * blog_p, BlogTraffic_t traffic,
                               BlogClient_t client );
#endif

/*
 *------------------------------------------------------------------------------
 *  blog_deactivate(): static deconfiguration function of blog application
 *------------------------------------------------------------------------------
 */
extern Blog_t * blog_deactivate( BlogActivateKey_t key, BlogTraffic_t traffic,
                                 BlogClient_t client );

/*
 * -----------------------------------------------------------------------------
 * User defined filter invoked invoked in the rx hook. A user may override the
 * Blog action defined by the client. To enable the invocation of this API
 * in blog_finit, ensure that CC_BLOG_SUPPORT_USER_FILTER is enabled. Also, a
 * network device driver may directly invoke blog_filter() to override PKT_BLOG
 * and return PKT_NORM (by releasing the associated Blog_t).
 * -----------------------------------------------------------------------------
 */
extern BlogAction_t blog_filter(Blog_t * blog_p);

/*
 * -----------------------------------------------------------------------------
 * Section 5. Binding Blog client applications:
 *
 * Blog defines three hooks:
 *
 *  RX Hook: If this hook is defined then blog_init() will pass the packet to
 *           the Rx Hook using the FkBuff_t context. L1 and encap information
 *           are passed to the receive hook. The private network device context 
 *           may be extracted using the passed net_device object, if needed.
 *
 *  TX Hook: If this hook is defined then blog_emit() will check to see whether
 *           the NBuff has a Blog_t, and if so pass the NBuff and Blog to the
 *           bound Tx hook.
 *
 *  NotifHook: When blog_notify is invoked, the bound hook is invoked. Based on
 *           event type the bound Blog client may perform a custom action.
 *
 *  SC Hook: If this hook is defined, blog_activate() will pass a blog with
 *           necessary information for statical configuration.
 *
 *  SD Hook: If this hook is defined, blog_deactivate() will pass a pointer
 *           to a network object with BlogActivateKey information. The
 *           respective flow entry will be deleted.
 *
 *  QueryHook: When blog_query is invoked, the bound hook is invoked. Based on
 *           query type the bound Blog client will return result of query.
 * -----------------------------------------------------------------------------
 */
typedef union {
    struct {
        uint16_t         QR_HOOK     : 1;
        uint16_t         RX_HOOK     : 1;
        uint16_t         TX_HOOK     : 1;
        uint16_t         XX_HOOK     : 1;
        uint16_t         SC_HOOK     : 1;
        uint16_t         SD_HOOK     : 1;
        uint16_t         FA_HOOK     : 1;
        uint16_t         FD_HOOK     : 1;
        uint16_t         PA_HOOK     : 1;
        uint16_t         reserved    : 7;
    } bmap;
    uint16_t             hook_info;
} BlogBind_t;

typedef BlogAction_t (* BlogDevRxHook_t)(struct fkbuff *fkb_p, void * dev_p,
                                       BlogFcArgs_t * args);

typedef BlogAction_t (* BlogDevTxHook_t)(struct sk_buff *skb_p, void * dev_p,
                                       uint32_t encap, uint32_t blogHash);

typedef int (* BlogNotifyHook_t)(blog_notify_api_t blog_notify_api, 
        BlogNotify_t notification, 
        void * net_p, unsigned long param1, unsigned long param2,
        blog_notify_async_cb_fn_t notify_cb_fn, void *notify_cb_data_p);

typedef int (* BlogQueryHook_t)(BlogQuery_t query, void * net_p,
                            uint32_t param1, uint32_t param2, unsigned long param3);

typedef BlogActivateKey_t * (* BlogScHook_t)(Blog_t * blog_p, BlogTraffic_t traffic);

typedef Blog_t * (* BlogSdHook_t)(BlogActivateKey_t key, BlogTraffic_t traffic);

typedef void (* BlogFaHook_t)(void *ct_p, BlogFlowEventInfo_t info, BlogFlowEventType_t type);

typedef void (* BlogFdHook_t)(void *ct_p, BlogFlowEventInfo_t info, BlogFlowEventType_t type);

typedef BlogAction_t (* BlogPaHook_t)(struct fkbuff * fkb_p, void * dev_p,
                                      uint32_t encap, uint32_t channel, uint32_t phyHdr);

extern int blog_get_hw_accel(void);
extern void blog_bind(BlogDevRxHook_t rx_hook,    /* Client Rx netdevice handler*/
                      BlogDevTxHook_t tx_hook,    /* Client Tx netdevice handler*/
                      BlogNotifyHook_t xx_hook, /* Client notification handler*/
                      BlogQueryHook_t qr_hook,  /* Client query handler       */
                      BlogBind_t   bind
                     );
                     
extern int hw_accelerator_client_get(void);
extern int sw_accelerator_client_get(void);
                     
extern void blog_bind_config(BlogScHook_t sc_hook,    /* Client static config handler*/
                             BlogSdHook_t sd_hook,    /* Client static deconf handler*/
                             BlogClient_t client,     /* Static configuration Client */
                             BlogBind_t   bind
                            );

void blog_bind_packet_accelerator( BlogPaHook_t blog_pa, BlogBind_t bind );
int blog_flowevent_register_notifier(struct notifier_block *nb);
int blog_flowevent_unregister_notifier(struct notifier_block *nb);

/*
 *------------------------------------------------------------------------------
 * blog notify event
 *------------------------------------------------------------------------------
 */
typedef struct {
    struct dll_t    node;       /* First element implements dll               */
    blog_notify_evt_type_t evt_type;
    void            *net_p;
    unsigned long   param1;
    unsigned long   param2;
    blog_notify_async_cb_fn_t notify_cb_fn;
    void            *notify_cb_data_p;
} ____cacheline_aligned blog_notify_evt_t;

/*
 * -----------------------------------------------------------------------------
 * Section 6. Miscellanous
 * -----------------------------------------------------------------------------
 */

/* Logging of L2|L3 headers */
extern void blog(struct sk_buff * skb_p, BlogDir_t dir, BlogEncap_t encap,  
                 size_t len, void * data_p);

/* Dump a Blog_t object */
extern void blog_dump(Blog_t * blog_p);

/* Get the minimum Tx MTU for a blog */
uint16_t blog_getTxMtu(Blog_t * blog_p);

/*
 * Lock and unlock the blog layer.  This is used to reduce the number of
 * times the blog lock must be acquired and released during bulk rx processing.
 * See also blog_finit_locked.
 */
extern void blog_lock(void);
extern void blog_unlock(void);

/*
  * Per packet basis modification feature
  */
#define BLOG_MAX_FEATURES               8

#define BLOG_LEN_PARAM_INDEX            0
#define BLOG_DSCP_PARAM_INDEX           1
#define BLOG_TOS_PARAM_INDEX            2

#define BLOG_MAX_LEN_TBLSZ              8
#define BLOG_MAX_DSCP_TBLSZ            64
#define BLOG_MAX_TOS_TBLSZ            256

#define BLOG_LEN_PARAM_NUM              4
#define BLOG_MAX_PARAM_NUM              4

#define BLOG_MIN_LEN_INDEX              0
#define BLOG_MAX_LEN_INDEX              1
#define BLOG_ORIGINAL_MARK_INDEX        2
#define BLOG_TARGET_MARK_INDEX          3

#define BLOG_MATCH_DSCP_INDEX           0
#define BLOG_TARGET_DSCP_INDEX          1

#define BLOG_MATCH_TOS_INDEX            0
#define BLOG_TARGET_TOS_INDEX           1

#define BLOG_INVALID_UINT8   ((uint8_t)(-1))
#define BLOG_INVALID_UINT16 ((uint16_t)(-1))
#define BLOG_INVALID_UINT32 ((uint32_t)(-1))

extern int blog_set_ack_tbl(uint32_t val[]);
extern int blog_clr_ack_tbl(void);
extern int blog_set_len_tbl(uint32_t val[]);
extern int blog_clr_len_tbl(void);
extern int blog_set_dscp_tbl(uint8_t idx, uint8_t val);
extern int blog_clr_dscp_tbl(void);
extern int blog_set_tos_tbl(uint8_t idx, uint8_t val);
extern int blog_clr_tos_tbl(void);
extern int blog_pre_mod_hook(Blog_t *blog_p, void *nbuff_p);
extern int blog_post_mod_hook(Blog_t *blog_p, void *nbuff_p);

#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE)
#define BLOG_GRE_RCV_NOT_GRE             2
#define BLOG_GRE_RCV_NO_SEQNO            1
#define BLOG_GRE_RCV_IN_SEQ              0
#define BLOG_GRE_RCV_NO_TUNNEL          -1
#define BLOG_GRE_RCV_FLAGS_MISSMATCH    -2
#define BLOG_GRE_RCV_CHKSUM_ERR         -3
#define BLOG_GRE_RCV_OOS_LT             -4
#define BLOG_GRE_RCV_OOS_GT             -5

extern int blog_gre_rcv( struct fkbuff *fkb_p, void * dev_p, uint32_t h_proto, 
                         void **tunl_pp, uint32_t *pkt_seqno_p);
extern void blog_gre_xmit( struct sk_buff *skb_p, uint32_t h_proto );
#endif

#if defined(CONFIG_ACCEL_PPTP)
#define BLOG_PPTP_ENCRYPTED               3
#define BLOG_PPTP_RCV_NOT_PPTP            2
#define BLOG_PPTP_RCV_NO_SEQNO            1
#define BLOG_PPTP_RCV_IN_SEQ              0
#define BLOG_PPTP_RCV_NO_TUNNEL          -1
#define BLOG_PPTP_RCV_FLAGS_MISSMATCH    -2
#define BLOG_PPTP_RCV_CHKSUM_ERR         -3
#define BLOG_PPTP_RCV_OOS_LT             -4
#define BLOG_PPTP_RCV_OOS_GT             -5
extern int blog_pptp_rcv( struct fkbuff *fkb_p, uint32_t h_proto, 
                          uint32_t *rcv_pktSeq);
extern void blog_pptp_xmit( struct sk_buff *skb_p, uint32_t h_proto );
#endif

#define BLOG_L2TP_RCV_TUNNEL_FOUND       1
#define BLOG_L2TP_RCV_NO_TUNNEL          0

#define BLOG_INCLUDE_VIRTUAL_DEVS 1

void blog_get_dev_stats(void *dev_p, void *bStats_p);
void blog_clr_dev_stats(void *dev_p);
void blog_get_dev_running_stats(void *dev_p, void * const bStats_p);
void blog_get_dev_running_stats_wlan(void *dev_p, void * const bStats_p); /* Remove once WLAN falls in place */
void blog_add_dev_accelerated_stats(void *dev_p, void *stats64_p);

typedef struct {
    wait_queue_head_t   wqh;
    unsigned long       work_avail;
#define BLOG_WORK_AVAIL               (1<<0)
} wq_info_t;

#define BLOG_WAKEUP_WORKER_THREAD(x, mask)                              \
do {                                                                    \
    if ( !((x)->work_avail & mask) ) {                                  \
        (x)->work_avail |= mask;                                        \
        wake_up_interruptible(&((x)->wqh));                             \
    }                                                                   \
} while (0)


#endif /* defined(__BLOG_H_INCLUDED__) */

#endif /* CONFIG_BCM_KF_BLOG */
