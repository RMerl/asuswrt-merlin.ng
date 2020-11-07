/*
<:copyright-BRCM:2015:GPL/GPL:spu

   Copyright (c) 2015 Broadcom 
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

#ifndef _SPU_H
#define _SPU_H

#include <linux/types.h>
#include <linux/scatterlist.h>

enum spu_cipher_alg {
	CIPHER_ALG_NONE = 0x0,
	CIPHER_ALG_RC4 = 0x1,
	CIPHER_ALG_DES = 0x2,
	CIPHER_ALG_3DES = 0x3,
	CIPHER_ALG_AES = 0x4,
};

enum spu_cipher_mode {
	CIPHER_MODE_NONE = 0x0,
	CIPHER_MODE_ECB = 0x0,
	CIPHER_MODE_CBC = 0x1,
	CIPHER_MODE_OFB = 0x2,
	CIPHER_MODE_CFB = 0x3,
	CIPHER_MODE_CTR = 0x4,
	CIPHER_MODE_CCM = 0x5,
	CIPHER_MODE_GCM = 0x6,
	CIPHER_MODE_XTS = 0x7,
};

enum spu_cipher_type {
	CIPHER_TYPE_NONE = 0x0,
	CIPHER_TYPE_DES = 0x0,
	CIPHER_TYPE_3DES = 0x0,
	CIPHER_TYPE_INIT = 0x0,	     /* used for ARC4 */
	CIPHER_TYPE_AES128 = 0x0,
	CIPHER_TYPE_AES192 = 0x1,
	CIPHER_TYPE_UPDT = 0x1,	     /* used for ARC4 */
	CIPHER_TYPE_AES256 = 0x2,
};

enum hash_alg {
	HASH_ALG_NONE = 0x0,
	HASH_ALG_MD5 = 0x1,
	HASH_ALG_SHA1 = 0x2,
	HASH_ALG_SHA224 = 0x3,
	HASH_ALG_SHA256 = 0x4,
	HASH_ALG_AES = 0x5,
	HASH_ALG_SHA384 = 0x6,
	HASH_ALG_SHA512 = 0x7,
};

enum hash_mode {
	HASH_MODE_NONE = 0x0,
	HASH_MODE_HASH = 0x0,
	HASH_MODE_XCBC = 0x0,
	HASH_MODE_CMAC = 0x1,
	HASH_MODE_CTXT = 0x1,
	HASH_MODE_HMAC = 0x2,
	HASH_MODE_FHMAC = 0x6,
	HASH_MODE_CCM = 0x5,
	HASH_MODE_GCM = 0x6,
};

enum hash_type {
	HASH_TYPE_NONE = 0x0,
	HASH_TYPE_FULL = 0x0,
	HASH_TYPE_INIT = 0x1,
	HASH_TYPE_UPDT = 0x2,
	HASH_TYPE_FIN = 0x3,
	HASH_TYPE_AES128 = 0x0,
	HASH_TYPE_AES192 = 0x1,
	HASH_TYPE_AES256 = 0x2
};

#define SPU_CRYPTO_OPERATION_GENERIC	0x1
#define SPU_SCTX_TYPE_GENERIC	        0x0

#define MD5_STATE_SIZE	                16
#define SHA1_STATE_SIZE	                20

/* SPU error codes */
#define SPU_STATUS_MASK                 0x0000FF00
#define SPU_STATUS_SUCCESS              0x00000000
#define SPU_STATUS_INVALID_ICV          0x00000100
#define SPU_STATUS_INVALID_KEY          0x00000200
#define SPU_STATUS_INVALID_KEY_HANDLE   0x00000300

#define SPU_STATUS_ERROR_FLAG           0x00020000

/* Buffer Descriptor Header [BDESC] */
struct BDESC_HEADER {
	uint16_t offsetMAC;	/* word 0 [31-16] */
	uint16_t lengthMAC;	/* word 0 [15-0]  */
	uint16_t offsetCrypto;	/* word 1 [31-16] */
	uint16_t lengthCrypto;	/* word 1 [15-0]  */
	uint16_t offsetICV;	/* word 2 [31-16] */
	uint16_t offsetIV;	/* word 2 [15-0]  */
};

