#ifdef BCMCCX

/*
 * ccx.c
 * Perform CCX key permutation and MIC functions.
 *
 * Original reference code from:
 *	IEEE 802.11 Wireless LAN Client
 *	Cisco Systems Compliance Specifications
 *	[Cisco Client eXtensions (CCX)]
 *	Version 1.11
 *	Cisco Proprietary and Confidential
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: ccx.c 451682 2014-01-27 20:30:17Z $
 */

#include <typedefs.h>
#include <bcmcrypto/aes.h>

#ifdef BCMDRIVER
#include <osl.h>
#else
#include <string.h>
#endif	/* BCMDRIVER */

/* macro for fetching unaligned big endian unsigned 32-bit integers */
#define GB(p, i, s)		(((uint32) *((uint8 *)(p) + i)) << (s))
#define GETBIG32(p)		GB(p, 0, 24) | GB(p, 1, 16) | GB(p, 2, 8) | GB(p, 3, 0)

#ifdef BCMCCX_TEST
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#endif // endif

#define CKIP_KEY_LENGTH	16

typedef struct {			/* --- MMH context */
	uint8 CK[CKIP_KEY_LENGTH];	/* the key */
	uint8 coefficient[16];		/* current aes counter mode coefficients */
	uint64 accum;			/* accumulated mic, reduced to uint32 in final() */
	int position;			/* current position (byte offset) in message */
	uint8 part[4];			/* for conversion of message to uint32 for mmh */
} mic_context;

/* mic encryption */
static void mic_init(mic_context *context, const uint8 *CK);
static void mic_update(mic_context *context, const uint8 *pOctets, int len);
static void mic_final(mic_context *context, uint8 digest[4]);

static uint32 mic_getcoefficient(mic_context *context);

#ifdef BCMCCX_TEST

typedef struct {
	uint8 S[256];
	uint8 i;
	uint8 j;
} wep_context;

static void crc32_init(void);
static uint32 crc32_get(const uint8 *buf, int size);
static void wep_calculateICV(const uint8 *buf, int n, uint8 *icv);
static void wep_keyinit(wep_context *ctx, const uint8 *key, int keylen);
static void wep_cipher(wep_context *ctx, uint8 *buf, int buflen);
static int ckip_decrypt(const uint8 *CK, const uint8 *pkt, uint8 *ecr, int ecrlen);
#endif // endif

void CKIP_key_permute(uint8  *PK, const uint8 *CK, uint8  toDsFromDs, const uint8 *piv);
int wsec_ckip_mic_compute(const uint8 *CK, const uint8 *pDA, const uint8 *pSA,
                          const uint8 *pSEQ, const uint8 *payload, int payloadlen,
                          const uint8 *p2, int len2, uint8 pMIC[]);
int wsec_ckip_mic_check(const uint8 *CK, const uint8 *pDA, const uint8 *pSA,
                        const uint8 *payload, int payloadlen, uint8 mic[]);

#ifdef BCMCCX_TEST

/* SUPPORT ROUTINES */

void
dumpbytes(char *header, const void *pb, int len)
{
	const uint8 *pc;
	int i;

	fprintf(stdout, "%s", header);
	pc = pb;
	for (i = 0; i < len; i++) {
		fprintf(stdout, "%02X ", *pc++);
	}
	fprintf(stdout, "\n");
}

/* WEP ICV */

#define CRC_TABSZ	256		/* no of entries in table */
uint32 ct_crctab[CRC_TABSZ];
#define CRC_32		0xedb88320L	/* CRC-32 polynomial */

static union {
	uint32 lg;
} crc;

static void
crc32_init(void)
{
	uint32 i, j, item, accum;

	for (i = 0;  i < CRC_TABSZ;  i++) {
		for (accum = 0, item = i << 1, j = 8;  j > 0;  j--) {
			item >>= 1;
			if ((item ^ accum) & 0x0001)
				accum = (accum >> 1) ^ CRC_32;
			else
				accum >>= 1;
		}
		ct_crctab[i] = accum;
	}
	crc.lg = 0xffffffff;
}

static uint32
crc32_get(const uint8 *buf, int size)
{
	int i;

	for (i = 0;  i < size;  i++) {
		crc.lg = ct_crctab[(crc.lg ^ buf[i]) & 0xff] ^ ((crc.lg >> 8) & 0xFFFFFFL);
	}
	crc.lg = ~crc.lg;
	return crc.lg;
}

