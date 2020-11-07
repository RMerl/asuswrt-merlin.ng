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

#ifndef _BCM_EPON_COMMON_H_
#define _BCM_EPON_COMMON_H_
#if defined(__KERNEL__)
#include <linux/bcm_log.h>
#endif
#include "bcmtypes.h"
#include "bcm_epon_cfg.h"

#if !defined  __KERNEL__ && !defined BDMF_SYSTEM_SIM 
#include "os_defs.h"
#endif

#define BCM_EPON_DRIVER_NAME  "bcmepon"
#define BCM_EPON_DRIVER_PATH  "/dev/" BCM_EPON_DRIVER_NAME
#define EPON_ASSOCIATE_LINK_IDX  0xFFFFU

#ifndef PACK
#define PACK __attribute__((__packed__))
#endif

/*
 *------------------------------------------------------------------------------
 * These definition is for both kernel space and userspace
 *
 *------------------------------------------------------------------------------
 */
#if defined(__KERNEL__)
#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_EPON, "EPON: "fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_EPON, "EPON: "fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_EPON, "EPON: "fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_EPON, "EPON: "fmt, ##arg)
#define __logLevelSet(level)      bcmLog_setLogLevel(BCM_LOG_ID_EPON, level)
#define __logLevelGet()           bcmLog_getLogLevel(BCM_LOG_ID_EPON)
#endif

#ifndef U8
typedef unsigned char U8;
#endif

#ifndef U16
typedef unsigned short U16;
#endif

#ifndef U24
typedef struct { U8 byte[3]; }      U24;
#endif

#ifndef U32
typedef unsigned int U32;
#endif

#ifndef U48
typedef struct { U8 byte[6]; }      U48;
#endif

#ifndef U64
typedef unsigned long long     U64;
#endif

#ifndef S8
typedef signed char    	S8;
#endif

#ifndef S16
typedef signed short   	S16;
#endif

#ifndef S32
typedef signed int    	S32;
#endif

#ifndef S48
typedef struct {U8 byte[6]; } PACK S48;
#endif

#ifndef S64
typedef signed long long	S64;
#endif

#ifndef F32
typedef float		F32;
#endif

#ifndef STATUS
typedef int		STATUS;
#endif

#ifndef bit
typedef U8 bit;
#endif


/* Some common definition */
#define OnuPonPortCount         1
typedef U8 TkOnuEthPort;

typedef enum
    {
    // Internal interfaces
    PortIfNone           = 0x00,
    PortIfPon            = 0x01,
    PortIfUni            = 0x02,
    PortIfProc           = 0x03,
    PortIfI2c            = 0x04,
    PortIfSpi            = 0x05,
    PortIfMdio           = 0x06,
    PortIfLink           = 0x07,
    PortIfBridge         = 0x08,
    
    // External interfaces
    PortIfExtUni         = 0x20,
    PortIfForceToU8      = 0x7F,
    } PortIfType;


typedef U8 PortInstance; // instance of the port within its port type
typedef U8 PortIndex; // externaly visible port number
typedef struct
    {
    PortIfType           pift;
    PortInstance         inst;
    } Port;

typedef U8 LinkIndex;
typedef U32 LinkMap;
typedef U16 PhyLlid;

typedef enum
    {
    OuiOamStart,
    OuiTeknovus = OuiOamStart,
    OuiCtc,
    OuiNtt,
    OuiDasan,
    OuiDpoe,
    OuiKt,
    OuiPmc,
    OuiCuc,
    OuiOamEnd,

    Oui802Dot1 = OuiOamEnd,
    OuiKnownCount
    } OuiVendor_e;
typedef U8 OuiVendor;



