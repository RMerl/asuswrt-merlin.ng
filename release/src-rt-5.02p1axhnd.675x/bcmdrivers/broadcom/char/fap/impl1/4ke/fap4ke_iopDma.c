/*
<:copyright-BRCM:2013:proprietary:standard

   Copyright (c) 2013 Broadcom 
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

/******************************************************************************
* File Name  : fap4ke_iopDma.c                                                *
*                                                                             *
* Description: This is the IOP DMA driver implementation for the FAP 4ke MIPs.*
******************************************************************************/

#include "fap_hw.h"
#include "fap4ke_local.h"
#include "fap4ke_task.h"
#include "fap4ke_iopDma.h"

/******************************************************************************
* Function: iopDma_Init                                                       *
*                                                                             *
* Description: Handles initialization of IOP DMA driver                       *
******************************************************************************/
fapRet iopDma_Init(void)
{
    return IOPDMA_SUCCESS;
}

#if defined(FAP_IOPDMA_BUILD_LIB)

/******************************************************************************
* Function: iopDma_StartDmaTransfer                                           *
*                                                                             *
* Description: Submits a DMA operation to either DMA0 or DMA1                 *
*                                                                             *
* Parameters: channel - DMA channel 0 or 1                                    *
*             pDestBuffer - Destination Buffer for DMA operation              *
*             pSrcBuffers - Source buffer structure                           *
*             numSrcBuffers - Number of source buffers (non 0 for scatter-gath*                                                                  
*             pCommandList - Ptr to Cmd List in memory                        *
*             bWait - TRUE to force DMA to wait until previous operation      *
*                     completes                                               *
*             pduLength - PDU length used for INSERT_LENGTH and REPLACE_LENGTH*
*                          extended commands                                  *
******************************************************************************/
fapRet iopDma_StartDmaTransfer( uint32   channel, 
                                uint8   *pDestBuffer, 
                                fap4keIopDma_SourceBuffer_t   *pSrcBuffers, 
                                uint32   numSrcBuffers, 
                                uint8    *pCommandList,
                                BOOL     bWait,
                                uint16   pduLength)
{
    uint32 srcIndex;
    fap4keIopDma_SourceBuffer_t   *pCurrSource;
    uint32 dmaStatus;
    uint32 lenCtl;

#if defined(FAP_IOPDMA_ERROR_CHECK)
    if (pDestBuffer == NULL)
    {
        fap4kePrt_Error("pDestBuffer is NULL in iopDma_StartDmaTransfer\n");
        return FAP_ERROR;    
    }
    if ( pSrcBuffers == NULL )
    {
        fap4kePrt_Error("pSrcBuffers is NULL in iopDma_StartDmaTransfer\n");
        return FAP_ERROR;    
    }
    
    if ( channel > 1 )
    {
        fap4kePrt_Error("incorrect channel ID in iopDma_StartDmaTransfer\n");
        return FAP_ERROR;
    }
#endif
    
    dmaStatus = FAP_4KE_REG_RD(_4keRegCntrl->dma_status);
    if ( ((channel == 0) && (dmaStatus & DMA_STS_DMA0_CMD_FULL_BIT)) ||
         ((channel == 1) && (dmaStatus & DMA_STS_DMA1_CMD_FULL_BIT)) )
    {
        return IOPDMA_CMD_FIFO_FULL;
    }

    /* Leave DEST_ADDR as b'00 (flast 32 bit address) and write in the passed in destination address */
            
    /* Remove Cache bit if necessary from addresses for HW */
    if ( ((uint32)pDestBuffer & 0xFFF00000) != 0xE0000000)
    {
        /* SDRAM */
        FAP_4KE_REG_WR(_4keDmaReg->dma_ch[channel].dma_dest, (uint32)((uint32)pDestBuffer & ~0x80000000));
    }
    else
    {
        /* Local Mem */
        FAP_4KE_REG_WR(_4keDmaReg->dma_ch[channel].dma_dest, (uint32)pDestBuffer);
    }
    
    srcIndex = 0;
    while ( srcIndex < numSrcBuffers )
    {
        pCurrSource    = &pSrcBuffers[srcIndex];
        
        /* Remove Cache bit if necessary from addresses for HW */
        if ( ((uint32)pCurrSource->pSourceBuffer & 0xFFF00000) != 0xE0000000)
        {
           /* SDRAM */
           FAP_4KE_REG_WR(_4keDmaReg->dma_ch[channel].dma_source, (uint32)((uint32)pCurrSource->pSourceBuffer & ~0x80000000));
        }
        else
        {
           /* Local Mem */
           FAP_4KE_REG_WR(_4keDmaReg->dma_ch[channel].dma_source, (uint32)pCurrSource->pSourceBuffer);
        }
            
        lenCtl = (pCurrSource->bufferLength & 0xFFF);
        
        if ( pCommandList != NULL )
        {
            /* Add in the command list every src buffer even though it will only be used on the first source buffer */
            lenCtl |= DMA_CTL_LEN_EXEC_CMD_LIST_BIT;
            FAP_4KE_REG_WR(_4keDmaReg->dma_ch[channel].dma_cmd_list, (uint32)pCommandList);
        }
        
        if ( srcIndex < (numSrcBuffers - 1) )
        {
            /* Not the last source buffer, set the continue bit */
            lenCtl |= DMA_CTL_LEN_CONTINUE_BIT;
        } 

        if ( bWait )
        {
            lenCtl |= DMA_CTL_LEN_WAIT_BIT;
        }

        if ( pduLength != 0 )
        {
            lenCtl |= (pduLength << DMA_CTL_LEN_LENGTH_N_VALUE_SHIFT) & DMA_CTL_LEN_LENGTH_N_VALUE_MASK;
        }

        /* Issue DMA command to block */
        FAP_4KE_REG_WR(_4keDmaReg->dma_ch[channel].dma_len_ctl, lenCtl );

        srcIndex++;
    } /* End While */
    return IOPDMA_SUCCESS;
}

