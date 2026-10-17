/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Defines data structures and APIs that allow an OS to interact with UEFI
 * firmware to query information about the device
 *
 * This file refers the following TCG specification.
 *  - TCG PC Client Platform Firmware Profile Specification
 *    https://trustedcomputinggroup.org/resource/pc-client-specific-platform-firmware-profile-specification/
 *
 *  - TCG EFI Protocol Specification
 *    https://trustedcomputinggroup.org/resource/tcg-efi-protocol-specification/
 *
 * Copyright (c) 2020, Linaro Limited
 */

#if !defined _EFI_TCG2_PROTOCOL_H_
#define _EFI_TCG2_PROTOCOL_H_

#include <efi_api.h>
#include <tpm-v2.h>

/* TPMV2 only */
#define TCG2_EVENT_LOG_FORMAT_TCG_2 0x00000002
#define EFI_TCG2_EXTEND_ONLY 0x0000000000000001
#define PE_COFF_IMAGE 0x0000000000000010

#define EFI_TCG2_MAX_PCR_INDEX 23

/* Algorithm Registry */
#define EFI_TCG2_BOOT_HASH_ALG_SHA1    0x00000001
#define EFI_TCG2_BOOT_HASH_ALG_SHA256  0x00000002
#define EFI_TCG2_BOOT_HASH_ALG_SHA384  0x00000004
#define EFI_TCG2_BOOT_HASH_ALG_SHA512  0x00000008
#define EFI_TCG2_BOOT_HASH_ALG_SM3_256 0x00000010

#define EFI_TCG2_FINAL_EVENTS_TABLE_VERSION 1

#define TPM2_EVENT_LOG_SIZE CONFIG_EFI_TCG2_PROTOCOL_EVENTLOG_SIZE

typedef u32 efi_tcg_event_log_bitmap;
typedef u32 efi_tcg_event_log_format;
typedef u32 efi_tcg_event_algorithm_bitmap;

/**
 * struct tdEFI_TCG2_VERSION - structure of EFI TCG2 version
 * @major:	major version
 * @minor:	minor version
 */
struct efi_tcg2_version {
	u8 major;
	u8 minor;
};

/**
 * struct tdEFI_TCG2_EVENT_HEADER - structure of EFI TCG2 event header
 * @header_size:	size of the event header
 * @header_version:	header version
 * @pcr_index:		index of the PCR that is extended
 * @event_type:		type of the event that is extended
 */
struct efi_tcg2_event_header {
	u32 header_size;
	u16 header_version;
	u32 pcr_index;
	u32 event_type;
} __packed;

/**
 * struct tdEFI_TCG2_EVENT - structure of EFI TCG2 event
 * @size:	total size of the event including the size component, the header
 *		and the event data
 * @header:	event header
 * @event:	event to add
 */
struct efi_tcg2_event {
	u32 size;
	struct efi_tcg2_event_header header;
	u8 event[];
} __packed;

/**
 * struct tdUEFI_IMAGE_LOAD_EVENT - structure of PE/COFF image measurement
 * @image_location_in_memory:	image address
 * @image_length_in_memory:	image size
 * @image_link_time_address:	image link time address
 * @length_of_device_path:	devive path size
 * @device_path:		device path
 */
struct uefi_image_load_event {
	efi_physical_addr_t image_location_in_memory;
	u64 image_length_in_memory;
	u64 image_link_time_address;
	u64 length_of_device_path;
	struct efi_device_path device_path[];
};

/**
 * struct tdEFI_TCG2_BOOT_SERVICE_CAPABILITY - protocol capability information
 * @size:			allocated size of the structure
 * @structure_version:		version of this structure
 * @protocol_version:		version of the EFI TCG2 protocol.
 * @hash_algorithm_bitmap:	supported hash algorithms
 * @supported_event_logs:	bitmap of supported event log formats
 * @tpm_present_flag:		false = TPM not present
 * @max_command_size:		max size (in bytes) of a command
 *				that can be sent to the TPM
 * @max_response_size:		max size (in bytes) of a response that
 *				can be provided by the TPM
 * @manufacturer_id:		4-byte Vendor ID
 * @number_of_pcr_banks:	maximum number of PCR banks
 * @active_pcr_banks:		bitmap of currently active
 *				PCR banks (hashing algorithms).
 */