/* IOCTL commands */
typedef enum 
    {
    BCMEPONCfgPers   = 0,               /* 0 */
    BCMEPONCfgDebug,
    BCMEPONCfgReg,
    BCMEPONCfgReport,
    BCMEPONCfgStats,
    BCMEPONGetLLID,
    BCMEPONCfgLink,
    BCMEPONCfgQueue,
    BCMEPONGetMpcpStatus,
    BCMEPONCfgFec,
    BCMEPONCfgProtParm,                 /* 10 */
    BCMEPONCfgTxPower,
    BCMEPONDumpStats,
    BCMEPONGather,
    BCMEPONSilence,
    BCMEPONBCap,
    BCMEPONKeyInuse,
    BCMEPONKeyMode,
    BCMEPONKeyData,
    BCMEPONShaper,
    BCMEPONCtcAlm,                      /* 20 */
    BCMEPONCtcThe,
    BCMEPONCtcPer,
    BCMEPONCtcStats,
    BCMEPONCtcDeallocSilence,
    BCMEPONHoldover,
    BCMEPONLosTime,
    BCMEPONSetPid,
    BCMEPONLoadPers,
    BCMEPONCfgL2PonState,
    BCMEPONCfgLaserEn,                  /* 30 */
    BCMEPONCfgAssignMcast,
    BCMEPONCfgByteLimit,
    BCMEPONCfgLoopback,
    BCMEPONCfgRogueOnuDet,
    BCMEPONCfgFailSafe,
    BCMEPONCfgMaxFrameSize,
#if defined(CONFIG_EPON_CLOCK_TRANSPORT) || defined(CLOCK_TRANSPORT)
    BCMEPONCfgClktrans,
    BCMEPONCfgClk1ppsTickTrans,
    BCMEPONCfgClk1ppsCompTrans,         /* 40 */
    BCMEPONCfgClkTodTrans,

    /* for DPoE Clock */
    BCMEPONCfgClkTransferTime,
    BCMEPONCfgClkPropagationParameter,
#endif
#ifdef CONFIG_EPON_10G_SUPPORT
    BCMEPONCfg10gFecAutoDet,
    BCMEPONCfg10gFecSWOnce,
#endif
    BCMEPONDumpEpnInfo,
    BCMEPONPortL2cpStats,
    BCMEPONGetLinkIndex,
    BCMEPONGetMaxPhyLlidNum,
    BCMEPONCfgMpcpRegisterEn,
    BCMEPONCfgLlidFlush,
    BCMEPONCfgPowerSavingConfig,
    BCMEPONCfgPowerSaveSleepCtrl,
    BCMEPONCfgPowerSavingDebugState,
    BCMEPONCfgHoldMacState,
    BCMEPONCfgTxTestPacket,
    BCMEPONCfgFatalErrRst,
    BCM_EPON_IOCTL_MAX
    } EponioctlCmd_e;

/* Return status values. */
typedef enum Epon_Status
    {
    EponSTATUSSUCCESS = 0,
    EponSTATUSERROR,
    EponSTATUSLOADERROR,
    EponSTATUSSTATEERROR,
    EponSTATUSPARAMETERERROR,
    EponSTATUSALLOCERROR,
    EponSTATUSRESOURCE_ERROR,
    EponSTATUSINUSE,
    EponSTATUSNOTFOUND,
    EponSTATUSNOTSUPPORTED,
    EponSTATUSNOTREGISTERED,
    EponSTATUSTIMEOUT
    } EponCtlSTATUS_e;
typedef U8 EponCtlSTATUS;

typedef enum Epon_Ope
    {
    EponSetOpe = 1,
    EponGetOpe = 2
    } PACK EponCtlOpe;

//Personality Info
typedef enum Epon_LoadCfg
    {
    EponLoadCfgCur   = 1,
    EponLoadCfgDef   = 2
    } PACK EponLoadCfg;

typedef enum
    {
    PsModeInternal,
    PsModeExternal
    } PACK PsMode;

typedef enum
    {
    PsConfigNone                    = 0,
    PsConfigDualXcvr                = 1,
    PsConfigOptSwitchPulse          = 2,
    PsConfigOptSwitchLevel          = 3,
    PsConfigOptSwitchLevelNoOutput  = 4
    } PACK PsConfig;

typedef struct
    {
    PsConfig    type;
    PsMode      mode;
    } PACK PsConfig10G;

/* MAC address */
typedef union
    {
    U8      byte[6];
    U16     word[3];
    U8      u8[6]; 
    U8     mac[6];
    struct
        {
        U16 hi;
        U32 low;
        }PACK lowHi;
    } PACK MacAddr;

typedef struct
    {
    U8 Onepps;
    U8 ToD;
    U8 Frame1588V2;
    } PACK DpoeClockTransportParam;

#define TodStringMaxLength 40

typedef struct
    {
    U32 mpcpRefClock;           /* unit TQ */
    U8  todLen;
    U8  strTod[TodStringMaxLength];
    } DpoeClockTransferTimeParam; /* No need to be "PACK" */

typedef struct
    {
    U32 ndown;
    U32 nup;
    }PACK ClockPropagationParam;

/** Epon link MPCP regitration state */
typedef enum
{
    EponMpcpUnregistered,
    EponMpcpRegistering,
    EponMpcpAwaitingRegister,
    EponMpcpInService,
    EponMpcpAwaitingGate
} EponMpcpStatus;

