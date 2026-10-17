// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#include <common.h>
#include <errno.h>
#include <fsl_sec.h>
#include <memalign.h>
#include "desc.h"
#include "desc_constr.h"
#include "jobdesc.h"
#include "jr.h"

/* Size of MFG descriptor */
#define MFG_PUBK_DSC_WORDS 4
#define MFG_SIGN_DSC_WORDS 8

static void mfg_build_sign_dsc(u32 *dsc_ptr, const u8 *m, int size,
			       u8 *dgst, u8 *c, u8 *d)
{
	u32 *dsc = dsc_ptr;
	struct pdb_mp_sign *pdb;

	init_job_desc_pdb(dsc, 0, sizeof(struct pdb_mp_sign));

	pdb = (struct pdb_mp_sign *)desc_pdb(dsc);

	/* Curve */
	pdb->pdb_hdr = (PDB_MP_CSEL_P256);

	/* Message Pointer */
	pdb_add_ptr(&pdb->dma_addr_msg, virt_to_phys((void *)m));

	/* mes-resp Pointer */
	pdb_add_ptr(&pdb->dma_addr_hash, virt_to_phys((void *)dgst));

	/* C Pointer */
	pdb_add_ptr(&pdb->dma_addr_c_sig, virt_to_phys((void *)c));

	/* d Pointer */
	pdb_add_ptr(&pdb->dma_addr_d_sig, virt_to_phys((void *)d));

	/* Message Size */
	pdb->img_size = size;

	/* MP PubK generate key command */
	append_cmd(dsc, (CMD_OPERATION | OP_TYPE_DECAP_PROTOCOL |
			 OP_PCLID_MP_SIGN));
}

static void mfg_build_pubk_dsc(u32 *dsc_ptr, u8 *dst)
{
	u32 *dsc = dsc_ptr;
	struct pdb_mp_pub_k *pdb;

	init_job_desc_pdb(dsc, 0, sizeof(struct pdb_mp_pub_k));

	pdb = (struct pdb_mp_pub_k *)desc_pdb(dsc);

	/* Curve */
	pdb->pdb_hdr = (PDB_MP_CSEL_P256);

	/* Message Pointer */
	pdb_add_ptr(&pdb->dma_pkey, virt_to_phys((void *)dst));

	/* MP Sign key command */
	append_cmd(dsc, (CMD_OPERATION | OP_TYPE_DECAP_PROTOCOL |
			 OP_PCLID_MP_PUB_KEY));
}

int gen_mppubk(u8 *dst)
{
	int size, ret;
	u32 *dsc;

	/* Job Descriptor initialization */
	dsc = memalign(ARCH_DMA_MINALIGN,
		       sizeof(uint32_t) * MFG_PUBK_DSC_WORDS);
	if (!dsc) {
		debug("Not enough memory for descriptor allocation\n");
		return -ENOMEM;
	}

	mfg_build_pubk_dsc(dsc, dst);

	size = roundup(sizeof(uint32_t) * MFG_PUBK_DSC_WORDS,
		       ARCH_DMA_MINALIGN);
	flush_dcache_range((unsigned long)dsc, (unsigned long)dsc + size);

	size = roundup(FSL_CAAM_MP_PUBK_BYTES, ARCH_DMA_MINALIGN);
	flush_dcache_range((unsigned long)dst, (unsigned long)dst + size);

	/* Execute Job Descriptor */
	puts("\nGenerating Manufacturing Protection Public Key\n");

	ret = run_descriptor_jr(dsc);
	if (ret) {
		debug("Error in public key generation %d\n", ret);
		goto err;
	}

	size = roundup(FSL_CAAM_MP_PUBK_BYTES, ARCH_DMA_MINALIGN);
	invalidate_dcache_range((unsigned long)dst, (unsigned long)dst + size);
err:
	free(dsc);
	return ret;
}

int sign_mppubk(const u8 *m, int data_size, u8 *dgst, u8 *c, u8 *d)
{
	int size, ret;
	u32 *dsc;

	/* Job Descriptor initialization */
	dsc = memalign(ARCH_DMA_MINALIGN,
		       sizeof(uint32_t) * MFG_SIGN_DSC_WORDS);
	if (!dsc) {
		debug("Not enough memory for descriptor allocation\n");
		return -ENOMEM;
	}

	mfg_build_sign_dsc(dsc, m, data_size, dgst, c, d);

	size = roundup(sizeof(uint32_t) * MFG_SIGN_DSC_WORDS,
		       ARCH_DMA_MINALIGN);
	flush_dcache_range((unsigned long)dsc, (unsigned long)dsc + size);

	size = roundup(data_size, ARCH_DMA_MINALIGN);
	flush_dcache_range((unsigned long)m, (unsigned long)m + size);

	size = roundup(FSL_CAAM_MP_MES_DGST_BYTES, ARCH_DMA_MINALIGN);
	flush_dcache_range((unsigned long)dgst, (unsigned long)dgst + size);

	size = roundup(FSL_CAAM_MP_PRVK_BYTES, ARCH_DMA_MINALIGN);
	flush_dcache_range((unsigned long)c, (unsigned long)c + size);
	flush_dcache_range((unsigned long)d, (unsigned long)d + size);

	/* Execute Job Descriptor */
	puts("\nSigning message with Manufacturing Protection Private Key\n");

	ret = run_descriptor_jr(dsc);
	if (ret) {
		debug("Error in public key generation %d\n", ret);
		goto err;
	}

	size = roundup(FSL_CAAM_MP_MES_DGST_BYTES, ARCH_DMA_MINALIGN);
	invalidate_dcache_range((unsigned long)dgst,
				(unsigned long)dgst + size);

	size = roundup(FSL_CAAM_MP_PRVK_BYTES, ARCH_DMA_MINALIGN);
	invalidate_dcache_range((unsigned long)c, (unsigned long)c + size);
	invalidate_dcache_range((unsigned long)d, (unsigned long)d + size);

err:
	free(dsc);
	return ret;
}
