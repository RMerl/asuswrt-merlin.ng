// SPDX-License-Identifier:     GPL-2.0+
/*
 *  EFI Human Interface Infrastructure ... database and packages
 *
 *  Copyright (c) 2017 Leif Lindholm
 *  Copyright (c) 2018 AKASHI Takahiro, Linaro Limited
 */

#include <common.h>
#include <efi_loader.h>
#include <malloc.h>
#include <asm/unaligned.h>

const efi_guid_t efi_guid_hii_database_protocol
		= EFI_HII_DATABASE_PROTOCOL_GUID;
const efi_guid_t efi_guid_hii_string_protocol = EFI_HII_STRING_PROTOCOL_GUID;

static LIST_HEAD(efi_package_lists);
static LIST_HEAD(efi_keyboard_layout_list);

struct efi_hii_packagelist {
	struct list_head link;
	// TODO should there be an associated efi_object?
	efi_handle_t driver_handle;
	u32 max_string_id;
	struct list_head string_tables;     /* list of efi_string_table */
	struct list_head guid_list;
	struct list_head keyboard_packages;

	/* we could also track fonts, images, etc */
};

static int efi_hii_packagelist_exists(efi_hii_handle_t package_list)
{
	struct efi_hii_packagelist *hii;
	int found = 0;

	list_for_each_entry(hii, &efi_package_lists, link) {
		if (hii == package_list) {
			found = 1;
			break;
		}
	}

	return found;
}

static u32 efi_hii_package_type(struct efi_hii_package_header *header)
{
	u32 fields;

	fields = get_unaligned_le32(&header->fields);

	return (fields >> __EFI_HII_PACKAGE_TYPE_SHIFT)
		& __EFI_HII_PACKAGE_TYPE_MASK;
}

static u32 efi_hii_package_len(struct efi_hii_package_header *header)
{
	u32 fields;

	fields = get_unaligned_le32(&header->fields);

	return (fields >> __EFI_HII_PACKAGE_LEN_SHIFT)
		& __EFI_HII_PACKAGE_LEN_MASK;
}

struct efi_string_info {
	efi_string_t string;
	/* we could also track font info, etc */
};

struct efi_string_table {
	struct list_head link;
	efi_string_id_t language_name;
	char *language;
	u32 nstrings;
	/*
	 * NOTE:
	 *  string id starts at 1 so value is stbl->strings[id-1],
	 *  and strings[] is a array of stbl->nstrings elements
	 */
	struct efi_string_info *strings;
};

struct efi_guid_data {
	struct list_head link;
	struct efi_hii_guid_package package;
};

struct efi_keyboard_layout_data {
	struct list_head link;		/* in package */
	struct list_head link_sys;	/* in global list */
	struct efi_hii_keyboard_layout keyboard_layout;
};

struct efi_keyboard_package_data {
	struct list_head link;		/* in package_list */
	struct list_head keyboard_layout_list;
};

static void free_strings_table(struct efi_string_table *stbl)
{
	int i;

	for (i = 0; i < stbl->nstrings; i++)
		free(stbl->strings[i].string);
	free(stbl->strings);
	free(stbl->language);
	free(stbl);
}

static void remove_strings_package(struct efi_hii_packagelist *hii)
{
	while (!list_empty(&hii->string_tables)) {
		struct efi_string_table *stbl;

		stbl = list_first_entry(&hii->string_tables,
					struct efi_string_table, link);
		list_del(&stbl->link);
		free_strings_table(stbl);
	}
}

static efi_status_t
add_strings_package(struct efi_hii_packagelist *hii,
		    struct efi_hii_strings_package *strings_package)
{
	struct efi_hii_string_block *block;
	void *end;
	u32 nstrings = 0, idx = 0;
	struct efi_string_table *stbl = NULL;
	efi_status_t ret;

	EFI_PRINT("header_size: %08x\n",
		  get_unaligned_le32(&strings_package->header_size));
	EFI_PRINT("string_info_offset: %08x\n",
		  get_unaligned_le32(&strings_package->string_info_offset));
	EFI_PRINT("language_name: %u\n",
		  get_unaligned_le16(&strings_package->language_name));
	EFI_PRINT("language: %s\n", strings_package->language);

