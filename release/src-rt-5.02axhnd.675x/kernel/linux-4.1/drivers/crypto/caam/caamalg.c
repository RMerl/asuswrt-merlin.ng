/*
 * caam - Freescale FSL CAAM support for crypto API
 *
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 *
 * Based on talitos crypto API driver.
 *
 * relationship of job descriptors to shared descriptors (SteveC Dec 10 2008):
 *
 * ---------------                     ---------------
 * | JobDesc #1  |-------------------->|  ShareDesc  |
 * | *(packet 1) |                     |   (PDB)     |
 * ---------------      |------------->|  (hashKey)  |
 *       .              |              | (cipherKey) |
 *       .              |    |-------->| (operation) |
 * ---------------      |    |         ---------------
 * | JobDesc #2  |------|    |
 * | *(packet 2) |           |
 * ---------------           |
 *       .                   |
 *       .                   |
 * ---------------           |
 * | JobDesc #3  |------------
 * | *(packet 3) |
 * ---------------
 *
 * The SharedDesc never changes for a connection unless rekeyed, but
 * each packet will likely be in a different place. So all we need
 * to know to process the packet is where the input is, where the
 * output goes, and what context we want to process with. Context is
 * in the SharedDesc, packet references in the JobDesc.
 *
 * So, a job desc looks like:
 *
 * ---------------------
 * | Header            |
 * | ShareDesc Pointer |
 * | SEQ_OUT_PTR       |
 * | (output buffer)   |
 * | (output length)   |
 * | SEQ_IN_PTR        |
 * | (input buffer)    |
 * | (input length)    |
 * ---------------------
 */

#include "compat.h"

#include "regs.h"
#include "intern.h"
#include "desc_constr.h"
#include "jr.h"
#include "error.h"
#include "sg_sw_sec4.h"
#include "key_gen.h"

/*
 * crypto alg
 */
#define CAAM_CRA_PRIORITY		3000
/* max key is sum of AES_MAX_KEY_SIZE, max split key size */
#define CAAM_MAX_KEY_SIZE		(AES_MAX_KEY_SIZE + \
					 CTR_RFC3686_NONCE_SIZE + \
					 SHA512_DIGEST_SIZE * 2)
/* max IV is max of AES_BLOCK_SIZE, DES3_EDE_BLOCK_SIZE */
#define CAAM_MAX_IV_LENGTH		16

/* length of descriptors text */
#define DESC_AEAD_BASE			(4 * CAAM_CMD_SZ)
#define DESC_AEAD_ENC_LEN		(DESC_AEAD_BASE + 15 * CAAM_CMD_SZ)
#define DESC_AEAD_DEC_LEN		(DESC_AEAD_BASE + 18 * CAAM_CMD_SZ)
#define DESC_AEAD_GIVENC_LEN		(DESC_AEAD_ENC_LEN + 7 * CAAM_CMD_SZ)

/* Note: Nonce is counted in enckeylen */
#define DESC_AEAD_CTR_RFC3686_LEN	(6 * CAAM_CMD_SZ)

#define DESC_AEAD_NULL_BASE		(3 * CAAM_CMD_SZ)
#define DESC_AEAD_NULL_ENC_LEN		(DESC_AEAD_NULL_BASE + 14 * CAAM_CMD_SZ)
#define DESC_AEAD_NULL_DEC_LEN		(DESC_AEAD_NULL_BASE + 17 * CAAM_CMD_SZ)

#define DESC_GCM_BASE			(3 * CAAM_CMD_SZ)
#define DESC_GCM_ENC_LEN		(DESC_GCM_BASE + 23 * CAAM_CMD_SZ)
#define DESC_GCM_DEC_LEN		(DESC_GCM_BASE + 19 * CAAM_CMD_SZ)

#define DESC_RFC4106_BASE		(3 * CAAM_CMD_SZ)
#define DESC_RFC4106_ENC_LEN		(DESC_RFC4106_BASE + 15 * CAAM_CMD_SZ)
#define DESC_RFC4106_DEC_LEN		(DESC_RFC4106_BASE + 14 * CAAM_CMD_SZ)
#define DESC_RFC4106_GIVENC_LEN		(DESC_RFC4106_BASE + 21 * CAAM_CMD_SZ)

#define DESC_RFC4543_BASE		(3 * CAAM_CMD_SZ)
#define DESC_RFC4543_ENC_LEN		(DESC_RFC4543_BASE + 25 * CAAM_CMD_SZ)
#define DESC_RFC4543_DEC_LEN		(DESC_RFC4543_BASE + 27 * CAAM_CMD_SZ)
#define DESC_RFC4543_GIVENC_LEN		(DESC_RFC4543_BASE + 30 * CAAM_CMD_SZ)

#define DESC_ABLKCIPHER_BASE		(3 * CAAM_CMD_SZ)
#define DESC_ABLKCIPHER_ENC_LEN		(DESC_ABLKCIPHER_BASE + \
					 20 * CAAM_CMD_SZ)
#define DESC_ABLKCIPHER_DEC_LEN		(DESC_ABLKCIPHER_BASE + \
					 15 * CAAM_CMD_SZ)

#define DESC_MAX_USED_BYTES		(DESC_RFC4543_GIVENC_LEN + \
					 CAAM_MAX_KEY_SIZE)
#define DESC_MAX_USED_LEN		(DESC_MAX_USED_BYTES / CAAM_CMD_SZ)

#ifdef DEBUG
/* for print_hex_dumps with line references */
#define debug(format, arg...) printk(format, arg)
#else
#define debug(format, arg...)
#endif
static struct list_head alg_list;

/* Set DK bit in class 1 operation if shared */
static inline void append_dec_op1(u32 *desc, u32 type)
{
	u32 *jump_cmd, *uncond_jump_cmd;

	/* DK bit is valid only for AES */
	if ((type & OP_ALG_ALGSEL_MASK) != OP_ALG_ALGSEL_AES) {
		append_operation(desc, type | OP_ALG_AS_INITFINAL |
				 OP_ALG_DECRYPT);
		return;
	}

	jump_cmd = append_jump(desc, JUMP_TEST_ALL | JUMP_COND_SHRD);
	append_operation(desc, type | OP_ALG_AS_INITFINAL |
			 OP_ALG_DECRYPT);
	uncond_jump_cmd = append_jump(desc, JUMP_TEST_ALL);
	set_jump_tgt_here(desc, jump_cmd);
	append_operation(desc, type | OP_ALG_AS_INITFINAL |
			 OP_ALG_DECRYPT | OP_ALG_AAI_DK);
	set_jump_tgt_here(desc, uncond_jump_cmd);
}

/*
 * For aead functions, read payload and write payload,
 * both of which are specified in req->src and req->dst
 */
static inline void aead_append_src_dst(u32 *desc, u32 msg_type)
{
	append_seq_fifo_store(desc, 0, FIFOST_TYPE_MESSAGE_DATA | KEY_VLF);
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_BOTH |
			     KEY_VLF | msg_type | FIFOLD_TYPE_LASTBOTH);
}

/*
 * For aead encrypt and decrypt, read iv for both classes
 */
static inline void aead_append_ld_iv(u32 *desc, int ivsize, int ivoffset)
{
	append_seq_load(desc, ivsize, LDST_CLASS_1_CCB |
			LDST_SRCDST_BYTE_CONTEXT |
			(ivoffset << LDST_OFFSET_SHIFT));
	append_move(desc, MOVE_SRC_CLASS1CTX | MOVE_DEST_CLASS2INFIFO |
		    (ivoffset << MOVE_OFFSET_SHIFT) | ivsize);
}

/*
 * For ablkcipher encrypt and decrypt, read from req->src and
 * write to req->dst
 */
static inline void ablkcipher_append_src_dst(u32 *desc)
{
	append_math_add(desc, VARSEQOUTLEN, SEQINLEN, REG0, CAAM_CMD_SZ);
	append_math_add(desc, VARSEQINLEN, SEQINLEN, REG0, CAAM_CMD_SZ);
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 |
			     KEY_VLF | FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST1);
	append_seq_fifo_store(desc, 0, FIFOST_TYPE_MESSAGE_DATA | KEY_VLF);
}

/*
 * If all data, including src (with assoc and iv) or dst (with iv only) are
 * contiguous
 */
#define GIV_SRC_CONTIG		1
#define GIV_DST_CONTIG		(1 << 1)

/*
 * per-session context
 */
struct caam_ctx {
	struct device *jrdev;
	u32 sh_desc_enc[DESC_MAX_USED_LEN];
	u32 sh_desc_dec[DESC_MAX_USED_LEN];
	u32 sh_desc_givenc[DESC_MAX_USED_LEN];
	dma_addr_t sh_desc_enc_dma;
	dma_addr_t sh_desc_dec_dma;
	dma_addr_t sh_desc_givenc_dma;
	u32 class1_alg_type;
	u32 class2_alg_type;
	u32 alg_op;
	u8 key[CAAM_MAX_KEY_SIZE];
	dma_addr_t key_dma;
	unsigned int enckeylen;
	unsigned int split_key_len;
	unsigned int split_key_pad_len;
	unsigned int authsize;
};

static void append_key_aead(u32 *desc, struct caam_ctx *ctx,
			    int keys_fit_inline, bool is_rfc3686)
{
	u32 *nonce;
	unsigned int enckeylen = ctx->enckeylen;

	/*
	 * RFC3686 specific:
	 *	| ctx->key = {AUTH_KEY, ENC_KEY, NONCE}
	 *	| enckeylen = encryption key size + nonce size
	 */
	if (is_rfc3686)
		enckeylen -= CTR_RFC3686_NONCE_SIZE;

	if (keys_fit_inline) {
		append_key_as_imm(desc, ctx->key, ctx->split_key_pad_len,
				  ctx->split_key_len, CLASS_2 |
				  KEY_DEST_MDHA_SPLIT | KEY_ENC);
		append_key_as_imm(desc, (void *)ctx->key +
				  ctx->split_key_pad_len, enckeylen,
				  enckeylen, CLASS_1 | KEY_DEST_CLASS_REG);
	} else {
		append_key(desc, ctx->key_dma, ctx->split_key_len, CLASS_2 |
			   KEY_DEST_MDHA_SPLIT | KEY_ENC);
		append_key(desc, ctx->key_dma + ctx->split_key_pad_len,
			   enckeylen, CLASS_1 | KEY_DEST_CLASS_REG);
	}

	/* Load Counter into CONTEXT1 reg */
	if (is_rfc3686) {
		nonce = (u32 *)((void *)ctx->key + ctx->split_key_pad_len +
			       enckeylen);
		append_load_imm_u32(desc, *nonce, LDST_CLASS_IND_CCB |
				    LDST_SRCDST_BYTE_OUTFIFO | LDST_IMM);
		append_move(desc,
			    MOVE_SRC_OUTFIFO |
			    MOVE_DEST_CLASS1CTX |
			    (16 << MOVE_OFFSET_SHIFT) |
			    (CTR_RFC3686_NONCE_SIZE << MOVE_LEN_SHIFT));
	}
}

static void init_sh_desc_key_aead(u32 *desc, struct caam_ctx *ctx,
				  int keys_fit_inline, bool is_rfc3686)
{
	u32 *key_jump_cmd;

	/* Note: Context registers are saved. */
	init_sh_desc(desc, HDR_SHARE_SERIAL | HDR_SAVECTX);

	/* Skip if already shared */
	key_jump_cmd = append_jump(desc, JUMP_JSL | JUMP_TEST_ALL |
				   JUMP_COND_SHRD);

	append_key_aead(desc, ctx, keys_fit_inline, is_rfc3686);

	set_jump_tgt_here(desc, key_jump_cmd);
}

