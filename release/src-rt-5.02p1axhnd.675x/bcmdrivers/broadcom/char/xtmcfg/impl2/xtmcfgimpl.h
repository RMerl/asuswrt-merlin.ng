/*
    <:copyright-BRCM:2011:proprietary:standard
    
       Copyright (c) 2011 Broadcom 
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
 * File Name  : xtmcfgimpl.h (impl2)
 *
 * Description: This file contains class definitions for the XTM configuration
 *              driver implementation.
 ***************************************************************************/

#if !defined(_XTMCFGIMPL_H_)
#define _XTMCFGIMPL_H_

/* The following definitions are defined here because the XTMCFG source files
 * do not include standard operating system header files.
 */
typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned int    UINT32;
typedef unsigned char   BOOL;

#define NULL            0
#define TRUE            1
#define FALSE           0

#include "bcmxtmcfg.h"
#include "bcmxtmrt.h"
#include "xtmprocregs_impl2.h"

#define XTM_EVT_TRAFFIC_TYPE_MISMATCH              1   /* Also duplicated in xtmoslinux.c */
#define XTM_EVT_TRAFFIC_TYPE_MISMATCH_AND_RESTART  2   /* Also duplicated in xtmoslinux.c */
#define XTM_EVT_BONDING_GROUP_ID_MISMATCH          3   /* Also duplicated in xtmoslinux.c */

/* For the 63138 and 63148, implement a workaround to strip bytes and
   allow OAM traffic due to JIRA HW63138-12 */
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
#define XTM_OAM_STRIP_BYTE_WORKAROUND_ENABLED      3   /* Also duplicated in xtmoslinux.c */
#define XTM_OAM_STRIP_BYTE_WORKAROUND_DISABLED     4   /* Also duplicated in xtmoslinux.c */
#endif

#define XTM_WAIT_TRIAL_DATA_COUNT_SYNC    5
#define XTM_WAIT_TRIAL_DATA_BUSY          0xfffff

extern "C" {
extern int memset( void *, int, unsigned int );
extern void *memcpy( void *, const void *, unsigned int );
extern int pktCmfSarConfig( int ulHwFwdTxChannel, unsigned int ulTrafficType );
extern int pktCmfSarAbort(void);
extern int pktCmfDslDiags(unsigned long fwdRxTx);

/* Prototypes for OS specific functions. */
void XtmOsInitialize( void );
char *XtmOsAlloc( UINT32 ulSize );
void XtmOsFree( char *pBuf );
void *XtmOsCreateSem( UINT32 ulInitialState );
void XtmOsDeleteSem( unsigned long ulSem );
void XtmOsRequestSemTo( unsigned long ulSem );
UINT32 XtmOsRequestSem( unsigned long ulSem, UINT32 ulTimeout );
void XtmOsReleaseSem( unsigned long ulSem );
void *XtmOsCreateLock(void);
void XtmOsDeleteLock(unsigned long pLock);
void XtmOsRequestLock(unsigned long pLock);
void XtmOsReleaseLock(unsigned long pLock);


UINT32 XtmOsDisableInts( void );
void XtmOsEnableInts( unsigned long flags );
void XtmOsDelay( UINT32 ulTimeout );
UINT32 XtmOsTickGet( void );
UINT32 XtmOsTickCheck( unsigned long ulWaitBase, unsigned long ulMsToWait );
void XtmOsDeferredTo( unsigned long ulHandle );
UINT32 XtmOsInitDeferredHandler( void *pFnEntry, UINT32 ulFnParm,
    UINT32 ulTimeout );
void XtmOsScheduleDeferred( UINT32 ulHandle );
void XtmOsUninitDeferredHandler( unsigned long ulHandle );
UINT32 XtmOsStartTimer( void *pFnEntry, unsigned long ulFnParm, UINT32 ulTimeout );
UINT32 XtmOsStopTimer( void *pFnEntry );
void DummyXtmOsPrintf( const char *pFmt, ... );
void XtmOsPrintf( const char *pFmt, ... );
UINT32 XtmOsChipRev( void );
void XtmOsGetTimeOfDay(UINT8 *vp, void *dummy);
void XtmOsAddTimeOfDay(UINT8 *vp, UINT32 sec) ;
UINT32 XtmOsGetTimeStamp(void);
UINT32 XtmOsGetTimeStampMs(void);
UINT32 XtmOsTimeInUsecs (UINT8 *vp);
BOOL XtmOsIsTimerGreater(UINT8 *ap, UINT8 *bp);
void XtmOsSendSysEvent (int EventId);
}


/* MIPS Processor specific defintions. */
#define CACHE_TO_NONCACHE(x)        ((unsigned)(x)|0xA0000000)
#define NONCACHE_TO_CACHE(x)        ((unsigned)(x)&0x9FFFFFFF)
#define CACHE_TO_PHYS(x)            ((unsigned)(x)&0x1FFFFFFF)
#define NONCACHE_TO_PHYS(x)         ((unsigned)(x)&0x1FFFFFFF)
#define PHYS_TO_CACHE(x)            ((unsigned)(x)|0x80000000)
#define PHYS_TO_NONCACHE(x)         ((unsigned)(x)|0xA0000000)

/* Limit definitions. */
#define MAX_INTERFACES              XP_MAX_PORTS
#define MAX_CONNS                   XP_MAX_CONNS
#define MAX_SHAPERS                 XP_MAX_CONNS
#define MAX_PTM_PRIORITIES          2
#define MAX_CONN_TABLE_ENTRIES      (XP_MAX_CONNS * 4) /* arbitrary number */
#define MAX_TIMEOUT                 ((UINT32) -1)

/* Shaping definitions. */
#define MCR_PRIORITY                7
#define CBR_PRIORITY                6
#define RTVBR_PRIORITY              5
#define MBR_ACCUMULATOR             3000
#define PTM_TOKEN_BUCKET_INCREMENT  1600  /* bytes */

/* Shaper type */
#define QUEUE_SHAPER                0
#define MPAAL_SHAPER                1

/* Interrupt mask.  Define bits to be handled by this driver. */
#define INTR_MASK                   0

/* Miscellaneous definions. */
#define INVALID_INDEX               0xffffffff
#define INVALID_HANDLE              0
#define INVALID_VCID                0xff
#define ANY_VCID                    0xfe
#define TEQ_DATA_VCID               15

/* RFC2684 headers as 32 bit words. (length, hdr_word0, hdr_word1, ...).
 * Order of headers must match HT_... definitions in bcmxtmcfg.h.
 */
