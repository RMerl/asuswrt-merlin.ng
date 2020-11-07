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
// File Name  : spuipsec.c
//
// Description: 
//               
//**************************************************************************
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif
#include <linux/module.h>
#include <net/inet_ecn.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/xfrm.h>
#include <net/esp.h>
#include <net/ah.h>
#include <asm/scatterlist.h>
#include <linux/crypto.h>
#include <linux/pfkeyv2.h>
#include <linux/random.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/if_arp.h>
#include <asm/uaccess.h>
#include "bcm_map_part.h"
#include "bcm_intr.h"
#include "bcmspudrv.h"
#include "spudrv.h"
#include <board.h>
#include "bcmtypes.h"
#include "bcm_mm.h"
#include "bcmipsec.h"
#include "spuipsec.h"
#include "spucrypt.h"
#include "srl5862.h"
#include "bcmsad.h"
#include "cnctx.h"
#include "spuipsec.h"
#include <linux/hw_random.h>
#include "spu.h"
#include <linux/spinlock.h>
#include <net/ah.h>

/* External Definitions */
extern pspu_dev_ctrl_t pdev_ctrl;
extern void spu_assign_output_desc (unsigned char *buf, uint16_t len, uint16_t flags);
extern void spu_assign_input_desc (unsigned char *buf,
					 uint16_t len, uint16_t flags);
extern void spu_dump_array(char *msg, unsigned char *buf, uint16 len);
extern unsigned long GetPhysicalAddress_ForOffset (void *ptr, int offset);
extern unsigned long GetVirtualAddress (void *ptr);
extern void spu_dma_enable (void);

static int spu_SCTXSA_getsize (BCM_CRYPTOOP * op, uint16_t * size)
{
    int status = BCM_STATUS_OK;
    *size = sizeof (SCTX);

    /* Add the size of the context for encryption operation */
    switch (op->encrAlg)
    {
	case BCMSAD_ENCR_ALG_NULL:
	    break;

	case BCMSAD_ENCR_ALG_DES:
	    *size += BCMSAD_ENCR_KEYLEN_DES;
	    break;

	case BCMSAD_ENCR_ALG_3DES:
	    *size += BCMSAD_ENCR_KEYLEN_3DES;
	    break;

	case BCMSAD_ENCR_ALG_AES:
	case BCMSAD_ENCR_ALG_AES_CTR:

	    switch (op->encrKeyLen)
	    {
		case BCMSAD_ENCR_KEYLEN_AES128:
		case BCMSAD_ENCR_KEYLEN_AES192:
		case BCMSAD_ENCR_KEYLEN_AES256:
		    *size += op->encrKeyLen;
		    break;

		default:

		    status = BCM_STATUS_INVALID_INPUT;
		    printk (KERN_ERR "SCTXSA_GetSize bad encrKeyLen %x\n",
                                      op->encrKeyLen);
	    }
	    break;
        case BCMSAD_ENCR_ALG_NONE:
	    break;

	default:
	    status = BCM_STATUS_INVALID_INPUT;
	    printk (KERN_ERR "SCTXSA_GetSize bad encrAlg %x\n", op->encrAlg);
    }

    /* Add the context for HMAC operation */
    switch (op->authAlg)
    {
	case BCMSAD_AUTH_ALG_SHA1:
	case BCMSAD_AUTH_ALG_MD5:
	    *size += 2 * op->authKeyLen; /* inner and outer auth contexts */
	    break;

	case BCMSAD_AUTH_ALG_NONE:
	    break;

	default:
	    status = BCM_STATUS_INVALID_INPUT;
	    printk (KERN_ERR "SCTXSA_GetSize bad authAlg %x\n", op->authAlg);
    }

    return status;
} /* spu_SCTXSA_getsize */

