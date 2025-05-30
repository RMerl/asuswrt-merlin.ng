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

#define BLOG_ESP_MODE_TUNNEL        0
#define BLOG_ESP_MODE_TRANSPORT     1

/* used to exchange info between fcache and drivers */
typedef struct {
    /* Rx Fields */
    uint32_t h_proto;    /* protocol */
    uint32_t key_match;  /* key */
    uint8_t group_fwd_exception; /* Hw Accelerator mcast forwarding exception */
    uint8_t esp_ivsize;
    uint8_t esp_icvsize;
    uint8_t esp_blksize;
    union {
        uint8_t esp_flags;
        struct
        {
            uint8_t esp_inner_pkt:1;
            uint8_t esp_over_udp:1;
            uint8_t esp_mode:1;
            uint8_t esp_rsv:5;
        };
    };
    uint32_t esp_spi;
    uint32_t fc_ctxt;
    union {
        uint16_t rx_flags;
#define FCARG_HDRM_BITS    9
#define FCARG_HDRM_MAX     ((1<<FCARG_HDRM_BITS)-1)
        struct
        {
            uint16_t is_shared :1;
            uint16_t is_skb :1;
            uint16_t hdrm :FCARG_HDRM_BITS;
            uint16_t rf_reserved :5;
        };
    };

    union {
        uint16_t rx_hwf_flags;
        struct
        {
            uint16_t is_rx_wan :1;
            uint16_t is_rx_ddos_q :1;
            uint16_t is_hwf_lookup_done:1; /*hwf lookup in hwf*/
            uint16_t is_rx_outer_ipv6 :1;
            uint16_t is_rx_inner_ipv6 :1;
            uint16_t is_hwf_data_valid :1;
            uint16_t is_hwf_host_done :1; /*hwf process in host */
            uint16_t hwf_rsvd :9;
        };
    };

	void *rx_inner_tuple;
	void *rx_outer_tuple;
	uint8_t rx_inner_l4proto;
	uint8_t rx_outer_l4proto;

    /* Tx Fields (used by dev_xmit_args) */
    union {
        uint8_t tx_flags;
        struct
        {
            uint8_t reserved:1;
            uint8_t tcp_discard:1;
            uint8_t fro:1;
            uint8_t use_xmit_args:1;
            uint8_t tx_is_ipv4:1;
            uint8_t use_tcplocal_xmit_enq_fn:1;
            uint8_t gdx_tx:1;
            uint8_t egress_tc:1;
        };
    };
    uint8_t  tx_l3_offset;
    uint8_t  tx_l4_offset;
    uint16_t local_rx_devid;
    unsigned long dev_xmit;
    void     *esptx_dst_p;
    union {
        void    *esprx_secPath_p;
        void    *esprx_xfrm_st;
    };
    uint16_t dst_entry_id;
} BlogFcArgs_t;

typedef struct {
    uint64_t rx_packets;
    uint64_t rx_bytes;
    uint64_t rx_mcast_packets;
    uint64_t rx_mcast_bytes;
    int32_t  rx_rtp_packets_lost; /*TODO chekc why this is defined as signed int */
    uint32_t pollTS_ms; // Poll timestamp in ms
}BlogFcStats_t;

typedef struct {
    u64 packet_count;
    u64 byte_count;
} blog_fast_stats_t;

typedef enum xmit_args_field_id {
    XMIT_ARGS_FIELD_MARK,
    XMIT_ARGS_FIELD_PRIO,
    XMIT_ARGS_FIELD_L3_OFFSET,
    XMIT_ARGS_FIELD_L4_OFFSET,
    XMIT_ARGS_FIELD_LOCAL_RXDEVID,
    XMIT_ARGS_FIELD_DST_ID,
    XMIT_ARGS_FIELD_MAX
}enum_xmit_args_t;

typedef struct xmit_arg_def {
   enum_xmit_args_t field;  
   uint16_t size;
   uint16_t pos;
   size_t blog_offset;
   size_t prepinfo_offset;
}xmit_arg_definition_t;

/*
 * Linux Netfilter Conntrack registers it's conntrack refresh function which
 * will be invoked to refresh a conntrack when packets belonging to a flow
 * managed by Linux conntrack are bypassed by a Blog client.
 */
typedef void (*blog_cttime_upd_t)(void * ct_p, BlogCtTime_t *ct_time_p);
extern blog_cttime_upd_t blog_cttime_update_fn;


extern int blog_ct_get_stats(const void *ct, uint32_t blog_key, uint32_t dir,
        BlogFcStats_t *stats);
extern int blog_ct_push_stats(void);

extern struct dst_entry *blog_get_dstentry_by_id(uint16_t dstid);
extern void blog_put_dstentry_id(uint16_t dstid);
extern int blog_get_dstentry_id(struct dst_entry *dst_p, uint16_t *dstid_p);
extern int blog_dstentry_id_tbl_init(void);

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

typedef int (*blog_pptp_xmit_upd_t)(uint16_t call_id, uint32_t *seqNum, 
                                    uint32_t *ackNum, uint32_t daddr);
extern blog_pptp_xmit_upd_t blog_pptp_xmit_update_fn;

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
        BLOG_DECL(BlogClient_mcast)
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

/* FLOWTRACK: param1 is ORIG=0 or REPLY=1 direction */
#define BLOG_PARAM1_DIR_ORIG    0U
#define BLOG_PARAM1_DIR_REPLY   1U
#define BLOG_PARAM1_DIR_MAX     2U

/* BRIDGEFDB: param1 is src|dst */
#define BLOG_PARAM1_SRCFDB      0U
#define BLOG_PARAM1_DSTFDB      1U

/* LLID/GEM index for the mcast data received at WAN side. 0xFE means any value is acceptable */
#define BLOG_CHAN_XPON_MCAST_ANY 0xFE

/* IF_DEVICE: param1 is direction RX or TX, param 2 is tx_max_pktlen */

typedef enum {
        BLOG_DECL(FLOWTRACK)            /* Flow (connection|session) tracker  */
        BLOG_DECL(BRIDGEFDB)            /* Bridge Forwarding Database entity  */
        BLOG_DECL(MCAST_FDB)            /* Multicast Client FDB entity        */
        BLOG_DECL(IF_DEVICE)            /* Virtual Interface (network device) */
        BLOG_DECL(IF_DEVICE_MCAST)      /* Virtual Interface (network device) */
        BLOG_DECL(GRE_TUNL)             /* GRE Tunnel                         */
        BLOG_DECL(TOS_MODE)             /* TOS_MODE                           */
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
        BLOG_DECL(DESTROY_NETDEVICE)    /* Network device going down          */
        BLOG_DECL(FETCH_NETIF_STATS)    /* Fetch accumulated stats            */
        BLOG_DECL(CLEAR_NETIF_STATS)    /* Clear accumulated stats            */
        BLOG_DECL(UPDATE_NETDEVICE)     /* Netdevice has been modified (MTU, etc) */
        BLOG_DECL(ARP_BIND_CHG)         /* ARP IP/MAC binding change event    */
        BLOG_DECL(CONFIG_CHANGE)        /* Certain configuration change event */
        BLOG_DECL(UP_NETDEVICE)         /* network device up                  */
        BLOG_DECL(DN_NETDEVICE)         /* network device down                */
        BLOG_DECL(CHANGE_ADDR)          /* network device change MAC addr     */
        BLOG_DECL(SET_DPI_PARAM)        /* Set the DPI parameters             */
        BLOG_DECL(FLUSH)                /* Flush flows based on parameters    */
        BLOG_DECL(DESTROY_MEGA)         /* Megaflow connection is deleted     */
        BLOG_DECL(FETCH_MEGA_STATS)     /* Fetch megaflow fast stats          */
        BLOG_DECL(CLEAR_MEGA_STATS)     /* Clear megaflow fast stats          */
        BLOG_DECL(UPDATE_FLOWTRACK_IDLE_TIMEOUT)/* update idle timeout		  */
        BLOG_DECL(CREATE_BRIDGEFDB)    /* Bridge FDB is created               */
        BLOG_DECL(BLOG_NOTIFY_MAX)
} BlogNotify_t;

typedef enum {
        BLOG_DECL(QUERY_FLOWTRACK)      /* Session/connection time is queried */
        BLOG_DECL(QUERY_BRIDGEFDB)      /* Bridge FDB time is queried         */
        BLOG_DECL(QUERY_FLOWTRACK_STATS)/* get stats of flows associated with NPE */
        BLOG_DECL(QUERY_GET_HW_ACCEL)
        BLOG_DECL(QUERY_GET_FLOWID)
        BLOG_DECL(QUERY_GET_FLOWINFO)
        BLOG_DECL(QUERY_GET_FLOWSTATS)
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
    uint32_t flush_chan     :1;
    uint32_t flush_unused   :24;
    uint8_t mac[6];
    uint8_t chan_id;
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
        BLOG_DECL(FLOWTRACK_CONFIRMED)  /* Test whether session is confirmed  */
        BLOG_DECL(FLOWTRACK_ALG_HELPER) /* Test whether flow has an ALG       */
        BLOG_DECL(FLOWTRACK_EXCLUDE)    /* Clear flow candidacy by Client     */
        BLOG_DECL(FLOWTRACK_TIME_SET)   /* Set time in a flow tracker         */
        BLOG_DECL(FLOWTRACK_IDLE_TIMEOUT_GET)  /* get idle timeout in a flow tracker  */
        BLOG_DECL(FLOWTRACK_PUT_STATS)  /* Push accumulated stats to conntrack*/
        BLOG_DECL(FLOWTRACK_L4PROTO_GET)/* Get L4 proto in Flowtracker        */
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
        BLOG_DECL(LINK_XMIT_FN_ARGS)    /* Fetch device link transmit function with args */
        BLOG_DECL(BLOG_REQUEST_MAX)
} BlogRequest_t;

/*
 *------------------------------------------------------------------------------
 * Denotes a type of update to an existing Blog flow.
 *------------------------------------------------------------------------------
 */

typedef enum {
        BLOG_DECL(BLOG_UPDATE_DPI_QUEUE)     /* DPI Queue assignment has changed */
        BLOG_DECL(BLOG_UPDATE_BITMAP)        /* Multicast client bitmap has changed */
        BLOG_DECL(BLOG_UPDATE_FWD_AND_TRAP)  /* fwd_and_trap bit has changed */
        BLOG_DECL(BLOG_UPDATE_BITMAP_FWD_AND_TRAP)  /* Mcast client bitmap and 
                                                fwd_and_trap bit have changed */
        BLOG_DECL(BLOG_UPDATE_MAX)
} BlogUpdate_t;

/*
 *------------------------------------------------------------------------------
 * Clean this up.
 *------------------------------------------------------------------------------
 */

#define BLOG_ENCAP_MAX          6       /* Maximum number of L2 encaps        */
#define BLOG_HDRSZ_MAX          38      /* Maximum size of L2 encaps          */

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
        BLOG_DECL(NPT6)                 /* NPT6                               */
        BLOG_DECL(PASS_THRU)            /* pass-through                       */
        BLOG_DECL(DEL_DST_OPTS)         /* Delivery IPv6 Dest options         */
        BLOG_DECL(PLD_DST_OPTS)         /* Payload IPv6 Dest options          */
        BLOG_DECL(LLC_SNAP)             /* LLC_SNAP                           */
        BLOG_DECL(VXLAN)                /* VXLAN Header                       */
        BLOG_DECL(GRE_ETH_IPv4)         /* L2 GRE inner header type IPv4      */
        BLOG_DECL(GRE_ETH_IPv6)         /* L2 GRE inner header type IPv6      */
        BLOG_DECL(ESPoUDP)              /* ESP tunnel over UDP                */
        BLOG_DECL(unused)               /* unused                             */
        BLOG_DECL(PROTO_MAX)
} BlogEncap_t;

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
    BLOG_DECL(blog_skip_reason_unknown_proto)
    BLOG_DECL(blog_skip_reason_unknown_proto_ah4)
    BLOG_DECL(blog_skip_reason_unknown_proto_ah6)
    BLOG_DECL(blog_skip_reason_unknown_proto_esp6)
    BLOG_DECL(blog_skip_reason_esp4_crypto_algo)
    BLOG_DECL(blog_skip_reason_esp4_spu_disabled)
    BLOG_DECL(blog_skip_reason_esp6_crypto_algo)
    BLOG_DECL(blog_skip_reason_esp6_spu_disabled)
    BLOG_DECL(blog_skip_reason_spudd_check_failure)
    BLOG_DECL(blog_skip_reason_dpi)
    BLOG_DECL(blog_skip_reason_sgs)
    BLOG_DECL(blog_skip_reason_bond)
    BLOG_DECL(blog_skip_reason_map_tcp)
    BLOG_DECL(blog_skip_reason_blog)
    BLOG_DECL(blog_skip_reason_l2_local_termination)
    BLOG_DECL(blog_skip_reason_local_tcp_termination)
    BLOG_DECL(blog_skip_reason_blog_xfer)
    BLOG_DECL(blog_skip_reason_skb_morph)
    BLOG_DECL(blog_skip_reason_mega_multi_output_ports)
    BLOG_DECL(blog_skip_reason_mega_attr_mismatch)
    BLOG_DECL(blog_skip_reason_mega_field_mismatch)
    BLOG_DECL(blog_skip_reason_blog_clone)
    BLOG_DECL(blog_skip_reason_known_exception_client)
    BLOG_DECL(blog_skip_reason_max)
} blog_skip_reason_t;

