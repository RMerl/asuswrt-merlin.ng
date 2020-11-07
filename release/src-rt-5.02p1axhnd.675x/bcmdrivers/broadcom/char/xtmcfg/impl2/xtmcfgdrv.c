
/*
<:copyright-BRCM:2007:proprietary:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/

/***************************************************************************
 * File Name  : xtmcfgdrv.c (impl2)
 *
 * Description: This file contains Linux character device driver entry points
 *              for the bcmxtmcfg driver.
 ***************************************************************************/


/* Includes. */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <bcmtypes.h>
#include <xtmcfgdrv.h>
#include "bcm_OS_Deps.h"
#include "board.h"

/* Typedefs. */
typedef void (*FN_IOCTL) (uintptr_t arg);


/* Prototypes. */
static int __init bcmxtmcfg_init( void );
static void __exit bcmxtmcfg_cleanup( void );
static int bcmxtmcfg_open( struct inode *inode, struct file *filp );
int bcmxtmcfg_ioctl( struct inode *inode, struct file *flip,
    unsigned int command, uintptr_t arg );
static void DoInitialize( uintptr_t arg );
static void DoUninitialize( uintptr_t arg );
static void DoGetTrafficDescrTable( uintptr_t arg );
static void DoSetTrafficDescrTable( uintptr_t arg );
static void DoGetInterfaceCfg( uintptr_t arg );
static void DoSetInterfaceCfg( uintptr_t arg );
static void DoGetConnCfg( uintptr_t arg );
static void DoSetConnCfg( uintptr_t arg );
static void DoGetConnAddrs( uintptr_t arg );
static void DoGetInterfaceStatistics( uintptr_t arg );
static void DoSetInterfaceLinkInfo( uintptr_t arg );
static void DoSendOamCell( uintptr_t arg );
static void DoCreateNetworkDevice( uintptr_t arg );
static void DoDeleteNetworkDevice( uintptr_t arg );
static void DoGetBondingInfo( uintptr_t arg );
static void DoConfigure( uintptr_t arg );
static void DoGetErrorStatistics( uintptr_t arg );
static void DoManageThreshold( uintptr_t arg );

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static DEFINE_MUTEX(ioctlMutex);

static long bcmxtmcfg_unlocked_ioctl(struct file *filep,
    unsigned int cmd, unsigned long arg )
{
	struct inode *inode;
    long rt;

    inode = file_inode(filep);

    mutex_lock(&ioctlMutex);
    rt = bcmxtmcfg_ioctl( inode, filep, cmd, arg );
    mutex_unlock(&ioctlMutex);
    return rt;
}
#endif

/* Globals. */
static struct file_operations bcmxtmcfg_fops =
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
	.unlocked_ioctl = bcmxtmcfg_unlocked_ioctl,
#if defined(CONFIG_COMPAT)
	.compat_ioctl = bcmxtmcfg_unlocked_ioctl,
#endif
#else
    .ioctl  = bcmxtmcfg_ioctl,
#endif
    .open   = bcmxtmcfg_open,
};

/* Defined for backward compatibility.  Not used. */
void *g_pfnAdslSetAtmLoopbackMode = NULL;
void *g_pfnAdslSetVcEntry = NULL;
void *g_pfnAdslSetVcEntryEx = NULL;
void *g_pfnAdslGetObjValue = NULL;
void *g_pfnAdslSetObjValue = NULL;
void *g_pfnAdslWanDevState = NULL;

/***************************************************************************
 * Function Name: bcmxtmcfg_init
 * Description  : Initial function that is called at system startup that
 *                registers this device.
 * Returns      : None.
 ***************************************************************************/
