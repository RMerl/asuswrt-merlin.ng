/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
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

/***************************************************************************
 * File Name  : xtmcfgdrv.h
 *
 * Description: This file contains the definitions and structures for the
 *              Linux IOCTL interface that used between the user mode
 *              bcmxtmcfg library and the kernel bcmxtmcfg driver.
 ***************************************************************************/

#if !defined(_XTMCFGDRV_H_)
#define _XTMCFGDRV_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* Includes. */
#include <bcmxtmcfg.h>
#include "bcmtypes.h"

/* Defines. */
#define XTMCFGDRV_MAJOR         329
#define XTMCFG_INVALID_FIELD    ((UINT32) -1)

#define XTMCFGIOCTL_INITIALIZE \
    _IOWR(XTMCFGDRV_MAJOR, 0, XTMCFGDRV_INITIALIZE)
#define XTMCFGIOCTL_UNINITIALIZE \
    _IOR(XTMCFGDRV_MAJOR, 1, XTMCFGDRV_STATUS_ONLY)
#define XTMCFGIOCTL_GET_TRAFFIC_DESCR_TABLE \
    _IOWR(XTMCFGDRV_MAJOR, 2, XTMCFGDRV_TRAFFIC_DESCR_TABLE)
#define XTMCFGIOCTL_SET_TRAFFIC_DESCR_TABLE \
    _IOWR(XTMCFGDRV_MAJOR, 3, XTMCFGDRV_TRAFFIC_DESCR_TABLE)
#define XTMCFGIOCTL_GET_INTERFACE_CFG \
    _IOWR(XTMCFGDRV_MAJOR, 4, XTMCFGDRV_INTERFACE_CFG)
#define XTMCFGIOCTL_SET_INTERFACE_CFG \
    _IOWR(XTMCFGDRV_MAJOR, 5, XTMCFGDRV_INTERFACE_CFG)
#define XTMCFGIOCTL_GET_CONN_CFG \
    _IOWR(XTMCFGDRV_MAJOR, 6, XTMCFGDRV_CONN_CFG)
#define XTMCFGIOCTL_SET_CONN_CFG \
    _IOWR(XTMCFGDRV_MAJOR, 7, XTMCFGDRV_CONN_CFG)
#define XTMCFGIOCTL_GET_CONN_ADDRS \
    _IOWR(XTMCFGDRV_MAJOR, 8, XTMCFGDRV_CONN_ADDRS)
#define XTMCFGIOCTL_GET_INTERFACE_STATISTICS \
    _IOWR(XTMCFGDRV_MAJOR, 9, XTMCFGDRV_INTERFACE_STATISTICS)
#define XTMCFGIOCTL_SET_INTERFACE_LINK_INFO \
    _IOWR(XTMCFGDRV_MAJOR, 10, XTMCFGDRV_INTERFACE_LINK_INFO)
#define XTMCFGIOCTL_SEND_OAM_CELL \
    _IOWR(XTMCFGDRV_MAJOR, 11, XTMCFGDRV_SEND_OAM_CELL)
#define XTMCFGIOCTL_CREATE_NETWORK_DEVICE \
    _IOWR(XTMCFGDRV_MAJOR, 12, XTMCFGDRV_CREATE_NETWORK_DEVICE)
#define XTMCFGIOCTL_DELETE_NETWORK_DEVICE \
    _IOWR(XTMCFGDRV_MAJOR, 13, XTMCFGDRV_DELETE_NETWORK_DEVICE)
#define XTMCFGIOCTL_REINITIALIZE \
    _IOWR(XTMCFGDRV_MAJOR, 14, XTMCFGDRV_STATUS_ONLY)
#define XTMCFGIOCTL_GET_BONDING_INFO \
    _IOWR(XTMCFGDRV_MAJOR, 15, XTMCFGDRV_BONDING_INFO)
#define XTMCFGIOCTL_SET_DS_PTMBONDING_DEVIATION \
    _IOWR(XTMCFGDRV_MAJOR, 16, XTMCFGDRV_PTMBONDINGCFG)
