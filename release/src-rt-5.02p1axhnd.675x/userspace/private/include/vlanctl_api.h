/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
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

#ifndef _VLANCTL_API_H_
#define _VLANCTL_API_H_

/*
 *------------------------------------------------------------------------------
 * Color encodings for console printing:
 *
 * To enable  color coded console printing: #define COLOR(clr_code)  clr_code
 * To disable color coded console printing: #define COLOR(clr_code)
 *
 * You may select a color specific to your subsystem by:
 *  #define CLRsys CLRg
 *
 *------------------------------------------------------------------------------
 */

#include "bcm_vlan.h"
#include "bcmctl_syslogdefs.h"

/* Defines the supported funtionality */
#define VLANCTL_LOG_COLOR_SUPPORTED
#define VLANCTL_ASSERT_SUPPORTED
#define VLANCTL_LOG_SUPPORTED
#define VLANCTL_ERROR_SUPPORTED

#define VLANCTL_DONT_CARE ~0

#if defined(VLANCTL_LOG_COLOR_SUPPORTED)
#define COLOR(clr_code)     clr_code
#else
#define COLOR(clr_code)
#endif

/* White background */
#define CLRr                COLOR("\e[0;31m")       /* red              */
#define CLRg                COLOR("\e[0;32m")       /* green            */
#define CLRy                COLOR("\e[0;33m")       /* yellow           */
#define CLRb                COLOR("\e[0;34m")       /* blue             */
#define CLRm                COLOR("\e[0;35m")       /* magenta          */
#define CLRc                COLOR("\e[0;36m")       /* cyan             */

/* blacK "inverted" background */
#define CLRrk               COLOR("\e[0;31;40m")    /* red     on blacK */
#define CLRgk               COLOR("\e[0;32;40m")    /* green   on blacK */
#define CLRyk               COLOR("\e[0;33;40m")    /* yellow  on blacK */
#define CLRmk               COLOR("\e[0;35;40m")    /* magenta on blacK */
#define CLRck               COLOR("\e[0;36;40m")    /* cyan    on blacK */
#define CLRwk               COLOR("\e[0;37;40m")    /* white   on blacK */

/* Colored background */
#define CLRcb               COLOR("\e[0;36;44m")    /* cyan    on blue  */
#define CLRyr               COLOR("\e[0;33;41m")    /* yellow  on red   */
#define CLRym               COLOR("\e[0;33;45m")    /* yellow  on magen */

/* Generic foreground colors */
#define CLRhigh             CLRm                    /* Highlight color  */
#define CLRbold             CLRcb                   /* Bold      color  */
#define CLRbold2            CLRym                   /* Bold2     color  */
#define CLRerr              CLRwk                   /* Error     color  */
#define CLRnorm             COLOR("\e[0m")          /* Normal    color  */
#define CLRnl               CLRnorm "\n"            /* Normal + newline */

#if defined(VLANCTL_LOG_SUPPORTED)
#define VLANCTL_LOGCODE(code)    code
#else
#define VLANCTL_LOGCODE(code)
#endif /*defined(VLANCTL_LOG_SUPPORTED)*/

#if defined(VLANCTL_ERROR_SUPPORTED)
#define VLANCTL_ERRORCODE(code)    code
#else
#define VLANCTL_ERRORCODE(code)
#endif /*defined(VLANCTL_ERROR_SUPPORTED)*/

#if defined(VLANCTL_ASSERT_SUPPORTED)
#define VLANCTL_ASSERTCODE(code)    code
#else
#define VLANCTL_ASSERTCODE(code)
#endif /*defined(VLANCTL_ASSERT_SUPPORTED)*/

typedef enum {
    VLANCTL_LOG_LEVEL_ERROR=0,
    VLANCTL_LOG_LEVEL_NOTICE,
    VLANCTL_LOG_LEVEL_INFO,
    VLANCTL_LOG_LEVEL_DEBUG,
    VLANCTL_LOG_LEVEL_MAX
} vlanCtl_logLevel_t;

typedef enum {
    VLANCTL_DIRECTION_RX=0,
    VLANCTL_DIRECTION_TX,
    VLANCTL_DIRECTION_MAX,
} vlanCtl_direction_t;

typedef enum {
    VLANCTL_POSITION_BEFORE=0,
    VLANCTL_POSITION_AFTER,
    VLANCTL_POSITION_APPEND, /* always before last if last is occupied. */
    VLANCTL_POSITION_LAST,   /* always at the bottom of the table */
    VLANCTL_POSITION_MAX,
} vlanCtl_ruleInsertPosition_t;

typedef enum {
    VLANCTL_ACTION_ACCEPT=0,
    VLANCTL_ACTION_DROP,
    VLANCTL_ACTION_MAX,
} vlanCtl_defaultAction_t;

typedef struct {
    int isRouted;
    int isMulticast;
    int isSwOnly;
} vlanCtl_createParams_t;

/**
 * Logging API: Activate by #defining VLANCTL_LOG_SUPPORTED
 **/
#define VLANCTL_LOG_NAME "vlanctl"