static int __init bcmxtmcfg_init( void )
{
#if defined(CONFIG_BCM_55153_DPU)
    XTM_INITIALIZATION_PARMS initparms;
#endif

   if (!kerSysGetDslPhyEnable())
   {
       printk( "bcmxtmcfg is not enabled\n");
       return -1;
   }

#if defined(CONFIG_BCM_55153_DPU)
    memset(&initparms, 0, sizeof(initparms));
    initparms.bondConfig.sConfig.atmBond = BC_ATM_BONDING_DISABLE;
    initparms.bondConfig.sConfig.ptmBond = BC_PTM_BONDING_DISABLE;
    initparms.bondConfig.sConfig.autoSenseAtm = BC_ATM_AUTO_SENSE_DISABLE;
    BcmXtm_Initialize_DPU(&initparms);
#endif
    printk( "bcmxtmcfg: bcmxtmcfg_init entry\n" );
    register_chrdev( XTMCFGDRV_MAJOR, "bcmxtmcfg", &bcmxtmcfg_fops );
    return( 0 );
} /* bcmxtmcfg_init */


/***************************************************************************
 * Function Name: bcmxtmcfg_cleanup
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 ***************************************************************************/
static void __exit bcmxtmcfg_cleanup( void )
{
    printk( "bcmxtmcfg: bcmxtmcfg_cleanup entry\n" );
    unregister_chrdev( XTMCFGDRV_MAJOR, "bcmxtmcfg" );
    BcmXtm_Uninitialize();
} /* bcmxtmcfg_cleanup */


/***************************************************************************
 * Function Name: bcmxtmcfg_open
 * Description  : Called when an application opens this device.
 * Returns      : 0 - success
 ***************************************************************************/
static int bcmxtmcfg_open( struct inode *inode, struct file *filp )
{
    return( 0 );
} /* bcmxtmcfg_open */


/***************************************************************************
 * Function Name: bcmxtmcfg_ioctl
 * Description  : Main entry point for an application send issue ATM API
 *                requests.
 * Returns      : 0 - success or error
 ***************************************************************************/
int bcmxtmcfg_ioctl( struct inode *inode, struct file *flip,
    unsigned int command, uintptr_t arg )
{
    int ret = 0;
    unsigned int cmdnr = _IOC_NR(command);

    FN_IOCTL IoctlFuncs[] = {DoInitialize, DoUninitialize,
        DoGetTrafficDescrTable, DoSetTrafficDescrTable, DoGetInterfaceCfg,
        DoSetInterfaceCfg, DoGetConnCfg, DoSetConnCfg, DoGetConnAddrs,
        DoGetInterfaceStatistics, DoSetInterfaceLinkInfo, DoSendOamCell,
        DoCreateNetworkDevice, DoDeleteNetworkDevice, NULL, DoGetBondingInfo,
        NULL, DoConfigure, DoGetErrorStatistics, DoManageThreshold};

    if( cmdnr >= 0 && cmdnr < MAX_XTMCFGDRV_IOCTL_COMMANDS &&
        IoctlFuncs[cmdnr] != NULL )
    {
        (*IoctlFuncs[cmdnr]) (arg);
    }
    else
        ret = -EINVAL;

    return( ret );
} /* bcmxtmcfg_ioctl */


/***************************************************************************
 * Function Name: DoInitialize
 * Description  : Calls BcmXtm_Initialize on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoInitialize( uintptr_t arg )
{
    XTMCFGDRV_INITIALIZE KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_Initialize( &KArg.Init );
        put_user( KArg.bxStatus, &((PXTMCFGDRV_INITIALIZE) arg)->bxStatus );
    }
} /* DoInitialize */


/***************************************************************************
 * Function Name: DoUninitialize
 * Description  : Calls BcmXtm_Uninitialize on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoUninitialize( uintptr_t arg )
{
    BCMXTM_STATUS bxStatus = BcmXtm_Uninitialize();
    put_user( bxStatus, &((PXTMCFGDRV_STATUS_ONLY) arg)->bxStatus );
} /* DoUninitialize */