#define BDESC_HDR_OFFSET_CRYPTO_SHIFT   16
#define BDESC_HDR_OFFSET_MAC_SHIFT      16
#define BDESC_HDR_OFFSET_ICV_SHIFT      16

/* Buffer Data Header [BD] */
struct BD_HEADER {
	uint16_t size;
	uint16_t PrevLength;
};

#define BD_HDR_SIZE_SHIFT               16

struct SCTX_sctx_info {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	uint32_t SCTX_size:8;    /* [7:0] */
	uint32_t reserved:22;    /* [29:8] */
	uint32_t type:2;         /* [31:30] */
#else
	uint32_t type:2;         /* [31:30] */
	uint32_t reserved:22;    /* [29:8] */
	uint32_t SCTX_size:8;    /* [7:0] */
#endif
};

#define  SCTX_TYPE_SHIFT                30

/* Command Context Header */
struct mh_fields {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	uint32_t reserved:16; /* [15:0] */
	uint32_t opCode:8;    /* [23:16] */
	uint32_t SUPDT_PR:1;  /* [24] */
	uint32_t reserved1:1; /* [25] */
	uint32_t HASH_PR:1;   /* [26] */
	uint32_t BD_PR:1;     /* [27] */
	uint32_t MFM_PR:1;    /* [28] */
	uint32_t BDESC_PR:1;  /* [29] */
	uint32_t reserved2:1; /* [30] */
	uint32_t SCTX_PR:1;   /* [31] */
#else
	uint32_t SCTX_PR:1;   /* [31] */
	uint32_t reserved2:1; /* [30] */
	uint32_t BDESC_PR:1;  /* [29] */
	uint32_t MFM_PR:1;    /* [28] */
	uint32_t BD_PR:1;     /* [27] */
	uint32_t HASH_PR:1;   /* [26] */
	uint32_t reserved1:1; /* [25] */
	uint32_t SUPDT_PR:1;  /* [24] */
	uint32_t opCode:8;    /* [23:16] */
	uint32_t reserved:16; /* [15:0] */
#endif
};

struct MHEADER {
	union {
		uint32_t bits;
		struct mh_fields flags;
	} headers;
};

#define  MH_OPCODE_SHIFT        16
#define  MH_SPUDT_PRES_SHIFT    24
#define  MH_HASH_PRES_SHIFT     26
#define  MH_BD_PRES_SHIFT       27
#define  MH_MFM_PRES_SHIFT      28
#define  MH_BDESC_PRES_SHIFT    29
#define  MH_SCTX_PRES_SHIFT     31

struct SCTX_cipher_flags {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	uint32_t UPDT_ofst:8;  /* [7:0] */
	uint32_t hashType:2;   /* [9:8] */
	uint32_t hashMode:3;   /* [12:10] */
	uint32_t hashAlg:3;    /* [15:13] */
	uint32_t cryptoType:2; /* [17:16] */
	uint32_t cryptoMode:3; /* [20:18] */
	uint32_t cryptoAlg:3;  /* [23:21] */
	uint32_t reserved2:3;  /* [26:24] */
	uint32_t icv_is_512:1; /* [27] */
	uint32_t reserved:2;   /* [29:28] */
	uint32_t order:1;      /* [30] */
	uint32_t inbound:1;    /* [31] */
#else
	uint32_t inbound:1;    /* [31] */
	uint32_t order:1;      /* [30] */
	uint32_t reserved:2;   /* [29:28] */
	uint32_t icv_is_512:1; /* [27] */
	uint32_t reserved2:3;  /* [26:24] */
	uint32_t cryptoAlg:3;  /* [23:21] */
	uint32_t cryptoMode:3; /* [20:18] */
	uint32_t cryptoType:2; /* [17:16] */
	uint32_t hashAlg:3;    /* [15:13] */
	uint32_t hashMode:3;   /* [12:10] */
	uint32_t hashType:2;   /* [9:8] */
	uint32_t UPDT_ofst:8;  /* [7:0] */
#endif
};