/** Epon holdover flags */
typedef enum
{
    EponHoldoverNoflags = 0,
    EponHoldoverRerange = 1
} EponHoldoverFlags;

/**
 * EPON Holdover
 */
typedef struct 
{
    uint16_t time;
    EponHoldoverFlags flags;
}PACK EponHoldoverCfg;

#define NumPonIf    1

typedef enum
    {
    EponCfgItemPortUpRate            = 0,
    EponCfgItemPortDnRate            = 1,
    EponCfgItemPortLaserOn              = 2,
    EponCfgItemPortLaserOff             = 3,    
    EponCfgItemPortTxPolarity           = 4,
    EponCfgItemPortOffTimeOffset        = 5,
    EponCfgItemPortMaxFrameSize         = 6,
    EponCfgItemPortPreDraft2dot1        = 7,
    EponCfgItemPortNttReporting         = 8,
    EponCfgItemPortPowerupTime          = 9,
    EponCfgItemPortTxOffIdle            = 10,           

    EponCfgItemShaperipg                = 11,
    EponCfgItemPonMac                   = 12,
    EponCfgItemHoldover                 = 13,
    EponCfgItemActiveTime               = 14,
    EponCfgItemStandbyTime              = 15,
    EponCfgItemDefaultLinkNum           = 16,
    EponCfgItemPsCfg                    = 17,
    EponCfgItemOamSel                   = 18,
    EponCfgItemSchMode                  = 19,
    EponCfgItemIdileTimOffset           = 20,
    
    EponCfgItemCount                    = 21
    } PACK EponCfgItem;

typedef struct 
    {
    EponCfgItem index;
    U8          inst;
    union {                
        U8 dnRate;
        U8 upRate;
        U16 laserOn;
        U16 laserOff;
        U8 txPolarity;
        U8 offTimeOffset;
        U16 maxFrameSize;
        U8 preDraft2dot1;
        U8 nttReporting;
        U16 powerupTime;
        BOOL txOffIdle;
        BOOL shaperipg;
        MacAddr ponMac;
        EponHoldoverCfg holdoverval;
        U16   activetime;
        U16   standbytime;
        U8      defaultlinknum;
        PsConfig10G pscfg;
        U8      oamsel;
        U8      schMode;
        U8      idleTimeOffset;
        U8      u8;
        U8      u16[2];
        U8      u32[4];
        U8      u48[6];
        } PACK eponparm;
    } PACK EponCfgParam;

//debug parameter
typedef enum
    {
    DebugDisable = 0,
    DebugIoctl   = 1,
    DebugMpcp,
    DebugNco,
    DebugEpon,
    DebugStats,
    DebugAlarm,
    DebugHoldOver,
    DebugProtectSwitch,
    DebugOptCtrl,
    DebugOntDir,
    DebugEponUsr,
    DebugEponRogue,
    DebugEponPowerSaving,
    DebugModuleCount
    } DebugModule_e;
typedef U8 DebugModule;

typedef struct {
	DebugModule id;
	BOOL        flag;	
} PACK DebugPara;


// register read/write
#define MaxRegCount 100
typedef struct {
	U32  regStart;
	U8  regCount;
	U32  regval; //if more than one ,the regval should be a point.	
} PACK RegRWPara;


//report mode
typedef enum
    {
    RptModeFrameAligned  = 0x01,
    //For RptModeMultiPriX X is the priority count
    RptModeMultiPri3     = 0x13,
    RptModeMultiPri4     = 0x14,
    RptModeMultiPri8     = 0x18,
    RptModeSharedL2      = 0x80,
    RptMode15Emulation   = 0x40,
    } PonMgrRptMode_e;
typedef U8 PonMgrRptMode;


//stats parameter
#define MaxStatsCount 100
typedef struct {
   U8   statsId;
   BOOL errorid;
   U64  statsVal;
} PACK StatsCntOne; 

typedef enum Epon_stats
    {
    EponStatsPon  = 1,
    EponStatsLink = 2,
    EponStatsLif  = 3,
    EponStatsXif  = 4,
    EponStatsXpcs32  = 5,
    EponStatsXpcs40  = 6
    } EponStatsIf_e;
typedef U8 EponStatsIf;

typedef struct {
    EponStatsIf port;
    U8          instance;
    U8          statsCount;
    union {
        //StatsCntOne statsVal1;
        StatsCntOne *statsVal2;
    }StatsCnt;
} PACK StatsPara;