static int spu_cch_sctx_init (BCM_CRYPTOOP * op, 
                              CCH_SCTX * cch_sctx, 
                              SACTX * devsa,
                              int sctxSize, 
                              u_int16_t * supdtFrag, 
                              u_int16_t * spsFrag,
                              u_int16_t * tunnelSize, 
                              u_int16_t * encapSize,
                              u_int16_t * sauSize, 
                              u_int16_t * spdSize)
{
    SCTX *sctx = NULL;
    CCH *cch = NULL;
    int status = BCM_STATUS_OK;

    memset (cch_sctx, 0, sizeof (CCH_SCTX));
    cch = &cch_sctx->cch;
    sctx = &cch_sctx->sctx;

    /* Write CCH fields */
    cch->field.flag.SCTX_PR = 1;
    cch->field.flag.BDESC_PR_HS_PR = 1;
    //cch->field.flag.MFM_PR = 1;
    cch->field.flag.BD_PR = 1;
    //dmaCtxSize = sctxSize + sizeof(CCH); 
    cch->length = sctxSize + sizeof (CCH) - ECH_SIZE;
    //cch->length = currentDMAOffset;

    //cch->ech = glb_req; /* FIXME: */
    //cch->ech = &glb_spu_transactions[glb_idx];

    /* Write SCTX protocol fields (word1) */
    sctx->protocol.ipsec_flags.type = TYPE_GENERIC;
#ifdef BCMAPI_USE_CACHED_SA
    sctx->protocol.ipsec_flags.cacheable = 0;
#endif
    if (op->flags.mode == BCMSAD_MODE_TRANSPORT)
	sctx->protocol.ipsec_flags.transport = 1;

    if (op->flags.proto == BCMSAD_PROTOCOL_ESP)
	sctx->protocol.ipsec_flags.ESP = 0;

    if (op->flags.proto == BCMSAD_PROTOCOL_AH)
	sctx->protocol.ipsec_flags.AH = 1;
    if (op->flags.dir == BCMSAD_DIR_OUTBOUND)
    {
	sctx->protocol.ipsec_flags.DF_Copy = 0;
    }

#ifdef DEC_TTL
    sctx->protocol.ipsec_flags.decrement_TTL = 0;
#endif

    if (op->flags.mode == BCMSAD_MODE_TUNNEL)
    {
	sctx->protocol.ipsec_flags.copy_TOS = 0;
	sctx->protocol.ipsec_flags.copy_Flow = 0;
    }

    sctx->protocol.ipsec_flags.cap_en = 0;
    sctx->protocol.ipsec_flags.update_en = 0;
    sctx->protocol.ipsec_flags.IPv4ChkSum = 0;

    if (op->flags.dir == BCMSAD_DIR_INBOUND)
    {
	devsa->inbound = 1;
	sctx->protocol.ipsec_flags.pad_en_pad_chk = 0;

#ifndef BCMAPI_USE_INBOUND_SPD_EN
	*supdtFrag = sizeof (SCTX_INSAU);
#endif

    }
    else if (op->flags.dir == BCMSAD_DIR_OUTBOUND)
    {
	//cch->field.flag.HASH_PR  = 1;
	sctx->protocol.ipsec_flags.pad_en_pad_chk = 0;
	*supdtFrag = sizeof (SCTX_OUTSAU);
    }

    switch (op->flags.dir)
    {
	case BCMSAD_DIR_INBOUND:
	    if (op->flags.mode == BCMSAD_MODE_TRANSPORT)
		cch->opCode = OPERATION_IPSEC_GENERIC;
	    else
		cch->opCode = OPERATION_IPSEC_GENERIC;

	    sctx->cipher.flags.inbound = 1;
	    //*sauSize = sizeof(SCTX_INSAU);
	   
#ifdef BCMAPI_USE_INBOUND_SPD_EN
	    /* Add context for SPD */
	    if (op->flags.ipv4)
		*spdSize = sizeof (SPD_V4);
	    if (op->flags.ipv6)
		*spdSize = sizeof (SPD_V6);
#endif
	    break;

	case BCMSAD_DIR_OUTBOUND:
	    if (op->flags.mode == BCMSAD_MODE_TRANSPORT)
		cch->opCode = OPERATION_IPSEC_GENERIC;
	    else
	    {
		cch->opCode = OPERATION_IPSEC_GENERIC;

		if (op->flags.ipv4)
		{
		    //tunnelSize = SIZE_IPV4_HEADER;
		    SPU_DATA_DUMP("IPV4 tunnelHeader", devsa->out.tunnelHeader,
			 SIZE_IPV4_HEADER);
		}

		if (op->flags.ipv6)
		{
		    *tunnelSize = SIZE_IPV6_HEADER;
		    SPU_DATA_DUMP("IPV6 tunnelHeader", devsa->out.tunnelHeader,
			 SIZE_IPV6_HEADER);
		}
	    }
	    //*sauSize = sizeof(SCTX_OUTSAU);
	    break;

	default:
	    status = BCM_STATUS_INVALID_INPUT;
	    printk (KERN_ERR "spu_cch_sctx_init bad mode %x\n", 
                              op->flags.mode);
    }

    /* Set SCTX cipher flags (word2) */
    /* Set authentication parameters */
    switch (op->authAlg)
    {
	case BCMSAD_AUTH_ALG_NONE:
	    SPU_TRACE(("Auth none\n"));
	    sctx->cipher.flags.hashAlg = HASH_ALG_NULL;
	    sctx->cipher.flags.hashMode = HASH_MODE_HASH;
	    break;

	case BCMSAD_AUTH_ALG_MD5:
	    SPU_TRACE(("Auth HMAC MD5\n"));
	    sctx->cipher.flags.hashAlg = HASH_ALG_MD5;
	    sctx->cipher.flags.hashMode = HASH_MODE_HMAC;
	    sctx->cipher.flags.hashType = HASH_TYPE_11;
	    devsa->flags.auth = 1;
	    break;

	case BCMSAD_AUTH_ALG_SHA1:
	    SPU_TRACE(("Auth HMAC SHA1\n"));
	    sctx->cipher.flags.hashAlg = HASH_ALG_SHA1;
	    sctx->cipher.flags.hashMode = HASH_MODE_HMAC;
	    //sctx->cipher.flags.hashType = HASH_TYPE_11;
	    sctx->cipher.flags.hashType = HASH_TYPE_00;
	    devsa->flags.auth = 1;
	    break;

	default:
	    status = BCM_STATUS_INVALID_INPUT;
	    printk (KERN_ERR "spu_cch_sctx_init bad authAlg %x\n",
                              op->authAlg);
    }

    /* Set encryption parameters */
    switch (op->encrAlg)
    {
	case BCMSAD_ENCR_ALG_NULL:
	    SPU_TRACE(("Crypto  none\n"));
	    break;

	case BCMSAD_ENCR_ALG_DES:
	    SPU_TRACE(("Crypto  DES CBC\n"));
	    sctx->cipher.flags.cryptoMode = CRYPTO_MODE_CBC;
	    sctx->cipher.flags.cryptoAlg = CRYPTO_ALG_DES;
	    sctx->ecf.flags.explicit_IV_size = 2;
	    devsa->ivLen = UBSEC_IV_LENGTH_BYTES;
	    break;

	case BCMSAD_ENCR_ALG_3DES:
	    SPU_TRACE(("Crypto  3DES CBC\n"));
	    sctx->cipher.flags.cryptoMode = CRYPTO_MODE_CBC;
	    sctx->cipher.flags.cryptoAlg = CRYPTO_ALG_DES3;
	    sctx->ecf.flags.explicit_IV_size = 2;
	    devsa->ivLen = UBSEC_IV_LENGTH_BYTES;
	    break;

	case BCMSAD_ENCR_ALG_AES:
	case BCMSAD_ENCR_ALG_AES_CTR:
	    if (op->encrAlg == BCMSAD_ENCR_ALG_AES)
	    {
		SPU_TRACE(("Crypto  AES CBC\n"));
		sctx->cipher.flags.cryptoMode = CRYPTO_MODE_CBC;
		sctx->ecf.flags.explicit_IV_size = 4;
		devsa->ivLen = UBSEC_AES_IV_LENGTH_BYTES;
	    }
	    else if (op->encrAlg == BCMSAD_ENCR_ALG_AES_CTR)
	    {
		SPU_TRACE(("Crypto  AES CTR\n"));
		sctx->cipher.flags.cryptoMode = CRYPTO_MODE_CTR;
		sctx->ecf.flags.explicit_IV_size = 2;
		sctx->ecf.flags.SCTX_IV = 1;
		/* Counter Mode IV length is 64 bits */
		devsa->ivLen = UBSEC_IV_LENGTH_BYTES;
	    }
	    sctx->cipher.flags.cryptoAlg = CRYPTO_ALG_AES;

	    switch (op->encrKeyLen)
	    {
		case BCMSAD_ENCR_KEYLEN_AES128:
		    SPU_TRACE(("AES keylength 128\n"));
		    sctx->cipher.flags.cryptoType = CRYPTO_TYPE_KEY128;
                    break;

		case BCMSAD_ENCR_KEYLEN_AES192:
		    SPU_TRACE(("AES keylength 192\n"));
		    sctx->cipher.flags.cryptoType = CRYPTO_TYPE_KEY192;
		    break;

		case BCMSAD_ENCR_KEYLEN_AES256:
		    SPU_TRACE(("AES keylength 256\n"));
		    sctx->cipher.flags.cryptoType = CRYPTO_TYPE_KEY256;
		    break;

		default:
		    status = BCM_STATUS_INVALID_INPUT;
		    printk (KERN_ERR "spu_cch_sctx_init bad AES keylen %d\n",
                                      op->encrKeyLen);
	    }
	    break;

        case BCMSAD_ENCR_ALG_NONE:
	    break;

	default:
	    status = BCM_STATUS_INVALID_INPUT;
	    printk (KERN_ERR "spu_cch_sctx_init bad encrAlg %x\n", 
                              op->encrAlg);
    }

    if (op->flags.proto == BCMSAD_PROTOCOL_ESP)
    {
	switch (op->flags.dir)
	{
	    case BCMSAD_DIR_OUTBOUND:
		SPU_TRACE(("ESP OUTBOUND\n"));
		SPU_TRACE(("encapSize %d esp_header %d ivlen %d\n",
                         *encapSize, SIZE_ESP_HEADER, devsa->ivLen));

		if (op->encrAlg != BCMSAD_ENCR_ALG_NULL)
		{
		    sctx->ecf.flags.gen_IV = 0;

		    /* Use explicit IV */
		    sctx->ecf.flags.explicit_IV = 1;
		}

		if (op->authAlg != BCMSAD_AUTH_ALG_NONE)
		{
#ifdef BCMAPI_USE_MAC_CKSUM
		    sctx->ecf.flags.insert_icv_mac = 1;
#endif
		    sctx->ecf.flags.ICV_MAC_size = 3;
		}
		break;

	    case BCMSAD_DIR_INBOUND:
		//cch->field.flag.BCT_PR_KPARAM_PR = 1;
		cch->field.flag.BCT_PR_KPARAM_PR = 0;

		/* BCT payload */
		if (op->encrAlg != BCMSAD_ENCR_ALG_NULL)
		{
		    sctx->ecf.flags.explicit_IV = 1;
		}

		if (op->authAlg != BCMSAD_AUTH_ALG_NONE)
		{
#ifdef BCMAPI_USE_MAC_CKSUM
		    //sctx->ecf.flags.insert_icv_mac = 1;
		    sctx->ecf.flags.check_icv_mac = 1;
#endif
		    sctx->ecf.flags.ICV_MAC_size = 3;
		}
                break;
	}
    }
    else if (op->flags.proto == BCMSAD_PROTOCOL_AH)
    {
	switch (op->flags.dir)
	{
	    case BCMSAD_DIR_OUTBOUND:
		if (op->authAlg != BCMSAD_AUTH_ALG_NONE)
		{
#ifdef BCMAPI_USE_MAC_CKSUM
		    sctx->ecf.flags.insert_icv_mac = 1;
#endif
		    sctx->ecf.flags.ICV_MAC_size = 3;
		}
		break;

	    case BCMSAD_DIR_INBOUND:
		if (op->authAlg != BCMSAD_AUTH_ALG_NONE)
		{
#ifdef BCMAPI_USE_MAC_CKSUM
		    sctx->ecf.flags.check_icv_mac = 1;
#endif
		    sctx->ecf.flags.ICV_MAC_size = 3;
		}
		break;
	}
    }

#ifdef BCMAPI_USE_SPS_FRAGMENT
    /* Disabling SPS has no effect */
    if (op->flags.dir == BCMSAD_DIR_INBOUND)
    {
	//cch->field.flag.SPS_PR = 1;  /* Fixme Gigi */ 
	if (op->flags.ipv4)
	    *spsFrag = SPSV4_FRAG_SIZE;

	if (op->flags.ipv6)
	    *spsFrag = SPSV6_FRAG_SIZE;
    }
#endif

    sctx->protocol.ipsec_flags.SCTX_size =
                (sctxSize + *sauSize + *spdSize + *tunnelSize + *encapSize) / 4;
    sctx->cipher.flags.UPDT_ofst = ((sctxSize / 4 - 3) * 4);
    //sctx->cipher.flags.UPDT_ofst = sctx->protocol.ipsec_flags.SCTX_size;
    //dmaCtxSize += sauSize + spdSize + tunnelSize + encapSize;

    SPU_TRACE(("sctxSize %d sauSize %d spdSize %d tunnelSize %d "
            "encapSize %d SCTX_size %d UPDT_ofst %d SCTX %d\n",
            sctxSize, *sauSize, *spdSize, *tunnelSize, *encapSize,
            sctx->protocol.ipsec_flags.SCTX_size,
            sctx->cipher.flags.UPDT_ofst, sizeof (SCTX)));
    return status;
} /* spu_cch_sctx_init */