/***************************************************************************
 * Function Name: DoGetTrafficDescrTable
 * Description  : Calls BcmXtm_GetTrafficDescrTable on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetTrafficDescrTable( uintptr_t arg )
{
    XTMCFGDRV_TRAFFIC_DESCR_TABLE KArg;
    PXTM_TRAFFIC_DESCR_PARM_ENTRY pKTbl = NULL;
    UINT32 ulSize;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
#if defined(CONFIG_COMPAT)
        if(is_compat_task())
           BCM_IOC_PTR_ZERO_EXT(KArg.pTrafficDescrTable);
#endif	//CONFIG_COMPAT
        ulSize = KArg.ulTrafficDescrTableSize *
            sizeof(XTM_TRAFFIC_DESCR_PARM_ENTRY);
        if( ulSize )
            pKTbl = (PXTM_TRAFFIC_DESCR_PARM_ENTRY) kmalloc(ulSize, GFP_KERNEL);

        if( pKTbl || ulSize == 0 )
        {
            KArg.bxStatus = BcmXtm_GetTrafficDescrTable( pKTbl,
                &KArg.ulTrafficDescrTableSize );

            if( KArg.bxStatus == XTMSTS_SUCCESS )
                copy_to_user((void *) KArg.pTrafficDescrTable,pKTbl,ulSize);

            put_user( KArg.ulTrafficDescrTableSize,
              &((PXTMCFGDRV_TRAFFIC_DESCR_TABLE)arg)->ulTrafficDescrTableSize);

            put_user( KArg.bxStatus,
                &((PXTMCFGDRV_TRAFFIC_DESCR_TABLE) arg)->bxStatus );

            if( pKTbl )
                kfree( pKTbl );
        }
    }
} /* DoGetTrafficDescrTable */


/***************************************************************************
 * Function Name: DoSetTrafficDescrTable
 * Description  : Calls BcmXtm_SetTrafficDescrTable on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetTrafficDescrTable( uintptr_t arg )
{
    XTMCFGDRV_TRAFFIC_DESCR_TABLE KArg;
    PXTM_TRAFFIC_DESCR_PARM_ENTRY pKTbl;
    UINT32 ulSize;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        ulSize = KArg.ulTrafficDescrTableSize *
            sizeof(XTM_TRAFFIC_DESCR_PARM_ENTRY);
        pKTbl = (PXTM_TRAFFIC_DESCR_PARM_ENTRY) kmalloc( ulSize, GFP_KERNEL );

        if( pKTbl )
        {
            if( copy_from_user( pKTbl, KArg.pTrafficDescrTable, ulSize ) == 0 )
            {
                KArg.bxStatus = BcmXtm_SetTrafficDescrTable( pKTbl,
                    KArg.ulTrafficDescrTableSize );

                put_user( KArg.bxStatus,
                    &((PXTMCFGDRV_TRAFFIC_DESCR_TABLE) arg)->bxStatus );

            }

            kfree( pKTbl );
        }
    }
} /* DoSetTrafficDescrTable */


/***************************************************************************
 * Function Name: DoGetInterfaceCfg
 * Description  : Calls BcmXtm_GetInterfaceCfg on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetInterfaceCfg( uintptr_t arg )
{
    XTMCFGDRV_INTERFACE_CFG KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_GetInterfaceCfg( KArg.ulPortId,
            &KArg.InterfaceCfg );

        if( KArg.bxStatus == XTMSTS_SUCCESS )
        {
            copy_to_user( &((PXTMCFGDRV_INTERFACE_CFG) arg)->InterfaceCfg,
                &KArg.InterfaceCfg, sizeof(KArg.InterfaceCfg) );
        }

        put_user( KArg.bxStatus, &((PXTMCFGDRV_INTERFACE_CFG) arg)->bxStatus );
    }
} /* DoGetInterfaceCfg */


