/*
 * Copyright © 2023 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or from the Free Software
 * Foundation at https://www.gnu.org/licenses/ .
 */

#include <asm/page.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/kasan.h>
#include <linux/libfdt.h>
#include <linux/memblock.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/vme.h>
#include <linux/sched.h>
#include <linux/sizes.h>
#include <linux/sort.h>
#include <linux/brcmstb/bmem.h>
#include <linux/brcmstb/cma_driver.h>
#include <linux/brcmstb/memory_api.h>
#include <asm/tlbflush.h>
#include <linux/pfn_t.h>
#include <linux/memory_hotplug.h>
#include <linux/swiotlb.h>
#include <linux/kmemleak.h>
#if IS_ENABLED(CONFIG_BRCMSTB_HUGEPAGES)
#include <linux/dmb.h>
#endif
#ifdef CONFIG_ARM
#include <asm/early_ioremap.h>
#endif
#include <asm/fixmap.h>
#include <asm/cputype.h>

#if IS_ENABLED(CONFIG_BRCMSTB_HUGEPAGES)
#include "bhpa.h"
#endif

extern unsigned long __initramfs_size;

/* -------------------- Constants -------------------- */

#define DEFAULT_LOWMEM_PCT	20  /* used if only one membank */

/* Macros to help extract property data */
#define U8TOU32(b, offs) \
	((((u32)b[0+offs] << 0)  & 0x000000ff) | \
	 (((u32)b[1+offs] << 8)  & 0x0000ff00) | \
	 (((u32)b[2+offs] << 16) & 0x00ff0000) | \
	 (((u32)b[3+offs] << 24) & 0xff000000))

#define DT_PROP_DATA_TO_U32(b, offs) (fdt32_to_cpu(U8TOU32(b, offs)))

/* Constants used when retrieving memc info */
#define NUM_BUS_RANGES 10
#define BUS_RANGE_ULIMIT_SHIFT 4
#define BUS_RANGE_LLIMIT_SHIFT 4
#define BUS_RANGE_PA_SHIFT 12

/* platform dependant memory flags */
#if defined(CONFIG_BMIPS_GENERIC)
#define BCM_MEM_MASK (_PAGE_VALID)
#elif defined(CONFIG_ARM64)
#define BCM_MEM_MASK (PTE_ATTRINDX_MASK | PTE_TYPE_MASK)
#elif defined(CONFIG_ARM)
#define BCM_MEM_MASK (L_PTE_MT_MASK | L_PTE_VALID)
#else
#error "Platform not supported by bmem"
#endif

enum {
	BUSNUM_MCP0 = 0x4,
	BUSNUM_MCP1 = 0x5,
	BUSNUM_MCP2 = 0x6,
};

#define BMEM_LARGE_ENOUGH	(SZ_1G)

#if IS_BUILTIN(CONFIG_BRCMSTB_MEMORY_API)
/* -------------------- Shared and local vars -------------------- */
enum brcmstb_reserve_type brcmstb_default_reserve =
#if IS_ENABLED(CONFIG_BRCMSTB_HUGEPAGES)
BRCMSTB_RESERVE_BHPA
#elif IS_ENABLED(CONFIG_BRCMSTB_BMEM)
BRCMSTB_RESERVE_BMEM
#elif IS_ENABLED(CONFIG_BRCMSTB_CMA)
BRCMSTB_RESERVE_CMA
#else
#error "no default reservation strategy"
#endif
;

bool brcmstb_memory_override_defaults = false;
bool brcmstb_bmem_is_bhpa = false;
#endif /* IS_BUILTIN(CONFIG_BRCMSTB_MEMORY_API) */

static bool brcmstb_populate_reserved_called;
static struct brcmstb_reserved_memory reserved_init;

#ifdef CONFIG_PAGE_AUTOMAP
static spinlock_t automap_lock = __SPIN_LOCK_UNLOCKED(automap_lock);
#endif

/* -------------------- Functions -------------------- */
static void __maybe_unused brcm_online_callback(struct page *page,
						unsigned int order)
{
	/* Do Nothing */
}

/*
 * If the DT nodes are handy, determine which MEMC holds the specified
 * physical address.
 */
#ifdef CONFIG_ARCH_BRCMSTB
int __brcmstb_memory_phys_addr_to_memc(phys_addr_t pa, void __iomem *base,
				       const char *compat)
{
	const char *bcm7211_biuctrl_match = "brcm,bcm7211-cpu-biu-ctrl";
	const char *bcm72113_biuctrl_match = "brcm,bcm72113-cpu-biu-ctrl";
	int memc = -1;
	int i;

	/* Single MEMC controller with unreadable ULIMIT values as of the A0 */
	if (!strncmp(compat, bcm7211_biuctrl_match,
		     strlen(bcm7211_biuctrl_match)))
		return 0;
	if (!strncmp(compat, bcm72113_biuctrl_match,
		     strlen(bcm72113_biuctrl_match)))
		return 0;

	for (i = 0; i < NUM_BUS_RANGES; i++, base += 8) {
		const u64 ulimit_raw = readl(base);
		const u64 llimit_raw = readl(base + 4);
		const u64 ulimit =
			((ulimit_raw >> BUS_RANGE_ULIMIT_SHIFT)
			 << BUS_RANGE_PA_SHIFT) | 0xfff;
		const u64 llimit = (llimit_raw >> BUS_RANGE_LLIMIT_SHIFT)
				   << BUS_RANGE_PA_SHIFT;
		const u32 busnum = (u32)(ulimit_raw & 0xf);

		if (pa >= llimit && pa <= ulimit) {
			if (busnum >= BUSNUM_MCP0 && busnum <= BUSNUM_MCP2) {
				memc = busnum - BUSNUM_MCP0;
				break;
			}
		}
	}

	return memc;
}

int brcmstb_memory_phys_addr_to_memc(phys_addr_t pa)
{
	int memc = 0;
	struct device_node *np;
	void __iomem *cpubiuctrl;
	const char *compat;

	np = of_find_compatible_node(NULL, NULL, "brcm,brcmstb-cpu-biu-ctrl");
	if (!np)
		return memc;

	compat = of_get_property(np, "compatible", NULL);

	cpubiuctrl = of_iomap(np, 0);
	if (!cpubiuctrl) {
		memc = -ENOMEM;
		goto cleanup;
	}

	memc = __brcmstb_memory_phys_addr_to_memc(pa, cpubiuctrl, compat);
	iounmap(cpubiuctrl);

cleanup:
	of_node_put(np);

	return memc;
}

#if IS_BUILTIN(CONFIG_BRCMSTB_MEMORY_API)
static int __init early_phys_addr_to_memc(phys_addr_t pa)
{
	int memc = -1;
	void __iomem *cpubiuctrl;
	resource_size_t start;
	const void *fdt = initial_boot_params;
	int offset, proplen, cpubiuctrl_size;
	const struct fdt_property *prop;

	if (!fdt)
		return memc;

	offset = fdt_node_offset_by_compatible(fdt, -1,
					       "brcm,brcmstb-cpu-biu-ctrl");
	if (offset < 0)
		return 0;

	prop = fdt_get_property(fdt, offset, "reg", &proplen);
	if (proplen != (2 * sizeof(u32)))
		return memc;

	start = (resource_size_t)DT_PROP_DATA_TO_U32(prop->data, 0);
	cpubiuctrl_size = DT_PROP_DATA_TO_U32(prop->data, sizeof(u32));
	cpubiuctrl = early_ioremap(start, cpubiuctrl_size);
	if (!cpubiuctrl)
		return memc;

	prop = fdt_get_property(fdt, offset, "compatible", NULL);

	memc = __brcmstb_memory_phys_addr_to_memc(pa, cpubiuctrl, prop->data);
	early_iounmap(cpubiuctrl, cpubiuctrl_size);

	return memc;
}

