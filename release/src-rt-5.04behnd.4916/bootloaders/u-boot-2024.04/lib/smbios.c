// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Adapted from coreboot src/arch/x86/smbios.c
 */

#include <dm.h>
#include <env.h>
#include <linux/stringify.h>
#include <linux/string.h>
#include <mapmem.h>
#include <smbios.h>
#include <sysinfo.h>
#include <tables_csum.h>
#include <version.h>
#include <malloc.h>
#include <dm/ofnode.h>
#ifdef CONFIG_CPU
#include <cpu.h>
#include <dm/uclass-internal.h>
#endif

/* Safeguard for checking that U_BOOT_VERSION_NUM macros are compatible with U_BOOT_DMI */
#if U_BOOT_VERSION_NUM < 2000 || U_BOOT_VERSION_NUM > 2099 || \
    U_BOOT_VERSION_NUM_PATCH < 1 || U_BOOT_VERSION_NUM_PATCH > 12
#error U_BOOT_VERSION_NUM macros are not compatible with DMI, fix U_BOOT_DMI macros
#endif

/*
 * U_BOOT_DMI_DATE contains BIOS Release Date in format mm/dd/yyyy.
 * BIOS Release Date is calculated from U-Boot version and fixed day 01.
 * So for U-Boot version 2021.04 it is calculated as "04/01/2021".
 * BIOS Release Date should contain date when code was released
 * and not when it was built or compiled.
 */
#if U_BOOT_VERSION_NUM_PATCH < 10
#define U_BOOT_DMI_MONTH "0" __stringify(U_BOOT_VERSION_NUM_PATCH)
#else
#define U_BOOT_DMI_MONTH __stringify(U_BOOT_VERSION_NUM_PATCH)
#endif
#define U_BOOT_DMI_DAY "01"
#define U_BOOT_DMI_YEAR __stringify(U_BOOT_VERSION_NUM)
#define U_BOOT_DMI_DATE U_BOOT_DMI_MONTH "/" U_BOOT_DMI_DAY "/" U_BOOT_DMI_YEAR

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct map_sysinfo - Mapping of sysinfo strings to DT
 *
 * @si_str: sysinfo string
 * @dt_str: DT string
 * @max: Max index of the tokenized string to pick. Counting starts from 0
 *
 */
struct map_sysinfo {
	const char *si_node;
	const char *si_str;
	const char *dt_str;
	int max;
};

static const struct map_sysinfo sysinfo_to_dt[] = {
	{ .si_node = "system", .si_str = "product", .dt_str = "model", 2 },
	{ .si_node = "system", .si_str = "manufacturer", .dt_str = "compatible", 1 },
	{ .si_node = "baseboard", .si_str = "product", .dt_str = "model", 2 },
	{ .si_node = "baseboard", .si_str = "manufacturer", .dt_str = "compatible", 1 },
};

/**
 * struct smbios_ctx - context for writing SMBIOS tables
 *
 * @node:		node containing the information to write (ofnode_null()
 *			if none)
 * @dev:		sysinfo device to use (NULL if none)
 * @subnode_name:	sysinfo subnode_name. Used for DT fallback
 * @eos:		end-of-string pointer for the table being processed.
 *			This is set up when we start processing a table
 * @next_ptr:		pointer to the start of the next string to be added.
 *			When the table is not empty, this points to the byte
 *			after the \0 of the previous string.
 * @last_str:		points to the last string that was written to the table,
 *			or NULL if none
 */
struct smbios_ctx {
	ofnode node;
	struct udevice *dev;
	const char *subnode_name;
	char *eos;
	char *next_ptr;
	char *last_str;
};

/**
 * Function prototype to write a specific type of SMBIOS structure
 *
 * @addr:	start address to write the structure
 * @handle:	the structure's handle, a unique 16-bit number
 * @ctx:	context for writing the tables
 * Return:	size of the structure
 */
typedef int (*smbios_write_type)(ulong *addr, int handle,
				 struct smbios_ctx *ctx);