/* Word 1: Cipher bit shifts */

#define  HASH_TYPE_SHIFT                 8
#define  HASH_MODE_SHIFT                10
#define  HASH_ALG_SHIFT                 13
#define  CIPHER_TYPE_SHIFT              16
#define  CIPHER_MODE_SHIFT              18
#define  CIPHER_ALG_SHIFT               21
#define  ICV_IS_512_SHIFT               27
#define  CIPHER_INBOUND_SHIFT           31
#define  CIPHER_ORDER_SHIFT             30

struct SCTX_extended_cipher_flags {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	uint32_t explicit_IV_size:3; /* [2:0] */
	uint32_t reserved4:2;        /* [4:3] */
	uint32_t gen_IV:1;           /* [5] */
	uint32_t explicit_IV:1;      /* [6] */
	uint32_t SCTX_IV:1;          /* [7] */
	uint32_t ICV_size:4;         /* [11:8] */
	uint32_t check_icv:1;        /* [12] */
	uint32_t insert_icv:1;       /* [13] */
	uint32_t reserved3:5;        /* [18:14] */
	uint32_t bd_suppress:1;      /* [19] */
	uint32_t kh:9;               /* [28:20] */
	uint32_t reserved2:2;        /* [30:29] */
	uint32_t pk:1;               /* [31] */
#else
	uint32_t pk:1;               /* [31] */
	uint32_t reserved2:2;        /* [30:29] */
	uint32_t kh:9;               /* [28:20] */
	uint32_t bd_suppress:1;      /* [19] */
	uint32_t reserved3:5;        /* [18:14] */
	uint32_t insert_icv:1;       /* [13] */
	uint32_t check_icv:1;        /* [12] */
	uint32_t ICV_size:4;         /* [11:8] */
	uint32_t SCTX_IV:1;          /* [7] */
	uint32_t explicit_IV:1;      /* [6] */
	uint32_t gen_IV:1;           /* [5] */
	uint32_t reserved:2;         /* [4:3] */
	uint32_t explicit_IV_size:3; /* [2:0] */
#endif
};

/* Word 2: extended cipher bit shifts */
#define  IV_OFFSET_SHIFT                 3
#define  GEN_IV_SHIFT                    5
#define  EXPLICIT_IV_SHIFT               6
#define  SCTX_IV_SHIFT                   7
#define  ICV_SIZE_SHIFT                  8
#define  CHECK_ICV_SHIFT                12
#define  INSERT_ICV_SHIFT               13
#define  CLE_TAIL_ICV_SHIFT             14
#define  BD_SUPPRESS                    19

#define  CTR_IV_SIZE                    16

/* Generic Mode Security Context Structure [SCTX] */
struct SCTX {
/* word 0: protocol flags */
	union {
		uint32_t bits;
		struct SCTX_sctx_info flags;
	} protocol;
/* word 1: cipher flags */
	union {
		uint32_t bits;
		struct SCTX_cipher_flags flags;
	} cipher;
/* word 2: Extended cipher flags */
	union {
		uint32_t bits;
		struct SCTX_extended_cipher_flags flags;
	} ecf;
};

/* Status Header */
struct STAT_HEADER {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	uint32_t pktTotal:8;	/* [7:0]     for users discretion */
	uint32_t errDetail:8;	/* [15:8]  */
	uint32_t reserved:1;	/* [16]    */
	uint32_t error:1;	/* [17]    */
	uint32_t pktIndex:8;	/* [25:18]   for users discretion */
	uint32_t errCategory:6;	/* [31:26] */
#else
	uint32_t errCategory:6;	/* [31:26] */
	uint32_t pktIndex:8;	/* [25:18]   for users discretion */
	uint32_t error:1;	/* [17]    */
	uint32_t reserved:1;	/* [16]    */
	uint32_t errDetail:8;	/* [15:8]  */
	uint32_t pktTotal:8;	/* [7:0]     for users discretion */
#endif
};