static int aead_null_set_sh_desc(struct crypto_aead *aead)
{
	struct aead_tfm *tfm = &aead->base.crt_aead;
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	bool keys_fit_inline = false;
	u32 *key_jump_cmd, *jump_cmd, *read_move_cmd, *write_move_cmd;
	u32 *desc;

	/*
	 * Job Descriptor and Shared Descriptors
	 * must all fit into the 64-word Descriptor h/w Buffer
	 */
	if (DESC_AEAD_NULL_ENC_LEN + DESC_JOB_IO_LEN +
	    ctx->split_key_pad_len <= CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	/* aead_encrypt shared descriptor */
	desc = ctx->sh_desc_enc;

	init_sh_desc(desc, HDR_SHARE_SERIAL);

	/* Skip if already shared */
	key_jump_cmd = append_jump(desc, JUMP_JSL | JUMP_TEST_ALL |
				   JUMP_COND_SHRD);
	if (keys_fit_inline)
		append_key_as_imm(desc, ctx->key, ctx->split_key_pad_len,
				  ctx->split_key_len, CLASS_2 |
				  KEY_DEST_MDHA_SPLIT | KEY_ENC);
	else
		append_key(desc, ctx->key_dma, ctx->split_key_len, CLASS_2 |
			   KEY_DEST_MDHA_SPLIT | KEY_ENC);
	set_jump_tgt_here(desc, key_jump_cmd);

	/* cryptlen = seqoutlen - authsize */
	append_math_sub_imm_u32(desc, REG3, SEQOUTLEN, IMM, ctx->authsize);

	/*
	 * NULL encryption; IV is zero
	 * assoclen = (assoclen + cryptlen) - cryptlen
	 */
	append_math_sub(desc, VARSEQINLEN, SEQINLEN, REG3, CAAM_CMD_SZ);

	/* read assoc before reading payload */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS2 | FIFOLD_TYPE_MSG |
			     KEY_VLF);

	/* Prepare to read and write cryptlen bytes */
	append_math_add(desc, VARSEQINLEN, ZERO, REG3, CAAM_CMD_SZ);
	append_math_add(desc, VARSEQOUTLEN, ZERO, REG3, CAAM_CMD_SZ);

	/*
	 * MOVE_LEN opcode is not available in all SEC HW revisions,
	 * thus need to do some magic, i.e. self-patch the descriptor
	 * buffer.
	 */
	read_move_cmd = append_move(desc, MOVE_SRC_DESCBUF |
				    MOVE_DEST_MATH3 |
				    (0x6 << MOVE_LEN_SHIFT));
	write_move_cmd = append_move(desc, MOVE_SRC_MATH3 |
				     MOVE_DEST_DESCBUF |
				     MOVE_WAITCOMP |
				     (0x8 << MOVE_LEN_SHIFT));

	/* Class 2 operation */
	append_operation(desc, ctx->class2_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT);

	/* Read and write cryptlen bytes */
	aead_append_src_dst(desc, FIFOLD_TYPE_MSG | FIFOLD_TYPE_FLUSH1);

	set_move_tgt_here(desc, read_move_cmd);
	set_move_tgt_here(desc, write_move_cmd);
	append_cmd(desc, CMD_LOAD | DISABLE_AUTO_INFO_FIFO);
	append_move(desc, MOVE_SRC_INFIFO_CL | MOVE_DEST_OUTFIFO |
		    MOVE_AUX_LS);

	/* Write ICV */
	append_seq_store(desc, ctx->authsize, LDST_CLASS_2_CCB |
			 LDST_SRCDST_BYTE_CONTEXT);

	ctx->sh_desc_enc_dma = dma_map_single(jrdev, desc,
					      desc_bytes(desc),
					      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_enc_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR,
		       "aead null enc shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	/*
	 * Job Descriptor and Shared Descriptors
	 * must all fit into the 64-word Descriptor h/w Buffer
	 */
	keys_fit_inline = false;
	if (DESC_AEAD_NULL_DEC_LEN + DESC_JOB_IO_LEN +
	    ctx->split_key_pad_len <= CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	desc = ctx->sh_desc_dec;

	/* aead_decrypt shared descriptor */
	init_sh_desc(desc, HDR_SHARE_SERIAL);

	/* Skip if already shared */
	key_jump_cmd = append_jump(desc, JUMP_JSL | JUMP_TEST_ALL |
				   JUMP_COND_SHRD);
	if (keys_fit_inline)
		append_key_as_imm(desc, ctx->key, ctx->split_key_pad_len,
				  ctx->split_key_len, CLASS_2 |
				  KEY_DEST_MDHA_SPLIT | KEY_ENC);
	else
		append_key(desc, ctx->key_dma, ctx->split_key_len, CLASS_2 |
			   KEY_DEST_MDHA_SPLIT | KEY_ENC);
	set_jump_tgt_here(desc, key_jump_cmd);

	/* Class 2 operation */
	append_operation(desc, ctx->class2_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_DECRYPT | OP_ALG_ICV_ON);

	/* assoclen + cryptlen = seqinlen - ivsize - authsize */
	append_math_sub_imm_u32(desc, REG3, SEQINLEN, IMM,
				ctx->authsize + tfm->ivsize);
	/* assoclen = (assoclen + cryptlen) - cryptlen */
	append_math_sub(desc, REG2, SEQOUTLEN, REG0, CAAM_CMD_SZ);
	append_math_sub(desc, VARSEQINLEN, REG3, REG2, CAAM_CMD_SZ);

	/* read assoc before reading payload */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS2 | FIFOLD_TYPE_MSG |
			     KEY_VLF);

	/* Prepare to read and write cryptlen bytes */
	append_math_add(desc, VARSEQINLEN, ZERO, REG2, CAAM_CMD_SZ);
	append_math_add(desc, VARSEQOUTLEN, ZERO, REG2, CAAM_CMD_SZ);

	/*
	 * MOVE_LEN opcode is not available in all SEC HW revisions,
	 * thus need to do some magic, i.e. self-patch the descriptor
	 * buffer.
	 */
	read_move_cmd = append_move(desc, MOVE_SRC_DESCBUF |
				    MOVE_DEST_MATH2 |
				    (0x6 << MOVE_LEN_SHIFT));
	write_move_cmd = append_move(desc, MOVE_SRC_MATH2 |
				     MOVE_DEST_DESCBUF |
				     MOVE_WAITCOMP |
				     (0x8 << MOVE_LEN_SHIFT));

	/* Read and write cryptlen bytes */
	aead_append_src_dst(desc, FIFOLD_TYPE_MSG | FIFOLD_TYPE_FLUSH1);

	/*
	 * Insert a NOP here, since we need at least 4 instructions between
	 * code patching the descriptor buffer and the location being patched.
	 */
	jump_cmd = append_jump(desc, JUMP_TEST_ALL);
	set_jump_tgt_here(desc, jump_cmd);

	set_move_tgt_here(desc, read_move_cmd);
	set_move_tgt_here(desc, write_move_cmd);
	append_cmd(desc, CMD_LOAD | DISABLE_AUTO_INFO_FIFO);
	append_move(desc, MOVE_SRC_INFIFO_CL | MOVE_DEST_OUTFIFO |
		    MOVE_AUX_LS);
	append_cmd(desc, CMD_LOAD | ENABLE_AUTO_INFO_FIFO);

	/* Load ICV */
	append_seq_fifo_load(desc, ctx->authsize, FIFOLD_CLASS_CLASS2 |
			     FIFOLD_TYPE_LAST2 | FIFOLD_TYPE_ICV);

	ctx->sh_desc_dec_dma = dma_map_single(jrdev, desc,
					      desc_bytes(desc),
					      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_dec_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR,
		       "aead null dec shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	return 0;
}

static int aead_set_sh_desc(struct crypto_aead *aead)
{
	struct aead_tfm *tfm = &aead->base.crt_aead;
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct crypto_tfm *ctfm = crypto_aead_tfm(aead);
	const char *alg_name = crypto_tfm_alg_name(ctfm);
	struct device *jrdev = ctx->jrdev;
	bool keys_fit_inline;
	u32 geniv, moveiv;
	u32 ctx1_iv_off = 0;
	u32 *desc;
	const bool ctr_mode = ((ctx->class1_alg_type & OP_ALG_AAI_MASK) ==
			       OP_ALG_AAI_CTR_MOD128);
	const bool is_rfc3686 = (ctr_mode &&
				 (strstr(alg_name, "rfc3686") != NULL));

	if (!ctx->authsize)
		return 0;

	/* NULL encryption / decryption */
	if (!ctx->enckeylen)
		return aead_null_set_sh_desc(aead);

	/*
	 * AES-CTR needs to load IV in CONTEXT1 reg
	 * at an offset of 128bits (16bytes)
	 * CONTEXT1[255:128] = IV
	 */
	if (ctr_mode)
		ctx1_iv_off = 16;

	/*
	 * RFC3686 specific:
	 *	CONTEXT1[255:128] = {NONCE, IV, COUNTER}
	 */
	if (is_rfc3686)
		ctx1_iv_off = 16 + CTR_RFC3686_NONCE_SIZE;

	/*
	 * Job Descriptor and Shared Descriptors
	 * must all fit into the 64-word Descriptor h/w Buffer
	 */
	keys_fit_inline = false;
	if (DESC_AEAD_ENC_LEN + DESC_JOB_IO_LEN +
	    ctx->split_key_pad_len + ctx->enckeylen +
	    (is_rfc3686 ? DESC_AEAD_CTR_RFC3686_LEN : 0) <=
	    CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	/* aead_encrypt shared descriptor */
	desc = ctx->sh_desc_enc;

	/* Note: Context registers are saved. */
	init_sh_desc_key_aead(desc, ctx, keys_fit_inline, is_rfc3686);

	/* Class 2 operation */
	append_operation(desc, ctx->class2_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT);

	/* cryptlen = seqoutlen - authsize */
	append_math_sub_imm_u32(desc, REG3, SEQOUTLEN, IMM, ctx->authsize);

	/* assoclen + cryptlen = seqinlen - ivsize */
	append_math_sub_imm_u32(desc, REG2, SEQINLEN, IMM, tfm->ivsize);

	/* assoclen = (assoclen + cryptlen) - cryptlen */
	append_math_sub(desc, VARSEQINLEN, REG2, REG3, CAAM_CMD_SZ);

	/* read assoc before reading payload */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS2 | FIFOLD_TYPE_MSG |
			     KEY_VLF);
	aead_append_ld_iv(desc, tfm->ivsize, ctx1_iv_off);

	/* Load Counter into CONTEXT1 reg */
	if (is_rfc3686)
		append_load_imm_u32(desc, be32_to_cpu(1), LDST_IMM |
				    LDST_CLASS_1_CCB |
				    LDST_SRCDST_BYTE_CONTEXT |
				    ((ctx1_iv_off + CTR_RFC3686_IV_SIZE) <<
				     LDST_OFFSET_SHIFT));

	/* Class 1 operation */
	append_operation(desc, ctx->class1_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT);

	/* Read and write cryptlen bytes */
	append_math_add(desc, VARSEQINLEN, ZERO, REG3, CAAM_CMD_SZ);
	append_math_add(desc, VARSEQOUTLEN, ZERO, REG3, CAAM_CMD_SZ);
	aead_append_src_dst(desc, FIFOLD_TYPE_MSG1OUT2);

	/* Write ICV */
	append_seq_store(desc, ctx->authsize, LDST_CLASS_2_CCB |
			 LDST_SRCDST_BYTE_CONTEXT);

	ctx->sh_desc_enc_dma = dma_map_single(jrdev, desc,
					      desc_bytes(desc),
					      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_enc_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "aead enc shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	/*
	 * Job Descriptor and Shared Descriptors
	 * must all fit into the 64-word Descriptor h/w Buffer
	 */
	keys_fit_inline = false;
	if (DESC_AEAD_DEC_LEN + DESC_JOB_IO_LEN +
	    ctx->split_key_pad_len + ctx->enckeylen +
	    (is_rfc3686 ? DESC_AEAD_CTR_RFC3686_LEN : 0) <=
	    CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	/* aead_decrypt shared descriptor */
	desc = ctx->sh_desc_dec;

	/* Note: Context registers are saved. */
	init_sh_desc_key_aead(desc, ctx, keys_fit_inline, is_rfc3686);

	/* Class 2 operation */
	append_operation(desc, ctx->class2_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_DECRYPT | OP_ALG_ICV_ON);

	/* assoclen + cryptlen = seqinlen - ivsize - authsize */
	append_math_sub_imm_u32(desc, REG3, SEQINLEN, IMM,
				ctx->authsize + tfm->ivsize);
	/* assoclen = (assoclen + cryptlen) - cryptlen */
	append_math_sub(desc, REG2, SEQOUTLEN, REG0, CAAM_CMD_SZ);
	append_math_sub(desc, VARSEQINLEN, REG3, REG2, CAAM_CMD_SZ);

	/* read assoc before reading payload */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS2 | FIFOLD_TYPE_MSG |
			     KEY_VLF);

	aead_append_ld_iv(desc, tfm->ivsize, ctx1_iv_off);

	/* Load Counter into CONTEXT1 reg */
	if (is_rfc3686)
		append_load_imm_u32(desc, be32_to_cpu(1), LDST_IMM |
				    LDST_CLASS_1_CCB |
				    LDST_SRCDST_BYTE_CONTEXT |
				    ((ctx1_iv_off + CTR_RFC3686_IV_SIZE) <<
				     LDST_OFFSET_SHIFT));

	/* Choose operation */
	if (ctr_mode)
		append_operation(desc, ctx->class1_alg_type |
				 OP_ALG_AS_INITFINAL | OP_ALG_DECRYPT);
	else
		append_dec_op1(desc, ctx->class1_alg_type);

	/* Read and write cryptlen bytes */
	append_math_add(desc, VARSEQINLEN, ZERO, REG2, CAAM_CMD_SZ);
	append_math_add(desc, VARSEQOUTLEN, ZERO, REG2, CAAM_CMD_SZ);
	aead_append_src_dst(desc, FIFOLD_TYPE_MSG);

	/* Load ICV */
	append_seq_fifo_load(desc, ctx->authsize, FIFOLD_CLASS_CLASS2 |
			     FIFOLD_TYPE_LAST2 | FIFOLD_TYPE_ICV);

	ctx->sh_desc_dec_dma = dma_map_single(jrdev, desc,
					      desc_bytes(desc),
					      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_dec_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "aead dec shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	/*
	 * Job Descriptor and Shared Descriptors
	 * must all fit into the 64-word Descriptor h/w Buffer
	 */
	keys_fit_inline = false;
	if (DESC_AEAD_GIVENC_LEN + DESC_JOB_IO_LEN +
	    ctx->split_key_pad_len + ctx->enckeylen +
	    (is_rfc3686 ? DESC_AEAD_CTR_RFC3686_LEN : 0) <=
	    CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	/* aead_givencrypt shared descriptor */
	desc = ctx->sh_desc_givenc;

	/* Note: Context registers are saved. */
	init_sh_desc_key_aead(desc, ctx, keys_fit_inline, is_rfc3686);

	/* Generate IV */
	geniv = NFIFOENTRY_STYPE_PAD | NFIFOENTRY_DEST_DECO |
		NFIFOENTRY_DTYPE_MSG | NFIFOENTRY_LC1 |
		NFIFOENTRY_PTYPE_RND | (tfm->ivsize << NFIFOENTRY_DLEN_SHIFT);
	append_load_imm_u32(desc, geniv, LDST_CLASS_IND_CCB |
			    LDST_SRCDST_WORD_INFO_FIFO | LDST_IMM);
	append_cmd(desc, CMD_LOAD | DISABLE_AUTO_INFO_FIFO);
	append_move(desc, MOVE_WAITCOMP |
		    MOVE_SRC_INFIFO | MOVE_DEST_CLASS1CTX |
		    (ctx1_iv_off << MOVE_OFFSET_SHIFT) |
		    (tfm->ivsize << MOVE_LEN_SHIFT));
	append_cmd(desc, CMD_LOAD | ENABLE_AUTO_INFO_FIFO);

	/* Copy IV to class 1 context */
	append_move(desc, MOVE_SRC_CLASS1CTX | MOVE_DEST_OUTFIFO |
		    (ctx1_iv_off << MOVE_OFFSET_SHIFT) |
		    (tfm->ivsize << MOVE_LEN_SHIFT));

	/* Return to encryption */
	append_operation(desc, ctx->class2_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT);

	/* ivsize + cryptlen = seqoutlen - authsize */
	append_math_sub_imm_u32(desc, REG3, SEQOUTLEN, IMM, ctx->authsize);

	/* assoclen = seqinlen - (ivsize + cryptlen) */
	append_math_sub(desc, VARSEQINLEN, SEQINLEN, REG3, CAAM_CMD_SZ);

	/* read assoc before reading payload */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS2 | FIFOLD_TYPE_MSG |
			     KEY_VLF);

	/* Copy iv from outfifo to class 2 fifo */
	moveiv = NFIFOENTRY_STYPE_OFIFO | NFIFOENTRY_DEST_CLASS2 |
		 NFIFOENTRY_DTYPE_MSG | (tfm->ivsize << NFIFOENTRY_DLEN_SHIFT);
	append_load_imm_u32(desc, moveiv, LDST_CLASS_IND_CCB |
			    LDST_SRCDST_WORD_INFO_FIFO | LDST_IMM);
	append_load_imm_u32(desc, tfm->ivsize, LDST_CLASS_2_CCB |
			    LDST_SRCDST_WORD_DATASZ_REG | LDST_IMM);

	/* Load Counter into CONTEXT1 reg */
	if (is_rfc3686)
		append_load_imm_u32(desc, be32_to_cpu(1), LDST_IMM |
				    LDST_CLASS_1_CCB |
				    LDST_SRCDST_BYTE_CONTEXT |
				    ((ctx1_iv_off + CTR_RFC3686_IV_SIZE) <<
				     LDST_OFFSET_SHIFT));

	/* Class 1 operation */
	append_operation(desc, ctx->class1_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT);

	/* Will write ivsize + cryptlen */
	append_math_add(desc, VARSEQOUTLEN, SEQINLEN, REG0, CAAM_CMD_SZ);

	/* Not need to reload iv */
	append_seq_fifo_load(desc, tfm->ivsize,
			     FIFOLD_CLASS_SKIP);

	/* Will read cryptlen */
	append_math_add(desc, VARSEQINLEN, SEQINLEN, REG0, CAAM_CMD_SZ);
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_BOTH | KEY_VLF |
			     FIFOLD_TYPE_MSG1OUT2 | FIFOLD_TYPE_LASTBOTH);
	append_seq_fifo_store(desc, 0, FIFOST_TYPE_MESSAGE_DATA | KEY_VLF);

	/* Write ICV */
	append_seq_store(desc, ctx->authsize, LDST_CLASS_2_CCB |
			 LDST_SRCDST_BYTE_CONTEXT);

	ctx->sh_desc_givenc_dma = dma_map_single(jrdev, desc,
						 desc_bytes(desc),
						 DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_givenc_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "aead givenc shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	return 0;
}

static int aead_setauthsize(struct crypto_aead *authenc,
				    unsigned int authsize)
{
	struct caam_ctx *ctx = crypto_aead_ctx(authenc);

	ctx->authsize = authsize;
	aead_set_sh_desc(authenc);

	return 0;
}

static int gcm_set_sh_desc(struct crypto_aead *aead)
{
	struct aead_tfm *tfm = &aead->base.crt_aead;
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	bool keys_fit_inline = false;
	u32 *key_jump_cmd, *zero_payload_jump_cmd,
	    *zero_assoc_jump_cmd1, *zero_assoc_jump_cmd2;
	u32 *desc;

	if (!ctx->enckeylen || !ctx->authsize)
		return 0;

	/*
	 * AES GCM encrypt shared descriptor
	 * Job Descriptor and Shared Descriptor
	 * must fit into the 64-word Descriptor h/w Buffer
	 */
	if (DESC_GCM_ENC_LEN + DESC_JOB_IO_LEN +
	    ctx->enckeylen <= CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	desc = ctx->sh_desc_enc;

	init_sh_desc(desc, HDR_SHARE_SERIAL);

	/* skip key loading if they are loaded due to sharing */
	key_jump_cmd = append_jump(desc, JUMP_JSL | JUMP_TEST_ALL |
				   JUMP_COND_SHRD | JUMP_COND_SELF);
	if (keys_fit_inline)
		append_key_as_imm(desc, (void *)ctx->key, ctx->enckeylen,
				  ctx->enckeylen, CLASS_1 | KEY_DEST_CLASS_REG);
	else
		append_key(desc, ctx->key_dma, ctx->enckeylen,
			   CLASS_1 | KEY_DEST_CLASS_REG);
	set_jump_tgt_here(desc, key_jump_cmd);

	/* class 1 operation */
	append_operation(desc, ctx->class1_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT);

	/* cryptlen = seqoutlen - authsize */
	append_math_sub_imm_u32(desc, REG3, SEQOUTLEN, IMM, ctx->authsize);

	/* assoclen + cryptlen = seqinlen - ivsize */
	append_math_sub_imm_u32(desc, REG2, SEQINLEN, IMM, tfm->ivsize);

	/* assoclen = (assoclen + cryptlen) - cryptlen */
	append_math_sub(desc, REG1, REG2, REG3, CAAM_CMD_SZ);

	/* if cryptlen is ZERO jump to zero-payload commands */
	append_math_add(desc, VARSEQOUTLEN, ZERO, REG3, CAAM_CMD_SZ);
	zero_payload_jump_cmd = append_jump(desc, JUMP_TEST_ALL |
					    JUMP_COND_MATH_Z);
	/* read IV */
	append_seq_fifo_load(desc, tfm->ivsize, FIFOLD_CLASS_CLASS1 |
			     FIFOLD_TYPE_IV | FIFOLD_TYPE_FLUSH1);

	/* if assoclen is ZERO, skip reading the assoc data */
	append_math_add(desc, VARSEQINLEN, ZERO, REG1, CAAM_CMD_SZ);
	zero_assoc_jump_cmd1 = append_jump(desc, JUMP_TEST_ALL |
					   JUMP_COND_MATH_Z);

	/* read assoc data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_AAD | FIFOLD_TYPE_FLUSH1);
	set_jump_tgt_here(desc, zero_assoc_jump_cmd1);

	append_math_add(desc, VARSEQINLEN, ZERO, REG3, CAAM_CMD_SZ);

	/* write encrypted data */
	append_seq_fifo_store(desc, 0, FIFOST_TYPE_MESSAGE_DATA | FIFOLDST_VLF);

	/* read payload data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST1);

	/* jump the zero-payload commands */
	append_jump(desc, JUMP_TEST_ALL | 7);

	/* zero-payload commands */
	set_jump_tgt_here(desc, zero_payload_jump_cmd);

	/* if assoclen is ZERO, jump to IV reading - is the only input data */
	append_math_add(desc, VARSEQINLEN, ZERO, REG1, CAAM_CMD_SZ);
	zero_assoc_jump_cmd2 = append_jump(desc, JUMP_TEST_ALL |
					   JUMP_COND_MATH_Z);
	/* read IV */
	append_seq_fifo_load(desc, tfm->ivsize, FIFOLD_CLASS_CLASS1 |
			     FIFOLD_TYPE_IV | FIFOLD_TYPE_FLUSH1);

	/* read assoc data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_AAD | FIFOLD_TYPE_LAST1);

	/* jump to ICV writing */
	append_jump(desc, JUMP_TEST_ALL | 2);

	/* read IV - is the only input data */
	set_jump_tgt_here(desc, zero_assoc_jump_cmd2);
	append_seq_fifo_load(desc, tfm->ivsize, FIFOLD_CLASS_CLASS1 |
			     FIFOLD_TYPE_IV | FIFOLD_TYPE_FLUSH1 |
			     FIFOLD_TYPE_LAST1);

	/* write ICV */
	append_seq_store(desc, ctx->authsize, LDST_CLASS_1_CCB |
			 LDST_SRCDST_BYTE_CONTEXT);

	ctx->sh_desc_enc_dma = dma_map_single(jrdev, desc,
					      desc_bytes(desc),
					      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_enc_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "gcm enc shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	/*
	 * Job Descriptor and Shared Descriptors
	 * must all fit into the 64-word Descriptor h/w Buffer
	 */
	keys_fit_inline = false;
	if (DESC_GCM_DEC_LEN + DESC_JOB_IO_LEN +
	    ctx->enckeylen <= CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	desc = ctx->sh_desc_dec;

	init_sh_desc(desc, HDR_SHARE_SERIAL);

	/* skip key loading if they are loaded due to sharing */
	key_jump_cmd = append_jump(desc, JUMP_JSL |
				   JUMP_TEST_ALL | JUMP_COND_SHRD |
				   JUMP_COND_SELF);
	if (keys_fit_inline)
		append_key_as_imm(desc, (void *)ctx->key, ctx->enckeylen,
				  ctx->enckeylen, CLASS_1 | KEY_DEST_CLASS_REG);
	else
		append_key(desc, ctx->key_dma, ctx->enckeylen,
			   CLASS_1 | KEY_DEST_CLASS_REG);
	set_jump_tgt_here(desc, key_jump_cmd);

	/* class 1 operation */
	append_operation(desc, ctx->class1_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_DECRYPT | OP_ALG_ICV_ON);

	/* assoclen + cryptlen = seqinlen - ivsize - icvsize */
	append_math_sub_imm_u32(desc, REG3, SEQINLEN, IMM,
				ctx->authsize + tfm->ivsize);

	/* assoclen = (assoclen + cryptlen) - cryptlen */
	append_math_sub(desc, REG2, SEQOUTLEN, REG0, CAAM_CMD_SZ);
	append_math_sub(desc, REG1, REG3, REG2, CAAM_CMD_SZ);

	/* read IV */
	append_seq_fifo_load(desc, tfm->ivsize, FIFOLD_CLASS_CLASS1 |
			     FIFOLD_TYPE_IV | FIFOLD_TYPE_FLUSH1);

	/* jump to zero-payload command if cryptlen is zero */
	append_math_add(desc, VARSEQOUTLEN, ZERO, REG2, CAAM_CMD_SZ);
	zero_payload_jump_cmd = append_jump(desc, JUMP_TEST_ALL |
					    JUMP_COND_MATH_Z);

	append_math_add(desc, VARSEQINLEN, ZERO, REG1, CAAM_CMD_SZ);
	/* if asoclen is ZERO, skip reading assoc data */
	zero_assoc_jump_cmd1 = append_jump(desc, JUMP_TEST_ALL |
					   JUMP_COND_MATH_Z);
	/* read assoc data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_AAD | FIFOLD_TYPE_FLUSH1);
	set_jump_tgt_here(desc, zero_assoc_jump_cmd1);

	append_math_add(desc, VARSEQINLEN, ZERO, REG2, CAAM_CMD_SZ);

	/* store encrypted data */
	append_seq_fifo_store(desc, 0, FIFOST_TYPE_MESSAGE_DATA | FIFOLDST_VLF);

	/* read payload data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_MSG | FIFOLD_TYPE_FLUSH1);

	/* jump the zero-payload commands */
	append_jump(desc, JUMP_TEST_ALL | 4);

	/* zero-payload command */
	set_jump_tgt_here(desc, zero_payload_jump_cmd);

	/* if assoclen is ZERO, jump to ICV reading */
	append_math_add(desc, VARSEQINLEN, ZERO, REG1, CAAM_CMD_SZ);
	zero_assoc_jump_cmd2 = append_jump(desc, JUMP_TEST_ALL |
					   JUMP_COND_MATH_Z);
	/* read assoc data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_AAD | FIFOLD_TYPE_FLUSH1);
	set_jump_tgt_here(desc, zero_assoc_jump_cmd2);

	/* read ICV */
	append_seq_fifo_load(desc, ctx->authsize, FIFOLD_CLASS_CLASS1 |
			     FIFOLD_TYPE_ICV | FIFOLD_TYPE_LAST1);

	ctx->sh_desc_dec_dma = dma_map_single(jrdev, desc,
					      desc_bytes(desc),
					      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_dec_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "gcm dec shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	return 0;
}

static int gcm_setauthsize(struct crypto_aead *authenc, unsigned int authsize)
{
	struct caam_ctx *ctx = crypto_aead_ctx(authenc);

	ctx->authsize = authsize;
	gcm_set_sh_desc(authenc);

	return 0;
}

static int rfc4106_set_sh_desc(struct crypto_aead *aead)
{
	struct aead_tfm *tfm = &aead->base.crt_aead;
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	bool keys_fit_inline = false;
	u32 *key_jump_cmd, *move_cmd, *write_iv_cmd;
	u32 *desc;
	u32 geniv;

	if (!ctx->enckeylen || !ctx->authsize)
		return 0;

	/*
	 * RFC4106 encrypt shared descriptor
	 * Job Descriptor and Shared Descriptor
	 * must fit into the 64-word Descriptor h/w Buffer
	 */
	if (DESC_RFC4106_ENC_LEN + DESC_JOB_IO_LEN +
	    ctx->enckeylen <= CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	desc = ctx->sh_desc_enc;

	init_sh_desc(desc, HDR_SHARE_SERIAL);

	/* Skip key loading if it is loaded due to sharing */
	key_jump_cmd = append_jump(desc, JUMP_JSL | JUMP_TEST_ALL |
				   JUMP_COND_SHRD);
	if (keys_fit_inline)
		append_key_as_imm(desc, (void *)ctx->key, ctx->enckeylen,
				  ctx->enckeylen, CLASS_1 | KEY_DEST_CLASS_REG);
	else
		append_key(desc, ctx->key_dma, ctx->enckeylen,
			   CLASS_1 | KEY_DEST_CLASS_REG);
	set_jump_tgt_here(desc, key_jump_cmd);

	/* Class 1 operation */
	append_operation(desc, ctx->class1_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT);

	/* cryptlen = seqoutlen - authsize */
	append_math_sub_imm_u32(desc, REG3, SEQOUTLEN, IMM, ctx->authsize);
	append_math_add(desc, VARSEQOUTLEN, ZERO, REG3, CAAM_CMD_SZ);

	/* assoclen + cryptlen = seqinlen - ivsize */
	append_math_sub_imm_u32(desc, REG2, SEQINLEN, IMM, tfm->ivsize);

	/* assoclen = (assoclen + cryptlen) - cryptlen */
	append_math_sub(desc, VARSEQINLEN, REG2, REG3, CAAM_CMD_SZ);

	/* Read Salt */
	append_fifo_load_as_imm(desc, (void *)(ctx->key + ctx->enckeylen),
				4, FIFOLD_CLASS_CLASS1 | FIFOLD_TYPE_IV);
	/* Read AES-GCM-ESP IV */
	append_seq_fifo_load(desc, tfm->ivsize, FIFOLD_CLASS_CLASS1 |
			     FIFOLD_TYPE_IV | FIFOLD_TYPE_FLUSH1);

	/* Read assoc data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_AAD | FIFOLD_TYPE_FLUSH1);

	/* Will read cryptlen bytes */
	append_math_add(desc, VARSEQINLEN, ZERO, REG3, CAAM_CMD_SZ);

	/* Write encrypted data */
	append_seq_fifo_store(desc, 0, FIFOST_TYPE_MESSAGE_DATA | FIFOLDST_VLF);

	/* Read payload data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST1);

	/* Write ICV */
	append_seq_store(desc, ctx->authsize, LDST_CLASS_1_CCB |
			 LDST_SRCDST_BYTE_CONTEXT);

	ctx->sh_desc_enc_dma = dma_map_single(jrdev, desc,
					      desc_bytes(desc),
					      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_enc_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "rfc4106 enc shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	/*
	 * Job Descriptor and Shared Descriptors
	 * must all fit into the 64-word Descriptor h/w Buffer
	 */
	keys_fit_inline = false;
	if (DESC_RFC4106_DEC_LEN + DESC_JOB_IO_LEN +
	    ctx->enckeylen <= CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	desc = ctx->sh_desc_dec;

	init_sh_desc(desc, HDR_SHARE_SERIAL);

	/* Skip key loading if it is loaded due to sharing */
	key_jump_cmd = append_jump(desc, JUMP_JSL |
				   JUMP_TEST_ALL | JUMP_COND_SHRD);
	if (keys_fit_inline)
		append_key_as_imm(desc, (void *)ctx->key, ctx->enckeylen,
				  ctx->enckeylen, CLASS_1 | KEY_DEST_CLASS_REG);
	else
		append_key(desc, ctx->key_dma, ctx->enckeylen,
			   CLASS_1 | KEY_DEST_CLASS_REG);
	set_jump_tgt_here(desc, key_jump_cmd);

	/* Class 1 operation */
	append_operation(desc, ctx->class1_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_DECRYPT | OP_ALG_ICV_ON);

	/* assoclen + cryptlen = seqinlen - ivsize - icvsize */
	append_math_sub_imm_u32(desc, REG3, SEQINLEN, IMM,
				ctx->authsize + tfm->ivsize);

	/* assoclen = (assoclen + cryptlen) - cryptlen */
	append_math_sub(desc, REG2, SEQOUTLEN, REG0, CAAM_CMD_SZ);
	append_math_sub(desc, VARSEQINLEN, REG3, REG2, CAAM_CMD_SZ);

	/* Will write cryptlen bytes */
	append_math_sub(desc, VARSEQOUTLEN, SEQOUTLEN, REG0, CAAM_CMD_SZ);

	/* Read Salt */
	append_fifo_load_as_imm(desc, (void *)(ctx->key + ctx->enckeylen),
				4, FIFOLD_CLASS_CLASS1 | FIFOLD_TYPE_IV);
	/* Read AES-GCM-ESP IV */
	append_seq_fifo_load(desc, tfm->ivsize, FIFOLD_CLASS_CLASS1 |
			     FIFOLD_TYPE_IV | FIFOLD_TYPE_FLUSH1);

	/* Read assoc data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_AAD | FIFOLD_TYPE_FLUSH1);

	/* Will read cryptlen bytes */
	append_math_add(desc, VARSEQINLEN, ZERO, REG2, CAAM_CMD_SZ);

	/* Store payload data */
	append_seq_fifo_store(desc, 0, FIFOST_TYPE_MESSAGE_DATA | FIFOLDST_VLF);

	/* Read encrypted data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_MSG | FIFOLD_TYPE_FLUSH1);

	/* Read ICV */
	append_seq_fifo_load(desc, ctx->authsize, FIFOLD_CLASS_CLASS1 |
			     FIFOLD_TYPE_ICV | FIFOLD_TYPE_LAST1);

	ctx->sh_desc_dec_dma = dma_map_single(jrdev, desc,
					      desc_bytes(desc),
					      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_dec_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "rfc4106 dec shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	/*
	 * Job Descriptor and Shared Descriptors
	 * must all fit into the 64-word Descriptor h/w Buffer
	 */
	keys_fit_inline = false;
	if (DESC_RFC4106_GIVENC_LEN + DESC_JOB_IO_LEN +
	    ctx->split_key_pad_len + ctx->enckeylen <=
	    CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	/* rfc4106_givencrypt shared descriptor */
	desc = ctx->sh_desc_givenc;

	init_sh_desc(desc, HDR_SHARE_SERIAL);

	/* Skip key loading if it is loaded due to sharing */
	key_jump_cmd = append_jump(desc, JUMP_JSL | JUMP_TEST_ALL |
				   JUMP_COND_SHRD);
	if (keys_fit_inline)
		append_key_as_imm(desc, (void *)ctx->key, ctx->enckeylen,
				  ctx->enckeylen, CLASS_1 | KEY_DEST_CLASS_REG);
	else
		append_key(desc, ctx->key_dma, ctx->enckeylen,
			   CLASS_1 | KEY_DEST_CLASS_REG);
	set_jump_tgt_here(desc, key_jump_cmd);

	/* Generate IV */
	geniv = NFIFOENTRY_STYPE_PAD | NFIFOENTRY_DEST_DECO |
		NFIFOENTRY_DTYPE_MSG | NFIFOENTRY_LC1 |
		NFIFOENTRY_PTYPE_RND | (tfm->ivsize << NFIFOENTRY_DLEN_SHIFT);
	append_load_imm_u32(desc, geniv, LDST_CLASS_IND_CCB |
			    LDST_SRCDST_WORD_INFO_FIFO | LDST_IMM);
	append_cmd(desc, CMD_LOAD | DISABLE_AUTO_INFO_FIFO);
	move_cmd = append_move(desc, MOVE_SRC_INFIFO | MOVE_DEST_DESCBUF |
			       (tfm->ivsize << MOVE_LEN_SHIFT));
	append_cmd(desc, CMD_LOAD | ENABLE_AUTO_INFO_FIFO);

	/* Copy generated IV to OFIFO */
	write_iv_cmd = append_move(desc, MOVE_SRC_DESCBUF | MOVE_DEST_OUTFIFO |
				   (tfm->ivsize << MOVE_LEN_SHIFT));

	/* Class 1 operation */
	append_operation(desc, ctx->class1_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT);

	/* ivsize + cryptlen = seqoutlen - authsize */
	append_math_sub_imm_u32(desc, REG3, SEQOUTLEN, IMM, ctx->authsize);

	/* assoclen = seqinlen - (ivsize + cryptlen) */
	append_math_sub(desc, VARSEQINLEN, SEQINLEN, REG3, CAAM_CMD_SZ);

	/* Will write ivsize + cryptlen */
	append_math_add(desc, VARSEQOUTLEN, REG3, REG0, CAAM_CMD_SZ);

	/* Read Salt and generated IV */
	append_cmd(desc, CMD_FIFO_LOAD | FIFOLD_CLASS_CLASS1 | FIFOLD_TYPE_IV |
		   FIFOLD_TYPE_FLUSH1 | IMMEDIATE | 12);
	/* Append Salt */
	append_data(desc, (void *)(ctx->key + ctx->enckeylen), 4);
	set_move_tgt_here(desc, move_cmd);
	set_move_tgt_here(desc, write_iv_cmd);
	/* Blank commands. Will be overwritten by generated IV. */
	append_cmd(desc, 0x00000000);
	append_cmd(desc, 0x00000000);
	/* End of blank commands */

	/* No need to reload iv */
	append_seq_fifo_load(desc, tfm->ivsize, FIFOLD_CLASS_SKIP);

	/* Read assoc data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_AAD | FIFOLD_TYPE_FLUSH1);

	/* Will read cryptlen */
	append_math_add(desc, VARSEQINLEN, SEQINLEN, REG0, CAAM_CMD_SZ);

	/* Store generated IV and encrypted data */
	append_seq_fifo_store(desc, 0, FIFOST_TYPE_MESSAGE_DATA | FIFOLDST_VLF);

	/* Read payload data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST1);

	/* Write ICV */
	append_seq_store(desc, ctx->authsize, LDST_CLASS_1_CCB |
			 LDST_SRCDST_BYTE_CONTEXT);

	ctx->sh_desc_givenc_dma = dma_map_single(jrdev, desc,
						 desc_bytes(desc),
						 DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_givenc_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR,
		       "rfc4106 givenc shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	return 0;
}

static int rfc4106_setauthsize(struct crypto_aead *authenc,
			       unsigned int authsize)
{
	struct caam_ctx *ctx = crypto_aead_ctx(authenc);

	ctx->authsize = authsize;
	rfc4106_set_sh_desc(authenc);

	return 0;
}

static int rfc4543_set_sh_desc(struct crypto_aead *aead)
{
	struct aead_tfm *tfm = &aead->base.crt_aead;
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	bool keys_fit_inline = false;
	u32 *key_jump_cmd, *write_iv_cmd, *write_aad_cmd;
	u32 *read_move_cmd, *write_move_cmd;
	u32 *desc;
	u32 geniv;

	if (!ctx->enckeylen || !ctx->authsize)
		return 0;

	/*
	 * RFC4543 encrypt shared descriptor
	 * Job Descriptor and Shared Descriptor
	 * must fit into the 64-word Descriptor h/w Buffer
	 */
	if (DESC_RFC4543_ENC_LEN + DESC_JOB_IO_LEN +
	    ctx->enckeylen <= CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	desc = ctx->sh_desc_enc;

	init_sh_desc(desc, HDR_SHARE_SERIAL);

	/* Skip key loading if it is loaded due to sharing */
	key_jump_cmd = append_jump(desc, JUMP_JSL | JUMP_TEST_ALL |
				   JUMP_COND_SHRD);
	if (keys_fit_inline)
		append_key_as_imm(desc, (void *)ctx->key, ctx->enckeylen,
				  ctx->enckeylen, CLASS_1 | KEY_DEST_CLASS_REG);
	else
		append_key(desc, ctx->key_dma, ctx->enckeylen,
			   CLASS_1 | KEY_DEST_CLASS_REG);
	set_jump_tgt_here(desc, key_jump_cmd);

	/* Class 1 operation */
	append_operation(desc, ctx->class1_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT);

	/* Load AES-GMAC ESP IV into Math1 register */
	append_cmd(desc, CMD_SEQ_LOAD | LDST_SRCDST_WORD_DECO_MATH1 |
		   LDST_CLASS_DECO | tfm->ivsize);

	/* Wait the DMA transaction to finish */
	append_jump(desc, JUMP_TEST_ALL | JUMP_COND_CALM |
		    (1 << JUMP_OFFSET_SHIFT));

	/* Overwrite blank immediate AES-GMAC ESP IV data */
	write_iv_cmd = append_move(desc, MOVE_SRC_MATH1 | MOVE_DEST_DESCBUF |
				   (tfm->ivsize << MOVE_LEN_SHIFT));

	/* Overwrite blank immediate AAD data */
	write_aad_cmd = append_move(desc, MOVE_SRC_MATH1 | MOVE_DEST_DESCBUF |
				    (tfm->ivsize << MOVE_LEN_SHIFT));

	/* cryptlen = seqoutlen - authsize */
	append_math_sub_imm_u32(desc, REG3, SEQOUTLEN, IMM, ctx->authsize);

	/* assoclen = (seqinlen - ivsize) - cryptlen */
	append_math_sub(desc, VARSEQINLEN, SEQINLEN, REG3, CAAM_CMD_SZ);

	/* Read Salt and AES-GMAC ESP IV */
	append_cmd(desc, CMD_FIFO_LOAD | FIFOLD_CLASS_CLASS1 | IMMEDIATE |
		   FIFOLD_TYPE_IV | FIFOLD_TYPE_FLUSH1 | (4 + tfm->ivsize));
	/* Append Salt */
	append_data(desc, (void *)(ctx->key + ctx->enckeylen), 4);
	set_move_tgt_here(desc, write_iv_cmd);
	/* Blank commands. Will be overwritten by AES-GMAC ESP IV. */
	append_cmd(desc, 0x00000000);
	append_cmd(desc, 0x00000000);
	/* End of blank commands */

	/* Read assoc data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_AAD);

	/* Will read cryptlen bytes */
	append_math_add(desc, VARSEQINLEN, ZERO, REG3, CAAM_CMD_SZ);

	/* Will write cryptlen bytes */
	append_math_add(desc, VARSEQOUTLEN, ZERO, REG3, CAAM_CMD_SZ);

	/*
	 * MOVE_LEN opcode is not available in all SEC HW revisions,
	 * thus need to do some magic, i.e. self-patch the descriptor
	 * buffer.
	 */
	read_move_cmd = append_move(desc, MOVE_SRC_DESCBUF | MOVE_DEST_MATH3 |
				    (0x6 << MOVE_LEN_SHIFT));
	write_move_cmd = append_move(desc, MOVE_SRC_MATH3 | MOVE_DEST_DESCBUF |
				     (0x8 << MOVE_LEN_SHIFT));

	/* Authenticate AES-GMAC ESP IV  */
	append_cmd(desc, CMD_FIFO_LOAD | FIFOLD_CLASS_CLASS1 | IMMEDIATE |
		   FIFOLD_TYPE_AAD | tfm->ivsize);
	set_move_tgt_here(desc, write_aad_cmd);
	/* Blank commands. Will be overwritten by AES-GMAC ESP IV. */
	append_cmd(desc, 0x00000000);
	append_cmd(desc, 0x00000000);
	/* End of blank commands */

	/* Read and write cryptlen bytes */
	aead_append_src_dst(desc, FIFOLD_TYPE_AAD);

	set_move_tgt_here(desc, read_move_cmd);
	set_move_tgt_here(desc, write_move_cmd);
	append_cmd(desc, CMD_LOAD | DISABLE_AUTO_INFO_FIFO);
	/* Move payload data to OFIFO */
	append_move(desc, MOVE_SRC_INFIFO_CL | MOVE_DEST_OUTFIFO);

	/* Write ICV */
	append_seq_store(desc, ctx->authsize, LDST_CLASS_1_CCB |
			 LDST_SRCDST_BYTE_CONTEXT);

	ctx->sh_desc_enc_dma = dma_map_single(jrdev, desc,
					      desc_bytes(desc),
					      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_enc_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "rfc4543 enc shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	/*
	 * Job Descriptor and Shared Descriptors
	 * must all fit into the 64-word Descriptor h/w Buffer
	 */
	keys_fit_inline = false;
	if (DESC_RFC4543_DEC_LEN + DESC_JOB_IO_LEN +
	    ctx->enckeylen <= CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	desc = ctx->sh_desc_dec;

	init_sh_desc(desc, HDR_SHARE_SERIAL);

	/* Skip key loading if it is loaded due to sharing */
	key_jump_cmd = append_jump(desc, JUMP_JSL |
				   JUMP_TEST_ALL | JUMP_COND_SHRD);
	if (keys_fit_inline)
		append_key_as_imm(desc, (void *)ctx->key, ctx->enckeylen,
				  ctx->enckeylen, CLASS_1 | KEY_DEST_CLASS_REG);
	else
		append_key(desc, ctx->key_dma, ctx->enckeylen,
			   CLASS_1 | KEY_DEST_CLASS_REG);
	set_jump_tgt_here(desc, key_jump_cmd);

	/* Class 1 operation */
	append_operation(desc, ctx->class1_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_DECRYPT | OP_ALG_ICV_ON);

	/* Load AES-GMAC ESP IV into Math1 register */
	append_cmd(desc, CMD_SEQ_LOAD | LDST_SRCDST_WORD_DECO_MATH1 |
		   LDST_CLASS_DECO | tfm->ivsize);

	/* Wait the DMA transaction to finish */
	append_jump(desc, JUMP_TEST_ALL | JUMP_COND_CALM |
		    (1 << JUMP_OFFSET_SHIFT));

	/* assoclen + cryptlen = (seqinlen - ivsize) - icvsize */
	append_math_sub_imm_u32(desc, REG3, SEQINLEN, IMM, ctx->authsize);

	/* Overwrite blank immediate AES-GMAC ESP IV data */
	write_iv_cmd = append_move(desc, MOVE_SRC_MATH1 | MOVE_DEST_DESCBUF |
				   (tfm->ivsize << MOVE_LEN_SHIFT));

	/* Overwrite blank immediate AAD data */
	write_aad_cmd = append_move(desc, MOVE_SRC_MATH1 | MOVE_DEST_DESCBUF |
				    (tfm->ivsize << MOVE_LEN_SHIFT));

	/* assoclen = (assoclen + cryptlen) - cryptlen */
	append_math_sub(desc, REG2, SEQOUTLEN, REG0, CAAM_CMD_SZ);
	append_math_sub(desc, VARSEQINLEN, REG3, REG2, CAAM_CMD_SZ);

	/*
	 * MOVE_LEN opcode is not available in all SEC HW revisions,
	 * thus need to do some magic, i.e. self-patch the descriptor
	 * buffer.
	 */
	read_move_cmd = append_move(desc, MOVE_SRC_DESCBUF | MOVE_DEST_MATH3 |
				    (0x6 << MOVE_LEN_SHIFT));
	write_move_cmd = append_move(desc, MOVE_SRC_MATH3 | MOVE_DEST_DESCBUF |
				     (0x8 << MOVE_LEN_SHIFT));

	/* Read Salt and AES-GMAC ESP IV */
	append_cmd(desc, CMD_FIFO_LOAD | FIFOLD_CLASS_CLASS1 | IMMEDIATE |
		   FIFOLD_TYPE_IV | FIFOLD_TYPE_FLUSH1 | (4 + tfm->ivsize));
	/* Append Salt */
	append_data(desc, (void *)(ctx->key + ctx->enckeylen), 4);
	set_move_tgt_here(desc, write_iv_cmd);
	/* Blank commands. Will be overwritten by AES-GMAC ESP IV. */
	append_cmd(desc, 0x00000000);
	append_cmd(desc, 0x00000000);
	/* End of blank commands */

	/* Read assoc data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_AAD);

	/* Will read cryptlen bytes */
	append_math_add(desc, VARSEQINLEN, ZERO, REG2, CAAM_CMD_SZ);

	/* Will write cryptlen bytes */
	append_math_add(desc, VARSEQOUTLEN, ZERO, REG2, CAAM_CMD_SZ);

	/* Authenticate AES-GMAC ESP IV  */
	append_cmd(desc, CMD_FIFO_LOAD | FIFOLD_CLASS_CLASS1 | IMMEDIATE |
		   FIFOLD_TYPE_AAD | tfm->ivsize);
	set_move_tgt_here(desc, write_aad_cmd);
	/* Blank commands. Will be overwritten by AES-GMAC ESP IV. */
	append_cmd(desc, 0x00000000);
	append_cmd(desc, 0x00000000);
	/* End of blank commands */

	/* Store payload data */
	append_seq_fifo_store(desc, 0, FIFOST_TYPE_MESSAGE_DATA | FIFOLDST_VLF);

	/* In-snoop cryptlen data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_BOTH | FIFOLDST_VLF |
			     FIFOLD_TYPE_AAD | FIFOLD_TYPE_LAST2FLUSH1);

	set_move_tgt_here(desc, read_move_cmd);
	set_move_tgt_here(desc, write_move_cmd);
	append_cmd(desc, CMD_LOAD | DISABLE_AUTO_INFO_FIFO);
	/* Move payload data to OFIFO */
	append_move(desc, MOVE_SRC_INFIFO_CL | MOVE_DEST_OUTFIFO);
	append_cmd(desc, CMD_LOAD | ENABLE_AUTO_INFO_FIFO);

	/* Read ICV */
	append_seq_fifo_load(desc, ctx->authsize, FIFOLD_CLASS_CLASS1 |
			     FIFOLD_TYPE_ICV | FIFOLD_TYPE_LAST1);

	ctx->sh_desc_dec_dma = dma_map_single(jrdev, desc,
					      desc_bytes(desc),
					      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_dec_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "rfc4543 dec shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	/*
	 * Job Descriptor and Shared Descriptors
	 * must all fit into the 64-word Descriptor h/w Buffer
	 */
	keys_fit_inline = false;
	if (DESC_RFC4543_GIVENC_LEN + DESC_JOB_IO_LEN +
	    ctx->enckeylen <= CAAM_DESC_BYTES_MAX)
		keys_fit_inline = true;

	/* rfc4543_givencrypt shared descriptor */
	desc = ctx->sh_desc_givenc;

	init_sh_desc(desc, HDR_SHARE_SERIAL);

	/* Skip key loading if it is loaded due to sharing */
	key_jump_cmd = append_jump(desc, JUMP_JSL | JUMP_TEST_ALL |
				   JUMP_COND_SHRD);
	if (keys_fit_inline)
		append_key_as_imm(desc, (void *)ctx->key, ctx->enckeylen,
				  ctx->enckeylen, CLASS_1 | KEY_DEST_CLASS_REG);
	else
		append_key(desc, ctx->key_dma, ctx->enckeylen,
			   CLASS_1 | KEY_DEST_CLASS_REG);
	set_jump_tgt_here(desc, key_jump_cmd);

	/* Generate IV */
	geniv = NFIFOENTRY_STYPE_PAD | NFIFOENTRY_DEST_DECO |
		NFIFOENTRY_DTYPE_MSG | NFIFOENTRY_LC1 |
		NFIFOENTRY_PTYPE_RND | (tfm->ivsize << NFIFOENTRY_DLEN_SHIFT);
	append_load_imm_u32(desc, geniv, LDST_CLASS_IND_CCB |
			    LDST_SRCDST_WORD_INFO_FIFO | LDST_IMM);
	append_cmd(desc, CMD_LOAD | DISABLE_AUTO_INFO_FIFO);
	/* Move generated IV to Math1 register */
	append_move(desc, MOVE_SRC_INFIFO | MOVE_DEST_MATH1 |
		    (tfm->ivsize << MOVE_LEN_SHIFT));
	append_cmd(desc, CMD_LOAD | ENABLE_AUTO_INFO_FIFO);

	/* Overwrite blank immediate AES-GMAC IV data */
	write_iv_cmd = append_move(desc, MOVE_SRC_MATH1 | MOVE_DEST_DESCBUF |
				   (tfm->ivsize << MOVE_LEN_SHIFT));

	/* Overwrite blank immediate AAD data */
	write_aad_cmd = append_move(desc, MOVE_SRC_MATH1 | MOVE_DEST_DESCBUF |
				    (tfm->ivsize << MOVE_LEN_SHIFT));

	/* Copy generated IV to OFIFO */
	append_move(desc, MOVE_SRC_MATH1 | MOVE_DEST_OUTFIFO |
		    (tfm->ivsize << MOVE_LEN_SHIFT));

	/* Class 1 operation */
	append_operation(desc, ctx->class1_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT);

	/* ivsize + cryptlen = seqoutlen - authsize */
	append_math_sub_imm_u32(desc, REG3, SEQOUTLEN, IMM, ctx->authsize);

	/* assoclen = seqinlen - (ivsize + cryptlen) */
	append_math_sub(desc, VARSEQINLEN, SEQINLEN, REG3, CAAM_CMD_SZ);

	/* Will write ivsize + cryptlen */
	append_math_add(desc, VARSEQOUTLEN, REG3, REG0, CAAM_CMD_SZ);

	/*
	 * MOVE_LEN opcode is not available in all SEC HW revisions,
	 * thus need to do some magic, i.e. self-patch the descriptor
	 * buffer.
	 */
	read_move_cmd = append_move(desc, MOVE_SRC_DESCBUF | MOVE_DEST_MATH3 |
				    (0x6 << MOVE_LEN_SHIFT));
	write_move_cmd = append_move(desc, MOVE_SRC_MATH3 | MOVE_DEST_DESCBUF |
				     (0x8 << MOVE_LEN_SHIFT));

	/* Read Salt and AES-GMAC generated IV */
	append_cmd(desc, CMD_FIFO_LOAD | FIFOLD_CLASS_CLASS1 | IMMEDIATE |
		   FIFOLD_TYPE_IV | FIFOLD_TYPE_FLUSH1 | (4 + tfm->ivsize));
	/* Append Salt */
	append_data(desc, (void *)(ctx->key + ctx->enckeylen), 4);
	set_move_tgt_here(desc, write_iv_cmd);
	/* Blank commands. Will be overwritten by AES-GMAC generated IV. */
	append_cmd(desc, 0x00000000);
	append_cmd(desc, 0x00000000);
	/* End of blank commands */

	/* No need to reload iv */
	append_seq_fifo_load(desc, tfm->ivsize, FIFOLD_CLASS_SKIP);

	/* Read assoc data */
	append_seq_fifo_load(desc, 0, FIFOLD_CLASS_CLASS1 | FIFOLDST_VLF |
			     FIFOLD_TYPE_AAD);

	/* Will read cryptlen */
	append_math_add(desc, VARSEQINLEN, SEQINLEN, REG0, CAAM_CMD_SZ);

	/* Authenticate AES-GMAC IV  */
	append_cmd(desc, CMD_FIFO_LOAD | FIFOLD_CLASS_CLASS1 | IMMEDIATE |
		   FIFOLD_TYPE_AAD | tfm->ivsize);
	set_move_tgt_here(desc, write_aad_cmd);
	/* Blank commands. Will be overwritten by AES-GMAC IV. */
	append_cmd(desc, 0x00000000);
	append_cmd(desc, 0x00000000);
	/* End of blank commands */

	/* Read and write cryptlen bytes */
	aead_append_src_dst(desc, FIFOLD_TYPE_AAD);

	set_move_tgt_here(desc, read_move_cmd);
	set_move_tgt_here(desc, write_move_cmd);
	append_cmd(desc, CMD_LOAD | DISABLE_AUTO_INFO_FIFO);
	/* Move payload data to OFIFO */
	append_move(desc, MOVE_SRC_INFIFO_CL | MOVE_DEST_OUTFIFO);

	/* Write ICV */
	append_seq_store(desc, ctx->authsize, LDST_CLASS_1_CCB |
			 LDST_SRCDST_BYTE_CONTEXT);

	ctx->sh_desc_givenc_dma = dma_map_single(jrdev, desc,
						 desc_bytes(desc),
						 DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_givenc_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR,
		       "rfc4543 givenc shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	return 0;
}

static int rfc4543_setauthsize(struct crypto_aead *authenc,
			       unsigned int authsize)
{
	struct caam_ctx *ctx = crypto_aead_ctx(authenc);

	ctx->authsize = authsize;
	rfc4543_set_sh_desc(authenc);

	return 0;
}

static u32 gen_split_aead_key(struct caam_ctx *ctx, const u8 *key_in,
			      u32 authkeylen)
{
	return gen_split_key(ctx->jrdev, ctx->key, ctx->split_key_len,
			       ctx->split_key_pad_len, key_in, authkeylen,
			       ctx->alg_op);
}

static int aead_setkey(struct crypto_aead *aead,
			       const u8 *key, unsigned int keylen)
{
	/* Sizes for MDHA pads (*not* keys): MD5, SHA1, 224, 256, 384, 512 */
	static const u8 mdpadlen[] = { 16, 20, 32, 32, 64, 64 };
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	struct crypto_authenc_keys keys;
	int ret = 0;

	if (crypto_authenc_extractkeys(&keys, key, keylen) != 0)
		goto badkey;

	/* Pick class 2 key length from algorithm submask */
	ctx->split_key_len = mdpadlen[(ctx->alg_op & OP_ALG_ALGSEL_SUBMASK) >>
				      OP_ALG_ALGSEL_SHIFT] * 2;
	ctx->split_key_pad_len = ALIGN(ctx->split_key_len, 16);

	if (ctx->split_key_pad_len + keys.enckeylen > CAAM_MAX_KEY_SIZE)
		goto badkey;

#ifdef DEBUG
	printk(KERN_ERR "keylen %d enckeylen %d authkeylen %d\n",
	       keys.authkeylen + keys.enckeylen, keys.enckeylen,
	       keys.authkeylen);
	printk(KERN_ERR "split_key_len %d split_key_pad_len %d\n",
	       ctx->split_key_len, ctx->split_key_pad_len);
	print_hex_dump(KERN_ERR, "key in @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, key, keylen, 1);
#endif

	ret = gen_split_aead_key(ctx, keys.authkey, keys.authkeylen);
	if (ret) {
		goto badkey;
	}

	/* postpend encryption key to auth split key */
	memcpy(ctx->key + ctx->split_key_pad_len, keys.enckey, keys.enckeylen);

	ctx->key_dma = dma_map_single(jrdev, ctx->key, ctx->split_key_pad_len +
				      keys.enckeylen, DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->key_dma)) {
		dev_err(jrdev, "unable to map key i/o memory\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "ctx.key@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, ctx->key,
		       ctx->split_key_pad_len + keys.enckeylen, 1);
#endif

	ctx->enckeylen = keys.enckeylen;

	ret = aead_set_sh_desc(aead);
	if (ret) {
		dma_unmap_single(jrdev, ctx->key_dma, ctx->split_key_pad_len +
				 keys.enckeylen, DMA_TO_DEVICE);
	}

	return ret;
badkey:
	crypto_aead_set_flags(aead, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return -EINVAL;
}

static int gcm_setkey(struct crypto_aead *aead,
		      const u8 *key, unsigned int keylen)
{
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	int ret = 0;

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "key in @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, key, keylen, 1);
#endif

	memcpy(ctx->key, key, keylen);
	ctx->key_dma = dma_map_single(jrdev, ctx->key, keylen,
				      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->key_dma)) {
		dev_err(jrdev, "unable to map key i/o memory\n");
		return -ENOMEM;
	}
	ctx->enckeylen = keylen;

	ret = gcm_set_sh_desc(aead);
	if (ret) {
		dma_unmap_single(jrdev, ctx->key_dma, ctx->enckeylen,
				 DMA_TO_DEVICE);
	}

	return ret;
}

static int rfc4106_setkey(struct crypto_aead *aead,
			  const u8 *key, unsigned int keylen)
{
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	int ret = 0;

	if (keylen < 4)
		return -EINVAL;

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "key in @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, key, keylen, 1);
#endif

	memcpy(ctx->key, key, keylen);

	/*
	 * The last four bytes of the key material are used as the salt value
	 * in the nonce. Update the AES key length.
	 */
	ctx->enckeylen = keylen - 4;

	ctx->key_dma = dma_map_single(jrdev, ctx->key, ctx->enckeylen,
				      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->key_dma)) {
		dev_err(jrdev, "unable to map key i/o memory\n");
		return -ENOMEM;
	}

	ret = rfc4106_set_sh_desc(aead);
	if (ret) {
		dma_unmap_single(jrdev, ctx->key_dma, ctx->enckeylen,
				 DMA_TO_DEVICE);
	}

	return ret;
}

static int rfc4543_setkey(struct crypto_aead *aead,
			  const u8 *key, unsigned int keylen)
{
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	int ret = 0;

	if (keylen < 4)
		return -EINVAL;

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "key in @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, key, keylen, 1);
#endif

	memcpy(ctx->key, key, keylen);

	/*
	 * The last four bytes of the key material are used as the salt value
	 * in the nonce. Update the AES key length.
	 */
	ctx->enckeylen = keylen - 4;

	ctx->key_dma = dma_map_single(jrdev, ctx->key, ctx->enckeylen,
				      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->key_dma)) {
		dev_err(jrdev, "unable to map key i/o memory\n");
		return -ENOMEM;
	}

	ret = rfc4543_set_sh_desc(aead);
	if (ret) {
		dma_unmap_single(jrdev, ctx->key_dma, ctx->enckeylen,
				 DMA_TO_DEVICE);
	}

	return ret;
}

static int ablkcipher_setkey(struct crypto_ablkcipher *ablkcipher,
			     const u8 *key, unsigned int keylen)
{
	struct caam_ctx *ctx = crypto_ablkcipher_ctx(ablkcipher);
	struct ablkcipher_tfm *crt = &ablkcipher->base.crt_ablkcipher;
	struct crypto_tfm *tfm = crypto_ablkcipher_tfm(ablkcipher);
	const char *alg_name = crypto_tfm_alg_name(tfm);
	struct device *jrdev = ctx->jrdev;
	int ret = 0;
	u32 *key_jump_cmd;
	u32 *desc;
	u32 *nonce;
	u32 geniv;
	u32 ctx1_iv_off = 0;
	const bool ctr_mode = ((ctx->class1_alg_type & OP_ALG_AAI_MASK) ==
			       OP_ALG_AAI_CTR_MOD128);
	const bool is_rfc3686 = (ctr_mode &&
				 (strstr(alg_name, "rfc3686") != NULL));

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "key in @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, key, keylen, 1);
#endif
	/*
	 * AES-CTR needs to load IV in CONTEXT1 reg
	 * at an offset of 128bits (16bytes)
	 * CONTEXT1[255:128] = IV
	 */
	if (ctr_mode)
		ctx1_iv_off = 16;

	/*
	 * RFC3686 specific:
	 *	| CONTEXT1[255:128] = {NONCE, IV, COUNTER}
	 *	| *key = {KEY, NONCE}
	 */
	if (is_rfc3686) {
		ctx1_iv_off = 16 + CTR_RFC3686_NONCE_SIZE;
		keylen -= CTR_RFC3686_NONCE_SIZE;
	}

	memcpy(ctx->key, key, keylen);
	ctx->key_dma = dma_map_single(jrdev, ctx->key, keylen,
				      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->key_dma)) {
		dev_err(jrdev, "unable to map key i/o memory\n");
		return -ENOMEM;
	}
	ctx->enckeylen = keylen;

	/* ablkcipher_encrypt shared descriptor */
	desc = ctx->sh_desc_enc;
	init_sh_desc(desc, HDR_SHARE_SERIAL | HDR_SAVECTX);
	/* Skip if already shared */
	key_jump_cmd = append_jump(desc, JUMP_JSL | JUMP_TEST_ALL |
				   JUMP_COND_SHRD);

	/* Load class1 key only */
	append_key_as_imm(desc, (void *)ctx->key, ctx->enckeylen,
			  ctx->enckeylen, CLASS_1 |
			  KEY_DEST_CLASS_REG);

	/* Load nonce into CONTEXT1 reg */
	if (is_rfc3686) {
		nonce = (u32 *)(key + keylen);
		append_load_imm_u32(desc, *nonce, LDST_CLASS_IND_CCB |
				    LDST_SRCDST_BYTE_OUTFIFO | LDST_IMM);
		append_move(desc, MOVE_WAITCOMP |
			    MOVE_SRC_OUTFIFO |
			    MOVE_DEST_CLASS1CTX |
			    (16 << MOVE_OFFSET_SHIFT) |
			    (CTR_RFC3686_NONCE_SIZE << MOVE_LEN_SHIFT));
	}

	set_jump_tgt_here(desc, key_jump_cmd);

	/* Load iv */
	append_seq_load(desc, crt->ivsize, LDST_SRCDST_BYTE_CONTEXT |
			LDST_CLASS_1_CCB | (ctx1_iv_off << LDST_OFFSET_SHIFT));

	/* Load counter into CONTEXT1 reg */
	if (is_rfc3686)
		append_load_imm_u32(desc, be32_to_cpu(1), LDST_IMM |
				    LDST_CLASS_1_CCB |
				    LDST_SRCDST_BYTE_CONTEXT |
				    ((ctx1_iv_off + CTR_RFC3686_IV_SIZE) <<
				     LDST_OFFSET_SHIFT));

	/* Load operation */
	append_operation(desc, ctx->class1_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT);

	/* Perform operation */
	ablkcipher_append_src_dst(desc);

	ctx->sh_desc_enc_dma = dma_map_single(jrdev, desc,
					      desc_bytes(desc),
					      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_enc_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR,
		       "ablkcipher enc shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif
	/* ablkcipher_decrypt shared descriptor */
	desc = ctx->sh_desc_dec;

	init_sh_desc(desc, HDR_SHARE_SERIAL | HDR_SAVECTX);
	/* Skip if already shared */
	key_jump_cmd = append_jump(desc, JUMP_JSL | JUMP_TEST_ALL |
				   JUMP_COND_SHRD);

	/* Load class1 key only */
	append_key_as_imm(desc, (void *)ctx->key, ctx->enckeylen,
			  ctx->enckeylen, CLASS_1 |
			  KEY_DEST_CLASS_REG);

	/* Load nonce into CONTEXT1 reg */
	if (is_rfc3686) {
		nonce = (u32 *)(key + keylen);
		append_load_imm_u32(desc, *nonce, LDST_CLASS_IND_CCB |
				    LDST_SRCDST_BYTE_OUTFIFO | LDST_IMM);
		append_move(desc, MOVE_WAITCOMP |
			    MOVE_SRC_OUTFIFO |
			    MOVE_DEST_CLASS1CTX |
			    (16 << MOVE_OFFSET_SHIFT) |
			    (CTR_RFC3686_NONCE_SIZE << MOVE_LEN_SHIFT));
	}

	set_jump_tgt_here(desc, key_jump_cmd);

	/* load IV */
	append_seq_load(desc, crt->ivsize, LDST_SRCDST_BYTE_CONTEXT |
			LDST_CLASS_1_CCB | (ctx1_iv_off << LDST_OFFSET_SHIFT));

	/* Load counter into CONTEXT1 reg */
	if (is_rfc3686)
		append_load_imm_u32(desc, be32_to_cpu(1), LDST_IMM |
				    LDST_CLASS_1_CCB |
				    LDST_SRCDST_BYTE_CONTEXT |
				    ((ctx1_iv_off + CTR_RFC3686_IV_SIZE) <<
				     LDST_OFFSET_SHIFT));

	/* Choose operation */
	if (ctr_mode)
		append_operation(desc, ctx->class1_alg_type |
				 OP_ALG_AS_INITFINAL | OP_ALG_DECRYPT);
	else
		append_dec_op1(desc, ctx->class1_alg_type);

	/* Perform operation */
	ablkcipher_append_src_dst(desc);

	ctx->sh_desc_dec_dma = dma_map_single(jrdev, desc,
					      desc_bytes(desc),
					      DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_dec_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}

#ifdef DEBUG
	print_hex_dump(KERN_ERR,
		       "ablkcipher dec shdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif
	/* ablkcipher_givencrypt shared descriptor */
	desc = ctx->sh_desc_givenc;

	init_sh_desc(desc, HDR_SHARE_SERIAL | HDR_SAVECTX);
	/* Skip if already shared */
	key_jump_cmd = append_jump(desc, JUMP_JSL | JUMP_TEST_ALL |
				   JUMP_COND_SHRD);

	/* Load class1 key only */
	append_key_as_imm(desc, (void *)ctx->key, ctx->enckeylen,
			  ctx->enckeylen, CLASS_1 |
			  KEY_DEST_CLASS_REG);

	/* Load Nonce into CONTEXT1 reg */
	if (is_rfc3686) {
		nonce = (u32 *)(key + keylen);
		append_load_imm_u32(desc, *nonce, LDST_CLASS_IND_CCB |
				    LDST_SRCDST_BYTE_OUTFIFO | LDST_IMM);
		append_move(desc, MOVE_WAITCOMP |
			    MOVE_SRC_OUTFIFO |
			    MOVE_DEST_CLASS1CTX |
			    (16 << MOVE_OFFSET_SHIFT) |
			    (CTR_RFC3686_NONCE_SIZE << MOVE_LEN_SHIFT));
	}
	set_jump_tgt_here(desc, key_jump_cmd);

	/* Generate IV */
	geniv = NFIFOENTRY_STYPE_PAD | NFIFOENTRY_DEST_DECO |
		NFIFOENTRY_DTYPE_MSG | NFIFOENTRY_LC1 |
		NFIFOENTRY_PTYPE_RND | (crt->ivsize << NFIFOENTRY_DLEN_SHIFT);
	append_load_imm_u32(desc, geniv, LDST_CLASS_IND_CCB |
			    LDST_SRCDST_WORD_INFO_FIFO | LDST_IMM);
	append_cmd(desc, CMD_LOAD | DISABLE_AUTO_INFO_FIFO);
	append_move(desc, MOVE_WAITCOMP |
		    MOVE_SRC_INFIFO |
		    MOVE_DEST_CLASS1CTX |
		    (crt->ivsize << MOVE_LEN_SHIFT) |
		    (ctx1_iv_off << MOVE_OFFSET_SHIFT));
	append_cmd(desc, CMD_LOAD | ENABLE_AUTO_INFO_FIFO);

	/* Copy generated IV to memory */
	append_seq_store(desc, crt->ivsize,
			 LDST_SRCDST_BYTE_CONTEXT | LDST_CLASS_1_CCB |
			 (ctx1_iv_off << LDST_OFFSET_SHIFT));

	/* Load Counter into CONTEXT1 reg */
	if (is_rfc3686)
		append_load_imm_u32(desc, (u32)1, LDST_IMM |
				    LDST_CLASS_1_CCB |
				    LDST_SRCDST_BYTE_CONTEXT |
				    ((ctx1_iv_off + CTR_RFC3686_IV_SIZE) <<
				     LDST_OFFSET_SHIFT));

	if (ctx1_iv_off)
		append_jump(desc, JUMP_JSL | JUMP_TEST_ALL | JUMP_COND_NCP |
			    (1 << JUMP_OFFSET_SHIFT));

	/* Load operation */
	append_operation(desc, ctx->class1_alg_type |
			 OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT);

	/* Perform operation */
	ablkcipher_append_src_dst(desc);

	ctx->sh_desc_givenc_dma = dma_map_single(jrdev, desc,
						 desc_bytes(desc),
						 DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, ctx->sh_desc_givenc_dma)) {
		dev_err(jrdev, "unable to map shared descriptor\n");
		return -ENOMEM;
	}
#ifdef DEBUG
	print_hex_dump(KERN_ERR,
		       "ablkcipher givenc shdesc@" __stringify(__LINE__) ": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, desc,
		       desc_bytes(desc), 1);
#endif

	return ret;
}

/*
 * aead_edesc - s/w-extended aead descriptor
 * @assoc_nents: number of segments in associated data (SPI+Seq) scatterlist
 * @assoc_chained: if source is chained
 * @src_nents: number of segments in input scatterlist
 * @src_chained: if source is chained
 * @dst_nents: number of segments in output scatterlist
 * @dst_chained: if destination is chained
 * @iv_dma: dma address of iv for checking continuity and link table
 * @desc: h/w descriptor (variable length; must not exceed MAX_CAAM_DESCSIZE)
 * @sec4_sg_bytes: length of dma mapped sec4_sg space
 * @sec4_sg_dma: bus physical mapped address of h/w link table
 * @hw_desc: the h/w job descriptor followed by any referenced link tables
 */
struct aead_edesc {
	int assoc_nents;
	bool assoc_chained;
	int src_nents;
	bool src_chained;
	int dst_nents;
	bool dst_chained;
	dma_addr_t iv_dma;
	int sec4_sg_bytes;
	dma_addr_t sec4_sg_dma;
	struct sec4_sg_entry *sec4_sg;
	u32 hw_desc[0];
};

/*
 * ablkcipher_edesc - s/w-extended ablkcipher descriptor
 * @src_nents: number of segments in input scatterlist
 * @src_chained: if source is chained
 * @dst_nents: number of segments in output scatterlist
 * @dst_chained: if destination is chained
 * @iv_dma: dma address of iv for checking continuity and link table
 * @desc: h/w descriptor (variable length; must not exceed MAX_CAAM_DESCSIZE)
 * @sec4_sg_bytes: length of dma mapped sec4_sg space
 * @sec4_sg_dma: bus physical mapped address of h/w link table
 * @hw_desc: the h/w job descriptor followed by any referenced link tables
 */
struct ablkcipher_edesc {
	int src_nents;
	bool src_chained;
	int dst_nents;
	bool dst_chained;
	dma_addr_t iv_dma;
	int sec4_sg_bytes;
	dma_addr_t sec4_sg_dma;
	struct sec4_sg_entry *sec4_sg;
	u32 hw_desc[0];
};

static void caam_unmap(struct device *dev, struct scatterlist *src,
		       struct scatterlist *dst, int src_nents,
		       bool src_chained, int dst_nents, bool dst_chained,
		       dma_addr_t iv_dma, int ivsize, dma_addr_t sec4_sg_dma,
		       int sec4_sg_bytes)
{
	if (dst != src) {
		dma_unmap_sg_chained(dev, src, src_nents ? : 1, DMA_TO_DEVICE,
				     src_chained);
		dma_unmap_sg_chained(dev, dst, dst_nents ? : 1, DMA_FROM_DEVICE,
				     dst_chained);
	} else {
		dma_unmap_sg_chained(dev, src, src_nents ? : 1,
				     DMA_BIDIRECTIONAL, src_chained);
	}

	if (iv_dma)
		dma_unmap_single(dev, iv_dma, ivsize, DMA_TO_DEVICE);
	if (sec4_sg_bytes)
		dma_unmap_single(dev, sec4_sg_dma, sec4_sg_bytes,
				 DMA_TO_DEVICE);
}

static void aead_unmap(struct device *dev,
		       struct aead_edesc *edesc,
		       struct aead_request *req)
{
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	int ivsize = crypto_aead_ivsize(aead);

	dma_unmap_sg_chained(dev, req->assoc, edesc->assoc_nents,
			     DMA_TO_DEVICE, edesc->assoc_chained);

	caam_unmap(dev, req->src, req->dst,
		   edesc->src_nents, edesc->src_chained, edesc->dst_nents,
		   edesc->dst_chained, edesc->iv_dma, ivsize,
		   edesc->sec4_sg_dma, edesc->sec4_sg_bytes);
}

static void ablkcipher_unmap(struct device *dev,
			     struct ablkcipher_edesc *edesc,
			     struct ablkcipher_request *req)
{
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	int ivsize = crypto_ablkcipher_ivsize(ablkcipher);

	caam_unmap(dev, req->src, req->dst,
		   edesc->src_nents, edesc->src_chained, edesc->dst_nents,
		   edesc->dst_chained, edesc->iv_dma, ivsize,
		   edesc->sec4_sg_dma, edesc->sec4_sg_bytes);
}

static void aead_encrypt_done(struct device *jrdev, u32 *desc, u32 err,
				   void *context)
{
	struct aead_request *req = context;
	struct aead_edesc *edesc;
#ifdef DEBUG
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	int ivsize = crypto_aead_ivsize(aead);

	dev_err(jrdev, "%s %d: err 0x%x\n", __func__, __LINE__, err);
#endif

	edesc = (struct aead_edesc *)((char *)desc -
		 offsetof(struct aead_edesc, hw_desc));

	if (err)
		caam_jr_strstatus(jrdev, err);

	aead_unmap(jrdev, edesc, req);

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "assoc  @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->assoc),
		       req->assoclen , 1);
	print_hex_dump(KERN_ERR, "dstiv  @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->src) - ivsize,
		       edesc->src_nents ? 100 : ivsize, 1);
	print_hex_dump(KERN_ERR, "dst    @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->src),
		       edesc->src_nents ? 100 : req->cryptlen +
		       ctx->authsize + 4, 1);
#endif

	kfree(edesc);

	aead_request_complete(req, err);
}

static void aead_decrypt_done(struct device *jrdev, u32 *desc, u32 err,
				   void *context)
{
	struct aead_request *req = context;
	struct aead_edesc *edesc;
#ifdef DEBUG
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	int ivsize = crypto_aead_ivsize(aead);

	dev_err(jrdev, "%s %d: err 0x%x\n", __func__, __LINE__, err);
#endif

	edesc = (struct aead_edesc *)((char *)desc -
		 offsetof(struct aead_edesc, hw_desc));

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "dstiv  @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, req->iv,
		       ivsize, 1);
	print_hex_dump(KERN_ERR, "dst    @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->dst),
		       req->cryptlen - ctx->authsize, 1);
#endif

	if (err)
		caam_jr_strstatus(jrdev, err);

	aead_unmap(jrdev, edesc, req);

	/*
	 * verify hw auth check passed else return -EBADMSG
	 */
	if ((err & JRSTA_CCBERR_ERRID_MASK) == JRSTA_CCBERR_ERRID_ICVCHK)
		err = -EBADMSG;

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "iphdrout@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4,
		       ((char *)sg_virt(req->assoc) - sizeof(struct iphdr)),
		       sizeof(struct iphdr) + req->assoclen +
		       ((req->cryptlen > 1500) ? 1500 : req->cryptlen) +
		       ctx->authsize + 36, 1);
	if (!err && edesc->sec4_sg_bytes) {
		struct scatterlist *sg = sg_last(req->src, edesc->src_nents);
		print_hex_dump(KERN_ERR, "sglastout@"__stringify(__LINE__)": ",
			       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(sg),
			sg->length + ctx->authsize + 16, 1);
	}
#endif

	kfree(edesc);

	aead_request_complete(req, err);
}

static void ablkcipher_encrypt_done(struct device *jrdev, u32 *desc, u32 err,
				   void *context)
{
	struct ablkcipher_request *req = context;
	struct ablkcipher_edesc *edesc;
#ifdef DEBUG
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	int ivsize = crypto_ablkcipher_ivsize(ablkcipher);

	dev_err(jrdev, "%s %d: err 0x%x\n", __func__, __LINE__, err);
#endif

	edesc = (struct ablkcipher_edesc *)((char *)desc -
		 offsetof(struct ablkcipher_edesc, hw_desc));

	if (err)
		caam_jr_strstatus(jrdev, err);

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "dstiv  @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, req->info,
		       edesc->src_nents > 1 ? 100 : ivsize, 1);
	print_hex_dump(KERN_ERR, "dst    @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->src),
		       edesc->dst_nents > 1 ? 100 : req->nbytes, 1);
#endif

	ablkcipher_unmap(jrdev, edesc, req);
	kfree(edesc);

	ablkcipher_request_complete(req, err);
}

static void ablkcipher_decrypt_done(struct device *jrdev, u32 *desc, u32 err,
				    void *context)
{
	struct ablkcipher_request *req = context;
	struct ablkcipher_edesc *edesc;
#ifdef DEBUG
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	int ivsize = crypto_ablkcipher_ivsize(ablkcipher);

	dev_err(jrdev, "%s %d: err 0x%x\n", __func__, __LINE__, err);
#endif

	edesc = (struct ablkcipher_edesc *)((char *)desc -
		 offsetof(struct ablkcipher_edesc, hw_desc));
	if (err)
		caam_jr_strstatus(jrdev, err);

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "dstiv  @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, req->info,
		       ivsize, 1);
	print_hex_dump(KERN_ERR, "dst    @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->src),
		       edesc->dst_nents > 1 ? 100 : req->nbytes, 1);
#endif

	ablkcipher_unmap(jrdev, edesc, req);
	kfree(edesc);

	ablkcipher_request_complete(req, err);
}

/*
 * Fill in aead job descriptor
 */
static void init_aead_job(u32 *sh_desc, dma_addr_t ptr,
			  struct aead_edesc *edesc,
			  struct aead_request *req,
			  bool all_contig, bool encrypt)
{
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	int ivsize = crypto_aead_ivsize(aead);
	int authsize = ctx->authsize;
	u32 *desc = edesc->hw_desc;
	u32 out_options = 0, in_options;
	dma_addr_t dst_dma, src_dma;
	int len, sec4_sg_index = 0;
	bool is_gcm = false;

#ifdef DEBUG
	debug("assoclen %d cryptlen %d authsize %d\n",
	      req->assoclen, req->cryptlen, authsize);
	print_hex_dump(KERN_ERR, "assoc  @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->assoc),
		       req->assoclen , 1);
	print_hex_dump(KERN_ERR, "presciv@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, req->iv,
		       edesc->src_nents ? 100 : ivsize, 1);
	print_hex_dump(KERN_ERR, "src    @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->src),
			edesc->src_nents ? 100 : req->cryptlen, 1);
	print_hex_dump(KERN_ERR, "shrdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sh_desc,
		       desc_bytes(sh_desc), 1);
#endif

	if (((ctx->class1_alg_type & OP_ALG_ALGSEL_MASK) ==
	      OP_ALG_ALGSEL_AES) &&
	    ((ctx->class1_alg_type & OP_ALG_AAI_MASK) == OP_ALG_AAI_GCM))
		is_gcm = true;

	len = desc_len(sh_desc);
	init_job_desc_shared(desc, ptr, len, HDR_SHARE_DEFER | HDR_REVERSE);

	if (all_contig) {
		if (is_gcm)
			src_dma = edesc->iv_dma;
		else
			src_dma = sg_dma_address(req->assoc);
		in_options = 0;
	} else {
		src_dma = edesc->sec4_sg_dma;
		sec4_sg_index += (edesc->assoc_nents ? : 1) + 1 +
				 (edesc->src_nents ? : 1);
		in_options = LDST_SGF;
	}

	append_seq_in_ptr(desc, src_dma, req->assoclen + ivsize + req->cryptlen,
			  in_options);

	if (likely(req->src == req->dst)) {
		if (all_contig) {
			dst_dma = sg_dma_address(req->src);
		} else {
			dst_dma = src_dma + sizeof(struct sec4_sg_entry) *
				  ((edesc->assoc_nents ? : 1) + 1);
			out_options = LDST_SGF;
		}
	} else {
		if (!edesc->dst_nents) {
			dst_dma = sg_dma_address(req->dst);
		} else {
			dst_dma = edesc->sec4_sg_dma +
				  sec4_sg_index *
				  sizeof(struct sec4_sg_entry);
			out_options = LDST_SGF;
		}
	}
	if (encrypt)
		append_seq_out_ptr(desc, dst_dma, req->cryptlen + authsize,
				   out_options);
	else
		append_seq_out_ptr(desc, dst_dma, req->cryptlen - authsize,
				   out_options);
}

/*
 * Fill in aead givencrypt job descriptor
 */
static void init_aead_giv_job(u32 *sh_desc, dma_addr_t ptr,
			      struct aead_edesc *edesc,
			      struct aead_request *req,
			      int contig)
{
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	int ivsize = crypto_aead_ivsize(aead);
	int authsize = ctx->authsize;
	u32 *desc = edesc->hw_desc;
	u32 out_options = 0, in_options;
	dma_addr_t dst_dma, src_dma;
	int len, sec4_sg_index = 0;
	bool is_gcm = false;

#ifdef DEBUG
	debug("assoclen %d cryptlen %d authsize %d\n",
	      req->assoclen, req->cryptlen, authsize);
	print_hex_dump(KERN_ERR, "assoc  @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->assoc),
		       req->assoclen , 1);
	print_hex_dump(KERN_ERR, "presciv@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, req->iv, ivsize, 1);
	print_hex_dump(KERN_ERR, "src    @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->src),
			edesc->src_nents > 1 ? 100 : req->cryptlen, 1);
	print_hex_dump(KERN_ERR, "shrdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sh_desc,
		       desc_bytes(sh_desc), 1);
#endif

	if (((ctx->class1_alg_type & OP_ALG_ALGSEL_MASK) ==
	      OP_ALG_ALGSEL_AES) &&
	    ((ctx->class1_alg_type & OP_ALG_AAI_MASK) == OP_ALG_AAI_GCM))
		is_gcm = true;

	len = desc_len(sh_desc);
	init_job_desc_shared(desc, ptr, len, HDR_SHARE_DEFER | HDR_REVERSE);

	if (contig & GIV_SRC_CONTIG) {
		if (is_gcm)
			src_dma = edesc->iv_dma;
		else
			src_dma = sg_dma_address(req->assoc);
		in_options = 0;
	} else {
		src_dma = edesc->sec4_sg_dma;
		sec4_sg_index += edesc->assoc_nents + 1 + edesc->src_nents;
		in_options = LDST_SGF;
	}
	append_seq_in_ptr(desc, src_dma, req->assoclen + ivsize + req->cryptlen,
			  in_options);

	if (contig & GIV_DST_CONTIG) {
		dst_dma = edesc->iv_dma;
	} else {
		if (likely(req->src == req->dst)) {
			dst_dma = src_dma + sizeof(struct sec4_sg_entry) *
				  (edesc->assoc_nents +
				   (is_gcm ? 1 + edesc->src_nents : 0));
			out_options = LDST_SGF;
		} else {
			dst_dma = edesc->sec4_sg_dma +
				  sec4_sg_index *
				  sizeof(struct sec4_sg_entry);
			out_options = LDST_SGF;
		}
	}

	append_seq_out_ptr(desc, dst_dma, ivsize + req->cryptlen + authsize,
			   out_options);
}

/*
 * Fill in ablkcipher job descriptor
 */
static void init_ablkcipher_job(u32 *sh_desc, dma_addr_t ptr,
				struct ablkcipher_edesc *edesc,
				struct ablkcipher_request *req,
				bool iv_contig)
{
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	int ivsize = crypto_ablkcipher_ivsize(ablkcipher);
	u32 *desc = edesc->hw_desc;
	u32 out_options = 0, in_options;
	dma_addr_t dst_dma, src_dma;
	int len, sec4_sg_index = 0;

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "presciv@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, req->info,
		       ivsize, 1);
	print_hex_dump(KERN_ERR, "src    @"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->src),
		       edesc->src_nents ? 100 : req->nbytes, 1);
#endif

	len = desc_len(sh_desc);
	init_job_desc_shared(desc, ptr, len, HDR_SHARE_DEFER | HDR_REVERSE);

	if (iv_contig) {
		src_dma = edesc->iv_dma;
		in_options = 0;
	} else {
		src_dma = edesc->sec4_sg_dma;
		sec4_sg_index += edesc->src_nents + 1;
		in_options = LDST_SGF;
	}
	append_seq_in_ptr(desc, src_dma, req->nbytes + ivsize, in_options);

	if (likely(req->src == req->dst)) {
		if (!edesc->src_nents && iv_contig) {
			dst_dma = sg_dma_address(req->src);
		} else {
			dst_dma = edesc->sec4_sg_dma +
				sizeof(struct sec4_sg_entry);
			out_options = LDST_SGF;
		}
	} else {
		if (!edesc->dst_nents) {
			dst_dma = sg_dma_address(req->dst);
		} else {
			dst_dma = edesc->sec4_sg_dma +
				sec4_sg_index * sizeof(struct sec4_sg_entry);
			out_options = LDST_SGF;
		}
	}
	append_seq_out_ptr(desc, dst_dma, req->nbytes, out_options);
}

/*
 * Fill in ablkcipher givencrypt job descriptor
 */
static void init_ablkcipher_giv_job(u32 *sh_desc, dma_addr_t ptr,
				    struct ablkcipher_edesc *edesc,
				    struct ablkcipher_request *req,
				    bool iv_contig)
{
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	int ivsize = crypto_ablkcipher_ivsize(ablkcipher);
	u32 *desc = edesc->hw_desc;
	u32 out_options, in_options;
	dma_addr_t dst_dma, src_dma;
	int len, sec4_sg_index = 0;

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "presciv@" __stringify(__LINE__) ": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, req->info,
		       ivsize, 1);
	print_hex_dump(KERN_ERR, "src    @" __stringify(__LINE__) ": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->src),
		       edesc->src_nents ? 100 : req->nbytes, 1);
#endif

	len = desc_len(sh_desc);
	init_job_desc_shared(desc, ptr, len, HDR_SHARE_DEFER | HDR_REVERSE);

	if (!edesc->src_nents) {
		src_dma = sg_dma_address(req->src);
		in_options = 0;
	} else {
		src_dma = edesc->sec4_sg_dma;
		sec4_sg_index += edesc->src_nents;
		in_options = LDST_SGF;
	}
	append_seq_in_ptr(desc, src_dma, req->nbytes, in_options);

	if (iv_contig) {
		dst_dma = edesc->iv_dma;
		out_options = 0;
	} else {
		dst_dma = edesc->sec4_sg_dma +
			  sec4_sg_index * sizeof(struct sec4_sg_entry);
		out_options = LDST_SGF;
	}
	append_seq_out_ptr(desc, dst_dma, req->nbytes + ivsize, out_options);
}

/*
 * allocate and map the aead extended descriptor
 */
static struct aead_edesc *aead_edesc_alloc(struct aead_request *req,
					   int desc_bytes, bool *all_contig_ptr,
					   bool encrypt)
{
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	gfp_t flags = (req->base.flags & (CRYPTO_TFM_REQ_MAY_BACKLOG |
		       CRYPTO_TFM_REQ_MAY_SLEEP)) ? GFP_KERNEL : GFP_ATOMIC;
	int assoc_nents, src_nents, dst_nents = 0;
	struct aead_edesc *edesc;
	dma_addr_t iv_dma = 0;
	int sgc;
	bool all_contig = true;
	bool assoc_chained = false, src_chained = false, dst_chained = false;
	int ivsize = crypto_aead_ivsize(aead);
	int sec4_sg_index, sec4_sg_len = 0, sec4_sg_bytes;
	unsigned int authsize = ctx->authsize;
	bool is_gcm = false;

	assoc_nents = sg_count(req->assoc, req->assoclen, &assoc_chained);

	if (unlikely(req->dst != req->src)) {
		src_nents = sg_count(req->src, req->cryptlen, &src_chained);
		dst_nents = sg_count(req->dst,
				     req->cryptlen +
					(encrypt ? authsize : (-authsize)),
				     &dst_chained);
	} else {
		src_nents = sg_count(req->src,
				     req->cryptlen +
					(encrypt ? authsize : 0),
				     &src_chained);
	}

	sgc = dma_map_sg_chained(jrdev, req->assoc, assoc_nents ? : 1,
				 DMA_TO_DEVICE, assoc_chained);
	if (likely(req->src == req->dst)) {
		sgc = dma_map_sg_chained(jrdev, req->src, src_nents ? : 1,
					 DMA_BIDIRECTIONAL, src_chained);
	} else {
		sgc = dma_map_sg_chained(jrdev, req->src, src_nents ? : 1,
					 DMA_TO_DEVICE, src_chained);
		sgc = dma_map_sg_chained(jrdev, req->dst, dst_nents ? : 1,
					 DMA_FROM_DEVICE, dst_chained);
	}

	iv_dma = dma_map_single(jrdev, req->iv, ivsize, DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, iv_dma)) {
		dev_err(jrdev, "unable to map IV\n");
		return ERR_PTR(-ENOMEM);
	}

	if (((ctx->class1_alg_type & OP_ALG_ALGSEL_MASK) ==
	      OP_ALG_ALGSEL_AES) &&
	    ((ctx->class1_alg_type & OP_ALG_AAI_MASK) == OP_ALG_AAI_GCM))
		is_gcm = true;

	/*
	 * Check if data are contiguous.
	 * GCM expected input sequence: IV, AAD, text
	 * All other - expected input sequence: AAD, IV, text
	 */
	if (is_gcm)
		all_contig = (!assoc_nents &&
			      iv_dma + ivsize == sg_dma_address(req->assoc) &&
			      !src_nents && sg_dma_address(req->assoc) +
			      req->assoclen == sg_dma_address(req->src));
	else
		all_contig = (!assoc_nents && sg_dma_address(req->assoc) +
			      req->assoclen == iv_dma && !src_nents &&
			      iv_dma + ivsize == sg_dma_address(req->src));
	if (!all_contig) {
		assoc_nents = assoc_nents ? : 1;
		src_nents = src_nents ? : 1;
		sec4_sg_len = assoc_nents + 1 + src_nents;
	}

	sec4_sg_len += dst_nents;

	sec4_sg_bytes = sec4_sg_len * sizeof(struct sec4_sg_entry);

	/* allocate space for base edesc and hw desc commands, link tables */
	edesc = kmalloc(sizeof(struct aead_edesc) + desc_bytes +
			sec4_sg_bytes, GFP_DMA | flags);
	if (!edesc) {
		dev_err(jrdev, "could not allocate extended descriptor\n");
		return ERR_PTR(-ENOMEM);
	}

	edesc->assoc_nents = assoc_nents;
	edesc->assoc_chained = assoc_chained;
	edesc->src_nents = src_nents;
	edesc->src_chained = src_chained;
	edesc->dst_nents = dst_nents;
	edesc->dst_chained = dst_chained;
	edesc->iv_dma = iv_dma;
	edesc->sec4_sg_bytes = sec4_sg_bytes;
	edesc->sec4_sg = (void *)edesc + sizeof(struct aead_edesc) +
			 desc_bytes;
	*all_contig_ptr = all_contig;

	sec4_sg_index = 0;
	if (!all_contig) {
		if (!is_gcm) {
			sg_to_sec4_sg(req->assoc,
				      assoc_nents,
				      edesc->sec4_sg +
				      sec4_sg_index, 0);
			sec4_sg_index += assoc_nents;
		}

		dma_to_sec4_sg_one(edesc->sec4_sg + sec4_sg_index,
				   iv_dma, ivsize, 0);
		sec4_sg_index += 1;

		if (is_gcm) {
			sg_to_sec4_sg(req->assoc,
				      assoc_nents,
				      edesc->sec4_sg +
				      sec4_sg_index, 0);
			sec4_sg_index += assoc_nents;
		}

		sg_to_sec4_sg_last(req->src,
				   src_nents,
				   edesc->sec4_sg +
				   sec4_sg_index, 0);
		sec4_sg_index += src_nents;
	}
	if (dst_nents) {
		sg_to_sec4_sg_last(req->dst, dst_nents,
				   edesc->sec4_sg + sec4_sg_index, 0);
	}
	edesc->sec4_sg_dma = dma_map_single(jrdev, edesc->sec4_sg,
					    sec4_sg_bytes, DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, edesc->sec4_sg_dma)) {
		dev_err(jrdev, "unable to map S/G table\n");
		return ERR_PTR(-ENOMEM);
	}

	return edesc;
}

static int aead_encrypt(struct aead_request *req)
{
	struct aead_edesc *edesc;
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	bool all_contig;
	u32 *desc;
	int ret = 0;

	/* allocate extended descriptor */
	edesc = aead_edesc_alloc(req, DESC_JOB_IO_LEN *
				 CAAM_CMD_SZ, &all_contig, true);
	if (IS_ERR(edesc))
		return PTR_ERR(edesc);

	/* Create and submit job descriptor */
	init_aead_job(ctx->sh_desc_enc, ctx->sh_desc_enc_dma, edesc, req,
		      all_contig, true);
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "aead jobdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, edesc->hw_desc,
		       desc_bytes(edesc->hw_desc), 1);
#endif

	desc = edesc->hw_desc;
	ret = caam_jr_enqueue(jrdev, desc, aead_encrypt_done, req);
	if (!ret) {
		ret = -EINPROGRESS;
	} else {
		aead_unmap(jrdev, edesc, req);
		kfree(edesc);
	}

	return ret;
}

static int aead_decrypt(struct aead_request *req)
{
	struct aead_edesc *edesc;
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	bool all_contig;
	u32 *desc;
	int ret = 0;

	/* allocate extended descriptor */
	edesc = aead_edesc_alloc(req, DESC_JOB_IO_LEN *
				 CAAM_CMD_SZ, &all_contig, false);
	if (IS_ERR(edesc))
		return PTR_ERR(edesc);

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "dec src@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->src),
		       req->cryptlen, 1);
#endif

	/* Create and submit job descriptor*/
	init_aead_job(ctx->sh_desc_dec,
		      ctx->sh_desc_dec_dma, edesc, req, all_contig, false);
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "aead jobdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, edesc->hw_desc,
		       desc_bytes(edesc->hw_desc), 1);
#endif

	desc = edesc->hw_desc;
	ret = caam_jr_enqueue(jrdev, desc, aead_decrypt_done, req);
	if (!ret) {
		ret = -EINPROGRESS;
	} else {
		aead_unmap(jrdev, edesc, req);
		kfree(edesc);
	}

	return ret;
}

/*
 * allocate and map the aead extended descriptor for aead givencrypt
 */
static struct aead_edesc *aead_giv_edesc_alloc(struct aead_givcrypt_request
					       *greq, int desc_bytes,
					       u32 *contig_ptr)
{
	struct aead_request *req = &greq->areq;
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	gfp_t flags = (req->base.flags & (CRYPTO_TFM_REQ_MAY_BACKLOG |
		       CRYPTO_TFM_REQ_MAY_SLEEP)) ? GFP_KERNEL : GFP_ATOMIC;
	int assoc_nents, src_nents, dst_nents = 0;
	struct aead_edesc *edesc;
	dma_addr_t iv_dma = 0;
	int sgc;
	u32 contig = GIV_SRC_CONTIG | GIV_DST_CONTIG;
	int ivsize = crypto_aead_ivsize(aead);
	bool assoc_chained = false, src_chained = false, dst_chained = false;
	int sec4_sg_index, sec4_sg_len = 0, sec4_sg_bytes;
	bool is_gcm = false;

	assoc_nents = sg_count(req->assoc, req->assoclen, &assoc_chained);
	src_nents = sg_count(req->src, req->cryptlen, &src_chained);

	if (unlikely(req->dst != req->src))
		dst_nents = sg_count(req->dst, req->cryptlen + ctx->authsize,
				     &dst_chained);

	sgc = dma_map_sg_chained(jrdev, req->assoc, assoc_nents ? : 1,
				 DMA_TO_DEVICE, assoc_chained);
	if (likely(req->src == req->dst)) {
		sgc = dma_map_sg_chained(jrdev, req->src, src_nents ? : 1,
					 DMA_BIDIRECTIONAL, src_chained);
	} else {
		sgc = dma_map_sg_chained(jrdev, req->src, src_nents ? : 1,
					 DMA_TO_DEVICE, src_chained);
		sgc = dma_map_sg_chained(jrdev, req->dst, dst_nents ? : 1,
					 DMA_FROM_DEVICE, dst_chained);
	}

	iv_dma = dma_map_single(jrdev, greq->giv, ivsize, DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, iv_dma)) {
		dev_err(jrdev, "unable to map IV\n");
		return ERR_PTR(-ENOMEM);
	}

	if (((ctx->class1_alg_type & OP_ALG_ALGSEL_MASK) ==
	      OP_ALG_ALGSEL_AES) &&
	    ((ctx->class1_alg_type & OP_ALG_AAI_MASK) == OP_ALG_AAI_GCM))
		is_gcm = true;

	/*
	 * Check if data are contiguous.
	 * GCM expected input sequence: IV, AAD, text
	 * All other - expected input sequence: AAD, IV, text
	 */

	if (is_gcm) {
		if (assoc_nents || iv_dma + ivsize !=
		    sg_dma_address(req->assoc) || src_nents ||
		    sg_dma_address(req->assoc) + req->assoclen !=
		    sg_dma_address(req->src))
			contig &= ~GIV_SRC_CONTIG;
	} else {
		if (assoc_nents ||
		    sg_dma_address(req->assoc) + req->assoclen != iv_dma ||
		    src_nents || iv_dma + ivsize != sg_dma_address(req->src))
			contig &= ~GIV_SRC_CONTIG;
	}

	if (dst_nents || iv_dma + ivsize != sg_dma_address(req->dst))
		contig &= ~GIV_DST_CONTIG;

	if (!(contig & GIV_SRC_CONTIG)) {
		assoc_nents = assoc_nents ? : 1;
		src_nents = src_nents ? : 1;
		sec4_sg_len += assoc_nents + 1 + src_nents;
		if (req->src == req->dst &&
		    (src_nents || iv_dma + ivsize != sg_dma_address(req->src)))
			contig &= ~GIV_DST_CONTIG;
	}

	/*
	 * Add new sg entries for GCM output sequence.
	 * Expected output sequence: IV, encrypted text.
	 */
	if (is_gcm && req->src == req->dst && !(contig & GIV_DST_CONTIG))
		sec4_sg_len += 1 + src_nents;

	if (unlikely(req->src != req->dst)) {
		dst_nents = dst_nents ? : 1;
		sec4_sg_len += 1 + dst_nents;
	}

	sec4_sg_bytes = sec4_sg_len * sizeof(struct sec4_sg_entry);

	/* allocate space for base edesc and hw desc commands, link tables */
	edesc = kmalloc(sizeof(struct aead_edesc) + desc_bytes +
			sec4_sg_bytes, GFP_DMA | flags);
	if (!edesc) {
		dev_err(jrdev, "could not allocate extended descriptor\n");
		return ERR_PTR(-ENOMEM);
	}

	edesc->assoc_nents = assoc_nents;
	edesc->assoc_chained = assoc_chained;
	edesc->src_nents = src_nents;
	edesc->src_chained = src_chained;
	edesc->dst_nents = dst_nents;
	edesc->dst_chained = dst_chained;
	edesc->iv_dma = iv_dma;
	edesc->sec4_sg_bytes = sec4_sg_bytes;
	edesc->sec4_sg = (void *)edesc + sizeof(struct aead_edesc) +
			 desc_bytes;
	*contig_ptr = contig;

	sec4_sg_index = 0;
	if (!(contig & GIV_SRC_CONTIG)) {
		if (!is_gcm) {
			sg_to_sec4_sg(req->assoc, assoc_nents,
				      edesc->sec4_sg + sec4_sg_index, 0);
			sec4_sg_index += assoc_nents;
		}

		dma_to_sec4_sg_one(edesc->sec4_sg + sec4_sg_index,
				   iv_dma, ivsize, 0);
		sec4_sg_index += 1;

		if (is_gcm) {
			sg_to_sec4_sg(req->assoc, assoc_nents,
				      edesc->sec4_sg + sec4_sg_index, 0);
			sec4_sg_index += assoc_nents;
		}

		sg_to_sec4_sg_last(req->src, src_nents,
				   edesc->sec4_sg +
				   sec4_sg_index, 0);
		sec4_sg_index += src_nents;
	}

	if (is_gcm && req->src == req->dst && !(contig & GIV_DST_CONTIG)) {
		dma_to_sec4_sg_one(edesc->sec4_sg + sec4_sg_index,
				   iv_dma, ivsize, 0);
		sec4_sg_index += 1;
		sg_to_sec4_sg_last(req->src, src_nents,
				   edesc->sec4_sg + sec4_sg_index, 0);
	}

	if (unlikely(req->src != req->dst && !(contig & GIV_DST_CONTIG))) {
		dma_to_sec4_sg_one(edesc->sec4_sg + sec4_sg_index,
				   iv_dma, ivsize, 0);
		sec4_sg_index += 1;
		sg_to_sec4_sg_last(req->dst, dst_nents,
				   edesc->sec4_sg + sec4_sg_index, 0);
	}
	edesc->sec4_sg_dma = dma_map_single(jrdev, edesc->sec4_sg,
					    sec4_sg_bytes, DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, edesc->sec4_sg_dma)) {
		dev_err(jrdev, "unable to map S/G table\n");
		return ERR_PTR(-ENOMEM);
	}

	return edesc;
}

static int aead_givencrypt(struct aead_givcrypt_request *areq)
{
	struct aead_request *req = &areq->areq;
	struct aead_edesc *edesc;
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct caam_ctx *ctx = crypto_aead_ctx(aead);
	struct device *jrdev = ctx->jrdev;
	u32 contig;
	u32 *desc;
	int ret = 0;

	/* allocate extended descriptor */
	edesc = aead_giv_edesc_alloc(areq, DESC_JOB_IO_LEN *
				     CAAM_CMD_SZ, &contig);

	if (IS_ERR(edesc))
		return PTR_ERR(edesc);

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "giv src@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sg_virt(req->src),
		       req->cryptlen, 1);
#endif

	/* Create and submit job descriptor*/
	init_aead_giv_job(ctx->sh_desc_givenc,
			  ctx->sh_desc_givenc_dma, edesc, req, contig);
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "aead jobdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, edesc->hw_desc,
		       desc_bytes(edesc->hw_desc), 1);
#endif

	desc = edesc->hw_desc;
	ret = caam_jr_enqueue(jrdev, desc, aead_encrypt_done, req);
	if (!ret) {
		ret = -EINPROGRESS;
	} else {
		aead_unmap(jrdev, edesc, req);
		kfree(edesc);
	}

	return ret;
}

static int aead_null_givencrypt(struct aead_givcrypt_request *areq)
{
	return aead_encrypt(&areq->areq);
}

/*
 * allocate and map the ablkcipher extended descriptor for ablkcipher
 */
static struct ablkcipher_edesc *ablkcipher_edesc_alloc(struct ablkcipher_request
						       *req, int desc_bytes,
						       bool *iv_contig_out)
{
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	struct caam_ctx *ctx = crypto_ablkcipher_ctx(ablkcipher);
	struct device *jrdev = ctx->jrdev;
	gfp_t flags = (req->base.flags & (CRYPTO_TFM_REQ_MAY_BACKLOG |
					  CRYPTO_TFM_REQ_MAY_SLEEP)) ?
		       GFP_KERNEL : GFP_ATOMIC;
	int src_nents, dst_nents = 0, sec4_sg_bytes;
	struct ablkcipher_edesc *edesc;
	dma_addr_t iv_dma = 0;
	bool iv_contig = false;
	int sgc;
	int ivsize = crypto_ablkcipher_ivsize(ablkcipher);
	bool src_chained = false, dst_chained = false;
	int sec4_sg_index;

	src_nents = sg_count(req->src, req->nbytes, &src_chained);

	if (req->dst != req->src)
		dst_nents = sg_count(req->dst, req->nbytes, &dst_chained);

	if (likely(req->src == req->dst)) {
		sgc = dma_map_sg_chained(jrdev, req->src, src_nents ? : 1,
					 DMA_BIDIRECTIONAL, src_chained);
	} else {
		sgc = dma_map_sg_chained(jrdev, req->src, src_nents ? : 1,
					 DMA_TO_DEVICE, src_chained);
		sgc = dma_map_sg_chained(jrdev, req->dst, dst_nents ? : 1,
					 DMA_FROM_DEVICE, dst_chained);
	}

	iv_dma = dma_map_single(jrdev, req->info, ivsize, DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, iv_dma)) {
		dev_err(jrdev, "unable to map IV\n");
		return ERR_PTR(-ENOMEM);
	}

	/*
	 * Check if iv can be contiguous with source and destination.
	 * If so, include it. If not, create scatterlist.
	 */
	if (!src_nents && iv_dma + ivsize == sg_dma_address(req->src))
		iv_contig = true;
	else
		src_nents = src_nents ? : 1;
	sec4_sg_bytes = ((iv_contig ? 0 : 1) + src_nents + dst_nents) *
			sizeof(struct sec4_sg_entry);

	/* allocate space for base edesc and hw desc commands, link tables */
	edesc = kmalloc(sizeof(struct ablkcipher_edesc) + desc_bytes +
			sec4_sg_bytes, GFP_DMA | flags);
	if (!edesc) {
		dev_err(jrdev, "could not allocate extended descriptor\n");
		return ERR_PTR(-ENOMEM);
	}

	edesc->src_nents = src_nents;
	edesc->src_chained = src_chained;
	edesc->dst_nents = dst_nents;
	edesc->dst_chained = dst_chained;
	edesc->sec4_sg_bytes = sec4_sg_bytes;
	edesc->sec4_sg = (void *)edesc + sizeof(struct ablkcipher_edesc) +
			 desc_bytes;

	sec4_sg_index = 0;
	if (!iv_contig) {
		dma_to_sec4_sg_one(edesc->sec4_sg, iv_dma, ivsize, 0);
		sg_to_sec4_sg_last(req->src, src_nents,
				   edesc->sec4_sg + 1, 0);
		sec4_sg_index += 1 + src_nents;
	}

	if (dst_nents) {
		sg_to_sec4_sg_last(req->dst, dst_nents,
			edesc->sec4_sg + sec4_sg_index, 0);
	}

	edesc->sec4_sg_dma = dma_map_single(jrdev, edesc->sec4_sg,
					    sec4_sg_bytes, DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, edesc->sec4_sg_dma)) {
		dev_err(jrdev, "unable to map S/G table\n");
		return ERR_PTR(-ENOMEM);
	}

	edesc->iv_dma = iv_dma;

#ifdef DEBUG
	print_hex_dump(KERN_ERR, "ablkcipher sec4_sg@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, edesc->sec4_sg,
		       sec4_sg_bytes, 1);
#endif

	*iv_contig_out = iv_contig;
	return edesc;
}

