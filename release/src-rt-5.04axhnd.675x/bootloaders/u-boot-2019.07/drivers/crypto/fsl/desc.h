/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * CAAM descriptor composition header
 * Definitions to support CAAM descriptor instruction generation
 *
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 *
 * Based on desc.h file in linux drivers/crypto/caam
 */

#ifndef DESC_H
#define DESC_H

#define KEY_BLOB_SIZE		32
#define MAC_SIZE			16

/* Max size of any CAAM descriptor in 32-bit words, inclusive of header */
#define MAX_CAAM_DESCSIZE	64

/* Size of DEK Blob  descriptor, inclusive of header */
#define DEK_BLOB_DESCSIZE	9

/* Block size of any entity covered/uncovered with a KEK/TKEK */
#define KEK_BLOCKSIZE		16

/*
 * Supported descriptor command types as they show up
 * inside a descriptor command word.
 */
#define CMD_SHIFT		27
#define CMD_MASK		0xf8000000

#define CMD_KEY			(0x00 << CMD_SHIFT)
#define CMD_SEQ_KEY		(0x01 << CMD_SHIFT)
#define CMD_LOAD		(0x02 << CMD_SHIFT)
#define CMD_SEQ_LOAD		(0x03 << CMD_SHIFT)
#define CMD_FIFO_LOAD		(0x04 << CMD_SHIFT)
#define CMD_SEQ_FIFO_LOAD	(0x05 << CMD_SHIFT)
#define CMD_STORE		(0x0a << CMD_SHIFT)
#define CMD_SEQ_STORE		(0x0b << CMD_SHIFT)
#define CMD_FIFO_STORE		(0x0c << CMD_SHIFT)
#define CMD_SEQ_FIFO_STORE	(0x0d << CMD_SHIFT)
#define CMD_MOVE_LEN		(0x0e << CMD_SHIFT)
#define CMD_MOVE		(0x0f << CMD_SHIFT)
#define CMD_OPERATION		(0x10 << CMD_SHIFT)
#define CMD_SIGNATURE		(0x12 << CMD_SHIFT)
#define CMD_JUMP		(0x14 << CMD_SHIFT)
#define CMD_MATH		(0x15 << CMD_SHIFT)
#define CMD_DESC_HDR		(0x16 << CMD_SHIFT)
#define CMD_SHARED_DESC_HDR	(0x17 << CMD_SHIFT)
#define CMD_SEQ_IN_PTR		(0x1e << CMD_SHIFT)
#define CMD_SEQ_OUT_PTR		(0x1f << CMD_SHIFT)

/* General-purpose class selector for all commands */
#define CLASS_SHIFT		25
#define CLASS_MASK		(0x03 << CLASS_SHIFT)

#define CLASS_NONE		(0x00 << CLASS_SHIFT)
#define CLASS_1			(0x01 << CLASS_SHIFT)
#define CLASS_2			(0x02 << CLASS_SHIFT)
#define CLASS_BOTH		(0x03 << CLASS_SHIFT)

/*
 * Descriptor header command constructs
 * Covers shared, job, and trusted descriptor headers
 */

/*
 * Do Not Run - marks a descriptor inexecutable if there was
 * a preceding error somewhere
 */
#define HDR_DNR			0x01000000

/*
 * ONE - should always be set. Combination of ONE (always
 * set) and ZRO (always clear) forms an endianness sanity check
 */
#define HDR_ONE			0x00800000
#define HDR_ZRO			0x00008000

/* Start Index or SharedDesc Length */
#define HDR_START_IDX_MASK	0x3f
#define HDR_START_IDX_SHIFT	16

/* If shared descriptor header, 6-bit length */
#define HDR_DESCLEN_SHR_MASK	0x3f

/* If non-shared header, 7-bit length */
#define HDR_DESCLEN_MASK	0x7f

/* This is a TrustedDesc (if not SharedDesc) */
#define HDR_TRUSTED		0x00004000

/* Make into TrustedDesc (if not SharedDesc) */
#define HDR_MAKE_TRUSTED	0x00002000

/* Save context if self-shared (if SharedDesc) */
#define HDR_SAVECTX		0x00001000

/* Next item points to SharedDesc */
#define HDR_SHARED		0x00001000

/*
 * Reverse Execution Order - execute JobDesc first, then
 * execute SharedDesc (normally SharedDesc goes first).
 */
#define HDR_REVERSE		0x00000800

/* Propagate DNR property to SharedDesc */
#define HDR_PROP_DNR		0x00000800

/* JobDesc/SharedDesc share property */
#define HDR_SD_SHARE_MASK	0x03
#define HDR_SD_SHARE_SHIFT	8
#define HDR_JD_SHARE_MASK	0x07
#define HDR_JD_SHARE_SHIFT	8