//link queue parameter
typedef struct 
    {
    U8  level;  //L2  = L2Base(link)  + level. 
    U8  weight; //Only used by L2 scheduling. So the same level should be the same weight.
    U16 l1_size;
    } PACK l1_q_cfg_t;

typedef struct 
    {
    PonMgrRptMode rpt_mode;
    U8 link_num;
    U8 link_q_num[TkOnuMaxBiDirLlids];
    l1_q_cfg_t q_cfg[EponNumL1Queues];
    } PACK epon_mac_q_cfg_t;

typedef struct {
    epon_mac_q_cfg_t *cfg;
} PACK pon_mac_q_Para;


//static link parameter
typedef struct {
    U8  link;
    U16 LLID;
} PACK PhyLLIDPara;

typedef struct {
    BOOL flag;
    PhyLLIDPara link;
}PACK StaticLinkPara;

typedef struct {
    U8   queue;
    U8   limit;
} PACK ByteLimitPara;

//Fec parameter
typedef struct {
    U8 link;
    U8 tx;
    U8 rx;
} PACK FecPara;

typedef struct {
    U8 link;
    EponMpcpStatus state;
} PACK LinkStatusPara;

//Tx power parameter
typedef struct {
    U8  actOpt;
    U16 enabletime;
} PACK TxPowerPara;

//Laser control Parameter
typedef enum {
    Upstream,
    Dnstream,
    NumDirections
} Direction_e;
typedef U8 Direction;

typedef struct {
    Direction dir;
    BOOL enable;
}PACK LaserEnablePara;

//Protect switch state
typedef enum{
    PsStateNone         = 0,
    PsStateSwitched     = (1 << 0),
    PsStateSigDetA      = (1 << 1),
    PsStateSigDetB      = (1 << 2),

    PsStateForce16      = 0x7FFF
} PsOnuState_e;
typedef U16 PsOnuState;

//Los check parameter
typedef struct {
    U16 losopttime;
    U16 losmpcptime;
} PACK LosCheckPara;

//Silence parameter
#define CtcDefaultSilenceTime 60
typedef struct {
    U8  flag;//dellaoc flag
    U8  time;//second	
} PACK SilencePara;


//encryption key in-use
typedef struct {
    U8 link;
    U8 keyinUse;	
} PACK KeyInUsePara;

//encryption key data
typedef struct {
    Direction dir;
    U8  link;
    U8  keyindex;
    U8  length;
    U32 *key;
    U32 *sci;
} PACK KeyDataPara;

//encryption mode parameter
typedef enum{
    EncryptModeDisable = 0,
    EncryptModeAes,
    EncryptModeZoh,
    EncryptModeTripleChurn,
    EncryptMode8021AE,
    EncryptModeMax
} EncryptMode_e;
typedef U8 EncryptMode;

typedef enum{
    EncryptOptNone          = 0,
    EncryptOptBiDir         = 1 << 0,
    // UNUSED
    EncryptOptImplicitSci   = 1 << 1,   // AES only
    EncryptOptNoOam         = 1 << 2,   // AES and ZOH only
    EncryptOptNoMpcp        = 1 << 3,   // AES and ZOH only
    EncryptOptNoDnMac       = 1 << 4,   // AES and ZOH only
    EncryptOptNoUpMac       = 1 << 5,   // AES and ZOH only
    EncryptOptAuthOnly      = 1 << 6,   // AES only
    EncryptOptExpiryOnly 	= 1 << 7,

    EncryptOptForce16       = 0x7FFF
} EncryptOptions_e;
typedef U16 EncryptOptions;

typedef struct {
    U8 link;
    EncryptMode mode;
    EncryptOptions opts;
} PACK KeyModPara;

//Burst caps parameter
typedef struct {
    U8  link;
    U8  count;
    U16 *bcapsize;
} PACK BcapPara;

//shaper parameter
typedef struct {
    BOOL add;
    U32  l1map;
    U32  rate;
    U16  size;
    U8   shpid;
} PACK ShaperPara;


//ctc stats alarm parameter
typedef struct {
    U16  alarmId;
    BOOL enable;
} PACK CtcStatsAlarm;

typedef struct {
    U8  statsCount;
    CtcStatsAlarm *statsAlmVal;	
}PACK CtcStatsAlarmPara;

//ctc stats alarm threshhold parameter
typedef struct {
    U16 alarmId;
    U32	setThe;
    U32 clearThe;
    } PACK CtcStatsAlmThe;

typedef struct {
    U8  statsCount;
    CtcStatsAlmThe *statsTheVal;
    } PACK CtcStatsAlmThePara;

