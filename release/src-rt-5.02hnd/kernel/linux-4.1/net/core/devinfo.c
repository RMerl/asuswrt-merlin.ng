#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BRCM_DPI)
/*
<:copyright-BRCM:2014:DUAL/GPL:standard 

   Copyright (c) 2014 Broadcom 
   All Rights Reserved

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
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/devinfo.h>
#include <linux/bcm_colors.h>

typedef struct {
    DevInfo_t      * htable[ DEVINFO_HTABLE_SIZE ];
    DevInfo_t        etable[ DEVINFO_MAX_ENTRIES ];

    Dll_t         frlist;           /* List of free devinfo entries */
} __attribute__((aligned(16))) DeviceInfo_t;

DeviceInfo_t deviceInfo;    /* Global device info context */

#if defined(CC_DEVINFO_SUPPORT_DEBUG)
#define devinfo_print(fmt, arg...)                                           \
    if ( devinfo_dbg )                                                       \
        printk( CLRc "DEVINFO %s :" fmt CLRnl, __FUNCTION__, ##arg )
#define devinfo_assertv(cond)                                                \
    if ( !cond ) {                                                           \
        printk( CLRerr "DEVINFO ASSERT %s : " #cond CLRnl, __FUNCTION__ );   \
        return;                                                              \
    }
#define devinfo_assertr(cond, rtn)                                           \
    if ( !cond ) {                                                           \
        printk( CLRerr "DEVINFO ASSERT %s : " #cond CLRnl, __FUNCTION__ );   \
        return rtn;                                                          \
    }
#define DEVINFO_DBG(debug_code)    do { debug_code } while(0)
#else
#define devinfo_print(fmt, arg...) DEVINFO_NULL_STMT
#define devinfo_assertv(cond) DEVINFO_NULL_STMT
#define devinfo_assertr(cond, rtn) DEVINFO_NULL_STMT
#define DEVINFO_DBG(debug_code) DEVINFO_NULL_STMT
#endif

int devinfo_dbg = 0;

/*
 *------------------------------------------------------------------------------
 * Function     : devinfo_alloc
 * Description  : Allocate a device info entry
 *------------------------------------------------------------------------------
 */
static DevInfo_t * devinfo_alloc( void )
{
    DevInfo_t * dev_p = DEVINFO_NULL;

    if (unlikely(dll_empty(&deviceInfo.frlist)))
    {
        devinfo_print("no free entry! No collect now");
        return dev_p;
    }

    if (likely(!dll_empty(&deviceInfo.frlist)))
    {
        dev_p = (DevInfo_t*)dll_head_p(&deviceInfo.frlist);
        dll_delete(&dev_p->node);
    }

    devinfo_print("idx<%u>", dev_p->entry.idx);

    return dev_p;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _hash
 * Description  : Computes a simple hash from a 32bit value.
 *------------------------------------------------------------------------------
 */
static inline uint32_t _hash( uint32_t hash_val )
{
    hash_val ^= ( hash_val >> 16 );
    hash_val ^= ( hash_val >>  8 );
    hash_val ^= ( hash_val >>  3 );

    return ( hash_val );
}

/*
 *------------------------------------------------------------------------------
 * Function     : _devinfo_hash
 * Description  : Compute the hash of a MAC
 *------------------------------------------------------------------------------
 */
static inline uint32_t _devinfo_hash( const uint8_t *mac )
{
    uint32_t hashix;

    hashix = _hash( (*((uint32_t *) (&(mac[2])))) );

    return hashix % DEVINFO_HTABLE_SIZE;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _devinfo_match
 * Description  : Checks whether the mac matches.
 *------------------------------------------------------------------------------
 */
static inline uint32_t _devinfo_match( const DevInfo_t *dev_p,
                                       const uint8_t *mac )
{
    return ( !memcmp(dev_p->mac, mac, ETH_ALEN) );
}

/*
 *------------------------------------------------------------------------------
 * Function     : devinfo_hashin
 * Description  : Insert a new entry into the devinfo at a given hash index.
 *------------------------------------------------------------------------------
 */
static void devinfo_hashin( DevInfo_t * dev_p, uint32_t hashix )
{
    devinfo_print("enter");

    dev_p->chain_p = deviceInfo.htable[ hashix ];  /* Insert into hash table */
    deviceInfo.htable[ hashix ] = dev_p;
}

static uint32_t devinfo_new( const uint8_t *mac, uint32_t hashix )
{
    DevInfo_t * dev_p;

    devinfo_print("enter");

    dev_p = devinfo_alloc();
    if ( unlikely(dev_p == DEVINFO_NULL) )
    {
        devinfo_print("failed devinfo_alloc");
        return DEVINFO_IX_INVALID;              /* Element table depletion */
    }

    memcpy(dev_p->mac, mac, ETH_ALEN);
    devinfo_hashin(dev_p, hashix);              /* Insert into hash table */

    devinfo_print("idx<%u>", dev_p->entry.idx);

    return dev_p->entry.idx;
}

#if 0
/*
 *------------------------------------------------------------------------------
 * Function     : devinfo_free
 * Description  : Free a device info entry
 *------------------------------------------------------------------------------
 */
void devinfo_free( DevInfo_t * dev_p )
{
    dev_p->entry.flags = 0;
    dev_p->entry.vendor_id = 0;
    dev_p->entry.os_id = 0;
    dev_p->entry.class_id = 0;
    dev_p->entry.type_id = 0;
    dev_p->entry.dev_id = 0;

    memset(dev_p->mac, 0, ETH_ALEN);

    dll_prepend(&deviceInfo.frlist, &dev_p->node);
}

/*
 *------------------------------------------------------------------------------
 * Function     : devinfo_unhash
 * Description  : Remove a devinfo from the device info at a given hash index.
 *------------------------------------------------------------------------------
 */
static void devinfo_unhash(DevInfo_t * dev_p, uint32_t hashix)
{
    register DevInfo_t * hDev_p = deviceInfo.htable[hashix];

    if ( unlikely(hDev_p == DEVINFO_NULL) )
    {
        devinfo_print( "ERROR: deviceInfo.htable[%u] is NULL", hashix );
        goto devinfo_notfound;
    }

    if ( likely(hDev_p == dev_p) )                /* At head */
    {
        deviceInfo.htable[ hashix ] = dev_p->chain_p;  /* Delete at head */
    }
    else
    {
        uint32_t found = 0;

        /* Traverse the single linked hash collision chain */
        for ( hDev_p = deviceInfo.htable[ hashix ];
              likely(hDev_p->chain_p != DEVINFO_NULL);
              hDev_p = hDev_p->chain_p )
        {
            if ( hDev_p->chain_p == dev_p )
            {
                hDev_p->chain_p = dev_p->chain_p;
                found = 1;
                break;
            }
        }

        if ( unlikely(found == 0) )
        {
            devinfo_print( "ERROR:deviceInfo.htable[%u] find failure", hashix );
            goto devinfo_notfound;
        }
    }

    return; /* SUCCESS */

devinfo_notfound:
    devinfo_print( "not found: hash<%u>", hashix );
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function     : devinfo_lookup
 * Description  : Given a mac, lookup device info.
 *------------------------------------------------------------------------------
 */
uint16_t devinfo_lookup( const uint8_t *mac )
{
    DevInfo_t * dev_p;
    uint16_t idx;
    uint32_t hashix;

    hashix = _devinfo_hash(mac);

    devinfo_print("hashix<%u> mac<%02x:%02x:%02x:%02x:%02x:%02x>", 
                  hashix, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    for ( dev_p = deviceInfo.htable[ hashix ]; dev_p != DEVINFO_NULL;
          dev_p = dev_p->chain_p)
    {
        devinfo_print("elem: idx<%u> mac<%02x:%02x:%02x:%02x:%02x:%02x>",
                      dev_p->entry.idx,
                      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        if (likely( _devinfo_match(dev_p, mac) ))
        {
            devinfo_print("idx<%u>", dev_p->entry.idx);
            return dev_p->entry.idx;
        }
    }

    /* New device found, alloc an entry */
    idx = devinfo_new(mac, hashix);

    devinfo_print("idx<%u>", idx);

    return idx;
}

/*
 *------------------------------------------------------------------------------
 * Function     : devinfo_get
 * Description  : Given devinfo index, return the devinfo_entry.
 *------------------------------------------------------------------------------
 */
void devinfo_get( uint16_t idx, DevInfoEntry_t *entry )
{
    DevInfo_t * dev_p;

    memset(entry, 0, sizeof(DevInfoEntry_t));

    dev_p = &deviceInfo.etable[idx];
    entry->idx = dev_p->entry.idx;
    entry->flags = dev_p->entry.flags;
    entry->vendor_id = dev_p->entry.vendor_id;
    entry->os_id = dev_p->entry.os_id;
    entry->class_id = dev_p->entry.class_id;
    entry->type_id = dev_p->entry.type_id;
    entry->dev_id = dev_p->entry.dev_id;

    devinfo_print("idx<%u> flag<%u> ven<%u> os<%u> class<%u> type<%u> dev<%u>",
                  entry->idx, entry->flags, entry->vendor_id, entry->os_id,
                  entry->class_id, entry->type_id, entry->dev_id);

    return;
}

/*
 *------------------------------------------------------------------------------
 * Function     : devinfo_set
 * Description  : Given devinfo index, set the devinfo_entry.
 *------------------------------------------------------------------------------
 */
void devinfo_set( const DevInfoEntry_t *entry )
{
    DevInfo_t * dev_p;

    devinfo_print("idx<%u> flag<%u> ven<%u> os<%u> class<%u> type<%u> dev<%u>",
                  entry->idx, entry->flags, entry->vendor_id, entry->os_id,
                  entry->class_id, entry->type_id, entry->dev_id);

    dev_p = &deviceInfo.etable[entry->idx];
    dev_p->entry.flags = entry->flags;
    dev_p->entry.vendor_id = entry->vendor_id;
    dev_p->entry.os_id = entry->os_id;
    dev_p->entry.class_id = entry->class_id;
    dev_p->entry.type_id = entry->type_id;
    dev_p->entry.dev_id = entry->dev_id;

    return;
}

/*
 *------------------------------------------------------------------------------
 * Function     : devinfo_getmac
 * Description  : Given devinfo index, get the mac address.
 *------------------------------------------------------------------------------
 */
void devinfo_getmac( uint16_t idx, uint8_t *mac )
{
    DevInfo_t * dev_p;

    dev_p = &deviceInfo.etable[idx];
    memcpy(mac, dev_p->mac, ETH_ALEN);

    devinfo_print("idx<%d> mac<%2x:%2x:%2x:%2x:%2x:%2x>", idx, 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); 
}

int devinfo_init( void )
{
    register int id;
    DevInfo_t * dev_p;

    memset( (void*)&deviceInfo, 0, sizeof(DeviceInfo_t) );

    /* Initialize list */
    dll_init( &deviceInfo.frlist );

    /* Initialize each devinfo entry and insert into free list */
    for ( id=DEVINFO_IX_INVALID; id < DEVINFO_MAX_ENTRIES; id++ )
    {
        dev_p = &deviceInfo.etable[id];
        dev_p->entry.idx = id;

        if ( unlikely(id == DEVINFO_IX_INVALID) )
            continue;           /* Exclude this entry from the free list */

        dll_append(&deviceInfo.frlist, &dev_p->node);/* Insert into free list */
    }

    DEVINFO_DBG( printk( "DEVINFO devinfo_dbg<0x%08x> = %d\n"
                         "%d Available entries\n",
                         (int)&devinfo_dbg, devinfo_dbg,
                         DEVINFO_MAX_ENTRIES-1 ); );
    
    return 0;
}

EXPORT_SYMBOL(devinfo_init);
EXPORT_SYMBOL(devinfo_lookup);
EXPORT_SYMBOL(devinfo_get);
EXPORT_SYMBOL(devinfo_getmac);
EXPORT_SYMBOL(devinfo_set);
#endif /* if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BRCM_DPI) */