typedef enum {
    BLOG_DECL(blog_free_reason_unknown = 0) /* unknown or customer defined */
    BLOG_DECL(blog_free_reason_blog_emit)
    BLOG_DECL(blog_free_reason_blog_iq_prio)
    BLOG_DECL(blog_free_reason_kfree)
    BLOG_DECL(blog_free_reason_ipmr_local)
    BLOG_DECL(blog_free_reason_known_exception_client)
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
    uint32_t blog_alloc_shared_info;
    uint32_t blog_free_shared_info;
} blog_info_stats_t;

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
 * When the acceleation mode is configured as L23, the accelerators decides
 * on per packet basis whether to use L2 or L3 tuple based acceleration.
 * -----------------------------------------------------------------------------
 */
#define BLOG_ACCEL_MODE_L3             0    /* Legacy. All platforms support*/
#define BLOG_ACCEL_MODE_L23            1    /* Platforms supporting both */

#define CC_BLOG_SUPPORT_ACCEL_MODE     (BLOG_ACCEL_MODE_L3)

extern int blog_support_accel_mode_g;

typedef int (*blog_accel_mode_set_t)(uint32_t accel_mode);
extern blog_accel_mode_set_t blog_accel_mode_set_fn;

extern void blog_support_accel_mode(int accel_mode);
extern int blog_support_get_accel_mode(void);

/* Support for TCP ACK multi flows */
extern int blog_support_tcp_ack_mflows_g;

typedef int (*blog_tcp_ack_mflows_set_t)(int enable);
extern blog_tcp_ack_mflows_set_t blog_tcp_ack_mflows_set_fn;

extern void blog_support_set_tcp_ack_mflows(int enable);
extern int blog_support_get_tcp_ack_mflows(void);

/* Support for ToS multi flows */
extern int blog_support_tos_mflows_g;
extern int blog_support_spdtest_retry_tos_g;

typedef int (*blog_tos_mflows_set_t)(int enable);
extern blog_tos_mflows_set_t blog_tos_mflows_set_fn;

extern void blog_support_set_tos_mflows(int enable);
extern void blog_support_spdtest_retry_tos(int enable);

extern int blog_support_get_tos_mflows(void);

/* Support for unknown ucast flows */
extern int blog_support_unknown_ucast_g;
#if defined(CONFIG_BCM_UNKNOWN_UCAST)
typedef int (*blog_unknown_ucast_set_t)(int enable);
extern blog_unknown_ucast_set_t blog_unknown_ucast_set_fn;

extern void blog_support_set_unknown_ucast(int enable);
extern int blog_support_get_unknown_ucast(void);
#endif

/* Support for Pure LLC flows */
extern int blog_support_pure_llc_g;
typedef int (*blog_pure_llc_set_t)(int enable);
extern blog_pure_llc_set_t blog_pure_llc_set_fn;

extern void blog_support_set_pure_llc(int enable);
extern int blog_support_get_pure_llc(void);

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

#define BCM_WLAN_PER_CLIENT_FLOW_LEARNING 1

/* 
 * Support for blog shared info feature used for completing the 
 * learning of multicast/unknown_ucast flows.
 */ 
#define BCM_WLAN_MCAST_BLOG_SHARED_INFO 1

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
#define BLOG_L2TP_ENABLE              1

#ifdef CONFIG_BLOG_L2TP
#define CC_BLOG_SUPPORT_L2TP       BLOG_L2TP_ENABLE
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

/* blog_ct_max_g: defines the max #of CTs that can be associated with a flow */
extern int blog_ct_max_g;

/* blog_nxe_max_g: defines the max #of NXEs that can be associated with a flow */
extern int blog_nxe_max_g;

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

            uint32_t resvd  : 23;
            uint32_t client_type:  1;
            uint32_t client :  8;
        )
        LE_DECL(
            uint32_t client :  8;
            uint32_t client_type:  1;
            uint32_t resvd  : 23;
        )
    };
} BlogKeyMc_t;

#define BLOG_FDB_KEY_INVALID        BLOG_KEY_NONE
#define BLOG_KEY_FC_INVALID         BLOG_KEY_NONE

#define BLOG_KEY_INVALID            BLOG_KEY_NONE
#define BLOG_KEY_MCAST_INVALID      BLOG_KEY_INVALID

/* mcast client type: TX device only, TX device & wlinfo */
#define BLOG_MCAST_CLIENT_TYPE_TXDEV         0
#define BLOG_MCAST_CLIENT_TYPE_TXDEV_WLINFO   1

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

    uint8_t         type;           /* npe Entity type                        */
    union {
        void        *nwe_p;
        void        *ct_p;
        void        *mega_p;
    };
    Dll_t           flow_list[BLOG_PARAM1_DIR_MAX];   /* flow lists       */
    uint32_t        flow_count[BLOG_PARAM1_DIR_MAX];  /* # of flows       */
} ____cacheline_aligned;
typedef struct blog_npe blog_npe_t;

/* status of blog_nxe_t entry:
 * BLOG_NXE_STS_CT: Initial status is invalid/unused
 * BLOG_NXE_STS_CT/MAP/MEGA: after blog_link()
 * BLOG_NXE_STS_NPE: after flow learning
 */
#define BLOG_NXE_STS_INV     0
#define BLOG_NXE_STS_CT      1
#define BLOG_NXE_STS_MEGA    BLOG_NXE_STS_CT
#define BLOG_NXE_STS_NPE     2

typedef struct {
    uint32_t unused: 29;     
    uint32_t dir   : 1;     
    uint32_t status: 2;     
} blog_nxe_flags_t;

/* union of NPE and NWE entities */
typedef struct {
    union {
        blog_npe_t  *npe_p; /* valid afer flow learning */
        union {
            void    *nwe_p;
            void    *ct_p;
            void    *mega_p;
        }; /* These fields are valid before flow learning */
    };
    blog_nxe_flags_t flags;     
} blog_nxe_t;

/* Max # of nf_conn linked per flow */
/* Change this value to increase/decrease the #of nf_conn */
#define BLOG_CT_MAX             4U

#define BLOG_NXE_MEGA           0U
#define BLOG_NXE_CT             1U
#define BLOG_NXE_MAX            (BLOG_NXE_CT + BLOG_CT_MAX)

#define BLOG_FDB_NPE_SRCFDB     0U
#define BLOG_FDB_NPE_DSTFDB     1U
#define BLOG_FDB_NPE_MAX        2U

/* Limit on Max # of nf_conn and NPE linked per flow */
/* CAUTION: DO NOT change these values */
#define BLOG_CT_MAX_LIMIT       8U
#define BLOG_NXE_MAX_LIMIT      (BLOG_NXE_CT + BLOG_CT_MAX_LIMIT)

#if (BLOG_CT_MAX > BLOG_NXE_MAX)
#error "BLOG_CT_MAX > BLOG_NXE_MAX)"
#endif
#if (BLOG_CT_MAX > BLOG_CT_MAX_LIMIT)
#error "BLOG_CT_MAX > BLOG_CT_MAX_LIMIT)"
#endif

#define BLOG_NPE_MAX         (BLOG_NXE_MAX)
#define BLOG_NPE_NULL        ((blog_npe_t*)NULL)
#define BLOG_NXE_CT_IDX(idx) (BLOG_NXE_CT+idx)


/* Is the megaflow valid ? */
#define IS_BLOG_MEGA(b)   ((b->nxe[BLOG_NXE_MEGA].flags.status == BLOG_NXE_STS_MEGA) && \
                           (b->nxe[BLOG_NXE_MEGA].mega_p != NULL))

/* Before the flow is learnt, use CT macros */
#define IS_BLOG_CT(b)  ((b)->ct_count)
#define IS_BLOG_CT_IDX(b, idx)  ((b->nxe[BLOG_NXE_CT_IDX(idx)].flags.status == BLOG_NXE_STS_CT) && \
                           (b->nxe[BLOG_NXE_CT_IDX(idx)].ct_p != NULL))

/* After the flow is learnt, use NPE macros instead of CT macros */
#define IS_BLOG_NPE(b, idx)    ((b->nxe[idx].flags.status == BLOG_NXE_STS_NPE) && \
                                (b->nxe[idx].npe_p != NULL))
#define BLOG_NPE_GET(b, idx)   (IS_BLOG_NPE(b, idx) ? b->nxe[idx].npe_p : NULL)

#define IS_BLOG_NPE_CT(b)    ((b)->ct_count)
#define IS_BLOG_NPE_CT_IDX(b, idx) (IS_BLOG_NPE(b, BLOG_NXE_CT_IDX(idx)))
#define IS_BLOG_NPE_MEGA(b)  (IS_BLOG_NPE(b, BLOG_NXE_MEGA))

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
    void *ct_p[BLOG_CT_MAX];
    int ct_count;
    union {
        struct {
            uint32_t is_downstream      :1;
            uint32_t flow_event_type    :2;
            uint32_t is_upstream        :1;
            uint32_t is_tr471_flow      :1;
            uint32_t spu_offload_us     :1;
            uint32_t spu_offload_ds     :1;
            uint32_t rx_channel         :8;
            uint32_t tx_channel         :8;
            uint32_t reserved           :1;
            uint32_t skb_mark_flow_id   :8;
        };
        uint32_t u32;
    };
} BlogFlowEventInfo_t;


/*
 * =============================================================================
 * CAUTION: OS and network stack may be built without CONFIG_BLOG defined.
 * =============================================================================
 */

#if defined(CONFIG_BLOG)

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

/* To enable debug packet filtering, see blog_dbg_pkt_filter(), invoked in blog_finit() */
//#define CC_BLOG_SUPPORT_DBG_PKT_FILTER


/*
 * -----------------------------------------------------------------------------
 *                      Section: Definition of a Blog_t
 * -----------------------------------------------------------------------------
 */

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

    uint8_t         channel;        /* e.g. port number, txchannel, ... */

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
                uint32_t         ESPoUDP     : 1;
                uint32_t         GRE_ETH_IPv6: 1;    /* GRE inner header type IPv6 */
                uint32_t         GRE_ETH_IPv4: 1;    /* GRE inner header type IPv4 */
                uint32_t         VXLAN       : 1;
                uint32_t         LLC_SNAP    : 1;
                uint32_t         PLD_DST_OPTS: 1; /* Payload IPv6 Ext Hdr Dest Options */
                uint32_t         DEL_DST_OPTS: 1; /* Delivery IPv6 Ext Hdr Dest Options */
                uint32_t         PASS_THRU   : 1;

                uint32_t         NPT6        : 1;
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
                uint32_t         NPT6        : 1;

                uint32_t         PASS_THRU   : 1;
                uint32_t         DEL_DST_OPTS: 1; /* Delivery IPv6 Ext Hdr Dest Options */
                uint32_t         PLD_DST_OPTS: 1; /* Payload IPv6 Ext Hdr Dest Options */
                uint32_t         LLC_SNAP    : 1;
                uint32_t         VXLAN       : 1;
                uint32_t         GRE_ETH_IPv4: 1;    /* GRE inner header type IPv4 */
                uint32_t         GRE_ETH_IPv6: 1;    /* GRE inner header type IPv6 */                
                uint32_t         ESPoUDP     : 1;
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