//ctc stats monitoring period parameter
typedef U32 CtcStatsPeriodPara;

//ctc stats parameter
typedef struct {
    U8  history;
    U8  statsCount;
    StatsCntOne *statsVal;		
} PACK CtcStatsPara;

//wan service control parameter
typedef struct {
    U8   index;
    BOOL enable;
} PACK WanStatePara;

//multicast link register parameter
typedef struct {
    U8          link;
    U16         assignedPort;
    U8          flags;
    U16         idxMcast;
} PACK AssignMcast;


//link user traffic state parameter
typedef struct {
    U8	 link;
    BOOL state;
} PACK UserTrafficPara;

//link loopback parameter
typedef struct {
    U8	 link;
    U8   isOn;
} PACK LinkLoopbackPara;

//Epon Rogue Onu Parameter
typedef struct 
{
    BOOL enable;
    U32  threshold;
    U8   times;
}PACK  EponRogueOnuPara;

//Epon link number Parameter
typedef struct 
{
    U8   maxLinks;
    U8   onlineLinks;
}PACK  EponLinksPara;

typedef struct {
    U64 rxL2cpFrames;
    U64 rxL2cpBytes;
    U64 txL2cpFrames;
    U64 txL2cpBytes;
    U64 rxL2cpFramesDropped;
    U64 rxL2cpBytesDropped;
} PACK L2cpStats;


typedef struct {
    U8  port;
    L2cpStats *l2cpStats;
} PACK PortL2cpStats;

//clock transport 
typedef enum 
{
    ClkTransOptionsNone = 0,
    ClkTransOptionsShow,
    ClkTransOptionsTodFmt,
    ClkTransOptionsTodCmccAscii,
    ClkTransOptionsTimeAdj,
    ClkTransOptionsCmccPara,
    ClkTransOptions8021AsMsg,
    ClkTransOptionsHalfperiod,
    ClkTransOptionsIgnoreMsg,
    ClkTransOptionsForce16 = 0x7FFF
} ClkTransOptions_e;
typedef U16 ClkTransOptions;

typedef struct{
    U16 flag;
    U32 data;
    U32 expandRate;
    U8  tod[10]; 
}PACK  ClkTransPara;

/*
 * ClockTransport - ONU configuration
*/
typedef enum{
    TkOnuKeyAll  = 0,
    TkOnuKeyInit,
    TkOnuKeyTodEnable,
    TkOnuKey1ppsEnable,
    TkOnuKeyAdjust,
    TkOnuKeyPonRange,
    TkOnuKeyFakeMpcpJump,
    TkOnuKeyHalfPeriod,
    TkOnuNumKeys
} PACK TkOamClkTransKey;

typedef struct
    {
    U8 key;
    U8 reinit;           /* Non 0 : Reinitialize ONU clk transport */
    U8 tod;              /* Non 0 : Enable tod output */
    U8 pulse;            /* Non 0 : Enable 1pps output */
    S32 adj;             /* Per-onu 1pps offset adjustment */
    U32 rtt;             /* Round trip time */
    U32 half;            // Half period
    } PACK TkOamOnuClkTransConfig;

/// value of the attribute OamExtAttrMpcpClock
typedef struct
    {
    U32 nextPulseTime;
    } PACK  OamExtMpcpClock;


/// value of the attribute OamExtAttrTimeOfDay
typedef struct
    {
    U8 seqNum;           // Sequence number to filter duplicates
    char tod[1];         // TOD string
    } PACK OamExtTimeOfDay;

/// value of the attribute OamExtAttrTimeOfDay
typedef struct
    {
    U8 seqNum;           // Sequence number to filter duplicates
    char tod[255];         // TOD string
    } PACK OamExtTimeOfDayOlt;
//debug parameter
typedef enum
    {
    EpnInfoNotUsed = 0,
    EpnInfoL1Cnt = 1,
    EpnInfoInterrupt  = 2
    } EpnInfoId_e;

typedef struct 
{
    EpnInfoId_e epnInfoId;
    U32 epnSetVal;
}PACK  EpnInfoPara;

typedef struct
    {
    U8  length;
    U32 mpcpRefClock;           /* unit TQ */
    U8  tod[TodStringMaxLength];
    }PACK DpoeClockTransferTime;

typedef enum
    {
    SleepFlagLeave            = 0x00,
    SleepFlagEnter            = 0x01,
    SleepFlagModify           = 0x02
    } PACK SleepFlag;

typedef enum
    {
    SleepModeNone             = 0x00,
    SleepModeTxOnly           = 0x01,
    SleepModeTRx              = 0x02
    } PACK SleepMode;