static const char *__get_mem_layout_early(int *size_cells, int *addr_cells,
					  int *proplen)
{
	const void *fdt = initial_boot_params;
	int mem_offset;
	const struct fdt_property *prop;

	if (!fdt) {
		pr_err("No device tree?\n");
		return NULL;
	}

	/* Get root size and address cells if specified */
	prop = fdt_get_property(fdt, 0, "#size-cells", proplen);
	if (prop)
		*size_cells = DT_PROP_DATA_TO_U32(prop->data, 0);

	prop = fdt_get_property(fdt, 0, "#address-cells", proplen);
	if (prop)
		*addr_cells = DT_PROP_DATA_TO_U32(prop->data, 0);

	mem_offset = fdt_path_offset(fdt, "/memory");
	if (mem_offset < 0) {
		pr_err("No memory node?\n");
		return NULL;
	}

	prop = fdt_get_property(fdt, mem_offset, "reg", proplen);
	if (!prop)
		return NULL;

	return prop->data;
}
#endif /* CONFIG_BRCMSTB_MEMORY_API */
#elif defined(CONFIG_MIPS)
#define early_phys_addr_to_memc brcmstb_memory_phys_addr_to_memc
int brcmstb_memory_phys_addr_to_memc(phys_addr_t pa)
{
	/* The logic here is fairly simple and hardcoded: if pa <= 0x5000_0000,
	 * then this is MEMC0, else MEMC1.
	 *
	 * For systems with 2GB on MEMC0, MEMC1 starts at 9000_0000, with 1GB
	 * on MEMC0, MEMC1 starts at 6000_0000.
	 */
	if (pa >= 0x50000000ULL)
		return 1;
	else
		return 0;
}
#endif
EXPORT_SYMBOL(brcmstb_memory_phys_addr_to_memc);


static const char *__get_mem_layout(int *size_cells, int *addr_cells,
				    int *proplen)
{
	struct device_node *dn;
	const char *value;

	dn = of_find_node_by_path("/");
	if (!dn)
		return NULL;

	*size_cells = of_n_size_cells(dn);
	*addr_cells = of_n_addr_cells(dn);
	of_node_put(dn);

	dn = of_find_node_by_path("/memory");
	if (!dn) {
		pr_err("No memory node?\n");
		return NULL;
	}

	value = of_get_property(dn, "reg", proplen);
	of_node_put(dn);
	return value;
}

static const char *get_mem_layout(int *size_cells, int *addr_cells,
				  int *proplen, bool early)
{
#if IS_BUILTIN(CONFIG_BRCMSTB_MEMORY_API)
	if (early)
		return __get_mem_layout_early(size_cells, addr_cells, proplen);
	else
#endif
		return __get_mem_layout(size_cells, addr_cells, proplen);
}

static int __ref __for_each_memc_range(int (*fn)(int m, u64 a, u64 s, void *c),
				       void *c, bool early)
{
	int addr_cells, size_cells;
	const char *prop_data;
	int cellslen, proplen;
	int i, ret;

	if (!fn)
		return -EINVAL;

	prop_data = get_mem_layout(&size_cells, &addr_cells, &proplen,
				   early);
	if (!prop_data)
		return -EINVAL;

	cellslen = (int)sizeof(u32) * (addr_cells + size_cells);
	if (!prop_data || (proplen % cellslen) != 0) {
		pr_err("Invalid length of reg prop: %d\n", proplen);
		return -EINVAL;
	}

	for (i = 0; i < proplen / cellslen; ++i) {
		u64 addr = 0;
		u64 size = 0;
		int memc;
		int j;

		for (j = 0; j < addr_cells; ++j) {
			int offset = (cellslen * i) + (sizeof(u32) * j);
			addr |= (u64)DT_PROP_DATA_TO_U32(prop_data, offset) <<
				((addr_cells - j - 1) * 32);
		}
		for (j = 0; j < size_cells; ++j) {
			int offset = (cellslen * i) +
				(sizeof(u32) * (j + addr_cells));
			size |= (u64)DT_PROP_DATA_TO_U32(prop_data, offset) <<
				((size_cells - j - 1) * 32);
		}

#if IS_BUILTIN(CONFIG_BRCMSTB_MEMORY_API)
		if (early)
			memc = early_phys_addr_to_memc((phys_addr_t)addr);
		else
#endif
			memc = brcmstb_memory_phys_addr_to_memc((phys_addr_t)addr);

		ret = fn(memc, addr, size, c);
		if (ret)
			return ret;
	}

	return 0;
}

int for_each_memc_range(int (*fn)(int m, u64 a, u64 s, void *c), void *c)
{
	return __for_each_memc_range(fn, c, false);
}

int __init early_for_each_memc_range(int (*fn)(int m, u64 a, u64 s, void *c),
				     void *c)
{
	return __for_each_memc_range(fn, c, true);
}

struct memc_size_ctx
{
	int	memc;
	u64	size;
};

static int memc_size(int memc, u64 addr, u64 size, void *context)
{
	struct memc_size_ctx *ctx = context;

	if ((phys_addr_t)addr != addr) {
		pr_err("phys_addr_t is smaller than provided address 0x%llx!\n",
			addr);
		return -EINVAL;
	}

	if (memc == ctx->memc)
		ctx->size += size;

	return 0;
}

u64 brcmstb_memory_memc_size(int memc)
{
	struct memc_size_ctx ctx;

	ctx.memc = memc;
	ctx.size = 0;
	for_each_memc_range(memc_size, &ctx);

	return ctx.size;
}
EXPORT_SYMBOL(brcmstb_memory_memc_size);

static int set_memc_range(int memc_idx, u64 addr, u64 size, void *context)
{
	struct brcmstb_memory *mem = context;
	int range_idx;

	if ((phys_addr_t)addr != addr) {
		pr_err("phys_addr_t is smaller than provided address 0x%llx!\n",
			addr);
		return -EINVAL;
	}

	if (memc_idx == -1) {
		pr_err("address 0x%llx does not appear to be in any memc\n",
			addr);
		return -EINVAL;
	}

	range_idx = mem->memc[memc_idx].count;
	if (mem->memc[memc_idx].count >= MAX_BRCMSTB_RANGE)
		pr_warn("%s: Exceeded max ranges for memc%d\n",
				__func__, memc_idx);
	else {
		mem->memc[memc_idx].range[range_idx].addr = addr;
		mem->memc[memc_idx].range[range_idx].size = size;
	}
	++mem->memc[memc_idx].count;

	return 0;
}

static int populate_memc(struct brcmstb_memory *mem)
{
	return for_each_memc_range(set_memc_range, mem);
}

static int populate_lowmem(struct brcmstb_memory *mem)
{
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64) || \
	defined(CONFIG_BMIPS_GENERIC)
	mem->lowmem.range[0].addr = __pa(PAGE_OFFSET);
	mem->lowmem.range[0].size = (unsigned long)high_memory - PAGE_OFFSET;
	++mem->lowmem.count;
	return 0;
#else
	return -ENOSYS;
#endif
}

static int populate_bmem(struct brcmstb_memory *mem)
{
#ifdef CONFIG_BRCMSTB_BMEM
	phys_addr_t addr, size;
	int i;

	for (i = 0; i < MAX_BRCMSTB_RANGE; ++i) {
		if (bmem_region_info(i, &addr, &size))
			break;  /* no more regions */
		mem->bmem.range[i].addr = addr;
		mem->bmem.range[i].size = size;
		++mem->bmem.count;
	}
	if (i >= MAX_BRCMSTB_RANGE) {
		while (bmem_region_info(i, &addr, &size) == 0) {
			pr_warn("%s: Exceeded max ranges\n", __func__);
			++mem->bmem.count;
		}
	}

	return 0;
#else
	return -ENOSYS;
#endif
}

static int populate_cma(struct brcmstb_memory *mem)
{
#ifdef CONFIG_BRCMSTB_CMA
	int i;

	for (i = 0; i < CMA_NUM_RANGES; ++i) {
		struct cma_dev *cdev = cma_dev_get_cma_dev(i);
		if (cdev == NULL)
			break;
		if (i >= MAX_BRCMSTB_RANGE)
			pr_warn("%s: Exceeded max ranges\n", __func__);
		else {
			mem->cma.range[i].addr = cdev->range.base;
			mem->cma.range[i].size = cdev->range.size;
		}
		++mem->cma.count;
	}

	return 0;
#else
	return -ENOSYS;
#endif
}

struct reserved_mem {
	phys_addr_t	base;
	phys_addr_t	size;
	char		reserved_name[MAX_BRCMSTB_RESERVED_NAME];
};