#define HDR_SHARE_NEVER		(0x00 << HDR_SD_SHARE_SHIFT)
#define HDR_SHARE_WAIT		(0x01 << HDR_SD_SHARE_SHIFT)
#define HDR_SHARE_SERIAL	(0x02 << HDR_SD_SHARE_SHIFT)
#define HDR_SHARE_ALWAYS	(0x03 << HDR_SD_SHARE_SHIFT)
#define HDR_SHARE_DEFER		(0x04 << HDR_SD_SHARE_SHIFT)

/* JobDesc/SharedDesc descriptor length */
#define HDR_JD_LENGTH_MASK	0x7f
#define HDR_SD_LENGTH_MASK	0x3f

/*
 * KEY/SEQ_KEY Command Constructs
 */

/* Key Destination Class: 01 = Class 1, 02 - Class 2 */
#define KEY_DEST_CLASS_SHIFT	25	/* use CLASS_1 or CLASS_2 */
#define KEY_DEST_CLASS_MASK	(0x03 << KEY_DEST_CLASS_SHIFT)

/* Scatter-Gather Table/Variable Length Field */
#define KEY_SGF			0x01000000
#define KEY_VLF			0x01000000

/* Immediate - Key follows command in the descriptor */
#define KEY_IMM			0x00800000

/*
 * Encrypted - Key is encrypted either with the KEK, or
 * with the TDKEK if TK is set
 */
#define KEY_ENC			0x00400000

/*
 * No Write Back - Do not allow key to be FIFO STOREd
 */
#define KEY_NWB			0x00200000

/*
 * Enhanced Encryption of Key
 */
#define KEY_EKT			0x00100000

/*
 * Encrypted with Trusted Key
 */
#define KEY_TK			0x00008000

/*
 * KDEST - Key Destination: 0 - class key register,
 * 1 - PKHA 'e', 2 - AFHA Sbox, 3 - MDHA split-key
 */
#define KEY_DEST_SHIFT		16
#define KEY_DEST_MASK		(0x03 << KEY_DEST_SHIFT)

#define KEY_DEST_CLASS_REG	(0x00 << KEY_DEST_SHIFT)
#define KEY_DEST_PKHA_E		(0x01 << KEY_DEST_SHIFT)
#define KEY_DEST_AFHA_SBOX	(0x02 << KEY_DEST_SHIFT)
#define KEY_DEST_MDHA_SPLIT	(0x03 << KEY_DEST_SHIFT)

/* Length in bytes */
#define KEY_LENGTH_MASK		0x000003ff

/*
 * LOAD/SEQ_LOAD/STORE/SEQ_STORE Command Constructs
 */

/*
 * Load/Store Destination: 0 = class independent CCB,
 * 1 = class 1 CCB, 2 = class 2 CCB, 3 = DECO
 */
#define LDST_CLASS_SHIFT	25
#define LDST_CLASS_MASK		(0x03 << LDST_CLASS_SHIFT)
#define LDST_CLASS_IND_CCB	(0x00 << LDST_CLASS_SHIFT)
#define LDST_CLASS_1_CCB	(0x01 << LDST_CLASS_SHIFT)
#define LDST_CLASS_2_CCB	(0x02 << LDST_CLASS_SHIFT)
#define LDST_CLASS_DECO		(0x03 << LDST_CLASS_SHIFT)

/* Scatter-Gather Table/Variable Length Field */
#define LDST_SGF		0x01000000
#define LDST_VLF		LDST_SGF

/* Immediate - Key follows this command in descriptor */
#define LDST_IMM_MASK		1
#define LDST_IMM_SHIFT		23
#define LDST_IMM		(LDST_IMM_MASK << LDST_IMM_SHIFT)

/* SRC/DST - Destination for LOAD, Source for STORE */
#define LDST_SRCDST_SHIFT	16
#define LDST_SRCDST_MASK	(0x7f << LDST_SRCDST_SHIFT)

#define LDST_SRCDST_BYTE_CONTEXT	(0x20 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_BYTE_KEY		(0x40 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_BYTE_INFIFO		(0x7c << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_BYTE_OUTFIFO	(0x7e << LDST_SRCDST_SHIFT)

#define LDST_SRCDST_WORD_MODE_REG	(0x00 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_KEYSZ_REG	(0x01 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DATASZ_REG	(0x02 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_ICVSZ_REG	(0x03 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_CHACTRL	(0x06 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DECOCTRL	(0x06 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_IRQCTRL	(0x07 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DECO_PCLOVRD	(0x07 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_CLRW		(0x08 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DECO_MATH0	(0x08 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_STAT		(0x09 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DECO_MATH1	(0x09 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DECO_MATH2	(0x0a << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DECO_AAD_SZ	(0x0b << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DECO_MATH3	(0x0b << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_CLASS1_ICV_SZ	(0x0c << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_ALTDS_CLASS1	(0x0f << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_PKHA_A_SZ	(0x10 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_PKHA_B_SZ	(0x11 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_PKHA_N_SZ	(0x12 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_PKHA_E_SZ	(0x13 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_CLASS_CTX	(0x20 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DESCBUF	(0x40 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DESCBUF_JOB	(0x41 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DESCBUF_SHARED	(0x42 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DESCBUF_JOB_WE	(0x45 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_DESCBUF_SHARED_WE (0x46 << LDST_SRCDST_SHIFT)
#define LDST_SRCDST_WORD_INFO_FIFO	(0x7a << LDST_SRCDST_SHIFT)

