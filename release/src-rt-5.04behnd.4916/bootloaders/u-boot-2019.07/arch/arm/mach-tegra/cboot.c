// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016-2018, NVIDIA CORPORATION.
 */

#include <common.h>
#include <environment.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <stdlib.h>
#include <string.h>

#include <linux/ctype.h>
#include <linux/sizes.h>

#include <asm/arch/tegra.h>
#include <asm/arch-tegra/cboot.h>
#include <asm/armv8/mmu.h>

/*
 * Size of a region that's large enough to hold the relocated U-Boot and all
 * other allocations made around it (stack, heap, page tables, etc.)
 * In practice, running "bdinfo" at the shell prompt, the stack reaches about
 * 5MB from the address selected for ram_top as of the time of writing,
 * so a 16MB region should be plenty.
 */
#define MIN_USABLE_RAM_SIZE SZ_16M
/*
 * The amount of space we expect to require for stack usage. Used to validate
 * that all reservations fit into the region selected for the relocation target
 */
#define MIN_USABLE_STACK_SIZE SZ_1M

DECLARE_GLOBAL_DATA_PTR;

extern struct mm_region tegra_mem_map[];

/*
 * These variables are written to before relocation, and hence cannot be
 * in.bss, since .bss overlaps the DTB that's appended to the U-Boot binary.
 * The section attribute forces this into .data and avoids this issue. This
 * also has the nice side-effect of the content being valid after relocation.
 */

/* The number of valid entries in ram_banks[] */
static int ram_bank_count __attribute__((section(".data")));

/*
 * The usable top-of-RAM for U-Boot. This is both:
 * a) Below 4GB to avoid issues with peripherals that use 32-bit addressing.
 * b) At the end of a region that has enough space to hold the relocated U-Boot
 *    and all other allocations made around it (stack, heap, page tables, etc.)
 */
static u64 ram_top __attribute__((section(".data")));
/* The base address of the region of RAM that ends at ram_top */
static u64 region_base __attribute__((section(".data")));

/*
 * Explicitly put this in the .data section because it is written before the
 * .bss section is zeroed out but it needs to persist.
 */
unsigned long cboot_boot_x0 __attribute__((section(".data")));

void cboot_save_boot_params(unsigned long x0, unsigned long x1,
			    unsigned long x2, unsigned long x3)
{
	cboot_boot_x0 = x0;
}

int cboot_dram_init(void)
{
	unsigned int na, ns;
	const void *cboot_blob = (void *)cboot_boot_x0;
	int node, len, i;
	const u32 *prop;

	if (!cboot_blob)
		return -EINVAL;

	na = fdtdec_get_uint(cboot_blob, 0, "#address-cells", 2);
	ns = fdtdec_get_uint(cboot_blob, 0, "#size-cells", 2);

	node = fdt_path_offset(cboot_blob, "/memory");
	if (node < 0) {
		pr_err("Can't find /memory node in cboot DTB");
		hang();
	}
	prop = fdt_getprop(cboot_blob, node, "reg", &len);
	if (!prop) {
		pr_err("Can't find /memory/reg property in cboot DTB");
		hang();
	}

	/* Calculate the true # of base/size pairs to read */
	len /= 4;		/* Convert bytes to number of cells */
	len /= (na + ns);	/* Convert cells to number of banks */
	if (len > CONFIG_NR_DRAM_BANKS)
		len = CONFIG_NR_DRAM_BANKS;

	/* Parse the /memory node, and save useful entries */
	gd->ram_size = 0;
	ram_bank_count = 0;
	for (i = 0; i < len; i++) {
		u64 bank_start, bank_end, bank_size, usable_bank_size;

		/* Extract raw memory region data from DTB */
		bank_start = fdt_read_number(prop, na);
		prop += na;
		bank_size = fdt_read_number(prop, ns);
		prop += ns;
		gd->ram_size += bank_size;
		bank_end = bank_start + bank_size;
		debug("Bank %d: %llx..%llx (+%llx)\n", i,
		      bank_start, bank_end, bank_size);

		/*
		 * Align the bank to MMU section size. This is not strictly
		 * necessary, since the translation table construction code
		 * handles page granularity without issue. However, aligning
		 * the MMU entries reduces the size and number of levels in the
		 * page table, so is worth it.
		 */
		bank_start = ROUND(bank_start, SZ_2M);
		bank_end = bank_end & ~(SZ_2M - 1);
		bank_size = bank_end - bank_start;
		debug("  aligned: %llx..%llx (+%llx)\n",
		      bank_start, bank_end, bank_size);
		if (bank_end <= bank_start)
			continue;

		/* Record data used to create MMU translation tables */
		ram_bank_count++;
		/* Index below is deliberately 1-based to skip MMIO entry */
		tegra_mem_map[ram_bank_count].virt = bank_start;
		tegra_mem_map[ram_bank_count].phys = bank_start;
		tegra_mem_map[ram_bank_count].size = bank_size;
		tegra_mem_map[ram_bank_count].attrs =
			PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_INNER_SHARE;

		/* Determine best bank to relocate U-Boot into */
		if (bank_end > SZ_4G)
			bank_end = SZ_4G;
		debug("  end  %llx (usable)\n", bank_end);
		usable_bank_size = bank_end - bank_start;
		debug("  size %llx (usable)\n", usable_bank_size);
		if ((usable_bank_size >= MIN_USABLE_RAM_SIZE) &&
		    (bank_end > ram_top)) {
			ram_top = bank_end;
			region_base = bank_start;
			debug("ram top now %llx\n", ram_top);
		}
	}

	/* Ensure memory map contains the desired sentinel entry */
	tegra_mem_map[ram_bank_count + 1].virt = 0;
	tegra_mem_map[ram_bank_count + 1].phys = 0;
	tegra_mem_map[ram_bank_count + 1].size = 0;
	tegra_mem_map[ram_bank_count + 1].attrs = 0;

	/* Error out if a relocation target couldn't be found */
	if (!ram_top) {
		pr_err("Can't find a usable RAM top");
		hang();
	}

	return 0;
}