static struct reserved_mem brcmstb_reserved_mem_entries[MAX_BRCMSTB_RANGE];
static unsigned int brcmstb_reserved_mem_count;
static inline int __reserved_mem_get_count(void)
{
	return brcmstb_reserved_mem_count;
}
static inline struct reserved_mem *__reserved_mem_get_entry(int pos)
{
	if (pos > brcmstb_reserved_mem_count)
		return NULL;

	return &brcmstb_reserved_mem_entries[pos];
}

static inline bool address_within_range(phys_addr_t addr, phys_addr_t size,
				      struct brcmstb_range *range)
{
	return (addr >= range->addr &&
		addr + size <= range->addr + range->size);
}

static inline bool rmem_within_range(struct reserved_mem *rmem,
				     struct brcmstb_range *range)
{
	if (!rmem || !rmem->size)
		return false;

	return address_within_range(rmem->base, rmem->size, range);
}

static int range_exists(phys_addr_t addr, phys_addr_t size,
			struct brcmstb_reserved_memory *reserved)
{
	struct brcmstb_range *range;
	unsigned int i;

	/* Don't consider 0 size ranges as valid */
	if (!size)
		return reserved->count;

	for (i = 0; i < reserved->count; i++) {
		range = &reserved->range[i];
		if (range->addr == addr)
			return i;
	}

	return -1;
}

static void range_truncate(phys_addr_t addr, phys_addr_t *size)
{
	struct brcmstb_range range = {
		.addr = addr,
		.size = *size,
	};
	int count = __reserved_mem_get_count();
	struct reserved_mem *rmem;
	int i;

	/* Check if we have an neighboring rmem entry, and if so, adjust
	 * the range by how much
	 */
	for (i = 0; i < count; i++) {
		rmem = __reserved_mem_get_entry(i);
		if (rmem_within_range(rmem, &range) && rmem->reserved_name[0])
			*size -= rmem->size;
	}
}

/*
 * Attempts the insertion of a given reserved memory entry (rmem) into
 * a larger range using reserved as a database entry
 *
 * return values:
 * true if the entry was successfully inserted (could have been more than just one)
 * false if the entry was not inserted (not a valid candidate)
 */
static bool rmem_insert_into_range(struct reserved_mem *rmem,
				   struct brcmstb_range *range,
				   struct brcmstb_reserved_memory *reserved)
{
	int len = MAX_BRCMSTB_RESERVED_NAME;
	phys_addr_t size1 = 0, size2 = 0;
	int i = reserved->count;
	int j;

	if (!rmem->size)
		return false;

	/* This region does not have a name, it must be part of its larger
	 * range then.
	 */
	if (!rmem->reserved_name[0])
		return false;

	/* If we have memory below us, we should report it but we also need to
	 * make sure we are not reporting an existing range.
	 */
	if (range) {
		/* Get the memory below */
		size1 = rmem->base - range->addr;
		if (range_exists(range->addr, size1, reserved) < 0) {
			reserved->range[i].addr = range->addr;
			reserved->range[i].size = size1;
			reserved->count++;
			i = reserved->count;
		}
	}

	/* We may have already inserted this rmem entry before, but
	 * without its name, so find it, and update the location
	 */
	j = range_exists(rmem->base, rmem->size, reserved);
	if (j >= 0 && range)
		i = j;

	strncpy(reserved->range_name[i].name, rmem->reserved_name, len);
	reserved->range[i].addr = rmem->base;
	reserved->range[i].size = rmem->size;
	/* Only increment if this was not an existing location we re-used */
	if (i != j)
		reserved->count++;
	i = reserved->count;

	/* If we have memory above us, we should also report it but
	 * we also need to make sure we are not reporting an existing
	 * range either
	 */
	if (range) {
		size2 = (range->addr + range->size) - (rmem->base + rmem->size);
		size1 = rmem->base + rmem->size;
		if (range_exists(rmem->base + rmem->size, size2, reserved) < 0) {
			range_truncate(size1, &size2);
			reserved->range[i].addr = size1;
			reserved->range[i].size = size2;
			reserved->count++;
		}
	}

	return true;
}

static bool contiguous_range_exists(struct brcmstb_range *range,
				    struct brcmstb_reserved_memory *reserved)
{
	struct brcmstb_range *iter;
	phys_addr_t total_size = 0;
	unsigned int i;

	for (i = 0; i < reserved->count; i++) {
		iter = &reserved->range[i];
		if (address_within_range(iter->addr, iter->size, range))
			total_size += iter->size;
	}

	return total_size == range->size;
}

static int reserved_mem_cmp_func(const void *a, const void *b)
{
	const struct reserved_mem *ra = a, *rb = b;

	return ra->base < rb->base ? -1 : 1;
}

#if IS_BUILTIN(CONFIG_BRCMSTB_MEMORY_API)
static int brcmstb_populate_reserved(void)
{
	int offset, lenp, ret, count, cellslen;
	const void *fdt = initial_boot_params;
	int addr_cells = 0, size_cells = 0;
	const struct fdt_property *prop;
	const char *name;
	int i = 0, j;
	u64 addr;
	u64 size;

	if (!fdt) {
		pr_err("No device tree?\n");
		return -EINVAL;
	}

	/* Start with the /memreserve entries */
	count = fdt_num_mem_rsv(fdt);
	if (count > 0) {
		for (i = 0; i < count && i < MAX_BRCMSTB_RESERVED_RANGE; i++) {
			ret = fdt_get_mem_rsv(fdt, i, &addr, &size);
			if (ret == 0) {
				brcmstb_reserved_mem_entries[i].base = addr;
				brcmstb_reserved_mem_entries[i].size = size;
				brcmstb_reserved_mem_count++;
			}
		}
	}

	if (i == MAX_BRCMSTB_RESERVED_RANGE)
		return 0;

	offset = fdt_path_offset(fdt, "/reserved-memory");
	if (offset < 0)
		return 0;

	prop = fdt_get_property(fdt, offset, "#address-cells", &lenp);
	if (prop)
		addr_cells = DT_PROP_DATA_TO_U32(prop->data, 0);

	prop = fdt_get_property(fdt, offset, "#size-cells", &lenp);
	if (prop)
		size_cells = DT_PROP_DATA_TO_U32(prop->data, 0);

	offset = fdt_first_subnode(fdt, offset);
	do {
		prop = fdt_get_property(fdt, offset, "reserved-names", &lenp);
		name = prop->data;
		if (name && lenp >= 0)
			count = lenp;
		else
			count = 0;

		prop = fdt_get_property(fdt, offset, "reg", &lenp);
		cellslen = (int)sizeof(u32) * (addr_cells + size_cells);
		if ((lenp % cellslen) != 0)
			goto out;

		addr = 0;
		size = 0;
		for (j = 0; j < addr_cells; ++j) {
			int offset = (sizeof(u32) * j);
			addr |= (u64)DT_PROP_DATA_TO_U32(prop->data, offset) <<
				((addr_cells - j - 1) * 32);
		}

		for (j = 0; j < size_cells; ++j) {
			int offset = (sizeof(u32) * (j + addr_cells));
			size |= (u64)DT_PROP_DATA_TO_U32(prop->data, offset) <<
				((size_cells - j - 1) * 32);
		}

		/* Add name, address and size to the reserved memory list */
		brcmstb_reserved_mem_entries[i].base = addr;
		brcmstb_reserved_mem_entries[i].size = size;
		if (count)
			strncpy(brcmstb_reserved_mem_entries[i].reserved_name,
				name, min(MAX_BRCMSTB_RESERVED_NAME, count));
		brcmstb_reserved_mem_count++;
		i++;
out:
		offset = fdt_next_subnode(fdt, offset);
	} while (offset >= 0 && i < MAX_BRCMSTB_RANGE);

	sort(brcmstb_reserved_mem_entries, brcmstb_reserved_mem_count,
	     sizeof(struct reserved_mem), reserved_mem_cmp_func, NULL);

	return 0;
}
#endif /* IS_BUILTIN(CONFIG_BRCMSTB_MEMORY_API) */