typedef struct
    {
    Bool    earlyWakupEnable;
    U64     refreshTime;
    }PACK PowerSaveCfgPara;

typedef struct
    {
    U32 sleep_duration;
    U32 wake_duration;
    SleepFlag  sleep_flag;
    SleepMode  sleep_mode;
    }PACK SleepCtrlPara;

typedef struct tagPowerSaveState
{
    Bool isRunning;
    Bool isWake;
    U16 switchCount;
    U32 sleepCount;
    U32 wakeCount;
    U64 refreshCount;
}PACK EponPowerSavingRunningState;

typedef struct tagTxTestPacket
{
    U16 queueid;
    U16 size;
    U16 txNum;
    U8  link;
}PACK EponTxTestPacketParam;

typedef union {
        EponCfgParam   *poncfg; 
        EponLoadCfg    eponact;
        DebugPara	   debug;
        int            nlpid;
        RegRWPara	   reg;
        PonMgrRptMode  reportmod;
        StatsPara	   stats;
        pon_mac_q_Para epon_mac_q;
        StaticLinkPara staticlink;
        PhyLLIDPara    llid;
        EponLinksPara  linksInfo;
        FecPara   fec;
        EponHoldoverCfg  holdover; 
        LinkStatusPara linkstatus;
        TxPowerPara    txpower;
        LaserEnablePara laserpara;
        PsOnuState   psstate;
        LosCheckPara   lostime; 
        SilencePara    silence;
        KeyInUsePara   keyinuse;
        KeyDataPara    keydata;
        KeyModPara	   keymode;
        U8	gather;
        U8	statsdumpid;//0:lif,1:epon,2-9: link
        BcapPara	   bcapval;
        ShaperPara	   shpval;
        CtcStatsAlarmPara  ctcstatsalm;
        CtcStatsAlmThePara ctcstatsthe;
        CtcStatsPeriodPara ctcstatsperiod;
        CtcStatsPara	   ctcstats;
        WanStatePara    wanstate;
        AssignMcast   assignMcast;  
        UserTrafficPara   userTraffic;  
        ByteLimitPara   bytelimit;
        OuiVendor	oamOui;
        LinkLoopbackPara   loopback;
        ClkTransPara clkTrans;
        EponRogueOnuPara rogueonudetect;
        OamExtMpcpClock extmpcpClk;
        TkOamOnuClkTransConfig clkTransCfg;
        OamExtTimeOfDay extTod;
        U8 failsafe;
        EpnInfoPara epninfo;
        PortL2cpStats portL2cpStats;
        U16 maxPhyLlidNum;
        DpoeClockTransferTimeParam clkTransportTime;
        ClockPropagationParam clkPropagationParameter;
        U8 fec10gAutoDet;
        BOOL MpcpRegisterEn;
        PowerSaveCfgPara powerSaveCfg;
        SleepCtrlPara sleepCtrl;
        EponPowerSavingRunningState runningState;
        BOOL holdMacState;
        EponTxTestPacketParam txTestPacketPara;
        U8 fatalErrRstEn;
} PACK EponParm;

typedef struct {
    EponCtlOpe ope;
    EponParm   eponparm;
} PACK EponCtlParamt;