#define PKT_HDRS_START_IDX          1
#define PKT_HDRS_END                0xffffffff
#define PKT_HDRS                                                            \
    {{ 0, 0},                                  /* HT_NONE */                \
     {10, 0xaaaa0300, 0x80c20007, 0x00000000}, /* HT_LLC_SNAP_ETHERNET */   \
     { 8, 0xaaaa0300, 0x00000800},             /* HT_LLC_SNAP_ROUTE_IP */   \
     { 4, 0xfefe03cf},                         /* HT_LLC_ENCAPS_PPP */      \
     { 2, 0x00000000},                         /* HT_VC_MUX_ETHERNET */     \
     { PKT_HDRS_END}}

#define XTM_ADDR_CMP(A,B)                                   \
    (((*((UINT32 *) (A)+0) == *((UINT32 *) (B)+0)) &&       \
      (*((UINT32 *) (A)+1) == *((UINT32 *) (B)+1)) &&       \
      (*((UINT32 *) (A)+2) == *((UINT32 *) (B)+2))) ? 1 : 0)

#define ADSL_SIT_UNIT_OF_TIME       500 /* SIT value is 5 */
#define CALC_SIT_VALUE(SIT,SCALER)  (m_ulBusSpeed * (SIT) * (SCALER)) / 10000


extern "C" {
inline char *strcpy(char *__dest, __const__ char *__src)
{
  char *__xdest = __dest;

  __asm__ __volatile__(
    ".set\tnoreorder\n\t"
    ".set\tnoat\n"
    "1:\tlbu\t$1,(%1)\n\t"
    "addiu\t%1,1\n\t"
    "sb\t$1,(%0)\n\t"
    "bnez\t$1,1b\n\t"
    "addiu\t%0,1\n\t"
    ".set\tat\n\t"
    ".set\treorder"
    : "=r" (__dest), "=r" (__src)
        : "0" (__dest), "1" (__src)
    : "memory");

  return __xdest;
}
}

extern "C" {
inline int strcmp(__const__ char *__s1, __const__ char *__s2)
{
    while(*__s1 && (*__s1==*__s2))
        __s1++,__s2++;
    return *(const unsigned char*)__s1-*(const unsigned char*)__s2;
}
}

#define XTM_NO_CONFIGURE         0
#define XTM_CONFIGURE            1

#if defined(CONFIG_BCM963158)

#if (CONFIG_BRCM_CHIP_REV!=0x63158A0)

#define XTMCFG_LINE_RATE_TIMER     0xf
#define XTMCFG_SST_SIT_VALUE       0x5
#define XTMCFG_UTO_PCR_INCR_SHIFT  0xA
#define XTMCFG_UTO_PCR_ACCLMT      0x1
#define XTMCFG_UTO_PCR_SITLMT      0x1

#else

#define XTMCFG_LINE_RATE_TIMER     0xf
#define XTMCFG_SST_SIT_VALUE       0xA
#define XTMCFG_UTO_PCR_INCR_SHIFT  0xA
#define XTMCFG_UTO_PCR_ACCLMT      0x1
#define XTMCFG_UTO_PCR_SITLMT      0x2

#endif
#endif

class XTM_CONNECTION_TABLE;
class XTM_PROCESSOR;

class XTM_CONNECTION
{
public:
    XTM_CONNECTION(FN_XTMRT_REQ pfnXtmrtReq, XTM_CONNECTION_TABLE *pConnTable,
        UINT32 ulConnSem, UINT32 *pulRxVpiVciCamShadow, XtmBondConfig bondConfig)
        { Cxtor(pfnXtmrtReq, pConnTable, ulConnSem, pulRxVpiVciCamShadow, bondConfig); }
    ~XTM_CONNECTION( void ) { Dxtor(); }

    int Cxtor(FN_XTMRT_REQ pfnXtmrtReq, XTM_CONNECTION_TABLE *pConnTable,
        UINT32 ulConnSem, UINT32 *pulRxVpiVciCamShadow, XtmBondConfig bondConfig);
    int Dxtor( void );

    void Initialize(FN_XTMRT_REQ pfnXtmrtReq, XTM_CONNECTION_TABLE *pConnTable, void *ulConnSem, UINT32 *pulRxVpiVciCamShadow, XtmBondConfig bondConfig);
    BCMXTM_STATUS GetAddr( PXTM_ADDR pAddr );
    BCMXTM_STATUS SetAddr( PXTM_ADDR pAddr );
    BCMXTM_STATUS GetCfg( PXTM_CONN_CFG pCfg );
    BCMXTM_STATUS SetCfg( PXTM_CONN_CFG pCfg, UINT32 ulXTMLinkMode, UINT32 ulXTMNitroMode );
    BCMXTM_STATUS SetTdt( PXTM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrEntries,
        UINT32 ulNumTrafficDescrEntries );
    BCMXTM_STATUS SetLinkInfo( UINT32 ulLinkMask, PXTM_INTERFACE_LINK_INFO pLinkInfo,
                               UINT32 ulBondingPort,
                               PXTM_INTERFACE_LINK_INFO pOtherLinkInfo,
                               UINT32 ulXTMLinkMode, UINT32 ulXTMNitroMode);
    BCMXTM_STATUS CreateNetworkDevice( char *pszNetDeviceName, UINT32 ulTxPafEnabled );
    BCMXTM_STATUS DeleteNetworkDevice( void );

    XTMRT_HANDLE GetHandle( void )
        { return( m_hDev ); }

    UINT32 Created( void )
        { return( m_ulCreated ); }

    UINT32 Connected( void )
        { return( m_ulConnected ); }

    UINT32 RemovePending( void )
        { return( m_ulRemovePending ); }

    UINT32 UpdatePending( void )
        { return( m_ulUpdatePending ); }