/* Offset in source/destination */
#define LDST_OFFSET_SHIFT	8
#define LDST_OFFSET_MASK	(0xff << LDST_OFFSET_SHIFT)

/* LDOFF definitions used when DST = LDST_SRCDST_WORD_DECOCTRL */
/* These could also be shifted by LDST_OFFSET_SHIFT - this reads better */
#define LDOFF_CHG_SHARE_SHIFT		0
#define LDOFF_CHG_SHARE_MASK		(0x3 << LDOFF_CHG_SHARE_SHIFT)
#define LDOFF_CHG_SHARE_NEVER		(0x1 << LDOFF_CHG_SHARE_SHIFT)
#define LDOFF_CHG_SHARE_OK_PROP		(0x2 << LDOFF_CHG_SHARE_SHIFT)
#define LDOFF_CHG_SHARE_OK_NO_PROP	(0x3 << LDOFF_CHG_SHARE_SHIFT)

#define LDOFF_ENABLE_AUTO_NFIFO		(1 << 2)
#define LDOFF_DISABLE_AUTO_NFIFO	(1 << 3)

#define LDOFF_CHG_NONSEQLIODN_SHIFT	4
#define LDOFF_CHG_NONSEQLIODN_MASK	(0x3 << LDOFF_CHG_NONSEQLIODN_SHIFT)
#define LDOFF_CHG_NONSEQLIODN_SEQ	(0x1 << LDOFF_CHG_NONSEQLIODN_SHIFT)
#define LDOFF_CHG_NONSEQLIODN_NON_SEQ	(0x2 << LDOFF_CHG_NONSEQLIODN_SHIFT)
#define LDOFF_CHG_NONSEQLIODN_TRUSTED	(0x3 << LDOFF_CHG_NONSEQLIODN_SHIFT)

#define LDOFF_CHG_SEQLIODN_SHIFT	6
#define LDOFF_CHG_SEQLIODN_MASK		(0x3 << LDOFF_CHG_SEQLIODN_SHIFT)
#define LDOFF_CHG_SEQLIODN_SEQ		(0x1 << LDOFF_CHG_SEQLIODN_SHIFT)
#define LDOFF_CHG_SEQLIODN_NON_SEQ	(0x2 << LDOFF_CHG_SEQLIODN_SHIFT)
#define LDOFF_CHG_SEQLIODN_TRUSTED	(0x3 << LDOFF_CHG_SEQLIODN_SHIFT)

/* Data length in bytes	*/
#define LDST_LEN_SHIFT		0
#define LDST_LEN_MASK		(0xff << LDST_LEN_SHIFT)

/* Special Length definitions when dst=deco-ctrl */
#define LDLEN_ENABLE_OSL_COUNT		(1 << 7)
#define LDLEN_RST_CHA_OFIFO_PTR		(1 << 6)
#define LDLEN_RST_OFIFO			(1 << 5)
#define LDLEN_SET_OFIFO_OFF_VALID	(1 << 4)
#define LDLEN_SET_OFIFO_OFF_RSVD	(1 << 3)
#define LDLEN_SET_OFIFO_OFFSET_SHIFT	0
#define LDLEN_SET_OFIFO_OFFSET_MASK	(3 << LDLEN_SET_OFIFO_OFFSET_SHIFT)

/*
 * AAD Definitions
 */
#define AES_KEY_SHIFT		8
#define LD_CCM_MODE		0x66
#define KEY_AES_SRC		(0x55 << AES_KEY_SHIFT)

/*
 * FIFO_LOAD/FIFO_STORE/SEQ_FIFO_LOAD/SEQ_FIFO_STORE
 * Command Constructs
 */

/*
 * Load Destination: 0 = skip (SEQ_FIFO_LOAD only),
 * 1 = Load for Class1, 2 = Load for Class2, 3 = Load both
 * Store Source: 0 = normal, 1 = Class1key, 2 = Class2key
 */
#define FIFOLD_CLASS_SHIFT	25
#define FIFOLD_CLASS_MASK	(0x03 << FIFOLD_CLASS_SHIFT)
#define FIFOLD_CLASS_SKIP	(0x00 << FIFOLD_CLASS_SHIFT)
#define FIFOLD_CLASS_CLASS1	(0x01 << FIFOLD_CLASS_SHIFT)
#define FIFOLD_CLASS_CLASS2	(0x02 << FIFOLD_CLASS_SHIFT)
#define FIFOLD_CLASS_BOTH	(0x03 << FIFOLD_CLASS_SHIFT)

