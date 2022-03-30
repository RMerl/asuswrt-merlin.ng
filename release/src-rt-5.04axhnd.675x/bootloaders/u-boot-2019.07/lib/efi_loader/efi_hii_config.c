// SPDX-License-Identifier:     GPL-2.0+
/*
 *  EFI Human Interface Infrastructure ... Configuration
 *
 *  Copyright (c) 2017 Leif Lindholm
 *  Copyright (c) 2018 AKASHI Takahiro, Linaro Limited
 */

#include <common.h>
#include <efi_loader.h>

const efi_guid_t efi_guid_hii_config_routing_protocol
		= EFI_HII_CONFIG_ROUTING_PROTOCOL_GUID;
const efi_guid_t efi_guid_hii_config_access_protocol
		= EFI_HII_CONFIG_ACCESS_PROTOCOL_GUID;

/*
 * EFI_HII_CONFIG_ROUTING_PROTOCOL
 */

static efi_status_t EFIAPI
extract_config(const struct efi_hii_config_routing_protocol *this,
	       const efi_string_t request,
	       efi_string_t *progress,
	       efi_string_t *results)
{
	EFI_ENTRY("%p, \"%ls\", %p, %p", this, request, progress, results);

	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
}

static efi_status_t EFIAPI
export_config(const struct efi_hii_config_routing_protocol *this,
	      efi_string_t *results)
{
	EFI_ENTRY("%p, %p", this, results);

	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
}

static efi_status_t EFIAPI
route_config(const struct efi_hii_config_routing_protocol *this,
	     const efi_string_t configuration,
	     efi_string_t *progress)
{
	EFI_ENTRY("%p, \"%ls\", %p", this, configuration, progress);

	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
}

static efi_status_t EFIAPI
block_to_config(const struct efi_hii_config_routing_protocol *this,
		const efi_string_t config_request,
		const u8 *block,
		const efi_uintn_t block_size,
		efi_string_t *config,
		efi_string_t *progress)
{
	EFI_ENTRY("%p, \"%ls\", %p, %zu, %p, %p", this, config_request,
		  block, block_size, config, progress);

	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
}

static efi_status_t EFIAPI
config_to_block(const struct efi_hii_config_routing_protocol *this,
		const efi_string_t config_resp,
		const u8 *block,
		const efi_uintn_t *block_size,
		efi_string_t *progress)
{
	EFI_ENTRY("%p, \"%ls\", %p, %p, %p", this, config_resp,
		  block, block_size, progress);

	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
}

static efi_status_t EFIAPI
get_alt_config(const struct efi_hii_config_routing_protocol *this,
	       const efi_string_t config_resp,
	       const efi_guid_t *guid,
	       const efi_string_t name,
	       const struct efi_device_path *device_path,
	       const efi_string_t alt_cfg_id,
	       efi_string_t *alt_cfg_resp)
{
	EFI_ENTRY("%p, \"%ls\", %pUl, \"%ls\", %p, \"%ls\", %p",
		  this, config_resp, guid, name, device_path,
		  alt_cfg_id, alt_cfg_resp);

	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
}

/*
 * EFI_HII_ACCESS_PROTOCOL
 */

efi_status_t EFIAPI
extract_config_access(const struct efi_hii_config_access_protocol *this,
		      const efi_string_t request,
		      efi_string_t *progress,
		      efi_string_t *results)
{
	EFI_ENTRY("%p, \"%ls\", %p, %p", this, request, progress, results);

	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
};

efi_status_t EFIAPI
route_config_access(const struct efi_hii_config_access_protocol *this,
		    const efi_string_t configuration,
		    efi_string_t *progress)
{
	EFI_ENTRY("%p, \"%ls\", %p", this, configuration, progress);

	return EFI_EXIT(EFI_OUT_OF_RESOURCES);
};

efi_status_t EFIAPI
form_callback(const struct efi_hii_config_access_protocol *this,
	      efi_browser_action_t action,
	      efi_question_id_t question_id,
	      u8 type,
	      union efi_ifr_type_value *value,
	      efi_browser_action_request_t *action_request)
{
	EFI_ENTRY("%p, 0x%zx, 0x%x, 0x%x, %p, %p", this, action,
		  question_id, type, value, action_request);

	return EFI_EXIT(EFI_DEVICE_ERROR);
};

const struct efi_hii_config_routing_protocol efi_hii_config_routing = {
	.extract_config = extract_config,
	.export_config = export_config,
	.route_config = route_config,
	.block_to_config = block_to_config,
	.config_to_block = config_to_block,
	.get_alt_config = get_alt_config
};

const struct efi_hii_config_access_protocol efi_hii_config_access = {
	.extract_config_access = extract_config_access,
	.route_config_access = route_config_access,
	.form_callback = form_callback
};
