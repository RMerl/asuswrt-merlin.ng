/*
<:copyright-BRCM:2007:proprietary:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/
#ifndef _BCM_VLAN_LOCAL_H_
#define _BCM_VLAN_LOCAL_H_

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/in.h>
#include <linux/init.h>
#include <asm/uaccess.h> /* for copy_from_user */
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <net/datalink.h>
#include <net/p8022.h>
#include <net/arp.h>
#include <linux/version.h>

#include "bcmtypes.h"
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include "bcm_vlan.h"

/*
 * Constants
 */

#if defined(CONFIG_BLOG) && defined(CONFIG_BLOG_MCAST)
#define CC_BCM_VLAN_FLOW
#endif

//#define BCM_VLAN_DATAPATH_DEBUG
//#define BCM_VLAN_IP_CHECK
#define BCM_VLAN_DATAPATH_ERROR_CHECK
//#define BCM_VLAN_PUSH_PADDING

#define BCM_VLAN_ENABLE_SKB_VLAN

#define BCM_VLAN_MAX_RULE_TABLES (BCM_VLAN_MAX_TAGS+1)

#define BCM_ETH_HEADER_LEN        14
#define BCM_VLAN_HEADER_LEN       4
#define BCM_ETH_VLAN_HEADER_LEN   (BCM_ETH_HEADER_LEN + BCM_VLAN_HEADER_LEN)
#define BCM_ETH_ADDR_LEN          6

#define BCM_VLAN_MAX_DSCP_VALUES  64

#define BCM_VLAN_FRAME_SIZE_MIN   64


/*
 * Macros
 */

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
extern spinlock_t bcmVlan_dp_lock_g; /* DP BCM VLAN lock */
extern spinlock_t bcmVlan_rx_lock_g; /* Rx BCM VLAN lock */
extern spinlock_t bcmVlan_tx_lock_g; /* Tx BCM VLAN lock */
void BcmVlanLock(spinlock_t*);
void BcmVlanUnLock(spinlock_t*);
#define BCM_VLAN_DP_LOCK() BcmVlanLock( &bcmVlan_dp_lock_g )
#define BCM_VLAN_DP_UNLOCK() BcmVlanUnLock( &bcmVlan_dp_lock_g )
#define BCM_VLAN_RX_LOCK() BcmVlanLock( &bcmVlan_rx_lock_g )
#define BCM_VLAN_RX_UNLOCK() BcmVlanUnLock( &bcmVlan_rx_lock_g )
#define BCM_VLAN_TX_LOCK() BcmVlanLock( &bcmVlan_tx_lock_g )
#define BCM_VLAN_TX_UNLOCK() BcmVlanUnLock( &bcmVlan_tx_lock_g )
#define BCM_VLAN_GLOBAL_LOCK()                  \
    do {                                        \
        BCM_VLAN_RX_LOCK();                     \
        BCM_VLAN_TX_LOCK();                     \
    } while(0)
#define BCM_VLAN_GLOBAL_UNLOCK()                  \
    do {                                          \
        BCM_VLAN_TX_UNLOCK();                     \
        BCM_VLAN_RX_UNLOCK();                     \
    } while(0)
#else
#define BCM_VLAN_RX_LOCK()
#define BCM_VLAN_RX_UNLOCK()
#define BCM_VLAN_TX_LOCK()
#define BCM_VLAN_TX_UNLOCK()
#define BCM_VLAN_GLOBAL_LOCK()
#define BCM_VLAN_GLOBAL_UNLOCK()
#endif

#ifdef BCM_VLAN_DATAPATH_DEBUG
#define BCM_VLAN_DP_DEBUG(_fmt, _arg...) \
    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, _fmt, ##_arg)
#define BCM_VLAN_DP_ERROR(_fmt, _arg...)                                \
    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "    **** DATAPATH ERROR **** " _fmt, ##_arg)
#else
#define BCM_VLAN_DP_DEBUG(_fmt, _arg...)
#define BCM_VLAN_DP_ERROR(_fmt, _arg...)
#endif

#define BCM_VLAN_GET_TCI_PBITS(_vlanHeader) \
    ( ((ntohs((_vlanHeader)->tci)) & BCM_VLAN_PBITS_MASK) >> BCM_VLAN_PBITS_SHIFT )

