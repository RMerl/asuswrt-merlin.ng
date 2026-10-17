/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Header file for Advanced Crypto Engine - SFR definitions
 *
 * Copyright (c) 2012  Samsung Electronics
 */

#ifndef __ACE_SHA_H
#define __ACE_SHA_H

struct exynos_ace_sfr {
	unsigned int	fc_intstat;	/* base + 0 */
	unsigned int	fc_intenset;
	unsigned int	fc_intenclr;
	unsigned int	fc_intpend;
	unsigned int	fc_fifostat;
	unsigned int	fc_fifoctrl;
	unsigned int	fc_global;
	unsigned int	res1;
	unsigned int	fc_brdmas;
	unsigned int	fc_brdmal;
	unsigned int	fc_brdmac;
	unsigned int	res2;
	unsigned int	fc_btdmas;
	unsigned int	fc_btdmal;
	unsigned int	fc_btdmac;
	unsigned int	res3;
	unsigned int	fc_hrdmas;
	unsigned int	fc_hrdmal;
	unsigned int	fc_hrdmac;
	unsigned int	res4;
	unsigned int	fc_pkdmas;
	unsigned int	fc_pkdmal;
	unsigned int	fc_pkdmac;
	unsigned int	fc_pkdmao;
	unsigned char   res5[0x1a0];

	unsigned int	aes_control;	/* base + 0x200 */
	unsigned int	aes_status;
	unsigned char	res6[0x8];
	unsigned int	aes_in[4];
	unsigned int	aes_out[4];
	unsigned int	aes_iv[4];
	unsigned int	aes_cnt[4];
	unsigned char   res7[0x30];
	unsigned int	aes_key[8];
	unsigned char   res8[0x60];

	unsigned int	tdes_control;	/* base + 0x300 */
	unsigned int	tdes_status;
	unsigned char   res9[0x8];
	unsigned int	tdes_key[6];
	unsigned int	tdes_iv[2];
	unsigned int	tdes_in[2];
	unsigned int	tdes_out[2];
	unsigned char   res10[0xc0];

	unsigned int	hash_control;	/* base + 0x400 */
	unsigned int	hash_control2;
	unsigned int	hash_fifo_mode;
	unsigned int	hash_byteswap;
	unsigned int	hash_status;
	unsigned char   res11[0xc];
	unsigned int	hash_msgsize_low;
	unsigned int	hash_msgsize_high;
	unsigned int	hash_prelen_low;
	unsigned int	hash_prelen_high;
	unsigned int	hash_in[16];
	unsigned int	hash_key_in[16];
	unsigned int	hash_iv[8];
	unsigned char   res12[0x30];
	unsigned int	hash_result[8];
	unsigned char   res13[0x20];
	unsigned int	hash_seed[5];
	unsigned char	res14[12];
	unsigned int	hash_prng[5];
	unsigned char	res15[0x18c];

	unsigned int	pka_sfr[5];		/* base + 0x700 */
};

/* ACE_FC_INT */
#define ACE_FC_PKDMA			(1 << 0)
#define ACE_FC_HRDMA			(1 << 1)
#define ACE_FC_BTDMA			(1 << 2)
#define ACE_FC_BRDMA			(1 << 3)
#define ACE_FC_PRNG_ERROR		(1 << 4)
#define ACE_FC_MSG_DONE		(1 << 5)
#define ACE_FC_PRNG_DONE		(1 << 6)
#define ACE_FC_PARTIAL_DONE		(1 << 7)

/* ACE_FC_FIFOSTAT */
#define ACE_FC_PKFIFO_EMPTY		(1 << 0)
#define ACE_FC_PKFIFO_FULL		(1 << 1)
#define ACE_FC_HRFIFO_EMPTY		(1 << 2)
#define ACE_FC_HRFIFO_FULL		(1 << 3)
#define ACE_FC_BTFIFO_EMPTY		(1 << 4)
#define ACE_FC_BTFIFO_FULL		(1 << 5)
#define ACE_FC_BRFIFO_EMPTY		(1 << 6)
#define ACE_FC_BRFIFO_FULL		(1 << 7)