#define FIFOST_CLASS_SHIFT	25
#define FIFOST_CLASS_MASK	(0x03 << FIFOST_CLASS_SHIFT)
#define FIFOST_CLASS_NORMAL	(0x00 << FIFOST_CLASS_SHIFT)
#define FIFOST_CLASS_CLASS1KEY	(0x01 << FIFOST_CLASS_SHIFT)
#define FIFOST_CLASS_CLASS2KEY	(0x02 << FIFOST_CLASS_SHIFT)

/*
 * Scatter-Gather Table/Variable Length Field
 * If set for FIFO_LOAD, refers to a SG table. Within
 * SEQ_FIFO_LOAD, is variable input sequence
 */
#define FIFOLDST_SGF_SHIFT	24
#define FIFOLDST_SGF_MASK	(1 << FIFOLDST_SGF_SHIFT)
#define FIFOLDST_VLF_MASK	(1 << FIFOLDST_SGF_SHIFT)
#define FIFOLDST_SGF		(1 << FIFOLDST_SGF_SHIFT)
#define FIFOLDST_VLF		(1 << FIFOLDST_SGF_SHIFT)

/* Immediate - Data follows command in descriptor */
#define FIFOLD_IMM_SHIFT	23
#define FIFOLD_IMM_MASK		(1 << FIFOLD_IMM_SHIFT)
#define FIFOLD_IMM		(1 << FIFOLD_IMM_SHIFT)

/* Continue - Not the last FIFO store to come */
#define FIFOST_CONT_SHIFT	23
#define FIFOST_CONT_MASK	(1 << FIFOST_CONT_SHIFT)

/*
 * Extended Length - use 32-bit extended length that
 * follows the pointer field. Illegal with IMM set
 */
#define FIFOLDST_EXT_SHIFT	22
#define FIFOLDST_EXT_MASK	(1 << FIFOLDST_EXT_SHIFT)
#define FIFOLDST_EXT		(1 << FIFOLDST_EXT_SHIFT)

/* Input data type.*/
#define FIFOLD_TYPE_SHIFT	16
#define FIFOLD_CONT_TYPE_SHIFT	19 /* shift past last-flush bits */
#define FIFOLD_TYPE_MASK	(0x3f << FIFOLD_TYPE_SHIFT)

/* PK types */
#define FIFOLD_TYPE_PK		(0x00 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_MASK	(0x30 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_TYPEMASK (0x0f << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_A0	(0x00 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_A1	(0x01 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_A2	(0x02 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_A3	(0x03 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_B0	(0x04 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_B1	(0x05 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_B2	(0x06 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_B3	(0x07 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_N	(0x08 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_A	(0x0c << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_PK_B	(0x0d << FIFOLD_TYPE_SHIFT)

/* Other types. Need to OR in last/flush bits as desired */
#define FIFOLD_TYPE_MSG_MASK	(0x38 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_MSG		(0x10 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_MSG1OUT2	(0x18 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_IV		(0x20 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_BITDATA	(0x28 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_AAD		(0x30 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_ICV		(0x38 << FIFOLD_TYPE_SHIFT)

/* Last/Flush bits for use with "other" types above */
#define FIFOLD_TYPE_ACT_MASK	(0x07 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_NOACTION	(0x00 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_FLUSH1	(0x01 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_LAST1	(0x02 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_LAST2FLUSH	(0x03 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_LAST2	(0x04 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_LAST2FLUSH1 (0x05 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_LASTBOTH	(0x06 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_LASTBOTHFL	(0x07 << FIFOLD_TYPE_SHIFT)
#define FIFOLD_TYPE_NOINFOFIFO	(0x0F << FIFOLD_TYPE_SHIFT)

#define FIFOLDST_LEN_MASK	0xffff
#define FIFOLDST_EXT_LEN_MASK	0xffffffff

/* Output data types */
#define FIFOST_TYPE_SHIFT	16
#define FIFOST_TYPE_MASK	(0x3f << FIFOST_TYPE_SHIFT)

#define FIFOST_TYPE_PKHA_A0	 (0x00 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_PKHA_A1	 (0x01 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_PKHA_A2	 (0x02 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_PKHA_A3	 (0x03 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_PKHA_B0	 (0x04 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_PKHA_B1	 (0x05 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_PKHA_B2	 (0x06 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_PKHA_B3	 (0x07 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_PKHA_N	 (0x08 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_PKHA_A	 (0x0c << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_PKHA_B	 (0x0d << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_AF_SBOX_JKEK (0x10 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_AF_SBOX_TKEK (0x21 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_PKHA_E_JKEK	 (0x22 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_PKHA_E_TKEK	 (0x23 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_KEY_KEK	 (0x24 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_KEY_TKEK	 (0x25 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_SPLIT_KEK	 (0x26 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_SPLIT_TKEK	 (0x27 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_OUTFIFO_KEK	 (0x28 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_OUTFIFO_TKEK (0x29 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_MESSAGE_DATA (0x30 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_RNGSTORE	 (0x34 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_RNGFIFO	 (0x35 << FIFOST_TYPE_SHIFT)
#define FIFOST_TYPE_SKIP	 (0x3f << FIFOST_TYPE_SHIFT)