    static void SetSitUt( UINT32 ulSitUt, UINT32 ulSitLoUt )
        { ms_ulSitUt = ulSitUt;  ms_ulSitLoUt = ulSitLoUt; }

private:
    FN_XTMRT_REQ m_pfnXtmrtReq;
    XTM_CONNECTION_TABLE *m_pConnTable;
#if defined(CONFIG_ARM64)
    unsigned long m_ulConnSem;
#else
    UINT32 m_ulConnSem;
#endif
    XTM_ADDR m_Addr;
    XTM_CONN_CFG m_Cfg;
    PXTM_TRAFFIC_DESCR_PARM_ENTRY m_pTrafficDescrEntries;
    UINT32 m_ulNumTrafficDescrEntries;
    char m_szNetDeviceName[NETWORK_DEVICE_NAME_SIZE];
    XTMRT_HANDLE m_hDev;
    UINT32 m_ulHwAddHdr;
    UINT32 m_ulHwRemoveHdr;
    UINT32 m_ulCreated;
    UINT32 m_ulConnected;
    UINT32 m_ulRemovePending;
    UINT32 m_ulUpdatePending;
    UINT32 m_ulVcidsSize;
    UINT32 m_ulXTMLinkMode ;
    UINT32 m_ulXTMNitroMode ;
    UINT8 m_ucVcids[MAX_INTERFACES][MAX_PTM_PRIORITIES];
    UINT32 *m_pulRxVpiVciCamShadow;
    UINT32 m_ulTxQIdsSize;
    XtmBondConfig m_BondConfig ;
    /* m_TxQIds stores all the tx queues that had been allocated to this connection */
    XTMRT_TRANSMIT_QUEUE_ID m_TxQIds[MAX_TRANSMIT_QUEUES];
    struct VcidInfo
    {
        /* Only ulQueueIdx or ulMpIdx is valid.  The other field is set to 
         * INVALID_INDEX.
         */
        UINT32 ulPort;
        UINT32 ulQueueIdx;
        UINT32 ulMpIdx;
        UINT32 ulMpQueueCount;
        PXTM_CONN_ARB pConnArb;
        UINT32 ulBondingPort;
        UINT32 ulPtmPri ;
    } m_VcidInfo[MAX_VCIDS];
    static UINT32 ms_ulTxQueuesInUse;
    static UINT32 ms_ulSitUt;
    static UINT32 ms_ulSitLoUt;

    BCMXTM_STATUS ReserveVcidEntries( void );
    BCMXTM_STATUS ReserveAtmVcidEntry( UINT32 ulPort );
    BCMXTM_STATUS ReservePtmVcidEntry( UINT32 ulPort, UINT32 ulPtmPri, UINT32 ulBondingPort );
    BCMXTM_STATUS ReserveRawVcidEntry( UINT32 ulPort );
    BCMXTM_STATUS ReserveOneMpGroup(VcidInfo *pInfo);
    BCMXTM_STATUS ReserveOneTxQueue( PXTM_TRANSMIT_QUEUE_PARMS pTxQParms );
    BCMXTM_STATUS UnreserveOneTxQueue( PXTMRT_TRANSMIT_QUEUE_ID pTxQId );
    BCMXTM_STATUS ConfigureAtmShaper( UINT32 ulShaperType, UINT32 ulShaperIdx );
    UINT16 CalculateMbs( UINT32 ulMinBitRate, UINT16 usMbs );
    BCMXTM_STATUS ConfigurePtmQueueShaper( UINT32 ulShaperIdx, UINT32 ulMinBitRate,
                                           UINT32 ulShapeRate, UINT16 usMbs );
    BCMXTM_STATUS UnconfigureShaper( UINT32 ulShaperType, UINT32 ulShaperIdx );
    void ConfigureArbitration( UINT32 ulTxQueueIdx, UINT32 ulMpIdx, UINT32 ulPort,
                               UINT32 ulWeightAlg, UINT32 ulWeightValue,
                               UINT32 ulSubPriority );
    BCMXTM_STATUS UnreserveVcidsAndTxShapers( void );
    void WaitForTxComplete( UINT32 ulQueueIdx);
    void CheckTransmitQueues( void );
    UINT32 GetTxQSize ( void );
    PXTM_TRAFFIC_DESCR_PARM_ENTRY GetTdtEntry( UINT32 ulTdtIdx );
    UINT32 GetVcid( UINT32 ulPortId, UINT32 ulPtmPriority );
    UINT32 CopyVcids( UINT8 *pucVcids );
};


class XTM_CONNECTION_TABLE
{
public:
    UINT32 m_ulAdslThreshold ;
    UINT32 m_ulVdslThreshold ;
    UINT32 m_ulVdslRtxThreshold ;
    UINT32 m_ulGfastThreshold ;

    XTM_CONNECTION_TABLE( void );
    ~XTM_CONNECTION_TABLE( void );

    void Initialize( XTM_CONNECTION_TABLE *pTable );
    BCMXTM_STATUS Add( XTM_CONNECTION *pConn );
    void Remove( XTM_CONNECTION *pConn );
    XTM_CONNECTION *Get( PXTM_ADDR pAddr );
    XTM_CONNECTION *GetForPortId( PXTM_ADDR pAddr );
    XTM_CONNECTION *Enum( UINT32 *pulIdx );
    void setThreshold( UINT32 mode, UINT32 ulThreshold);
    UINT32 getThreshold( UINT32 mode );

private:
    XTM_CONNECTION *m_pConnTable[MAX_CONN_TABLE_ENTRIES];
#if defined(CONFIG_ARM64)
    unsigned long m_ulMuSem;
#else
    UINT32 m_ulMuSem;
#endif
};


typedef struct _IfMonitorErrorCounters {
   UINT32 rx_loss_of_sync ;
   UINT32 rx_SES_count_change ;
   UINT32 tx_SES_count_change ;
   UINT32 tx_LOS_change ;
   UINT32 rx_uncorrected ;
} IfMonitorErrorCounters ;

typedef struct XtmInterfaceLinkDelay
{
    UINT32 ulLinkUsDelay ;
    UINT32 ulLinkDsBondingDelay ;
} XTM_INTERFACE_LINK_DELAY, *PXTM_INTERFACE_LINK_DELAY;

#define XTM_NO_FLUSH     1
#define XTM_UNDO_FLUSH   2
#define XTM_FLUSH        3

#define XTM_PORT_SHAPING_RATIO_FULL 100

class XTM_INTERFACE
{
public:
    XTM_INTERFACE( void );
    ~XTM_INTERFACE( void );

    void PreInit ( void ) ;
    BCMXTM_STATUS Initialize( UINT32 ulPort, UINT32 ulInternalPort, UINT32 ulBondingPort,
                              UINT32 autoSenseATM, FN_XTMRT_REQ pfnXtmrtReq);
    BCMXTM_STATUS Uninitialize( void );
    BCMXTM_STATUS ReInitialize( UINT32 ulBondingPort );
    BCMXTM_STATUS GetCfg(PXTM_INTERFACE_CFG pCfg,XTM_CONNECTION_TABLE *pConnTbl);
    BCMXTM_STATUS SetCfg( PXTM_INTERFACE_CFG pCfg );
    BCMXTM_STATUS GetStats( PXTM_INTERFACE_STATS pStats, UINT32 ulReset );
    void UpdateLinkDelay (void) ;
    BCMXTM_STATUS SetLinkInfo( PXTM_INTERFACE_LINK_INFO pLinkInfo, UINT8 rxConfig, UINT8 linkInfoConfig,
                               UINT8 txConfig, UINT32 ulXTMLinkMode );
    BCMXTM_STATUS UpdateLinkInfo( UINT32 ulTrafficType );

