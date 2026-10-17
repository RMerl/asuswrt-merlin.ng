// SPDX-License-Identifier: GPL-2.0+
/*
 *  UEFI Shell-like command
 *
 *  Copyright (c) 2018 AKASHI Takahiro, Linaro Limited
 */

#include <charset.h>
#include <common.h>
#include <command.h>
#include <dm/device.h>
#include <efi_dt_fixup.h>
#include <efi_load_initrd.h>
#include <efi_loader.h>
#include <efi_rng.h>
#include <efi_variable.h>
#include <exports.h>
#include <hexdump.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <net.h>
#include <part.h>
#include <search.h>
#include <linux/ctype.h>
#include <linux/err.h>

#define BS systab.boottime

#ifdef CONFIG_EFI_HAVE_CAPSULE_SUPPORT
/**
 * do_efi_capsule_update() - process a capsule update
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "capsule update" sub-command.
 * process a capsule update.
 *
 *     efidebug capsule update [-v] <capsule address>
 */
static int do_efi_capsule_update(struct cmd_tbl *cmdtp, int flag,
				 int argc, char * const argv[])
{
	struct efi_capsule_header *capsule;
	int verbose = 0;
	char *endp;
	efi_status_t ret;

	if (argc != 2 && argc != 3)
		return CMD_RET_USAGE;

	if (argc == 3) {
		if (strcmp(argv[1], "-v"))
			return CMD_RET_USAGE;

		verbose = 1;
		argc--;
		argv++;
	}

	capsule = (typeof(capsule))hextoul(argv[1], &endp);
	if (endp == argv[1]) {
		printf("Invalid address: %s", argv[1]);
		return CMD_RET_FAILURE;
	}

	if (verbose) {
		printf("Capsule guid: %pUl\n", &capsule->capsule_guid);
		printf("Capsule flags: 0x%x\n", capsule->flags);
		printf("Capsule header size: 0x%x\n", capsule->header_size);
		printf("Capsule image size: 0x%x\n",
		       capsule->capsule_image_size);
	}

	ret = EFI_CALL(efi_update_capsule(&capsule, 1, 0));
	if (ret) {
		printf("Cannot handle a capsule at %p\n", capsule);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

#ifdef CONFIG_EFI_CAPSULE_ON_DISK
static int do_efi_capsule_on_disk_update(struct cmd_tbl *cmdtp, int flag,
					 int argc, char * const argv[])
{
	efi_status_t ret;

	ret = efi_launch_capsules();

	return ret == EFI_SUCCESS ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}
#endif

/**
 * do_efi_capsule_show() - show capsule information
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "capsule show" sub-command.
 * show capsule information.
 *
 *     efidebug capsule show <capsule address>
 */
static int do_efi_capsule_show(struct cmd_tbl *cmdtp, int flag,
			       int argc, char * const argv[])
{
	struct efi_capsule_header *capsule;
	char *endp;

	if (argc != 2)
		return CMD_RET_USAGE;

	capsule = (typeof(capsule))hextoul(argv[1], &endp);
	if (endp == argv[1]) {
		printf("Invalid address: %s", argv[1]);
		return CMD_RET_FAILURE;
	}

	printf("Capsule guid: %pUl\n", &capsule->capsule_guid);
	printf("Capsule flags: 0x%x\n", capsule->flags);
	printf("Capsule header size: 0x%x\n", capsule->header_size);
	printf("Capsule image size: 0x%x\n",
	       capsule->capsule_image_size);

	return CMD_RET_SUCCESS;
}

#ifdef CONFIG_EFI_ESRT

#define EFI_ESRT_FW_TYPE_NUM 4
char *efi_fw_type_str[EFI_ESRT_FW_TYPE_NUM] = {"unknown", "system FW", "device FW",
	 "UEFI driver"};

#define EFI_ESRT_UPDATE_STATUS_NUM 9
char *efi_update_status_str[EFI_ESRT_UPDATE_STATUS_NUM] = {"success", "unsuccessful",
	"insufficient resources", "incorrect version", "invalid format",
	"auth error", "power event (AC)", "power event (batt)",
	"unsatisfied dependencies"};

#define EFI_FW_TYPE_STR_GET(idx) (\
EFI_ESRT_FW_TYPE_NUM > (idx) ? efi_fw_type_str[(idx)] : "error"\
)

#define EFI_FW_STATUS_STR_GET(idx) (\
EFI_ESRT_UPDATE_STATUS_NUM  > (idx) ? efi_update_status_str[(idx)] : "error"\
)

/**
 * do_efi_capsule_esrt() - manage UEFI capsules
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success,
 *		CMD_RET_USAGE or CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "capsule esrt" sub-command.
 * The prints the current ESRT table.
 *
 *     efidebug capsule esrt
 */
static int do_efi_capsule_esrt(struct cmd_tbl *cmdtp, int flag,
			       int argc, char * const argv[])
{
	struct efi_system_resource_table *esrt;

	if (argc != 1)
		return CMD_RET_USAGE;

	esrt = efi_get_configuration_table(&efi_esrt_guid);
	if (!esrt) {
		log_info("ESRT: table not present\n");
		return CMD_RET_SUCCESS;
	}

	printf("========================================\n");
	printf("ESRT: fw_resource_count=%d\n", esrt->fw_resource_count);
	printf("ESRT: fw_resource_count_max=%d\n", esrt->fw_resource_count_max);
	printf("ESRT: fw_resource_version=%lld\n", esrt->fw_resource_version);

	for (int idx = 0; idx < esrt->fw_resource_count; idx++) {
		printf("[entry %d]==============================\n", idx);
		printf("ESRT: fw_class=%pUL\n", &esrt->entries[idx].fw_class);
		printf("ESRT: fw_type=%s\n", EFI_FW_TYPE_STR_GET(esrt->entries[idx].fw_type));
		printf("ESRT: fw_version=%d\n", esrt->entries[idx].fw_version);
		printf("ESRT: lowest_supported_fw_version=%d\n",
		       esrt->entries[idx].lowest_supported_fw_version);
		printf("ESRT: capsule_flags=%d\n",
		       esrt->entries[idx].capsule_flags);
		printf("ESRT: last_attempt_version=%d\n",
		       esrt->entries[idx].last_attempt_version);
		printf("ESRT: last_attempt_status=%s\n",
		       EFI_FW_STATUS_STR_GET(esrt->entries[idx].last_attempt_status));
	}
	printf("========================================\n");

	return CMD_RET_SUCCESS;
}
#endif /*  CONFIG_EFI_ESRT */
/**
 * do_efi_capsule_res() - show a capsule update result
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "capsule result" sub-command.
 * show a capsule update result.
 * If result number is not specified, CapsuleLast will be shown.
 *
 *     efidebug capsule result [<capsule result number>]
 */
