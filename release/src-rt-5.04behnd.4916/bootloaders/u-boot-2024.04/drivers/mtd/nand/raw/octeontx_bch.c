// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <dm.h>
#include <dm/of_access.h>
#include <malloc.h>
#include <memalign.h>
#include <nand.h>
#include <pci.h>
#include <pci_ids.h>
#include <time.h>
#include <linux/bitfield.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <linux/libfdt.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand_bch.h>
#include <linux/mtd/nand_ecc.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/dma-mapping.h>
#include <asm/arch/clock.h>
#include "octeontx_bch.h"

static LIST_HEAD(octeontx_bch_devices);
static unsigned int num_vfs = BCH_NR_VF;
static void *bch_pf;
static void *bch_vf;
static void *token;
static bool bch_pf_initialized;
static bool bch_vf_initialized;

static int pci_enable_sriov(struct udevice *dev, int nr_virtfn)
{
	int ret;

	ret = pci_sriov_init(dev, nr_virtfn);
	if (ret)
		printf("%s(%s): pci_sriov_init returned %d\n", __func__,
		       dev->name, ret);
	return ret;
}

void *octeontx_bch_getv(void)
{
	if (!bch_vf)
		return NULL;
	if (bch_vf_initialized && bch_pf_initialized)
		return bch_vf;
	else
		return NULL;
}

void octeontx_bch_putv(void *token)
{
	bch_vf_initialized = !!token;
	bch_vf = token;
}

void *octeontx_bch_getp(void)
{
	return token;
}

void octeontx_bch_putp(void *token)
{
	bch_pf = token;
	bch_pf_initialized = !!token;
}

static int do_bch_init(struct bch_device *bch)
{
	return 0;
}

static void bch_reset(struct bch_device *bch)
{
	writeq(1, bch->reg_base + BCH_CTL);
	mdelay(2);
}

static void bch_disable(struct bch_device *bch)
{
	writeq(~0ull, bch->reg_base + BCH_ERR_INT_ENA_W1C);
	writeq(~0ull, bch->reg_base + BCH_ERR_INT);
	bch_reset(bch);
}

static u32 bch_check_bist_status(struct bch_device *bch)
{
	return readq(bch->reg_base + BCH_BIST_RESULT);
}

static int bch_device_init(struct bch_device *bch)
{
	u64 bist;
	int rc;

	debug("%s: Resetting...\n", __func__);
	/* Reset the PF when probed first */
	bch_reset(bch);

	debug("%s: Checking BIST...\n", __func__);
	/* Check BIST status */
	bist = (u64)bch_check_bist_status(bch);
	if (bist) {
		dev_err(dev, "BCH BIST failed with code 0x%llx\n", bist);
		return -ENODEV;
	}

	/* Get max VQs/VFs supported by the device */

	bch->max_vfs = pci_sriov_get_totalvfs(bch->dev);
	debug("%s: %d vfs\n", __func__, bch->max_vfs);
	if (num_vfs > bch->max_vfs) {
		dev_warn(dev, "Num of VFs to enable %d is greater than max available.  Enabling %d VFs.\n",
			 num_vfs, bch->max_vfs);
		num_vfs = bch->max_vfs;
	}
	bch->vfs_enabled = bch->max_vfs;
	/* Get number of VQs/VFs to be enabled */
	/* TODO: Get CLK frequency */
	/* Reset device parameters */

	debug("%s: Doing initialization\n", __func__);
	rc = do_bch_init(bch);

	return rc;
}