fapRet iopDma_StartDmaTransferBlocking(uint32   channel,
                                       uint8   *pDestBuffer, 
                                       uint8   *pSourceBuffer,
                                       uint32   bufferLength,
                                       uint8    *pCommandList,
                                       uint16   pduLength)
{
    uint32 reg32;
    int dmaRetry = 1000;
    volatile mDma_regs_S *dma_ch_p;

#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pSourceBuffer == NULL )
    {
        fap4kePrt_Error("Error: pSrcBuffers is NULL in iopDma_StartDmaTransfer\n");
        return FAP_ERROR;    
    }
    
    if ( channel > 1 )
    {
        fap4kePrt_Error("Error: incorrect channel ID in iopDma_StartDmaTransfer\n");
        return FAP_ERROR;
    }

    if (pDestBuffer == NULL)
    {
        fap4kePrt_Error("Error: pDestBuffer is NULL in iopDma_StartDmaTransfer\n");
        return FAP_ERROR;
    }
#endif

    /* FIXME: Assumption is we always finish current transfer to end before starting a new one */
#if 0
    {
        uint32 dmaStatus FAP_4KE_REG_RD(_4keRegCntrl->dma_status);
        if ( ((channel == 0) && (dmaStatus & DMA_STS_DMA0_CMD_FULL_BIT)) ||
             ((channel == 1) && (dmaStatus & DMA_STS_DMA1_CMD_FULL_BIT)) )
        {
            return IOPDMA_CMD_FIFO_FULL;
        }
    }
#endif

    /************** Setup DMA ****************/

    dma_ch_p = &_4keDmaReg->dma_ch[channel];

    FAP_4KE_REG_WR(dma_ch_p->dma_source, (uint32)(pSourceBuffer));
    FAP_4KE_REG_WR(dma_ch_p->dma_dest, (uint32)(pDestBuffer));

    reg32 = (bufferLength & 0xFFF);

    if (pCommandList != NULL)
    {
        /* Add in the command list every src buffer even though it will only be used on the first source buffer */
        reg32 |= DMA_CTL_LEN_EXEC_CMD_LIST_BIT;

        FAP_4KE_REG_WR(dma_ch_p->dma_cmd_list, (uint32)pCommandList);
    }

    reg32 |= DMA_CTL_LEN_WAIT_BIT;

    if ( pduLength != 0 )
    {
        reg32 |= (pduLength << DMA_CTL_LEN_LENGTH_N_VALUE_SHIFT) & DMA_CTL_LEN_LENGTH_N_VALUE_MASK;
    }

    /* Issue DMA command to block */
    FAP_4KE_REG_WR(dma_ch_p->dma_len_ctl, reg32 );

    /************** Wait until DMA Completes ****************/

    do
    {
        reg32 = FAP_4KE_REG_RD(_4keRegCntrl->dma_status);

        if (channel == 0)
        {
            reg32 &= DMA_STS_DMA0_RSLT_EMPTY_BIT;
        }
        else
        {
            reg32 &= DMA_STS_DMA1_RSLT_EMPTY_BIT;
        }

    } while(reg32 && --dmaRetry);

    if(dmaRetry == 0)
    {
#if defined(FAP_IOPDMA_ERROR_CHECK)
        fap4kePrt_Error("IOP_DMA timeout!");
#endif
        return FAP_ERROR;
    }

    /************** Retrieve DMA Result ****************/

    reg32 = FAP_4KE_REG_RD(dma_ch_p->dma_rslt_len_stat);
    
    return (reg32 & DMA_RSLT_ERROR_MASK);
}