#define BLOG_RX_L2(b)               (blog_p->rx.info.bmap.PLD_L2 && !b->rx.unknown_ucast)
#define BLOG_RX_MCAST(b)            (b->rx.multicast)
#define BLOG_RX_UCAST(b)            (!b->rx.multicast && !b->rx.unknown_ucast)
#define BLOG_RX_UNKNOWN_UCAST(b)    (!b->rx.multicast && b->rx.unknown_ucast)
#define BLOG_IS_MASTER(b)           ((BLOG_RX_UNKNOWN_UCAST(b) || b->rx.multicast) && b->client_id == BLOG_MCAST_MASTER_CLIENT_ID)
#define BLOG_RX_MCAST_MASTER(b)     (b->rx.multicast && b->client_id == BLOG_MCAST_MASTER_CLIENT_ID)

#define GRE_MAX_HDR_LEN  (sizeof(struct ipv6hdr) + sizeof(BLOG_HDRSZ_MAX) + BLOG_GRE_HDR_LEN)

#define HDRS_IPinIP     ((1<<GRE) | (1<<PLD_IPv4) | (1<<PLD_IPv6) | (1<<PLD_L2) | \
                         (1<<HDR0_IPv4) | (1<<HDR0_IPv6) | (1<<HDR0_L2) |  \
                         (1<<DEL_IPv4) | (1<<DEL_IPv6) | (1<<DEL_L2) | (1<<VXLAN))

#define HDRS_EIPinIP    (HDRS_IPinIP | (1<<GREoESP) | (3<<GREoESP_type) | (1<<ESP) | (1<<ESPoUDP)) 

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
#define HDRS_EIP4in6    ((1<<PLD_IPv4) | (1<<DEL_IPv6) | (1<<ESP))
#define HDRS_2in2       ((1<<PLD_L2) | (1<<DEL_L2))

#define RX_IP4in6(b)    (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_IP4in6)
#define RX_IP6in4(b)    (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_IP6in4)
#define TX_IP4in6(b)    (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_IP4in6)
#define TX_IP6in4(b)    (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_IP6in4)
#define RX_L2inL2(b)    (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_2in2)
#define TX_NONE(b)      (((b)->tx.info.hdrs & HDRS_IPinIP)==0)

#define RX_IPV4(b)      ((b)->rx.info.bmap.PLD_IPv4)
#define TX_IPV4(b)      ((b)->tx.info.bmap.PLD_IPv4)
#define RX_IPV6(b)      ((b)->rx.info.bmap.PLD_IPv6)
#define TX_IPV6(b)      ((b)->tx.info.bmap.PLD_IPv6)
#define RX_IPV4_DEL(b)  ((b)->rx.info.bmap.DEL_IPv4)
#define TX_IPV4_DEL(b)  ((b)->tx.info.bmap.DEL_IPv4)
#define RX_IPV6_DEL(b)  ((b)->rx.info.bmap.DEL_IPv6)
#define TX_IPV6_DEL(b)  ((b)->tx.info.bmap.DEL_IPv6)
#define PT(b)           ((b)->tx.info.bmap.PASS_THRU)

#define TX_PHY_TYPE(b)  ((b)->tx.info.phyHdrType)
#define RX_PHY_TYPE(b)  ((b)->rx.info.phyHdrType)

#define RX_GRE(b)       ((b)->rx.info.bmap.GRE)
#define TX_GRE(b)       ((b)->tx.info.bmap.GRE)
#define RX_ESP(b)       ((b)->rx.info.bmap.ESP)
#define TX_ESP(b)       ((b)->tx.info.bmap.ESP)
#define RX_ESPoUDP(b)   ((b)->rx.info.bmap.ESPoUDP)
#define TX_ESPoUDP(b)   ((b)->tx.info.bmap.ESPoUDP)
#define ESPoUDP(b)      ((b)->rx.info.bmap.ESPoUDP || (b)->tx.info.bmap.ESPoUDP)
#define PT_ESPoUDP(b)   (RX_ESPoUDP(b) && TX_ESPoUDP(b))
#define IS_ESPoUDP_SPU_DS(b)  ( RX_ESPoUDP(b) && TX_ESPoUDP(b) && (TX_PHY_TYPE(b) == BLOG_SPU_DS) )
#define IS_ESPoUDP_SPU_US(b)  ( RX_ESPoUDP(b) && TX_ESPoUDP(b) && (RX_PHY_TYPE(b) == BLOG_SPU_US) )
#define RX_GRE_ETH(b)   ((b)->rx.info.bmap.GRE_ETH)
#define TX_GRE_ETH(b)   ((b)->tx.info.bmap.GRE_ETH)
#define TX_VXLAN(b)     ((b)->tx.info.bmap.VXLAN)
#define RX_VXLAN(b)     ((b)->rx.info.bmap.VXLAN)
#define VXLAN(b)        (TX_VXLAN(b) || RX_VXLAN(b))

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

/* RX/TX is ESPv4 */
#define RX_E4(b)        (((b)->rx.info.hdrs & HDRS_EIPinIP)==((1 << PLD_IPv4)|(1 << ESP)))
#define TX_E4(b)        (((b)->tx.info.hdrs & HDRS_EIPinIP)==((1 << PLD_IPv4)|(1 << ESP)))

/* RX/TX ESPv4 over DSLite tunnel WAN side */
#define RX_E4in6(b)    (((b)->rx.info.hdrs & HDRS_EIPinIP)==HDRS_EIP4in6)
#define TX_E4in6(b)    (((b)->tx.info.hdrs & HDRS_EIPinIP)==HDRS_EIP4in6)

/* ESPv4 pass-thru over DSLite tunnel */
#define EoT4in6UP(b)  (RX_E4(b) && TX_E4in6(b))
#define EoT4in6DN(b)  (RX_E4in6(b) && TX_E4(b))

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

#define T4in6UP(b)     (TX_IP4in6(b) && (RX_IPV4ONLY(b) || RX_GIP4in4(b) || RX_GIP4in6(b)))
#define T4in6DN(b)     (RX_IP4in6(b) && (TX_IPV4ONLY(b) || TX_GIP4in4(b) || TX_GIP4in6(b)))
#define T4in6DN_NoRxVx(b)   (T4in6DN(b) && !RX_VXLAN(b))

#define T6in4UP(b)      (TX_IP6in4(b) && (RX_IPV6ONLY(b) || RX_GIP6in4(b) || RX_GIP6in6(b)))
#define T6in4DN(b)      (RX_IP6in4(b) && (TX_IPV6ONLY(b) || TX_GIP6in4(b) || TX_GIP6in6(b)))

#define CHK4in6(b)      (T4in6UP(b) || T4in6DN(b))
#define CHK6in4(b)      (T6in4UP(b) || T6in4DN(b)) 
#define CHK4to4(b)      (RX_IPV4ONLY(b) && TX_IPV4ONLY(b))
#define CHK6to6(b)      (RX_IPV6ONLY(b) && TX_IPV6ONLY(b))


#define TG4in4UP(b)     (TX_GIP4in4(b) && (RX_IPV4ONLY(b) || RX_IPV6ONLY(b) || RX_IP4in6(b)))
#define TG4in4DN(b)     (RX_GIP4in4(b) && (TX_IPV4ONLY(b) || TX_IPV6ONLY(b) || TX_IP4in6(b)))
#define TG6in4UP(b)     (TX_GIP6in4(b) && (RX_IPV4ONLY(b) || RX_IPV6ONLY(b) || RX_IP6in4(b)))
#define TG6in4DN(b)     (RX_GIP6in4(b) && (TX_IPV4ONLY(b) || TX_IPV6ONLY(b) || TX_IP6in4(b)))
#define TG2in4UP(b)     (RX_L2ONLY(b) && TX_GIP2in4(b))
#define TG2in4DN(b)     (RX_GIP2in4(b) && TX_L2ONLY(b))

#define TGL2_4in6UP(b)  (RX_L2ONLY(b) && TX_GRE_ETH(b) && TX_GIP2in6(b) && !((b)->tx.info.bmap.GRE_ETH_IPv6) && ((b)->tx.info.bmap.GRE_ETH_IPv4))
#define TGL2_4in6DN(b)  (RX_GRE_ETH(b) && RX_GIP2in6(b) && TX_L2ONLY(b) && !((b)->rx.info.bmap.GRE_ETH_IPv6) && ((b)->rx.info.bmap.GRE_ETH_IPv4))
#define TGL2_6in6UP(b)  (RX_L2ONLY(b) && TX_GRE_ETH(b) && TX_GIP2in6(b) && ((b)->tx.info.bmap.GRE_ETH_IPv6) && !((b)->tx.info.bmap.GRE_ETH_IPv4))
#define TGL2_6in6DN(b)  (RX_GRE_ETH(b) && RX_GIP2in6(b) && TX_L2ONLY(b) && ((b)->rx.info.bmap.GRE_ETH_IPv6) && !((b)->rx.info.bmap.GRE_ETH_IPv4))
#define TGL2_6in4UP(b)  (RX_L2ONLY(b) && TX_GRE_ETH(b) && TX_GIP2in4(b) && ((b)->tx.info.bmap.GRE_ETH_IPv6) && !((b)->tx.info.bmap.GRE_ETH_IPv4))
#define TGL2_6in4DN(b)  (RX_GRE_ETH(b) && RX_GIP2in4(b) && TX_L2ONLY(b) && ((b)->rx.info.bmap.GRE_ETH_IPv6) && !((b)->rx.info.bmap.GRE_ETH_IPv4))

#define TG4in6UP(b)     (TX_GIP4in6(b) && (RX_IPV4ONLY(b) || RX_IPV6ONLY(b) || RX_IP4in6(b)))
#define TG4in6DN(b)     (RX_GIP4in6(b) && (TX_IPV4ONLY(b) || TX_IPV6ONLY(b) || TX_IP4in6(b)))
#define TG6in6UP(b)     (TX_GIP6in6(b) && (RX_IPV4ONLY(b) || RX_IPV6ONLY(b) || RX_IP6in4(b)))
#define TG6in6DN(b)     (RX_GIP6in6(b) && (TX_IPV4ONLY(b) || TX_IPV6ONLY(b) || TX_IP6in4(b)))
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

#define RX_EIPV4ONLY(b)  (((b)->rx.info.hdrs & HDRS_EIPinIP)== HDRS_EIP4)
#define TX_EIPV4ONLY(b)  (((b)->tx.info.hdrs & HDRS_EIPinIP)== HDRS_EIP4)

#define RX_EIP4in4(b)   (((b)->rx.info.hdrs & HDRS_EIPinIP)==HDRS_EIP4in4)
#define TX_EIP4in4(b)   (((b)->tx.info.hdrs & HDRS_EIPinIP)==HDRS_EIP4in4)
#define RX_EIP6in4(b)   (((b)->rx.info.hdrs & HDRS_EIPinIP)==HDRS_EIP6in4)
#define TX_EIP6in4(b)   (((b)->tx.info.hdrs & HDRS_EIPinIP)==HDRS_EIP6in4)

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

#define RX_4oG4oE4(b)   (((b)->rx.info.hdrs & HDRS_EIPinIP)==HDRS_4oG4oE4)
#define TX_4oG4oE4(b)   (((b)->tx.info.hdrs & HDRS_EIPinIP)==HDRS_4oG4oE4)
#define RX_6oG4oE4(b)   (((b)->rx.info.hdrs & HDRS_EIPinIP)==HDRS_6oG4oE4)
#define TX_6oG4oE4(b)   (((b)->tx.info.hdrs & HDRS_EIPinIP)==HDRS_6oG4oE4)
#define RX_2oG4oE4(b)   (((b)->rx.info.hdrs & HDRS_EIPinIP)==HDRS_2oG4oE4)
#define TX_2oG4oE4(b)   (((b)->tx.info.hdrs & HDRS_EIPinIP)==HDRS_2oG4oE4)
#define RX_GoEo2(b)     (((b)->rx.info.hdrs & HDRS_EIPinIP)==HDRS_GE2)
#define TX_GoEo2(b)     (((b)->tx.info.hdrs & HDRS_EIPinIP)==HDRS_GE2)

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

