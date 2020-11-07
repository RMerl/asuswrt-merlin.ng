/***********************************************************************
 *
 *  Copyright (c) 2006  Broadcom Corporation
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


/* Includes. */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include "DiagDef.h"
#include "devctl_adsl.h"

/* Globals. */
#ifdef SUPPORT_DSL_BONDING
#define MAX_DSL_LINE    2
static char    *g_xdslDevName[MAX_DSL_LINE] = {"/dev/bcmadsl0", "/dev/bcmadsl1"}; /* Make sure "/dev/bcmadsl1" is created in makeDevs */
#else
#define MAX_DSL_LINE    1
static char    *g_xdslDevName[MAX_DSL_LINE] = {"/dev/bcmadsl0"};
#endif

static int xdslCtl_Open(unsigned char lineId)
{
#ifdef DESKTOP_LINUX
   return (-1);
#else
   if(lineId >= MAX_DSL_LINE)
      return -1;
   return (open(g_xdslDevName[lineId], O_RDWR));
#endif
} /* xdslCtl_Open */

#ifdef XDSLIOCTL_SEND_HMI_MSG
CmsRet xdslCtl_SendHmiMessage(
            unsigned char lineId,
            unsigned char *header,
            unsigned short headerSize,
            unsigned char *payload,
            unsigned short payloadSize,
            unsigned char *replyMessage,
            unsigned short replyMaxMessageSize)
{
   int             fd;
   XDSLDRV_HMI_MSG Arg;
   CmsRet nRet = CMSRET_SUCCESS;

   Arg.bvStatus = BCMADSL_STATUS_ERROR;

   if(lineId >= MAX_DSL_LINE)
     return CMSRET_INTERNAL_ERROR;

   fd = xdslCtl_Open(lineId);
   if( -1 == fd )
   {
      nRet = CMSRET_INTERNAL_ERROR;
   }
   else
   {
      Arg.header = header;
      Arg.payload = payload;
      Arg.replyMessage = replyMessage;
      Arg.headerSize = headerSize;
      Arg.payloadSize = payloadSize;
      Arg.replyMaxMessageSize = replyMaxMessageSize;
      ioctl( fd, XDSLIOCTL_SEND_HMI_MSG, &Arg );
      close(fd);
      if (Arg.bvStatus != BCMADSL_STATUS_SUCCESS)
      {
         nRet = CMSRET_INTERNAL_ERROR;
      }
   }
   return( nRet );
}
#endif

CmsRet xdslCtl_OpenEocIntf(unsigned char lineId, int eocMsgType, char *pIfName)
{
   int               fd;
   XDSLDRV_EOC_IFACE Arg;
   CmsRet nRet = CMSRET_SUCCESS;

   Arg.bvStatus = BCMADSL_STATUS_ERROR;

   if(lineId >= MAX_DSL_LINE)
     return CMSRET_INTERNAL_ERROR;

   fd = xdslCtl_Open(lineId);
   if( -1 == fd )
   {
      nRet = CMSRET_INTERNAL_ERROR;
   }
   else
   {
      Arg.eocMsgType = eocMsgType;
      Arg.ifName = pIfName;
      Arg.ifNameLen = strlen(pIfName);
      ioctl( fd, XDSLIOCTL_OPEN_EOC_IFACE, &Arg );
      close(fd);
      if (Arg.bvStatus != BCMADSL_STATUS_SUCCESS)
      {
         nRet = CMSRET_INTERNAL_ERROR;
      }
   }
   return( nRet );
}

CmsRet xdslCtl_CloseEocIntf(unsigned char lineId, int eocMsgType, char *pIfName)
{
   int               fd;
   XDSLDRV_EOC_IFACE Arg;
   CmsRet nRet = CMSRET_SUCCESS;

   Arg.bvStatus = BCMADSL_STATUS_ERROR;

   if(lineId >= MAX_DSL_LINE)
     return CMSRET_INTERNAL_ERROR;

   fd = xdslCtl_Open(lineId);
   if( -1 == fd )
   {
      nRet = CMSRET_INTERNAL_ERROR;
   }
   else
   {
      Arg.eocMsgType = eocMsgType;
      Arg.ifName = pIfName;
      Arg.ifNameLen = strlen(pIfName);
      ioctl( fd, XDSLIOCTL_CLOSE_EOC_IFACE, &Arg );
      close(fd);
      if (Arg.bvStatus != BCMADSL_STATUS_SUCCESS)
      {
         nRet = CMSRET_INTERNAL_ERROR;
      }
   }
   return( nRet );
}

