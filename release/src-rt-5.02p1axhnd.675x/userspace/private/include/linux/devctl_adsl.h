/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2011:proprietary:standard

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

#ifndef __DEVCTL_ADSL_H__
#define __DEVCTL_ADSL_H__

#define	XDSL_CTL_API

/*!\file devctl_adsl.h
 * \brief Header file for the user mode ADSL device control library API.
 *  This is in the devCtl library.
 *
 * These API are called by user applications to perform ADSL driver operations.
 * These API make Linux ioctl calls to ADSL driver. 
 *
 */

#ifdef BRCM_CMS_BUILD
#include "cms.h"
#include "cms_core.h"
#else
#include "os_defs.h"
#endif
#include "adsldrv.h"

CmsRet xdslCtl_SendHmiMessage(
            unsigned char lineId,
            unsigned char *header,
            unsigned short headerSize,
            unsigned char *payload,
            unsigned short payloadSize,
            unsigned char *replyMessage,
            unsigned short replyMaxMessageSize);

CmsRet xdslCtl_OpenEocIntf(unsigned char lineId, int eocMsgType, char *pIfName);
CmsRet xdslCtl_CloseEocIntf(unsigned char lineId, int eocMsgType, char *pIfName);

/** Process Diagnostic Command Frame
 *
 * This function is called to pass a diag command to ADSL driver.
 *
 * @param diagBuf (IN)  A pointer to diag command buffer.
 * @param diagBufLen (IN)  Length of diag command buffer.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslDiagProcessCommandFrame(void *diagBuf, int diagBufLen);
CmsRet xdslCtl_DiagProcessCommandFrame(unsigned char lineId, void *diagBuf, int diagBufLen);


/** Process Diagnostic Debug Command
 *
 * This function is called to pass a diag debug command to ADSL driver.
 *
 * @param cmd   (IN)  A command
 * @param cmdId (IN)  CommandId
 * @param param1 (IN)  Parameter 1
 * @param param2 (IN)  Parameter 2
 *
 * @return CmsRet enum.
 */
CmsRet BcmAdsl_DiagProcessDbgCommand(UINT16 cmd, UINT16 cmdId, UINT32 param1, UINT32 param2);
CmsRet xdslCtl_DiagProcessDbgCommand(UINT8 lineId, UINT16 cmd, UINT16 cmdId, UINT32 param1, UINT32 param2);