static void
wep_calculateICV(const uint8 *buf, int n, uint8 *icv)
{
	uint32 crc32;

	crc32_init();
	crc32 = crc32_get(buf, n);

	icv[0] = (crc32 >>  0) & 0xFF;
	icv[1] = (crc32 >>  8) & 0xFF;
	icv[2] = (crc32 >> 16) & 0xFF;
	icv[3] = (crc32 >> 24) & 0xFF;
}

/* WEP CIPHER */

/* basic 802.11 wep key initialization routine and en/decipher */

#define swap_byte(x, y) { uint8 t; t = *(x); *(x) = *(y); *(y) = t; }

static void
wep_keyinit(wep_context *ctx, const uint8 *key, int keylen)
{
	uint8 i, j, k;
	uint8* S;

	ctx->i = 0;
	ctx->j = 0;

	S = &ctx->S[0];

	i = 0;
	do {
		S[i] = i;
	} while (++i != 0);

	i = j = k = 0;
	do {
		j += S[i] + key[k];
		swap_byte(&S[i], &S[j]);
		if (++k >= keylen) k = 0;
	} while (++i != 0);
}

static void
wep_cipher(wep_context *ctx, uint8 *buf, int buflen)
{
	uint8 i, j;
	uint8 *S;

	i = ctx->i;
	j = ctx->j;
	S = &ctx->S[0];
	while (buflen--) {
		i++;
		j += S[i];
		swap_byte(&S[i], &S[j]);
		*buf ^= S[ (uint8)(S[i]+S[j]) ];
		buf++;
	}
	ctx->i = i;
	ctx->j = j;
}

#endif /* BCMCCX_TEST */

/* CKIP KEY PERMUTATION */

/*
 * Note that the Sbox[] table below is a subset of rijndael-alg-fst.c's Te0[256] array.
 * To save 512 bytes of memory, define SHARE_RIJNDAEL_SBOX to re-use that one here.
 */
#ifdef SHARE_RIJNDAEL_SBOX

extern const uint32 rijndaelTe0[256];

#define SBOX(ind)	(((rijndaelTe0[ind] >> 16) & 0xff00) | \
			 ((rijndaelTe0[ind] >> 0) & 0x00ff))

#else /* !SHARE_RIJNDAEL_SBOX */

