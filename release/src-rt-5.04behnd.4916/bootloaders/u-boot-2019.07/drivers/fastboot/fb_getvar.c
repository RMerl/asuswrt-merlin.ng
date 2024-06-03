// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include <common.h>
#include <fastboot.h>
#include <fastboot-internal.h>
#include <fb_mmc.h>
#include <fb_nand.h>
#include <fs.h>
#include <version.h>

static void getvar_version(char *var_parameter, char *response);
static void getvar_bootloader_version(char *var_parameter, char *response);
static void getvar_downloadsize(char *var_parameter, char *response);
static void getvar_serialno(char *var_parameter, char *response);
static void getvar_version_baseband(char *var_parameter, char *response);
static void getvar_product(char *var_parameter, char *response);
static void getvar_platform(char *var_parameter, char *response);
static void getvar_current_slot(char *var_parameter, char *response);
static void getvar_slot_suffixes(char *var_parameter, char *response);
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
static void getvar_has_slot(char *var_parameter, char *response);
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
static void getvar_partition_type(char *part_name, char *response);
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
static void getvar_partition_size(char *part_name, char *response);
#endif

static const struct {
	const char *variable;
	void (*dispatch)(char *var_parameter, char *response);
} getvar_dispatch[] = {
	{
		.variable = "version",
		.dispatch = getvar_version
	}, {
		.variable = "bootloader-version",
		.dispatch = getvar_bootloader_version
	}, {
		.variable = "version-bootloader",
		.dispatch = getvar_bootloader_version
	}, {
		.variable = "downloadsize",
		.dispatch = getvar_downloadsize
	}, {
		.variable = "max-download-size",
		.dispatch = getvar_downloadsize
	}, {
		.variable = "serialno",
		.dispatch = getvar_serialno
	}, {
		.variable = "version-baseband",
		.dispatch = getvar_version_baseband
	}, {
		.variable = "product",
		.dispatch = getvar_product
	}, {
		.variable = "platform",
		.dispatch = getvar_platform
	}, {
		.variable = "current-slot",
		.dispatch = getvar_current_slot
	}, {
		.variable = "slot-suffixes",
		.dispatch = getvar_slot_suffixes
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
	}, {
		.variable = "has-slot",
		.dispatch = getvar_has_slot
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	}, {
		.variable = "partition-type",
		.dispatch = getvar_partition_type
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
	}, {
		.variable = "partition-size",
		.dispatch = getvar_partition_size
#endif
	}
};

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
/**
 * Get partition number and size for any storage type.
 *
 * Can be used to check if partition with specified name exists.
 *
 * If error occurs, this function guarantees to fill @p response with fail
 * string. @p response can be rewritten in caller, if needed.
 *
 * @param[in] part_name Info for which partition name to look for
 * @param[in,out] response Pointer to fastboot response buffer
 * @param[out] size If not NULL, will contain partition size (in blocks)
 * @return Partition number or negative value on error
 */
static int getvar_get_part_info(const char *part_name, char *response,
				size_t *size)
{
	int r;
# if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	struct blk_desc *dev_desc;
	disk_partition_t part_info;

	r = fastboot_mmc_get_part_info(part_name, &dev_desc, &part_info,
				       response);
	if (r >= 0 && size)
		*size = part_info.size;
# elif CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND)
	struct part_info *part_info;

	r = fastboot_nand_get_part_info(part_name, &part_info, response);
	if (r >= 0 && size)
		*size = part_info->size;
# else
	fastboot_fail("this storage is not supported in bootloader", response);
	r = -ENODEV;
# endif

	return r;
}
#endif

static void getvar_version(char *var_parameter, char *response)
{
	fastboot_okay(FASTBOOT_VERSION, response);
}

static void getvar_bootloader_version(char *var_parameter, char *response)
{
	fastboot_okay(U_BOOT_VERSION, response);
}

static void getvar_downloadsize(char *var_parameter, char *response)
{
	fastboot_response("OKAY", response, "0x%08x", fastboot_buf_size);
}