#define VLANCTL_LOG_FUNC() VLANCTL_LOG_DEBUG(" ")

#define VLANCTL_LOG_DEBUG(fmt, arg...)                                     \
{ \
    VLANCTL_LOGCODE( if(vlanCtl_logLevelIsEnabled(VLANCTL_LOG_LEVEL_DEBUG)) \
                         printf(CLRg "[DBG " "%s" "] %-10s: " fmt CLRnl, \
                             VLANCTL_LOG_NAME, __FUNCTION__, ##arg); ) \
    BCMCTL_SYSLOGCODE(vlanCtl, LOG_DEBUG, fmt, ##arg); \
}

#define VLANCTL_LOG_INFO(fmt, arg...)                                      \
{ \
    VLANCTL_LOGCODE( if(vlanCtl_logLevelIsEnabled(VLANCTL_LOG_LEVEL_INFO)) \
                         printf(CLRm "[INF " "%s" "] %-10s: " fmt CLRnl, \
                                VLANCTL_LOG_NAME, __FUNCTION__, ##arg); ) \
    BCMCTL_SYSLOGCODE(vlanCtl, LOG_INFO, fmt, ##arg); \
}

#define VLANCTL_LOG_NOTICE(fmt, arg...)                                    \
{ \
    VLANCTL_LOGCODE( if(vlanCtl_logLevelIsEnabled(VLANCTL_LOG_LEVEL_NOTICE)) \
                         printf(CLRb "[NTC " "%s" "] %-10s: " fmt CLRnl, \
                                VLANCTL_LOG_NAME, __FUNCTION__, ##arg); ) \
    BCMCTL_SYSLOGCODE(vlanCtl, LOG_NOTICE, fmt, ##arg); \
}


/**
 * Error Reporting API: Activate by #defining VLANCTL_ERROR_SUPPORTED
 **/

#define VLANCTL_LOG_ERROR(fmt, arg...)                                     \
{ \
    VLANCTL_ERRORCODE( if(vlanCtl_logLevelIsEnabled(VLANCTL_LOG_LEVEL_ERROR)) \
                           printf(CLRerr "[ERROR " "%s" "] %-10s, %d: " fmt CLRnl, \
                                  VLANCTL_LOG_NAME, __FUNCTION__, __LINE__, ##arg); ) \
    BCMCTL_SYSLOGCODE(vlanCtl, LOG_ERR, fmt, ##arg); \
}


/**
 * Assert API: Activate by #defining VLANCTL_ASSERT_SUPPORTED
 **/

#define VLANCTL_ASSERT(_cond)                                              \
    VLANCTL_ASSERTCODE( if(!(_cond))                                       \
                        { printf(CLRerr "[ASSERT " "%s" "] %-10s,%d: " #_cond CLRnl, \
                                 __FILE__, __FUNCTION__, __LINE__);     \
                        } )

int vlanCtl_init(void);
void vlanCtl_cleanup(void);

int vlanCtl_setLogLevel(vlanCtl_logLevel_t logLevel);
vlanCtl_logLevel_t vlanCtl_getLogLevel(void);
int vlanCtl_logLevelIsEnabled(vlanCtl_logLevel_t logLevel);

#if defined(BCMCTL_SYSLOG_SUPPORTED)
DECLARE_setSyslogLevel(vlanCtl);
DECLARE_getSyslogLevel(vlanCtl);
DECALRE_isSyslogLevelEnabled(vlanCtl);
DECLARE_setSyslogMode(vlanCtl);
DECLARE_isSyslogEnabled(vlanCtl);
#endif /* BCMCTL_SYSLOG_SUPPORTED */

int vlanCtl_createVlanInterface(const char *realDevName, unsigned int vlanDevId,
                                int isRouted, int isMulticast);
int vlanCtl_createVlanInterfaceExt(const char *realDevName, unsigned int vlanDevId,
                                vlanCtl_createParams_t *createParamsP);

int vlanCtl_createVlanInterfaceByName(char *realDevName, char *vlanDevName,
                                      int isRouted, int isMulticast);
int vlanCtl_createVlanInterfaceByNameExt(char *realDevName, char *vlanDevName,
                                         vlanCtl_createParams_t *createParamsP);

int vlanCtl_deleteVlanInterface(char *vlanDevName);
int vlanCtl_insertTagRule(const char *realDevName, vlanCtl_direction_t tableDir, unsigned int nbrOfTags,
                          vlanCtl_ruleInsertPosition_t position, unsigned int posTagRuleId,
                          unsigned int *tagRuleId);
int vlanCtl_removeAllTagRule(char *vlanDevName);
int vlanCtl_removeTagRule(char *realDevName, vlanCtl_direction_t tableDir,
                          unsigned int nbrOfTags, unsigned int tagRuleId);
int vlanCtl_removeTagRuleByFilter(char *realDevName, vlanCtl_direction_t tableDir,
                                  unsigned int nbrOfTags);
int vlanCtl_dumpRuleTable(char *realDevName, vlanCtl_direction_t tableDir, unsigned int nbrOfTags);

int vlanCtl_dumpAllRules(void);

int vlanCtl_getNbrOfRulesInTable(char *realDevName, vlanCtl_direction_t tableDir,
                                 unsigned int nbrOfTags, unsigned int *nbrOfRules);

int vlanCtl_setDefaultVlanTag(char *realDevName, vlanCtl_direction_t tableDir, unsigned int nbrOfTags,
                              unsigned int defaultTpid, unsigned int defaultPbits, unsigned int defaultCfi,
                              unsigned int defaultVid);

int vlanCtl_setDscpToPbits(char *realDevName, unsigned int dscp, unsigned int pbits);
int vlanCtl_dumpDscpToPbits(char *realDevName, unsigned int dscp);

int vlanCtl_setTpidTable(char *realDevName, unsigned int *tpidTable);
int vlanCtl_dumpTpidTable(char *realDevName);

int vlanCtl_dumpLocalStats(char *vlanDevName);

int vlanCtl_setIfSuffix(char *ifSuffix);

int vlanCtl_setDefaultAction(const char *realDevName, vlanCtl_direction_t tableDir, unsigned int nbrOfTags,
                             vlanCtl_defaultAction_t defaultAction, char *defaultRxVlanDevName);

int vlanCtl_setRealDevMode(char *realDevName, bcmVlan_realDevMode_t mode);

void vlanCtl_setIptvOnly(void);

int vlanCtl_runTest(unsigned int testNbr, char *rxVlanDevName, char *txVlanDevName);

void vlanCtl_initTagRule(void);

void vlanCtl_filterOnSkbPriority(unsigned int priority);
void vlanCtl_filterOnSkbMarkFlowId(unsigned int flowId);
void vlanCtl_filterOnSkbMarkPort(unsigned int port);
void vlanCtl_filterOnEthertype(unsigned int etherType);
void vlanCtl_filterOnIpProto(unsigned int ipProto);
void vlanCtl_filterOnDscp(unsigned int dscp);
void vlanCtl_filterOnDscp2Pbits(unsigned int dscp2pbits);
void vlanCtl_filterOnVlanDeviceMacAddr(unsigned int acceptMulticast);
int vlanCtl_filterOnFlags(unsigned int flags);

int vlanCtl_filterOnTagPbits(unsigned int pbits, unsigned int tagIndex);
int vlanCtl_filterOnTagCfi(unsigned int cfi, unsigned int tagIndex);
int vlanCtl_filterOnTagVid(unsigned int vid, unsigned int tagIndex);
int vlanCtl_filterOnTagEtherType(unsigned int etherType, unsigned int tagIndex);

int vlanCtl_filterOnTxVlanDevice(char *vlanDevName);
int vlanCtl_filterOnRxRealDevice(char *realDevName);

int vlanCtl_cmdPopVlanTag(void);
int vlanCtl_cmdPushVlanTag(void);
#if defined(CONFIG_BCM_VLAN_AGGREGATION)
int vlanCtl_cmdDeaggrVlanTag(void);
#endif

int vlanCtl_cmdSetEtherType(unsigned int etherType);

int vlanCtl_cmdSetTagPbits(unsigned int pbits, unsigned int tagIndex);
int vlanCtl_cmdSetTagCfi(unsigned int cfi, unsigned int tagIndex);
int vlanCtl_cmdSetTagVid(unsigned int vid, unsigned int tagIndex);
int vlanCtl_cmdSetTagEtherType(unsigned int etherType, unsigned int tagIndex);

int vlanCtl_cmdCopyTagPbits(unsigned int sourceTagIndex, unsigned int targetTagIndex);
int vlanCtl_cmdCopyTagCfi(unsigned int sourceTagIndex, unsigned int targetTagIndex);
int vlanCtl_cmdCopyTagVid(unsigned int sourceTagIndex, unsigned int targetTagIndex);
int vlanCtl_cmdCopyTagEtherType(unsigned int sourceTagIndex, unsigned int targetTagIndex);

int vlanCtl_cmdDscpToPbits(unsigned int tagIndex);
int vlanCtl_cmdSetDscp(unsigned int dscp);

int vlanCtl_cmdDropFrame(void);

int vlanCtl_cmdSetSkbPriority(unsigned int priority);
int vlanCtl_cmdSetSkbMarkPort(unsigned int port);
int vlanCtl_cmdSetSkbMarkQueue(unsigned int queue);
int vlanCtl_cmdSetSkbMarkQueueByPbits(void);
int vlanCtl_cmdSetSkbMarkFlowId(unsigned int flowId);

int vlanCtl_cmdOvrdLearningVid(unsigned int vid);

int vlanCtl_cmdContinue(void);

int vlanCtl_setReceiveVlanDevice(char *vlanDevName);

int vlanCtl_createVlanFlows(char *rxVlanDevName, char *txVlanDevName);
int vlanCtl_deleteVlanFlows(char *rxVlanDevName, char *txVlanDevName);

int vlanCtl_setVlanRuleTableType(unsigned int type);

int vlanCtl_setDropPrecedence(bcmVlan_flowDir_t dir,
                              bcmVlan_dpCode_t dpCode);
#endif /* _VLANCTL_API_H_ */

