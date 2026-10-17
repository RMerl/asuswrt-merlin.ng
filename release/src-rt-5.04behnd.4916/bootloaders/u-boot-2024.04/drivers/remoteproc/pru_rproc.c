// SPDX-License-Identifier: GPL-2.0
/*
 * PRU-RTU remoteproc driver for various SoCs
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 *	Keerthy <j-keerthy@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <elf.h>
#include <dm/of_access.h>
#include <remoteproc.h>
#include <errno.h>
#include <clk.h>
#include <reset.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <power-domain.h>
#include <linux/pruss_driver.h>
#include <dm/device_compat.h>

/* PRU_ICSS_PRU_CTRL registers */
#define PRU_CTRL_CTRL		0x0000
#define PRU_CTRL_STS		0x0004
#define PRU_CTRL_WAKEUP_EN	0x0008
#define PRU_CTRL_CYCLE		0x000C
#define PRU_CTRL_STALL		0x0010
#define PRU_CTRL_CTBIR0		0x0020
#define PRU_CTRL_CTBIR1		0x0024
#define PRU_CTRL_CTPPR0		0x0028
#define PRU_CTRL_CTPPR1		0x002C

/* CTRL register bit-fields */
#define CTRL_CTRL_SOFT_RST_N	BIT(0)
#define CTRL_CTRL_EN		BIT(1)
#define CTRL_CTRL_SLEEPING	BIT(2)
#define CTRL_CTRL_CTR_EN	BIT(3)
#define CTRL_CTRL_SINGLE_STEP	BIT(8)
#define CTRL_CTRL_RUNSTATE	BIT(15)

#define RPROC_FLAGS_SHIFT	16
#define RPROC_FLAGS_NONE	0
#define RPROC_FLAGS_ELF_PHDR	BIT(0 + RPROC_FLAGS_SHIFT)
#define RPROC_FLAGS_ELF_SHDR	BIT(1 + RPROC_FLAGS_SHIFT)

/**
 * enum pru_mem - PRU core memory range identifiers
 */
enum pru_mem {
	PRU_MEM_IRAM = 0,
	PRU_MEM_CTRL,
	PRU_MEM_DEBUG,
	PRU_MEM_MAX,
};

struct pru_privdata {
	phys_addr_t pru_iram;
	phys_addr_t pru_ctrl;
	phys_addr_t pru_debug;
	fdt_size_t pru_iramsz;
	fdt_size_t pru_ctrlsz;
	fdt_size_t pru_debugsz;
	const char *fw_name;
	u32 iram_da;
	u32 pdram_da;
	u32 sdram_da;
	u32 shrdram_da;
	u32 bootaddr;
	int id;
	struct pruss *prusspriv;
};

static inline u32 pru_control_read_reg(struct pru_privdata *pru, unsigned int reg)
{
	return readl(pru->pru_ctrl + reg);
}

static inline
void pru_control_write_reg(struct pru_privdata *pru, unsigned int reg, u32 val)
{
	writel(val, pru->pru_ctrl + reg);
}

static inline
void pru_control_set_reg(struct pru_privdata *pru, unsigned int reg,
			 u32 mask, u32 set)
{
	u32 val;

	val = pru_control_read_reg(pru, reg);
	val &= ~mask;
	val |= (set & mask);
	pru_control_write_reg(pru, reg, val);
}

/**
 * pru_rproc_set_ctable() - set the constant table index for the PRU
 * @rproc: the rproc instance of the PRU
 * @c: constant table index to set
 * @addr: physical address to set it to
 */
static int pru_rproc_set_ctable(struct pru_privdata *pru, enum pru_ctable_idx c, u32 addr)
{
	unsigned int reg;
	u32 mask, set;
	u16 idx;
	u16 idx_mask;

	/* pointer is 16 bit and index is 8-bit so mask out the rest */
	idx_mask = (c >= PRU_C28) ? 0xFFFF : 0xFF;

	/* ctable uses bit 8 and upwards only */
	idx = (addr >> 8) & idx_mask;

	/* configurable ctable (i.e. C24) starts at PRU_CTRL_CTBIR0 */
	reg = PRU_CTRL_CTBIR0 + 4 * (c >> 1);
	mask = idx_mask << (16 * (c & 1));
	set = idx << (16 * (c & 1));

	pru_control_set_reg(pru, reg, mask, set);

	return 0;
}