CmsRet xdslCtl_DiagProcessCommandFrame(UINT8 lineId, void *diagBuf, int diagBufLen)
{
    int                 fd;
    ADSLDRV_DIAG        Arg;
    DiagProtoFrame      *diagCmd = diagBuf;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;
    
    fd = xdslCtl_Open(lineId);
    if( fd == -1 )
    {
        nRet = CMSRET_INTERNAL_ERROR;
    }
    else
    {
        Arg.diagCmd = diagCmd->diagHdr.logCommmand;
        if (LOG_CMD_CONNECT != Arg.diagCmd)
        {
            if (LOG_CMD_DISCONNECT != Arg.diagCmd) {
                Arg.diagCmd |= (((diagCmd->diagHdr.logPartyId & DIAG_PARTY_TYPE_SEND_MASK) >> DIAG_PARTY_TYPE_SEND_SHIFT) << DIAG_TYPE_CMD_SHIFT);
                Arg.diagMap = (unsigned long) diagCmd->diagData;
                Arg.srvIpAddr = *(short *)diagCmd->diagHdr.logProtoId;
            }
            else {
                Arg.diagMap = diagCmd->diagHdr.logPartyId & 0xFF;
                Arg.srvIpAddr = *((int *)diagCmd->diagData);
            }
            Arg.logTime = diagBufLen - sizeof(LogProtoHeader);
        }
        else
        {
            int * pConnect = (int *) diagCmd->diagData;
            Arg.diagMap   = pConnect[0];
            Arg.logTime   = pConnect[1];
            Arg.srvIpAddr = pConnect[2];
            Arg.gwIpAddr = pConnect[3];
        }
        ioctl( fd, ADSLIOCTL_DIAG_COMMAND, &Arg );
        close(fd);

        if (Arg.bvStatus != BCMADSL_STATUS_SUCCESS)
        {
            nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    return( nRet );
}

CmsRet devCtl_adslDiagProcessCommandFrame(void *diagBuf, int diagBufLen)
{
   return xdslCtl_DiagProcessCommandFrame(0, diagBuf, diagBufLen);
}

CmsRet xdslCtl_DiagProcessDbgCommand(UINT8 lineId, UINT16 cmd, UINT16 cmdId, UINT32 param1, UINT32 param2)
{
    int             fd;
    ADSLDRV_DIAG    Arg;
    DiagDebugData   diagCmd;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd == -1  )
   {
        return CMSRET_INTERNAL_ERROR;
   }
   else
   {
      diagCmd.cmd       = cmd;
      diagCmd.cmdId = cmdId;
      diagCmd.param1    = param1;
      diagCmd.param2    = param2;
    
      Arg.diagCmd       = LOG_CMD_DEBUG;
      Arg.diagMap       = (unsigned long) &diagCmd;
      Arg.logTime       = sizeof(diagCmd);
      Arg.srvIpAddr    = 0;
    
      ioctl( fd, ADSLIOCTL_DIAG_COMMAND, &Arg );
      close(fd);

      if (Arg.bvStatus != BCMADSL_STATUS_SUCCESS)
      {
         nRet = CMSRET_INTERNAL_ERROR;
      }
   }

   return( nRet );
}

CmsRet BcmAdsl_DiagProcessDbgCommand(UINT16 cmd, UINT16 cmdId, UINT32 param1, UINT32 param2)
{
   return xdslCtl_DiagProcessDbgCommand(0, cmd, cmdId, param1, param2);
}

CmsRet xdslCtl_Check(UINT8 lineId)
{
    int                 fd;
    ADSLDRV_STATUS_ONLY Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1  )
    {
        ioctl( fd, ADSLIOCTL_CHECK, &Arg );
        close(fd);

        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
           nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
       nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
} /* Bcmxdsl_Check */

CmsRet devCtl_adslCheck(void)
{
    return xdslCtl_Check(0);
} /* BcmAdsl_Check */

CmsRet xdslCtl_Initialize(UINT8 lineId, ADSL_FN_NOTIFY_CB pFnNotifyCb, void *pParm, adslCfgProfile *pAdslCfg)
{
    int                fd;
    ADSLDRV_INITIALIZE Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.pFnNotifyCb = pFnNotifyCb;
    Arg.pParm = pParm;
    Arg.pAdslCfg = pAdslCfg;
    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;
    
    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
        ioctl( fd, ADSLIOCTL_INITIALIZE, &Arg );
        close(fd);

        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
           nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
       nRet = CMSRET_INTERNAL_ERROR;
    }
    
    return( nRet );
} /* BcmAdsl_Initialize */

