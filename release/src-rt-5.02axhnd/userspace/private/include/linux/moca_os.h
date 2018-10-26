/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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
 *
 ************************************************************************/

#ifndef __MOCA_OS_H__
#define __MOCA_OS_H__

#include <stdint.h>

typedef uintptr_t MoCAOS_Handle;
typedef uintptr_t MoCAOS_ClientHandle;
typedef uintptr_t MoCAOS_MutexHandle;
typedef uintptr_t MoCAOS_ThreadHandle;

typedef void (*MoCAOS_ThreadEntry)(void *);

#define MoCAOS_TIMEOUT_INFINITE 0xFFFFFFFF

#define MoCAOS_CLIENT_NULL      (MoCAOS_ClientHandle)0xFFFFFFFF
#define MoCAOS_CLIENT_TIMEOUT   (MoCAOS_ClientHandle)0xFFFFFFFE
#define MoCAOS_CLIENT_CORE      (MoCAOS_ClientHandle)0xFFFFFFFD
#define MoCAOS_CLIENT_BROADCAST (MoCAOS_ClientHandle)0xFFFFFFFC
#define MoCAOS_CLIENT_NL        (MoCAOS_ClientHandle)0xFFFFFFFB
#define MoCAOS_CLIENT_GMII      (MoCAOS_ClientHandle)0xFFFFFFFA

#define MoCAOS_CLIENT_TOP       (MoCAOS_ClientHandle)0xFFFFFFF9
#define MoCAOS_CLIENT_BASE      (MoCAOS_ClientHandle)0

#define MOCAOS_INVALID_THREAD   ((MoCAOS_ThreadHandle)0)

#define MOCAOS_BOOT_FLAGS_BONDING_EN  (1 << 0)

#if defined(STANDALONE)
#define MoCAOS_IFNAMSIZE        8
#else
#define MoCAOS_IFNAMSIZE        16
#endif

#if (!defined(_GNU_SOURCE) && !defined(DSL_MOCA) && !defined(STANDALONE))
#define __attribute__(x)   
#endif

#if (defined(WIN32) && !defined(snprintf))
#define snprintf(buf, len, fmt, ...) sprintf(buf, fmt, __VA_ARGS__)
#endif

typedef struct _MoCAOS_DrvInfo {
    unsigned int version;
    unsigned int build_number;
    unsigned int builtin_fw;

    unsigned int hw_rev;
    unsigned int rf_band;

    unsigned int uptime;
    int refcount;
    unsigned int gp1;

    char enet_name[MoCAOS_IFNAMSIZE];
    unsigned int enet_id;

    unsigned int macaddr_hi;
    unsigned int macaddr_lo;

    unsigned int phy_freq;
    unsigned int device_id;

    unsigned int chip_id;
} MoCAOS_DrvInfo;

MoCAOS_Handle MoCAOS_Init(const char *chardev, char *ifname, const char *workdir, int daemon, void *moca_handle);

MoCAOS_ClientHandle MoCAOS_WaitForRequest(MoCAOS_Handle handle, unsigned int timeout_sec);
int MoCAOS_SendMMP(MoCAOS_Handle handle, MoCAOS_ClientHandle client, const unsigned char *IE, int len);
int MoCAOS_ReadMMP(MoCAOS_Handle handle, MoCAOS_ClientHandle client, 
    unsigned int timeout_sec, unsigned char *IE, int *len);  // return 0 timeout, <0 failure, >0 success
int MoCAOS_ReadMem(MoCAOS_Handle handle, unsigned char *buf, int len, unsigned int addr);
int MoCAOS_WriteMem(MoCAOS_Handle handle, unsigned char *buf, int len, unsigned int addr);
int MoCAOS_StartCore(MoCAOS_Handle handle, unsigned char *fw_img, int fw_len,
    unsigned int boot_flags);
int MoCAOS_StopCore(MoCAOS_Handle handle);
int MoCAOS_GetDriverInfo(MoCAOS_Handle handle, MoCAOS_DrvInfo *kdrv_info);
int MoCAOS_WolCtrl(MoCAOS_Handle handle, int enable);
int MoCAOS_PMDone(MoCAOS_Handle handle);
unsigned char *MoCAOS_GetFw(MoCAOS_Handle handle, unsigned char *filename, int *fw_len);

int MoCAOS_Get3450Reg(MoCAOS_Handle handle, unsigned int reg, unsigned int * val);
int MoCAOS_Set3450Reg(MoCAOS_Handle handle, unsigned int reg, unsigned int val);
void MoCAOS_EnableDataIf(MoCAOS_Handle handle, char *ifname, int enable);
void MoCAOS_CloseClient(MoCAOS_Handle handle, MoCAOS_ClientHandle client);
MoCAOS_ClientHandle MoCAOS_ConnectToMocad(MoCAOS_Handle handle, const char *fmt, char *ifname);

MoCAOS_MutexHandle MoCAOS_MutexInit();
void MoCAOS_MutexLock(MoCAOS_MutexHandle x);
void MoCAOS_MutexUnlock(MoCAOS_MutexHandle x);
void MoCAOS_MutexClose(MoCAOS_MutexHandle x);

int MoCAOS_MemAlign(void **memptr, int alignment, int size);
void MoCAOS_FreeMemAlign(void *x);
void MoCAOS_MSleep(int msec);
void MoCAOS_GetTimeOfDay(unsigned int *sec, unsigned int *usec);

MoCAOS_ThreadHandle MoCAOS_CreateThread(MoCAOS_ThreadEntry func, void *arg);
unsigned int MoCAOS_GetTimeSec();
unsigned int MoCAOS_GetRandomValue();
int MoCAOS_GetRMON(unsigned int *rmon_hz, unsigned int *rmon_vt);
void MoCAOS_Alarm(unsigned int seconds);

#if !defined(DSL_MOCA)
void mocaos_setLinkStatus(MoCAOS_Handle handle, int status);
#endif

unsigned int MoCAOS_SetCpuClk(MoCAOS_Handle handle, uint32_t freq_mhz);
unsigned int MoCAOS_SetPhyClk(MoCAOS_Handle handle, uint32_t cpu_freq_hz);

#ifdef STANDALONE
#define MoCAOS_Printf(handle,fmt, ...) xprintf_nonblocking(fmt, ##__VA_ARGS__)
#else
void MoCAOS_Printf(MoCAOS_Handle handle, const char *fmt, ...);
#endif

int MoCAOS_SetLedMode(MoCAOS_Handle handle, uint32_t led_mode);
int MoCAOS_SetLoopback(MoCAOS_Handle handle, uint32_t enable, uint32_t device_id);

int MoCAOS_SetSSC(MoCAOS_Handle handle, int enable);


#endif