/* 2-byte by 2-byte subset of the full AES table */
static const uint16 Sbox[256] =
{
	0xC6A5, 0xF884, 0xEE99, 0xF68D, 0xFF0D, 0xD6BD, 0xDEB1, 0x9154,
	0x6050, 0x0203, 0xCEA9, 0x567D, 0xE719, 0xB562, 0x4DE6, 0xEC9A,
	0x8F45, 0x1F9D, 0x8940, 0xFA87, 0xEF15, 0xB2EB, 0x8EC9, 0xFB0B,
	0x41EC, 0xB367, 0x5FFD, 0x45EA, 0x23BF, 0x53F7, 0xE496, 0x9B5B,
	0x75C2, 0xE11C, 0x3DAE, 0x4C6A, 0x6C5A, 0x7E41, 0xF502, 0x834F,
	0x685C, 0x51F4, 0xD134, 0xF908, 0xE293, 0xAB73, 0x6253, 0x2A3F,
	0x080C, 0x9552, 0x4665, 0x9D5E, 0x3028, 0x37A1, 0x0A0F, 0x2FB5,
	0x0E09, 0x2436, 0x1B9B, 0xDF3D, 0xCD26, 0x4E69, 0x7FCD, 0xEA9F,
	0x121B, 0x1D9E, 0x5874, 0x342E, 0x362D, 0xDCB2, 0xB4EE, 0x5BFB,
	0xA4F6, 0x764D, 0xB761, 0x7DCE, 0x527B, 0xDD3E, 0x5E71, 0x1397,
	0xA6F5, 0xB968, 0x0000, 0xC12C, 0x4060, 0xE31F, 0x79C8, 0xB6ED,
	0xD4BE, 0x8D46, 0x67D9, 0x724B, 0x94DE, 0x98D4, 0xB0E8, 0x854A,
	0xBB6B, 0xC52A, 0x4FE5, 0xED16, 0x86C5, 0x9AD7, 0x6655, 0x1194,
	0x8ACF, 0xE910, 0x0406, 0xFE81, 0xA0F0, 0x7844, 0x25BA, 0x4BE3,
	0xA2F3, 0x5DFE, 0x80C0, 0x058A, 0x3FAD, 0x21BC, 0x7048, 0xF104,
	0x63DF, 0x77C1, 0xAF75, 0x4263, 0x2030, 0xE51A, 0xFD0E, 0xBF6D,
	0x814C, 0x1814, 0x2635, 0xC32F, 0xBEE1, 0x35A2, 0x88CC, 0x2E39,
	0x9357, 0x55F2, 0xFC82, 0x7A47, 0xC8AC, 0xBAE7, 0x322B, 0xE695,
	0xC0A0, 0x1998, 0x9ED1, 0xA37F, 0x4466, 0x547E, 0x3BAB, 0x0B83,
	0x8CCA, 0xC729, 0x6BD3, 0x283C, 0xA779, 0xBCE2, 0x161D, 0xAD76,
	0xDB3B, 0x6456, 0x744E, 0x141E, 0x92DB, 0x0C0A, 0x486C, 0xB8E4,
	0x9F5D, 0xBD6E, 0x43EF, 0xC4A6, 0x39A8, 0x31A4, 0xD337, 0xF28B,
	0xD532, 0x8B43, 0x6E59, 0xDAB7, 0x018C, 0xB164, 0x9CD2, 0x49E0,
	0xD8B4, 0xACFA, 0xF307, 0xCF25, 0xCAAF, 0xF48E, 0x47E9, 0x1018,
	0x6FD5, 0xF088, 0x4A6F, 0x5C72, 0x3824, 0x57F1, 0x73C7, 0x9751,
	0xCB23, 0xA17C, 0xE89C, 0x3E21, 0x96DD, 0x61DC, 0x0D86, 0x0F85,
	0xE090, 0x7C42, 0x71C4, 0xCCAA, 0x90D8, 0x0605, 0xF701, 0x1C12,
	0xC2A3, 0x6A5F, 0xAEF9, 0x69D0, 0x1791, 0x9958, 0x3A27, 0x27B9,
	0xD938, 0xEB13, 0x2BB3, 0x2233, 0xD2BB, 0xA970, 0x0789, 0x33A7,
	0x2DB6, 0x3C22, 0x1592, 0xC920, 0x8749, 0xAAFF, 0x5078, 0xA57A,
	0x038F, 0x59F8, 0x0980, 0x1A17, 0x65DA, 0xD731, 0x84C6, 0xD0B8,
	0x82C3, 0x29B0, 0x5A77, 0x1E11, 0x7BCB, 0xA8FC, 0x6DD6, 0x2C3A
};

#define SBOX(ind)	Sbox[ind]

#endif /* !SHARE_RIJNDAEL_SBOX */

#define Lo8(v16)	((v16) & 0xFF)
#define Hi8(v16)	(((v16) >> 8) & 0xFF)
#define u16Swap(i)	((((i) >> 8) & 0xFF) | (((i) << 8) & 0xFF00))
#define _S_(i)		(SBOX(Lo8(i)) ^ u16Swap(SBOX(Hi8(i))))

#define rotLeft_1(x)	((((x) << 1) | ((x) >> 15)) & 0xFFFF)

