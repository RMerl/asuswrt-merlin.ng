#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BRCM_DPI)
/*
<:copyright-BRCM:2014:DUAL/GPL:standard 

   Copyright (c) 2014 Broadcom 
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
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/dpistats.h>
#include <linux/bcm_colors.h>
#include <linux/devinfo.h>
#include <linux/seq_file.h>

typedef struct {
    DpiStats_t      * htable[ DPISTATS_HTABLE_SIZE ];
    DpiStats_t        etable[ DPISTATS_MAX_ENTRIES ];

    Dll_t         usedlist;           /* List of used dpistats entries */
    Dll_t         frlist;           /* List of free dpistats entries */
} __attribute__((aligned(16))) DpiStatistic_t;

DpiStatistic_t dpistats;    /* Global dpi stats context */

#if defined(CC_DPISTATS_SUPPORT_DEBUG)
#define dpistats_print(fmt, arg...)                                           \
    if ( dpistats_dbg )                                                       \
        printk( CLRc "DPISTATS %s :" fmt CLRnl, __FUNCTION__, ##arg )
#define dpistats_assertv(cond)                                                \
    if ( !cond ) {                                                           \
        printk( CLRerr "DPISTATS ASSERT %s : " #cond CLRnl, __FUNCTION__ );   \
        return;                                                              \
    }
#define dpistats_assertr(cond, rtn)                                           \
    if ( !cond ) {                                                           \
        printk( CLRerr "DPISTATS ASSERT %s : " #cond CLRnl, __FUNCTION__ );   \
        return rtn;                                                          \
    }
#define DPISTATS_DBG(debug_code)    do { debug_code } while(0)
#else
#define dpistats_print(fmt, arg...) DPISTATS_NULL_STMT
#define dpistats_assertv(cond) DPISTATS_NULL_STMT
#define dpistats_assertr(cond, rtn) DPISTATS_NULL_STMT
#define DPISTATS_DBG(debug_code) DPISTATS_NULL_STMT
#endif

int dpistats_dbg = 0;

/*
 *------------------------------------------------------------------------------
 * Function     : dpistats_alloc
 * Description  : Allocate a dpi stats entry
 *------------------------------------------------------------------------------
 */
static DpiStats_t * dpistats_alloc( void )
{
    DpiStats_t * stats_p = DPISTATS_NULL;

    if (unlikely(dll_empty(&dpistats.frlist)))
    {
        dpistats_print("no free entry! No collect now");
        return stats_p;
    }

    if (likely(!dll_empty(&dpistats.frlist)))
    {
        stats_p = (DpiStats_t*)dll_head_p(&dpistats.frlist);
        dll_delete(&stats_p->node);
    }

    dpistats_print("idx<%u>", stats_p->entry.idx);

    return stats_p;
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
 * Function     : _dpistats_hash
 * Description  : Compute the hash of dpi info
 *------------------------------------------------------------------------------
 */
static inline uint32_t _dpistats_hash( unsigned int app_id, uint16_t dev_key )
{
    uint32_t hashix;

    hashix = _hash( app_id + dev_key );

    return hashix % DPISTATS_HTABLE_SIZE;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _dpistats_match
 * Description  : Checks whether the dpi info matches.
 *------------------------------------------------------------------------------
 */
static inline uint32_t _dpistats_match( const dpi_info_t *elem_p,
                                        const dpi_info_t *res_p )
{
    return ( (elem_p->app_id == res_p->app_id) &&
             (elem_p->dev_key == res_p->dev_key) );
}

/*
 *------------------------------------------------------------------------------
 * Function     : dpistats_hashin
 * Description  : Insert a new entry into the dpistats at a given hash index.
 *------------------------------------------------------------------------------
 */
static void dpistats_hashin( DpiStats_t * stats_p, uint32_t hashix )
{
    dpistats_print("enter");

    dll_prepend(&dpistats.usedlist, &stats_p->node);
    stats_p->chain_p = dpistats.htable[ hashix ];  /* Insert into hash table */
    dpistats.htable[ hashix ] = stats_p;
}

static uint32_t dpistats_new( const dpi_info_t *res_p, uint32_t hashix )
{
    DpiStats_t * stats_p;

    dpistats_print("enter");

    stats_p = dpistats_alloc();
    if ( unlikely(stats_p == DPISTATS_NULL) )
    {
        dpistats_print("failed dpistats_alloc");
        return DPISTATS_IX_INVALID;              /* Element table depletion */
    }

    stats_p->entry.result.app_id = res_p->app_id;
    stats_p->entry.result.dev_key = res_p->dev_key;

    dpistats_hashin(stats_p, hashix);              /* Insert into hash table */

    dpistats_print("idx<%u>", stats_p->entry.idx);

    return stats_p->entry.idx;
}

#if 0
/*
 *------------------------------------------------------------------------------
 * Function     : dpistats_free
 * Description  : Free a device info entry
 *------------------------------------------------------------------------------
 */
void dpistats_free( DpiStats_t * dev_p )
{
    dev_p->entry.flags = 0;
    dev_p->entry.vendor_id = 0;
    dev_p->entry.os_id = 0;
    dev_p->entry.class_id = 0;
    dev_p->entry.type_id = 0;
    dev_p->entry.dev_id = 0;

    memset(dev_p->mac, 0, ETH_ALEN);

    dll_prepend(&dpistats.frlist, &dev_p->node);
}

/*
 *------------------------------------------------------------------------------
 * Function     : dpistats_unhash
 * Description  : Remove a dpistats from the device info at a given hash index.
 *------------------------------------------------------------------------------
 */
static void dpistats_unhash(DpiStats_t * dev_p, uint32_t hashix)
{
    register DpiStats_t * hDev_p = dpistats.htable[hashix];

    if ( unlikely(hDev_p == DPISTATS_NULL) )
    {
        dpistats_print( "dpistats.htable[%u] is NULL", hashix );
        goto dpistats_notfound;
    }

    if ( likely(hDev_p == dev_p) )                /* At head */
    {
        dpistats.htable[ hashix ] = dev_p->chain_p;  /* Delete at head */
    }
    else
    {
        uint32_t found = 0;

        /* Traverse the single linked hash collision chain */
        for ( hDev_p = dpistats.htable[ hashix ];
              likely(hDev_p->chain_p != DPISTATS_NULL);
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
            dpistats_print( "dpistats.htable[%u] find failure", hashix );
            goto dpistats_notfound;
        }
    }

    return; /* SUCCESS */

dpistats_notfound:
    dpistats_print( "not found: hash<%u>\n", hashix );
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function     : dpistats_lookup
 * Description  : Given appID and devKey, lookup corresponding appinst entry.
 *------------------------------------------------------------------------------
 */
uint32_t dpistats_lookup( const dpi_info_t *res_p )
{
    DpiStats_t * stats_p;
    uint32_t idx;
    uint32_t hashix;

    hashix = _dpistats_hash(res_p->app_id, res_p->dev_key);

    dpistats_print("hashix<%u> appID<%06x> devkey<%u>",
                  hashix, res_p->app_id, res_p->dev_key);

    for ( stats_p = dpistats.htable[ hashix ]; stats_p != DPISTATS_NULL;
          stats_p = stats_p->chain_p)
    {
        dpistats_print("elem: idx<%u> appID<%06x> devkey<%u>",
                      stats_p->entry.idx, stats_p->entry.result.app_id,
                      stats_p->entry.result.dev_key);

        if (likely( _dpistats_match(&stats_p->entry.result, res_p) ))
        {
            dpistats_print("idx<%u>", stats_p->entry.idx);
            return stats_p->entry.idx;
        }
    }

    /* New device found, alloc an entry */
    idx = dpistats_new(res_p, hashix);

    dpistats_print("idx<%u>", idx);

    return idx;
}

/*
 *------------------------------------------------------------------------------
 * Function     : dpistats_info
 * Description  : when querying stats, get current statistic of one conntrack
 *------------------------------------------------------------------------------
 */
void dpistats_info( uint32_t idx, const DpiStatsEntry_t *stats_p )
{
    /*
     * Null stats_p means beginning of query: reset stats.
     */
    if (!(stats_p == (DpiStatsEntry_t *)NULL))
    {
        DpiStats_t *elem_p;
        CtkStats_t *ctk1_p;
        const CtkStats_t *ctk2_p;

        dpistats_print("idx<%d> appID<%06x> dev_key<%u>", idx, 
                        stats_p->result.app_id, stats_p->result.dev_key);

        dpistats_assertv( (idx != DPISTATS_IX_INVALID) );

        elem_p = &dpistats.etable[idx];

        ctk1_p = &elem_p->entry.upstream;
        ctk2_p = &stats_p->upstream;

        ctk1_p->pkts += ctk2_p->pkts;
        ctk1_p->bytes += ctk2_p->bytes;
        if (ctk1_p->ts < ctk2_p->ts) ctk1_p->ts = ctk2_p->ts;

        ctk1_p = &elem_p->entry.dnstream;
        ctk2_p = &stats_p->dnstream;

        ctk1_p->pkts += ctk2_p->pkts;
        ctk1_p->bytes += ctk2_p->bytes;
        if (ctk1_p->ts < ctk2_p->ts) ctk1_p->ts = ctk2_p->ts;
    }
    else
    {
        Dll_t  *tmp_p;
        Dll_t  *list_p;
        DpiStats_t *elem_p;

        dpistats_print("Reset");

        list_p = &dpistats.usedlist;

        if (!dll_empty(list_p))
        {
            dll_for_each(tmp_p, list_p) 
            {
                CtkStats_t *ctk_p;
                elem_p = (DpiStats_t *)tmp_p;

                dpistats_print("idx<%d> appID<%06x> dev_key<%u>",
                                elem_p->entry.idx, elem_p->entry.result.app_id,
                                elem_p->entry.result.dev_key); 

                ctk_p = &elem_p->entry.upstream;
                ctk_p->pkts = 0;
                ctk_p->bytes = 0;

                ctk_p = &elem_p->entry.dnstream;
                ctk_p->pkts = 0;
                ctk_p->bytes = 0;
            }
        }
    }
    dpistats_print("exit");
}

/*
 *------------------------------------------------------------------------------
 * Function     : dpistats_update
 * Description  : when a conntrack evicts, record the statistics
 *------------------------------------------------------------------------------
 */
void dpistats_update( uint32_t idx, const DpiStatsEntry_t *stats_p )
{
    DpiStats_t *elem_p;
    CtkStats_t *ctk1_p;
    const CtkStats_t *ctk2_p;

    dpistats_print("idx<%d> uppkt<%lu> upbyte<%llu> upts<%lu> "
                   "dnpkt<%lu> dnbyte<%llu> dnts<%lu>", idx, 
                   stats_p->upstream.pkts, stats_p->upstream.bytes, 
                   stats_p->upstream.ts, stats_p->dnstream.pkts, 
                   stats_p->dnstream.bytes, stats_p->dnstream.ts); 

    dpistats_assertv( ((idx != DPISTATS_IX_INVALID) && (stats_p != NULL)) );

    elem_p = &dpistats.etable[idx];

    ctk1_p = &elem_p->evict_up;
    ctk2_p = &stats_p->upstream;

    ctk1_p->pkts += ctk2_p->pkts;
    ctk1_p->bytes += ctk2_p->bytes;
    if (ctk1_p->ts < ctk2_p->ts) ctk1_p->ts = ctk2_p->ts;

    ctk1_p = &elem_p->evict_dn;
    ctk2_p = &stats_p->dnstream;

    ctk1_p->pkts += ctk2_p->pkts;
    ctk1_p->bytes += ctk2_p->bytes;
    if (ctk1_p->ts < ctk2_p->ts) ctk1_p->ts = ctk2_p->ts;
}

/*
 *------------------------------------------------------------------------------
 * Function     : dpistats_show
 * Description  : show dpi statistics
 *------------------------------------------------------------------------------
 */
void dpistats_show( struct seq_file *s )
{
    Dll_t  *tmp_p;
    Dll_t  *list_p;
    DpiStats_t *elem_p;

    dpistats_print("enter");

    list_p = &dpistats.usedlist;

    if (!dll_empty(list_p))
    {
        dll_for_each(tmp_p, list_p) 
        {
            CtkStats_t *ctk_p, *evict_p;
            elem_p = (DpiStats_t *)tmp_p;

            seq_printf(s, "%08x ", elem_p->entry.result.app_id);

            if (elem_p->entry.result.dev_key != DEVINFO_IX_INVALID)
            {
                uint8_t mac[6];
                DevInfoEntry_t entry;

                devinfo_getmac(elem_p->entry.result.dev_key, mac);
                devinfo_get(elem_p->entry.result.dev_key, &entry);

                seq_printf(s, "%02x:%02x:%02x:%02x:%02x:%02x ",
                        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

                seq_printf(s, "%u %u %u %u %u ",
                        entry.vendor_id, entry.os_id, entry.class_id,
                        entry.type_id, entry.dev_id);
            }
            else
            {
                seq_printf(s, "NoMac ");
            }

            ctk_p = &elem_p->entry.upstream;
            evict_p = &elem_p->evict_up;
//            printk("%lu %llu %lu ", ctk_p->pkts + evict_p->pkts,
//                    ctk_p->bytes + evict_p->bytes,
//                    ctk_p->ts);
            seq_printf(s, "%lu %llu ", ctk_p->pkts + evict_p->pkts,
                    ctk_p->bytes + evict_p->bytes);
            
            ctk_p = &elem_p->entry.dnstream;
            evict_p = &elem_p->evict_dn;
//            printk("%lu %llu %lu ", ctk_p->pkts + evict_p->pkts,
//                    ctk_p->bytes + evict_p->bytes,
//                    ctk_p->ts);
            seq_printf(s, "%lu %llu ", ctk_p->pkts + evict_p->pkts,
                    ctk_p->bytes + evict_p->bytes);

//            printk("%x ", elem_p->entry.result.flags);
            seq_printf(s, "\n");
        }
    }
}

int dpistats_init( void )
{
    register uint32_t id;
    DpiStats_t * stats_p;

    memset( (void*)&dpistats, 0, sizeof(DpiStatistic_t) );

    /* Initialize list */
    dll_init( &dpistats.frlist );
    dll_init( &dpistats.usedlist );

    /* Initialize each dpistats entry and insert into free list */
    for ( id=DPISTATS_IX_INVALID; id < DPISTATS_MAX_ENTRIES; id++ )
    {
        stats_p = &dpistats.etable[id];
        stats_p->entry.idx = id;

        if ( unlikely(id == DPISTATS_IX_INVALID) )
            continue;           /* Exclude this entry from the free list */

        dll_append(&dpistats.frlist, &stats_p->node);/* Insert into free list */
    }

    DPISTATS_DBG( printk( "DPISTATS dpistats_dbg<0x%08x> = %d\n"
                         "%d Available entries\n",
                         (int)&dpistats_dbg, dpistats_dbg,
                         DPISTATS_MAX_ENTRIES-1 ); );
    
    return 0;
}

EXPORT_SYMBOL(dpistats_init);
EXPORT_SYMBOL(dpistats_info);
EXPORT_SYMBOL(dpistats_update);
EXPORT_SYMBOL(dpistats_show);
EXPORT_SYMBOL(dpistats_lookup);
#endif /* if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BRCM_DPI) */