/***************************************************************************
 * Function Name: DoSetInterfaceCfg
 * Description  : Calls BcmXtm_SetInterfaceCfg on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetInterfaceCfg( uintptr_t arg )
{
    XTMCFGDRV_INTERFACE_CFG KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_SetInterfaceCfg( KArg.ulPortId,
            &KArg.InterfaceCfg );
        put_user( KArg.bxStatus, &((PXTMCFGDRV_INTERFACE_CFG) arg)->bxStatus );
    }
} /* DoSetInterfaceCfg */


/***************************************************************************
 * Function Name: DoGetConnCfg
 * Description  : Calls BcmXtm_GetConnCfg on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetConnCfg( uintptr_t arg )
{
    XTMCFGDRV_CONN_CFG KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_GetConnCfg( &KArg.ConnAddr, &KArg.ConnCfg );

        if( KArg.bxStatus == XTMSTS_SUCCESS )
        {
            copy_to_user( &((PXTMCFGDRV_CONN_CFG) arg)->ConnCfg,
                &KArg.ConnCfg, sizeof(KArg.ConnCfg) );
        }

        put_user( KArg.bxStatus, &((PXTMCFGDRV_CONN_CFG) arg)->bxStatus );
    }
} /* DoGetConnCfg */


/***************************************************************************
 * Function Name: DoSetConnCfg
 * Description  : Calls BcmXtm_SetConnCfg on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetConnCfg( uintptr_t arg )
{
    XTMCFGDRV_CONN_CFG KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        /* If an invalid AAL type field is set, delete the configuration.  The
         * caller passed a NULL pointer for the configuration record which means
         * to delete the configuration for the specified connection address.
         */
        if( KArg.ConnCfg.ulAtmAalType == XTMCFG_INVALID_FIELD )
        {
            KArg.bxStatus = BcmXtm_SetConnCfg( &KArg.ConnAddr, NULL );
            put_user( KArg.bxStatus, &((PXTMCFGDRV_CONN_CFG) arg)->bxStatus );
        }
        else
        {
            KArg.bxStatus = BcmXtm_SetConnCfg( &KArg.ConnAddr, &KArg.ConnCfg );
            put_user( KArg.bxStatus, &((PXTMCFGDRV_CONN_CFG) arg)->bxStatus );
        }
    }
} /* DoSetConnCfg */


/***************************************************************************
 * Function Name: DoGetConnAddrs
 * Description  : Calls BcmXtm_GetConnAddrs on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetConnAddrs( uintptr_t arg )
{
    XTMCFGDRV_CONN_ADDRS KArg;
    PXTM_ADDR pKAddrs = NULL;
    UINT32 ulSize;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
#if defined(CONFIG_COMPAT)
        if(is_compat_task())
           BCM_IOC_PTR_ZERO_EXT(KArg.pConnAddrs);
#endif	//CONFIG_COMPAT
        ulSize = KArg.ulNumConns * sizeof(XTM_ADDR);
        if( ulSize )
            pKAddrs = (PXTM_ADDR) kmalloc( ulSize, GFP_KERNEL );

        if( pKAddrs || ulSize == 0 )
        {
            KArg.bxStatus = BcmXtm_GetConnAddrs( pKAddrs, &KArg.ulNumConns );

            if( ulSize )
                copy_to_user( KArg.pConnAddrs, pKAddrs, ulSize );
            put_user( KArg.ulNumConns,
                &((PXTMCFGDRV_CONN_ADDRS) arg)->ulNumConns );

            put_user(KArg.bxStatus, &((PXTMCFGDRV_CONN_ADDRS) arg)->bxStatus);

            if( pKAddrs )
                kfree( pKAddrs );
        }
    }
} /* DoGetConnAddrs */