/// identifies statistic
typedef enum
    {
    StatIdFirst,
    // Rx stats
    StatIdRxFirst = StatIdFirst,

    StatIdBytesRx = StatIdRxFirst,

    StatIdTotalFramesRx,
    StatIdUnicastFramesRx,
    StatIdMcastFramesRx,
    StatIdBcastFramesRx,

    StatIdFcsErr,
    StatIdCrc8Err,
    StatIdLineCodeErr,

    StatIdFrameTooShort,
    StatIdFrameTooLong,
    StatIdInRangeLenErr,
    StatIdOutRangeLenErr,
    StatIdAlignErr,

    // bin sizes available on Ethernet ports only
    StatIdRx64Bytes,
    StatIdRx65_127Bytes,
    StatIdRx128_255Bytes,
    StatIdRx256_511Bytes,
    StatIdRx512_1023Bytes,
    StatIdRx1024_1518Bytes,
    StatIdRx1519PlusBytes,

    StatIdRxFramesDropped,                 // dropped in queue, that is
    StatIdRxBytesDropped,                 // dropped in queue, that is
    StatIdRxBytesDelayed,
    StatIdRxMaxDelay,
    StatIdRxDelayThresh,

    StatIdRxPauseFrames,
    StatIdRxControlFrames,

    StatIdErrFrames,
    StatIdErrFramePeriods,
    StatIdErrFrameSummary,

    StatIdNumRxStats,

    // Tx stats
    StatIdTxFirst = StatIdNumRxStats,

    StatIdBytesTx = StatIdTxFirst,
    StatIdTotalFramesTx,
    StatIdUnicastFramesTx,
    StatIdMcastFramesTx,
    StatIdBcastFramesTx,

    StatIdSingleColl,
    StatIdMultiColl,
    StatIdLateColl,
    StatIdFrAbortXsColl,

    // bin sizes available on Ethernet ports only
    StatIdTx64Bytes,
    StatIdTx65_127Bytes,
    StatIdTx128_255Bytes,
    StatIdTx256_511Bytes,
    StatIdTx512_1023Bytes,
    StatIdTx1024_1518Bytes,
    StatIdTx1519PlusBytes,

    StatIdTxFramesDropped,                 // dropped in queue, that is
    StatIdTxBytesDropped,                 // dropped in queue, that is
    StatIdTxBytesDelayed,
    StatIdTxMaxDelay,
    StatIdTxDelayThresh,
    StatIdTxUpUnusedBytes,

    StatIdTxPauseFrames,
    StatIdTxControlFrames,

    StatIdTxDeferredFrames,
    StatIdTxExcessiveDeferralFrames,

    
    StatIdMpcpMACCtrlFramesTx,
    StatIdMpcpMACCtrlFramesRx,
    StatIdMpcpTxRegAck,
    StatIdMpcpTxRegRequest,
    StatIdMpcpTxReport,
    StatIdMpcpRxGate,
    StatIdMpcpRxRegister,

    StatIdL2cpFramesRx,
    StatIdL2cpOctetsRx,
    StatIdL2cpFramesTx,
    StatIdL2cpOctetsTx,
    StatIdL2cpFramesDiscarded,
    StatIdL2cpOctetsDiscarded,
    StatIdL2ErrorsTx,
    StatIdL2ErrorsRx,

    StatIdEponIfRxFecCorrBlocks,
    StatIdEponIfRxFecUnCorrBlocks,

    StatIdNumStats

    } StatId;

typedef enum
    {
    StatDirRx,
    StatDirTx,
    StatDirNum
    }StatDir;

// Calculated by software
typedef enum
    {
    SwStatIdTxBytesDropped,
    SwStatIdTxFramesDropped,
    SwStatIdTxTotalFrames,
    
    SwStatIdRxBytesDropped,
    SwStatIdRxFramesDropped,
    SwStatIdRxTotalFrames,
    
    SwStatIdNum
    } SwStatId;

//Shaper types
typedef U8 EponShpElement;

//Shaper types
typedef U32 EponShaperRate;
typedef U16 EponMaxBurstSize;

typedef enum
    {
    DiscIdleTime,
    NonDiscIdleTime
    } IdleTimeType;

typedef enum 
    {
    EpnRptOpt15Emulation = 0x40,
    EpnRptOptSharedL2 = 0x80,    
    } EpnRptOpt;

typedef enum
    {
    EponUnmappedLlidBigFrameAboutCount,
    EponUnmappedLlidFrameCount,
    EponUnmappedLlidFcsErrorCount,
    EponUnmappedLlidGateCount,
    EponUnmappedLlidOamFrameCount,
    EponUnmappedLlidSmallFrameCount,
    } EponGeneralStatId ;

typedef enum
    {
    EponAesDisable      = 0,
    EponAesImplicitSci  = 1,
    EponAesExplicitSci  = 2,
    EponAesUndefined    = 3,
    EponAesClearAll     = 3
    } EponAesMode;

typedef enum
    {
    EponTxLaserNormal         = 0,
    EponTxLaserAlwaysOn       = 1,
    EponTxLaserDisable        = 2,
    EponTxLaserStatusNum
    } EponTxLaserStatus;