static int populate_reserved_late(struct brcmstb_reserved_memory *reserved)
{
	struct device_node *rn, *dn;
	int lenp, count, ret;
	unsigned int i = 0;
	struct resource r;
	const char *name;

	rn = of_find_node_by_path("/reserved-memory");
	if (!rn)
		return 0;

	for_each_child_of_node(rn, dn) {
		if (of_device_is_compatible(dn, "brcm,bhpa") ||
		    of_device_is_compatible(dn, "designated-movable-block"))
			continue;

		name = of_get_property(dn, "reserved-names", &lenp);
		if (name && lenp >= 0)
			count = lenp;
		else
			count = 0;

		ret = of_address_to_resource(dn, 0, &r);
		if (ret)
			continue;

		/* Add name, address and size to the reserved memory list */
		brcmstb_reserved_mem_entries[i].base = r.start;
		brcmstb_reserved_mem_entries[i].size = resource_size(&r);
		if (count)
			strncpy(brcmstb_reserved_mem_entries[i].reserved_name,
				name, min(MAX_BRCMSTB_RESERVED_NAME, count));
		i++;
		if (i >= MAX_BRCMSTB_RANGE)
			break;
	}

	of_node_put(rn);

	brcmstb_reserved_mem_count = i;
	reserved->count = i;

	sort(brcmstb_reserved_mem_entries, brcmstb_reserved_mem_count,
	     sizeof(struct reserved_mem), reserved_mem_cmp_func, NULL);

	for (i = 0; i < brcmstb_reserved_mem_count; i++) {
		reserved->range[i].addr = brcmstb_reserved_mem_entries[i].base;
		reserved->range[i].size = brcmstb_reserved_mem_entries[i].size;
		if (!brcmstb_reserved_mem_entries[i].reserved_name[0])
			continue;

		count = strlen(brcmstb_reserved_mem_entries[i].reserved_name);
		strncpy(reserved->range_name[i].name,
			brcmstb_reserved_mem_entries[i].reserved_name,
			min(count, MAX_BRCMSTB_RESERVED_NAME));
	}

	return 0;
}

static int populate_reserved(struct brcmstb_memory *mem)
{
	struct brcmstb_reserved_memory reserved;
	struct brcmstb_range *range;
	struct reserved_mem *rmem;
	int count, i, j;
	bool added;

	/* Only call populate_reserved_late if the early population was
	 * not done
	 */
	if (!brcmstb_populate_reserved_called) {
		populate_reserved_late(&mem->reserved);
		return 0;
	}

	memset(&reserved, 0, sizeof(reserved));

	count = __reserved_mem_get_count();

	/* No reserved-memory entries, or OF_RESERVED_MEM not built, just
	 * report what we already have */
	if (count <= 0) {
		memcpy(&mem->reserved, &reserved_init, sizeof(reserved_init));
		return 0;
	}

	count = min(count, MAX_BRCMSTB_RESERVED_RANGE);

	/* Loop through the FDT reserved memory regions, first pass
	 * will split the existing reserved ranges into smaller
	 * name-based reserved regions
	 */
	for (i = 0; i < reserved_init.count; i++) {
		range = &reserved_init.range[i];
		added = false;
		for (j = 0; j < count; j++) {
			added = false;
			rmem = __reserved_mem_get_entry(j);
			if (rmem_within_range(rmem, range))
				added = rmem_insert_into_range(rmem, range,
							       &reserved);
		}

		/* rmem_insert_into_range() may be splitting a larger range into
		 * contiguous parts, so we need to check that here too to avoid
		 * re-inserting it another time
		 */
		if (!added && range->size &&
		    !contiguous_range_exists(range, &reserved)) {
			reserved.range[reserved.count].addr = range->addr;
			reserved.range[reserved.count].size = range->size;
			reserved.count++;
		}
	}

	/* Second loop takes care of "no-map" regions which do not show up
	 * in reserved_init and need to be checked separately
	 */
	for (i = 0; i < count; i++) {
		rmem = __reserved_mem_get_entry(i);
		if (!memblock_is_map_memory(rmem->base))
			rmem_insert_into_range(rmem, NULL, &reserved);
	}

	memcpy(&mem->reserved, &reserved, sizeof(reserved));

	return 0;
}

#if IS_ENABLED(CONFIG_BRCMSTB_HUGEPAGES)
struct bhpa_region bhpa_regions[MAX_BHPA_REGIONS];
EXPORT_SYMBOL(bhpa_regions);
unsigned int n_bhpa_regions;
EXPORT_SYMBOL(n_bhpa_regions);

/*
 * Finds the IDX'th bhpa region, and fills in addr/size if it exists.
 * Returns 0 on success, <0 on failure.
 * Can pass in NULL for addr and/or size if you only care about return value.
 */
static int _bhpa_region_info(int idx, phys_addr_t *addr, phys_addr_t *size)
{
	if (idx >= n_bhpa_regions)
		return -ENOENT;

	if (addr)
		*addr = bhpa_regions[idx].addr;
	if (size)
		*size = bhpa_regions[idx].size;

	return 0;
}
#endif

static int populate_bhpa(struct brcmstb_memory *mem)
{
#if IS_ENABLED(CONFIG_BRCMSTB_HUGEPAGES)
	phys_addr_t addr, size;
	int i;

	for (i = 0; i < MAX_BRCMSTB_RANGE; ++i) {
		if (_bhpa_region_info(i, &addr, &size))
			break;  /* no more regions */
		mem->bhpa.range[i].addr = addr;
		mem->bhpa.range[i].size = size;
		++mem->bhpa.count;
	}
	if (i >= MAX_BRCMSTB_RANGE) {
		while (_bhpa_region_info(i, &addr, &size) == 0) {
			pr_warn("%s: Exceeded max ranges\n", __func__);
			++mem->bhpa.count;
		}
	}

	return 0;
#else
	return -ENOSYS;
#endif
}

#if IS_BUILTIN(CONFIG_BRCMSTB_MEMORY_API)
#if IS_ENABLED(CONFIG_BRCMSTB_HUGEPAGES)
static bool bhpa_disabled;

static int __init __bhpa_setup(phys_addr_t addr, phys_addr_t size)
{
	phys_addr_t end = addr + size;
	int i;

	/* Consolidate overlapping regions */
	for (i = 0; i < n_bhpa_regions; i++) {
		if (addr > bhpa_regions[i].addr + bhpa_regions[i].size)
			continue;
		if (end < bhpa_regions[i].addr)
			continue;
		end = max(end, bhpa_regions[i].addr + bhpa_regions[i].size);
		addr = min(addr, bhpa_regions[i].addr);
		bhpa_regions[i].addr = bhpa_regions[--n_bhpa_regions].addr;
		bhpa_regions[i--].size = bhpa_regions[n_bhpa_regions].size;
	}

	if (n_bhpa_regions == MAX_BHPA_REGIONS) {
		pr_warn_once("too many regions, ignoring extras\n");
		return -E2BIG;
	}

	bhpa_regions[n_bhpa_regions].addr = addr;
	bhpa_regions[n_bhpa_regions].size = end - addr;
	n_bhpa_regions++;

	return 0;
}

/*
 * Parses command line for bhpa= options
 */
static int __init bhpa_setup(char *str)
{
	phys_addr_t addr, end = 0, size;
	char *orig_str = str;
	int ret;

	addr = memparse(str, &str);
	if (*str == '@') {
		size = addr;
		addr = memparse(str + 1, &str);
		end = addr + size;
	} else if (*str == '-') {
		end = memparse(str + 1, &str);
	}

	addr = ALIGN(addr, BHPA_ALIGN);
	end = ALIGN_DOWN(end, BHPA_ALIGN);
	size = end - addr;

	if (size == 0) {
		pr_info("disabling reserved memory\n");
		bhpa_disabled = true;
		return 0;
	}

	if (addr < memblock_start_of_DRAM()) {
		pr_warn("ignoring invalid range '%s' below addressable DRAM\n",
			orig_str);
		return 0;
	}

	if (addr > end || size < pageblock_nr_pages << PAGE_SHIFT) {
		pr_warn("ignoring invalid range '%s' (too small)\n",
				orig_str);
		return 0;
	}

	ret = __bhpa_setup(addr, size);
	if (!ret)
		brcmstb_memory_override_defaults = true;
	return ret;
}
early_param("bhpa", bhpa_setup);