/* ACE_FC_FIFOCTRL */
#define ACE_FC_SELHASH_MASK		(3 << 0)
#define ACE_FC_SELHASH_EXOUT		(0 << 0)	/* independent source */
#define ACE_FC_SELHASH_BCIN		(1 << 0)	/* blk cipher input */
#define ACE_FC_SELHASH_BCOUT		(2 << 0)	/* blk cipher output */
#define ACE_FC_SELBC_MASK		(1 << 2)
#define ACE_FC_SELBC_AES		(0 << 2)
#define ACE_FC_SELBC_DES		(1 << 2)

/* ACE_FC_GLOBAL */
#define ACE_FC_SSS_RESET		(1 << 0)
#define ACE_FC_DMA_RESET		(1 << 1)
#define ACE_FC_AES_RESET		(1 << 2)
#define ACE_FC_DES_RESET		(1 << 3)
#define ACE_FC_HASH_RESET		(1 << 4)
#define ACE_FC_AXI_ENDIAN_MASK		(3 << 6)
#define ACE_FC_AXI_ENDIAN_LE		(0 << 6)
#define ACE_FC_AXI_ENDIAN_BIBE		(1 << 6)
#define ACE_FC_AXI_ENDIAN_WIBE		(2 << 6)

/* Feed control - BRDMA control */
#define ACE_FC_BRDMACFLUSH_OFF		(0 << 0)
#define ACE_FC_BRDMACFLUSH_ON		(1 << 0)
#define ACE_FC_BRDMACSWAP_ON		(1 << 1)
#define ACE_FC_BRDMACARPROT_MASK	(0x7 << 2)
#define ACE_FC_BRDMACARPROT_OFS	2
#define ACE_FC_BRDMACARCACHE_MASK	(0xf << 5)
#define ACE_FC_BRDMACARCACHE_OFS	5

/* Feed control - BTDMA control */
#define ACE_FC_BTDMACFLUSH_OFF		(0 << 0)
#define ACE_FC_BTDMACFLUSH_ON		(1 << 0)
#define ACE_FC_BTDMACSWAP_ON		(1 << 1)
#define ACE_FC_BTDMACAWPROT_MASK	(0x7 << 2)
#define ACE_FC_BTDMACAWPROT_OFS	2
#define ACE_FC_BTDMACAWCACHE_MASK	(0xf << 5)
#define ACE_FC_BTDMACAWCACHE_OFS	5

/* Feed control - HRDMA control */
#define ACE_FC_HRDMACFLUSH_OFF		(0 << 0)
#define ACE_FC_HRDMACFLUSH_ON		(1 << 0)
#define ACE_FC_HRDMACSWAP_ON		(1 << 1)
#define ACE_FC_HRDMACARPROT_MASK	(0x7 << 2)
#define ACE_FC_HRDMACARPROT_OFS	2
#define ACE_FC_HRDMACARCACHE_MASK	(0xf << 5)
#define ACE_FC_HRDMACARCACHE_OFS	5

/* Feed control - PKDMA control */
#define ACE_FC_PKDMACBYTESWAP_ON	(1 << 3)
#define ACE_FC_PKDMACDESEND_ON		(1 << 2)
#define ACE_FC_PKDMACTRANSMIT_ON	(1 << 1)
#define ACE_FC_PKDMACFLUSH_ON		(1 << 0)

/* Feed control - PKDMA offset */
#define ACE_FC_SRAMOFFSET_MASK		0xfff