/**
 * struct smbios_write_method - Information about a table-writing function
 *
 * @write: Function to call
 * @subnode_name: Name of subnode which has the information for this function,
 *	NULL if none
 */
struct smbios_write_method {
	smbios_write_type write;
	const char *subnode_name;
};

static const struct map_sysinfo *convert_sysinfo_to_dt(const char *node, const char *si)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sysinfo_to_dt); i++) {
		if (node && !strcmp(node, sysinfo_to_dt[i].si_node) &&
		    !strcmp(si, sysinfo_to_dt[i].si_str))
			return &sysinfo_to_dt[i];
	}

	return NULL;
}

/**
 * smbios_add_string() - add a string to the string area
 *
 * This adds a string to the string area which is appended directly after
 * the formatted portion of an SMBIOS structure.
 *
 * @ctx:	SMBIOS context
 * @str:	string to add
 * Return:	string number in the string area. 0 if str is NULL.
 */
static int smbios_add_string(struct smbios_ctx *ctx, const char *str)
{
	int i = 1;
	char *p = ctx->eos;

	if (!str)
		return 0;

	for (;;) {
		if (!*p) {
			ctx->last_str = p;
			strcpy(p, str);
			p += strlen(str);
			*p++ = '\0';
			ctx->next_ptr = p;
			*p++ = '\0';

			return i;
		}

		if (!strcmp(p, str)) {
			ctx->last_str = p;
			return i;
		}

		p += strlen(p) + 1;
		i++;
	}
}

/**
 * get_str_from_dt - Get a substring from a DT property.
 *                   After finding the property in the DT, the function
 *                   will parse comma-separated values and return the value.
 *                   If nprop->max exceeds the number of comma-separated
 *                   elements, the last non NULL value will be returned.
 *                   Counting starts from zero.
 *
 * @nprop: sysinfo property to use
 * @str: pointer to fill with data
 * @size: str buffer length
 */
static
void get_str_from_dt(const struct map_sysinfo *nprop, char *str, size_t size)
{
	const char *dt_str;
	int cnt = 0;
	char *token;

	memset(str, 0, size);
	if (!nprop || !nprop->max)
		return;

	dt_str = ofnode_read_string(ofnode_root(), nprop->dt_str);
	if (!dt_str)
		return;

	memcpy(str, dt_str, size);
	token = strtok(str, ",");
	while (token && cnt < nprop->max) {
		strlcpy(str, token, strlen(token) + 1);
		token = strtok(NULL, ",");
		cnt++;
	}
}

/**
 * smbios_add_prop_si() - Add a property from the devicetree or sysinfo
 *
 * Sysinfo is used if available, with a fallback to devicetree
 *
 * @ctx:	context for writing the tables
 * @prop:	property to write
 * @sysinfo_id: unique identifier for the string value to be read
 * @dval:	Default value to use if the string is not found or is empty
 * Return:	0 if not found, else SMBIOS string number (1 or more)
 */
static int smbios_add_prop_si(struct smbios_ctx *ctx, const char *prop,
			      int sysinfo_id, const char *dval)
{
	int ret;

	if (!dval || !*dval)
		dval = NULL;

	if (!prop)
		return smbios_add_string(ctx, dval);

	if (sysinfo_id && ctx->dev) {
		char val[SMBIOS_STR_MAX];

		ret = sysinfo_get_str(ctx->dev, sysinfo_id, sizeof(val), val);
		if (!ret)
			return smbios_add_string(ctx, val);
	}
	if (IS_ENABLED(CONFIG_OF_CONTROL)) {
		const char *str = NULL;
		char str_dt[128] = { 0 };
		/*
		 * If the node is not valid fallback and try the entire DT
		 * so we can at least fill in manufacturer and board type
		 */
		if (ofnode_valid(ctx->node)) {
			str = ofnode_read_string(ctx->node, prop);
		} else {
			const struct map_sysinfo *nprop;

			nprop = convert_sysinfo_to_dt(ctx->subnode_name, prop);
			get_str_from_dt(nprop, str_dt, sizeof(str_dt));
			str = (const char *)str_dt;
		}

		ret = smbios_add_string(ctx, str && *str ? str : dval);
		return ret;
	}

	return 0;
}