/***************************************************************************
 * Function Name: DoGetInterfaceStatistics
 * Description  : Calls BcmXtm_GetInterfaceStatistics on behalf of a user
 *                program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetInterfaceStatistics( uintptr_t arg )
{
    XTMCFGDRV_INTERFACE_STATISTICS KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_GetInterfaceStatistics( KArg.ulPortId,
            &KArg.Statistics, KArg.ulReset );

        if( KArg.bxStatus == XTMSTS_SUCCESS )
        {
            copy_to_user( &((PXTMCFGDRV_INTERFACE_STATISTICS) arg)->Statistics,
                &KArg.Statistics, sizeof(KArg.Statistics) );
        }

        put_user( KArg.bxStatus,
            &((PXTMCFGDRV_INTERFACE_STATISTICS)arg)->bxStatus );
    }
} /* DoGetInterfaceStatistics */


/***************************************************************************
 * Function Name: DoSetInterfaceLinkInfo
 * Description  : Calls BcmXtm_SetInterfaceLinkInfo on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetInterfaceLinkInfo( uintptr_t arg )
{
    XTMCFGDRV_INTERFACE_LINK_INFO KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_SetInterfaceLinkInfo( KArg.ulPortId,
            &KArg.LinkInfo );
        put_user( KArg.bxStatus,
            &((PXTMCFGDRV_INTERFACE_LINK_INFO) arg)->bxStatus );
    }
} /* DoSetInterfaceLinkInfo */


/***************************************************************************
 * Function Name: DoSendOamCell
 * Description  : Calls BcmXtm_SendOamCell on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSendOamCell( uintptr_t arg )
{
    XTMCFGDRV_SEND_OAM_CELL KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_SendOamCell(&KArg.ConnAddr, &KArg.OamCellInfo);
        copy_to_user( &((PXTMCFGDRV_SEND_OAM_CELL) arg)->OamCellInfo,
            &KArg.OamCellInfo, sizeof(KArg.OamCellInfo) );
        put_user( KArg.bxStatus, &((PXTMCFGDRV_SEND_OAM_CELL) arg)->bxStatus );
    }
} /* DoSendOamCell */


/***************************************************************************
 * Function Name: DoCreateNetworkDevice
 * Description  : Calls BcmXtm_CreateNetworkDevice on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoCreateNetworkDevice( uintptr_t arg )
{
    XTMCFGDRV_CREATE_NETWORK_DEVICE KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_CreateNetworkDevice( &KArg.ConnAddr,
            KArg.szNetworkDeviceName );
        put_user( KArg.bxStatus,
            &((PXTMCFGDRV_CREATE_NETWORK_DEVICE) arg)->bxStatus );
    }
} /* DoCreateNetworkDevice */


/***************************************************************************
 * Function Name: DoDeleteNetworkDevice
 * Description  : Calls BcmXtm_DeleteNetworkDevice on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoDeleteNetworkDevice( uintptr_t arg )
{
    XTMCFGDRV_DELETE_NETWORK_DEVICE KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_DeleteNetworkDevice( &KArg.ConnAddr );
        put_user( KArg.bxStatus,
            &((PXTMCFGDRV_DELETE_NETWORK_DEVICE) arg)->bxStatus );
    }
} /* DoDeleteNetworkDevice */


/***************************************************************************
 * Function Name: DoGetBondingInfo
 * Description  : Gets the information for bonding.
 *                For bonded pair, it returns the available aggregate/traffic
 *                mode and for non-bonded pair, it returns no information with
 *                the status set to Non-Success.
 * Returns      : None.
 ***************************************************************************/

static void DoGetBondingInfo ( uintptr_t arg )
{
    XTMCFGDRV_BONDING_INFO KArg ;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_GetBondingInfo(&KArg.info) ;
        copy_to_user( &((PXTMCFGDRV_BONDING_INFO) arg)->info, &KArg.info, sizeof(KArg.info) );
        put_user( KArg.bxStatus, &((PXTMCFGDRV_BONDING_INFO) arg)->bxStatus );
    }
} /* DoGetBondingInfo */