#define CHK_RX_L2TP(b)    (((b)->rx.info.hdrs & ((1 << DEL_IPv4) | (1 << PLD_IPv4))) && RX_L2TP(b))
#define CHK_TX_L2TP(b)    (((b)->rx.info.hdrs & ((1 << DEL_IPv4) | (1 << PLD_IPv4))) && TX_L2TP(b))

#define RX_PPPOE(b)       ((b)->rx.info.bmap.PPPoE_2516)
#define TX_PPPOE(b)       ((b)->tx.info.bmap.PPPoE_2516)
#define PT_PPPOE(b)       (RX_PPPOE(b) && TX_PPPOE(b))

#define MAPT_UP(b)       ((RX_IPV4ONLY(b) || RX_GIP4in4(b)) && TX_IPV6ONLY(b))
#define MAPT_DN(b)       (RX_IPV6ONLY(b) && (TX_IPV4ONLY(b) || TX_GIP4in4(b)))
#define MAPT(b)          (MAPT_DN(b) || MAPT_UP(b))

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
        uint32_t    esp_spi;
    };

    union {
        struct {
            uint16_t    unused2;    /* may be used for extending IPv6 ID */
            uint16_t    ipid;       /* 6in4 Upstream IPv4 identification */
        };
        uint32_t   word2;
    };

    union {
        struct {
            uint8_t     exthdrs:6;  /* Bit field of IPv6 extension headers */
            uint8_t     fragflag:1; /* 6in4 Upstream IPv4 fragmentation flag */
            uint8_t     tunnel:1;   /* Indication of IPv6 tunnel */
            uint8_t     tx_hop_limit;
            uint8_t     unused;
            uint8_t     rx_tos;  /* RX traffic class */
        };
        uint32_t   word3;
    };
    
    union {      
        struct {
            uint8_t nextHdr; uint8_t hdrLen; uint16_t data16;
            uint32_t data32;
        };
        uint64_t ip6_ExtHdr;
    }; 

    ip6_addr_t      addr_npt6;
} ____cacheline_aligned;
typedef struct blogTupleV6_t BlogTupleV6_t;

#define BLOG_GRE_FLAGS_SEQ_ENABLE   0x1000
#define BLOG_GRE_FLAGS_KEY_ENABLE   0x2000

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
            uint16_t offsetBit   : 1;
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

#define BLOG_PPP_ADDR_CTL       0xFF03
#define BLOG_L2TP_PPP_LEN       4   /* used when PPP address and control is 0xFF03 */
#define BLOG_L2TP_PPP_LEN2      2   /* used when PPP address and control is NOT 0xFF03 */
#define BLOG_L2TP_PORT          1701

#define BLOG_PPTP_PPP_LEN       4
#define BLOG_PPTP_NOAC_PPPINFO  0X2145  /* pptp packet without ppp address control field 0xff03 */

#define BLOG_ESP_SPI_LEN         4
#define BLOG_ESP_SEQNUM_LEN      4
#define BLOG_ESP_SEQNUM_HI_LEN   4
#define BLOG_ESP_PADLEN_LEN      1
#define BLOG_ESP_NEXT_PROTO_LEN  1

#define BLOG_ESP_ICV_LEN_64      8
#define BLOG_ESP_ICV_LEN_96      12
#define BLOG_ESP_ICV_LEN_128     16
#define BLOG_ESP_ICV_LEN_192     24
#define BLOG_ESP_ICV_LEN_224     28
#define BLOG_ESP_ICV_LEN_256     32
#define BLOG_ESP_ICV_LEN_384     48
#define BLOG_ESP_ICV_LEN_512     64

#define BLOG_ESP_OVER_UDP_PORT   4500   /* ESP over UDP for NAT-Traversal */

struct blogEsp_t {
    union {
        uint32_t    u32;
        struct {
            BE_DECL(
                uint32_t blksize    :  5;
                uint32_t rsv        :  27;
            )
            LE_DECL(
                uint32_t rsv        :  27;
                uint32_t blksize    :  5;
            )
        };
    };
    union {
        uint16_t    u16;
        struct {
            BE_DECL(
                uint16_t icvsize    :  7;
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
                uint16_t icvsize    :  7;
            )
        };
    };
    uint16_t    ipid;
    void        *dst_p;
    union {
        void    *secPath_p;
        void    *xfrm_st;
    };
};
typedef struct blogEsp_t BlogEsp_t;

#define BLOG_VXLAN_PORT          4789
#define BLOG_VXLAN_TUNNEL_MAX_LEN (BLOG_HDRSZ_MAX + BLOG_IPV6_HDR_LEN + BLOG_UDP_HDR_LEN + BLOG_VXLAN_HDR_LEN)

struct blogVxlan_t {
    uint32_t vni;
    union {
        uint16_t u16;
        struct {
            BE_DECL(
                uint16_t reserved   :  6;
                uint16_t ipv6       :  1;
                uint16_t ipv4       :  1;
                uint16_t length     :  8;
            )
            LE_DECL(
                uint16_t length     :  8;
                uint16_t ipv4       :  1;
                uint16_t ipv6       :  1;
                uint16_t reserved   :  6;              
            )
        };
    };
    uint8_t     l2len;
    uint8_t     outer_tos;
    uint8_t     tunnel_data[BLOG_VXLAN_TUNNEL_MAX_LEN];
};
typedef struct blogVxlan_t BlogVxlan_t;


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
        uint8_t             unknown_ucast:1;     /* device type */
        uint8_t             multicast   :1;     /* multicast flag */
        uint8_t             fkbInSkb    :1;     /* fkb from skb */
        uint8_t             count       :4;     /* # of L2 encapsulations */
    };
    uint8_t             length;         /* L2 header total length */
    uint8_t /*BlogEncap_t*/ encap[ BLOG_ENCAP_MAX ];/* All L2 header types */

    uint8_t             l2hdr[ BLOG_HDRSZ_MAX ];    /* Data of all L2 headers */
    struct {
        uint8_t             unused;
        uint8_t             len_offset;
        union {
            uint16_t        frame_len;
            int16_t         len_delta;
        };
    } llc_snap;
} ____cacheline_aligned;

typedef struct blogHeader_t BlogHeader_t;           /* L2 and L3+4 tuple */

/* Coarse hash key: L1, L3, L4 hash */
union blogHash_t {
    uint32_t        match;
    struct {
        union {
            struct {
                uint8_t  tcp_pure_ack : 1;
                uint8_t  llc_snap     : 1;
                uint8_t  unused       : 6;
            };
            uint8_t ext_match;
        };
        uint8_t     protocol;           /* IP protocol */