static int ablkcipher_encrypt(struct ablkcipher_request *req)
{
	struct ablkcipher_edesc *edesc;
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	struct caam_ctx *ctx = crypto_ablkcipher_ctx(ablkcipher);
	struct device *jrdev = ctx->jrdev;
	bool iv_contig;
	u32 *desc;
	int ret = 0;

	/* allocate extended descriptor */
	edesc = ablkcipher_edesc_alloc(req, DESC_JOB_IO_LEN *
				       CAAM_CMD_SZ, &iv_contig);
	if (IS_ERR(edesc))
		return PTR_ERR(edesc);

	/* Create and submit job descriptor*/
	init_ablkcipher_job(ctx->sh_desc_enc,
		ctx->sh_desc_enc_dma, edesc, req, iv_contig);
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "ablkcipher jobdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, edesc->hw_desc,
		       desc_bytes(edesc->hw_desc), 1);
#endif
	desc = edesc->hw_desc;
	ret = caam_jr_enqueue(jrdev, desc, ablkcipher_encrypt_done, req);

	if (!ret) {
		ret = -EINPROGRESS;
	} else {
		ablkcipher_unmap(jrdev, edesc, req);
		kfree(edesc);
	}

	return ret;
}

static int ablkcipher_decrypt(struct ablkcipher_request *req)
{
	struct ablkcipher_edesc *edesc;
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	struct caam_ctx *ctx = crypto_ablkcipher_ctx(ablkcipher);
	struct device *jrdev = ctx->jrdev;
	bool iv_contig;
	u32 *desc;
	int ret = 0;

	/* allocate extended descriptor */
	edesc = ablkcipher_edesc_alloc(req, DESC_JOB_IO_LEN *
				       CAAM_CMD_SZ, &iv_contig);
	if (IS_ERR(edesc))
		return PTR_ERR(edesc);

	/* Create and submit job descriptor*/
	init_ablkcipher_job(ctx->sh_desc_dec,
		ctx->sh_desc_dec_dma, edesc, req, iv_contig);
	desc = edesc->hw_desc;
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "ablkcipher jobdesc@"__stringify(__LINE__)": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, edesc->hw_desc,
		       desc_bytes(edesc->hw_desc), 1);
