/*
<:copyright-BRCM:2002:DUAL/GPL:standard

   Copyright (c) 2002 Broadcom 
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
#ifndef _BCMSWSHARED_H_
#define _BCMSWSHARED_H_

#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>

/****************************************************************************
    Registers for pseudo PHY access
****************************************************************************/

#define PSEUDO_PHY_ADDR                                   0x1e

#define REG_PSEUDO_PHY_MII_REG16                          0x10

    #define REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT        8
    #define REG_PPM_REG16_MDIO_ENABLE                     0x01

#define REG_PSEUDO_PHY_MII_REG17                          0x11

    #define REG_PPM_REG17_REG_NUMBER_SHIFT                8
    #define REG_PPM_REG17_OP_DONE                         0x00
    #define REG_PPM_REG17_OP_WRITE                        0x01
    #define REG_PPM_REG17_OP_READ                         0x02

#define REG_PSEUDO_PHY_MII_REG24                          0x18
#define REG_PSEUDO_PHY_MII_REG25                          0x19
#define REG_PSEUDO_PHY_MII_REG26                          0x1a
#define REG_PSEUDO_PHY_MII_REG27                          0x1b

/****************************************************************************
    Switch SPI Interface
****************************************************************************/

#define BCM5325_SPI_CMD_LEN                 1
#define BCM5325_SPI_ADDR_LEN                1
#define BCM5325_SPI_PREPENDCNT              (BCM5325_SPI_CMD_LEN+BCM5325_SPI_ADDR_LEN)

/* 5325 SPI Status Register */
#define BCM5325_SPI_STS                     0xfe

/* 5325 SPI Status Register definition */
#define BCM5325_SPI_CMD_SPIF                0x80
#define BCM5325_SPI_CMD_RACK                0x20

/* 5325 Command Byte definition */
#define BCM5325_SPI_CMD_READ                0x00    /* bit 0 - Read/Write */
#define BCM5325_SPI_CMD_WRITE               0x01    /* bit 0 - Read/Write */
#define BCM5325_SPI_CHIPID_MASK             0x7     /* bit 3:1 - Chip ID */
#define BCM5325_SPI_CHIPID_SHIFT            1
#define BCM5325_SPI_CMD_NORMAL              0x60    /* bit 7:4 - Mode */
#define BCM5325_SPI_CMD_FAST                0x10    /* bit 4 - Mode */

/****************************************************************************
    External switch pseudo PHY: Page (0xff)
****************************************************************************/

#define PAGE_SELECT                         0xff

#if defined(BCM_ENET_LOG)
#define BCM_ENET_DEBUG(fmt, arg...)  BCM_LOG_DEBUG(BCM_LOG_ID_ENET, fmt, ##arg)
#define BCM_ENET_INFO(fmt, arg...)   BCM_LOG_INFO(BCM_LOG_ID_ENET, fmt, ##arg)
#define BCM_ENET_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_ENET, fmt, ##arg)
#define BCM_ENET_ERROR(fmt, arg...)  BCM_LOG_ERROR(BCM_LOG_ID_ENET, fmt, ##arg)
#else
#define BCM_ENET_DEBUG(fmt, arg...)   
#define BCM_ENET_INFO(fmt, arg...)  
#define BCM_ENET_NOTICE(fmt, arg...)  
#define BCM_ENET_ERROR(fmt, arg...)  
#endif

#if defined(BCM_ENET_RX_LOG)
#define BCM_ENET_RX_DEBUG(fmt, arg...)  BCM_LOG_DEBUG(BCM_LOG_ID_ENET, fmt, ##arg)
#define BCM_ENET_RX_INFO(fmt, arg...)   BCM_LOG_INFO(BCM_LOG_ID_ENET, fmt, ##arg)
#define BCM_ENET_RX_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_ENET, fmt, ##arg)
#define BCM_ENET_RX_ERROR(fmt, arg...)  BCM_LOG_ERROR(BCM_LOG_ID_ENET, fmt, ##arg)
#else
#define BCM_ENET_RX_DEBUG(fmt, arg...)   
#define BCM_ENET_RX_INFO(fmt, arg...)  
#define BCM_ENET_RX_NOTICE(fmt, arg...)  
#define BCM_ENET_RX_ERROR(fmt, arg...)  
#endif

#if defined(BCM_ENET_TX_LOG)
#define BCM_ENET_TX_DEBUG(fmt, arg...)  BCM_LOG_DEBUG(BCM_LOG_ID_ENET, fmt, ##arg)
#define BCM_ENET_TX_INFO(fmt, arg...)   BCM_LOG_INFO(BCM_LOG_ID_ENET, fmt, ##arg)
#define BCM_ENET_TX_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_ENET, fmt, ##arg)
#define BCM_ENET_TX_ERROR(fmt, arg...)  BCM_LOG_ERROR(BCM_LOG_ID_ENET, fmt, ##arg)
#else
#define BCM_ENET_TX_DEBUG(fmt, arg...)   
#define BCM_ENET_TX_INFO(fmt, arg...)  
#define BCM_ENET_TX_NOTICE(fmt, arg...)  
#define BCM_ENET_TX_ERROR(fmt, arg...)  
#endif

#if defined(BCM_ENET_LINK_LOG)
#define BCM_ENET_LINK_DEBUG(fmt, arg...)	BCM_LOG_DEBUG(BCM_LOG_ID_ENET, fmt, ##arg)
#define BCM_ENET_LINK_INFO(fmt, arg...)	BCM_LOG_INFO(BCM_LOG_ID_ENET, fmt, ##arg)
#define BCM_ENET_LINK_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_ENET, fmt, ##arg)
#define BCM_ENET_LINK_ERROR(fmt, arg...)	BCM_LOG_ERROR(BCM_LOG_ID_ENET, fmt, ##arg)
#else
#define BCM_ENET_LINK_DEBUG(fmt, arg...)	 
#define BCM_ENET_LINK_INFO(fmt, arg...)  
#define BCM_ENET_LINK_NOTICE(fmt, arg...)  
#define BCM_ENET_LINK_ERROR(fmt, arg...)	
#endif

#endif /* _BCMSWSHARED_H_ */