/**
 * smbios_add_prop() - Add a property from the devicetree
 *
 * @prop:	property to write. The default string will be written if
 *		prop is NULL
 * @dval:	Default value to use if the string is not found or is empty
 * Return:	0 if not found, else SMBIOS string number (1 or more)
 */
static int smbios_add_prop(struct smbios_ctx *ctx, const char *prop,
			   const char *dval)
{
	return smbios_add_prop_si(ctx, prop, SYSINFO_ID_NONE, dval);
}

static void smbios_set_eos(struct smbios_ctx *ctx, char *eos)
{
	ctx->eos = eos;
	ctx->next_ptr = eos;
	ctx->last_str = NULL;
}

int smbios_update_version(const char *version)
{
	char *ptr = gd->smbios_version;
	uint old_len, len;

	if (!ptr)
		return log_ret(-ENOENT);

	/*
	 * This string is supposed to have at least enough bytes and is
	 * padded with spaces. Update it, taking care not to move the
	 * \0 terminator, so that other strings in the string table
	 * are not disturbed. See smbios_add_string()
	 */
	old_len = strnlen(ptr, SMBIOS_STR_MAX);
	len = strnlen(version, SMBIOS_STR_MAX);
	if (len > old_len)
		return log_ret(-ENOSPC);

	log_debug("Replacing SMBIOS type 0 version string '%s'\n", ptr);
	memcpy(ptr, version, len);
#ifdef LOG_DEBUG
	print_buffer((ulong)ptr, ptr, 1, old_len + 1, 0);
#endif

	return 0;
}

/**
 * smbios_string_table_len() - compute the string area size
 *
 * This computes the size of the string area including the string terminator.
 *
 * @ctx:	SMBIOS context
 * Return:	string area size
 */
static int smbios_string_table_len(const struct smbios_ctx *ctx)
{
	/* In case no string is defined we have to return two \0 */
	if (ctx->next_ptr == ctx->eos)
		return 2;

	/* Allow for the final \0 after all strings */
	return (ctx->next_ptr + 1) - ctx->eos;
}

static int smbios_write_type0(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type0 *t;
	int len = sizeof(struct smbios_type0);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type0));
	fill_smbios_header(t, SMBIOS_BIOS_INFORMATION, len, handle);
	smbios_set_eos(ctx, t->eos);
	t->vendor = smbios_add_prop(ctx, NULL, "U-Boot");

	t->bios_ver = smbios_add_prop(ctx, "version", PLAIN_VERSION);
	if (t->bios_ver)
		gd->smbios_version = ctx->last_str;
	log_debug("smbios_version = %p: '%s'\n", gd->smbios_version,
		  gd->smbios_version);
#ifdef LOG_DEBUG
	print_buffer((ulong)gd->smbios_version, gd->smbios_version,
		     1, strlen(gd->smbios_version) + 1, 0);
#endif
	t->bios_release_date = smbios_add_prop(ctx, NULL, U_BOOT_DMI_DATE);
#ifdef CONFIG_ROM_SIZE
	t->bios_rom_size = (CONFIG_ROM_SIZE / 65536) - 1;
#endif
	t->bios_characteristics = BIOS_CHARACTERISTICS_PCI_SUPPORTED |
				  BIOS_CHARACTERISTICS_SELECTABLE_BOOT |
				  BIOS_CHARACTERISTICS_UPGRADEABLE;
#ifdef CONFIG_GENERATE_ACPI_TABLE
	t->bios_characteristics_ext1 = BIOS_CHARACTERISTICS_EXT1_ACPI;
#endif
#ifdef CONFIG_EFI_LOADER
	t->bios_characteristics_ext2 |= BIOS_CHARACTERISTICS_EXT2_UEFI;