struct SPUHEADER {
	struct MHEADER mh;
	uint32_t emh;
	struct SCTX sa;
};

/************** SPU sizes ***************/

/* On Northstar2, the max length of a SPU message is 64k - 1. The
 * max pkt size must be a multiple of block size, so that when
 * we break a request into chunks, the chunks are a multiple
 * of block size. So we set the max pkt size to 64k and remember
 * to always check that message sizes are strictly less than the
 * max pkt size.
 */
#define MAX_SPU_PKT_SIZE  65536
#define MIN_SPU_PKT_SIZE      0

/* Request message. MH + EMH + SCTX[3] + BDESC[3] + BD header */
#define SPU_REQ_FIXED_LEN 36

/* Max length of a SPU message header. Used to allocate a buffer where
 * the SPU message header is constructed. Sum of the following:
 *    MH - 4 bytes
 *    EMH - 4
 *    SCTX - 12 +
 *      max auth key len - 64
 *      max cipher key len - 264 (RC4)
 *      max IV len - 16
 *    BDESC - 12
 *    BD header - 4
 * Total:  371
 */
#define SPU_HEADER_ALLOC_LEN  (SPU_REQ_FIXED_LEN + MAX_AUTH_KEY_SIZE + \
				MAX_KEY_SIZE + MAX_IV_SIZE)

/* Response message header length. Normally MH, EMH, BD header, but when
 * BD_SUPPRESS is used for hash requests, there is no BD header.
 */
#define SPU_RESP_HDR_LEN 12
#define SPU_HASH_RESP_HDR_LEN 8

/* Max length of padding for 4-byte alignment of STATUS field */
#define SPU_STAT_PAD_MAX  4

/* Length of STATUS field */
#define SPU_STATUS_LEN  4

/* GCM and CCM requires 16-byte alignment */
#define SPU_GCM_CCM_ALIGN 16

/* Max length of pad fragment. 4 is for 4-byte alignment of STATUS field */
#define SPU_PAD_LEN_MAX (SPU_GCM_CCM_ALIGN + ALIGN(MAX_BLOCK_SIZE, 4) + ALIGN(SPU_STAT_PAD_MAX, 4))

/* Length up SUPDT field in SPU response message for RC4 */
#define SPU_SUPDT_LEN 260

/* SPU status error codes. These used as common error codes across all
 * SPU variants.
 */
#define SPU_INVALID_ICV  1

/* Size of XTS tweak ("i" parameter), in bytes */
#define SPU_XTS_TWEAK_SIZE 16

/* CCM B_0 field definitions, common for SPU-M and SPU2 */
#define CCM_B0_ADATA		0x40
#define CCM_B0_ADATA_SHIFT	   6
#define CCM_B0_M_PRIME		0x38
#define CCM_B0_M_PRIME_SHIFT	   3
#define CCM_B0_L_PRIME		0x07
#define CCM_B0_L_PRIME_SHIFT	   0

static __always_inline uint32_t spu_real_db_size(uint32_t assoc_size,
					uint32_t aead_iv_buf_len,
					uint32_t prebuf_len,
					uint32_t data_size,
					uint32_t aad_pad_len,
					uint32_t data_pad_len,
					uint32_t hash_pad_len)
{
	return assoc_size + aead_iv_buf_len + prebuf_len + data_size +
		aad_pad_len + data_pad_len + hash_pad_len;
}

/**
 * spu_wordalign_padlen() - Given the length of a data field, determine the
 * padding required to align the data following this field on a 4-byte boundary.
 * @data_size: length of data field in bytes
 *
 * Return: length of status field padding, in bytes
 */