static int bch_sriov_configure(struct udevice *dev, int numvfs)
{
	struct bch_device *bch = dev_get_priv(dev);
	int ret = -EBUSY;

	debug("%s(%s, %d), bch: %p, vfs_in_use: %d, enabled: %d\n", __func__,
	      dev->name, numvfs, bch, bch->vfs_in_use, bch->vfs_enabled);
	if (bch->vfs_in_use)
		goto exit;

	ret = 0;

	if (numvfs > 0) {
		debug("%s: Enabling sriov\n", __func__);
		ret = pci_enable_sriov(dev, numvfs);
		if (ret == 0) {
			bch->flags |= BCH_FLAG_SRIOV_ENABLED;
			ret = numvfs;
			bch->vfs_enabled = numvfs;
		}
	}

	debug("VFs enabled: %d\n", ret);
exit:
	debug("%s: Returning %d\n", __func__, ret);
	return ret;
}

static int octeontx_pci_bchpf_probe(struct udevice *dev)
{
	struct bch_device *bch;
	int ret;

	debug("%s(%s)\n", __func__, dev->name);
	bch = dev_get_priv(dev);
	if (!bch)
		return -ENOMEM;

	bch->reg_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0,
				       PCI_REGION_TYPE, PCI_REGION_MEM);
	bch->dev = dev;

	debug("%s: base address: %p\n", __func__, bch->reg_base);
	ret = bch_device_init(bch);
	if (ret) {
		printf("%s(%s): init returned %d\n", __func__, dev->name, ret);
		return ret;
	}
	INIT_LIST_HEAD(&bch->list);
	list_add(&bch->list, &octeontx_bch_devices);
	token = (void *)dev;

	debug("%s: Configuring SRIOV\n", __func__);
	bch_sriov_configure(dev, num_vfs);
	debug("%s: Done.\n", __func__);
	octeontx_bch_putp(bch);

	return 0;
}

static const struct pci_device_id octeontx_bchpf_pci_id_table[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_BCH) },
	{},
};

static const struct pci_device_id octeontx_bchvf_pci_id_table[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_BCHVF)},
	{},
};

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
			u8 bch_level, dma_addr_t ecc, dma_addr_t resp)
{
	union bch_cmd cmd;
	int rc;

	memset(&cmd, 0, sizeof(cmd));
	cmd.s.cword.ecc_gen = eg_gen;
	cmd.s.cword.ecc_level = bch_level;
	cmd.s.cword.size = block_size;

	cmd.s.oword.ptr = ecc;
	cmd.s.iword.ptr = block;
	cmd.s.rword.ptr = resp;
	rc = octeontx_cmd_queue_write(QID_BCH, 1,
				      sizeof(cmd) / sizeof(uint64_t), cmd.u);
	if (rc)
		return -1;

	octeontx_bch_write_doorbell(1, vf);

	return 0;
}

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
			dma_addr_t block_out, dma_addr_t resp)
{
	union bch_cmd cmd;
	int rc;

	memset(&cmd, 0, sizeof(cmd));
	cmd.s.cword.ecc_gen = eg_correct;
	cmd.s.cword.ecc_level = bch_level;
	cmd.s.cword.size = block_size;

	cmd.s.oword.ptr = block_out;
	cmd.s.iword.ptr = block_ecc_in;
	cmd.s.rword.ptr = resp;
	rc = octeontx_cmd_queue_write(QID_BCH, 1,
				      sizeof(cmd) / sizeof(uint64_t), cmd.u);
	if (rc)
		return -1;

	octeontx_bch_write_doorbell(1, vf);
	return 0;
}
EXPORT_SYMBOL(octeontx_bch_decode);

int octeontx_bch_wait(struct bch_vf *vf, union bch_resp *resp,
		      dma_addr_t handle)
{
	ulong start = get_timer(0);

	__iormb(); /* HW is updating *resp */
	while (!resp->s.done && get_timer(start) < 10)
		__iormb(); /* HW is updating *resp */

	if (resp->s.done)
		return 0;

	return -ETIMEDOUT;
}

struct bch_q octeontx_bch_q[QID_MAX];