/*
 * OPERATION Command Constructs
 */

/* Operation type selectors - OP TYPE */
#define OP_TYPE_SHIFT		24
#define OP_TYPE_MASK		(0x07 << OP_TYPE_SHIFT)

#define OP_TYPE_UNI_PROTOCOL	(0x00 << OP_TYPE_SHIFT)
#define OP_TYPE_PK		(0x01 << OP_TYPE_SHIFT)
#define OP_TYPE_CLASS1_ALG	(0x02 << OP_TYPE_SHIFT)
#define OP_TYPE_CLASS2_ALG	(0x04 << OP_TYPE_SHIFT)
#define OP_TYPE_DECAP_PROTOCOL	(0x06 << OP_TYPE_SHIFT)
#define OP_TYPE_ENCAP_PROTOCOL	(0x07 << OP_TYPE_SHIFT)

/* ProtocolID selectors - PROTID */
#define OP_PCLID_SHIFT		16
#define OP_PCLID_MASK		(0xff << 16)

/* Assuming OP_TYPE = OP_TYPE_UNI_PROTOCOL */
#define OP_PCLID_SECMEM		0x08
#define OP_PCLID_BLOB		(0x0d << OP_PCLID_SHIFT)
#define OP_PCLID_SECRETKEY	(0x11 << OP_PCLID_SHIFT)
#define OP_PCLID_PUBLICKEYPAIR	(0x14 << OP_PCLID_SHIFT)
#define OP_PCLID_DSA_SIGN	(0x15 << OP_PCLID_SHIFT)
#define OP_PCLID_DSA_VERIFY	(0x16 << OP_PCLID_SHIFT)

/* Assuming OP_TYPE = OP_TYPE_DECAP_PROTOCOL */
#define OP_PCLID_MP_PUB_KEY	(0x14 << OP_PCLID_SHIFT)
#define OP_PCLID_MP_SIGN	(0x15 << OP_PCLID_SHIFT)

/* Assuming OP_TYPE = OP_TYPE_ENCAP_PROTOCOL */
#define OP_PCLID_MP_PRIV_KEY	(0x14 << OP_PCLID_SHIFT)

/* PROTINFO fields for discrete log public key protocols */
#define OP_PROTINFO_F2M_FP	0x00000001
#define OP_PROTINFO_ECC_DL	0x00000002
#define OP_PROTINFO_ENC_PRI	0x00000004
#define OP_PROTINFO_TEST	0x00000008
#define OP_PROTINFO_EXT_PRI	0x00000010
#define OP_PROTINFO_ENC_Z	0x00000020
#define OP_PROTINFO_EKT_Z	0x00000040
#define OP_PROTINFO_MES_REP	0x00000400
#define OP_PROTINFO_HASH_MD5	0x00000000
#define OP_PROTINFO_HASH_SHA1	0x00000080
#define OP_PROTINFO_HASH_SHA224	0x00000100
#define OP_PROTINFO_HASH_SHA256	0x00000180
#define OP_PROTINFO_HASH_SHA384	0x00000200
#define OP_PROTINFO_HASH_SHA512	0x00000280

/* For non-protocol/alg-only op commands */
#define OP_ALG_TYPE_SHIFT	24
#define OP_ALG_TYPE_MASK	(0x7 << OP_ALG_TYPE_SHIFT)
#define OP_ALG_TYPE_CLASS1	2
#define OP_ALG_TYPE_CLASS2	4

#define OP_ALG_ALGSEL_SHIFT	16
#define OP_ALG_ALGSEL_MASK	(0xff << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_SUBMASK	(0x0f << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_AES	(0x10 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_DES	(0x20 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_3DES	(0x21 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_ARC4	(0x30 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_MD5	(0x40 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_SHA1	(0x41 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_SHA224	(0x42 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_SHA256	(0x43 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_SHA384	(0x44 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_SHA512	(0x45 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_RNG	(0x50 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_SNOW	(0x60 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_SNOW_F8	(0x60 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_KASUMI	(0x70 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_CRC	(0x90 << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_ALGSEL_SNOW_F9	(0xA0 << OP_ALG_ALGSEL_SHIFT)

#define OP_ALG_AAI_SHIFT	4
#define OP_ALG_AAI_MASK		(0x1ff << OP_ALG_AAI_SHIFT)

/* randomizer AAI set */
#define OP_ALG_AAI_RNG		(0x00 << OP_ALG_AAI_SHIFT)
#define OP_ALG_AAI_RNG_NZB	(0x10 << OP_ALG_AAI_SHIFT)
#define OP_ALG_AAI_RNG_OBP	(0x20 << OP_ALG_AAI_SHIFT)