#endif
	t->bios_characteristics_ext2 |= BIOS_CHARACTERISTICS_EXT2_TARGET;

	/* bios_major_release has only one byte, so drop century */
	t->bios_major_release = U_BOOT_VERSION_NUM % 100;
	t->bios_minor_release = U_BOOT_VERSION_NUM_PATCH;
	t->ec_major_release = 0xff;
	t->ec_minor_release = 0xff;

	len = t->length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type1(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type1 *t;
	int len = sizeof(struct smbios_type1);
	char *serial_str = env_get("serial#");

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type1));
	fill_smbios_header(t, SMBIOS_SYSTEM_INFORMATION, len, handle);
	smbios_set_eos(ctx, t->eos);
	t->manufacturer = smbios_add_prop(ctx, "manufacturer", NULL);
	t->product_name = smbios_add_prop(ctx, "product", NULL);
	t->version = smbios_add_prop_si(ctx, "version",
					SYSINFO_ID_SMBIOS_SYSTEM_VERSION,
					NULL);
	if (serial_str) {
		t->serial_number = smbios_add_prop(ctx, NULL, serial_str);
		strncpy((char *)t->uuid, serial_str, sizeof(t->uuid));
	} else {
		t->serial_number = smbios_add_prop(ctx, "serial", NULL);
	}
	t->sku_number = smbios_add_prop(ctx, "sku", NULL);
	t->family = smbios_add_prop(ctx, "family", NULL);

	len = t->length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type2(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type2 *t;
	int len = sizeof(struct smbios_type2);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type2));
	fill_smbios_header(t, SMBIOS_BOARD_INFORMATION, len, handle);
	smbios_set_eos(ctx, t->eos);
	t->manufacturer = smbios_add_prop(ctx, "manufacturer", NULL);
	t->product_name = smbios_add_prop(ctx, "product", NULL);
	t->version = smbios_add_prop_si(ctx, "version",
					SYSINFO_ID_SMBIOS_BASEBOARD_VERSION,
					NULL);
	t->asset_tag_number = smbios_add_prop(ctx, "asset-tag", NULL);
	t->feature_flags = SMBIOS_BOARD_FEATURE_HOSTING;
	t->board_type = SMBIOS_BOARD_MOTHERBOARD;
	t->chassis_handle = handle + 1;

	len = t->length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type3(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type3 *t;
	int len = sizeof(struct smbios_type3);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type3));
	fill_smbios_header(t, SMBIOS_SYSTEM_ENCLOSURE, len, handle);
	smbios_set_eos(ctx, t->eos);
	t->manufacturer = smbios_add_prop(ctx, "manufacturer", NULL);
	t->chassis_type = SMBIOS_ENCLOSURE_DESKTOP;
	t->bootup_state = SMBIOS_STATE_SAFE;
	t->power_supply_state = SMBIOS_STATE_SAFE;
	t->thermal_state = SMBIOS_STATE_SAFE;
	t->security_status = SMBIOS_SECURITY_NONE;

	len = t->length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static void smbios_write_type4_dm(struct smbios_type4 *t,
				  struct smbios_ctx *ctx)
{
	u16 processor_family = SMBIOS_PROCESSOR_FAMILY_UNKNOWN;
	const char *vendor = NULL;
	const char *name = NULL;

#ifdef CONFIG_CPU
	char processor_name[49];
	char vendor_name[49];
	struct udevice *cpu = NULL;

	uclass_find_first_device(UCLASS_CPU, &cpu);
	if (cpu) {
		struct cpu_plat *plat = dev_get_parent_plat(cpu);

		if (plat->family)
			processor_family = plat->family;
		t->processor_id[0] = plat->id[0];
		t->processor_id[1] = plat->id[1];

		if (!cpu_get_vendor(cpu, vendor_name, sizeof(vendor_name)))
			vendor = vendor_name;
		if (!cpu_get_desc(cpu, processor_name, sizeof(processor_name)))
			name = processor_name;
	}
#endif

	t->processor_family = 0xfe;
	t->processor_family2 = processor_family;
	t->processor_manufacturer = smbios_add_prop(ctx, NULL, vendor);
	t->processor_version = smbios_add_prop(ctx, NULL, name);
}

