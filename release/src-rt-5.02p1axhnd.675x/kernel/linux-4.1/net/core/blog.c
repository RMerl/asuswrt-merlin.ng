#if defined(CONFIG_BCM_KF_BLOG)

/*
*    Copyright (c) 2003-2012 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2012:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*
 *******************************************************************************
 * File Name  : blog.c
 * Description: Implements the tracing of L2 and L3 modifications to a packet
 *              buffer while it traverses the Linux networking stack.
 *******************************************************************************
 */

#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/blog.h>
#include <linux/blog_net.h>
#include <linux/nbuff.h>
#include <linux/skbuff.h>
#if defined(CONFIG_BCM_KF_SKB_DEFINES)
#include <linux/bcm_skb_defines.h>
#endif
#include <linux/notifier.h>
#include <net/netevent.h>
#if defined(CONFIG_XFRM) 
#include <net/xfrm.h>
#endif

#if defined(CONFIG_BLOG)

#include <linux/netdevice.h>
#include <linux/slab.h>
#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)   
#define BLOG_NF_CONNTRACK
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#endif /* defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE) */

#include "../bridge/br_private.h"

#include <linux/bcm_colors.h>

#include <net/dsfield.h>
#include <linux/netfilter/xt_dscp.h>
#if defined(CONFIG_BCM_KF_MAP) || defined(CONFIG_BCM_MAP_MODULE)
#include "../../../bcmdrivers/opensource/char/map/impl1/ivi_map.h"
#endif
#include <blog_ioctl.h>

/*--- globals ---*/

/* RFC4008 */

/* Debug macros */
int blog_dbg = 0;

DEFINE_SPINLOCK(blog_lock_tbl_g);
#define BLOG_LOCK_TBL()         spin_lock_bh( &blog_lock_tbl_g )
#define BLOG_UNLOCK_TBL()       spin_unlock_bh( &blog_lock_tbl_g )

/* Length prioritization table index */
static uint8_t blog_len_tbl_idx = 0;
/* Length prioritization table
 * {tbl idx}{min, max, original mark, target mark}
 */
static uint32_t blog_len_tbl[BLOG_MAX_LEN_TBLSZ][BLOG_LEN_PARAM_NUM];

/* DSCP mangle table
 * {target dscp}
 */
static uint8_t blog_dscp_tbl[BLOG_MAX_DSCP_TBLSZ];

/* TOS mangle table
 * {target tos}
 */
static uint8_t blog_tos_tbl[BLOG_MAX_TOS_TBLSZ];

/* Temporary storage for passing the values from pre-modify hook to
 * post-modify hook.
 * {ack priority, length priority, dscp value, tos value}
 */
static uint32_t blog_mangl_params[BLOG_MAX_FEATURES];

#if defined(CC_BLOG_SUPPORT_DEBUG)
#define blog_print(fmt, arg...)                                         \
    if ( blog_dbg )                                                     \
    printk( CLRc "BLOG %s :" fmt CLRnl, __FUNCTION__, ##arg )
#define blog_assertv(cond)                                              \
    if ( !cond ) {                                                      \
        printk( CLRerr "BLOG ASSERT %s : " #cond CLRnl, __FUNCTION__ ); \
        return;                                                         \
    }
#define blog_assertr(cond, rtn)                                         \
    if ( !cond ) {                                                      \
        printk( CLRerr "BLOG ASSERT %s : " #cond CLRnl, __FUNCTION__ ); \
        return rtn;                                                     \
    }
#define BLOG_DBG(debug_code)    do { debug_code } while(0)
#else
#define blog_print(fmt, arg...) NULL_STMT
#define blog_assertv(cond)      NULL_STMT
#define blog_assertr(cond, rtn) NULL_STMT
#define BLOG_DBG(debug_code)    NULL_STMT
#endif

#define blog_error(fmt, arg...)                                         \
    printk( CLRerr "BLOG ERROR %s :" fmt CLRnl, __FUNCTION__, ##arg)

#undef  BLOG_DECL
#define BLOG_DECL(x)        #x,         /* string declaration */
#define BLOG_ARY_INIT(x)    [x] = BLOG_DECL(x)  /* initialization of member of string array  */

/*--- globals ---*/

DEFINE_SPINLOCK(blog_lock_g);               /* blogged packet flow */
EXPORT_SYMBOL(blog_lock_g);
static DEFINE_SPINLOCK(blog_pool_lock_g);   /* blog pool only */
#define BLOG_POOL_LOCK()   spin_lock_irqsave(&blog_pool_lock_g, lock_flags)
#define BLOG_POOL_UNLOCK() spin_unlock_irqrestore(&blog_pool_lock_g, lock_flags)


static ATOMIC_NOTIFIER_HEAD(blog_flowevent_chain);


/*----- Forward declarations -----*/
static long blog_drv_ioctl(struct file *filep, unsigned int command, 
                           unsigned long arg);

static int  blog_drv_open(struct inode *inode, struct file *filp);

typedef struct {
    struct file_operations fops;
} __attribute__((aligned(16))) blog_drv_t;


static blog_drv_t blog_drv_g = {
    .fops = {
        .unlocked_ioctl = blog_drv_ioctl,
#if defined(CONFIG_COMPAT)
        .compat_ioctl = blog_drv_ioctl,
#endif
        .open           = blog_drv_open
    }
};


const char *blog_drv_ioctl_name[] =
{
    BLOG_ARY_INIT(BLOG_IOCTL_GET_STATS)
    BLOG_ARY_INIT(BLOG_IOCTL_RESET_STATS)
    BLOG_ARY_INIT(BLOG_IOCTL_DUMP_BLOG)
    BLOG_ARY_INIT(BLOG_IOCTL_INVALID)
};


/* Accelerator functions binds to this pointer to set the accel mode */
blog_accel_mode_set_t blog_accel_mode_set_fn = NULL;

/*
 * blog_support_accel_mode_g inherits the default value from CC_BLOG_SUPPORT_ACCEL_MODE
 */
int blog_support_accel_mode_g = CC_BLOG_SUPPORT_ACCEL_MODE;

/*
 * blog_support_get_accel_mode_g returns the current accel mode
 * Exported blog_support_get_accel_mode()
 */
int blog_support_get_accel_mode(void) 
{ 
    return blog_support_accel_mode_g;
}

/*
 * Exported blog_support_accel_mode() may be used to set blog_support_accel_mode_g.
 */
void blog_support_accel_mode(int accel_mode)
{
#if !defined(CONFIG_BRIDGE_NETFILTER)
    if (blog_accel_mode_set_fn)
        blog_accel_mode_set_fn( accel_mode );

    blog_support_accel_mode_g = accel_mode;
#endif
}

/*TCP ACK Multi-Flow */
blog_tcp_ack_mflows_set_t blog_tcp_ack_mflows_set_fn = NULL;

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM_XRDP) || defined(CONFIG_BCM_ARCHER) || defined(CONFIG_BCM_ARCHER_MODULE)
int blog_support_tcp_ack_mflows_g = 1;
#else
int blog_support_tcp_ack_mflows_g = 0;
#endif

int blog_support_get_tcp_ack_mflows(void) 
{ 
    return blog_support_tcp_ack_mflows_g;
}

/*
 * Exported blog_support_set_tcp_ack_mflows() may be used to set blog_support_tcp_ack_mflows_g.
 */
void blog_support_set_tcp_ack_mflows(int enable)
{
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM_XRDP) || defined(CONFIG_BCM_ARCHER) || defined(CONFIG_BCM_ARCHER_MODULE)
    if (blog_tcp_ack_mflows_set_fn)
        blog_tcp_ack_mflows_set_fn( enable );

    blog_support_tcp_ack_mflows_g = enable;
#else
    printk(KERN_ERR "TCP ACK Multi-Flow is not supported \n");
#endif
}

/*
 * blog_support_mcast_g inherits the default value from CC_BLOG_SUPPORT_MCAST
 * Exported blog_support_mcast() may be used to set blog_support_mcast_g.
 */
int blog_support_mcast_g = CC_BLOG_SUPPORT_MCAST;
void blog_support_mcast(int config) { blog_support_mcast_g = config; }

/*
 * blog_support_mcast_learn_g inherits the default value from
 * CC_BLOG_SUPPORT_MCAST_LEARN
 * Exported blog_support_mcast_learn() may be used to set
 * blog_support_mcast_learn_g.
 */
int blog_support_mcast_learn_g = CC_BLOG_SUPPORT_MCAST_LEARN;
void blog_support_mcast_learn(int config) 
{ blog_support_mcast_learn_g = config; }

/*
 * blog_support_ipv6_g inherits the value from CC_BLOG_SUPPORT_IPV6
 * Exported blog_support_ipv6() may be used to set blog_support_ipv6_g.
 */
int blog_support_ipv6_g = CC_BLOG_SUPPORT_IPV6;
void blog_support_ipv6(int config) { blog_support_ipv6_g = config; }

/*
 * Exported blog_set_notify_proc_mode() may be used to set blog_notify_proc_mode_g.
 */
int blog_notify_proc_mode_g = BLOG_NOTIFY_PROC_MODE_HYBRID; 
void blog_set_notify_proc_mode(int mode) {blog_notify_proc_mode_g = mode;}

/*
 * blog_tunl_tos_g gets the value from BLOG_DEFAULT_TUNL_TOS
 * Exported blog_tunl_tos_g() may be used to set blog_tunl_tos_g.
 */

/*
 * blog_support_gre_g inherits the default value from CC_BLOG_SUPPORT_GRE
 * Exported blog_support_gre() may be used to set blog_support_gre_g.
 */
int blog_gre_tunnel_accelerated_g = BLOG_GRE_DISABLE;

int blog_support_gre_g = CC_BLOG_SUPPORT_GRE;
void blog_support_gre(int config) 
{ 
    blog_support_gre_g = config; 

    if (blog_fc_enabled() && (blog_support_gre_g == BLOG_GRE_ENABLE))
        blog_gre_tunnel_accelerated_g = BLOG_GRE_ENABLE;  
    else
        blog_gre_tunnel_accelerated_g = BLOG_GRE_DISABLE;  
}

int blog_rcv_chk_gre(struct fkbuff *fkb_p, uint32_t h_proto, uint16_t *gflags_p);
int blog_xmit_chk_gre(struct sk_buff *skb_p, uint32_t h_proto); 

/*
 * blog_support_l2tp_g inherits the default value from CC_BLOG_SUPPORT_L2TP
 * Exported blog_support_l2tp() may be used to set blog_support_l2tp_g.
 */

int blog_l2tp_tunnel_accelerated_g = BLOG_L2TP_DISABLE;
int blog_support_l2tp_g = CC_BLOG_SUPPORT_L2TP;
void blog_support_l2tp(int config) 
{ 
    blog_support_l2tp_g = config; 
    if (blog_fc_enabled())
    {
        if( !blog_support_l2tp_g )
            blog_l2tp_tunnel_accelerated_g = BLOG_L2TP_DISABLE; 
        else if ( blog_support_l2tp_g == BLOG_L2TP_TUNNEL )
            blog_l2tp_tunnel_accelerated_g = BLOG_L2TP_TUNNEL; 
        else if ( blog_support_l2tp_g == BLOG_L2TP_TUNNEL_WITHCHKSUM )
            blog_l2tp_tunnel_accelerated_g = BLOG_L2TP_TUNNEL_WITHCHKSUM;        
    }   

}

/*
 * blog_support_4o6_frag_g 
 * Exported blog_support_4o6_frag() may be used to set blog_support_4o6_frag_g.
 */
int blog_support_4o6_frag_g = BLOG_4O6_FRAG_ENABLE;
void blog_support_4o6_frag(int config) { blog_support_4o6_frag_g = config; }

/*
 * Traffic flow generator, keep conntrack alive during idle traffic periods
 * by refreshing the conntrack. 
 * Netfilter may not be statically loaded.
 */
blog_cttime_upd_t blog_cttime_update_fn = (blog_cttime_upd_t) NULL;
struct sk_buff * nfskb_p = (struct sk_buff *) NULL;

#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE)
blog_gre_rcv_check_t blog_gre_rcv_check_fn = NULL;
blog_gre_xmit_upd_t blog_gre_xmit_update_fn = NULL;
blog_gre6_rcv_check_t blog_gre6_rcv_check_fn = NULL;
blog_gre6_xmit_upd_t blog_gre6_xmit_update_fn = NULL;
#endif

blog_pptp_rcv_check_t blog_pptp_rcv_check_fn = NULL;
blog_pptp_xmit_upd_t blog_pptp_xmit_update_fn = NULL; 
blog_pptp_xmit_get_t blog_pptp_xmit_get_fn = NULL; 

blog_dhd_flow_update_t blog_dhd_flow_update_fn = NULL;
blog_l2tp_rcv_check_t blog_l2tp_rcv_check_fn = NULL;

#if defined(CONFIG_BCM_OVS)
blog_ovs_hooks_t blog_ovs_hooks_g = {};
#endif

/*----- Constant string representation of enums for print -----*/
const char * strBlogAction[BLOG_ACTION_MAX] =
{
    BLOG_ARY_INIT(PKT_DONE)
    BLOG_ARY_INIT(PKT_NORM)
    BLOG_ARY_INIT(PKT_BLOG)
    BLOG_ARY_INIT(PKT_DROP)
    BLOG_ARY_INIT(PKT_TCP4_LOCAL)
};

const char * strBlogDir[BLOG_DIR_MAX] =
{
    BLOG_ARY_INIT(DIR_RX)
    BLOG_ARY_INIT(DIR_TX)
};

const char * strBlogNetEntity[BLOG_NET_ENTITY_MAX] =
{
    BLOG_ARY_INIT(FLOWTRACK)
    BLOG_ARY_INIT(BRIDGEFDB)
    BLOG_ARY_INIT(MCAST_FDB)
    BLOG_ARY_INIT(IF_DEVICE)
    BLOG_ARY_INIT(IF_DEVICE_MCAST)
    BLOG_ARY_INIT(GRE_TUNL)
    BLOG_ARY_INIT(TOS_MODE)
    BLOG_ARY_INIT(MAP_TUPLE)
    BLOG_ARY_INIT(MEGA)
};

const char * strBlogNotify[BLOG_NOTIFY_MAX] =
{
    BLOG_ARY_INIT(DESTROY_FLOWTRACK)
    BLOG_ARY_INIT(DESTROY_BRIDGEFDB)
    BLOG_ARY_INIT(MCAST_CONTROL_EVT)
    BLOG_ARY_INIT(MCAST_SYNC_EVT)
    BLOG_ARY_INIT(DESTROY_NETDEVICE)
    BLOG_ARY_INIT(FETCH_NETIF_STATS)
    BLOG_ARY_INIT(CLEAR_NETIF_STATS)
    BLOG_ARY_INIT(DYNAMIC_DSCP_EVENT)
    BLOG_ARY_INIT(UPDATE_NETDEVICE)
    BLOG_ARY_INIT(ARP_BIND_CHG)
    BLOG_ARY_INIT(CONFIG_CHANGE)
    BLOG_ARY_INIT(UP_NETDEVICE)
    BLOG_ARY_INIT(DN_NETDEVICE)
    BLOG_ARY_INIT(CHANGE_ADDR)
    BLOG_ARY_INIT(SET_DPI_PARAM)
    BLOG_ARY_INIT(DESTROY_MAP_TUPLE)
    BLOG_ARY_INIT(FLUSH)
    BLOG_ARY_INIT(DESTROY_MEGA)
    BLOG_ARY_INIT(FETCH_MEGA_STATS)
    BLOG_ARY_INIT(CLEAR_MEGA_STATS)
};

const char * strBlogQuery[BLOG_QUERY_MAX] =
{
    BLOG_ARY_INIT(QUERY_FLOWTRACK)
    BLOG_ARY_INIT(QUERY_BRIDGEFDB)
    BLOG_ARY_INIT(QUERY_MAP_TUPLE)
    BLOG_ARY_INIT(QUERY_FLOWTRACK_STATS)
    BLOG_ARY_INIT(QUERY_GET_HW_ACCEL)
};

const char * strBlogRequest[BLOG_REQUEST_MAX] =
{
    BLOG_ARY_INIT(FLOWTRACK_KEY_SET)
    BLOG_ARY_INIT(FLOWTRACK_KEY_GET)
    BLOG_ARY_INIT(FLOWTRACK_DSCP_GET)
    BLOG_ARY_INIT(FLOWTRACK_CONFIRMED)
    BLOG_ARY_INIT(FLOWTRACK_ALG_HELPER)
    BLOG_ARY_INIT(FLOWTRACK_EXCLUDE)
    BLOG_ARY_INIT(FLOWTRACK_TIME_SET)
    BLOG_ARY_INIT(FLOWTRACK_PUT_STATS)
    BLOG_ARY_INIT(NETIF_PUT_STATS)
    BLOG_ARY_INIT(LINK_XMIT_FN)
    BLOG_ARY_INIT(LINK_NOCARRIER)
    BLOG_ARY_INIT(NETDEV_NAME)
    BLOG_ARY_INIT(MCAST_DFLT_MIPS)
    BLOG_ARY_INIT(IQPRIO_SKBMARK_SET)
    BLOG_ARY_INIT(DPIQ_SKBMARK_SET)
    BLOG_ARY_INIT(BRIDGEFDB_KEY_SET)
    BLOG_ARY_INIT(BRIDGEFDB_KEY_GET)
    BLOG_ARY_INIT(BRIDGEFDB_TIME_SET)
    BLOG_ARY_INIT(BRIDGEFDB_IFIDX_GET)
    BLOG_ARY_INIT(SYS_TIME_GET) 
    BLOG_ARY_INIT(GRE_TUNL_XMIT)
    BLOG_ARY_INIT(GRE6_TUNL_XMIT)
    BLOG_ARY_INIT(SKB_DST_ENTRY_SET)
    BLOG_ARY_INIT(SKB_DST_ENTRY_RELEASE)
    BLOG_ARY_INIT(NETDEV_ADDR)
    BLOG_ARY_INIT(FLOW_EVENT_ACTIVATE)
    BLOG_ARY_INIT(FLOW_EVENT_DEACTIVATE)
    BLOG_ARY_INIT(CHK_HOST_DEV_MAC)
    BLOG_ARY_INIT(MAP_TUPLE_KEY_SET)
    BLOG_ARY_INIT(MAP_TUPLE_KEY_GET)
    BLOG_ARY_INIT(MEGA_KEY_SET)
    BLOG_ARY_INIT(MEGA_KEY_GET)
    BLOG_ARY_INIT(MEGA_PUT_STATS)
};

const char * strBlogEncap[PROTO_MAX] =
{
    BLOG_ARY_INIT(GRE_ETH)
    BLOG_ARY_INIT(BCM_XPHY)
    BLOG_ARY_INIT(BCM_SWC)
    BLOG_ARY_INIT(ETH_802x)
    BLOG_ARY_INIT(VLAN_8021Q)
    BLOG_ARY_INIT(PPPoE_2516)
    BLOG_ARY_INIT(PPP_1661)
    BLOG_ARY_INIT(PLD_IPv4)
    BLOG_ARY_INIT(PLD_IPv6)
    BLOG_ARY_INIT(PPTP)
    BLOG_ARY_INIT(L2TP)
    BLOG_ARY_INIT(GRE)
    BLOG_ARY_INIT(ESP)
    BLOG_ARY_INIT(DEL_IPv4)
    BLOG_ARY_INIT(DEL_IPv6)
    BLOG_ARY_INIT(PLD_L2)
    BLOG_ARY_INIT(HDR0_IPv4)
    BLOG_ARY_INIT(HDR0_IPv6)
    BLOG_ARY_INIT(GREoESP_type)
    BLOG_ARY_INIT(GREoESP_type_resvd)
    BLOG_ARY_INIT(GREoESP)
};

/*
 *------------------------------------------------------------------------------
 * Support for RFC 2684 headers logging.
 *------------------------------------------------------------------------------
 */
const char * strRfc2684[RFC2684_MAX] =
{
    BLOG_ARY_INIT(RFC2684_NONE)         /*                               */
    BLOG_ARY_INIT(LLC_SNAP_ETHERNET)    /* AA AA 03 00 80 C2 00 07 00 00 */
    BLOG_ARY_INIT(LLC_SNAP_ROUTE_IP)    /* AA AA 03 00 00 00 08 00       */
    BLOG_ARY_INIT(LLC_ENCAPS_PPP)       /* FE FE 03 CF                   */
    BLOG_ARY_INIT(VC_MUX_ETHERNET)      /* 00 00                         */
    BLOG_ARY_INIT(VC_MUX_IPOA)          /*                               */
    BLOG_ARY_INIT(VC_MUX_PPPOA)         /*                               */
    BLOG_ARY_INIT(PTM)                  /*                               */
};

const uint8_t rfc2684HdrLength[RFC2684_MAX] =
{
     0, /* header was already stripped. :                               */
    10, /* LLC_SNAP_ETHERNET            : AA AA 03 00 80 C2 00 07 00 00 */
     8, /* LLC_SNAP_ROUTE_IP            : AA AA 03 00 00 00 08 00       */
     4, /* LLC_ENCAPS_PPP               : FE FE 03 CF                   */
     2, /* VC_MUX_ETHERNET              : 00 00                         */
     0, /* VC_MUX_IPOA                  :                               */
     0, /* VC_MUX_PPPOA                 :                               */
     0, /* PTM                          :                               */
};

const uint8_t rfc2684HdrData[RFC2684_MAX][16] =
{
    {},
    { 0xAA, 0xAA, 0x03, 0x00, 0x80, 0xC2, 0x00, 0x07, 0x00, 0x00 },
    { 0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00 },
    { 0xFE, 0xFE, 0x03, 0xCF },
    { 0x00, 0x00 },
    {},
    {},
    {}
};

const char * strBlogPhy[BLOG_MAXPHY] =
{
    BLOG_ARY_INIT(BLOG_NOPHY)
    BLOG_ARY_INIT(BLOG_XTMPHY)
    BLOG_ARY_INIT(BLOG_ENETPHY)
    BLOG_ARY_INIT(BLOG_GPONPHY)
    BLOG_ARY_INIT(BLOG_EPONPHY)
    BLOG_ARY_INIT(BLOG_USBPHY)
    BLOG_ARY_INIT(BLOG_WLANPHY)
    BLOG_ARY_INIT(BLOG_MOCAPHY)
    BLOG_ARY_INIT(BLOG_EXTRA1PHY)
    BLOG_ARY_INIT(BLOG_LTEPHY)
    BLOG_ARY_INIT(BLOG_SIDPHY)
    BLOG_ARY_INIT(BLOG_TCP4_LOCALPHY)
    BLOG_ARY_INIT(BLOG_SPU_DS)
    BLOG_ARY_INIT(BLOG_SPU_US)
    BLOG_ARY_INIT(BLOG_NETXLPHY)
    BLOG_ARY_INIT(BLOG_SPDTST)
};

const char * strIpctDir[] = {   /* in reference to enum ip_conntrack_dir */
    BLOG_DECL(DIR_ORIG)
    BLOG_DECL(DIR_RPLY)
    BLOG_DECL(DIR_UNKN)
};

#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
const char * strIpctStatus[] =  /* in reference to enum ip_conntrack_status */
{
    BLOG_ARY_INIT(IPS_EXPECTED_BIT)
    BLOG_ARY_INIT(IPS_SEEN_REPLY_BIT)
    BLOG_ARY_INIT(IPS_ASSURED_BIT)
    BLOG_ARY_INIT(IPS_CONFIRMED_BIT)
    BLOG_ARY_INIT(IPS_SRC_NAT_BIT)
    BLOG_ARY_INIT(IPS_DST_NAT_BIT)
    BLOG_ARY_INIT(IPS_SEQ_ADJUST_BIT)
    BLOG_ARY_INIT(IPS_SRC_NAT_DONE_BIT)
    BLOG_ARY_INIT(IPS_DST_NAT_DONE_BIT)
    BLOG_ARY_INIT(IPS_DYING_BIT)
    BLOG_ARY_INIT(IPS_FIXED_TIMEOUT_BIT)
    BLOG_ARY_INIT(IPS_TEMPLATE_BIT)
    BLOG_ARY_INIT(IPS_UNTRACKED_BIT)
    BLOG_ARY_INIT(IPS_HELPER_BIT)
#if defined(CONFIG_BCM_KF_NETFILTER)
    BLOG_ARY_INIT(IPS_IQOS_BIT)
#endif
    BLOG_ARY_INIT(IPS_BLOG_BIT)
};
#endif

const char * strBlogTos[] =
{
    BLOG_ARY_INIT(BLOG_TOS_FIXED)
    BLOG_ARY_INIT(BLOG_TOS_INHERIT)
};


/*
 *------------------------------------------------------------------------------
 * Default Rx and Tx hooks.
 * FIXME: Group these hooks into a structure and change blog_bind to use
 *        a structure.
 *------------------------------------------------------------------------------
 */
static BlogDevRxHook_t blog_rx_hook_g = (BlogDevRxHook_t)NULL;
static BlogDevTxHook_t blog_tx_hook_g = (BlogDevTxHook_t)NULL;
static BlogNotifyHook_t blog_xx_hook_g = (BlogNotifyHook_t)NULL;
static BlogQueryHook_t blog_qr_hook_g = (BlogQueryHook_t)NULL;
static BlogScHook_t blog_sc_hook_g[BlogClient_MAX] = { (BlogScHook_t)NULL };
static BlogSdHook_t blog_sd_hook_g[BlogClient_MAX] = { (BlogSdHook_t)NULL };
static BlogPaHook_t blog_pa_hook_g = (BlogPaHook_t)NULL;

#if defined(CONFIG_BCM_KF_WL)
void (*wl_pktc_del_hook)(unsigned long addr,
                         struct net_device * net_device) = NULL;
void (*dhd_pktc_del_hook)(unsigned long addr,
                         struct net_device * net_device) = NULL;
EXPORT_SYMBOL(wl_pktc_del_hook);
EXPORT_SYMBOL(dhd_pktc_del_hook);
#endif

const char *str_blog_skip_reason[blog_skip_reason_max] =
{
    BLOG_ARY_INIT(blog_skip_reason_unknown) /* unknown or customer defined */
    BLOG_ARY_INIT(blog_skip_reason_br_flood)
    BLOG_ARY_INIT(blog_skip_reason_ct_tcp_state_not_est)
    BLOG_ARY_INIT(blog_skip_reason_ct_tcp_state_ignore)
    BLOG_ARY_INIT(blog_skip_reason_ct_status_donot_blog)
    BLOG_ARY_INIT(blog_skip_reason_nf_xt_skiplog)
    BLOG_ARY_INIT(blog_skip_reason_nf_ebt_skiplog)
    BLOG_ARY_INIT(blog_skip_reason_scrub_pkt)
    BLOG_ARY_INIT(blog_skip_reason_sch_htb)
    BLOG_ARY_INIT(blog_skip_reason_sch_dsmark)
    BLOG_ARY_INIT(blog_skip_reason_unknown_proto)
    BLOG_ARY_INIT(blog_skip_reason_unknown_proto_ah4)
    BLOG_ARY_INIT(blog_skip_reason_unknown_proto_ah6)
    BLOG_ARY_INIT(blog_skip_reason_unknown_proto_esp6)
    BLOG_ARY_INIT(blog_skip_reason_esp4_crypto_algo)
    BLOG_ARY_INIT(blog_skip_reason_esp4_spu_disabled)
    BLOG_ARY_INIT(blog_skip_reason_spudd_check_failure)
    BLOG_ARY_INIT(blog_skip_reason_dpi)
    BLOG_ARY_INIT(blog_skip_reason_bond)
    BLOG_ARY_INIT(blog_skip_reason_map_tcp)
    BLOG_ARY_INIT(blog_skip_reason_blog)
    BLOG_ARY_INIT(blog_skip_reason_l2_local_termination)
    BLOG_ARY_INIT(blog_skip_reason_mega_multi_output_ports)
    BLOG_ARY_INIT(blog_skip_reason_mega_attr_mismatch)
    BLOG_ARY_INIT(blog_skip_reason_mega_field_mismatch)
};

const char *str_blog_free_reason[blog_free_reason_max] =
{
    BLOG_ARY_INIT(blog_free_reason_unknown) /* unknown or customer defined */
    BLOG_ARY_INIT(blog_free_reason_blog_emit)
    BLOG_ARY_INIT(blog_free_reason_blog_iq_prio)
    BLOG_ARY_INIT(blog_free_reason_kfree)
    BLOG_ARY_INIT(blog_free_reason_ipmr_local)
};

blog_ctx_t blog_ctx_g, *blog_ctx_p = &blog_ctx_g;
EXPORT_SYMBOL(blog_ctx_p);

/* *****  Blog Device Interface statistics related functions **** START *** */
/*
 *------------------------------------------------------------------------------
 * Function Name: blog_fold_bstats_stats64
 * Description  : Helper function to fold BlogStats_t into stats64 structure.
 * Returns      : void
 *------------------------------------------------------------------------------
 */
static void blog_fold_bstats_stats64(struct rtnl_link_stats64 *stats64_p,
                                     const BlogStats_t * const bStats_p)
{
    stats64_p->rx_packets += bStats_p->rx_packets;
    stats64_p->tx_packets += bStats_p->tx_packets;
    stats64_p->rx_bytes   += bStats_p->rx_bytes;
    stats64_p->tx_bytes   += bStats_p->tx_bytes;
    stats64_p->multicast  += bStats_p->multicast;
#if defined(CONFIG_BCM_KF_EXTSTATS)	
    stats64_p->tx_multicast_packets += bStats_p->tx_multicast_packets;
    stats64_p->rx_multicast_bytes   += bStats_p->rx_multicast_bytes;
    stats64_p->tx_multicast_bytes   += bStats_p->tx_multicast_bytes;
#endif	
    return;
}
static inline void blog_exclude_mcast(BlogStats_t * const bStats_p)
{
    /* Rollover ?? */
    bStats_p->rx_packets -= bStats_p->multicast;        /* subtract RX multicast pkts */
    bStats_p->rx_bytes -= bStats_p->rx_multicast_bytes; /* subtract RX multicast bytes */
    bStats_p->multicast = 0;                            /* Zero out RX multicast pkts */
    bStats_p->rx_multicast_bytes = 0;                   /* Zero out RX multicast bytes */

    bStats_p->tx_packets -= bStats_p->tx_multicast_packets; /* subtract TX multicast pkts */
    bStats_p->tx_bytes -= bStats_p->tx_multicast_bytes;     /* subtract TX multicast bytes */
    bStats_p->tx_multicast_packets = 0;                     /* Zero out TX multicast pkts */
    bStats_p->tx_multicast_bytes = 0;                       /* Zero out TX multicast bytes */
}
static inline void blog_exclude_ucast(BlogStats_t * const bStats_p)
{
    bStats_p->rx_packets = bStats_p->multicast;         /* only count RX multicast pkt */
    bStats_p->rx_bytes = bStats_p->rx_multicast_bytes;  /* only count RX multicast bytes */
    bStats_p->tx_packets = bStats_p->tx_multicast_packets;  /* only count TX multicast pkt */
    bStats_p->tx_bytes = bStats_p->tx_multicast_bytes;      /* only count TX multicast bytes */
}
/*
 *------------------------------------------------------------------------------
 * Function Name: blog_get_dev_specific_stats
 * Description  : Helper function to nullify stats that device is not interested in.
 * Returns      : void
 *------------------------------------------------------------------------------
 */
static void blog_get_dev_specific_stats(BlogStats_t * const bStats_sw_p,
                                        BlogStats_t * const bStats_hw_p,
                                        unsigned int stats_flags)
{
    if ( !(stats_flags & BLOG_DEV_STAT_FLAG_INCLUDE_SW_MC) ) {
        blog_exclude_mcast(bStats_sw_p);
    }
    /* NOTE : Order of Multicast followed by unicast is important */
    if ( !(stats_flags & BLOG_DEV_STAT_FLAG_INCLUDE_SW_UC) ) {
        blog_exclude_ucast(bStats_sw_p);
    }
    if ( !(stats_flags & BLOG_DEV_STAT_FLAG_INCLUDE_HW_MC) ) {
        blog_exclude_mcast(bStats_hw_p);
    }
    /* NOTE : Order of Multicast followed by unicast is important */
    if ( !(stats_flags & BLOG_DEV_STAT_FLAG_INCLUDE_HW_UC) ) {
        blog_exclude_ucast(bStats_hw_p);
    }
    return;
}
/*
 *------------------------------------------------------------------------------
 * Function Name: blog_fold_stats
 * Description  : Helper function to fold one BlogStats_t into another.
 * Returns      : void
 *------------------------------------------------------------------------------
 */
static void blog_fold_stats(BlogStats_t * const d,
                            const BlogStats_t * const s)
{
    /* Assumption - all the variables of BlogStats_t are of same size */
	int i;
    int n = sizeof(BlogStats_t) / sizeof(uint64_t);
	const uint64_t *const src = (const uint64_t *const)s;
	uint64_t *dst = (uint64_t *)d;

	BUILD_BUG_ON(sizeof(uint64_t) != sizeof(d->rx_packets));

	for (i = 0; i < n; i++)
		dst[i] += src[i];
    return;
}
/*
 *------------------------------------------------------------------------------
 * Function Name: blog_put_dev_stats
 * Description  : Function to accumulate stats from flow-cache.
 * Returns      : void
 *------------------------------------------------------------------------------
 */
static void blog_put_dev_stats(struct net_device *dev_p, 
                               BlogStats_t * const bStats_sw_p,
                               BlogStats_t * const bStats_hw_p)
{
    if ( dev_p && (dev_p->blog_stats_flags & BLOG_DEV_STAT_FLAG_INCLUDE_ALL) ) { /* */
        blog_get_dev_specific_stats(bStats_sw_p, bStats_hw_p, dev_p->blog_stats_flags);
        blog_fold_stats(&dev_p->blog_stats, bStats_sw_p); /* Fold SW stats */
        blog_fold_stats(&dev_p->blog_stats, bStats_hw_p); /* Fold HW stats */
    }
    return;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: blog_notify_callback
 * Description  : callback function used for fetch_netif_stats
 *------------------------------------------------------------------------------
 */
static void blog_notify_callback(void *data)
{
    BLOG_WAKEUP_WORKER_THREAD((wq_info_t *)data, BLOG_WORK_AVAIL);
}

int blog_preemptible_task(void)
{
    int preempt_offset = 0;
	int nested = (preempt_count() & ~PREEMPT_ACTIVE) + rcu_preempt_depth();

    /* Is it preemptible? */
    if (preemptible() && (nested == preempt_offset))
        return 1;
    else 
        return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: blog_notify_async_wait
 * Description  : Calls blog_notify_async() and then waits for completion of 
 *                event. No callback function needed from the caller, this 
 *                function uses its own callback function.
 * Note         : If called from NOT preempt-safe context, this function will
 *                change blog_notify_async() to blog_notify(), which means the
 *                event is processed synchronously.
 *------------------------------------------------------------------------------
 */
void blog_notify_async_wait(BlogNotify_t event, void *net_p,
        unsigned long param1, unsigned long param2)
{
    int ret_val;
    wq_info_t   blog_notify_thread; /* blog_notify_async() caller thread */
    blog_notify_thread.work_avail = 0;
    init_waitqueue_head(&blog_notify_thread.wqh);

    if (blog_preemptible_task())
    {
        blog_lock();
        ret_val = blog_notify_async(event, net_p, param1, param2,
                blog_notify_callback, &blog_notify_thread);
        blog_unlock();
        if (ret_val)
        {
            do
            {
                wait_event_interruptible(blog_notify_thread.wqh,
                        blog_notify_thread.work_avail);
            } while (!(blog_notify_thread.work_avail & BLOG_WORK_AVAIL));
            blog_notify_thread.work_avail &= (~BLOG_WORK_AVAIL);
        }
    }
    else
    {
        /* event called from NOT Preempt Safe context!!!
         * switching to blog_notify() */
        blog_lock();
        blog_notify(event, net_p, param1, param2);
        blog_unlock();
    }
}

/*
 *------------------------------------------------------------------------------
 * Function Name: blog_get_dev_running_stats
 * Description  : Exported Accessor function to fetch running stats from flow-cache.
 * Returns      : void
 *------------------------------------------------------------------------------
 */
static void _blog_get_dev_running_stats(struct net_device *dev_p, BlogStats_t * const bStats_p)
{
    if ( dev_p && (dev_p->blog_stats_flags & BLOG_DEV_STAT_FLAG_INCLUDE_ALL) ) { /* */
        BlogStats_t sw_bStats = {0};
        BlogStats_t hw_bStats = {0};

        blog_notify_async_wait(FETCH_NETIF_STATS, (void*)dev_p,
                    (unsigned long)&sw_bStats, (unsigned long)&hw_bStats);
        blog_get_dev_specific_stats(&sw_bStats, &hw_bStats, dev_p->blog_stats_flags);
        blog_fold_stats(bStats_p, &sw_bStats); /* Fold SW stats */
        blog_fold_stats(bStats_p, &hw_bStats); /* Fold HW stats */
    }
    return;
}
void blog_get_dev_running_stats(void *dev_p, void * const bStats_p)
{
    _blog_get_dev_running_stats(dev_p, bStats_p);
}
EXPORT_SYMBOL(blog_get_dev_running_stats);

/* Remove once WLAN falls in place */
void blog_get_dev_running_stats_wlan(void *dev_p, void * const bStats_p)
{
    if ( dev_p ) { /* */
        BlogStats_t sw_bStats = {0};
        BlogStats_t hw_bStats = {0};

        blog_notify_async_wait(FETCH_NETIF_STATS, (void*)dev_p,
                    (unsigned long)&sw_bStats, (unsigned long)&hw_bStats);
        blog_fold_stats((BlogStats_t*)bStats_p, &sw_bStats); /* Fold SW stats */
        blog_fold_stats((BlogStats_t*)bStats_p, &hw_bStats); /* Fold HW stats */
    }
}
EXPORT_SYMBOL(blog_get_dev_running_stats_wlan);
/*
 *------------------------------------------------------------------------------
 * Function Name: blog_add_dev_accelerated_stats
 * Description  : Exported Accessor function to add acclerated stats into dev stats64.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
static void _blog_add_dev_accelerated_stats(struct net_device *dev_p, struct rtnl_link_stats64 *stats64_p)
{
    if ( dev_p && (dev_p->blog_stats_flags & BLOG_DEV_STAT_FLAG_INCLUDE_ALL) ) { /* */
        BlogStats_t bStats = {0};
        blog_get_dev_stats(dev_p, &bStats);
        blog_fold_bstats_stats64(stats64_p, &bStats);
    }
}
void blog_add_dev_accelerated_stats(void *dev_p, void *stats64_p)
{
    _blog_add_dev_accelerated_stats(dev_p,stats64_p);
}
EXPORT_SYMBOL(blog_add_dev_accelerated_stats);
/*
 *------------------------------------------------------------------------------
 * Function Name: blog_get_dev_stats
 * Description  : Exported Accessor function to get accelerated blog stats.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
static void _blog_get_dev_stats(struct net_device *dev_p, BlogStats_t * bStats_p)
{
    if ( dev_p && (dev_p->blog_stats_flags & BLOG_DEV_STAT_FLAG_INCLUDE_ALL) ) { /* */
        blog_get_dev_running_stats(dev_p, bStats_p);
        blog_fold_stats(bStats_p, &dev_p->blog_stats);
    }
}
void blog_get_dev_stats(void *dev_p, void *bStats_p)
{
    _blog_get_dev_stats(dev_p, bStats_p);
}
EXPORT_SYMBOL(blog_get_dev_stats);