    BCMXTM_STATUS GetLinkErrorStatus (UINT32 *pulUsStatus, UINT32 *pulDsStats) ;
    void SetPortDataStatus (UINT32 status, UINT32 flush) ;

    UINT8 GetPortNum( void )
        { return( m_ulPhysPort ); }

    UINT8 GetBondingPortNum( void )
        { return( m_ulPhysBondingPort ); }

    PXTM_INTERFACE_LINK_INFO GetLinkInfo( void )
        { return( &m_LinkInfo ); }

    UINT32 IsInterfaceUp( void )
        { return( (m_Cfg.ulIfAdminStatus == ADMSTS_UP) ? 1 : 0 ); }

    UINT32 GetPortDataStatus ()
        { return (m_ulDataStatus); }

    PXTM_INTERFACE_LINK_DELAY GetLinkDelay( void )
        { return( &m_LinkDelay ); }
#if defined(XTM_PORT_SHAPING)
    void SetSitUt( UINT32 ulSitUt, UINT32 ulSitLoUt )
        { ms_ulSitUt = ulSitUt;  ms_ulSitLoUt = ulSitLoUt; }
    void ConfigureShapingRatio( UINT32 ratio )
        { m_ulPortShapingRatio = ratio; }
    UINT32 ConfigureTxPortShaping (UINT32 ulTrafficType) ;
    void DisableTxPortShaping();
    void ResetHWTxPortShaping (void) ;
    BCMXTM_STATUS ConfigureMaxUTPortShaping ( UINT32 ulTrafficType );
#endif

private:
    UINT32 m_ulPhysPort;
    UINT32 m_ulInternalPort;
    UINT32 m_ulPhysBondingPort;
    UINT32 m_ulDataStatus ;

    /* In 63268, ptm rx packet count is NOT read from register ulRxPortPktCnt[].
     * It is read from register ulRxPafPktCount[] which is always clear after read.
     * Therefore, we need m_ulIfInPacketsPtm to save the accumulative count for
     * non-reset stats show, i.e., xtm intf --stats 1
     */
    UINT32 m_ulIfInPacketsPtm;

    UINT32 m_ulAutoSenseATM ;
    FN_XTMRT_REQ m_pfnXtmrtReq;
    
    XTM_INTERFACE_CFG m_Cfg;
    XTM_INTERFACE_LINK_INFO m_LinkInfo;
    XTM_INTERFACE_LINK_DELAY m_LinkDelay;
    XTM_INTERFACE_STATS m_Stats;

    IfMonitorErrorCounters m_PrevIfMonitorInfo ;
    UINT32 m_ulErrTickCount ;
    UINT32  m_ulUsStatus ;
    UINT32  m_ulDsStatus ;
    UINT32  m_ulXTMLinkMode;
#if defined(XTM_PORT_SHAPING)
    UINT32  ms_ulSitUt;
    UINT32  ms_ulSitLoUt;
    UINT32  m_ulEnableShaping;
    UINT32  m_ulShapeRate;
    UINT16  m_usMbs;
    UINT32  m_ulPortShapingRatio;
#endif
    BCMXTM_STATUS SetRxUtopiaLinkInfo( UINT32 ulLinkState );
    bool    m_bInitialized;
};


class XTM_OAM_HANDLER
{
public:
    XTM_OAM_HANDLER( );
    ~XTM_OAM_HANDLER( void );

    BCMXTM_STATUS Initialize( FN_XTMRT_REQ pfnXtmrtReq, XTM_PROCESSOR *pXtmProcessor );
    BCMXTM_STATUS Uninitialize( void );
    BCMXTM_STATUS SendCell( PXTM_ADDR pConnAddr, PXTM_ADDR pInternalConnAddr,
        PXTM_OAM_CELL_INFO pOamCellInfo, XTM_CONNECTION_TABLE *pConnTable );
    static int TimerUpdate(void);          /* Perform periodic tasks for OAM handler. */

private:
    FN_XTMRT_REQ m_pfnXtmrtReq;
    XTM_ADDR m_PendingOamReqs[16];
#if defined(CONFIG_ARM64)
    UINT32 m_ulRspTimes[16];
#else
    unsigned long m_ulRspTimes[16];
#endif
    UINT32 m_ulRegistered;
    static XTM_PROCESSOR   *m_pXtmProcessor;  /* Pointer to parent XTM_PROCESSOR object */

    static int ReceiveOamCb( XTMRT_HANDLE hDev, UINT32 ulCommand, void *pParam,
        void *pContext );

    /* For the 63138 and 63148, implement a workaround to strip bytes and
       allow OAM traffic due to JIRA HW63138-12 */
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
    static bool m_bStripByteWorkaroundActive;
    static UINT32 m_pulSavedRxVcamRxRamReg[XP_MAX_CONNS];
    static UINT32 m_ulStripByteWorkaroundTimeStart;

#define XTM_STRIPBYTEWORKAROUND_TIMEOUT_MS     10000 /* 10 second timeout for workaround */

    static int EnableStripByteWorkaround(void);
    static int DisableStripByteWorkaround(void);
#endif
};

/* Definitions corresponding to PTM/ATM Bonding ASM (Autonomous State Message)
 * protocol implementation. G.998.1
 */
#define XTM_MAJ_VER           24
#define XTM_MIN_VER           1
#define XTM_BUILD_VER         0

#define MAX_BONDED_LINES      2
#define MAX_TIME_INTERVAL     1000   /* max time step for the virtual time */
#define TIME_BETWEEN_CPE_POLL 500000
#define XTM_BOND_DSL_MONITOR_TIMEOUT_MS 10
#define XTM_BOND_TRAFFIC_MONITOR_TIMEOUT_MS (2000*XTM_BOND_DSL_MONITOR_TIMEOUT_MS)
#define XTM_BOND_RX_PAF_LOCKUP_MONITOR_TIMEOUT_MS (200*XTM_BOND_DSL_MONITOR_TIMEOUT_MS)
#define XTM_BOND_DSL_ERR_DURATION_TIMEOUT_MS (100*XTM_BOND_DSL_MONITOR_TIMEOUT_MS) /* multiple of dsl_monitor_timeout */
#define XTM_BOND_DSL_PEND_EVT_DURATION_TIMEOUT_MS (400*XTM_BOND_DSL_MONITOR_TIMEOUT_MS) /* multiple of dsl_monitor_timeout */
#define XTM_BOND_MGMT_TIMEOUT_MS 10
#define ASM_DOWN_TIMEOUT      (2*TIME_BETWEEN_CPE_POLL*MAX_BONDED_LINES) /* in usec */