#endif

	ret = caam_jr_enqueue(jrdev, desc, ablkcipher_decrypt_done, req);
	if (!ret) {
		ret = -EINPROGRESS;
	} else {
		ablkcipher_unmap(jrdev, edesc, req);
		kfree(edesc);
	}

	return ret;
}

/*
 * allocate and map the ablkcipher extended descriptor
 * for ablkcipher givencrypt
 */
static struct ablkcipher_edesc *ablkcipher_giv_edesc_alloc(
				struct skcipher_givcrypt_request *greq,
				int desc_bytes,
				bool *iv_contig_out)
{
	struct ablkcipher_request *req = &greq->creq;
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	struct caam_ctx *ctx = crypto_ablkcipher_ctx(ablkcipher);
	struct device *jrdev = ctx->jrdev;
	gfp_t flags = (req->base.flags & (CRYPTO_TFM_REQ_MAY_BACKLOG |
					  CRYPTO_TFM_REQ_MAY_SLEEP)) ?
		       GFP_KERNEL : GFP_ATOMIC;
	int src_nents, dst_nents = 0, sec4_sg_bytes;
	struct ablkcipher_edesc *edesc;
	dma_addr_t iv_dma = 0;
	bool iv_contig = false;
	int sgc;
	int ivsize = crypto_ablkcipher_ivsize(ablkcipher);
	bool src_chained = false, dst_chained = false;
	int sec4_sg_index;

	src_nents = sg_count(req->src, req->nbytes, &src_chained);

	if (unlikely(req->dst != req->src))
		dst_nents = sg_count(req->dst, req->nbytes, &dst_chained);

	if (likely(req->src == req->dst)) {
		sgc = dma_map_sg_chained(jrdev, req->src, src_nents ? : 1,
					 DMA_BIDIRECTIONAL, src_chained);
	} else {
		sgc = dma_map_sg_chained(jrdev, req->src, src_nents ? : 1,
					 DMA_TO_DEVICE, src_chained);
		sgc = dma_map_sg_chained(jrdev, req->dst, dst_nents ? : 1,
					 DMA_FROM_DEVICE, dst_chained);
	}

	/*
	 * Check if iv can be contiguous with source and destination.
	 * If so, include it. If not, create scatterlist.
	 */
	iv_dma = dma_map_single(jrdev, greq->giv, ivsize, DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, iv_dma)) {
		dev_err(jrdev, "unable to map IV\n");
		return ERR_PTR(-ENOMEM);
	}

	if (!dst_nents && iv_dma + ivsize == sg_dma_address(req->dst))
		iv_contig = true;
	else
		dst_nents = dst_nents ? : 1;
	sec4_sg_bytes = ((iv_contig ? 0 : 1) + src_nents + dst_nents) *
			sizeof(struct sec4_sg_entry);

	/* allocate space for base edesc and hw desc commands, link tables */
	edesc = kmalloc(sizeof(*edesc) + desc_bytes +
			sec4_sg_bytes, GFP_DMA | flags);
	if (!edesc) {
		dev_err(jrdev, "could not allocate extended descriptor\n");
		return ERR_PTR(-ENOMEM);
	}

	edesc->src_nents = src_nents;
	edesc->src_chained = src_chained;
	edesc->dst_nents = dst_nents;
	edesc->dst_chained = dst_chained;
	edesc->sec4_sg_bytes = sec4_sg_bytes;
	edesc->sec4_sg = (void *)edesc + sizeof(struct ablkcipher_edesc) +
			 desc_bytes;

	sec4_sg_index = 0;
	if (src_nents) {
		sg_to_sec4_sg_last(req->src, src_nents, edesc->sec4_sg, 0);
		sec4_sg_index += src_nents;
	}

	if (!iv_contig) {
		dma_to_sec4_sg_one(edesc->sec4_sg + sec4_sg_index,
				   iv_dma, ivsize, 0);
		sec4_sg_index += 1;
		sg_to_sec4_sg_last(req->dst, dst_nents,
				   edesc->sec4_sg + sec4_sg_index, 0);
	}

	edesc->sec4_sg_dma = dma_map_single(jrdev, edesc->sec4_sg,
					    sec4_sg_bytes, DMA_TO_DEVICE);
	if (dma_mapping_error(jrdev, edesc->sec4_sg_dma)) {
		dev_err(jrdev, "unable to map S/G table\n");
		return ERR_PTR(-ENOMEM);
	}
	edesc->iv_dma = iv_dma;