/** Check ADSL adapter installation
 *
 * This function is called to check if ADSL adapter is installed.
 * If installed, CSMRET_SUCCESS is returned.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslCheck(void);
CmsRet xdslCtl_Check(unsigned char lineId);


/** Initializes ADSL.
 *
 * This function is called to do ADSL interface initialization.
 *
 * @param pFnNotifyCb (IN)  A call back function pointer for ADSL driver to notify it's link state.
 * @param ulParm (IN)  0
 * @param ulParm (IN) Pointer to ADSL configuration profile (adslCfgProfile). 
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslInitialize(ADSL_FN_NOTIFY_CB pFnNotifyCb, void *pParm, adslCfgProfile *pAdslCfg);
CmsRet xdslCtl_Initialize(UINT8 lineId, ADSL_FN_NOTIFY_CB pFnNotifyCb, void *pParm, adslCfgProfile *pAdslCfg);


/** Uninitializes ADSL.
 *
 * This function is called to uninitialize ADSL interface.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslUninitialize(void);
CmsRet xdslCtl_Uninitialize(unsigned char lineId);

/** Start ADSL Connection.
 *
 * This function is called to start ADSL connection.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslConnectionStart(void);
CmsRet xdslCtl_ConnectionStart(unsigned char lineId);

/** Stop ADSL Connection.
 *
 * This function is called to stop ADSL connection.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslConnectionStop(void);
CmsRet xdslCtl_ConnectionStop(unsigned char lineId);

/** Get ADSL Phy Addresses.
 *
 * This function is called to get ADSL PHY channel addresses.
 *
 * @param PADSL_CHANNEL_ADDR (IN/OUT)  A pointer to ADSL_CHANNEL_ADDR to store Fast/Interleave channel address.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslGetPhyAddresses(PADSL_CHANNEL_ADDR pChannelAddr);
CmsRet xdslCtl_GetPhyAddresses(UINT8 lineId, PADSL_CHANNEL_ADDR pChannelAddr);

/** Set ADSL Phy Addresses.
 *
 * This function is called to set ADSL PHY channel addresses.
 *
 * @param PADSL_CHANNEL_ADDR (IN)  A pointer to ADSL_CHANNEL_ADDR that contain Fast/Interleave channel address.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslSetPhyAddresses(PADSL_CHANNEL_ADDR pChannelAddr);
CmsRet xdslCtl_SetPhyAddresses(UINT8 lineId, PADSL_CHANNEL_ADDR pChannelAddr);

/** Map ATM PORT IDs
 *
 * This function is called to do mapping between ATM port and ADSL channels.
 *
 * @param usAtmFastPortId(IN)  ATM port mapped to fast ADSL channel.
 * @param usAtmInterleavedPortId(IN)  ATM port mapped to interleaved ADSL channel.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslMapAtmPortIDs(UINT16 usAtmFastPortId, UINT16 usAtmInterleavedPortId);
CmsRet xdslCtl_MapAtmPortIDs(UINT8 lineId, UINT16 usAtmFastPortId, UINT16 usAtmInterleavedPortId);

/** Get ADSL Connection Info
 *
 * This function is called to get ADSL Connection Info (statistics, configuration...).
 *
 * @param usAtmFastPortId(OUT)  Pointer to ADSL_CONNECTION_INFO
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslGetConnectionInfo(PADSL_CONNECTION_INFO pConnInfo);
CmsRet xdslCtl_GetConnectionInfo(UINT8 lineId, PADSL_CONNECTION_INFO pConnInfo);

/** Set ADSL Object Value
 *
 * This function is called to set an ADSL object in ADSL driver.
 *
 * @param objId(IN) A string that identifies the object caller wishes to set.
 * @param objIdLen(IN) Length of the object ID.
 * @param dataBuf(IN) Data buffer to set to object ID.
 * @param dataBufLen(IN/OUT) Data buffer length.  Driver set success set length. 0 if error occured.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslSetObjectValue(char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
CmsRet xdslCtl_SetObjectValue(unsigned char lineId, char *objId, int objIdLen, char *dataBuf, long *dataBufLen);

/** Set ADSL Object Value
 *
 * This function is called to set an ADSL object in ADSL driver.
 *
 * @param objId(IN) A string that identifies the object caller wishes to set.
 * @param objIdLen(IN) Length of the object ID.
 * @param dataBuf(IN) Data buffer to set to object ID.
 * @param dataBufLen(IN/OUT) Data buffer length.  Driver set success set length. 0 if error occured.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslGetObjectValue(char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
CmsRet xdslCtl_GetObjectValue(unsigned char lineId, char *objId, int objIdLen, char *dataBuf, long *dataBufLen);

/** Start BERT test with totalBits specified
 *
 * This function is called to start ADSL BERT test.
 *
 * @param totalBits(IN)
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslStartBERT(UINT32  totalBits);
CmsRet xdslCtl_StartBERT(unsigned char lineId, UINT32  totalBits);

/** Stop BERT test
 *
 * This function is called to stop ADSL BERT test.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslStopBERT(void);
CmsRet xdslCtl_StopBERT(unsigned char lineId);

/** Start BERT test with duration in seconds specified
 *
 * This function is called to start ADSL BERT test.
 *
 * @param bertSec(IN) Number of seconds BERT test is to be run.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslBertStartEx(UINT32  bertSec);
CmsRet xdslCtl_BertStartEx(unsigned char lineId, UINT32  bertSec);

/** Stop BERT test (started with devCtl_adslBertStartEx)
 *
 * This function is called to stop ADSL BERT test.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslBertStopEx(void);
CmsRet xdslCtl_BertStopEx(unsigned char lineId);

/** Configure ADSL 
 *
 * This function is called to configure ADSL interface.
 *
 * @param pAdslCfg(IN) Pointer to adslCfgProfile which contains ADSL configuration info.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslConfigure(adslCfgProfile *pAdslCfg);
CmsRet xdslCtl_Configure(unsigned char lineId, adslCfgProfile *pAdslCfg);

/** Get ADSL version
 *
 * This function is called to get ADSL software version (driver and PHY version).
 *
 * @param pAdslVer(OUT) Pointer to ADSL version info.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslGetVersion(adslVersionInfo *pAdslVer);
CmsRet xdslCtl_GetVersion(unsigned char lineId, adslVersionInfo *pAdslVer);

/** Set SDRAM Base Address
 *
 * This function is called to set SDRAM base address.
 *
 * @param pAddr(IN) Pointer to address of base SDRAM.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslSetSDRAMBaseAddr(void *pAddr);
CmsRet xdslCtl_SetSDRAMBaseAddr(unsigned char lineId, void *pAddr);

/** Reset Statistics Counters
 *
 * This function is called to clear ADSL statistics counters.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslResetStatCounters(void);
CmsRet xdslCtl_ResetStatCounters(unsigned char lineId);

/** Set Test Mode
 *
 * This function is called to set ADSL test mode.
 *
 * @param testMode(IN) Pointer to ADSL_TEST_MODE
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslSetTestMode(ADSL_TEST_MODE testMode);
CmsRet xdslCtl_SetTestMode(unsigned char lineId, ADSL_TEST_MODE testMode);

/** ADSL Select Tones
 *
 * This function is called to set ADSL test mode.
 *
 * @param xmtStartTone(IN) Transmit start tone
 * @param xmtNumTones(IN) Number of transmit start tone
 * @param rcvStartTone(IN) Receive start tone
 * @param rcvNumTones(IN) Number of receive tone
 * @param xmtToneMap(OUT) Pointer to transmit tone map
 * @param rcvToneMap(OUT) Pointer to receive tone map
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_adslSelectTones(int xmtStartTone, int xmtNumTones, int rcvStartTone,
                              int rcvNumTones, char *xmtToneMap, char *rcvToneMap);
CmsRet xdslCtl_SelectTones(unsigned char lineId, int xmtStartTone, int xmtNumTones,
                                int rcvStartTone, int rcvNumTones, char *xmtToneMap, char *rcvToneMap);

/** Get ADSL Constellation Points
 *
 * This function is called to get constellation points.
 *
 * @param toneId(IN) Tone ID
 * @param pointBuf(OUT) Pointer to ADSL_CONSTELLATION_POINT
 * @param numPoints(IN) Number of points to get
 *
 * @return number of constellation points if success; 0 if error.
 */