/**
 * pru_start() - start the pru processor
 * @dev:	corresponding k3 remote processor device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int pru_start(struct udevice *dev)
{
	struct pru_privdata *priv;
	int val = 0;

	priv = dev_get_priv(dev);

	pru_rproc_set_ctable(priv, PRU_C28, 0x100 << 8);

	val = CTRL_CTRL_EN | ((priv->bootaddr >> 2) << 16);
	writel(val, priv->pru_ctrl + PRU_CTRL_CTRL);

	return 0;
}

/**
 * pru_stop() - Stop pru processor
 * @dev:	corresponding k3 remote processor device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int pru_stop(struct udevice *dev)
{
	struct pru_privdata *priv;
	int val = 0;

	priv = dev_get_priv(dev);

	val = readl(priv->pru_ctrl + PRU_CTRL_CTRL);
	val &= ~CTRL_CTRL_EN;
	writel(val, priv->pru_ctrl + PRU_CTRL_CTRL);

	return 0;
}

/**
 * pru_init() - Initialize the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int pru_init(struct udevice *dev)
{
	return 0;
}

/*
 * Convert PRU device address (data spaces only) to kernel virtual address
 *
 * Each PRU has access to all data memories within the PRUSS, accessible at
 * different ranges. So, look through both its primary and secondary Data
 * RAMs as well as any shared Data RAM to convert a PRU device address to
 * kernel virtual address. Data RAM0 is primary Data RAM for PRU0 and Data
 * RAM1 is primary Data RAM for PRU1.
 */
static void *pru_d_da_to_pa(struct pru_privdata *priv, u32 da, int len)
{
	u32 offset;
	void *pa = NULL;
	phys_addr_t dram0, dram1, shrdram2;
	u32 dram0sz, dram1sz, shrdram2sz;

	if (len <= 0)
		return NULL;

	dram0 = priv->prusspriv->mem_regions[PRUSS_MEM_DRAM0].pa;
	dram1 = priv->prusspriv->mem_regions[PRUSS_MEM_DRAM1].pa;
	shrdram2 = priv->prusspriv->mem_regions[PRUSS_MEM_SHRD_RAM2].pa;
	dram0sz = priv->prusspriv->mem_regions[PRUSS_MEM_DRAM0].size;
	dram1sz = priv->prusspriv->mem_regions[PRUSS_MEM_DRAM1].size;
	shrdram2sz = priv->prusspriv->mem_regions[PRUSS_MEM_SHRD_RAM2].size;

	/* PRU1 has its local RAM addresses reversed */
	if (priv->id == 1) {
		dram1 = dram0;
		dram1sz = dram0sz;

		dram0 =  priv->prusspriv->mem_regions[PRUSS_MEM_DRAM1].pa;
		dram0sz = priv->prusspriv->mem_regions[PRUSS_MEM_DRAM1].size;
	}

	if (da >= priv->pdram_da && da + len <= priv->pdram_da + dram0sz) {
		offset = da - priv->pdram_da;
		pa = (__force void *)(dram0 + offset);
	} else if (da >= priv->sdram_da &&
		   da + len <= priv->sdram_da + dram1sz) {
		offset = da - priv->sdram_da;
		pa = (__force void *)(dram1 + offset);
	} else if (da >= priv->shrdram_da &&
		   da + len <= priv->shrdram_da + shrdram2sz) {
		offset = da - priv->shrdram_da;
		pa = (__force void *)(shrdram2 + offset);
	}

	return pa;
}

/*
 * Convert PRU device address (instruction space) to kernel virtual address
 *
 * A PRU does not have an unified address space. Each PRU has its very own
 * private Instruction RAM, and its device address is identical to that of
 * its primary Data RAM device address.
 */
static void *pru_i_da_to_pa(struct pru_privdata *priv, u32 da, int len)
{
	u32 offset;
	void *pa = NULL;

	if (len <= 0)
		return NULL;

	if (da >= priv->iram_da &&
	    da + len <= priv->iram_da + priv->pru_iramsz) {
		offset = da - priv->iram_da;
		pa = (__force void *)(priv->pru_iram + offset);
	}

	return pa;
}

/* PRU-specific address translator */
static void *pru_da_to_pa(struct pru_privdata *priv, u64 da, int len, u32 flags)
{
	void *pa;
	u32 exec_flag;

	exec_flag = ((flags & RPROC_FLAGS_ELF_SHDR) ? flags & SHF_EXECINSTR :
		     ((flags & RPROC_FLAGS_ELF_PHDR) ? flags & PF_X : 0));

	if (exec_flag)
		pa = pru_i_da_to_pa(priv, da, len);
	else
		pa = pru_d_da_to_pa(priv, da, len);

	return pa;
}

/*
 * Custom memory copy implementation for ICSSG PRU/RTU Cores
 *
 * The ICSSG PRU/RTU cores have a memory copying issue with IRAM memories, that
 * is not seen on previous generation SoCs. The data is reflected properly in
 * the IRAM memories only for integer (4-byte) copies. Any unaligned copies
 * result in all the other pre-existing bytes zeroed out within that 4-byte
 * boundary, thereby resulting in wrong text/code in the IRAMs. Also, the
 * IRAM memory port interface does not allow any 8-byte copies (as commonly
 * used by ARM64 memcpy implementation) and throws an exception. The DRAM
 * memory ports do not show this behavior. Use this custom copying function
 * to properly load the PRU/RTU firmware images on all memories for simplicity.
 *
 * TODO: Improve the function to deal with additional corner cases like
 * unaligned copy sizes or sub-integer trailing bytes when the need arises.
 */
