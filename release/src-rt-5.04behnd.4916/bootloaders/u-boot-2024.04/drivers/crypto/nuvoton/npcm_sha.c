// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#include <common.h>
#include <dm.h>
#include <hash.h>
#include <malloc.h>
#include <uboot_aes.h>
#include <asm/io.h>

#define HASH_DIG_H_NUM			8

#define HASH_CTR_STS_SHA_EN             BIT(0)
#define HASH_CTR_STS_SHA_BUSY           BIT(1)
#define HASH_CTR_STS_SHA_RST            BIT(2)
#define HASH_CFG_SHA1_SHA2              BIT(0)

/* SHA type */
enum npcm_sha_type {
	npcm_sha_type_sha2 = 0,
	npcm_sha_type_sha1,
	npcm_sha_type_num
};

struct npcm_sha_regs {
	unsigned int hash_data_in;
	unsigned char hash_ctr_sts;
	unsigned char reserved_0[0x03];
	unsigned char hash_cfg;
	unsigned char reserved_1[0x03];
	unsigned char hash_ver;
	unsigned char reserved_2[0x13];
	unsigned int hash_dig[HASH_DIG_H_NUM];
};

struct npcm_sha_priv {
	struct npcm_sha_regs *regs;
};

static struct npcm_sha_priv *sha_priv;

#ifdef SHA_DEBUG_MODULE
#define sha_print(fmt, args...)  printf(fmt, ##args)
#else
#define sha_print(fmt, args...)  (void)0
#endif

#define SHA_BLOCK_LENGTH        (512 / 8)
#define SHA_2_HASH_LENGTH       (256 / 8)
#define SHA_1_HASH_LENGTH       (160 / 8)
#define SHA_HASH_LENGTH(type)   ((type == npcm_sha_type_sha2) ? \
							(SHA_2_HASH_LENGTH) : (SHA_1_HASH_LENGTH))

#define SHA_SECRUN_BUFF_SIZE    64
#define SHA_TIMEOUT             100
#define SHA_DATA_LAST_BYTE      0x80

#define SHA2_NUM_OF_SELF_TESTS  3
#define SHA1_NUM_OF_SELF_TESTS  4

#define NUVOTON_ALIGNMENT       4

/*-----------------------------------------------------------------------------*/
/* SHA instance struct handler                                                 */
/*-----------------------------------------------------------------------------*/
struct SHA_HANDLE_T {
	u32                 hv[SHA_2_HASH_LENGTH / sizeof(u32)];
	u32                 length0;
	u32                 length1;
	u32					block[SHA_BLOCK_LENGTH / sizeof(u32)];
	u8					type;
	bool                active;
};

// The # of bytes currently in the sha  block buffer
#define SHA_BUFF_POS(length)        ((length) & (SHA_BLOCK_LENGTH - 1))

// The # of free bytes in the sha block buffer
#define SHA_BUFF_FREE(length)       (SHA_BLOCK_LENGTH - SHA_BUFF_POS(length))

static void SHA_FlushLocalBuffer_l(const u32 *buff);
static int  SHA_BusyWait_l(void);
static void SHA_GetShaDigest_l(u8 *hashdigest, u8 type);
static void SHA_SetShaDigest_l(const u32 *hashdigest, u8 type);
static void SHA_SetBlock_l(const u8 *data, u32 len, u16 position, u32 *block);
static void SHA_ClearBlock_l(u16 len, u16 position, u32 *block);
static void SHA_SetLength32_l(struct SHA_HANDLE_T *handleptr, u32 *block);

static int SHA_Init(struct SHA_HANDLE_T *handleptr);
static int SHA_Start(struct SHA_HANDLE_T *handleptr, u8 type);
static int SHA_Update(struct SHA_HANDLE_T *handleptr, const u8 *buffer, u32 len);
static int SHA_Finish(struct SHA_HANDLE_T *handleptr, u8 *hashdigest);
static int SHA_Reset(void);
static int SHA_Power(bool on);
#ifdef SHA_PRINT
static void SHA_PrintRegs(void);
static void SHA_PrintVersion(void);
#endif

static struct SHA_HANDLE_T sha_handle;

