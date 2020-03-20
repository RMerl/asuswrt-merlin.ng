/*
<:copyright-BRCM:2007:GPL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
//**************************************************************************
// File Name  : spuciphr.c
//
// Description: 
//               
//**************************************************************************
#include <linux/module.h>
#include "bcmipsec.h"
#include "bcmsad.h"
#include "spuipsec.h"
#include "spucrypt.h"
#include "spuext.h"


extern void InitSHA1State(ubsec_HMAC_State_pt HMAC_State, unsigned char *HashKey);
extern void InitMD5State(ubsec_HMAC_State_pt HMAC_State, unsigned char *HashKey, int donxor);

/*
 * ubsec_CipherCommand: Process a list of Cipher commands.
 *
 * Immediate Status is returned. Completion status is returned
 * on a per command callback
 */
#if 0 // Pavan
ubsec_Status_t
ubsec_CipherCommand(ubsec_DeviceContext_t Context,
            ubsec_CipherCommandInfo_pt pCommand, int *NumCommands)
{
    DeviceInfo_pt pDevice = (DeviceInfo_pt) Context;
    VOLATILE MasterCommand_t *pMCR;
    VOLATILE Packet_t *pPacket;
    VOLATILE PacketContext_t *pContext;
    VOLATILE CipherContext_t *pCipherContext;
    VOLATILE int PacketIndex;
    VOLATILE int NumFrags;    /* Number of fragments */
    ubsec_FragmentInfo_t ExtraFragment, *pExtraFragment;
    int CommandIndex = 0;
    int CommandCount = *NumCommands;
    ubsec_Status_t Status;
    UBS_UINT32 SaveConfig = 0;

    UBS_UINT32 *Context_CryptoKey_pt = NULL;
    int CryptoKey_len = 0;
    UBS_UINT32 *Context_ComputedIV_IC_pt = NULL;    /* IV or IC is copie here */
    int ComputedIV_IC_len = 0;
    UBS_UINT32 *Context_HMAC_pt = NULL;

    int CurContextSize = 0;    /* in UBSEC_UINT32's */

    Dbg_Print(DBG_CMD, ("ubsec:  ubsec command %d", *NumCommands));

    /*
     * Check some parameters
     */
    if (pDevice == NULL_DEVICE_INFO) {
        Dbg_Print(DBG_FATAL, ("NO DEV\n "));
        return (UBSEC_STATUS_NO_DEVICE);
    }
    Dbg_Print(DBG_CMD, ("\n"));
#if 1
    if (pDevice->Status != UBSEC_STATUS_SUCCESS) {
      ASSERT_PRINT("Trying to cipher with device in error state\n");
      return (pDevice->Status);
    }
#endif

    if (OS_EnterCriticalSection(pDevice, SaveConfig)) {
        return (UBSEC_STATUS_DEVICE_BUSY);
    }

    /* Get the next MCR to load */
      Get_New_MCR:
    *NumCommands = CommandIndex;    /* Update number completed */
    if ((pMCR =
         GetFreeMCR(pDevice, UBSEC_CIPHER_LIST,
            &Status)) == NULL_MASTER_COMMAND) {
        Dbg_Print(DBG_CMD_FAIL,
              ("ubsec: device busy MCR %x\n", Status));
        goto Error_Return;
    }

    /* Add packets to this MCR. */

    Dbg_Print(DBG_CMD,
          ("ubsec: mcr_index %d MCR %0x\n", pMCR->Index, pMCR));
    /* Initialize the packet information */

    PacketIndex = pMCR->NumberOfPackets;
    pPacket = &(pMCR->PacketArray[PacketIndex]);    /* Set up the current packet. */
    pContext = &pMCR->ContextList[PacketIndex];
    Status = UBSEC_STATUS_SUCCESS;    /* Wishful thinking? */

    /* Process all the commands in the command list. */
    for (; CommandIndex < CommandCount; CommandIndex++) {    /* Add all the packets to the MCR */
        if (PacketIndex >= MCR_MAXIMUM_PACKETS) {
            Dbg_Print(DBG_CMD,
                  ("ubsec:  overran mcr buffer. %d %d\n",
                   PacketIndex, CommandIndex));
            /* 
             * We have filled this MCR with the max # of packets,
             * but still have more packets (commands) to do.
             * Advance next free. Wrap around if necessary
             */
            pDevice->NextFreeMCR[UBSEC_CIPHER_LIST] =
                (MasterCommand_pt) pMCR->pNextMCR;

            /* For crypto MCRs, the contexts are accessed using a single handle   */
            /* for an array of contexts. This means that all contexts for an MCR  */
            /* are contiguous in memory, and that we can sync all contexts at     */
            /* once (now that we know that we're finished loading this MCR).      */
            /* Make DMA memory actually hold CPU-initialized context data         */
            Dbg_Print(DBG_CNTXT_SYNC,
                  ("ubsec: ubsec_CipherCommand Sync %d Contexts to Device (0x%08X,%d,%d)\n",
                   pMCR->NumberOfPackets,
                   pMCR->ContextListHandle[0], 0,
                   pMCR->NumberOfPackets *
                   sizeof (PacketContext_t)));
            OS_SyncToDevice(pMCR->ContextListHandle[0], 0,
                    pMCR->NumberOfPackets *
                    sizeof (PacketContext_t));

            PushMCR(pDevice);    /* Get it going (pipeline) */
            goto Get_New_MCR;    /* Try to add to the next MCR */
        }

        /* Save the callback information. */
        pMCR->CompletionArray[PacketIndex].CompletionCallback =
            pCommand->CompletionCallback;
        pMCR->CompletionArray[PacketIndex].CommandContext =
            pCommand->CommandContext;

        /* Now set up the packet processing parameters */
        Dbg_Print(DBG_PACKET,
              ("ubsec: packet_Index %d, Context Buf %0x\n",
               PacketIndex, pContext));
        pPacket->PacketContextBuffer = pContext->PhysicalAddress;
        pCipherContext = &pContext->Context.Cipher;
        RTL_MemZero(pCipherContext, sizeof (*pCipherContext));
#ifdef UBSEC_582x_CLASS_DEVICE
        /* Some extra fields to be filled in . */
        /* pContext->cmd_structure_length= CPU_TO_CTRL_SHORT(sizeof(*pCipherContext)+4); *//* For header. *//* Done latter */

        if ((pCommand->Command & UBSEC_AES) == UBSEC_AES) {
#ifdef BCM_OEM_4
            if (UBSEC_USING_EXPLICIT_IV(pCommand->Command))
                pContext->operation_type =
                    OPERATION_IPSEC_AES_EXPLICIT_IV;
            else
#endif
                pContext->operation_type = OPERATION_IPSEC_AES;
        } else {
#ifdef BCM_OEM_4
            if (UBSEC_USING_EXPLICIT_IV(pCommand->Command))
                pContext->operation_type =
                    OPERATION_IPSEC_3DES_EXPLICIT_IV;
            else
#endif
                pContext->operation_type = OPERATION_IPSEC;    /* Default */
        }
#endif
        /*
         * Now add the packet input fragment information
         * First fragment will need to skip the MAC Header
         * We need at least one fragment.
         */
        /* Sanity checks. */
        if (!(NumFrags = pCommand->NumSource)) {
            Dbg_Print(DBG_PACKET, ("ubsec:  No Input fragments\n"));
            Status = UBSEC_STATUS_INVALID_PARAMETER;
            goto MCR_Done;
        }
        if (NumFrags > (UBSEC_MAX_FRAGMENTS + 1)) {
            Dbg_Print(DBG_PACKET,
                  ("ubsec:  Too Many Input fragments\n"));
            Status = UBSEC_STATUS_INVALID_PARAMETER;
            goto MCR_Done;
        }

        Dbg_Print(DBG_PACKET,
              ("ubsec: Num Input Frags %d \n", NumFrags));

        /* SetupInputFragmentList will always be successful here because of */
        /* the sanity checks performed above.                               */
        SetupInputFragmentList((MasterCommand_t *) pMCR,
                       (Packet_t *) pPacket, NumFrags,
                       pCommand->SourceFragments);

        /*
         * Now add the packet output fragment information
         * We need at least one fragment.
         */
        /* Sanity checks */
        if (!(NumFrags = pCommand->NumDestination)) {
            Dbg_Print(DBG_PACKET,
                  ("ubsec:  No Output fragments\n"));
            Status = UBSEC_STATUS_INVALID_PARAMETER;
            goto MCR_Done;
        }
        if (NumFrags > (UBSEC_MAX_FRAGMENTS + 1)) {
            Dbg_Print(DBG_PACKET,
                  ("ubsec:  Too Many Output fragments\n"));
            Status = UBSEC_STATUS_INVALID_PARAMETER;
            goto MCR_Done;
        }

        Dbg_Print(DBG_PACKET,
              ("ubsec: Num Output Frags %d \n", NumFrags));

        if (UBSEC_USING_MAC(pCommand->Command)) {
            /* We need an 'extra' fragment info struct for the auth data */
            ExtraFragment.FragmentAddress =
                pCommand->AuthenticationInfo.FragmentAddress;
            /* Easy to do check here for invalid 'extra' fragment address */
            if ((long) ExtraFragment.FragmentAddress & 0x03) {
                Dbg_Print(DBG_PACKET,
                      ("ubsec:  ################INVALID HMAC ADDRESS %08x\n",
                       ExtraFragment.FragmentAddress));
                Status = UBSEC_STATUS_INVALID_PARAMETER;
                goto Error_Return;
            }
            /* The CryptoNet chip knows how big the auth fragment is, but */
            /* SetupOutputFragmentList() needs to see a length of zero.   */
            ExtraFragment.FragmentLength = 0;
            pExtraFragment = &ExtraFragment;
        } else {    /* not doing authentication; pass NULL extra fragment info */
            pExtraFragment = (ubsec_FragmentInfo_pt) 0;
        }
        /* SetupOutputFragmentList() checks frag list for allowable fragment */
        /* addresses (4-byte aligned) and lengths (4-byte multiples).        */
        if (((pCommand->Command & UBSEC_NO_CRYPTO) || 
            (!(UBSEC_USING_CRYPT(pCommand->Command))) ) &&
            (UBSEC_USING_MAC(pCommand->Command))) {
            ubsec_FragmentInfo_t nullDestinationFragments ;
            nullDestinationFragments.FragmentLength=0;
            nullDestinationFragments.FragmentAddress=0;
            if (SetupOutputFragmentList((MasterCommand_t *) pMCR, (Packet_t *) pPacket, 1,
                     &nullDestinationFragments, pExtraFragment)) {
                Status = UBSEC_STATUS_INVALID_PARAMETER;
                goto Error_Return;
            }
        } else {
            if (SetupOutputFragmentList((MasterCommand_t *) pMCR, (Packet_t *) pPacket, NumFrags,
                     pCommand->DestinationFragments, pExtraFragment)) {
                Status = UBSEC_STATUS_INVALID_PARAMETER;
                goto Error_Return;
            }
        }

        /* Set up the context flags */
        if (pCommand->Command & UBSEC_ENCODE)
            pCipherContext->CryptoFlag = CF_ENCODE;
        else
            pCipherContext->CryptoFlag = CF_DECODE;

        if (UBSEC_USING_CRYPT(pCommand->Command)
            && !(pCommand->Command & UBSEC_NO_CRYPTO)) {
            pCipherContext->CryptoOffset =
                CPU_TO_CTRL_SHORT(pCommand->CryptHeaderSkip);
            pCipherContext->CryptoFlag |= CF_CRYPTO;
        }

        /* if UBSEC_USING_CRYPT */
        /* Find the Context Structure packing ptrs  and size */
        if (pCommand->Command & UBSEC_AES) {
#ifdef BCM_OEM_4
            /* CTR mode not supported in Explicit IV OPERATION */
            if (UBSEC_USING_EXPLICIT_IV(pCommand->Command)) {
                if (UBSEC_USING_CTR_MODE(pCommand->Command)) {
                    Dbg_Print(DBG_PACKET,
                          ("ubsec:  ################INVALID CTR MODE Crypto Key command %08x\n",
                           pCommand->Command));
                    Status = UBSEC_STATUS_INVALID_PARAMETER;
                    goto Error_Return;
                }
            }
#endif
            Context_CryptoKey_pt =
                (UBS_UINT32 *) & pCipherContext->Context[0];
            if (pCommand->Command & UBSEC_AES_128BITKEY) {
                CryptoKey_len = 16;    /* in bytes */
                /* pCipherContext->CryptoFlag |= CF_AES_128; *//* 0x0000 OR */
            } else if (pCommand->Command & UBSEC_AES_192BITKEY) {
                CryptoKey_len = 24;    /* in bytes */
                pCipherContext->CryptoFlag |= CF_AES_192;
            } else if (pCommand->Command & UBSEC_AES_256BITKEY) {
                CryptoKey_len = 32;    /* in bytes */
                pCipherContext->CryptoFlag |= CF_AES_256;
            } else {
                Dbg_Print(DBG_PACKET,
                      ("ubsec:  ################INVALID AES Crypto Key command %08x\n",
                       pCommand->Command));
                Status = UBSEC_STATUS_INVALID_PARAMETER;
                goto Error_Return;
            }
            CurContextSize = CryptoKey_len / 4;    /* in UINT32's */

#ifdef BCM_OEM_4
            /* IV for CBC; Initial count for CTR; EXPLICIT_IV is always false for CTR mode */
            if (UBSEC_USING_EXPLICIT_IV(pCommand->Command) !=
                UBSEC_EXPLICIT_IV)
#endif
            {
                Context_ComputedIV_IC_pt =
                    (UBS_UINT32 *) & pCipherContext->
                    Context[CurContextSize];
                ComputedIV_IC_len = 16;    /* in bytes */
                CurContextSize += 4;    /* 16/4 in UINT32's */
            }

            Context_HMAC_pt =
                (UBS_UINT32 *) & pCipherContext->
                Context[CurContextSize];
            CurContextSize += 10;    /*  UINT32's */

            /* Set the CTR Flag if req */
            if (UBSEC_USING_CTR_MODE(pCommand->Command))
                pCipherContext->CryptoFlag |= CF_AES_CTR;
        } else {    /* pCommand->Command Default 3DES for DES and 3DES *//* Default */
            Context_CryptoKey_pt =
                (UBS_UINT32 *) & pCipherContext->Context[0];
            CryptoKey_len = ((pCommand->Command & UBSEC_3DES) ? 24 : 8);    /* in bytes */
            CurContextSize = 6;    /* Always the chip does  3DES hence  24/4 in UINT32's */
#ifdef UBSEC_582x_CLASS_DEVICE
#ifdef BCM_OEM_4
            if (UBSEC_USING_EXPLICIT_IV(pCommand->Command) !=
                UBSEC_EXPLICIT_IV)
#endif
            {
                Context_ComputedIV_IC_pt =
                    (UBS_UINT32 *) & pCipherContext->
                    Context[CurContextSize];
                ComputedIV_IC_len = 8;    /* in bytes */
                CurContextSize += 2;    /* 8/4 in UINT32's */
            }
            Context_HMAC_pt =
                (UBS_UINT32 *) & pCipherContext->
                Context[CurContextSize];
            CurContextSize += 10;    /*  UINT32's */
#else                /* 580x */
            Context_HMAC_pt =
                (UBS_UINT32 *) & pCipherContext->
                Context[CurContextSize];
            CurContextSize += 10;    /*  UINT32's */
            Context_ComputedIV_IC_pt =
                (UBS_UINT32 *) & pCipherContext->
                Context[CurContextSize];
            ComputedIV_IC_len = 8;    /* in bytes */
            CurContextSize += 2;    /* 8/2  in UINT32's */
#endif                /* 582x */
        }        /* pCommand->Command */

        /* Copies only required if doing crypto */
        if (UBSEC_USING_CRYPT(pCommand->Command)
            && !(pCommand->Command & UBSEC_NO_CRYPTO)) {
            /* Copy the CryptoKey */
#if (UBS_CRYPTONET_ATTRIBUTE == UBS_BIG_ENDIAN)
            RTL_Memcpy(Context_CryptoKey_pt, pCommand->CryptKey,
                   CryptoKey_len);
#else
            copywords(Context_CryptoKey_pt,
                  (UBS_UINT32 *) pCommand->CryptKey,
                  CryptoKey_len / 4);
#endif

            /* Patch up for DES:  Chip does 3DES with same keys (k1=k2=k3) */
            if (pCommand->Command & UBSEC_DES) {
                RTL_Memcpy(Context_CryptoKey_pt + 8,
                       Context_CryptoKey_pt, CryptoKey_len);
                RTL_Memcpy(Context_CryptoKey_pt + 16,
                       Context_CryptoKey_pt, CryptoKey_len);
            }
#ifdef BCM_OEM_4
            /* Copy the Computed IV if !EXPLICIT_IV */
            /* for CTR mode EXPLICIT_IV is always false; so copy the Initial count  */
            if (UBSEC_USING_EXPLICIT_IV(pCommand->Command) !=
                UBSEC_EXPLICIT_IV)
#endif
            {
#if (UBS_CRYPTONET_ATTRIBUTE == UBS_BIG_ENDIAN)
                RTL_Memcpy(Context_ComputedIV_IC_pt,
                       &pCommand->InitialVector[0],
                       ComputedIV_IC_len);
#else
                copywords(Context_ComputedIV_IC_pt,
                      (UBS_UINT32 *) & pCommand->
                      InitialVector[0],
                      ComputedIV_IC_len / 4);
#endif
            }

        }

        /* NO_CRYPTO */
        /* If using HMAC then copy the authentication state to the context. */
        if (UBSEC_USING_MAC(pCommand->Command)) {
            RTL_Memcpy(Context_HMAC_pt,
                   pCommand->HMACState,
                   sizeof (ubsec_HMAC_State_t));
            if (UBSEC_MAC_MD5 & pCommand->Command)
                pCipherContext->CryptoFlag |= CF_MD5;
            else if (UBSEC_MAC_SHA1 & pCommand->Command)
                pCipherContext->CryptoFlag |= CF_SHA1;
        }
#ifdef UBSEC_582x_CLASS_DEVICE
        /* Some extra fields to be filled in . */
        pContext->cmd_structure_length = CPU_TO_CTRL_SHORT(8 + CurContextSize * 4);    /* For operationtype/length(4) + Offset/flag(4) + context */
#endif                /* UBSEC_582x_CLASS_DEVICE */

        Dbg_Print(DBG_PACKET,
              ("ubsec:  CryptoOffset and Flag [%04x][%04x]\n",
               CTRL_TO_CPU_SHORT(pCipherContext->CryptoOffset),
               CTRL_TO_CPU_SHORT(pCipherContext->CryptoFlag)));

#ifdef DEBUG
        Dbg_Print(DBG_PACKET,
              ("ubsec:PacketContext:  cmd_structure_length and operation_type [%04x][%04x]\n",
               CTRL_TO_CPU_SHORT(pContext->operation_type),
               CTRL_TO_CPU_SHORT(pContext->cmd_structure_length)));
        Dbg_Print(DBG_PACKET,
              ("ubsec:  CryptoOffset and Flag [%04x][%04x]\n",
               CTRL_TO_CPU_SHORT(pCipherContext->CryptoOffset),
               CTRL_TO_CPU_SHORT(pCipherContext->CryptoFlag)));

        {
            int WordLen, i;
            long *p = (long *) &(pCipherContext->Context[0]);
            Dbg_Print(DBG_CMD, ("In cmd: Context -[physicalAddress %08x] ", p));    //context->PhysicalAddress));
            WordLen =
                (CTRL_TO_CPU_SHORT(pContext->cmd_structure_length))
                / 4;
            for (i = 0; i < WordLen; i++, p++) {
                Dbg_Print(DBG_CMD,
                      ("%08x ", CPU_TO_CTRL_LONG(*p)));
            }
            Dbg_Print(DBG_CMD, ("]\n"));
        }
#endif

#ifdef UBSEC_STATS
        if (pCipherContext->CryptoFlag & CF_DECODE) {
            pDevice->Statistics.BlocksDecryptedCount++;
            pDevice->Statistics.BytesDecryptedCount +=
                CTRL_TO_CPU_SHORT(pPacket->PacketLength);
        } else {
            pDevice->Statistics.BlocksEncryptedCount++;
            pDevice->Statistics.BytesEncryptedCount +=
                CTRL_TO_CPU_SHORT(pPacket->PacketLength);
        }
#endif

        /* Now inc the number of packets and prepare for the next command. */
        pMCR->NumberOfPackets++;
        pCommand++;
        PacketIndex++;
        pPacket++;
        pContext++;

    }            /* For NumCommands-- */

    /*
     * If we are here then the last packet(s) (commands) have been added to
     * the current MCR.
     * Push the MCR to the device. 
     */
      MCR_Done:
    *NumCommands = CommandIndex;    /* Update number completed */

    /* For crypto MCRs, the contexts are accessed using a single handle   */
    /* for an array of contexts. This means that all contexts for an MCR  */
    /* are contiguous in memory, and that we can sync all contexts at     */
    /* once (now that we know that we're finished loading this MCR).      */
    /* Make DMA memory actually hold CPU-initialized context data         */
    Dbg_Print(DBG_CNTXT_SYNC,
          ("ubsec: ubsec_CipherCommand Sync %d Contexts to Device (0x%08X,%d,%d)\n",
           pMCR->NumberOfPackets, pMCR->ContextListHandle[0], 0,
           pMCR->NumberOfPackets * sizeof (PacketContext_t)));
    OS_SyncToDevice(pMCR->ContextListHandle[0], 0,
            pMCR->NumberOfPackets * sizeof (PacketContext_t));

    /*DumpCipherMCR(pMCR); *//* Test  */

    PushMCR(pDevice);

#ifdef BLOCK
    /* Wait for all outstanding  to complete */
    while ((Status =
        WaitForCompletion(pDevice, (UBS_UINT32) 100000,
                  UBSEC_CIPHER_LIST))
           == UBSEC_STATUS_SUCCESS) ;
    if (Status != UBSEC_STATUS_TIMEOUT)    /* We are nested, return success */
        Status = UBSEC_STATUS_SUCCESS;
      Error_Return:
#else

      Error_Return:        /* Label to make sure that IRQs are enabled. */
#ifdef COMPLETE_ON_COMMAND_THREAD
    ubsec_PollDevice(pDevice);    /* Try to complete some & cut down on ints */
#endif
#endif
    OS_LeaveCriticalSection(pDevice, SaveConfig);

#ifdef UBSEC_STATS
    if (Status != UBSEC_STATUS_SUCCESS)
        pDevice->Statistics.CryptoFailedCount++;
#endif
    return (Status);
}
#endif // Pavan