static int pru_rproc_memcpy(void *dest, void *src, size_t count)
{
	const int *s = src;
	int *d = dest;
	int size = count / 4;
	int *tmp_src = NULL;

	/* limited to 4-byte aligned addresses and copy sizes */
	if ((long)dest % 4 || count % 4)
		return -EINVAL;

	/* src offsets in ELF firmware image can be non-aligned */
	if ((long)src % 4) {
		tmp_src = malloc(count);
		if (!tmp_src)
			return -ENOMEM;

		memcpy(tmp_src, src, count);
		s = tmp_src;
	}

	while (size--)
		*d++ = *s++;

	kfree(tmp_src);

	return 0;
}

/**
 * pru_load() - Load pru firmware
 * @dev:	corresponding k3 remote processor device
 * @addr:	Address on the RAM from which firmware is to be loaded
 * @size:	Size of the pru firmware in bytes
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int pru_load(struct udevice *dev, ulong addr, ulong size)
{
	struct pru_privdata *priv;
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdr;
	int i, ret = 0;

	priv = dev_get_priv(dev);

	ehdr = (Elf32_Ehdr *)addr;
	phdr = (Elf32_Phdr *)(addr + ehdr->e_phoff);

	/* go through the available ELF segments */
	for (i = 0; i < ehdr->e_phnum; i++, phdr++) {
		u32 da = phdr->p_paddr;
		u32 memsz = phdr->p_memsz;
		u32 filesz = phdr->p_filesz;
		u32 offset = phdr->p_offset;
		void *ptr;

		if (phdr->p_type != PT_LOAD)
			continue;

		dev_dbg(dev, "phdr: type %d da 0x%x memsz 0x%x filesz 0x%x\n",
			phdr->p_type, da, memsz, filesz);

		if (filesz > memsz) {
			dev_dbg(dev, "bad phdr filesz 0x%x memsz 0x%x\n",
				filesz, memsz);
			ret = -EINVAL;
			break;
		}

		if (offset + filesz > size) {
			dev_dbg(dev, "truncated fw: need 0x%x avail 0x%zx\n",
				offset + filesz, size);
			ret = -EINVAL;
			break;
		}

		/* grab the kernel address for this device address */
		ptr = pru_da_to_pa(priv, da, memsz,
				   RPROC_FLAGS_ELF_PHDR | phdr->p_flags);
		if (!ptr) {
			dev_dbg(dev, "bad phdr da 0x%x mem 0x%x\n", da, memsz);
			ret = -EINVAL;
			break;
		}

		/* skip the memzero logic performed by remoteproc ELF loader */
		if (!phdr->p_filesz)
			continue;

		ret = pru_rproc_memcpy(ptr,
				       (void *)addr + phdr->p_offset, filesz);
		if (ret) {
			dev_dbg(dev, "PRU custom memory copy failed for da 0x%x memsz 0x%x\n",
				da, memsz);
			break;
		}
	}

	priv->bootaddr = ehdr->e_entry;

	return ret;
}

static const struct dm_rproc_ops pru_ops = {
	.init = pru_init,
	.start = pru_start,
	.stop = pru_stop,
	.load = pru_load,
};

static void pru_set_id(struct pru_privdata *priv, struct udevice *dev)
{
	u32 mask2 = 0x38000;

	if (device_is_compatible(dev, "ti,am654-rtu"))
		mask2 = 0x6000;

	if (device_is_compatible(dev, "ti,am654-tx-pru"))
		mask2 = 0xc000;

	if ((priv->pru_iram & mask2) == mask2)
		priv->id = 1;
	else
		priv->id = 0;
}

/**
 * pru_probe() - Basic probe
 * @dev:	corresponding k3 remote processor device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int pru_probe(struct udevice *dev)
{
	struct pru_privdata *priv;
	ofnode node;

	node = dev_ofnode(dev);

	priv = dev_get_priv(dev);
	priv->prusspriv = dev_get_priv(dev->parent);

	priv->pru_iram = devfdt_get_addr_size_index(dev, PRU_MEM_IRAM,
						    &priv->pru_iramsz);
	priv->pru_ctrl = devfdt_get_addr_size_index(dev, PRU_MEM_CTRL,
						    &priv->pru_ctrlsz);
	priv->pru_debug = devfdt_get_addr_size_index(dev, PRU_MEM_DEBUG,
						     &priv->pru_debugsz);

	priv->iram_da = 0;
	priv->pdram_da = 0;
	priv->sdram_da = 0x2000;
	priv->shrdram_da = 0x10000;

	pru_set_id(priv, dev);

	return 0;
}

static const struct udevice_id pru_ids[] = {
	{ .compatible = "ti,am654-pru"},
	{ .compatible = "ti,am654-rtu"},
	{ .compatible = "ti,am654-tx-pru" },
	{}
};

U_BOOT_DRIVER(pru) = {
	.name = "pru",
	.of_match = pru_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &pru_ops,
	.probe = pru_probe,
	.priv_auto = sizeof(struct pru_privdata),
};