CmsRet devCtl_adslInitialize(ADSL_FN_NOTIFY_CB pFnNotifyCb, void *pParm, adslCfgProfile *pAdslCfg)
{
    return xdslCtl_Initialize(0, pFnNotifyCb, pParm, pAdslCfg);
} /* BcmAdsl_Initialize */

/***************************************************************************
 * Function Name: BcmAdsl_Uninitialize
 * Description  : Clean up resources allocated during BcmAdsl_Initialize.
 * Returns      : STS_SUCCESS if successful or error status.
 ***************************************************************************/
CmsRet xdslCtl_Uninitialize(UINT8 lineId)
{
    int                 fd;
    ADSLDRV_STATUS_ONLY Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;
    
    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
        ioctl( fd, ADSLIOCTL_UNINITIALIZE, &Arg );
        close(fd);

        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
           nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
        nRet = CMSRET_INTERNAL_ERROR;
    }
    
    return( nRet );
} /* BcmAdsl_Uninitialize */

CmsRet devCtl_adslUninitialize(void)
{
    return xdslCtl_Uninitialize(0);
} /* BcmAdsl_Uninitialize */

/***************************************************************************
 * Function Name: BcmAdsl_ConnectionStart
 * Description  : Start ADSL connection.
 * Returns      : STS_SUCCESS if successful or error status.
 ***************************************************************************/
CmsRet xdslCtl_ConnectionStart(UINT8 lineId)
{
    int                 fd;
    ADSLDRV_STATUS_ONLY Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;
    
    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
        ioctl( fd, ADSLIOCTL_CONNECTION_START, &Arg );
        close(fd);

        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
           nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else 
    {
        nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
} /* BcmAdsl_ConnectionStart */

CmsRet devCtl_adslConnectionStart( void )
{
    return xdslCtl_ConnectionStart(0);
} /* BcmAdsl_ConnectionStart */


/***************************************************************************
 * Function Name: BcmAdsl_ConnectionStop
 * Description  : Clean up resources allocated during BcmAdsl_Initialize.
 * Returns      : STS_SUCCESS if successful or error status.
 ***************************************************************************/
CmsRet xdslCtl_ConnectionStop(UINT8 lineId)
{
    int                 fd;
    ADSLDRV_STATUS_ONLY Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;

    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
        ioctl( fd, ADSLIOCTL_CONNECTION_STOP, &Arg );
        close(fd);

        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
           nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
        nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
} /* BcmAdsl_ConnectionStop */

CmsRet devCtl_adslConnectionStop( void )
{
    return xdslCtl_ConnectionStop(0);
} /* BcmAdsl_ConnectionStop */


/***************************************************************************
 * Function Name: BcmAdsl_GetPhyAddresses
 * Description  : Returns a ADSL PHY address.
 * Returns      : STS_SUCCESS if successful or error status.
 ***************************************************************************/
CmsRet xdslCtl_GetPhyAddresses(UINT8 lineId, PADSL_CHANNEL_ADDR pChannelAddr )
{
    int              fd;
    ADSLDRV_PHY_ADDR Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;
    
    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
        ioctl( fd, ADSLIOCTL_GET_PHY_ADDR, &Arg );
        close(fd);

        pChannelAddr->usFastChannelAddr = Arg.ChannelAddr.usFastChannelAddr;
        pChannelAddr->usInterleavedChannelAddr = Arg.ChannelAddr.usInterleavedChannelAddr;
        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
            nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
        nRet = CMSRET_INTERNAL_ERROR;
    }

    return( nRet );
} /* BcmAdsl_GetPhyAddresses */