/*
 * InitHMACState: Initialize the inner and outer state of the HMAC
 *
 * This is to allow it to be used for the authentication commands.
 *
 */
ubsec_Status_t
ubsec_InitHMACState(ubsec_HMAC_State_pt HMAC_State,
            ubsec_CipherCommand_t type, ubsec_HMAC_Key_pt Key)
{
    memset(HMAC_State, 0, sizeof (*HMAC_State));
    if (type == UBSEC_MAC_SHA1) {
        InitSHA1State(HMAC_State, Key);
        return (UBSEC_STATUS_SUCCESS);
    } else if (type == UBSEC_MAC_MD5) {
        InitMD5State(HMAC_State, Key, 1);
        return (UBSEC_STATUS_SUCCESS);
    }
    return (UBSEC_STATUS_INVALID_PARAMETER);
}

ubsec_Status_t
ubsec_InitCipherContext(ubsec_PacketContext_t * context,
            ubsec_CipherCommand_t command,
            ubsec_CipherContextInfo_t * pCipherInfo)
{
    volatile PacketContext_t *pContext;
    volatile CipherContext_t *pCipherContext;

    ubsec_HMAC_State_t HMAC_State;

    unsigned int *Context_CryptoKey_pt = NULL;
    int CryptoKey_len = 0;
    unsigned int *Context_HMAC_pt = NULL;

    int CurContextSize = 0;    /* in unsigned int's */

    if (context->PhysicalAddress == (unsigned int) NULL) {
        printk(KERN_ERR 
              "ubsec: INVALID Context. context->PhysicalAddress is NULL.");
        goto Error_Return;
    }

    pContext = (PacketContext_t *) context;
    pCipherContext = (CipherContext_t *) & context->ContextArray[1];

    if ((command & UBSEC_AES) == UBSEC_AES) {
        if (UBSEC_USING_EXPLICIT_IV(command))
            pContext->operation_type =
                OPERATION_IPSEC_AES_EXPLICIT_IV;
        else
            pContext->operation_type = OPERATION_IPSEC_AES;
    } else {
        if (UBSEC_USING_EXPLICIT_IV(command))
            pContext->operation_type =
                OPERATION_IPSEC_3DES_EXPLICIT_IV;
        else
            pContext->operation_type = OPERATION_IPSEC;    /* Default */
    }

    if (command & UBSEC_ENCODE)
        pCipherContext->CryptoFlag = CF_ENCODE;
    else
        pCipherContext->CryptoFlag = CF_DECODE;

    if (UBSEC_USING_CRYPT(command) && !(command & UBSEC_NO_CRYPTO)) {
        pCipherContext->CryptoOffset =
            pCipherInfo->CryptHeaderSkip;
        pCipherContext->CryptoFlag |= CF_CRYPTO;
    }

    /* if UBSEC_USING_CRYPT */
    /* Find the Context Structure packing ptrs  and size */
    if (command & UBSEC_AES) {
        Context_CryptoKey_pt =
            (unsigned int *) & pCipherContext->Context[0];
        /* CTR mode not supported in Explicit IV OPERATION */
        if (UBSEC_USING_EXPLICIT_IV(command)) {
            if (UBSEC_USING_CTR_MODE(command)) {
                printk(KERN_ERR "ubsec: INVALID CTR MODE Crypto Key command %x\n",
                       (int)command);
                goto Error_Return;
            }
        }
        if (command & UBSEC_AES_128BITKEY) {
            CryptoKey_len = 16;    /* in bytes */
            /* pCipherContext->CryptoFlag |= CF_AES_128; *//* 0x0000 OR */
        } else if (command & UBSEC_AES_192BITKEY) {
            CryptoKey_len = 24;    /* in bytes */
            pCipherContext->CryptoFlag |= CF_AES_192;
        } else if (command & UBSEC_AES_256BITKEY) {
            CryptoKey_len = 32;    /* in bytes */
            pCipherContext->CryptoFlag |= CF_AES_256;
        } else {
            printk(KERN_ERR "ubsec: INVALID AES Crypto Key command %x\n",
                   (int)command);
            goto Error_Return;
        }
        CurContextSize = CryptoKey_len / 4;    /* in UINT32's */

        Context_HMAC_pt =
            (unsigned int *) & pCipherContext->Context[CurContextSize];
        CurContextSize += 10;    /*  UINT32's */

        /* Set the CTR Flag if req */
        if (UBSEC_USING_CTR_MODE(command))
            pCipherContext->CryptoFlag |= CF_AES_CTR;

    } else {        /* pCommand->Command Default 3DES for DES and 3DES *//* Default */
        Context_CryptoKey_pt =
            (unsigned int *) & pCipherContext->Context[0];
        CryptoKey_len = ((command & UBSEC_3DES) ? 24 : 8);    /* in bytes */
        CurContextSize = 6;    /* Always the chip does  3DES hence  24/4 in UINT32's */
        Context_HMAC_pt =
            (unsigned int *) & pCipherContext->Context[CurContextSize];
        CurContextSize += 10;    /*  UINT32's */
        printk(KERN_ERR "ubsec:PacketContext: pCryptoKey: %x, pHMAC: %x\n",
               (int)Context_CryptoKey_pt, (int)Context_HMAC_pt);
    }            /* command */

    /* Copies only required if doing crypto */
    if (UBSEC_USING_CRYPT(command) && !(command & UBSEC_NO_CRYPTO)) {
        /* Copy the CryptoKey */
        memcpy((void *)Context_CryptoKey_pt, (void *)pCipherInfo->CryptKey,
               CryptoKey_len);
        /* Patch up for DES:  Chip does 3DES with same keys (k1=k2=k3) */
        if (command & UBSEC_DES) {
            memcpy(Context_CryptoKey_pt + 8,
                   Context_CryptoKey_pt, CryptoKey_len);
            memcpy(Context_CryptoKey_pt + 16,
                   Context_CryptoKey_pt, CryptoKey_len);
        }

    }

    /* NO_CRYPTO */
    /* If using HMAC then copy the authentication state to the context. */
    if (UBSEC_USING_MAC(command)) {

        ubsec_InitHMACState(&HMAC_State,
                    UBSEC_USING_MAC(command), pCipherInfo->Key);
        memcpy(Context_HMAC_pt,
               &HMAC_State, sizeof (ubsec_HMAC_State_t));
        if (UBSEC_MAC_MD5 & command)
            pCipherContext->CryptoFlag |= CF_MD5;
        else if (UBSEC_MAC_SHA1 & command)
            pCipherContext->CryptoFlag |= CF_SHA1;
    }

    /*
     * Some extra fields to be filled in.
     * For operationtype/length(4) + Offset/flag(4) + context
     */
    pContext->cmd_structure_length = 8 + CurContextSize * 4;    

#ifdef SPU_DEBUG
    printk(KERN_ERR
           "ubsec:PacketContext: cmd_structure_length "
           "and operation_type [%04x][%04x]\n",
           pContext->operation_type,
           pContext->cmd_structure_length);
    printk(KERN_ERR "ubsec:  CryptoOffset and Flag [%04x][%04x]\n",
           pCipherContext->CryptoOffset,
           pCipherContext->CryptoFlag);

    {
        int WordLen, i;
        long *p = (long *) &(pCipherContext->Context[0]);
        printk(KERN_ERR "In initConext: Context -[physicalAddress %08x] ", p);
        WordLen =
            (pContext->cmd_structure_length) / 4;
        for (i = 0; i < WordLen; i++, p++) {
            printk(KERN_ERR "%08x ", *p);
        }
        printk(KERN_ERR "]\n");
    }
#endif
    return UBSEC_STATUS_SUCCESS;

      Error_Return:
    return UBSEC_STATUS_INVALID_PARAMETER;
}