static int do_efi_capsule_res(struct cmd_tbl *cmdtp, int flag,
			      int argc, char * const argv[])
{
	int capsule_id;
	char *endp;
	u16 var_name16[12];
	efi_guid_t guid;
	struct efi_capsule_result_variable_header *result = NULL;
	efi_uintn_t size;
	efi_status_t ret;

	if (argc != 1 && argc != 2)
		return CMD_RET_USAGE;

	guid = efi_guid_capsule_report;
	if (argc == 1) {
		size = sizeof(var_name16);
		ret = efi_get_variable_int(u"CapsuleLast", &guid, NULL,
					   &size, var_name16, NULL);

		if (ret != EFI_SUCCESS) {
			if (ret == EFI_NOT_FOUND)
				printf("CapsuleLast doesn't exist\n");
			else
				printf("Failed to get CapsuleLast\n");

			return CMD_RET_FAILURE;
		}
		printf("CapsuleLast is %ls\n", var_name16);
	} else {
		argc--;
		argv++;

		capsule_id = hextoul(argv[0], &endp);
		if (capsule_id < 0 || capsule_id > 0xffff)
			return CMD_RET_USAGE;

		efi_create_indexed_name(var_name16, sizeof(var_name16),
					"Capsule", capsule_id);
	}

	size = 0;
	ret = efi_get_variable_int(var_name16, &guid, NULL, &size, NULL, NULL);
	if (ret == EFI_BUFFER_TOO_SMALL) {
		result = malloc(size);
		if (!result)
			return CMD_RET_FAILURE;
		ret = efi_get_variable_int(var_name16, &guid, NULL, &size,
					   result, NULL);
	}
	if (ret != EFI_SUCCESS) {
		free(result);
		printf("Failed to get %ls\n", var_name16);

		return CMD_RET_FAILURE;
	}

	printf("Result total size: 0x%x\n", result->variable_total_size);
	printf("Capsule guid: %pUl\n", &result->capsule_guid);
	printf("Time processed: %04d-%02d-%02d %02d:%02d:%02d\n",
	       result->capsule_processed.year, result->capsule_processed.month,
	       result->capsule_processed.day, result->capsule_processed.hour,
	       result->capsule_processed.minute,
	       result->capsule_processed.second);
	printf("Capsule status: 0x%lx\n", result->capsule_status);

	free(result);

	return CMD_RET_SUCCESS;
}

static struct cmd_tbl cmd_efidebug_capsule_sub[] = {
	U_BOOT_CMD_MKENT(update, CONFIG_SYS_MAXARGS, 1, do_efi_capsule_update,
			 "", ""),
	U_BOOT_CMD_MKENT(show, CONFIG_SYS_MAXARGS, 1, do_efi_capsule_show,
			 "", ""),
#ifdef CONFIG_EFI_ESRT
	U_BOOT_CMD_MKENT(esrt, CONFIG_SYS_MAXARGS, 1, do_efi_capsule_esrt,
			 "", ""),
#endif
#ifdef CONFIG_EFI_CAPSULE_ON_DISK
	U_BOOT_CMD_MKENT(disk-update, 0, 0, do_efi_capsule_on_disk_update,
			 "", ""),
#endif
	U_BOOT_CMD_MKENT(result, CONFIG_SYS_MAXARGS, 1, do_efi_capsule_res,
			 "", ""),
};

/**
 * do_efi_capsule() - manage UEFI capsules
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success,
 *		CMD_RET_USAGE or CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "capsule" sub-command.
 */
static int do_efi_capsule(struct cmd_tbl *cmdtp, int flag,
			  int argc, char * const argv[])
{
	struct cmd_tbl *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--; argv++;

	cp = find_cmd_tbl(argv[0], cmd_efidebug_capsule_sub,
			  ARRAY_SIZE(cmd_efidebug_capsule_sub));
	if (!cp)
		return CMD_RET_USAGE;

	return cp->cmd(cmdtp, flag, argc, argv);
}
#endif /* CONFIG_EFI_HAVE_CAPSULE_SUPPORT */

#define EFI_HANDLE_WIDTH ((int)sizeof(efi_handle_t) * 2)

static const char spc[] = "                ";
static const char sep[] = "================";

/**
 * efi_get_driver_handle_info() - get information of UEFI driver
 *
 * @handle:		Handle of UEFI device
 * @driver_name:	Driver name
 * @image_path:		Pointer to text of device path
 * Return:		0 on success, -1 on failure
 *
 * Currently return no useful information as all UEFI drivers are
 * built-in..
 */
static int efi_get_driver_handle_info(efi_handle_t handle, u16 **driver_name,
				      u16 **image_path)
{
	struct efi_handler *handler;
	struct efi_loaded_image *image;
	efi_status_t ret;

	/*
	 * driver name
	 * TODO: support EFI_COMPONENT_NAME2_PROTOCOL
	 */
	*driver_name = NULL;

	/* image name */
	ret = efi_search_protocol(handle, &efi_guid_loaded_image, &handler);
	if (ret != EFI_SUCCESS) {
		*image_path = NULL;
		return 0;
	}

	image = handler->protocol_interface;
	*image_path = efi_dp_str(image->file_path);

	return 0;
}

/**
 * do_efi_show_drivers() - show UEFI drivers
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "drivers" sub-command.
 * Show all UEFI drivers and their information.
 */