static __init void split_bhpa_region(phys_addr_t addr, struct bhpa_region *p)
{
	struct bhpa_region *tmp;
	phys_addr_t end;

	if (p->addr + p->size > addr + BHPA_SIZE) {
		if (n_bhpa_regions < MAX_BHPA_REGIONS) {
			tmp = &bhpa_regions[n_bhpa_regions++];
			while (tmp > p) {
				*tmp = *(tmp - 1);
				tmp--;
			}
			(++tmp)->addr = addr;
			tmp->size -= addr - p->addr;
			end = addr + tmp->size;
			B_LOG_DBG("region split: %pa-%pa", &addr, &end);
		} else {
			B_LOG_WRN("bhpa region truncated (MAX_BHPA_REGIONS)");
		}
	}
	p->size = addr - p->addr;
	end = p->addr + p->size;
	B_LOG_DBG("region added: %pa-%pa", &p->addr, &end);
}

static __init void intersect_bhpa_ranges(phys_addr_t start, phys_addr_t size,
					 struct bhpa_region **ptr)
{
	struct bhpa_region *tmp, *p = *ptr;
	phys_addr_t end = start + size;

	B_LOG_DBG("range: %pa-%pa", &start, &end);
	while (p < &bhpa_regions[n_bhpa_regions] &&
	       p->addr + p->size <= start) {
		tmp = p;
		end = p->addr + p->size;
		B_LOG_WRN("unmapped bhpa region %pa-%pa",
			   &p->addr, &end);

		n_bhpa_regions--;
		while (tmp < &bhpa_regions[n_bhpa_regions]) {
			*tmp = *(tmp + 1);
			tmp++;
		}
	}

	end = start + size;
	while (p < &bhpa_regions[n_bhpa_regions] && p->addr < end) {
		phys_addr_t last;

		start = max(start, p->addr);
		start = ALIGN(start, BHPA_ALIGN);
		last = min(end, p->addr + p->size);
		last = ALIGN_DOWN(last, BHPA_ALIGN);

		if (start + BHPA_ALIGN >= last) {
			*ptr = p;
			return;
		}

		B_LOG_DBG("intersection: %pa-%pa", &start, &last);
		p->size -= start - p->addr;
		p->addr = start;

		split_bhpa_region(last, p);
		p++;
	}

	*ptr = p;
}

static __init int memc_map(int memc, u64 addr, u64 size, void *context)
{
	struct bhpa_region **ptr = context, *p;
	phys_addr_t start = (phys_addr_t)addr;

	if (start != addr) {
		pr_err("phys_addr_t smaller than provided address 0x%llx!\n",
			addr);
		return -EINVAL;
	}

	if (memc == -1) {
		pr_err("address 0x%llx does not appear to be in any memc\n",
			addr);
		return -EINVAL;
	}

	p = *ptr;
	intersect_bhpa_ranges(start, (phys_addr_t)size, ptr);

	while (p != *ptr) {
		p->memc = memc;
		p++;
	}

	return 0;
}

static void __init bhpa_alloc_ranges(void)
{
	struct bhpa_region *p = bhpa_regions;
	phys_addr_t end;

	while (p < &bhpa_regions[n_bhpa_regions]) {
		end = p->addr + p->size;
		/*
		 * This is based on memblock_alloc_range_nid(), but excludes
		 * the search for efficiency.
		 * The min_count is set to 0 so that memblock
		 * allocations are never reported as leaks.
		 */
		memblock_reserve(p->addr, p->size);
		kmemleak_alloc_phys(p->addr, p->size, 0, 0);
		kmemleak_ignore_phys(p->addr);
		if (!dmb_reserve(p->addr, p->size, NULL)) {
			B_LOG_MSG("Alloc: MEMC%d: %pa-%pa", p->memc,
				&p->addr, &end);
			p->mt = MIGRATE_MOVABLE;
			p++;
		} else {
			B_LOG_WRN("bhpa reservation %pa-%pa failed!",
				&p->addr, &end);
			while (++p < &bhpa_regions[n_bhpa_regions])
				*(p - 1) = *p;
			n_bhpa_regions--;
		}
	}
}

static void __init brcmstb_bhpa_reserve(void)
{
	phys_addr_t addr, size, start, end;
	struct bhpa_region *p, *tmp;
	u64 loop;
	int i;

	if (bhpa_disabled) {
		n_bhpa_regions = 0;
		return;
	}

	if (brcmstb_default_reserve == BRCMSTB_RESERVE_BHPA &&
			!n_bhpa_regions &&
			!brcmstb_memory_override_defaults)
		brcmstb_memory_default_reserve(__bhpa_setup);

	if (!n_bhpa_regions)
		return;

	for (i = 0; i < n_bhpa_regions; i++) {
		bhpa_regions[i].memc = -1;
		if (!i)
			continue;

		/* Sort regions */
		p = &bhpa_regions[i];
		addr = p->addr;
		size = p->size;
		while (p != bhpa_regions && p->addr < (p - 1)->addr) {
			p->addr = (p - 1)->addr;
			p->size = (p - 1)->size;
			p--;
		}
		p->addr = addr;
		p->size = size;
	}
	for (i = 0; i < n_bhpa_regions; i++) {
		p = &bhpa_regions[i];
		end = p->addr + p->size;
		B_LOG_DBG("region: %pa-%pa", &p->addr, &end);
	}

	p = bhpa_regions;
	early_for_each_memc_range(memc_map, &p);
	while (p < &bhpa_regions[n_bhpa_regions]) {
		tmp = &bhpa_regions[--n_bhpa_regions];
		end = tmp->addr + tmp->size;
		B_LOG_WRN("Drop region: %pa-%pa", &tmp->addr, &end);
	}

	if (!n_bhpa_regions)
		return;

	p = bhpa_regions;
	for_each_free_mem_range(loop, NUMA_NO_NODE, MEMBLOCK_NONE, &start,
				&end, NULL) {
		intersect_bhpa_ranges(start, end - start, &p);

		if (p >= &bhpa_regions[n_bhpa_regions])
			break;
	}
	while (p < &bhpa_regions[n_bhpa_regions]) {
		tmp = &bhpa_regions[--n_bhpa_regions];
		end = tmp->addr + tmp->size;
		B_LOG_WRN("Drop region: %pa-%pa", &tmp->addr, &end);
	}

	bhpa_alloc_ranges();
}

void __init brcmstb_bhpa_setup(phys_addr_t addr, phys_addr_t size)
{
	__bhpa_setup(addr, size);
}
#endif

static int __init brcmstb_memory_set_range(phys_addr_t start, phys_addr_t end,
					   int (*setup)(phys_addr_t start,
							phys_addr_t size));

static int __init brcmstb_memory_region_check(phys_addr_t *start,
					      phys_addr_t *size,
					      phys_addr_t *end,
					      phys_addr_t reg_start,
					      phys_addr_t reg_size,
					      int (*setup)(phys_addr_t start,
						           phys_addr_t size))
{
	/* range is entirely below the reserved region */
	if (*end <= reg_start)
		return 1;

	/* range is entirely above the reserved region */
	if (*start >= reg_start + reg_size)
		return 0;

	if (*start < reg_start) {
		if (*end <= reg_start + reg_size) {
			/* end of range overlaps reservation */
			pr_debug("%s: Reduced default region %pa@%pa\n",
					__func__, size, start);

			*end = reg_start;
			*size = *end - *start;
			pr_debug("%s: to %pa@%pa\n",
					__func__, size, start);
			return 1;
		}

		/* range contains the reserved region */
		pr_debug("%s: Split default region %pa@%pa\n",
			 __func__, size, start);

		*size = reg_start - *start;
		pr_debug("%s: into %pa@%pa\n",
			 __func__, size, start);
		brcmstb_memory_set_range(*start, reg_start, setup);

		*start = reg_start + reg_size;
		*size = *end - *start;
		pr_debug("%s: and %pa@%pa\n", __func__, size, start);
	} else if (*end > reg_start + reg_size) {
		/* start of range overlaps reservation */
		pr_debug("%s: Reduced default region %pa@%pa\n",
			 __func__, size, start);
		*start = reg_start + reg_size;
		*size = *end - *start;
		pr_debug("%s: to %pa@%pa\n", __func__, &size, &start);
	} else {
		/* range is contained by the reserved region */
		pr_debug("%s: Default region %pa@%pa is reserved\n",
			 __func__, size, start);

		return -EINVAL;
	}

	return 0;
}