/******************************************************************************
* Function: iopDma_GetDmaResults                                              *
*                                                                             *
* Description: Gets the current DMA results out of the DMA channel Result FIFO*
*            If there are no results this fucntion will return                *
*                IOPDMA_RESULT_FIFO_EMPTY                                     *
* Parameters: channel - DMA channel 0 or 1                                    *
*              pDmaResults - Return structure containing DMA results.         *
******************************************************************************/                         
fapRet iopDma_GetDmaResults ( uint32  channel, 
                              fap4keIopDma_DmaResults_t *pDmaResults) 
{
    uint32 lenStatReg;
    uint32 dmaStatus;

#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pDmaResults == NULL )
    {
        fap4kePrt_Error("pDmaResults is NULL in iopDma_GetDmaResults\n");
        return FAP_ERROR;
    }

    if ( channel > 1 )
    {
        fap4kePrt_Error("incorrect channel ID in iopDma_GetDmaResults\n");
        return FAP_ERROR;
    }
#endif

    dmaStatus = FAP_4KE_REG_RD(_4keRegCntrl->dma_status);
    if ( ((channel == 0) && (dmaStatus & DMA_STS_DMA0_RSLT_EMPTY_BIT)) ||
         ((channel == 1) && (dmaStatus & DMA_STS_DMA1_RSLT_EMPTY_BIT)) )
    {
        return IOPDMA_RESULT_FIFO_EMPTY;
    }
    
    /* There is a result to be processed, pop it off and fill the fap4keIopDma_DmaResults_t structure */
    pDmaResults->pResultSource = (uint8 *)FAP_4KE_REG_RD(_4keDmaReg->dma_ch[channel].dma_rslt_source);
    pDmaResults->pResultDestination = (uint8 *)FAP_4KE_REG_RD(_4keDmaReg->dma_ch[channel].dma_rslt_dest);
    pDmaResults->HCS0_Value = FAP_4KE_REG_RD(_4keDmaReg->dma_ch[channel].dma_rslt_hcs) & DMA_RSLT_HCS_HCS0_MASK;
    pDmaResults->HCS1_Value = (FAP_4KE_REG_RD(_4keDmaReg->dma_ch[channel].dma_rslt_hcs) & DMA_RSLT_HCS_HCS1_MASK) >> DMA_RSLT_HCS_HCS1_SHIFT;
    
    lenStatReg = FAP_4KE_REG_RD(_4keDmaReg->dma_ch[channel].dma_rslt_len_stat);
    
    pDmaResults->BytesTransferred = lenStatReg & DMA_RSLT_DMA_LEN;
    pDmaResults->Continue = lenStatReg & DMA_RSLT_CONTINUE;

    if ( lenStatReg & DMA_RSLT_NOT_END_CMDS )
    {
        return IOPDMA_RESULT_NOT_END_OF_COMMANDS;
    }
    else if ( lenStatReg & DMA_RSLT_FLUSHED )
    {
        return IOPDMA_RESULT_FLUSHED;
    }
    else if ( lenStatReg & DMA_RSLT_ABORTED )
    {
        return IOPDMA_RESULT_ABORTED;
    }
    else if ( lenStatReg & DMA_RSLT_ERR_CMD_FMT )
    {
        return IOPDMA_RESULT_IMPROPER_CMDLIST;
    }
    else if ( lenStatReg & DMA_RSLT_ERR_DEST )
    {
        return IOPDMA_RESULT_DEST_INVALID;
    }
    else if ( lenStatReg & DMA_RSLT_ERR_SRC )
    {
        return IOPDMA_RESULT_SRC_INVALID;
    }
    else if ( lenStatReg & DMA_RSLT_ERR_CMD_LIST )
    {
        return IOPDMA_RESULT_CMDLIST_PTR_INVALID;
    }
    else if ( lenStatReg & DMA_RSLT_ERR_DEST_LEN )
    {
        return IOPDMA_RESULT_DEST_LEN_INVALID;
    }
    else if ( lenStatReg & DMA_RSLT_ERR_SRC_LEN )
    {
        return IOPDMA_RESULT_SRC_LEN_INVALID;
    }
    else
    {
        return IOPDMA_SUCCESS;
    }
}

