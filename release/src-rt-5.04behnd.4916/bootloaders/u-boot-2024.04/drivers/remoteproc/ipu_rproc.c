// SPDX-License-Identifier: GPL-2.0
/*
 * IPU remoteproc driver for various SoCs
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Angela Stegmaier  <angelabaker@ti.com>
 *	Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *      Keerthy <j-keerthy@ti.com>
 */

#include <common.h>
#include <hang.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <elf.h>
#include <env.h>
#include <dm/of_access.h>
#include <fs_loader.h>
#include <remoteproc.h>
#include <errno.h>
#include <clk.h>
#include <reset.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <misc.h>
#include <power-domain.h>
#include <timer.h>
#include <fs.h>
#include <spl.h>
#include <timer.h>
#include <reset.h>
#include <linux/bitmap.h>

#define IPU1_LOAD_ADDR         (0xa17ff000)
#define MAX_REMOTECORE_BIN_SIZE (8 * 0x100000)

enum ipu_num {
	IPU1 = 0,
	IPU2,
	RPROC_END_ENUMS,
};

#define IPU2_LOAD_ADDR         (IPU1_LOAD_ADDR + MAX_REMOTECORE_BIN_SIZE)

#define PAGE_SHIFT			12
#define PAGESIZE_1M                          0x0
#define PAGESIZE_64K                         0x1
#define PAGESIZE_4K                          0x2
#define PAGESIZE_16M                         0x3
#define LE                                   0
#define BE                                   1
#define ELEMSIZE_8                           0x0
#define ELEMSIZE_16                          0x1
#define ELEMSIZE_32                          0x2
#define MIXED_TLB                            0x0
#define MIXED_CPU                            0x1

#define PGT_SMALLPAGE_SIZE                   0x00001000
#define PGT_LARGEPAGE_SIZE                   0x00010000
#define PGT_SECTION_SIZE                     0x00100000
#define PGT_SUPERSECTION_SIZE                0x01000000

#define PGT_L1_DESC_PAGE                     0x00001
#define PGT_L1_DESC_SECTION                  0x00002
#define PGT_L1_DESC_SUPERSECTION             0x40002

#define PGT_L1_DESC_PAGE_MASK                0xfffffC00
#define PGT_L1_DESC_SECTION_MASK             0xfff00000
#define PGT_L1_DESC_SUPERSECTION_MASK        0xff000000

#define PGT_L1_DESC_SMALLPAGE_INDEX_SHIFT    12
#define PGT_L1_DESC_LARGEPAGE_INDEX_SHIFT    16
#define PGT_L1_DESC_SECTION_INDEX_SHIFT      20
#define PGT_L1_DESC_SUPERSECTION_INDEX_SHIFT 24

#define PGT_L2_DESC_SMALLPAGE               0x02
#define PGT_L2_DESC_LARGEPAGE               0x01

#define PGT_L2_DESC_SMALLPAGE_MASK          0xfffff000
#define PGT_L2_DESC_LARGEPAGE_MASK          0xffff0000

/*
 * The memory for the page tables (256 KB per IPU) is placed just before
 * the carveout memories for the remote processors. 16 KB of memory is
 * needed for the L1 page table (4096 entries * 4 bytes per 1 MB section).
 * Any smaller page (64 KB or 4 KB) entries are supported through L2 page
 * tables (1 KB per table). The remaining 240 KB can provide support for
 * 240 L2 page tables. Any remoteproc firmware image requiring more than
 * 240 L2 page table entries would need more memory to be reserved.
 */
#define PAGE_TABLE_SIZE_L1 (0x00004000)
#define PAGE_TABLE_SIZE_L2 (0x400)
#define MAX_NUM_L2_PAGE_TABLES (240)
#define PAGE_TABLE_SIZE_L2_TOTAL (MAX_NUM_L2_PAGE_TABLES * PAGE_TABLE_SIZE_L2)
#define PAGE_TABLE_SIZE (PAGE_TABLE_SIZE_L1 + (PAGE_TABLE_SIZE_L2_TOTAL))