#define BCM_VLAN_GET_TCI_CFI(_vlanHeader) \
    ( ((ntohs((_vlanHeader)->tci)) & BCM_VLAN_CFI_MASK) >> BCM_VLAN_CFI_SHIFT )

#define BCM_VLAN_GET_TCI_VID(_vlanHeader) \
    ( (ntohs((_vlanHeader)->tci)) & BCM_VLAN_VID_MASK )

#define BCM_VLAN_SET_SUB_FIELD(_field, _val, _mask, _shift)      \
    do {                                                        \
        (_field) &= ~(_mask);                                   \
        (_field) |= ((typeof((_field)))(_val)) << (_shift);     \
    } while(0)

#define BCM_VLAN_SET_TCI_PBITS(_tci, _pbits)                     \
    BCM_VLAN_SET_SUB_FIELD((_tci), (_pbits), BCM_VLAN_PBITS_MASK, BCM_VLAN_PBITS_SHIFT)

#define BCM_VLAN_SET_TCI_CFI(_tci, _cfi)                                \
    BCM_VLAN_SET_SUB_FIELD((_tci), (_cfi), BCM_VLAN_CFI_MASK, BCM_VLAN_CFI_SHIFT)

#define BCM_VLAN_SET_TCI_VID(_tci, _vid)                         \
    BCM_VLAN_SET_SUB_FIELD((_tci), (_vid), BCM_VLAN_VID_MASK, BCM_VLAN_VID_SHIFT)

#define BCM_VLAN_SET_SUB_FIELD_IN_NWK_ORDER(_field, _val, _mask, _shift)      \
    do {                                                        \
        (_field) = htons(ntohs(_field) & (~(_mask)));           \
        (_field) = (_field) | htons(((typeof((_field)))(_val)) << (_shift));   \
    } while(0)

#define BCM_VLAN_SET_TCI_PBITS_IN_NWK_ORDER(_tci, _pbits)                     \
    BCM_VLAN_SET_SUB_FIELD_IN_NWK_ORDER((_tci), (_pbits), BCM_VLAN_PBITS_MASK, BCM_VLAN_PBITS_SHIFT)

#define BCM_VLAN_SET_TCI_CFI_IN_NWK_ORDER(_tci, _cfi)                                \
    BCM_VLAN_SET_SUB_FIELD_IN_NWK_ORDER((_tci), (_cfi), BCM_VLAN_CFI_MASK, BCM_VLAN_CFI_SHIFT)

#define BCM_VLAN_SET_TCI_VID_IN_NWK_ORDER(_tci, _vid)                         \
    BCM_VLAN_SET_SUB_FIELD_IN_NWK_ORDER((_tci), (_vid), BCM_VLAN_VID_MASK, BCM_VLAN_VID_SHIFT)

#define BCM_VLAN_COPY_TCI_SUBFIELD(_toVlanHeader, _fromVlanHeader, _mask) \
    do {                                                                \
        (_toVlanHeader)->tci = htons( (ntohs((_toVlanHeader)->tci)) & (~(_mask))); \
        (_toVlanHeader)->tci = (_toVlanHeader)->tci | htons(((unsigned long)(ntohs((_fromVlanHeader)->tci))) & (_mask)); \
    } while(0)

#define BCM_VLAN_COPY_TCI_PBITS(_toVlanHeader, _fromVlanHeader)         \
    BCM_VLAN_COPY_TCI_SUBFIELD(_toVlanHeader, _fromVlanHeader, BCM_VLAN_PBITS_MASK)

#define BCM_VLAN_COPY_TCI_CFI(_toVlanHeader, _fromVlanHeader)         \
    BCM_VLAN_COPY_TCI_SUBFIELD(_toVlanHeader, _fromVlanHeader, BCM_VLAN_CFI_MASK)

#define BCM_VLAN_COPY_TCI_VID(_toVlanHeader, _fromVlanHeader)         \
    BCM_VLAN_COPY_TCI_SUBFIELD(_toVlanHeader, _fromVlanHeader, BCM_VLAN_VID_MASK)