/***************************************************************************
 * Function Name: DoConfigure
 * Description  : Calls BcmXtm_Configure on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoConfigure( uintptr_t arg )
{
    XTMCFGDRV_CONFIGURE KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_Configure(&KArg.Config) ;
        put_user( KArg.bxStatus, &((PXTMCFGDRV_CONFIGURE) arg)->bxStatus );
    }
} /* DoConfigure */


/***************************************************************************
 * Function Name: DoManageThreshold
 * Description  : Calls BcmXtm_ManageThreshold on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoManageThreshold( uintptr_t arg )
{
    XTMCFGDRV_THRESHOLD KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_ManageThreshold (&KArg.Threshold) ;
        put_user( KArg.bxStatus, &((PXTMCFGDRV_THRESHOLD) arg)->bxStatus );
    }

} /* DoManageThreshold */


/***************************************************************************
 * Function Name: DoGetErrorStatistics
 * Description  : Calls BcmXtm_GetErrorStatistics on behalf of a user
 *                program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetErrorStatistics( uintptr_t arg )
{
    XTMCFGDRV_ERROR_STATISTICS KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bxStatus = BcmXtm_GetErrorStatistics( &KArg.ErrorStatistics);

        if( KArg.bxStatus == XTMSTS_SUCCESS )
        {
            copy_to_user( &((PXTMCFGDRV_ERROR_STATISTICS) arg)->ErrorStatistics,
                &KArg.ErrorStatistics, sizeof(KArg.ErrorStatistics) );
        }

        put_user( KArg.bxStatus,
            &((PXTMCFGDRV_ERROR_STATISTICS)arg)->bxStatus );
    }
} /* DoGetInterfaceStatistics */

/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init( bcmxtmcfg_init );
module_exit( bcmxtmcfg_cleanup );
MODULE_LICENSE("Proprietary");

EXPORT_SYMBOL(BcmXtm_Initialize);
EXPORT_SYMBOL(BcmXtm_Uninitialize);
EXPORT_SYMBOL(BcmXtm_GetTrafficDescrTable);
EXPORT_SYMBOL(BcmXtm_SetTrafficDescrTable);
EXPORT_SYMBOL(BcmXtm_GetInterfaceCfg);
EXPORT_SYMBOL(BcmXtm_SetInterfaceCfg);
EXPORT_SYMBOL(BcmXtm_GetConnCfg);
EXPORT_SYMBOL(BcmXtm_SetConnCfg);
EXPORT_SYMBOL(BcmXtm_GetConnAddrs);
EXPORT_SYMBOL(BcmXtm_GetInterfaceStatistics);
EXPORT_SYMBOL(BcmXtm_SetInterfaceLinkInfo);
EXPORT_SYMBOL(BcmXtm_SendOamCell);
EXPORT_SYMBOL(BcmXtm_CreateNetworkDevice);
EXPORT_SYMBOL(BcmXtm_DeleteNetworkDevice);
EXPORT_SYMBOL(BcmXtm_GetBondingInfo);

/* Backward compatibility. */
BCMXTM_STATUS BcmAtm_SetInterfaceLinkInfo( UINT32 ulInterfaceId,
    void *pInterfaceLinkInfo );
BCMXTM_STATUS BcmAtm_GetInterfaceId( UINT8 ucPhyPort, uintptr_t pulInterfaceId );

EXPORT_SYMBOL(BcmAtm_SetInterfaceLinkInfo);
EXPORT_SYMBOL(BcmAtm_GetInterfaceId);
EXPORT_SYMBOL(g_pfnAdslSetAtmLoopbackMode);
EXPORT_SYMBOL(g_pfnAdslSetVcEntry);
EXPORT_SYMBOL(g_pfnAdslSetVcEntryEx);
EXPORT_SYMBOL(g_pfnAdslGetObjValue);
EXPORT_SYMBOL(g_pfnAdslSetObjValue);
EXPORT_SYMBOL(g_pfnAdslWanDevState);