/* RNG4 AAI set */
#define OP_ALG_AAI_RNG4_SH_0	(0x00 << OP_ALG_AAI_SHIFT)
#define OP_ALG_AAI_RNG4_SH_1	(0x01 << OP_ALG_AAI_SHIFT)
#define OP_ALG_AAI_RNG4_PS	(0x40 << OP_ALG_AAI_SHIFT)
#define OP_ALG_AAI_RNG4_AI	(0x80 << OP_ALG_AAI_SHIFT)
#define OP_ALG_AAI_RNG4_SK	(0x100 << OP_ALG_AAI_SHIFT)

/* hmac/smac AAI set */
#define OP_ALG_AAI_HASH		(0x00 << OP_ALG_AAI_SHIFT)
#define OP_ALG_AAI_HMAC		(0x01 << OP_ALG_AAI_SHIFT)
#define OP_ALG_AAI_SMAC		(0x02 << OP_ALG_AAI_SHIFT)
#define OP_ALG_AAI_HMAC_PRECOMP	(0x04 << OP_ALG_AAI_SHIFT)

#define OP_ALG_AS_SHIFT		2
#define OP_ALG_AS_MASK		(0x3 << OP_ALG_AS_SHIFT)
#define OP_ALG_AS_UPDATE	(0 << OP_ALG_AS_SHIFT)
#define OP_ALG_AS_INIT		(1 << OP_ALG_AS_SHIFT)
#define OP_ALG_AS_FINALIZE	(2 << OP_ALG_AS_SHIFT)
#define OP_ALG_AS_INITFINAL	(3 << OP_ALG_AS_SHIFT)

#define OP_ALG_ICV_SHIFT	1
#define OP_ALG_ICV_MASK		(1 << OP_ALG_ICV_SHIFT)
#define OP_ALG_ICV_OFF		(0 << OP_ALG_ICV_SHIFT)
#define OP_ALG_ICV_ON		(1 << OP_ALG_ICV_SHIFT)

#define OP_ALG_DIR_SHIFT	0
#define OP_ALG_DIR_MASK		1
#define OP_ALG_DECRYPT		0
#define OP_ALG_ENCRYPT		1

/* PKHA algorithm type set */
#define OP_ALG_PK		0x00800000
#define OP_ALG_PK_FUN_MASK	0x3f /* clrmem, modmath, or cpymem */

/* PKHA mode modular-arithmetic functions */
#define OP_ALG_PKMODE_MOD_EXPO		0x006

/*
 * SEQ_IN_PTR Command Constructs
 */

/* Release Buffers */
#define SQIN_RBS	0x04000000

/* Sequence pointer is really a descriptor */
#define SQIN_INL	0x02000000

/* Sequence pointer is a scatter-gather table */
#define SQIN_SGF	0x01000000

/* Appends to a previous pointer */
#define SQIN_PRE	0x00800000

/* Use extended length following pointer */
#define SQIN_EXT	0x00400000

/* Restore sequence with pointer/length */
#define SQIN_RTO	0x00200000

/* Replace job descriptor */
#define SQIN_RJD	0x00100000

#define SQIN_LEN_SHIFT		 0
#define SQIN_LEN_MASK		(0xffff << SQIN_LEN_SHIFT)

/*
 * SEQ_OUT_PTR Command Constructs
 */

/* Sequence pointer is a scatter-gather table */
#define SQOUT_SGF	0x01000000

/* Appends to a previous pointer */
#define SQOUT_PRE	SQIN_PRE

/* Restore sequence with pointer/length */
#define SQOUT_RTO	 SQIN_RTO

/* Use extended length following pointer */
#define SQOUT_EXT	0x00400000

#define SQOUT_LEN_SHIFT		0
#define SQOUT_LEN_MASK		(0xffff << SQOUT_LEN_SHIFT)

/*
 * MOVE Command Constructs
 */

#define MOVE_AUX_SHIFT		25
#define MOVE_AUX_MASK		(3 << MOVE_AUX_SHIFT)
#define MOVE_AUX_MS		(2 << MOVE_AUX_SHIFT)
#define MOVE_AUX_LS		(1 << MOVE_AUX_SHIFT)

#define MOVE_WAITCOMP_SHIFT	24
#define MOVE_WAITCOMP_MASK	(1 << MOVE_WAITCOMP_SHIFT)
#define MOVE_WAITCOMP		(1 << MOVE_WAITCOMP_SHIFT)