int cboot_dram_init_banksize(void)
{
	int i;

	if (ram_bank_count == 0)
		return -EINVAL;

	if ((gd->start_addr_sp - region_base) < MIN_USABLE_STACK_SIZE) {
		pr_err("Reservations exceed chosen region size");
		hang();
	}

	for (i = 0; i < ram_bank_count; i++) {
		gd->bd->bi_dram[i].start = tegra_mem_map[1 + i].virt;
		gd->bd->bi_dram[i].size = tegra_mem_map[1 + i].size;
	}

#ifdef CONFIG_PCI
	gd->pci_ram_top = ram_top;
#endif

	return 0;
}

ulong cboot_get_usable_ram_top(ulong total_size)
{
	return ram_top;
}

/*
 * The following few functions run late during the boot process and dynamically
 * calculate the load address of various binaries. To keep track of multiple
 * allocations, some writable list of RAM banks must be used. tegra_mem_map[]
 * is used for this purpose to avoid making yet another copy of the list of RAM
 * banks. This is safe because tegra_mem_map[] is only used once during very
 * early boot to create U-Boot's page tables, long before this code runs. If
 * this assumption becomes invalid later, we can just fix the code to copy the
 * list of RAM banks into some private data structure before running.
 */

static char *gen_varname(const char *var, const char *ext)
{
	size_t len_var = strlen(var);
	size_t len_ext = strlen(ext);
	size_t len = len_var + len_ext + 1;
	char *varext = malloc(len);

	if (!varext)
		return 0;
	strcpy(varext, var);
	strcpy(varext + len_var, ext);
	return varext;
}

static void mark_ram_allocated(int bank, u64 allocated_start, u64 allocated_end)
{
	u64 bank_start = tegra_mem_map[bank].virt;
	u64 bank_size = tegra_mem_map[bank].size;
	u64 bank_end = bank_start + bank_size;
	bool keep_front = allocated_start != bank_start;
	bool keep_tail = allocated_end != bank_end;

	if (keep_front && keep_tail) {
		/*
		 * There are CONFIG_NR_DRAM_BANKS DRAM entries in the array,
		 * starting at index 1 (index 0 is MMIO). So, we are at DRAM
		 * entry "bank" not "bank - 1" as for a typical 0-base array.
		 * The number of remaining DRAM entries is therefore
		 * "CONFIG_NR_DRAM_BANKS - bank". We want to duplicate the
		 * current entry and shift up the remaining entries, dropping
		 * the last one. Thus, we must copy one fewer entry than the
		 * number remaining.
		 */
		memmove(&tegra_mem_map[bank + 1], &tegra_mem_map[bank],
			CONFIG_NR_DRAM_BANKS - bank - 1);
		tegra_mem_map[bank].size = allocated_start - bank_start;
		bank++;
		tegra_mem_map[bank].virt = allocated_end;
		tegra_mem_map[bank].phys = allocated_end;
		tegra_mem_map[bank].size = bank_end - allocated_end;
	} else if (keep_front) {
		tegra_mem_map[bank].size = allocated_start - bank_start;
	} else if (keep_tail) {
		tegra_mem_map[bank].virt = allocated_end;
		tegra_mem_map[bank].phys = allocated_end;
		tegra_mem_map[bank].size = bank_end - allocated_end;
	} else {
		/*
		 * We could move all subsequent banks down in the array but
		 * that's not necessary for subsequent allocations to work, so
		 * we skip doing so.
		 */
		tegra_mem_map[bank].size = 0;
	}
}