/*
 * brcmstb_memory_set_range() - validate and set middleware memory range
 * @start: the physical address of the start of a candidate range
 * @end: the physical address one beyond the end of a candidate range
 * @setup: function for setting the start and size of a region
 *
 * This function adjusts a candidate default memory range to accommodate
 * memory reservations and alignment constraints.  If a valid range can
 * be determined, then the setup function is called to actually record
 * the region.
 *
 * This function assumes the memblock.reserved type is incrementally
 * ordered and non-overlapping.  If that changes then this function must
 * be updated.
 */
static int __init brcmstb_memory_set_range(phys_addr_t start, phys_addr_t end,
					   int (*setup)(phys_addr_t start,
							phys_addr_t size))
{
	/* min alignment for mm core */
	const phys_addr_t alignment =
		PAGE_SIZE << max(MAX_ORDER - 1, pageblock_order);
	phys_addr_t temp, size = end - start;
	int i, ret;

	for (i = 0; i < memblock.reserved.cnt; i++) {
		struct memblock_region *region = &memblock.reserved.regions[i];

		ret = brcmstb_memory_region_check(&start, &size, &end,
						  region->base, region->size,
						  setup);
		if (ret == 0)
			continue;

		if (ret == 1)
			break;

		if (ret < 0)
			return ret;

	}

	/* Also range check reserved-memory 'no-map' entries from being
	 * possible candidates
	 */
	for (i = 0; i < __reserved_mem_get_count(); i++) {
		struct reserved_mem *rmem = __reserved_mem_get_entry(i);

		if (memblock_is_map_memory(rmem->base))
			continue;

		ret = brcmstb_memory_region_check(&start, &size, &end,
						  rmem->base, rmem->size,
						  setup);
		if (ret == 0)
			continue;

		if (ret == 1)
			break;

		if (ret < 0)
			return ret;

	}

	/* Exclude reserved-memory 'no-map' entries from being possible
	 * candidates
	 */
	if (!memblock_is_map_memory(start)) {
		pr_debug("%s: Cannot add nomap %pa%p@\n",
			 __func__, &start, &end);
		return -EINVAL;
	}

	/* Fix up alignment */
	temp = ALIGN(start, alignment);
	if (temp != start) {
		pr_debug("adjusting start from %pa to %pa\n",
			 &start, &temp);

		if (size > (temp - start))
			size -= (temp - start);
		else
			size = 0;

		start = temp;
	}

	temp = round_down(size, alignment);
	if (temp != size) {
		pr_debug("adjusting size from %pa to %pa\n",
			 &size, &temp);
		size = temp;
	}

	if (size == 0) {
		pr_debug("size available in bank was 0 - skipping\n");
		return -EINVAL;
	}

	return setup(start, size);
}

/*
 * brcmstb_memory_default_reserve() - create default reservations
 * @setup: function for setting the start and size of a region
 *
 * This determines the size and address of the default regions
 * reserved for refsw based on the flattened device tree.
 */
struct default_res
{
	int count;
	int prev_memc;
	int (*setup)(phys_addr_t start,	phys_addr_t size);
	u64 total_size;
	unsigned int num_memcs;
};

static __init int memc_reserve_bmem(int memc, u64 addr, u64 size, void *context)
{
	struct default_res *ctx = context;
	u64 adj = 0, end = addr + size, limit, tmp;
	phys_addr_t p_start, p_end;

	if ((phys_addr_t)addr != addr) {
		pr_err("phys_addr_t too small for address 0x%llx!\n",
			addr);
		return 0;
	}

	if ((phys_addr_t)size != size) {
		pr_err("phys_addr_t too small for size 0x%llx!\n", size);
		return 0;
	}

#if defined(CONFIG_KASAN) && defined(KASAN_SHADOW_SCALE_SHIFT)
	/* KASAN requires a fraction of the memory */
	adj += size >> KASAN_SHADOW_SCALE_SHIFT;
#endif

	if (!ctx->count++) {	/* First Bank */
		limit = (u64)memblock_get_current_limit();

#ifdef CONFIG_BLK_DEV_INITRD
		/* Assume that a compressed initramfs will expand roughly to 5
		 * times its compressed size. Determining its exact size early
		 * on boot with no memory allocator available is not possible
		 * since decompressors assume kmalloc() is available.
		 */
		adj += __initramfs_size * 5;
#endif

		/*
		 *  On ARM64 systems, force the first memory controller
		 * to be partitioned the same way it would on ARM
		 * (32-bit) by giving 256MB to the kernel, the rest to
		 * BMEM. If we have 4GB or more available on this MEMC,
		 * give 512MB to the kernel.
		 */
#ifdef CONFIG_ARM64
		if (size >= VME_A32_MAX)
			adj += SZ_512M;
		else
			adj += SZ_256M;
#else
		if (brcmstb_default_reserve == BRCMSTB_RESERVE_BMEM ||
		    brcmstb_default_reserve == BRCMSTB_RESERVE_BHPA)
			adj += SZ_128M;
#endif

		/* Compensate for any reserved memory */
		for_each_reserved_mem_range(tmp, &p_start, &p_end) {
			if (p_end <= addr)
				continue;
			if (p_start >= addr + adj)
				break;

			adj += p_end - max(addr, (u64)p_start);
		}

#ifdef CONFIG_ARM64
		if (limit > addr + adj)
			limit = addr + adj;
#endif

		if (end <= limit && end == (u64)memblock_end_of_DRAM()) {
			if (size < adj + SZ_32M) {
				pr_err("low memory too small for default bmem\n");
				return 0;
			}

			/* kernel reserves X percent,
			 * bmem gets the rest */
			tmp = (size - adj) * (100 - DEFAULT_LOWMEM_PCT);
			do_div(tmp, 100);
			size = tmp;
			addr = end - size;
		} else if (end > limit) {
			addr = limit;
			size = end - addr;
		} else {
			if (size >= SZ_1G)
				addr += SZ_512M;
			else if (size >= SZ_512M)
				addr += SZ_256M;
			else
				return 0;
			size = end - addr;
		}
	} else if (memc > ctx->prev_memc) {
#ifdef CONFIG_ARM64
		if (addr >= VME_A32_MAX && size >= BMEM_LARGE_ENOUGH) {
			/* Give 256M back to Linux */
			adj += SZ_256M;
		}
#endif
		/* Use the rest of the first region of this MEMC */
		ctx->prev_memc = memc;
		addr += adj;
		size = end - addr;
	} else if (addr >= VME_A32_MAX && size > adj + SZ_64M) {
		/*
		 * Nexus doesn't use the address extension range yet,
		 * just reserve 64 MiB in these areas until we have a
		 * firmer specification
		 */
		addr += adj;
		size = SZ_64M;
	} else {
		addr += adj;
		size = end - addr;
	}

	brcmstb_memory_set_range((phys_addr_t)addr, (phys_addr_t)(addr + size),
				 ctx->setup);

	return 0;
}

static int __init memc_count_and_size(int memc, u64 addr, u64 size,
				      void *context)
{
	struct default_res *ctx = context;

	ctx->total_size += size;
	ctx->num_memcs++;

	return 0;
}

static __init int memc_reserve_bhpa(int memc, u64 addr, u64 size, void *context)
{
	struct default_res *ctx = context;
	u32 ratio = 2;
	u64 adj;

	if ((phys_addr_t)addr != addr) {
		pr_err("phys_addr_t too small for address 0x%llx!\n",
		       addr);
		return 0;
	}

	if ((phys_addr_t)size != size) {
		pr_err("phys_addr_t too small for size 0x%llx!\n", size);
		return 0;
	}

	/* First memory controller */
	if (!ctx->count++) {
		adj = ctx->total_size;
		do_div(adj, ratio);

#if defined(CONFIG_KASAN) && defined(KASAN_SHADOW_SCALE_SHIFT)
		/* KASAN requires a fraction of the memory */
		adj += ctx->total_size >> KASAN_SHADOW_SCALE_SHIFT;
#endif
#ifdef CONFIG_BLK_DEV_INITRD
		/* Assume that a compressed initramfs will expand roughly to 5
		 * times its compressed size. Determining its exact size early
		 * on boot with no memory allocator available is not possible
		 * since decompressors assume kmalloc() is available.
		 */
		adj += __initramfs_size * 5;
#endif
		if (adj >= size)
			return 0;

		addr += adj;
		size  -= adj;
	}

	brcmstb_memory_set_range((phys_addr_t)addr, (phys_addr_t)(addr + size),
				 ctx->setup);

	return 0;
}