static void getvar_serialno(char *var_parameter, char *response)
{
	const char *tmp = env_get("serial#");

	if (tmp)
		fastboot_okay(tmp, response);
	else
		fastboot_fail("Value not set", response);
}

static void getvar_version_baseband(char *var_parameter, char *response)
{
	fastboot_okay("N/A", response);
}

static void getvar_product(char *var_parameter, char *response)
{
	const char *board = env_get("board");

	if (board)
		fastboot_okay(board, response);
	else
		fastboot_fail("Board not set", response);
}

static void getvar_platform(char *var_parameter, char *response)
{
	const char *p = env_get("platform");

	if (p)
		fastboot_okay(p, response);
	else
		fastboot_fail("platform not set", response);
}

static void getvar_current_slot(char *var_parameter, char *response)
{
	/* A/B not implemented, for now always return "a" */
	fastboot_okay("a", response);
}

static void getvar_slot_suffixes(char *var_parameter, char *response)
{
	fastboot_okay("a,b", response);
}

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
static void getvar_has_slot(char *part_name, char *response)
{
	char part_name_wslot[PART_NAME_LEN];
	size_t len;
	int r;

	if (!part_name || part_name[0] == '\0')
		goto fail;

	/* part_name_wslot = part_name + "_a" */
	len = strlcpy(part_name_wslot, part_name, PART_NAME_LEN - 3);
	if (len > PART_NAME_LEN - 3)
		goto fail;
	strcat(part_name_wslot, "_a");

	r = getvar_get_part_info(part_name_wslot, response, NULL);
	if (r >= 0) {
		fastboot_okay("yes", response); /* part exists and slotted */
		return;
	}

	r = getvar_get_part_info(part_name, response, NULL);
	if (r >= 0)
		fastboot_okay("no", response); /* part exists but not slotted */

	/* At this point response is filled with okay or fail string */
	return;

fail:
	fastboot_fail("invalid partition name", response);
}
#endif

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
static void getvar_partition_type(char *part_name, char *response)
{
	int r;
	struct blk_desc *dev_desc;
	disk_partition_t part_info;

	r = fastboot_mmc_get_part_info(part_name, &dev_desc, &part_info,
				       response);
	if (r >= 0) {
		r = fs_set_blk_dev_with_part(dev_desc, r);
		if (r < 0)
			fastboot_fail("failed to set partition", response);
		else
			fastboot_okay(fs_get_type_name(), response);
	}
}
#endif

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
static void getvar_partition_size(char *part_name, char *response)
{
	int r;
	size_t size;

	r = getvar_get_part_info(part_name, response, &size);
	if (r >= 0)
		fastboot_response("OKAY", response, "0x%016zx", size);
}
#endif

/**
 * fastboot_getvar() - Writes variable indicated by cmd_parameter to response.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 *
 * Look up cmd_parameter first as an environment variable of the form
 * fastboot.<cmd_parameter>, if that exists return use its value to set
 * response.
 *
 * Otherwise lookup the name of variable and execute the appropriate
 * function to return the requested value.
 */
void fastboot_getvar(char *cmd_parameter, char *response)
{
	if (!cmd_parameter) {
		fastboot_fail("missing var", response);
	} else {
#define FASTBOOT_ENV_PREFIX	"fastboot."
		int i;
		char *var_parameter = cmd_parameter;
		char envstr[FASTBOOT_RESPONSE_LEN];
		const char *s;

		snprintf(envstr, sizeof(envstr) - 1,
			 FASTBOOT_ENV_PREFIX "%s", cmd_parameter);
		s = env_get(envstr);
		if (s) {
			fastboot_response("OKAY", response, "%s", s);
			return;
		}

		strsep(&var_parameter, ":");
		for (i = 0; i < ARRAY_SIZE(getvar_dispatch); ++i) {
			if (!strcmp(getvar_dispatch[i].variable,
				    cmd_parameter)) {
				getvar_dispatch[i].dispatch(var_parameter,
							    response);
				return;
			}
		}
		pr_warn("WARNING: unknown variable: %s\n", cmd_parameter);
		fastboot_fail("Variable not implemented", response);
	}
}