#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)<(b) ? (b) : (a))

#define LK_ST_SHOULD_NOT_USE   1
#define LK_ST_ACCEPTABLE       2
#define LK_ST_SELECTED         3
#define LK_ST_MASK           0x3

#define BND_IDLE        DATA_STATUS_DISABLED  /* complete mapping is not yet known -- we should not send
                            * any asm */
#define BND_RUNNING     DATA_STATUS_ENABLED
#define BND_FORCE_RESET DATA_STATUS_RESET

#define ATM_HEADER_BYTES  4
#define ATM_CELL_SIZE     52     /* HEC is never seen by Linux Mips SW */

typedef struct LineInfo {
   int        inUseTx; /* means we should send traffic over the line */
   int        inUseRx; /* means we can rx traffic over the line */
   /* ASM info  (use a global prepared ASM) */
   int        asmReady; /* indicate an asm should be sent on this line  */
   UINT32     lastAsmElapsed; /* elapsed time since asmReady has been set */
   UINT32     usecTxAsm; /* number of usec to send a asm => reflect asm frequency
                            it is about 100 times the usecTx
                          */
   int        localRxState; /* local RX line status */
   int        localTxState; /* local TX line status */
   UINT32     groupId;      /* Group Id in the received ASM cell on this line */
} LineInfo ;


typedef struct _Asm {

   UINT8  msgType; /* 00 = 12 bit SID format, 01 = 8 bit SID format */
   UINT8  asmId;
   UINT8  txLinkId;/* 1 bit insufficient buffer - 4 LSB link id */
   UINT8  txLinkNumber; 
   UINT16 rxLinkStatus[4];
   UINT16 txLinkStatus[4];
   UINT16 groupId;
   UINT8  rxAsmStatus[4];
   UINT8  groupLostCell;
   UINT8  reservedA;
   UINT32 timestamp;
   UINT16 delay;
   UINT16 appliedDelay;
   UINT16 reservedC[3];
   UINT16 msgLen; /* should be 0x2800 */
   UINT32 crc;

} Asm ;

typedef struct _LineMapping
{
   int           localIdToRemoteId;
   UINT16        requestedDelay; /* delay to introduce on this line in 100 usecs */
   UINT32        lastAsmTimestamp;
   UINT32        receivedUnKnowns;
   UINT32        receivedAsm;
   UINT32        transmittedAsm;
   UINT32        stateChanges;
   UINT32        localStateChanges;
} LineMapping ;

#define TIMEOUT_VALUE_SIZE       16    /* Similar to struct timeval */
typedef struct _XtmBondMgmt {
   UINT32     printAsm ;
   UINT32     knownMapping; /* indicates which are the known mapping */
   int        remoteIdToLocal[MAX_BONDED_LINES]; /* -1 means it is unknown */
   LineMapping  mapping[MAX_BONDED_LINES];
   LineInfo   lines[MAX_BONDED_LINES];
   UINT32     bondingId; /* 0 upon init */
   UINT32     sidType ;
   UINT8      asmLastRxId;
   UINT8      resetState;
   int        numberOfLines; /* set as soon an ASM is received */
   UINT16     remoteTxState;
   UINT16     remoteRxState;
   UINT8      resetTimeout [TIMEOUT_VALUE_SIZE];
   Asm        asmCell ;       /* contains the ASM information to be sent to remote side */

   /* leaky bucket for CPE state ?? */
   int        nextLineToPoll; /* perform a polling every 150 msec - line -1 =>
                               * broadcast.
                               */
   UINT32     timeSinceLastPoll; /* usec */

   UINT32     timeInterval;    /* MIN between minTransmit time and x usec */

   UINT32     vt; /* virtual time */

   int        rxCell;

   /* measure differential delay between link */
   int        refLink; /* link that is known as the reference link to measure the
                        * end-to-end differential delay */
   int        lastPropagationRefDelay; /* last propoagation delay measured on the ref
                                        * link */
   int        refDiffDelay; /* fixed differential delay on the ref link */
} XtmBondMgmt ;

#define BYTESWAP32(a) \
(((UINT32)(a)<<24) | (((UINT32)(a)<<8)&0xff0000) | (((UINT32)(a)>>8)&0xff00) | ((UINT32)(a)>>24))
#define BYTESWAP16(a) \
(((UINT16)(a)<<8) | ((UINT16)(a)>>8))

#define TO_FROM_LE_32(a)  (BYTESWAP32(a))

#define XTM_BONDING_FALLBACK_DISABLED  0
#define XTM_BONDING_FALLBACK_ENABLED   1

// Allow XTM_ASM_HANDLER to access the parent XTM_PROCESSOR's XTM_AUTOSENSE object
class XTM_AUTOSENSE;

class XTM_ASM_HANDLER
{
   public:
      XTM_ASM_HANDLER( void );
      ~XTM_ASM_HANDLER( void );
                void initializeVars(void);

      BCMXTM_STATUS Initialize( XtmBondConfig bondConfig,
            FN_XTMRT_REQ pfnXtmrtReq,
            XTM_INTERFACE *pInterfaces,
            XTM_CONNECTION_TABLE *pConnTable,
                XTM_AUTOSENSE *pXtmAutoSense);
      BCMXTM_STATUS Uninitialize( void );
      void LinkStateNotify (UINT32 ulLogicalPort);
                UINT32 GetGroupId () ;
                UINT32 GetDataStatus () ;
      UINT32 getSIDMode (void) ;
      BOOL CheckASMActivity () ;
      UINT32 GetLineInfo(UINT32 ulLogicalPort, 
                         UINT32 *groupId,
                         int *localTxState,
                         int *localRxState);
      void StopTimerThread(void);

