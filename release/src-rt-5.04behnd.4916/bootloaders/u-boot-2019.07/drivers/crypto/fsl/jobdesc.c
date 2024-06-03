// SPDX-License-Identifier: GPL-2.0+
/*
 * SEC Descriptor Construction Library
 * Basic job descriptor construction
 *
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 */

#include <common.h>
#include <fsl_sec.h>
#include "desc_constr.h"
#include "jobdesc.h"
#include "rsa_caam.h"

#if defined(CONFIG_MX6) || defined(CONFIG_MX7)
/*!
 * Secure memory run command
 *
 * @param   sec_mem_cmd  Secure memory command register
 * @return  cmd_status  Secure memory command status register
 */
uint32_t secmem_set_cmd(uint32_t sec_mem_cmd)
{
	uint32_t temp_reg;

	ccsr_sec_t *sec = (void *)CONFIG_SYS_FSL_SEC_ADDR;
	uint32_t sm_vid = SM_VERSION(sec_in32(&sec->smvid));
	uint32_t jr_id = 0;

	sec_out32(CAAM_SMCJR(sm_vid, jr_id), sec_mem_cmd);

	do {
		temp_reg = sec_in32(CAAM_SMCSJR(sm_vid, jr_id));
	} while (temp_reg & CMD_COMPLETE);

	return temp_reg;
}

/*!
 * CAAM page allocation:
 * Allocates a partition from secure memory, with the id
 * equal to partition_num. This will de-allocate the page
 * if it is already allocated. The partition will have
 * full access permissions. The permissions are set before,
 * running a job descriptor. A memory page of secure RAM
 * is allocated for the partition.
 *
 * @param   page  Number of the page to allocate.
 * @param   partition  Number of the partition to allocate.
 * @return  0 on success, ERROR_IN_PAGE_ALLOC otherwise
 */
int caam_page_alloc(uint8_t page_num, uint8_t partition_num)
{
	uint32_t temp_reg;

	ccsr_sec_t *sec = (void *)CONFIG_SYS_FSL_SEC_ADDR;
	uint32_t sm_vid = SM_VERSION(sec_in32(&sec->smvid));
	uint32_t jr_id = 0;

	/*
	 * De-Allocate partition_num if already allocated to ARM core
	 */
	if (sec_in32(CAAM_SMPO_0) & PARTITION_OWNER(partition_num)) {
		temp_reg = secmem_set_cmd(PARTITION(partition_num) |
						CMD_PART_DEALLOC);
		if (temp_reg & SMCSJR_AERR) {
			printf("Error: De-allocation status 0x%X\n", temp_reg);
			return ERROR_IN_PAGE_ALLOC;
		}
	}

	/* set the access rights to allow full access */
	sec_out32(CAAM_SMAG1JR(sm_vid, jr_id, partition_num), 0xF);
	sec_out32(CAAM_SMAG2JR(sm_vid, jr_id, partition_num), 0xF);
	sec_out32(CAAM_SMAPJR(sm_vid, jr_id, partition_num), 0xFF);

	/* Now need to allocate partition_num of secure RAM. */
	/* De-Allocate page_num by starting with a page inquiry command */
	temp_reg = secmem_set_cmd(PAGE(page_num) | CMD_INQUIRY);

	/* if the page is owned, de-allocate it */
	if ((temp_reg & SMCSJR_PO) == PAGE_OWNED) {
		temp_reg = secmem_set_cmd(PAGE(page_num) | CMD_PAGE_DEALLOC);
		if (temp_reg & SMCSJR_AERR) {
			printf("Error: Allocation status 0x%X\n", temp_reg);
			return ERROR_IN_PAGE_ALLOC;
		}
	}

	/* Allocate page_num to partition_num */
	temp_reg = secmem_set_cmd(PAGE(page_num) | PARTITION(partition_num)
						| CMD_PAGE_ALLOC);
	if (temp_reg & SMCSJR_AERR) {
		printf("Error: Allocation status 0x%X\n", temp_reg);
		return ERROR_IN_PAGE_ALLOC;
	}
	/* page inquiry command to ensure that the page was allocated */
	temp_reg = secmem_set_cmd(PAGE(page_num) | CMD_INQUIRY);

	/* if the page is not owned => problem */
	if ((temp_reg & SMCSJR_PO) != PAGE_OWNED) {
		printf("Allocation of page %d in partition %d failed 0x%X\n",
		       temp_reg, page_num, partition_num);

		return ERROR_IN_PAGE_ALLOC;
	}

	return 0;
}