        union {
            uint16_t u16;
            struct {
                uint8_t         channel;
                union {
                    struct {
                        uint8_t         phyLen   : 4;
                        uint8_t         phyType  : 4;
                    };
                    uint8_t         phy;
                };
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
           uint32_t            is_wmf_enabled       : 1;/* =0 unused */
           uint32_t            chain_idx            :16;/* Tx chain index */
           uint32_t            is_chain             : 1;/* is_chain=1 */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            priority             : 4;/* Tx Priority */
           uint32_t            llcsnap_flag         : 1;
           uint32_t            reserved0            : 3;/* unused */
        )
        LE_DECL(
           uint32_t            reserved0            : 3;/* unused */
           uint32_t            llcsnap_flag         : 1;
           uint32_t            priority             : 4;/* Tx Priority */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            is_chain             : 1;/* is_chain=1 */
           uint32_t            chain_idx            :16;/* Tx chain index */
           uint32_t            is_wmf_enabled       : 1;/* =0 unused */
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
           uint32_t            is_wmf_enabled       : 1;/* =0 unused */
           uint32_t            flowring_idx         :16;/* Tx flowring index */
           uint32_t            is_chain             : 1;/* is_chain=0 */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            ssid                 : 4;/* SSID for WLAN, keep it the same location as mcast */
           uint32_t            priority             : 3;/* Tx Priority */
           uint32_t            reserved0            : 1;/* unused */
        )
        LE_DECL(
           uint32_t            reserved0            : 1;/* unused */
           uint32_t            priority             : 3;/* Tx Priority */
           uint32_t            ssid                 : 4;/* SSID for WLAN, keep it the same location as mcast */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            is_chain             : 1;/* is_chain=0 */
           uint32_t            flowring_idx         :16;/* Tx flowring index */
           uint32_t            is_wmf_enabled       : 1;/* =0 unused */
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
           uint32_t            is_wmf_enabled       : 1;/* =1 if wmf enabled */
           uint32_t            sta_id               :16;/* uniq wifi identifier, NIC max 2048*/
           uint32_t            is_chain             : 1;/* is_chain=0 */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            ssid                 : 4;/* SSID, keep it the same location as dhd_ucast */
           uint32_t            reserved             : 4;/* unused */
        )
        LE_DECL(
           uint32_t            reserved             : 4;/* unused */
           uint32_t            ssid                 : 4;/* SSID, keep it the same location as dhd_ucast */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            is_chain             : 1;/* is_chain=0 */
           uint32_t            sta_id               :16;/* uniq wifi sta identifier, NIC max 2048*/
           uint32_t            is_wmf_enabled       : 1;/* =1 if wmf enabled */
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
       uint32_t            is_wmf_enabled       : 1;/* =1 if wmf enabled */
       uint32_t            flowring_idx         :16;/* Tx flowring index */
       uint32_t            radio_idx            : 2;/* Radio index */
       uint32_t            llcsnap_flag         : 1;/* llcsnap_flag */
       uint32_t            priority             : 3;/* Tx Priority */
       uint32_t            ssid                 : 4;/* SSID */
       uint32_t            flow_prio            : 2;/* flow priority (normal,high, exclusive) - used for packet buffer reservation */
       )
    LE_DECL(
       uint32_t            flow_prio            : 2;/* flow priority (normal,high, exclusive) - used for packet buffer reservation */
       uint32_t            ssid                 : 4;/* SSID */
       uint32_t            priority             : 3;/* Tx Priority */
       uint32_t            llcsnap_flag         : 1;/* llcsnap_flag */
       uint32_t            radio_idx            : 2;/* Radio index */
       uint32_t            flowring_idx         :16;/* Tx flowring index */
       uint32_t            is_wmf_enabled       : 1;/* =1 if wmf enabled */
       uint32_t            is_wfd               : 1;/* rnr (is_wfd=0) */
       uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
       uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
       )
};

typedef struct blogRnr_t BlogRnr_t;

/* GDX device blog info */
struct blog_gdx {
    union {
        uint32_t u32;
        struct {
            BE_DECL(
               uint32_t            unused       : 12;
               uint32_t            gdx_idx      :  2;/* GDX device index */
               uint32_t            is_gdx_tx    :  1;/* GDX device */
               uint32_t            gdx_prio     :  1;/* 0=high, 1=low */
               uint32_t            gdx_ifid     : 16;/* interface id */
               )
            LE_DECL(
               uint32_t            gdx_ifid     : 16;/* interface id */
               uint32_t            gdx_prio     :  1;/* 0=high, 1=low */
               uint32_t            is_gdx_tx    :  1;/* GDX device */
               uint32_t            gdx_idx      :  2;/* GDX device index */
               uint32_t            unused       : 12;
               )
        };
    };
};

typedef struct blog_gdx blog_gdx_t;


#define MAX_NUM_VLAN_TAG        2

/* Blog ingress priority derived from IQOS */
typedef enum  {
    BLOG_DECL(BLOG_IQ_PRIO_LOW)
    BLOG_DECL(BLOG_IQ_PRIO_HIGH)
} BlogIqPrio_t;

#define BLOG_GET_MASK(b)	    (1ULL<<(b))

#define BLOG_BITS_PER_WORD		32
#if (BLOG_BITS_PER_WORD != 32)
#error "BLOG_BITS_PER_WORD should be 32"
#endif

#define BLOG_UCAST_MASTER_CLIENT_ID  0

#define BLOG_GROUP_MASTER_CLIENT_ID  0
#define BLOG_GROUP_FIRST_CLIENT_ID   1

/* DSL RDP: first few mcast/group client ids reserved for non-WLAN clients (Enet, XTM (DPU case, etc.) */
#define BLOG_GROUP_ENET_CLIENT_RESERVED_IDS   7

#define BLOG_MCAST_CLIENT_BITMAP_MAX_WORDS ((CONFIG_BCM_MAX_MCAST_CLIENTS_PER_GROUP>>5)+1)
#define BLOG_MCAST_CLIENT_BITMAP_SIZE (BLOG_MCAST_CLIENT_BITMAP_MAX_WORDS * BLOG_BITS_PER_WORD)
#define BLOG_MCAST_MASTER_CLIENT_ID  BLOG_GROUP_MASTER_CLIENT_ID
#define BLOG_MCAST_FIRST_CLIENT_ID   BLOG_GROUP_FIRST_CLIENT_ID

#define BLOG_MCAST_ENET_CLIENT_RESERVED_IDS   BLOG_GROUP_ENET_CLIENT_RESERVED_IDS
#define BLOG_MCAST_LAST_CLIENT_ID    (CONFIG_BCM_MAX_MCAST_CLIENTS_PER_GROUP)

#if defined(CONFIG_BCM_UNKNOWN_UCAST)
#define BLOG_UNKNOWN_UCAST_CLIENT_BITMAP_MAX_WORDS ((CONFIG_BCM_MAX_UNKNOWN_UCAST_CLIENTS_PER_GROUP>>5)+1)
#define BLOG_UNKNOWN_UCAST_CLIENT_BITMAP_SIZE (BLOG_UNKNOWN_UCAST_CLIENT_BITMAP_MAX_WORDS * BLOG_BITS_PER_WORD)
#define BLOG_UNKNOWN_UCAST_MASTER_CLIENT_ID  BLOG_GROUP_MASTER_CLIENT_ID
#define BLOG_UNKNOWN_UCAST_FIRST_CLIENT_ID   BLOG_GROUP_FIRST_CLIENT_ID

#define BLOG_UNKNOWN_UCAST_ENET_CLIENT_RESERVED_IDS   BLOG_GROUP_ENET_CLIENT_RESERVED_IDS
#define BLOG_UNKNOWN_UCAST_LAST_CLIENT_ID    (CONFIG_BCM_MAX_UNKNOWN_UCAST_CLIENTS_PER_GROUP)
#endif

#define BLOG_GROUP_DEV_REALLOC_COUNT    16

typedef union {
    uint8_t u8;
    struct {
        uint8_t unused: 6;
        uint8_t bridge_stats_updated: 1; /* bridge device stats have been updated */
        uint8_t bridge_dev: 1; /* virtual device is of type bridge_dev, e.g. bridge */
    };
} blog_virt_dev_flags_t;

typedef struct {
    uint8_t ref_cnt;    /* ref count to this virtual dev */
    uint8_t bridge_info_idx; /* index into bridge_base_stats_tbl[] */
    blog_virt_dev_flags_t flags;
    int8_t  delta;      /* virtual dev delta */   
    int8_t  adjust;     /* MTU adjust */   
    void    *dev_p;     /* pointer to virtual dev */
} blog_virt_dev_info_t;

/*
 *------------------------------------------------------------------------------
 * These are the stats of master flow at the time of first client of bridge dev
 * JOINing the mcast group. These stats will be used as base stats for calculating
 * bridge dev stats for the mcast flow.
 *------------------------------------------------------------------------------
 */
typedef struct {
    void        *dev_p;
    uint16_t    master_dev_idx;
    uint64_t    sw_tx_packets;
    uint64_t    sw_tx_bytes;
    uint64_t    hw_tx_packets;
    uint64_t    hw_tx_bytes;
} blog_master_bridge_base_stats_t;

/* Max 4 different bridge Tx dev in a mcast flow */
#define BLOG_MCAST_MAX_BRIDGE_STATS      4
#define BLOG_MCAST_INVALID_BRIDGE_STATS  BLOG_MCAST_MAX_BRIDGE_STATS

typedef struct {
    uint16_t unused;
    uint16_t max_dev;   /* max number of allocated devices in the table */
    uint16_t num_dev;   /* number of used devices in the table */
    uint16_t last_dev_idx;  /* index of last used device in the table */
    blog_master_bridge_base_stats_t bridge_base_stats_tbl[BLOG_MCAST_MAX_BRIDGE_STATS];
    blog_virt_dev_info_t *virt_dev_info_tbl_p; /* pointer to table/list of virtual devices */   
} group_dev_info_t;

typedef enum group_type {
    group_type_mcast,
    group_type_unknown_ucast,
    group_type_max
} blog_group_type_t;

typedef struct {
    uint8_t is_tcp:1;
    uint8_t is_hw:1;
    uint8_t is_dir_upload:1;
    uint8_t stream_idx:2;
    uint8_t unused:3;
} spdtst_bits_t;

typedef struct {
    uint8_t inner_esp : 1;
    uint8_t offload_us : 1;
    uint8_t offload_ds : 1;
    uint8_t is_esn : 1;
    uint8_t gcm_esn : 1;
    uint8_t unused : 3;
} spu_bits_t;

typedef struct blog_shared_info {
    atomic_t ref_count;     /* # of blogs referencing this shared_info */
    atomic_t clone_count;   /* # of times a blog cloned for replication */
    atomic_t wlan_mcast_intf_count;   /* # of WLAN intf */
    atomic_t wlan_mcast_clone_count;  /* # of times a blog cloned for replication by WLAN */
    atomic_t wlan_mcast_client_count; /* # of times a pkt transmitted to WLAN client */
    atomic_t emit_count; /* # of times a blog_emit() was called by a client */
    atomic_t known_exception_client_count; /* # of times blog was cloned but a 
                   blog_emit() was not called because of a known exception for a client */
    atomic_t client_count;
    atomic_t host_client_count;
    void *master_p;
} blog_shared_info_t;

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
    blog_gdx_t          gdx;
    uint32_t            fc_context;     
    void                * mc_fdb;       /* physical rx network device */
    uint32_t            esp_over_udp_spi;

    /* --- [ARM32]32 byte cacheline boundary --- */
    BlogEthAddr_t       src_mac;        /* Flow src MAC */
    BlogEthAddr_t       dst_mac;        /* Flow dst MAC */
    /* --- [ARM64]64 byte cacheline boundary --- */

    void                * fdb[2];       /* depricated, will be removed */
    uint16_t            fdbid[2];       /* src and dst fdbid */
    uint32_t            ifidx[2];       /* fdb src and fdb dst bridge ifidx */
    int8_t              tx_dev_delta; /* octet delta of TX dev */
    int8_t              tx_dev_adjust; /* MTU adjust */
    uint8_t             l2_dirty_offset;
    uint8_t             outer_vtag_num: 4; /* used for outer header */
    uint8_t             vtag_num: 4; /* used for tuple header */
    uint8_t             hdr_count: 4;
    uint8_t             vtag_tx_num: 4;
    uint8_t             unused2: 4;
    uint8_t             ucast_vtag_num: 4; /* VLAN VTAG NUM used for ucast flow classification */
    uint16_t            eth_type;
    /* --- [ARM32]32 byte cacheline boundary --- */

    union {
        uint32_t        flags;
        struct {
        BE_DECL(
            uint32_t    fwd_and_trap:1;
            uint32_t    group_fwd_exception: 1;
            uint32_t    is_routed:   1;
            uint32_t    fc_hybrid:   1;  /* hybrid flow accelarate in HW and SW */
            uint32_t    l2_mode:     1;
            uint32_t    is_ssm:      1;
            uint32_t    pkt_drop:    1; /* Driver indicates this packet will be dropped */
            uint32_t    host_client_add: 1;

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
        )
        LE_DECL(
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

            uint32_t    host_client_add: 1;
            uint32_t    pkt_drop:    1; /* Driver indicates this packet will be dropped */
            uint32_t    is_ssm:      1;
            uint32_t    l2_mode:     1;
            uint32_t    fc_hybrid:   1;  /* hybrid flow accelarate in HW and SW */
            uint32_t    is_routed:   1;
            uint32_t    group_fwd_exception: 1;
            uint32_t    fwd_and_trap:1;
        )
        };
    };
    union {
        uint32_t        flags2;
        struct {
        BE_DECL(
            uint32_t    unused3:        14;
            uint32_t    rx_ipfrag:       1; /* Rx packet was a IP fragment */
            uint32_t    brcm_tag:        1; /* BRCM Tag */
            uint32_t    esp_over_udp_pt: 1; /* This flag should be set by WCC SCS in Linux path */
                                            /* after idenitfying the ESPoUDP pass-thru packet */
            uint32_t    esp_mode:        1;
            uint32_t    pkt_accel_mode:  1;
            uint32_t    fpi_mode:        2;
            uint32_t    force_l2_hdr_copy: 1; /* Force L2 header copy when preparing flow context */
            uint32_t    tcp_discard:     1;
            uint32_t    fro:             1;
            uint32_t    local_tcp:       1;  /* Locally terminated TCP flows */
            uint32_t    hw_cso:          1;
            uint32_t    is_tr471_flow:   1;
            uint32_t    use_xmit_args:   1;
            uint32_t    hw_accel_force_disable:   1; /* HW acceleration force disabled or not */
            uint32_t    cloned_intf:      1; /* need to increment the interface counter */
            uint32_t    use_tcplocal_xmit_enq_fn:  1;
            uint32_t    group_dev_added:  1;
        )
        LE_DECL(
            uint32_t    group_dev_added:  1;
            uint32_t    use_tcplocal_xmit_enq_fn:  1;
            uint32_t    cloned_intf:      1;
            uint32_t    hw_accel_force_disable:   1; /* HW acceleration force disabled or not */
            uint32_t    use_xmit_args:   1;
            uint32_t    is_tr471_flow:   1;
            uint32_t    hw_cso:          1;
            uint32_t    local_tcp:       1;  /* Locally terminated TCP flows */
            uint32_t    fro:             1;
            uint32_t    tcp_discard:     1;
            uint32_t    force_l2_hdr_copy: 1; /* Force L2 header copy when preparing flow context */
            uint32_t    fpi_mode:        2;
            uint32_t    pkt_accel_mode:  1;
            uint32_t    esp_mode:        1;
            uint32_t    esp_over_udp_pt: 1; /* This flag should be set by WCC SCS in Linux path */
                                            /* after idenitfying the ESPoUDP pass-thru packet */
            uint32_t    brcm_tag:        1; /* BRCM Tag */
            uint32_t    rx_ipfrag:       1; /* Rx packet was a IP fragment */
            uint32_t    unused3:        14;
        )
        };
    };
    union {
        uint32_t        flags3;
        struct {
        BE_DECL(
            uint32_t    unused4:        16;
            uint32_t    xmit_args_mask: 16;
        )
        LE_DECL(
            uint32_t    xmit_args_mask: 16;
            uint32_t    unused4:        16;
        )
        };
    };

    uint32_t       mark;           /* NF mark value on tx */
    union {
        uint32_t            priority;       /* Tx  priority */
        uint32_t            flowid;         /* used only for local in */
    };
    struct {
        uint16_t dst_entry_id;	/* skb dst_entry for local_in */
        uint16_t reserved2;
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
            uint32_t outVtagCk :  1;
            uint32_t spGre     :  1;
            uint32_t reserved  :  6;
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
            int8_t tunl_l2tx_adj;   /* L2 TX adjustment when dealing with tunnel */
            uint8_t reserved8_3[2];
        };
        uint32_t offsets[3];
    };
    /* --- [ARM32][ARM64] cacheline boundary --- */
    int (*preHook)(Blog_t *blog_p, void *nbuff_p);  /* Pre-modify hook */
    int (*postHook)(Blog_t *blog_p, void *nbuff_p); /* Post-modify hook */
    /* vtag[] stored in network order to improve fcache performance */
    uint32_t            vtag[MAX_NUM_VLAN_TAG];
    uint32_t            outer_vtag[MAX_NUM_VLAN_TAG];
    uint32_t            ucast_vtag; /* VLAN VTAG used for ucast flow classification */
    /* pointers to the devices which the flow goes thru */
    blog_virt_dev_info_t virt_dev_info[MAX_VIRT_DEV];