static int spu_auth_init (u_int8_t * cmd_buf, int *offset, BCM_CRYPTOOP * op)
{
    int status = BCM_STATUS_OK;

    /* Make HMAC contexts */
    if (op->authKeyLen)
    {
	if (op->authKeyLen <= UBSEC_HMAC_LENGTH)
	{
	    ubsec_HMAC_State_t state;
	    ubsec_CipherCommand_t type = 0;
	    uint8_t tmp_authKey[UBSEC_MAC_KEY_LENGTH];

	    memset (&tmp_authKey, 0, sizeof (tmp_authKey));

	    switch (op->authAlg)
	    {
		case BCMSAD_AUTH_ALG_SHA1:
		    type = UBSEC_MAC_SHA1;
		    memcpy (&tmp_authKey[0], &op->authKey[0],
                                             BCMSAD_AUTH_SHA1_KEYLEN);
		    break;

		case BCMSAD_AUTH_ALG_MD5:
		    type = UBSEC_MAC_MD5;
		    memcpy (&tmp_authKey[0], &op->authKey[0],
                                             BCMSAD_AUTH_MD5_KEYLEN);
		    break;
	    }
	    ubsec_InitHMACState (&state, type,
			       (ubsec_HMAC_Key_pt) & tmp_authKey[0]);

#ifdef DEBUG_SA
	    SPU_DATA_DUMP("Hmac state", &state, sizeof (ubsec_HMAC_State_t));
#endif
	    /* Copy Inner Context */
	    memcpy ((uint8_t *) cmd_buf + *offset, state.InnerState,
		  op->authKeyLen);
	    *offset += op->authKeyLen;

	    /* Copy the outer context */
	    memcpy ((uint8_t *) cmd_buf + *offset, state.OuterState, 
                                                                op->authKeyLen);
	    *offset += op->authKeyLen;
	}
	else
	{
	    status = BCM_STATUS_INVALID_INPUT;
	    printk (KERN_ERR "bad authKeyLen %d\n", op->authKeyLen);
	}
    }

    return status;
} /* spu_auth_init */