fapRet iopDma_GetDmaResultsFast ( uint32  channel, 
                                  fap4keIopDma_DmaResults_t *pDmaResults) 
{
    uint32 lenStatReg;

#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pDmaResults == NULL )
    {
        fap4kePrt_Error("pDmaResults is NULL in iopDma_GetDmaResults\n");
        return FAP_ERROR;
    }

    if ( channel > 1 )
    {
        fap4kePrt_Error("incorrect channel ID in iopDma_GetDmaResults\n");
        return FAP_ERROR;
    }
#endif

    /* We already know that a DMA has ended by polling iopDma_GetDmaStatus(), so
       there is no need to check the _4keRegCntrl->dma_status register */
    
    lenStatReg = FAP_4KE_REG_RD(_4keDmaReg->dma_ch[channel].dma_rslt_len_stat);
    
    pDmaResults->BytesTransferred = lenStatReg & DMA_RSLT_DMA_LEN;
    pDmaResults->Continue = lenStatReg & DMA_RSLT_CONTINUE;

    return (lenStatReg & DMA_RSLT_ERROR_MASK);
}

/******************************************************************************
* Function: iopDma_GetDmaStatus                                               *
*                                                                             *
* Description: Gets the current DMA status of the DMA block                   *
* Parameters: channel - DMA channel 0 or 1                                    *
******************************************************************************/
fapRet iopDma_GetDmaStatus ( uint32 channel)
{
    uint32 status;
    int32 returnVal = 0;

#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( channel > 1 )
    {
        fap4kePrt_Error("incorrect channel ID in iopDma_GetDmaStatus\n");
        return FAP_ERROR;
    }
#endif
    
    status = FAP_4KE_REG_RD(_4keRegCntrl->dma_status);

    if ( channel == 0 )
    {
        returnVal = status & ( DMA_STS_DMA0_BUSY | DMA_STS_DMA0_CMD_FULL_BIT | 
                                DMA_STS_DMA0_RSLT_EMPTY_BIT | DMA_STS_DMA0_RSLT_FULL_BIT);
    }
    else
    {
        returnVal = (status & ( DMA_STS_DMA1_BUSY | DMA_STS_DMA1_CMD_FULL_BIT | 
                                DMA_STS_DMA1_RSLT_EMPTY_BIT | DMA_STS_DMA1_RSLT_FULL_BIT)) >> DMA_STS_DMA1_SHIFT;
    }                

    return returnVal;
    
}