static int smbios_write_type4(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type4 *t;
	int len = sizeof(struct smbios_type4);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type4));
	fill_smbios_header(t, SMBIOS_PROCESSOR_INFORMATION, len, handle);
	smbios_set_eos(ctx, t->eos);
	t->processor_type = SMBIOS_PROCESSOR_TYPE_CENTRAL;
	smbios_write_type4_dm(t, ctx);
	t->status = SMBIOS_PROCESSOR_STATUS_ENABLED;
	t->processor_upgrade = SMBIOS_PROCESSOR_UPGRADE_NONE;
	t->l1_cache_handle = 0xffff;
	t->l2_cache_handle = 0xffff;
	t->l3_cache_handle = 0xffff;

	len = t->length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type32(ulong *current, int handle,
			       struct smbios_ctx *ctx)
{
	struct smbios_type32 *t;
	int len = sizeof(struct smbios_type32);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type32));
	fill_smbios_header(t, SMBIOS_SYSTEM_BOOT_INFORMATION, len, handle);
	smbios_set_eos(ctx, t->eos);

	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type127(ulong *current, int handle,
				struct smbios_ctx *ctx)
{
	struct smbios_type127 *t;
	int len = sizeof(struct smbios_type127);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type127));
	fill_smbios_header(t, SMBIOS_END_OF_TABLE, len, handle);

	*current += len;
	unmap_sysmem(t);

	return len;
}

static struct smbios_write_method smbios_write_funcs[] = {
	{ smbios_write_type0, "bios", },
	{ smbios_write_type1, "system", },
	{ smbios_write_type2, "baseboard", },
	/* Type 3 must immediately follow type 2 due to chassis handle. */
	{ smbios_write_type3, "chassis", },
	{ smbios_write_type4, },
	{ smbios_write_type32, },
	{ smbios_write_type127 },
};

ulong write_smbios_table(ulong addr)
{
	ofnode parent_node = ofnode_null();
	ulong table_addr, start_addr;
	struct smbios3_entry *se;
	struct smbios_ctx ctx;
	ulong tables;
	int len = 0;
	int handle = 0;
	int i;

	ctx.node = ofnode_null();
	if (IS_ENABLED(CONFIG_OF_CONTROL)) {
		uclass_first_device(UCLASS_SYSINFO, &ctx.dev);
		if (ctx.dev)
			parent_node = dev_read_subnode(ctx.dev, "smbios");
	} else {
		ctx.dev = NULL;
	}

	start_addr = addr;

	/* move past the (so-far-unwritten) header to start writing structs */
	addr = ALIGN(addr + sizeof(struct smbios3_entry), 16);
	tables = addr;

	/* populate minimum required tables */
	for (i = 0; i < ARRAY_SIZE(smbios_write_funcs); i++) {
		const struct smbios_write_method *method;

		method = &smbios_write_funcs[i];
		ctx.subnode_name = NULL;
		if (method->subnode_name) {
			ctx.subnode_name = method->subnode_name;
			if (IS_ENABLED(CONFIG_OF_CONTROL))
				ctx.node = ofnode_find_subnode(parent_node,
							       method->subnode_name);
		}
		len += method->write((ulong *)&addr, handle++, &ctx);
	}

	/*
	 * We must use a pointer here so things work correctly on sandbox. The
	 * user of this table is not aware of the mapping of addresses to
	 * sandbox's DRAM buffer.
	 */
	table_addr = (ulong)map_sysmem(tables, 0);

	/* now go back and write the SMBIOS3 header */
	se = map_sysmem(start_addr, sizeof(struct smbios3_entry));
	memset(se, '\0', sizeof(struct smbios3_entry));
	memcpy(se->anchor, "_SM3_", 5);
	se->length = sizeof(struct smbios3_entry);
	se->major_ver = SMBIOS_MAJOR_VER;
	se->minor_ver = SMBIOS_MINOR_VER;
	se->doc_rev = 0;
	se->entry_point_rev = 1;
	se->table_maximum_size = len;
	se->struct_table_address = table_addr;
	se->checksum = table_compute_checksum(se, sizeof(struct smbios3_entry));
	unmap_sysmem(se);

	return addr;
}