    /* --- [ARM32][ARM64]cacheline boundary --- */
    void                *fdb_npe_p[BLOG_FDB_NPE_MAX]; /* FDB NPEs */
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
    uint16_t            local_rx_devid; /* RX device id for terminated traffic */

    /* This function dev_xmit_args takes precedence over dev_xmit. If this
       function pointer is set, fcache will call dev_xmit_args() */
    void                *dev_xmit_args;
    unsigned long       dev_xmit;
 
    /* Flow connection/session tracker */
    blog_nxe_t          nxe[BLOG_NXE_MAX];
    uint8_t             ct_count;
    /* --- [ARM32]32 byte cacheline boundary --- */
    void                *rx_tunl_p;
    /* --- [ARM64]64 byte cacheline boundary --- */
    void                *tx_tunl_p;
    BlogActivateKey_t   activate_key;
    uint8_t            tx_l4_offset; /*offset to inner most L4 header*/
    uint8_t            tx_l3_offset; /*offset to inner most L3 header*/
    uint16_t            mcast_excl_udp_port;
    /* max Rx packet length that is accelerated (does not include CRC) */
    uint16_t            rx_max_pktlen;
    int16_t             mtu_adj;    /* can be a -ve value */
    uint16_t            tx_max_pktlen;
    uint16_t            rx_max_pktlen_old; /* using pkt mod */
    uint16_t            rx_max_pktlen_new; /* using pkt length delta */
    uint16_t            rx_max_pktlen_fcs;
    uint8_t             dpi_queue;
    uint8_t             tuple_offset; /* offset of flow tuple header */
    uint8_t             hw_pathstat_idx;    /* HWACC Pathstat index */
    uint8_t             host_mac_hashix;
    union {
        uint8_t             spdtst;
        spdtst_bits_t       spdtst_bits;
    };

    union {
        uint8_t         spu_bits;
        spu_bits_t      spu;
    };

    union {
        uint16_t        wlinfo;
        uint16_t        wlsta_id;
    };

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
        BlogVxlan_t     vxlan;
    };
    /* --- [ARM32]32 byte cacheline boundary (was 24 bytes ago)--- */
    union {
        BlogTuple_t         *esprx_tuple_p; /* ESP proto RX Tuple pointer */
        BlogTupleV6_t       *esp6rx_tuple_p;
    };
    union {
        BlogTuple_t         *esptx_tuple_p; /* ESP proto TX Tuple pointer */
        BlogTupleV6_t       *esp6tx_tuple_p;
    };
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
    void                *br_dev_p;
    atomic_t            ref_count;
    
    uint32_t            l2_keymap;
    uint32_t            l3_keymap;

    uint8_t         client_type;
    uint32_t        client_id;
    int16_t         group_bitmap_idx; /* index into group bitmap pool */
    blog_group_type_t   group_type;
    group_dev_info_t   *group_dev_info_p; /* master group dev info */
    blog_shared_info_t *shared_info_p;
    uint32_t        join_id;

    uint32_t        spdt_so_mark;
    uint32_t        svtag; /* BRCM SVTag header */
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

/* There is one additional mcast flow entry for each mcast group because of a master flow */
#define BLOG_CONFIG_MAX_MCAST_FLOWS (CONFIG_BCM_MAX_MCAST_GROUPS + CONFIG_BCM_MAX_MCAST_CLIENTS)
#if defined(CONFIG_BCM_UNKNOWN_UCAST)
#define BLOG_CONFIG_MAX_UNKNOWN_UCAST_FLOWS (CONFIG_BCM_MAX_UNKNOWN_UCAST_GROUPS + CONFIG_BCM_MAX_UNKNOWN_UCAST_CLIENTS)
#define BLOG_CONFIG_MAX_FLOWS       (CONFIG_BCM_MAX_UCAST_FLOWS + BLOG_CONFIG_MAX_MCAST_FLOWS + BLOG_CONFIG_MAX_UNKNOWN_UCAST_FLOWS)
#else
#define BLOG_CONFIG_MAX_FLOWS       (CONFIG_BCM_MAX_UCAST_FLOWS + BLOG_CONFIG_MAX_MCAST_FLOWS)
#endif

/* Maximum extensions allowed */
#define BLOG_EXTEND_MAX_ENGG    ((BLOG_CONFIG_MAX_FLOWS/BLOG_EXTEND_SIZE_ENGG) + 1)



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

#if defined(CONFIG_BLOG)
#define blog_ptr(skb_p)         skb_p->blog_p
#else
#define blog_ptr(skb_p)         BLOG_NULL
#endif

/* Allocate or deallocate a Blog_t */
Blog_t * blog_get(void);
void blog_put(Blog_t * blog_p);
void blog_hold( Blog_t * blog_p );

/* Allocate a Blog_t and associate with sk_buff or fkbuff */
extern Blog_t * blog_skb(struct sk_buff  * skb_p);
extern Blog_t * blog_fkb(struct fkbuff  * fkb_p);

/* Clear association of Blog_t with sk_buff */
extern Blog_t * blog_snull(struct sk_buff * skb_p);
extern Blog_t * blog_fnull(struct fkbuff  * fkb_p);

/* increment refcount of ct's associated with blog */
extern void blog_ct_get(Blog_t * blog_p);
/* decrement refcount of ct's associated with blog */
extern void blog_ct_put(Blog_t * blog_p, bool force);

/* increment refcount for devices in virt_dev_p array */
extern void blog_dev_hold(const Blog_t * blog_p);
/* decrement refcount for devices in virt_dev_p array */
extern void blog_dev_put(Blog_t * blog_p);

/* Clear association of Blog_t with sk_buff and free Blog_t object */
extern void blog_free( struct sk_buff * skb_p, blog_skip_reason_t reason );

/* Disable further logging. Dis-associate with skb and free Blog object */
extern void blog_skip(struct sk_buff * skb_p, blog_skip_reason_t reason);

/* Transfer association of a Blog_t object between two sk_buffs. */
extern void blog_xfer(struct sk_buff * skb_p, const struct sk_buff * prev_p);

/* clones a Blog_t object for another skb. */
extern struct blog_t *blog_clone(struct sk_buff * skb_p, struct blog_t * prev_p);

/* WLAN specific API: clones a Blog_t object for another skb. */
extern int blog_clone_wlan(struct sk_buff *skb2, struct sk_buff *skb1);

/* Copy a Blog_t object another blog object. */
extern void blog_copy(struct blog_t * new_p, const struct blog_t * prev_p);

extern void  blog_inc_emit_count(struct sk_buff *skb);
extern void  blog_inc_known_exception_client_count(struct sk_buff *skb);
extern void  blog_inc_wlan_mcast_client_count(struct sk_buff *skb);
extern blog_shared_info_t *blog_alloc_shared_info(void);
extern void blog_free_shared_info(blog_shared_info_t *blog_shared_info_p);
extern void blog_print_blog_shared_info(const char *func, uint32_t line, char *dev_name, Blog_t *blog_p);
extern void blog_print_shared_info(const char *func, uint32_t line, struct sk_buff *skb);
extern void  blog_inc_client_count(const struct sk_buff *skb);
extern void  blog_inc_host_client_count(struct sk_buff *skb);
extern void  blog_alloc_shared_info_if_null(const struct sk_buff *skb);

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

#define BLOG_HW_ACCEL_FORCE_DISABLE_OFF                     0
#define BLOG_HW_ACCEL_FORCE_DISABLE_ON                      1

extern void blog_hw_accel_force_disable( pNBuff_t pNBuf, int mode ) ;

#if defined(CONFIG_BLOG)
/* gets an NWE from blog based on the NPE type */
static inline void *_blog_get_ct_nwe(Blog_t *blog_p, uint32_t ct_idx)
{
    blog_npe_t *npe_p = (blog_npe_t *)
        ((blog_p->nxe[BLOG_NXE_CT_IDX(ct_idx)].flags.status == BLOG_NXE_STS_NPE) ?
        blog_p->nxe[BLOG_NXE_CT_IDX(ct_idx)].npe_p : NULL);

    return (npe_p ? npe_p->nwe_p : (void *) NULL);
}
#endif

/*
 *------------------------------------------------------------------------------
 *  Section 2. Associating native OS or 3rd-party network constructs
 *------------------------------------------------------------------------------
 */

extern void blog_link2(BlogNetEntity_t entity_type, Blog_t * blog_p,
                      void * net_p, uint32_t param1, uint32_t param2, uint32_t param3);

#define blog_link(entity_type, blog_p, net_p, param1, param2) \
    blog_link2(entity_type, blog_p, net_p, param1, param2, 0)

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
extern int blog_sinit_generic(struct sk_buff *skb, int *ret);
extern BlogAction_t _blog_sinit( struct sk_buff * skb_p, void * dev_p,
                                 uint32_t encap, uint32_t channel,
                                 uint32_t phyHdr, BlogFcArgs_t *fc_args );

static inline BlogAction_t blog_sinit( struct sk_buff * skb_p, void * dev_p,
                                       uint32_t encap, uint32_t channel,
                                       uint32_t phyHdr, BlogFcArgs_t *fc_args )
{
    return _blog_sinit(skb_p, dev_p, encap, channel, phyHdr, fc_args);
}

extern BlogAction_t _blog_finit( struct fkbuff * fkb_p, void * dev_p,
                                 uint32_t encap, uint32_t channel,
                                 uint32_t phyHdr, BlogFcArgs_t *fc_args );

static inline BlogAction_t blog_finit( struct fkbuff * fkb_p, void * dev_p,
                                       uint32_t encap, uint32_t channel,
                                       uint32_t phyHdr, BlogFcArgs_t *fc_args )
{
    return _blog_finit(fkb_p, dev_p, encap, channel, phyHdr, fc_args);
}

static inline void blog_set_pkt_drop(Blog_t *blog_p, int pkt_drop)
{
    blog_p->pkt_drop = pkt_drop;
}

static inline int blog_get_pkt_drop(Blog_t *blog_p)
{
    return (blog_p->pkt_drop);
}

#if defined(CONFIG_BLOG)
extern BlogAction_t blog_emit_generic(void * nbuff_p, void * dev_p,
                                      uint32_t channel, uint32_t phyHdr);
extern BlogAction_t _blog_emit(void * nbuff_p, void * dev_p,
                               uint32_t encap, uint32_t channel,
                               uint32_t phyHdr, BlogFcArgs_t *fc_args);

static inline BlogAction_t blog_emit_args(void * nbuff_p, void * dev_p,
                                          uint32_t encap, uint32_t channel,
                                          uint32_t phyHdr, BlogFcArgs_t *fc_args)
{
    if ( nbuff_p == NULL ) return PKT_NORM;
    if ( !IS_SKBUFF_PTR(nbuff_p) ) return PKT_NORM;
    // OK, this is something worth looking at, call real function
    return ( _blog_emit(nbuff_p, dev_p, encap, channel, phyHdr, fc_args) );
}

static inline BlogAction_t blog_emit(void * nbuff_p, void * dev_p,
                                     uint32_t encap, uint32_t channel,
                                     uint32_t phyHdr)
{
    BlogFcArgs_t fc_args;
    memset(&fc_args, 0, sizeof(BlogFcArgs_t));
    return ( blog_emit_args(nbuff_p, dev_p, encap, channel, phyHdr, &fc_args) );
}

#else
BlogAction_t blog_emit( void * nbuff_p, void * dev_p,
                        uint32_t encap, uint32_t channel, uint32_t phyHdr );

BlogAction_t blog_emit_args( void * nbuff_p, void * dev_p,
                             uint32_t encap, uint32_t channel, uint32_t phyHdr,
                             BlogFcArgs_t *fc_args);
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
#if defined(CONFIG_BLOG)
extern BlogActivateKey_t *blog_activate( Blog_t * blog_p, BlogTraffic_t traffic );
#else
extern uint32_t blog_activate( Blog_t * blog_p, BlogTraffic_t traffic );
#endif

/*
 *------------------------------------------------------------------------------
 *  blog_deactivate(): static deconfiguration function of blog application
 *------------------------------------------------------------------------------
 */
extern Blog_t * blog_deactivate( BlogActivateKey_t key, BlogTraffic_t traffic );

