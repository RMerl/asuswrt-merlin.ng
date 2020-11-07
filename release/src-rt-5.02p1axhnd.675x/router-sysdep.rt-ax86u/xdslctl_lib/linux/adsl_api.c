/***********************************************************************
 *
 *  Copyright (c) 2006  Broadcom Corporation
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


/* Includes. */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include "cms.h"
#include "adsl_api_trace.h"
#include "adslctlapi.h"
#include "devctl_adsl.h"

/* Bert status includes: state, elapse time, test result statistics */
CmsRet BcmAdslCtl_GetBertStatus(adslBertStatusEx *bertStatus)
{
   CmsRet nRet;
   adslMibInfo adslMib;
   long size = sizeof(adslMib);

   nRet = devCtl_adslGetObjectValue(NULL, 0, (char *)&adslMib, &size);
   *bertStatus = adslMib.adslBertStatus;
   return (nRet);
}

// Description  : Get ADSL PHY version.
void cmsAdsl_getPhyVersion(char *version, int len)
{
   adslVersionInfo adslVer;
   int verLen;

   if ( version == NULL ) 
      return;

   memset(version,0,len);

   if (devCtl_adslGetVersion(&adslVer) == CMSRET_SUCCESS)
   {
      verLen = strlen(adslVer.phyVerStr);
      if (len > verLen)
         strcpy(version,adslVer.phyVerStr);
      else
         strncpy(version,adslVer.phyVerStr,len);
   }
}


// Description  : Get ADSL connection information.
CmsRet cmsAdsl_getConnectionInfo( PADSL_CONNECTION_INFO pConnInfo )
{
#ifdef DESKTOP_LINUX
   return CMSRET_SUCCESS;
#else
   return xdslCtl_GetConnectionInfo(0, pConnInfo);
#endif
} // BcmAdslCtl_GetConnectionInfo

//***************************************************************************
// Function Name: parseAdslInfo
// Description  : parse info to get value of the given variable.
// Returns      : none.
//***************************************************************************/
void parseAdslInfo(char *info, char *var, char *val, int len)
{
   char *pChar = NULL;
   int i = 0;

   if ( info == NULL || var == NULL || val == NULL ) return;

   pChar = strstr(info, var);
   if ( pChar == NULL ) return;

   // move pass the variable string in line
   pChar += strlen(var);

   // Remove spaces from beginning of value string
   while ( *pChar != '\0' && isspace((int)*pChar) != 0 )
      pChar++;

   // get data until end of line, or comma, or space char
   for ( i = 0;
         i < len && *pChar != '\0' &&
         *pChar != ',' && isspace((int)*pChar) == 0;
         i++, pChar++ )
      val[i] = *pChar;
   val[i] = '\0';
} // parseAdslInfo

CmsRet cmsAdsl_initialize(adslCfgProfile *pAdslCfg)
{
   cmsAdsl_initializeTrace();
   return(devCtl_adslInitialize(NULL,0,pAdslCfg));
}

CmsRet cmsAdsl_uninitialize(void)
{
   cmsAdsl_uninitializeTrace();
   return(devCtl_adslUninitialize());
}

/* this function is called to configure ADSL and also start it by bring it up */
CmsRet cmsAdsl_start(void)
{
   cmsAdsl_startTrace();
   return(devCtl_adslConnectionStart());
}

CmsRet cmsAdsl_stop(void)
{
   cmsAdsl_stopTrace();
   return(devCtl_adslConnectionStop());
}

CmsRet cmsAdsl_getAdslMib(adslMibInfo *adslMib)
{
   CmsRet nRet;
   long size = sizeof(adslMibInfo);

   nRet = devCtl_adslGetObjectValue(NULL, 0, (char *)adslMib, &size);
   return nRet;
}

CmsRet cmsAdsl_ResetStatCounters(void)
{
   cmsAdsl_ResetStatCountersTrace();
   return(devCtl_adslResetStatCounters());
}

CmsRet cmsAdsl_BertTestStart(UINT32  duration)
{
   cmsAdsl_BertTestStartTrace();
   return(devCtl_adslBertStartEx(duration));
}

CmsRet cmsAdsl_BertTestStop(void)
{
   cmsAdsl_BertTestStopTrace();
   return(devCtl_adslBertStopEx());
}

CmsRet cmsAdsl_getAdslMibObject(char *oidStr, int oidLen, char *data, long *dataLen)
{
   CmsRet nRet;

   nRet = devCtl_adslGetObjectValue(oidStr, oidLen, data, dataLen);
   return nRet;
}

CmsRet cmsAdsl_setTestDiagMode(void)
{
   cmsAdsl_setTestDiagModeTrace();
   return(devCtl_adslSetTestMode(ADSL_TEST_DIAGMODE));
}

/* utilities functions */
SINT32 Qn2DecI(SINT32 qnVal, int q)
{
   return (qnVal >> q) - (qnVal >> 31);
}

SINT32 Qn2DecF(SINT32 qnVal, int q)
{
   int      sn = qnVal >> 31;
   return (((qnVal ^ sn) - sn) & ((1 << q) - 1)) * (10000 >> q);
}

char* QnToString(SINT32 val, int q)
{
   static   char  str1[32];
   SINT32        iPart;

   if (val < 0) {
      val = -val;
      iPart = -(val >> q);
      if (0 == iPart) {
         sprintf(str1, "-0.%04u", Qn2DecF(val,q));
         return str1;
      }
   }
   else
      iPart = val >> q;
   sprintf( str1, "%d.%04d", iPart, Qn2DecF(val,q));
   return str1;
}

/* to be tested with new driver */

CmsRet cmsAdsl_setAdslMibObject(char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
   CmsRet nRet;
   cmsAdsl_setAdslMibObjectTrace();
   nRet = devCtl_adslSetObjectValue(objId, objIdLen, dataBuf, dataBufLen);
   return nRet;
}


/* for debug */
void cmsAdsl_printMibObject(int oid,int oidStrSize, char* data, ulong dataLen)
{
   int i;

   printf("\nMIB object retrieved by TR69c after dslDiag completes\n");
   printf("oid %d, oidStrSize %d, datatLen %ld\n",oid,oidStrSize,dataLen);
   for (i = 0; i < dataLen; i+=8)
      printf("   %d\t%x %x %x %x %x %x %x %x\n", i, data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6],data[i+7]);
   printf("--------------------------------------------------------------------\n");
}