static void reserve_ram(u64 start, u64 size)
{
	int bank;
	u64 end = start + size;

	for (bank = 1; bank <= CONFIG_NR_DRAM_BANKS; bank++) {
		u64 bank_start = tegra_mem_map[bank].virt;
		u64 bank_size = tegra_mem_map[bank].size;
		u64 bank_end = bank_start + bank_size;

		if (end <= bank_start || start > bank_end)
			continue;
		mark_ram_allocated(bank, start, end);
		break;
	}
}

static u64 alloc_ram(u64 size, u64 align, u64 offset)
{
	int bank;

	for (bank = 1; bank <= CONFIG_NR_DRAM_BANKS; bank++) {
		u64 bank_start = tegra_mem_map[bank].virt;
		u64 bank_size = tegra_mem_map[bank].size;
		u64 bank_end = bank_start + bank_size;
		u64 allocated = ROUND(bank_start, align) + offset;
		u64 allocated_end = allocated + size;

		if (allocated_end > bank_end)
			continue;
		mark_ram_allocated(bank, allocated, allocated_end);
		return allocated;
	}
	return 0;
}

static void set_calculated_aliases(char *aliases, u64 address)
{
	char *tmp, *alias;
	int err;

	aliases = strdup(aliases);
	if (!aliases) {
		pr_err("strdup(aliases) failed");
		return;
	}

	tmp = aliases;
	while (true) {
		alias = strsep(&tmp, " ");
		if (!alias)
			break;
		debug("%s: alias: %s\n", __func__, alias);
		err = env_set_hex(alias, address);
		if (err)
			pr_err("Could not set %s\n", alias);
	}

	free(aliases);
}

static void set_calculated_env_var(const char *var)
{
	char *var_size;
	char *var_align;
	char *var_offset;
	char *var_aliases;
	u64 size;
	u64 align;
	u64 offset;
	char *aliases;
	u64 address;
	int err;

	var_size = gen_varname(var, "_size");
	if (!var_size)
		return;
	var_align = gen_varname(var, "_align");
	if (!var_align)
		goto out_free_var_size;
	var_offset = gen_varname(var, "_offset");
	if (!var_offset)
		goto out_free_var_align;
	var_aliases = gen_varname(var, "_aliases");
	if (!var_aliases)
		goto out_free_var_offset;

	size = env_get_hex(var_size, 0);
	if (!size) {
		pr_err("%s not set or zero\n", var_size);
		goto out_free_var_aliases;
	}
	align = env_get_hex(var_align, 1);
	/* Handle extant variables, but with a value of 0 */
	if (!align)
		align = 1;
	offset = env_get_hex(var_offset, 0);
	aliases = env_get(var_aliases);

	debug("%s: Calc var %s; size=%llx, align=%llx, offset=%llx\n",
	      __func__, var, size, align, offset);
	if (aliases)
		debug("%s: Aliases: %s\n", __func__, aliases);

	address = alloc_ram(size, align, offset);
	if (!address) {
		pr_err("Could not allocate %s\n", var);
		goto out_free_var_aliases;
	}
	debug("%s: Address %llx\n", __func__, address);

	err = env_set_hex(var, address);
	if (err)
		pr_err("Could not set %s\n", var);
	if (aliases)
		set_calculated_aliases(aliases, address);

out_free_var_aliases:
	free(var_aliases);
out_free_var_offset:
	free(var_offset);
out_free_var_align:
	free(var_align);
out_free_var_size:
	free(var_size);
}

#ifdef DEBUG
static void dump_ram_banks(void)
{
	int bank;

	for (bank = 1; bank <= CONFIG_NR_DRAM_BANKS; bank++) {
		u64 bank_start = tegra_mem_map[bank].virt;
		u64 bank_size = tegra_mem_map[bank].size;
		u64 bank_end = bank_start + bank_size;

		if (!bank_size)
			continue;
		printf("%d: %010llx..%010llx (+%010llx)\n", bank - 1,
		       bank_start, bank_end, bank_size);
	}
}
#endif