/**
 * struct omap_rproc_mem - internal memory structure
 * @cpu_addr: MPU virtual address of the memory region
 * @bus_addr: bus address used to access the memory region
 * @dev_addr: device address of the memory region from DSP view
 * @size: size of the memory region
 */
struct omap_rproc_mem {
	void __iomem *cpu_addr;
	phys_addr_t bus_addr;
	u32 dev_addr;
	size_t size;
};

struct ipu_privdata {
	struct omap_rproc_mem mem;
	struct list_head mappings;
	const char *fw_name;
	u32 bootaddr;
	int id;
	struct udevice *rdev;
};

typedef int (*handle_resource_t) (void *, int offset, int avail);

unsigned int *page_table_l1 = (unsigned int *)0x0;
unsigned int *page_table_l2 = (unsigned int *)0x0;

/*
 * Set maximum carveout size to 96 MB
 */
#define DRA7_RPROC_MAX_CO_SIZE (96 * 0x100000)

/*
 * These global variables are used for deriving the MMU page tables. They
 * are initialized for each core with the appropriate values. The length
 * of the array mem_bitmap is set as per a 96 MB carveout which the
 * maximum set aside in the current memory map.
 */
unsigned long mem_base;
unsigned long mem_size;
unsigned long

mem_bitmap[BITS_TO_LONGS(DRA7_RPROC_MAX_CO_SIZE >> PAGE_SHIFT)];
unsigned long mem_count;

unsigned int pgtable_l2_map[MAX_NUM_L2_PAGE_TABLES];
unsigned int pgtable_l2_cnt;

void *ipu_alloc_mem(struct udevice *dev, unsigned long len, unsigned long align)
{
	unsigned long mask;
	unsigned long pageno;
	int count;

	count = ((len + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1)) >> PAGE_SHIFT;
	mask = (1 << align) - 1;
	pageno =
	    bitmap_find_next_zero_area(mem_bitmap, mem_count, 0, count, mask);
	debug("%s: count %d mask %#lx pageno %#lx\n", __func__, count, mask,
	      pageno);

	if (pageno >= mem_count) {
		debug("%s: %s Error allocating memory; "
		       "Please check carveout size\n", __FILE__, __func__);
		return NULL;
	}

	bitmap_set(mem_bitmap, pageno, count);
	return (void *)(mem_base + (pageno << PAGE_SHIFT));
}

int find_pagesz(unsigned int virt, unsigned int phys, unsigned int len)
{
	int pg_sz_ind = -1;
	unsigned int min_align = __ffs(virt);

	if (min_align > __ffs(phys))
		min_align = __ffs(phys);

	if (min_align >= PGT_L1_DESC_SUPERSECTION_INDEX_SHIFT &&
	    len >= 0x1000000) {
		pg_sz_ind = PAGESIZE_16M;
		goto ret_block;
	}
	if (min_align >= PGT_L1_DESC_SECTION_INDEX_SHIFT &&
	    len >= 0x100000) {
		pg_sz_ind = PAGESIZE_1M;
		goto ret_block;
	}
	if (min_align >= PGT_L1_DESC_LARGEPAGE_INDEX_SHIFT &&
	    len >= 0x10000) {
		pg_sz_ind = PAGESIZE_64K;
		goto ret_block;
	}
	if (min_align >= PGT_L1_DESC_SMALLPAGE_INDEX_SHIFT &&
	    len >= 0x1000) {
		pg_sz_ind = PAGESIZE_4K;
		goto ret_block;
	}

 ret_block:
	return pg_sz_ind;
}

int get_l2_pg_tbl_addr(unsigned int virt, unsigned int *pg_tbl_addr)
{
	int ret = -1;
	int i = 0;
	int match_found = 0;
	unsigned int tag = (virt & PGT_L1_DESC_SECTION_MASK);

	*pg_tbl_addr = 0;
	for (i = 0; (i < pgtable_l2_cnt) && (match_found == 0); i++) {
		if (tag == pgtable_l2_map[i]) {
			*pg_tbl_addr =
			    ((unsigned int)page_table_l2) +
			    (i * PAGE_TABLE_SIZE_L2);
			match_found = 1;
			ret = 0;
		}
	}

	if (match_found == 0 && i < MAX_NUM_L2_PAGE_TABLES) {
		pgtable_l2_map[i] = tag;
		pgtable_l2_cnt++;
		*pg_tbl_addr =
		    ((unsigned int)page_table_l2) + (i * PAGE_TABLE_SIZE_L2);
		ret = 0;
	}

	return ret;
}

