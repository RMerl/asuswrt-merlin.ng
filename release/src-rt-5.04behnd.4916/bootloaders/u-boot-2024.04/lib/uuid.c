// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011 Calxeda, Inc.
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#define LOG_CATEGOT LOGC_CORE

#include <command.h>
#include <efi_api.h>
#include <env.h>
#include <rand.h>
#include <time.h>
#include <uuid.h>
#include <linux/ctype.h>
#include <errno.h>
#include <asm/io.h>
#include <part_efi.h>
#include <malloc.h>
#include <dm/uclass.h>
#include <rng.h>

int uuid_str_valid(const char *uuid)
{
	int i, valid;

	if (uuid == NULL)
		return 0;

	for (i = 0, valid = 1; uuid[i] && valid; i++) {
		switch (i) {
		case 8: case 13: case 18: case 23:
			valid = (uuid[i] == '-');
			break;
		default:
			valid = isxdigit(uuid[i]);
			break;
		}
	}

	if (i != UUID_STR_LEN || !valid)
		return 0;

	return 1;
}

static const struct {
	const char *string;
	efi_guid_t guid;
} list_guid[] = {
#ifdef CONFIG_PARTITION_TYPE_GUID
	{"system",	PARTITION_SYSTEM_GUID},
	{"mbr",		LEGACY_MBR_PARTITION_GUID},
	{"msft",	PARTITION_MSFT_RESERVED_GUID},
	{"data",	PARTITION_BASIC_DATA_GUID},
	{"linux",	PARTITION_LINUX_FILE_SYSTEM_DATA_GUID},
	{"raid",	PARTITION_LINUX_RAID_GUID},
	{"swap",	PARTITION_LINUX_SWAP_GUID},
	{"lvm",		PARTITION_LINUX_LVM_GUID},
	{"u-boot-env",	PARTITION_U_BOOT_ENVIRONMENT},
	{"cros-kern",	PARTITION_CROS_KERNEL},
	{"cros-root",	PARTITION_CROS_ROOT},
	{"cros-fw",	PARTITION_CROS_FIRMWARE},
	{"cros-rsrv",	PARTITION_CROS_RESERVED},
#endif
#if defined(CONFIG_CMD_EFIDEBUG) || defined(CONFIG_EFI)
	{
		"Device Path",
		EFI_DEVICE_PATH_PROTOCOL_GUID,
	},
	{
		"Device Path To Text",
		EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID,
	},
	{
		"Device Path Utilities",
		EFI_DEVICE_PATH_UTILITIES_PROTOCOL_GUID,
	},
	{
		"Unicode Collation 2",
		EFI_UNICODE_COLLATION_PROTOCOL2_GUID,
	},
	{
		"Driver Binding",
		EFI_DRIVER_BINDING_PROTOCOL_GUID,
	},
	{
		"Simple Text Input",
		EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID,
	},
	{
		"Simple Text Input Ex",
		EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID,
	},
	{
		"Simple Text Output",
		EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID,
	},
	{
		"Block IO",
		EFI_BLOCK_IO_PROTOCOL_GUID,
	},
	{
		"Simple File System",
		EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
	},
	{
		"Loaded Image",
		EFI_LOADED_IMAGE_PROTOCOL_GUID,
	},
	{
		"Graphics Output",
		EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
	},
	{
		"HII String",
		EFI_HII_STRING_PROTOCOL_GUID,
	},
	{
		"HII Database",
		EFI_HII_DATABASE_PROTOCOL_GUID,
	},
	{
		"HII Config Routing",
		EFI_HII_CONFIG_ROUTING_PROTOCOL_GUID,
	},
	{
		"Load File2",
		EFI_LOAD_FILE2_PROTOCOL_GUID,
	},
	{
		"Random Number Generator",
		EFI_RNG_PROTOCOL_GUID,
	},
	{
		"Simple Network",
		EFI_SIMPLE_NETWORK_PROTOCOL_GUID,
	},
	{
		"PXE Base Code",
		EFI_PXE_BASE_CODE_PROTOCOL_GUID,
	},
	{
		"Device-Tree Fixup",
		EFI_DT_FIXUP_PROTOCOL_GUID,
	},
	{
		"TCG2",
		EFI_TCG2_PROTOCOL_GUID,
		},
	{
		"System Partition",
		PARTITION_SYSTEM_GUID
	},
	{
		"Firmware Management",
		EFI_FIRMWARE_MANAGEMENT_PROTOCOL_GUID
	},
	/* Configuration table GUIDs */
	{
		"ACPI table",
		EFI_ACPI_TABLE_GUID,
	},
	{
		"EFI System Resource Table",
		EFI_SYSTEM_RESOURCE_TABLE_GUID,
	},
	{
		"device tree",
		EFI_FDT_GUID,
	},
	{
		"SMBIOS table",
		SMBIOS_TABLE_GUID,
	},
	{
		"SMBIOS3 table",
		SMBIOS3_TABLE_GUID,
	},
	{
		"Runtime properties",
		EFI_RT_PROPERTIES_TABLE_GUID,
	},
	{
		"TCG2 Final Events Table",
		EFI_TCG2_FINAL_EVENTS_TABLE_GUID,
	},
	{
		"EFI Conformance Profiles Table",
		EFI_CONFORMANCE_PROFILES_TABLE_GUID,
	},
#ifdef CONFIG_EFI_RISCV_BOOT_PROTOCOL
	{
		"RISC-V Boot",
		RISCV_EFI_BOOT_PROTOCOL_GUID,
	},
#endif
#endif /* CONFIG_CMD_EFIDEBUG */
#ifdef CONFIG_CMD_NVEDIT_EFI
	/* signature database */
	{
		"EFI_GLOBAL_VARIABLE_GUID",
		EFI_GLOBAL_VARIABLE_GUID,
	},
	{
		"EFI_IMAGE_SECURITY_DATABASE_GUID",
		EFI_IMAGE_SECURITY_DATABASE_GUID,
	},
	/* certificate types */
	{
		"EFI_CERT_SHA256_GUID",
		EFI_CERT_SHA256_GUID,
	},
	{
		"EFI_CERT_X509_GUID",
		EFI_CERT_X509_GUID,
	},
	{
		"EFI_CERT_TYPE_PKCS7_GUID",
		EFI_CERT_TYPE_PKCS7_GUID,
	},
#endif
#if defined(CONFIG_CMD_EFIDEBUG) || defined(CONFIG_EFI)
	{ "EFI_LZMA_COMPRESSED", EFI_LZMA_COMPRESSED },
	{ "EFI_DXE_SERVICES", EFI_DXE_SERVICES },
	{ "EFI_HOB_LIST", EFI_HOB_LIST },
	{ "EFI_MEMORY_TYPE", EFI_MEMORY_TYPE },
	{ "EFI_MEM_STATUS_CODE_REC", EFI_MEM_STATUS_CODE_REC },
	{ "EFI_GUID_EFI_ACPI1", EFI_GUID_EFI_ACPI1 },
#endif
};