int devCtl_adslGetConstellationPoints(int toneId, ADSL_CONSTELLATION_POINT *pointBuf, int numPoints);
int xdslCtl_GetConstellationPoints(unsigned char lineId, int toneId, ADSL_CONSTELLATION_POINT *pointBuf, int numPoints);

CmsRet xdslCtl_CallBackDrv(unsigned char lineId);

#ifdef BRCM_CMS_BUILD
void xdslUtil_CfgProfileInit(adslCfgProfile * pAdslCfg, WanDslIntfCfgObject * pDlIntfCfg);
void xdslUtil_IntfCfgInit(adslCfgProfile *pAdslCfg, WanDslIntfCfgObject *pDlIntfCfg);

#ifdef DMP_DEVICE2_DSL_1
void xdslUtil_IntfCfgInit_dev2(adslCfgProfile *pAdslCfg,  Dev2DslLineObject *pDslLineObj);
void xdslUtil_CfgProfileInit_dev2(adslCfgProfile * pAdslCfg, Dev2DslLineObject * pDlIntfCfg);
#endif
#endif

#ifdef SUPPORT_SELT

/** Start SELT Test *
 * This function is called to initiate a SELT TEST
 *
 * @param lineId(IN) Bonding line id
 * @param steps(IN)  Pointer to unsigned char to control steps 
 *             performed by the SELT Test.  NULL leaves steps
 *             unchanged
 * @param cfg(IN)    Pointer to the packed SELT signal 
 *           configuration.  NULL leaves cfg unchanged
 *
 * @return CMS_SUCCESS if ok, CMS error codes if not
 */

CmsRet xdslCtl_StartSeltTest(unsigned char lineId, unsigned char *steps, unsigned int *cfg);

/** Check test is complete *
 * This function is called to check a SELT is completed
 *
 * @param lineId(IN)     Bonding line id
 * @param complete(OUT)  Pointer to int to indicate completion 
 *                (1= complete, 0= not complete)
 *
 * @return CMS_SUCCESS if ok, CMS error codes if not
 */

CmsRet xdslCtl_isSeltTestComplete(unsigned char lineId, int *complete);

/** Configure SELT Test parameters *
 * This function is called to configure selt signal paramaters
 *
 * @param lineId(IN)    Bonding line id
 * @param psdLevel(IN)  Float to signal PSD level (dBm/Hz)
 * @param duration(IN)  Float to signal duration  (sec) 
 * @param maxFreq(IN)   Float to max frequency (Hz) 
 * @param steps(IN)     Pointer to bitmap to selt test steps 
 *                            (defined in AdslMibDef.h), NULL
 *                            uses defaults.
 *               
 * @return CMS_SUCCESS if ok, CMS error codes if not
 */

CmsRet xdslCtl_SeltConfigureTest(unsigned char lineId, float psdLevel, float duration, float maxFreq, unsigned char *steps);

/** Fetch data from driver, save input and output of selt
 *  algorithm and runs algorithm
 * This function is called to trigger the run of the algorithm 
 * and saving data and results 
 *  
 * @param lineId(IN)    Bonding line id
 * @param calibrate(IN) Indicate that the measurement is used 
 *                 for calibration.   Results are saved in the
 *                 folderOut folder
 * @param process(IN) Indicate that the post-processing is 
 *               required
 * @param folderIn(IN)  String to folder including the input 
 *                data.  if not NULL, the data is not fetched
 *                from the driver.
 * @param folderOut(IN)  String to result destination folder. 
 *                  Always need to be passed.
 * @param errReport(OUT)  Pointer to file system error report
 *               
 * @return CMS_SUCCESS if ok, CMS error codes if not
 */


CmsRet xdslCtl_SeltFetchAndPostProcess(unsigned char lineId, int calibrate, int process, char *folderIn, char *folderOut, int *errReport);

/** Get SELT Algorithm version information  *
 * This function is called to get selt algorithm version 
 * information 
 *
 * @param version(IN) String Ptr where to write the version
 *
 * @return CMS_SUCCESS or CMSRET_INVALID_ARGUMENTS if version is 
 *         NULL
*/
CmsRet xdslCtl_GetSeltAlgorithmVersion(char *version);

/** Stop SELT measurement *
 * This function is called to get stop the SELT measurement 
 * information 
 *
 * @param lineId(IN)    Bonding line id 
 *  
 * @return CMS_SUCCESS or CMSRET_INVALID_ARGUMENTS if version is 
 *         NULL
*/
CmsRet xdslCtl_StopSeltMeasurement(unsigned char lineId);

#endif

#endif /* __DEVCTL_ADSL_H__ */