CmsRet devCtl_adslGetPhyAddresses( PADSL_CHANNEL_ADDR pChannelAddr )
{
    return xdslCtl_GetPhyAddresses(0, pChannelAddr);
} /* BcmAdsl_GetPhyAddresses */


/***************************************************************************
 * Function Name: BcmAdsl_SetPhyAddresses
 * Description  : Sets a ADSL PHY address.
 * Returns      : STS_SUCCESS if successful or error status.
 ***************************************************************************/
CmsRet xdslCtl_SetPhyAddresses(UINT8 lineId, PADSL_CHANNEL_ADDR pChannelAddr)
{
    int              fd;
    ADSLDRV_PHY_ADDR Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.ChannelAddr.usFastChannelAddr = pChannelAddr->usFastChannelAddr;
    Arg.ChannelAddr.usInterleavedChannelAddr = pChannelAddr->usInterleavedChannelAddr;
    Arg.bvStatus = BCMADSL_STATUS_ERROR;

    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1  )
    {
        ioctl( fd, ADSLIOCTL_SET_PHY_ADDR, &Arg );
        close(fd);

        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
            nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
        nRet = CMSRET_INTERNAL_ERROR;
    }

    return( nRet );
} /* BcmAdsl_SetPhyAddresses */

CmsRet devCtl_adslSetPhyAddresses( PADSL_CHANNEL_ADDR pChannelAddr )
{
    return xdslCtl_SetPhyAddresses(0, pChannelAddr);
} /* BcmAdsl_SetPhyAddresses */


/***************************************************************************
 * Function Name: BcmAdsl_MapAtmPortIDs
 * Description  : Maps ATM Port IDs to DSL PHY Utopia Addresses.
 * Returns      : STS_SUCCESS if successful or error status.
 ***************************************************************************/
CmsRet xdslCtl_MapAtmPortIDs(UINT8 lineId, UINT16 usAtmFastPortId, UINT16 usAtmInterleavedPortId)
{
    int                  fd;
    ADSLDRV_MAP_ATM_PORT Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.usAtmFastPortId = usAtmFastPortId;
    Arg.usAtmInterleavedPortId = usAtmInterleavedPortId;
    Arg.bvStatus = BCMADSL_STATUS_ERROR;

    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
        ioctl( fd, ADSLIOCTL_MAP_ATM_PORT_IDS, &Arg );
        close(fd);

        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
            nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
        nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
} /* BcmAdsl_SetPhyAddresses */

CmsRet devCtl_adslMapAtmPortIDs(UINT16 usAtmFastPortId, UINT16 usAtmInterleavedPortId)
{
   return xdslCtl_MapAtmPortIDs(0, usAtmFastPortId, usAtmInterleavedPortId);
} /* BcmAdsl_SetPhyAddresses */


/***************************************************************************
 * Function Name: BcmAdsl_GetConnectionInfo
 * Description  : Sets a ADSL PHY address.
 * Returns      : STS_SUCCESS if successful or error status.
 ***************************************************************************/
CmsRet xdslCtl_GetConnectionInfo(UINT8 lineId, PADSL_CONNECTION_INFO pConnInfo )
{
    int                     fd;
    ADSLDRV_CONNECTION_INFO Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1  )
    {
        ioctl( fd, ADSLIOCTL_GET_CONNECTION_INFO, &Arg );
        close(fd);

        pConnInfo->LinkState = Arg.ConnectionInfo.LinkState;
#ifdef SUPPORT_VECTORINGD
        pConnInfo->errorSamplesAvailable = Arg.ConnectionInfo.errorSamplesAvailable;
#endif
        pConnInfo->ulFastUpStreamRate = Arg.ConnectionInfo.ulFastUpStreamRate;
        pConnInfo->ulFastDnStreamRate = Arg.ConnectionInfo.ulFastDnStreamRate;
        pConnInfo->ulInterleavedUpStreamRate = Arg.ConnectionInfo.ulInterleavedUpStreamRate;
        pConnInfo->ulInterleavedDnStreamRate = Arg.ConnectionInfo.ulInterleavedDnStreamRate;
        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
           nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
        nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
} /* BcmAdsl_GetConnectionInfo */

