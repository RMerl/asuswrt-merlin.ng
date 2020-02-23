/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
//**************************************************************************
// File Name  : BcmAtmApi.h
//
// Description: This file contains the definitions, structures and function
//              prototypes for the Broadcom Asynchronous Transfer Mode (ATM)
//              Application Program Interface (API).
//
// Updates    : 09/15/2000  lat.  Created.
//**************************************************************************

#if !defined(_ATMOSSERVICES_H_)
#define _ATMOSSERVICES_H_

#if defined(__cplusplus)
extern "C" {
#endif

//**************************************************************************
// Constant Definitions
//**************************************************************************
#define RTN_SUCCESS                 0
#define RTN_ERROR                   1
#define USE_CURRENT_THREAD_PRIORITY 0

//**************************************************************************
// Type Definitions
//**************************************************************************
typedef void (*FN_GENERIC) (void *);
typedef struct AtmOsFuncs
{
    FN_GENERIC pfnAlloc;
    FN_GENERIC pfnFree;
    FN_GENERIC pfnDelay;
    FN_GENERIC pfnCreateSem;
    FN_GENERIC pfnRequestSem;
    FN_GENERIC pfnReleaseSem;
    FN_GENERIC pfnDeleteSem;
    FN_GENERIC pfnDisableInts;
    FN_GENERIC pfnEnableInts;
    FN_GENERIC pfnInvalidateCache;
    FN_GENERIC pfnFlushCache;
    FN_GENERIC pfnGetTopMemAddr;
    FN_GENERIC pfnBlinkLed;
    FN_GENERIC pfnGetSystemTick;
    FN_GENERIC pfnStartTimer;
    FN_GENERIC pfnPrintf;
} ATM_OS_FUNCS, *PATM_OS_FUNCS;

//**************************************************************************
// Function Prototypes
//**************************************************************************

UINT32 AtmOsInitialize( PATM_OS_FUNCS pFuncs );
char *AtmOsAlloc( UINT32 ulSize );
void AtmOsFree( char *pBuf );
UINT32 AtmOsCreateThread( char *pszName, void *pFnEntry, UINT32 ulFnParm,
    UINT32 ulPriority, UINT32 ulStackSize, UINT32 *pulThreadId );
UINT32 AtmOsCreateSem( UINT32 ulInitialState );
UINT32 AtmOsRequestSem( UINT32 ulSem, UINT32 ulTimeoutMs );
void AtmOsReleaseSem( UINT32 ulSem );
UINT32 AtmOsRequestSemCli( UINT32 ulSem, UINT32 ulTimeout );
void AtmOsReleaseSemSti( UINT32 ulSem );
void AtmOsDeleteSem( UINT32 ulSem );
UINT32 AtmOsDisableInts( void );
void AtmOsEnableInts( UINT32 ulLevel );
void AtmOsDelay( UINT32 ulTimeoutMs );
UINT32 AtmOsTickGet( void );
UINT32 AtmOsTickCheck( UINT32 ulWaitTime, UINT32 ulMsToWait );
void AtmOsInvalidateCache( void *pBuf, UINT32 ulLength );
void AtmOsFlushCache( void *pBuf, UINT32 ulLength );
char *AtmOsTopMemAddr( void );
void AtmOsBlinkLed( void );
UINT32 AtmOsInitDeferredHandler( void *pFnEntry, UINT32 ulFnParm,
    UINT32 ulTimeout );
void AtmOsScheduleDeferred( UINT32 ulHandle );
void AtmOsUninitDeferredHandler( UINT32 ulHandle );
UINT32 AtmOsStartTimer( void *pFnEntry, UINT32 ulFnParm, UINT32 ulTimeout );
void AtmOsPrintf( char *, ... );

#if defined(__cplusplus)
}
#endif

#endif // _ATMOSSERVICES_H_

