/***********************************************************
 *
 * <:copyright-BRCM:2009:DUAL/GPL:standard
 * 
 *    Copyright (c) 2009 Broadcom 
 *    All Rights Reserved
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
 ************************************************************/

#ifndef __FAP4KE_IOPDMA_H_INCLUDED__
#define __FAP4KE_IOPDMA_H_INCLUDED__

/*
 *******************************************************************************
 * File Name  : fap4ke_iopDma.h
 *
 * Description: This file contains the public API for the 4KE IOP DMA driver
 *
 *******************************************************************************
 */

// needed for FAP_IOPDMA_ERROR_CHECK
#include "fap4ke_mailBox.h"
#include "fap4ke_printer.h"

//#define FAP_IOPDMA_ERROR_CHECK
//#define FAP_IOPDMA_DEBUG
//#define FAP_IOPDMA_BUILD_LIB

/* IOPDMA channels */
#define IOPDMA_DMA0                          0
#define IOPDMA_DMA1                          1
/* IOPDMA depth */
#define IOPDMA_DMA_DEPTH                     4
/* IOPDMA function returns */
#define IOPDMA_SUCCESS                       FAP_SUCCESS
   /* Returns for iopDma_GetDmaResults */
#define IOPDMA_RESULT_NOT_END_OF_COMMANDS    1
#define IOPDMA_RESULT_FLUSHED                2
#define IOPDMA_RESULT_ABORTED                3
#define IOPDMA_RESULT_IMPROPER_CMDLIST       4
#define IOPDMA_RESULT_DEST_INVALID           5
#define IOPDMA_RESULT_SRC_INVALID            6
#define IOPDMA_RESULT_CMDLIST_PTR_INVALID    7
#define IOPDMA_RESULT_DEST_LEN_INVALID       8
#define IOPDMA_RESULT_SRC_LEN_INVALID        9
#define IOPDMA_RESULT_FIFO_EMPTY             10
   /* Returns for iopDma_StartDmaTransfer */
#define IOPDMA_CMD_FIFO_FULL                 11

/* Returns for iopDma_GetDmaStatus */
#define IOPDMA_STATUS_RESULT_FIFO_FULL       (1<<3)
#define IOPDMA_STATUS_RESULT_FIFO_EMPTY      (1<<2)
#define IOPDMA_STATUS_CMD_FIFO_FULL          (1<<1)
#define IOPDMA_STATUS_BUSY                   (1<<0)

/* Opcodes used internally, but available for use by anyone */ 
#define IOPDMA_EXTCMDLIST_OPCODE_END                  0
#define IOPDMA_EXTCMDLIST_OPCODE_INSERT               1
#define IOPDMA_EXTCMDLIST_OPCODE_REPLACE              2
#define IOPDMA_EXTCMDLIST_OPCODE_DELETE               3
#define IOPDMA_EXTCMDLIST_OPCODE_CHECKSUM1            4
#define IOPDMA_EXTCMDLIST_OPCODE_CHECKSUM2            5
#define IOPDMA_EXTCMDLIST_OPCODE_INSERT_LENGTH        6
#define IOPDMA_EXTCMDLIST_OPCODE_REPLACE_LENGTH       7
#define IOPDMA_EXTCMDLIST_OPCODE_MEMSET               8    

/* Opcodes required to be passed into iopDma_ChecksumCmdListAdd */
#define IOPDMA_HCS_OPCODE_END                      0
#define IOPDMA_HCS_OPCODE_FIRST                    1
#define IOPDMA_HCS_OPCODE_FIRST_WITH_CONST         2
#define IOPDMA_HCS_OPCODE_FIRST_WITH_LEN           3
#define IOPDMA_HCS_OPCODE_CONTINUE                 4
#define IOPDMA_HCS_OPCODE_CONTINUE_WITH_CONST      5
#define IOPDMA_HCS_OPCODE_CONTINUE_WITH_LEN        6
#define IOPDMA_HCS_OPCODE_LAST                     7

/* special address indicating bit bucket transfer */
#define IOPDMA_BIT_BUCKET_LOCAL_ADDR               FAP_4KE_PSM_BASE_IOP
#define IOPDMA_BIT_BUCKET_LENCTL_DEST_ADDR         0x8000

#define IOPDMA_LOCAL_ADDR(_addr) ( (uint8 *)(_addr) )
#define IOPDMA_SDRAM_ADDR(_addr) ( (uint8 *)((uint32)(_addr) & ~0xE0000000) )

/* Source Buffer structure for use in iopDma_StartDmaTransfer */
typedef struct {
   uint8   *pSourceBuffer;
   uint32   bufferLength;
} fap4keIopDma_SourceBuffer_t;