static void set_calculated_env_vars(void)
{
	char *vars, *tmp, *var;

#ifdef DEBUG
	printf("RAM banks before any calculated env. var.s:\n");
	dump_ram_banks();
#endif

	reserve_ram(cboot_boot_x0, fdt_totalsize(cboot_boot_x0));

#ifdef DEBUG
	printf("RAM after reserving cboot DTB:\n");
	dump_ram_banks();
#endif

	vars = env_get("calculated_vars");
	if (!vars) {
		debug("%s: No env var calculated_vars\n", __func__);
		return;
	}

	vars = strdup(vars);
	if (!vars) {
		pr_err("strdup(calculated_vars) failed");
		return;
	}

	tmp = vars;
	while (true) {
		var = strsep(&tmp, " ");
		if (!var)
			break;
		debug("%s: var: %s\n", __func__, var);
		set_calculated_env_var(var);
#ifdef DEBUG
		printf("RAM banks after allocating %s:\n", var);
		dump_ram_banks();
#endif
	}

	free(vars);
}

static int set_fdt_addr(void)
{
	int ret;

	ret = env_set_hex("fdt_addr", cboot_boot_x0);
	if (ret) {
		printf("Failed to set fdt_addr to point at DTB: %d\n", ret);
		return ret;
	}

	return 0;
}

/*
 * Attempt to use /chosen/nvidia,ether-mac in the cboot DTB to U-Boot's
 * ethaddr environment variable if possible.
 */
static int cboot_get_ethaddr_legacy(const void *fdt, uint8_t mac[ETH_ALEN])
{
	const char *const properties[] = {
		"nvidia,ethernet-mac",
		"nvidia,ether-mac",
	};
	const char *prop;
	unsigned int i;
	int node, len;

	node = fdt_path_offset(fdt, "/chosen");
	if (node < 0) {
		printf("Can't find /chosen node in cboot DTB\n");
		return node;
	}

	for (i = 0; i < ARRAY_SIZE(properties); i++) {
		prop = fdt_getprop(fdt, node, properties[i], &len);
		if (prop)
			break;
	}

	if (!prop) {
		printf("Can't find Ethernet MAC address in cboot DTB\n");
		return -ENOENT;
	}

	eth_parse_enetaddr(prop, mac);

	if (!is_valid_ethaddr(mac)) {
		printf("Invalid MAC address: %s\n", prop);
		return -EINVAL;
	}

	debug("Legacy MAC address: %pM\n", mac);

	return 0;
}

int cboot_get_ethaddr(const void *fdt, uint8_t mac[ETH_ALEN])
{
	int node, len, err = 0;
	const uchar *prop;
	const char *path;

	path = fdt_get_alias(fdt, "ethernet");
	if (!path) {
		err = -ENOENT;
		goto out;
	}

	debug("ethernet alias found: %s\n", path);

	node = fdt_path_offset(fdt, path);
	if (node < 0) {
		err = -ENOENT;
		goto out;
	}

	prop = fdt_getprop(fdt, node, "local-mac-address", &len);
	if (!prop) {
		err = -ENOENT;
		goto out;
	}

	if (len != ETH_ALEN) {
		err = -EINVAL;
		goto out;
	}

	debug("MAC address: %pM\n", prop);
	memcpy(mac, prop, ETH_ALEN);

out:
	if (err < 0)
		err = cboot_get_ethaddr_legacy(fdt, mac);

	return err;
}

static char *strip(const char *ptr)
{
	const char *end;

	while (*ptr && isblank(*ptr))
		ptr++;

	/* empty string */
	if (*ptr == '\0')
		return strdup(ptr);

	end = ptr;

	while (end[1])
		end++;

	while (isblank(*end))
		end--;

	return strndup(ptr, end - ptr + 1);
}

static char *cboot_get_bootargs(const void *fdt)
{
	const char *args;
	int offset, len;

	offset = fdt_path_offset(fdt, "/chosen");
	if (offset < 0)
		return NULL;

	args = fdt_getprop(fdt, offset, "bootargs", &len);
	if (!args)
		return NULL;

	return strip(args);
}

int cboot_late_init(void)
{
	const void *fdt = (const void *)cboot_boot_x0;
	uint8_t mac[ETH_ALEN];
	char *bootargs;
	int err;

	set_calculated_env_vars();
	/*
	 * Ignore errors here; the value may not be used depending on
	 * extlinux.conf or boot script content.
	 */
	set_fdt_addr();

	/* Ignore errors here; not all cases care about Ethernet addresses */
	err = cboot_get_ethaddr(fdt, mac);
	if (!err) {
		void *blob = (void *)gd->fdt_blob;

		err = fdtdec_set_ethernet_mac_address(blob, mac, sizeof(mac));
		if (err < 0)
			printf("failed to set MAC address %pM: %d\n", mac, err);
	}

	bootargs = cboot_get_bootargs(fdt);
	if (bootargs) {
		env_set("cbootargs", bootargs);
		free(bootargs);
	}

	return 0;
}
