#ifndef _BCM_BR_FDBID_H
#define _BCM_BR_FDBID_H
/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
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

#define BCM_FDBID_EVT_CREATE         0
#define BCM_FDBID_EVT_DELETE         1


/*------------------------------------------------------------------------------------
 * BCM fdbid map table and its related functions
 *-----------------------------------------------------------------------------------*/
#define BCM_FDBID_MAX_BITS      (14)
#define BCM_FDBID_MAX_ENTRIES   (1 << BCM_FDBID_MAX_BITS)

#if (BCM_FDBID_MAX_ENTRIES < CONFIG_BCM_MAX_FDB)
#error "Invalid number of FDBID entries"
#endif

#define BCM_FDBID_IDX_MASK      (BCM_FDBID_MAX_ENTRIES - 1)
#define BCM_FDBID_INCARN_BITS   (2)
#define BCM_FDBID_AVAIL_BITMAP_NUM_WORDS  (BCM_FDBID_MAX_ENTRIES / (sizeof(unsigned long) * 8))
#define BCM_FDBID_NULL_ID       (0)

#define bcm_fdbid_id_get_fdbid(x)     (x.fdbid)
#define bcm_fdbid_id_get_idx(x)       (x.bmp.idx)
#define bcm_fdbid_id_get_incarn(x)    (x.bmp.incarn)

typedef struct {
    union {
       struct {
           uint16_t idx:BCM_FDBID_MAX_BITS;
           uint16_t incarn:BCM_FDBID_INCARN_BITS;
       } bmp;
       uint16_t fdbid;
    };
} bcm_fdbid_t;

typedef struct {
    struct net_bridge_fdb_entry *fdb;
    atomic_t user_count;
    bcm_fdbid_t fdbid;
} bcm_fdbid_map_ent_t;

typedef struct {
    spinlock_t lock;
    // bitmap for availability
    unsigned long free_idx_bitmap[BCM_FDBID_AVAIL_BITMAP_NUM_WORDS];
    bcm_fdbid_map_ent_t map_ent[BCM_FDBID_MAX_ENTRIES];
} bcm_fdbid_tbl_t;

int bcm_fdbid_tbl_init(void);
bool bcm_fdbid_is_fdbid_valid(uint32_t fdbid);
struct net_bridge_fdb_entry *bcm_fdbid_get_fdb_by_id(uint32_t fdbid);
int bcm_fdbid_event(uint32_t event, struct net_bridge_fdb_entry *fdb);

#endif