/* DMA results structure returned by iopDma_GetDmaResults */
typedef struct {
   uint8   *pResultSource;
   uint8   *pResultDestination;
   uint16   HCS0_Value;
   uint16   HCS1_Value;
   uint16   BytesTransferred;
   uint16   Continue;
} fap4keIopDma_DmaResults_t;

void dumpHeader(uint8 *packet_p);

/*
 * Inline Functions
*/
#ifdef FAP_4KE
/* This file is included by host, but these functions should not
   be made available to host */

static inline fapRet iopDma_start(uint32   channel,
                                  uint8   *pDestBuffer, 
                                  uint8   *pSourceBuffer,
                                  uint32   bufferLength,
                                  uint8    *pCommandList,
                                  uint16   pduLength)
{
    uint32 reg32;
    volatile mDma_regs_S *dma_ch_p;

#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pSourceBuffer == NULL )
    {
        fap4kePrt_Error("Error: pSourceBuffer is NULL\n");
        return FAP_ERROR;    
    }
    
    if ( channel > 1 )
    {
        fap4kePrt_Error("Error: incorrect channel ID\n");
        return FAP_ERROR;
    }

    if (pDestBuffer == NULL)
    {
        fap4kePrt_Error("Error: pDestBuffer is NULL\n");
        return FAP_ERROR;
    }

    reg32 = FAP_4KE_REG_RD(_4keRegCntrl->dma_status);
    if ( ((channel == 0) && (reg32 & DMA_STS_DMA0_CMD_FULL_BIT)) ||
         ((channel == 1) && (reg32 & DMA_STS_DMA1_CMD_FULL_BIT)) )
    {
        fap4kePrt_Error("DMA Channel %ld is full!\n", channel);
        return IOPDMA_CMD_FIFO_FULL;
    }
#endif

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

    return IOPDMA_SUCCESS;
}

static inline fapRet iopDma_bitBucket(uint32   channel,
                                      uint8   *pSourceBuffer,
                                      uint32   bufferLength,
                                      uint8    *pCommandList,
                                      uint16   pduLength)
{
    uint32 reg32;
    volatile mDma_regs_S *dma_ch_p;

#if defined(FAP_IOPDMA_ERROR_CHECK)
    if ( pSourceBuffer == NULL )
    {
        fap4kePrt_Error("Error: pSourceBuffer is NULL\n");
        return FAP_ERROR;    
    }
    
    if ( channel > 1 )
    {
        fap4kePrt_Error("Error: incorrect channel ID\n");
        return FAP_ERROR;
    }

    reg32 = FAP_4KE_REG_RD(_4keRegCntrl->dma_status);
    if ( ((channel == 0) && (reg32 & DMA_STS_DMA0_CMD_FULL_BIT)) ||
         ((channel == 1) && (reg32 & DMA_STS_DMA1_CMD_FULL_BIT)) )
    {
        fap4kePrt_Error("DMA Channel %ld is full!\n", channel);
        return IOPDMA_CMD_FIFO_FULL;
    }
#endif

    dma_ch_p = &_4keDmaReg->dma_ch[channel];

    FAP_4KE_REG_WR(dma_ch_p->dma_source, (uint32)(pSourceBuffer));
    FAP_4KE_REG_WR(dma_ch_p->dma_dest, IOPDMA_BIT_BUCKET_LOCAL_ADDR);

    reg32 = IOPDMA_BIT_BUCKET_LENCTL_DEST_ADDR | (bufferLength & 0xFFF);

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

    return IOPDMA_SUCCESS;
}