/*
 *------------------------------------------------------------------------------
 * Function Name: blog_clr_dev_stats
 * Description  : Exported Accessor function to clear accelerated blog stats.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
void _blog_clr_dev_stats(struct net_device *dev_p)
{
    blog_notify_async_wait(CLEAR_NETIF_STATS, (void*)dev_p, (unsigned long)NULL, 
            (unsigned long)NULL);
    /* Now clear the accumulated stats;
     * As part of clear, flow-cache flushes the current stats in dev->blog_stats */
    memset(&dev_p->blog_stats, 0, sizeof(dev_p->blog_stats));
}
void blog_clr_dev_stats(void *dev_p)
{
    _blog_clr_dev_stats(dev_p);
}
EXPORT_SYMBOL(blog_clr_dev_stats);

/* *****  Blog Device Interface statistics related functions **** END *** */



/*
 *------------------------------------------------------------------------------
 * Blog_t Free Pool Management.
 * The free pool of Blog_t is self growing (extends upto an engineered
 * value). Could have used a kernel slab cache. 
 *------------------------------------------------------------------------------
 */

/* Global pointer to the free pool of Blog_t */
static Blog_t * blog_list_gp = BLOG_NULL;

static int blog_extends = 0;        /* Extension of Pool on depletion */
#if defined(CC_BLOG_SUPPORT_DEBUG)
static int blog_cnt_free = 0;       /* Number of Blog_t free */
static int blog_cnt_used = 0;       /* Number of in use Blog_t */
static int blog_cnt_hwm  = 0;       /* In use high water mark for engineering */
static int blog_cnt_fails = 0;
#endif

/*
 *------------------------------------------------------------------------------
 * Function   : blog_extend
 * Description: Create a pool of Blog_t objects. When a pool is exhausted
 *              this function may be invoked to extend the pool. The pool is
 *              identified by a global pointer, blog_list_gp. All objects in
 *              the pool chained together in a single linked list.
 * Parameters :
 *   num      : Number of Blog_t objects to be allocated.
 * Returns    : Number of Blog_t objects allocated in pool.
 *
 * CAUTION: blog_extend must be called with blog_pool_lock_g acquired.
 *------------------------------------------------------------------------------
 */