#if 0
static int spu_sa_init (u_int8_t * cmd_buf, 
                        int *offset, 
                        BCM_CRYPTOOP * op,
                        SCTX_INSAU * insau, 
                        u_int16_t * sauSize)
{
    int status = BCM_STATUS_OK;
    SAU * sau;

    if (*sauSize)
    {
	/* Format the SA update context */
	sau = (SAU *) ((uint8_t *) cmd_buf + *offset);

	/* Save the SAU physical Address for offset */
	//devsa->SAUPhysicalAddress = GetPhysicalAddress_ForOffset(devsa->dmactx,
	//         currentDMAOffset);
	
	memset (sau, 0, sizeof (SAU));

	switch (op->flags.dir)
	{
	    case BCMSAD_DIR_INBOUND:
		insau = &sau->in;
		/*insau->flags.seqnum_en = 1;  only for outbound */
#ifdef BCMAPI_USE_INBOUND_SPD_EN
		insau->flags.spd_en = 1;
#endif
		insau->flags.soft_expired = 1;
#ifdef BCMAPI_USE_INBOUND_AUDIT
		insau->flags.audit_en = 1;
#endif
		insau->flags.UPDT_size = sizeof (SCTX_INSAU) / 4;
		insau->SeqNum[0] = 0;
		insau->SeqNum[1] = 0;
		insau->ByteCount[1] = 0xffffff;
#ifdef DEBUG_SA
		SPU_DATA_DUMP("SAU inbound", sau, sizeof (SCTX_INSAU));
#endif
		break;

	    case BCMSAD_DIR_OUTBOUND:
		SPU_TRACE(("OUTBOUNT Do Nothing\n"));
		break;
	}
	*offset += *sauSize;
    }

    return status;

} /* spu_sa_init */
#endif

static int spu_spd_init (u_int8_t * cmd_buf, 
                         int *offset, 
                         BCM_CRYPTOOP * op,
                         u_int16_t * spdSize, 
                         SACTX * devsa)
{
    int status = BCM_STATUS_OK;
    SPD_V4 * spdv4;
    SPD_V6 * spdv6;

    if (*spdSize)
    {
	/* Format the Inbound SPD */
	if (op->flags.ipv4)
	{
	    spdv4 = (SPD_V4 *) ((uint8_t *) cmd_buf + *offset);
	    memset ((void *) spdv4, 0, *spdSize);
	    spdv4->SrcAddr = htonl (devsa->in.pol_src[0]);
	    spdv4->SrcMask = htonl (devsa->in.srcMask[0]);
	    spdv4->DstAddr = htonl (devsa->in.pol_dst[0]);
	    spdv4->DstMask = htonl (devsa->in.dstMask[0]);
	    spdv4->flags.dadr_en = htonl (SPD_ADDR_MASK);
	    spdv4->flags.sadr_en = htonl (SPD_ADDR_MASK);
	}

	if (op->flags.ipv6)
	{
	    spdv6 = (SPD_V6 *) ((uint8_t *) cmd_buf + *offset);
	    memset ((void *) spdv6, 0, *spdSize);
	    memcpy (&spdv6->SrcAddr[0], &devsa->in.pol_src[0], SIZE_IPV6_ADDR);
	    memcpy (&spdv6->SrcMask[0], &devsa->in.srcMask[0], SIZE_IPV6_ADDR);
	    memcpy (&spdv6->DstAddr[0], &devsa->in.pol_dst[0], SIZE_IPV6_ADDR);
	    memcpy (&spdv6->DstMask[0], &devsa->in.dstMask[0], SIZE_IPV6_ADDR);
	    spdv6->flags.ipv6 = 1;
	    spdv6->flags.dadr_en = SPD_ADDR_MASK;
	    spdv6->flags.sadr_en = SPD_ADDR_MASK;
	}
	*offset += *spdSize;
    }
    return status;
} /* spu_spd_init */