static int do_efi_show_drivers(struct cmd_tbl *cmdtp, int flag,
			       int argc, char *const argv[])
{
	efi_handle_t *handles;
	efi_uintn_t num, i;
	u16 *driver_name, *image_path_text;
	efi_status_t ret;

	ret = EFI_CALL(efi_locate_handle_buffer(
				BY_PROTOCOL, &efi_guid_driver_binding_protocol,
				NULL, &num, &handles));
	if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	if (!num)
		return CMD_RET_SUCCESS;

	printf("Driver%.*s Name                 Image Path\n",
	       EFI_HANDLE_WIDTH - 6, spc);
	printf("%.*s ==================== ====================\n",
	       EFI_HANDLE_WIDTH, sep);
	for (i = 0; i < num; i++) {
		if (!efi_get_driver_handle_info(handles[i], &driver_name,
						&image_path_text)) {
			if (image_path_text)
				printf("%p %-20ls %ls\n", handles[i],
				       driver_name, image_path_text);
			else
				printf("%p %-20ls <built-in>\n",
				       handles[i], driver_name);
			efi_free_pool(driver_name);
			efi_free_pool(image_path_text);
		}
	}

	efi_free_pool(handles);

	return CMD_RET_SUCCESS;
}

/**
 * do_efi_show_handles() - show UEFI handles
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "dh" sub-command.
 * Show all UEFI handles and their information, currently all protocols
 * added to handle.
 */
static int do_efi_show_handles(struct cmd_tbl *cmdtp, int flag,
			       int argc, char *const argv[])
{
	efi_handle_t *handles;
	efi_guid_t **guid;
	efi_uintn_t num, count, i, j;
	efi_status_t ret;

	ret = EFI_CALL(efi_locate_handle_buffer(ALL_HANDLES, NULL, NULL,
						&num, &handles));
	if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	if (!num)
		return CMD_RET_SUCCESS;

	for (i = 0; i < num; i++) {
		struct efi_handler *handler;

		printf("\n%p", handles[i]);
		if (handles[i]->dev)
			printf(" (%s)", handles[i]->dev->name);
		printf("\n");
		/* Print device path */
		ret = efi_search_protocol(handles[i], &efi_guid_device_path,
					  &handler);
		if (ret == EFI_SUCCESS)
			printf("  %pD\n", handler->protocol_interface);
		ret = EFI_CALL(BS->protocols_per_handle(handles[i], &guid,
							&count));
		/* Print other protocols */
		for (j = 0; j < count; j++) {
			if (guidcmp(guid[j], &efi_guid_device_path))
				printf("  %pUs\n", guid[j]);
		}
		efi_free_pool(guid);
	}

	efi_free_pool(handles);

	return CMD_RET_SUCCESS;
}

/**
 * do_efi_show_images() - show UEFI images
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "images" sub-command.
 * Show all UEFI loaded images and their information.
 */
static int do_efi_show_images(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	efi_print_image_infos(NULL);

	return CMD_RET_SUCCESS;
}

static const char * const efi_mem_type_string[] = {
	[EFI_RESERVED_MEMORY_TYPE] = "RESERVED",
	[EFI_LOADER_CODE] = "LOADER CODE",
	[EFI_LOADER_DATA] = "LOADER DATA",
	[EFI_BOOT_SERVICES_CODE] = "BOOT CODE",
	[EFI_BOOT_SERVICES_DATA] = "BOOT DATA",
	[EFI_RUNTIME_SERVICES_CODE] = "RUNTIME CODE",
	[EFI_RUNTIME_SERVICES_DATA] = "RUNTIME DATA",
	[EFI_CONVENTIONAL_MEMORY] = "CONVENTIONAL",
	[EFI_UNUSABLE_MEMORY] = "UNUSABLE MEM",
	[EFI_ACPI_RECLAIM_MEMORY] = "ACPI RECLAIM MEM",
	[EFI_ACPI_MEMORY_NVS] = "ACPI NVS",
	[EFI_MMAP_IO] = "IO",
	[EFI_MMAP_IO_PORT] = "IO PORT",
	[EFI_PAL_CODE] = "PAL",
	[EFI_PERSISTENT_MEMORY_TYPE] = "PERSISTENT",
};

static const struct efi_mem_attrs {
	const u64 bit;
	const char *text;
} efi_mem_attrs[] = {
	{EFI_MEMORY_UC, "UC"},
	{EFI_MEMORY_UC, "UC"},
	{EFI_MEMORY_WC, "WC"},
	{EFI_MEMORY_WT, "WT"},
	{EFI_MEMORY_WB, "WB"},
	{EFI_MEMORY_UCE, "UCE"},
	{EFI_MEMORY_WP, "WP"},
	{EFI_MEMORY_RP, "RP"},
	{EFI_MEMORY_XP, "WP"},
	{EFI_MEMORY_NV, "NV"},
	{EFI_MEMORY_MORE_RELIABLE, "REL"},
	{EFI_MEMORY_RO, "RO"},
	{EFI_MEMORY_SP, "SP"},
	{EFI_MEMORY_RUNTIME, "RT"},
};

/**
 * print_memory_attributes() - print memory map attributes
 *
 * @attributes:	Attribute value
 *
 * Print memory map attributes
 */
static void print_memory_attributes(u64 attributes)
{
	int sep, i;

	for (sep = 0, i = 0; i < ARRAY_SIZE(efi_mem_attrs); i++)
		if (attributes & efi_mem_attrs[i].bit) {
			if (sep) {
				putc('|');
			} else {
				putc(' ');
				sep = 1;
			}
			puts(efi_mem_attrs[i].text);
		}
}

#define EFI_PHYS_ADDR_WIDTH (int)(sizeof(efi_physical_addr_t) * 2)

/**
 * do_efi_show_memmap() - show UEFI memory map
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "memmap" sub-command.
 * Show UEFI memory map.
 */