	/* count # of string entries: */
	end = ((void *)strings_package)
			+ efi_hii_package_len(&strings_package->header);
	block = ((void *)strings_package)
		+ get_unaligned_le32(&strings_package->string_info_offset);

	while ((void *)block < end) {
		switch (block->block_type) {
		case EFI_HII_SIBT_STRING_UCS2: {
			struct efi_hii_sibt_string_ucs2_block *ucs2;

			ucs2 = (void *)block;
			nstrings++;
			block = efi_hii_sibt_string_ucs2_block_next(ucs2);
			break;
		}
		case EFI_HII_SIBT_END:
			block = end;
			break;
		default:
			EFI_PRINT("unknown HII string block type: %02x\n",
				  block->block_type);
			return EFI_INVALID_PARAMETER;
		}
	}

	stbl = calloc(sizeof(*stbl), 1);
	if (!stbl) {
		ret = EFI_OUT_OF_RESOURCES;
		goto error;
	}
	stbl->strings = calloc(sizeof(stbl->strings[0]), nstrings);
	if (!stbl->strings) {
		ret = EFI_OUT_OF_RESOURCES;
		goto error;
	}
	stbl->language_name =
			get_unaligned_le16(&strings_package->language_name);
	stbl->language = strdup((char *)strings_package->language);
	if (!stbl->language) {
		ret = EFI_OUT_OF_RESOURCES;
		goto error;
	}
	stbl->nstrings = nstrings;

	/* and now parse string entries and populate efi_string_table */
	block = ((void *)strings_package)
		+ get_unaligned_le32(&strings_package->string_info_offset);

	while ((void *)block < end) {
		switch (block->block_type) {
		case EFI_HII_SIBT_STRING_UCS2: {
			struct efi_hii_sibt_string_ucs2_block *ucs2;

			ucs2 = (void *)block;
			EFI_PRINT("%4u: \"%ls\"\n", idx + 1, ucs2->string_text);
			stbl->strings[idx].string =
				u16_strdup(ucs2->string_text);
			if (!stbl->strings[idx].string) {
				ret = EFI_OUT_OF_RESOURCES;
				goto error;
			}
			idx++;
			/* FIXME: accessing u16 * here */
			block = efi_hii_sibt_string_ucs2_block_next(ucs2);
			break;
		}
		case EFI_HII_SIBT_END:
			goto out;
		default:
			EFI_PRINT("unknown HII string block type: %02x\n",
				  block->block_type);
			ret = EFI_INVALID_PARAMETER;
			goto error;
		}
	}

out:
	list_add(&stbl->link, &hii->string_tables);
	if (hii->max_string_id < nstrings)
		hii->max_string_id = nstrings;

	return EFI_SUCCESS;

error:
	if (stbl) {
		free(stbl->language);
		while (idx > 0)
			free(stbl->strings[--idx].string);
		free(stbl->strings);
	}
	free(stbl);

	return ret;
}

static void remove_guid_package(struct efi_hii_packagelist *hii)
{
	struct efi_guid_data *data;

	while (!list_empty(&hii->guid_list)) {
		data = list_first_entry(&hii->guid_list,
					struct efi_guid_data, link);
		list_del(&data->link);
		free(data);
	}
}

static efi_status_t
add_guid_package(struct efi_hii_packagelist *hii,
		 struct efi_hii_guid_package *package)
{
	struct efi_guid_data *data;

	data = calloc(sizeof(*data), 1);
	if (!data)
		return EFI_OUT_OF_RESOURCES;

	/* TODO: we don't know any about data field */
	memcpy(&data->package, package, sizeof(*package));
	list_add_tail(&data->link, &hii->guid_list);

	return EFI_SUCCESS;
}

static void free_keyboard_layouts(struct efi_keyboard_package_data *package)
{
	struct efi_keyboard_layout_data *layout_data;

	while (!list_empty(&package->keyboard_layout_list)) {
		layout_data = list_first_entry(&package->keyboard_layout_list,
					       struct efi_keyboard_layout_data,
					       link);
		list_del(&layout_data->link);
		list_del(&layout_data->link_sys);
		free(layout_data);
	}
}

