#ifndef __GDX_H__
#define __GDX_H__
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

#include <linux/netdevice.h>
#include <linux/bcm_colors.h>
#include "pktHdr.h"
#include "bcm_assert_locks.h"
#include "linux/bcm_log.h"

/** GDX return codes */
#define GDX_SUCCESS           0
#define GDX_FAILURE          -1
#define GDX_ERR_NOMEM        -2 
#define GDX_ERR_NO_MORE      -3
#define GDX_ERR_NO_SKB_IDX   -4
#define GDX_ERR_INV_DEV_IDX  -5
#define GDX_ERR_INV_DEV      -6

#define GDX_PRINT_LVL_ERROR   0
#define GDX_PRINT_LVL_INFO    1
#define GDX_PRINT_LVL_DBG     2
#define GDX_PRINT_LVL_DBG1    3

#define GDX_ASSERT(exp)     BCM_ASSERT_A(exp)

//#define GDX_PRINT_LVL_ENABLE

#define GDX_PRINT(fmt, arg...) \
        bcm_printk("%s: " fmt "\n", __FUNCTION__, ##arg)

#define GDX_PRINT_ERROR(fmt, arg...) \
    if (gdx_print_lvl >= GDX_PRINT_LVL_ERROR) \
        bcm_printk(CLRr "%s: " fmt CLRnl, __FUNCTION__, ##arg)

#if defined(GDX_PRINT_LVL_ENABLE)
#define GDX_PRINT_INFO(fmt, arg...) \
    if (gdx_print_lvl >= GDX_PRINT_LVL_INFO) \
        bcm_printk("%s: " fmt "\n", __FUNCTION__, ##arg)

#define GDX_PRINT_DBG(fmt, arg...) \
    if (gdx_print_lvl >= GDX_PRINT_LVL_DBG) \
        bcm_printk("%s: " fmt "\n", __FUNCTION__, ##arg)

#define GDX_PRINT_DBG1(fmt, arg...) \
    if (gdx_print_lvl >= GDX_PRINT_LVL_DBG1) \
        bcm_printk("%s: " fmt "\n", __FUNCTION__, ##arg)

#define GDX_PKT_DUMP
#else
#define GDX_PRINT_INFO(fmt, arg...)
#define GDX_PRINT_DBG(fmt, arg...)
#define GDX_PRINT_DBG1(fmt, arg...)
#endif

#define GDX_GENDEV_NOTIFY_EVENT_DEL    0
#define GDX_GENDEV_NOTIFY_EVENT_ADD    1

#define GDX_FILL_ARGS_GET_PREPDATA(args)  (((GDX_prepend_fill_args_t*)args)->prepend_data)
#define GDX_FILL_ARGS_GET_BLOGP(args)  ((Blog_t *)(((GDX_prepend_fill_args_t*)args)->blog_p))
#define GDX_FILL_ARGS_GET_MAXSIZE(args)  (((GDX_prepend_fill_args_t*)args)->max_prep_size)

/* Types must match the definitions in global variable xmit_args_fields[] */
typedef struct {
    uint32_t mark; 
    uint32_t priority;
    BlogFcArgs_t fc_args;
}GDX_HwAccPrependInfo_t;
#define GDX_PREPINFO_SIZE sizeof(GDX_HwAccPrependInfo_t)

typedef struct {
    uint16_t  field_mask;
    union {
        uint8_t tx_flags;
        struct
        {
            uint8_t reserved:3;
            uint8_t tcp_discard:1;
            uint8_t fro:1;
            uint8_t use_xmit_args:1;
            uint8_t tx_is_ipv4:1;
            uint8_t use_tcplocal_xmit_enq_fn:1;
        };
    };
    uint8_t   prepend_total_size;
    /* Prepend fields are populated after this data header
       based on prepend fields mask. Place holder to access
       the memory and start writing */
    uint8_t   field_start;
}GDX_HwAccPrepend_data_t;
#define GDX_PREPDATA_GET_FIELD_MASK(prep_data) (((GDX_HwAccPrepend_data_t*)prep_data)->field_mask)
#define GDX_PREPDATA_GET_TX_FLAGS(prep_data) (((GDX_HwAccPrepend_data_t*)prep_data)->tx_flags)
#define GDX_PREPDATA_GET_FIELD_START_PTR(prep_data) (&((GDX_HwAccPrepend_data_t*)prep_data)->field_start)
#define GDX_PREPDATA_GET_PREPEND_SIZE(prep_data) (((GDX_HwAccPrepend_data_t*)prep_data)->prepend_total_size)

typedef struct {
    GDX_HwAccPrependInfo_t prep_info; 
    uint8_t *prepend_data;
}GDX_Prepend_FillInfo_t;
#define GDX_FILLINFO_GET_PREP_INFO(args) (((GDX_Prepend_FillInfo_t *)args)->prep_info)
#define GDX_FILLINFO_GET_PREP_DATA(args) ((GDX_HwAccPrepend_data_t *)(((GDX_Prepend_FillInfo_t *)args)->prepend_data))
#define GDX_FILLINFO_GET_PREP_INFO_FC_ARGS(args) (&(((GDX_Prepend_FillInfo_t *)args)->prep_info.fc_args))

/* These are the list of skb fields accessed by GDX binary.
   Each field in this structure stores the actual offset value in skb.
   This structure is used to check any offset mismatches between
   opensource and binary at init time */
typedef struct 
{
    size_t data;
    size_t __pkt_type_offset;
    size_t protocol;
    size_t head;
    size_t transport_header;
    size_t network_header;
    size_t dev;
    size_t bcm_ext;
    size_t flags; /* skb->bcm_ext.flags */
}gdx_skb_field_offset_t;

#define GDX_CHECK_SKB_OFFSET_RET(ptr, field)  \
if (BCM_SKBOFFSETOF(field) != ptr->field)    \
    {   \
        GDX_PRINT_ERROR("field %s offset src %lu binary %lu", #field, BCM_SKBOFFSETOF(field), ptr->field); \
        return -1;  \
    } 

#define GDX_CHECK_SKBEXT_OFFSET_RET(ptr, field)  \
if (BCM_SKB_EXTOFFSETOF(field) != ptr->field)    \
    {   \
        GDX_PRINT_ERROR("field %s offset src %lu binary %lu", #field, BCM_SKB_EXTOFFSETOF(field), ptr->field); \
        return -1;  \
    } 

/* These are the list of netdevice fields accessed by GDX binary.
   Each field in this structure stores the actual offset value in netdevice.
   This structure is used to check any offset mismatches between
   opensource and binary at init time */
typedef struct
{
    size_t tx_dropped;
    size_t type;
    size_t iff_flags; /* netdevice->bcm_nd_ext.iff_flags */
}gdx_netdev_field_offset_t;

#define GDX_CHECK_NETDEV_OFFSET_RET(ptr, field)  \
if (BCM_NETDEVOFFSETOF(field) != ptr->field)    \
    {   \
        GDX_PRINT_ERROR("field %s offset src %lu binary %lu", #field, BCM_NETDEVOFFSETOF(field), ptr->field); \
        return -1;  \
    } 

#define GDX_CHECK_NETDEVEXT_OFFSET_RET(ptr, field)  \
if (BCM_NETDEV_EXTOFFSETOF(field) != ptr->field)    \
    {   \
        GDX_PRINT_ERROR("field %s offset src %lu binary %lu", #field, BCM_NETDEV_EXTOFFSETOF(field), ptr->field); \
        return -1;  \
    } 
#endif /* __GDX_H__ */