int uuid_guid_get_bin(const char *guid_str, unsigned char *guid_bin)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(list_guid); i++) {
		if (!strcmp(list_guid[i].string, guid_str)) {
			memcpy(guid_bin, &list_guid[i].guid, 16);
			return 0;
		}
	}
	return -ENODEV;
}

const char *uuid_guid_get_str(const unsigned char *guid_bin)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(list_guid); i++) {
		if (!memcmp(list_guid[i].guid.b, guid_bin, 16)) {
			return list_guid[i].string;
		}
	}
	return NULL;
}

int uuid_str_to_bin(const char *uuid_str, unsigned char *uuid_bin,
		    int str_format)
{
	uint16_t tmp16;
	uint32_t tmp32;
	uint64_t tmp64;

	if (!uuid_str_valid(uuid_str)) {
		log_debug("not valid\n");
#ifdef CONFIG_PARTITION_TYPE_GUID
		if (!uuid_guid_get_bin(uuid_str, uuid_bin))
			return 0;
#endif
		return -EINVAL;
	}

	if (str_format == UUID_STR_FORMAT_STD) {
		tmp32 = cpu_to_be32(hextoul(uuid_str, NULL));
		memcpy(uuid_bin, &tmp32, 4);

		tmp16 = cpu_to_be16(hextoul(uuid_str + 9, NULL));
		memcpy(uuid_bin + 4, &tmp16, 2);

		tmp16 = cpu_to_be16(hextoul(uuid_str + 14, NULL));
		memcpy(uuid_bin + 6, &tmp16, 2);
	} else {
		tmp32 = cpu_to_le32(hextoul(uuid_str, NULL));
		memcpy(uuid_bin, &tmp32, 4);

		tmp16 = cpu_to_le16(hextoul(uuid_str + 9, NULL));
		memcpy(uuid_bin + 4, &tmp16, 2);

		tmp16 = cpu_to_le16(hextoul(uuid_str + 14, NULL));
		memcpy(uuid_bin + 6, &tmp16, 2);
	}

	tmp16 = cpu_to_be16(hextoul(uuid_str + 19, NULL));
	memcpy(uuid_bin + 8, &tmp16, 2);

	tmp64 = cpu_to_be64(simple_strtoull(uuid_str + 24, NULL, 16));
	memcpy(uuid_bin + 10, (char *)&tmp64 + 2, 6);

	return 0;
}