   private:
      FN_XTMRT_REQ   m_pfnXtmrtReq;
      UINT32         m_ulRegistered;
      XtmBondMgmt    m_BondMgmt;
      UINT32         m_BondingFallback;
      XtmBondConfig  m_BondCfg;
      XTM_INTERFACE  *m_pInterfaces;
      XTM_CONNECTION_TABLE *m_pConnTable;
        XTM_AUTOSENSE *m_pXtmAutoSense;
        bool            m_bInitialized;
        bool            m_bUninitialized;

      void resetBondingGroup (void);
      void mappingReset(UINT32 ulLogicalPort, int timeout);
      static void LineInfo_init(LineInfo *objp);
      static int  IsElapsedSinceLastAsm (LineMapping *mappingp);
      void PollLines ();
      void TxAsm ();
      static void MgmtTimerCb( void *ulParm );
      BCMXTM_STATUS SendAsmCell ( UINT32 ulLogicalPort, UINT8 *pAsmData);
      static void byteSwapAsmCell (Asm *pAsm);
      static void printNewAsm (Asm *pAsm, const char *pMsg);
      UINT32 LineInfo_setGroupId(UINT32 ulLogicalPort, UINT32 groupId);
      UINT32 LineInfo_getGroupId(UINT32 ulLogicalPort);
      UINT32 LineInfo_getUpdateLocalTxState (UINT32 ulLogicalPort, UINT32 rRx);
      UINT32 LineInfo_getUpdateLocalRxState (UINT32 ulLogicalPort, UINT32 rTx);
      void setSIDMode (void) ;
      int mappingKnown (void);
      UINT32 UpdateResetFsm(int wrongCheck, int newMsgType, UINT32 ulLogicalPort);
      void UpdateLineState (UINT32 ulLogicalPort) ;
      int  ProcessReceivedAsm (Asm *pAsm, UINT32 ulLogicalPort);

      static int  ReceiveAsmCb( XTMRT_HANDLE hDev, UINT32 ulCommand, void *pParam,
                   void *pContext );
};


// For the XTM_AUTOSENSE autosense control state machine, define states
typedef enum {
    BOTH_LINES_IDLE=10,       // Begin with both lines idle.
    ONE_LINE_NONIDLE,         // One (and only one) line is currently non-idle.
    BOTH_LINES_NONIDLE,       // Non-idle activity has been seen on both lines,
                                // even if one or both lines has slipped back to idle.
    SINGLE_LINE_DATA_OPERATION, // All timeouts have expired and we're running Single Line PHY
    BONDING_DATA_OPERATION,     // All timeouts have expired and we are running Bonding PHY
   
} XtmAutoSenseState;

   
// For the XTM_AUTOSENSE autosense control state machine, define state transitions
typedef enum {
    TRANSITION_NON_ATM_TRAFFIC,            // Detected we are not using ATM
    TRANSITION_BOTH_LINES_IDLE,            // Both lines currently idle
    TRANSITION_ONE_LINE_NONIDLE,           // One line has gone nonidle (i.e. training or showtime) from idle
    TRANSITION_BOTH_LINES_NONIDLE,         // Both lines at some point have gone nonidle, regardless of 
                                // line's current state (i.e. it may have reverted to idle)
    TRANSITION_BONDING_INDICATION_RX,      // A bonding indication (ASM or G.HS with bonding bit set) recieved.
    TRANSITION_TIMERSTRIKE,                // Periodic timer runs - check for timeouts
    TRANSITION_FORCEBONDING,               // Override all other states and force the Bonding PHY
    TRANSITION_FORCESINGLELINE             // Override all other states and force Single Line PHY
} XtmAutoSenseTransition;


// For the XTM_AUTOSENSE autosense control state machine, define status values
typedef enum {
    PHYSTATUS_AUTOSENSE_DISABLED,           // Autosense is disabled
    PHYSTATUS_UNKNOWN,                      // Still calculating the PHY status
    PHYSTATUS_ATM_SINGLELINE,               // Use Single Line PHY for ATM
    PHYSTATUS_ATM_BONDING                   // Use Bonding PHY for ATM
} XtmAutoSensePhyStatus;

// Define the XTM_AUTOSENSE class
class XTM_AUTOSENSE
{

public:
    // Constructors and initializers
    XTM_AUTOSENSE();
    ~XTM_AUTOSENSE();
    void initializeVars( bool );
    BCMXTM_STATUS Initialize( PXTM_INITIALIZATION_PARMS pInitParms );
    BCMXTM_STATUS Uninitialize( void );


    // Methods to set operational parameters
    void SetOneLineTimeoutMs(unsigned long);             // Set timeout to determine if only one line is configured
    void SetBondingTrafficTimeoutMs(unsigned long );     // Set timeout for bonding indication traffic

    // Methods to retrieve operational parameters
    unsigned long GetOneLineTimeoutMs(void);             // Get timeout to determine if only one line is configured
    unsigned long GetBondingTrafficTimeoutMs(void);      // Get timeout for bonding indication traffic


    // Methods for state transition events
    int LineIdle(UINT32);     // Line has entered idle state.
    int LineActive(UINT32);   // Line has entered nonidle (active or training) state.
    int NonAtmMode(void);            // Not in an ATM mode (i.e. PTM).  Reset state machine.
    int BondingIndicationRx(void);   // A bonding indication (ASM or G.HS with bonding bit set) recieved.
    void TimerUpdate(void);          // Called periodically to update internal timers
    int ForceBondingPhy(void);      // Override all other states and force the Bonding PHY
    int ForceSingleLinePhy(void);   // Override all other states and force the Single Line PHY

    // Status methods
    XtmAutoSensePhyStatus PhyStatus(void);       // Indicate what PHY to use
    bool PhyChangePending(void);                 // Return and clear flag indicating PHY needs to change

private:

   ////////////////////
   // Private members
   ////////////////////
   
   // Since this tracks the system state, make all variables common (static).
   bool m_bAutoSenseEnabled;             // True if autosense is in use
   unsigned long m_ulOneLineTimeoutMs;          // Timeout value to determine if only one line is configured
   unsigned long m_ulBondingTrafficTimeoutMs;   // Timeout value for bonding indication traffic
   unsigned long m_ulOneLineStartedTimestampMs;          // Actual time first line went nonidle
   XtmAutoSenseState m_eCurrentState;           // Current state of state machine
   bool m_pbLineActivityDetected[MAX_BONDED_LINES];
                                                // Activity detected on the given line
   bool m_bPhyChangePending;                     // One shot flag indicates PHY needs to change

   //int  timerStrike ;

   ////////////////////
   // Private methods
   ////////////////////
   int Transition(XtmAutoSenseTransition); // Perform the given state transition


};


