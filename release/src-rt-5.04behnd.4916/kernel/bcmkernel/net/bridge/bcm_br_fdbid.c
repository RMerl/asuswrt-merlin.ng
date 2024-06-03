/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
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

#include <linux/spinlock.h>
#include "br_private.h"
#include <linux/bcm_br_fdbid.h>

static bcm_fdbid_tbl_t fdbid_tbl;
static bcm_fdbid_tbl_t *fdbid_tbl_p = NULL;

/*
 * Finds the next available free entry and marks it as used.
 * It starts searching the next entry from the last allocated entry. It wraps
 * around after reaching the end of the bitmap and starts from first entry.
 */
static inline int get_fdbidx(uint32_t *idx_p)
{
    static int offset = 1;
    uint32_t idx = 0;
    volatile unsigned long *addr;

    addr = &fdbid_tbl_p->free_idx_bitmap[0];
    idx = find_next_bit((const unsigned long *)addr, BCM_FDBID_MAX_ENTRIES, offset);

    if ((idx == BCM_FDBID_MAX_ENTRIES) && (offset != 1))
        idx = find_next_bit((const unsigned long *)addr, BCM_FDBID_MAX_ENTRIES, 1);
        
    if ((idx > 0) && (idx < BCM_FDBID_MAX_ENTRIES)) {
        *idx_p = idx;
        clear_bit(*idx_p, addr);

        if (idx == (BCM_FDBID_MAX_ENTRIES -1))
            offset = 1;
        else
            offset = idx + 1;

        return 0;
    } else {
        offset = 1;
    }

    return -1;
}

/*
 * Marks the specified entry as free in bitmap.
 */
static void put_fdbidx(uint32_t *idx_p)
{
    volatile unsigned long *addr;

    if (*idx_p < BCM_FDBID_MAX_ENTRIES) {
        addr = &fdbid_tbl_p->free_idx_bitmap[0];
        set_bit(*idx_p, addr);
    }
}

static inline void free_fdbid_entry(bcm_fdbid_map_ent_t *map_ent_p, uint32_t idx)
{
    spin_lock(&fdbid_tbl_p->lock);
    put_fdbidx(&idx);
    map_ent_p->fdb->fdbid = BCM_FDBID_NULL_ID; // unbind
    map_ent_p->fdb = NULL;
    spin_unlock(&fdbid_tbl_p->lock);
}

/*
 * Allocates an entry in fdbid_tbl to create an fdb to fdbid mapping.
 * Finds a free entry and increments the user_count.
 * Called from BCM_FDBID_EVT_CREATE notification
 */
static int alloc_fdbid(struct net_bridge_fdb_entry *fdb, uint16_t *fdbid)
{
    uint32_t idx = 0;
    int status = 0;
    bcm_fdbid_map_ent_t *map_ent_p;

    spin_lock(&fdbid_tbl_p->lock);
    if (get_fdbidx(&idx)) {
        status = -1;
        printk(KERN_ERR "[%s: %d]: invalid idx for fdb to fdbid mapping [%04x]\n",
            __FUNCTION__, __LINE__, idx);
        goto alloc_fdbid_exit;
    }

    map_ent_p = &fdbid_tbl_p->map_ent[idx];
    if (atomic_read(&map_ent_p->user_count) == 0) {
        map_ent_p->fdb = fdb;
        atomic_inc(&map_ent_p->user_count);
        *fdbid = map_ent_p->fdbid.fdbid;
    } else {
        status = -1;
        printk(KERN_ERR "[%s: %d] user count is not zero something is wrong\n",
            __FUNCTION__, __LINE__);
        goto alloc_fdbid_exit;
    }
//    printk(KERN_INFO "Assigning fdb idx[%04x]: fdbid[%04x] for %pM\n",
//        map_ent_p->fdbid.bmp.idx, map_ent_p->fdbid.fdbid, &fdb->key.addr.addr);

alloc_fdbid_exit:
    spin_unlock(&fdbid_tbl_p->lock);
    return status;
}