int inline_cnstr_jobdesc_blob_dek(uint32_t *desc, const uint8_t *plain_txt,
				       uint8_t *dek_blob, uint32_t in_sz)
{
	ccsr_sec_t *sec = (void *)CONFIG_SYS_FSL_SEC_ADDR;
	uint32_t sm_vid = SM_VERSION(sec_in32(&sec->smvid));
	uint32_t jr_id = 0;

	uint32_t ret = 0;
	u32 aad_w1, aad_w2;
	/* output blob will have 32 bytes key blob in beginning and
	 * 16 byte HMAC identifier at end of data blob */
	uint32_t out_sz = in_sz + KEY_BLOB_SIZE + MAC_SIZE;
	/* Setting HDR for blob */
	uint8_t wrapped_key_hdr[8] = {HDR_TAG, 0x00, WRP_HDR_SIZE + out_sz,
			     HDR_PAR, HAB_MOD, HAB_ALG, in_sz, HAB_FLG};

	/* initialize the blob array */
	memset(dek_blob, 0, out_sz + 8);
	/* Copy the header into the DEK blob buffer */
	memcpy(dek_blob, wrapped_key_hdr, sizeof(wrapped_key_hdr));

	/* allocating secure memory */
	ret = caam_page_alloc(PAGE_1, PARTITION_1);
	if (ret)
		return ret;

	/* Write DEK to secure memory */
	memcpy((uint32_t *)SEC_MEM_PAGE1, (uint32_t *)plain_txt, in_sz);

	unsigned long start = (unsigned long)SEC_MEM_PAGE1 &
				~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + 0x1000, ARCH_DMA_MINALIGN);
	flush_dcache_range(start, end);

	/* Now configure the access rights of the partition */
	sec_out32(CAAM_SMAG1JR(sm_vid, jr_id, PARTITION_1), KS_G1);
	sec_out32(CAAM_SMAG2JR(sm_vid, jr_id, PARTITION_1), 0);
	sec_out32(CAAM_SMAPJR(sm_vid, jr_id, PARTITION_1), PERM);

	/* construct aad for AES */
	aad_w1 = (in_sz << OP_ALG_ALGSEL_SHIFT) | KEY_AES_SRC | LD_CCM_MODE;
	aad_w2 = 0x0;

	init_job_desc(desc, 0);

	append_cmd(desc, CMD_LOAD | CLASS_2 | KEY_IMM | KEY_ENC |
				(0x0c << LDST_OFFSET_SHIFT) | 0x08);

	append_u32(desc, aad_w1);

	append_u32(desc, aad_w2);

	append_cmd_ptr(desc, (dma_addr_t)SEC_MEM_PAGE1, in_sz, CMD_SEQ_IN_PTR);

	append_cmd_ptr(desc, (dma_addr_t)dek_blob + 8, out_sz, CMD_SEQ_OUT_PTR);

	append_operation(desc, OP_TYPE_ENCAP_PROTOCOL | OP_PCLID_BLOB |
						OP_PCLID_SECMEM);

	return ret;
}
#endif

void inline_cnstr_jobdesc_hash(uint32_t *desc,
			  const uint8_t *msg, uint32_t msgsz, uint8_t *digest,
			  u32 alg_type, uint32_t alg_size, int sg_tbl)
{
	/* SHA 256 , output is of length 32 words */
	uint32_t storelen = alg_size;
	u32 options;
	dma_addr_t dma_addr_in, dma_addr_out;

	dma_addr_in = virt_to_phys((void *)msg);
	dma_addr_out = virt_to_phys((void *)digest);

	init_job_desc(desc, 0);
	append_operation(desc, OP_TYPE_CLASS2_ALG |
			 OP_ALG_AAI_HASH | OP_ALG_AS_INITFINAL |
			 OP_ALG_ENCRYPT | OP_ALG_ICV_OFF | alg_type);

	options = LDST_CLASS_2_CCB | FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST2;
	if (sg_tbl)
		options |= FIFOLDST_SGF;
	if (msgsz > 0xffff) {
		options |= FIFOLDST_EXT;
		append_fifo_load(desc, dma_addr_in, 0, options);
		append_cmd(desc, msgsz);
	} else {
		append_fifo_load(desc, dma_addr_in, msgsz, options);
	}

	append_store(desc, dma_addr_out, storelen,
		     LDST_CLASS_2_CCB | LDST_SRCDST_BYTE_CONTEXT);
}
#ifndef CONFIG_SPL_BUILD
void inline_cnstr_jobdesc_blob_encap(uint32_t *desc, uint8_t *key_idnfr,
				     uint8_t *plain_txt, uint8_t *enc_blob,
				     uint32_t in_sz)
{
	dma_addr_t dma_addr_key_idnfr, dma_addr_in, dma_addr_out;
	uint32_t key_sz = KEY_IDNFR_SZ_BYTES;
	/* output blob will have 32 bytes key blob in beginning and
	 * 16 byte HMAC identifier at end of data blob */
	uint32_t out_sz = in_sz + KEY_BLOB_SIZE + MAC_SIZE;

	dma_addr_key_idnfr = virt_to_phys((void *)key_idnfr);
	dma_addr_in	= virt_to_phys((void *)plain_txt);
	dma_addr_out	= virt_to_phys((void *)enc_blob);

	init_job_desc(desc, 0);

	append_key(desc, dma_addr_key_idnfr, key_sz, CLASS_2);

	append_seq_in_ptr(desc, dma_addr_in, in_sz, 0);

	append_seq_out_ptr(desc, dma_addr_out, out_sz, 0);

	append_operation(desc, OP_TYPE_ENCAP_PROTOCOL | OP_PCLID_BLOB);
}