#define XTMCFGIOCTL_CONFIGURE \
    _IOWR(XTMCFGDRV_MAJOR, 17, XTMCFGDRV_CONFIGURE)
#define XTMCFGIOCTL_GET_ERROR_STATISTICS \
    _IOWR(XTMCFGDRV_MAJOR, 18, XTMCFGDRV_ERROR_STATISTICS)
#define XTMCFGIOCTL_MANAGE_THRESHOLD \
    _IOWR(XTMCFGDRV_MAJOR, 19, XTMCFGDRV_THRESHOLD)

#define MAX_XTMCFGDRV_IOCTL_COMMANDS   20

/* Typedefs. */
typedef struct
{
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_STATUS_ONLY, *PXTMCFGDRV_STATUS_ONLY;

typedef struct
{   XTM_INITIALIZATION_PARMS Init;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_INITIALIZE, *PXTMCFGDRV_INITIALIZE;

typedef struct
{   XTM_CONFIGURATION_PARMS Config;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_CONFIGURE, *PXTMCFGDRV_CONFIGURE;

typedef struct
{   XTM_THRESHOLD_PARMS Threshold;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_THRESHOLD, *PXTMCFGDRV_THRESHOLD;

typedef struct
{
    BCM_IOC_PTR(PXTM_TRAFFIC_DESCR_PARM_ENTRY, pTrafficDescrTable);
    UINT32 ulTrafficDescrTableSize;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_TRAFFIC_DESCR_TABLE, *PXTMCFGDRV_TRAFFIC_DESCR_TABLE;

typedef struct
{
    UINT32 ulPortId;
    XTM_INTERFACE_CFG InterfaceCfg;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_INTERFACE_CFG, *PXTMCFGDRV_INTERFACE_CFG;

typedef struct
{
    XTM_ADDR ConnAddr;
    XTM_CONN_CFG ConnCfg;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_CONN_CFG, *PXTMCFGDRV_CONN_CFG;

typedef struct
{
    BCM_IOC_PTR(PXTM_ADDR, pConnAddrs);
    UINT32 ulNumConns;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_CONN_ADDRS, *PXTMCFGDRV_CONN_ADDRS;

typedef struct
{
    UINT32 ulPortId;
    XTM_INTERFACE_STATS Statistics;
    UINT32 ulReset;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_INTERFACE_STATISTICS, *PXTMCFGDRV_INTERFACE_STATISTICS;

typedef struct
{
    UINT32 ulPortId;
    XTM_INTERFACE_LINK_INFO LinkInfo;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_INTERFACE_LINK_INFO, *PXTMCFGDRV_INTERFACE_LINK_INFO;

typedef struct
{
    XTM_ADDR ConnAddr;
    XTM_OAM_CELL_INFO OamCellInfo;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_SEND_OAM_CELL, *PXTMCFGDRV_SEND_OAM_CELL;

typedef struct
{
    XTM_ADDR ConnAddr;
    char szNetworkDeviceName[NETWORK_DEVICE_NAME_SIZE];
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_CREATE_NETWORK_DEVICE, *PXTMCFGDRV_CREATE_NETWORK_DEVICE;

typedef struct
{
    UINT32 ulDeviation;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_PTMBONDINGCFG, *PXTMCFGDRV_PTMBONDINGCFG;


typedef struct
{
    XTM_ADDR ConnAddr;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_DELETE_NETWORK_DEVICE, *PXTMCFGDRV_DELETE_NETWORK_DEVICE;

typedef struct
{
    XTM_BOND_INFO info ;
    BCMXTM_STATUS bxStatus ;
} XTMCFGDRV_BONDING_INFO, *PXTMCFGDRV_BONDING_INFO;

typedef struct
{
    XTM_ERROR_STATS ErrorStatistics;
    BCMXTM_STATUS bxStatus;
} XTMCFGDRV_ERROR_STATISTICS, *PXTMCFGDRV_ERROR_STATISTICS;


#if defined(__cplusplus)
}
#endif

#endif // #if !defined(_XTMCFGDRV_H_)