int
config_l2_pagetable(unsigned int virt, unsigned int phys,
		    unsigned int pg_sz, unsigned int pg_tbl_addr)
{
	int ret = -1;
	unsigned int desc = 0;
	int i = 0;
	unsigned int *pg_tbl = (unsigned int *)pg_tbl_addr;

	/*
	 * Pick bit 19:12 of the virtual address as index
	 */
	unsigned int index = (virt & (~PGT_L1_DESC_SECTION_MASK)) >> PAGE_SHIFT;

	switch (pg_sz) {
	case PAGESIZE_64K:
		desc =
		    (phys & PGT_L2_DESC_LARGEPAGE_MASK) | PGT_L2_DESC_LARGEPAGE;
		for (i = 0; i < 16; i++)
			pg_tbl[index + i] = desc;
		ret = 0;
		break;
	case PAGESIZE_4K:
		desc =
		    (phys & PGT_L2_DESC_SMALLPAGE_MASK) | PGT_L2_DESC_SMALLPAGE;
		pg_tbl[index] = desc;
		ret = 0;
		break;
	default:
		break;
	}

	return ret;
}

unsigned int
ipu_config_pagetable(struct udevice *dev, unsigned int virt, unsigned int phys,
		     unsigned int len)
{
	unsigned int index;
	unsigned int l = len;
	unsigned int desc;
	int pg_sz = 0;
	int i = 0, err = 0;
	unsigned int pg_tbl_l2_addr = 0;
	unsigned int tmp_pgsz;

	if ((len & 0x0FFF) != 0)
		return 0;

	while (l > 0) {
		pg_sz = find_pagesz(virt, phys, l);
		index = virt >> PGT_L1_DESC_SECTION_INDEX_SHIFT;
		switch (pg_sz) {
			/*
			 * 16 MB super section
			 */
		case PAGESIZE_16M:
			/*
			 * Program the next 16 descriptors
			 */
			desc =
			    (phys & PGT_L1_DESC_SUPERSECTION_MASK) |
			    PGT_L1_DESC_SUPERSECTION;
			for (i = 0; i < 16; i++)
				page_table_l1[index + i] = desc;
			l -= PGT_SUPERSECTION_SIZE;
			phys += PGT_SUPERSECTION_SIZE;
			virt += PGT_SUPERSECTION_SIZE;
			break;
			/*
			 * 1 MB section
			 */
		case PAGESIZE_1M:
			desc =
			    (phys & PGT_L1_DESC_SECTION_MASK) |
			    PGT_L1_DESC_SECTION;
			page_table_l1[index] = desc;
			l -= PGT_SECTION_SIZE;
			phys += PGT_SECTION_SIZE;
			virt += PGT_SECTION_SIZE;
			break;
			/*
			 * 64 KB large page
			 */
		case PAGESIZE_64K:
		case PAGESIZE_4K:
			if (pg_sz == PAGESIZE_64K)
				tmp_pgsz = 0x10000;
			else
				tmp_pgsz = 0x1000;

			err = get_l2_pg_tbl_addr(virt, &pg_tbl_l2_addr);
			if (err != 0) {
				debug
				    ("Unable to get level 2 PT address\n");
				hang();
			}
			err =
			    config_l2_pagetable(virt, phys, pg_sz,
						pg_tbl_l2_addr);
			desc =
			    (pg_tbl_l2_addr & PGT_L1_DESC_PAGE_MASK) |
			    PGT_L1_DESC_PAGE;
			page_table_l1[index] = desc;
			l -= tmp_pgsz;
			phys += tmp_pgsz;
			virt += tmp_pgsz;
			break;
		case -1:
		default:
			return 0;
		}
	}

	return len;
}