CmsRet devCtl_adslGetConnectionInfo( PADSL_CONNECTION_INFO pConnInfo )
{
    return xdslCtl_GetConnectionInfo(0, pConnInfo);
} /* BcmAdsl_GetConnectionInfo */

/***************************************************************************
 * Function Name: BcmAdsl_SetObjectValue
 * Description  : Sets MIB object value
 * Returns      : STS_SUCCESS if successful or error status.
 ***************************************************************************/
CmsRet xdslCtl_SetObjectValue(unsigned char lineId, char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
    int                 fd;
    ADSLDRV_GET_OBJ     Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;
    
    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
       Arg.objId        = objId;
       Arg.objIdLen = objIdLen;
       Arg.dataBuf      = dataBuf;
       Arg.dataBufLen   = *dataBufLen;
       ioctl( fd, ADSLIOCTL_SET_OBJ_VALUE, &Arg );
       close(fd);

       if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
       {
          nRet = CMSRET_INTERNAL_ERROR;
       }
    }
    else
    {
       nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
}

CmsRet devCtl_adslSetObjectValue(char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
    return xdslCtl_SetObjectValue(0, objId, objIdLen, dataBuf, dataBufLen);
}

/***************************************************************************
 * Function Name: BcmAdsl_GetObjectValue
 * Description  : Gets MIB object value
 * Returns      : STS_SUCCESS if successful or error status.
 ***************************************************************************/
CmsRet xdslCtl_GetObjectValue(unsigned char lineId, char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
    int                 fd;
    ADSLDRV_GET_OBJ     Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
       Arg.objId        = objId;
       Arg.objIdLen = objIdLen;
       Arg.dataBuf      = dataBuf;
       Arg.dataBufLen   = *dataBufLen;
       ioctl( fd, ADSLIOCTL_GET_OBJ_VALUE, &Arg );
       close(fd);

       *dataBufLen = Arg.dataBufLen;
       if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
       {
          nRet = CMSRET_INTERNAL_ERROR;
       }
    }
    else
    {
       nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
}

CmsRet devCtl_adslGetObjectValue(char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
    return xdslCtl_GetObjectValue(0, objId, objIdLen, dataBuf, dataBufLen);
}

/***************************************************************************
 * Function Name: BcmAdsl_StartBERT
 * Description  : Starts BERT test in ADSL PHY
 * Returns      : STS_SUCCESS if successful or error status.
 ***************************************************************************/
CmsRet xdslCtl_StartBERT(unsigned char lineId, UINT32  totalBits)
{
    int                 fd;
    ADSLDRV_BERT        Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
       Arg.totalBits = totalBits;
       ioctl( fd, ADSLIOCTL_START_BERT, &Arg );
       close(fd);

       if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
       {
           nRet = CMSRET_INTERNAL_ERROR;
       }
    }
    else
    {
       nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
}

CmsRet devCtl_adslStartBERT(UINT32  totalBits)
{
    return xdslCtl_StartBERT(0, totalBits);
}

/***************************************************************************
 * Function Name: BcmAdsl_StopBERT
 * Description  : Stops BERT test in ADSL PHY
 * Returns      : STS_SUCCESS if successful or error status.
 ***************************************************************************/
CmsRet xdslCtl_StopBERT(UINT8 lineId)
{
    int                 fd;
    ADSLDRV_STATUS_ONLY Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
       ioctl( fd, ADSLIOCTL_STOP_BERT, &Arg );
       close(fd);

       if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
       {
          nRet = CMSRET_INTERNAL_ERROR;
       }
    }
    else
    {
       nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
}

CmsRet devCtl_adslStopBERT(void)
{
    return xdslCtl_StopBERT(0);
}