struct periphbase_cfg {
	u32 midr;
	u8 lshift;
	u8 ushift;
};

static const __initdata struct periphbase_cfg cfgs[] = {
#ifdef CONFIG_ARM
	{ ARM_CPU_PART_BRAHMA_B15, 15, 39 },
	{ ARM_CPU_PART_BRAHMA_B53, 18, 39 },
	{ ARM_CPU_PART_CORTEX_A72, 18, 43 },
#else
	{ MIDR_BRAHMA_B53, 18, 39 },
	{ MIDR_CORTEX_A72, 18, 43 },
#endif
	{ /* sentinel */ },
};

static bool __init soc_uses_v7_mmap(void)
{
	const struct periphbase_cfg *cfg = cfgs;
#ifdef CONFIG_ARM
	u32 midr = read_cpuid_part();
	u32 umask, tmp;
#else
	u32 midr = read_cpuid_id() & MIDR_CPU_MODEL_MASK;
#endif
	bool found = false;
	u64 base;

	while (cfg && cfg->midr) {
		if (midr == cfg->midr) {
			found = true;
			break;
		}
		cfg++;
	}

	if (!found)
		return found;

#ifdef CONFIG_ARM
	if (midr == ARM_CPU_PART_BRAHMA_B15)
		asm volatile("mrc p15, 4, %0, c15, c0, 0" : "=r" (tmp));
	else
		asm volatile("mrc p15, 1, %0, c15, c3, 0" : "=r" (tmp));
	umask = GENMASK(cfg->ushift - 32, 0);
	base = (u64)(tmp & umask) << 32 | (tmp & GENMASK(31, cfg->lshift));
#else
	asm volatile("mrs %0, S3_1_C15_C3_0" : "=r"(base));
	base &= GENMASK(cfg->ushift, cfg->lshift);
#endif
	return base == SZ_2M;
}

static inline bool brcmstb_uses_4kb_mpu_granule(void)
{
	const void *fdt = initial_boot_params;
	const struct fdt_property *prop;
	bool ret = false;
	int offset;

	offset = fdt_path_offset(fdt, "/bolt");
	if (offset < 0)
		return ret;

	prop = fdt_get_property(fdt, offset, "mpu-granularity", NULL);
	if (!prop)
		goto out;

	ret = DT_PROP_DATA_TO_U32(prop->data, 0) == 4096;
out:
	return ret;
}

void __init brcmstb_memory_default_reserve(int (*setup)(phys_addr_t start,
							phys_addr_t size))
{
	struct default_res ctx;

	ctx.count = 0;
	ctx.prev_memc = 0;
	ctx.setup = setup;
	ctx.total_size = 0;
	ctx.num_memcs = 0;

	early_for_each_memc_range(memc_count_and_size, &ctx);
	if (!ctx.total_size || !ctx.num_memcs) {
		pr_err("Unable to determine total size or number of banks\n");
		return;
	}

	if (brcmstb_uses_4kb_mpu_granule())
		return;

	if (brcmstb_default_reserve == BRCMSTB_RESERVE_BHPA &&
	    soc_uses_v7_mmap() &&
	    ctx.total_size > SZ_2G && ctx.num_memcs == 1) {
		early_for_each_memc_range(memc_reserve_bhpa, &ctx);
	} else {
		early_for_each_memc_range(memc_reserve_bmem, &ctx);
	}
}

static void __init add_rsvd(phys_addr_t start, phys_addr_t end)
{
	if (start >= end)
		return;

	if (reserved_init.count >= MAX_BRCMSTB_RESERVED_RANGE) {
		pr_warn_once("%s: Exceeded max ranges\n", __func__);
		return;
	}

	reserved_init.range[reserved_init.count].addr = start;
	reserved_init.range[reserved_init.count++].size = end - start;
}

/**
 * brcmstb_memory_reserve() - fill in static brcmstb_memory structure
 *
 * This is a boot-time initialization function used to copy the information
 * stored in the memblock reserve function that is discarded after boot.
 */
void __init brcmstb_memory_reserve(void)
{
	struct memblock_type *mem = &memblock.memory;
	struct memblock_region *region = &mem->regions[0];
	phys_addr_t start, end;
	int midx = 0;
	u64 i;

	for_each_reserved_mem_range(i, &start, &end) {
		while (start < end) {
			/* remove movable blocks from reserved ranges */
			while (midx < mem->cnt &&
				(!memblock_is_movable(region) ||
				 region->base + region->size <= start)) {
				region = &mem->regions[++midx];
			}
			if (midx < mem->cnt) {
				if (region->base < end) {
					add_rsvd(start, region->base);
					start = region->base + region->size;
					continue;
				}
			}
			add_rsvd(start, end);
			break;
		}
	}

	brcmstb_populate_reserved();
	brcmstb_populate_reserved_called = true;
}

/*
 * brcmstb_memory_init() - Initialize Broadcom proprietary memory extensions
 *
 * This function is a hook from the architecture specific mm initialization
 * that allows the memory extensions used by Broadcom Set-Top-Box middleware
 * to be initialized.
 */
void __init brcmstb_memory_init(void)
{
#ifdef CONFIG_ARM
	/* Default B15-based CPUs to use BMEM to avoid speculative fetches
	 * in reserved memory regions.
	 */
	if (read_cpuid_part() == ARM_CPU_PART_BRAHMA_B15)
		brcmstb_default_reserve = BRCMSTB_RESERVE_BMEM;
#endif
	/* Disable SWIOTLB for all v7 memory map SoCs since all of their
	 * peripherals are 64-bit capable. We let architectures determine
	 * whether it is necessary to use a SWIOTLB or not.
	 */
	if (soc_uses_v7_mmap())
		swiotlb_force = SWIOTLB_NO_FORCE;
	brcmstb_memory_reserve();
#ifdef CONFIG_BRCMSTB_BMEM
#if IS_ENABLED(CONFIG_BRCMSTB_HUGEPAGES)
	if (brcmstb_bmem_is_bhpa)
		bmem_reserve(brcmstb_bhpa_setup);
	else
#endif
		bmem_reserve(NULL);
#endif
#ifdef CONFIG_BRCMSTB_CMA
	cma_reserve();
#endif
#if IS_ENABLED(CONFIG_BRCMSTB_HUGEPAGES)
	brcmstb_bhpa_reserve();
#endif
#ifdef CONFIG_MEMORY_HOTPLUG
	set_online_page_callback(brcm_online_callback);
#endif
}
#endif /* CONFIG_BRCMSTB_MEMORY_API */

/*
 * brcmstb_memory_get() - fill in brcmstb_memory structure
 * @mem: pointer to allocated struct brcmstb_memory to fill
 *
 * The brcmstb_memory struct is required by the brcmstb middleware to
 * determine how to set up its memory heaps.  This function expects that the
 * passed pointer is valid.  The struct does not need to have be zeroed
 * before calling.
 */
int brcmstb_memory_get(struct brcmstb_memory *mem)
{
	int ret;

	if (!mem)
		return -EFAULT;

	memset(mem, 0, sizeof(*mem));

	ret = populate_memc(mem);
	if (ret)
		return ret;

	ret = populate_lowmem(mem);
	if (ret)
		pr_debug("no lowmem defined\n");

	ret = populate_bmem(mem);
	if (ret)
		pr_debug("bmem is disabled\n");

	ret = populate_cma(mem);
	if (ret)
		pr_debug("cma is disabled\n");

	ret = populate_reserved(mem);
	if (ret)
		return ret;

	ret = populate_bhpa(mem);
	if (ret)
		pr_debug("bhpa is disabled\n");

	return 0;
}
EXPORT_SYMBOL(brcmstb_memory_get);

#ifdef CONFIG_PAGE_AUTOMAP
static void map(phys_addr_t start, phys_addr_t size)
{
	unsigned long va_start = (unsigned long)__va(start);
	unsigned long va_end = va_start + size;
	struct page *page = phys_to_page(start);

	pr_debug("AutoMap kernel pages 0x%llx size = 0x%llx\n", start, size);

	while (va_start != va_end) {
		map_kernel_range_noflush(va_start, PAGE_SIZE,
					 PAGE_KERNEL, &page);
		va_start += PAGE_SIZE;
		page++;
	}

	flush_tlb_kernel_range(va_start, va_start + (unsigned long)size);
}