static void remove_keyboard_package(struct efi_hii_packagelist *hii)
{
	struct efi_keyboard_package_data *package;

	while (!list_empty(&hii->keyboard_packages)) {
		package = list_first_entry(&hii->keyboard_packages,
					   struct efi_keyboard_package_data,
					   link);
		free_keyboard_layouts(package);
		list_del(&package->link);
		free(package);
	}
}

static efi_status_t
add_keyboard_package(struct efi_hii_packagelist *hii,
		     struct efi_hii_keyboard_package *keyboard_package)
{
	struct efi_keyboard_package_data *package_data;
	struct efi_hii_keyboard_layout *layout;
	struct efi_keyboard_layout_data *layout_data;
	u16 layout_count, layout_length;
	int i;

	package_data = malloc(sizeof(*package_data));
	if (!package_data)
		return EFI_OUT_OF_RESOURCES;
	INIT_LIST_HEAD(&package_data->link);
	INIT_LIST_HEAD(&package_data->keyboard_layout_list);

	layout = &keyboard_package->layout[0];
	layout_count = get_unaligned_le16(&keyboard_package->layout_count);
	for (i = 0; i < layout_count; i++) {
		layout_length = get_unaligned_le16(&layout->layout_length);
		layout_data = malloc(sizeof(*layout_data) + layout_length);
		if (!layout_data)
			goto out;

		memcpy(&layout_data->keyboard_layout, layout, layout_length);
		list_add_tail(&layout_data->link,
			      &package_data->keyboard_layout_list);
		list_add_tail(&layout_data->link_sys,
			      &efi_keyboard_layout_list);

		layout += layout_length;
	}

	list_add_tail(&package_data->link, &hii->keyboard_packages);

	return EFI_SUCCESS;

out:
	free_keyboard_layouts(package_data);
	free(package_data);

	return EFI_OUT_OF_RESOURCES;
}

static struct efi_hii_packagelist *new_packagelist(void)
{
	struct efi_hii_packagelist *hii;

	hii = malloc(sizeof(*hii));
	list_add_tail(&hii->link, &efi_package_lists);
	hii->max_string_id = 0;
	INIT_LIST_HEAD(&hii->string_tables);
	INIT_LIST_HEAD(&hii->guid_list);
	INIT_LIST_HEAD(&hii->keyboard_packages);

	return hii;
}

static void free_packagelist(struct efi_hii_packagelist *hii)
{
	remove_strings_package(hii);
	remove_guid_package(hii);
	remove_keyboard_package(hii);

	list_del(&hii->link);
	free(hii);
}

static efi_status_t
add_packages(struct efi_hii_packagelist *hii,
	     const struct efi_hii_package_list_header *package_list)
{
	struct efi_hii_package_header *package;
	void *end;
	efi_status_t ret = EFI_SUCCESS;

	end = ((void *)package_list)
		+ get_unaligned_le32(&package_list->package_length);

	EFI_PRINT("package_list: %pUl (%u)\n", &package_list->package_list_guid,
		  get_unaligned_le32(&package_list->package_length));

	package = ((void *)package_list) + sizeof(*package_list);
	while ((void *)package < end) {
		EFI_PRINT("package=%p, package type=%x, length=%u\n", package,
			  efi_hii_package_type(package),
			  efi_hii_package_len(package));

		switch (efi_hii_package_type(package)) {
		case EFI_HII_PACKAGE_TYPE_GUID:
			ret = add_guid_package(hii,
				(struct efi_hii_guid_package *)package);
			break;
		case EFI_HII_PACKAGE_FORMS:
			EFI_PRINT("Form package not supported\n");
			ret = EFI_INVALID_PARAMETER;
			break;
		case EFI_HII_PACKAGE_STRINGS:
			ret = add_strings_package(hii,
				(struct efi_hii_strings_package *)package);
			break;
		case EFI_HII_PACKAGE_FONTS:
			EFI_PRINT("Font package not supported\n");
			ret = EFI_INVALID_PARAMETER;
			break;
		case EFI_HII_PACKAGE_IMAGES:
			EFI_PRINT("Image package not supported\n");
			ret = EFI_INVALID_PARAMETER;
			break;
		case EFI_HII_PACKAGE_SIMPLE_FONTS:
			EFI_PRINT("Simple font package not supported\n");
			ret = EFI_INVALID_PARAMETER;
			break;
		case EFI_HII_PACKAGE_DEVICE_PATH:
			EFI_PRINT("Device path package not supported\n");
			ret = EFI_INVALID_PARAMETER;
			break;
		case EFI_HII_PACKAGE_KEYBOARD_LAYOUT:
			ret = add_keyboard_package(hii,
				(struct efi_hii_keyboard_package *)package);
			break;
		case EFI_HII_PACKAGE_ANIMATIONS:
			EFI_PRINT("Animation package not supported\n");
			ret = EFI_INVALID_PARAMETER;
			break;
		case EFI_HII_PACKAGE_END:
			goto out;
		case EFI_HII_PACKAGE_TYPE_SYSTEM_BEGIN:
		case EFI_HII_PACKAGE_TYPE_SYSTEM_END:
		default:
			break;
		}

		if (ret != EFI_SUCCESS)
			return ret;

		package = (void *)package + efi_hii_package_len(package);
	}
out:
	// TODO in theory there is some notifications that should be sent..
	return EFI_SUCCESS;
}