int uuid_str_to_le_bin(const char *uuid_str, unsigned char *uuid_bin)
{
	u16 tmp16;
	u32 tmp32;
	u64 tmp64;

	if (!uuid_str_valid(uuid_str) || !uuid_bin)
		return -EINVAL;

	tmp32 = cpu_to_le32(hextoul(uuid_str, NULL));
	memcpy(uuid_bin, &tmp32, 4);

	tmp16 = cpu_to_le16(hextoul(uuid_str + 9, NULL));
	memcpy(uuid_bin + 4, &tmp16, 2);

	tmp16 = cpu_to_le16(hextoul(uuid_str + 14, NULL));
	memcpy(uuid_bin + 6, &tmp16, 2);

	tmp16 = cpu_to_le16(hextoul(uuid_str + 19, NULL));
	memcpy(uuid_bin + 8, &tmp16, 2);

	tmp64 = cpu_to_le64(simple_strtoull(uuid_str + 24, NULL, 16));
	memcpy(uuid_bin + 10, &tmp64, 6);

	return 0;
}

void uuid_bin_to_str(const unsigned char *uuid_bin, char *uuid_str,
		     int str_format)
{
	const u8 uuid_char_order[UUID_BIN_LEN] = {0, 1, 2, 3, 4, 5, 6, 7, 8,
						  9, 10, 11, 12, 13, 14, 15};
	const u8 guid_char_order[UUID_BIN_LEN] = {3, 2, 1, 0, 5, 4, 7, 6, 8,
						  9, 10, 11, 12, 13, 14, 15};
	const u8 *char_order;
	const char *format;
	int i;

	/*
	 * UUID and GUID bin data - always in big endian:
	 * 4B-2B-2B-2B-6B
	 * be be be be be
	 */
	if (str_format & UUID_STR_FORMAT_GUID)
		char_order = guid_char_order;
	else
		char_order = uuid_char_order;
	if (str_format & UUID_STR_UPPER_CASE)
		format = "%02X";
	else
		format = "%02x";

	for (i = 0; i < 16; i++) {
		sprintf(uuid_str, format, uuid_bin[char_order[i]]);
		uuid_str += 2;
		switch (i) {
		case 3:
		case 5:
		case 7:
		case 9:
			*uuid_str++ = '-';
			break;
		}
	}
}

#if defined(CONFIG_RANDOM_UUID) || defined(CONFIG_CMD_UUID)
void gen_rand_uuid(unsigned char *uuid_bin)
{
	u32 ptr[4];
	struct uuid *uuid = (struct uuid *)ptr;
	int i, ret;
	struct udevice *devp;
	u32 randv = 0;

	if (IS_ENABLED(CONFIG_DM_RNG)) {
		ret = uclass_get_device(UCLASS_RNG, 0, &devp);
		if (!ret) {
			ret = dm_rng_read(devp, &randv, sizeof(randv));
			if (ret < 0)
				randv = 0;
		}
	}
	if (randv)
		srand(randv);
	else
		srand(get_ticks() + rand());

	/* Set all fields randomly */
	for (i = 0; i < 4; i++)
		ptr[i] = rand();

	clrsetbits_be16(&uuid->time_hi_and_version,
			UUID_VERSION_MASK,
			UUID_VERSION << UUID_VERSION_SHIFT);

	clrsetbits_8(&uuid->clock_seq_hi_and_reserved,
		     UUID_VARIANT_MASK,
		     UUID_VARIANT << UUID_VARIANT_SHIFT);

	memcpy(uuid_bin, uuid, 16);
}

void gen_rand_uuid_str(char *uuid_str, int str_format)
{
	unsigned char uuid_bin[UUID_BIN_LEN];

	/* Generate UUID (big endian) */
	gen_rand_uuid(uuid_bin);

	/* Convert UUID bin to UUID or GUID formated STRING  */
	uuid_bin_to_str(uuid_bin, uuid_str, str_format);
}

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_CMD_UUID)
int do_uuid(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char uuid[UUID_STR_LEN + 1];
	int str_format;

	if (!strcmp(argv[0], "uuid"))
		str_format = UUID_STR_FORMAT_STD;
	else
		str_format = UUID_STR_FORMAT_GUID;

	if (argc > 2)
		return CMD_RET_USAGE;

	gen_rand_uuid_str(uuid, str_format);

	if (argc == 1)
		printf("%s\n", uuid);
	else
		env_set(argv[1], uuid);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(uuid, CONFIG_SYS_MAXARGS, 1, do_uuid,
	   "UUID - generate random Universally Unique Identifier",
	   "[<varname>]\n"
	   "Argument:\n"
	   "varname: for set result in a environment variable\n"
	   "e.g. uuid uuid_env"
);

U_BOOT_CMD(guid, CONFIG_SYS_MAXARGS, 1, do_uuid,
	   "GUID - generate Globally Unique Identifier based on random UUID",
	   "[<varname>]\n"
	   "Argument:\n"
	   "varname: for set result in a environment variable\n"
	   "e.g. guid guid_env"
);
#endif /* CONFIG_CMD_UUID */
#endif /* CONFIG_RANDOM_UUID || CONFIG_CMD_UUID */