void
CKIP_key_permute(uint8  *PK,		/* output permuted key */
		 const uint8 *CK,	/* input CKIP key */
		 uint8  toDsFromDs,	/* input toDs/FromDs bits */
		 const uint8 *piv)	/* input pointer to IV */
{
	int i;
	uint16 H[2], tmp;          /* H=32-bits of per-packet hash value */
	uint16 L[8], R[8];         /* L=uint16 array of CK, R=uint16 array of PK */

	/* build L from input key */
	memset(L, 0, sizeof(L));
	for (i = 0; i < 16; i++) {
		L[i >> 1] |= (((uint16)(CK[i])) << (i & 1 ? 8 : 0));
	}

	H[0] = (((uint16)piv[0]) << 8) + piv[1];
	H[1] = (((uint16)toDsFromDs) << 8) | piv[2];

	for (i = 0; i < 8; i++) {
		H[0] ^= L[i];		/* 16-bits of key material */
		tmp = _S_(H[0]);	/* 16x16 permutation */
		H[0] = tmp ^ H[1];	/* set up for next round */
		H[1] = tmp;
		R[i] = H[0];		/* store into key array  */
	}

	/* sweep in the other direction */
	tmp = L[0];
	for (i = 7; i > 0; i--) {
		R[i] = tmp = rotLeft_1(tmp) + R[i];
	}

	/* IV of the permuted key is unchanged */
	PK[0] = piv[0];
	PK[1] = piv[1];
	PK[2] = piv[2];

	/* key portion of the permuted key is changed */
	for (i = 3; i < 16; i++) {
		PK[i] = (uint8) (R[i>>1] >> (i & 1 ? 8 : 0));
	}
}

#ifdef BCMCCX_TEST

/* CKIP DECRYPTION */

uint8 ckip_pk[16];	/* permuted key */
wep_context wepctx;	/* context for wep decipher */
uint8 calcicv[4];	/* icv calculated for payload */

int
ckip_decrypt(const uint8 *CK, const uint8 *pkt, uint8 *ecr, int ecrlen)
{
	/* CKIP Key Permutation */
	CKIP_key_permute(ckip_pk, CK, pkt[1] & 3, ecr);
	dumpbytes("==== PK (permuted key) ====\n", ckip_pk, sizeof(ckip_pk));

	/* WEP decrypt */
	wep_keyinit(&wepctx, ckip_pk, sizeof(ckip_pk));
	wep_cipher(&wepctx, ecr + 4, ecrlen-4);

	/* verify the ICV */
	wep_calculateICV(ecr + 4, ecrlen - 8, &calcicv[0]);
	if (memcmp(calcicv, ecr + (ecrlen - 4), 4) != 0) {
		return 0;
	}
	return 1;
}
#endif /* BCMCCX_TEST */

/* CKIP MIC MMH FUNCTION */

/* CKIP MIC MMH COEFFICIENTS */

/* AES returns 16-bytes of coefficient which are used in 4-byte (uint32) units
 * aes_encrypt() is used to generate the MMH coefficients in counter mode
 * aes_encrypt(K,IN,OUT) -- applies aes to IN using key K, generating OUT
 * K: input key, IN: input data to encrypt, OUT: output encrypted data
 * all of the parameters K, IN, OUT are 16-byte in length
 */
#include <bcmcrypto/aes.h>

static uint32
mic_getcoefficient(mic_context *context)
{
	uint8 aes_counter[16];
	int coeff_position;
	uint8 *p;

	coeff_position = (context->position - 1) >> 2;
	if ((coeff_position & 3) == 0) {
		/* fetching the first coefficient -- get new 16-byte aes counter output */
		uint32 counter = (coeff_position >> 2);

		/* new counter value */
		memset(&aes_counter[0], 0, sizeof(aes_counter));
		aes_counter[15] = (uint8)(counter >> 0);
		aes_counter[14] = (uint8)(counter >> 8);
		aes_counter[13] = (uint8)(counter >> 16);
		aes_counter[12] = (uint8)(counter >> 24);

		aes_encrypt(CKIP_KEY_LENGTH, &context->CK[0], &aes_counter[0],
		            context->coefficient);
	}
	p = &(context->coefficient[ (coeff_position & 3) << 2 ]);
	return GETBIG32(p);
}

/* MIC MMH ROUTINES */

/* mic accumulate */
#define MIC_ACCUM(v)	context->accum += (uint64)v * mic_getcoefficient(context)

/* prepare for calculation of a new mic */
static void
mic_init(mic_context *context, const uint8 *CK)
{
	/* prepare for new mic calculation */
	memcpy(context->CK, CK, sizeof(context->CK));
	context->accum = 0;
	context->position = 0;
}

/* add some bytes to the mic calculation */
static void
mic_update(mic_context *context, const uint8 *pOctets, int len)
{
	int byte_position;
	uint32 val;

	byte_position = (context->position & 3);
	while (len > 0) {
		/* build a 32-bit word for MIC multiply accumulate */
		do {
			if (len == 0) return;
			context->part[byte_position++] = *pOctets++;
			context->position++;
			len--;
		} while (byte_position < 4);
		/* have a full 32-bit word to process */
		val = GETBIG32(&context->part[0]);
		MIC_ACCUM(val);
		byte_position = 0;
	}
}

