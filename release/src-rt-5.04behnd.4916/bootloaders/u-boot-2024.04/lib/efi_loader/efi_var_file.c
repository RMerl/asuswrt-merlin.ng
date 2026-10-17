// SPDX-License-Identifier: GPL-2.0+
/*
 * File interface for UEFI variables
 *
 * Copyright (c) 2020, Heinrich Schuchardt
 */

#define LOG_CATEGORY LOGC_EFI

#include <charset.h>
#include <fs.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <u-boot/crc.h>

#define PART_STR_LEN 10

/* GUID used by Shim to store the MOK database */
#define SHIM_LOCK_GUID \
	EFI_GUID(0x605dab50, 0xe046, 0x4300, \
		 0xab, 0xb6, 0x3d, 0xd8, 0x10, 0xdd, 0x8b, 0x23)

static const efi_guid_t shim_lock_guid = SHIM_LOCK_GUID;

/**
 * efi_set_blk_dev_to_system_partition() - select EFI system partition
 *
 * Set the EFI system partition as current block device.
 *
 * Return:	status code
 */
static efi_status_t __maybe_unused efi_set_blk_dev_to_system_partition(void)
{
	char part_str[PART_STR_LEN];
	int r;

	if (efi_system_partition.uclass_id == UCLASS_INVALID) {
		log_err("No EFI system partition\n");
		return EFI_DEVICE_ERROR;
	}
	snprintf(part_str, PART_STR_LEN, "%x:%x",
		 efi_system_partition.devnum, efi_system_partition.part);
	r = fs_set_blk_dev(blk_get_uclass_name(efi_system_partition.uclass_id),
			   part_str, FS_TYPE_ANY);
	if (r) {
		log_err("Cannot read EFI system partition\n");
		return EFI_DEVICE_ERROR;
	}
	return EFI_SUCCESS;
}

efi_status_t __maybe_unused efi_var_collect(struct efi_var_file **bufp, loff_t *lenp,
					    u32 check_attr_mask)
{
	size_t len = EFI_VAR_BUF_SIZE;
	struct efi_var_file *buf;
	struct efi_var_entry *var, *old_var;
	size_t old_var_name_length = 2;

	*bufp = NULL; /* Avoid double free() */
	buf = calloc(1, len);
	if (!buf)
		return EFI_OUT_OF_RESOURCES;
	var = buf->var;
	old_var = var;
	for (;;) {
		efi_uintn_t data_length, var_name_length;
		u8 *data;
		efi_status_t ret;

		if ((uintptr_t)buf + len <=
		    (uintptr_t)var->name + old_var_name_length)
			return EFI_BUFFER_TOO_SMALL;

		var_name_length = (uintptr_t)buf + len - (uintptr_t)var->name;
		memcpy(var->name, old_var->name, old_var_name_length);
		guidcpy(&var->guid, &old_var->guid);
		ret = efi_get_next_variable_name_int(
				&var_name_length, var->name, &var->guid);
		if (ret == EFI_NOT_FOUND)
			break;
		if (ret != EFI_SUCCESS) {
			free(buf);
			return ret;
		}
		old_var_name_length = var_name_length;
		old_var = var;

		data = (u8 *)var->name + old_var_name_length;
		data_length = (uintptr_t)buf + len - (uintptr_t)data;
		ret = efi_get_variable_int(var->name, &var->guid,
					   &var->attr, &data_length, data,
					   &var->time);
		if (ret != EFI_SUCCESS) {
			free(buf);
			return ret;
		}
		if ((var->attr & check_attr_mask) == check_attr_mask) {
			var->length = data_length;
			var = (struct efi_var_entry *)ALIGN((uintptr_t)data + data_length, 8);
		}
	}

	buf->reserved = 0;
	buf->magic = EFI_VAR_FILE_MAGIC;
	len = (uintptr_t)var - (uintptr_t)buf;
	buf->crc32 = crc32(0, (u8 *)buf->var,
			   len - sizeof(struct efi_var_file));
	buf->length = len;
	*bufp = buf;
	*lenp = len;

	return EFI_SUCCESS;
}