/* AES control */
#define ACE_AES_MODE_MASK		(1 << 0)
#define ACE_AES_MODE_ENC		(0 << 0)
#define ACE_AES_MODE_DEC		(1 << 0)
#define ACE_AES_OPERMODE_MASK		(3 << 1)
#define ACE_AES_OPERMODE_ECB		(0 << 1)
#define ACE_AES_OPERMODE_CBC		(1 << 1)
#define ACE_AES_OPERMODE_CTR		(2 << 1)
#define ACE_AES_FIFO_MASK		(1 << 3)
#define ACE_AES_FIFO_OFF		(0 << 3)	/* CPU mode */
#define ACE_AES_FIFO_ON		(1 << 3)	/* FIFO mode */
#define ACE_AES_KEYSIZE_MASK		(3 << 4)
#define ACE_AES_KEYSIZE_128		(0 << 4)
#define ACE_AES_KEYSIZE_192		(1 << 4)
#define ACE_AES_KEYSIZE_256		(2 << 4)
#define ACE_AES_KEYCNGMODE_MASK	(1 << 6)
#define ACE_AES_KEYCNGMODE_OFF		(0 << 6)
#define ACE_AES_KEYCNGMODE_ON		(1 << 6)
#define ACE_AES_SWAP_MASK		(0x1f << 7)
#define ACE_AES_SWAPKEY_OFF		(0 << 7)
#define ACE_AES_SWAPKEY_ON		(1 << 7)
#define ACE_AES_SWAPCNT_OFF		(0 << 8)
#define ACE_AES_SWAPCNT_ON		(1 << 8)
#define ACE_AES_SWAPIV_OFF		(0 << 9)
#define ACE_AES_SWAPIV_ON		(1 << 9)
#define ACE_AES_SWAPDO_OFF		(0 << 10)
#define ACE_AES_SWAPDO_ON		(1 << 10)
#define ACE_AES_SWAPDI_OFF		(0 << 11)
#define ACE_AES_SWAPDI_ON		(1 << 11)
#define ACE_AES_COUNTERSIZE_MASK	(3 << 12)
#define ACE_AES_COUNTERSIZE_128	(0 << 12)
#define ACE_AES_COUNTERSIZE_64		(1 << 12)
#define ACE_AES_COUNTERSIZE_32		(2 << 12)
#define ACE_AES_COUNTERSIZE_16		(3 << 12)

/* AES status */
#define ACE_AES_OUTRDY_MASK		(1 << 0)
#define ACE_AES_OUTRDY_OFF		(0 << 0)
#define ACE_AES_OUTRDY_ON		(1 << 0)
#define ACE_AES_INRDY_MASK		(1 << 1)
#define ACE_AES_INRDY_OFF		(0 << 1)
#define ACE_AES_INRDY_ON		(1 << 1)
#define ACE_AES_BUSY_MASK		(1 << 2)
#define ACE_AES_BUSY_OFF		(0 << 2)
#define ACE_AES_BUSY_ON		(1 << 2)

/* TDES control */
#define ACE_TDES_MODE_MASK		(1 << 0)
#define ACE_TDES_MODE_ENC		(0 << 0)
#define ACE_TDES_MODE_DEC		(1 << 0)
#define ACE_TDES_OPERMODE_MASK		(1 << 1)
#define ACE_TDES_OPERMODE_ECB		(0 << 1)
#define ACE_TDES_OPERMODE_CBC		(1 << 1)
#define ACE_TDES_SEL_MASK		(3 << 3)
#define ACE_TDES_SEL_DES		(0 << 3)
#define ACE_TDES_SEL_TDESEDE		(1 << 3)	/* TDES EDE mode */
#define ACE_TDES_SEL_TDESEEE		(3 << 3)	/* TDES EEE mode */
#define ACE_TDES_FIFO_MASK		(1 << 5)
#define ACE_TDES_FIFO_OFF		(0 << 5)	/* CPU mode */
#define ACE_TDES_FIFO_ON		(1 << 5)	/* FIFO mode */
#define ACE_TDES_SWAP_MASK		(0xf << 6)
#define ACE_TDES_SWAPKEY_OFF		(0 << 6)
#define ACE_TDES_SWAPKEY_ON		(1 << 6)
#define ACE_TDES_SWAPIV_OFF		(0 << 7)
#define ACE_TDES_SWAPIV_ON		(1 << 7)
#define ACE_TDES_SWAPDO_OFF		(0 << 8)
#define ACE_TDES_SWAPDO_ON		(1 << 8)
#define ACE_TDES_SWAPDI_OFF		(0 << 9)
#define ACE_TDES_SWAPDI_ON		(1 << 9)

/* TDES status */
#define ACE_TDES_OUTRDY_MASK		(1 << 0)
#define ACE_TDES_OUTRDY_OFF		(0 << 0)
#define ACE_TDES_OUTRDY_ON		(1 << 0)
#define ACE_TDES_INRDY_MASK		(1 << 1)
#define ACE_TDES_INRDY_OFF		(0 << 1)
#define ACE_TDES_INRDY_ON		(1 << 1)
#define ACE_TDES_BUSY_MASK		(1 << 2)
#define ACE_TDES_BUSY_OFF		(0 << 2)
#define ACE_TDES_BUSY_ON		(1 << 2)