/* calculate the mic */
static void
mic_final(mic_context *context, uint8 digest[4])
{
	int byte_position;
	uint32 val;
	uint64 sum, utmp;
	int64 stmp;

	/* deal with partial 32-bit word left over from last update */
	if ((byte_position = (context->position & 3)) != 0) {
		/* have a partial word in part to deal with -- zero unused bytes */
		do {
			context->part[byte_position++] = 0;
			context->position++;
		} while (byte_position < 4);
		val = GETBIG32(&context->part[0]);
		MIC_ACCUM(val);
	}

	/* reduce the accumulated uint64 to a 32-bit MIC */
	sum = context->accum;
	stmp = (sum  & 0xffffffff) - ((sum >> 32)  * 15);
	utmp = (stmp & 0xffffffff) - ((stmp >> 32) * 15);
	sum = utmp & 0xffffffff;
	if (utmp > (((uint64)0x1 << 32) | 0x0000000f))
		sum -= 15;

	val = (uint32)sum;
	digest[0] = (val>>24) & 0xFF;
	digest[1] = (val>>16) & 0xFF;
	digest[2] = (val>>8) & 0xFF;
	digest[3] = (val>>0) & 0xFF;
}

/* CKIP MIC PACKET CALCULATION */

static const uint8 mic_snap[] = { 0xAA, 0xAA, 0x03, 0x00, 0x40, 0x96, 0x00, 0x02 };

/* CK is 16 bytes CKIP key
 * payload is data only(may include ethertype), no mic_snap
 * payloadlen(and len2) are just data len, it will be adjusted to ckip length inside
 * pSEQ is 4 bytes sequence number
 *
 * return mic[]: computed 4 bytes CKIP mic value.
 */
int
wsec_ckip_mic_compute(const uint8 *CK, const uint8 *pDA, const uint8 *pSA,
                      const uint8 *pSEQ, const uint8 *payload, int payloadlen,
                      const uint8 *p2, int len2, uint8 pMIC[])
{
	mic_context ctx;
	uint8 calcmic[4];
	uint8 bigethlen[2];
	uint32 ckip_mic_len = payloadlen + len2 + sizeof(mic_snap) + 4 + 4;

	bigethlen[0] = (uint8)(ckip_mic_len >> 8);
	bigethlen[1] = (uint8)ckip_mic_len;

	mic_init(&ctx, CK);                             /* initialize for calculating the mic */
	mic_update(&ctx, pDA, 6);                       /* Mic <-- Destination address (DA) */
	mic_update(&ctx, pSA, 6);                       /* Mic <-- Source address (SA) */
	mic_update(&ctx, bigethlen, 2);                 /* Mic <-- Length field (big endian) */
	mic_update(&ctx, mic_snap, sizeof(mic_snap));   /* MIC <-- snap header */
	mic_update(&ctx, pSEQ, 4);                      /* Mic <-- SEQ field */
	/* note, MIC field is not included in the calculation */
	mic_update(&ctx, payload, payloadlen); 		/* Mic <-- payload1 */
	if ((p2 != NULL) && (len2 != 0)) {
		mic_update(&ctx, p2, len2); 		/* Mic <-- payload2 */
	}
	mic_final(&ctx, calcmic);                       /* Calculate the MIC */

	memcpy(pMIC, calcmic, 4);
	return 1;
}

static const uint8 iapp_snap[] = {0xAA, 0xAA, 0x03, 0x00, 0x40, 0x96, 0x00, 0x00};

/* CK is 16 bytes CKIP key
 * payload should start with mic_llc_snap
 * -----------------------------------------------------------------------------
 *         LLC   |      SNAP      |     MIC      |    SEQ    |  EthernetLoad
 *      AA:AA:03   00:40:96:00:02   space holder   bigendian	inc. ethertype
 * -----------------------------------------------------------------------------
 * payloadlen include LLC, SNAP, MIC, SEQ and Ethernet payload even though MIC
 * does not exist yet. This number will be converted to BIG Endian format internally.
 *
 * return 1:     MIC matches,
 *        0:     MIC mismatch or any error
 *       -1:     IAPP frame not MMH encapsulated
 */