/**
 * efi_var_to_file() - save non-volatile variables as file
 *
 * File ubootefi.var is created on the EFI system partion.
 *
 * Return:	status code
 */
efi_status_t efi_var_to_file(void)
{
#ifdef CONFIG_EFI_VARIABLE_FILE_STORE
	efi_status_t ret;
	struct efi_var_file *buf;
	loff_t len;
	loff_t actlen;
	int r;

	ret = efi_var_collect(&buf, &len, EFI_VARIABLE_NON_VOLATILE);
	if (ret != EFI_SUCCESS)
		goto error;

	ret = efi_set_blk_dev_to_system_partition();
	if (ret != EFI_SUCCESS)
		goto error;

	r = fs_write(EFI_VAR_FILE_NAME, map_to_sysmem(buf), 0, len, &actlen);
	if (r || len != actlen)
		ret = EFI_DEVICE_ERROR;

error:
	if (ret != EFI_SUCCESS)
		log_err("Failed to persist EFI variables\n");
	free(buf);
	return ret;
#else
	return EFI_SUCCESS;
#endif
}

efi_status_t efi_var_restore(struct efi_var_file *buf, bool safe)
{
	struct efi_var_entry *var, *last_var;
	u16 *data;
	efi_status_t ret;

	if (buf->reserved || buf->magic != EFI_VAR_FILE_MAGIC ||
	    buf->crc32 != crc32(0, (u8 *)buf->var,
				buf->length - sizeof(struct efi_var_file))) {
		log_err("Invalid EFI variables file\n");
		return EFI_INVALID_PARAMETER;
	}

	last_var = (struct efi_var_entry *)((u8 *)buf + buf->length);
	for (var = buf->var; var < last_var;
	     var = (struct efi_var_entry *)
		   ALIGN((uintptr_t)data + var->length, 8)) {

		data = var->name + u16_strlen(var->name) + 1;

		/*
		 * Secure boot related and volatile variables shall only be
		 * restored from U-Boot's preseed.
		 */
		if (!safe &&
		    (efi_auth_var_get_type(var->name, &var->guid) !=
		     EFI_AUTH_VAR_NONE ||
		     !guidcmp(&var->guid, &shim_lock_guid) ||
		     !(var->attr & EFI_VARIABLE_NON_VOLATILE)))
			continue;
		if (!var->length)
			continue;
		if (efi_var_mem_find(&var->guid, var->name, NULL))
			continue;
		ret = efi_var_mem_ins(var->name, &var->guid, var->attr,
				      var->length, data, 0, NULL,
				      var->time);
		if (ret != EFI_SUCCESS)
			log_err("Failed to set EFI variable %ls\n", var->name);
	}
	return EFI_SUCCESS;
}

/**
 * efi_var_from_file() - read variables from file
 *
 * File ubootefi.var is read from the EFI system partitions and the variables
 * stored in the file are created.
 *
 * On first boot the file ubootefi.var does not exist yet. This is why we must
 * return EFI_SUCCESS in this case.
 *
 * If the variable file is corrupted, e.g. incorrect CRC32, we do not want to
 * stop the boot process. We deliberately return EFI_SUCCESS in this case, too.
 *
 * Return:	status code
 */
efi_status_t efi_var_from_file(void)
{
#ifdef CONFIG_EFI_VARIABLE_FILE_STORE
	struct efi_var_file *buf;
	loff_t len;
	efi_status_t ret;
	int r;

	buf = calloc(1, EFI_VAR_BUF_SIZE);
	if (!buf) {
		log_err("Out of memory\n");
		return EFI_OUT_OF_RESOURCES;
	}

	ret = efi_set_blk_dev_to_system_partition();
	if (ret != EFI_SUCCESS)
		goto error;
	r = fs_read(EFI_VAR_FILE_NAME, map_to_sysmem(buf), 0, EFI_VAR_BUF_SIZE,
		    &len);
	if (r || len < sizeof(struct efi_var_file)) {
		log_err("Failed to load EFI variables\n");
		goto error;
	}
	if (buf->length != len || efi_var_restore(buf, false) != EFI_SUCCESS)
		log_err("Invalid EFI variables file\n");
error:
	free(buf);
#endif
	return EFI_SUCCESS;
}