#ifndef kXdslGinpDsEnabled
#define kXdslGinpDsEnabled     0x4
#endif

#define XTM_ADD         1
#define XTM_REMOVE      2

typedef struct _LogicalPhysicalPortmap
{
   UINT32 LogicalPortId;
   UINT32 PhysicalPortId;
}LogicalPhysicalPortmap;

class XTM_PROCESSOR
{
public:
    XTM_PROCESSOR( void ) { Cxtor(); }
    ~XTM_PROCESSOR( void ) { Dxtor(); }

    int Cxtor( void );
    int Dxtor( void );

    BCMXTM_STATUS Initialize( PXTM_INITIALIZATION_PARMS pInitValues,
        FN_XTMRT_REQ pfnXtmrtReq );
    BCMXTM_STATUS Uninitialize( FN_XTMRT_REQ pfnXtmrtReq );
    void ReconfigureSAR ( UINT32 ulPort, UINT32 trafficType );
    UINT32  LinkModeDelay (void);
    void ReprogramRxPaf ( UINT32 ulTrafficType, UINT32 ulPort,
             PXTM_INTERFACE_LINK_INFO pLinkInfo, UINT32 ulBondingPort,
             PXTM_INTERFACE_LINK_INFO pOtherLinkInfo) ;
    BCMXTM_STATUS GetTrafficDescrTable( PXTM_TRAFFIC_DESCR_PARM_ENTRY
        pTrafficDescTable, UINT32 *pulTrafficDescrTableSize );
    BCMXTM_STATUS SetTrafficDescrTable( PXTM_TRAFFIC_DESCR_PARM_ENTRY
        pTrafficDescTable, UINT32  ulTrafficDescrTableSize );
    BCMXTM_STATUS GetInterfaceCfg( UINT32 ulPort, PXTM_INTERFACE_CFG
        pInterfaceCfg );
    BCMXTM_STATUS SetInterfaceCfg( UINT32 ulPort, PXTM_INTERFACE_CFG
        pInterfaceCfg );
    BCMXTM_STATUS GetConnCfg( PXTM_ADDR pConnAddr, PXTM_CONN_CFG pConnCfg );
    BCMXTM_STATUS SetConnCfg( PXTM_ADDR pConnAddr, PXTM_CONN_CFG pConnCfg );
    BCMXTM_STATUS GetConnAddrs( PXTM_ADDR pConnAddrs, UINT32 *pulNumConns );
    void UpdateConnAddrs( UINT32 ) ;
    BCMXTM_STATUS GetInterfaceStatistics( UINT32 ulPort,
        PXTM_INTERFACE_STATS pIntfStats, UINT32 ulReset );
    BCMXTM_STATUS GetErrorStatistics( PXTM_ERROR_STATS pErrStats );
    BCMXTM_STATUS SetInterfaceLinkInfo( UINT32 ulPort,
        PXTM_INTERFACE_LINK_INFO pLinkInfo, UINT32 timerExpired );
    BCMXTM_STATUS SendOamCell( PXTM_ADDR pConnAddr,
        PXTM_OAM_CELL_INFO pOamCellInfo );
    BCMXTM_STATUS CreateNetworkDevice( PXTM_ADDR pConnAddr,
        char *pszNetworkDeviceName );
    BCMXTM_STATUS DeleteNetworkDevice( PXTM_ADDR pConnAddr );
    BCMXTM_STATUS GetBondingInfo ( PXTM_BOND_INFO pBondInfo );
    BCMXTM_STATUS CheckTrafficCompatibility ( UINT32 ulTrafficType ) ;
    BCMXTM_STATUS CheckAndSetIfLinkInfo ( UINT32 ulTrafficType, UINT32 ulPortId,
         PXTM_INTERFACE_LINK_INFO pLinkInfo, UINT32 ulBondingPort,
         PXTM_INTERFACE_LINK_INFO pOtherLinkInfo);
    UINT32 GetAvailLinkStatus (void) ;
    void CheckAndResetSAR( UINT32 ulPortId, PXTM_INTERFACE_LINK_INFO pLinkInfo) ;
    BCMXTM_STATUS Configure ( PXTM_CONFIGURATION_PARMS pConfigInfo );
    BCMXTM_STATUS ManageThreshold (PXTM_THRESHOLD_PARMS pThreshold);
#if defined(XTM_PORT_SHAPING)
    void ConfigureTxPortShapingRatios(UINT32 ulPhysPort, UINT32 ulTrafficType);
    void UpdateSitSlr(UINT32 ulPort) ;
#endif
private:
    UINT32 m_ulChipIdRev ;
#if defined(CONFIG_ARM64)
    unsigned long m_ulConnSem;
#else
    UINT32 m_ulConnSem;
#endif
    XTM_INITIALIZATION_PARMS m_InitParms;
    FN_XTMRT_REQ m_pfnXtmrtReq;
    PXTM_TRAFFIC_DESCR_PARM_ENTRY m_pTrafficDescrEntries;
    UINT32 m_ulNumTrafficDescrEntries;
    UINT32 m_ulRxVpiVciCamShadow[MAX_VCIDS];
    UINT32 m_ulTrafficType;
    UINT32 m_ulXTMLinkMode ;
    UINT32 m_ulXTMNitroMode ;

