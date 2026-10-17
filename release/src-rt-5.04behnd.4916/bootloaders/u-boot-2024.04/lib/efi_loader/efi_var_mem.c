// SPDX-License-Identifier: GPL-2.0+
/*
 * File interface for UEFI variables
 *
 * Copyright (c) 2020, Heinrich Schuchardt
 */

#include <efi_loader.h>
#include <efi_variable.h>
#include <u-boot/crc.h>

/*
 * The variables efi_var_file and efi_var_entry must be static to avoid
 * referencing them via the global offset table (section .got). The GOT
 * is neither mapped as EfiRuntimeServicesData nor do we support its
 * relocation during SetVirtualAddressMap().
 */
static struct efi_var_file __efi_runtime_data *efi_var_buf;
static struct efi_var_entry __efi_runtime_data *efi_current_var;

/**
 * efi_var_mem_compare() - compare GUID and name with a variable
 *
 * @var:	variable to compare
 * @guid:	GUID to compare
 * @name:	variable name to compare
 * @next:	pointer to next variable
 * Return:	true if match
 */
static bool __efi_runtime
efi_var_mem_compare(struct efi_var_entry *var, const efi_guid_t *guid,
		    const u16 *name, struct efi_var_entry **next)
{
	int i;
	u8 *guid1, *guid2;
	const u16 *data, *var_name;
	bool match = true;

	for (guid1 = (u8 *)&var->guid, guid2 = (u8 *)guid, i = 0;
	     i < sizeof(efi_guid_t) && match; ++i)
		match = (guid1[i] == guid2[i]);

	for (data = var->name, var_name = name;; ++data) {
		if (match)
			match = (*data == *var_name);
		if (!*data)
			break;
		if (*var_name)
			++var_name;
	}

	++data;

	if (next)
		*next = (struct efi_var_entry *)
			ALIGN((uintptr_t)data + var->length, 8);

	if (match)
		efi_current_var = var;

	return match;
}

struct efi_var_entry __efi_runtime
*efi_var_mem_find(const efi_guid_t *guid, const u16 *name,
		  struct efi_var_entry **next)
{
	struct efi_var_entry *var, *last;

	last = (struct efi_var_entry *)
	       ((uintptr_t)efi_var_buf + efi_var_buf->length);

	if (!*name) {
		if (next) {
			*next = efi_var_buf->var;
			if (*next >= last)
				*next = NULL;
		}
		return NULL;
	}
	if (efi_current_var &&
	    efi_var_mem_compare(efi_current_var, guid, name, next)) {
		if (next && *next >= last)
			*next = NULL;
		return efi_current_var;
	}

	var = efi_var_buf->var;
	if (var < last) {
		for (; var;) {
			struct efi_var_entry *pos;
			bool match;

			match = efi_var_mem_compare(var, guid, name, &pos);
			if (pos >= last)
				pos = NULL;
			if (match) {
				if (next)
					*next = pos;
				return var;
			}
			var = pos;
		}
	}
	if (next)
		*next = NULL;
	return NULL;
}

void __efi_runtime efi_var_mem_del(struct efi_var_entry *var)
{
	u16 *data;
	struct efi_var_entry *next, *last;

	if (!var)
		return;

	last = (struct efi_var_entry *)
	       ((uintptr_t)efi_var_buf + efi_var_buf->length);
	if (var <= efi_current_var)
		efi_current_var = NULL;

	for (data = var->name; *data; ++data)
		;
	++data;
	next = (struct efi_var_entry *)
	       ALIGN((uintptr_t)data + var->length, 8);
	efi_var_buf->length -= (uintptr_t)next - (uintptr_t)var;

	/* efi_memcpy_runtime() can be used because next >= var. */
	efi_memcpy_runtime(var, next, (uintptr_t)last - (uintptr_t)next);
	efi_var_buf->crc32 = crc32(0, (u8 *)efi_var_buf->var,
				   efi_var_buf->length -
				   sizeof(struct efi_var_file));
}

efi_status_t __efi_runtime efi_var_mem_ins(
				const u16 *variable_name,
				const efi_guid_t *vendor, u32 attributes,
				const efi_uintn_t size1, const void *data1,
				const efi_uintn_t size2, const void *data2,
				const u64 time)
{
	u16 *data;
	struct efi_var_entry *var;
	u32 var_name_len;

	var = (struct efi_var_entry *)
	      ((uintptr_t)efi_var_buf + efi_var_buf->length);
	var_name_len = u16_strlen(variable_name) + 1;
	data = var->name + var_name_len;

	if ((uintptr_t)data - (uintptr_t)efi_var_buf + size1 + size2 >
	    EFI_VAR_BUF_SIZE)
		return EFI_OUT_OF_RESOURCES;

	var->attr = attributes;
	var->length = size1 + size2;
	var->time = time;

	efi_memcpy_runtime(&var->guid, vendor, sizeof(efi_guid_t));
	efi_memcpy_runtime(var->name, variable_name,
			   sizeof(u16) * var_name_len);
	efi_memcpy_runtime(data, data1, size1);
	efi_memcpy_runtime((u8 *)data + size1, data2, size2);

	var = (struct efi_var_entry *)
	      ALIGN((uintptr_t)data + var->length, 8);
	efi_var_buf->length = (uintptr_t)var - (uintptr_t)efi_var_buf;
	efi_var_buf->crc32 = crc32(0, (u8 *)efi_var_buf->var,
				   efi_var_buf->length -
				   sizeof(struct efi_var_file));

	return EFI_SUCCESS;
}