#define BCM_VLAN_CREATE_TCI(_pbits, _cfi, _vid) \
  ( (((UINT16)(_pbits))<<BCM_VLAN_PBITS_SHIFT) | (((UINT16)(_cfi))<<BCM_VLAN_CFI_SHIFT) | ((UINT16)(_vid)) )

#define BCM_VLAN_GET_IP_PROTO(_ipHeader) ( (_ipHeader)->protocol)
#define BCM_VLAN_GET_IPV6_PROTO(_ipV6Header) ( (_ipV6Header)->nh)
#define BCM_VLAN_GET_IP_VERSION(_ipHeader) ( ((_ipHeader)->version) )

#define BCM_VLAN_SKB_ETH_HEADER(_skb) ( (bcmVlan_ethHeader_t *)(skb_mac_header(_skb) ) )

#define BCM_VLAN_DEV_INFO(_vlanDev) ((bcmVlan_devInfo_t *)netdev_priv(_vlanDev))

#define BCM_VLAN_REAL_DEV(_vlanDev) BCM_VLAN_DEV_INFO(_vlanDev)->realDev

#define BCM_VLAN_GET_NEXT_VLAN_HEADER(_tpidTable, _vlanHeader)                      \
    ( BCM_VLAN_TPID_MATCH((_tpidTable), ntohs((_vlanHeader)->etherType)) ? ((_vlanHeader)+1) : NULL )

#define BCM_VLAN_TPID_MATCH(_tpidTable, _tpid) bcmVlan_tpidMatch((_tpidTable), (_tpid))

/* Linked List API */

#define BCM_VLAN_DECLARE_LL(_name) bcmVlan_linkedList_t _name

#define BCM_VLAN_DECLARE_LL_ENTRY() bcmVlan_linkedListEntry_t llEntry

#define BCM_VLAN_LL_INIT(_linkedList)                           \
    do {                                                        \
        (_linkedList)->head = NULL;                             \
        (_linkedList)->tail = NULL;                             \
    } while(0)

/* #define BCM_VLAN_LL_IS_EMPTY(_linkedList)                               \ */
/*     ( ((_linkedList)->head == NULL) && ((_linkedList)->tail == NULL) ) */

#define BCM_VLAN_LL_IS_EMPTY(_linkedList) ( (_linkedList)->head == NULL )