#ifdef DEBUG
	print_hex_dump(KERN_ERR,
		       "ablkcipher sec4_sg@" __stringify(__LINE__) ": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, edesc->sec4_sg,
		       sec4_sg_bytes, 1);
#endif

	*iv_contig_out = iv_contig;
	return edesc;
}

static int ablkcipher_givencrypt(struct skcipher_givcrypt_request *creq)
{
	struct ablkcipher_request *req = &creq->creq;
	struct ablkcipher_edesc *edesc;
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	struct caam_ctx *ctx = crypto_ablkcipher_ctx(ablkcipher);
	struct device *jrdev = ctx->jrdev;
	bool iv_contig;
	u32 *desc;
	int ret = 0;

	/* allocate extended descriptor */
	edesc = ablkcipher_giv_edesc_alloc(creq, DESC_JOB_IO_LEN *
				       CAAM_CMD_SZ, &iv_contig);
	if (IS_ERR(edesc))
		return PTR_ERR(edesc);

	/* Create and submit job descriptor*/
	init_ablkcipher_giv_job(ctx->sh_desc_givenc, ctx->sh_desc_givenc_dma,
				edesc, req, iv_contig);
#ifdef DEBUG
	print_hex_dump(KERN_ERR,
		       "ablkcipher jobdesc@" __stringify(__LINE__) ": ",
		       DUMP_PREFIX_ADDRESS, 16, 4, edesc->hw_desc,
		       desc_bytes(edesc->hw_desc), 1);
#endif
	desc = edesc->hw_desc;
	ret = caam_jr_enqueue(jrdev, desc, ablkcipher_encrypt_done, req);

	if (!ret) {
		ret = -EINPROGRESS;
	} else {
		ablkcipher_unmap(jrdev, edesc, req);
		kfree(edesc);
	}

	return ret;
}

