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

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "os_defs.h"

#include "tmctl_api.h"
#include "tmctl_api_trace.h"
#include "cms_actionlog.h"

//#define CC_TMCTL_TRACE_DEFAULT_ON
#define CC_TMCTL_TRACE_DEFAULT_PRINT_TO_FILE

#define TMCTL_TRACE_PREFIX_NAME    "tmctl"
#define TMCTL_TRACE_IF_NAME_SIZE   16
#define TMCTL_TRACE_FILE_NAME      "/var/tmctl.log"

#define __error(fmt, arg...) fprintf(stderr, "ERROR[%s, %d]: " fmt, __FUNCTION__, __LINE__, ##arg)

#define __print(fmt, arg...)                                            \
    do {                                                                \
        if(tmctlApiTraceEnable_g) {                                     \
        if(tmctlApiTracePrintToFile_g) {                                \
            FILE *fd = fopen(TMCTL_TRACE_FILE_NAME, "a+");              \
            if(fd == NULL) {                                            \
                __error("Could not open %s\n", TMCTL_TRACE_FILE_NAME);  \
            } else {                                                    \
                fprintf(fd, fmt, ##arg);                                \
                fclose(fd);                                             \
            }                                                           \
        } else {                                                        \
            fprintf(stdout, fmt, ##arg);                                \
        }                                                               \
        }                                                               \
        calLog_library(fmt, ##arg);                                     \
    } while(0)

#define __trace(cmd, fmt, arg...)                               \
    do {                                                        \
        if(tmctlApiTraceEnable_g || cmsActionTraceEnable_g)     \
            __print(TMCTL_TRACE_PREFIX_NAME " " #cmd " " fmt, ##arg); \
        else                                                    \
            return TMCTL_SUCCESS;                               \
    } while(0)

#if defined(CC_TMCTL_TRACE_DEFAULT_ON)
static int tmctlApiTraceEnable_g = 1;
#else
static int tmctlApiTraceEnable_g = 0;
#endif

#if defined(CC_TMCTL_TRACE_DEFAULT_PRINT_TO_FILE)
static int tmctlApiTracePrintToFile_g = 1;
#else
static int tmctlApiTracePrintToFile_g = 0;
#endif

#ifdef CMS_ACTION_LOG
static int cmsActionTraceEnable_g = 1;
#else
static int cmsActionTraceEnable_g = 0;
#endif

static void __tmctlIfToStr(tmctl_devType_e devType, tmctl_if_t *if_p, char *ifName)
{
    if(tmctlApiTraceEnable_g || cmsActionTraceEnable_g)
    {
        if (devType == TMCTL_DEV_ETH)
        {
            strncpy(ifName, if_p->ethIf.ifname, TMCTL_TRACE_IF_NAME_SIZE - 1);
        }
        else if (devType == TMCTL_DEV_XTM)
        {
            strncpy(ifName, if_p->xtmIf.ifname, TMCTL_TRACE_IF_NAME_SIZE - 1);
        }
        else if (devType == TMCTL_DEV_EPON)
        {
            sprintf(ifName, "%d", if_p->eponIf.llid);
        }
        else if (devType == TMCTL_DEV_GPON)
        {
            sprintf(ifName, "%d", if_p->gponIf.tcontid);
        }
        else if (devType == TMCTL_DEV_SVCQ)
        {
            sprintf(ifName, "%d", 0);
        }
    }
}

static void __shaperCfgTrace(tmctl_shaper_t *pShaperCfg)
{
    __print("--shapingrate %d --burstsize %d --minrate %d\n",
            pShaperCfg->shapingRate, pShaperCfg->shapingBurstSize,
            pShaperCfg->minRate);
}

static void __queueCfgTrace(tmctl_queueCfg_t *pQueueCfg)
{
    __print("--qid %d --priority %d --qsize %d --weight %d --minbufs %d --schedmode %d ",
            pQueueCfg->qid, pQueueCfg->priority, pQueueCfg->qsize,
            pQueueCfg->weight, pQueueCfg->minBufs, pQueueCfg->schedMode);

    __shaperCfgTrace(&pQueueCfg->shaper);
}

static void __queueProfileTrace(tmctl_queueProfile_t *pQueueProf)
{
    __print("--redminthr %d --redmaxthr %d --redpct %d\n",
            pQueueProf->minThreshold, pQueueProf->maxThreshold,
            pQueueProf->dropProb);
}

static void __queueDropAlgTrace(tmctl_queueDropAlg_t *pQueueCfg)
{
    __print("--dropalg %d --qprofid %d --qprofidhi %d --priomask0 0x%08X --priomask1 0x%08X\n",
            pQueueCfg->dropAlgorithm, pQueueCfg->queueProfileIdLo, pQueueCfg->queueProfileIdHi,
            pQueueCfg->priorityMask0, pQueueCfg->priorityMask1);
}

static void __queueDropAlgExtTrace(tmctl_queueDropAlg_t *pQueueCfg)
{
    tmctl_queueDropAlgExt_t *dropAlgLo_p = &pQueueCfg->dropAlgLo;
    tmctl_queueDropAlgExt_t *dropAlgHi_p = &pQueueCfg->dropAlgHi;

    __print("--dropalg %d --loredminthr %d --loredmaxthr %d --loredpct %d "
            "--hiredminthr %d --hiredmaxthr %d --hiredpct %d "
            "--priomask0 0x%08X --priomask1 0x%08X\n",
            pQueueCfg->dropAlgorithm, dropAlgLo_p->redMinThreshold, dropAlgLo_p->redMaxThreshold,
            dropAlgLo_p->redPercentage, dropAlgHi_p->redMinThreshold,
            dropAlgHi_p->redMaxThreshold, dropAlgHi_p->redPercentage,
            pQueueCfg->priorityMask0, pQueueCfg->priorityMask1);
}

static void __dscpToPbitTrace(tmctl_dscpToPbitCfg_t* cfg_p)
{
    int i;
    
    for (i = 0; i < 64; i++)
    {
        __print("--dscp[%d]=%d \n", i,cfg_p->dscp[i]);
    }
}

static void __pbitToQTrace(tmctl_pbitToQCfg_t* cfg_p)
{
    int i;
    
    for (i = 0; i < 8; i++)
    {
        __print("--pbit[%d]=%d \n", i,cfg_p->pbit[i]);
    }
}

void tmctl_configTrace(int enable, int printToFile)
{
    fprintf(stdout, "tmctl trace is %s, print to %s\n", enable ? "enabled" : "disabled",
            (printToFile) ? TMCTL_TRACE_FILE_NAME : "stdout");

    tmctlApiTraceEnable_g = (enable) ? 1 : 0;
    tmctlApiTracePrintToFile_g = (printToFile) ? 1 : 0;
}

tmctl_ret_e tmctl_portTmInitTrace(tmctl_devType_e devType,
                                  tmctl_if_t *if_p, uint32_t cfgFlags, int numQueues)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(porttminit, "--devtype %d --if %s --flag %d, --numqueues%d\n",
            devType, ifName, cfgFlags, numQueues);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_portTmUninitTrace(tmctl_devType_e devType,
                                    tmctl_if_t *if_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(porttmuninit, "--devtype %d --if %s --flag 0\n",
            devType, ifName);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_getQueueCfgTrace(tmctl_devType_e devType,
                                   tmctl_if_t *if_p, int queueId, tmctl_queueCfg_t *qcfg_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(getqcfg, "--devtype %d --if %s --qid %d\n",
            devType, ifName, queueId);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_setQueueCfgTrace(tmctl_devType_e devType,
                                   tmctl_if_t *if_p, tmctl_queueCfg_t *qcfg_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(setqcfg, "--devtype %d --if %s ", devType, ifName);

    __queueCfgTrace(qcfg_p);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_delQueueCfgTrace(tmctl_devType_e devType,
                                   tmctl_if_t *if_p, int queueId)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(delqcfg, "--devtype %d --if %s --qid %d\n",
            devType, ifName, queueId);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_getPortShaperTrace(tmctl_devType_e devType,
                                     tmctl_if_t *if_p, tmctl_shaper_t *shaper_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(getportshaper, "--devtype %d --if %s\n", devType, ifName);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_setPortShaperTrace(tmctl_devType_e devType,
                                     tmctl_if_t *if_p, tmctl_shaper_t *shaper_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(setportshaper, "--devtype %d --if %s ", devType, ifName);

    __shaperCfgTrace(shaper_p);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_getOverAllShaperTrace(tmctl_devType_e devType,
                                     tmctl_if_t *if_p, tmctl_shaper_t *shaper_p)
{
    __trace(getorlshaper, "--devtype %d\n", devType);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_setOverAllShaperTrace(tmctl_devType_e devType, tmctl_shaper_t *shaper_p)
{
    __trace(setorlshaper, "--devtype %d \n", devType);

    __shaperCfgTrace(shaper_p);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_linkOverAllShaperTrace(tmctl_devType_e devType, tmctl_if_t *if_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(linkorlshaper, "--devtype %d --if %s \n", devType, ifName);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_unlinkOverAllShaperTrace(tmctl_devType_e devType, tmctl_if_t *if_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(unlinkorlshaper, "--devtype %d --if %s \n", devType, ifName);

    return TMCTL_SUCCESS;
}


tmctl_ret_e tmctl_allocQueueProfileIdTrace(int* queueProfileId_p)
{
    __trace(allocqprof, "\n");

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_freeQueueProfileIdTrace(int queueProfileId)
{
    __trace(freeqprof, "--qprofid %d\n", queueProfileId);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_getQueueProfileTrace(int queueProfId,
                                       tmctl_queueProfile_t *qProf_p)
{
    __trace(getqprof, "--qprofid %d\n", queueProfId);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_setQueueProfileTrace(int queueProfId,
                                       tmctl_queueProfile_t *qProf_p)
{
    __trace(setqprof, "--qprofid %d ", queueProfId);

    __queueProfileTrace(qProf_p);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_getQueueDropAlgTrace(tmctl_devType_e devType,
                                       tmctl_if_t *if_p, int queueId, tmctl_queueDropAlg_t *dropAlg_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(getqdropalg, "--devtype %d --if %s --qid %d\n", devType, ifName, queueId);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_setQueueDropAlgTrace(tmctl_devType_e devType,
                                       tmctl_if_t *if_p, int queueId, tmctl_queueDropAlg_t *dropAlg_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(setqdropalg, "--devtype %d --if %s --qid %d ", devType, ifName, queueId);

    __queueDropAlgTrace(dropAlg_p);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_setQueueSizeTrace(tmctl_devType_e devType,
                                       tmctl_if_t *if_p, int queueId, int qsize)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(setqsize, "--devtype %d --if %s --qid %d --qsize %d\n", devType, ifName, queueId, qsize);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_setQueueSizeShaperTrace(tmctl_devType_e          devType,
                                     tmctl_if_t*        if_p,
                                     int                queueId,
                                     tmctl_shaper_t     *shaper_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(setqshaper, "--devtype %d --if %s --qid %d --minRate %d --shapingRate %d --shapingBurstSize %d\n", 
                    devType, ifName, queueId, shaper_p->minRate, shaper_p->shapingRate, shaper_p->shapingBurstSize);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_getXtmChannelDropAlgTrace(tmctl_devType_e devType,
                                            int channelId, tmctl_queueDropAlg_t *dropAlg_p)
{
    __trace(getqdropalg, "--devtype %d --if xtm --qid %d\n", devType, channelId);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_setXtmChannelDropAlgTrace(tmctl_devType_e devType,
                                            int channelId, tmctl_queueDropAlg_t *dropAlg_p)
{
    __trace(setqdropalg, "--devtype %d --if xtm --qid %d ", devType, channelId);

    __queueDropAlgTrace(dropAlg_p);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_getQueueStatsTrace(tmctl_devType_e devType,
                                     tmctl_if_t *if_p, int queueId, tmctl_queueStats_t *stats_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(getqstats, "--devtype %d --if %s --qid %d\n", devType, ifName, queueId);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_getPortTmParmsTrace(tmctl_devType_e devType,
                                      tmctl_if_t *if_p, tmctl_portTmParms_t *tmParms_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(getporttmparms, "--devtype %d --if %s\n", devType, ifName);

    return TMCTL_SUCCESS;
}


tmctl_ret_e tmctl_setQueueDropAlgExtTrace(tmctl_devType_e devType,
                                          tmctl_if_t *if_p,
                                          int queueId,
                                          tmctl_queueDropAlg_t *dropAlg_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(setqdropalgx, "--devtype %d --if %s --qid %d ", devType, ifName, queueId);

    __queueDropAlgExtTrace(dropAlg_p);

    return TMCTL_SUCCESS;

}

tmctl_ret_e tmctl_getDscpToPbitTrace(void)
{
    __trace(getdscptopbit, "\n");

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_setDscpToPbitTrace(tmctl_dscpToPbitCfg_t* cfg_p)
{
    __trace(setdscptopbit, "\n");
    
    __dscpToPbitTrace(cfg_p);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_getPbitToQTrace(tmctl_devType_e devType, 
                                 tmctl_if_t* if_p, 
                                 tmctl_pbitToQCfg_t* cfg_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(getpbittoq, "--devtype %d --if %s \n", devType, ifName);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_setPbitToQTrace(tmctl_devType_e devType, 
                                 tmctl_if_t* if_p, 
                                 tmctl_pbitToQCfg_t* cfg_p)
{
    char ifName[TMCTL_TRACE_IF_NAME_SIZE];

    __tmctlIfToStr(devType, if_p, ifName);

    __trace(setpbittoq, "--devtype %d --if %s \n", devType, ifName);

    __pbitToQTrace(cfg_p);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_getForceDscpToPbitTrace(tmctl_dir_e dir, BOOL* enable_p)
{
    __trace(getforcedscptopbit, "--dir %d \n", dir);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_setForceDscpToPbitTrace(tmctl_dir_e dir, BOOL* enable_p)
{
    __trace(setforcedscptopbit, "--dir %d --enable %d \n", dir, (*enable_p));
    
    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_getPktBasedQosTrace(tmctl_dir_e dir, 
                                  tmctl_qosType_e type, 
                                  BOOL* enable_p)
{
    __trace(getpktbasedqos, "--dir %d --type %d \n", dir, type);

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_setPktBasedQosTrace(tmctl_dir_e dir, 
                                  tmctl_qosType_e type, 
                                  BOOL* enable_p)
{
  __trace(setpktbasedqos, 
      "--dir %d --type %d --enable %d \n", dir, type, (*enable_p));

  return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_createPolicerTrace(tmctl_policer_t *policer_p)
{
  __trace(createpolicer, 
      "--dir %d --pid %d --cir %d \n", policer_p->dir, policer_p->policerId, policer_p->cir);

  return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_modifyPolicerTrace(tmctl_policer_t *policer_p)
{
  __trace(modifypolicer, 
      "--dir %d --pid %d --cir %d \n", policer_p->dir, policer_p->policerId, policer_p->cir);

  return TMCTL_SUCCESS;
}
tmctl_ret_e tmctl_deletePolicerTrace(tmctl_dir_e dir, int policerId)
{
  __trace(deletepolicer, 
      "--dir %d --policerId %d \n", dir, policerId);

  return TMCTL_SUCCESS;
}