struct efi_tcg2_boot_service_capability {
	u8 size;
	struct efi_tcg2_version structure_version;
	struct efi_tcg2_version protocol_version;
	efi_tcg_event_algorithm_bitmap hash_algorithm_bitmap;
	efi_tcg_event_log_bitmap supported_event_logs;
	u8 tpm_present_flag;
	u16 max_command_size;
	u16 max_response_size;
	u32 manufacturer_id;
	u32 number_of_pcr_banks;
	efi_tcg_event_algorithm_bitmap active_pcr_banks;
};

/* up to and including the vendor ID (manufacturer_id) field */
#define BOOT_SERVICE_CAPABILITY_MIN \
	offsetof(struct efi_tcg2_boot_service_capability, number_of_pcr_banks)

/**
 * struct tdEFI_TCG2_FINAL_EVENTS_TABLE - log entries after Get Event Log
 * @version:		version number for this structure
 * @number_of_events:	number of events recorded after invocation of
 *			GetEventLog()
 * @event:		List of events of type tcg_pcr_event2
 */
struct efi_tcg2_final_events_table {
	u64 version;
	u64 number_of_events;
	struct tcg_pcr_event2 event[];
};

/**
 * struct tdUEFI_VARIABLE_DATA - event log structure of UEFI variable
 * @variable_name:		The vendorGUID parameter in the
 *				GetVariable() API.
 * @unicode_name_length:	The length in CHAR16 of the Unicode name of
 *				the variable.
 * @variable_data_length:	The size of the variable data.
 * @unicode_name:		The CHAR16 unicode name of the variable
 *				without NULL-terminator.
 * @variable_data:		The data parameter of the efi variable
 *				in the GetVariable() API.
 */
struct efi_tcg2_uefi_variable_data {
	efi_guid_t variable_name;
	u64 unicode_name_length;
	u64 variable_data_length;
	u16 unicode_name[1];
	u8 variable_data[1];
};

/**
 * struct tdUEFI_HANDOFF_TABLE_POINTERS2 - event log structure of SMBOIS tables
 * @table_description_size:	size of table description
 * @table_description:		table description
 * @number_of_tables:		number of uefi configuration table
 * @table_entry:		uefi configuration table entry
 */
#define SMBIOS_HANDOFF_TABLE_DESC  "SmbiosTable"
struct smbios_handoff_table_pointers2 {
	u8 table_description_size;
	u8 table_description[sizeof(SMBIOS_HANDOFF_TABLE_DESC)];
	u64 number_of_tables;
	struct efi_configuration_table table_entry[];
} __packed;

/**
 * struct tdUEFI_GPT_DATA - event log structure of industry standard tables
 * @uefi_partition_header:	gpt partition header
 * @number_of_partitions:	the number of partition
 * @partitions:			partition entries
 */
struct efi_gpt_data {
	gpt_header uefi_partition_header;
	u64 number_of_partitions;
	gpt_entry partitions[];
} __packed;

/**
 * struct tdUEFI_PLATFORM_FIRMWARE_BLOB2
 * @blob_description_size:	Byte size of @data
 * @data:			Description data
 */
struct uefi_platform_firmware_blob2 {
	u8 blob_description_size;
	u8 data[];
} __packed;

struct efi_tcg2_protocol {
	efi_status_t (EFIAPI * get_capability)(struct efi_tcg2_protocol *this,
					       struct efi_tcg2_boot_service_capability *capability);
	efi_status_t (EFIAPI * get_eventlog)(struct efi_tcg2_protocol *this,
					     efi_tcg_event_log_format log_format,
					     u64 *event_log_location, u64 *event_log_last_entry,
					     bool *event_log_truncated);
	efi_status_t (EFIAPI * hash_log_extend_event)(struct efi_tcg2_protocol *this,
						      u64 flags,
						      efi_physical_addr_t data_to_hash,
						      u64 data_to_hash_len,
						      struct efi_tcg2_event *efi_tcg_event);
	efi_status_t (EFIAPI * submit_command)(struct efi_tcg2_protocol *this,
					       u32 input_parameter_block_size,
					       u8 *input_parameter_block,
					       u32 output_parameter_block_size,
					       u8 *output_parameter_block);
	efi_status_t (EFIAPI * get_active_pcr_banks)(struct efi_tcg2_protocol *this,
						     u32 *active_pcr_banks);
	efi_status_t (EFIAPI * set_active_pcr_banks)(struct efi_tcg2_protocol *this,
						     u32 active_pcr_banks);
	efi_status_t (EFIAPI * get_result_of_set_active_pcr_banks)(struct efi_tcg2_protocol *this,
								   u32 *operation_present,
								   u32 *response);
};
#endif