static int do_efi_show_memmap(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	struct efi_mem_desc *memmap, *map;
	efi_uintn_t map_size;
	const char *type;
	int i;
	efi_status_t ret;

	ret = efi_get_memory_map_alloc(&map_size, &memmap);
	if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	printf("Type             Start%.*s End%.*s Attributes\n",
	       EFI_PHYS_ADDR_WIDTH - 5, spc, EFI_PHYS_ADDR_WIDTH - 3, spc);
	printf("================ %.*s %.*s ==========\n",
	       EFI_PHYS_ADDR_WIDTH, sep, EFI_PHYS_ADDR_WIDTH, sep);
	/*
	 * Coverity check: dereferencing null pointer "map."
	 * This is a false positive as memmap will always be
	 * populated by allocate_pool() above.
	 */
	for (i = 0, map = memmap; i < map_size / sizeof(*map); map++, i++) {
		if (map->type < ARRAY_SIZE(efi_mem_type_string))
			type = efi_mem_type_string[map->type];
		else
			type = "(unknown)";

		printf("%-16s %.*llx-%.*llx", type,
		       EFI_PHYS_ADDR_WIDTH,
		       (u64)map_to_sysmem((void *)(uintptr_t)
					  map->physical_start),
		       EFI_PHYS_ADDR_WIDTH,
		       (u64)map_to_sysmem((void *)(uintptr_t)
					  (map->physical_start +
					   map->num_pages * EFI_PAGE_SIZE)));

		print_memory_attributes(map->attribute);
		putc('\n');
	}

	efi_free_pool(memmap);

	return CMD_RET_SUCCESS;
}

/**
 * do_efi_show_tables() - show UEFI configuration tables
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "tables" sub-command.
 * Show UEFI configuration tables.
 */
static int do_efi_show_tables(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	efi_show_tables(&systab);

	return CMD_RET_SUCCESS;
}

/**
 * create_initrd_dp() - create a special device for our Boot### option
 *
 * @dev:	device
 * @part:	disk partition
 * @file:	filename
 * @shortform:	create short form device path
 * Return:	pointer to the device path or ERR_PTR
 */
static
struct efi_device_path *create_initrd_dp(const char *dev, const char *part,
					 const char *file, int shortform)

{
	struct efi_device_path *tmp_dp = NULL, *tmp_fp = NULL, *short_fp = NULL;
	struct efi_device_path *initrd_dp = NULL;
	efi_status_t ret;
	const struct efi_initrd_dp id_dp = {
		.vendor = {
			{
			DEVICE_PATH_TYPE_MEDIA_DEVICE,
			DEVICE_PATH_SUB_TYPE_VENDOR_PATH,
			sizeof(id_dp.vendor),
			},
			EFI_INITRD_MEDIA_GUID,
		},
		.end = {
			DEVICE_PATH_TYPE_END,
			DEVICE_PATH_SUB_TYPE_END,
			sizeof(id_dp.end),
		}
	};

	ret = efi_dp_from_name(dev, part, file, &tmp_dp, &tmp_fp);
	if (ret != EFI_SUCCESS) {
		printf("Cannot create device path for \"%s %s\"\n", part, file);
		goto out;
	}
	if (shortform)
		short_fp = efi_dp_shorten(tmp_fp);
	if (!short_fp)
		short_fp = tmp_fp;

	initrd_dp = efi_dp_concat((const struct efi_device_path *)&id_dp,
				  short_fp, false);

out:
	efi_free_pool(tmp_dp);
	efi_free_pool(tmp_fp);
	return initrd_dp;
}

/**
 * efi_boot_add_uri() - set URI load option
 *
 * @argc:		Number of arguments
 * @argv:		Argument array
 * @var_name16:		variable name buffer
 * @var_name16_size:	variable name buffer size
 * @lo:			pointer to the load option
 * @file_path:		buffer to set the generated device path pointer
 * @fp_size:		file_path size
 * Return:		CMD_RET_SUCCESS on success,
 *			CMD_RET_USAGE or CMD_RET_RET_FAILURE on failure
 */
static int efi_boot_add_uri(int argc, char *const argv[], u16 *var_name16,
			    size_t var_name16_size, struct efi_load_option *lo,
			    struct efi_device_path **file_path,
			    efi_uintn_t *fp_size)
{
	int id;
	char *pos;
	char *endp;
	u16 *label;
	efi_uintn_t uridp_len;
	struct efi_device_path_uri *uridp;

	if (argc < 3 || lo->label)
		return CMD_RET_USAGE;

	id = (int)hextoul(argv[1], &endp);
	if (*endp != '\0' || id > 0xffff)
		return CMD_RET_USAGE;

	label = efi_convert_string(argv[2]);
	if (!label)
		return CMD_RET_FAILURE;

	if (!wget_validate_uri(argv[3])) {
		printf("ERROR: invalid URI\n");
		return CMD_RET_FAILURE;
	}

	efi_create_indexed_name(var_name16, var_name16_size, "Boot", id);
	lo->label = label;

	uridp_len = sizeof(struct efi_device_path) + strlen(argv[3]) + 1;
	uridp = efi_alloc(uridp_len + sizeof(END));
	if (!uridp) {
		log_err("Out of memory\n");
		return CMD_RET_FAILURE;
	}
	uridp->dp.type = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
	uridp->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_URI;
	uridp->dp.length = uridp_len;
	strcpy(uridp->uri, argv[3]);
	pos = (char *)uridp + uridp_len;
	memcpy(pos, &END, sizeof(END));

	*file_path = &uridp->dp;
	*fp_size += uridp_len + sizeof(END);

	return CMD_RET_SUCCESS;
}

/**
 * do_efi_boot_add() - set UEFI load option
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success,
 *		CMD_RET_USAGE or CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "boot add" sub-command. Create or change UEFI load option.
 *
 * efidebug boot add -b <id> <label> <interface> <devnum>[:<part>] <file>
 *                   -i <file> <interface2> <devnum2>[:<part>] <initrd>
 *                   -s '<options>'
 */