/*
 *  Frees the entry associated to the fdbid once the user_count reaches zero.
 */
static int free_fdbid(uint16_t fdbid)
{
    int status = 0;
    bcm_fdbid_map_ent_t *map_ent_p;
    uint32_t idx = (fdbid & BCM_FDBID_IDX_MASK);

    if (unlikely((idx == 0) || (idx >= BCM_FDBID_MAX_ENTRIES))) {
        status = -1;
        printk(KERN_ERR "[%s: %d] invalid idx for fdb to fdbid<%04x> mapping\n",
            __FUNCTION__, __LINE__, idx);
        goto free_fdbid_exit;
    }

    map_ent_p = &fdbid_tbl_p->map_ent[idx];
    if (unlikely(fdbid != map_ent_p->fdbid.fdbid)) {
        status = -1;
        printk(KERN_ERR "[%s: %d] fdbid<%04x> doesn't match with fdbid in table[%04x]\n",
            __FUNCTION__, __LINE__, fdbid, map_ent_p->fdbid.fdbid);
        goto free_fdbid_exit;
    }

    if (atomic_read(&map_ent_p->user_count) > 0) {
        if (atomic_dec_and_test(&map_ent_p->user_count)) {
//            printk(KERN_INFO "Freeing fdb<%pM> fdbid<%04x>\n",
//                &map_ent_p->fdb->key.addr.addr, map_ent_p->fdbid.fdbid);

            map_ent_p->fdbid.bmp.incarn++;
            if (map_ent_p->fdbid.bmp.incarn == 0)
                map_ent_p->fdbid.bmp.incarn = 1;
            //users is zero now delete this entry
            free_fdbid_entry(map_ent_p, idx);
        }
    } else {
        printk(KERN_ERR "[%s: %d] usercount is invalid for %pM\n",
            __FUNCTION__, __LINE__, &map_ent_p->fdb->key.addr.addr);
    }

free_fdbid_exit:
    return status;
}

#if 0
/*
 * Function to dump mapping table. 
 */
static void fdbid_dump(void)
{
    int idx = 0;
    bcm_fdbid_map_ent_t *map_ent_p;

    printk("\n\nFDBID Table Dump\n");
    for (idx=0; idx < BCM_FDBID_MAX_ENTRIES; idx++) {
        map_ent_p = &fdbid_tbl_p->map_ent[idx];
        if (atomic_read(&map_ent_p->user_count) > 0) {
            printk("idx:[%d] fdbid:[%04x: %d] fdb<%px> fdb:[%pM] user_count[%d]\n",
                map_ent_p->fdbid.bmp.idx, map_ent_p->fdbid.fdbid,
                map_ent_p->fdbid.fdbid, map_ent_p->fdb, &map_ent_p->fdb->key.addr.addr,
                atomic_read(&map_ent_p->user_count));
        }
    }
    printk("\n");
    return;
}
#endif

/*
 *  Given a fdbid checks if its valid or not.
 */
bool bcm_fdbid_is_fdbid_valid(uint32_t fdbid)
{
    bcm_fdbid_map_ent_t *map_ent_p;
    uint32_t idx = (fdbid & BCM_FDBID_IDX_MASK);

    map_ent_p = &fdbid_tbl_p->map_ent[idx];
    if (likely((map_ent_p->fdbid.fdbid == fdbid) &&
               (atomic_read(&map_ent_p->user_count) > 0)))
        return true;
    else
        return false;
}
EXPORT_SYMBOL(bcm_fdbid_is_fdbid_valid);

/*
 *  Given a fdbid locates mapped bridge FDB pointer and returns FDB pointer
 */
struct net_bridge_fdb_entry *bcm_fdbid_get_fdb_by_id(uint32_t fdbid)
{
    bcm_fdbid_map_ent_t *map_ent_p;
    uint32_t idx = (fdbid & BCM_FDBID_IDX_MASK);