/*
 * EFI_HII_DATABASE_PROTOCOL
 */

static efi_status_t EFIAPI
new_package_list(const struct efi_hii_database_protocol *this,
		 const struct efi_hii_package_list_header *package_list,
		 const efi_handle_t driver_handle,
		 efi_hii_handle_t *handle)
{
	struct efi_hii_packagelist *hii;
	efi_status_t ret;

	EFI_ENTRY("%p, %p, %p, %p", this, package_list, driver_handle, handle);

	if (!package_list || !handle)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	hii = new_packagelist();
	if (!hii)
		return EFI_EXIT(EFI_OUT_OF_RESOURCES);

	ret = add_packages(hii, package_list);
	if (ret != EFI_SUCCESS) {
		free_packagelist(hii);
		return EFI_EXIT(ret);
	}

	hii->driver_handle = driver_handle;
	*handle = hii;

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI
remove_package_list(const struct efi_hii_database_protocol *this,
		    efi_hii_handle_t handle)
{
	struct efi_hii_packagelist *hii = handle;

	EFI_ENTRY("%p, %p", this, handle);

	if (!handle || !efi_hii_packagelist_exists(handle))
		return EFI_EXIT(EFI_NOT_FOUND);

	free_packagelist(hii);

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI
update_package_list(const struct efi_hii_database_protocol *this,
		    efi_hii_handle_t handle,
		    const struct efi_hii_package_list_header *package_list)
{
	struct efi_hii_packagelist *hii = handle;
	struct efi_hii_package_header *package;
	void *end;
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %p, %p", this, handle, package_list);

	if (!handle || !efi_hii_packagelist_exists(handle))
		return EFI_EXIT(EFI_NOT_FOUND);

	if (!package_list)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	EFI_PRINT("package_list: %pUl (%u)\n", &package_list->package_list_guid,
		  get_unaligned_le32(&package_list->package_length));

	package = ((void *)package_list) + sizeof(*package_list);
	end = ((void *)package_list)
		+ get_unaligned_le32(&package_list->package_length);

	while ((void *)package < end) {
		EFI_PRINT("package=%p, package type=%x, length=%u\n", package,
			  efi_hii_package_type(package),
			  efi_hii_package_len(package));

		switch (efi_hii_package_type(package)) {
		case EFI_HII_PACKAGE_TYPE_GUID:
			remove_guid_package(hii);
			break;
		case EFI_HII_PACKAGE_FORMS:
			EFI_PRINT("Form package not supported\n");
			ret = EFI_INVALID_PARAMETER;
			break;
		case EFI_HII_PACKAGE_STRINGS:
			remove_strings_package(hii);
			break;
		case EFI_HII_PACKAGE_FONTS:
			EFI_PRINT("Font package not supported\n");
			ret = EFI_INVALID_PARAMETER;
			break;
		case EFI_HII_PACKAGE_IMAGES:
			EFI_PRINT("Image package not supported\n");
			ret = EFI_INVALID_PARAMETER;
			break;
		case EFI_HII_PACKAGE_SIMPLE_FONTS:
			EFI_PRINT("Simple font package not supported\n");
			ret = EFI_INVALID_PARAMETER;
			break;
		case EFI_HII_PACKAGE_DEVICE_PATH:
			EFI_PRINT("Device path package not supported\n");
			ret = EFI_INVALID_PARAMETER;
			break;
		case EFI_HII_PACKAGE_KEYBOARD_LAYOUT:
			remove_keyboard_package(hii);
			break;
		case EFI_HII_PACKAGE_ANIMATIONS:
			EFI_PRINT("Animation package not supported\n");
			ret = EFI_INVALID_PARAMETER;
			break;
		case EFI_HII_PACKAGE_END:
			goto out;
		case EFI_HII_PACKAGE_TYPE_SYSTEM_BEGIN:
		case EFI_HII_PACKAGE_TYPE_SYSTEM_END:
		default:
			break;
		}

		/* TODO: already removed some packages */
		if (ret != EFI_SUCCESS)
			return EFI_EXIT(ret);

		package = ((void *)package)
			  + efi_hii_package_len(package);
	}
out:
	ret = add_packages(hii, package_list);

	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI
list_package_lists(const struct efi_hii_database_protocol *this,
		   u8 package_type,
		   const efi_guid_t *package_guid,
		   efi_uintn_t *handle_buffer_length,
		   efi_hii_handle_t *handle)
{
	struct efi_hii_packagelist *hii =
				(struct efi_hii_packagelist *)handle;
	int package_cnt, package_max;
	efi_status_t ret = EFI_NOT_FOUND;

	EFI_ENTRY("%p, %u, %pUl, %p, %p", this, package_type, package_guid,
		  handle_buffer_length, handle);

	if (!handle_buffer_length ||
	    (*handle_buffer_length && !handle)) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	if ((package_type != EFI_HII_PACKAGE_TYPE_GUID && package_guid) ||
	    (package_type == EFI_HII_PACKAGE_TYPE_GUID && !package_guid)) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	EFI_PRINT("package type=%x, guid=%pUl, length=%zu\n", (int)package_type,
		  package_guid, *handle_buffer_length);

	package_cnt = 0;
	package_max = *handle_buffer_length / sizeof(*handle);
	list_for_each_entry(hii, &efi_package_lists, link) {
		switch (package_type) {
		case EFI_HII_PACKAGE_TYPE_ALL:
			break;
		case EFI_HII_PACKAGE_TYPE_GUID:
			if (!list_empty(&hii->guid_list))
				break;
			continue;
		case EFI_HII_PACKAGE_STRINGS:
			if (!list_empty(&hii->string_tables))
				break;
			continue;
		case EFI_HII_PACKAGE_KEYBOARD_LAYOUT:
			if (!list_empty(&hii->keyboard_packages))
				break;
			continue;
		default:
			continue;
		}

		package_cnt++;
		if (package_cnt <= package_max) {
			*handle++ = hii;
			ret = EFI_SUCCESS;
		} else {
			ret = EFI_BUFFER_TOO_SMALL;
		}
	}
	*handle_buffer_length = package_cnt * sizeof(*handle);
out:
	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI
export_package_lists(const struct efi_hii_database_protocol *this,
		     efi_hii_handle_t handle,
		     efi_uintn_t *buffer_size,
		     struct efi_hii_package_list_header *buffer)
{
	EFI_ENTRY("%p, %p, %p, %p", this, handle, buffer_size, buffer);

	if (!buffer_size || !buffer)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	return EFI_EXIT(EFI_NOT_FOUND);
}

static efi_status_t EFIAPI
register_package_notify(const struct efi_hii_database_protocol *this,
			u8 package_type,
			const efi_guid_t *package_guid,
			const void *package_notify_fn,
			efi_uintn_t notify_type,
			efi_handle_t *notify_handle)
{
	EFI_ENTRY("%p, %u, %pUl, %p, %zu, %p", this, package_type,
		  package_guid, package_notify_fn, notify_type,
		  notify_handle);

	if (!notify_handle)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	if ((package_type != EFI_HII_PACKAGE_TYPE_GUID && package_guid) ||
	    (package_type == EFI_HII_PACKAGE_TYPE_GUID && !package_guid))
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
}

static efi_status_t EFIAPI
unregister_package_notify(const struct efi_hii_database_protocol *this,
			  efi_handle_t notification_handle)
{
	EFI_ENTRY("%p, %p", this, notification_handle);

	return EFI_EXIT(EFI_NOT_FOUND);
}

static efi_status_t EFIAPI
find_keyboard_layouts(const struct efi_hii_database_protocol *this,
		      u16 *key_guid_buffer_length,
		      efi_guid_t *key_guid_buffer)
{
	struct efi_keyboard_layout_data *layout_data;
	int package_cnt, package_max;
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %p, %p", this, key_guid_buffer_length, key_guid_buffer);

	if (!key_guid_buffer_length ||
	    (*key_guid_buffer_length && !key_guid_buffer))
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	package_cnt = 0;
	package_max = *key_guid_buffer_length / sizeof(*key_guid_buffer);
	list_for_each_entry(layout_data, &efi_keyboard_layout_list, link_sys) {
		package_cnt++;
		if (package_cnt <= package_max)
			memcpy(key_guid_buffer++,
			       &layout_data->keyboard_layout.guid,
			       sizeof(*key_guid_buffer));
		else
			ret = EFI_BUFFER_TOO_SMALL;
	}
	*key_guid_buffer_length = package_cnt * sizeof(*key_guid_buffer);

	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI
get_keyboard_layout(const struct efi_hii_database_protocol *this,
		    efi_guid_t *key_guid,
		    u16 *keyboard_layout_length,
		    struct efi_hii_keyboard_layout *keyboard_layout)
{
	struct efi_keyboard_layout_data *layout_data;
	u16 layout_length;

	EFI_ENTRY("%p, %pUl, %p, %p", this, key_guid, keyboard_layout_length,
		  keyboard_layout);

	if (!keyboard_layout_length ||
	    (*keyboard_layout_length && !keyboard_layout))
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	/* TODO: no notion of current keyboard layout */
	if (!key_guid)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	list_for_each_entry(layout_data, &efi_keyboard_layout_list, link_sys) {
		if (!guidcmp(&layout_data->keyboard_layout.guid, key_guid))
			goto found;
	}

	return EFI_EXIT(EFI_NOT_FOUND);

found:
	layout_length =
		get_unaligned_le16(&layout_data->keyboard_layout.layout_length);
	if (*keyboard_layout_length < layout_length) {
		*keyboard_layout_length = layout_length;
		return EFI_EXIT(EFI_BUFFER_TOO_SMALL);
	}

	memcpy(keyboard_layout, &layout_data->keyboard_layout, layout_length);

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI
set_keyboard_layout(const struct efi_hii_database_protocol *this,
		    efi_guid_t *key_guid)
{
	EFI_ENTRY("%p, %pUl", this, key_guid);

	return EFI_EXIT(EFI_NOT_FOUND);
}

static efi_status_t EFIAPI
get_package_list_handle(const struct efi_hii_database_protocol *this,
			efi_hii_handle_t package_list_handle,
			efi_handle_t *driver_handle)
{
	struct efi_hii_packagelist *hii;

	EFI_ENTRY("%p, %p, %p", this, package_list_handle, driver_handle);

	if (!driver_handle)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	list_for_each_entry(hii, &efi_package_lists, link) {
		if (hii == package_list_handle) {
			*driver_handle = hii->driver_handle;
			return EFI_EXIT(EFI_SUCCESS);
		}
	}

	return EFI_EXIT(EFI_NOT_FOUND);
}

const struct efi_hii_database_protocol efi_hii_database = {
	.new_package_list = new_package_list,
	.remove_package_list = remove_package_list,
	.update_package_list = update_package_list,
	.list_package_lists = list_package_lists,
	.export_package_lists = export_package_lists,
	.register_package_notify = register_package_notify,
	.unregister_package_notify = unregister_package_notify,
	.find_keyboard_layouts = find_keyboard_layouts,
	.get_keyboard_layout = get_keyboard_layout,
	.set_keyboard_layout = set_keyboard_layout,
	.get_package_list_handle = get_package_list_handle
};

/*
 * EFI_HII_STRING_PROTOCOL
 */

static bool language_match(char *language, char *languages)
{
	size_t n;

	n = strlen(language);
	/* match primary language? */
	if (!strncasecmp(language, languages, n) &&
	    (languages[n] == ';' || languages[n] == '\0'))
		return true;

	return false;
}

static efi_status_t EFIAPI
new_string(const struct efi_hii_string_protocol *this,
	   efi_hii_handle_t package_list,
	   efi_string_id_t *string_id,
	   const u8 *language,
	   const u16 *language_name,
	   const efi_string_t string,
	   const struct efi_font_info *string_font_info)
{
	struct efi_hii_packagelist *hii = package_list;
	struct efi_string_table *stbl;

	EFI_ENTRY("%p, %p, %p, \"%s\", %p, \"%ls\", %p", this, package_list,
		  string_id, language, language_name, string,
		  string_font_info);

	if (!package_list || !efi_hii_packagelist_exists(package_list))
		return EFI_EXIT(EFI_NOT_FOUND);

	if (!string_id || !language || !string)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	list_for_each_entry(stbl, &hii->string_tables, link) {
		if (language_match((char *)language, stbl->language)) {
			efi_string_id_t new_id;
			void *buf;
			efi_string_t str;

			new_id = ++hii->max_string_id;
			if (stbl->nstrings < new_id) {
				buf = realloc(stbl->strings,
					      sizeof(stbl->strings[0])
						* new_id);
				if (!buf)
					return EFI_EXIT(EFI_OUT_OF_RESOURCES);

				memset(&stbl->strings[stbl->nstrings], 0,
				       (new_id - stbl->nstrings)
					 * sizeof(stbl->strings[0]));
				stbl->strings = buf;
				stbl->nstrings = new_id;
			}

			str = u16_strdup(string);
			if (!str)
				return EFI_EXIT(EFI_OUT_OF_RESOURCES);

			stbl->strings[new_id - 1].string = str;
			*string_id = new_id;

			return EFI_EXIT(EFI_SUCCESS);
		}
	}

	return EFI_EXIT(EFI_NOT_FOUND);
}

static efi_status_t EFIAPI
get_string(const struct efi_hii_string_protocol *this,
	   const u8 *language,
	   efi_hii_handle_t package_list,
	   efi_string_id_t string_id,
	   efi_string_t string,
	   efi_uintn_t *string_size,
	   struct efi_font_info **string_font_info)
{
	struct efi_hii_packagelist *hii = package_list;
	struct efi_string_table *stbl;

	EFI_ENTRY("%p, \"%s\", %p, %u, %p, %p, %p", this, language,
		  package_list, string_id, string, string_size,
		  string_font_info);

	if (!package_list || !efi_hii_packagelist_exists(package_list))
		return EFI_EXIT(EFI_NOT_FOUND);

	list_for_each_entry(stbl, &hii->string_tables, link) {
		if (language_match((char *)language, stbl->language)) {
			efi_string_t str;
			size_t len;

			if (stbl->nstrings < string_id)
				return EFI_EXIT(EFI_NOT_FOUND);

			str = stbl->strings[string_id - 1].string;
			if (str) {
				len = (u16_strlen(str) + 1) * sizeof(u16);
				if (*string_size < len) {
					*string_size = len;

					return EFI_EXIT(EFI_BUFFER_TOO_SMALL);
				}
				memcpy(string, str, len);
				*string_size = len;
			} else {
				return EFI_EXIT(EFI_NOT_FOUND);
			}

			return EFI_EXIT(EFI_SUCCESS);
		}
	}

	return EFI_EXIT(EFI_NOT_FOUND);
}

static efi_status_t EFIAPI
set_string(const struct efi_hii_string_protocol *this,
	   efi_hii_handle_t package_list,
	   efi_string_id_t string_id,
	   const u8 *language,
	   const efi_string_t string,
	   const struct efi_font_info *string_font_info)
{
	struct efi_hii_packagelist *hii = package_list;
	struct efi_string_table *stbl;

	EFI_ENTRY("%p, %p, %u, \"%s\", \"%ls\", %p", this, package_list,
		  string_id, language, string, string_font_info);

	if (!package_list || !efi_hii_packagelist_exists(package_list))
		return EFI_EXIT(EFI_NOT_FOUND);

	if (string_id > hii->max_string_id)
		return EFI_EXIT(EFI_NOT_FOUND);

	if (!string || !language)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	list_for_each_entry(stbl, &hii->string_tables, link) {
		if (language_match((char *)language, stbl->language)) {
			efi_string_t str;

			if (hii->max_string_id < string_id)
				return EFI_EXIT(EFI_NOT_FOUND);

			if (stbl->nstrings < string_id) {
				void *buf;

				buf = realloc(stbl->strings,
					      string_id
						* sizeof(stbl->strings[0]));
				if (!buf)
					return EFI_EXIT(EFI_OUT_OF_RESOURCES);

				memset(&stbl->strings[string_id - 1], 0,
				       (string_id - stbl->nstrings)
					 * sizeof(stbl->strings[0]));
				stbl->strings = buf;
			}

			str = u16_strdup(string);
			if (!str)
				return EFI_EXIT(EFI_OUT_OF_RESOURCES);

			free(stbl->strings[string_id - 1].string);
			stbl->strings[string_id - 1].string = str;

			return EFI_EXIT(EFI_SUCCESS);
		}
	}

	return EFI_EXIT(EFI_NOT_FOUND);
}

static efi_status_t EFIAPI
get_languages(const struct efi_hii_string_protocol *this,
	      efi_hii_handle_t package_list,
	      u8 *languages,
	      efi_uintn_t *languages_size)
{
	struct efi_hii_packagelist *hii = package_list;
	struct efi_string_table *stbl;
	size_t len = 0;
	char *p;

	EFI_ENTRY("%p, %p, %p, %p", this, package_list, languages,
		  languages_size);

	if (!package_list || !efi_hii_packagelist_exists(package_list))
		return EFI_EXIT(EFI_NOT_FOUND);

	if (!languages_size ||
	    (*languages_size && !languages))
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	/* figure out required size: */
	list_for_each_entry(stbl, &hii->string_tables, link) {
		len += strlen((char *)stbl->language) + 1;
	}

	if (*languages_size < len) {
		*languages_size = len;

		return EFI_EXIT(EFI_BUFFER_TOO_SMALL);
	}

	p = (char *)languages;
	list_for_each_entry(stbl, &hii->string_tables, link) {
		if (p != (char *)languages)
			*p++ = ';';
		strcpy(p, stbl->language);
		p += strlen((char *)stbl->language);
	}
	*p = '\0';

	EFI_PRINT("languages: %s\n", languages);

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI
get_secondary_languages(const struct efi_hii_string_protocol *this,
			efi_hii_handle_t package_list,
			const u8 *primary_language,
			u8 *secondary_languages,
			efi_uintn_t *secondary_languages_size)
{
	struct efi_hii_packagelist *hii = package_list;
	struct efi_string_table *stbl;
	bool found = false;

	EFI_ENTRY("%p, %p, \"%s\", %p, %p", this, package_list,
		  primary_language, secondary_languages,
		  secondary_languages_size);

	if (!package_list || !efi_hii_packagelist_exists(package_list))
		return EFI_EXIT(EFI_NOT_FOUND);

	if (!secondary_languages_size ||
	    (*secondary_languages_size && !secondary_languages))
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	list_for_each_entry(stbl, &hii->string_tables, link) {
		if (language_match((char *)primary_language, stbl->language)) {
			found = true;
			break;
		}
	}
	if (!found)
		return EFI_EXIT(EFI_INVALID_LANGUAGE);

	/*
	 * TODO: What is secondary language?
	 * *secondary_languages = '\0';
	 * *secondary_languages_size = 0;
	 */

	return EFI_EXIT(EFI_NOT_FOUND);
}

const struct efi_hii_string_protocol efi_hii_string = {
	.new_string = new_string,
	.get_string = get_string,
	.set_string = set_string,
	.get_languages = get_languages,
	.get_secondary_languages = get_secondary_languages
};