static int do_efi_boot_add(struct cmd_tbl *cmdtp, int flag,
			   int argc, char *const argv[])
{
	int id;
	char *endp;
	u16 var_name16[9];
	efi_guid_t guid;
	u16 *label;
	struct efi_device_path *file_path = NULL;
	struct efi_device_path *fp_free = NULL;
	struct efi_device_path *final_fp = NULL;
	struct efi_device_path *initrd_dp = NULL;
	struct efi_load_option lo;
	void *data = NULL;
	efi_uintn_t size;
	efi_uintn_t fp_size = 0;
	efi_status_t ret;
	int r = CMD_RET_SUCCESS;

	guid = efi_global_variable_guid;

	/* attributes */
	lo.attributes = LOAD_OPTION_ACTIVE; /* always ACTIVE */
	lo.optional_data = NULL;
	lo.label = NULL;

	argc--;
	argv++; /* 'add' */
	for (; argc > 0; argc--, argv++) {
		int shortform;

		if (*argv[0] != '-' || strlen(argv[0]) != 2) {
				r = CMD_RET_USAGE;
				goto out;
		}
		shortform = 0;
		switch (argv[0][1]) {
		case 'b':
			shortform = 1;
			/* fallthrough */
		case 'B':
			if (argc <  5 || lo.label) {
				r = CMD_RET_USAGE;
				goto out;
			}
			id = (int)hextoul(argv[1], &endp);
			if (*endp != '\0' || id > 0xffff)
				return CMD_RET_USAGE;

			efi_create_indexed_name(var_name16, sizeof(var_name16),
						"Boot", id);

			/* label */
			label = efi_convert_string(argv[2]);
			if (!label)
				return CMD_RET_FAILURE;
			lo.label = label; /* label will be changed below */

			/* file path */
			ret = efi_dp_from_name(argv[3], argv[4], argv[5],
					       NULL, &fp_free);
			if (ret != EFI_SUCCESS) {
				printf("Cannot create device path for \"%s %s\"\n",
				       argv[3], argv[4]);
				r = CMD_RET_FAILURE;
				goto out;
			}
			if (shortform)
				file_path = efi_dp_shorten(fp_free);
			if (!file_path)
				file_path = fp_free;
			fp_size += efi_dp_size(file_path) +
				sizeof(struct efi_device_path);
			argc -= 5;
			argv += 5;
			break;
		case 'i':
			shortform = 1;
			/* fallthrough */
		case 'I':
			if (argc < 3 || initrd_dp) {
				r = CMD_RET_USAGE;
				goto out;
			}

			initrd_dp = create_initrd_dp(argv[1], argv[2], argv[3],
						     shortform);
			if (!initrd_dp) {
				printf("Cannot add an initrd\n");
				r = CMD_RET_FAILURE;
				goto out;
			}
			argc -= 3;
			argv += 3;
			fp_size += efi_dp_size(initrd_dp) +
				sizeof(struct efi_device_path);
			break;
		case 's':
			if (argc < 1 || lo.optional_data) {
				r = CMD_RET_USAGE;
				goto out;
			}
			lo.optional_data = (const u8 *)argv[1];
			argc -= 1;
			argv += 1;
			break;
		case 'u':
			if (IS_ENABLED(CONFIG_EFI_HTTP_BOOT)) {
				r = efi_boot_add_uri(argc, argv, var_name16,
						     sizeof(var_name16), &lo,
						     &file_path, &fp_size);
				if (r != CMD_RET_SUCCESS)
					goto out;
				fp_free = file_path;
				argc -= 3;
				argv += 3;
			} else{
				r = CMD_RET_USAGE;
				goto out;
			}
			break;
		default:
			r = CMD_RET_USAGE;
			goto out;
		}
	}

	if (!file_path) {
		printf("Missing binary\n");
		r = CMD_RET_USAGE;
		goto out;
	}

	final_fp = efi_dp_concat(file_path, initrd_dp, true);
	if (!final_fp) {
		printf("Cannot create final device path\n");
		r = CMD_RET_FAILURE;
		goto out;
	}

	lo.file_path = final_fp;
	lo.file_path_length = fp_size;

	size = efi_serialize_load_option(&lo, (u8 **)&data);
	if (!size) {
		r = CMD_RET_FAILURE;
		goto out;
	}

	ret = efi_set_variable_int(var_name16, &guid,
				   EFI_VARIABLE_NON_VOLATILE |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   size, data, false);
	if (ret != EFI_SUCCESS) {
		printf("Cannot set %ls\n", var_name16);
		r = CMD_RET_FAILURE;
	}

out:
	free(data);
	efi_free_pool(final_fp);
	efi_free_pool(initrd_dp);
	efi_free_pool(fp_free);
	free(lo.label);

	return r;
}

/**
 * do_efi_boot_rm() - delete UEFI load options
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "boot rm" sub-command.
 * Delete UEFI load options.
 *
 *     efidebug boot rm <id> ...
 */
static int do_efi_boot_rm(struct cmd_tbl *cmdtp, int flag,
			  int argc, char *const argv[])
{
	efi_guid_t guid;
	int id, i;
	char *endp;
	u16 var_name16[9];
	efi_status_t ret;

	if (argc == 1)
		return CMD_RET_USAGE;

	guid = efi_global_variable_guid;
	for (i = 1; i < argc; i++, argv++) {
		id = (int)hextoul(argv[1], &endp);
		if (*endp != '\0' || id > 0xffff)
			return CMD_RET_FAILURE;

		efi_create_indexed_name(var_name16, sizeof(var_name16),
					"Boot", id);
		ret = efi_set_variable_int(var_name16, &guid, 0, 0, NULL,
					   false);
		if (ret) {
			printf("Cannot remove %ls\n", var_name16);
			return CMD_RET_FAILURE;
		}
	}

	return CMD_RET_SUCCESS;
}

/**
 * show_efi_boot_opt_data() - dump UEFI load option
 *
 * @varname16:	variable name
 * @data:	value of UEFI load option variable
 * @size:	size of the boot option
 *
 * Decode the value of UEFI load option variable and print information.
 */