#define BCM_VLAN_LL_INSERT(_linkedList, _newObj, _pos, _currObj)        \
    do {                                                                \
        if(BCM_VLAN_LL_IS_EMPTY(_linkedList))                           \
        {                                                               \
            (_linkedList)->head = (void *)(_newObj);                    \
            (_linkedList)->tail = (void *)(_newObj);                    \
            (_newObj)->llEntry.prev = NULL;                             \
            (_newObj)->llEntry.next = NULL;                             \
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Inserted FIRST object in %s", #_linkedList); \
        }                                                               \
        else                                                            \
        {                                                               \
            if((_pos) == BCM_VLAN_POSITION_APPEND)                      \
            {                                                           \
                typeof(*(_newObj)) *_tailObj = (_linkedList)->tail;     \
                _tailObj->llEntry.next = (_newObj);                     \
                (_newObj)->llEntry.prev = _tailObj;                     \
                (_newObj)->llEntry.next = NULL;                         \
                (_linkedList)->tail = (_newObj);                        \
                BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "APPENDED object in %s", #_linkedList); \
            }                                                           \
            else                                                        \
            {                                                           \
                if((_pos) == BCM_VLAN_POSITION_BEFORE)                  \
                {                                                       \
                    typeof(*(_newObj)) *_prevObj = (_currObj)->llEntry.prev; \
                    (_currObj)->llEntry.prev = (_newObj);               \
                    (_newObj)->llEntry.prev = _prevObj;                 \
                    (_newObj)->llEntry.next = (_currObj);               \
                    if(_prevObj != NULL)                                \
                    {                                                   \
                        _prevObj->llEntry.next = (_newObj);             \
                        BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Inserted INNER object (BEFORE) in %s", #_linkedList); \
                    }                                                   \
                    if((_linkedList)->head == (_currObj))               \
                    {                                                   \
                        (_linkedList)->head = (_newObj);                \
                        BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Inserted HEAD object in %s", #_linkedList); \
                    }                                                   \
                }                                                       \
                else                                                    \
                {                                                       \
                    typeof(*(_newObj)) *_nextObj = (_currObj)->llEntry.next; \
                    (_currObj)->llEntry.next = (_newObj);               \
                    (_newObj)->llEntry.prev = (_currObj);               \
                    (_newObj)->llEntry.next = _nextObj;                 \
                    if(_nextObj != NULL)                                \
                    {                                                   \
                        _nextObj->llEntry.prev = (_newObj);             \
                        BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Inserted INNER object (AFTER) in %s", #_linkedList); \
                    }                                                   \
                    if((_linkedList)->tail == (_currObj))               \
                    {                                                   \
                        (_linkedList)->tail = (_newObj);                \
                        BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Inserted TAIL object in %s", #_linkedList); \
                    }                                                   \
                }                                                       \
            }                                                           \
        }                                                               \
    } while(0)

#define BCM_VLAN_LL_APPEND(_linkedList, _obj)                           \
    BCM_VLAN_LL_INSERT(_linkedList, _obj, BCM_VLAN_POSITION_APPEND, _obj)

#define BCM_VLAN_LL_REMOVE(_linkedList, _obj)                           \
    do {                                                                \
        BCM_ASSERT(!BCM_VLAN_LL_IS_EMPTY(_linkedList));                 \
        if((_linkedList)->head == (_obj) && (_linkedList)->tail == (_obj)) \
        {                                                               \
            (_linkedList)->head = NULL;                                 \
            (_linkedList)->tail = NULL;                                 \
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Removed LAST object from %s", #_linkedList); \
        }                                                               \
        else                                                            \
        {                                                               \
            if((_linkedList)->head == (_obj))                           \
            {                                                           \
                typeof(*(_obj)) *_nextObj = (_obj)->llEntry.next;       \
                (_linkedList)->head = _nextObj;                         \
                _nextObj->llEntry.prev = NULL;                          \
                BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Removed HEAD object from %s", #_linkedList); \
            }                                                           \
            else if((_linkedList)->tail == (_obj))                      \
            {                                                           \
                typeof(*(_obj)) *_prevObj = (_obj)->llEntry.prev;       \
                (_linkedList)->tail = _prevObj;                         \
                _prevObj->llEntry.next = NULL;                          \
                BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Removed TAIL object from %s", #_linkedList); \
            }                                                           \
            else                                                        \
            {                                                           \
                typeof(*(_obj)) *_prevObj = (_obj)->llEntry.prev;       \
                typeof(*(_obj)) *_nextObj = (_obj)->llEntry.next;       \
                _prevObj->llEntry.next = (_obj)->llEntry.next;          \
                _nextObj->llEntry.prev = (_obj)->llEntry.prev;          \
                BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Removed INNER object from %s", #_linkedList); \
            }                                                           \
        }                                                               \
    } while(0)

#define BCM_VLAN_LL_GET_HEAD(_linkedList) (_linkedList).head

#define BCM_VLAN_LL_GET_NEXT(_obj) (_obj)->llEntry.next


/*
 * Type definitions
 */

typedef struct {
    void* head;
    void* tail;
} bcmVlan_linkedList_t;

typedef struct {
    void* prev;
    void* next;
} bcmVlan_linkedListEntry_t;

typedef struct {
    struct net_device *rxRealDev;
    struct net_device *txVlanDev;
    struct net_device *rxVlanDev;
    bcmVlan_tagRule_t tagRule;
    UINT32 cmdCount;
    unsigned int hitCount;
    unsigned int mergeCount;
    BCM_VLAN_DECLARE_LL_ENTRY();
} bcmVlan_tableEntry_t;

typedef struct {
    bcmVlan_tagRuleIndex_t tagRuleIdCount;
    UINT16 defaultTpid;
    UINT8 defaultPbits;
    UINT8 defaultCfi;
    UINT16 defaultVid;
    bcmVlan_defaultAction_t defaultAction;
    bcmVlan_tableEntry_t *lastTableEntry;
    struct net_device *defaultRxVlanDev;
    BCM_VLAN_DECLARE_LL(tableEntryLL);
} bcmVlan_ruleTable_t;

typedef struct {
    UINT32 rx_Misses;
    UINT32 tx_Misses;
    UINT32 error_PopUntagged;
    UINT32 error_PopNoMem;
    UINT32 error_PushTooManyTags;
    UINT32 error_PushNoMem;
    UINT32 error_SetEtherType;
    UINT32 error_SetTagEtherType;
    UINT32 error_InvalidTagNbr;
    UINT32 error_UnknownL3Header;
} bcmVlan_localStats_t;

/* This data structure will be accessed by system call and kernel thread paths.
 * It shall be protected by BCM_VLAN_LOCK.
 */
struct realDeviceControl {
    BCM_VLAN_DECLARE_LL_ENTRY();
    struct net_device *realDev;
    bcmVlan_ruleTable_t ruleTable[BCM_VLAN_TABLE_DIR_MAX][BCM_VLAN_MAX_RULE_TABLES];
    UINT8 dscpToPbits[BCM_VLAN_MAX_DSCP_VALUES];
    unsigned int tpidTable[BCM_VLAN_MAX_TPID_VALUES];
    bcmVlan_localStats_t localStats;
    bcmVlan_realDevMode_t mode;
    BCM_VLAN_DECLARE_LL(vlanDevCtrlLL);
};

typedef union {
    struct {
        uint32 multicast : 1;
        uint32 routed    : 1;
        uint32 swOnly    : 1;
        uint32 reserved  : 29;
    };
    uint32 u32;
} bcmVlan_vlanDevFlags_t;

/* This data structure will be accessed by system call and kernel thread paths.
 * It shall be protected by BCM_VLAN_LOCK.
 */
#define BCM_VLAN_IPTV_REF_MAX  127
struct vlanDeviceControl {
    BCM_VLAN_DECLARE_LL_ENTRY();
    struct net_device *vlanDev;
    struct realDeviceControl *realDevCtrl;
    bcmVlan_vlanDevFlags_t flags;
    char iptvOnly;                    /* reference counter, to mark if have IPTV-only
                                         rule on this vlan device */
};

typedef struct {
    struct dev_mc_list *old_mc_list;  /* old multi-cast list for the VLAN interface..
                                       * we save this so we can tell what changes were
                                       * made, in order to feed the right changes down
                                       * to the real hardware...
                                       */
    int old_allmulti;               /* similar to above. */
    int old_promiscuity;            /* similar to above. */
    struct vlanDeviceControl *vlanDevCtrl;
    struct net_device *realDev; /* this pointer is needed because Linux still calls
                                   the handlers of the VLAN interface after it has
                                   been unregistered. vlanDevCtrl is freed *before*
                                   unregistration takes place, so it cannot be used */
    unsigned char realDev_addr[ETH_ALEN];
    struct rtnl_link_stats64 stats64; /* Device stats (rx-bytes, tx-pkts, etc...) */
#if defined(CONFIG_BCM_VLAN_ISOLATION)
    char vids[4096]; /* reference count for rx vid filters (rdpa vlan objects) */
#endif
} bcmVlan_devInfo_t;

typedef struct {
    __be16 tci;
    __be16 etherType;
} __attribute__((packed)) bcmVlan_vlanHeader_t;

typedef struct {
    unsigned char macDest[ETH_ALEN];
    unsigned char macSrc[ETH_ALEN];
    __be16 etherType;
    bcmVlan_vlanHeader_t vlanHeader;
} __attribute__((packed)) bcmVlan_ethHeader_t;

typedef struct {
#if defined(__BIG_ENDIAN_BITFIELD)
    UINT8  version:4;
    UINT8  ihl:4;
#else
    UINT8  ihl:4;
    UINT8  version:4;
#endif
    UINT8  tos;
    UINT16 totalLength;
    UINT16 id;
    UINT16 flags_fragOffset;
    UINT8  ttl;
    UINT8  protocol;
    UINT16 headerChecksum;
    UINT32 ipSrc;
    UINT32 ipDest;
} __attribute__((packed)) bcmVlan_ipHeader_t;

/*  
 *	IPv6 fixed header
 *
 *	BEWARE, it is incorrect. The first 4 bits of flow_lbl
 *	are glued to priority now, forming "class".
 */
typedef struct {
#if defined(__BIG_ENDIAN_BITFIELD)
    UINT8  version:4;
    UINT8  tos:4;
#else
    UINT8  tos:4;
    UINT8  version:4;
#endif
    UINT8  flowLabel[3];
    UINT16 totalLength;
    UINT8  nh;
    UINT8  ttl;
    UINT32 ipSrc[4];
    UINT32 ipDest[4];
} __attribute__((packed)) bcmVlan_ipv6Header_t;

typedef struct {
    UINT8 version_type;
    UINT8 code;
    UINT16 sessionId;
    UINT16 length;
    UINT16 pppHeader;
    bcmVlan_ipHeader_t ipHeader;
} __attribute__((packed)) bcmVlan_pppoeSessionHeader_t;

static inline void dumpHexData(void *head, int len)
{
    int i;
    unsigned char *curPtr = (unsigned char*)head;

    if(bcmLog_getLogLevel(BCM_LOG_ID_VLAN) < BCM_LOG_LEVEL_DEBUG)
    {
        return;
    }

    printk("addr : %p, length : %d bytes", (void *)curPtr, len);

    for (i = 0; i < len; ++i) {
        if (i % 4 == 0)
            printk(" ");       
        if (i % 16 == 0)
            printk("\n0x%04X:  ", i);
        printk("%02X ", *curPtr++);
    }

    printk("\n");
}

static inline int bcmVlan_tpidMatch(unsigned int *tpidTable, unsigned int tpid)
{
    int i;

    for(i=0; i<BCM_VLAN_MAX_TPID_VALUES; ++i)
    {
        if(tpidTable[i] == tpid)
        {
            return 1;
        }
    }

    return 0;
}

static inline UINT8 bcmVlan_getIpDscp(bcmVlan_ipHeader_t *ipHeader)
{    
    if (BCM_VLAN_GET_IP_VERSION(ipHeader) == 4) 
    {
        return ((ipHeader->tos) & 0xFC) >> 2;
    } else if (BCM_VLAN_GET_IP_VERSION(ipHeader) == 6) 
    {
        return ((ntohs(*(const __u16 *)ipHeader) >> 4)& 0xFC) >> 2;
    }
    return 0;
}

#define BCM_VLAN_GET_IP_DSCP(_ipHeader) (bcmVlan_getIpDscp(_ipHeader) )

/*
 * Function Prototypes
 */

/* bcm_vlan_local.c */
int bcmVlan_initVlanDevices(void);
void bcmVlan_cleanupVlanDevices(void);
struct realDeviceControl *bcmVlan_getRealDevCtrl(struct net_device *realDev);
struct net_device *bcmVlan_getVlanDeviceByName(struct realDeviceControl *realDevCtrl, char *vlanDevName);
struct net_device *bcmVlan_getRealDeviceByName(char *realDevName, struct realDeviceControl **pRealDevCtrl);
void bcmVlan_freeRealDeviceVlans(struct net_device *realDev);
void bcmVlan_freeVlanDevice(struct net_device *vlanDev, int unregisterFlag);
int bcmVlan_createVlanDevice(struct net_device *realDev, struct net_device *vlanDev,
                             bcmVlan_vlanDevFlags_t flags);
void bcmVlan_transferOperstate(struct net_device *realDev);
void bcmVlan_updateInterfaceState(struct net_device *realDev, int up);
void __bcmVlan_dumpPacket(unsigned int *tpidTable, struct sk_buff *skb);
bool bcmVlan_isIptvOnlyVlanDevice(struct net_device *vlanDev);
int bcmVlan_setIptvOnlyVlanDevice(struct net_device *vlanDev);
int bcmVlan_unsetIptvOnlyVlanDevice(struct net_device *vlanDev, bool clearAll);
#ifdef BCM_VLAN_DATAPATH_DEBUG
#define bcmVlan_dumpPacket(tpidTable, skb)                              \
    do {                                                                \
        if(bcmLog_getLogLevel(BCM_LOG_ID_VLAN) == BCM_LOG_LEVEL_DEBUG)  \
            printk("%s,%d : bcmVlan_dumpPacket\n", __FUNCTION__, __LINE__); \
        __bcmVlan_dumpPacket(tpidTable, skb);                           \
    } while(0)
#else
#define bcmVlan_dumpPacket(tpidTable, skb)
#endif

/* bcm_vlan_user.c */
int bcmVlan_userInit(void);
void bcmVlan_userCleanup(void);
#if defined(CONFIG_BCM_VLAN_ISOLATION)
int vlanctl_notify_vlan_set_iso(struct net_device *vlan_dev, int enable);
#endif /* CONFIG_BCM_VLAN_ISOLATION */
int vlanctl_notify_route_mac(unsigned char * mac, int enable);

/* bcm_vlan_rule.c */
int bcmVlan_initTagRules(void);
void bcmVlan_cleanupTagRules(void);
void bcmVlan_initTpidTable(struct realDeviceControl *realDevCtrl);
int bcmVlan_setTpidTable(char *realDevName, unsigned int *tpidTable);
int bcmVlan_dumpTpidTable(char *realDevName);
void bcmVlan_initRuleTables(struct realDeviceControl *realDevCtrl);
int bcmVlan_setDefaultAction(char *realDevName, UINT8 nbrOfTags,
                             bcmVlan_ruleTableDirection_t tableDir,
                             bcmVlan_defaultAction_t defaultAction,
                             char *defaultRxVlanDevName);
void bcmVlan_cleanupRxDefaultActions(struct net_device *realDev,
                                     struct net_device *vlanDev);
int bcmVlan_setRealDevMode(char *realDevName, bcmVlan_realDevMode_t mode);
void bcmVlan_cleanupRuleTables(struct realDeviceControl *realDevCtrl);
int bcmVlan_insertTagRule(char *realDevName, UINT8 nbrOfTags, bcmVlan_ruleTableDirection_t tableDir,
                          bcmVlan_tagRule_t *tagRule, bcmVlan_ruleInsertPosition_t position,
                          bcmVlan_tagRuleIndex_t posTagRuleId);
int bcmVlan_removeTagRuleById(char *realDevName, UINT8 nbrOfTags,
                              bcmVlan_ruleTableDirection_t tableDir, bcmVlan_tagRuleIndex_t tagRuleId);
int bcmVlan_removeTagRuleByFilter(char *realDevName, UINT8 nbrOfTags,
                                  bcmVlan_ruleTableDirection_t tableDir,
                                  bcmVlan_tagRule_t *tagRule);
int bcmVlan_removeAllTagRulesByDev(char *vlanDevName);
int bcmVlan_removeTagRulesByVlanDev(struct vlanDeviceControl *vlanDevCtrl);
int bcmVlan_dumpTagRulesByTable(struct realDeviceControl *realDevCtrl, UINT8 nbrOfTags, bcmVlan_ruleTableDirection_t tableDir);
int bcmVlan_dumpTagRules(char *realDevName, unsigned int nbrOfTags, bcmVlan_ruleTableDirection_t tableDir);
int bcmVlan_dumpAllTagRules(void);
int bcmVlan_getNbrOfTagRulesByTable(char *realDevName, UINT8 nbrOfTags,
                                    bcmVlan_ruleTableDirection_t tableDir, unsigned int *nbrOfRules);
int bcmVlan_setDefaultVlanTag(char *realDevName, UINT8 nbrOfTags, bcmVlan_ruleTableDirection_t tableDir,
                              UINT16 defaultTpid, UINT8 defaultPbits, UINT8 defaultCfi, UINT16 defaultVid);
int bcmVlan_setDscpToPbitsTable(char *realDevName, UINT8 dscp, UINT8 pbits);
int bcmVlan_dumpDscpToPbitsTable(char *realDevName, UINT8 dscp);
int bcmVlan_processFrame(struct net_device *realDev, struct net_device *vlanDev,
                         struct sk_buff **skbp, bcmVlan_ruleTableDirection_t tableDir,
                         int *rxVlanDevInStackp);

/* bcm_vlan_test.c */
void bcmVlan_runTest(bcmVlan_iocRunTest_t *iocRunTest_p);

/* bcm_vlan_dp.c */
void bcmVlan_setDp(bcmVlan_iocSetDropPrecedence_t *iocSetDpPtr);
UBOOL8 bcmVlan_lookupDp(struct net_device *dev, uint8 *data, unsigned int len);


#endif /* _BCM_VLAN_LOCAL_H_ */