    map_ent_p = &fdbid_tbl_p->map_ent[idx];

    if (likely((map_ent_p->fdbid.fdbid == fdbid) &&
               (atomic_read(&map_ent_p->user_count) > 0)))
        return map_ent_p->fdb;
    else
        printk(KERN_ERR "[%s: %d] match fails: fdbid[%04x], entry fdbid[%04x]\n",
            __FUNCTION__, __LINE__, fdbid, map_ent_p->fdbid.fdbid);

    return NULL;
}
EXPORT_SYMBOL(bcm_fdbid_get_fdb_by_id);

/*
 * This is the API which interfaces with the bridge FDB for creating or 
 * deleting an FDBID entry. 
 */
int bcm_fdbid_event(uint32_t event, struct net_bridge_fdb_entry *fdb)
{
    int ret = 0;

    switch (event) {
    case BCM_FDBID_EVT_CREATE:
        /* if the fdbid already exists, free it first. */
        if (bcm_fdbid_is_fdbid_valid(fdb->fdbid)) {
            if (free_fdbid(fdb->fdbid)) {
                printk(KERN_ERR "[%s: %d] BCM_FDBID_EVT_CREATE: Failed to "
                        "create fdb<%pM> fdbid<%04x> mapping\n",
                    __FUNCTION__, __LINE__, &fdb->key.addr.addr, fdb->fdbid);
                ret = -1;
                goto bcm_fdbid_event_exit;
            }
        }

        if (alloc_fdbid(fdb, &fdb->fdbid)) {
            printk(KERN_ERR "[%s: %d] BCM_FDBID_EVT_CREATE: Failed to "
                    "create fdb<%pM> fdbid mapping\n",
                __FUNCTION__, __LINE__, &fdb->key.addr.addr);
            ret = -1;
            goto bcm_fdbid_event_exit;
        }
        break;

    case BCM_FDBID_EVT_DELETE:
        if (free_fdbid(fdb->fdbid)) {
            printk(KERN_ERR "[%s: %d] BCM_FDBID_EVT_DELETE: Failed to delete "
                    "fdb<%pM> fdbid<%04x> mapping\n",
                __FUNCTION__, __LINE__, &fdb->key.addr.addr, fdb->fdbid);
            ret = -1;
            goto bcm_fdbid_event_exit;
        }
        break;

    default:
        printk(KERN_ERR "[%s: %d] unknown event fdb<%px><%pM>\n",
                __FUNCTION__, __LINE__, fdb, &fdb->key.addr.addr);
        ret = -1;
        goto bcm_fdbid_event_exit;
    }

#if 0
    fdbid_dump();
#endif

bcm_fdbid_event_exit:
    return ret;
}
EXPORT_SYMBOL(bcm_fdbid_event);

/*
 * Initializes the fdbid_tbl.  
 */
int bcm_fdbid_tbl_init(void)
{
    int idx;

    fdbid_tbl_p = &fdbid_tbl;
    memset(fdbid_tbl_p, 0, sizeof(bcm_fdbid_tbl_t));
    spin_lock_init(&fdbid_tbl_p->lock);

    for (idx=0; idx < BCM_FDBID_MAX_ENTRIES; idx++) {
        fdbid_tbl_p->map_ent[idx].fdbid.bmp.idx = idx;
        fdbid_tbl_p->map_ent[idx].fdbid.bmp.incarn = 1;
        fdbid_tbl_p->map_ent[idx].fdb = (struct net_bridge_fdb_entry *)NULL;
        atomic_set(&fdbid_tbl_p->map_ent[idx].user_count,0);

        //Mark this map_ent to be available for mapping
        set_bit(idx, &fdbid_tbl_p->free_idx_bitmap[0]);
    }

    // Mark the very first index with fdbid as 0 to be non-available
    clear_bit(0, &fdbid_tbl_p->free_idx_bitmap[0]);
    return 0;
}
EXPORT_SYMBOL(bcm_fdbid_tbl_init);