#if 0
static int spu_ipsec_hdr_init (u_int8_t * cmd_buf, 
                               int *offset, 
                               BCM_CRYPTOOP * op,
                               u_int16_t * encapSize, 
                               SCTX_INSAU * insau,
                               SCTX_OUTSAU * outsau, 
                               SACTX * devsa)
{
    int status = BCM_STATUS_OK;

    if (*encapSize && op->flags.proto == BCMSAD_PROTOCOL_ESP)
    {
	ESP_HEADER *esp;
	uint32_t SeqNum = 0;

	esp = (ESP_HEADER *) ((uint8_t *) cmd_buf + *offset);
	memset (esp, 0, sizeof (ESP_HEADER));
	esp->spi = htonl (op->spi);

	if (op->flags.dir == BCMSAD_DIR_INBOUND)
	    SeqNum = insau->SeqNum[1];
	else
	    SeqNum = outsau->SeqNum[1];

	esp->SeqNum = SeqNum;

#ifdef DEBUG_SA
	SPU_DATA_DUMP("ESP header", &esp, *encapSize);
#endif
	*offset += *encapSize;
    }
    else if (*encapSize && op->flags.proto == BCMSAD_PROTOCOL_AH)
    {
	AH_HEADER *ah;
	uint32_t SeqNum = 0;

	ah = (AH_HEADER *) ((uint8_t *) cmd_buf + *offset);
	memset (ah, 0, sizeof (AH_HEADER));

	/* assign in host endian struct */
	ah->NextHeader = 4;
	ah->PayloadLen = 4;

	/* short swap the host endian to network order */
	*(unsigned short *) ah = htons (*(unsigned short *) ah);
	*(unsigned int *) ah = htonl (*(unsigned int *) ah);

	ah->spi = htonl (op->spi);

	if (op->flags.dir == BCMSAD_DIR_INBOUND)
	    SeqNum = insau->SeqNum[1];
	else
	{
	    SeqNum = outsau->SeqNum[1];
	    devsa->icvOFrag = 1;
	}
	ah->SeqNum = SeqNum;
#ifdef DEBUG_SA
	SPU_DATA_DUMP("AH header", &ah, *encapSize);
#endif
	*offset += *encapSize;
    }
    return status;
} /* spu_ipsec_hdr_init */
#endif

