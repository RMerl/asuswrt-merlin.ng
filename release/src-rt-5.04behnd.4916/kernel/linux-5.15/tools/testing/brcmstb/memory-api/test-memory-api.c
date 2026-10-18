#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/brcmstb/memory_api.h>
#include <linux/soc/brcmstb/brcmstb.h>
#include "linux/delay.h"

void brcmstb_hugepage_print(struct seq_file *seq);

#define B_LOG_(level, fmt, args...) printk( level "BRCMSTB_HUGEPAGES: " fmt "\n", ## args)
#define B_LOG_WRN(args...) B_LOG_(KERN_WARNING, args)
#define B_LOG_MSG(args...) B_LOG_(KERN_INFO, args)
#define B_LOG_TRACE(args...) /* B_LOG_(KERN_INFO, args) */


static struct brcmstb_memory bm;

#if IS_ENABLED(CONFIG_BRCMSTB_MEMORY_API)
static void test_kva_mem_map(struct brcmstb_range *range)
{
	size_t size = range->size / 2;
	void *addr;
	unsigned long *data, check;

	addr = brcmstb_memory_kva_map_phys(range->addr, size, true);
	if (!addr) {
		pr_err("failed to map cached %llu MiB at %#016llx\n",
		       (unsigned long long)size / SZ_1M, range->addr);
		return;
	} else {

		pr_info("%s: cached virt: %px, phys: 0x%llx\n",
                      __func__, addr, (unsigned long long)virt_to_phys(addr));

		/* Now try to read from there as well */
		data = addr;
		*data = 0xdeadbeefUL;
		check = *data;
		if (check != 0xdeadbeefUL)
			pr_err("memory mismatch: %llu != %llu\n",
			       (unsigned long long)check, 0xdeadbeefULL);

		brcmstb_memory_kva_unmap(addr);
	}

	addr = brcmstb_memory_kva_map_phys(range->addr, size, false);
	if (!addr) {
		pr_err("failed to map uncached %llu MiB at %#016llx\n",
		       (unsigned long long)size / SZ_1M, range->addr);
	} else {

		pr_info("%s: uncached virt: %px, phys: 0x%llx\n",
                      __func__, addr, (unsigned long long)virt_to_phys(addr));

		/* Now try to read from there as well */
		data = addr;
		*data = 0xdeadbeefUL;
		check = *data;
		if (check != 0xdeadbeefUL)
			pr_err("memory mismatch: %llu != %llu\n",
			       (unsigned long long)check, 0xdeadbeefULL);

		brcmstb_memory_kva_unmap(addr);
	}
}
#endif

#if IS_ENABLED(CONFIG_BRCMSTB_HUGEPAGES)
static void bhpa_test(void)
{
	int rc;
	static struct allocation {
		unsigned int memc;
		unsigned int count;
		uint64_t pages[64];
	} allocations[16];
	struct allocation *ptr;
	unsigned memcIndex;
	unsigned loop;

	brcmstb_hugepage_print(NULL);

	for (loop = 0; loop < 2; loop++) {
		unsigned int i, j;

		memcIndex = 0;
		B_LOG_MSG("[%u] alloc memory", loop);
		for (i = 0; i < ARRAY_SIZE(allocations); i++) {
			ptr = &allocations[i];
			rc = brcmstb_hugepage_alloc(memcIndex,
						    ptr->pages,
						    ARRAY_SIZE(ptr->pages),
						    &ptr->count,
						    NULL);
			ptr->memc = memcIndex;
			B_LOG_MSG("Alloc memory  -> %d %u %d", memcIndex,
				  ptr->count, rc);
			if(!ptr->count) {
				if (++memcIndex >= MAX_BRCMSTB_MEMC)
					break;
				i--;
			} else {
				brcmstb_hugepage_print(NULL);
			}
		}
		msleep(msecs_to_jiffies(5*1000));

		B_LOG_MSG("[%u] free memory", loop);
		for (j = 0; j < i; j++) {
			ptr = &allocations[j];
			B_LOG_MSG("free memory %u pages", ptr->count);
			brcmstb_hugepage_free(ptr->memc, ptr->pages,
					      ptr->count);
			brcmstb_hugepage_print(NULL);
		}
	}

	return;
}
#else
static inline void bhpa_test(void)
{
}
#endif