static inline fapRet iopDma_scatherGather(uint32   channel, 
                                          uint8   *pDestBuffer, 
                                          fap4keIopDma_SourceBuffer_t *pSrcBuffers, 
                                          uint32   numSrcBuffers, 
                                          uint8    *pCommandList,
                                          uint16   pduLength)
{
    volatile mDma_regs_S *dma_ch_p;
    uint32 dmaLenCtl;
    int i;

#if defined(FAP_IOPDMA_ERROR_CHECK)
    if (pDestBuffer == NULL)
    {
        fap4kePrt_Error("pDestBuffer is NULL\n");
        return FAP_ERROR;    
    }

    if ( pSrcBuffers == NULL )
    {
        fap4kePrt_Error("Error: pSourceBuffers is NULL\n");
        return FAP_ERROR;
    }

    if ( numSrcBuffers == 0)
    {
        fap4kePrt_Error("Error: numSrcBuffers is 0\n");
        return FAP_ERROR;
    }
    
    if ( channel > 1 )
    {
        fap4kePrt_Error("Error: incorrect channel ID\n");
        return FAP_ERROR;
    }

    {
        uint32 dmaStatus;

        dmaStatus = FAP_4KE_REG_RD(_4keRegCntrl->dma_status);
        if ( ((channel == 0) && (dmaStatus & DMA_STS_DMA0_CMD_FULL_BIT)) ||
             ((channel == 1) && (dmaStatus & DMA_STS_DMA1_CMD_FULL_BIT)) )
        {
            fap4kePrt_Error("DMA Channel %ld is full!\n", channel);
            return IOPDMA_CMD_FIFO_FULL;
        }
    }
#endif

#if defined(FAP_IOPDMA_DEBUG)
    fap4kePrt_Print("%s : channel %ld, pDestBuffer 0x%08lX, numSrcBuffers %ld, pduLength %d\n",
                    __FUNCTION__, channel, (uint32)pDestBuffer, (uint32)numSrcBuffers, pduLength);
    for(i=0; i<numSrcBuffers; ++i)
    {
        fap4kePrt_Print("\t%d : pSourceBuffer 0x%08lX, bufferLength %ld\n",
                        i, (uint32)pSrcBuffers->pSourceBuffer, pSrcBuffers->bufferLength);
    }
    if(pCommandList)
    {
        dumpHeader(pCommandList);
    }
    fap4kePrt_Print("\n\n");
#endif

    dma_ch_p = &_4keDmaReg->dma_ch[channel];

    FAP_4KE_REG_WR(dma_ch_p->dma_dest, (uint32)(pDestBuffer));

    dmaLenCtl = (pduLength << DMA_CTL_LEN_LENGTH_N_VALUE_SHIFT) & DMA_CTL_LEN_LENGTH_N_VALUE_MASK;

    if (pCommandList != NULL)
    {
        /* Add in the command list every src buffer even though it will only be used on the first source buffer */
        dmaLenCtl |= DMA_CTL_LEN_EXEC_CMD_LIST_BIT;

        FAP_4KE_REG_WR(dma_ch_p->dma_cmd_list, (uint32)pCommandList);
    }

#if defined(FAP_IOPDMA_DEBUG)
    fap4kePrt_Debug("0 : dma_dest <0x%08lX>, dma_cmd_list <0x%08lX>\n",
                    FAP_4KE_REG_RD(dma_ch_p->dma_dest),
                    FAP_4KE_REG_RD(dma_ch_p->dma_cmd_list));
#endif

    for(i=0; i<numSrcBuffers; ++i)
    {
        FAP_4KE_REG_WR(dma_ch_p->dma_source, (uint32)(pSrcBuffers[i].pSourceBuffer));

        dmaLenCtl |= (pSrcBuffers[i].bufferLength & 0xFFF);

        if (i < (numSrcBuffers-1))
        {
            dmaLenCtl |= DMA_CTL_LEN_CONTINUE_BIT;
        }

        /* Issue DMA command to block */
        FAP_4KE_REG_WR(dma_ch_p->dma_len_ctl, dmaLenCtl);

#if defined(FAP_IOPDMA_DEBUG)
        fap4kePrt_Debug("%d : dma_source <0x%08lX>, dma_len_ctl <0x%08lX>\n",
                        i, FAP_4KE_REG_RD(dma_ch_p->dma_source),
                        FAP_4KE_REG_RD(dma_ch_p->dma_len_ctl));
#endif
        dmaLenCtl = 0;
    }

    return IOPDMA_SUCCESS;
}

static inline fapRet iopDma_finish(uint32 channel)
{
    int dmaRetry = 1000000;
    uint32 reg32;
    const uint32 emptyMask = (channel == 0) ? DMA_STS_DMA0_RSLT_EMPTY_BIT :
        DMA_STS_DMA1_RSLT_EMPTY_BIT;
#if defined(FAP_IOPDMA_DEBUG)
    uint32 dma_rslt_source, dma_rslt_dest, dma_rslt_hcs;
#endif

    /************** Wait until DMA Completes ****************/

    do
    {
        reg32 = FAP_4KE_REG_RD(_4keRegCntrl->dma_status) & emptyMask;

    } while(reg32 && --dmaRetry);

    if(dmaRetry == 0)
    {
#if defined(FAP_IOPDMA_ERROR_CHECK)
        fap4kePrt_Error("IOP_DMA timeout!");
#endif
        return FAP_ERROR;
    }

    /************** Retrieve DMA Result ****************/

#if defined(FAP_IOPDMA_DEBUG)
    dma_rslt_source = FAP_4KE_REG_RD(_4keDmaReg->dma_ch[channel].dma_rslt_source);
    dma_rslt_dest = FAP_4KE_REG_RD(_4keDmaReg->dma_ch[channel].dma_rslt_dest);
    dma_rslt_hcs = FAP_4KE_REG_RD(_4keDmaReg->dma_ch[channel].dma_rslt_hcs);
#endif

    reg32 = FAP_4KE_REG_RD(_4keDmaReg->dma_ch[channel].dma_rslt_len_stat);

#if defined(FAP_IOPDMA_DEBUG)
    fap4kePrt_Debug("dma_rslt_source <0x%08lX>, dma_rslt_dest <0x%08lX>, "
                    "dma_rslt_hcs <0x%08lX>, dma_rslt_len_stat <0x%08lX>\n",
                    dma_rslt_source, dma_rslt_dest, dma_rslt_hcs, reg32);
#endif

    return (reg32 & DMA_RSLT_ERROR_MASK);
}