static void show_efi_boot_opt_data(u16 *varname16, void *data, size_t *size)
{
	struct efi_device_path *initrd_path = NULL;
	struct efi_load_option lo;
	efi_status_t ret;

	ret = efi_deserialize_load_option(&lo, data, size);
	if (ret != EFI_SUCCESS) {
		printf("%ls: invalid load option\n", varname16);
		return;
	}

	printf("%ls:\nattributes: %c%c%c (0x%08x)\n",
	       varname16,
	       /* ACTIVE */
	       lo.attributes & LOAD_OPTION_ACTIVE ? 'A' : '-',
	       /* FORCE RECONNECT */
	       lo.attributes & LOAD_OPTION_FORCE_RECONNECT ? 'R' : '-',
	       /* HIDDEN */
	       lo.attributes & LOAD_OPTION_HIDDEN ? 'H' : '-',
	       lo.attributes);
	printf("  label: %ls\n", lo.label);

	printf("  file_path: %pD\n", lo.file_path);

	initrd_path = efi_dp_from_lo(&lo, &efi_lf2_initrd_guid);
	if (initrd_path) {
		printf("  initrd_path: %pD\n", initrd_path);
		efi_free_pool(initrd_path);
	}

	printf("  data:\n");
	print_hex_dump("    ", DUMP_PREFIX_OFFSET, 16, 1,
		       lo.optional_data, *size, true);
}

/**
 * show_efi_boot_opt() - dump UEFI load option
 *
 * @varname16:	variable name
 *
 * Dump information defined by UEFI load option.
 */
static void show_efi_boot_opt(u16 *varname16)
{
	void *data;
	efi_uintn_t size;
	efi_status_t ret;

	size = 0;
	ret = efi_get_variable_int(varname16, &efi_global_variable_guid,
				   NULL, &size, NULL, NULL);
	if (ret == EFI_BUFFER_TOO_SMALL) {
		data = malloc(size);
		if (!data) {
			printf("ERROR: Out of memory\n");
			return;
		}
		ret = efi_get_variable_int(varname16, &efi_global_variable_guid,
					   NULL, &size, data, NULL);
		if (ret == EFI_SUCCESS)
			show_efi_boot_opt_data(varname16, data, &size);
		free(data);
	}
}

/**
 * show_efi_boot_dump() - dump all UEFI load options
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "boot dump" sub-command.
 * Dump information of all UEFI load options defined.
 *
 *     efidebug boot dump
 */
static int do_efi_boot_dump(struct cmd_tbl *cmdtp, int flag,
			    int argc, char *const argv[])
{
	u16 *var_name16, *p;
	efi_uintn_t buf_size, size;
	efi_guid_t guid;
	efi_status_t ret;

	if (argc > 1)
		return CMD_RET_USAGE;

	buf_size = 128;
	var_name16 = malloc(buf_size);
	if (!var_name16)
		return CMD_RET_FAILURE;

	var_name16[0] = 0;
	for (;;) {
		size = buf_size;
		ret = efi_get_next_variable_name_int(&size, var_name16, &guid);
		if (ret == EFI_NOT_FOUND)
			break;
		if (ret == EFI_BUFFER_TOO_SMALL) {
			buf_size = size;
			p = realloc(var_name16, buf_size);
			if (!p) {
				free(var_name16);
				return CMD_RET_FAILURE;
			}
			var_name16 = p;
			ret = efi_get_next_variable_name_int(&size, var_name16,
							     &guid);
		}
		if (ret != EFI_SUCCESS) {
			free(var_name16);
			return CMD_RET_FAILURE;
		}

		if (efi_varname_is_load_option(var_name16, NULL))
			show_efi_boot_opt(var_name16);
	}

	free(var_name16);

	return CMD_RET_SUCCESS;
}

/**
 * show_efi_boot_order() - show order of UEFI load options
 *
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 *
 * Show order of UEFI load options defined by BootOrder variable.
 */
static int show_efi_boot_order(void)
{
	u16 *bootorder;
	efi_uintn_t size;
	int num, i;
	u16 var_name16[9];
	void *data;
	struct efi_load_option lo;
	efi_status_t ret;

	size = 0;
	ret = efi_get_variable_int(u"BootOrder", &efi_global_variable_guid,
				   NULL, &size, NULL, NULL);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		if (ret == EFI_NOT_FOUND) {
			printf("BootOrder not defined\n");
			return CMD_RET_SUCCESS;
		} else {
			return CMD_RET_FAILURE;
		}
	}
	bootorder = malloc(size);
	if (!bootorder) {
		printf("ERROR: Out of memory\n");
		return CMD_RET_FAILURE;
	}
	ret = efi_get_variable_int(u"BootOrder", &efi_global_variable_guid,
				   NULL, &size, bootorder, NULL);
	if (ret != EFI_SUCCESS) {
		ret = CMD_RET_FAILURE;
		goto out;
	}

	num = size / sizeof(u16);
	for (i = 0; i < num; i++) {
		efi_create_indexed_name(var_name16, sizeof(var_name16),
					"Boot", bootorder[i]);

		size = 0;
		ret = efi_get_variable_int(var_name16,
					   &efi_global_variable_guid, NULL,
					   &size, NULL, NULL);
		if (ret != EFI_BUFFER_TOO_SMALL) {
			printf("%2d: %ls: (not defined)\n", i + 1, var_name16);
			continue;
		}

		data = malloc(size);
		if (!data) {
			ret = CMD_RET_FAILURE;
			goto out;
		}
		ret = efi_get_variable_int(var_name16,
					   &efi_global_variable_guid, NULL,
					   &size, data, NULL);
		if (ret != EFI_SUCCESS) {
			free(data);
			ret = CMD_RET_FAILURE;
			goto out;
		}

		ret = efi_deserialize_load_option(&lo, data, &size);
		if (ret != EFI_SUCCESS) {
			printf("%ls: invalid load option\n", var_name16);
			ret = CMD_RET_FAILURE;
			goto out;
		}

		printf("%2d: %ls: %ls\n", i + 1, var_name16, lo.label);

		free(data);
	}
out:
	free(bootorder);

	return ret;
}

/**
 * do_efi_boot_next() - manage UEFI BootNext variable
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success,
 *		CMD_RET_USAGE or CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "boot next" sub-command.
 * Set BootNext variable.
 *
 *     efidebug boot next <id>
 */