/******************************************************************************
* Function: iopDma_GetDmaCmdAvailable                                         *
*                                                                             *
* Description: Gets the number of available spots for commands in the DMA FIFO*
* Parameters: channel - DMA channel 0 or 1                                    *
******************************************************************************/
fapRet iopDma_GetDmaCmdAvailable ( uint32   channel )
{
    int32 returnVal;
    uint32 dmaControl;
    
#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( channel > 1 )
    {
        fap4kePrt_Error("incorrect channel ID in iopDma_GetDmaCmdAvailable\n");
        return FAP_ERROR;
    }
#endif
    
    dmaControl = FAP_4KE_REG_RD(_4keRegCntrl->dma_control);

    if ( channel == 0 )
    {
        returnVal = ( dmaControl & DMA0_CMD_FIFO_AVAIL_MASK ) >> DMA0_CMD_FIFO_AVAIL_SHIFT;
    }
    else
    {
        returnVal = ( dmaControl & DMA1_CMD_FIFO_AVAIL_MASK ) >> DMA1_CMD_FIFO_AVAIL_SHIFT;
    }
    
    return returnVal;
}

/******************************************************************************
* Function: iopDma_GetDmaResultFifoDepth                                      *
*                                                                             *
* Description: Gets the number of available results in the DMA Result FIFO    *
* Parameters: channel - DMA channel 0 or 1                                    *
******************************************************************************/
fapRet iopDma_GetDmaResultFifoDepth ( uint32   channel )
{
    int32 returnVal;
    uint32 dmaControl;
    
#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( channel > 1 )
    {
        fap4kePrt_Error("incorrect channel ID in iopDma_GetDmaResultFifoDepth\n");
        return FAP_ERROR;
    }
#endif
    
    dmaControl = FAP_4KE_REG_RD(_4keRegCntrl->dma_control);

    if ( channel == 0 )
    {
        returnVal = ( dmaControl & DMA0_RESULT_FIFO_DEPTH_MASK ) >> DMA0_RESULT_FIFO_DEPTH_SHIFT;
    }
    else
    {
        returnVal = ( dmaControl & DMA1_RESULT_FIFO_DEPTH_MASK ) >> DMA1_RESULT_FIFO_DEPTH_SHIFT;
    }
    
    return returnVal;
}

#endif /* FAP_IOPDMA_BUILD_LIB */


/* Functions to build command lists with.  All return length of the bytes written to command list pointer */
/******************************************************************************
* Function: iopDma_CmdListAddEndOfCommands                                    *
*                                                                             *
* Description: Writes the End Of Commands extended command to the location    *
*              passed in and returns the number of bytes written or FAP_ERROR *
* Parameters: pCmdList - Ptr to location to write command                     *
*                                                                             *
* used by both 4ke and host                                                   *
******************************************************************************/
fapRet iopDma_CmdListAddEndOfCommands( uint8 *pCmdList )
{
#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pCmdList == NULL )
    {
        fap4kePrt_Error("pCmdList is NULL in iopDma_CmdListAddEndOfCommands\n");
        return FAP_ERROR;
    }
#endif

    pCmdList[0] = IOPDMA_EXTCMDLIST_OPCODE_END;
    pCmdList[1] = 0;
    pCmdList[2] = 0;
    pCmdList[3] = 0;

    return 4;
}

/******************************************************************************
* Function: iopDma_CmdListAddInsert                                           *
*                                                                             *
* Description: Writes the Insert extended command to the location             *
*              passed in and returns the number of bytes written or FAP_ERROR *
* Parameters: pCmdList - Ptr to location to write command                     *
*             offset - Offset for command                                     *
*             pData - location of data for command                            * 
*             length - length for command                                     *
*                                                                             *
* only used by host side                                                      *
******************************************************************************/
fapRet iopDma_CmdListAddInsert( uint8   *pCmdList, 
                                uint16   offset,
                                uint8    *pData,
                                uint8    length )
{
    uint32 i, j, pad = 0;

#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( (pCmdList == NULL) || (pData == NULL) )
    {
        fap4kePrt_Error("pCmdList or pData is NULL in iopDma_CmdListAddInsert\n");
        return FAP_ERROR;
    }
#endif

    i = 0;
    pCmdList[i++] = IOPDMA_EXTCMDLIST_OPCODE_INSERT;
    pCmdList[i++] = (offset & 0xFF00) >> 8;
    pCmdList[i++] = (offset & 0xFF);
    pCmdList[i++] = length;
    j = 0;
    while (    j < length )
    {
       pCmdList[i++] = pData[j++];
    }
    
    pad = length % 4;
    if (pad != 0)
      pad = 4 - pad;

    if (pad)
    {
        for (j=0; j<pad; j++)
        {
           pCmdList[i++] = 0;
        }        
                    
    }
    return i;
}

