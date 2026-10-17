/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 */

#ifndef __OCTEONTX_BCH_H__
#define __OCTEONTX_BCH_H__

#include "octeontx_bch_regs.h"

/* flags to indicate the features supported */
#define BCH_FLAG_SRIOV_ENABLED		BIT(1)

/*
 * BCH Registers map for 81xx
 */

/* PF registers */
#define BCH_CTL				0x0ull
#define BCH_ERR_CFG			0x10ull
#define BCH_BIST_RESULT			0x80ull
#define BCH_ERR_INT			0x88ull
#define BCH_ERR_INT_W1S			0x90ull
#define BCH_ERR_INT_ENA_W1C		0xA0ull
#define BCH_ERR_INT_ENA_W1S		0xA8ull

/* VF registers */
#define BCH_VQX_CTL(z)			0x0ull
#define BCH_VQX_CMD_BUF(z)		0x8ull
#define BCH_VQX_CMD_PTR(z)		0x20ull
#define BCH_VQX_DOORBELL(z)		0x800ull

#define BCHPF_DRIVER_NAME	"octeontx-bchpf"
#define BCHVF_DRIVER_NAME	"octeontx-bchvf"

struct bch_device {
	struct list_head list;
	u8 max_vfs;
	u8 vfs_enabled;
	u8 vfs_in_use;
	u32 flags;
	void __iomem *reg_base;
	struct udevice *dev;
};

struct bch_vf {
	u16 flags;
	u8 vfid;
	u8 node;
	u8 priority;
	struct udevice *dev;
	void __iomem *reg_base;
};

struct buf_ptr {
	u8 *vptr;
	dma_addr_t dma_addr;
	u16 size;
};

void *octeontx_bch_getv(void);
void octeontx_bch_putv(void *token);
void *octeontx_bch_getp(void);
void octeontx_bch_putp(void *token);
int octeontx_bch_wait(struct bch_vf *vf, union bch_resp *resp,
		      dma_addr_t handle);
/**
 * Given a data block calculate the ecc data and fill in the response
 *
 * @param[in] block	8-byte aligned pointer to data block to calculate ECC
 * @param block_size	Size of block in bytes, must be a multiple of two.
 * @param bch_level	Number of errors that must be corrected.  The number of
 *			parity bytes is equal to ((15 * bch_level) + 7) / 8.
 *			Must be 4, 8, 16, 24, 32, 40, 48, 56, 60 or 64.
 * @param[out] ecc	8-byte aligned pointer to where ecc data should go
 * @param[in] resp	pointer to where responses will be written.
 *
 * Return: Zero on success, negative on failure.
 */
int octeontx_bch_encode(struct bch_vf *vf, dma_addr_t block, u16 block_size,
			u8 bch_level, dma_addr_t ecc, dma_addr_t resp);

/**
 * Given a data block and ecc data correct the data block
 *
 * @param[in] block_ecc_in	8-byte aligned pointer to data block with ECC
 *				data concatenated to the end to correct
 * @param block_size		Size of block in bytes, must be a multiple of
 *				two.
 * @param bch_level		Number of errors that must be corrected.  The
 *				number of parity bytes is equal to
 *				((15 * bch_level) + 7) / 8.
 *				Must be 4, 8, 16, 24, 32, 40, 48, 56, 60 or 64.
 * @param[out] block_out	8-byte aligned pointer to corrected data buffer.
 *				This should not be the same as block_ecc_in.
 * @param[in] resp		pointer to where responses will be written.
 *
 * Return: Zero on success, negative on failure.
 */

int octeontx_bch_decode(struct bch_vf *vf, dma_addr_t block_ecc_in,
			u16 block_size, u8 bch_level,
			dma_addr_t block_out, dma_addr_t resp);

/**
 * Ring the BCH doorbell telling it that new commands are
 * available.
 *
 * @param num_commands	Number of new commands
 * @param vf		virtual function handle
 */
static inline void octeontx_bch_write_doorbell(u64 num_commands,
					       struct bch_vf *vf)
{
	u64 num_words = num_commands * sizeof(union bch_cmd) / sizeof(uint64_t);

	writeq(num_words, vf->reg_base + BCH_VQX_DOORBELL(0));
}

/**
 * Since it's possible (and even likely) that the NAND device will be probed
 * before the BCH device has been probed, we may need to defer the probing.
 *
 * In this case, the initial probe returns success but the actual probing
 * is deferred until the BCH VF has been probed.
 *
 * Return:	0 for success, otherwise error
 */
int octeontx_pci_nand_deferred_probe(void);

#endif /* __OCTEONTX_BCH_H__ */