/*----------------------------------------------------------------------------*/
/* Checks if give function returns int error, and returns the error           */
/* immediately after SHA disabling                                            */
/*----------------------------------------------------------------------------*/
int npcm_sha_check(int status)
{
	if (status != 0) {
		SHA_Power(false);
		return status;
	}
	return 0;
}

/*----------------------------------------------------------------------------*/
/* Function:        npcm_sha_calc                                          */
/*                                                                            */
/* Parameters:      type - SHA module type                                    */
/*                  inBuff  - Pointer to a buffer containing the data to      */
/*                            be hashed                                       */
/*                  len     - Length of the data to hash                      */
/*                  hashDigest - Pointer to a buffer where the reseulting     */
/*                               digest will be copied to                     */
/*                                                                            */
/* Returns:         0 on success or other int error code on error             */
/* Side effects:                                                              */
/* Description:                                                               */
/*                  This routine performs complete SHA calculation in one     */
/*                  step                                                      */
/*----------------------------------------------------------------------------*/
int npcm_sha_calc(u8 type, const u8 *inbuff, u32 len, u8 *hashdigest)
{
	int status;
	struct SHA_HANDLE_T handle;

	SHA_Init(&handle);
	SHA_Power(true);
	SHA_Reset();
	SHA_Start(&handle, type);
	status = SHA_Update(&handle, inbuff, len);
	npcm_sha_check(status);
	status = SHA_Finish(&handle, hashdigest);
	npcm_sha_check(status);
	SHA_Power(false);

	return 0;
}

/*
 * Computes hash value of input pbuf using h/w acceleration
 *
 * @param in_addr    A pointer to the input buffer
 * @param bufleni    Byte length of input buffer
 * @param out_addr   A pointer to the output buffer. When complete
 *           32 bytes are copied to pout[0]...pout[31]. Thus, a user
 *           should allocate at least 32 bytes at pOut in advance.
 * @param chunk_size chunk size for sha256
 */
void hw_sha256(const uchar *in_addr, uint buflen, uchar *out_addr, uint chunk_size)
{
	puts("\nhw_sha256 using BMC HW accelerator\t");
	npcm_sha_calc(npcm_sha_type_sha2, (u8 *)in_addr, buflen, (u8 *)out_addr);
}

/*
 * Computes hash value of input pbuf using h/w acceleration
 *
 * @param in_addr    A pointer to the input buffer
 * @param bufleni    Byte length of input buffer
 * @param out_addr   A pointer to the output buffer. When complete
 *           32 bytes are copied to pout[0]...pout[31]. Thus, a user
 *           should allocate at least 32 bytes at pOut in advance.
 * @param chunk_size chunk_size for sha1
 */
void hw_sha1(const uchar *in_addr, uint buflen, uchar *out_addr, uint chunk_size)
{
	puts("\nhw_sha1 using BMC HW accelerator\t");
	npcm_sha_calc(npcm_sha_type_sha1, (u8 *)in_addr, buflen, (u8 *)out_addr);
}

/*
 * Create the context for sha progressive hashing using h/w acceleration
 *
 * @algo: Pointer to the hash_algo struct
 * @ctxp: Pointer to the pointer of the context for hashing
 * @return 0 if ok, -ve on error
 */
int hw_sha_init(struct hash_algo *algo, void **ctxp)
{
	const char *algo_name1 = "sha1";
	const char *algo_name2 = "sha256";

	SHA_Init(&sha_handle);
	SHA_Power(true);
	SHA_Reset();
	if (!strcmp(algo_name1, algo->name))
		return SHA_Start(&sha_handle, npcm_sha_type_sha1);
	else if (!strcmp(algo_name2, algo->name))
		return SHA_Start(&sha_handle, npcm_sha_type_sha2);
	else
		return -EPROTO;
}

/*
 * Update buffer for sha progressive hashing using h/w acceleration
 *
 * The context is freed by this function if an error occurs.
 *
 * @algo: Pointer to the hash_algo struct
 * @ctx: Pointer to the context for hashing
 * @buf: Pointer to the buffer being hashed
 * @size: Size of the buffer being hashed
 * @is_last: 1 if this is the last update; 0 otherwise
 * @return 0 if ok, -ve on error
 */
int hw_sha_update(struct hash_algo *algo, void *ctx, const void *buf,
		  unsigned int size, int is_last)
{
	return SHA_Update(&sha_handle, buf, size);
}