static inline fapRet iopDma_finishHcs(uint32 channel, uint32 *hcsResult)
{
    int dmaRetry = 1000000;
    uint32 reg32;
    const uint32 emptyMask = (channel == 0) ? DMA_STS_DMA0_RSLT_EMPTY_BIT :
        DMA_STS_DMA1_RSLT_EMPTY_BIT;

    /************** Wait until DMA Completes ****************/

    do
    {
        reg32 = FAP_4KE_REG_RD(_4keRegCntrl->dma_status) & emptyMask;

    } while(reg32 && --dmaRetry);

    if(dmaRetry == 0)
    {
#if defined(FAP_IOPDMA_ERROR_CHECK)
        fap4kePrt_Error("IOP_DMA timeout!");
#endif
        return FAP_ERROR;
    }

    /************** Retrieve DMA Result ****************/

    *hcsResult = FAP_4KE_REG_RD(_4keDmaReg->dma_ch[channel].dma_rslt_hcs);

    reg32 = FAP_4KE_REG_RD(_4keDmaReg->dma_ch[channel].dma_rslt_len_stat);

    return (reg32 & DMA_RSLT_ERROR_MASK);
}

#endif /* FAP_4KE */

/* Public APIs */
extern fapRet iopDma_Init(void);

#if defined(FAP_IOPDMA_BUILD_LIB)
extern fapRet iopDma_StartDmaTransfer( uint32   channel, 
                                       uint8   *pDestBuffer, 
                                       fap4keIopDma_SourceBuffer_t   *pSrcBuffers, 
                                       uint32   numSrcBuffers, 
                                       uint8    *pCommandList,
                                       BOOL     bWait,
                                       uint16   pduLength);
extern fapRet iopDma_StartDmaTransferBlocking(uint32   channel,
                                              uint8   *pDestBuffer, 
                                              uint8   *pSourceBuffer,
                                              uint32   bufferLength,
                                              uint8    *pCommandList,
                                              uint16   pduLength);
extern fapRet iopDma_GetDmaResults ( uint32  channel, 
                                     fap4keIopDma_DmaResults_t *pDmaResults); 
extern fapRet iopDma_GetDmaResultsFast ( uint32  channel, 
                                  fap4keIopDma_DmaResults_t *pDmaResults);
extern fapRet iopDma_GetDmaStatus ( uint32 channel);
extern fapRet iopDma_GetDmaCmdAvailable ( uint32   channel );
extern fapRet iopDma_GetDmaResultFifoDepth ( uint32   channel );
#endif /* FAP_IOPDMA_BUILD_LIB */

/* Functions to build command lists with.  All return length of the bytes written to command list pointer */
extern fapRet iopDma_CmdListAddEndOfCommands( uint8 *pCmdList );
extern fapRet iopDma_CmdListAddInsert( uint8   *pCmdList, 
                                       uint16   offset,
                                       uint8    *pData,
                                       uint8    length );
extern fapRet iopDma_CmdListAddReplace(   uint8   *pCmdList,
                                          uint16   offset,
                                          uint8    *pData,
                                          uint8    length );
extern fapRet iopDma_CmdListAddDelete( uint8   *pCmdList,
                                       uint16   offset,
                                       uint8    length );
extern fapRet iopDma_CmdListAddInsertLength( uint8   *pCmdList,
                                             uint16   offset,
                                             uint16   data);
extern fapRet iopDma_CmdListAddReplaceLength( uint8   *pCmdList,
                                              uint16   offset,
                                              uint16   data);
extern fapRet iopDma_CmdListAddMemset( uint8   *pCmdList,
                                       uint16   offset,
                                       uint8    data,
                                       uint8    length );
extern fapRet iopDma_CmdListAddChecksum1( uint8   *pCmdList,
                                          uint8   *pChecksumCmdList);
extern fapRet iopDma_CmdListAddChecksum2( uint8   *pCmdList,
                                          uint8   *pChecksumCmdList);
extern fapRet iopDma_ChecksumCmdListAdd( uint8   *pChecksumCmdList,
                                         uint8    opcode,
                                         uint16   offset,
                                         uint16   lengthOrConstData);

extern void iopDma_debugDumpRegs( void );
#endif
