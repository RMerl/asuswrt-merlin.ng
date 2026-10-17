/***********************************************************************
 *
 * Copyright (c) 2023  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2023:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 *
 ************************************************************************/


/*****************************************************************************
*    Description:
*
*      Life Cycle Management header file.
*
*****************************************************************************/

#ifndef LCM_H
#define LCM_H

#include <stdio.h>
#include <errno.h>
#include <stdbool.h>

#include "openplat.h"
#include "beep_common.h"
#include "cms_params_modsw.h"
#include "beep_manifest.h"

/* ---- Constants and Types ----------------------------------------------- */
#define LCM_HOSTEE_EE_TABLE_INDEX    1

#define LCM_DU_PER_UNINSTALL_MAX     8
#define LCM_DU_PER_UPDATE_MAX        4

#define LCM_UID_BASE_HOSTEE      9000
#define LCM_UID_MAX_NUM          1000

#define DEFAULT_HOSTEE_ENABLE    FALSE

#define STATUS_INSTALLED        "Installed"
#define STATUS_STARTING         "Starting"
#define STATUS_ACTIVE           "Active"
#define STATUS_STOPPING         "Stopping"
#define STATUS_IDLE             "Idle"


#define LCM_TBL_NAME_EE       "EE"
#define LCM_TBL_NAME_DU       "DU"
#define LCM_TBL_NAME_EU       "EU"
#define LCM_TBL_NAME_ALL      ""  /* all tables */

#define LCM_INDEX_LIST_MAX    64
#define MAX_EU_NUM_PER_DU     (LCM_INDEX_LIST_MAX / 4)

typedef struct lcm_ee_entry_t
{
    int  rowIdx;
    int  enable;
    int  reset;
    int  initialRunLevel;
    int  requestedRunLevel;
    int  currentRunLevel;
    int  initialExecutionUnitRunLevel;
    int  execEnvRef;
    int  allocatedDiskSpace;
    int  availableDiskSpace;
    int  allocatedMemory;
    int  availableMemory;
    int  mngrEid;
    int  isPreinstall;
    int  uidBase;
    int  maxActiveContainers;
    int  numberOfActiveContainers;
    int  isPrivileged;
    int  enableAfterInstall;
    int  accessHostFiles;
    long long quota;
    int  realtimeRuntime;
    char name[32];
    char vendor[32];
    char version[32];
    char alias[32];
    char status[32];
    char type[256];
    char containerName[64];
    char appName[32];
    char username[64];
    char iPv4Addresses[64];
    char iPv6Addresses[184];
    char exposedPorts[128];
    char apparmorProfile[256];
    char cpus[64];
    char networkMode[16];
    char primaryIfName[32];
    char primaryIfAddr[64];
    char secondaryIfName[32];
    char secondaryIfAddr[64];
    char wanOnlyIfName[32];
    char wanOnlyIfAddr[64];
} lcm_ee_entry_t;

typedef struct lcm_du_entry_t
{
    int  rowIdx;
    int  execEnvRef;
    int  resolved;
    int  isPreinstall;
    int  bundleId;
    char name[32];
    char vendor[32];
    char version[32];
    char alias[32];
    char status[32];
    char description[256];
    char uuid[64];
    char duid[64];
    char url[1024];
    char username[256];   
    char password[256];
    char euList[LCM_INDEX_LIST_MAX];    /* applicable to DU installed and produce one or more EU */
    char eeList[LCM_INDEX_LIST_MAX];    /* applicable to DU installed and produce one or more EE */
    char startTime[64];                 /* DU installation start time */
    char completeTime[64];              /* DU installation complete time */
} lcm_du_entry_t;

typedef struct lcm_eu_entry_t
{
    int rowIdx;
    int execEnvRef;
    int autoStart;
    int autoRelaunch;
    int bundleId;
    int diskSpaceInUse;
    int memoryInUse;
    int maxFlashSize;
    int availableDiskSpace;
    int runLevel;
    int autoStartOrder;
    int maxRestarts;
    int restartInterval;
    int successfulStartPeriod;
    int restartCount;
    int interfaceIdentifier;
    int realtimeRuntime;
    long long maxMemory;
    long long quota;
    char name[32];
    char vendor[32];
    char version[32];
    char alias[32];
    char status[32];
    char description[256];
    char euid[64];
    char execEnvLabel[64];
    char requestedState[32];
    char executionFaultCode[32];
    char executionFaultMessage[256];
    char associatedProcessList[1024];
    char stateChangeReason[64];
    char stateLastChange[64];
    char appDigest[145];
    char manifestDigest[145];
    char mediaType[16];
    char containerName[64];
    char mngrAppName[32];
    char username[64];
    char iPv4Addresses[64];
    char iPv6Addresses[184];
    char exposedPorts[128];
    char apparmorProfile[256];
    char cpus[64];
    char networkMode[16];
    char user[32];
} lcm_eu_entry_t;


BcmRet initMsgBus(void);
int msgLoop(int fd);
void cleanupMsgBus(void);

#endif /* LCM_H */