//**************************************************************************
// Function Name: BcmAdsl_BertStartEx
// Description  : Start BERT test
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
CmsRet xdslCtl_BertStartEx(unsigned char lineId, UINT32  bertSec)
{
    int                 fd;
    ADSLDRV_BERT_EX     Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
        Arg.totalSec = bertSec;
        ioctl( fd, ADSLIOCTL_START_BERT_EX, &Arg );
        close(fd);

        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
           nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
       nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
}

CmsRet devCtl_adslBertStartEx(UINT32  bertSec)
{
    return xdslCtl_BertStartEx(0, bertSec);
}

//**************************************************************************
// Function Name: BcmAdsl_BertStopEx
// Description  : Stops BERT test
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
CmsRet xdslCtl_BertStopEx(unsigned char lineId)
{
    int                 fd;
    ADSLDRV_STATUS_ONLY Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
       ioctl( fd, ADSLIOCTL_STOP_BERT_EX, &Arg );
       close(fd);

       if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
       {
          nRet = CMSRET_INTERNAL_ERROR;
       }
    }
    else
    {
       nRet = CMSRET_INTERNAL_ERROR;
    }

    return( nRet );
}

CmsRet devCtl_adslBertStopEx(void)
{
    return xdslCtl_BertStopEx(0);
}

//**************************************************************************
// Function Name: BcmAdsl_Configure
// Description  : Changes ADSL current configuration
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
CmsRet xdslCtl_Configure(unsigned char lineId, adslCfgProfile *pAdslCfg)
{
    int                 fd;
    ADSLDRV_CONFIGURE   Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.pAdslCfg = pAdslCfg;
    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
        ioctl( fd, ADSLIOCTL_CONFIGURE, &Arg );
        close(fd);

        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
           nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
       nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
}

CmsRet devCtl_adslConfigure(adslCfgProfile *pAdslCfg)
{
    return xdslCtl_Configure(0, pAdslCfg);
}

//**************************************************************************
// Function Name: BcmAdsl_GetVersion
// Description  : Changes ADSL version information
// Returns      : STS_SUCCESS 
//**************************************************************************
CmsRet xdslCtl_GetVersion(unsigned char lineId, adslVersionInfo *pAdslVer)
{
    int                 fd;
    ADSLDRV_GET_VERSION     Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.pAdslVer  = pAdslVer;
    Arg.bvStatus = BCMADSL_STATUS_ERROR;

    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
        ioctl( fd, ADSLIOCTL_GET_VERSION, &Arg );
        close(fd);

        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
            nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
        nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
}

CmsRet devCtl_adslGetVersion(adslVersionInfo *pAdslVer)
{
    return xdslCtl_GetVersion(0, pAdslVer);
}

CmsRet xdslCtl_SetSDRAMBaseAddr(unsigned char lineId, void *pAddr)
{
    int                     fd;
    ADSLDRV_SET_SDRAM_BASE  Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
       Arg.sdramBaseAddr = pAddr;
       ioctl( fd, ADSLIOCTL_SET_SDRAM_BASE, &Arg );
       close(fd);

       if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
       {
          nRet = CMSRET_INTERNAL_ERROR;
       }
    }
    else
    {
       nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
}

CmsRet devCtl_adslSetSDRAMBaseAddr(void *pAddr)
{
    return xdslCtl_SetSDRAMBaseAddr(0, pAddr);
}

CmsRet xdslCtl_ResetStatCounters(unsigned char lineId)
{
    int                 fd;
    ADSLDRV_STATUS_ONLY Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
       ioctl( fd, ADSLIOCTL_RESET_STAT_COUNTERS, &Arg );
       close(fd);

       if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
       {
          nRet = CMSRET_INTERNAL_ERROR;
       }
    }
    else
    {
       nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
}

CmsRet devCtl_adslResetStatCounters(void)
{
    return xdslCtl_ResetStatCounters(0);
}

//**************************************************************************
// Function Name: BcmAdsl_SetTestMode
// Description  : Sets ADSL special test mode
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
CmsRet xdslCtl_SetTestMode(unsigned char lineId, ADSL_TEST_MODE testMode)
{
    int             fd;
    ADSLDRV_TEST    Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;

    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
        Arg.testCmd   = testMode;
        Arg.xmtStartTone    = 0;
        Arg.xmtNumTones     = 0;
        Arg.rcvStartTone    = 0;
        Arg.rcvNumTones     = 0;
        Arg.xmtToneMap      = NULL;
        Arg.rcvToneMap      = NULL;
        ioctl( fd, ADSLIOCTL_TEST, &Arg );
        close(fd);

        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
            nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
        nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
}