static void unmap(phys_addr_t start, phys_addr_t size)
{
	unsigned long va_start = (unsigned long)__va(start);

	pr_debug("AutoUnmap kernel pages 0x%llx size = 0x%llx\n", start, size);
	unmap_kernel_range_noflush(va_start, (unsigned long)size);
	flush_tlb_kernel_range(va_start, va_start + (unsigned long)size);
}

void put_automap_page(struct page *page)
{
	int count;

	spin_lock(&automap_lock);
	count = page_ref_dec_return(page);
	WARN_ON(!count);
	if (count == 1)
		unmap(page_to_phys(page), PAGE_SIZE);
	spin_unlock(&automap_lock);
}
EXPORT_SYMBOL(put_automap_page);

static void inc_automap_pages(struct page *page, int nr)
{
	phys_addr_t end, start = 0;
	int count;

	spin_lock(&automap_lock);
	while (nr--) {
		if (unlikely(PageAutoMap(page))) {
			count = page_ref_inc_return(page);
			if (count == 2) {
				/* Needs to be mapped */
				if (!start)
					start = page_to_phys(page);
				end = page_to_phys(page);
			} else if (start) {
				map(start, end + PAGE_SIZE - start);
				start = 0;
			}
		} else if (start) {
			map(start, end + PAGE_SIZE - start);
			start = 0;
		}
		page++;
	}
	if (start)
		map(start, end + PAGE_SIZE - start);
	spin_unlock(&automap_lock);
}

static void dec_automap_pages(struct page *page, int nr)
{
	phys_addr_t end, start = 0;
	int count;

	spin_lock(&automap_lock);
	while (nr--) {
		if (unlikely(PageAutoMap(page))) {
			count = page_ref_dec_return(page);
			if (count == 1) {
				/* Needs to be unmapped */
				if (!start)
					start = page_to_phys(page);
				end = page_to_phys(page);
			} else if (start) {
				unmap(start, end + PAGE_SIZE - start);
				start = 0;
			}
		} else if (start) {
			unmap(start, end + PAGE_SIZE - start);
			start = 0;
		}
		page++;
	}
	if (start)
		unmap(start, end + PAGE_SIZE - start);
	spin_unlock(&automap_lock);
}

int track_pfn_remap(struct vm_area_struct *vma, pgprot_t *prot,
		    unsigned long pfn, unsigned long addr,
		    unsigned long size)
{
	if (pfn_valid(pfn))
		inc_automap_pages(pfn_to_page(pfn), size >> PAGE_SHIFT);
	return 0;
}

int track_pfn_insert(struct vm_area_struct *vma, pgprot_t *prot, pfn_t pfn)
{
	struct page *page = pfn_t_to_page(pfn);

	if (unlikely(!page))
		return 0;

	inc_automap_pages(page, 1);
	return 0;
}

int track_pfn_copy(struct vm_area_struct *vma)
{
	unsigned long pfn;

	if (follow_pfn(vma, vma->vm_start, &pfn)) {
		WARN_ON_ONCE(1);
		return 0;
	}

	if (pfn_valid(pfn))
		inc_automap_pages(pfn_to_page(pfn),
				  (vma->vm_end - vma->vm_start) >> PAGE_SHIFT);
	return 0;
}

void untrack_pfn(struct vm_area_struct *vma, unsigned long pfn,
		 unsigned long size)
{
	if (!pfn && !size) {
		if (!vma) {
			WARN_ON_ONCE(1);
			return;
		}

		if (follow_pfn(vma, vma->vm_start, &pfn))
			WARN_ON_ONCE(1);

		size = vma->vm_end - vma->vm_start;
	}

	if (pfn_valid(pfn))
		dec_automap_pages(pfn_to_page(pfn), size >> PAGE_SHIFT);
}

void untrack_pfn_moved(struct vm_area_struct *vma, unsigned long pfn,
		       unsigned long size)
{
	if (!pfn && !size) {
		if (!vma) {
			WARN_ON_ONCE(1);
			return;
		}

		if (follow_pfn(vma, vma->vm_start, &pfn))
			WARN_ON_ONCE(1);

		size = vma->vm_end - vma->vm_start;
	}

	if (pfn_valid(pfn))
		inc_automap_pages(pfn_to_page(pfn), size >> PAGE_SHIFT);
}
#endif /* CONFIG_PAGE_AUTOMAP */

/**
 * brcmstb_memory_kva_map_phys() - map phys range to kernel virtual address
 *
 * @phys: physical address base
 * @size: size of range to map
 * @cached: whether to use cached or uncached mapping
 *
 * Return: NULL on failure, addr on success
 */
void *brcmstb_memory_kva_map_phys(phys_addr_t phys, size_t size, bool cached)
{
	void *addr;

	addr = memremap(phys, size, cached ? MEMREMAP_WB : MEMREMAP_WC);
	if (!addr)
		vm_unmap_aliases();

	return addr;
}
EXPORT_SYMBOL(brcmstb_memory_kva_map_phys);

/**
 * brcmstb_memory_kva_map() - Map page(s) to a kernel virtual address
 *
 * @page: A struct page * that points to the beginning of a chunk of physical
 * contiguous memory.
 * @num_pages: Number of pages
 * @pgprot: Page protection bits
 *
 * Return: pointer to mapping, or NULL on failure
 */
void *brcmstb_memory_kva_map(struct page *page, int num_pges, pgprot_t pgprot)
{
	/* Only supports PAGE_KERNEL protection */
	if (pgprot_val(pgprot) != pgprot_val(PAGE_KERNEL) || page == NULL)
		return NULL;

	return brcmstb_memory_kva_map_phys(__pfn_to_phys(page_to_pfn(page)),
					   (size_t)(num_pges * PAGE_SIZE),
					   true);
}
EXPORT_SYMBOL(brcmstb_memory_kva_map);

/**
 * brcmstb_memory_kva_unmap() - Unmap a kernel virtual address associated
 * to physical pages mapped by brcmstb_memory_kva_map()
 *
 * @kva: Kernel virtual address previously mapped by brcmstb_memory_kva_map*
 *
 * Return: 0 on success, negative on failure.
 */
int brcmstb_memory_kva_unmap(const void *kva)
{
	bool vm = is_vmalloc_addr(kva);

	memunmap((void *)kva);
	if (vm)
		vm_unmap_aliases();

	return 0;
}
EXPORT_SYMBOL(brcmstb_memory_kva_unmap);

#if IS_BUILTIN(CONFIG_BRCMSTB_MEMORY_API)
void *brcmstb_ioremap(phys_addr_t phys_addr, size_t size)
{
	unsigned long last_addr;
	unsigned long offset = phys_addr & ~PAGE_MASK;
	int err;
	unsigned long addr;
	struct vm_struct *area;

	/*
	 * Page align the mapping address and size, taking account of any
	 * offset.
	 */
	phys_addr &= PAGE_MASK;
	size = PAGE_ALIGN(size + offset);

	/*
	 * Don't allow wraparound, zero size or outside PHYS_MASK.
	 */
	last_addr = phys_addr + size - 1;
	if (!size || last_addr < phys_addr || (last_addr & ~PHYS_MASK))
		return NULL;

	area = get_vm_area_caller(size, VM_IOREMAP,
				  __builtin_return_address(0));
	if (!area)
		return NULL;
	addr = (unsigned long)area->addr;
	area->phys_addr = phys_addr;

	err = ioremap_page_range(addr, addr + size, phys_addr,
				 FIXMAP_PAGE_IO);
	if (err) {
		vunmap((void *)addr);
		return NULL;
	}

	return (void __iomem *)(offset + addr);
}
EXPORT_SYMBOL(brcmstb_ioremap);
#else
static int __init brcmstb_memory_module_init(void)
{
#ifdef CONFIG_MEMORY_HOTPLUG
	set_online_page_callback(brcm_online_callback);
#endif
	return 0;
}
module_init(brcmstb_memory_module_init);
#endif /* IS_BUILTIN(CONFIG_BRCMSTB_MEMORY_API) */

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom STB memory services");
MODULE_LICENSE("GPL v2");