static int do_efi_boot_next(struct cmd_tbl *cmdtp, int flag,
			    int argc, char *const argv[])
{
	u16 bootnext;
	efi_uintn_t size;
	char *endp;
	efi_guid_t guid;
	efi_status_t ret;
	int r = CMD_RET_SUCCESS;

	if (argc != 2)
		return CMD_RET_USAGE;

	bootnext = (u16)hextoul(argv[1], &endp);
	if (*endp) {
		printf("invalid value: %s\n", argv[1]);
		r = CMD_RET_FAILURE;
		goto out;
	}

	guid = efi_global_variable_guid;
	size = sizeof(u16);
	ret = efi_set_variable_int(u"BootNext", &guid,
				   EFI_VARIABLE_NON_VOLATILE |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   size, &bootnext, false);
	if (ret != EFI_SUCCESS) {
		printf("Cannot set BootNext\n");
		r = CMD_RET_FAILURE;
	}
out:
	return r;
}

/**
 * do_efi_boot_order() - manage UEFI BootOrder variable
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "boot order" sub-command.
 * Show order of UEFI load options, or change it in BootOrder variable.
 *
 *     efidebug boot order [<id> ...]
 */
static int do_efi_boot_order(struct cmd_tbl *cmdtp, int flag,
			     int argc, char *const argv[])
{
	u16 *bootorder = NULL;
	efi_uintn_t size;
	int id, i;
	char *endp;
	efi_guid_t guid;
	efi_status_t ret;
	int r = CMD_RET_SUCCESS;

	if (argc == 1)
		return show_efi_boot_order();

	argc--;
	argv++;

	size = argc * sizeof(u16);
	bootorder = malloc(size);
	if (!bootorder)
		return CMD_RET_FAILURE;

	for (i = 0; i < argc; i++) {
		id = (int)hextoul(argv[i], &endp);
		if (*endp != '\0' || id > 0xffff) {
			printf("invalid value: %s\n", argv[i]);
			r = CMD_RET_FAILURE;
			goto out;
		}

		bootorder[i] = (u16)id;
	}

	guid = efi_global_variable_guid;
	ret = efi_set_variable_int(u"BootOrder", &guid,
				   EFI_VARIABLE_NON_VOLATILE |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   size, bootorder, true);
	if (ret != EFI_SUCCESS) {
		printf("Cannot set BootOrder\n");
		r = CMD_RET_FAILURE;
	}
out:
	free(bootorder);

	return r;
}

static struct cmd_tbl cmd_efidebug_boot_sub[] = {
	U_BOOT_CMD_MKENT(add, CONFIG_SYS_MAXARGS, 1, do_efi_boot_add, "", ""),
	U_BOOT_CMD_MKENT(rm, CONFIG_SYS_MAXARGS, 1, do_efi_boot_rm, "", ""),
	U_BOOT_CMD_MKENT(dump, CONFIG_SYS_MAXARGS, 1, do_efi_boot_dump, "", ""),
	U_BOOT_CMD_MKENT(next, CONFIG_SYS_MAXARGS, 1, do_efi_boot_next, "", ""),
	U_BOOT_CMD_MKENT(order, CONFIG_SYS_MAXARGS, 1, do_efi_boot_order,
			 "", ""),
};

/**
 * do_efi_boot_opt() - manage UEFI load options
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success,
 *		CMD_RET_USAGE or CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "boot" sub-command.
 */
static int do_efi_boot_opt(struct cmd_tbl *cmdtp, int flag,
			   int argc, char *const argv[])
{
	struct cmd_tbl *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--; argv++;

	cp = find_cmd_tbl(argv[0], cmd_efidebug_boot_sub,
			  ARRAY_SIZE(cmd_efidebug_boot_sub));
	if (!cp)
		return CMD_RET_USAGE;

	return cp->cmd(cmdtp, flag, argc, argv);
}

/**
 * do_efi_test_bootmgr() - run simple bootmgr for test
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success,
 *		CMD_RET_USAGE or CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "test bootmgr" sub-command.
 * Run simple bootmgr for test.
 *
 *     efidebug test bootmgr
 */
static __maybe_unused int do_efi_test_bootmgr(struct cmd_tbl *cmdtp, int flag,
					      int argc, char * const argv[])
{
	efi_handle_t image;
	efi_uintn_t exit_data_size = 0;
	u16 *exit_data = NULL;
	efi_status_t ret;
	void *load_options = NULL;

	ret = efi_bootmgr_load(&image, &load_options);
	printf("efi_bootmgr_load() returned: %ld\n", ret & ~EFI_ERROR_MASK);

	/* We call efi_start_image() even if error for test purpose. */
	ret = EFI_CALL(efi_start_image(image, &exit_data_size, &exit_data));
	printf("efi_start_image() returned: %ld\n", ret & ~EFI_ERROR_MASK);
	if (ret && exit_data)
		efi_free_pool(exit_data);

	efi_restore_gd();

	free(load_options);
	return CMD_RET_SUCCESS;
}

static struct cmd_tbl cmd_efidebug_test_sub[] = {
#ifdef CONFIG_EFI_BOOTMGR
	U_BOOT_CMD_MKENT(bootmgr, CONFIG_SYS_MAXARGS, 1, do_efi_test_bootmgr,
			 "", ""),
#endif
};

/**
 * do_efi_test() - manage UEFI load options
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success,
 *		CMD_RET_USAGE or CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug "test" sub-command.
 */
static int do_efi_test(struct cmd_tbl *cmdtp, int flag,
		       int argc, char * const argv[])
{
	struct cmd_tbl *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--; argv++;

	cp = find_cmd_tbl(argv[0], cmd_efidebug_test_sub,
			  ARRAY_SIZE(cmd_efidebug_test_sub));
	if (!cp)
		return CMD_RET_USAGE;

	return cp->cmd(cmdtp, flag, argc, argv);
}

/**
 * do_efi_query_info() - QueryVariableInfo EFI service
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success,
 *		CMD_RET_USAGE or CMD_RET_FAILURE on failure
 *
 * Implement efidebug "test" sub-command.
 */

