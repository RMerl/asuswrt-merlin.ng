#ifndef __FCACHE_H_INCLUDED__
#define __FCACHE_H_INCLUDED__

/*
*
* Patented Flow Cache Acceleration
* Patent no : US7908376B2
*
*
*  Copyright 2011, Broadcom Corporation
*
* <:label-BRCM:2007:proprietary:standard
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


/*
 *******************************************************************************
 * File Name : fcache.h
 * Description of Flow Cache is CONFIDENTIAL and available ONLY in fcache.c .
 *
 *  Version 0.1: Prototype
 *  Version 1.0: BCM963xx
 *  Version 1.1: Multicast
 *  Version 1.2: L4 protocol, L1
 *  Version 2.0: FKB based
 *  Version 2.1: IPv6 Support
 *  Version 2.2: Fkb based Multicast Support (IPv4)
 *
 *******************************************************************************
 */
#define PKTFLOW_VERSION             "v4.0"

#define PKTFLOW_VER_STR             PKTFLOW_VERSION
#define PKTFLOW_MODNAME             "Broadcom Packet Flow Cache "
#define PKTFLOW_NAME                "fcache"
#define FCACHE_PROCFS_DIR_PATH      PKTFLOW_NAME    /* dir: /procfs/fcache    */
#define FCACHE_STATS_PROCFS_DIR_PATH "fcache/stats" /*dir: /proc/fcache/stats */
#define FCACHE_MISC_PROCFS_DIR_PATH "fcache/misc" /*dir: /proc/fcache/misc */

/* Flow Cache Character Device */
#define FCACHE_DRV_MAJOR             3002
#define FCACHE_DRV_NAME              PKTFLOW_NAME
#define FCACHE_DRV_DEVICE_NAME       "/dev/" FCACHE_DRV_NAME

/*
 * Conditional compilation of cache aligned declaration of flow members
 */
#define CC_FCACHE_ALIGNED_DECLARE
#if defined(CC_FCACHE_ALIGNED_DECLARE)
// #include <linux/cache.h>
#define _FCALIGN_     ____cacheline_aligned
#else
#define _FCALIGN_
#endif

/*
 * Conditional Compilation for Debug Support: global and per layer override
 * - Commenting out CC_CONFIG_FCACHE_DEBUG will disable debug for all layers.
 * - Selectively disable per subsystem by commenting out its define.
 * - Debug levels listed in pktDbg.h
 */
#ifdef PKTDBG
#define CC_CONFIG_FCACHE_DEBUG

/* LAB ONLY: Design development */
#define CC_CONFIG_FCACHE_COLOR          /* Color highlighted debugging     */
#define CC_CONFIG_FCACHE_DBGLVL     0   /* DBG_BASIC Level                 */
#define CC_CONFIG_FCACHE_DRV_DBGLVL 0   /* DBG_BASIC Level Basic           */
#define CC_CONFIG_FCACHE_STATS          /* Statistics design engineering   */
#endif

/* Functional interface return status */
#define FCACHE_ERROR                (-1)    /* Functional interface error     */
#define FCACHE_SUCCESS              0       /* Functional interface success   */

#define FCACHE_CHECK                1       /* Boolean enforcing key audits   */