int
wsec_ckip_mic_check(const uint8 *CK, const uint8 *pDA, const uint8 *pSA,
                    const uint8 *payload, int payloadlen, uint8 mic[])
{
	mic_context ctx;
	const uint8 *pMIC, *pSEQ, *pOrigPayload;
	int OrigPayloadLen;
	uint8 calcmic[4];
	uint8 bigethlen[2];

	if (memcmp(payload, mic_snap, sizeof(mic_snap)) != 0) {
		/* printf("Packet does not have MIC SNAP header\n"); */

		/* Aironet IAPP frames don't have to be MMH encapsulated... */
		return (memcmp(payload, iapp_snap, sizeof(iapp_snap)) != 0) ? 0 : -1;
	}

	pMIC = payload + sizeof(mic_snap);
	pSEQ = pMIC + 4;

	pOrigPayload = pSEQ + 4;
	OrigPayloadLen = payloadlen - sizeof(mic_snap) - 4 - 4;

	bigethlen[0] = (uint8)(payloadlen >> 8);
	bigethlen[1] = (uint8)payloadlen;

	mic_init(&ctx, CK);                             /* initialize for calculating the mic */
	mic_update(&ctx, pDA, 6);                       /* Mic <-- Destination address (DA) */
	mic_update(&ctx, pSA, 6);                       /* Mic <-- Source address (SA) */
	mic_update(&ctx, bigethlen, 2);                 /* Mic <-- Length field (big endian) */
	mic_update(&ctx, payload, sizeof(mic_snap));    /* MIC <-- snap header */
	mic_update(&ctx, pSEQ, 4);                      /* Mic <-- SEQ field */
	/* note, MIC field is not included in the calculation */

	/* after defragmentation, buffer is continuous */
	mic_update(&ctx, pOrigPayload, OrigPayloadLen); /* Mic <-- original payload */
	mic_final(&ctx, calcmic);                       /* Calculate the MIC */

	if (memcmp(pMIC, calcmic, 4) != 0) {
		/* MIC failed */
		memcpy(mic, calcmic, 4);                    /* save it for debugging */
		return 0;
	}

	return 1;
}

#ifdef BCMCCX_TEST
int
ckip_micverify(const uint8 *CK, const uint8 *pDA, const uint8 *pSA,
               const uint8 *payload, int payloadlen)
{
	mic_context ctx;
	const uint8 *pMIC, *pSEQ, *pOrigPayload;
	int OrigPayloadLen;
	uint8 calcmic[4];
	uint8 bigethlen[2];

	if (memcmp(payload, mic_snap, sizeof(mic_snap)) != 0) {
		printf("Packet does not have MIC SNAP header\n");
		return 0;
	}

	pMIC = payload + sizeof(mic_snap);
	pSEQ = pMIC + 4;

	pOrigPayload = pSEQ + 4;
	OrigPayloadLen = payloadlen - sizeof(mic_snap) - 4 - 4;

	bigethlen[0] = (uint8)(payloadlen >> 8);
	bigethlen[1] = (uint8)payloadlen;

	mic_init(&ctx, CK);                             /* initialize for calculating the mic */
	mic_update(&ctx, pDA, 6);                       /* Mic <-- Destination address (DA) */
	mic_update(&ctx, pSA, 6);                       /* Mic <-- Source address (SA) */
	mic_update(&ctx, bigethlen, 2);                 /* Mic <-- Length field (big endian) */
	mic_update(&ctx, payload, sizeof(mic_snap));    /* MIC <-- snap header */
	mic_update(&ctx, pSEQ, 4);                      /* Mic <-- SEQ field */
	/* note, MIC field is not included in the calculation */
	mic_update(&ctx, pOrigPayload, OrigPayloadLen); /* Mic <-- original payload */
	mic_final(&ctx, calcmic);                       /* Calculate the MIC */

	if (memcmp(pMIC, calcmic, 4) != 0) {
		/* MIC failed */
		return 0;
	}
	return 1;
}

/* TEST PACKET */

/* input is an 802.11 packet which is CKIP decrypted and MIC verified */