int da_to_pa(struct udevice *dev, int da)
{
	struct rproc_mem_entry *maps = NULL;
	struct ipu_privdata *priv = dev_get_priv(dev);

	list_for_each_entry(maps, &priv->mappings, node) {
		if (da >= maps->da && da < (maps->da + maps->len))
			return maps->dma + (da - maps->da);
	}

	return 0;
}

u32 ipu_config_mmu(u32 core_id, struct rproc *cfg)
{
	u32 i = 0;
	u32 reg = 0;

	/*
	 * Clear the entire pagetable location before programming the
	 * address into the MMU
	 */
	memset((void *)cfg->page_table_addr, 0x00, PAGE_TABLE_SIZE);

	for (i = 0; i < cfg->num_iommus; i++) {
		u32 mmu_base = cfg->mmu_base_addr[i];

		__raw_writel((int)cfg->page_table_addr, mmu_base + 0x4c);
		reg = __raw_readl(mmu_base + 0x88);

		/*
		 * enable bus-error back
		 */
		__raw_writel(reg | 0x1, mmu_base + 0x88);

		/*
		 * Enable the MMU IRQs during MMU programming for the
		 * late attachcase. This is to allow the MMU fault to be
		 * detected by the kernel.
		 *
		 * MULTIHITFAULT|EMMUMISS|TRANSLATIONFAULT|TABLEWALKFAULT
		 */
		__raw_writel(0x1E, mmu_base + 0x1c);

		/*
		 * emutlbupdate|TWLENABLE|MMUENABLE
		 */
		__raw_writel(0x6, mmu_base + 0x44);
	}

	return 0;
}

/**
 * enum ipu_mem - PRU core memory range identifiers
 */
enum ipu_mem {
	PRU_MEM_IRAM = 0,
	PRU_MEM_CTRL,
	PRU_MEM_DEBUG,
	PRU_MEM_MAX,
};

static int ipu_start(struct udevice *dev)
{
	struct ipu_privdata *priv;
	struct reset_ctl reset;
	struct rproc *cfg = NULL;
	int ret;

	priv = dev_get_priv(dev);

	cfg = rproc_cfg_arr[priv->id];
	if (cfg->config_peripherals)
		cfg->config_peripherals(priv->id, cfg);

	/*
	 * Start running the remote core
	 */
	ret = reset_get_by_index(dev, 0, &reset);
	if (ret < 0) {
		dev_err(dev, "%s: error getting reset index %d\n", __func__, 0);
		return ret;
	}

	ret = reset_deassert(&reset);
	if (ret < 0) {
		dev_err(dev, "%s: error deasserting reset %d\n", __func__, 0);
		return ret;
	}

	ret = reset_get_by_index(dev, 1, &reset);
	if (ret < 0) {
		dev_err(dev, "%s: error getting reset index %d\n", __func__, 1);
		return ret;
	}

	ret = reset_deassert(&reset);
	if (ret < 0) {
		dev_err(dev, "%s: error deasserting reset %d\n", __func__, 1);
		return ret;
	}

	return 0;
}

static int ipu_stop(struct udevice *dev)
{
	return 0;
}

/**
 * ipu_init() - Initialize the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int ipu_init(struct udevice *dev)
{
	return 0;
}

static int ipu_add_res(struct udevice *dev, struct rproc_mem_entry *mapping)
{
	struct ipu_privdata *priv = dev_get_priv(dev);

	list_add_tail(&mapping->node, &priv->mappings);
	return 0;
}

static int ipu_load(struct udevice *dev, ulong addr, ulong size)
{
	Elf32_Ehdr *ehdr;	/* Elf header structure pointer */
	Elf32_Phdr *phdr;	/* Program header structure pointer */
	Elf32_Phdr proghdr;
	int va;
	int pa;
	int i;

	ehdr = (Elf32_Ehdr *)addr;
	phdr = (Elf32_Phdr *)(addr + ehdr->e_phoff);
	/*
	 * Load each program header
	 */
	for (i = 0; i < ehdr->e_phnum; ++i) {
		memcpy(&proghdr, phdr, sizeof(Elf32_Phdr));

		if (proghdr.p_type != PT_LOAD) {
			++phdr;
			continue;
		}

		va = proghdr.p_paddr;
		pa = da_to_pa(dev, va);
		if (pa)
			proghdr.p_paddr = pa;

		void *dst = (void *)(uintptr_t)proghdr.p_paddr;
		void *src = (void *)addr + proghdr.p_offset;

		debug("Loading phdr %i to 0x%p (%i bytes)\n", i, dst,
		      proghdr.p_filesz);
		if (proghdr.p_filesz)
			memcpy(dst, src, proghdr.p_filesz);

		flush_cache((unsigned long)dst, proghdr.p_memsz);

		++phdr;
	}

	return 0;
}