uint32_t blog_extend( uint32_t num )
{
    register int i;
    register Blog_t * list_p;

    blog_print( "%u", num );

    list_p = (Blog_t *) kmalloc( num * sizeof(Blog_t), GFP_ATOMIC);
    if ( list_p == BLOG_NULL )
    {
        blog_ctx_p->blog_mem_fails++;
#if defined(CC_BLOG_SUPPORT_DEBUG)
        blog_cnt_fails++;
#endif
        blog_print( "WARNING: Failure to initialize %d Blog_t", num );
        return 0;
    }

    /* memset( (void *)list_p, 0, (sizeof(Blog_t) * num ); */
    for ( i = 0; i < num; i++ )
        list_p[i].blog_p = &list_p[i+1];

    blog_extends++;
    blog_ctx_p->blog_extends++;

    blog_ctx_p->blog_total += num;
    blog_ctx_p->blog_avail += num;
    BLOG_DBG( blog_cnt_free += num; );
    list_p[num-1].blog_p = blog_list_gp; /* chain last Blog_t object */
    blog_list_gp = list_p;  /* Head of list */

    return num;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _fast_memset
 * Description  : sets the memory starting from dst_p to val.
 * Note         : dst_p should be atleast 32-bit aligned, and len is in bytes
 *------------------------------------------------------------------------------
 */
static inline 
void _fast_memset( uint32_t *dst_p, uint32_t val, uint32_t len )
{
    int num_words = len >> 2;
    int num_bytes = len & 3;
    uint8_t *byte_p;
    int i;

    for( i=0; i < num_words; i++ )
        *dst_p++ = val;

    if (num_bytes)
    {
        byte_p = (uint8_t *) dst_p;
        for( i=0; i < num_bytes; i++ )
            *byte_p++ = val;
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_clr
 * Description  : Clear the data of a Blog_t
 *                Need not be protected by blog_pool_lock_g
 *------------------------------------------------------------------------------
 */
static inline void blog_clr( Blog_t * blog_p )
{
    blog_assertv( ((blog_p != BLOG_NULL) && (_IS_BPTR_(blog_p))) );
    BLOG_DBG( memset( (void*)blog_p, 0, sizeof(Blog_t) ); );

    _fast_memset( (void*)blog_p, 0, sizeof(Blog_t) );

    /* clear phyHdr, count, bmap, and channel */
    blog_p->minMtu = BLOG_ETH_MTU_LEN;
    blog_p->vtag[0] = 0xFFFFFFFF;
    blog_p->vtag[1] = 0xFFFFFFFF;
    blog_p->wl = 0; /* Clear the WL-METADATA */
    blog_p->dpi_queue = ~0;
    blog_p->spdtst = 0;

    blog_print( "blog<%p>", blog_p );
}

void blog_ct_get(Blog_t * blog_p)
{
#if defined(BLOG_NF_CONNTRACK)
    int i;
    for(i=0; i<BLOG_CT_MAX; i++)
    {
        if(blog_p->ct_p[i])
            nf_conntrack_get(&((struct nf_conn *)blog_p->ct_p[i])->ct_general);
    }
    blog_p->nf_ct_skip_ref_dec=0;
#endif
}
EXPORT_SYMBOL(blog_ct_get);

void blog_ct_put(Blog_t * blog_p)
{
#if defined(BLOG_NF_CONNTRACK)
    if(!blog_p->nf_ct_skip_ref_dec)
    {
        int i;

        blog_p->nf_ct_skip_ref_dec=1;
        /*decrement refcnt of any associated conntrack's */
        for(i=0; i<BLOG_CT_MAX; i++)
        {
            if(blog_p->ct_p[i])
            {
                nf_conntrack_put(&((struct nf_conn *)blog_p->ct_p[i])->ct_general);
                blog_p->ct_p[i] = NULL;
            }
        }

    }
#endif
}
EXPORT_SYMBOL(blog_ct_put);
/*
 *------------------------------------------------------------------------------
 * Function     : blog_get
 * Description  : Allocate a Blog_t from the free list
 * Returns      : Pointer to an Blog_t or NULL, on depletion.
 *------------------------------------------------------------------------------
 */
Blog_t * blog_get( void )
{
    register Blog_t * blog_p;
    unsigned long lock_flags;

    BLOG_POOL_LOCK();   /* DO NOT USE blog_assertr() until BLOG_POOL_UNLOCK() */

    if ( blog_list_gp == BLOG_NULL )
    {
#ifdef CC_BLOG_SUPPORT_EXTEND
        if ( (blog_extends >= BLOG_EXTEND_MAX_ENGG)/* Try extending free pool */
          || (blog_extend( BLOG_EXTEND_SIZE_ENGG ) != BLOG_EXTEND_SIZE_ENGG))
        {
            blog_ctx_p->blog_extend_fails++;
            blog_print( "WARNING: free list exhausted" );
        }
#else
        if ( blog_extend( BLOG_EXTEND_SIZE_ENGG ) == 0 )
        {
            blog_print( "WARNING: out of memory" );
        }
#endif
        if (blog_list_gp == BLOG_NULL)
        {
            blog_p = BLOG_NULL;
            BLOG_POOL_UNLOCK(); /* May use blog_assertr() now onwards */
            goto blog_get_return;
        }
    }

    blog_ctx_p->info_stats.blog_get++;;
    blog_ctx_p->blog_avail--;
    if ( unlikely(blog_ctx_p->info_stats.blog_min_avail > blog_ctx_p->blog_avail) )
        blog_ctx_p->info_stats.blog_min_avail = blog_ctx_p->blog_avail;

    BLOG_DBG(
        blog_cnt_free--;
        if ( ++blog_cnt_used > blog_cnt_hwm )
            blog_cnt_hwm = blog_cnt_used;
        );

    blog_p = blog_list_gp;
    blog_list_gp = blog_list_gp->blog_p;

    BLOG_POOL_UNLOCK();     /* May use blog_assertr() now onwards */

    blog_clr( blog_p );     /* quickly clear the contents */

blog_get_return:

    blog_print( "blog<%p>", blog_p );

    return blog_p;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_put
 * Description  : Release a Blog_t back into the free pool
 * Parameters   :
 *  blog_p      : Pointer to a non-null Blog_t to be freed.
 *------------------------------------------------------------------------------
 */
void blog_put( Blog_t * blog_p )
{
    unsigned long lock_flags;

    blog_assertv( ((blog_p != BLOG_NULL) && (_IS_BPTR_(blog_p))) );

    blog_ct_put(blog_p);

#if defined(CONFIG_XFRM)
    if (TX_ESP(blog_p))
    {
        dst_release(blog_p->esptx.dst_p);
        secpath_put(blog_p->esptx.secPath_p);
    }
#endif

    blog_clr( blog_p );

    BLOG_POOL_LOCK();   /* DO NOT USE blog_assertv() until BLOG_POOL_UNLOCK() */

    blog_ctx_p->blog_avail++;
    BLOG_DBG( blog_cnt_used--; blog_cnt_free++; );
    blog_p->blog_p = blog_list_gp;  /* clear pointer to skb_p */
    blog_list_gp = blog_p;          /* link into free pool */

    BLOG_POOL_UNLOCK();/* May use blog_assertv() now onwards */

    blog_ctx_p->info_stats.blog_put++;;
    blog_print( "blog<%p>", blog_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_skb
 * Description  : Allocate and associate a Blog_t with an sk_buff.
 * Parameters   :
 *  skb_p       : pointer to a non-null sk_buff
 * Returns      : A Blog_t object or NULL,
 *------------------------------------------------------------------------------
 */
Blog_t * blog_skb( struct sk_buff * skb_p )
{
    blog_assertr( (skb_p != (struct sk_buff *)NULL), BLOG_NULL );
    blog_assertr( (!_IS_BPTR_(skb_p->blog_p)), BLOG_NULL ); /* avoid leak */

    skb_p->blog_p = blog_get(); /* Allocate and associate with sk_buff */

    blog_print( "skb<%p> blog<%p>", skb_p, skb_p->blog_p );

    /* CAUTION: blog_p does not point back to the skb, do it explicitly */
    return skb_p->blog_p;       /* May be null */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_fkb
 * Description  : Allocate and associate a Blog_t with an fkb.
 * Parameters   :
 *  fkb_p       : pointer to a non-null FkBuff_t
 * Returns      : A Blog_t object or NULL,
 *------------------------------------------------------------------------------
 */
Blog_t * blog_fkb( struct fkbuff * fkb_p )
{
    uint32_t in_skb_tag;
    blog_assertr( (fkb_p != (FkBuff_t *)NULL), BLOG_NULL );
    blog_assertr( (!_IS_BPTR_(fkb_p->blog_p)), BLOG_NULL ); /* avoid leak */

    in_skb_tag = _is_in_skb_tag_( fkb_p->ptr, fkb_p->flags );

    fkb_p->blog_p = blog_get(); /* Allocate and associate with fkb */

    if ( fkb_p->blog_p != BLOG_NULL )   /* Move in_skb_tag to blog rx info */
        fkb_p->blog_p->rx.fkbInSkb = in_skb_tag;

    blog_print( "fkb<%p> blog<%p> in_skb_tag<%u>",
                fkb_p, fkb_p->blog_p, in_skb_tag );
    return fkb_p->blog_p;       /* May be null */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_snull
 * Description  : Dis-associate a sk_buff with any Blog_t
 * Parameters   :
 *  skb_p       : Pointer to a non-null sk_buff
 * Returns      : Previous Blog_t associated with sk_buff
 *------------------------------------------------------------------------------
 */
inline Blog_t * _blog_snull( struct sk_buff * skb_p )
{
    register Blog_t * blog_p;
    blog_p = skb_p->blog_p;
    skb_p->blog_p = BLOG_NULL;
    return blog_p;
}

Blog_t * blog_snull( struct sk_buff * skb_p )
{
    blog_assertr( (skb_p != (struct sk_buff *)NULL), BLOG_NULL );
    blog_print( "skb<%p> blog<%p>", skb_p, skb_p->blog_p );
    return _blog_snull( skb_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_fnull
 * Description  : Dis-associate a fkbuff with any Blog_t
 * Parameters   :
 *  fkb_p       : Pointer to a non-null fkbuff
 * Returns      : Previous Blog_t associated with fkbuff
 *------------------------------------------------------------------------------
 */
inline Blog_t * _blog_fnull( struct fkbuff * fkb_p )
{
    register Blog_t * blog_p;
    blog_p = fkb_p->blog_p;
    fkb_p->blog_p = BLOG_NULL;
    return blog_p;
}

Blog_t * blog_fnull( struct fkbuff * fkb_p )
{
    blog_assertr( (fkb_p != (struct fkbuff *)NULL), BLOG_NULL );
    blog_print( "fkb<%p> blogp<%p>", fkb_p,fkb_p->blog_p );
    return _blog_fnull( fkb_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : _blog_free
 * Description  : Free any Blog_t associated with a sk_buff
 * Parameters   :
 *  skb_p       : Pointer to a non-null sk_buff
 *------------------------------------------------------------------------------
 */
inline void _blog_free( struct sk_buff * skb_p )
{
    register Blog_t * blog_p;
    blog_p = _blog_snull( skb_p );   /* Dis-associate Blog_t from skb_p */
    if ( likely(blog_p != BLOG_NULL) )
        blog_put( blog_p );         /* Recycle blog_p into free list */
}


/*
 *------------------------------------------------------------------------------
 * Function     : blog_free
 * Description  : Free any Blog_t associated with a sk_buff
 * Parameters   :
 *  skb_p       : Pointer to a non-null sk_buff
 *  reason      : The reason/location why blog_free was called
 *------------------------------------------------------------------------------
 */
void blog_free( struct sk_buff * skb_p, blog_skip_reason_t reason )
{
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );

    if ( likely(skb_p->blog_p != BLOG_NULL) )
        blog_ctx_p->blog_free_stats_table[reason]++; 

    BLOG_DBG(
        if ( skb_p->blog_p != BLOG_NULL )
            blog_print( "skb<%p> blog<%p> [<%p>]",
                        skb_p, skb_p->blog_p,
                        __builtin_return_address(0) ); );
    _blog_free( skb_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_skip
 * Description  : Disable further tracing of sk_buff by freeing associated
 *                Blog_t (if any)
 * Parameters   :
 *  skb_p       : Pointer to a sk_buff
 *  reason      : The reason/location why blog_skip was called
 *------------------------------------------------------------------------------
 */
void blog_skip( struct sk_buff * skb_p, blog_skip_reason_t reason )
{
    blog_print( "skb<%p> [<%p>]",
                skb_p, __builtin_return_address(0) );

    if ( likely(skb_p->blog_p != BLOG_NULL) )
        blog_ctx_p->blog_skip_stats_table[reason]++; 

    blog_assertv( (skb_p != (struct sk_buff *)NULL) );
    _blog_free( skb_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_xfer
 * Description  : Transfer ownership of a Blog_t between two sk_buff(s)
 * Parameters   :
 *  skb_p       : New owner of Blog_t object 
 *  prev_p      : Former owner of Blog_t object
 *------------------------------------------------------------------------------
 */
void blog_xfer( struct sk_buff * skb_p, const struct sk_buff * prev_p )
{
    Blog_t * blog_p;
    struct sk_buff * mod_prev_p;
    blog_assertv( (prev_p != (struct sk_buff *)NULL) );
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );

    mod_prev_p = (struct sk_buff *) prev_p; /* const removal without warning */
    blog_p = _blog_snull( mod_prev_p );
    skb_p->blog_p = blog_p;

    if ( likely(blog_p != BLOG_NULL) )
    {
        blog_ctx_p->info_stats.blog_xfer++;
        blog_print( "skb<%p> to new<%p> blog<%p> [<%p>]",
                    prev_p, skb_p, blog_p,
                    __builtin_return_address(0) );
        blog_assertv( (_IS_BPTR_(blog_p)) );
        blog_p->skb_p = skb_p;
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_clone
 * Description  : Duplicate a Blog_t for another sk_buff
 * Parameters   :
 *  skb_p       : New owner of cloned Blog_t object 
 *  prev_p      : Blog_t object to be cloned
 *------------------------------------------------------------------------------
 */
void blog_clone( struct sk_buff * skb_p, const struct blog_t * prev_p )
{
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );

    if ( likely(prev_p != BLOG_NULL) )
    {
        Blog_t * blog_p;
        int      i;

        blog_assertv( (_IS_BPTR_(prev_p)) );
        
        skb_p->blog_p = blog_get(); /* Allocate and associate with skb */
        blog_p = skb_p->blog_p;

        blog_print( "orig blog<%p> new skb<%p> blog<%p> [<%p>]",
                    prev_p, skb_p, blog_p,
                    __builtin_return_address(0) );

        if ( likely(blog_p != BLOG_NULL) )
        {
            blog_ctx_p->info_stats.blog_clone++;;
            blog_p->skb_p = skb_p;
#define CPY(x) blog_p->x = prev_p->x
            CPY(key.match);
            CPY(hash);
            CPY(mark);
            CPY(priority);
            CPY(rx);
            CPY(rx_dev_p);
            CPY(vtag[0]);
            CPY(vtag[1]);
            CPY(vtag_num);
            CPY(tupleV6);
            CPY(tuple_offset);
            CPY(mega_p);
            CPY(map_p);

            if(!prev_p->nf_ct_skip_ref_dec)
            {
                for(i=0; i<BLOG_CT_MAX; i++)
                {
                    blog_p->ct_p[i] = prev_p->ct_p[i];
                }

                /*increment refcnt of ct's */
                blog_ct_get(blog_p);
            }

            for(i=0; i<MAX_VIRT_DEV; i++)
            {
               if( prev_p->virt_dev_p[i] )
               {
                  blog_p->virt_dev_p[i] = prev_p->virt_dev_p[i];
                  blog_p->delta[i] = prev_p->delta[i];
               }
               else
                  break;
            }
            blog_p->tx.word = 0;
        }
    }
#undef CPY
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_copy
 * Description  : Copy a Blog_t object another blog object.
 * Parameters   :
 *  new_p       : Blog_t object to be filled in
 *  prev_p      : Blog_t object with the data information
 *------------------------------------------------------------------------------
 */
void blog_copy(struct blog_t * new_p, const struct blog_t * prev_p)
{
    blog_assertv( (new_p != BLOG_NULL) );
    blog_print( "new_p<%p> prev_p<%p>", new_p, prev_p );

    if ( likely(prev_p != BLOG_NULL) )
    {
        blog_ctx_p->info_stats.blog_copy++;
        memcpy( new_p, prev_p, sizeof(Blog_t) );

        if(!prev_p->nf_ct_skip_ref_dec)
        {
            /*increment refcnt of ct's */
            blog_ct_get(new_p);
        }
    }
}


/*
 *------------------------------------------------------------------------------
 * Function     : blog_iq
 * Description  : get the iq prio from blog
 * Parameters   :
 *  skb_p       : Pointer to a sk_buff
 *------------------------------------------------------------------------------
 */
int blog_iq( const struct sk_buff * skb_p )
{
    int prio = BLOG_IQ_PRIO_LOW;

    blog_print( "skb<%p> [<%p>]",
                skb_p, __builtin_return_address(0) );

    if (skb_p)
    {
        Blog_t *blog_p = skb_p->blog_p;

        if (blog_p)
            prio = blog_p->iq_prio;
    }
    return prio;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_fc_enabled
 * Description  : get the enabled/disabled status of flow cache
 * Parameters   :
 *  none        :
 *------------------------------------------------------------------------------
 */
int blog_fc_enabled( void )
{
    if ( likely(blog_rx_hook_g != (BlogDevRxHook_t)NULL) )
        return 1;
    else
        return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_gre_tunnel_accelerated
 * Description  : get the accelerated status of GRE tunnels
 * Parameters   :
 *  none        :
 *------------------------------------------------------------------------------
 */

inline int blog_gre_tunnel_accelerated( void )
{
    return blog_gre_tunnel_accelerated_g;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_l2tp_tunnel_accelerated
 * Description  : get the accelerated status of L2TP tunnels
 * Parameters   :
 *  none        :
 *------------------------------------------------------------------------------
 */

inline int blog_l2tp_tunnel_accelerated( void )
{
    return blog_l2tp_tunnel_accelerated_g;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_link
 * Description  : Associate a network entity with an skb's blog object
 * Parameters   :
 *  entity_type : Network entity type
 *  blog_p      : Pointer to a Blog_t
 *  net_p       : Pointer to a network stack entity 
 *  param1      : optional parameter 1
 *  param2      : optional parameter 2
 * PreRequisite : acquire blog_lock_g before calling blog_link()
 *------------------------------------------------------------------------------
 */
void blog_link( BlogNetEntity_t entity_type, Blog_t * blog_p,
                void * net_p, uint32_t param1, uint32_t param2 )
{
    if ( unlikely(blog_p == BLOG_NULL) )
        return;

    blog_assertv( (entity_type < BLOG_NET_ENTITY_MAX) );
    blog_assertv( (net_p != (void *)NULL) );
    blog_assertv( (_IS_BPTR_(blog_p)) );

    blog_print( "link<%s> skb<%p> blog<%p> net<%p> %u %u [<%p>]",
                strBlogNetEntity[entity_type], blog_p->skb_p, blog_p,
                net_p, param1, param2, __builtin_return_address(0) );

    switch ( entity_type )
    {
        case FLOWTRACK:
        {
#if defined(BLOG_NF_CONNTRACK)
            uint32_t idx = BLOG_CT_PLD;

            blog_assertv( ((param1 == BLOG_PARAM1_DIR_ORIG) ||
                           (param1 == BLOG_PARAM1_DIR_REPLY)||
                           (param2 == BLOG_PARAM2_IPV4)     ||
                           (param2 == BLOG_PARAM2_IPV6)     ||
                           (param2 == BLOG_PARAM2_GRE_IPV4)) );

            if ( unlikely(blog_p->rx.multicast) )
                return;

            switch (param2)
            {
                case BLOG_PARAM2_IPV4:
                    if (RX_IPV4_DEL(blog_p) && (blog_p->ct_p[BLOG_CT_DEL] == NULL))
                            idx = BLOG_CT_DEL;
                    else if (RX_IPV4(blog_p) && (blog_p->ct_p[BLOG_CT_PLD] == NULL))
                        idx = BLOG_CT_PLD;
                    else if (RX_IPV4(blog_p) 
#if defined(CONFIG_BCM_KF_MAP) || defined(CONFIG_BCM_MAP_MODULE)
                            || blog_p->map_p
#endif
                            )
                        idx = BLOG_CT_PLD;
                    else if (blog_p->ct_p[BLOG_CT_DEL] == NULL)
                        idx = BLOG_CT_DEL;
                    else
                        blog_print( "invalid param2 %u", param2 );
                    break;

                case BLOG_PARAM2_IPV6:
                    if (RX_IPV6_DEL(blog_p) && (blog_p->ct_p[BLOG_CT_DEL] == NULL))
                        idx = BLOG_CT_DEL;
                    else if (RX_IPV6(blog_p) && (blog_p->ct_p[BLOG_CT_PLD] == NULL))
                        idx = BLOG_CT_PLD;
                    else if (RX_IPV6(blog_p)
#if defined(CONFIG_BCM_KF_MAP) || defined(CONFIG_BCM_MAP_MODULE)
                            || blog_p->map_p
#endif
                            )
                         idx = BLOG_CT_PLD;
                    else if (blog_p->ct_p[BLOG_CT_DEL] == NULL)
                        idx = BLOG_CT_DEL;
                    else
                        blog_print( "invalid param2 %u", param2 );
                    break;

                case BLOG_PARAM2_GRE_IPV4:
                        idx = BLOG_CT_DEL;
                    break;

                case BLOG_PARAM2_L2TP_IPV4:
                    if (RX_IPV4_DEL(blog_p) && (blog_p->ct_p[BLOG_CT_DEL] == NULL))                         
                        idx = BLOG_CT_DEL;
                    else if (RX_IPV4(blog_p))
                    {
                        if(blog_p->ct_p[BLOG_CT_PLD] == NULL)
                            idx = BLOG_CT_PLD;                        
                            
                        if(blog_p->ct_p[BLOG_CT_PLD] != NULL && blog_p->ct_p[BLOG_CT_DEL] == NULL)
                            idx = BLOG_CT_DEL;												
                    }
                    break;
                default:
                    blog_print( "unknown param2 %u", param2 );
                    return;
            }

            if(blog_p->ct_p[idx])
            {
                /*ct already exists decrement it's ref count */
                nf_conntrack_put(&((struct nf_conn *)blog_p->ct_p[idx])->ct_general);
                printk(KERN_WARNING "blog_link: overwriting ct_p=%px, new_ct=%px at idx=%d\n",
                        blog_p->ct_p[idx], net_p, idx);
            }

            blog_p->ct_p[idx] = net_p; /* Pointer to conntrack */
            /*increment refcount of ct, till flow is learned */
            nf_conntrack_get(&((struct nf_conn *)net_p)->ct_general);

            /* param2 indicates the ct_p belongs to IPv4 or IPv6 */
            blog_p->ct_ver[idx] = (param2 == BLOG_PARAM2_GRE_IPV4) ?
                                                BLOG_PARAM2_IPV4 : param2;
            /* 
             * Save flow direction
             */
            if(idx == BLOG_CT_PLD)
                blog_p->nf_dir_pld = param1;
            else
                blog_p->nf_dir_del = param1;


            blog_print( "idx<%d> ct_p<%p> ct_ver<%d>\n",
                    idx, blog_p->ct_p[idx], blog_p->ct_ver[idx] );

#endif
            break;
        }

        case BRIDGEFDB:
        {
           blog_assertv( ((param1 == BLOG_PARAM1_SRCFDB) ||
                           (param1 == BLOG_PARAM1_DSTFDB)) );

           /* Note:- In bridging mode, dest bridge FDB pointer could be NULL if
            * the dest MAC has not been learned yet */  
           if ( unlikely(net_p != (void *) NULL) )
           {
                uint8_t *mac_p;
                struct net_bridge_fdb_entry * fdb_p 
                        = (struct net_bridge_fdb_entry *)net_p;
 
                if ( param1 == BLOG_PARAM1_SRCFDB)
                    mac_p = (uint8_t *) &blog_p->src_mac;
                else
                    mac_p = (uint8_t *) &blog_p->dst_mac;

                /* copy the MAC from Bridge FDB to blog */
                memcpy(mac_p, fdb_p->addr.addr, BLOG_ETH_ADDR_LEN);
            }

            blog_p->fdb[param1] = net_p;
            break;
        }

        case MCAST_FDB:
        {
            blog_p->mc_fdb = net_p; /* Pointer to mc_fdb */
            break;
        }

        case IF_DEVICE: /* link virtual interfaces traversed by flow */
        case IF_DEVICE_MCAST:
        {
            int i;

            blog_assertv( (param1 < BLOG_DIR_MAX) );

            for (i=0; i<MAX_VIRT_DEV; i++)
            {
                /* A flow should not rx and tx with the same device!!  */
                blog_assertv((net_p != DEVP_DETACH_DIR(blog_p->virt_dev_p[i])));

                if ( blog_p->virt_dev_p[i] == NULL )
                {
                    blog_p->virt_dev_p[i] = DEVP_APPEND_DIR(net_p, param1);
                    if (IF_DEVICE_MCAST == entity_type )
                    {
                       blog_p->delta[i] = -(param2 & 0xFF);
                    }
                    else
                    {
                       blog_p->delta[i] = (param2 - blog_p->tx.pktlen) & 0xFF;
                    }
                    break;
                }
            }

            blog_assertv( (i != MAX_VIRT_DEV) );
            break;
        }

        case GRE_TUNL:
        {
            if (param1 == DIR_RX) 
                blog_p->rx_tunl_p = net_p; /* Pointer to tunnel */
            else
                blog_p->tx_tunl_p = net_p; /* Pointer to tunnel */

            break;
        }

        case TOS_MODE:
        {
            if (param1 == DIR_RX) 
                blog_p->tos_mode_ds = param2;
            else
                blog_p->tos_mode_us = param2;

            break;
        }

#if defined(CONFIG_BCM_KF_MAP) || defined(CONFIG_BCM_MAP_MODULE)
        case MAP_TUPLE:
        {
            blog_assertv( ((param1 == BLOG_PARAM1_MAP_DIR_US) ||
                          (param1 == BLOG_PARAM1_MAP_DIR_DS)) );

            blog_print( "b4 multicast check" );
            if ( unlikely(blog_p->rx.multicast) )
                return;

            blog_p->map_p = net_p; /* Pointer to MAPT tuple */
            blog_print( "map_p<%p> net<%p>", blog_p->map_p, net_p );

            break;
        }
#endif

#if defined(CONFIG_BCM_OVS)
        case MEGA:
        {
            blog_p->mega_p = (uint32_t *) net_p; /* Pointer to megaflow */
            blog_print( "mega_p<%p> net<%p>", blog_p->mega_p, net_p );
            break;
        }
#endif

        default:
            break;
    }
    return;
}

blog_notify_evt_enqueue_hook_t blog_notify_evt_enqueue_hook_g = NULL;

/*
 *------------------------------------------------------------------------------
 * Function     : blog_bind_notify_evt_enqueue
 * Description  : Bind the blog notify event enqueue function to be called
 *------------------------------------------------------------------------------
 */
void blog_bind_notify_evt_enqueue( 
        blog_notify_evt_enqueue_hook_t blog_notify_evt_enqueue_fn )
{
    printk( "Bind blog_notify_evt_enqueue_fn[<%p>]\n", 
            blog_notify_evt_enqueue_fn );
    blog_notify_evt_enqueue_hook_g = blog_notify_evt_enqueue_fn;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: blog_notify_evt_enqueue
 * Description  : Prepare and enqueue event work
 *------------------------------------------------------------------------------
 */
void blog_notify_evt_enqueue(blog_notify_evt_type_t evt_type, void *net_p, 
        unsigned long param1, unsigned long param2,
        blog_notify_async_cb_fn_t notify_cb_fn, void *notify_cb_data_p)
{
    if (blog_notify_evt_enqueue_hook_g)
        blog_notify_evt_enqueue_hook_g(evt_type, net_p, param1, param2,
            notify_cb_fn, notify_cb_data_p);
}

/*
 *------------------------------------------------------------------------------
 * Function     : _blog_notify_async
 * Description  : Notify a Blog client (xx_hook) of an event.
 * Parameters   :
 *  blog_notify_api: blog_notify() or blog_notify_async()
 *  event       : notification
 *  net_p       : Pointer to a network stack entity
 *  param1      : optional parameter 1
 *  param2      : optional parameter 2
 * PreRequisite : acquire blog_lock_g before calling blog_notify/blog_notify_async()
 * return value : Caller MUST check the return value.
 *              : 1 = caller's notify callback function will be called
 *              : 0 = caller's notify callback function will NOT be called.
 *              :   Situations where fc_evt task is not running.
 *------------------------------------------------------------------------------
 */
static inline int _blog_notify_async(blog_notify_api_t blog_notify_api, 
        BlogNotify_t event, void *net_p,
        unsigned long param1, unsigned long param2,
        blog_notify_async_cb_fn_t notify_cb_fn, void *notify_cb_data_p)
{
    int ret_val = 0;
    struct net_bridge_fdb_entry * fdb_p;

    blog_assertr( (event < BLOG_NOTIFY_MAX), 0 );
    blog_assertr( (blog_notify_api < BLOG_NOTIFY_API_MAX), 0 );
    blog_assertr( (net_p != (void *)NULL), 0 );

    blog_print( "blog_notify_api<%d> notify<%s> net_p<0x%p>"
                " param1<%lu:0x%lx> param2<%lu:0x%lx>"
                " notify_cb_fn<%p> notify_cb_data_p<%p> [<0x%p>]\n",
                blog_notify_api, strBlogNotify[event], 
                net_p, param1, param1, param2, param2,
                notify_cb_fn, notify_cb_data_p, 
                __builtin_return_address(0) );

    if (event == DESTROY_BRIDGEFDB)
    {
        fdb_p = (struct net_bridge_fdb_entry *)net_p;

        if (fdb_p->fdb_key == BLOG_FDB_KEY_INVALID)
            goto blog_notify_async_bypass;
    }

    if (unlikely(blog_xx_hook_g == (BlogNotifyHook_t)NULL) ||
        unlikely(blog_notify_evt_enqueue_hook_g == (blog_notify_evt_enqueue_hook_t) NULL))
    {   /* flow cache is disabled or fc_evt task is not running */
        if ( likely(blog_notify_evt_enqueue_hook_g))
        {
            /* fc_evt task is running. we can enqueue the events */
            if (notify_cb_fn)
                blog_notify_evt_enqueue(BLOG_NOTIFY_EVT_NONE, NULL, 0, 0,
                    notify_cb_fn, notify_cb_data_p);
            return 1; /* caller can wait, callback function will be invoked */
        }
        else
        return 0; /* caller should not wait, callback won't be invoked */
    }

    ret_val = blog_xx_hook_g( blog_notify_api, event, net_p, 
                    param1, param2, notify_cb_fn, notify_cb_data_p );

blog_notify_async_bypass:

#if defined(CONFIG_BCM_KF_WL)
    /* first flush the flows from Flow-cache/FAP and then clear the BRC_HOT */
    if (event == DESTROY_BRIDGEFDB) { /* for WLAN PKTC use */
        struct net_device * dev_p;

        fdb_p = (struct net_bridge_fdb_entry *)net_p;
        dev_p = ((fdb_p->dst == NULL) || (fdb_p->dst->dev == NULL)) ?
                 NULL : fdb_p->dst->dev;

        if ( likely(wl_pktc_del_hook != NULL) )
        {
            wl_pktc_del_hook((unsigned long)(fdb_p->addr.addr), dev_p);
        }

        if ( likely(dhd_pktc_del_hook != NULL) )
        { 
            dhd_pktc_del_hook((unsigned long)(fdb_p->addr.addr), dev_p);
        }
   }
#endif /* CONFIG_BCM_KF_WL */

    return ret_val;
}


/*
 *------------------------------------------------------------------------------
 * Function     : blog_notify()
 * Description  : blog_notify() synchronous API
 * Parameters   :
 *  event       : notification
 *  net_p       : Pointer to a network stack entity
 *  param1      : optional parameter 1
 *  param2      : optional parameter 2
 * PreRequisite : acquire blog_lock_g before calling blog_notify/blog_notify_async()
 * return value : Caller MUST check the return value.
 *              : 1 = caller's notify callback function will be called
 *              : 0 = caller's notify callback function will NOT be called.
 *              :   Situations where fc_evt task is not running.
 *------------------------------------------------------------------------------
 */

void blog_notify(BlogNotify_t event, void *net_p, unsigned long param1, unsigned long param2)
{
    _blog_notify_async(BLOG_NOTIFY_API_SYNC, event, net_p, 
            param1, param2, NULL, NULL);
}


/*
 *------------------------------------------------------------------------------
 * Function     : blog_notify_async()
 * Description  : blog_notify_async() asynchronous API
 * Parameters   :
 *  event       : notification
 *  net_p       : Pointer to a network stack entity
 *  param1      : optional parameter 1
 *  param2      : optional parameter 2
 * PreRequisite : acquire blog_lock_g before calling blog_notify_async()
 * return value : Caller MUST check the return value.
 *              : 1 = means caller may wait, and caller's notify callback 
 *              function will be called,
 *              : 0 = caller's notify callback function will NOT be called.
 *              :   Situations where fc_evt task is not running.
 *------------------------------------------------------------------------------
 */
int blog_notify_async(BlogNotify_t event, void *net_p,
        unsigned long param1, unsigned long param2,
        blog_notify_async_cb_fn_t notify_cb_fn, void *notify_cb_data_p)
{
    return (_blog_notify_async(BLOG_NOTIFY_API_ASYNC, event, net_p, 
            param1, param2, notify_cb_fn, notify_cb_data_p));
}


int blog_flowevent_register_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&blog_flowevent_chain, nb);
}

int blog_flowevent_unregister_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&blog_flowevent_chain, nb);
}

static inline void blog_flowevent_notifier_call_chain(unsigned long event,
        BlogFlowEventInfo_t *info)
{
    atomic_notifier_call_chain(&blog_flowevent_chain,
            event, info);
}

int blog_ct_get_stats( const void * ct, uint32_t blog_key, uint32_t dir,
        BlogFcStats_t * stats)
{
    int ret = -1; 

    /*this is a hack to avoid const warning */
    void **net_pp = (void *)&ct;

    blog_lock();

        /*invoke blog_query only if key is valid */

    if((blog_key != BLOG_KEY_FC_INVALID) &&
            (blog_query(QUERY_FLOWTRACK_STATS, *net_pp, blog_key, dir,
                        (unsigned long) stats) == 0))
        ret = 0;
         
    /* store the poll time stamp in case users needed it */
    stats->pollTS_ms = jiffies_to_msecs(jiffies);
    blog_unlock();

    return ret;
}


blog_ct_put_stats_t blog_ct_put_stats_fn = NULL;

static inline void blog_ct_put_stats(void * net_p, uint32_t dir,
        BlogFcStats_t * stats)
{
    /* store the poll time stamp in case users needed it */
    stats->pollTS_ms = jiffies_to_msecs(jiffies);

    if(blog_ct_put_stats_fn)
        blog_ct_put_stats_fn(net_p, dir, stats);
}

#if defined(CONFIG_BCM_OVS)
int blog_is_ovs_internal_dev(void *net_p)
{
    int ret = 0;

    if (blog_ovs_hooks_g.is_ovs_internal_dev)
        ret = blog_ovs_hooks_g.is_ovs_internal_dev(net_p);

    return ret;
}

unsigned long blog_mega_get_key(void *net_p)
{
    unsigned long blog_key = BLOG_KEY_FC_INVALID;

    if (blog_ovs_hooks_g.mega_get_key)
        blog_key = blog_ovs_hooks_g.mega_get_key(net_p);

    return blog_key;
}

void blog_mega_set_key(void *net_p, unsigned long blog_key)
{
    if (blog_ovs_hooks_g.mega_set_key)
        blog_ovs_hooks_g.mega_set_key(net_p, blog_key);
}

void blog_mega_put_fast_stats(void *net_p, blog_fast_stats_t *stats)
{
    if (blog_ovs_hooks_g.mega_put_fast_stats)
        blog_ovs_hooks_g.mega_put_fast_stats(net_p, stats);
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_bind_ovs
 * Description  : Bind OVS hooks to blog
 *------------------------------------------------------------------------------
 */
void blog_bind_ovs(blog_ovs_hooks_t *blog_ovs_hooks_p)
{
    memcpy(&blog_ovs_hooks_g, blog_ovs_hooks_p, sizeof(blog_ovs_hooks_t));
}
EXPORT_SYMBOL(blog_bind_ovs);
#endif

/*
 *------------------------------------------------------------------------------
 * Function     : blog_request
 * Description  : Blog client requests an operation to be performed on a network
 *                stack entity.
 * Parameters   :
 *  request     : request type
 *  net_p       : Pointer to a network stack entity
 *  param1      : optional parameter 1
 *  param2      : optional parameter 2
 *------------------------------------------------------------------------------
 */
extern int blog_rule_delete_action( void *rule_p );

unsigned long blog_request( BlogRequest_t request, void * net_p,
                       unsigned long param1, unsigned long param2 )
{
    unsigned long ret=0;

    blog_assertr( (request < BLOG_REQUEST_MAX), 0 );
    blog_assertr( (net_p != (void *)NULL), 0 );

#if defined(CC_BLOG_SUPPORT_DEBUG)
    if ((request != SYS_TIME_GET) )
#endif
        blog_print( "request<%s> net_p<%p>"
                    " param1<%lu:0x%08x> param2<%lu:0x%08x>",
                    strBlogRequest[request], net_p,
                    param1, (int)param1, param2, (int)param2);

    switch ( request )
    {
#if defined(BLOG_NF_CONNTRACK)
        case FLOWTRACK_KEY_SET:
            blog_assertr( ((param1 == BLOG_PARAM1_DIR_ORIG) ||
                           (param1 == BLOG_PARAM1_DIR_REPLY)), 0 );

            /* check pld connections for any corruptions*/
            if((param2 != BLOG_KEY_FC_TUNNEL_IPV6) && (param2 != BLOG_KEY_FC_TUNNEL_IPV4))
            {
                if(param2 != BLOG_KEY_FC_INVALID)
                {
                    /*check blog_key[param1] should be BLOG_KEY_FC_INVALID */
                    if(((struct nf_conn *)net_p)->blog_key[param1] != BLOG_KEY_FC_INVALID )
                    {
                        blog_error("nf_conn: blog_key corruption when adding flow net_p=%p "
                            "dir=%ld old_key=0x%08x new_key=0x%08lx\n", net_p, param1,
                             ((struct nf_conn *)net_p)->blog_key[param1], param2);
                    }
                }
                else
                {
                    if(((struct nf_conn *)net_p)->blog_key[param1] == BLOG_KEY_FC_INVALID )
                    {
                        blog_error("nf_conn: blog_key corruption when deleting flow for net_p=%p "
                            "dir=%ld old_key=0x%08x new_key=0x%08lx\n", net_p, param1,
                             ((struct nf_conn *)net_p)->blog_key[param1], param2);
                    }
                }
            }

            ((struct nf_conn *)net_p)->blog_key[param1] = (uint32_t) param2;
            return 0;

        case FLOWTRACK_KEY_GET:
            blog_assertr( ((param1 == BLOG_PARAM1_DIR_ORIG) ||
                           (param1 == BLOG_PARAM1_DIR_REPLY)), 0 );
            ret = ((struct nf_conn *)net_p)->blog_key[param1];
            break;

#if defined(CONFIG_NF_DYNDSCP) || defined(CONFIG_NF_DYNDSCP_MODULE)
        case FLOWTRACK_DSCP_GET:
            blog_assertr( ((param1 == BLOG_PARAM1_DIR_ORIG) ||
                           (param1 == BLOG_PARAM1_DIR_REPLY)), 0 );
            ret = ((struct nf_conn *)net_p)->dyndscp.dscp[param1];
            break;
#endif

        case FLOWTRACK_CONFIRMED:    /* E.g. UDP connection confirmed */
            ret = test_bit( IPS_CONFIRMED_BIT,
                            &((struct nf_conn *)net_p)->status );
            break;

        case FLOWTRACK_ALG_HELPER:
        {
            struct nf_conn * nfct_p;
            struct nf_conn_help * help;

            nfct_p = (struct nf_conn *)net_p;
            help = nfct_help(nfct_p);

            if ( (help != (struct nf_conn_help *)NULL )
                && (help->helper != (struct nf_conntrack_helper *)NULL) 
                && (help->helper->name && strcmp(help->helper->name, "BCM-NAT")) )
            {
                blog_print( "HELPER ct<%p> helper<%s>",
                            net_p, help->helper->name );
                return 1;
            }
            return 0;
        }

        case FLOWTRACK_EXCLUDE:  /* caution: modifies net_p */
            clear_bit(IPS_BLOG_BIT, &((struct nf_conn *)net_p)->status);
            return 0;

        case FLOWTRACK_TIME_SET:
        {
            struct nf_conn *ct = (struct nf_conn *)net_p;
            BlogCtTime_t *ct_time_p = (BlogCtTime_t *) param1;

            blog_assertr( (ct_time_p != NULL), 0 );

            if (blog_cttime_update_fn && ct && ct_time_p)
            {
                (*blog_cttime_update_fn)(ct, ct_time_p);
            }
            return 0;
        }

        case FLOWTRACK_PUT_STATS:
        {
            if ( likely(net_p)  )
            {
                Blog_t *blog_p = (Blog_t *)net_p;
                BlogFcStats_t *stats_p = (BlogFcStats_t *)(param1);
                void *pld_nwe_p = _blog_get_nwe(blog_p, BLOG_NPE_PLD);
                void *del_nwe_p = _blog_get_nwe(blog_p, BLOG_NPE_DEL);

                if (pld_nwe_p)
                    blog_ct_put_stats(pld_nwe_p, blog_p->nf_dir_pld, stats_p);

                if (del_nwe_p)
                    blog_ct_put_stats(del_nwe_p, blog_p->nf_dir_del, stats_p);
            }
            return 0;
        }
#endif /* defined(BLOG_NF_CONNTRACK) */

        case BRIDGEFDB_KEY_SET:
            blog_assertr( ((param1 == BLOG_PARAM1_SRCFDB) ||
                           (param1 == BLOG_PARAM1_DSTFDB)), 0 );
            ((struct net_bridge_fdb_entry *)net_p)->fdb_key = (uint32_t) param2;
            return 0;

        case BRIDGEFDB_KEY_GET:
            blog_assertr( ((param1 == BLOG_PARAM1_SRCFDB) ||
                           (param1 == BLOG_PARAM1_DSTFDB)), 0 );
            ret = ((struct net_bridge_fdb_entry *)net_p)->fdb_key;
            break;

        case BRIDGEFDB_TIME_SET:
            ((struct net_bridge_fdb_entry *)net_p)->updated = param2;
            return 0;

        case BRIDGEFDB_IFIDX_GET:
        {
            struct net_bridge_fdb_entry *fdb_p = (struct net_bridge_fdb_entry *)net_p;
            if (fdb_p && fdb_p->dst && fdb_p->dst->dev)
                ret = fdb_p->dst->dev->ifindex;
            else
                ret = 0;
            break;
        }

        case NETIF_PUT_STATS:
        {
            struct net_device * dev_p = (struct net_device *)net_p;
            BlogStats_t * sw_bstats_p = (BlogStats_t *) param1;
            BlogStats_t * hw_bstats_p = (BlogStats_t *) param2;
            blog_assertr( (sw_bstats_p != (BlogStats_t *)NULL), 0 );
            blog_assertr( (hw_bstats_p != (BlogStats_t *)NULL), 0 );

            blog_print("dev_p<%p> rx_pkt<%llu> rx_byte<%llu> tx_pkt<%llu>"
                       " tx_byte<%llu> multicast<%llu>", dev_p,
                       sw_bstats_p->rx_packets+hw_bstats_p->rx_packets, 
                       sw_bstats_p->rx_bytes+hw_bstats_p->rx_bytes,
                       sw_bstats_p->tx_packets+hw_bstats_p->tx_packets, 
                       sw_bstats_p->tx_bytes+hw_bstats_p->tx_bytes,
                       sw_bstats_p->multicast+hw_bstats_p->multicast);
            /* Work-around : put_stats only used by WLAN for backward compatibility */
            if ( dev_p->put_stats )
            {
                BlogStats_t bstats; 
                memcpy(&bstats, sw_bstats_p, sizeof(bstats)); 
                blog_fold_stats(&bstats, hw_bstats_p);
                dev_p->put_stats( dev_p, &bstats );
            }
            else
            {
                blog_put_dev_stats(dev_p, sw_bstats_p, hw_bstats_p);
            }

            return 0;
        }
        
        case LINK_XMIT_FN:
        {
            struct net_device * dev_p = (struct net_device *)net_p;
#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_QOS_CPU)
            struct net_device * rx_dev_p = (struct net_device *)(param1);

            if(rx_dev_p->netdev_ops->ndo_dpi_enqueue && (rx_dev_p->priv_flags & IFF_WANDEV))
            {
                ret = (unsigned long)(rx_dev_p->netdev_ops->ndo_dpi_enqueue);
            }
            else
#endif
            {
            ret = (unsigned long)(dev_p->netdev_ops->ndo_start_xmit);
            }
            break;
        }

        case LINK_NOCARRIER:
            ret = test_bit( __LINK_STATE_NOCARRIER,
                            &((struct net_device *)net_p)->state );
            break;

        case NETDEV_NAME:
        {
            struct net_device * dev_p = (struct net_device *)net_p;
            ret = (unsigned long)(dev_p->name);
            break;
        }

        case NETDEV_ADDR:
        {
            struct net_device * dev_p = (struct net_device *)net_p;
            ret = (unsigned long)(dev_p->dev_addr);
            break;
        }

        case IQPRIO_SKBMARK_SET:
        {
            Blog_t *blog_p = (Blog_t *)net_p;
            blog_p->mark = SKBMARK_SET_IQPRIO_MARK(blog_p->mark, param1 );
            return 0;
        }

        case DPIQ_SKBMARK_SET:
        {
#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_QOS_CPU)
            Blog_t *blog_p = (Blog_t *)net_p;
            blog_p->mark = SKBMARK_SET_DPIQ_MARK( blog_p->mark, param1 );
#endif
            return 0;
        }

        case MCAST_DFLT_MIPS:
        {
            blog_rule_delete_action( net_p );
            return 0;
        }

        case SYS_TIME_GET:
        {
           *(unsigned long *)net_p = jiffies;
            return 0;
        }

#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE)
        case GRE_TUNL_XMIT:
        {
            blog_assertr( ((BlogIpv4Hdr_t *)param1 != NULL), 0 );

            if (blog_gre_xmit_update_fn != NULL)
                return blog_gre_xmit_update_fn(net_p, (BlogIpv4Hdr_t *)param1,
                        (uint32_t) param2);
        }

        case GRE6_TUNL_XMIT:
        {
            blog_assertr( ((BlogIpv6Hdr_t *)param1 != NULL), 0 );

            if (blog_gre6_xmit_update_fn != NULL)
            {
                return blog_gre6_xmit_update_fn(net_p, (BlogIpv6Hdr_t *)param1,
                        (uint32_t) param2);
            }
        }
#endif

        case SKB_DST_ENTRY_SET:
        {
            Blog_t *blog_p = (Blog_t *)net_p;
            struct sk_buff *skb_p = (struct sk_buff *)param1;
            struct dst_entry *dst_p;

            blog_assertr( (skb_p != (void *)NULL), 0 );

            dst_p = skb_dst(skb_p);
            dst_hold(dst_p);
            blog_p->dst_entry = (void *)dst_p;
            return 0;
        }

        case SKB_DST_ENTRY_RELEASE:
        {
            dst_release((struct dst_entry *)net_p);
            return 0;
        }

        case FLOW_EVENT_ACTIVATE:
        case FLOW_EVENT_DEACTIVATE:
        {
            if ( likely(net_p)  )
            {
                Blog_t *blog_p = (Blog_t *)net_p;
                struct net_device * rx_dev_p = (struct net_device *)(param1);
                BlogFlowEventInfo_t info = {};

                info.is_downstream = (rx_dev_p->priv_flags & IFF_WANDEV) ? 1 : 0;
                info.skb_mark_flow_id = SKBMARK_GET_FLOW_ID(blog_p->mark);
                info.ct_pld_p = _blog_get_nwe(blog_p, BLOG_NPE_PLD);

                /*TODO check if any extra checks are neeeded to ensure ct_del_p is 
                 * a valid nf_conn*/
                info.ct_del_p = _blog_get_nwe(blog_p, BLOG_NPE_DEL);
                info.flow_event_type = param2;

                blog_flowevent_notifier_call_chain(request, &info);
            }
            return 0;
        }

        case CHK_HOST_DEV_MAC:
        {
            return blog_is_config_netdev_mac(net_p, param1);
        }

#if defined(CONFIG_BCM_KF_MAP) || defined(CONFIG_BCM_MAP_MODULE)
        case MAP_TUPLE_KEY_SET:
        {
            struct map_tuple *map_p = (struct map_tuple *)net_p;
	        struct timeval now;

            blog_assertr( ((param1 == BLOG_PARAM1_MAP_DIR_US) ||
                           (param1 == BLOG_PARAM1_MAP_DIR_DS)), 0 );

            /* check map tuple connections for any corruptions*/
            if (param2 != BLOG_KEY_FC_INVALID)
            {
                /*check blog_key[param1] should be BLOG_KEY_FC_INVALID */
                if(map_p->blog_key[param1] != BLOG_KEY_FC_INVALID)
                {
                    blog_error("MAP: blog_key corruption when adding flow map_p=%p "
                        "dir=%ld old_key=0x%08x new_key=0x%08lx\n", map_p, param1,
                         map_p->blog_key[param1], param2);
                }
            }
            else
            {
                if (map_p->blog_key[param1] == BLOG_KEY_FC_INVALID)
                {
                    blog_error("MAP: blog_key corruption when deleting flow "
                        "map_p=%p dir=%ld old_key=0x%08x new_key=0x%08lx\n", 
                        map_p, param1, map_p->blog_key[param1], param2);
                }

	            do_gettimeofday(&now);
                map_p->evict_time.tv_sec = now.tv_sec;
            }

            ((struct map_tuple *)net_p)->blog_key[param1] = (uint32_t) param2;
            return 0;
        }

        case MAP_TUPLE_KEY_GET:
            blog_assertr( ((param1 == BLOG_PARAM1_MAP_DIR_US) ||
                           (param1 == BLOG_PARAM1_MAP_DIR_DS)), 0 );
            ret = ((struct map_tuple *)net_p)->blog_key[param1];
            break;
#endif

#if defined(CONFIG_BCM_OVS)
        case MEGA_KEY_SET:
        {
            blog_mega_set_key(net_p, param2);
            return 0;
        }

        case MEGA_KEY_GET:
        {
            ret = blog_mega_get_key(net_p);
            break;
        }

        case MEGA_PUT_STATS:
        {
            Blog_t *blog_p = (Blog_t *)net_p;
            blog_npe_t *npe_p = BLOG_NPE_NULL;
            blog_fast_stats_t *stats_p = (blog_fast_stats_t *)(param1);

            if (IS_BLOG_NPE_MEGA(blog_p))
            {
                npe_p = blog_p->npe_p[BLOG_NPE_MEGA];

                if (likely(npe_p))
                {
                    if (npe_p->nwe_p)
                        blog_mega_put_fast_stats(npe_p->nwe_p, stats_p);
                }
            }
            ret = 0;
        }
#endif

        default:
            return 0;
    }

    blog_print("ret<%lu:0x%08x>", ret, (int)ret);

    return ret;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_query
 * Description  : Query a Blog client (qr_hook) of an event.
 * Parameters   :
 *  query       : query
 *  net_p       : Pointer to a network stack entity
 *  param1      : optional parameter 1
 *  param2      : optional parameter 2
 *  param3      : optional parameter 3
 * PreRequisite : acquire blog_lock_g before calling blog_query()
 *------------------------------------------------------------------------------
 */
int blog_query( BlogQuery_t query, void * net_p,
                 uint32_t param1, uint32_t param2, unsigned long param3 )
{
    blog_assertr( (query < BLOG_QUERY_MAX), -1 );
    blog_assertr( (net_p != (void *)NULL), -1 );

    if ( unlikely(blog_qr_hook_g == (BlogQueryHook_t)NULL) )
        return -1;

    blog_print( "Query<%s> net_p<%p> param1<%u:0x%08x> "
                "param2<%u:0x%08x> param3<%lu:0x%08x> [<%p>] ",
                strBlogQuery[query], net_p, param1, (int)param1, 
                param2, (int)param2, param3, (int)param3,
                __builtin_return_address(0) );

    return blog_qr_hook_g( query, net_p, param1, param2, param3 );
}



/*
 *------------------------------------------------------------------------------
 * Function     : blog_filter
 * Description  : Filter packets that need blogging.
 *                E.g. To skip logging of control versus data type packet.
 *   blog_p     : Received packet parsed and logged into a blog
 * Returns      :
 *   PKT_NORM   : If normal stack processing without logging
 *   PKT_BLOG   : If stack processing with logging
 *------------------------------------------------------------------------------
 */
BlogAction_t blog_filter( Blog_t * blog_p )
{
    blog_assertr( ((blog_p != BLOG_NULL) && (_IS_BPTR_(blog_p))), PKT_NORM );
    blog_assertr( (blog_p->rx.info.hdrs != 0), PKT_NORM );

    /*
     * E.g. IGRS/UPnP using Simple Service Discovery Protocol SSDP over HTTPMU
     *      HTTP Multicast over UDP 239.255.255.250:1900,
     *
     *  if ( ! RX_IPinIP(blog_p) && RX_IPV4(blog_p)
     *      && (blog_p->rx.tuple.daddr == htonl(0xEFFFFFFA))
     *      && (blog_p->rx.tuple.port.dest == 1900)
     *      && (blog_p->key.protocol == IPPROTO_UDP) )
     *          return PKT_NORM;
     *
     *  E.g. To filter IPv4 Local Network Control Block 224.0.0/24
     *             and IPv4 Internetwork Control Block  224.0.1/24
     *
     *  if ( ! RX_IPinIP(blog_p) && RX_IPV4(blog_p)
     *      && ( (blog_p->rx.tuple.daddr & htonl(0xFFFFFE00))
     *           == htonl(0xE0000000) )
     *          return PKT_NORM;
     *  
     */
    return PKT_BLOG;    /* continue in stack with logging */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_finit, blog_sinit
 * Description  : This function may be inserted in a physical network device's
 *                packet receive handler. A receive handler typically extracts
 *                the packet data from the rx DMA buffer ring, allocates and
 *                sets up a sk_buff, decodes the l2 headers and passes the
 *                sk_buff into the network stack via netif_receive_skb/netif_rx.
 *
 *                Prior to constructing a sk_buff, blog_finit() may be invoked
 *                using a fast kernel buffer to carry the received buffer's
 *                context <data,len>, and the receive net_device and l1 info.
 *
 *                This function invokes the bound receive blog hook.
 *
 * Parameters   :
 *  blog_finit() fkb_p: Pointer to a fast kernel buffer<data,len>
 *  blog_sinit() skb_p: Pointer to a Linux kernel skbuff
 *  dev_p       : Pointer to the net_device on which the packet arrived.
 *  encap       : First encapsulation type
 *  channel     : Channel/Port number on which the packet arrived.
 *  phyHdr      : e.g. XTM device RFC2684 header type
 *  txdev_pp    : tx net_devce on whic pkt will be xmitted, used for LOCAL TCP
 *
 * Returns      :
 *  PKT_DONE    : The fkb|skb is consumed and device should not process fkb|skb.
 *
 *  PKT_NORM    : Device may invoke netif_receive_skb for normal processing.
 *                No Blog is associated and fkb reference count = 0.
 *                [invoking fkb_release() has no effect]
 *
 *  PKT_BLOG    : PKT_NORM behaviour + Blogging enabled.
 *                Must call fkb_release() to free associated Blog
 *  PKT_TCP4_LOCAL : Locally terminated IPV4 TCP traffic,inject into 
 *                   tcp stack directly 
 *
 *------------------------------------------------------------------------------
 */
inline
BlogAction_t _blog_finit( struct fkbuff * fkb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr,
                         BlogFcArgs_t *fc_args)
{
    BlogHash_t blogHash;
    BlogAction_t action = PKT_NORM;
#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE) || defined(CONFIG_ACCEL_PPTP)   
    int gre_rcv_version;
    uint16_t flags;
#endif   
    BlogDevRxHook_t rx_hook = blog_rx_hook_g;

    if ( blog_pa_hook_g != (BlogPaHook_t)NULL )
    {
        action = blog_pa_hook_g(fkb_p, dev_p, encap, channel, phyHdr);
        if( likely(action == PKT_DONE) )
            goto bypass;
    }

    blogHash.match = 0U;     /* also clears hash, protocol = 0 */

    if ( unlikely(rx_hook == (BlogDevRxHook_t)NULL) )
        goto bypass;

    blogHash.l1_tuple.channel = (uint8_t)channel;
    blogHash.l1_tuple.phyType = BLOG_GET_PHYTYPE(phyHdr);
    blogHash.l1_tuple.phyLen = BLOG_GET_PHYLEN(phyHdr);

    blog_assertr( (blogHash.l1_tuple.phyType < BLOG_MAXPHY), PKT_NORM);
    blog_print( "fkb<%p:%x> pData<%p> length<%d> dev<%p>"
                " chnl<%u> %s PhyHdrLen<%u> key<0x%08x>",
                fkb_p, _is_in_skb_tag_(fkb_p->ptr, fkb_p->flags),
                fkb_p->data, fkb_p->len, dev_p,
                channel, strBlogPhy[blogHash.l1_tuple.phyType],
                rfc2684HdrLength[blogHash.l1_tuple.phyLen],
                blogHash.match );   
#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE) || defined(CONFIG_ACCEL_PPTP)
    gre_rcv_version = blog_rcv_chk_gre (fkb_p, encap, &flags);     
#endif           
#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE)
    if (blog_gre_tunnel_accelerated() && gre_rcv_version == PPTP_GRE_VER_0 && flags)
    {
        int gre_status;
        void *tunl_p = NULL;
        uint32_t pkt_seqno;
        gre_status = blog_gre_rcv( fkb_p, (void *)dev_p, encap, &tunl_p,
            &pkt_seqno );

        switch (gre_status)
        {
            case BLOG_GRE_RCV_NOT_GRE:
            case BLOG_GRE_RCV_NO_SEQNO:
            case BLOG_GRE_RCV_IN_SEQ:
                break;

            case BLOG_GRE_RCV_NO_TUNNEL:
                blog_print( "RX GRE no matching tunnel" );
                break;

            case BLOG_GRE_RCV_FLAGS_MISSMATCH:
                blog_print( "RX GRE flags miss-match" );
                action = PKT_DROP;
                goto bypass;

            case BLOG_GRE_RCV_CHKSUM_ERR:
                blog_print( "RX GRE checksum error" );
                action = PKT_DROP;
                goto bypass;

            case BLOG_GRE_RCV_OOS_LT:
                blog_print( "RX GRE out-of-seq LT pkt seqno <%u>", pkt_seqno );
                action = PKT_DROP;
                goto bypass;

            case BLOG_GRE_RCV_OOS_GT:
                blog_print( "RX GRE out-of-seq GT pkt seqno <%u>", pkt_seqno );
                break;

            default:
                blog_print( "RX GRE unkown status <%u>", gre_status );
                break;
        }
    }
#endif

#if defined(CONFIG_ACCEL_PPTP) 
	if (blog_gre_tunnel_accelerated() && gre_rcv_version == PPTP_GRE_VER_1)
	{
		int pptp_status;
        uint32_t rcv_pktSeq;
        pptp_status = blog_pptp_rcv( fkb_p, encap, &rcv_pktSeq );
        switch (pptp_status)
        {
            case BLOG_PPTP_ENCRYPTED:
                blog_print( "RX PPTP encrypted pkt seqno <%u>", rcv_pktSeq );
                action = PKT_NORM;
                fkb_release( fkb_p );
                goto bypass;

            case BLOG_PPTP_RCV_NOT_PPTP:
            case BLOG_PPTP_RCV_NO_SEQNO:
            case BLOG_PPTP_RCV_IN_SEQ:
            	break;

            case BLOG_PPTP_RCV_NO_TUNNEL:
                blog_print( "RX PPTP no matching tunnel" );
            	break;

            case BLOG_PPTP_RCV_FLAGS_MISSMATCH:
               	blog_print( "RX PPTP flags miss-match" );
                action = PKT_DROP;
                goto bypass;

            case BLOG_PPTP_RCV_OOS_LT:
                blog_print( "RX PPTP out-of-seq LT pkt seqno <%u>", rcv_pktSeq );
                action = PKT_DROP;
                goto bypass;

            case BLOG_PPTP_RCV_OOS_GT:
                blog_print( "RX PPTP out-of-seq GT pkt seqno <%u>", rcv_pktSeq );
                break;

            default:
                blog_print( "RX PPTP unkown status <%u>", pptp_status );
                break;
        }       
        
	}	
#endif

    fc_args->h_proto = encap;
    fc_args->key_match = blogHash.match;
    fc_args->txdev_p = NULL;

    action = rx_hook( fkb_p, (void *)dev_p, fc_args);
    
    if ( action == PKT_BLOG )
    {
        if (blog_ctx_p->blog_dump & BLOG_DUMP_RXBLOG)
            blog_dump(fkb_p->blog_p);

        fkb_p->blog_p->rx_dev_p = (void *)dev_p;           /* Log device info */
#if defined(CC_BLOG_SUPPORT_USER_FILTER)
        action = blog_filter(fkb_p->blog_p);
#endif
    }

    if ( unlikely(action == PKT_NORM) )
        fkb_release( fkb_p );

bypass:
    return action;
}

BlogAction_t blog_finit( struct fkbuff * fkb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr )
{
    /*TODO move this allocation to drivers calling this function */
    BlogFcArgs_t fc_args;
    return _blog_finit(fkb_p, dev_p, encap, channel, phyHdr, &fc_args);
}

BlogAction_t blog_finit_args( struct fkbuff * fkb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr, BlogFcArgs_t *fc_args )
{
    return _blog_finit(fkb_p, dev_p, encap, channel, phyHdr, fc_args);
}

/*
 * blog_sinit serves as a wrapper to blog_finit() by overlaying an fkb into a
 * skb and invoking blog_finit().
 */
static inline BlogAction_t _blog_sinit( struct sk_buff * skb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr)
{
    struct fkbuff * fkb_p;
    BlogAction_t action = PKT_NORM;

    if ( unlikely(blog_rx_hook_g == (BlogDevRxHook_t)NULL) )
        goto bypass;

    blog_assertr( (BLOG_GET_PHYTYPE(phyHdr) < BLOG_MAXPHY), PKT_NORM );
    blog_print( "skb<%p> pData<%p> length<%d> dev<%p>"
                " chnl<%u> %s PhyHdrLen<%u>",
                skb_p, skb_p->data, skb_p->len, dev_p,
                channel, strBlogPhy[BLOG_GET_PHYTYPE(phyHdr)],
                rfc2684HdrLength[BLOG_GET_PHYLEN(phyHdr)] );

    /* CAUTION: Tag that the fkbuff is from sk_buff */
    fkb_p = (FkBuff_t *) &skb_p->fkbInSkb;
    fkb_p->flags = _set_in_skb_tag_(0); /* clear and set in_skb tag */
    FKB_CLEAR_LEN_WORD_FLAGS(fkb_p->len_word); /*clears bits 31-24 of skb->len */

    action = blog_finit( fkb_p, dev_p, encap, channel, phyHdr );

    if ( action == PKT_BLOG )
    {
         blog_assertr( (fkb_p->blog_p != BLOG_NULL), PKT_NORM );
         fkb_p->blog_p->skb_p = skb_p;
    } 

bypass:
    return action;
}

BlogAction_t blog_sinit( struct sk_buff * skb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr )
{
    return _blog_sinit(skb_p, dev_p, encap, channel, phyHdr);
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_emit
 * Description  : This function may be inserted in a physical network device's
 *                hard_start_xmit function just before the packet data is
 *                extracted from the sk_buff and enqueued for DMA transfer.
 *
 *                This function invokes the transmit blog hook.
 * Parameters   :
 *  nbuff_p     : Pointer to a NBuff
 *  dev_p       : Pointer to the net_device on which the packet is transmited.
 *  encap       : First encapsulation type
 *  channel     : Channel/Port number on which the packet is transmited.
 *  phyHdr      : e.g. XTM device RFC2684 header type
 *
 * Returns      :
 *  PKT_DONE    : The skb_p is consumed and device should not process skb_p.
 *  PKT_NORM    : Device may use skb_p and proceed with hard xmit 
 *                Blog object is disassociated and freed.
 *------------------------------------------------------------------------------
 */
BlogAction_t _blog_emit( void * nbuff_p, void * dev_p,
                        uint32_t encap, uint32_t channel, uint32_t phyHdr )
{
    BlogHash_t blogHash;
    struct sk_buff * skb_p;
    Blog_t * blog_p;
    BlogAction_t action = PKT_NORM;   
#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE) || defined(CONFIG_ACCEL_PPTP)    
    int gre_xmit_version;
#endif    
    // outer inline function has already verified this is a skbuff
    skb_p = PNBUFF_2_SKBUFF(nbuff_p);   /* same as nbuff_p */

    blog_p = skb_p->blog_p;
    if ( ( blog_p == BLOG_NULL ) || ( dev_p == NULL ) )
        goto bypass;

    blog_assertr( (_IS_BPTR_(blog_p)), PKT_NORM );

    blogHash.match = 0U;

    if ( likely(blog_tx_hook_g != (BlogDevTxHook_t)NULL) )
    {

        blog_p->tx_dev_p = (void *)dev_p;           /* Log device info */
        /* Log TX dev delta  (tx.pktlen has the original RX pktlen) */
        blog_p->tx_dev_delta = (skb_p->len - blog_p->tx.pktlen) & 0xFF;

        blogHash.l1_tuple.channel = (uint8_t)channel;
        blogHash.l1_tuple.phyType = BLOG_GET_PHYTYPE(phyHdr);
        blogHash.l1_tuple.phyLen  = BLOG_GET_PHYLEN(phyHdr);

        blog_p->priority = skb_p->priority;         /* Log skb info */
        blog_p->mark = skb_p->fkb_mark;
        blog_p->minMtu   = blog_getTxMtu(blog_p);
        blog_assertr( (BLOG_GET_PHYTYPE(phyHdr) < BLOG_MAXPHY), PKT_NORM);
        blog_print( "skb<%p> blog<%p> pData<%p> length<%d>"
                    " dev<%p> chnl<%u> %s PhyHdrLen<%u> key<0x%08x>",
            skb_p, blog_p, skb_p->data, skb_p->len,
            dev_p, channel, strBlogPhy[BLOG_GET_PHYTYPE(phyHdr)],
            rfc2684HdrLength[BLOG_GET_PHYLEN(phyHdr)],
            blogHash.match );

        if (skb_p->priority)
            printk(KERN_DEBUG "%s, blogp = 0x%x\n", __FUNCTION__, blog_p->priority);

        /* blog lock/unlock is done inside tx_hook */
        action = blog_tx_hook_g( skb_p, (void*)skb_p->dev,
                                 encap, blogHash.match );
    }
    blog_free( skb_p, blog_free_reason_blog_emit );   /* Dis-associate w/ skb */

bypass:
	
#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE) || defined(CONFIG_ACCEL_PPTP)	
	gre_xmit_version = blog_xmit_chk_gre(skb_p, encap);
#endif	

#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE)
    if(gre_xmit_version == PPTP_GRE_VER_0)
    {	
    	blog_gre_xmit(skb_p, encap);
    }
#endif

#if defined(CONFIG_ACCEL_PPTP)
    if(gre_xmit_version == PPTP_GRE_VER_1)
    {	
        blog_pptp_xmit(skb_p, encap); 
    }		
#endif    

    return action;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_activate
 * Description  : This function is a static configuration function of blog
 *                application. It invokes blog configuration hook
 * Parameters   :
 *  blog_p      : pointer to a blog with configuration information
 *  traffic     : type of the traffic
 *  client      : configuration client
 *
 * Returns      :
 *  ActivateKey : If the configuration is successful, a key is returned.
 *                Otherwise, NULL is returned
 *------------------------------------------------------------------------------
 */
BlogActivateKey_t *blog_activate( Blog_t * blog_p, BlogTraffic_t traffic,
                        BlogClient_t client )
{
    BlogActivateKey_t *key_p = NULL;
    
    if ( blog_p == BLOG_NULL ||
         traffic >= BlogTraffic_MAX ||
         client >= BlogClient_MAX )
    {
        blog_assertr( ( blog_p != BLOG_NULL ), NULL );
        goto bypass;
    }

    if ( unlikely(blog_sc_hook_g[client] == (BlogScHook_t)NULL) )
        goto bypass;

#if defined(CC_BLOG_SUPPORT_DEBUG)
    blog_print( "blog_p<%p> traffic<%u> client<%u>", blog_p, traffic, client );
    blog_dump( blog_p );
#endif

    blog_lock();
    key_p = blog_sc_hook_g[client]( blog_p, traffic );
    blog_unlock();

    /* on sucess blog_p is transferred to flow, decrement refcnt's of ct's */
    if(key_p)
        blog_ct_put(blog_p);

bypass:
    return key_p;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_deactivate
 * Description  : This function is a deconfiguration function of blog
 *                application
 * Parameters   :
 *  key         : blog key information
 *  traffic     : type of traffic
 *  client      : configuration client
 *
 * Returns      :
 *  blog_p      : If the deconfiguration is successful, the associated blog 
 *                pointer is returned to the caller
 *------------------------------------------------------------------------------
 */
Blog_t * blog_deactivate( BlogActivateKey_t key, BlogTraffic_t traffic,
                          BlogClient_t client )
{
    Blog_t * blog_p = NULL;

    if ( key.fc.word == BLOG_KEY_FC_INVALID || key.mc.word == BLOG_KEY_MCAST_INVALID ||
         traffic >= BlogTraffic_MAX ||
         client >= BlogClient_MAX )
    {
        blog_assertr( (key.fc.word != BLOG_KEY_FC_INVALID), blog_p );
        blog_assertr( (key.mc.word != BLOG_KEY_MCAST_INVALID), blog_p );
        goto bypass;
    }

    if ( unlikely(blog_sd_hook_g[client] == (BlogSdHook_t)NULL) )
        goto bypass;

    blog_print( "fc<0x%08x> mc<0x%08x> traffic<%u> client<%u>", 
                key.fc.word, key.mc.word, traffic, client );

    blog_lock();
    blog_p = blog_sd_hook_g[client]( key, traffic );
    blog_unlock();

#if defined(CC_BLOG_SUPPORT_DEBUG)
    blog_dump( blog_p );
#endif

bypass:
    return blog_p;
}

/*
 * blog_iq_prio determines the Ingress QoS priority of the packet
 */
int blog_iq_prio( struct sk_buff * skb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr )
{
    struct fkbuff * fkb_p;
    BlogAction_t action = PKT_NORM;
    int iq_prio = BLOG_IQ_PRIO_HIGH;
    uint32_t dummy;
    void *dummy_dev_p = &dummy;

    if ( unlikely(blog_rx_hook_g == (BlogDevRxHook_t)NULL) )
        goto bypass;

    blog_assertr( (BLOG_GET_PHYTYPE(phyHdr) < BLOG_MAXPHY), 1 );
    blog_print( "skb<%p> pData<%p> length<%d> dev<%p>"
                " chnl<%u> %s PhyHdrLen<%u>",
                skb_p, skb_p->data, skb_p->len, dev_p,
                channel, strBlogPhy[BLOG_GET_PHYTYPE(phyHdr)],
                rfc2684HdrLength[BLOG_GET_PHYLEN(phyHdr)] );

    /* CAUTION: Tag that the fkbuff is from sk_buff */
    fkb_p = (FkBuff_t *) &skb_p->fkbInSkb;

    /* set in_skb and chk_iq_prio tag */
    fkb_p->flags = _set_in_skb_n_chk_iq_prio_tag_(0); 
    action = blog_finit( fkb_p, dummy_dev_p, encap, channel, phyHdr );

    /* blog_iq_prio should return only PKT_BLOG, PKT_DROP or PKT_NORM*/

    if ( action == PKT_BLOG )
    {
         blog_assertr( (fkb_p->blog_p != BLOG_NULL), iq_prio);
         fkb_p->blog_p->skb_p = skb_p;
         iq_prio = fkb_p->blog_p->iq_prio;
         blog_free( skb_p, blog_free_reason_blog_iq_prio );
    } 
    else
    {
         blog_assertr(((action == PKT_NORM) || (action == PKT_DROP)), iq_prio);
    }

bypass:
    return iq_prio;
}

static int blog_notify_netevent(struct notifier_block *nb, unsigned long event, void *_neigh)
{
    struct neighbour *neigh = _neigh;
    switch (event)
    {
        case NETEVENT_ARP_BINDING_CHANGE:
              blog_lock();
              blog_notify_async(ARP_BIND_CHG, nb, *(uint32_t *)neigh->primary_key, 
                      (unsigned long)neigh->ha, NULL, NULL);
              blog_unlock();
              return 0;
        default:
              return 1;
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_get_hw_accel
 * Description: Get current status of Enable/Disable acceleration of flows by HW.
 * Return Val : 0(Disable), 1(Enable)
 *------------------------------------------------------------------------------
 */
int blog_get_hw_accel(void)
{
    int hw_accel = 0;

    blog_query(QUERY_GET_HW_ACCEL, (void*)&hw_accel, 0, 0, 0);
    return hw_accel;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_bind
 * Description  : Override default rx and tx hooks.
 *  blog_rx     : Function pointer to be invoked in blog_finit(), blog_sinit()
 *  blog_tx     : Function pointer to be invoked in blog_emit()
 *  blog_xx     : Function pointer to be invoked in blog_notify()
 *  info        : Mask of the function pointers for configuration
 *------------------------------------------------------------------------------
 */
void blog_bind( BlogDevRxHook_t blog_rx, BlogDevTxHook_t blog_tx,
                BlogNotifyHook_t blog_xx, BlogQueryHook_t blog_qr, 
                BlogBind_t bind)
{
    blog_print( "Bind Rx[<%p>] Tx[<%p>] Notify[<%p>] bind[<%u>]",
                blog_rx, blog_tx, blog_xx,
                (uint8_t)bind.hook_info );

    if ( bind.bmap.RX_HOOK )
        blog_rx_hook_g = blog_rx;   /* Receive  hook */
    if ( bind.bmap.TX_HOOK )
        blog_tx_hook_g = blog_tx;   /* Transmit hook */
    if ( bind.bmap.XX_HOOK )
        blog_xx_hook_g = blog_xx;   /* Notify hook */
    if ( bind.bmap.QR_HOOK )
        blog_qr_hook_g = blog_qr;   /* Query hook */
}

static BlogClient_t hw_accelerator_client = BlogClient_MAX;
static BlogClient_t sw_accelerator_client = BlogClient_MAX;

/*
 *------------------------------------------------------------------------------
 * Function     : is_hw_accelerator
 * Description  : 
 *------------------------------------------------------------------------------
 */
static int is_hw_accelerator(BlogClient_t client)
{
    switch(client)
    {
#if defined(CONFIG_BCM_KF_FAP)
    case BlogClient_fap:
#endif
#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
    case BlogClient_runner: 
#endif /* (CONFIG_BCM_RDPA) || (CONFIG_BCM_RDPA_MODULE) */
#endif /* CONFIG_BCM_KF_RUNNER */
        return 1;
    default:
        break;
    }
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : hw_accelerator_client_get
 * Description  :
 *------------------------------------------------------------------------------
 */
int hw_accelerator_client_get(void)
{
    return hw_accelerator_client;
}

/*
 *------------------------------------------------------------------------------
 * Function     : sw_accelerator_client_get
 * Description  :
 *------------------------------------------------------------------------------
 */
int sw_accelerator_client_get(void)
{
    return sw_accelerator_client;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_bind_config
 * Description  : Override default sc and sd hooks.
 *  blog_sc     : Function pointer to be invoked in blog_activate()
 *  blog_sd     : Function pointer to be invoked in blog_deactivate()
 *  client      : configuration client
 *  info        : Mask of the function pointers for configuration
 *------------------------------------------------------------------------------
 */
void blog_bind_config( BlogScHook_t blog_sc, BlogSdHook_t blog_sd,
                       BlogClient_t client, BlogBind_t bind)
{
    blog_print( "Bind Sc[<%p>] Sd[<%p>] Client[<%u>] bind[<%u>]",
                blog_sc, blog_sd, client,
                (uint8_t)bind.hook_info );

    if ( bind.bmap.SC_HOOK )
        blog_sc_hook_g[client] = blog_sc;   /* Static config hook */
    if ( bind.bmap.SD_HOOK )
        blog_sd_hook_g[client] = blog_sd;   /* Static deconf hook */

    if (is_hw_accelerator(client))
        hw_accelerator_client = client;
    else
        sw_accelerator_client = client;
}


/*
 *------------------------------------------------------------------------------
 * Function     : blog_bind_packet_accelerator
 * Description  : Bind a packet accelerator
 *  blog_pa     : Function pointer to packet accelerator 
 *  bind        : Mask of the function pointers for configuration
 *------------------------------------------------------------------------------
 */
void blog_bind_packet_accelerator( BlogPaHook_t blog_pa, BlogBind_t bind )
{
    blog_print( "Bind pa[<%p>] bind[<%x>]", blog_pa, bind.hook_info );

    if ( bind.bmap.PA_HOOK )
        blog_pa_hook_g = blog_pa;   /* Static packet accelerator hook */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog
 * Description  : Log the L2 or L3+4 tuple information
 * Parameters   :
 *  skb_p       : Pointer to the sk_buff
 *  dir         : rx or tx path
 *  encap       : Encapsulation type
 *  len         : Length of header
 *  data_p      : Pointer to encapsulation header data.
 *------------------------------------------------------------------------------
 */
void blog( struct sk_buff * skb_p, BlogDir_t dir, BlogEncap_t encap,
           size_t len, void * data_p )
{
    BlogHeader_t * bHdr_p;
    Blog_t * blog_p;

    blog_assertv( (skb_p != (struct sk_buff *)NULL ) );
    blog_assertv( (skb_p->blog_p != BLOG_NULL) );
    blog_assertv( (_IS_BPTR_(skb_p->blog_p)) );
    blog_assertv( (data_p != (void *)NULL ) );
    blog_assertv( (len <= BLOG_HDRSZ_MAX) );
    blog_assertv( (encap < PROTO_MAX) );

    blog_p = skb_p->blog_p;
    blog_assertv( (blog_p->skb_p == skb_p) );

    bHdr_p = &blog_p->rx + dir;

    if ( encap == PLD_IPv4 )    /* Log the IP Tuple */
    {
        BlogTuple_t * bTuple_p = &bHdr_p->tuple;
        BlogIpv4Hdr_t * ip_p   = (BlogIpv4Hdr_t *)data_p;

        /* Discontinue if non IPv4 or with IP options, or fragmented */
        if ( (ip_p->ver != 4) || (ip_p->ihl != 5)
             || (ip_p->flagsFrag & htons(BLOG_IP_FRAG_OFFSET|BLOG_IP_FLAG_MF)) )
            goto skip;

        if ( ip_p->proto == BLOG_IPPROTO_TCP )
        {
            BlogTcpHdr_t * th_p;
            th_p = (BlogTcpHdr_t*)( (uint8_t *)ip_p + BLOG_IPV4_HDR_LEN );

            /* Discontinue if TCP RST/FIN */
            if ( TCPH_RST(th_p) | TCPH_FIN(th_p) )
                goto skip;
            bTuple_p->port.source = th_p->sPort;
            bTuple_p->port.dest = th_p->dPort;
        }
        else if ( ip_p->proto == BLOG_IPPROTO_UDP )
        {
            BlogUdpHdr_t * uh_p;
            uh_p = (BlogUdpHdr_t *)( (uint8_t *)ip_p + BLOG_UDP_HDR_LEN );
            bTuple_p->port.source = uh_p->sPort;
            bTuple_p->port.dest = uh_p->dPort;
        }
        else
            goto skip;  /* Discontinue if non TCP or UDP upper layer protocol */

        bTuple_p->ttl = ip_p->ttl;
        bTuple_p->tos = ip_p->tos;
        bTuple_p->check = ip_p->chkSum;
        bTuple_p->saddr = blog_read32_align16( (uint16_t *)&ip_p->sAddr );
        bTuple_p->daddr = blog_read32_align16( (uint16_t *)&ip_p->dAddr );
        blog_p->key.protocol = ip_p->proto;
    }
    else if ( encap == PLD_IPv6 )    /* Log the IPv6 Tuple */
    {
        printk("FIXME blog encap PLD_IPv6 \n");
    }
    else    /* L2 encapsulation */
    {
        register short int * d;
        register const short int * s;

        blog_assertv( (bHdr_p->count < BLOG_ENCAP_MAX) );
        blog_assertv( ((len<=20) && ((len & 0x1)==0)) );
        blog_assertv( ((bHdr_p->length + len) < BLOG_HDRSZ_MAX) );

        bHdr_p->info.hdrs |= (1U << encap);
        bHdr_p->encap[ bHdr_p->count++ ] = encap;
        s = (const short int *)data_p;
        d = (short int *)&(bHdr_p->l2hdr[bHdr_p->length]);
        bHdr_p->length += len;

        switch ( len ) /* common lengths, using half word alignment copy */
        {
            case 20: *(d+9)=*(s+9);
                     *(d+8)=*(s+8);
                     *(d+7)=*(s+7);
            case 14: *(d+6)=*(s+6);
            case 12: *(d+5)=*(s+5);
            case 10: *(d+4)=*(s+4);
            case  8: *(d+3)=*(s+3);
            case  6: *(d+2)=*(s+2);
            case  4: *(d+1)=*(s+1);
            case  2: *(d+0)=*(s+0);
                 break;
            default:
                 goto skip;
        }
    }

    return;

skip:   /* Discontinue further logging by dis-associating Blog_t object */

    blog_skip( skb_p, blog_skip_reason_blog );

    /* DO NOT ACCESS blog_p !!! */
}


/*
 *------------------------------------------------------------------------------
 * Function     : blog_nfct_dump
 * Description  : Dump the nf_conn context
 *  ct          : Pointer to a nf_conn object
 * CAUTION      : nf_conn is not held !!!
 *------------------------------------------------------------------------------
 */
#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
void blog_nfct_dump( struct nf_conn * ct, uint32_t dir )
{
#if defined(BLOG_NF_CONNTRACK)
    struct nf_conn_help *help_p;
    struct nf_conn_nat  *nat_p;
    int bitix;
    if ( ct == NULL )
    {
        blog_error( "NULL NFCT error" );
        return;
    }

#ifdef CONFIG_NF_NAT_NEEDED
    nat_p = nfct_nat(ct);
#else
    nat_p = (struct nf_conn_nat *)NULL;
#endif

    help_p = nfct_help(ct);
    printk("\tNFCT: ct<0x%p>, master<0x%p>\n"
           "\t\tF_NAT<%p> keys[0x%08x 0x%08x] dir<%s>\n"
           "\t\thelp<0x%p> helper<%s>\n",
            ct, 
            ct->master,
            nat_p, 
            ct->blog_key[IP_CT_DIR_ORIGINAL], 
            ct->blog_key[IP_CT_DIR_REPLY],
            (dir<IP_CT_DIR_MAX)?strIpctDir[dir]:strIpctDir[IP_CT_DIR_MAX],
            help_p,
            (help_p && help_p->helper) ? help_p->helper->name : "NONE" );

    printk( "\t\tSTATUS[ " );
    for ( bitix = 0; bitix <= IPS_BLOG_BIT; bitix++ )
        if ( ct->status & (1 << bitix) )
            printk( "%s ", strIpctStatus[bitix] );
    printk( "]\n" );
#endif /* defined(BLOG_NF_CONNTRACK) */
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function     : blog_netdev_dump
 * Description  : Dump the contents of a net_device object.
 *  dev_p       : Pointer to a net_device object
 *
 * CAUTION      : Net device is not held !!!
 *
 *------------------------------------------------------------------------------
 */
static void blog_netdev_dump( struct net_device * dev_p )
{
    int i;
    printk( "\tDEVICE: %s dev<%p> ndo_start_xmit[<%p>]\n"
            "\t  dev_addr[ ", dev_p->name,
            dev_p, dev_p->netdev_ops->ndo_start_xmit );
    for ( i=0; i<dev_p->addr_len; i++ )
        printk( "%02x ", *((uint8_t *)(dev_p->dev_addr) + i) );
    printk( "]\n" );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_tuple_dump
 * Description  : Dump the contents of a BlogTuple_t object.
 *  bTuple_p    : Pointer to the BlogTuple_t object
 *------------------------------------------------------------------------------
 */
static void blog_tuple_dump( BlogTuple_t * bTuple_p )
{
    if (bTuple_p )
    {
    printk( "\tIPv4:\n"
            "\t\tSrc" BLOG_IPV4_ADDR_PORT_FMT
             " Dst" BLOG_IPV4_ADDR_PORT_FMT "\n"
            "\t\tttl<%3u> tos<%3u> check<0x%04x>\n",
            BLOG_IPV4_ADDR(bTuple_p->saddr), ntohs(bTuple_p->port.source),
            BLOG_IPV4_ADDR(bTuple_p->daddr), ntohs(bTuple_p->port.dest),
            bTuple_p->ttl, bTuple_p->tos, bTuple_p->check );
    }
}
 
/*
 *------------------------------------------------------------------------------
 * Function     : blog_tupleV6_dump
 * Description  : Dump the contents of a BlogTupleV6_t object.
 *  bTupleV6_p    : Pointer to the BlogTupleV6_t object
 *------------------------------------------------------------------------------
 */
static void blog_tupleV6_dump( BlogTupleV6_t * bTupleV6_p )
{
    printk( "\tIPv6:\n"
            "\t\tSrc" BLOG_IPV6_ADDR_PORT_FMT "\n"
            "\t\tDst" BLOG_IPV6_ADDR_PORT_FMT "\n"
            "\t\trx_hop_limit<%3u> tx_hop_limit<%3u>\n",
            BLOG_IPV6_ADDR(bTupleV6_p->saddr), ntohs(bTupleV6_p->port.source),
            BLOG_IPV6_ADDR(bTupleV6_p->daddr), ntohs(bTupleV6_p->port.dest),
            bTupleV6_p->rx_hop_limit, bTupleV6_p->tx_hop_limit );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_l2_dump
 * Description  : parse and dump the contents of all L2 headers
 *  bHdr_p      : Pointer to logged header
 *------------------------------------------------------------------------------
 */
void blog_l2_dump( BlogHeader_t * bHdr_p )
{
    register int i, ix, length, offset = 0;
    BlogEncap_t type;
    char * value = bHdr_p->l2hdr;

    for ( ix=0; ix<bHdr_p->count; ix++ )
    {
        type = bHdr_p->encap[ix];

        switch ( type )
        {
            case PPP_1661   : length = BLOG_PPP_HDR_LEN;    break;
            case PPPoE_2516 : length = BLOG_PPPOE_HDR_LEN;  break;
            case VLAN_8021Q : length = BLOG_VLAN_HDR_LEN;   break;
            case ETH_802x   : length = BLOG_ETH_HDR_LEN;    break;
            case BCM_SWC    : 
                              if ( *((uint16_t *)(bHdr_p->l2hdr + 12) ) 
                                   == htons(BLOG_ETH_P_BRCM4TAG))
                                  length = BLOG_BRCM4_HDR_LEN;
                              else
                                  length = BLOG_BRCM6_HDR_LEN;
                              break;

            case PLD_IPv4   :
            case PLD_IPv6   :
            case DEL_IPv4   :
            case DEL_IPv6   :
            case BCM_XPHY   :
            default         : printk( "Unsupported type %d\n", type );
                              return;
        }

        printk( "\tENCAP %d. %10s +%2d %2d [ ",
                ix, strBlogEncap[type], offset, length );

        for ( i=0; i<length; i++ )
            printk( "%02x ", (uint8_t)value[i] );

        offset += length;
        value += length;

        printk( "]\n" );
    }
}

void blog_virdev_dump( Blog_t * blog_p )
{
    int i;
    int dev_dir;

    printk( "    VirtDev: ");

    for (i=0; i<MAX_VIRT_DEV; i++)
    {
        struct net_device *dev_p = (struct net_device *) blog_p->virt_dev_p[i];

        if ( dev_p == (void *)NULL ) continue;

        dev_dir = DEV_DIR(dev_p);
        dev_p = DEVP_DETACH_DIR( dev_p );
        printk("<%p: %s: %d: %d> ", dev_p, dev_p ? dev_p->name : " ", dev_dir, dev_p->mtu);
    }

    printk("\n");
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_grerx_dump
 * Description  : Dump the contents of a BlogTuple_t object.
 *  blog_p      : Pointer to the Blog
 *------------------------------------------------------------------------------
 */
static void blog_grerx_dump( Blog_t *blog_p )
{
    BlogGre_t *bGreRx_p = &blog_p->grerx; 
    int i;
    char *value;

    printk( "    GRE RX: hlen<%u> l2_hlen<%u> ipid<0x%04x:%u> flags<0x%04x> rx_tunl_p<%p>\n",
            bGreRx_p->hlen, bGreRx_p->l2_hlen, ntohs(bGreRx_p->ipid), 
            ntohs(bGreRx_p->ipid), bGreRx_p->gre_flags.u16, blog_p->rx_tunl_p 
            ); 

    if ( blog_p->rx.info.bmap.GRE && blog_p->rx.info.bmap.GRE_ETH && 
        (bGreRx_p->l2_hlen>0) )
    {
        value = bGreRx_p->l2hdr;
        printk( "\t[ ");
        for ( i=0; i<bGreRx_p->l2_hlen; i++ )
            printk( "%02x ", (uint8_t)value[i] );
        printk( "]\n" );
    }
}


/*
 *------------------------------------------------------------------------------
 * Function     : blog_gretx_dump
 * Description  : Dump the contents of a BlogTuple_t object.
 *  blog_p      : Pointer to the Blog
 *------------------------------------------------------------------------------
 */
static void blog_gretx_dump( Blog_t *blog_p )
{
    BlogGre_t *bGreTx_p = &blog_p->gretx; 
    int i;
    char *value;

    printk( "    GRE TX: hlen<%u> l2_hlen<%u> ipid<0x%04x:%u> flags<0x%04x> tx_tunl_p<%p>\n",
            bGreTx_p->hlen, bGreTx_p->l2_hlen, ntohs(bGreTx_p->ipid), 
            ntohs(bGreTx_p->ipid), bGreTx_p->gre_flags.u16, blog_p->tx_tunl_p ); 

    if ( blog_p->tx.info.bmap.GRE && blog_p->tx.info.bmap.GRE_ETH && 
        (bGreTx_p->l2_hlen>0) )
    {
        value = bGreTx_p->l2hdr;
        printk( "\t[ ");
        for ( i=0; i<bGreTx_p->l2_hlen; i++ )
            printk( "%02x ", (uint8_t)value[i] );
        printk( "]\n" );
    }
}

void blog_lock(void)
{
    BLOG_LOCK_BH();
}

void blog_unlock(void)
{
    BLOG_UNLOCK_BH();
}

void blog_fdb_dump(Blog_t *blog_p)
{
    struct net_bridge_fdb_entry *src_fdb_p;
    struct net_bridge_fdb_entry *dst_fdb_p;

    /* For learnt flows bridge FDB pointer is stored in NPE */
    src_fdb_p = (struct net_bridge_fdb_entry *) 
        (blog_p->fdb[0] ? blog_p->fdb[0] : NULL);

    dst_fdb_p = (struct net_bridge_fdb_entry *) 
        (blog_p->fdb[1] ? blog_p->fdb[1] : NULL);

    printk( "\tfdb_src<%p> key[0x%08x] fdb_dst<%p> key[0x%08x]\n",
            src_fdb_p, src_fdb_p ? src_fdb_p->fdb_key : 0, 
            dst_fdb_p, dst_fdb_p ? dst_fdb_p->fdb_key : 0 );

    printk( "\tMAC src<%02x:%02x:%02x:%02x:%02x:%02x> dst<%02x:%02x:%02x:%02x:%02x:%02x>\n", 
            blog_p->src_mac.u8[0], blog_p->src_mac.u8[1], blog_p->src_mac.u8[2], 
            blog_p->src_mac.u8[3], blog_p->src_mac.u8[4], blog_p->src_mac.u8[5], 
            blog_p->dst_mac.u8[0], blog_p->dst_mac.u8[1], blog_p->dst_mac.u8[2],
            blog_p->dst_mac.u8[3], blog_p->dst_mac.u8[4], blog_p->dst_mac.u8[5] ); 
}

void blog_ct_dump(Blog_t *blog_p)
{
#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
    blog_npe_t *npe_pld_p = (blog_npe_t *)blog_p->npe_p[BLOG_NPE_PLD];
    blog_npe_t *npe_del_p = (blog_npe_t *)blog_p->npe_p[BLOG_NPE_DEL];
    struct nf_conn *nwe_pld_p;
    struct nf_conn *nwe_del_p;

    /* For learnt flows NFCT pointer is stored in NPE */
    nwe_pld_p = (struct nf_conn *) (npe_pld_p ? npe_pld_p->nwe_p : NULL);
    nwe_del_p = (struct nf_conn *) (npe_del_p ? npe_del_p->nwe_p : NULL);

    if (!nwe_pld_p && !nwe_del_p)
    {
        nwe_pld_p = blog_p->ct_p[BLOG_CT_PLD];
        nwe_del_p = blog_p->ct_p[BLOG_CT_DEL];
    }

    printk("\tdelCt<%p> key[0x%08x:%d] pldCt<%p> key[0x%08x:%d]\n",
            nwe_del_p, nwe_del_p ?
            nwe_del_p->blog_key[blog_p->nf_dir_del] : 0, blog_p->nf_dir_del,
            nwe_pld_p, nwe_pld_p ?
            nwe_pld_p->blog_key[blog_p->nf_dir_del] : 0, blog_p->nf_dir_del);

    if (nwe_pld_p)
        blog_nfct_dump(nwe_pld_p, blog_p->nf_dir_pld);

    if (nwe_del_p)
        blog_nfct_dump(nwe_del_p, blog_p->nf_dir_del);
#endif
}

void blog_map_dump(Blog_t *blog_p)
{
#if defined(CONFIG_BCM_KF_MAP) || defined(CONFIG_BCM_MAP_MODULE)
    blog_npe_t *npe_map_p = (blog_npe_t *)blog_p->npe_p[BLOG_NPE_MAP];
    struct map_tuple *map_p = (struct map_tuple *) blog_p->map_p;
    struct map_tuple *nwe_map_p = npe_map_p ? npe_map_p->nwe_p : NULL;

    /* For learnt flows MAP pointer is stored in NPE */
    nwe_map_p = nwe_map_p ? nwe_map_p : map_p;

    printk("\tMAP: map<0x%p>, key[US:0x%08x] key[DS:0x%08x] \n",
            nwe_map_p, 
            nwe_map_p ? nwe_map_p->blog_key[BLOG_PARAM1_MAP_DIR_US]: 0,  
            nwe_map_p ? nwe_map_p->blog_key[BLOG_PARAM1_MAP_DIR_DS]: 0);
#endif
}

void blog_mega_dump(Blog_t *blog_p)
{
#if defined(CONFIG_BCM_OVS)
    blog_npe_t *npe_mega_p = (blog_npe_t *)blog_p->npe_p[BLOG_NPE_MEGA];
    void *nwe_mega_p = npe_mega_p ? npe_mega_p->nwe_p : NULL;

    /* For learnt flows megaflow blog_key pointer is stored in NPE */
    nwe_mega_p = nwe_mega_p ? nwe_mega_p : blog_p->mega_p;

    printk("\tMEGA: mega_p<0x%p>, key[0x%08x]\n",
            nwe_mega_p, nwe_mega_p ? (unsigned int) blog_mega_get_key(nwe_mega_p): 0);
#endif
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_dump
 * Description  : Dump the contents of a Blog object.
 *  blog_p      : Pointer to the Blog_t object
 *------------------------------------------------------------------------------
 */
void blog_dump( Blog_t * blog_p )
{
    if ( blog_p == BLOG_NULL )
        return;

    blog_assertv( (_IS_BPTR_(blog_p)) );

    printk( "BLOG <%p> owner<%p> flags<0x%08x> tos_mode<%u:%u>\n"
            "\tL1 channel<%u> phyLen<%u> phy<%u> <%s> match<0x%08x>\n"
            "\thash<0x%08x> prot<%u> wl<0x%08x>\n"
            "\tprio<0x%08x> mark<0x%08x> minMTU<%u> tuple_offset<%u>\n"
            "\teth_type<0x%04x> vtag_num<%u> vtag[0]<0x%8x> vtag[1]<0x%08x>\n",
            blog_p, blog_p->skb_p, blog_p->flags,
            (int)blog_p->tos_mode_us, (int)blog_p->tos_mode_ds, 
            blog_p->key.l1_tuple.channel,
            rfc2684HdrLength[blog_p->key.l1_tuple.phyLen],
            blog_p->key.l1_tuple.phy,
            strBlogPhy[blog_p->key.l1_tuple.phyType], blog_p->key.match,
            blog_p->hash, blog_p->key.protocol, blog_p->wl,
            blog_p->priority, (uint32_t)blog_p->mark, blog_p->minMtu, 
            blog_p->tuple_offset,
            ntohs(blog_p->eth_type), blog_p->vtag_num, 
            ntohl(blog_p->vtag[0]), ntohl(blog_p->vtag[1]));
    
    blog_ct_dump(blog_p);
    blog_map_dump(blog_p);
    blog_mega_dump(blog_p);
    blog_fdb_dump(blog_p);

    printk( "\tfeature<0x%08x> offsets[0]<0x%08x> offsets[1]<0x%08x> \n"
            "\tprehook<0x%p> posthook<0x%p>\n",
            blog_p->feature, blog_p->offsets[0], blog_p->offsets[1],
            blog_p->preHook, blog_p->postHook );

    printk( "  RX count<%u> channel<%02u> bmap<0x%08x> phyLen<%u> "
            "phyHdr<%u> %s\n"
            "     wan_qdisc<%u> multicast<%u> fkbInSkb<%u>\n",
            blog_p->rx.count, blog_p->rx.info.channel,
            blog_p->rx.info.hdrs,
            rfc2684HdrLength[blog_p->rx.info.phyHdrLen],
            blog_p->rx.info.phyHdr, 
            strBlogPhy[blog_p->rx.info.phyHdrType],
            blog_p->rx.wan_qdisc,
            blog_p->rx.multicast, blog_p->rx.fkbInSkb );

    blog_l2_dump( &blog_p->rx );

    printk("    Del Tuple:\n" );
    if ( blog_p->rx.info.bmap.DEL_IPv4 )
        blog_tuple_dump( &blog_p->delrx_tuple );
    else if ( blog_p->rx.info.bmap.DEL_IPv6 )
    {
        if ( blog_p->rx.info.bmap.GRE )
        {
            if ( !blog_p->tx.info.bmap.GRE )
                blog_tupleV6_dump( &blog_p->del_tupleV6 );
        }
        else
            blog_tupleV6_dump( &blog_p->tupleV6 );
    }

    if ( blog_p->rx.info.bmap.HDR0_IPv4 )
    {
        printk("    Hdr0 Tuple:\n" );
        blog_tuple_dump( &blog_p->rx_tuple[0] );
    }

    printk("    Payload Tuple:\n" );
    if ( blog_p->rx.info.bmap.PLD_IPv4 )
        blog_tuple_dump( &blog_p->rx.tuple );
    else if ( blog_p->rx.info.bmap.PLD_IPv6 )
        blog_tupleV6_dump( &blog_p->tupleV6 );

    if ( blog_p->rx.info.bmap.GRE )
        blog_grerx_dump( blog_p );

    printk("  TX count<%u> channel<%02u> bmap<0x%08x> phyLen<%u> "
           "phyHdr<%u> %s\n",
            blog_p->tx.count, blog_p->tx.info.channel,
            blog_p->tx.info.hdrs, 
            rfc2684HdrLength[blog_p->tx.info.phyHdrLen],
            blog_p->tx.info.phyHdr, 
            strBlogPhy[blog_p->tx.info.phyHdrType] );
    if ( blog_p->tx_dev_p )
        blog_netdev_dump( blog_p->tx_dev_p );

    blog_l2_dump( &blog_p->tx );

    printk("    Del Tuple:\n" );
    if ( blog_p->tx.info.bmap.DEL_IPv4 )
        blog_tuple_dump( &blog_p->deltx_tuple );
    else if ( blog_p->tx.info.bmap.DEL_IPv6 )
    {
        if ( blog_p->tx.info.bmap.GRE )
            blog_tupleV6_dump( &blog_p->del_tupleV6 );
        else
            blog_tupleV6_dump( &blog_p->tupleV6 );
    }

    if ( blog_p->tx.info.bmap.HDR0_IPv4 )
    {
        printk("    Hdr0 Tuple:\n" );
        blog_tuple_dump( &blog_p->tx_tuple[0] );
    }

    printk("    Payload Tuple:\n" );
    if ( blog_p->tx.info.bmap.PLD_IPv4 )
        blog_tuple_dump( &blog_p->tx.tuple );
    else if ( blog_p->tx.info.bmap.PLD_IPv6 )
        blog_tupleV6_dump( &blog_p->tupleV6 );

    if ( blog_p->tx.info.bmap.GRE )
        blog_gretx_dump( blog_p );


    blog_virdev_dump( blog_p );

#if defined(CC_BLOG_SUPPORT_DEBUG)
    printk( "\t\textends<%d> free<%d> used<%d> HWM<%d> fails<%d>\n",
            blog_extends, blog_cnt_free, blog_cnt_used, blog_cnt_hwm,
            blog_cnt_fails );
#endif
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_getTxMtu
 * Description  : Gets unadjusted mtu from tx network devices associated with blog.
 *  blog_p      : Pointer to the Blog_t object
 *------------------------------------------------------------------------------
 */
uint16_t blog_getTxMtu(Blog_t * blog_p)
{
    int     i;
    uint16_t  minMtu;
    void *  dir_dev_p; 
    struct net_device *  dev_p;

    dev_p = (struct net_device *)blog_p->tx_dev_p;
    if (dev_p)
        minMtu = dev_p->mtu;
    else
        minMtu = 0xFFFF;
    
    
    for (i = 0; i < MAX_VIRT_DEV; i++)
    {
        dir_dev_p = blog_p->virt_dev_p[i];
        if ( dir_dev_p == (void *)NULL ) 
            continue;
        if ( IS_RX_DIR(dir_dev_p) )
            continue;
        dev_p = (struct net_device *)DEVP_DETACH_DIR(dir_dev_p);
#ifdef CONFIG_BCM_IGNORE_BRIDGE_MTU
        /* Exclude Bridge device - bridge always has the least MTU of all attached interfaces -
         * irrespective of this specific flow path */
        if (dev_p && !(dev_p->priv_flags&IFF_EBRIDGE) && dev_p->mtu < minMtu)
#else
        if (dev_p && dev_p->mtu < minMtu)
#endif
        {
            minMtu = dev_p->mtu;
        }
    }

    if (MAPT_UP(blog_p))
        minMtu -= (sizeof(struct ipv6hdr) - sizeof(struct iphdr)) ;

    blog_print( "minMtu <%d>", (int)minMtu );

    return minMtu;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_set_len_tbl
 * Description  : Set the values learnt from iptables rule for length
 *                prioritization.
 * Parameters   :
 *  val[]       : Array that stores {minimum length, maximum length, original
 *                mark, target mark}.
 * Returns      :
 *  Zero        : Success
 *  Non-zero    : Fail
 *------------------------------------------------------------------------------
 */
int blog_set_len_tbl(uint32_t val[])
{
    if ( blog_len_tbl_idx >= BLOG_MAX_LEN_TBLSZ )
    {
        blog_print("%s: Length priority entries exceed the table size.\n", __func__);
        return -1;
    }

    BLOG_LOCK_TBL();

    blog_len_tbl[blog_len_tbl_idx][BLOG_MIN_LEN_INDEX] = val[BLOG_MIN_LEN_INDEX];
    blog_len_tbl[blog_len_tbl_idx][BLOG_MAX_LEN_INDEX] = val[BLOG_MAX_LEN_INDEX];
    blog_len_tbl[blog_len_tbl_idx][BLOG_ORIGINAL_MARK_INDEX] = val[BLOG_ORIGINAL_MARK_INDEX];
    blog_len_tbl[blog_len_tbl_idx][BLOG_TARGET_MARK_INDEX] = val[BLOG_TARGET_MARK_INDEX];
    blog_len_tbl_idx++;

    BLOG_UNLOCK_TBL();
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_clr_len_tbl
 * Description  : Clear the table for length prioritization.
 * Returns      :
 *  Zero        : Success
 *  Non-zero    : Fail
 *------------------------------------------------------------------------------
 */
int blog_clr_len_tbl()
{
    int i;

    BLOG_LOCK_TBL();

    for ( i = 0; i < BLOG_MAX_LEN_TBLSZ; i++ )
    {
        blog_len_tbl[i][BLOG_MIN_LEN_INDEX] = BLOG_INVALID_UINT32;
        blog_len_tbl[i][BLOG_MAX_LEN_INDEX] = BLOG_INVALID_UINT32;
        blog_len_tbl[i][BLOG_ORIGINAL_MARK_INDEX] = BLOG_INVALID_UINT32;
        blog_len_tbl[i][BLOG_TARGET_MARK_INDEX] = BLOG_INVALID_UINT32;
    }
    blog_len_tbl_idx = 0;

    BLOG_UNLOCK_TBL();
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_set_dscp_tbl
 * Description  : Set the values learnt from iptables rule for DSCP mangle.
 * Parameters   :
 *  idx         : DSCP match value
 *  val         : DSCP target value
 * Returns      :
 *  Zero        : Success
 *  Non-zero    : Fail
 *------------------------------------------------------------------------------
 */
int blog_set_dscp_tbl(uint8_t idx, uint8_t val)
{
    BLOG_LOCK_TBL();

    blog_dscp_tbl[idx] = val;

    BLOG_UNLOCK_TBL();
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_clr_dscp_tbl
 * Description  : Clear the table for DSCP mangle.
 * Returns      :
 *  Zero        : Success
 *  Non-zero    : Fail
 *------------------------------------------------------------------------------
 */
int blog_clr_dscp_tbl()
{
    int i;

    BLOG_LOCK_TBL();

    for ( i = 0; i < BLOG_MAX_DSCP_TBLSZ; i++ )
        blog_dscp_tbl[i] = BLOG_INVALID_UINT8;

    BLOG_UNLOCK_TBL();
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_set_tos_tbl
 * Description  : Set the values learnt from iptables rule for TOS mangle.
 * Parameters   :
 *  idx         : TOS match value
 *  val         : TOS target value
 * Returns      :
 *  Zero        : Success
 *  Non-zero    : Fail
 *------------------------------------------------------------------------------
 */
int blog_set_tos_tbl(uint8_t idx, uint8_t val)
{
    BLOG_LOCK_TBL();

    blog_tos_tbl[idx] = val;

    BLOG_UNLOCK_TBL();
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_clr_tos_tbl
 * Description  : Clear the table for TOS mangle.
 * Returns      :
 *  Zero        : Success
 *  Non-zero    : Fail
 *------------------------------------------------------------------------------
 */
int blog_clr_tos_tbl()
{
    int i;

    BLOG_LOCK_TBL();

    for ( i = 0; i < BLOG_MAX_TOS_TBLSZ; i++ )
        blog_tos_tbl[i] = BLOG_INVALID_UINT8;

    BLOG_UNLOCK_TBL();
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_pre_mod_hook
 * Description  : Called by flow cache prior to the modification phase.
 * Parameters   :
 *  blog_p      : Pointer to the Blog_t object
 *  nbuff_p     : Pointer to a NBuff
 * Returns      :
 *  PKT_DONE    : Success
 *  PKT_DROP    : Drop the packet
 *  PKT_NORM    : Return to normal network stack
 *------------------------------------------------------------------------------
 */
int blog_pre_mod_hook(Blog_t *blog_p, void *nbuff_p)
{
    FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(nbuff_p);
    BlogIpv4Hdr_t *ip_p = (BlogIpv4Hdr_t *)&fkb_p->data[blog_p->ip_offset];

    if ( blog_p->lenPrior )
    {
        int i;

        for ( i = blog_len_tbl_idx; i >= 0; i-- )
        {
            if ( (ip_p->len >= blog_len_tbl[i][BLOG_MIN_LEN_INDEX]) &&
                 (ip_p->len <= blog_len_tbl[i][BLOG_MAX_LEN_INDEX]) )
            {
                blog_mangl_params[BLOG_LEN_PARAM_INDEX] = blog_len_tbl[i][BLOG_TARGET_MARK_INDEX];
                break;
            }
            else
                blog_mangl_params[BLOG_LEN_PARAM_INDEX] = blog_len_tbl[i][BLOG_ORIGINAL_MARK_INDEX];
        }
    }

    if ( blog_p->dscpMangl )
    {
        blog_mangl_params[BLOG_DSCP_PARAM_INDEX] = blog_dscp_tbl[ip_p->tos>>XT_DSCP_SHIFT];
    }

    if ( blog_p->tosMangl )
    {
        blog_mangl_params[BLOG_TOS_PARAM_INDEX] = blog_tos_tbl[ip_p->tos];
    }

    return PKT_DONE;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_post_mod_hook
 * Description  : Called by flow cache after the modification phase.
 * Parameters   :
 *  blog_p      : Pointer to the Blog_t object
 *  nbuff_p     : Pointer to a NBuff
 * Returns      :
 *  Zero        : Success
 *  Non-zero    : Fail
 *------------------------------------------------------------------------------
 */
int blog_post_mod_hook(Blog_t *blog_p, void *nbuff_p)
{
    FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(nbuff_p);

    if ( blog_p->lenPrior )
    {
        fkb_p->mark = blog_mangl_params[BLOG_LEN_PARAM_INDEX];
    }

    if ( blog_p->dscpMangl )
    {
        if ( blog_mangl_params[BLOG_DSCP_PARAM_INDEX] != BLOG_INVALID_UINT8 )
        {
            struct iphdr *ip_p = (struct iphdr *)(fkb_p->data + blog_p->ip_offset +
                (sizeof(blog_p->tx.l2hdr) - sizeof(blog_p->rx.l2hdr)));
            ipv4_change_dsfield(ip_p, (uint8_t)(~XT_DSCP_MASK),
                (uint8_t)(blog_mangl_params[BLOG_DSCP_PARAM_INDEX] << XT_DSCP_SHIFT));
        }
    }

    if ( blog_p->tosMangl )
    {
        if ( blog_mangl_params[BLOG_TOS_PARAM_INDEX] != BLOG_INVALID_UINT8 )
        {
            struct iphdr *ip_p = (struct iphdr *)(fkb_p->data + blog_p->ip_offset +
                (sizeof(blog_p->tx.l2hdr) - sizeof(blog_p->rx.l2hdr)));
            ipv4_change_dsfield(ip_p, 0, (uint8_t)blog_mangl_params[BLOG_TOS_PARAM_INDEX]);
        }
    }

    return 0;
}


static struct notifier_block net_nb =
{
    .notifier_call = blog_notify_netevent,
};

#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE) || defined(CONFIG_ACCEL_PPTP)
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
            if ( unlikely(*(uint8_t*)ip_p != 0x45) )
            {
                blog_print( "ABORT IP ver<%d> len<%d>", ip_p->ver, ip_p->ihl );
                return 0;
            }
            ip_proto = ip_p->proto;
        }
        else if ( ver == 6 )
        {
            /*  Support extension headers? */
            ip_proto = ((BlogIpv6Hdr_t*)ip_p)->nextHdr;
        }

        if ( ip_proto == BLOG_IPPROTO_GRE ) 
        {
            hdr_p = (char *)ip_p;
            hdr_p += (ver == 4)? BLOG_IPV4_HDR_LEN : BLOG_IPV6_HDR_LEN;
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

#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE)
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
        else if ( ver == 6 )
        {
            blog_print( "BLOG PARSE IPv6:" );

            /* Support extension headers? */
            if ( ((BlogIpv6Hdr_t*)ip_p)->nextHdr == BLOG_IPPROTO_GRE )
            {
                blog_print( "BLOG PARSE GRE:" );
                if (blog_gre6_rcv_check_fn != NULL)
                    ret = blog_gre6_rcv_check_fn( dev_p, (BlogIpv6Hdr_t *)ip_p, 
                        fkb_p->len - ((uintptr_t)ip_p - (uintptr_t)fkb_p->data), 
                        tunl_pp, pkt_seqno_p );
            }
        }
    }

pkt_not_gre:
    return ret;
}

void blog_gre_xmit(struct sk_buff *skb_p, uint32_t h_proto)
{
    if (skb_p && skb_p->tunl && blog_gre_tunnel_accelerated())
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
                skb_p->tunl, skb_p, skb_p->data, skb_p->len, ip_p, 
                skb_p->len - (uint32_t)((uintptr_t) ip_p - (uintptr_t) skb_p->data)); 

            if ( ver == 4 )
            {
                if (blog_gre_xmit_update_fn != NULL)
                    blog_gre_xmit_update_fn((struct ip_tunnel *)skb_p->tunl, ip_p, 
                        skb_p->len - ((uintptr_t) ip_p - (uintptr_t) skb_p->data));
            }
            else if ( ver == 6 )
            {
                if (blog_gre6_xmit_update_fn != NULL)
                    blog_gre6_xmit_update_fn((struct ip6_tnl *)skb_p->tunl, (BlogIpv6Hdr_t *)ip_p, 
                        skb_p->len - ((uintptr_t) ip_p - (uintptr_t) skb_p->data));
            }
        }
    }
}
#endif

#if defined(CONFIG_ACCEL_PPTP) 

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
            if (gre_flags.seqIe && gre_flags.keyIe && gre_flags.ver) 
            {	
            	call_id = ntohs(*(uint16_t*) (grehdr_p + 3));
            	seqNum = ntohl(*(uint32_t*) (grehdr_p + 4));
            	
            	saddr  = blog_read32_align16( (uint16_t *)&ip_p->sAddr );
            	daddr  = blog_read32_align16( (uint16_t *)&ip_p->dAddr );

            	blog_print( "call id = %d, seqNum = %d, daddr = %X\n", 
            	             call_id, seqNum, daddr );
                if(gre_flags.ackIe) /* the pkt is PPTP with ack number */
                {	
                	ackNum = ntohl(*(uint32_t*) (grehdr_p + 6));
                	blog_print( "ackNum = %d \n", ackNum );
                }

            	if (blog_pptp_xmit_update_fn != NULL)
            	   blog_pptp_xmit_update_fn(call_id, seqNum, ackNum, daddr);
            } 
        }
    }
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function     : blog_ptm_us_bonding
 * Description  : Sets/Clears the PTM US bonding mode for the flow
 * Parameters   :
 *  blog_p      : Pointer to a blog
 *  mode        : enable=1, disable=0 
 * Note         : FIXME This is a temporary fix and should be removed shortly.
 *------------------------------------------------------------------------------
 */
void blog_ptm_us_bonding( struct sk_buff *skb_p, int mode )
{
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );

    if ((skb_p != NULL) &&
        ( likely(skb_p->blog_p != BLOG_NULL) ))
    {
        skb_p->blog_p->ptm_us_bond = mode;
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_is_config_netdev_mac
 * Description  : This function checks whether the device's MAC address should be
 *                used for addition/deletion to/from host MAC address table.
 *                On the LAN side, the bridge device MAC address is used instead
 *                of each LAN device's MAC address.
 *                There are some device's which are ignored like: point-to-point, 
 *                PPP, etc.
 *                Important devices are: WAN and LAN bridge.
 * Parameters   :
 *  ptr         : Pointer to the net device
 *  incl_vmacs  : include macs of virtual devs(ex:containers etc..)
 * Return values:
 *  1           : Use the device MAC address 
 *  0           : Ignore the device MAC address 
 *------------------------------------------------------------------------------
 */
int blog_is_config_netdev_mac(void *ptr, unsigned long incl_vmacs)
{
	struct net_device *dev_p = (struct net_device *) ptr;
#if defined(CONFIG_BCM_KF_WANDEV)
	struct net_device *root_dev_p = netdev_path_get_root(dev_p);
#endif

	if ( (dev_p->priv_flags & IFF_POINTOPOINT) || (dev_p->priv_flags & IFF_ISATAP) )
		return 0;

#if defined(CONFIG_BCM_KF_PPP)
	if (dev_p->priv_flags & IFF_PPP) 
		return 0;
#endif
	if(incl_vmacs){
		return 1;
	}else{
		if ( (dev_p->priv_flags & IFF_EBRIDGE)
#if defined(CONFIG_BCM_OVS)
            || blog_is_ovs_internal_dev((void *)dev_p)
#endif
#if defined(CONFIG_BCM_KF_WANDEV)
			|| (root_dev_p->priv_flags & IFF_WANDEV)
#endif
		   )
			return 1;
		else
			return 0;
	}
}

void blog_get_stats(void)
{
    blog_skip_reason_t skip_reason;
    blog_free_reason_t free_reason;

    blog_ctx_p->info_stats.blog_skip = 0;
    for (skip_reason = blog_skip_reason_unknown; 
            skip_reason < blog_skip_reason_max; skip_reason++) 
    {
        blog_ctx_p->info_stats.blog_skip 
                += blog_ctx_p->blog_skip_stats_table[skip_reason]; 
    }

    blog_ctx_p->info_stats.blog_free = 0; 
    for (free_reason = blog_free_reason_unknown; 
            free_reason < blog_free_reason_max; free_reason++) 
    {
        blog_ctx_p->info_stats.blog_free 
            += blog_ctx_p->blog_free_stats_table[free_reason]; 
    }

    printk("blog_info stats:\n");
    printk("blog_total = %u blog_avail = %u blog_mem_fails = %u\n", 
            blog_ctx_p->blog_total, blog_ctx_p->blog_avail, 
            blog_ctx_p->blog_mem_fails);
    printk("blog_extends = %u blog_extend_fails = %u\n", 
            blog_ctx_p->blog_extends, blog_ctx_p->blog_extend_fails);
    printk("blog_extend_max_engg = %u blog_extend_size_engg = %u\n",
            (unsigned int)BLOG_EXTEND_MAX_ENGG, (unsigned int)BLOG_EXTEND_SIZE_ENGG);
    printk("blog_get  = %u blog_put = %u\n", 
            blog_ctx_p->info_stats.blog_get, blog_ctx_p->info_stats.blog_put);
    printk("blog_xfer = %u blog_clone = %u blog_copy = %u\n", 
            blog_ctx_p->info_stats.blog_xfer, blog_ctx_p->info_stats.blog_clone,
            blog_ctx_p->info_stats.blog_copy);
    printk("blog_skip = %u blog_free = %u\n", 
            blog_ctx_p->info_stats.blog_skip, blog_ctx_p->info_stats.blog_free);
    printk("blog_min_avail = %u\n", blog_ctx_p->info_stats.blog_min_avail);

    printk("\nblog_skip stats:\n");
    for (skip_reason = blog_skip_reason_unknown; 
            skip_reason < blog_skip_reason_max; skip_reason++) 
        printk("%s = %u\n", str_blog_skip_reason[skip_reason], 
                blog_ctx_p->blog_skip_stats_table[skip_reason]);

    printk("\nblog_free stats:\n");
    for (free_reason = blog_free_reason_unknown; 
            free_reason < blog_free_reason_max; free_reason++) 
        printk("%s = %u\n", str_blog_free_reason[free_reason], 
                blog_ctx_p->blog_free_stats_table[free_reason]);
}

void blog_reset_stats(void)
{
    memset(&blog_ctx_g.info_stats, 0, sizeof(blog_info_stats_t));
    blog_ctx_g.info_stats.blog_min_avail = U32_MAX;
    memset(&blog_ctx_g.blog_skip_stats_table, 0, sizeof(uint32_t) * blog_skip_reason_max);
    memset(&blog_ctx_g.blog_free_stats_table, 0, sizeof(uint32_t) * blog_free_reason_max);
}


/*
 *------------------------------------------------------------------------------
 * Function Name: blog_drv_ioctl
 * Description  : Main entry point to handle user applications IOCTL requests
 *                Flow Cache Utility.
 * Returns      : 0 - success or error
 *------------------------------------------------------------------------------
 */
static long blog_drv_ioctl(struct file *filep, unsigned int command, 
                           unsigned long arg)
{
    blog_ioctl_t cmd;
    int ret = BLOG_SUCCESS;

    if ( command > BLOG_IOCTL_INVALID )
        cmd = BLOG_IOCTL_INVALID;
    else
        cmd = (blog_ioctl_t)command;

    blog_print( "cmd<%d> %s arg<%lu>",
                command, blog_drv_ioctl_name[cmd], arg );

    /* protect the fc linked lists by disabling all interrupts */
    BLOG_LOCK_BH();

    switch ( cmd )
    {
        case BLOG_IOCTL_GET_STATS:
        {
            blog_get_stats();
            break;
        }

        case BLOG_IOCTL_RESET_STATS:
        {
            blog_reset_stats();
            break;
        }

        case BLOG_IOCTL_DUMP_BLOG:
        {
            blog_ctx_p->blog_dump = arg;
            break;
        }

        default :
        {
            printk( "Invalid cmd[%u]\n", command );
            ret = BLOG_ERROR;
        }
    }

    BLOG_UNLOCK_BH();

    return ret;

} /* blog_drv_ioctl */

/*
 *------------------------------------------------------------------------------
 * Function Name: blog_drv_open
 * Description  : Called when a user application opens this device.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
int blog_drv_open(struct inode *inode, struct file *filp)
{
    blog_print( "Access Blog Char Device" );
    return BLOG_SUCCESS;
} /* blog_drv_open */



/*
 *------------------------------------------------------------------------------
 * Function     : __init_blog
 * Description  : Incarnates the blog system during kernel boot sequence,
 *                in phase subsys_initcall()
 *------------------------------------------------------------------------------
 */
static int __init __init_blog( void )
{
    /* Clear the feature tables for per-packet modification */
    blog_clr_len_tbl();
    blog_clr_dscp_tbl();
    blog_clr_tos_tbl();
    blog_reset_stats();

    /* Register a character device for Ioctl handling */
    if ( register_chrdev(BLOG_DRV_MAJOR, BLOG_DRV_NAME, &blog_drv_g.fops) )
    {
        printk( CLRerr "BLOG %s Unable to get major number <%d>" CLRnl,
                  __FUNCTION__, BLOG_DRV_MAJOR);
        return BLOG_ERROR;
    }

    nfskb_p = alloc_skb( 0, GFP_ATOMIC );
    blog_cttime_update_fn = (blog_cttime_upd_t) NULL;
    blog_extend( BLOG_POOL_SIZE_ENGG ); /* Build preallocated pool */
    BLOG_DBG( printk( CLRb "BLOG blog_dbg<%p> = %d\n"
                           "%d Blogs allocated of size %lu" CLRnl,
                           &blog_dbg, blog_dbg,
                           BLOG_POOL_SIZE_ENGG, (unsigned long)sizeof(Blog_t) ););
    register_netevent_notifier(&net_nb);

    printk( CLRb "BLOG %s Initialized" CLRnl, BLOG_VERSION );
    return 0;
}

subsys_initcall(__init_blog);

EXPORT_SYMBOL(_blog_emit);
EXPORT_SYMBOL(blog_extend);

EXPORT_SYMBOL(strBlogAction);
EXPORT_SYMBOL(strBlogEncap);

EXPORT_SYMBOL(strRfc2684);
EXPORT_SYMBOL(rfc2684HdrLength);
EXPORT_SYMBOL(rfc2684HdrData);

EXPORT_SYMBOL(blog_set_len_tbl);
EXPORT_SYMBOL(blog_clr_len_tbl);
EXPORT_SYMBOL(blog_set_dscp_tbl);
EXPORT_SYMBOL(blog_clr_dscp_tbl);
EXPORT_SYMBOL(blog_set_tos_tbl);
EXPORT_SYMBOL(blog_clr_tos_tbl);
EXPORT_SYMBOL(blog_pre_mod_hook);
EXPORT_SYMBOL(blog_post_mod_hook);

#else   /* !defined(CONFIG_BLOG) */

int blog_dbg = 0;

int blog_support_accel_mode_g = BLOG_ACCEL_MODE_L3; /* = CC_BLOG_SUPPORT_ACCEL_MODE; */
void blog_support_accel_mode(int accel_mode) {blog_support_accel_mode_g = BLOG_ACCEL_MODE_L3;}
int blog_support_get_accel_mode(void) {return blog_support_accel_mode_g;}

blog_accel_mode_set_t blog_accel_mode_set_fn = NULL;
blog_tcp_ack_mflows_set_t blog_tcp_ack_mflows_set_fn = NULL;

int blog_support_mcast_g = BLOG_MCAST_DISABLE; /* = CC_BLOG_SUPPORT_MCAST; */
void blog_support_mcast(int enable) {blog_support_mcast_g = BLOG_MCAST_DISABLE;}

/* = CC_BLOG_SUPPORT_MCAST_LEARN; */
int blog_support_mcast_learn_g = BLOG_MCAST_LEARN_DISABLE; 
void blog_support_mcast_learn(int enable) {blog_support_mcast_learn_g = BLOG_MCAST_LEARN_DISABLE;}

int blog_support_ipv6_g = BLOG_IPV6_DISABLE; /* = CC_BLOG_SUPPORT_IPV6; */
void blog_support_ipv6(int enable) {blog_support_ipv6_g = BLOG_IPV6_DISABLE;}

int blog_notify_proc_mode_g = BLOG_NOTIFY_PROC_MODE_NOW; 
void blog_set_notify_proc_mode(int mode) {blog_notify_proc_mode_g = mode;}

blog_cttime_upd_t blog_cttime_update_fn = (blog_cttime_upd_t) NULL;

int blog_gre_tunnel_accelerated_g = BLOG_GRE_DISABLE;
int blog_gre_tunnel_accelerated(void) { return blog_gre_tunnel_accelerated_g; }

int blog_support_gre_g = BLOG_GRE_DISABLE; /* = CC_BLOG_SUPPORT_GRE; */
void blog_support_gre(int enable) {blog_support_gre_g = BLOG_GRE_DISABLE;}

#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE)
blog_gre_rcv_check_t blog_gre_rcv_check_fn = NULL;
blog_gre_xmit_upd_t blog_gre_xmit_update_fn = NULL;
blog_gre6_rcv_check_t blog_gre6_rcv_check_fn = NULL;
blog_gre6_xmit_upd_t blog_gre6_xmit_update_fn = NULL;
#endif

blog_pptp_rcv_check_t blog_pptp_rcv_check_fn = NULL;
blog_pptp_xmit_upd_t blog_pptp_xmit_update_fn = NULL;
blog_pptp_xmit_get_t blog_pptp_xmit_get_fn = NULL;

blog_l2tp_rcv_check_t blog_l2tp_rcv_check_fn = NULL;

int blog_l2tp_tunnel_accelerated_g = BLOG_L2TP_DISABLE;
int blog_support_l2tp_g = BLOG_L2TP_DISABLE; /* = CC_BLOG_SUPPORT_l2TP; */
void blog_support_l2tp(int enable) {blog_support_l2tp_g = BLOG_L2TP_DISABLE;}

int blog_support_4o6_frag_g = BLOG_4O6_FRAG_DISABLE;
void blog_support_4o6_frag(int enable) {blog_support_4o6_frag_g = BLOG_4O6_FRAG_DISABLE;}


/* Stub functions for Blog APIs that may be used by modules */
Blog_t * blog_get( void ) { return BLOG_NULL; }
void     blog_put( Blog_t * blog_p ) { return; }

Blog_t * blog_skb( struct sk_buff * skb_p) { return BLOG_NULL; }
Blog_t * blog_fkb( struct fkbuff * fkb_p ) { return BLOG_NULL; }

Blog_t * blog_snull( struct sk_buff * skb_p ) { return BLOG_NULL; }
Blog_t * blog_fnull( struct fkbuff * fkb_p ) { return BLOG_NULL; }

void     blog_free( struct sk_buff * skb_p, blog_skip_reason_t reason ) { return; }

void     blog_skip( struct sk_buff * skb_p, blog_skip_reason_t reason ) { return; }
void     blog_xfer( struct sk_buff * skb_p, const struct sk_buff * prev_p )
         { return; }
void     blog_clone( struct sk_buff * skb_p, const struct blog_t * prev_p )
         { return; }
void     blog_copy(struct blog_t * new_p, const struct blog_t * prev_p)
         { return; }

int blog_iq( const struct sk_buff * skb_p ) { return BLOG_IQ_PRIO_LOW; }
int blog_fc_enabled(void) { return 0; };

void     blog_link( BlogNetEntity_t entity_type, Blog_t * blog_p,
                    void * net_p, uint32_t param1, uint32_t param2 ) { return; }

void     blog_notify( BlogNotify_t event, void * net_p,
                      unsigned long param1, unsigned long param2 ) { return; }

void blog_notify_async(BlogNotify_t event, void *net_p,
        unsigned long param1, unsigned long param2,
        blog_notify_async_cb_fn_t notify_cb_fn, void *notify_cb_data_p) { return; }

void blog_notify_async_wait(BlogNotify_t event, void *net_p,
        unsigned long param1, unsigned long param2) { return; }

unsigned long blog_request( BlogRequest_t event, void * net_p,
                       unsigned long param1, unsigned long param2 ) { return 0; }

int     blog_query( BlogQuery_t event, void * net_p,
           uint32_t param1, uint32_t param2, unsigned long param3 ) { return 0; }

BlogAction_t blog_filter( Blog_t * blog_p )
         { return PKT_NORM; }

BlogAction_t blog_sinit( struct sk_buff * skb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr )
         { return PKT_NORM; }

BlogAction_t blog_finit( struct fkbuff * fkb_p, void * dev_p,
                        uint32_t encap, uint32_t channel, uint32_t phyHdr )
         { return PKT_NORM; }

BlogAction_t blog_emit( void * nbuff_p, void * dev_p,
                        uint32_t encap, uint32_t channel, uint32_t phyHdr )
         { return PKT_NORM; }

int blog_iq_prio( struct sk_buff * skb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr )
         { return 1; }

void blog_bind( BlogDevRxHook_t blog_rx, BlogDevTxHook_t blog_tx,
                BlogNotifyHook_t blog_xx, BlogQueryHook_t blog_qr, 
                BlogBind_t bind) { return; }

void blog_bind_config( BlogScHook_t blog_sc, BlogSdHook_t blog_sd,
                       BlogClient_t client, BlogBind_t bind ) { return; }

int blog_flowevent_register_notifier(struct notifier_block *nb) { return 0;}

int blog_flowevent_unregister_notifier(struct notifier_block *nb) { return 0;}

void     blog( struct sk_buff * skb_p, BlogDir_t dir, BlogEncap_t encap,
               size_t len, void * data_p ) { return; }

void     blog_dump( Blog_t * blog_p ) { return; }

void     blog_lock(void) {return; }

void     blog_unlock(void) {return; }

uint16_t   blog_getTxMtu(Blog_t * blog_p) {return 0;}

uint32_t blog_activate( Blog_t * blog_p, BlogTraffic_t traffic,
                        BlogClient_t client ) { return 0; }

Blog_t * blog_deactivate( BlogActivateKey_t key, BlogTraffic_t traffic,
                          BlogClient_t client ) { return BLOG_NULL; }

#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE)
int blog_gre_rcv( struct fkbuff *fkb_p, void * dev_p, uint32_t h_proto, 
                  void **tunl_pp, uint32_t *pkt_seqno_p ) { return 1; }
void blog_gre_xmit(struct sk_buff *skb_p, uint32_t h_proto) { return; }
#endif

#if defined(CONFIG_ACCEL_PPTP) 
int blog_pptp_rcv( struct fkbuff *fkb_p, uint32_t h_proto, 
                    uint32_t *rcv_pktSeq) { return 1; }
void blog_pptp_xmit( struct sk_buff *skb_p, uint32_t h_proto ) { return; }
#endif

void blog_ptm_us_bonding( struct sk_buff *skb_p, int mode ) { return; }

int blog_is_config_netdev_mac(void *ptr, unsigned long incl_vmacs) { return 0; }
inline int blog_preemptible_task(void) { return 0;}

void blog_bind_packet_accelerator( BlogPaHook_t blog_pa, BlogBind_t bind ) { return; }

BlogAction_t blog_finit_args( struct fkbuff * fkb_p, void * dev_p,
                    uint32_t encap, uint32_t channel, uint32_t phyHdr, BlogFcArgs_t *fc_args ) { return 0; }
EXPORT_SYMBOL(blog_emit);

#endif  /* else !defined(CONFIG_BLOG) */

EXPORT_SYMBOL(blog_dbg);
EXPORT_SYMBOL(blog_support_accel_mode_g);
EXPORT_SYMBOL(blog_support_accel_mode);
EXPORT_SYMBOL(blog_support_get_accel_mode);
EXPORT_SYMBOL(blog_support_tcp_ack_mflows_g);
EXPORT_SYMBOL(blog_support_set_tcp_ack_mflows);
EXPORT_SYMBOL(blog_support_get_tcp_ack_mflows);
EXPORT_SYMBOL(blog_accel_mode_set_fn);
EXPORT_SYMBOL(blog_tcp_ack_mflows_set_fn);
EXPORT_SYMBOL(blog_ct_get_stats);
EXPORT_SYMBOL(blog_ct_put_stats_fn);
EXPORT_SYMBOL(blog_support_mcast_g);
EXPORT_SYMBOL(blog_support_mcast);
EXPORT_SYMBOL(blog_support_mcast_learn_g);
EXPORT_SYMBOL(blog_support_mcast_learn);
EXPORT_SYMBOL(blog_support_ipv6_g);
EXPORT_SYMBOL(blog_support_ipv6);
EXPORT_SYMBOL(blog_cttime_update_fn);
EXPORT_SYMBOL(blog_gre_tunnel_accelerated_g);
EXPORT_SYMBOL(blog_support_gre_g);
EXPORT_SYMBOL(blog_support_gre);
EXPORT_SYMBOL(blog_support_4o6_frag_g);
EXPORT_SYMBOL(blog_support_4o6_frag);
#if defined(CONFIG_NET_IPGRE) || defined(CONFIG_NET_IPGRE_MODULE)
EXPORT_SYMBOL(blog_gre_rcv_check_fn);
EXPORT_SYMBOL(blog_gre_xmit_update_fn);
EXPORT_SYMBOL(blog_gre6_rcv_check_fn);
EXPORT_SYMBOL(blog_gre6_xmit_update_fn);
EXPORT_SYMBOL(blog_gre_rcv);
EXPORT_SYMBOL(blog_gre_xmit);
#endif

EXPORT_SYMBOL(blog_pptp_rcv_check_fn);
EXPORT_SYMBOL(blog_pptp_xmit_update_fn); 
EXPORT_SYMBOL(blog_pptp_xmit_get_fn);

#if defined(CONFIG_ACCEL_PPTP) 
EXPORT_SYMBOL(blog_pptp_rcv);
EXPORT_SYMBOL(blog_pptp_xmit);
#endif

EXPORT_SYMBOL(blog_l2tp_tunnel_accelerated_g);
EXPORT_SYMBOL(blog_support_l2tp_g);
EXPORT_SYMBOL(blog_support_l2tp);
EXPORT_SYMBOL(blog_l2tp_rcv_check_fn);

EXPORT_SYMBOL(blog_get);
EXPORT_SYMBOL(blog_put);
EXPORT_SYMBOL(blog_skb);
EXPORT_SYMBOL(blog_fkb);
EXPORT_SYMBOL(blog_snull);
EXPORT_SYMBOL(blog_fnull);
EXPORT_SYMBOL(blog_free);
EXPORT_SYMBOL(blog_dump);
EXPORT_SYMBOL(blog_skip);
EXPORT_SYMBOL(blog_xfer);
EXPORT_SYMBOL(blog_clone);
EXPORT_SYMBOL(blog_copy);
EXPORT_SYMBOL(blog_iq);
EXPORT_SYMBOL(blog_fc_enabled);
EXPORT_SYMBOL(blog_gre_tunnel_accelerated);
EXPORT_SYMBOL(blog_link);
EXPORT_SYMBOL(blog_notify);
EXPORT_SYMBOL(blog_notify_async);
EXPORT_SYMBOL(blog_notify_async_wait);
EXPORT_SYMBOL(blog_bind_notify_evt_enqueue);
EXPORT_SYMBOL(blog_set_notify_proc_mode);
EXPORT_SYMBOL(blog_notify_proc_mode_g);
EXPORT_SYMBOL(blog_preemptible_task);
EXPORT_SYMBOL(blog_request);
EXPORT_SYMBOL(blog_query);
EXPORT_SYMBOL(blog_filter);
EXPORT_SYMBOL(blog_sinit);
EXPORT_SYMBOL(blog_finit);
EXPORT_SYMBOL(blog_finit_args);
EXPORT_SYMBOL(blog_lock);
EXPORT_SYMBOL(blog_unlock);
EXPORT_SYMBOL(blog_bind);
EXPORT_SYMBOL(blog_bind_config);
EXPORT_SYMBOL(blog_flowevent_register_notifier);
EXPORT_SYMBOL(blog_flowevent_unregister_notifier);
EXPORT_SYMBOL(blog_bind_packet_accelerator);
EXPORT_SYMBOL(blog_iq_prio);
EXPORT_SYMBOL(blog_getTxMtu);
EXPORT_SYMBOL(blog_activate);
EXPORT_SYMBOL(blog_deactivate);
EXPORT_SYMBOL(blog_ptm_us_bonding);
EXPORT_SYMBOL(blog_dhd_flow_update_fn);
EXPORT_SYMBOL(blog_is_config_netdev_mac);
EXPORT_SYMBOL(blog_get_hw_accel);

EXPORT_SYMBOL(blog);

#endif // defined(BCM_KF_BLOG)
