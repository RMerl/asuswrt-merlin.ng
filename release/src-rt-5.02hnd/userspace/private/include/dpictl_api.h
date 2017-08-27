#ifndef __DPICTL_API_H_INCLUDED__
#define __DPICTL_API_H_INCLUDED__

/***********************************************************************
 *
 *  Copyright (c) 2006-2010  Broadcom Corporation
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
/***************************************************************************
 * File Name  : dpictl_api.h
 * Description: APIs for library that controls the Broadcom DPI feature.
 ***************************************************************************/

#include <bcmdpi.h>

#define DPIRET_ERROR            (-1)    /* Functional interface error     */
#define DPIRET_SUCCESS          0       /* Functional interface success   */

#define DPI_MODE_ALL            1
#define DPI_MODE_CURRENT        0

#define DPI_INVALID_APPID       0
#define DPI_INVALID_APPCATID    (-1)
#define DPI_INVALID_DEVID       0

/* Application define */
#define DPI_MAX_NAME_LEN        80
#define APPDB_MAX_ENTRIES       3600
#define APPCATDB_MAX_ENTRIES    64

/* Device define */
#define DPI_MAX_DEV_NAME_LEN    88
#define DEVDB_MAX_ENTRIES       480
#define DEVSTAT_MAX_ENTRIES     64

#define DPI_MACADDR_LEN         (16+1) //XX:XX:XX:XX:XX:XX + string termination
#define DPI_STATS_MAX_ENTRIES   4096

#define DPI_CONFIG_OPT_PARENTAL 0
#define DPI_CONFIG_OPT_QOS      1
#define DPI_CONFIG_OPT_MAX      2

#define DPI_CONFIG_ENABLE       1
#define DPI_CONFIG_DISABLE      0


typedef struct {
    unsigned int id;
    char name[DPI_MAX_NAME_LEN];
    unsigned long upPkt;
    unsigned long long upByte;
    unsigned long dnPkt;
    unsigned long long dnByte;
} AppDBEntry_t;

typedef struct {
    unsigned int id;
    char name[DPI_MAX_DEV_NAME_LEN];
} DevDBEntry_t;

typedef struct {
    char mac[DPI_MACADDR_LEN];
    unsigned int devDbId;
    unsigned long upPkt;
    unsigned long long upByte;
    unsigned long dnPkt;
    unsigned long long dnByte;
} DevStatEntry_t;

typedef struct {
    unsigned int appId;
    char mac[DPI_MACADDR_LEN];
    unsigned int devId;
    unsigned long upPkt;
    unsigned long long upByte;
    unsigned long dnPkt;
    unsigned long long dnByte;
} DpiStatEntry_t;

typedef struct {
    unsigned int appId;
    char mac[DPI_MACADDR_LEN];
    unsigned long upPkt;
    unsigned long long upByte;
    unsigned long upTs;
    unsigned long dnPkt;
    unsigned long long dnByte;
    unsigned long dnTs;
    unsigned int status;
    char tuple[2][46];
} DpiFlowEntry_t;

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlupdateDb
 * Description  : Update signature database
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlupdateDb(const char *appFile, const char *appCatFile,
                   const char *devFile);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetAppList
 * Description  : Get application list information
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetAppList(int mode);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetAppNames
 * Description  : Get list of application names
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetAppNames
    (int mode,
     unsigned int len,
     unsigned int *counter,
     char *appNames);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetNumberOfAppNames
 * Description  : Get number of application names
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetNumberOfAppNames
    (int mode,
     unsigned int *counter);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetAppCatList
 * Description  : Get application category list information
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetAppCatList(int mode);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetCategoryNames
 * Description  : Get list of category names
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetCategoryNames
    (int mode,
     unsigned int len,
     unsigned int *counter,
     char *catNames);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetNumberOfCategoryNames
 * Description  : Get number of category names
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetNumberOfCategoryNames
    (int mode,
     unsigned int *counter);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetCategoryIdFromCategoryName
 * Description  : Get category ID that matches category name
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetCategoryIdFromCategoryName
    (const char *catName,
     unsigned int *catId);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetAppStat
 * Description  : Get one application statistics
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetAppStat(int id);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetAppStatFromAppName
 * Description  : Get one application statistics that matches its name
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetAppStatFromAppName
    (const char *appName,
     AppDBEntry_t *pAppDb);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetAppCatStat
 * Description  : Get one application category statistics
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetAppCatStat(int id);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetCategoryStatFromCategoryName
 * Description  : Get one category statistics that matches its name
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetCategoryStatFromCategoryName
    (const char *catName,
     AppDBEntry_t *pCatDb);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetDevList
 * Description  : Get device list information
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetDevList(int mode);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetDevStat
 * Description  : Get one device statistics
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetDevStat(char *mac);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetDeviceNames
 * Description  : Get list of device names
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetDeviceNames
    (int mode,
     unsigned int len,
     unsigned int *counter,
     char *devNames);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetNumberOfDeviceNames
 * Description  : Get number of device names
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetNumberOfDeviceNames
    (int mode,
     unsigned int *counter);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetDeviceStatFromDeviceName
 * Description  : Get one device statistics
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetDeviceStatFromDeviceName
    (const char *devName,
     DevStatEntry_t *pDevStat);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetAppInst
 * Description  : Get application instance information
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetAppInst(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetAppInstStat
 * Description  : Get one application instance statistics
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetAppInstStat(int id, char *mac);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetAppInstStatFromAppDevName
 * Description  : Get one application statistics that matches its name
 *                and its device name
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetAppInstStatFromAppDevName
    (const char *appName,
     const char *devName,
     AppDBEntry_t *pAppDb);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetAppFlow
 * Description  : Get applications' detailed flow information
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetAppFlow(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetAppCatFlow
 * Description  : Get application categories' detailed flow information
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetAppCatFlow(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetDevFlow
 * Description  : Get devices' detailed flow information
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetDevFlow(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetAppInstFlow
 * Description  : Get application instances' detailed flow information
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetAppInstFlow(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetUrl
 * Description  : Get visited URL information
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetUrl(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlEnable
 * Description  : Enable/disable DPI feature.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlEnable(int enable);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlStatus
 * Description  : Show DPI status.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlStatus(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlSetParental
 * Description  : Set parental control configuration
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlSetParental(int mode, int id);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlConfig
 * Description  : Set DPI feature configuration
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlConfig(int option, int arg1);

/*
 *------------------------------------------------------------------------------
 * Function Name: int dpiCtlTable( char *tableFileName, DpictlTableType_t tableType )
 * Description  : Configures DPI Qos Tables into the Driver
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlTable( char *tableFileName, DpictlTableType_t tableType );

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlGetQos
 * Description  : Get the current DPI QoS mappings
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlGetQos( void );

/*
 *------------------------------------------------------------------------------
 * Function Name: dpiCtlAvailBw
 * Description  : Set the Downstream WAN Available Bandwidth
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpiCtlAvailBw( int kbps );

#endif  /* defined(__DPICTL_API_H_INCLUDED__) */