static int octeontx_cmd_queue_initialize(struct udevice *dev, int queue_id,
					 int max_depth, int fpa_pool,
					 int pool_size)
{
	/* some params are for later merge with CPT or cn83xx */
	struct bch_q *q = &octeontx_bch_q[queue_id];
	unsigned long paddr;
	u64 *chunk_buffer;
	int chunk = max_depth + 1;
	int i, size;

	if ((unsigned int)queue_id >= QID_MAX)
		return -EINVAL;
	if (max_depth & chunk) /* must be 2^N - 1 */
		return -EINVAL;

	size = NQS * chunk * sizeof(u64);
	chunk_buffer = dma_alloc_coherent(size, &paddr);
	if (!chunk_buffer)
		return -ENOMEM;

	q->base_paddr = paddr;
	q->dev = dev;
	q->index = 0;
	q->max_depth = max_depth;
	q->pool_size_m1 = pool_size;
	q->base_vaddr = chunk_buffer;

	for (i = 0; i < NQS; i++) {
		u64 *ixp;
		int inext = (i + 1) * chunk - 1;
		int j = (i + 1) % NQS;
		int jnext = j * chunk;
		dma_addr_t jbase = q->base_paddr + jnext * sizeof(u64);

		ixp = &chunk_buffer[inext];
		*ixp = jbase;
	}

	return 0;
}

static int octeontx_pci_bchvf_probe(struct udevice *dev)
{
	struct bch_vf *vf;
	union bch_vqx_ctl ctl;
	union bch_vqx_cmd_buf cbuf;
	int err;

	debug("%s(%s)\n", __func__, dev->name);
	vf = dev_get_priv(dev);
	if (!vf)
		return -ENOMEM;

	vf->dev = dev;

	/* Map PF's configuration registers */
	vf->reg_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0,
				      PCI_REGION_TYPE, PCI_REGION_MEM);
	debug("%s: reg base: %p\n", __func__, vf->reg_base);

	err = octeontx_cmd_queue_initialize(dev, QID_BCH, QDEPTH - 1, 0,
					    sizeof(union bch_cmd) * QDEPTH);
	if (err) {
		dev_err(dev, "octeontx_cmd_queue_initialize() failed\n");
		goto release;
	}

	ctl.u = readq(vf->reg_base + BCH_VQX_CTL(0));

	cbuf.u = 0;
	cbuf.s.ldwb = 1;
	cbuf.s.dfb = 1;
	cbuf.s.size = QDEPTH;
	writeq(cbuf.u, vf->reg_base + BCH_VQX_CMD_BUF(0));

	writeq(ctl.u, vf->reg_base + BCH_VQX_CTL(0));

	writeq(octeontx_bch_q[QID_BCH].base_paddr,
	       vf->reg_base + BCH_VQX_CMD_PTR(0));

	octeontx_bch_putv(vf);

	debug("%s: bch vf initialization complete\n", __func__);

	if (octeontx_bch_getv())
		return octeontx_pci_nand_deferred_probe();

	return -1;

release:
	return err;
}

static int octeontx_pci_bchpf_remove(struct udevice *dev)
{
	struct bch_device *bch = dev_get_priv(dev);

	bch_disable(bch);
	return 0;
}

U_BOOT_DRIVER(octeontx_pci_bchpf) = {
	.name	= BCHPF_DRIVER_NAME,
	.id	= UCLASS_MISC,
	.probe	= octeontx_pci_bchpf_probe,
	.remove = octeontx_pci_bchpf_remove,
	.priv_auto	= sizeof(struct bch_device),
	.flags = DM_FLAG_OS_PREPARE,
};

U_BOOT_DRIVER(octeontx_pci_bchvf) = {
	.name	= BCHVF_DRIVER_NAME,
	.id	= UCLASS_MISC,
	.probe = octeontx_pci_bchvf_probe,
	.priv_auto	= sizeof(struct bch_vf),
};

U_BOOT_PCI_DEVICE(octeontx_pci_bchpf, octeontx_bchpf_pci_id_table);
U_BOOT_PCI_DEVICE(octeontx_pci_bchvf, octeontx_bchvf_pci_id_table);
