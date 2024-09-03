/*
<:copyright-BRCM:2021:DUAL/GPL:standard

   Copyright (c) 2021 Broadcom 
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