extern int blog_host_client_config(Blog_t *blog_p, BlogTraffic_t traffic);

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
        uint16_t         BM_HOOK     : 1;
        uint16_t         HC_HOOK     : 1;
        uint16_t         GLC_HOOK    : 1;
        uint16_t         reserved    : 4;
    } bmap;
    uint16_t             hook_info;
} BlogBind_t;

typedef BlogAction_t (* BlogDevRxHook_t)(struct fkbuff *fkb_p, void * dev_p,
                                       BlogFcArgs_t * args);

typedef BlogAction_t (* BlogDevTxHook_t)(struct sk_buff *skb_p, void * dev_p,
                                       uint32_t encap, uint32_t blogHash, BlogFcArgs_t * args);

typedef int (* BlogNotifyHook_t)(blog_notify_api_t blog_notify_api, 
        BlogNotify_t notification, 
        void * net_p, unsigned long param1, unsigned long param2,
        blog_notify_async_cb_fn_t notify_cb_fn, void *notify_cb_data_p);

typedef int (* BlogQueryHook_t)(BlogQuery_t query, void * net_p,
                            uint32_t param1, uint32_t param2, unsigned long param3);

typedef BlogActivateKey_t * (* BlogScHook_t)(Blog_t * blog_p, BlogTraffic_t traffic);

typedef Blog_t * (* BlogSdHook_t)(BlogActivateKey_t key, BlogTraffic_t traffic);
typedef int (* BlogHostClientHook_t)(Blog_t *blog_p, BlogTraffic_t traffic);

typedef void (* BlogFaHook_t)(void *ct_p, BlogFlowEventInfo_t info, BlogFlowEventType_t type);

typedef void (* BlogFdHook_t)(void *ct_p, BlogFlowEventInfo_t info, BlogFlowEventType_t type);

typedef BlogAction_t (* BlogPaHook_t)(struct fkbuff * fkb_p, void * dev_p,
                                      uint32_t encap, uint32_t channel, uint32_t phyHdr);

typedef int (* BlogBitMapHook_t)(blog_group_type_t group_type, uint32_t bitmap_idx, uint32_t *dst_p, uint32_t dst_size_words);
typedef void (* BlogGroupLearnComplete_t)(Blog_t *blog_p);

extern int blog_get_hw_accel(void);
extern void blog_bind(BlogDevRxHook_t rx_hook,    /* Client Rx netdevice handler*/
                      BlogDevTxHook_t tx_hook,    /* Client Tx netdevice handler*/
                      BlogNotifyHook_t xx_hook, /* Client notification handler*/
                      BlogQueryHook_t qr_hook,  /* Client query handler       */
                      BlogBitMapHook_t blog_bm, /* group BitMap copy handler */
                      BlogGroupLearnComplete_t glc_hook, /* group learn complete handler */
                      BlogBind_t   bind
                     );
                     
extern void blog_bind_config(BlogScHook_t sc_hook,    /* Client static config handler*/
                             BlogSdHook_t sd_hook,    /* Client static deconf handler*/
                             BlogHostClientHook_t hc_hook,
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

/* Dump a Blog_t object */
extern void blog_hw_formatted_dump(Blog_t *blog_p);

/* Logging of L2|L3 headers */
extern void blog(struct sk_buff * skb_p, BlogDir_t dir, BlogEncap_t encap,  
                 size_t len, void * data_p);

/* Dump a Blog_t object */
extern void blog_dump(Blog_t * blog_p);

/* Get the minimum Tx MTU for a blog */
uint16_t blog_getTxMtu(Blog_t * blog_p);
extern void blog_spu_us_tx_dev_delta(Blog_t *blog_p);
extern int blog_get_max_unfragment_pktlen(Blog_t *blog_p, uint16_t *max_rx_pktlen_p, uint16_t *max_tx_pktlen_p, int *headroom_p);

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

#define BLOG_FLOW_HW_INVALID            BLOG_INVALID_UINT32

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
	spinlock_t		wakeup_lock;
	bool wakeup_done;
} wq_info_t;

#define BLOG_WAKEUP_WORKER_THREAD(x, mask)                              \
do {                                                                    \
    if ( !((x)->work_avail & mask) ) {                                  \
        (x)->work_avail |= mask;                                        \
        wake_up_interruptible(&((x)->wqh));                             \
    }                                                                   \
} while (0)

/*wake up with spinlock to avoid preemption/bh processing between
 *setting work_avail & wakeup
 */
#define BLOG_WAKEUP_WORKER_THREAD_NO_PREEMPT(x, mask)               \
do {                                                                \
    spin_lock_bh(&((x)->wakeup_lock));								\
    BLOG_WAKEUP_WORKER_THREAD(x, mask);	                         	\
    (x)->wakeup_done = true;                                        \
    spin_unlock_bh(&((x)->wakeup_lock));                            \
} while (0)


void blog_fold_stats(BlogStats_t * const d,
                            const BlogStats_t * const s);
int blog_copy_group_client_bitmap(blog_group_type_t group_type, uint16_t bitmap_idx, uint32_t *dst_p, uint32_t dst_size_words);
#define blog_copy_mcast_client_bitmap(bmidx, dst_p, dst_size_words)   blog_copy_group_client_bitmap(group_type_mcast, bmidx, dst_p, dst_size_words)

#if defined(CONFIG_BCM_UNKNOWN_UCAST)
#define blog_copy_unknown_ucast_client_bitmap(bmidx, dst_p, dst_size_words)   blog_copy_group_client_bitmap(group_type_unknown_ucast, bmidx, dst_p, dst_size_words)
#endif

int blog_set_bridge_tx_dev(Blog_t *cblog_p);
void *blog_group_dev_realloc(Blog_t *mblog_p, uint8_t new);
void blog_group_dev_free(Blog_t *mblog_p);
int blog_group_find_matching_master_dev(Blog_t *mblog_p, void *dev_p, int8_t delta);
int blog_group_add_rx_dev(Blog_t *mblog_p, Blog_t *cblog_p);
void blog_group_del_rx_dev(Blog_t *mblog_p, Blog_t *cblog_p);
int blog_group_add_tx_dev(Blog_t *mblog_p, Blog_t *cblog_p);
void blog_group_del_tx_dev(Blog_t *mblog_p, Blog_t *cblog_p);
void blog_group_del_all_devs(Blog_t *mblog_p);
int blog_group_dev_get_bridge_dev_ref_cnt(Blog_t *mblog_p, void *dev_p);
int blog_group_add_bridge_dev_base_stats(Blog_t *mblog_p, void *dev_p, int master_dev_idx);
int blog_group_del_bridge_dev_base_stats(Blog_t *mblog_p, uint32_t idx);
int blog_group_update_bridge_dev_base_stats(Blog_t *mblog_p, uint32_t idx,
        uint64_t sw_tx_packets, uint64_t sw_tx_bytes,
        uint64_t hw_tx_packets, uint64_t hw_tx_bytes);
int blog_group_get_bridge_dev_base_stats(Blog_t *mblog_p, uint32_t idx,
        uint64_t *sw_tx_packets_p, uint64_t *sw_tx_bytes_p,
        uint64_t *hw_tx_packets_p, uint64_t *hw_tx_bytes_p);

#define BLOG_DUMP_DISABLE   0
#define BLOG_DUMP_RXBLOG    1
#define BLOG_DUMP_TXBLOG    2
#define BLOG_DUMP_RXTXBLOG  3
#define BLOG_DUMP_MCASTBLOG 4

typedef struct blog_config {
    union {
        uint32_t flags;
        struct {
            uint32_t    unused: 28;
            uint32_t    dbg_filter: 1;
            uint32_t    blog_dump: 3;
        };
    };
} blog_config_t;

typedef struct blog_ctx {
    uint32_t  blog_total;
    uint32_t  blog_avail;
    uint32_t  blog_mem_fails;
    uint32_t  blog_extends;
    uint32_t  blog_extend_fails;
    uint32_t  blog_dbg_pkt_filter_no_match;
    blog_info_stats_t  info_stats;
    blog_skip_reason_t blog_skip_stats_table[blog_skip_reason_max];
    blog_free_reason_t blog_free_stats_table[blog_free_reason_max];
    blog_config_t config;
} blog_ctx_t;

/* 
 * These are various major flow types (categories) supported by flow cache.
 */
typedef enum blog_flow_type {
    BLOG_FLOW_TYPE_IPV4_UCAST,
    BLOG_FLOW_TYPE_IPV4_MCAST,
    BLOG_FLOW_TYPE_IPV6_UCAST,
    BLOG_FLOW_TYPE_IPV6_MCAST,
    BLOG_FLOW_TYPE_L2_UCAST,
    BLOG_FLOW_TYPE_MAX
} blog_flow_type_t;

/* 
 * This structure defines whether an optional field is used as part of
 * lookup key by flow cache for classification for a particular flow type
 *
 * Note: more optional fields will be added in future as and when reqd.
 *
 * This is the list of optional field support for various flows:
 * 1. pbit:
 *    - BLOG_FLOW_TYPE_IPV4_UCAST: only outer pbit is supported.
 *    - BLOG_FLOW_TYPE_IPV6_UCAST: only outer pbit is supported.
 *    - BLOG_FLOW_TYPE_IPV4_MCAST: both innner and outer pbit is supported.
 *    - BLOG_FLOW_TYPE_IPV6_MCAST: both innner and outer pbit is supported.
 *
 *    Note: - BLOG_FLOW_TYPE_L2_UCAST: pbit field is don't care for L2 flows.
 */
typedef struct blog_key_opt_fields {
    union {
        uint32_t u32;
        struct {
            BE_DECL(
                uint32_t unused: 31;
                uint32_t pbit: 1;
            )
            LE_DECL(
                uint32_t pbit: 1;
                uint32_t unused: 31;
            )
        };
    };
} blog_key_opt_fields_t;

#define BLOG_VTAG_VID_OFFSET      0
#define BLOG_VTAG_DEI_OFFSET     12
#define BLOG_VTAG_PBIT_OFFSET    13
#define BLOG_VTAG_TPID_OFFSET    16

#define BLOG_VTAG_VID_MASK      0xFFF
#define BLOG_VTAG_DEI_MASK      0x1
#define BLOG_VTAG_PBIT_MASK     0x07
#define BLOG_VTAG_TCI_MASK      0xFFFF
#define BLOG_VTAG_TPID_MASK     0xFFFF

#define BLOG_VTAG_MASK_MAX       2
#define BLOG_VTAG_MASK_UNTAGGED  0   /* mask when a VLAN tag is absent */
#define BLOG_VTAG_MASK_TAGGED    1   /* mask when a VLAN tag is present */

/* These are the masks applied to the packet VLAN tags (max 2 VLAN tags) */
typedef struct blog_vtag_masks {
    uint32_t mask[BLOG_VTAG_MASK_MAX];
} blog_vtag_masks_t;

/*
 * L3 Ucast VLAN tag for classification:
 * 1. When only 1 header is present the VLAN tags are in vtags.
 * 2. when multiple headers are present, inner header VLAN tags are in vtags
 *    and outer header VLAN tags are in outer_vtags.
 * 3. In case of unicast only 1 VLAN tag (outermost) is used for classification
 *    (even when multiple VLAN tags are present in the header, or multiple
 *    headers are present).
 * 
 * VLAN tag selection criterion:
 * 1: if (vtag_num == 0) && (outer_vtag_num != 0) --> Use outer VLAN Tag
 *    from outer_vtags, i.e, outer_vtag[0]
 * 2: if (vtag_num != 0) --> use outer VLAN Tag from vtags, i.e. vtag[0]
 * 3: if (vtag_num == 0) && (outer_vtag_num == 0) --> use vid=pbit=0
*/
static inline void _blog_get_ucast_vtag(uint32_t vtag_num, uint32_t vtag0, 
        uint32_t outer_vtag_num, uint32_t outer_vtag0, 
        uint32_t *ucast_vtag_p, uint32_t *ucast_vtag_num_p)
{
    if (vtag_num == 0)
    {
        if (outer_vtag_num == 0)
        {
            *ucast_vtag_p = 0;
            *ucast_vtag_num_p = 0;
        }
        else
        {
            *ucast_vtag_p = outer_vtag0;
            *ucast_vtag_num_p = outer_vtag_num;
        }
    }
    else
    {
        *ucast_vtag_p = vtag0;
        *ucast_vtag_num_p = vtag_num;
    }
}