/******************************************************************************
* Function: iopDma_CmdListAddReplace                                          *
*                                                                             *
* Description: Writes the Replace extended command to the location            *
*              passed in and returns the number of bytes written or FAP_ERROR *
* Parameters: pCmdList - Ptr to location to write command                     *
*             offset - Offset for command                                     *
*             pData - location of data for command                            * 
*             length - length for command                                     *
******************************************************************************/
fapRet iopDma_CmdListAddReplace(   uint8   *pCmdList,
                                   uint16   offset,
                                   uint8    *pData,
                                   uint8    length )
{
    uint32 i, j, pad = 0;

#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( (pCmdList == NULL) || (pData == NULL) )
    {
        fap4kePrt_Error("pCmdList or pData is NULL in iopDma_CmdListAddReplace\n");
        return FAP_ERROR;
    }
#endif

    i=0;
    pCmdList[i++] = IOPDMA_EXTCMDLIST_OPCODE_REPLACE;
    pCmdList[i++] = (offset & 0xFF00) >> 8;
    pCmdList[i++] = (offset & 0xFF);
    pCmdList[i++] = length;
    j = 0;
    while (    j < length )
    {
       pCmdList[i++] = pData[j++];
    }
    
    pad = length % 4;
    if (pad != 0)
      pad = 4 - pad;

    if (pad)
    {
        for (j=0; j<pad; j++)
        {
           pCmdList[i++] = 0;
        }        
                    
    }
    return i;
}

/******************************************************************************
* Function: iopDma_CmdListAddDelete                                           *
*                                                                             *
* Description: Writes the Delete extended command to the location             *
*              passed in and returns the number of bytes written or FAP_ERROR *
* Parameters: pCmdList - Ptr to location to write command                     *
*             offset - Offset for command                                     *
*             length - length for command                                     *
******************************************************************************/
fapRet iopDma_CmdListAddDelete( uint8   *pCmdList,
                                uint16   offset,
                                uint8    length )
{
    uint32 i = 0;

#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pCmdList == NULL )
    {
        fap4kePrt_Error("pCmdList is NULL in iopDma_CmdListAddDelete\n");
        return FAP_ERROR;
    }
#endif

    i=0;
    pCmdList[i++] = IOPDMA_EXTCMDLIST_OPCODE_DELETE;
    pCmdList[i++] = (offset & 0xFF00) >> 8;
    pCmdList[i++] = (offset & 0xFF);
    pCmdList[i++] = length;

    return 4;
    
    
}

/******************************************************************************
* Function: iopDma_CmdListAddInsertLength                                     *
*                                                                             *
* Description: Writes the Insert Length extended command to the location      *
*              passed in and returns the number of bytes written or FAP_ERROR *
* Parameters: pCmdList - Ptr to location to write command                     *
*             offset - Offset for command                                     *
*             data -   data for command                                       * 
*                                                                             *
******************************************************************************/
fapRet iopDma_CmdListAddInsertLength( uint8   *pCmdList,
                                      uint16   offset,
                                      uint16   data)
{
    uint32 i = 0;
    
#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pCmdList == NULL )
    {
        fap4kePrt_Error("pCmdList is NULL in iopDma_CmdListAddInsertLength\n");
        return FAP_ERROR;
    }
#endif

    i=0;
    pCmdList[i++] = IOPDMA_EXTCMDLIST_OPCODE_INSERT_LENGTH;
    pCmdList[i++] = (offset & 0xFF00) >> 8;
    pCmdList[i++] = (offset & 0xFF);
    pCmdList[i++] = 2;
    pCmdList[i++] = (data & 0xFF00) >> 8;
    pCmdList[i++] = (data & 0xFF);
    pCmdList[i++] = 0; /* Padding */
    pCmdList[i++] = 0;

    return 8;
}