#define template_aead		template_u.aead
#define template_ablkcipher	template_u.ablkcipher
struct caam_alg_template {
	char name[CRYPTO_MAX_ALG_NAME];
	char driver_name[CRYPTO_MAX_ALG_NAME];
	unsigned int blocksize;
	u32 type;
	union {
		struct ablkcipher_alg ablkcipher;
		struct aead_alg aead;
		struct blkcipher_alg blkcipher;
		struct cipher_alg cipher;
		struct compress_alg compress;
		struct rng_alg rng;
	} template_u;
	u32 class1_alg_type;
	u32 class2_alg_type;
	u32 alg_op;
};

static struct caam_alg_template driver_algs[] = {
	/* single-pass ipsec_esp descriptor */
	{
		.name = "authenc(hmac(md5),ecb(cipher_null))",
		.driver_name = "authenc-hmac-md5-ecb-cipher_null-caam",
		.blocksize = NULL_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_null_givencrypt,
			.geniv = "<built-in>",
			.ivsize = NULL_IV_SIZE,
			.maxauthsize = MD5_DIGEST_SIZE,
			},
		.class1_alg_type = 0,
		.class2_alg_type = OP_ALG_ALGSEL_MD5 | OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_MD5 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha1),ecb(cipher_null))",
		.driver_name = "authenc-hmac-sha1-ecb-cipher_null-caam",
		.blocksize = NULL_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_null_givencrypt,
			.geniv = "<built-in>",
			.ivsize = NULL_IV_SIZE,
			.maxauthsize = SHA1_DIGEST_SIZE,
			},
		.class1_alg_type = 0,
		.class2_alg_type = OP_ALG_ALGSEL_SHA1 | OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA1 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha224),ecb(cipher_null))",
		.driver_name = "authenc-hmac-sha224-ecb-cipher_null-caam",
		.blocksize = NULL_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_null_givencrypt,
			.geniv = "<built-in>",
			.ivsize = NULL_IV_SIZE,
			.maxauthsize = SHA224_DIGEST_SIZE,
			},
		.class1_alg_type = 0,
		.class2_alg_type = OP_ALG_ALGSEL_SHA224 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA224 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha256),ecb(cipher_null))",
		.driver_name = "authenc-hmac-sha256-ecb-cipher_null-caam",
		.blocksize = NULL_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_null_givencrypt,
			.geniv = "<built-in>",
			.ivsize = NULL_IV_SIZE,
			.maxauthsize = SHA256_DIGEST_SIZE,
			},
		.class1_alg_type = 0,
		.class2_alg_type = OP_ALG_ALGSEL_SHA256 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA256 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha384),ecb(cipher_null))",
		.driver_name = "authenc-hmac-sha384-ecb-cipher_null-caam",
		.blocksize = NULL_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_null_givencrypt,
			.geniv = "<built-in>",
			.ivsize = NULL_IV_SIZE,
			.maxauthsize = SHA384_DIGEST_SIZE,
			},
		.class1_alg_type = 0,
		.class2_alg_type = OP_ALG_ALGSEL_SHA384 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA384 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha512),ecb(cipher_null))",
		.driver_name = "authenc-hmac-sha512-ecb-cipher_null-caam",
		.blocksize = NULL_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_null_givencrypt,
			.geniv = "<built-in>",
			.ivsize = NULL_IV_SIZE,
			.maxauthsize = SHA512_DIGEST_SIZE,
			},
		.class1_alg_type = 0,
		.class2_alg_type = OP_ALG_ALGSEL_SHA512 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA512 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(md5),cbc(aes))",
		.driver_name = "authenc-hmac-md5-cbc-aes-caam",
		.blocksize = AES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = AES_BLOCK_SIZE,
			.maxauthsize = MD5_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_MD5 | OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_MD5 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha1),cbc(aes))",
		.driver_name = "authenc-hmac-sha1-cbc-aes-caam",
		.blocksize = AES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = AES_BLOCK_SIZE,
			.maxauthsize = SHA1_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA1 | OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA1 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha224),cbc(aes))",
		.driver_name = "authenc-hmac-sha224-cbc-aes-caam",
		.blocksize = AES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = AES_BLOCK_SIZE,
			.maxauthsize = SHA224_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA224 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA224 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha256),cbc(aes))",
		.driver_name = "authenc-hmac-sha256-cbc-aes-caam",
		.blocksize = AES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = AES_BLOCK_SIZE,
			.maxauthsize = SHA256_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA256 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA256 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha384),cbc(aes))",
		.driver_name = "authenc-hmac-sha384-cbc-aes-caam",
		.blocksize = AES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = AES_BLOCK_SIZE,
			.maxauthsize = SHA384_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA384 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA384 | OP_ALG_AAI_HMAC,
	},

	{
		.name = "authenc(hmac(sha512),cbc(aes))",
		.driver_name = "authenc-hmac-sha512-cbc-aes-caam",
		.blocksize = AES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = AES_BLOCK_SIZE,
			.maxauthsize = SHA512_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA512 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA512 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(md5),cbc(des3_ede))",
		.driver_name = "authenc-hmac-md5-cbc-des3_ede-caam",
		.blocksize = DES3_EDE_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES3_EDE_BLOCK_SIZE,
			.maxauthsize = MD5_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_3DES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_MD5 | OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_MD5 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha1),cbc(des3_ede))",
		.driver_name = "authenc-hmac-sha1-cbc-des3_ede-caam",
		.blocksize = DES3_EDE_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES3_EDE_BLOCK_SIZE,
			.maxauthsize = SHA1_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_3DES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA1 | OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA1 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha224),cbc(des3_ede))",
		.driver_name = "authenc-hmac-sha224-cbc-des3_ede-caam",
		.blocksize = DES3_EDE_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES3_EDE_BLOCK_SIZE,
			.maxauthsize = SHA224_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_3DES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA224 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA224 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha256),cbc(des3_ede))",
		.driver_name = "authenc-hmac-sha256-cbc-des3_ede-caam",
		.blocksize = DES3_EDE_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES3_EDE_BLOCK_SIZE,
			.maxauthsize = SHA256_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_3DES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA256 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA256 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha384),cbc(des3_ede))",
		.driver_name = "authenc-hmac-sha384-cbc-des3_ede-caam",
		.blocksize = DES3_EDE_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES3_EDE_BLOCK_SIZE,
			.maxauthsize = SHA384_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_3DES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA384 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA384 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha512),cbc(des3_ede))",
		.driver_name = "authenc-hmac-sha512-cbc-des3_ede-caam",
		.blocksize = DES3_EDE_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES3_EDE_BLOCK_SIZE,
			.maxauthsize = SHA512_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_3DES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA512 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA512 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(md5),cbc(des))",
		.driver_name = "authenc-hmac-md5-cbc-des-caam",
		.blocksize = DES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES_BLOCK_SIZE,
			.maxauthsize = MD5_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_DES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_MD5 | OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_MD5 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha1),cbc(des))",
		.driver_name = "authenc-hmac-sha1-cbc-des-caam",
		.blocksize = DES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES_BLOCK_SIZE,
			.maxauthsize = SHA1_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_DES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA1 | OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA1 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha224),cbc(des))",
		.driver_name = "authenc-hmac-sha224-cbc-des-caam",
		.blocksize = DES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES_BLOCK_SIZE,
			.maxauthsize = SHA224_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_DES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA224 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA224 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha256),cbc(des))",
		.driver_name = "authenc-hmac-sha256-cbc-des-caam",
		.blocksize = DES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES_BLOCK_SIZE,
			.maxauthsize = SHA256_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_DES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA256 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA256 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha384),cbc(des))",
		.driver_name = "authenc-hmac-sha384-cbc-des-caam",
		.blocksize = DES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES_BLOCK_SIZE,
			.maxauthsize = SHA384_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_DES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA384 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA384 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha512),cbc(des))",
		.driver_name = "authenc-hmac-sha512-cbc-des-caam",
		.blocksize = DES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES_BLOCK_SIZE,
			.maxauthsize = SHA512_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_DES | OP_ALG_AAI_CBC,
		.class2_alg_type = OP_ALG_ALGSEL_SHA512 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA512 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(md5),rfc3686(ctr(aes)))",
		.driver_name = "authenc-hmac-md5-rfc3686-ctr-aes-caam",
		.blocksize = 1,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = CTR_RFC3686_IV_SIZE,
			.maxauthsize = MD5_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CTR_MOD128,
		.class2_alg_type = OP_ALG_ALGSEL_MD5 | OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_MD5 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha1),rfc3686(ctr(aes)))",
		.driver_name = "authenc-hmac-sha1-rfc3686-ctr-aes-caam",
		.blocksize = 1,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = CTR_RFC3686_IV_SIZE,
			.maxauthsize = SHA1_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CTR_MOD128,
		.class2_alg_type = OP_ALG_ALGSEL_SHA1 | OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA1 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha224),rfc3686(ctr(aes)))",
		.driver_name = "authenc-hmac-sha224-rfc3686-ctr-aes-caam",
		.blocksize = 1,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = CTR_RFC3686_IV_SIZE,
			.maxauthsize = SHA224_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CTR_MOD128,
		.class2_alg_type = OP_ALG_ALGSEL_SHA224 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA224 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha256),rfc3686(ctr(aes)))",
		.driver_name = "authenc-hmac-sha256-rfc3686-ctr-aes-caam",
		.blocksize = 1,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = CTR_RFC3686_IV_SIZE,
			.maxauthsize = SHA256_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CTR_MOD128,
		.class2_alg_type = OP_ALG_ALGSEL_SHA256 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA256 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha384),rfc3686(ctr(aes)))",
		.driver_name = "authenc-hmac-sha384-rfc3686-ctr-aes-caam",
		.blocksize = 1,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = CTR_RFC3686_IV_SIZE,
			.maxauthsize = SHA384_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CTR_MOD128,
		.class2_alg_type = OP_ALG_ALGSEL_SHA384 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA384 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "authenc(hmac(sha512),rfc3686(ctr(aes)))",
		.driver_name = "authenc-hmac-sha512-rfc3686-ctr-aes-caam",
		.blocksize = 1,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = CTR_RFC3686_IV_SIZE,
			.maxauthsize = SHA512_DIGEST_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CTR_MOD128,
		.class2_alg_type = OP_ALG_ALGSEL_SHA512 |
				   OP_ALG_AAI_HMAC_PRECOMP,
		.alg_op = OP_ALG_ALGSEL_SHA512 | OP_ALG_AAI_HMAC,
	},
	{
		.name = "rfc4106(gcm(aes))",
		.driver_name = "rfc4106-gcm-aes-caam",
		.blocksize = 1,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = rfc4106_setkey,
			.setauthsize = rfc4106_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = 8,
			.maxauthsize = AES_BLOCK_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_GCM,
	},
	{
		.name = "rfc4543(gcm(aes))",
		.driver_name = "rfc4543-gcm-aes-caam",
		.blocksize = 1,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = rfc4543_setkey,
			.setauthsize = rfc4543_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = 8,
			.maxauthsize = AES_BLOCK_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_GCM,
	},
	/* Galois Counter Mode */
	{
		.name = "gcm(aes)",
		.driver_name = "gcm-aes-caam",
		.blocksize = 1,
		.type = CRYPTO_ALG_TYPE_AEAD,
		.template_aead = {
			.setkey = gcm_setkey,
			.setauthsize = gcm_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = NULL,
			.geniv = "<built-in>",
			.ivsize = 12,
			.maxauthsize = AES_BLOCK_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_GCM,
	},
	/* ablkcipher descriptor */
	{
		.name = "cbc(aes)",
		.driver_name = "cbc-aes-caam",
		.blocksize = AES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_GIVCIPHER,
		.template_ablkcipher = {
			.setkey = ablkcipher_setkey,
			.encrypt = ablkcipher_encrypt,
			.decrypt = ablkcipher_decrypt,
			.givencrypt = ablkcipher_givencrypt,
			.geniv = "<built-in>",
			.min_keysize = AES_MIN_KEY_SIZE,
			.max_keysize = AES_MAX_KEY_SIZE,
			.ivsize = AES_BLOCK_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CBC,
	},
	{
		.name = "cbc(des3_ede)",
		.driver_name = "cbc-3des-caam",
		.blocksize = DES3_EDE_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_GIVCIPHER,
		.template_ablkcipher = {
			.setkey = ablkcipher_setkey,
			.encrypt = ablkcipher_encrypt,
			.decrypt = ablkcipher_decrypt,
			.givencrypt = ablkcipher_givencrypt,
			.geniv = "<built-in>",
			.min_keysize = DES3_EDE_KEY_SIZE,
			.max_keysize = DES3_EDE_KEY_SIZE,
			.ivsize = DES3_EDE_BLOCK_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_3DES | OP_ALG_AAI_CBC,
	},
	{
		.name = "cbc(des)",
		.driver_name = "cbc-des-caam",
		.blocksize = DES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_GIVCIPHER,
		.template_ablkcipher = {
			.setkey = ablkcipher_setkey,
			.encrypt = ablkcipher_encrypt,
			.decrypt = ablkcipher_decrypt,
			.givencrypt = ablkcipher_givencrypt,
			.geniv = "<built-in>",
			.min_keysize = DES_KEY_SIZE,
			.max_keysize = DES_KEY_SIZE,
			.ivsize = DES_BLOCK_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_DES | OP_ALG_AAI_CBC,
	},
	{
		.name = "ctr(aes)",
		.driver_name = "ctr-aes-caam",
		.blocksize = 1,
		.type = CRYPTO_ALG_TYPE_ABLKCIPHER,
		.template_ablkcipher = {
			.setkey = ablkcipher_setkey,
			.encrypt = ablkcipher_encrypt,
			.decrypt = ablkcipher_decrypt,
			.geniv = "chainiv",
			.min_keysize = AES_MIN_KEY_SIZE,
			.max_keysize = AES_MAX_KEY_SIZE,
			.ivsize = AES_BLOCK_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CTR_MOD128,
	},
	{
		.name = "rfc3686(ctr(aes))",
		.driver_name = "rfc3686-ctr-aes-caam",
		.blocksize = 1,
		.type = CRYPTO_ALG_TYPE_GIVCIPHER,
		.template_ablkcipher = {
			.setkey = ablkcipher_setkey,
			.encrypt = ablkcipher_encrypt,
			.decrypt = ablkcipher_decrypt,
			.givencrypt = ablkcipher_givencrypt,
			.geniv = "<built-in>",
			.min_keysize = AES_MIN_KEY_SIZE +
				       CTR_RFC3686_NONCE_SIZE,
			.max_keysize = AES_MAX_KEY_SIZE +
				       CTR_RFC3686_NONCE_SIZE,
			.ivsize = CTR_RFC3686_IV_SIZE,
			},
		.class1_alg_type = OP_ALG_ALGSEL_AES | OP_ALG_AAI_CTR_MOD128,
	}
};

struct caam_crypto_alg {
	struct list_head entry;
	int class1_alg_type;
	int class2_alg_type;
	int alg_op;
	struct crypto_alg crypto_alg;
};

static int caam_cra_init(struct crypto_tfm *tfm)
{
	struct crypto_alg *alg = tfm->__crt_alg;
	struct caam_crypto_alg *caam_alg =
		 container_of(alg, struct caam_crypto_alg, crypto_alg);
	struct caam_ctx *ctx = crypto_tfm_ctx(tfm);

	ctx->jrdev = caam_jr_alloc();
	if (IS_ERR(ctx->jrdev)) {
		pr_err("Job Ring Device allocation for transform failed\n");
		return PTR_ERR(ctx->jrdev);
	}

	/* copy descriptor header template value */
	ctx->class1_alg_type = OP_TYPE_CLASS1_ALG | caam_alg->class1_alg_type;
	ctx->class2_alg_type = OP_TYPE_CLASS2_ALG | caam_alg->class2_alg_type;
	ctx->alg_op = OP_TYPE_CLASS2_ALG | caam_alg->alg_op;

	return 0;
}

static void caam_cra_exit(struct crypto_tfm *tfm)
{
	struct caam_ctx *ctx = crypto_tfm_ctx(tfm);

	if (ctx->sh_desc_enc_dma &&
	    !dma_mapping_error(ctx->jrdev, ctx->sh_desc_enc_dma))
		dma_unmap_single(ctx->jrdev, ctx->sh_desc_enc_dma,
				 desc_bytes(ctx->sh_desc_enc), DMA_TO_DEVICE);
	if (ctx->sh_desc_dec_dma &&
	    !dma_mapping_error(ctx->jrdev, ctx->sh_desc_dec_dma))
		dma_unmap_single(ctx->jrdev, ctx->sh_desc_dec_dma,
				 desc_bytes(ctx->sh_desc_dec), DMA_TO_DEVICE);
	if (ctx->sh_desc_givenc_dma &&
	    !dma_mapping_error(ctx->jrdev, ctx->sh_desc_givenc_dma))
		dma_unmap_single(ctx->jrdev, ctx->sh_desc_givenc_dma,
				 desc_bytes(ctx->sh_desc_givenc),
				 DMA_TO_DEVICE);
	if (ctx->key_dma &&
	    !dma_mapping_error(ctx->jrdev, ctx->key_dma))
		dma_unmap_single(ctx->jrdev, ctx->key_dma,
				 ctx->enckeylen + ctx->split_key_pad_len,
				 DMA_TO_DEVICE);

	caam_jr_free(ctx->jrdev);
}

static void __exit caam_algapi_exit(void)
{

	struct caam_crypto_alg *t_alg, *n;

	if (!alg_list.next)
		return;

	list_for_each_entry_safe(t_alg, n, &alg_list, entry) {
		crypto_unregister_alg(&t_alg->crypto_alg);
		list_del(&t_alg->entry);
		kfree(t_alg);
	}
}

static struct caam_crypto_alg *caam_alg_alloc(struct caam_alg_template
					      *template)
{
	struct caam_crypto_alg *t_alg;
	struct crypto_alg *alg;

	t_alg = kzalloc(sizeof(struct caam_crypto_alg), GFP_KERNEL);
	if (!t_alg) {
		pr_err("failed to allocate t_alg\n");
		return ERR_PTR(-ENOMEM);
	}

	alg = &t_alg->crypto_alg;

	snprintf(alg->cra_name, CRYPTO_MAX_ALG_NAME, "%s", template->name);
	snprintf(alg->cra_driver_name, CRYPTO_MAX_ALG_NAME, "%s",
		 template->driver_name);
	alg->cra_module = THIS_MODULE;
	alg->cra_init = caam_cra_init;
	alg->cra_exit = caam_cra_exit;
	alg->cra_priority = CAAM_CRA_PRIORITY;
	alg->cra_blocksize = template->blocksize;
	alg->cra_alignmask = 0;
	alg->cra_ctxsize = sizeof(struct caam_ctx);
	alg->cra_flags = CRYPTO_ALG_ASYNC | CRYPTO_ALG_KERN_DRIVER_ONLY |
			 template->type;
	switch (template->type) {
	case CRYPTO_ALG_TYPE_GIVCIPHER:
		alg->cra_type = &crypto_givcipher_type;
		alg->cra_ablkcipher = template->template_ablkcipher;
		break;
	case CRYPTO_ALG_TYPE_ABLKCIPHER:
		alg->cra_type = &crypto_ablkcipher_type;
		alg->cra_ablkcipher = template->template_ablkcipher;
		break;
	case CRYPTO_ALG_TYPE_AEAD:
		alg->cra_type = &crypto_aead_type;
		alg->cra_aead = template->template_aead;
		break;
	}

	t_alg->class1_alg_type = template->class1_alg_type;
	t_alg->class2_alg_type = template->class2_alg_type;
	t_alg->alg_op = template->alg_op;

	return t_alg;
}

static int __init caam_algapi_init(void)
{
	struct device_node *dev_node;
	struct platform_device *pdev;
	struct device *ctrldev;
	void *priv;
	int i = 0, err = 0;

	dev_node = of_find_compatible_node(NULL, NULL, "fsl,sec-v4.0");
	if (!dev_node) {
		dev_node = of_find_compatible_node(NULL, NULL, "fsl,sec4.0");
		if (!dev_node)
			return -ENODEV;
	}

	pdev = of_find_device_by_node(dev_node);
	if (!pdev) {
		of_node_put(dev_node);
		return -ENODEV;
	}

	ctrldev = &pdev->dev;
	priv = dev_get_drvdata(ctrldev);
	of_node_put(dev_node);

	/*
	 * If priv is NULL, it's probably because the caam driver wasn't
	 * properly initialized (e.g. RNG4 init failed). Thus, bail out here.
	 */
	if (!priv)
		return -ENODEV;


	INIT_LIST_HEAD(&alg_list);

	/* register crypto algorithms the device supports */
	for (i = 0; i < ARRAY_SIZE(driver_algs); i++) {
		/* TODO: check if h/w supports alg */
		struct caam_crypto_alg *t_alg;

		t_alg = caam_alg_alloc(&driver_algs[i]);
		if (IS_ERR(t_alg)) {
			err = PTR_ERR(t_alg);
			pr_warn("%s alg allocation failed\n",
				driver_algs[i].driver_name);
			continue;
		}

		err = crypto_register_alg(&t_alg->crypto_alg);
		if (err) {
			pr_warn("%s alg registration failed\n",
				t_alg->crypto_alg.cra_driver_name);
			kfree(t_alg);
		} else
			list_add_tail(&t_alg->entry, &alg_list);
	}
	if (!list_empty(&alg_list))
		pr_info("caam algorithms registered in /proc/crypto\n");

	return err;
}

module_init(caam_algapi_init);
module_exit(caam_algapi_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("FSL CAAM support for crypto API");
MODULE_AUTHOR("Freescale Semiconductor - NMG/STC");