static const struct dm_rproc_ops ipu_ops = {
	.init = ipu_init,
	.start = ipu_start,
	.stop = ipu_stop,
	.load = ipu_load,
	.add_res = ipu_add_res,
	.config_pagetable = ipu_config_pagetable,
	.alloc_mem = ipu_alloc_mem,
};

/*
 * If the remotecore binary expects any peripherals to be setup before it has
 * booted, configure them here.
 *
 * These functions are left empty by default as their operation is usecase
 * specific.
 */

u32 ipu1_config_peripherals(u32 core_id, struct rproc *cfg)
{
	return 0;
}

u32 ipu2_config_peripherals(u32 core_id, struct rproc *cfg)
{
	return 0;
}

struct rproc_intmem_to_l3_mapping ipu1_intmem_to_l3_mapping = {
	.num_entries = 1,
	.mappings = {
		     /*
		      * L2 SRAM
		      */
			{
				.priv_addr = 0x55020000,
				.l3_addr = 0x58820000,
				.len = (64 * 1024)},
			}
};

struct rproc_intmem_to_l3_mapping ipu2_intmem_to_l3_mapping = {
	.num_entries = 1,
	.mappings = {
		     /*
		      * L2 SRAM
		      */
			{
				.priv_addr = 0x55020000,
				.l3_addr = 0x55020000,
				.len = (64 * 1024)},
			}
};

struct rproc ipu1_config = {
	.num_iommus = 1,
	.mmu_base_addr = {0x58882000, 0},
	.load_addr = IPU1_LOAD_ADDR,
	.core_name = "IPU1",
	.firmware_name = "dra7-ipu1-fw.xem4",
	.config_mmu = ipu_config_mmu,
	.config_peripherals = ipu1_config_peripherals,
	.intmem_to_l3_mapping = &ipu1_intmem_to_l3_mapping
};

struct rproc ipu2_config = {
	.num_iommus = 1,
	.mmu_base_addr = {0x55082000, 0},
	.load_addr = IPU2_LOAD_ADDR,
	.core_name = "IPU2",
	.firmware_name = "dra7-ipu2-fw.xem4",
	.config_mmu = ipu_config_mmu,
	.config_peripherals = ipu2_config_peripherals,
	.intmem_to_l3_mapping = &ipu2_intmem_to_l3_mapping
};

struct rproc *rproc_cfg_arr[2] = {
	[IPU2] = &ipu2_config,
	[IPU1] = &ipu1_config,
};