static int __init test_init(void)
{
#if IS_ENABLED(CONFIG_BRCMSTB_MEMORY_API)
	struct brcmstb_named_range *nrange;
	struct brcmstb_range *range;
	int ret;
	int i, j;

	ret = brcmstb_memory_get(&bm);
	if (ret) {
		pr_err("could not get memory struct\n");
		return ret;
	}

	/* print ranges */
	pr_info("Range info:\n");
	for (i = 0; i < MAX_BRCMSTB_MEMC; ++i) {
		pr_info(" memc%d\n", i);
		for (j = 0; j < bm.memc[i].count; ++j) {
			if (j >= MAX_BRCMSTB_RANGE) {
				pr_warn("  Need to increase MAX_BRCMSTB_RANGE!\n");
				break;
			}
			range = &bm.memc[i].range[j];
			pr_info("  %llu MiB at %#016llx\n",
					range->size / SZ_1M, range->addr);
		}
	}

	pr_info("lowmem info:\n");
	for (i = 0; i < bm.lowmem.count; ++i) {
		if (i >= MAX_BRCMSTB_RANGE) {
			pr_warn(" Need to increase MAX_BRCMSTB_RANGE!\n");
			break;
		}
		range = &bm.lowmem.range[i];
		pr_info(" %llu MiB at %#016llx\n",
				range->size / SZ_1M, range->addr);
	}

	pr_info("bmem info:\n");
	for (i = 0; i < bm.bmem.count; ++i) {
		if (i >= MAX_BRCMSTB_RANGE) {
			pr_warn(" Need to increase MAX_BRCMSTB_RANGE!\n");
			break;
		}
		range = &bm.bmem.range[i];
		pr_info(" %llu MiB at %#016llx\n",
				range->size / SZ_1M, range->addr);
		test_kva_mem_map(range);
	}

	pr_info("cma info:\n");
	for (i = 0; i < bm.cma.count; ++i) {
		if (i >= MAX_BRCMSTB_RANGE) {
			pr_warn(" Need to increase MAX_BRCMSTB_RANGE!\n");
			break;
		}
		range = &bm.cma.range[i];
		pr_info(" %llu MiB at %#016llx\n",
				range->size / SZ_1M, range->addr);
	}

	pr_info("reserved info:\n");
	for (i = 0; i < bm.reserved.count; ++i) {
		if (i >= MAX_BRCMSTB_RESERVED_RANGE) {
			pr_warn(" Need to increase MAX_BRCMSTB_RESERVED_RANGE!\n");
			break;
		}
		range = &bm.reserved.range[i];
		nrange = &bm.reserved.range_name[i];
		pr_info(" %#016llx-%#016llx (%s)\n",
				range->addr, range->addr + range->size,
				nrange->name);
		if (!strncmp(nrange->name, "bmem", 4))
			test_kva_mem_map(range);
	}
	pr_info("bhpa info:\n");
	for (i = 0; i < bm.bhpa.count; ++i) {
		if (i >= MAX_BRCMSTB_RANGE) {
			pr_warn(" Need to increase MAX_BRCMSTB_RANGE!\n");
			break;
		}
		range = &bm.bhpa.range[i];
		pr_info(" %llu MiB at %#016llx\n",
				range->size / SZ_1M, range->addr);
	}
#else
	struct brcmstb_range *range;
	int i;

	pr_info("bhpa info:\n");
	for (i = 0;
	     brcmstb_hugepage_region_info(i, &bm.bhpa.range[i].addr, &bm.bhpa.range[i].size) == 0;
	     i++) {
		range = &bm.bhpa.range[i];
		pr_info(" %llu MiB at %#016llx\n",
				range->size / SZ_1M, range->addr);
	}
	bm.bhpa.count = i;
#endif
	if (i)
		bhpa_test();

#if IS_ENABLED(CONFIG_BRCMSTB_MEMORY_API)
	/* Test the obtention of the MEMC size */
	for (i = 0; i < MAX_BRCMSTB_MEMC; i++) {
		pr_info("MEMC%d size %llu MiB (%#016llx)\n",
			i, brcmstb_memory_memc_size(i) / SZ_1M,
			brcmstb_memory_memc_size(i));
	}
#endif

	return -EINVAL;
}

static void __exit test_exit(void)
{
	pr_info("Goodbye world\n");
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gregory Fong (Broadcom Corporation)");