#define IN
#define OUT

#define BLOG_FLOW_DEV_NAME       16

/* match device type */
typedef enum blog_flow_dev_dir {
    BLOG_FLOW_DEV_TYPE_NONE,    /* ignore device */
    BLOG_FLOW_DEV_TYPE_RX,      /* RX device */
    BLOG_FLOW_DEV_TYPE_TX,      /* TX device */
    BLOG_FLOW_DEV_TYPE_RX_OR_TX,/* RX or TX device */
    BLOG_FLOW_DEV_TYPE_MAX,     /* MAX */
} blog_flow_dev_type_t;

typedef enum blog_flow_flowtype {
    BLOG_FLOW_FLOWTYPE_UCAST_RXIPV4_TXIPV4,     /* Both RX and TX are IPv4 */
    BLOG_FLOW_FLOWTYPE_UCAST_RXIPV6_TXIPV6,     /* Both RX and TX are IPv6 */
    BLOG_FLOW_FLOWTYPE_UCAST_RXL2_TXL2,         /* Both RX and TX are L2 */
    BLOG_FLOW_FLOWTYPE_MAX
} blog_flow_flowtype_t;

typedef struct {
    blog_flow_flowtype_t flow_type; /* flow types to match */
    blog_flow_dev_type_t dev_type;  /* device type to match */
    char *dev_name;                 /* device name to match */
} blog_flow_filter_t;

typedef union {
    uint8_t u8; 
    struct {
        uint8_t unused: 5;
        uint8_t pbits: 1;
        uint8_t llc_snap: 1;
        uint8_t tcp_pure_ack: 1;
    };
} blog_flow_tuple_flags_t;

typedef union {
    struct {
        uint16_t source;     /* L4 source port */
        uint16_t dest;       /* L4 dest port */
    } port;
    uint32_t ports;
} blog_flow_tuple_l4_ports_t;

/* IPv4 flow Tuple */
typedef struct {
    uint32_t saddr;
    uint32_t daddr;
    blog_flow_tuple_l4_ports_t l4_ports;
    uint8_t proto;
    uint8_t tos;
    blog_flow_tuple_flags_t flags;
    uint8_t unused;
} blog_flow_tuple_ipv4_t;

/* IPv6 flow Tuple */
typedef struct {
    ip6_addr_t saddr;
    ip6_addr_t daddr;
    blog_flow_tuple_l4_ports_t l4_ports;
    uint8_t proto;
    uint8_t tos;
    blog_flow_tuple_flags_t flags;
    uint8_t unused;
} blog_flow_tuple_ipv6_t;

/* L2 flow Tuple */
typedef struct {
    uint8_t mac_sa[6];
    uint8_t mac_da[6];
    uint16_t eth_type;
    blog_flow_tuple_flags_t flags;
    uint8_t tos;
    uint8_t vtag_num;
    uint32_t vtag[MAX_NUM_VLAN_TAG];
} blog_flow_tuple_l2_t;

typedef union {
    blog_flow_tuple_ipv4_t ipv4;
    blog_flow_tuple_ipv6_t ipv6;
    blog_flow_tuple_l2_t l2;
} blog_flow_tuple_t;

typedef struct {
    blog_flow_tuple_t tuple;
    uint8_t channel;
    char dev_name[BLOG_FLOW_DEV_NAME];
    uint8_t mac_sa[6];
    uint8_t mac_da[6];
} blog_flow_rx_t;

typedef blog_flow_rx_t  blog_flow_tx_t;

typedef struct {
    uint32_t valid;
    uint32_t total_pkts;
    uint64_t total_bytes;
} blog_flow_stats_t;

typedef struct {
    uint32_t valid;
    blog_flow_rx_t rx;
    blog_flow_tx_t tx;
} blog_flow_info_t;

typedef struct {
    uint32_t flow_handle;
    uint32_t flow_idx;                  /* use this field for flusing a flow */
} blog_flow_flowid_t;

typedef struct {
    IN  uint32_t start_flow_idx;        /* first flow_idx */
    IN  uint32_t flow_count;            /* max # of flows to return */
    IN  blog_flow_filter_t filter;      /* flow filter */
    OUT blog_flow_flowid_t *flowid_p;   /* array of flowids matched */
} blog_flow_get_flowid_t;

typedef struct {
    IN  blog_flow_flowid_t *flowid_p;   /* array of flowids to match */
    IN  uint32_t flow_count;            /* max # of flowinfo to return */
    OUT blog_flow_info_t *flowinfo_p;   /* array of flowinfo */
    OUT blog_flow_stats_t *stats_p;     /* array of flow stats */
} blog_flow_get_flowinfo_t;

typedef struct {
    IN  blog_flow_flowid_t *flowid_p;   /* array of flowids to match */
    IN  uint32_t flow_count;            /* max # of flows to return */
    OUT blog_flow_stats_t *stats_p;     /* array of flow stats */
} blog_flow_get_flowstats_t;

/*
 *------------------------------------------------------------------------------
 * blog flow APIs
 *
 * Description:
 * 1.   First, the user should call blog_flow_get_flowid() to get the list of
 *      active flows matching the filter criterion (blog_flow_filter_t, specified
 *      by flow_type and dev_type, and dev_name).
 * 2.   Second, the user should call blog_flow_get_flowinfo() to get the flowinfo
 *      for the matching flowids returned by above call to blog_flow_get_flowdid().
 *      The blog_flow_get_flowinfo() returns both the flowinfo and stats.
 * 3.   Third, the user should call blog_flow_get_flowstats() to get the stats
 *      for the matching flowids to keep polling the flow stats multiple times.
 *
 * Note:
 * - It is recommended to call the blog_flow_get_flowinfo() once for the flowids
 *   and then keep calling the blog_flow_get_flowstats().
 * - It is recommended to use a small value for flow_count (may be 16, 32).
 * - It is possible that a flow has been evicted by the time blog_flow_get_flowinfo()
 *   or blog_flow_get_flowstats() is invoked. For these evicted flows the valid flag
 *   is cleared in the flowinfo and flowstats.
 * - In case some more flows have been added or evicted, then the above 3 steps
 *   should be repeated again for the active flows.
 *------------------------------------------------------------------------------
 */

/*
 *------------------------------------------------------------------------------
 * Function     : blog_flow_get_flowid
 * Description  : gets the blog_flow_flowid_t (flow_handle and flow_idx) of the
 *              : active flows, begining from start_flow_idx upto max of flow_count 
 *              : number of flows.
 *              :
 *              : When invoking the next call use flow_idx from the (n-1) index
 *              : from the previous invokation and add 1 to calculate the value 
 *              : of the start_flow_idx in the new call.
 *              :
 * Input        :
 * get_flowid_p : pointer to blog_flow_get_flowid_t
 * Return       : count of flowids
 *          -1  : error
 *           n  : n is equal to flow_count. more active flows present
 *         < n  : reached the end of active flows 
 *------------------------------------------------------------------------------
 */
int blog_flow_get_flowid(blog_flow_get_flowid_t *get_flowid_p);

/*
 *------------------------------------------------------------------------------
 * Function     : blog_flow_get_flowinfo
 * Description  : gets the flowinfo, and stats for the flows, begining from 
 *              : start_flow_idx upto max of flow_count number of flows.
 *              :
 * Input        :
 * get_flowinfo_p : pointer to blog_flow_get_flowinfo_t
 * Return       :
 *          -1  : error
 *           1  : no error
 *------------------------------------------------------------------------------
 */
int blog_flow_get_flowinfo(blog_flow_get_flowinfo_t *get_flowinfo_p);

/*
 *------------------------------------------------------------------------------
 * Function     : blog_flow_get_flowstats
 * Description  : gets the stats for the flows, begining from 
 *              : start_flow_idx upto max of flow_count number of flows.
 *              :
 * Input        :
 * get_flowstats_p : pointer to blog_flow_get_flowstats_t
 * Return       :
 *          -1  : error
 *           1  : no error
 *------------------------------------------------------------------------------
 */
int blog_flow_get_flowstats(blog_flow_get_flowstats_t *get_flowstats_p);

#define BLOG_ESP4_RXHDR_LEN(b)  \
    (BLOG_IPV4_HDR_LEN + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN + (b)->esprx.ivsize)

#define BLOG_ESP4_TXHDR_LEN(b)  \
    (BLOG_IPV4_HDR_LEN + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN + (b)->esptx.ivsize)

#define BLOG_ESP6_RXHDR_LEN(b)  \
    (BLOG_IPV6_HDR_LEN + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN + (b)->esprx.ivsize)

#define BLOG_ESP6_TXHDR_LEN(b)  \
    (BLOG_IPV6_HDR_LEN + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN + (b)->esptx.ivsize)

#define BLOG_GRE4_RXHDR_LEN(b)  \
    (BLOG_IPV4_HDR_LEN + (b)->grerx.hlen + (b)->grerx.l2_hlen)

#define BLOG_GRE4_TXHDR_LEN(b)  \
    (BLOG_IPV4_HDR_LEN + (b)->gretx.hlen + (b)->gretx.l2_hlen)

#define BLOG_GRE6_RXHDR_LEN(b)  \
    (BLOG_IPV6_HDR_LEN + (b)->grerx.hlen + (b)->grerx.l2_hlen)

#define BLOG_GRE6_TXHDR_LEN(b)  \
    (BLOG_IPV6_HDR_LEN + (b)->gretx.hlen + (b)->gretx.l2_hlen)

#define BLOG_ESP4_GRE4_RXHDR_LEN(b)  (BLOG_ESP4_RXHDR_LEN(b) + BLOG_GRE4_RXHDR_LEN(b))
#define BLOG_ESP4_GRE4_TXHDR_LEN(b)  (BLOG_ESP4_TXHDR_LEN(b) + BLOG_GRE4_TXHDR_LEN(b))
#define BLOG_ESP4_GRE6_RXHDR_LEN(b)  (BLOG_ESP4_RXHDR_LEN(b) + BLOG_GRE6_RXHDR_LEN(b))
#define BLOG_ESP4_GRE6_TXHDR_LEN(b)  (BLOG_ESP4_TXHDR_LEN(b) + BLOG_GRE6_TXHDR_LEN(b))

#define BLOG_ESP6_GRE4_RXHDR_LEN(b)  (BLOG_ESP6_RXHDR_LEN(b) + BLOG_GRE4_RXHDR_LEN(b))
#define BLOG_ESP6_GRE4_TXHDR_LEN(b)  (BLOG_ESP6_TXHDR_LEN(b) + BLOG_GRE4_TXHDR_LEN(b))
#define BLOG_ESP6_GRE6_RXHDR_LEN(b)  (BLOG_ESP6_RXHDR_LEN(b) + BLOG_GRE6_RXHDR_LEN(b))
#define BLOG_ESP6_GRE6_TXHDR_LEN(b)  (BLOG_ESP6_TXHDR_LEN(b) + BLOG_GRE6_TXHDR_LEN(b))

#define BLOG_ESP4_L2TP_RXHDR_LEN(b)  (BLOG_ESP4_RXHDR_LEN(b) + BLOG_UDP_HDR_LEN + \
    (b)->l2tprx.hlen + BLOG_L2TP_PPP_LENGTH(&(b)->l2tprx))

#define BLOG_ESP4_L2TP_TXHDR_LEN(b)  (BLOG_ESP4_TXHDR_LEN(b) + BLOG_UDP_HDR_LEN + \
    (b)->l2tptx.hlen + BLOG_L2TP_PPP_LENGTH(&(b)->l2tptx))

#define BLOG_L2TP_PPP_LENGTH(l2tp_p)  (((l2tp_p)->pppInfo == BLOG_PPP_ADDR_CTL) ? BLOG_L2TP_PPP_LEN : BLOG_L2TP_PPP_LEN2)

#define BLOG_GROUP_JOIN_ID_INVALID  0

#endif /* defined(__BLOG_H_INCLUDED__) */

#endif /* CONFIG_BCM_KF_BLOG */