uint8 ckip_ck[16];	/* basic key extended to 16 bytes */

int hdrlen;		/* length of 802.11 header (24, or 30) */
uint8 *pDA;		/* pointer to DA -- destination address */
uint8 *pSA;		/* pointer to SA -- source address */

uint8 *ecr;		/* pointer to encrypted payload IV:payload:ICV */
int ecrlen;		/* length of encrypted payload */

uint8 *dcr;		/* pointer to decrypted payload */
int dcrlen;		/* length of decrypted payload */

uint8 *msdu;		/* pointer to msdu payload */
int msdulen;		/* length of msdu payload */

void
test_packet(uint8 *pkt, int pktlen, const uint8 *key, int keylen)
{
	dumpbytes("==== 802.11 packet (encrypted) ====\n", pkt, pktlen);

	/* extend ckip key (CK) to 16 bytes */
	memcpy(ckip_ck, key, keylen);
	if (keylen < 16) {
		/* memcpy(&ckip_ck[keylen], ckip_ck, 16-keylen); */

		/* BROADCOM FIX: the above original cisco code using the memcpy doesn't
		 * work with overlapping memory on Windows. Use memmove instead
		 */
		if (keylen == 5) {
			memmove(&ckip_ck[5],  ckip_ck, 5);
			memmove(&ckip_ck[10], ckip_ck, 5);
			memmove(&ckip_ck[15], ckip_ck, 1);
		} else if (keylen == 13) {
			memmove(&ckip_ck[13], ckip_ck, 3);
		}
	}

	dumpbytes("==== CK (basic CKIP key) ====\n", ckip_ck, sizeof(ckip_ck));

	/* determine 802.11 header length */
	/* determine 802.11 DA and SA (destination and source addresses) */
	hdrlen = 24;
	switch (pkt[1] & 3) {
	case 0: /* FromDs=0, ToDs=0 */
		pDA = &pkt[4];
		pSA = &pkt[10];
		break;
	case 1: /* FromDs=0, ToDs=1 */
		pDA = &pkt[16];
		pSA = &pkt[10];
		break;
	case 2: /* FromDs=1, ToDs=0 */
		pDA = &pkt[4];
		pSA = &pkt[16];
		break;
	case 3: /* FromDs=1, ToDs=1 */
		hdrlen = 30;
		pDA = &pkt[16];
		pSA = &pkt[24];
		break;
	}

	/* pointer and length of encrypted payload including IV and ICV */
	ecr = pkt + hdrlen;
	ecrlen = pktlen - hdrlen;

	/* CKIP decryption */
	if (!ckip_decrypt(ckip_ck, pkt, ecr, ecrlen)) {
		printf("Decryption failed\n");
		return;
	}

	dumpbytes("==== 802.11 packet decrypted ====\n", pkt, pktlen);

	/* pointer and length of decrypted payload */
	dcr = ecr + 4;
	dcrlen = ecrlen - 8;

	/* CKIP MIC validation */
	if (!ckip_micverify(ckip_ck, pDA, pSA, dcr, dcrlen)) {
		printf("MIC verification failed\n");
		return;
	}

	/* original MSDU/ethernet payload */
	msdu = dcr + sizeof(mic_snap) + 4 + 4;
	msdulen = dcrlen - sizeof(mic_snap) - 4 - 4;

	printf("==== Original MSDU ====\n");
	dumpbytes("DA: ", pDA, 6);
	dumpbytes("SA: ", pSA, 6);
	dumpbytes("Payload: ", msdu, msdulen);

}

/* TEST SAMPLE PACKETS */

#include "examppkt.h"
#define NUM_SAMPLE  (sizeof(aSamplePacket)/sizeof(aSamplePacket[0]))

#include "examccx.h"
#define NUM_SAMPLE_2  (sizeof(bSamplePacket)/sizeof(bSamplePacket[0]))

int main(void)
{
	tdsSamplePacket *p;

	int i;
	for (i = 0; i < NUM_SAMPLE; i++) {
		p = &aSamplePacket[i];
		test_packet(p->ppkt, p->pktlen, p->pkey, p->keylen);
		fprintf(stdout,
		        "================================================================\n");
	}

	return (0);
}
#endif /* BCMCCX_TEST */

#endif /* BCMCCX */