/******************************************************************************
* Function: iopDma_CmdListAddReplaceLength                                    *
*                                                                             *
* Description: Writes the Replace Length extended command to the location     *
*              passed in and returns the number of bytes written or FAP_ERROR *
* Parameters: pCmdList - Ptr to location to write command                     *
*             offset - Offset for command                                     *
*             data -   data for command                                       * 
*                                                                             *
******************************************************************************/
fapRet iopDma_CmdListAddReplaceLength( uint8   *pCmdList,
                                       uint16   offset,
                                       uint16   data )
{
    uint32 i = 0;
    
#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pCmdList == NULL )
    {
        fap4kePrt_Error("pCmdList is NULL in iopDma_CmdListAddReplaceLength\n");
        return FAP_ERROR;
    }
#endif

    i=0;
    pCmdList[i++] = IOPDMA_EXTCMDLIST_OPCODE_REPLACE_LENGTH;
    pCmdList[i++] = (offset & 0xFF00) >> 8;
    pCmdList[i++] = (offset & 0xFF);
    pCmdList[i++] = 2;
    pCmdList[i++] = (data & 0xFF00) >> 8;
    pCmdList[i++] = (data & 0xFF);
    pCmdList[i++] = 0; /* Padding */
    pCmdList[i++] = 0;

    return 8;
}

/******************************************************************************
* Function: iopDma_CmdListAddMemset                                           *
*                                                                             *
* Description: Writes the Replace Memset extended command to the location     *
*              passed in and returns the number of bytes written or FAP_ERROR *
* Parameters: pCmdList - Ptr to location to write command                     *
*             offset - Offset for command                                     *
*             data -   data for command                                       * 
*             length - length for command                                     *
******************************************************************************/
fapRet iopDma_CmdListAddMemset( uint8   *pCmdList,
                                uint16   offset,
                                uint8    data,
                                uint8    length )
{
    uint32 i = 0;
    
#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pCmdList == NULL )
    {
        fap4kePrt_Error("pCmdList is NULL in iopDma_CmdListAddMemset\n");
        return FAP_ERROR;
    }
#endif

    i=0;
    pCmdList[i++] = IOPDMA_EXTCMDLIST_OPCODE_MEMSET;
    pCmdList[i++] = (offset & 0xFF00) >> 8;
    pCmdList[i++] = (offset & 0xFF);
    pCmdList[i++] = 2;
    pCmdList[i++] = data;
    pCmdList[i++] = 0; /* Padding */
    pCmdList[i++] = 0;
    pCmdList[i++] = 0;

    return 8;
}

/******************************************************************************
* Function: iopDma_CmdListAddChecksum1                                        *
*                                                                             *
* Description: Writes the Checksum1 extended command to the location          *
*              passed in and returns the number of bytes written or FAP_ERROR *
* Parameters: pCmdList - Ptr to location to write command                     *
*             pChecksumCmdList - Ptr location of command list for command     *
******************************************************************************/
fapRet iopDma_CmdListAddChecksum1( uint8   *pCmdList,
                                   uint8   *pChecksumCmdList)
{
    uint32 i = 0;
    
#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pCmdList == NULL )
    {
        fap4kePrt_Error("pCmdList is NULL in iopDma_CmdListAddChecksum1\n");
        return FAP_ERROR;
    }
#endif

    i=0;
    pCmdList[i++] = IOPDMA_EXTCMDLIST_OPCODE_CHECKSUM1;
    pCmdList[i++] = ((uint32)pChecksumCmdList & 0xFF0000) >> 16;
    pCmdList[i++] = ((uint32)pChecksumCmdList & 0xFF00) >> 8;
    pCmdList[i++] = ((uint32)pChecksumCmdList & 0xFF);
    
    return 4;
}

/******************************************************************************
* Function: iopDma_CmdListAddChecksum2                                        *
*                                                                             *
* Description: Writes the Checksum2 extended command to the location          *
*              passed in and returns the number of bytes written or FAP_ERROR *
* Parameters: pCmdList - Ptr to location to write command                     *
*             pChecksumCmdList - Ptr location of command list for command     *
******************************************************************************/
fapRet iopDma_CmdListAddChecksum2( uint8   *pCmdList,
                                   uint8   *pChecksumCmdList)
{
    uint32 i = 0;
    
#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pCmdList == NULL )
    {
        fap4kePrt_Error("pCmdList is NULL in iopDma_CmdListAddChecksum2\n");
        return FAP_ERROR;
    }
#endif

    i=0;
    pCmdList[i++] = IOPDMA_EXTCMDLIST_OPCODE_CHECKSUM2;
    pCmdList[i++] = ((uint32)pChecksumCmdList & 0xFF0000) >> 16;
    pCmdList[i++] = ((uint32)pChecksumCmdList & 0xFF00) >> 8;
    pCmdList[i++] = ((uint32)pChecksumCmdList & 0xFF);
    
    return 4;

}