/* Hash control */
#define ACE_HASH_ENGSEL_MASK		(0xf << 0)
#define ACE_HASH_ENGSEL_SHA1HASH	(0x0 << 0)
#define ACE_HASH_ENGSEL_SHA1HMAC	(0x1 << 0)
#define ACE_HASH_ENGSEL_SHA1HMACIN	(0x1 << 0)
#define ACE_HASH_ENGSEL_SHA1HMACOUT	(0x9 << 0)
#define ACE_HASH_ENGSEL_MD5HASH	(0x2 << 0)
#define ACE_HASH_ENGSEL_MD5HMAC	(0x3 << 0)
#define ACE_HASH_ENGSEL_MD5HMACIN	(0x3 << 0)
#define ACE_HASH_ENGSEL_MD5HMACOUT	(0xb << 0)
#define ACE_HASH_ENGSEL_SHA256HASH	(0x4 << 0)
#define ACE_HASH_ENGSEL_SHA256HMAC	(0x5 << 0)
#define ACE_HASH_ENGSEL_PRNG		(0x8 << 0)
#define ACE_HASH_STARTBIT_ON		(1 << 4)
#define ACE_HASH_USERIV_EN		(1 << 5)
#define ACE_HASH_PAUSE_ON		(1 << 0)

/* Hash control - FIFO mode */
#define ACE_HASH_FIFO_MASK		(1 << 0)
#define ACE_HASH_FIFO_OFF		(0 << 0)
#define ACE_HASH_FIFO_ON		(1 << 0)

/* Hash control - byte swap */
#define ACE_HASH_SWAP_MASK		(0xf << 0)
#define ACE_HASH_SWAPKEY_OFF		(0 << 0)
#define	ACE_HASH_SWAPKEY_ON	(1 << 0)
#define ACE_HASH_SWAPIV_OFF		(0 << 1)
#define	ACE_HASH_SWAPIV_ON	(1 << 1)
#define ACE_HASH_SWAPDO_OFF		(0 << 2)
#define ACE_HASH_SWAPDO_ON		(1 << 2)
#define ACE_HASH_SWAPDI_OFF		(0 << 3)
#define ACE_HASH_SWAPDI_ON		(1 << 3)

/* Hash status */
#define ACE_HASH_BUFRDY_MASK		(1 << 0)
#define ACE_HASH_BUFRDY_OFF		(0 << 0)
#define ACE_HASH_BUFRDY_ON		(1 << 0)
#define ACE_HASH_SEEDSETTING_MASK	(1 << 1)
#define ACE_HASH_SEEDSETTING_OFF	(0 << 1)
#define ACE_HASH_SEEDSETTING_ON	(1 << 1)
#define ACE_HASH_PRNGBUSY_MASK		(1 << 2)
#define ACE_HASH_PRNGBUSY_OFF		(0 << 2)
#define ACE_HASH_PRNGBUSY_ON		(1 << 2)
#define ACE_HASH_PARTIALDONE_MASK	(1 << 4)
#define ACE_HASH_PARTIALDONE_OFF	(0 << 4)
#define ACE_HASH_PARTIALDONE_ON	(1 << 4)
#define ACE_HASH_PRNGDONE_MASK		(1 << 5)
#define ACE_HASH_PRNGDONE_OFF		(0 << 5)
#define ACE_HASH_PRNGDONE_ON		(1 << 5)
#define ACE_HASH_MSGDONE_MASK		(1 << 6)
#define ACE_HASH_MSGDONE_OFF		(0 << 6)
#define ACE_HASH_MSGDONE_ON		(1 << 6)
#define ACE_HASH_PRNGERROR_MASK	(1 << 7)
#define ACE_HASH_PRNGERROR_OFF		(0 << 7)
#define ACE_HASH_PRNGERROR_ON		(1 << 7)
#define ACE_HASH_PRNG_REG_NUM		5

#define ACE_SHA_TYPE_SHA1		1
#define ACE_SHA_TYPE_SHA256		2

/**
 * Computes hash value of input pbuf using ACE
 *
 * @param in_addr	A pointer to the input buffer
 * @param bufleni	Byte length of input buffer
 * @param out_addr	A pointer to the output buffer. When complete
 *			32 bytes are copied to pout[0]...pout[31]. Thus, a user
 *			should allocate at least 32 bytes at pOut in advance.
 * @param hash_type SHA1 or SHA256
 *
 * Return:		0 on Success, -1 on Failure (Timeout)
 */
int ace_sha_hash_digest(const uchar * in_addr, uint buflen,
			uchar * out_addr, uint hash_type);
#endif