/*
 * Copy sha hash result at destination location
 *
 * The context is freed after completion of hash operation or after an error.
 *
 * @algo: Pointer to the hash_algo struct
 * @ctx: Pointer to the context for hashing
 * @dest_buf: Pointer to the destination buffer where hash is to be copied
 * @size: Size of the buffer being hashed
 * @return 0 if ok, -ve on error
 */
int hw_sha_finish(struct hash_algo *algo, void *ctx, void *dest_buf, int size)
{
	int status;

	status = SHA_Finish(&sha_handle, dest_buf);
	npcm_sha_check(status);
	return SHA_Power(false);
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_Init                                                  */
/*                                                                            */
/* Parameters:      handlePtr - SHA processing handle pointer                 */
/* Returns:         0 on success or other int error code on error.            */
/* Side effects:                                                              */
/* Description:                                                               */
/*                  This routine initialize the SHA module                    */
/*----------------------------------------------------------------------------*/
static int SHA_Init(struct SHA_HANDLE_T *handleptr)
{
	handleptr->active = false;

	return 0;
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_Start                                                 */
/*                                                                            */
/* Parameters:      handlePtr   - SHA processing handle pointer               */
/*                  type        - SHA module type                             */
/*                                                                            */
/* Returns:         0 on success or other int error code on error.            */
/* Side effects:                                                              */
/* Description:                                                               */
/*                  This routine start a single SHA process                   */
/*----------------------------------------------------------------------------*/
static int SHA_Start(struct SHA_HANDLE_T *handleptr, u8 type)
{
	struct npcm_sha_regs *regs = sha_priv->regs;

	// Initialize handle
	handleptr->length0 = 0;
	handleptr->length1 = 0;
	handleptr->type = type;
	handleptr->active = true;

	// Set SHA type
	writeb(handleptr->type & HASH_CFG_SHA1_SHA2, &regs->hash_cfg);

	// Reset SHA hardware
	SHA_Reset();

	/* The handlePtr->hv is initialized with the correct IV as the SHA engine
	 * automatically fill the HASH_DIG_Hn registers according to SHA spec
	 * (following SHA_RST assertion)
	 */
	SHA_GetShaDigest_l((u8 *)handleptr->hv, type);

	// Init block with zeros
	memset(handleptr->block, 0, sizeof(handleptr->block));

	return 0;
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_Update                                                */
/*                                                                            */
/* Parameters:      handlePtr - SHA processing handle pointer                 */
/*                  buffer    - Pointer to the data that will be added to     */
/*                              the hash calculation                          */
/*                  len -      Length of data to add to SHA calculation       */
/*                                                                            */
/*                                                                            */
/* Returns:         0 on success or other int error code on error             */
/* Side effects:                                                              */
/* Description:                                                               */
/*                  This routine adds data to previously started SHA          */
/*                  calculation                                               */
/*----------------------------------------------------------------------------*/
static int SHA_Update(struct SHA_HANDLE_T *handleptr, const u8 *buffer, u32 len)
{
	struct npcm_sha_regs *regs = sha_priv->regs;
	u32 localbuffer[SHA_SECRUN_BUFF_SIZE / sizeof(u32)];
	u32 bufferlen = len;
	u16 pos = 0;
	u8 *blockptr;
	int status;

	// Error check
	if (!handleptr->active)
		return -EPROTO;

	// Wait till SHA is not busy
	status = SHA_BusyWait_l();
	npcm_sha_check(status);

	// Set SHA type
	writeb(handleptr->type & HASH_CFG_SHA1_SHA2, &regs->hash_cfg);

	// Write SHA latest digest into SHA module
	SHA_SetShaDigest_l(handleptr->hv, handleptr->type);

	// Set number of unhashed bytes which remained from last update
	pos = SHA_BUFF_POS(handleptr->length0);

	// Copy unhashed bytes which remained from last update to secrun buffer
	SHA_SetBlock_l((u8 *)handleptr->block, pos, 0, localbuffer);

	while (len) {
		// Wait for the hardware to be available (in case we are hashing)
		status = SHA_BusyWait_l();
		npcm_sha_check(status);

		// Move as much bytes  as we can into the secrun buffer
		bufferlen = min(len, SHA_BUFF_FREE(handleptr->length0));

		// Copy current given buffer to the secrun buffer
		SHA_SetBlock_l((u8 *)buffer, bufferlen, pos, localbuffer);

		// Update size of hashed bytes
		handleptr->length0 += bufferlen;

		if (handleptr->length0 < bufferlen)
			handleptr->length1++;

		// Update length of data left to digest
		len -= bufferlen;

		// Update given buffer pointer
		buffer += bufferlen;

		// If secrun buffer is full
		if (SHA_BUFF_POS(handleptr->length0) == 0) {
			/* We just filled up the buffer perfectly, so let it hash (we'll
			 * unload the hash only when we are done with all hashing)
			 */
			SHA_FlushLocalBuffer_l(localbuffer);

			pos = 0;
			bufferlen = 0;
		}
	}

	// Wait till SHA is not busy
	status = SHA_BusyWait_l();
	npcm_sha_check(status);

	/* Copy unhashed bytes from given buffer to handle block for next update/finish */
	blockptr = (u8 *)handleptr->block;
	while (bufferlen)
		blockptr[--bufferlen + pos] = *(--buffer);

	// Save SHA current digest
	SHA_GetShaDigest_l((u8 *)handleptr->hv, handleptr->type);

	return 0;
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_Finish                                                */
/*                                                                            */
/* Parameters:      handlePtr  - SHA processing handle pointer                */
/*                  hashDigest - Pointer to a buffer where the final digest   */
/*                               will be copied to                            */
/*                                                                            */
/* Returns:         0 on success or other int error code on error             */
/* Side effects:                                                              */
/* Description:                                                               */
/*                  This routine finish SHA calculation and get               */
/*                  the resulting SHA digest                                  */
/*----------------------------------------------------------------------------*/
static int SHA_Finish(struct SHA_HANDLE_T *handleptr, u8 *hashdigest)
{
	struct npcm_sha_regs *regs = sha_priv->regs;
	u32 localbuffer[SHA_SECRUN_BUFF_SIZE / sizeof(u32)];
	const u8 lastbyte = SHA_DATA_LAST_BYTE;
	u16 pos;
	int status;

	// Error check
	if (!handleptr->active)
		return -EPROTO;

	// Set SHA type
	writeb(handleptr->type & HASH_CFG_SHA1_SHA2, &regs->hash_cfg);

	// Wait till SHA is not busy
	status = SHA_BusyWait_l();
	npcm_sha_check(status);

	// Finish off the current buffer with the SHA spec'ed padding
	pos = SHA_BUFF_POS(handleptr->length0);

	// Init SHA digest
	SHA_SetShaDigest_l(handleptr->hv, handleptr->type);

	// Load data into secrun buffer
	SHA_SetBlock_l((u8 *)handleptr->block, pos, 0, localbuffer);

	// Set data last byte as in SHA algorithm spec
	SHA_SetBlock_l(&lastbyte, 1, pos++, localbuffer);

	// If the remainder of data is longer then one block
	if (pos > (SHA_BLOCK_LENGTH - 8)) {
		/* The length will be in the next block Pad the rest of the last block with 0's */
		SHA_ClearBlock_l((SHA_BLOCK_LENGTH - pos), pos, localbuffer);

		// Hash the current block
		SHA_FlushLocalBuffer_l(localbuffer);

		pos = 0;

		// Wait till SHA is not busy
		status = SHA_BusyWait_l();
		npcm_sha_check(status);
	}

	// Pad the rest of the last block with 0's except for the last 8-3 bytes
	SHA_ClearBlock_l((SHA_BLOCK_LENGTH - (8 - 3)) - pos, pos, localbuffer);

	/* The last 8-3 bytes are set to the bit-length of the message in big-endian form */
	SHA_SetLength32_l(handleptr, localbuffer);

	// Hash all that, and save the hash for the caller
	SHA_FlushLocalBuffer_l(localbuffer);

	// Wait till SHA is not busy
	status = SHA_BusyWait_l();
	npcm_sha_check(status);

	// Save SHA final digest into given buffer
	SHA_GetShaDigest_l(hashdigest, handleptr->type);

	// Free handle
	handleptr->active = false;

	return 0;
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_Reset                                                 */
/*                                                                            */
/* Parameters:      none                                                      */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:                                                               */
/*                  This routine reset SHA module                             */
/*----------------------------------------------------------------------------*/
static int SHA_Reset(void)
{
	struct npcm_sha_regs *regs = sha_priv->regs;

	writel(readl(&regs->hash_ctr_sts) | HASH_CTR_STS_SHA_RST, &regs->hash_ctr_sts);

	return 0;
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_Power                                                 */
/*                                                                            */
/* Parameters:      on - true enable the module, false disable the module     */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:                                                               */
/*                  This routine set SHA module power on/off                  */
/*----------------------------------------------------------------------------*/
static int SHA_Power(bool on)
{
	struct npcm_sha_regs *regs = sha_priv->regs;
	u8 hash_sts;

	hash_sts = readb(&regs->hash_ctr_sts) & ~HASH_CTR_STS_SHA_EN;
	writeb(hash_sts | (on & HASH_CTR_STS_SHA_EN), &regs->hash_ctr_sts);

	return 0;
}

#ifdef SHA_PRINT
/*----------------------------------------------------------------------------*/
/* Function:        SHA_PrintRegs                                             */
/*                                                                            */
/* Parameters:      none                                                      */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:                                                               */
/*                  This routine prints the module registers                  */
/*----------------------------------------------------------------------------*/
static void SHA_PrintRegs(void)
{
#ifdef SHA_DEBUG_MODULE
	struct npcm_sha_regs *regs = sha_priv->regs;
#endif
	unsigned int i;

	sha_print("/*--------------*/\n");
	sha_print("/*     SHA      */\n");
	sha_print("/*--------------*/\n\n");

	sha_print("HASH_CTR_STS    = 0x%02X\n", readb(&regs->hash_ctr_sts));
	sha_print("HASH_CFG        = 0x%02X\n", readb(&regs->hash_cfg));

	for (i = 0; i < HASH_DIG_H_NUM; i++)
		sha_print("HASH_DIG_H%d     = 0x%08X\n", i, readl(&regs->hash_dig[i]));

	sha_print("HASH_VER         = 0x%08X\n", readb(&regs->hash_ver));

	sha_print("\n");
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_PrintVersion                                          */
/*                                                                            */
/* Parameters:      none                                                      */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:                                                               */
/*                  This routine prints the module version                    */
/*----------------------------------------------------------------------------*/
static void SHA_PrintVersion(void)
{
	struct npcm_sha_regs *regs = sha_priv->regs;

	printf("SHA MODULE VER  = %d\n", readb(&regs->hash_ver));
}
#endif

/*----------------------------------------------------------------------------*/
/* Function:        npcm_sha_selftest                                      */
/*                                                                            */
/* Parameters:      type - SHA module type                                    */
/* Returns:         0 on success or other int error code on error             */
/* Side effects:                                                              */
/* Description:                                                               */
/*                  This routine performs various tests on the SHA HW and SW  */
/*----------------------------------------------------------------------------*/
int npcm_sha_selftest(u8 type)
{
	int status;
	struct SHA_HANDLE_T handle;
	u8 hashdigest[max(SHA_1_HASH_LENGTH, SHA_2_HASH_LENGTH)];
	u16 i, j;

	/*------------------------------------------------------------------------*/
	/* SHA1 tests info                                                        */
	/*------------------------------------------------------------------------*/

	static const u8 sha1selftestbuff[SHA1_NUM_OF_SELF_TESTS][94] = {
		{"abc"},
		{"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"},
		{"0123456789012345678901234567890123456789012345678901234567890123"},
		{0x30, 0x5c, 0x30, 0x2c, 0x02, 0x01, 0x00, 0x30, 0x09, 0x06, 0x05, 0x2b,
		 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x30, 0x06, 0x06, 0x04, 0x67, 0x2a,
		 0x01, 0x0c, 0x04, 0x14, 0xe1, 0xb6, 0x93, 0xfe, 0x33, 0x43, 0xc1, 0x20,
		 0x5d, 0x4b, 0xaa, 0xb8, 0x63, 0xfb, 0xcf, 0x6c, 0x46, 0x1e, 0x88, 0x04,
		 0x30, 0x2c, 0x02, 0x01, 0x00, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03,
		 0x02, 0x1a, 0x05, 0x00, 0x30, 0x06, 0x06, 0x04, 0x67, 0x2a, 0x01, 0x0c,
		 0x04, 0x14, 0x13, 0xc1, 0x0c, 0xfc, 0xc8, 0x92, 0xd7, 0xde, 0x07, 0x1c,
		 0x40, 0xde, 0x4f, 0xcd, 0x07, 0x5b, 0x68, 0x20, 0x5a, 0x6c}
	};

	static const u8 sha1selftestbufflen[SHA1_NUM_OF_SELF_TESTS] = {
		3, 56, 64, 94
	};

	static const u8 sha1selftestexpres[SHA1_NUM_OF_SELF_TESTS][SHA_1_HASH_LENGTH] = {
		{0xA9, 0x99, 0x3E, 0x36,
		 0x47, 0x06, 0x81, 0x6A,
		 0xBA, 0x3E, 0x25, 0x71,
		 0x78, 0x50, 0xC2, 0x6C,
		 0x9C, 0xD0, 0xD8, 0x9D},
		{0x84, 0x98, 0x3E, 0x44,
		 0x1C, 0x3B, 0xD2, 0x6E,
		 0xBA, 0xAE, 0x4A, 0xA1,
		 0xF9, 0x51, 0x29, 0xE5,
		 0xE5, 0x46, 0x70, 0xF1},
		{0xCF, 0x08, 0x00, 0xF7,
		 0x64, 0x4A, 0xCE, 0x3C,
		 0xB4, 0xC3, 0xFA, 0x33,
		 0x38, 0x8D, 0x3B, 0xA0,
		 0xEA, 0x3C, 0x8B, 0x6E},
		{0xc9, 0x84, 0x45, 0xc8,
		 0x64, 0x04, 0xb1, 0xe3,
		 0x3c, 0x6b, 0x0a, 0x8c,
		 0x8b, 0x80, 0x94, 0xfc,
		 0xf3, 0xc9, 0x98, 0xab}
	};

	/*------------------------------------------------------------------------*/
	/* SHA2 tests info                                                        */
	/*------------------------------------------------------------------------*/

	static const u8 sha2selftestbuff[SHA2_NUM_OF_SELF_TESTS][100] = {
		{ "abc" },
		{ "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" },
		{'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
		 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
		 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
		 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
		 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
		 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
		 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
		 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
		 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
		 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a'}
	};

	static const u8 sha2selftestbufflen[SHA2_NUM_OF_SELF_TESTS] = {
		3, 56, 100
	};

	static const u8 sha2selftestexpres[SHA2_NUM_OF_SELF_TESTS][SHA_2_HASH_LENGTH] = {
		/*
		 * SHA-256 test vectors
		 */
		{ 0xBA, 0x78, 0x16, 0xBF, 0x8F, 0x01, 0xCF, 0xEA,
		  0x41, 0x41, 0x40, 0xDE, 0x5D, 0xAE, 0x22, 0x23,
		  0xB0, 0x03, 0x61, 0xA3, 0x96, 0x17, 0x7A, 0x9C,
		  0xB4, 0x10, 0xFF, 0x61, 0xF2, 0x00, 0x15, 0xAD },
		{ 0x24, 0x8D, 0x6A, 0x61, 0xD2, 0x06, 0x38, 0xB8,
		  0xE5, 0xC0, 0x26, 0x93, 0x0C, 0x3E, 0x60, 0x39,
		  0xA3, 0x3C, 0xE4, 0x59, 0x64, 0xFF, 0x21, 0x67,
		  0xF6, 0xEC, 0xED, 0xD4, 0x19, 0xDB, 0x06, 0xC1 },
		{ 0xCD, 0xC7, 0x6E, 0x5C, 0x99, 0x14, 0xFB, 0x92,
		  0x81, 0xA1, 0xC7, 0xE2, 0x84, 0xD7, 0x3E, 0x67,
		  0xF1, 0x80, 0x9A, 0x48, 0xA4, 0x97, 0x20, 0x0E,
		  0x04, 0x6D, 0x39, 0xCC, 0xC7, 0x11, 0x2C, 0xD0 },
	};

	if (type == npcm_sha_type_sha1) {
		/*--------------------------------------------------------------------*/
		/* SHA 1 TESTS                                                        */
		/*--------------------------------------------------------------------*/
		for (i = 0; i < SHA1_NUM_OF_SELF_TESTS; i++) {
			if (i != 3) {
				status = npcm_sha_calc(npcm_sha_type_sha1, sha1selftestbuff[i], sha1selftestbufflen[i], hashdigest);
				npcm_sha_check(status);
			} else {
				SHA_Power(true);
				SHA_Reset();
				status = SHA_Start(&handle, npcm_sha_type_sha1);
				npcm_sha_check(status);
				status = SHA_Update(&handle, sha1selftestbuff[i], 73);
				npcm_sha_check(status);
				status = SHA_Update(&handle, &sha1selftestbuff[i][73], sha1selftestbufflen[i] - 73);
				npcm_sha_check(status);
				status = SHA_Finish(&handle, hashdigest);
				npcm_sha_check(status);
				SHA_Power(false);
			}

			if (memcmp(hashdigest, sha1selftestexpres[i], SHA_1_HASH_LENGTH))
				return -1;
		}

	} else {
		/*--------------------------------------------------------------------*/
		/* SHA 2 TESTS                                                        */
		/*--------------------------------------------------------------------*/
		for (i = 0; i < SHA2_NUM_OF_SELF_TESTS; i++) {
			SHA_Power(true);
			SHA_Reset();
			status = SHA_Start(&handle, npcm_sha_type_sha2);
			npcm_sha_check(status);
			if (i == 2) {
				for (j = 0; j < 10000; j++) { //not working
					status = SHA_Update(&handle, sha2selftestbuff[i], sha2selftestbufflen[i]);
					npcm_sha_check(status);
				}
			} else {
				status = SHA_Update(&handle, sha2selftestbuff[i], sha2selftestbufflen[i]);
				npcm_sha_check(status);
			}

			status = SHA_Finish(&handle, hashdigest);
			npcm_sha_check(status);
			SHA_Power(false);
			if (memcmp(hashdigest, sha2selftestexpres[i], SHA_2_HASH_LENGTH))
				return -1;

			npcm_sha_calc(npcm_sha_type_sha2, sha2selftestbuff[i], sha2selftestbufflen[i], hashdigest);
			if (memcmp(hashdigest, sha2selftestexpres[i], SHA_2_HASH_LENGTH))
				return -1;
		}
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_FlushLocalBuffer_l                                    */
/*                                                                            */
/* Parameters:                                                                */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:     This routine flush secrun buffer to SHA module            */
/*----------------------------------------------------------------------------*/
static void SHA_FlushLocalBuffer_l(const u32 *buff)
{
	struct npcm_sha_regs *regs = sha_priv->regs;
	u32 i;

	for (i = 0; i < (SHA_BLOCK_LENGTH / sizeof(u32)); i++)
		writel(buff[i], &regs->hash_data_in);
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_BusyWait_l                                            */
/*                                                                            */
/* Parameters:                                                                */
/* Returns:         0 if no error was found or DEFS_STATUS_ERROR otherwise    */
/* Side effects:                                                              */
/* Description:     This routine wait for SHA unit to no longer be busy       */
/*----------------------------------------------------------------------------*/
static int SHA_BusyWait_l(void)
{
	struct npcm_sha_regs *regs = sha_priv->regs;
	u32 timeout = SHA_TIMEOUT;

	do {
		if (timeout-- == 0)
			return -ETIMEDOUT;
	} while ((readb(&regs->hash_ctr_sts) & HASH_CTR_STS_SHA_BUSY)
						== HASH_CTR_STS_SHA_BUSY);

	return 0;
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_GetShaDigest_l                                        */
/*                                                                            */
/* Parameters:      hashDigest - buffer for the hash output.                  */
/*                  type - SHA module type                                    */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:     This routine copy the hash digest from the hardware       */
/*                  and into given buffer (in ram)                            */
/*----------------------------------------------------------------------------*/
static void SHA_GetShaDigest_l(u8 *hashdigest, u8 type)
{
	struct npcm_sha_regs *regs = sha_priv->regs;
	u16 j;
	u8 len = SHA_HASH_LENGTH(type) / sizeof(u32);

	// Copy Bytes from SHA module to given buffer
	for (j = 0; j < len; j++)
		((u32 *)hashdigest)[j] = readl(&regs->hash_dig[j]);
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_SetShaDigest_l                                        */
/*                                                                            */
/* Parameters:      hashDigest - input buffer to set as hash digest           */
/*                  type - SHA module type                                    */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:     This routine set the hash digest in the hardware from     */
/*                  a given buffer (in ram)                                   */
/*----------------------------------------------------------------------------*/
static void SHA_SetShaDigest_l(const u32 *hashdigest, u8 type)
{
	struct npcm_sha_regs *regs = sha_priv->regs;
	u16 j;
	u8 len = SHA_HASH_LENGTH(type) / sizeof(u32);

	// Copy Bytes from given buffer to SHA module
	for (j = 0; j < len; j++)
		writel(hashdigest[j], &regs->hash_dig[j]);
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_SetBlock_l                                            */
/*                                                                            */
/* Parameters:      data        - data to copy                                */
/*                  len         - size of data                                */
/*                  position    - byte offset into the block at which data    */
/*                                should be placed                            */
/*                  block       - block buffer                                */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:     This routine load bytes into block buffer                 */
/*----------------------------------------------------------------------------*/
static void SHA_SetBlock_l(const u8 *data, u32 len, u16 position, u32 *block)
{
	u8 *dest = (u8 *)block;

	memcpy(dest + position, data, len);
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_SetBlock_l                                            */
/*                                                                            */
/* Parameters:                                                                */
/*                  len - size of data                                        */
/*                  position - byte offset into the block at which data       */
/*                             should be placed                               */
/*                  block - block buffer                                      */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:     This routine load zero's into the block buffer            */
/*----------------------------------------------------------------------------*/
static void SHA_ClearBlock_l(u16 len, u16 position, u32 *block)
{
	u8 *dest = (u8 *)block;

	memset(dest + position, 0, len);
}

/*----------------------------------------------------------------------------*/
/* Function:        SHA_SetLength32_l                                         */
/*                                                                            */
/* Parameters:                                                                */
/*                  handlePtr  -   SHA processing handle pointer              */
/*                  block - block buffer                                      */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:     This routine set the length of the hash's data            */
/*                  len is the 32-bit byte length of the message              */
/*lint -efunc(734,SHA_SetLength32_l) Supperess loss of percision lint warning */
/*----------------------------------------------------------------------------*/
static void SHA_SetLength32_l(struct SHA_HANDLE_T *handleptr, u32 *block)
{
	u16 *secrunbufferswappedptr = (u16 *)(void *)(block);

	secrunbufferswappedptr[(SHA_BLOCK_LENGTH / sizeof(u16)) - 1] = (u16)
	((handleptr->length0 << 3) << 8) | ((u16)(handleptr->length0 << 3) >> 8);
	secrunbufferswappedptr[(SHA_BLOCK_LENGTH / sizeof(u16)) - 2] = (u16)
	((handleptr->length0 >> (16 - 3)) >> 8) | ((u16)(handleptr->length0 >> (16 - 3)) << 8);
	secrunbufferswappedptr[(SHA_BLOCK_LENGTH / sizeof(u16)) - 3] = (u16)
	((handleptr->length1 << 3) << 8) | ((u16)(handleptr->length1 << 3) >> 8);
	secrunbufferswappedptr[(SHA_BLOCK_LENGTH / sizeof(u16)) - 4] = (u16)
	((handleptr->length1 >> (16 - 3)) >> 8) | ((u16)(handleptr->length1 >> (16 - 3)) << 8);
}

static int npcm_sha_bind(struct udevice *dev)
{
	sha_priv = calloc(1, sizeof(struct npcm_sha_priv));
	if (!sha_priv)
		return -ENOMEM;

	sha_priv->regs = dev_remap_addr_index(dev, 0);
	if (!sha_priv->regs) {
		printf("Cannot find sha reg address, binding failed\n");
		return -EINVAL;
	}

	printf("SHA: NPCM SHA module bind OK\n");

	return 0;
}

static const struct udevice_id npcm_sha_ids[] = {
	{ .compatible = "nuvoton,npcm845-sha" },
	{ .compatible = "nuvoton,npcm750-sha" },
	{ }
};

U_BOOT_DRIVER(npcm_sha) = {
	.name = "npcm_sha",
	.id = UCLASS_MISC,
	.of_match = npcm_sha_ids,
	.priv_auto = sizeof(struct npcm_sha_priv),
	.bind = npcm_sha_bind,
};