/******************************************************************************
* Function: iopDma_ChecksumCmdListAdd                                         *
*                                                                             *
* Description: Writes a checksum command to the Checksum command list to the  *
*              location passed in and returns the number of bytes written     *
*              or FAP_ERROR                                                   *
* Parameters: pChecksumCmdList - Ptr to location to write command             *
*             opcode - Checksum opcode                                        *
*             offset - offset for checksum command                            *
*             lengthOrConstData - length of constant data for checksum command*
*             depending on particular opcode                                  *
******************************************************************************/
fapRet iopDma_ChecksumCmdListAdd( uint8   *pChecksumCmdList,
                                  uint8    opcode,
                                  uint16   offset,
                                  uint16   lengthOrConstData)
{
    uint32 i = 0;
    
#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pChecksumCmdList == NULL )
    {
        fap4kePrt_Error("pCmdList is NULL in iopDma_ChecksumCmdListAdd\n");
        return FAP_ERROR;
    }
#endif

    i=0;
    pChecksumCmdList[i++] = ((opcode & 0xF) << 4) | ((offset & 0xF00) >> 8);
    pChecksumCmdList[i++] = offset & 0xFF;
    pChecksumCmdList[i++] = (lengthOrConstData & 0xFF00) >> 8;
    pChecksumCmdList[i++] = lengthOrConstData & 0xFF;
    
    return 4;
}

void iopDma_debugDumpRegs( void )
{
    fap4kePrt_Notice("DumpReg\n");
    fap4kePrt_Notice("dma_status           0x%x\n", FAP_4KE_REG_RD(_4keRegCntrl->dma_status));
    fap4kePrt_Notice("dma0_3_fifo_status   0x%x\n", FAP_4KE_REG_RD(_4keRegCntrl->dma0_3_fifo_status));
    fap4kePrt_Notice("dma0 dma_source      0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[0].dma_source));
    fap4kePrt_Notice("dma0 dma_dest        0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[0].dma_dest));
    fap4kePrt_Notice("dma0 dma_cmd_list    0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[0].dma_cmd_list));
    fap4kePrt_Notice("dma0 dma_len_ctl     0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[0].dma_len_ctl));
    fap4kePrt_Notice("dma0 dma_rslt_source 0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[0].dma_rslt_source));
    fap4kePrt_Notice("dma0 dma_rslt_dest   0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[0].dma_rslt_dest));
    fap4kePrt_Notice("dma0 dma_rslt_hcs    0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[0].dma_rslt_hcs));
    //fap4kePrt_Notice("dma0 dma_rslt_len_st 0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[0].dma_rslt_len_stat));
    fap4kePrt_Notice("dma1 dma_source      0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[1].dma_source));
    fap4kePrt_Notice("dma1 dma_dest        0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[1].dma_dest));
    fap4kePrt_Notice("dma1 dma_cmd_list    0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[1].dma_cmd_list));
    fap4kePrt_Notice("dma1 dma_len_ctl     0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[1].dma_len_ctl));
    fap4kePrt_Notice("dma1 dma_rslt_source 0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[1].dma_rslt_source));
    fap4kePrt_Notice("dma1 dma_rslt_dest   0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[1].dma_rslt_dest));
    fap4kePrt_Notice("dma1 dma_rslt_hcs    0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[1].dma_rslt_hcs));
    //fap4kePrt_Notice("dma1 dma_rslt_len_st 0x%x\n", FAP_4KE_REG_RD(_4keDmaReg->dma_ch[1].dma_rslt_len_stat));
    
    fap4kePrt_Notice("DumpReg\n");

}