#if 0 // Pavan
ubsec_Status_t
ubsec_CipherCommand_withSC(ubsec_DeviceContext_t Context,
               ubsec_CipherCommandInfo_withSC_t * pCommand,
               int *NumCommands)
{
    DeviceInfo_pt pDevice = (DeviceInfo_pt) Context;
    VOLATILE MasterCommand_t *pMCR;
    VOLATILE Packet_t *pPacket;
    /*VOLATILE PacketContext_t  *pContext; */
    VOLATILE ubsec_PacketContext_t *pContext;
    VOLATILE CipherContext_t *pCipherContext;
    VOLATILE int PacketIndex;
    VOLATILE int NumFrags;    /* Number of fragments */
    ubsec_FragmentInfo_t ExtraFragment, *pExtraFragment;
    int CommandIndex = 0;
    int CommandCount = *NumCommands;
    ubsec_Status_t Status;
    UBS_UINT32 SaveConfig = 0;

    printk(KERN_ERR "ubsec:  ubsec command %d", *NumCommands);

    /*
     * Check some parameters
     */
    if (pDevice == NULL_DEVICE_INFO) {
        printk(KERN_ERR "NO DEV\n ");
        return (UBSEC_STATUS_NO_DEVICE);
    }
    printk(KERN_ERR "\n");

    if (OS_EnterCriticalSection(pDevice, SaveConfig)) {
        return (UBSEC_STATUS_DEVICE_BUSY);
    }

    /* Get the next MCR to load */
      Get_New_MCR:
    *NumCommands = CommandIndex;    /* Update number completed */
    if ((pMCR =
         GetFreeMCR(pDevice, UBSEC_CIPHER_LIST,
            &Status)) == NULL_MASTER_COMMAND) {
        Dbg_Print(DBG_CMD_FAIL,
              ("ubsec: device busy MCR %x\n", Status));
        goto Error_Return;
    }

    /* Add packets to this MCR. */

    Dbg_Print(DBG_CMD,
          ("ubsec: mcr_index %d MCR %0x\n", pMCR->Index, pMCR));
    /* Initialize the packet information */

    PacketIndex = pMCR->NumberOfPackets;
    pPacket = &(pMCR->PacketArray[PacketIndex]);    /* Set up the current packet. */
    /*pContext = &pMCR->ContextList[PacketIndex]; */
    pContext = (ubsec_PacketContext_t *) pCommand->pContext;
    Status = UBSEC_STATUS_SUCCESS;    /* Wishful thinking? */

    /* Process all the commands in the command list. */
    for (; CommandIndex < CommandCount; CommandIndex++) {    /* Add all the packets to the MCR */
        if (PacketIndex >= MCR_MAXIMUM_PACKETS) {
            Dbg_Print(DBG_CMD,
                  ("ubsec:  overran mcr buffer. %d %d\n",
                   PacketIndex, CommandIndex));
            /* 
             * We have filled this MCR with the max # of packets,
             * but still have more packets (commands) to do.
             * Advance next free. Wrap around if necessary
             */
            pDevice->NextFreeMCR[UBSEC_CIPHER_LIST] =
                (MasterCommand_pt) pMCR->pNextMCR;

            /* For crypto MCRs, the contexts are accessed using a single handle   */
            /* for an array of contexts. This means that all contexts for an MCR  */
            /* are contiguous in memory, and that we can sync all contexts at     */
            /* once (now that we know that we're finished loading this MCR).      */
            /* Make DMA memory actually hold CPU-initialized context data         */
            Dbg_Print(DBG_CNTXT_SYNC,
                  ("ubsec: ubsec_CipherCommand Sync %d Contexts to Device (0x%08X,%d,%d)\n",
                   pMCR->NumberOfPackets,
                   pMCR->ContextListHandle[0], 0,
                   pMCR->NumberOfPackets *
                   sizeof (PacketContext_t)));
            OS_SyncToDevice(pMCR->ContextListHandle[0], 0,
                    pMCR->NumberOfPackets *
                    sizeof (PacketContext_t));

            PushMCR(pDevice);    /* Get it going (pipeline) */
            goto Get_New_MCR;    /* Try to add to the next MCR */
        }

        /* Save the callback information. */
        pMCR->CompletionArray[PacketIndex].CompletionCallback =
            pCommand->CompletionCallback;
        pMCR->CompletionArray[PacketIndex].CommandContext =
            pCommand->CommandContext;

        /* Now set up the packet processing parameters */
        Dbg_Print(DBG_PACKET,
              ("ubsec: packet_Index %d, Context Buf %0x\n",
               PacketIndex, pContext));
        pPacket->PacketContextBuffer = pContext->PhysicalAddress;

        /*
         * Now add the packet input fragment information
         * First fragment will need to skip the MAC Header
         * We need at least one fragment.
         */
        /* Sanity checks. */
        if (!(NumFrags = pCommand->NumSource)) {
            Dbg_Print(DBG_PACKET, ("ubsec:  No Input fragments\n"));
            Status = UBSEC_STATUS_INVALID_PARAMETER;
            goto MCR_Done;
        }
        if (NumFrags > (UBSEC_MAX_FRAGMENTS + 1)) {
            Dbg_Print(DBG_PACKET,
                  ("ubsec:  Too Many Input fragments\n"));
            Status = UBSEC_STATUS_INVALID_PARAMETER;
            goto MCR_Done;
        }

        Dbg_Print(DBG_PACKET,
              ("ubsec: Num Input Frags %d \n", NumFrags));

        /* SetupInputFragmentList will always be successful here because of */
        /* the sanity checks performed above.                               */
        SetupInputFragmentList((MasterCommand_t *) pMCR,
                       (Packet_t *) pPacket, NumFrags,
                       pCommand->SourceFragments);

        /*
         * Now add the packet output fragment information
         * We need at least one fragment.
         */
        /* Sanity checks */
        if (!(NumFrags = pCommand->NumDestination)) {
            Dbg_Print(DBG_PACKET,
                  ("ubsec:  No Output fragments\n"));
            Status = UBSEC_STATUS_INVALID_PARAMETER;
            goto MCR_Done;
        }
        if (NumFrags > (UBSEC_MAX_FRAGMENTS + 1)) {
            Dbg_Print(DBG_PACKET,
                  ("ubsec:  Too Many Output fragments\n"));
            Status = UBSEC_STATUS_INVALID_PARAMETER;
            goto MCR_Done;
        }

        Dbg_Print(DBG_PACKET,
              ("ubsec: Num Output Frags %d \n", NumFrags));

        if (UBSEC_USING_MAC(pCommand->Command)) {
            /* We need an 'extra' fragment info struct for the auth data */
            ExtraFragment.FragmentAddress =
                pCommand->AuthenticationInfo.FragmentAddress;
            /* Easy to do check here for invalid 'extra' fragment address */
            if ((long) ExtraFragment.FragmentAddress & 0x03) {
                Dbg_Print(DBG_PACKET,
                      ("ubsec:  ################INVALID HMAC ADDRESS %08x\n",
                       ExtraFragment.FragmentAddress));
                Status = UBSEC_STATUS_INVALID_PARAMETER;
                goto Error_Return;
            }
            /* The CryptoNet chip knows how big the auth fragment is, but */
            /* SetupOutputFragmentList() needs to see a length of zero.   */
            ExtraFragment.FragmentLength = 0;
            pExtraFragment = &ExtraFragment;
        } else {    /* not doing authentication; pass NULL extra fragment info */
            pExtraFragment = (ubsec_FragmentInfo_pt) 0;
        }
        /* SetupOutputFragmentList() checks frag list for allowable fragment */
        /* addresses (4-byte aligned) and lengths (4-byte multiples).        */
        if (pCommand->Command & UBSEC_NO_CRYPTO) {
            ubsec_FragmentInfo_t nullDestinationFragments ;
            nullDestinationFragments.FragmentLength=0;
            nullDestinationFragments.FragmentAddress=0;
            if (SetupOutputFragmentList((MasterCommand_t *) pMCR, (Packet_t *) pPacket, 1,
                     &nullDestinationFragments, pExtraFragment)) {
                Status = UBSEC_STATUS_INVALID_PARAMETER;
                goto Error_Return;
            }
        } else {
            if (SetupOutputFragmentList((MasterCommand_t *) pMCR, (Packet_t *) pPacket, NumFrags,
                     pCommand->DestinationFragments, pExtraFragment)) {
                Status = UBSEC_STATUS_INVALID_PARAMETER;
                goto Error_Return;
            }
        }

        /* Now inc the number of packets and prepare for the next command. */
        pMCR->NumberOfPackets++;
        pCommand++;
        PacketIndex++;
        pPacket++;
        pContext++;

    }            /* For NumCommands-- */

    /*
     * If we are here then the last packet(s) (commands) have been added to
     * the current MCR.
     * Push the MCR to the device. 
     */
      MCR_Done:
    *NumCommands = CommandIndex;    /* Update number completed */

    /* For crypto MCRs, the contexts are accessed using a single handle   */
    /* for an array of contexts. This means that all contexts for an MCR  */
    /* are contiguous in memory, and that we can sync all contexts at     */
    /* once (now that we know that we're finished loading this MCR).      */
    /* Make DMA memory actually hold CPU-initialized context data         */
    Dbg_Print(DBG_CNTXT_SYNC,
          ("ubsec: ubsec_CipherCommand Sync %d Contexts to Device (0x%08X,%d,%d)\n",
           pMCR->NumberOfPackets, pMCR->ContextListHandle[0], 0,
           pMCR->NumberOfPackets * sizeof (PacketContext_t)));
    OS_SyncToDevice(pMCR->ContextListHandle[0], 0,
            pMCR->NumberOfPackets * sizeof (PacketContext_t));

    /*DumpCipherMCR(pMCR); *//* Test  */

    PushMCR(pDevice);

    /* Wait for all outstanding  to complete */
    //printk("going to wait now.......\n");
    while ((Status =
        WaitForCompletion(pDevice, (UBS_UINT32) 100000,
                  UBSEC_CIPHER_LIST))
           == UBSEC_STATUS_SUCCESS) ;
    if (Status != UBSEC_STATUS_TIMEOUT)    /* We are nested, return success */
        Status = UBSEC_STATUS_SUCCESS;
      Error_Return:
    OS_LeaveCriticalSection(pDevice, SaveConfig);

    return (Status);
}
#endif // Pavan

MODULE_LICENSE("GPL");