/* fc_error: unconditionally compiled */
#define fc_error(fmt, arg...)      \
        bcm_printk( CLRerr DBGsys "%-10s ERROR: " fmt CLRnl, __FUNCTION__, ##arg )

#undef FCACHE_DECL
#define FCACHE_DECL(x)      x,  /* for enum declaration in H file */

typedef enum
{
    FCACHE_DECL(FCACHE_DBG_DRV_LAYER)
    FCACHE_DECL(FCACHE_DBG_FC_LAYER)
    FCACHE_DECL(FCACHE_DBG_FHW_LAYER)
    FCACHE_DECL(FCACHE_DBG_PATHSTAT_LAYER)    
    FCACHE_DECL(FCACHE_DBG_LAYER_MAX)
} FcacheDbgLayer_t;


/*
 *------------------------------------------------------------------------------
 *              Flow Cache character device driver IOCTL enums
 * A character device and the associated userspace utility for design debug.
 *------------------------------------------------------------------------------
 */
typedef enum FcacheIoctl
{
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
   processor. Hence start all IOCTL values from 100 to prevent conflicts */
    FCACHE_IOCTL_DUMMY=99,
    FCACHE_DECL(FCACHE_IOCTL_STATUS)
    FCACHE_DECL(FCACHE_IOCTL_ENABLE)
    FCACHE_DECL(FCACHE_IOCTL_UNUSED)
    FCACHE_DECL(FCACHE_IOCTL_DISABLE)
    FCACHE_DECL(FCACHE_IOCTL_FLUSH)
    FCACHE_DECL(FCACHE_IOCTL_DEFER)
    FCACHE_DECL(FCACHE_IOCTL_MCAST)
    FCACHE_DECL(FCACHE_IOCTL_IPV6)
    FCACHE_DECL(FCACHE_IOCTL_RESET_STATS)
    FCACHE_DECL(FCACHE_IOCTL_MONITOR)
    FCACHE_DECL(FCACHE_IOCTL_TIMER)
    FCACHE_DECL(FCACHE_IOCTL_CREATE_FLWSTATS)
    FCACHE_DECL(FCACHE_IOCTL_GET_FLWSTATS)
    FCACHE_DECL(FCACHE_IOCTL_DELETE_FLWSTATS)
    FCACHE_DECL(FCACHE_IOCTL_CLEAR_FLWSTATS)
    FCACHE_DECL(FCACHE_IOCTL_GET_FLWSTATS_NUM)
    FCACHE_DECL(FCACHE_IOCTL_DUMP_FLWSTATS)
    FCACHE_DECL(FCACHE_IOCTL_GET_FLOWSTATS_POLL_PARAMS)
    FCACHE_DECL(FCACHE_IOCTL_SET_FLOWSTATS_POLL_PARAMS)
    FCACHE_DECL(FCACHE_IOCTL_GRE)
    FCACHE_DECL(FCACHE_IOCTL_L2TP)
    FCACHE_DECL(FCACHE_IOCTL_DEBUG)
    FCACHE_DECL(FCACHE_IOCTL_MCAST_LEARN)
    FCACHE_DECL(FCACHE_IOCTL_ACCEL_MODE)
    FCACHE_DECL(FCACHE_IOCTL_DUMP_FLOW_INFO)
    FCACHE_DECL(FCACHE_IOCTL_TCP_ACK_MFLOWS)
    FCACHE_DECL(FCACHE_IOCTL_SET_HW_ACCEL)
    FCACHE_DECL(FCACHE_IOCTL_SW_DEFER)
    FCACHE_DECL(FCACHE_IOCTL_4O6_FRAG)
    FCACHE_DECL(FCACHE_IOCTL_INVALID)
} FcacheIoctl_t;


#include <pktHdr.h>

/*
 *------------------------------------------------------------------------------
 *              Flow Cache character device driver IOCTL struct
 * A character device and the associated userspace utility for design debug.
 *------------------------------------------------------------------------------
 */
typedef struct {
	int interval;
	int defer;
	int sw_defer;
	int max_ent;
	uint32_t cumm_insert;
	uint32_t cumm_remove;

	struct {
        uint16_t monitor        : 1;
        uint16_t mcastIPv4      : 1;
        uint16_t mcastIPv6      : 1;
        uint16_t enableIPv6     : 1;
        uint16_t fc_status      : 1;
        uint16_t fc_gre         : 2;
        uint16_t mcast_learn    : 1;
        uint16_t accel_mode     : 1;
        uint16_t tcp_ack_mflows : 1;
        uint16_t hw_accel       : 1;
        uint16_t fc_4o6_frag    : 1;
        uint16_t unused         : 4;
      } flags;	
}FcStatusInfo_t;

/*
 *------------------------------------------------------------------------------
 *                 Flow Cache flush by parameters struct
 * A struct to allow flushing flows by matching parameters from kernel or
 * user space.
 *------------------------------------------------------------------------------
 */
#define FCACHE_FLUSH_ALL       (1 << 0)
#define FCACHE_FLUSH_FLOW      (1 << 1)
#define FCACHE_FLUSH_DEV       (1 << 2)
#define FCACHE_FLUSH_DSTMAC    (1 << 3)
#define FCACHE_FLUSH_SRCMAC    (1 << 4)
#define FCACHE_FLUSH_HW        (1 << 5)

#define FCACHE_FLUSH_MAC       (FCACHE_FLUSH_DSTMAC | FCACHE_FLUSH_SRCMAC)

typedef struct {
    uint32_t flags;
    uint8_t mac[6];
    int devid;
    int flowid;
}FcFlushParams_t;

#if defined(CONFIG_BCM_PKTFLOW_MODULE) || defined(CONFIG_BCM_PKTFLOW)

#if !defined(CONFIG_BLOG)
#error "Attempting to build Flow cache without BLOG"
#endif

#include <linux/blog.h>

/*
 *------------------------------------------------------------------------------
 * Conditional Compile configuration for Packet Flow Cache
 *------------------------------------------------------------------------------
 */

#if defined(CONFIG_BLOG_IPV6)
#define CC_FCACHE_IPV6_SUPPORT
#endif

//#define CC_CONFIG_FCACHE_BLOG_MANUAL    /* LAB ONLY: Manual blog enabling  */

#define CC_CONFIG_FCACHE_PROCFS         /* Proc filesystem debug dumps     */

#define CC_CONFIG_FCACHE_STACK          /* Patent Pending: sw acceleration */

#define CC_CONFIG_FCACHE_DEFER          /* Defer HW activation on swhit   */
// #define CC_CONFIG_FCACHE_JENKINS_HASH   /* Jenkins 3word hash algorithm    */

typedef enum {
    FCACHE_HWACC_PRIO_0,            /* Highest Priority */
    FCACHE_HWACC_PRIO_1,            /* Lowest Priority  */
    FCACHE_HWACC_PRIO_MAX
} FcacheHwAccPrio_t;

/*
 *------------------------------------------------------------------------------
 * Implementation Constants 
 *------------------------------------------------------------------------------
 */

/* Flow cache static engineering: runtime poll board memory availability ...  */
#define FCACHE_MAX_ENTRIES      (128*1024)  /* Maximum number of entries      */
#define FCACHE_DEF_MAX_ENTRIES   256        /* Def Max number of entries      */
#if (FCACHE_MAX_ENTRIES > (128*1024))       
#error "Invalid number of flow cache entries, DO NOT EXCEED 128K" 
#endif
#define FCACHE_HTABLE_SIZE         (FCACHE_MAX_ENTRIES>>2) /* Must not be greater than 128K */
#if (FCACHE_HTABLE_SIZE > FCACHE_MAX_ENTRIES)
#error "Invalid number of flow cache hash table entries, DO NOT EXCEED 128K" 
#endif

#define FCACHE_STACK_SIZE           8               /* goto stack size        */
#define FCACHE_JHASH_RAND           0xBABEEBAB      /* Sufficiently random    */

/* Flow cache system periodic timer */
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#define FCACHE_REFRESH              ( 10 )          /* Poll timer interval   */
#define FCACHE_REFRESH_MIN          ( 5000 )        /* Min Poll timer interval*/
#else
#define FCACHE_REFRESH              ( 1 )          /* Poll timer interval   */
#define FCACHE_REFRESH_MIN          ( 500 )        /* Min Poll timer interval*/
#endif
#define FCACHE_REFRESH_INTERVAL     ( FCACHE_REFRESH SECONDS )
#define FCACHE_REFRESH_MIN_INTERVAL ( FCACHE_REFRESH_MIN )


/* Refresh based on layer-4 protocol */
#define FCACHE_REFRESH_IDLE         120
#define FCACHE_REFRESH_IDLE_INTERVAL ( FCACHE_REFRESH_IDLE SECONDS )

/* Flow cache entry experiencing hw|sw hits has idle quota of FLOW_MAX_QUOTA  */
#define FLOW_MAX_QUOTA              0xFF

/* Reconfigure Hardware HW if software hits larger than threshold */
#define FCACHE_REACTIVATE           (50)      /* Lookup threshold to reactivate */
#define FCACHE_MAX_PENALTY          8

/* Flow cache hash table IP-Tuple lookup result */
#define FCACHE_MISS                 0       /* Lookup IPTuple hash table miss */
#define FCACHE_HIT                  1       /* Lookup IPTuple hash table hit  */

/* Special tuple to signify an invalid tuple. */
#define FLOW_HW_INVALID             0xFFFFFFFF

#define FLOW_NF_INVALID             0x0

#define FLOW_IN_INVALID             0x07    /* Incarnation 0x07 is invalid    */
#define FLOW_IX_INVALID             BLOG_KEY_FC_INVALID /* 0 reserved         */
#define FLOW_NULL                   ((Flow_t*)NULL)

#define FDB_IN_INVALID             0x07    /* Incarnation 0x07 is invalid    */
#define FDB_IX_INVALID             0       /* Element at index 0 reserved    */
#define FDB_NULL                   ((FdbEnt_t*)NULL)
#define FDB_KEY_INVALID            BLOG_FDB_KEY_INVALID

#define NPE_IN_INVALID              0x07    /* Incarnation 0x07 is invalid    */
#define NPE_IX_INVALID              0       /* Element at index 0 reserved    */
#define NPE_NULL                    ((npe_t*)NULL)
#define NPE_KEY_INVALID             BLOG_KEY_FC_INVALID

#define PATH_IX_INVALID            0 /* 0 reserved for exception cases */

/*
 *------------------------------------------------------------------------------
 * All the low prio packets are dropped when the CPU congestion is experienced
 * except when the ANDing of low prio packet counts under CPU congestion in
 * fcache and the mask given below is 0. e.g. if the mask is 0x7F, then 1 out
 * of every 128 low prio packets will be accepted under congestion.  
 * This will relieve CPU congestion because of low prio packets.   
 *------------------------------------------------------------------------------
 */
#define FCACHE_IQOS_LOWPRIO_PKTCNT_MASK 0x7F

typedef enum {
    FCACHE_DECL(HW_CAP_NONE)
    FCACHE_DECL(HW_CAP_IPV4_UCAST)
    FCACHE_DECL(HW_CAP_IPV4_MCAST)
    FCACHE_DECL(HW_CAP_IPV6_UCAST)
    FCACHE_DECL(HW_CAP_IPV6_MCAST)
    FCACHE_DECL(HW_CAP_IPV6_TUNNEL)
    FCACHE_DECL(HW_CAP_MCAST_DFLT_MIPS)
    FCACHE_DECL(HW_CAP_L2_UCAST)
    FCACHE_DECL(HW_CAP_PATH_STATS)
    FCACHE_DECL(HW_CAP_MAX)
} HwCap_t;


/* OS memory allocation flags */
enum {
    FCACHE_ALLOC_TYPE_ATOMIC=0, /* allocation, fails if memory is not present */
    FCACHE_ALLOC_TYPE_KERNEL,   /* if memory is not present,try to reclaim */ 
                                /* and allocate before returning a failure */
    FCACHE_ALLOC_TYPE_MAX
};


/*
 *------------------------------------------------------------------------------
 *  Invoked by Packet HW Protocol layer to clear HW association.
 *  Based on the scope of the request:
 *      System_e scope: Clear hw association for all active flows.
 *      Engine_e scope: Clear hw associations of flows on an engine.
 *      Match_e  scope: Clear a uniquely identified flow.
 *------------------------------------------------------------------------------
 */
typedef enum {
    System_e,       /* System wide active flows */
    Match_e         /* Unique active flow of a specified match id */
} FlowScope_t;

typedef int ( *FC_CLEAR_HOOK)(uint32_t key, FlowScope_t scope);

/*
 * hooks initialized by HW Protocol Layers.
 * Fcache makes upcalls into packet HW Protocol layer via theses hooks
 */
typedef struct FcBindFhwHooks {
    HOOK3PARM       activate_fn; 
    HOOK3PARM       deactivate_fn;
    HOOK3PARM       update_fn;
    HOOK3PARM       refresh_fn; 
    HOOK3PARM       refresh_pathstat_fn;
    HOOK2PARM       reset_stats_fn;
    HOOK32          clear_fn; 
    FC_CLEAR_HOOK  *fc_clear_fn;
    HOOK4PARM       stats_fn; 
    HOOKP           hwsupport_fn; 
    HOOKV           mcast_dflt_fn;
    HOOKV           get_path_num_fn;
} FcBindFhwHooks_t; 

/*
 * hooks initialized by pathStats driver.
 * Fcache makes upcalls into path stats collection driver via theses hooks
 */
typedef struct FcBindPathStatHooks {
    HOOKP           add_flow_fn; 
    HOOKP           evict_flow_fn;
    HOOKP           activate_fhw_fn; 
    HOOKP           deactivate_fhw_fn;
    HOOK3PARM       update_stat_fn;
    HOOK3PARM       query_dev_stat_fn;
    HOOKP           clear_dev_stat_fn;     
    HOOKP           exclude_dev_fn;
} FcBindPathStatHooks_t; 

/*
 * Structures defined for buffering Multicast buffers that will get
 * transmitted at a later time after blog_lock is released
 */
typedef struct {
    struct net_device * txdev;
    void              * xmit_fn; 
    pNBuff_t            nbuff_p;
} FcacheMcastXmitInfo_t;

/*
 *------------------------------------------------------------------------------
 * Flow cache binding to HW to register HW upcalls and downcalls
 * Upcalls from Flow cache to HW: activate, deactivate and refresh functions.
 * Downcalls from HW to Flow cache: clear hardware associations function.
 *------------------------------------------------------------------------------
 */
extern void fc_bind_fhw( FcBindFhwHooks_t *fhwHooks_p );

/*
 *------------------------------------------------------------------------------
 * Function     : fc_update_hw_support
 * Description  : Update hw_support for active flows.
 * Design Note  : Invoked by fhw_bind_hw() or enable/disable hw-accel in fcachedrv.c
 *------------------------------------------------------------------------------
 */
extern void fc_update_hw_support(int force_disable);

extern void fc_flwstats_bind(HOOK3PARM *getFlwStatsFn, 
                             HOOK4PARM flwEvictCallBack);

/*
 *------------------------------------------------------------------------------
 * Flow cache binding to pathStats driver to register upcalls and downcalls
 * Upcalls from Flow cache to pathStats: add, evict, update and query functions.
 *------------------------------------------------------------------------------
 */
extern void fc_bind_pathstat( FcBindPathStatHooks_t *pathstatHooks_p );

extern void fc_refresh_pathstat_fhw(uint8_t pathstat_idx, 
                    uint32_t *hw_hits_p, uint32_t *hw_bytes_p);

extern uint32_t fc_get_path_num_fhw(void);

/*
 *------------------------------------------------------------------------------
 * Defer activation of HW. On every fcache defer number of packets per
 * interval fcache will attempt to activate HW. The interval is specified by
 * FCACHE_REFRESH_INTERVAL. To avoid a performance impact of repeated activation
 * attempts when HW tables are depleted, a penalty is applied (factored into
 * fcache deferral. Bursty traffic will have the penalty reduced.
 *
 * An argument of -1, implies a get of corresponding value.
 *------------------------------------------------------------------------------
 */
extern int fcacheDebug(int debug_level);

extern int  fcacheStatus(void);
extern uint16_t fcacheDefer(uint16_t deferral);
extern uint16_t fcache_set_sw_defer_count(uint16_t sw_defer_count);
extern int  fcacheMonitor(int monitor);
extern int  fcacheChkHwSupport(Blog_t * blog_p);
extern void fcacheBindHwSupportHook(HOOKP hw_support_fn);
extern unsigned int  fcacheChkHwFeature(void);

/*
 *------------------------------------------------------------------------------
 * Manual enabling and disabling of Flow cache to Blog binding
 *  flag = 0 : disables binding to blog. No more logging.
 *  flag != 0: enables binding to blog via Flow cache receive/transmit.
 *------------------------------------------------------------------------------
 */
extern void fc_bind_blog(int flag);         /* disable[flag=0] enable[flag=1] */

/*
 *------------------------------------------------------------------------------
 * IP Flow learning status [defined by binding with blog]
 *------------------------------------------------------------------------------
 */
extern void fc_status(void);

/*
 *------------------------------------------------------------------------------
 * IP Flow learning status for IOCTL [defined by binding with blog]
 *------------------------------------------------------------------------------
 */
extern void fc_status_ioctl(FcStatusInfo_t *fcStatusInfo_p);

/*
 *------------------------------------------------------------------------------
 * Flush all learnt entries in flow cache
 *------------------------------------------------------------------------------
 */
extern int  fc_flush(void);


/*
 *------------------------------------------------------------------------------
 * Flush all learned entries in flow cache for device dev_p
 *------------------------------------------------------------------------------
 */
extern void fc_flush_dev(void * dev_p);

#define MDNAT_IN_INVALID            0x07    /* Incarnation 0x07 is invalid    */
#define MDNAT_IX_INVALID            0       /* Element at index 0 reserved    */
#define MDNAT_NULL                  ((MdnatEnt_t*)NULL)
#define MDNAT_KEY_INVALID           0

#define FCACHE_MCAST_DNAT_ENTRIES   128     /* max # of RTP seq groups        */
#define FCACHE_MDNAT_HTABLE_SIZE    (FCACHE_MCAST_DNAT_ENTRIES>>2)
#define FCACHE_RTP_SEQ_GROUP_SIZE   4       /* # of flows in RTP seq group    */
#define FCACHE_RTP_HDR_TS_OFFSET    8       /* RTP time stamp offset + 4      */
#define FCACHE_RTP_SEQ_MOD          (1<<16)
#define FCACHE_RTP_SEQ_MAX_DROPOUT  3000    /* max allowed delta between prev 
                                            and current seq for the cycle.    */ 
#define FCACHE_RTP_SEQ_MAX_MISORDER 100     /* max misorder                   */
#define FCACHE_RTP_SEQ_MIN_SEQ      2       /* Min # of sequential packets    */
#define FCACHE_RTP_SEQ_ERR_MAX_HIST 4       /* Max # of seq errs numbers       */

/*
 *------------------------------------------------------------------------------
 * Mdnat Entry Key:
 * A 32bit key that contains:
 *  -  3bit incarnation id (avoid latent access)
 *  - 29bit entry id (index of entry in FDB cache table)
 *------------------------------------------------------------------------------
 */
typedef struct {
    union {
        struct {
            BE_DECL(
                uint32_t incarn  :  3; /* Allocation instance identification */
                uint32_t self    : 29; /* Index into static allocation table */
            )
            LE_DECL(
                uint32_t self    : 29; /* Index into static allocation table */
                uint32_t incarn  :  3; /* Allocation instance identification */
            )
        } id;
        uint32_t word;
    };
} MdnatKey_t;

typedef struct {
        uint16_t unused[1];
        uint16_t cycles;
        uint32_t seq_num;
} RtpSeqErr_t;

typedef struct {
    struct {
        uint32_t pkts_rx;   /* current seq packet receive count */
        uint32_t tot_pkts_rx; /* total packets rx including current sequence */
        int32_t  pkts_lost; /* total packets lost (exluding current sequence) */
        int32_t  prev_seq_pkts_lost; /* RTP pkts lost in the prev sequences of 
                 the flow. Also, it does not include pkts lost in current seq */

        uint16_t cycles;    /* shifted count of  seq number cycles */
        uint16_t base_seq;  /* base seq number */
        uint16_t cur_seq;   /* current seq number seen */
        uint16_t max_seq;   /* highest seq number seen */

        uint32_t prob            : 4; /* sequential packets till src is valid */
        uint32_t seq_err_hist_idx: 4;
        uint32_t sw_cnt          : 8;
        uint32_t seq_err_cnt     :16; /* count of packets with seq errors */
        uint32_t bad_seq;   /* last bad seq number + 1 */

        uint32_t ssrc;      /* Synchronization source identifier */
        uint32_t unsed;
        /* history of last seq errors including bad_seq */
        RtpSeqErr_t seq_err_hist[FCACHE_RTP_SEQ_ERR_MAX_HIST];
    };
} RtpSeqEnt_t;

typedef struct {
    union {
        struct {
            uint8_t  alloc_idx_map; /* bitmap of allocated stream idx */
            uint8_t  act_idx;       /* last RTP seq packet Rx for this idx */
            uint16_t max_seq;       /* max RTP seq of the last Rx packet */
        } common;
        uint32_t word;
    };
    RtpSeqEnt_t seq[FCACHE_RTP_SEQ_GROUP_SIZE];
} RtpSeqGroupEnt_t;

typedef struct mdnatEnt_t {
    struct dll_t    node;       /* First element implements dll               */
    struct mdnatEnt_t *chain_p; /* Single linked list hash table chaining     */
    MdnatKey_t      key;        /* Mcast DNAT entry id                        */
    uint8_t         unused;     
    uint8_t         count;     
    uint16_t        hashix;     /* hash */
    uint32_t        tx_dest_ip;
    RtpSeqGroupEnt_t group;
} MdnatEnt_t;


#define FcKey_t     BlogKeyFc_t

/* Forward declarations */
struct npe;
typedef struct npe npe_t;

#define FCACHE_NPE_PLD             0U
#define FCACHE_NPE_DEL             1U
#define FCACHE_NPE_MAP             2U
#define FCACHE_NPE_MAX             3U

/*
 *------------------------------------------------------------------------------
 * Flow Cache Entry Key:
 * A 64-bit key that contains:
 *  - 32bit flow cache flow id
 *  - 32bit hardware connection id (encoding of HW engine and matchIx)
 *------------------------------------------------------------------------------
 */

typedef struct {
    union {
        struct {
            BE_DECL(
                FcKey_t  fc;
                uint32_t hw;
            )
            LE_DECL(
                uint32_t hw;
                FcKey_t  fc;
            )
        } id;
        struct {
            BE_DECL(
                uint32_t word;
                uint32_t hw;
            )
            LE_DECL(
                uint32_t hw;
                uint32_t word;
            )
        };
    };
} FlowKey_t;

/*
 *------------------------------------------------------------------------------
 * Flow Cache Table Entry:
 *------------------------------------------------------------------------------
 */
struct flow_t {
    struct dll_t    node;       /* First element implements dll               */
    FlowKey_t       key;        /* Second element implements incarnation      */
    struct flow_t   * chain_p;  /* Single linked list hash table chaining     */

    Blog_t          * blog_p;   /* Buffer log carrying flow context data      */
    uint8_t         idle;       /* Idle quota in seconds before cache flush   */
    uint8_t         hw_pathstat_idx;    /* HWACC pathstat index, uint8_t */
    uint8_t         list_depth; /* Depth of jump_list (for multicast only)    */
    uint8_t         unused;

    uint32_t        swhits;     /* Software lookup hits in last interval      */
    int16_t         mtuAdj;     /* max Rx packet length that is accelerated   */
    uint8_t         hwAccIx;    /* HW Accelerator Index                       */
    uint8_t         hwPolledCnt;

    uint16_t        udp_dport_excl; /* UDP dest port to be excluded           */
    uint16_t        mcast_port_map;   /* mcast flow Eth TX port map           */
    MdnatEnt_t      *mdnat_ent_p;
    uint8_t         rtp_idx;
    uint8_t         tx_vtag_num;
    uint16_t        sw_pathstat_idx;  /* Fcache/SW pathstat index, uint16_t */
    uint32_t        tx_vtag[MAX_NUM_VLAN_TAG];

    union {
       uint32_t      flags;
       struct {
        BE_DECL(
          uint32_t   unused3      :23;
          uint32_t   skip_hw_ageing:1;  /* skip ageing of the HW entry        */

          uint32_t   rtp_seq_chk  :1;   /* mcast RTP Sequence check enabled   */
          uint32_t   new_flow     :1;   /* Flow is considered "new" if it is 
                                          not yet pushed to hw acc. This flag 
                                          is reset to 0 after pushing to hw 
                                          acc and will continue to be 0 even if
                                          evicted from hw acc and accelerated 
                                          by fcache. This flag is used to 
                                          maintain fcache stats */
          uint32_t   l2l_mcast    :1;
          uint32_t   mcast_dflt   :1;   /* mcast default rule */
          uint32_t   iq_prio      :1;   /* Ingress Qos Priority */
          uint32_t   hw_support   :1;   /* e.g. hw acceleration */
          uint32_t   is_ssm       :1;   /* SSM/ASM mcast acceleration         */
          uint32_t   incomplete   :1;   /* Indication of static configuration */
        )
        LE_DECL(
          uint32_t   incomplete   :1;   /* Indication of static configuration */
          uint32_t   is_ssm       :1;   /* SSM/ASM mcast acceleration         */
          uint32_t   hw_support   :1;   /* e.g. hw acceleration */
          uint32_t   iq_prio      :1;   /* Ingress Qos Priority */
          uint32_t   mcast_dflt   :1;   /* mcast default rule */
          uint32_t   l2l_mcast    :1;
          uint32_t   new_flow     :1;   /* Flow is considered "new" if it is 
                                          not yet pushed to hw acc. This flag 
                                          is reset to 0 after pushing to hw 
                                          acc and will continue to be 0 even if
                                          evicted from hw acc and accelerated 
                                          by fcache. This flag is used to 
                                          maintain fcache stats */
          uint32_t   rtp_seq_chk  :1;   /* mcast RTP Sequence check enabled   */

          uint32_t   skip_hw_ageing:1;  /* skip ageing of the HW entry        */
          uint32_t   unused3      :23;
        )
       };
    };

    uint16_t        sw_defer_hits;  /* # of packets sw deferred */
    uint16_t        unused2;
    uint32_t        cumm_hits;  /* Cummulative sw hit count since creation    */
    unsigned long long  cumm_bytes; /* Cummulative byte count since creation      */
    uint32_t        expires;

#if defined(CC_CONFIG_FCACHE_STACK)
                                /* Command sequence for packet mangling   */
    void            * jump_list[FCACHE_STACK_SIZE] _FCALIGN_;
#endif

    struct dll_t    src_fdb_node;   /* linked into FDB src list */
    struct dll_t    dst_fdb_node;   /* linked into FDB dst list */

    npe_t           *npe_p[FCACHE_NPE_MAX];   /* linked into npe list */
} _FCALIGN_;                    /* 5 cache lines wide */

typedef struct flow_t Flow_t;

/*
 *------------------------------------------------------------------------------
 * Flow Cache Slice Timer Entry:
 *------------------------------------------------------------------------------
 */
struct sliceEnt_t {
    struct dll_t    node;       /* First element implements dll               */
    uint32_t        id;         /* slice timer entry id                       */
    Flow_t        * flow_p;     /* points to owned by flow                    */
} _FCALIGN_;                    
typedef struct sliceEnt_t SliceEnt_t;

/*
 *------------------------------------------------------------------------------
 * FDB Entry Key:
 * A 32bit key that contains:
 *  -  3bit incarnation id (avoid latent access)
 *  -  29bit entry id (index of entry in FDB cache table)
 *------------------------------------------------------------------------------
 */
typedef struct {
    union {
        struct {
            BE_DECL(
                uint32_t incarn  :  3; /* Allocation instance identification */
                uint32_t self    : 29; /* Index into static allocation table */
            )
            LE_DECL(
                uint32_t self    : 29; /* Index into static allocation table */
                uint32_t incarn  :  3; /* Allocation instance identification */
            )
        } id;
        uint32_t word;
    };
} FdbKey_t;

typedef union {
    uint8_t         u8[6];  /* MAC */
    uint16_t        u16[3]; /* MAC */
} FcMac_t;

/*
 *------------------------------------------------------------------------------
 * Flow Cache FDB Entry:
 *------------------------------------------------------------------------------
 */
struct fdbEnt_t {
    struct dll_t    node;           /* FDB list */
    FdbKey_t        key;            /* linking Linux and fcache FDB entries   */
    struct fdbEnt_t *chain_p;       /* FDB Hash list node                     */

    struct dll_t    src_act_list;   /* active flows                           */
    struct dll_t    src_idle_list;  /* idle flows                             */
    struct dll_t    dst_act_list;   /* active flows                           */
    struct dll_t    dst_idle_list;  /* idle flows                             */

    uint16_t        hashix;         /* FDB hash index                         */
    FcMac_t         mac;
    unsigned long   upd_time;       /* last update time in jiffies            */
    uint32_t        unused;         
#if defined(CC_CONFIG_FCACHE_STATS)
    uint32_t        src_flow_count; /* # of flows linked to src FDB list      */
    uint32_t        dst_flow_count; /* # of flows linked to dst FDB list      */
#endif
} _FCALIGN_;                    
typedef struct fdbEnt_t FdbEnt_t;

/* Flow cache static engineering: runtime poll board memory availability ...  */
#define FCACHE_FDB_MAX_ENTRIES      (16*1024)/* Maximum number of FDB entries */
#define FCACHE_FDB_DEF_MAX_ENTRIES  256     /* Def Max number of entries      */
#if (FCACHE_FDB_MAX_ENTRIES > (16*1024))          
#error "Invalid number of flow cache FDB entries, DO NOT EXCEED 16K" 
#endif

#define npe_key_t     BlogKeyFc_t


/* Flow cache static engineering: runtime poll board memory availability ...  */
#define FCACHE_NPE_MAX_ENTRIES      (256*1024)/* Maximum number of npe entries*/
#define FCACHE_NPE_DEF_MAX_ENTRIES  512     /* Def Max number of entries      */
#if (FCACHE_NPE_MAX_ENTRIES > (256*1024))          
#error "Invalid number of flow cache npe entries, DO NOT EXCEED 256K" 
#endif

/*
 *------------------------------------------------------------------------------
 * Flow Cache npe flow list Entry:
 *------------------------------------------------------------------------------
 */
typedef struct {
    struct dll_t    node;       /* First element implements dll               */
    uint32_t        id;         /* entry id                                   */
    struct flow_t   *flow_p;    /* points to owned by flow                    */
} npe_flow_t;


/*
 *------------------------------------------------------------------------------
 * Flow Cache network proxiy entity (npe) Entry:
 *------------------------------------------------------------------------------
 */
struct npe {
    struct dll_t    node;           /* npe Entity list                        */
    BlogKeyFc_t     key;            /* linking Linux nwe and npe entity       */
    struct npe      *chain_p;       /* npe Entity Hash list node              */
    uint32_t        hashix;         /* npe Entity hash index                  */

    int             type;           /* npe Entity type                        */
    void            *nwe_p;
    Dll_t           flow_list[BLOG_PARAM1_DIR_MAX];   /* flow lists       */
    uint32_t        flow_count[BLOG_PARAM1_DIR_MAX];  /* # of flows       */
} _FCALIGN_;                    
typedef struct npe npe_t;


#else
#define     fc_bind_blog(enable)        NULL_STMT
#endif  /* defined(CONFIG_BCM_PKTFLOW_MODULE) || defined(CONFIG_BCM_PKTFLOW) */

typedef enum
{
    FCACHE_DRV_PROC_TYPE_BR,
    FCACHE_DRV_PROC_TYPE_NF,
    FCACHE_DRV_PROC_TYPE_MCAST,
    FCACHE_DRV_PROC_TYPE_MDNAT,
    FCACHE_DRV_PROC_TYPE_RTP_SEQ,
    FCACHE_DRV_PROC_TYPE_L2,
}enumFcacheDrvProcType;

#endif  /* defined(__FCACHE_H_INCLUDED__) */
