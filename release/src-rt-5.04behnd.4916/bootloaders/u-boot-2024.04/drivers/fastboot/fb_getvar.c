// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include <common.h>
#include <env.h>
#include <fastboot.h>
#include <fastboot-internal.h>
#include <fb_mmc.h>
#include <fb_nand.h>
#include <fs.h>
#include <part.h>
#include <version.h>
#include <linux/printk.h>

static void getvar_version(char *var_parameter, char *response);
static void getvar_version_bootloader(char *var_parameter, char *response);
static void getvar_downloadsize(char *var_parameter, char *response);
static void getvar_serialno(char *var_parameter, char *response);
static void getvar_version_baseband(char *var_parameter, char *response);
static void getvar_product(char *var_parameter, char *response);
static void getvar_platform(char *var_parameter, char *response);
static void getvar_current_slot(char *var_parameter, char *response);
static void getvar_has_slot(char *var_parameter, char *response);
static void getvar_partition_type(char *part_name, char *response);
static void getvar_partition_size(char *part_name, char *response);
static void getvar_is_userspace(char *var_parameter, char *response);

static const struct {
	const char *variable;
	bool list;
	void (*dispatch)(char *var_parameter, char *response);
} getvar_dispatch[] = {
	{
		.variable = "version",
		.dispatch = getvar_version,
		.list = true,
	}, {
		.variable = "version-bootloader",
		.dispatch = getvar_version_bootloader,
		.list = true
	}, {
		.variable = "downloadsize",
		.dispatch = getvar_downloadsize,
		.list = true
	}, {
		.variable = "max-download-size",
		.dispatch = getvar_downloadsize,
		.list = true
	}, {
		.variable = "serialno",
		.dispatch = getvar_serialno,
		.list = true
	}, {
		.variable = "version-baseband",
		.dispatch = getvar_version_baseband,
		.list = true
	}, {
		.variable = "product",
		.dispatch = getvar_product,
		.list = true
	}, {
		.variable = "platform",
		.dispatch = getvar_platform,
		.list = true
	}, {
		.variable = "current-slot",
		.dispatch = getvar_current_slot,
		.list = true
#if IS_ENABLED(CONFIG_FASTBOOT_FLASH)
	}, {
		.variable = "has-slot",
		.dispatch = getvar_has_slot,
		.list = false
#endif
#if IS_ENABLED(CONFIG_FASTBOOT_FLASH_MMC)
	}, {
		.variable = "partition-type",
		.dispatch = getvar_partition_type,
		.list = false
#endif
#if IS_ENABLED(CONFIG_FASTBOOT_FLASH)
	}, {
		.variable = "partition-size",
		.dispatch = getvar_partition_size,
		.list = false
#endif
	}, {
		.variable = "is-userspace",
		.dispatch = getvar_is_userspace,
		.list = true
	}
};

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
 * @param[out] size If not NULL, will contain partition size
 * Return: Partition number or negative value on error
 */
static int getvar_get_part_info(const char *part_name, char *response,
				size_t *size)
{
	int r;
	struct blk_desc *dev_desc;
	struct disk_partition disk_part;
	struct part_info *part_info;

	if (IS_ENABLED(CONFIG_FASTBOOT_FLASH_MMC)) {
		r = fastboot_mmc_get_part_info(part_name, &dev_desc, &disk_part,
					       response);
		if (r >= 0 && size)
			*size = disk_part.size * disk_part.blksz;
	} else if (IS_ENABLED(CONFIG_FASTBOOT_FLASH_NAND)) {
		r = fastboot_nand_get_part_info(part_name, &part_info, response);
		if (r >= 0 && size)
			*size = part_info->size;
	} else {
		fastboot_fail("this storage is not supported in bootloader", response);
		r = -ENODEV;
	}

	return r;
}

static void getvar_version(char *var_parameter, char *response)
{
	fastboot_okay(FASTBOOT_VERSION, response);
}

static void getvar_version_bootloader(char *var_parameter, char *response)
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

static void __maybe_unused getvar_has_slot(char *part_name, char *response)
{
	char part_name_wslot[PART_NAME_LEN];
	size_t len;
	int r;

	if (!part_name || part_name[0] == '\0')
		goto fail;

	/* part_name_wslot = part_name + "_a" */
	len = strlcpy(part_name_wslot, part_name, PART_NAME_LEN - 3);
	if (len >= PART_NAME_LEN - 3)
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

static void __maybe_unused getvar_partition_type(char *part_name, char *response)
{
	int r;
	struct blk_desc *dev_desc;
	struct disk_partition part_info;

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

static void __maybe_unused getvar_partition_size(char *part_name, char *response)
{
	int r;
	size_t size;

	r = getvar_get_part_info(part_name, response, &size);
	if (r >= 0)
		fastboot_response("OKAY", response, "0x%016zx", size);
}

static void getvar_is_userspace(char *var_parameter, char *response)
{
	fastboot_okay("no", response);
}

static int current_all_dispatch;
void fastboot_getvar_all(char *response)
{
	/*
	 * Find a dispatch getvar that can be listed and send
	 * it as INFO until we reach the end.
	 */
	while (current_all_dispatch < ARRAY_SIZE(getvar_dispatch)) {
		if (!getvar_dispatch[current_all_dispatch].list) {
			current_all_dispatch++;
			continue;
		}

		char envstr[FASTBOOT_RESPONSE_LEN] = { 0 };

		getvar_dispatch[current_all_dispatch].dispatch(NULL, envstr);

		char *envstr_start = envstr;

		if (!strncmp("OKAY", envstr, 4) || !strncmp("FAIL", envstr, 4))
			envstr_start += 4;

		fastboot_response("INFO", response, "%s: %s",
				  getvar_dispatch[current_all_dispatch].variable,
				  envstr_start);

		current_all_dispatch++;
		return;
	}

	fastboot_response("OKAY", response, NULL);
	current_all_dispatch = 0;
}

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
	} else if (!strncmp("all", cmd_parameter, 3) && strlen(cmd_parameter) == 3) {
		current_all_dispatch = 0;
		fastboot_response(FASTBOOT_MULTIRESPONSE_START, response, NULL);
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