static __always_inline u32 spu_wordalign_padlen(u32 data_size)
{
	return ((data_size + 3) & ~3) - data_size;
}

/* Determine the length of GCM padding required. */
static __always_inline uint32_t spu_gcm_ccm_padlen(enum spu_cipher_mode cipher_mode,
                                                   unsigned data_size)
{
	u32 pad_len = 0;
	u32 m1 = SPU_GCM_CCM_ALIGN - 1;

	if ((cipher_mode == CIPHER_MODE_GCM) ||
	    (cipher_mode == CIPHER_MODE_CCM)) {
		pad_len = ((data_size + m1) & ~m1) - data_size;
	}

	return pad_len;
}

/************** SPU Functions Prototypes **************/

void dump_spu_msg_hdr(uint8_t *buf, unsigned buf_len);

u32 spu_payload_length(u8 *spu_hdr);
u32 spu_assoc_resp_len(enum spu_cipher_mode cipher_mode, bool dtls_hmac,
			unsigned int assoc_len, unsigned int iv_len);
bool spu_req_incl_icv(enum spu_cipher_mode cipher_mode, bool is_encrypt);

u32 spu_aead_req_create(u8 *spu_hdr,
		       u32 isInbound, u32 authFirst,
		       enum spu_cipher_alg cipher_alg,
		       enum spu_cipher_mode cipher_mode,
		       enum spu_cipher_type cipher_type,
		       u8 *cipher_key_buf,
		       u32 cipher_key_len,
		       u8 *cipher_iv_buf,
		       unsigned cipher_iv_len, enum hash_alg auth_alg,
		       enum hash_mode auth_mode, enum hash_type auth_type,
		       u8 digestsize, u8 *auth_key_buf,
		       u32 auth_key_len,
		       u32 assoc_size,
		       u32 data_size,
		       u32 dtls_aead,
		       u32 hmac_offset,
		       u32 aead_iv_buf_len,
		       u32 aad_pad_len,
		       u32 data_pad_len,
		       bool is_rfc4543);

u32 spu_hash_req_create(u8 *spu_hdr,
                        enum hash_alg auth_alg,
                        enum hash_mode auth_mode, 
                        enum hash_type auth_type,
                        u8 digestsize,
                        u8 *auth_key_buf,
                        unsigned auth_key_len,
                        u32 auth_len,
                        u32 prev_length_blocks,
                        u32 data_len);

u16 spu_cipher_req_init(u8 *spu_hdr,
			enum spu_cipher_alg cipher_alg,
			enum spu_cipher_mode cipher_mode,
			enum spu_cipher_type cipher_type,
			u8 *cipher_key_buf,
			unsigned cipher_key_len,
			u32 cipher_iv_len);

void spu_cipher_req_finish(u8 *spu_hdr,
			   u16 spu_req_hdr_len,
			   unsigned isInbound,
			   enum spu_cipher_alg cipher_alg,
			   enum spu_cipher_type cipher_type,
			   u8 *cipher_key_buf,
			   unsigned cipher_key_len,
			   bool update_key,
			   u8 *cipher_iv_buf,
			   unsigned cipher_iv_len,
			   unsigned data_size);

void spu_request_pad(u8 *pad_start,
		     u32 gcm_padding,
		     u32 hash_pad_len,
		     enum hash_alg auth_alg,
		     unsigned total_sent,
		     u32 status_padding);

static __always_inline int spu_status_process(u8 *statp)
{
	u32 status;
	status = __be32_to_cpu(*(__be32 *) statp);
	if (status & SPU_STATUS_ERROR_FLAG) {
		if (status & SPU_STATUS_INVALID_ICV)
			return SPU_INVALID_ICV;
		return -EBADMSG;
	}
	return 0;
}

void spu_ccm_update_iv(unsigned int digestsize,
		       int iv_len,
		       char *iv_buf,
		       unsigned int assoclen,
		       unsigned int chunksize,
		       bool is_encrypt,
		       bool is_esp);

#endif /* _SPU_H */