static int spu_buff_desc_init (u_int8_t * cmd_buf, 
                               int *cmdOffset, 
                               BDESC * bufDesc,
                               BCM_CRYPTOOP * op, 
                               int offset, 
                               int len,
                               SACTX * devsa, 
                               CCH * cch)
{
    uint32_t bdHeader = 0;
    int status = BCM_STATUS_OK;

    bdHeader = (len << 16);

    memset ((void *) bufDesc, 0, sizeof (BDESC));

    /*
     * Assemble the BDESC. First MAC fields.
     */
    if (op->flags.proto == BCMSAD_PROTOCOL_ESP)
    {
        if (op->authAlg != HASH_ALG_NULL)
        {
            bufDesc->offsetMAC = offset;
            if (op->flags.dir == BCMSAD_DIR_INBOUND)
            {
                bufDesc->lengthMAC = len - (offset + SIZE_IPSEC_ICV);
            }
            else
            {
                bufDesc->lengthMAC = len - offset;
            }
        }
        else
        {
            bufDesc->offsetMAC = 0;
            bufDesc->lengthMAC = 0;
        }
    }
    else
    {
        /*
         * AH protocol
         */
        bufDesc->offsetMAC = 0;
        bufDesc->lengthMAC = len;
    }

    /*
     * Next Crypto fields.
     */
    if (op->encrAlg != CRYPTO_ALG_NULL)
    {
        bufDesc->offsetCrypto = SIZE_ESP_HEADER + devsa->ivLen + offset;
    }
    else
    {
        bufDesc->offsetCrypto = 0;
    }

    if (op->authAlg != HASH_ALG_NULL)
    {
        if (op->flags.dir == BCMSAD_DIR_INBOUND)
        {
            if (op->encrAlg != CRYPTO_ALG_NULL)
            {
                bufDesc->lengthCrypto = len - (offset + 
                                   SIZE_ESP_HEADER + devsa->ivLen + SIZE_IPSEC_ICV);
            }
            else
            {
                bufDesc->lengthCrypto = 0;
            }
        }
        else
        {
            if (op->encrAlg != CRYPTO_ALG_NULL)
            {
                bufDesc->lengthCrypto = len -
                              (offset + SIZE_ESP_HEADER + devsa->ivLen);
            }
            else
            {
                bufDesc->lengthCrypto = 0;
            }
        }
    }
    else
    {
        bufDesc->lengthCrypto = len - (offset + SIZE_ESP_HEADER + devsa->ivLen);
    }

    /*
     * For ESP compute the ICV offset.
     */
    if (op->flags.proto == BCMSAD_PROTOCOL_ESP)
    {
        if (op->flags.dir == BCMSAD_DIR_OUTBOUND)
        {
            if (op->authAlg != HASH_ALG_NULL)
            {
                bufDesc->offsetICV = len;
            }
        }
        else
        {
            /* Inbound */
            if (op->authAlg != HASH_ALG_NULL)
            {
                bufDesc->offsetICV = (len - SIZE_IPSEC_ICV);
            }
            else
            {
                bufDesc->offsetICV = 0;
            }
        }
        bufDesc->offsetIV = offset + SIZE_ESP_HEADER; 
    }
    else
    {
        if (op->flags.dir == BCMSAD_DIR_OUTBOUND)
        {
            /* destination buffer accounts for extra bytes required for alignment */
            bufDesc->offsetICV = (len + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
        }
        else
        {
            bufDesc->offsetICV = offset;
        }
    }

    SPU_TRACE(("====> BDESC <====\n"));
    SPU_TRACE(("offsetMAC %d\n", bufDesc->offsetMAC));
    SPU_TRACE(("lengthMAC %d\n", bufDesc->lengthMAC));
    SPU_TRACE(("offsetCrypto %d\n", bufDesc->offsetCrypto));
    SPU_TRACE(("lengthCrypto %d\n", bufDesc->lengthCrypto));
    SPU_TRACE(("offsetICV %d\n", bufDesc->offsetICV));
    SPU_TRACE(("offsetIV %d\n", bufDesc->offsetIV));

    SPU_DATA_DUMP("**** Buffer Descriptor ****",
         (unsigned char *) bufDesc, (uint16_t) sizeof (BDESC));

    /*
     *  Add the BDESC to the header here.
     */
    if (cch->field.flag.BDESC_PR_HS_PR)
    {
        CTX_COPY ((uint8_t *) (cmd_buf + *cmdOffset),
            (uint8_t *) bufDesc, sizeof (BDESC));
    
        SPU_DATA_DUMP("**** Added Buffer Descriptor to Header ****",
             (unsigned char *) (cmd_buf + *cmdOffset),
             (uint16_t) sizeof (BDESC));
    
        *cmdOffset += sizeof (BDESC);
    }

    if (cch->field.flag.BD_PR)
    {

        CTX_COPY ((uint8_t *) (cmd_buf + *cmdOffset),
            (uint8_t *) & bdHeader, sizeof (bdHeader));
    
        SPU_TRACE(("Copying BD header base dmactx %p ofst %p curdmaofst %d\n",
              cmd_buf, (cmd_buf + *cmdOffset), *cmdOffset));
        SPU_DATA_DUMP("**** BD Header****",
             (unsigned char *) &bdHeader, (uint16_t) sizeof (bdHeader));
    
        *cmdOffset += sizeof (bdHeader);

    }
    return status;
} /* spu_buff_desc_init */

static int spu_avail_desc(struct spu_trans_req *trans_req)
{
    struct spu_pkt_frag *pkt_frags;
    int nchains;

    /* make sure we have enough RX descriptors */
    pkt_frags = trans_req->dfrags_list;
    nchains = 0;
    while(pkt_frags)
    {
        nchains++;
        pkt_frags = pkt_frags->next;
    }

    if ( pdev_ctrl->rx_free_bds < nchains )
    {
        SPU_TRACE("No output descriptors available\n");
        return 0;
    }

    /*
     * Check number of available descriptors. Must have additional
     * 2 descriptors for message header and status
     */
    if (pdev_ctrl->tx_free_bds < (trans_req->sfrags + 2))
    {
        SPU_TRACE("No input descriptors available\n");
        return 0;
    }

    return 1;
}

static int spu_format_output (struct spu_pkt_frag *frag_list, 
                              int nfrags,
                              SACTX * devsa, 
                              BCM_CRYPTOOP * op)
{
    int status = BCM_STATUS_OK;
    struct spu_pkt_frag *pkt_frags = frag_list;
    uint32_t flags = 0;
    int nchains = 0;
    int startbd;

    while(pkt_frags)
    {
        nchains++;
        pkt_frags = pkt_frags->next;
    }

    pkt_frags = frag_list;
    if((nfrags == 1) || (nchains == 1))
    {
        flags = DMA_SOP | DMA_EOP;
        startbd = pdev_ctrl->rx_tail;
        spu_assign_output_desc (pkt_frags->buf, pkt_frags->len, flags);
    }
    else
    {
        flags = DMA_SOP;
        startbd = pdev_ctrl->rx_tail;
        spu_assign_output_desc (pkt_frags->buf, pkt_frags->len, flags);
        pkt_frags = pkt_frags->next;
        while(pkt_frags)
        {
            SPU_TRACE(("spu_format_output: Assigned descriptor for data %p len %d\n",
                                     pkt_frags->buf, pkt_frags->len));
            if(pkt_frags->next) 
            {
                flags = DMA_OWN;
                spu_assign_output_desc (pkt_frags->buf, pkt_frags->len, flags);
            }
            else
            {
                flags = DMA_OWN | DMA_EOP;
                spu_assign_output_desc (pkt_frags->buf, (pkt_frags->len + 4), flags);
            }

            pkt_frags = pkt_frags->next;
        }
    }

    /* pass SOP to HW now */
    pdev_ctrl->rx_bds[startbd].status |= DMA_OWN;

    /*
     * If we have auth data add an extra descriptor to hold
     * the auth data.
     */

    SPU_TRACE(("spu_format_output: outputFrags %d authOFrag %d\n",
                                     devsa->outputFrags, devsa->authOFrag));
    return status;
} /* spu_format_output */

static int spu_format_input (u_int8_t * cmd_buf, 
                             int *offset, 
                             struct spu_pkt_frag *frag_list, 
                             int nfrags,
                             SACTX * devsa)
{
    uint32_t flags = 0;
    int status = BCM_STATUS_OK;
    struct spu_pkt_frag *pkt_frags = frag_list;
    int startbd;

    SPU_TRACE(("spu_format_input: Descriptors available dmactx %p "
              "phys %lx \n", devsa->dmactx, VIRT_TO_PHYS (devsa->dmactx)));

    SPU_DATA_DUMP("cmd buf", cmd_buf, *offset);

    /*
     * Assign the first buffer descriptor for the message header.
     */
    flags = DMA_SOP;
    startbd = pdev_ctrl->tx_tail;
    spu_assign_input_desc (cmd_buf, *offset, flags);

    flags = DMA_OWN;
    while(pkt_frags)
    {
        SPU_DATA_DUMP("**** Tx Data ****", pkt_frags->buf, pkt_frags->len);
        spu_assign_input_desc (pkt_frags->buf, pkt_frags->len, flags);
        pkt_frags = pkt_frags->next;
    }

    /*
     * Setup the descriptor with the status word.
     */
    flags = DMA_EOP | DMA_OWN;
    memset(devsa->dmaStatus, 0, sizeof (uint32_t));
    SPU_TRACE(("spu_format_input: Last frag data %p phys %lx\n",
               devsa->dmaStatus, VIRT_TO_PHYS(devsa->dmaStatus)));
    spu_assign_input_desc (devsa->dmaStatus, sizeof (uint32_t), flags);

    /* pass SOP to HW now */
    pdev_ctrl->tx_bds[startbd].status |= DMA_OWN;

    return status;
} /* spu_format_input */

int spu_process_ipsec(struct spu_trans_req *trans_req, BCM_CRYPTOOP *op)
{
    CCH_SCTX cch_sctx;
#ifdef SPU_DEBUG
    SCTX *sctx = &cch_sctx.sctx;
#endif
    CCH *cch = &cch_sctx.cch;
    uint16_t sctxSize = 0, sauSize = 0, spdSize = 0;
    uint16_t tunnelSize = 0, encapSize = 0;
    uint16_t supdtFrag = 0, spsFrag = 0, bctFrag = 0;
#if 0
    SCTX_OUTSAU *outsau = NULL;
    SCTX_INSAU *insau = NULL;
#endif
    SACTX DEV_SA;
    SACTX *devsa = &DEV_SA;
    int status = 0;
    BDESC bufDesc;
    int spu_cmd_offset = 0;
    u_int8_t *spu_cmd_ptr = NULL;
    unsigned long irq_flags;
    int offset;

    SPU_DATA_DUMP("spu_process_ipsec-sbuf", trans_req->sfrags_list->buf, 
                                             trans_req->sfrags_list->len);
    
    SPU_DATA_DUMP("spu_process_ipsec-dbuf", trans_req->dfrags_list->buf, 
                                             trans_req->dfrags_list->len);

    SPU_DATA_DUMP("ency/decrypt", op->encrKey, op->encrKeyLen);
    SPU_DATA_DUMP("auth", op->authKey, op->authKeyLen);

    memset(devsa, 0, sizeof(SACTX));
    devsa->flags.proto = op->flags.proto;
    devsa->flags.dir = op->flags.dir;

    if(devsa->flags.dir == BCMSAD_DIR_INBOUND)
        pdev_ctrl->stats.decIngress++;
    else
        pdev_ctrl->stats.encIngress++;

    status = spu_SCTXSA_getsize (op, &sctxSize);
    if(status != BCM_STATUS_OK)
    {
        SPU_TRACE(("spu_SCTXSA_getsize error %d\n", status));
        return status;;
    }

    status = spu_cch_sctx_init (op, &cch_sctx, devsa, sctxSize,
                                &supdtFrag, &spsFrag, &tunnelSize,
                                &encapSize, &sauSize, &spdSize);
    if(status != BCM_STATUS_OK)
    {
        SPU_TRACE(("spu_cch_sctx_init error %d\n", status));
        return status;
    }

    cch->ech = (uint32)trans_req;

    /* Estimate the size of the BCT input fragment */
    if (op->flags.dir == BCMSAD_DIR_INBOUND)
    {
        if (op->encrAlg != BCMSAD_ENCR_ALG_NULL)
        {
            bctFrag = 2 * devsa->ivLen;
        }
        else if (op->flags.proto == BCMSAD_PROTOCOL_ESP)
        {
            /* esp but not ah ie esp_null */
            bctFrag = 16;
        }
    }

    SPU_TRACE(("Creating SA sizes: total %d sctxSize %d "
               "sauSize %d spdSize %d tunnelSize %d encapSize %d\n",
               sctx->protocol.ipsec_flags.SCTX_size * 4,
               sctxSize, sauSize, spdSize, tunnelSize, encapSize));

    spu_cmd_ptr = (unsigned char *)&trans_req->cmd_buf[0];
    devsa->dmaStatus = (void *)(spu_cmd_ptr + BCM_XTRA_DMA_HDR_SIZE);

    /* Copy the DMA context for this SA */
    /* Copy  SCTX proto, SCTX cipher, SCTX ecf words */
    spu_cmd_offset = sizeof (CCH_SCTX);

    status = spu_auth_init (spu_cmd_ptr, &spu_cmd_offset, op);
    if(status != BCM_STATUS_OK)
    {
        SPU_TRACE(("spu_auth_init error %d\n", status));
        return status;
    }

    /* Copy the encryption keys */
    if (op->encrKeyLen)
    {
        if (op->encrKeyLen <= BCMSAD_ENCR_KEYLEN_MAX)
        {
            CTX_COPY ((uint8_t *) spu_cmd_ptr + spu_cmd_offset, op->encrKey,
                      op->encrKeyLen);
        }
        else
        {
            status = BCM_STATUS_INVALID_INPUT;
            printk (KERN_ERR "bad encrKeyLen %d\n", op->encrKeyLen);
            return status;
        }
        spu_cmd_offset += op->encrKeyLen;
    }

#if 0
    status = spu_sa_init (spu_cmd_ptr, &spu_cmd_offset, op, insau, &sauSize);
    if(status != BCM_STATUS_OK)
    {
	SPU_TRACE(("spu_sa_init error %d\n", status));
	return status;
    }
#endif

    status = spu_spd_init (spu_cmd_ptr, &spu_cmd_offset, op, &spdSize, devsa);
    if(status != BCM_STATUS_OK)
    {
        SPU_TRACE(("spu_spd_init error %d\n", status));
        return status;
    }

    /* Copy the tunnel header for outbound tunnel mode only */
    if (tunnelSize)
    {
        CTX_COPY ((uint8_t *) spu_cmd_ptr + spu_cmd_offset,
                  devsa->out.tunnelHeader, tunnelSize);
        spu_cmd_offset += tunnelSize;
        SPU_DATA_DUMP("Tunnel header", devsa->out.tunnelHeader, tunnelSize);
    }

    /* Copy CCH words */
    memcpy ((uint8_t *) spu_cmd_ptr, (uint8_t *) &cch_sctx, sizeof (CCH_SCTX));

    /* Add input fragments */
    if (bctFrag)
    {
        devsa->bctIFrag = 1;
        devsa->bctLen = bctFrag;
    }

    devsa->inputFrags = devsa->bctIFrag;

    /* Add output fragments */
    /* Always have a status fragment */
    devsa->statusOFrag = 1;
#ifdef BCMAPI_USE_SPS_FRAGMENT
    /* Add the SPS context for inbound SA only */
    if (spsFrag)
    {
        devsa->spsOFrag = 1;
        devsa->spsFragLen = spsFrag;
    }
#endif
    /* Add the SUPDT fragment context if any */
    if (supdtFrag)
    {
        devsa->updateOFrag = 1;
        devsa->updateFragLen = supdtFrag;
    }

#ifdef BCMAPI_USE_HASH_FRAGMENT
    if (op->authAlg != BCMSAD_AUTH_ALG_NONE)
    {
        if (op->flags.dir == BCMSAD_DIR_INBOUND)
        {
            devsa->authOFrag = 1;
        }
    }
#endif

    devsa->outputFrags = devsa->authOFrag + devsa->statusOFrag
                       + devsa->updateOFrag + devsa->icvOFrag + devsa->spsOFrag;

    /* Sanity check */
    if (spu_cmd_offset != (sctxSize + sizeof(CCH)))
    {
        status = BCM_STATUS_INVALID_INPUT;
        SPU_TRACE(("context size mismatch %d:%d\n", 
                            spu_cmd_offset, (sctxSize + sizeof(CCH))));
        return status;
    }

    SPU_TRACE(("Static Context at %p current size %d currentDmaOfst %d\n",
                         spu_cmd_ptr, cch_sctx.cch.length, spu_cmd_offset));

#if 0
    status = spu_ipsec_hdr_init (spu_cmd_ptr, &spu_cmd_offset, op, 
                                 &encapSize, insau, outsau, devsa);
    if(status != BCM_STATUS_OK)
    {
        SPU_TRACE(("spu_ipsec_hdr_init error %d\n", status));
        return status;
    }
#endif

    if (op->flags.proto == BCMSAD_PROTOCOL_ESP)
    {
        /* for ESP this is the offset to the MAC */
        offset = trans_req->headerLen - devsa->ivLen - SIZE_ESP_HEADER;
    }
    else
    {
        /* for AH this is the offset to the ICV for inbound */
        offset = trans_req->headerLen;
    }
    status = spu_buff_desc_init (spu_cmd_ptr, &spu_cmd_offset, &bufDesc, 
                                 op, offset, trans_req->slen, devsa, cch);
    if(status != BCM_STATUS_OK)
    {
        SPU_TRACE(("spu_buff_desc_init error %d\n", status));
        return status;
    }

    WARN_ON(spu_cmd_offset >= BCM_XTRA_DMA_HDR_SIZE);

    SPU_TRACE(("Message header length %d hdrLen %d\n",
                                cch_sctx.cch.length, spu_cmd_offset));

    spin_lock_irqsave (&pdev_ctrl->spin_lock, irq_flags);

    /* verify there are enough RX and TX descriptors */
    if ( 0 == spu_avail_desc(trans_req) ) {
        spin_unlock_irqrestore (&pdev_ctrl->spin_lock, irq_flags);
        return BCM_STATUS_RESOURCE;
    }

    status = spu_format_output (trans_req->dfrags_list, 
                                trans_req->dfrags,
                                devsa, 
                                op);
    if(status != BCM_STATUS_OK)
    {
        SPU_TRACE(("spu_format_output error %d\n", status));
        spin_unlock_irqrestore (&pdev_ctrl->spin_lock, irq_flags);
        return status;
    }

    status = spu_format_input (spu_cmd_ptr, 
                               &spu_cmd_offset, 
                               trans_req->sfrags_list,
                               trans_req->sfrags,
                               devsa);
    if(status != BCM_STATUS_OK)
    {
        SPU_TRACE(("spu_format_input error %d\n", status));
        spin_unlock_irqrestore (&pdev_ctrl->spin_lock, irq_flags);
        return status;
    }

    trans_req->numtxbds = (trans_req->sfrags + 2);

    spin_unlock_irqrestore (&pdev_ctrl->spin_lock, irq_flags);

    BcmHalInterruptEnable(pdev_ctrl->rx_irq);
    pdev_ctrl->tx_dma->cfg |= DMA_ENABLE;
    pdev_ctrl->rx_dma->cfg |= DMA_ENABLE;

    return status;
}/* spu_process_ipsec_esp */

#ifdef CONFIG_BCM_SPU_TEST
int spu_finish_processing (uint8_t *addr, int test_mode)
#else
int spu_finish_processing (uint8_t *addr)
#endif
{
    unsigned char *ptr = NULL;
    unsigned char *ptr1 = NULL;
    unsigned char *ptr2 = NULL;
    struct spu_trans_req *spu_req;
    unsigned int status;
    int numbds;

    ptr = addr;
    ptr1 = (char *) PHYS_TO_UNCACHED ((uint32) ptr);
    ptr2 = (unsigned char *)*(uint32 *) (ptr1 + 4);

    if(!ptr2)
    {
        printk(KERN_ERR "Invalid pointer at output DMA\n");
        return BCM_STATUS_OUTPUT_ERROR;
    }
    /* test code */
#ifdef CONFIG_BCM_SPU_TEST
    if((test_mode) || (!((uint32)ptr2 & 0x0000FFFF))) 
    {
        return 0;
    }
#endif /* CONFIG_BCM_SPU_TEST */

    spu_req = (struct spu_trans_req *) ptr2;
    spu_req->err = 0;
    if ( spu_req->dStatus )
    {
        memcpy(&status, spu_req->dStatus, RX_STS_SIZE);
        /* bit 17 indicates there was an error with the transaction */
        if (status & 0x20000)
        {
            SPU_TRACE(("SPU error occured in the decryption process - SPU status 0x%08x\n", status));
            spu_req->err = -EBADMSG;
        }
    }

    numbds = spu_req->numtxbds;
    /* spu_req is unavailable after this call */
    spu_req->callback (spu_req);

    return numbds;

}/* spu_finish_processing */


MODULE_LICENSE("GPL");