static int do_efi_query_info(struct cmd_tbl *cmdtp, int flag,
			     int argc, char * const argv[])
{
	efi_status_t ret;
	u32 attr = 0;
	u64 max_variable_storage_size;
	u64 remain_variable_storage_size;
	u64 max_variable_size;
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-bs"))
			attr |= EFI_VARIABLE_BOOTSERVICE_ACCESS;
		else if (!strcmp(argv[i], "-rt"))
			attr |= EFI_VARIABLE_RUNTIME_ACCESS;
		else if (!strcmp(argv[i], "-nv"))
			attr |= EFI_VARIABLE_NON_VOLATILE;
		else if (!strcmp(argv[i], "-at"))
			attr |=
				EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
	}

	ret = efi_query_variable_info_int(attr, &max_variable_storage_size,
					  &remain_variable_storage_size,
					  &max_variable_size);
	if (ret != EFI_SUCCESS) {
		printf("Error: Cannot query UEFI variables, r = %lu\n",
		       ret & ~EFI_ERROR_MASK);
		return CMD_RET_FAILURE;
	}

	printf("Max storage size %llu\n", max_variable_storage_size);
	printf("Remaining storage size %llu\n", remain_variable_storage_size);
	printf("Max variable size %llu\n", max_variable_size);

	return CMD_RET_SUCCESS;
}

static struct cmd_tbl cmd_efidebug_sub[] = {
	U_BOOT_CMD_MKENT(boot, CONFIG_SYS_MAXARGS, 1, do_efi_boot_opt, "", ""),
#ifdef CONFIG_EFI_HAVE_CAPSULE_SUPPORT
	U_BOOT_CMD_MKENT(capsule, CONFIG_SYS_MAXARGS, 1, do_efi_capsule,
			 "", ""),
#endif
	U_BOOT_CMD_MKENT(drivers, CONFIG_SYS_MAXARGS, 1, do_efi_show_drivers,
			 "", ""),
	U_BOOT_CMD_MKENT(dh, CONFIG_SYS_MAXARGS, 1, do_efi_show_handles,
			 "", ""),
	U_BOOT_CMD_MKENT(images, CONFIG_SYS_MAXARGS, 1, do_efi_show_images,
			 "", ""),
	U_BOOT_CMD_MKENT(memmap, CONFIG_SYS_MAXARGS, 1, do_efi_show_memmap,
			 "", ""),
	U_BOOT_CMD_MKENT(tables, CONFIG_SYS_MAXARGS, 1, do_efi_show_tables,
			 "", ""),
	U_BOOT_CMD_MKENT(test, CONFIG_SYS_MAXARGS, 1, do_efi_test,
			 "", ""),
	U_BOOT_CMD_MKENT(query, CONFIG_SYS_MAXARGS, 1, do_efi_query_info,
			 "", ""),
};

/**
 * do_efidebug() - display and configure UEFI environment
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success,
 *		CMD_RET_USAGE or CMD_RET_RET_FAILURE on failure
 *
 * Implement efidebug command which allows us to display and
 * configure UEFI environment.
 */
static int do_efidebug(struct cmd_tbl *cmdtp, int flag,
		       int argc, char *const argv[])
{
	struct cmd_tbl *cp;
	efi_status_t r;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--; argv++;

	/* Initialize UEFI drivers */
	r = efi_init_obj_list();
	if (r != EFI_SUCCESS) {
		printf("Error: Cannot initialize UEFI sub-system, r = %lu\n",
		       r & ~EFI_ERROR_MASK);
		return CMD_RET_FAILURE;
	}

	cp = find_cmd_tbl(argv[0], cmd_efidebug_sub,
			  ARRAY_SIZE(cmd_efidebug_sub));
	if (!cp)
		return CMD_RET_USAGE;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_LONGHELP(efidebug,
	"  - UEFI Shell-like interface to configure UEFI environment\n"
	"\n"
	"efidebug boot add - set UEFI BootXXXX variable\n"
	"  -b|-B <bootid> <label> <interface> <devnum>[:<part>] <file path>\n"
	"  -i|-I <interface> <devnum>[:<part>] <initrd file path>\n"
	"  (-b, -i for short form device path)\n"
#if (IS_ENABLED(CONFIG_EFI_HTTP_BOOT))
	"  -u <bootid> <label> <uri>\n"
#endif
	"  -s '<optional data>'\n"
	"efidebug boot rm <bootid#1> [<bootid#2> [<bootid#3> [...]]]\n"
	"  - delete UEFI BootXXXX variables\n"
	"efidebug boot dump\n"
	"  - dump all UEFI BootXXXX variables\n"
	"efidebug boot next <bootid>\n"
	"  - set UEFI BootNext variable\n"
	"efidebug boot order [<bootid#1> [<bootid#2> [<bootid#3> [...]]]]\n"
	"  - set/show UEFI boot order\n"
	"\n"
#ifdef CONFIG_EFI_HAVE_CAPSULE_SUPPORT
	"efidebug capsule update [-v] <capsule address>\n"
	"  - process a capsule\n"
	"efidebug capsule disk-update\n"
	"  - update a capsule from disk\n"
	"efidebug capsule show <capsule address>\n"
	"  - show capsule information\n"
	"efidebug capsule result [<capsule result var>]\n"
	"  - show a capsule update result\n"
#ifdef CONFIG_EFI_ESRT
	"efidebug capsule esrt\n"
	"  - print the ESRT\n"
#endif
	"\n"
#endif
	"efidebug drivers\n"
	"  - show UEFI drivers\n"
	"efidebug dh\n"
	"  - show UEFI handles\n"
	"efidebug images\n"
	"  - show loaded images\n"
	"efidebug memmap\n"
	"  - show UEFI memory map\n"
	"efidebug tables\n"
	"  - show UEFI configuration tables\n"
#ifdef CONFIG_EFI_BOOTMGR
	"efidebug test bootmgr\n"
	"  - run simple bootmgr for test\n"
#endif
	"efidebug query [-nv][-bs][-rt][-at]\n"
	"  - show size of UEFI variables store\n");

U_BOOT_CMD(
	efidebug, CONFIG_SYS_MAXARGS, 0, do_efidebug,
	"Configure UEFI environment",
	efidebug_help_text
);