    UINT32 m_ulTrafficMonitorPort ;
    UINT32 m_ulPrevRelatedUpEvtValid ;
    UINT32 m_ulPendingEvtValid [MAX_BONDED_LINES];
    UINT32 m_ulPendingEvtPortId [MAX_BONDED_LINES];
    XTM_INTERFACE_LINK_INFO m_PendingEvtLinkInfo [MAX_BONDED_LINES];
    UINT32 m_ulRxBufferPrevOverrunCounter;
    UINT32 m_ulRxBufferPrevUnderrunCounter;
    UINT32 m_ulRxBufferPrevErrorPacketCounter;
    UINT32 m_ulRxBufferPrevErrorFragmentCounter;
    UINT32 m_ulRxPafLockupMonitorEvtValid ;
    UINT32 m_ulRxBondPrevErrStatus ;
    UINT32 m_ulRxBondPrevExpSid01 ;
    UINT32 m_ulRxBondPrevExpSid23 ;
    UINT32 m_ulRxPafLongLinkListThreshold;
    UINT32 m_ulRxPafShortLinkListThreshold;
    UINT32 m_ulTxPafEnabled ;
    UINT32 m_ulPortShapingConfig;
    UINT32 m_ulQShapingConfig;
    UINT32 m_ulSlrCnt;
    UINT32 m_ulSitCnt;
    UINT32 m_ulSitLoCnt;
    LogicalPhysicalPortmap m_LogPhyPortmap[MAX_INTERFACES];
    XTM_INTERFACE m_Interfaces[MAX_INTERFACES];
    XTM_CONNECTION_TABLE m_ConnTable;
    XTM_OAM_HANDLER m_OamHandler;
    XTM_ASM_HANDLER m_AsmHandler;
    static XTM_ASM_HANDLER *m_pAsmHandler; /* Static reference for ASM handler */
    BCMXTM_STATUS ReInitialize(UINT32 ulTrafficType) ;
    void UpdateConnAddrForBonding ( PXTM_ADDR pConnAddr, UINT32 operation ) ;
    void UpdateTransmitQParamsForBonding ( XTM_TRANSMIT_QUEUE_PARMS *pTxQParms, 
                                           UINT32 ulTxQParmsSize, UINT32 operation ) ;
    static void XtmMonitorTimerCb (void *ulParm) ;
    void AutoSensePreProcess (UINT32 ulPhysPort, UINT32 ulTrafficType, 
                              PXTM_INTERFACE_LINK_INFO pLinkInfo) ;
    void PollLines ();
    void UpdateLinkMode (UINT32 ulDSLGinpMode, UINT32 ulDSLMode) ;
    void UpdateLinkStatus (UINT32 ulPhysPorts, PXTM_INTERFACE_LINK_INFO pLinkInfo) ;
#ifdef SAR_RX_CTL_LED_CFG
    void SetupWanDataLed (PXTM_INTERFACE_LINK_INFO pLinkInfo, PXTM_INTERFACE_LINK_INFO pOtherLinkInfo) ;
#endif
    BCMXTM_STATUS validateThreshold (UINT32 ulThreshold);
    XTM_AUTOSENSE m_XtmAutoSense;
};


/* Cache related definitions and inline functions. */
#define CACHE_LINE_SIZE                  16   /* should read from CP0 config, 1*/
#define Hit_Invalidate_D                 0x11
#define Hit_Writeback_Inv_D              0x15        /* 5       1 */

#define cache_op(op,addr)                   \
    __asm__ __volatile__(                   \
    "   .set    noreorder               \n" \
    "   .set    mips3\n\t               \n" \
    "   cache   %0, %1                  \n" \
    "   .set    mips0                   \n" \
    "   .set    reorder"                    \
    :                                       \
    : "i" (op), "m" (*(unsigned char *)(addr)))

/* Use Hit_Writeback_Inv_D cacheop for both "flush" and "invalidate" cache
 * operations.  Cacheop Hit_Invalidate_D does not seem to work.
 */
static inline void flush_dcache_line(unsigned long addr)
{
    cache_op(Hit_Writeback_Inv_D, addr);
}

/*
 * Always Writeback flush invalidate.
 * Rewrite of cache write back flush functions to not flush one extra cache
 * line post the end region demarcated by a pointer or the length.
 */
/* Macros to round down and round up an address to be cache aligned */
#define ROUNDDN(addr, align)  ( (addr) & ~((align) - 1) )
#define ROUNDUP(addr, align)  ( ((addr) + (align) - 1) & ~((align) - 1) )

/*
 * Writeback flush, then invalidate a region given an address and a length.
 * The demarcation end is computed by applying length to address before
 * rounding down address. End is rounded up.
 *
 * Boundary case:
 * - if (len == 0) and (addr is cache-aligned), no action performed.
 * - if (len == 0) and (addr is not cache-aligned), then flush cache line
 *                                                     containing address
 *
 * end is computed by adding len to addr, and then rounded "up" to cache line.
 * addr is then rounded "down" to cache line.
 * NOTE: Cacheline pointed to by the 'rounded up end' is NOT flushed.
 *
 * cache_wbflush_len() obsoletes cache_wback_inv() which would result in
 * a cache operation one cache line beyond the len.
 */
static inline void cache_wbflush_len(void *addr, int len)
{
    const unsigned long dc_lsize = CACHE_LINE_SIZE;
    unsigned long a = ROUNDDN( (unsigned long)addr, dc_lsize );
    unsigned long e = ROUNDUP( ((unsigned long)addr + len), dc_lsize );
    while ( a < e )
    {
        flush_dcache_line(a);   /* Hit_Writeback_Inv_D */
        a += dc_lsize;
    }
}

#define XTM_IS_WRCHAN_FLUSH_BUSY  ((XP_REGS->ulRxPafStatus & RP_STATUS_WRCHN_FLUSH_BUSY_MASK)?1:0)

/* RxPaf Error Handling definitions */
#define RXPAF_PTM_OVERHEAD             5
#define RXPAF_ATM_OVERHEAD            12
#define RXPAF_JITTER                  12 /* 12 ms  - 36 PHY symbols */
#if defined(CONFIG_BCM_DSL_GINP_RTX) || defined(SUPPORT_DSL_GINP_RTX)
#define RXPAF_RTX_DELAY               20
#else
#define RXPAF_RTX_DELAY               0
#endif
#define RXPAF_SHORT_LINK_LIST_THRESHOLD 10
#define RXPAF_LONG_LINK_LIST_THRESHOLD 1082

/* On C0 versions of the 63268, include a safety margin for the long list threshold.
   This is done because until the D0 revision, there sould be problems with hanging the line if we try to use all available entires. */
#define RXPAF_LONG_LINK_LIST_THRESHOLD_63268C0 (RXPAF_LONG_LINK_LIST_THRESHOLD - 16)

#define RXBOND_CELL_FULL_WR_CHAN_SHIFT  16
#define RXBOND_TIMEOUT_WR_CHAN_SHIFT  8
#define RXBOND_TIMEOUT_1_25_MSECS     5

/* Following derived from DSL --mediaSearchCfg command options */
/* Bit Format as defined in mediaSearchCfg command:
   [0] PHY Switch:        1=Disabled, 0=Enabled
   [1] Media Search:      1=Disabled, 0=Enabled
   [2] Force new PHY/line configuration as defined in bits 3 and 4
   [3] PHY Type:          1=Single Line, 0=Bonded
   [4] AFE:               1=external, 0=internal
*/ 
#define XTM_USE_NON_BONDED_PHY        0xC
#define XTM_USE_BONDED_PHY            0x4

#define MapXtmOsPrintf           DummyXtmOsPrintf

#endif /* _XTMCFGIMPL_H_ */