#define MOVE_SRC_SHIFT		20
#define MOVE_SRC_MASK		(0x0f << MOVE_SRC_SHIFT)
#define MOVE_SRC_CLASS1CTX	(0x00 << MOVE_SRC_SHIFT)
#define MOVE_SRC_CLASS2CTX	(0x01 << MOVE_SRC_SHIFT)
#define MOVE_SRC_OUTFIFO	(0x02 << MOVE_SRC_SHIFT)
#define MOVE_SRC_DESCBUF	(0x03 << MOVE_SRC_SHIFT)
#define MOVE_SRC_MATH0		(0x04 << MOVE_SRC_SHIFT)
#define MOVE_SRC_MATH1		(0x05 << MOVE_SRC_SHIFT)
#define MOVE_SRC_MATH2		(0x06 << MOVE_SRC_SHIFT)
#define MOVE_SRC_MATH3		(0x07 << MOVE_SRC_SHIFT)
#define MOVE_SRC_INFIFO		(0x08 << MOVE_SRC_SHIFT)
#define MOVE_SRC_INFIFO_CL	(0x09 << MOVE_SRC_SHIFT)

#define MOVE_DEST_SHIFT		16
#define MOVE_DEST_MASK		(0x0f << MOVE_DEST_SHIFT)
#define MOVE_DEST_CLASS1CTX	(0x00 << MOVE_DEST_SHIFT)
#define MOVE_DEST_CLASS2CTX	(0x01 << MOVE_DEST_SHIFT)
#define MOVE_DEST_OUTFIFO	(0x02 << MOVE_DEST_SHIFT)
#define MOVE_DEST_DESCBUF	(0x03 << MOVE_DEST_SHIFT)
#define MOVE_DEST_MATH0		(0x04 << MOVE_DEST_SHIFT)
#define MOVE_DEST_MATH1		(0x05 << MOVE_DEST_SHIFT)
#define MOVE_DEST_MATH2		(0x06 << MOVE_DEST_SHIFT)
#define MOVE_DEST_MATH3		(0x07 << MOVE_DEST_SHIFT)
#define MOVE_DEST_CLASS1INFIFO	(0x08 << MOVE_DEST_SHIFT)
#define MOVE_DEST_CLASS2INFIFO	(0x09 << MOVE_DEST_SHIFT)
#define MOVE_DEST_INFIFO_NOINFO (0x0a << MOVE_DEST_SHIFT)
#define MOVE_DEST_PK_A		(0x0c << MOVE_DEST_SHIFT)
#define MOVE_DEST_CLASS1KEY	(0x0d << MOVE_DEST_SHIFT)
#define MOVE_DEST_CLASS2KEY	(0x0e << MOVE_DEST_SHIFT)

#define MOVE_OFFSET_SHIFT	8
#define MOVE_OFFSET_MASK	(0xff << MOVE_OFFSET_SHIFT)

#define MOVE_LEN_SHIFT		0
#define MOVE_LEN_MASK		(0xff << MOVE_LEN_SHIFT)

#define MOVELEN_MRSEL_SHIFT	0
#define MOVELEN_MRSEL_MASK	(0x3 << MOVE_LEN_SHIFT)

/*
 * JUMP Command Constructs
 */

#define JUMP_CLASS_SHIFT	25
#define JUMP_CLASS_MASK		(3 << JUMP_CLASS_SHIFT)
#define JUMP_CLASS_NONE		0
#define JUMP_CLASS_CLASS1	(1 << JUMP_CLASS_SHIFT)
#define JUMP_CLASS_CLASS2	(2 << JUMP_CLASS_SHIFT)
#define JUMP_CLASS_BOTH		(3 << JUMP_CLASS_SHIFT)

#define JUMP_JSL_SHIFT		24
#define JUMP_JSL_MASK		(1 << JUMP_JSL_SHIFT)
#define JUMP_JSL		(1 << JUMP_JSL_SHIFT)

#define JUMP_TYPE_SHIFT		22
#define JUMP_TYPE_MASK		(0x03 << JUMP_TYPE_SHIFT)
#define JUMP_TYPE_LOCAL		(0x00 << JUMP_TYPE_SHIFT)
#define JUMP_TYPE_NONLOCAL	(0x01 << JUMP_TYPE_SHIFT)
#define JUMP_TYPE_HALT		(0x02 << JUMP_TYPE_SHIFT)
#define JUMP_TYPE_HALT_USER	(0x03 << JUMP_TYPE_SHIFT)

#define JUMP_TEST_SHIFT		16
#define JUMP_TEST_MASK		(0x03 << JUMP_TEST_SHIFT)
#define JUMP_TEST_ALL		(0x00 << JUMP_TEST_SHIFT)
#define JUMP_TEST_INVALL	(0x01 << JUMP_TEST_SHIFT)
#define JUMP_TEST_ANY		(0x02 << JUMP_TEST_SHIFT)
#define JUMP_TEST_INVANY	(0x03 << JUMP_TEST_SHIFT)