void inline_cnstr_jobdesc_blob_decap(uint32_t *desc, uint8_t *key_idnfr,
				     uint8_t *enc_blob, uint8_t *plain_txt,
				     uint32_t out_sz)
{
	dma_addr_t dma_addr_key_idnfr, dma_addr_in, dma_addr_out;
	uint32_t key_sz = KEY_IDNFR_SZ_BYTES;
	uint32_t in_sz = out_sz + KEY_BLOB_SIZE + MAC_SIZE;

	dma_addr_key_idnfr = virt_to_phys((void *)key_idnfr);
	dma_addr_in	= virt_to_phys((void *)enc_blob);
	dma_addr_out	= virt_to_phys((void *)plain_txt);

	init_job_desc(desc, 0);

	append_key(desc, dma_addr_key_idnfr, key_sz, CLASS_2);

	append_seq_in_ptr(desc, dma_addr_in, in_sz, 0);

	append_seq_out_ptr(desc, dma_addr_out, out_sz, 0);

	append_operation(desc, OP_TYPE_DECAP_PROTOCOL | OP_PCLID_BLOB);
}
#endif
/*
 * Descriptor to instantiate RNG State Handle 0 in normal mode and
 * load the JDKEK, TDKEK and TDSK registers
 */
void inline_cnstr_jobdesc_rng_instantiation(uint32_t *desc, int handle)
{
	u32 *jump_cmd;

	init_job_desc(desc, 0);

	/* INIT RNG in non-test mode */
	append_operation(desc, OP_TYPE_CLASS1_ALG | OP_ALG_ALGSEL_RNG |
			(handle << OP_ALG_AAI_SHIFT) | OP_ALG_AS_INIT);

	/* For SH0, Secure Keys must be generated as well */
	if (handle == 0) {
		/* wait for done */
		jump_cmd = append_jump(desc, JUMP_CLASS_CLASS1);
		set_jump_tgt_here(desc, jump_cmd);

		/*
		 * load 1 to clear written reg:
		 * resets the done interrupt and returns the RNG to idle.
		 */
		append_load_imm_u32(desc, 1, LDST_SRCDST_WORD_CLRW);

		/* generate secure keys (non-test) */
		append_operation(desc, OP_TYPE_CLASS1_ALG | OP_ALG_ALGSEL_RNG |
				OP_ALG_RNG4_SK);
	}
}

/* Change key size to bytes form bits in calling function*/
void inline_cnstr_jobdesc_pkha_rsaexp(uint32_t *desc,
				      struct pk_in_params *pkin, uint8_t *out,
				      uint32_t out_siz)
{
	dma_addr_t dma_addr_e, dma_addr_a, dma_addr_n, dma_addr_out;

	dma_addr_e = virt_to_phys((void *)pkin->e);
	dma_addr_a = virt_to_phys((void *)pkin->a);
	dma_addr_n = virt_to_phys((void *)pkin->n);
	dma_addr_out = virt_to_phys((void *)out);

	init_job_desc(desc, 0);
	append_key(desc, dma_addr_e, pkin->e_siz, KEY_DEST_PKHA_E | CLASS_1);

	append_fifo_load(desc, dma_addr_a,
			 pkin->a_siz, LDST_CLASS_1_CCB | FIFOLD_TYPE_PK_A);

	append_fifo_load(desc, dma_addr_n,
			 pkin->n_siz, LDST_CLASS_1_CCB | FIFOLD_TYPE_PK_N);

	append_operation(desc, OP_TYPE_PK | OP_ALG_PK | OP_ALG_PKMODE_MOD_EXPO);

	append_fifo_store(desc, dma_addr_out, out_siz,
			  LDST_CLASS_1_CCB | FIFOST_TYPE_PKHA_B);
}