typedef enum
    {
    CtcStatIdFirst          = 0,
    CtcStatIdDnDropEvents   = CtcStatIdFirst,
    CtcStatIdUpDropEvents,
    CtcStatIdDnBytes,
    CtcStatIdUpBytes,
    CtcStatIdDnFrames,
    CtcStatIdUpFrames,
    CtcStatIdDnBcastFrames,
    CtcStatIdUpBcastFrames,
    CtcStatIdDnMcastFrames,
    CtcStatIdUpMcastFrames,
    CtcStatIdDnCrcErrFrames,
    CtcStatIdUpCrcErrFrames,
    CtcStatIdDnUndersizeFrames,
    CtcStatIdUpUndersizeFrames,
    CtcStatIdDnOversizeFrames,
    CtcStatIdUpOversizeFrames,
    
    CtcStatIdDnFragments,
    CtcStatIdUpFragments,
    CtcStatIdDnJabbers,
    CtcStatIdUpJabbers,    
    CtcStatIdDn64Bytes,
    CtcStatIdDn64_127Bytes,
    CtcStatIdDn128_255Bytes,
    CtcStatIdDn256_511Bytes,
    CtcStatIdDn512_1023Bytes,
    CtcStatIdDn1024_1518Bytes,
    CtcStatIdUp64Bytes,
    CtcStatIdUp64_127Bytes,
    CtcStatIdUp128_255Bytes,
    CtcStatIdUp256_511Bytes,
    CtcStatIdUp512_1023Bytes,
    CtcStatIdUp1024_1518Bytes,
    
    CtcStatIdDnDiscards,
    CtcStatIdUpDiscards,
    CtcStatIdDnErrors,
    CtcStatIdUpErrors,
    CtcStatIdPortChgs,
    CtcStatIdNums
    } CtcStatId_e;

typedef U8 CtcStatId;

typedef enum
    {
    PerfMonDnDropEvents = 0x0,          /* Downstream DropEvents        */
    PerfMonUpDropEvents,                /* Upstream DropEvents          */
    PerfMonDnOctets,                    /* Downstream Octets            */
    PerfMonUpOctets,                    /* Upstream Octets              */
    PerfMonDnFrames,                    /* Downstream Frames            */
    PerfMonUpFrames,                    /* Upstream Frames              */
    PerfMonDnBcFrames,                  /* Downstream Broadcast Frames  */
    PerfMonUpBcFrames,                  /* Upstream Broadcast Frames    */
    PerfMonDnMcFrames,                  /* Downstream Multicast Frames  */
    PerfMonUpMcFrames,                  /* Upstream Multicast Frames    */
    PerfMonDnCrcErrFrames,              /* Downstream CRC errored frames*/                           
    PerfMonUpCrcErrFrames,              /* Upstream CRC errored frames  */
    PerfMonDnUndersizeFrames,           /* Downstream Undersize Frames  */
    PerfMonUpUndersizeFrames,           /* Upstream Undersize Frames    */
    PerfMonDnOversizeFrames,            /* Downstream Oversize Frames   */
    PerfMonUpOversizeFrames,            /* Upstream Oversize Frames     */
    PerfMonDnFragments,                 /* Downstream Fragments         */
    PerfMonUpFragments,                 /* Upstream Fragments           */
    PerfMonDnJabbers,                   /* Downstream Jabbers           */
    PerfMonUpJabbers,                   /* Upstream Jabbers             */
    PerfMonDnFrame64Octets,             /* Downstream Frames 64 octets  */
    PerfMonDnFrame65_127Octets,         /* Downstream Frames 65 to 127 octets   */
    PerfMonDnFrame128_255Octets,        /* Downstream Frames 128 to 255 octets  */
    PerfMonDnFrame256_511Octets,        /* Downstream Frames 256 to 511 octets  */
    PerfMonDnFrame512_1023Octets,       /* Downstream Frames 512 to 1023 octets */
    PerfMonDnFrame1024_1518Octets,      /* Downstream Frames 1024 to 1518octets */
    PerfMonUpFrame64Octets,             /* Upstream Frames 64 octets            */
    PerfMonUpFrame65_127Octets,         /* Upstream Frames 65 to 127 octets     */
    PerfMonUpFrame128_255Octets,        /* Upstream Frames 128 to 255 octets    */
    PerfMonUpFrame256_511Octets,        /* Upstream Frames 256 to 511 octets    */
    PerfMonUpFrame512_1023Octets,       /* Upstream Frames 512 to 1023 octets   */
    PerfMonUpFrame1024_1518Octets,      /* Upstream Frames 1024 to 1518 octets  */
    PerfMonDnDiscards,                  /* Downstream Discards  */
    PerfMonUpDiscards,                  /* Upstream Discards    */
    PerfMonDnErrors,                    /* Downstream errors    */
    PerfMonUpErrors,                    /* Upstream errors      */
    PerfMonStateChangetimes,            /* Status Change times  */
    PerfMonStateButt                    /* BUTT                 */
    }OamCtcPerfMonStat_e;

typedef U8 OamCtcPerfMonStat;


#endif //_BCM_EPON_COMMON_H_