u32 spl_pre_boot_core(struct udevice *dev, u32 core_id)
{
	struct rproc *cfg = NULL;
	unsigned long load_elf_status = 0;
	int tablesz;

	cfg = rproc_cfg_arr[core_id];
	/*
	 * Check for valid elf image
	 */
	if (!valid_elf_image(cfg->load_addr))
		return 1;

	if (rproc_find_resource_table(dev, cfg->load_addr, &tablesz))
		cfg->has_rsc_table = 1;
	else
		cfg->has_rsc_table = 0;

	/*
	 * Configure the MMU
	 */
	if (cfg->config_mmu && cfg->has_rsc_table)
		cfg->config_mmu(core_id, cfg);

	/*
	 * Load the remote core. Fill the page table of the first(possibly
	 * only) IOMMU during ELF loading.  Copy the page table to the second
	 * IOMMU before running the remote core.
	 */

	page_table_l1 = (unsigned int *)cfg->page_table_addr;
	page_table_l2 =
	    (unsigned int *)(cfg->page_table_addr + PAGE_TABLE_SIZE_L1);
	mem_base = cfg->cma_base;
	mem_size = cfg->cma_size;
	memset(mem_bitmap, 0x00, sizeof(mem_bitmap));
	mem_count = (cfg->cma_size >> PAGE_SHIFT);

	/*
	 * Clear variables used for level 2 page table allocation
	 */
	memset(pgtable_l2_map, 0x00, sizeof(pgtable_l2_map));
	pgtable_l2_cnt = 0;

	load_elf_status = rproc_parse_resource_table(dev, cfg);
	if (load_elf_status == 0) {
		debug("load_elf_image_phdr returned error for core %s\n",
		      cfg->core_name);
		return 1;
	}

	flush_cache(cfg->page_table_addr, PAGE_TABLE_SIZE);

	return 0;
}

static fdt_addr_t ipu_parse_mem_nodes(struct udevice *dev, char *name,
				      int privid, fdt_size_t *sizep)
{
	int ret;
	u32 sp;
	ofnode mem_node;

	ret = ofnode_read_u32(dev_ofnode(dev), name, &sp);
	if (ret) {
		dev_err(dev, "memory-region node fetch failed %d\n", ret);
		return ret;
	}

	mem_node = ofnode_get_by_phandle(sp);
	if (!ofnode_valid(mem_node))
		return -EINVAL;

	return ofnode_get_addr_size_index(mem_node, 0, sizep);
}

/**
 * ipu_probe() - Basic probe
 * @dev:	corresponding k3 remote processor device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int ipu_probe(struct udevice *dev)
{
	struct ipu_privdata *priv;
	struct rproc *cfg = NULL;
	struct reset_ctl reset;
	static const char *const ipu_mem_names[] = { "l2ram" };
	int ret;
	fdt_size_t sizep;

	priv = dev_get_priv(dev);

	priv->mem.bus_addr =
		devfdt_get_addr_size_name(dev,
					  ipu_mem_names[0],
					  (fdt_addr_t *)&priv->mem.size);

	ret = reset_get_by_index(dev, 2, &reset);
	if (ret < 0) {
		dev_err(dev, "%s: error getting reset index %d\n", __func__, 2);
		return ret;
	}

	ret = reset_deassert(&reset);
	if (ret < 0) {
		dev_err(dev, "%s: error deasserting reset %d\n", __func__, 2);
		return ret;
	}

	if (priv->mem.bus_addr == FDT_ADDR_T_NONE) {
		dev_err(dev, "%s bus address not found\n", ipu_mem_names[0]);
		return -EINVAL;
	}
	priv->mem.cpu_addr = map_physmem(priv->mem.bus_addr,
					 priv->mem.size, MAP_NOCACHE);

	if (devfdt_get_addr(dev) == 0x58820000)
		priv->id = 0;
	else
		priv->id = 1;

	cfg = rproc_cfg_arr[priv->id];
	cfg->cma_base = ipu_parse_mem_nodes(dev, "memory-region", priv->id,
					    &sizep);
	cfg->cma_size = sizep;

	cfg->page_table_addr = ipu_parse_mem_nodes(dev, "pg-tbl", priv->id,
						   &sizep);

	dev_info(dev,
		 "ID %d memory %8s: bus addr %pa size 0x%zx va %p da 0x%x\n",
		priv->id, ipu_mem_names[0], &priv->mem.bus_addr,
		priv->mem.size, priv->mem.cpu_addr, priv->mem.dev_addr);

	INIT_LIST_HEAD(&priv->mappings);
	if (spl_pre_boot_core(dev, priv->id))
		return -EINVAL;

	return 0;
}

static const struct udevice_id ipu_ids[] = {
	{.compatible = "ti,dra7-ipu"},
	{}
};

U_BOOT_DRIVER(ipu) = {
	.name = "ipu",
	.of_match = ipu_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &ipu_ops,
	.probe = ipu_probe,
	.priv_auto = sizeof(struct ipu_privdata),
};