CmsRet devCtl_adslSetTestMode(ADSL_TEST_MODE testMode)
{
   return xdslCtl_SetTestMode(0, testMode);
}

//**************************************************************************
// Function Name: BcmAdsl_SelectTones
// Description  : Test function to set tones used by the ADSL modem
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
CmsRet xdslCtl_SelectTones(
    unsigned char lineId,
    int     xmtStartTone,
    int     xmtNumTones,
    int     rcvStartTone,
    int     rcvNumTones,
    char    *xmtToneMap,
    char    *rcvToneMap)
{
    int                 fd;
    ADSLDRV_TEST        Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;

    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
        Arg.testCmd         = ADSL_TEST_SELECT_TONES;
        Arg.xmtStartTone    = xmtStartTone;
        Arg.xmtNumTones     = xmtNumTones;
        Arg.rcvStartTone    = rcvStartTone;
        Arg.rcvNumTones     = rcvNumTones;
        Arg.xmtToneMap      = xmtToneMap;
        Arg.rcvToneMap      = rcvToneMap;
        ioctl( fd, ADSLIOCTL_TEST, &Arg );
        close(fd);

        if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
        {
            nRet = CMSRET_INTERNAL_ERROR;
        }
    }
    else
    {
        nRet = CMSRET_INTERNAL_ERROR;
    }
    return( nRet );
}

CmsRet devCtl_adslSelectTones(
    int     xmtStartTone,
    int     xmtNumTones,
    int     rcvStartTone,
    int     rcvNumTones,
    char    *xmtToneMap,
    char    *rcvToneMap)
{
    return xdslCtl_SelectTones(0, xmtStartTone, xmtNumTones, rcvStartTone, rcvNumTones, xmtToneMap, rcvToneMap);
}

/***************************************************************************
 * Function Name: BcmAdsl_GetConstellationPoints
 * Description  : Gets constellation point for selected tone
 * Returns      : number of points rettrieved 
 ***************************************************************************/
int xdslCtl_GetConstellationPoints (unsigned char lineId, int toneId, ADSL_CONSTELLATION_POINT *pointBuf, int numPoints)
{
    int                         fd;
    ADSLDRV_GET_CONSTEL_POINTS  Arg;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
       Arg.toneId       = toneId;
       Arg.pointBuf = pointBuf;
       Arg.numPoints    = numPoints;
       ioctl( fd, ADSLIOCTL_GET_CONSTEL_POINTS, &Arg );
       close(fd);

       if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
       {
          numPoints = 0;
       }
       else 
       {
          numPoints = Arg.numPoints;
       }
    }
    else
    {
       numPoints = 0;
    }
    return (numPoints);
}

int devCtl_adslGetConstellationPoints (int toneId, ADSL_CONSTELLATION_POINT *pointBuf, int numPoints)
{
    return xdslCtl_GetConstellationPoints(0, toneId, pointBuf, numPoints);
}

CmsRet xdslCtl_CallBackDrv(unsigned char lineId)
{
    int                 fd;
    ADSLDRV_STATUS_ONLY Arg;
    CmsRet nRet = CMSRET_SUCCESS;

    Arg.bvStatus = BCMADSL_STATUS_ERROR;
    
    if(lineId >= MAX_DSL_LINE)
        return CMSRET_INTERNAL_ERROR;

    fd = xdslCtl_Open(lineId);
    if( fd != -1 )
    {
       ioctl( fd, ADSLIOCTL_DRV_CALLBACK, &Arg );
       close(fd);

       if (Arg.bvStatus == BCMADSL_STATUS_ERROR)
       {
          nRet = CMSRET_INTERNAL_ERROR;
       }
    }
    else
    {
       nRet = CMSRET_INTERNAL_ERROR;
    }

    return( nRet );
}