/* Condition codes. JSL bit is factored in */
#define JUMP_COND_SHIFT		8
#define JUMP_COND_MASK		(0x100ff << JUMP_COND_SHIFT)
#define JUMP_COND_PK_0		(0x80 << JUMP_COND_SHIFT)
#define JUMP_COND_PK_GCD_1	(0x40 << JUMP_COND_SHIFT)
#define JUMP_COND_PK_PRIME	(0x20 << JUMP_COND_SHIFT)
#define JUMP_COND_MATH_N	(0x08 << JUMP_COND_SHIFT)
#define JUMP_COND_MATH_Z	(0x04 << JUMP_COND_SHIFT)
#define JUMP_COND_MATH_C	(0x02 << JUMP_COND_SHIFT)
#define JUMP_COND_MATH_NV	(0x01 << JUMP_COND_SHIFT)

#define JUMP_COND_JRP		((0x80 << JUMP_COND_SHIFT) | JUMP_JSL)
#define JUMP_COND_SHRD		((0x40 << JUMP_COND_SHIFT) | JUMP_JSL)
#define JUMP_COND_SELF		((0x20 << JUMP_COND_SHIFT) | JUMP_JSL)
#define JUMP_COND_CALM		((0x10 << JUMP_COND_SHIFT) | JUMP_JSL)
#define JUMP_COND_NIP		((0x08 << JUMP_COND_SHIFT) | JUMP_JSL)
#define JUMP_COND_NIFP		((0x04 << JUMP_COND_SHIFT) | JUMP_JSL)
#define JUMP_COND_NOP		((0x02 << JUMP_COND_SHIFT) | JUMP_JSL)
#define JUMP_COND_NCP		((0x01 << JUMP_COND_SHIFT) | JUMP_JSL)

#define JUMP_OFFSET_SHIFT	0
#define JUMP_OFFSET_MASK	(0xff << JUMP_OFFSET_SHIFT)

#define OP_ALG_RNG4_SHIFT      4
#define OP_ALG_RNG4_MAS                (0x1f3 << OP_ALG_RNG4_SHIFT)
#define OP_ALG_RNG4_SK         (0x100 << OP_ALG_RNG4_SHIFT)


/* Structures for Protocol Data Blocks */
struct __packed pdb_ecdsa_verify {
	uint32_t pdb_hdr;
	dma_addr_t dma_q;	/* Pointer to q (elliptic curve) */
	dma_addr_t dma_r;	/* Pointer to r (elliptic curve) */
	dma_addr_t dma_g_xy;	/* Pointer to Gx,y (elliptic curve) */
	dma_addr_t dma_pkey;	/* Pointer to Wx,y (public key) */
	dma_addr_t dma_hash;	/* Pointer to hash input */
	dma_addr_t dma_c;	/* Pointer to C_signature */
	dma_addr_t dma_d;	/* Pointer to D_signature */
	dma_addr_t dma_buf;	/* Pointer to 64-byte temp buffer */
	dma_addr_t dma_ab;	/* Pointer to a,b (elliptic curve ) */
	uint32_t img_size;	/* Length of Message */
};

struct __packed pdb_ecdsa_sign {
	uint32_t pdb_hdr;
	dma_addr_t dma_q;	/* Pointer to q (elliptic curve) */
	dma_addr_t dma_r;	/* Pointer to r (elliptic curve) */
	dma_addr_t dma_g_xy;	/* Pointer to Gx,y (elliptic curve) */
	dma_addr_t dma_pri_key;	/* Pointer to S (Private key) */
	dma_addr_t dma_hash;	/* Pointer to hash input */
	dma_addr_t dma_c;	/* Pointer to C_signature */
	dma_addr_t dma_d;	/* Pointer to D_signature */
	dma_addr_t dma_ab;	/* Pointer to a,b (elliptic curve ) */
	dma_addr_t dma_u;	/* Pointer to Per Message Random */
	uint32_t img_size;	/* Length of Message */
};

#define PDB_ECDSA_SGF_SHIFT	23
#define PDB_ECDSA_L_SHIFT	7
#define PDB_ECDSA_N_SHIFT	0

struct __packed pdb_mp_pub_k {
	uint32_t pdb_hdr;
	#define PDB_MP_PUB_K_SGF_SHIFT		31
	dma_addr_t dma_pkey;	/* Pointer to Wx,y (public key) */
};

struct __packed pdb_mp_sign {
	uint32_t pdb_hdr;
	#define PDB_MP_SIGN_SGF_SHIFT		28
	dma_addr_t dma_addr_msg;	/* Pointer to Message */
	dma_addr_t dma_addr_hash;	/* Pointer to hash output */
	dma_addr_t dma_addr_c_sig;	/* Pointer to C_signature */
	dma_addr_t dma_addr_d_sig;	/* Pointer to D_signature */
	uint32_t img_size;		/* Length of Message */
};

#define PDB_MP_CSEL_SHIFT	17
#define PDB_MP_CSEL_P256	0x3 << PDB_MP_CSEL_SHIFT	/* P-256 */
#define PDB_MP_CSEL_P384	0x4 << PDB_MP_CSEL_SHIFT	/* P-384 */
#define PDB_MP_CSEL_P521	0x5 << PDB_MP_CSEL_SHIFT	/* P-521 */

#endif /* DESC_H */
