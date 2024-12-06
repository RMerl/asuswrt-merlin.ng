/*
<:copyright-BRCM:2021:DUAL/GPL:standard

   Copyright (c) 2021 Broadcom 
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
#ifndef __GDX_HW__H__
#define __GDX_HW__H__

/* # of device groups (e.g. CPU ports on Runner). default = 1 */
#define GDX_MAX_DEV_GROUPS      1
#define GDX_GENDEV_MAX_NUM_DEV  BCM_NETDEV_DEVID_MAX_ENTRIES
#define GDX_SKB_STATUS_FREE     0
#define GDX_SKB_STATUS_IN_USE   1
#define GDX_SKB_STATUS_MISS     GDX_SKB_STATUS_IN_USE
#define GDX_SKB_STATUS_HIT      2
#define GDX_SKB_STATUS_DROP     3

/** Receive packet info */
typedef struct
{
    void *data;           /* data pointer */
    uint32_t size;        /* data size */
    uint32_t data_offset; /* data offset inside pointer */
    uint16_t gdx_pd_data;
    uint8_t is_exception;
    int dev_idx;
    struct net_device *tx_dev; /* tx device */
    struct {
        uint32_t rx_csum_verified:1;
        uint32_t reserved:31;
    };
} gdx_hwacc_rx_info_t;

#endif /* __GDX_HW__H__ */