u64 __efi_runtime efi_var_mem_free(void)
{
	if (efi_var_buf->length + sizeof(struct efi_var_entry) >=
	    EFI_VAR_BUF_SIZE)
		return 0;

	return EFI_VAR_BUF_SIZE - efi_var_buf->length -
	       sizeof(struct efi_var_entry);
}

/**
 * efi_var_mem_bs_del() - delete boot service only variables
 */
static void efi_var_mem_bs_del(void)
{
	struct efi_var_entry *var = efi_var_buf->var;

	for (;;) {
		struct efi_var_entry *last;

		last = (struct efi_var_entry *)
		       ((uintptr_t)efi_var_buf + efi_var_buf->length);
		if (var >= last)
			break;
		if (var->attr & EFI_VARIABLE_RUNTIME_ACCESS) {
			u16 *data;

			/* skip variable */
			for (data = var->name; *data; ++data)
				;
			++data;
			var = (struct efi_var_entry *)
			      ALIGN((uintptr_t)data + var->length, 8);
		} else {
			/* delete variable */
			efi_var_mem_del(var);
		}
	}
}

/**
 * efi_var_mem_notify_exit_boot_services() - ExitBootService callback
 *
 * @event:	callback event
 * @context:	callback context
 */
static void EFIAPI
efi_var_mem_notify_exit_boot_services(struct efi_event *event, void *context)
{
	EFI_ENTRY("%p, %p", event, context);

	/* Delete boot service only variables */
	efi_var_mem_bs_del();

	EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_var_mem_notify_exit_boot_services() - SetVirtualMemoryMap callback
 *
 * @event:	callback event
 * @context:	callback context
 */
static void EFIAPI __efi_runtime
efi_var_mem_notify_virtual_address_map(struct efi_event *event, void *context)
{
	efi_convert_pointer(0, (void **)&efi_var_buf);
	efi_current_var = NULL;
}

efi_status_t efi_var_mem_init(void)
{
	u64 memory;
	efi_status_t ret;
	struct efi_event *event;

	ret = efi_allocate_pages(EFI_ALLOCATE_ANY_PAGES,
				 EFI_RUNTIME_SERVICES_DATA,
				 efi_size_in_pages(EFI_VAR_BUF_SIZE),
				 &memory);
	if (ret != EFI_SUCCESS)
		return ret;
	efi_var_buf = (struct efi_var_file *)(uintptr_t)memory;
	memset(efi_var_buf, 0, EFI_VAR_BUF_SIZE);
	efi_var_buf->magic = EFI_VAR_FILE_MAGIC;
	efi_var_buf->length = (uintptr_t)efi_var_buf->var -
			      (uintptr_t)efi_var_buf;
	/* crc32 for 0 bytes = 0 */

	ret = efi_create_event(EVT_SIGNAL_EXIT_BOOT_SERVICES, TPL_CALLBACK,
			       efi_var_mem_notify_exit_boot_services, NULL,
			       NULL, &event);
	if (ret != EFI_SUCCESS)
		return ret;
	ret = efi_create_event(EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE, TPL_CALLBACK,
			       efi_var_mem_notify_virtual_address_map, NULL,
			       NULL, &event);
	if (ret != EFI_SUCCESS)
		return ret;
	return ret;
}

efi_status_t __efi_runtime
efi_get_variable_mem(const u16 *variable_name, const efi_guid_t *vendor,
		     u32 *attributes, efi_uintn_t *data_size, void *data,
		     u64 *timep)
{
	efi_uintn_t old_size;
	struct efi_var_entry *var;
	u16 *pdata;

	if (!variable_name || !vendor || !data_size)
		return EFI_INVALID_PARAMETER;
	var = efi_var_mem_find(vendor, variable_name, NULL);
	if (!var)
		return EFI_NOT_FOUND;

	if (attributes)
		*attributes = var->attr;
	if (timep)
		*timep = var->time;

	old_size = *data_size;
	*data_size = var->length;
	if (old_size < var->length)
		return EFI_BUFFER_TOO_SMALL;

	if (!data)
		return EFI_INVALID_PARAMETER;

	for (pdata = var->name; *pdata; ++pdata)
		;
	++pdata;

	efi_memcpy_runtime(data, pdata, var->length);

	return EFI_SUCCESS;
}

efi_status_t __efi_runtime
efi_get_next_variable_name_mem(efi_uintn_t *variable_name_size,
			       u16 *variable_name, efi_guid_t *vendor)
{
	struct efi_var_entry *var;
	efi_uintn_t len, old_size;
	u16 *pdata;

	if (!variable_name_size || !variable_name || !vendor)
		return EFI_INVALID_PARAMETER;

	len = *variable_name_size >> 1;
	if (u16_strnlen(variable_name, len) == len)
		return EFI_INVALID_PARAMETER;

	if (!efi_var_mem_find(vendor, variable_name, &var) && *variable_name)
		return EFI_INVALID_PARAMETER;

	if (!var)
		return EFI_NOT_FOUND;

	for (pdata = var->name; *pdata; ++pdata)
		;
	++pdata;

	old_size = *variable_name_size;
	*variable_name_size = (uintptr_t)pdata - (uintptr_t)var->name;

	if (old_size < *variable_name_size)
		return EFI_BUFFER_TOO_SMALL;

	efi_memcpy_runtime(variable_name, var->name, *variable_name_size);
	efi_memcpy_runtime(vendor, &var->guid, sizeof(efi_guid_t));

	return EFI_SUCCESS;
}

void efi_var_buf_update(struct efi_var_file *var_buf)
{
	memcpy(efi_var_buf, var_buf, EFI_VAR_BUF_SIZE);
}
