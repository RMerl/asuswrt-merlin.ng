#ifndef _BCM_BR_FDBID_H
#define _BCM_BR_FDBID_H
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
