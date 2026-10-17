/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Extensible Firmware Interface
 * Based on 'Extensible Firmware Interface Specification' version 0.9,
 * April 30, 1999
 *
 * Copyright (C) 1999 VA Linux Systems
 * Copyright (C) 1999 Walt Drummond <drummond@valinux.com>
 * Copyright (C) 1999, 2002-2003 Hewlett-Packard Co.
 *	David Mosberger-Tang <davidm@hpl.hp.com>
 *	Stephane Eranian <eranian@hpl.hp.com>
 *
 * From include/linux/efi.h in kernel 4.1 with some additions/subtractions
 */

#ifndef _EFI_API_H
#define _EFI_API_H

#include <efi.h>
#include <charset.h>
#include <pe.h>

/* UEFI spec version 2.9 */
#define EFI_SPECIFICATION_VERSION (2 << 16 | 100)

/* Types and defines for EFI CreateEvent */
enum efi_timer_delay {
	EFI_TIMER_STOP = 0,
	EFI_TIMER_PERIODIC = 1,
	EFI_TIMER_RELATIVE = 2
};

typedef void *efi_hii_handle_t;
typedef u16 *efi_string_t;
typedef u16 efi_string_id_t;
typedef u32 efi_hii_font_style_t;
typedef u16 efi_question_id_t;
typedef u16 efi_image_id_t;
typedef u16 efi_form_id_t;

#define EVT_TIMER				0x80000000
#define EVT_RUNTIME				0x40000000
#define EVT_NOTIFY_WAIT				0x00000100
#define EVT_NOTIFY_SIGNAL			0x00000200
#define EVT_SIGNAL_EXIT_BOOT_SERVICES		0x00000201
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE	0x60000202

#define TPL_APPLICATION		0x04
#define TPL_CALLBACK		0x08
#define TPL_NOTIFY		0x10
#define TPL_HIGH_LEVEL		0x1F

struct efi_event;

/* OsIndicationsSupported flags */
#define EFI_OS_INDICATIONS_BOOT_TO_FW_UI		    0x0000000000000001
#define EFI_OS_INDICATIONS_TIMESTAMP_REVOCATION		    0x0000000000000002
#define EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED  0x0000000000000004
#define EFI_OS_INDICATIONS_FMP_CAPSULE_SUPPORTED	    0x0000000000000008
#define EFI_OS_INDICATIONS_CAPSULE_RESULT_VAR_SUPPORTED	    0x0000000000000010
#define EFI_OS_INDICATIONS_START_OS_RECOVERY		    0x0000000000000020
#define EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY	    0x0000000000000040
#define EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH	    0x0000000000000080

/* EFI Boot Services table */
#define EFI_BOOT_SERVICES_SIGNATURE 0x56524553544f4f42
struct efi_boot_services {
	struct efi_table_hdr hdr;
	efi_status_t (EFIAPI *raise_tpl)(efi_uintn_t new_tpl);
	void (EFIAPI *restore_tpl)(efi_uintn_t old_tpl);

	efi_status_t (EFIAPI *allocate_pages)(int, int, efi_uintn_t,
					      efi_physical_addr_t *);
	efi_status_t (EFIAPI *free_pages)(efi_physical_addr_t, efi_uintn_t);
	efi_status_t (EFIAPI *get_memory_map)(efi_uintn_t *memory_map_size,
					      struct efi_mem_desc *desc,
					      efi_uintn_t *key,
					      efi_uintn_t *desc_size,
					      u32 *desc_version);
	efi_status_t (EFIAPI *allocate_pool)(int, efi_uintn_t, void **);
	efi_status_t (EFIAPI *free_pool)(void *);

	efi_status_t (EFIAPI *create_event)(uint32_t type,
			efi_uintn_t notify_tpl,
			void (EFIAPI *notify_function) (
					struct efi_event *event,
					void *context),
			void *notify_context, struct efi_event **event);
	efi_status_t (EFIAPI *set_timer)(struct efi_event *event,
					 enum efi_timer_delay type,
					 uint64_t trigger_time);
	efi_status_t (EFIAPI *wait_for_event)(efi_uintn_t number_of_events,
					      struct efi_event **event,
					      efi_uintn_t *index);
	efi_status_t (EFIAPI *signal_event)(struct efi_event *event);
	efi_status_t (EFIAPI *close_event)(struct efi_event *event);
	efi_status_t (EFIAPI *check_event)(struct efi_event *event);
#define EFI_NATIVE_INTERFACE	0x00000000
	efi_status_t (EFIAPI *install_protocol_interface)(
			efi_handle_t *handle, const efi_guid_t *protocol,
			int protocol_interface_type, void *protocol_interface);
	efi_status_t (EFIAPI *reinstall_protocol_interface)(
			efi_handle_t handle, const efi_guid_t *protocol,
			void *old_interface, void *new_interface);
	efi_status_t (EFIAPI *uninstall_protocol_interface)(
			efi_handle_t handle, const efi_guid_t *protocol,
			void *protocol_interface);
	efi_status_t (EFIAPI *handle_protocol)(
			efi_handle_t handle, const efi_guid_t *protocol,
			void **protocol_interface);
	void *reserved;
	efi_status_t (EFIAPI *register_protocol_notify)(
			const efi_guid_t *protocol, struct efi_event *event,
			void **registration);
	efi_status_t (EFIAPI *locate_handle)(
			enum efi_locate_search_type search_type,
			const efi_guid_t *protocol, void *search_key,
			efi_uintn_t *buffer_size, efi_handle_t *buffer);
	efi_status_t (EFIAPI *locate_device_path)(const efi_guid_t *protocol,
			struct efi_device_path **device_path,
			efi_handle_t *device);
	efi_status_t (EFIAPI *install_configuration_table)(
			const efi_guid_t *guid, void *table);

	efi_status_t (EFIAPI *load_image)(bool boot_policiy,
			efi_handle_t parent_image,
			struct efi_device_path *file_path, void *source_buffer,
			efi_uintn_t source_size, efi_handle_t *image);
	efi_status_t (EFIAPI *start_image)(efi_handle_t handle,
					   efi_uintn_t *exitdata_size,
					   u16 **exitdata);
	efi_status_t (EFIAPI *exit)(efi_handle_t handle,
				    efi_status_t exit_status,
				    efi_uintn_t exitdata_size, u16 *exitdata);
	efi_status_t (EFIAPI *unload_image)(efi_handle_t image_handle);
	efi_status_t (EFIAPI *exit_boot_services)(efi_handle_t image_handle,
						  efi_uintn_t map_key);

	efi_status_t (EFIAPI *get_next_monotonic_count)(u64 *count);
	efi_status_t (EFIAPI *stall)(unsigned long usecs);
	efi_status_t (EFIAPI *set_watchdog_timer)(unsigned long timeout,
			uint64_t watchdog_code, unsigned long data_size,
			uint16_t *watchdog_data);
	efi_status_t(EFIAPI *connect_controller)(efi_handle_t controller_handle,
			efi_handle_t *driver_image_handle,
			struct efi_device_path *remaining_device_path,
			bool recursive);
	efi_status_t (EFIAPI *disconnect_controller)(
			efi_handle_t controller_handle,
			efi_handle_t driver_image_handle,
			efi_handle_t child_handle);
#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL        0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL       0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER           0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE           0x00000020
	efi_status_t (EFIAPI *open_protocol)(efi_handle_t handle,
			const efi_guid_t *protocol, void **interface,
			efi_handle_t agent_handle,
			efi_handle_t controller_handle, u32 attributes);
	efi_status_t (EFIAPI *close_protocol)(
			efi_handle_t handle, const efi_guid_t *protocol,
			efi_handle_t agent_handle,
			efi_handle_t controller_handle);
	efi_status_t(EFIAPI *open_protocol_information)(efi_handle_t handle,
			const efi_guid_t *protocol,
			struct efi_open_protocol_info_entry **entry_buffer,
			efi_uintn_t *entry_count);
	efi_status_t (EFIAPI *protocols_per_handle)(efi_handle_t handle,
			efi_guid_t ***protocol_buffer,
			efi_uintn_t *protocols_buffer_count);
	efi_status_t (EFIAPI *locate_handle_buffer) (
			enum efi_locate_search_type search_type,
			const efi_guid_t *protocol, void *search_key,
			efi_uintn_t *no_handles, efi_handle_t **buffer);
	efi_status_t (EFIAPI *locate_protocol)(const efi_guid_t *protocol,
			void *registration, void **protocol_interface);
	efi_status_t (EFIAPI *install_multiple_protocol_interfaces)(
			efi_handle_t *handle, ...);
	efi_status_t (EFIAPI *uninstall_multiple_protocol_interfaces)(
			efi_handle_t handle, ...);
	efi_status_t (EFIAPI *calculate_crc32)(const void *data,
					       efi_uintn_t data_size,
					       u32 *crc32);
	void (EFIAPI *copy_mem)(void *destination, const void *source,
			size_t length);
	void (EFIAPI *set_mem)(void *buffer, size_t size, uint8_t value);
	efi_status_t (EFIAPI *create_event_ex)(
				uint32_t type, efi_uintn_t notify_tpl,
				void (EFIAPI *notify_function) (
					struct efi_event *event,
					void *context),
				void *notify_context,
				const efi_guid_t *event_group,
				struct efi_event **event);
};

/* Types and defines for EFI ResetSystem */
enum efi_reset_type {
	EFI_RESET_COLD = 0,
	EFI_RESET_WARM = 1,
	EFI_RESET_SHUTDOWN = 2,
	EFI_RESET_PLATFORM_SPECIFIC = 3,
};

/* EFI Runtime Services table */
#define EFI_RUNTIME_SERVICES_SIGNATURE	0x56524553544e5552ULL

#define CAPSULE_FLAGS_PERSIST_ACROSS_RESET	0x00010000
#define CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE	0x00020000
#define CAPSULE_FLAGS_INITIATE_RESET		0x00040000

#define CAPSULE_SUPPORT_AUTHENTICATION		0x0000000000000001
#define CAPSULE_SUPPORT_DEPENDENCY		0x0000000000000002

#define EFI_CAPSULE_REPORT_GUID \
	EFI_GUID(0x39b68c46, 0xf7fb, 0x441b, 0xb6, 0xec, \
		 0x16, 0xb0, 0xf6, 0x98, 0x21, 0xf3)

#define EFI_MEMORY_RANGE_CAPSULE_GUID \
	EFI_GUID(0xde9f0ec, 0x88b6, 0x428f, 0x97, 0x7a, \
		 0x25, 0x8f, 0x1d, 0xe, 0x5e, 0x72)

#define EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID \
	EFI_GUID(0x6dcbd5ed, 0xe82d, 0x4c44, 0xbd, 0xa1, \
		 0x71, 0x94, 0x19, 0x9a, 0xd9, 0x2a)

#define EFI_CONFORMANCE_PROFILES_TABLE_GUID \
	EFI_GUID(0x36122546, 0xf7ef, 0x4c8f, 0xbd, 0x9b, \
		 0xeb, 0x85, 0x25, 0xb5, 0x0c, 0x0b)

#define EFI_CONFORMANCE_PROFILES_TABLE_VERSION 1

#define EFI_CONFORMANCE_PROFILE_EBBR_2_1_GUID \
	EFI_GUID(0xcce33c35, 0x74ac, 0x4087, 0xbc, 0xe7, \
		 0x8b, 0x29, 0xb0, 0x2e, 0xeb, 0x27)

struct efi_conformance_profiles_table {
	u16 version;
	u16 number_of_profiles;
	efi_guid_t	conformance_profiles[];
} __packed;

struct efi_capsule_header {
	efi_guid_t capsule_guid;
	u32 header_size;
	u32 flags;
	u32 capsule_image_size;
} __packed;

struct efi_capsule_result_variable_header {
	u32 variable_total_size;
	u32 reserved;
	efi_guid_t capsule_guid;
	struct efi_time capsule_processed;
	efi_status_t capsule_status;
} __packed;

struct efi_memory_range {
	efi_physical_addr_t	address;
	u64			length;
};

struct efi_memory_range_capsule {
	struct efi_capsule_header *header;
	/* EFI_MEMORY_TYPE: 0x80000000-0xFFFFFFFF */
	enum efi_memory_type os_requested_memory_type;
	u64 number_of_memory_ranges;
	struct efi_memory_range memory_ranges[];
} __packed;

struct efi_firmware_management_capsule_header {
	u32 version;
	u16 embedded_driver_count;
	u16 payload_item_count;
	u64 item_offset_list[];
} __packed;

struct efi_firmware_management_capsule_image_header {
	u32 version;
	efi_guid_t update_image_type_id;
	u8 update_image_index;
	u8 reserved[3];
	u32 update_image_size;
	u32 update_vendor_code_size;
	u64 update_hardware_instance;
	u64 image_capsule_support;
} __packed;

struct efi_capsule_result_variable_fmp {
	u16 version;
	u8 payload_index;
	u8 update_image_index;
	efi_guid_t update_image_type_id;
	// u16 capsule_file_name[];
	// u16 capsule_target[];
} __packed;

#define EFI_RT_SUPPORTED_GET_TIME			0x0001
#define EFI_RT_SUPPORTED_SET_TIME			0x0002
#define EFI_RT_SUPPORTED_GET_WAKEUP_TIME		0x0004
#define EFI_RT_SUPPORTED_SET_WAKEUP_TIME		0x0008
#define EFI_RT_SUPPORTED_GET_VARIABLE			0x0010
#define EFI_RT_SUPPORTED_GET_NEXT_VARIABLE_NAME		0x0020
#define EFI_RT_SUPPORTED_SET_VARIABLE			0x0040
#define EFI_RT_SUPPORTED_SET_VIRTUAL_ADDRESS_MAP	0x0080
#define EFI_RT_SUPPORTED_CONVERT_POINTER		0x0100
#define EFI_RT_SUPPORTED_GET_NEXT_HIGH_MONOTONIC_COUNT	0x0200
#define EFI_RT_SUPPORTED_RESET_SYSTEM			0x0400
#define EFI_RT_SUPPORTED_UPDATE_CAPSULE			0x0800
#define EFI_RT_SUPPORTED_QUERY_CAPSULE_CAPABILITIES	0x1000
#define EFI_RT_SUPPORTED_QUERY_VARIABLE_INFO		0x2000

#define EFI_RT_PROPERTIES_TABLE_GUID \
	EFI_GUID(0xeb66918a, 0x7eef, 0x402a, 0x84, 0x2e, \
		 0x93, 0x1d, 0x21, 0xc3, 0x8a, 0xe9)

#define EFI_RT_PROPERTIES_TABLE_VERSION	0x1

struct efi_rt_properties_table {
	u16 version;
	u16 length;
	u32 runtime_services_supported;
};

#define EFI_OPTIONAL_PTR	0x00000001

struct efi_runtime_services {
	struct efi_table_hdr hdr;
	efi_status_t (EFIAPI *get_time)(struct efi_time *time,
			struct efi_time_cap *capabilities);
	efi_status_t (EFIAPI *set_time)(struct efi_time *time);
	efi_status_t (EFIAPI *get_wakeup_time)(char *enabled, char *pending,
			struct efi_time *time);
	efi_status_t (EFIAPI *set_wakeup_time)(char enabled,
			struct efi_time *time);
	efi_status_t (EFIAPI *set_virtual_address_map)(
			efi_uintn_t memory_map_size,
			efi_uintn_t descriptor_size,
			uint32_t descriptor_version,
			struct efi_mem_desc *virtmap);
	efi_status_t (EFIAPI *convert_pointer)(
			efi_uintn_t debug_disposition, void **address);
	efi_status_t (EFIAPI *get_variable)(u16 *variable_name,
					    const efi_guid_t *vendor,
					    u32 *attributes,
					    efi_uintn_t *data_size, void *data);
	efi_status_t (EFIAPI *get_next_variable_name)(
			efi_uintn_t *variable_name_size,
			u16 *variable_name, efi_guid_t *vendor);
	efi_status_t (EFIAPI *set_variable)(u16 *variable_name,
					    const efi_guid_t *vendor,
					    u32 attributes,
					    efi_uintn_t data_size,
					    const void *data);
	efi_status_t (EFIAPI *get_next_high_mono_count)(
			uint32_t *high_count);
	void (EFIAPI *reset_system)(enum efi_reset_type reset_type,
				    efi_status_t reset_status,
				    unsigned long data_size, void *reset_data);
	efi_status_t (EFIAPI *update_capsule)(
			struct efi_capsule_header **capsule_header_array,
			efi_uintn_t capsule_count,
			u64 scatter_gather_list);
	efi_status_t (EFIAPI *query_capsule_caps)(
			struct efi_capsule_header **capsule_header_array,
			efi_uintn_t capsule_count,
			u64 *maximum_capsule_size,
			u32 *reset_type);
	efi_status_t (EFIAPI *query_variable_info)(
			u32 attributes,
			u64 *maximum_variable_storage_size,
			u64 *remaining_variable_storage_size,
			u64 *maximum_variable_size);
};

/* EFI event group GUID definitions */

#define EFI_EVENT_GROUP_EXIT_BOOT_SERVICES \
	EFI_GUID(0x27abf055, 0xb1b8, 0x4c26, 0x80, 0x48, \
		 0x74, 0x8f, 0x37, 0xba, 0xa2, 0xdf)

#define EFI_EVENT_GROUP_BEFORE_EXIT_BOOT_SERVICES \
	EFI_GUID(0x8be0e274, 0x3970, 0x4b44, 0x80, 0xc5, \
		 0x1a, 0xb9, 0x50, 0x2f, 0x3b, 0xfc)

#define EFI_EVENT_GROUP_VIRTUAL_ADDRESS_CHANGE \
	EFI_GUID(0x13fa7698, 0xc831, 0x49c7, 0x87, 0xea, \
		 0x8f, 0x43, 0xfc, 0xc2, 0x51, 0x96)

#define EFI_EVENT_GROUP_MEMORY_MAP_CHANGE \
	EFI_GUID(0x78bee926, 0x692f, 0x48fd, 0x9e, 0xdb, \
		 0x01, 0x42, 0x2e, 0xf0, 0xd7, 0xab)

#define EFI_EVENT_GROUP_READY_TO_BOOT \
	EFI_GUID(0x7ce88fb3, 0x4bd7, 0x4679, 0x87, 0xa8, \
		 0xa8, 0xd8, 0xde, 0xe5, 0x0d, 0x2b)

#define EFI_EVENT_GROUP_AFTER_READY_TO_BOOT \
	EFI_GUID(0x3a2a00ad, 0x98b9, 0x4cdf, 0xa4, 0x78, \
		 0x70, 0x27, 0x77, 0xf1, 0xc1, 0xb)

#define EFI_EVENT_GROUP_RESET_SYSTEM \
	EFI_GUID(0x62da6a56, 0x13fb, 0x485a, 0xa8, 0xda, \
		 0xa3, 0xdd, 0x79, 0x12, 0xcb, 0x6b)
#define EFI_EVENT_GROUP_RETURN_TO_EFIBOOTMGR \
	EFI_GUID(0xb4a40fe6, 0x9149, 0x4f29, 0x94, 0x47, \
		 0x49, 0x38, 0x7a, 0x7f, 0xab, 0x87)

/* EFI Configuration Table and GUID definitions */
#define NULL_GUID \
	EFI_GUID(0x00000000, 0x0000, 0x0000, 0x00, 0x00, \
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)

#define EFI_GLOBAL_VARIABLE_GUID \
	EFI_GUID(0x8be4df61, 0x93ca, 0x11d2, 0xaa, 0x0d, \
		 0x00, 0xe0, 0x98, 0x03, 0x2b, 0x8c)

#define EFI_IMAGE_SECURITY_DATABASE_GUID \
	EFI_GUID(0xd719b2cb, 0x3d3a, 0x4596, 0xa3, 0xbc, \
		 0xda, 0xd0, 0x0e, 0x67, 0x65, 0x6f)

#define EFI_FDT_GUID \
	EFI_GUID(0xb1b621d5, 0xf19c, 0x41a5, \
		 0x83, 0x0b, 0xd9, 0x15, 0x2c, 0x69, 0xaa, 0xe0)

#define EFI_ACPI_TABLE_GUID \
	EFI_GUID(0x8868e871, 0xe4f1, 0x11d3, \
		 0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81)

#define SMBIOS_TABLE_GUID \
	EFI_GUID(0xeb9d2d31, 0x2d88, 0x11d3,  \
		 0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d)

#define SMBIOS3_TABLE_GUID \
	EFI_GUID(0xf2fd1544, 0x9794, 0x4a2c, \
		 0x99, 0x2e, 0xe5, 0xbb, 0xcf, 0x20, 0xe3, 0x94)

#define EFI_LOAD_FILE_PROTOCOL_GUID \
	EFI_GUID(0x56ec3091, 0x954c, 0x11d2, \
		 0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define EFI_LOAD_FILE2_PROTOCOL_GUID \
	EFI_GUID(0x4006c0c1, 0xfcb3, 0x403e, \
		 0x99, 0x6d, 0x4a, 0x6c, 0x87, 0x24, 0xe0, 0x6d)

#define EFI_TCG2_FINAL_EVENTS_TABLE_GUID \
	EFI_GUID(0x1e2ed096, 0x30e2, 0x4254, 0xbd, \
		 0x89, 0x86, 0x3b, 0xbe, 0xf8, 0x23, 0x25)

#define EFI_RNG_PROTOCOL_GUID \
	EFI_GUID(0x3152bca5, 0xeade, 0x433d, 0x86, 0x2e, \
		 0xc0, 0x1c, 0xdc, 0x29, 0x1f, 0x44)

#define EFI_DT_FIXUP_PROTOCOL_GUID \
	EFI_GUID(0xe617d64c, 0xfe08, 0x46da, 0xf4, 0xdc, \
		 0xbb, 0xd5, 0x87, 0x0c, 0x73, 0x00)

#define EFI_TCG2_PROTOCOL_GUID \
	EFI_GUID(0x607f766c, 0x7455, 0x42be, 0x93, \
		 0x0b, 0xe4, 0xd7, 0x6d, 0xb2, 0x72, 0x0f)

#define RISCV_EFI_BOOT_PROTOCOL_GUID \
	EFI_GUID(0xccd15fec, 0x6f73, 0x4eec, 0x83, \
		 0x95, 0x3e, 0x69, 0xe4, 0xb9, 0x40, 0xbf)

/**
 * struct efi_configuration_table - EFI Configuration Table
 *
 * This table contains a set of GUID/pointer pairs.
 * The EFI Configuration Table may contain at most one instance of each table type.
 *
 * @guid:		GUID that uniquely identifies the system configuration table
 * @table:		A pointer to the table associated with guid
 */
struct efi_configuration_table {
	efi_guid_t guid;
	void *table;
} __packed;

#define EFI_SYSTEM_TABLE_SIGNATURE ((u64)0x5453595320494249ULL)

/**
 * struct efi_system_table - EFI System Table
 *
 * EFI System Table contains pointers to the runtime and boot services tables.
 *
 * @hdr:		The table header for the EFI System Table
 * @fw_vendor:		A pointer to a null terminated string that identifies the vendor
 *			that produces the system firmware
 * @fw_revision:	The revision of the system firmware
 * @con_in_handle:	The handle for the active console input device
 * @con_in:		A pointer to the EFI_SIMPLE_TEXT_INPUT_PROTOCOL interface
 *			that is associated with con_in_handle
 * @con_out_handle:	The handle for the active console output device
 * @con_out:		A pointer to the EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL interface
 *			that is associated with con_out_handle
 * @stderr_handle:	The handle for the active standard error console device
 * @std_err:		A pointer to the EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL interface
 *			that is associated with stderr_handle
 * @runtime:		A pointer to the EFI Runtime Services Table
 * @boottime:		A pointer to the EFI Boot Services Table
 * @nr_tables:		The number of system configuration tables
 * @tables:		A pointer to the system configuration tables
 */
struct efi_system_table {
	struct efi_table_hdr hdr;
	u16 *fw_vendor;   /* physical addr of wchar_t vendor string */
	u32 fw_revision;
	efi_handle_t con_in_handle;
	struct efi_simple_text_input_protocol *con_in;
	efi_handle_t con_out_handle;
	struct efi_simple_text_output_protocol *con_out;
	efi_handle_t stderr_handle;
	struct efi_simple_text_output_protocol *std_err;
	struct efi_runtime_services *runtime;
	struct efi_boot_services *boottime;
	efi_uintn_t nr_tables;
	struct efi_configuration_table *tables;
};

/**
 * efi_main() - entry point of EFI applications
 *
 * @image_handle:	handle with the Loaded Image Protocol
 * @systab:		pointer to the system table
 * Return:		status code
 */
efi_status_t EFIAPI efi_main(efi_handle_t image_handle,
			     struct efi_system_table *systab);

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
	EFI_GUID(0x5b1b31a1, 0x9562, 0x11d2, \
		 0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define EFI_LOADED_IMAGE_DEVICE_PATH_PROTOCOL_GUID \
	EFI_GUID(0xbc62157e, 0x3e33, 0x4fec, \
		 0x99, 0x20, 0x2d, 0x3b, 0x36, 0xd7, 0x50, 0xdf)

#define EFI_LOADED_IMAGE_PROTOCOL_REVISION 0x1000

struct efi_loaded_image {
	u32 revision;
	void *parent_handle;
	struct efi_system_table *system_table;
	efi_handle_t device_handle;
	struct efi_device_path *file_path;
	void *reserved;
	u32 load_options_size;
	void *load_options;
	void *image_base;
	aligned_u64 image_size;
	unsigned int image_code_type;
	unsigned int image_data_type;
	efi_status_t (EFIAPI *unload)(efi_handle_t image_handle);
};

#define EFI_DEVICE_PATH_PROTOCOL_GUID \
	EFI_GUID(0x09576e91, 0x6d3f, 0x11d2, \
		 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define DEVICE_PATH_TYPE_END			0x7f
#  define DEVICE_PATH_SUB_TYPE_INSTANCE_END	0x01
#  define DEVICE_PATH_SUB_TYPE_END		0xff

struct efi_mac_addr {
	u8 addr[32];
} __packed;

#define DEVICE_PATH_TYPE_HARDWARE_DEVICE	0x01
#  define DEVICE_PATH_SUB_TYPE_MEMORY		0x03
#  define DEVICE_PATH_SUB_TYPE_VENDOR		0x04
#  define DEVICE_PATH_SUB_TYPE_CONTROLLER	0x05

struct efi_device_path_memory {
	struct efi_device_path dp;
	u32 memory_type;
	u64 start_address;
	u64 end_address;
} __packed;

struct efi_device_path_vendor {
	struct efi_device_path dp;
	efi_guid_t guid;
	u8 vendor_data[];
} __packed;

struct efi_device_path_udevice {
	struct efi_device_path dp;
	efi_guid_t guid;
	int uclass_id;
	int dev_number;
} __packed;

struct efi_device_path_controller {
	struct efi_device_path dp;
	u32 controller_number;
} __packed;

#define DEVICE_PATH_TYPE_ACPI_DEVICE		0x02
#  define DEVICE_PATH_SUB_TYPE_ACPI_DEVICE	0x01

#define EFI_PNP_ID(ID)				(u32)(((ID) << 16) | 0x41D0)
#define EISA_PNP_ID(ID)				EFI_PNP_ID(ID)
#define EISA_PNP_NUM(ID)			((ID) >> 16)

struct efi_device_path_acpi_path {
	struct efi_device_path dp;
	u32 hid;
	u32 uid;
} __packed;

#define DEVICE_PATH_TYPE_MESSAGING_DEVICE	0x03
#  define DEVICE_PATH_SUB_TYPE_MSG_ATAPI	0x01
#  define DEVICE_PATH_SUB_TYPE_MSG_SCSI		0x02
#  define DEVICE_PATH_SUB_TYPE_MSG_USB		0x05
#  define DEVICE_PATH_SUB_TYPE_MSG_MAC_ADDR	0x0b
#  define DEVICE_PATH_SUB_TYPE_MSG_UART		0x0e
#  define DEVICE_PATH_SUB_TYPE_MSG_USB_CLASS	0x0f
#  define DEVICE_PATH_SUB_TYPE_MSG_USB_WWI	0x10
#  define DEVICE_PATH_SUB_TYPE_MSG_SATA		0x12
#  define DEVICE_PATH_SUB_TYPE_MSG_NVME		0x17
#  define DEVICE_PATH_SUB_TYPE_MSG_URI		0x18
#  define DEVICE_PATH_SUB_TYPE_MSG_SD		0x1a
#  define DEVICE_PATH_SUB_TYPE_MSG_MMC		0x1d

struct efi_device_path_atapi {
	struct efi_device_path dp;
	u8 primary_secondary;
	u8 slave_master;
	u16 logical_unit_number;
} __packed;

struct efi_device_path_scsi {
	struct efi_device_path dp;
	u16 target_id;
	u16 logical_unit_number;
} __packed;

struct efi_device_path_uart {
	struct efi_device_path dp;
	u32 reserved;
	u64 baud_rate;
	u8 data_bits;
	u8 parity;
	u8 stop_bits;
} __packed;

struct efi_device_path_usb {
	struct efi_device_path dp;
	u8 parent_port_number;
	u8 usb_interface;
} __packed;

struct efi_device_path_sata {
	struct efi_device_path dp;
	u16 hba_port;
	u16 port_multiplier_port;
	u16 logical_unit_number;
} __packed;

struct efi_device_path_mac_addr {
	struct efi_device_path dp;
	struct efi_mac_addr mac;
	u8 if_type;
} __packed;

struct efi_device_path_usb_class {
	struct efi_device_path dp;
	u16 vendor_id;
	u16 product_id;
	u8 device_class;
	u8 device_subclass;
	u8 device_protocol;
} __packed;

struct efi_device_path_sd_mmc_path {
	struct efi_device_path dp;
	u8 slot_number;
} __packed;

struct efi_device_path_nvme {
	struct efi_device_path dp;
	u32 ns_id;
	u8 eui64[8];
} __packed;

struct efi_device_path_uri {
	struct efi_device_path dp;
	u8 uri[];
} __packed;

#define DEVICE_PATH_TYPE_MEDIA_DEVICE		0x04
#  define DEVICE_PATH_SUB_TYPE_HARD_DRIVE_PATH	0x01
#  define DEVICE_PATH_SUB_TYPE_CDROM_PATH	0x02
#  define DEVICE_PATH_SUB_TYPE_VENDOR_PATH	0x03
#  define DEVICE_PATH_SUB_TYPE_FILE_PATH	0x04

struct efi_device_path_hard_drive_path {
	struct efi_device_path dp;
	u32 partition_number;
	u64 partition_start;
	u64 partition_end;
	u8 partition_signature[16];
	u8 partmap_type;
	u8 signature_type;
} __packed;

struct efi_device_path_cdrom_path {
	struct efi_device_path dp;
	u32 boot_entry;
	u64 partition_start;
	u64 partition_size;
} __packed;

struct efi_device_path_file_path {
	struct efi_device_path dp;
	u16 str[];
} __packed;

#define EFI_BLOCK_IO_PROTOCOL_GUID \
	EFI_GUID(0x964e5b21, 0x6459, 0x11d2, \
		 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

struct efi_block_io_media {
	u32 media_id;
	char removable_media;
	char media_present;
	char logical_partition;
	char read_only;
	char write_caching;
	u8 pad[3];
	u32 block_size;
	u32 io_align;
	u8 pad2[4];
	u64 last_block;
	/* Added in revision 2 of the protocol */
	u64 lowest_aligned_lba;
	u32 logical_blocks_per_physical_block;
	/* Added in revision 3 of the protocol */
	u32 optimal_transfer_length_granualarity;
};

#define EFI_BLOCK_IO_PROTOCOL_REVISION2	0x00020001
#define EFI_BLOCK_IO_PROTOCOL_REVISION3	0x0002001f

struct efi_block_io {
	u64 revision;
	struct efi_block_io_media *media;
	efi_status_t (EFIAPI *reset)(struct efi_block_io *this,
			char extended_verification);
	efi_status_t (EFIAPI *read_blocks)(struct efi_block_io *this,
			u32 media_id, u64 lba, efi_uintn_t buffer_size,
			void *buffer);
	efi_status_t (EFIAPI *write_blocks)(struct efi_block_io *this,
			u32 media_id, u64 lba, efi_uintn_t buffer_size,
			void *buffer);
	efi_status_t (EFIAPI *flush_blocks)(struct efi_block_io *this);
};

struct simple_text_output_mode {
	s32 max_mode;
	s32 mode;
	s32 attribute;
	s32 cursor_column;
	s32 cursor_row;
	bool cursor_visible;
};

#define EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID \
	EFI_GUID(0x387477c2, 0x69c7, 0x11d2, \
		 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define EFI_BLACK                0x00
#define EFI_BLUE                 0x01
#define EFI_GREEN                0x02
#define EFI_CYAN                 0x03
#define EFI_RED                  0x04
#define EFI_MAGENTA              0x05
#define EFI_BROWN                0x06
#define EFI_LIGHTGRAY            0x07
#define EFI_BRIGHT               0x08
#define EFI_DARKGRAY             0x08
#define EFI_LIGHTBLUE            0x09
#define EFI_LIGHTGREEN           0x0a
#define EFI_LIGHTCYAN            0x0b
#define EFI_LIGHTRED             0x0c
#define EFI_LIGHTMAGENTA         0x0d
#define EFI_YELLOW               0x0e
#define EFI_WHITE                0x0f
#define EFI_BACKGROUND_BLACK     0x00
#define EFI_BACKGROUND_BLUE      0x10
#define EFI_BACKGROUND_GREEN     0x20
#define EFI_BACKGROUND_CYAN      0x30
#define EFI_BACKGROUND_RED       0x40
#define EFI_BACKGROUND_MAGENTA   0x50
#define EFI_BACKGROUND_BROWN     0x60
#define EFI_BACKGROUND_LIGHTGRAY 0x70

/* extract foreground color from EFI attribute */
#define EFI_ATTR_FG(attr)        ((attr) & 0x07)
/* treat high bit of FG as bright/bold (similar to edk2) */
#define EFI_ATTR_BOLD(attr)      (((attr) >> 3) & 0x01)
/* extract background color from EFI attribute */
#define EFI_ATTR_BG(attr)        (((attr) >> 4) & 0x7)

struct efi_simple_text_output_protocol {
	efi_status_t (EFIAPI *reset)(
			struct efi_simple_text_output_protocol *this,
			char extended_verification);
	efi_status_t (EFIAPI *output_string)(
			struct efi_simple_text_output_protocol *this,
			const u16 *str);
	efi_status_t (EFIAPI *test_string)(
			struct efi_simple_text_output_protocol *this,
			const u16 *str);
	efi_status_t(EFIAPI *query_mode)(
			struct efi_simple_text_output_protocol *this,
			unsigned long mode_number, unsigned long *columns,
			unsigned long *rows);
	efi_status_t(EFIAPI *set_mode)(
			struct efi_simple_text_output_protocol *this,
			unsigned long mode_number);
	efi_status_t(EFIAPI *set_attribute)(
			struct efi_simple_text_output_protocol *this,
			unsigned long attribute);
	efi_status_t(EFIAPI *clear_screen) (
			struct efi_simple_text_output_protocol *this);
	efi_status_t(EFIAPI *set_cursor_position) (
			struct efi_simple_text_output_protocol *this,
			unsigned long column, unsigned long row);
	efi_status_t(EFIAPI *enable_cursor)(
			struct efi_simple_text_output_protocol *this,
			bool enable);
	struct simple_text_output_mode *mode;
};

#define EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID \
	EFI_GUID(0xdd9e7534, 0x7762, 0x4698, \
		 0x8c, 0x14, 0xf5, 0x85, 0x17, 0xa6, 0x25, 0xaa)

struct efi_input_key {
	u16 scan_code;
	u16 unicode_char;
};

#define EFI_SHIFT_STATE_INVALID		0x00000000
#define EFI_RIGHT_SHIFT_PRESSED		0x00000001
#define EFI_LEFT_SHIFT_PRESSED		0x00000002
#define EFI_RIGHT_CONTROL_PRESSED	0x00000004
#define EFI_LEFT_CONTROL_PRESSED	0x00000008
#define EFI_RIGHT_ALT_PRESSED		0x00000010
#define EFI_LEFT_ALT_PRESSED		0x00000020
#define EFI_RIGHT_LOGO_PRESSED		0x00000040
#define EFI_LEFT_LOGO_PRESSED		0x00000080
#define EFI_MENU_KEY_PRESSED		0x00000100
#define EFI_SYS_REQ_PRESSED		0x00000200
#define EFI_SHIFT_STATE_VALID		0x80000000

#define EFI_TOGGLE_STATE_INVALID	0x00
#define EFI_SCROLL_LOCK_ACTIVE		0x01
#define EFI_NUM_LOCK_ACTIVE		0x02
#define EFI_CAPS_LOCK_ACTIVE		0x04
#define EFI_KEY_STATE_EXPOSED		0x40
#define EFI_TOGGLE_STATE_VALID		0x80

struct efi_key_state {
	u32 key_shift_state;
	u8 key_toggle_state;
};

struct efi_key_data {
	struct efi_input_key key;
	struct efi_key_state key_state;
};

struct efi_simple_text_input_ex_protocol {
	efi_status_t (EFIAPI *reset) (
		struct efi_simple_text_input_ex_protocol *this,
		bool extended_verification);
	efi_status_t (EFIAPI *read_key_stroke_ex) (
		struct efi_simple_text_input_ex_protocol *this,
		struct efi_key_data *key_data);
	struct efi_event *wait_for_key_ex;
	efi_status_t (EFIAPI *set_state) (
		struct efi_simple_text_input_ex_protocol *this,
		u8 *key_toggle_state);
	efi_status_t (EFIAPI *register_key_notify) (
		struct efi_simple_text_input_ex_protocol *this,
		struct efi_key_data *key_data,
		efi_status_t (EFIAPI *key_notify_function)(
			struct efi_key_data *key_data),
		void **notify_handle);
	efi_status_t (EFIAPI *unregister_key_notify) (
		struct efi_simple_text_input_ex_protocol *this,
		void *notification_handle);
};

#define EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID \
	EFI_GUID(0x387477c1, 0x69c7, 0x11d2, \
		 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

struct efi_simple_text_input_protocol {
	efi_status_t(EFIAPI *reset)(struct efi_simple_text_input_protocol *this,
				    bool extended_verification);
	efi_status_t(EFIAPI *read_key_stroke)(
			struct efi_simple_text_input_protocol *this,
			struct efi_input_key *key);
	struct efi_event *wait_for_key;
};

#define EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID \
	EFI_GUID(0x8b843e20, 0x8132, 0x4852, \
		 0x90, 0xcc, 0x55, 0x1a, 0x4e, 0x4a, 0x7f, 0x1c)

struct efi_device_path_to_text_protocol {
	uint16_t *(EFIAPI *convert_device_node_to_text)(
			struct efi_device_path *device_node,
			bool display_only,
			bool allow_shortcuts);
	uint16_t *(EFIAPI *convert_device_path_to_text)(
			struct efi_device_path *device_path,
			bool display_only,
			bool allow_shortcuts);
};

#define EFI_DEVICE_PATH_UTILITIES_PROTOCOL_GUID \
	EFI_GUID(0x0379be4e, 0xd706, 0x437d, \
		 0xb0, 0x37, 0xed, 0xb8, 0x2f, 0xb7, 0x72, 0xa4)

struct efi_device_path_utilities_protocol {
	efi_uintn_t (EFIAPI *get_device_path_size)(
		const struct efi_device_path *device_path);
	struct efi_device_path *(EFIAPI *duplicate_device_path)(
		const struct efi_device_path *device_path);
	struct efi_device_path *(EFIAPI *append_device_path)(
		const struct efi_device_path *src1,
		const struct efi_device_path *src2);
	struct efi_device_path *(EFIAPI *append_device_node)(
		const struct efi_device_path *device_path,
		const struct efi_device_path *device_node);
	struct efi_device_path *(EFIAPI *append_device_path_instance)(
		const struct efi_device_path *device_path,
		const struct efi_device_path *device_path_instance);
	struct efi_device_path *(EFIAPI *get_next_device_path_instance)(
		struct efi_device_path **device_path_instance,
		efi_uintn_t *device_path_instance_size);
	bool (EFIAPI *is_device_path_multi_instance)(
		const struct efi_device_path *device_path);
	struct efi_device_path *(EFIAPI *create_device_node)(
		uint8_t node_type,
		uint8_t node_sub_type,
		uint16_t node_length);
};

/*
 * Human Interface Infrastructure (HII)
 */
struct efi_hii_package_list_header {
	efi_guid_t package_list_guid;
	u32 package_length;
} __packed;

/**
 * struct efi_hii_package_header - EFI HII package header
 *
 * @fields:	'fields' replaces the bit-fields defined in the EFI
 *		specification to to avoid possible compiler incompatibilities::
 *
 *		    u32 length:24;
 *		    u32 type:8;
 */
struct efi_hii_package_header {
	u32 fields;
} __packed;

#define __EFI_HII_PACKAGE_LEN_SHIFT	0
#define __EFI_HII_PACKAGE_TYPE_SHIFT	24
#define __EFI_HII_PACKAGE_LEN_MASK	0xffffff
#define __EFI_HII_PACKAGE_TYPE_MASK	0xff

#define EFI_HII_PACKAGE_TYPE_ALL          0x00
#define EFI_HII_PACKAGE_TYPE_GUID         0x01
#define EFI_HII_PACKAGE_FORMS             0x02
#define EFI_HII_PACKAGE_STRINGS           0x04
#define EFI_HII_PACKAGE_FONTS             0x05
#define EFI_HII_PACKAGE_IMAGES            0x06
#define EFI_HII_PACKAGE_SIMPLE_FONTS      0x07
#define EFI_HII_PACKAGE_DEVICE_PATH       0x08
#define EFI_HII_PACKAGE_KEYBOARD_LAYOUT   0x09
#define EFI_HII_PACKAGE_ANIMATIONS        0x0A
#define EFI_HII_PACKAGE_END               0xDF
#define EFI_HII_PACKAGE_TYPE_SYSTEM_BEGIN 0xE0
#define EFI_HII_PACKAGE_TYPE_SYSTEM_END   0xFF

/*
 * HII GUID package
 */
struct efi_hii_guid_package {
	struct efi_hii_package_header header;
	efi_guid_t guid;
	char data[];
} __packed;

/*
 * HII string package
 */
struct efi_hii_strings_package {
	struct efi_hii_package_header header;
	u32 header_size;
	u32 string_info_offset;
	u16 language_window[16];
	efi_string_id_t language_name;
	u8  language[];
} __packed;

struct efi_hii_string_block {
	u8 block_type;
	/* u8 block_body[]; */
} __packed;

#define EFI_HII_SIBT_END               0x00
#define EFI_HII_SIBT_STRING_SCSU       0x10
#define EFI_HII_SIBT_STRING_SCSU_FONT  0x11
#define EFI_HII_SIBT_STRINGS_SCSU      0x12
#define EFI_HII_SIBT_STRINGS_SCSU_FONT 0x13
#define EFI_HII_SIBT_STRING_UCS2       0x14
#define EFI_HII_SIBT_STRING_UCS2_FONT  0x15
#define EFI_HII_SIBT_STRINGS_UCS2      0x16
#define EFI_HII_SIBT_STRINGS_UCS2_FONT 0x17
#define EFI_HII_SIBT_DUPLICATE         0x20
#define EFI_HII_SIBT_SKIP2             0x21
#define EFI_HII_SIBT_SKIP1             0x22
#define EFI_HII_SIBT_EXT1              0x30
#define EFI_HII_SIBT_EXT2              0x31
#define EFI_HII_SIBT_EXT4              0x32
#define EFI_HII_SIBT_FONT              0x40

struct efi_hii_sibt_string_ucs2_block {
	struct efi_hii_string_block header;
	u16 string_text[];
} __packed;

static inline struct efi_hii_string_block *
efi_hii_sibt_string_ucs2_block_next(struct efi_hii_sibt_string_ucs2_block *blk)
{
	return ((void *)blk) + sizeof(*blk) +
		(u16_strlen(blk->string_text) + 1) * 2;
}

/*
 * HII forms package
 * TODO: full scope of definitions
 */
struct efi_hii_time {
	u8 hour;
	u8 minute;
	u8 second;
};

struct efi_hii_date {
	u16 year;
	u8 month;
	u8 day;
};

struct efi_hii_ref {
	efi_question_id_t question_id;
	efi_form_id_t form_id;
	efi_guid_t form_set_guid;
	efi_string_id_t device_path;
};

union efi_ifr_type_value {
	u8 u8;				// EFI_IFR_TYPE_NUM_SIZE_8
	u16 u16;			// EFI_IFR_TYPE_NUM_SIZE_16
	u32 u32;			// EFI_IFR_TYPE_NUM_SIZE_32
	u64 u64;			// EFI_IFR_TYPE_NUM_SIZE_64
	bool b;				// EFI_IFR_TYPE_BOOLEAN
	struct efi_hii_time time;	// EFI_IFR_TYPE_TIME
	struct efi_hii_date date;	// EFI_IFR_TYPE_DATE
	efi_string_id_t string;	// EFI_IFR_TYPE_STRING, EFI_IFR_TYPE_ACTION
	struct efi_hii_ref ref;		// EFI_IFR_TYPE_REF
	// u8 buffer[];			// EFI_IFR_TYPE_BUFFER
};

#define EFI_IFR_TYPE_NUM_SIZE_8		0x00
#define EFI_IFR_TYPE_NUM_SIZE_16	0x01
#define EFI_IFR_TYPE_NUM_SIZE_32	0x02
#define EFI_IFR_TYPE_NUM_SIZE_64	0x03
#define EFI_IFR_TYPE_BOOLEAN		0x04
#define EFI_IFR_TYPE_TIME		0x05
#define EFI_IFR_TYPE_DATE		0x06
#define EFI_IFR_TYPE_STRING		0x07
#define EFI_IFR_TYPE_OTHER		0x08
#define EFI_IFR_TYPE_UNDEFINED		0x09
#define EFI_IFR_TYPE_ACTION		0x0A
#define EFI_IFR_TYPE_BUFFER		0x0B
#define EFI_IFR_TYPE_REF		0x0C
#define EFI_IFR_OPTION_DEFAULT		0x10
#define EFI_IFR_OPTION_DEFAULT_MFG	0x20

#define EFI_IFR_ONE_OF_OPTION_OP	0x09

struct efi_ifr_op_header {
	u8 opCode;
	u8 length:7;
	u8 scope:1;
};

struct efi_ifr_one_of_option {
	struct efi_ifr_op_header header;
	efi_string_id_t option;
	u8 flags;
	u8 type;
	union efi_ifr_type_value value;
};

typedef efi_uintn_t efi_browser_action_t;

#define EFI_BROWSER_ACTION_REQUEST_NONE			0
#define EFI_BROWSER_ACTION_REQUEST_RESET		1
#define EFI_BROWSER_ACTION_REQUEST_SUBMIT		2
#define EFI_BROWSER_ACTION_REQUEST_EXIT			3
#define EFI_BROWSER_ACTION_REQUEST_FORM_SUBMIT_EXIT	4
#define EFI_BROWSER_ACTION_REQUEST_FORM_DISCARD_EXIT	5
#define EFI_BROWSER_ACTION_REQUEST_FORM_APPLY		6
#define EFI_BROWSER_ACTION_REQUEST_FORM_DISCARD		7
#define EFI_BROWSER_ACTION_REQUEST_RECONNECT		8

typedef efi_uintn_t efi_browser_action_request_t;

#define EFI_BROWSER_ACTION_CHANGING			0
#define EFI_BROWSER_ACTION_CHANGED			1
#define EFI_BROWSER_ACTION_RETRIEVE			2
#define EFI_BROWSER_ACTION_FORM_OPEN			3
#define EFI_BROWSER_ACTION_FORM_CLOSE			4
#define EFI_BROWSER_ACTION_SUBMITTED			5
#define EFI_BROWSER_ACTION_DEFAULT_STANDARD		0x1000
#define EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING	0x1001
#define EFI_BROWSER_ACTION_DEFAULT_SAFE			0x1002
#define EFI_BROWSER_ACTION_DEFAULT_PLATFORM		0x2000
#define EFI_BROWSER_ACTION_DEFAULT_HARDWARE		0x3000
#define EFI_BROWSER_ACTION_DEFAULT_FIRMWARE		0x4000

/*
 * HII keyboard package
 */
typedef enum {
	EFI_KEY_LCTRL, EFI_KEY_A0, EFI_KEY_LALT, EFI_KEY_SPACE_BAR,
	EFI_KEY_A2, EFI_KEY_A3, EFI_KEY_A4, EFI_KEY_RCTRL, EFI_KEY_LEFT_ARROW,
	EFI_KEY_DOWN_ARROW, EFI_KEY_RIGHT_ARROW, EFI_KEY_ZERO,
	EFI_KEY_PERIOD, EFI_KEY_ENTER, EFI_KEY_LSHIFT, EFI_KEY_B0,
	EFI_KEY_B1, EFI_KEY_B2, EFI_KEY_B3, EFI_KEY_B4, EFI_KEY_B5, EFI_KEY_B6,
	EFI_KEY_B7, EFI_KEY_B8, EFI_KEY_B9, EFI_KEY_B10, EFI_KEY_RSHIFT,
	EFI_KEY_UP_ARROW, EFI_KEY_ONE, EFI_KEY_TWO, EFI_KEY_THREE,
	EFI_KEY_CAPS_LOCK, EFI_KEY_C1, EFI_KEY_C2, EFI_KEY_C3, EFI_KEY_C4,
	EFI_KEY_C5, EFI_KEY_C6, EFI_KEY_C7, EFI_KEY_C8, EFI_KEY_C9,
	EFI_KEY_C10, EFI_KEY_C11, EFI_KEY_C12, EFI_KEY_FOUR, EFI_KEY_FIVE,
	EFI_KEY_SIX, EFI_KEY_PLUS, EFI_KEY_TAB, EFI_KEY_D1, EFI_KEY_D2,
	EFI_KEY_D3, EFI_KEY_D4, EFI_KEY_D5, EFI_KEY_D6, EFI_KEY_D7, EFI_KEY_D8,
	EFI_KEY_D9, EFI_KEY_D10, EFI_KEY_D11, EFI_KEY_D12, EFI_KEY_D13,
	EFI_KEY_DEL, EFI_KEY_END, EFI_KEY_PG_DN, EFI_KEY_SEVEN, EFI_KEY_EIGHT,
	EFI_KEY_NINE, EFI_KEY_E0, EFI_KEY_E1, EFI_KEY_E2, EFI_KEY_E3,
	EFI_KEY_E4, EFI_KEY_E5, EFI_KEY_E6, EFI_KEY_E7, EFI_KEY_E8, EFI_KEY_E9,
	EFI_KEY_E10, EFI_KEY_E11, EFI_KEY_E12, EFI_KEY_BACK_SPACE,
	EFI_KEY_INS, EFI_KEY_HOME, EFI_KEY_PG_UP, EFI_KEY_NLCK, EFI_KEY_SLASH,
	EFI_KEY_ASTERISK, EFI_KEY_MINUS, EFI_KEY_ESC, EFI_KEY_F1, EFI_KEY_F2,
	EFI_KEY_F3, EFI_KEY_F4, EFI_KEY_F5, EFI_KEY_F6, EFI_KEY_F7, EFI_KEY_F8,
	EFI_KEY_F9, EFI_KEY_F10, EFI_KEY_F11, EFI_KEY_F12, EFI_KEY_PRINT,
	EFI_KEY_SLCK, EFI_KEY_PAUSE,
} efi_key;

struct efi_key_descriptor {
	u32 key;
	u16 unicode;
	u16 shifted_unicode;
	u16 alt_gr_unicode;
	u16 shifted_alt_gr_unicode;
	u16 modifier;
	u16 affected_attribute;
} __packed;

struct efi_hii_keyboard_layout {
	u16 layout_length;
	/*
	 * The EFI spec defines this as efi_guid_t.
	 * clang and gcc both report alignment problems here.
	 * clang with -Wunaligned-access
	 * warning: field guid within 'struct efi_hii_keyboard_layout' is less
	 * aligned than 'efi_guid_t' and is usually due to
	 * 'struct efi_hii_keyboard_layout' being packed, which can lead to
	 * unaligned accesses
	 *
	 * GCC with -Wpacked-not-aligned -Waddress-of-packed-member
	 * 'efi_guid_t' offset 2 in 'struct efi_hii_keyboard_layout'
	 * isn't aligned to 4
	 *
	 * Removing the alignment from efi_guid_t is not an option, since
	 * it is also used in non-packed structs and that would break
	 * calculations with offsetof
	 *
	 * This is the only place we get a report for. That happens because
	 * all other declarations of efi_guid_t within a packed struct happens
	 * to be 4-byte aligned.  i.e a u32, a u64 a 2 * u16 or any combination
	 * that ends up landing efi_guid_t on a 4byte boundary precedes.
	 *
	 * Replace this with a 1-byte aligned counterpart of b[16].  This is a
	 * packed struct so the memory  placement of efi_guid_t should not change
	 *
	 */
	u8 guid[16];
	u32 layout_descriptor_string_offset;
	u8 descriptor_count;
	/* struct efi_key_descriptor descriptors[]; follows here */
} __packed;

struct efi_hii_keyboard_package {
	struct efi_hii_package_header header;
	u16 layout_count;
	struct efi_hii_keyboard_layout layout[];
} __packed;

/*
 * HII protocols
 */
#define EFI_HII_STRING_PROTOCOL_GUID \
	EFI_GUID(0x0fd96974, 0x23aa, 0x4cdc, \
		 0xb9, 0xcb, 0x98, 0xd1, 0x77, 0x50, 0x32, 0x2a)

struct efi_font_info {
	efi_hii_font_style_t font_style;
	u16 font_size;
	u16 font_name[1];
};

struct efi_hii_string_protocol {
	efi_status_t(EFIAPI *new_string)(
		const struct efi_hii_string_protocol *this,
		efi_hii_handle_t package_list,
		efi_string_id_t *string_id,
		const u8 *language,
		const u16 *language_name,
		const efi_string_t string,
		const struct efi_font_info *string_font_info);
	efi_status_t(EFIAPI *get_string)(
		const struct efi_hii_string_protocol *this,
		const u8 *language,
		efi_hii_handle_t package_list,
		efi_string_id_t string_id,
		efi_string_t string,
		efi_uintn_t *string_size,
		struct efi_font_info **string_font_info);
	efi_status_t(EFIAPI *set_string)(
		const struct efi_hii_string_protocol *this,
		efi_hii_handle_t package_list,
		efi_string_id_t string_id,
		const u8 *language,
		const efi_string_t string,
		const struct efi_font_info *string_font_info);
	efi_status_t(EFIAPI *get_languages)(
		const struct efi_hii_string_protocol *this,
		efi_hii_handle_t package_list,
		u8 *languages,
		efi_uintn_t *languages_size);
	efi_status_t(EFIAPI *get_secondary_languages)(
		const struct efi_hii_string_protocol *this,
		efi_hii_handle_t package_list,
		const u8 *primary_language,
		u8 *secondary_languages,
		efi_uintn_t *secondary_languages_size);
};

#define EFI_HII_DATABASE_PROTOCOL_GUID	     \
	EFI_GUID(0xef9fc172, 0xa1b2, 0x4693, \
		 0xb3, 0x27, 0x6d, 0x32, 0xfc, 0x41, 0x60, 0x42)

struct efi_hii_database_protocol {
	efi_status_t(EFIAPI *new_package_list)(
		const struct efi_hii_database_protocol *this,
		const struct efi_hii_package_list_header *package_list,
		const efi_handle_t driver_handle,
		efi_hii_handle_t *handle);
	efi_status_t(EFIAPI *remove_package_list)(
		const struct efi_hii_database_protocol *this,
		efi_hii_handle_t handle);
	efi_status_t(EFIAPI *update_package_list)(
		const struct efi_hii_database_protocol *this,
		efi_hii_handle_t handle,
		const struct efi_hii_package_list_header *package_list);
	efi_status_t(EFIAPI *list_package_lists)(
		const struct efi_hii_database_protocol *this,
		u8 package_type,
		const efi_guid_t *package_guid,
		efi_uintn_t *handle_buffer_length,
		efi_hii_handle_t *handle);
	efi_status_t(EFIAPI *export_package_lists)(
		const struct efi_hii_database_protocol *this,
		efi_hii_handle_t handle,
		efi_uintn_t *buffer_size,
		struct efi_hii_package_list_header *buffer);
	efi_status_t(EFIAPI *register_package_notify)(
		const struct efi_hii_database_protocol *this,
		u8 package_type,
		const efi_guid_t *package_guid,
		const void *package_notify_fn,
		efi_uintn_t notify_type,
		efi_handle_t *notify_handle);
	efi_status_t(EFIAPI *unregister_package_notify)(
		const struct efi_hii_database_protocol *this,
		efi_handle_t notification_handle
		);
	efi_status_t(EFIAPI *find_keyboard_layouts)(
		const struct efi_hii_database_protocol *this,
		u16 *key_guid_buffer_length,
		efi_guid_t *key_guid_buffer);
	efi_status_t(EFIAPI *get_keyboard_layout)(
		const struct efi_hii_database_protocol *this,
		efi_guid_t *key_guid,
		u16 *keyboard_layout_length,
		struct efi_hii_keyboard_layout *keyboard_layout);
	efi_status_t(EFIAPI *set_keyboard_layout)(
		const struct efi_hii_database_protocol *this,
		efi_guid_t *key_guid);
	efi_status_t(EFIAPI *get_package_list_handle)(
		const struct efi_hii_database_protocol *this,
		efi_hii_handle_t package_list_handle,
		efi_handle_t *driver_handle);
};

#define EFI_HII_CONFIG_ROUTING_PROTOCOL_GUID \
	EFI_GUID(0x587e72d7, 0xcc50, 0x4f79, \
		 0x82, 0x09, 0xca, 0x29, 0x1f, 0xc1, 0xa1, 0x0f)

struct efi_hii_config_routing_protocol {
	efi_status_t(EFIAPI *extract_config)(
		const struct efi_hii_config_routing_protocol *this,
		const efi_string_t request,
		efi_string_t *progress,
		efi_string_t *results);
	efi_status_t(EFIAPI *export_config)(
		const struct efi_hii_config_routing_protocol *this,
		efi_string_t *results);
	efi_status_t(EFIAPI *route_config)(
		const struct efi_hii_config_routing_protocol *this,
		const efi_string_t configuration,
		efi_string_t *progress);
	efi_status_t(EFIAPI *block_to_config)(
		const struct efi_hii_config_routing_protocol *this,
		const efi_string_t config_request,
		const uint8_t *block,
		const efi_uintn_t block_size,
		efi_string_t *config,
		efi_string_t *progress);
	efi_status_t(EFIAPI *config_to_block)(
		const struct efi_hii_config_routing_protocol *this,
		const efi_string_t config_resp,
		const uint8_t *block,
		const efi_uintn_t *block_size,
		efi_string_t *progress);
	efi_status_t(EFIAPI *get_alt_config)(
		const struct efi_hii_config_routing_protocol *this,
		const efi_string_t config_resp,
		const efi_guid_t *guid,
		const efi_string_t name,
		const struct efi_device_path *device_path,
		const efi_string_t alt_cfg_id,
		efi_string_t *alt_cfg_resp);
};

#define EFI_HII_CONFIG_ACCESS_PROTOCOL_GUID \
	EFI_GUID(0x330d4706, 0xf2a0, 0x4e4f, \
		 0xa3, 0x69, 0xb6, 0x6f, 0xa8, 0xd5, 0x43, 0x85)

struct efi_hii_config_access_protocol {
	efi_status_t(EFIAPI *extract_config_access)(
		const struct efi_hii_config_access_protocol *this,
		const efi_string_t request,
		efi_string_t *progress,
		efi_string_t *results);
	efi_status_t(EFIAPI *route_config_access)(
		const struct efi_hii_config_access_protocol *this,
		const efi_string_t configuration,
		efi_string_t *progress);
	efi_status_t(EFIAPI *form_callback)(
		const struct efi_hii_config_access_protocol *this,
		efi_browser_action_t action,
		efi_question_id_t question_id,
		u8 type,
		union efi_ifr_type_value *value,
		efi_browser_action_request_t *action_request);
};

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
	EFI_GUID(0x9042a9de, 0x23dc, 0x4a38, \
		 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a)

#define EFI_GOT_RGBA8		0
#define EFI_GOT_BGRA8		1
#define EFI_GOT_BITMASK		2

struct efi_gop_mode_info {
	u32 version;
	u32 width;
	u32 height;
	u32 pixel_format;
	u32 pixel_bitmask[4];
	u32 pixels_per_scanline;
};

struct efi_gop_mode {
	u32 max_mode;
	u32 mode;
	struct efi_gop_mode_info *info;
	unsigned long info_size;
	efi_physical_addr_t fb_base;
	unsigned long fb_size;
};

struct efi_gop_pixel {
	u8 blue;
	u8 green;
	u8 red;
	u8 reserved;
};

#define EFI_BLT_VIDEO_FILL		0
#define EFI_BLT_VIDEO_TO_BLT_BUFFER	1
#define EFI_BLT_BUFFER_TO_VIDEO		2
#define EFI_BLT_VIDEO_TO_VIDEO		3

struct efi_gop {
	efi_status_t (EFIAPI *query_mode)(struct efi_gop *this, u32 mode_number,
					  efi_uintn_t *size_of_info,
					  struct efi_gop_mode_info **info);
	efi_status_t (EFIAPI *set_mode)(struct efi_gop *this, u32 mode_number);
	efi_status_t (EFIAPI *blt)(struct efi_gop *this,
				   struct efi_gop_pixel *buffer,
				   u32 operation, efi_uintn_t sx,
				   efi_uintn_t sy, efi_uintn_t dx,
				   efi_uintn_t dy, efi_uintn_t width,
				   efi_uintn_t height, efi_uintn_t delta);
	struct efi_gop_mode *mode;
};

#define EFI_SIMPLE_NETWORK_PROTOCOL_GUID \
	EFI_GUID(0xa19832b9, 0xac25, 0x11d3, \
		 0x9a, 0x2d, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d)

struct efi_mac_address {
	char mac_addr[32];
};

struct efi_ip_address {
	u8 ip_addr[16];
} __attribute__((aligned(4)));

enum efi_simple_network_state {
	EFI_NETWORK_STOPPED,
	EFI_NETWORK_STARTED,
	EFI_NETWORK_INITIALIZED,
};

struct efi_simple_network_mode {
	enum efi_simple_network_state state;
	u32 hwaddr_size;
	u32 media_header_size;
	u32 max_packet_size;
	u32 nvram_size;
	u32 nvram_access_size;
	u32 receive_filter_mask;
	u32 receive_filter_setting;
	u32 max_mcast_filter_count;
	u32 mcast_filter_count;
	struct efi_mac_address mcast_filter[16];
	struct efi_mac_address current_address;
	struct efi_mac_address broadcast_address;
	struct efi_mac_address permanent_address;
	u8 if_type;
	u8 mac_changeable;
	u8 multitx_supported;
	u8 media_present_supported;
	u8 media_present;
};

/* receive_filters bit mask */
#define EFI_SIMPLE_NETWORK_RECEIVE_UNICAST               0x01
#define EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST             0x02
#define EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST             0x04
#define EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS           0x08
#define EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST 0x10

/* interrupt status bit mask */
#define EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT	0x01
#define EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT	0x02
#define EFI_SIMPLE_NETWORK_COMMAND_INTERRUPT	0x04
#define EFI_SIMPLE_NETWORK_SOFTWARE_INTERRUPT	0x08

/* revision of the simple network protocol */
#define EFI_SIMPLE_NETWORK_PROTOCOL_REVISION	0x00010000

struct efi_simple_network {
	u64 revision;
	efi_status_t (EFIAPI *start)(struct efi_simple_network *this);
	efi_status_t (EFIAPI *stop)(struct efi_simple_network *this);
	efi_status_t (EFIAPI *initialize)(struct efi_simple_network *this,
			ulong extra_rx, ulong extra_tx);
	efi_status_t (EFIAPI *reset)(struct efi_simple_network *this,
			int extended_verification);
	efi_status_t (EFIAPI *shutdown)(struct efi_simple_network *this);
	efi_status_t (EFIAPI *receive_filters)(struct efi_simple_network *this,
			u32 enable, u32 disable, int reset_mcast_filter,
			ulong mcast_filter_count,
			struct efi_mac_address *mcast_filter);
	efi_status_t (EFIAPI *station_address)(struct efi_simple_network *this,
			int reset, struct efi_mac_address *new_mac);
	efi_status_t (EFIAPI *statistics)(struct efi_simple_network *this,
			int reset, ulong *stat_size, void *stat_table);
	efi_status_t (EFIAPI *mcastiptomac)(struct efi_simple_network *this,
			int ipv6, struct efi_ip_address *ip,
			struct efi_mac_address *mac);
	efi_status_t (EFIAPI *nvdata)(struct efi_simple_network *this,
			int read_write, ulong offset, ulong buffer_size,
			char *buffer);
	efi_status_t (EFIAPI *get_status)(struct efi_simple_network *this,
			u32 *int_status, void **txbuf);
	efi_status_t (EFIAPI *transmit)(struct efi_simple_network *this,
			size_t header_size, size_t buffer_size, void *buffer,
			struct efi_mac_address *src_addr,
			struct efi_mac_address *dest_addr, u16 *protocol);
	efi_status_t (EFIAPI *receive)(struct efi_simple_network *this,
			size_t *header_size, size_t *buffer_size, void *buffer,
			struct efi_mac_address *src_addr,
			struct efi_mac_address *dest_addr, u16 *protocol);
	struct efi_event *wait_for_packet;
	struct efi_simple_network_mode *mode;
	/* private fields */
	u32 int_status;
};

#define EFI_PXE_BASE_CODE_PROTOCOL_GUID \
	EFI_GUID(0x03c4e603, 0xac28, 0x11d3, \
		 0x9a, 0x2d, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d)

#define EFI_PXE_BASE_CODE_PROTOCOL_REVISION 0x00010000
#define EFI_PXE_BASE_CODE_MAX_IPCNT 8

struct efi_pxe_packet {
	u8 packet[1472];
};

struct efi_pxe_mode {
	u8 started;
	u8 ipv6_available;
	u8 ipv6_supported;
	u8 using_ipv6;
	u8 bis_supported;
	u8 bis_detected;
	u8 auto_arp;
	u8 send_guid;
	u8 dhcp_discover_valid;
	u8 dhcp_ack_received;
	u8 proxy_offer_received;
	u8 pxe_discover_valid;
	u8 pxe_reply_received;
	u8 pxe_bis_reply_received;
	u8 icmp_error_received;
	u8 tftp_error_received;
	u8 make_callbacks;
	u8 ttl;
	u8 tos;
	u8 pad;
	struct efi_ip_address station_ip;
	struct efi_ip_address subnet_mask;
	struct efi_pxe_packet dhcp_discover;
	struct efi_pxe_packet dhcp_ack;
	struct efi_pxe_packet proxy_offer;
	struct efi_pxe_packet pxe_discover;
	struct efi_pxe_packet pxe_reply;
};

struct efi_pxe_base_code_srvlist {
	u16 type;
	u8 accept_any_response;
	u8 reserved;
	struct efi_ip_address ip_addr;
};

struct efi_pxe_base_code_discover_info {
	u8 use_m_cast;
	u8 use_b_cast;
	u8 use_u_cast;
	u8 must_use_list;
	struct efi_ip_address server_m_cast_ip;
	u16 ip_cnt;
	struct efi_pxe_base_code_srvlist srv_list[];
};

struct efi_pxe_base_code_mtftp_info {
	struct efi_ip_address m_cast_ip;
	u16 cport;
	u16 sport;
	u16 listen_timeout;
	u16 transit_timeout;
};

struct efi_pxe_base_code_filter {
	u8 filters;
	u8 ip_cnt;
	u16 reserved;
	struct efi_ip_address ip_list[EFI_PXE_BASE_CODE_MAX_IPCNT];
};

struct efi_pxe_base_code_dhcpv4_packet {
	u8 bootp_op_code;
	u8 bootp_hw_type;
	u8 bootp_addr_len;
	u8 bootp_gate_hops;
	u32 bootp_ident;
	u16 bootp_seconds;
	u16 bootp_flags;
	u8 bootp_ci_addr[4];
	u8 bootp_yi_addr[4];
	u8 bootp_si_addr[4];
	u8 bootp_gi_addr[4];
	u8 bootp_hw_addr[16];
	u8 bootp_srv_name[64];
	u8 bootp_boot_file[128];
	u32 dhcp_magick;
	u8 dhcp_options[56];
};

struct efi_pxe_base_code_dhcpv6_packet {
	u8 message_type;
	u8 transaction_id[3];
	u8 dhcp_options[1024];
};

typedef union {
	u8 raw[1472];
	struct efi_pxe_base_code_dhcpv4_packet dhcpv4;
	struct efi_pxe_base_code_dhcpv6_packet dhcpv6;
} EFI_PXE_BASE_CODE_PACKET;

struct efi_pxe_base_code_protocol {
	u64 revision;
	efi_status_t (EFIAPI *start)(struct efi_pxe_base_code_protocol *this,
				     u8 use_ipv6);
	efi_status_t (EFIAPI *stop)(struct efi_pxe_base_code_protocol *this);
	efi_status_t (EFIAPI *dhcp)(struct efi_pxe_base_code_protocol *this,
				    u8 sort_offers);
	efi_status_t (EFIAPI *discover)(
				struct efi_pxe_base_code_protocol *this,
				u16 type, u16 *layer, u8 bis,
				struct efi_pxe_base_code_discover_info *info);
	efi_status_t (EFIAPI *mtftp)(
				struct efi_pxe_base_code_protocol *this,
				u32 operation, void *buffer_ptr,
				u8 overwrite, efi_uintn_t *buffer_size,
				struct efi_ip_address server_ip, char *filename,
				struct efi_pxe_base_code_mtftp_info *info,
				u8 dont_use_buffer);
	efi_status_t (EFIAPI *udp_write)(
				struct efi_pxe_base_code_protocol *this,
				u16 op_flags, struct efi_ip_address *dest_ip,
				u16 *dest_port,
				struct efi_ip_address *gateway_ip,
				struct efi_ip_address *src_ip, u16 *src_port,
				efi_uintn_t *header_size, void *header_ptr,
				efi_uintn_t *buffer_size, void *buffer_ptr);
	efi_status_t (EFIAPI *udp_read)(
				struct efi_pxe_base_code_protocol *this,
				u16 op_flags, struct efi_ip_address *dest_ip,
				u16 *dest_port, struct efi_ip_address *src_ip,
				u16 *src_port, efi_uintn_t *header_size,
				void *header_ptr, efi_uintn_t *buffer_size,
				void *buffer_ptr);
	efi_status_t (EFIAPI *set_ip_filter)(
				struct efi_pxe_base_code_protocol *this,
				struct efi_pxe_base_code_filter *new_filter);
	efi_status_t (EFIAPI *arp)(struct efi_pxe_base_code_protocol *this,
				   struct efi_ip_address *ip_addr,
				   struct efi_mac_address *mac_addr);
	efi_status_t (EFIAPI *set_parameters)(
				struct efi_pxe_base_code_protocol *this,
				u8 *new_auto_arp, u8 *new_send_guid,
				u8 *new_ttl, u8 *new_tos,
				u8 *new_make_callback);
	efi_status_t (EFIAPI *set_station_ip)(
				struct efi_pxe_base_code_protocol *this,
				struct efi_ip_address *new_station_ip,
				struct efi_ip_address *new_subnet_mask);
	efi_status_t (EFIAPI *set_packets)(
				struct efi_pxe_base_code_protocol *this,
				u8 *new_dhcp_discover_valid,
				u8 *new_dhcp_ack_received,
				u8 *new_proxy_offer_received,
				u8 *new_pxe_discover_valid,
				u8 *new_pxe_reply_received,
				u8 *new_pxe_bis_reply_received,
				EFI_PXE_BASE_CODE_PACKET *new_dchp_discover,
				EFI_PXE_BASE_CODE_PACKET *new_dhcp_acc,
				EFI_PXE_BASE_CODE_PACKET *new_proxy_offer,
				EFI_PXE_BASE_CODE_PACKET *new_pxe_discover,
				EFI_PXE_BASE_CODE_PACKET *new_pxe_reply,
				EFI_PXE_BASE_CODE_PACKET *new_pxe_bis_reply);
	struct efi_pxe_mode *mode;
};

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
	EFI_GUID(0x964e5b22, 0x6459, 0x11d2, \
		 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b)
#define EFI_FILE_PROTOCOL_REVISION	0x00010000
#define EFI_FILE_PROTOCOL_REVISION2	0x00020000
#define EFI_FILE_PROTOCOL_LATEST_REVISION EFI_FILE_PROTOCOL_REVISION2

struct efi_file_io_token {
	struct efi_event *event;
	efi_status_t status;
	efi_uintn_t buffer_size;
	void *buffer;};

struct efi_file_handle {
	u64 rev;
	efi_status_t (EFIAPI *open)(struct efi_file_handle *this,
			struct efi_file_handle **new_handle,
			u16 *file_name, u64 open_mode, u64 attributes);
	efi_status_t (EFIAPI *close)(struct efi_file_handle *this);
	efi_status_t (EFIAPI *delete)(struct efi_file_handle *this);
	efi_status_t (EFIAPI *read)(struct efi_file_handle *this,
			efi_uintn_t *buffer_size, void *buffer);
	efi_status_t (EFIAPI *write)(struct efi_file_handle *this,
			efi_uintn_t *buffer_size, void *buffer);
	efi_status_t (EFIAPI *getpos)(struct efi_file_handle *this,
				      u64 *pos);
	efi_status_t (EFIAPI *setpos)(struct efi_file_handle *this,
				      u64 pos);
	efi_status_t (EFIAPI *getinfo)(struct efi_file_handle *this,
			const efi_guid_t *info_type, efi_uintn_t *buffer_size,
			void *buffer);
	efi_status_t (EFIAPI *setinfo)(struct efi_file_handle *this,
			const efi_guid_t *info_type, efi_uintn_t buffer_size,
			void *buffer);
	efi_status_t (EFIAPI *flush)(struct efi_file_handle *this);
	efi_status_t (EFIAPI *open_ex)(struct efi_file_handle *this,
			struct efi_file_handle **new_handle,
			u16 *file_name, u64 open_mode, u64 attributes,
			struct efi_file_io_token *token);
	efi_status_t (EFIAPI *read_ex)(struct efi_file_handle *this,
			struct efi_file_io_token *token);
	efi_status_t (EFIAPI *write_ex)(struct efi_file_handle *this,
			struct efi_file_io_token *token);
	efi_status_t (EFIAPI *flush_ex)(struct efi_file_handle *this,
			struct efi_file_io_token *token);
};

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION 0x00010000

struct efi_simple_file_system_protocol {
	u64 rev;
	efi_status_t (EFIAPI *open_volume)(struct efi_simple_file_system_protocol *this,
			struct efi_file_handle **root);
};

#define EFI_FILE_INFO_GUID \
	EFI_GUID(0x9576e92, 0x6d3f, 0x11d2, \
		 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define EFI_FILE_SYSTEM_INFO_GUID \
	EFI_GUID(0x09576e93, 0x6d3f, 0x11d2, \
		 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

#define EFI_FILE_SYSTEM_VOLUME_LABEL_ID \
	EFI_GUID(0xdb47d7d3, 0xfe81, 0x11d3, \
		 0x9a, 0x35, 0x00, 0x90, 0x27, 0x3f, 0xC1, 0x4d)

#define EFI_FILE_MODE_READ	0x0000000000000001
#define EFI_FILE_MODE_WRITE	0x0000000000000002
#define EFI_FILE_MODE_CREATE	0x8000000000000000

#define EFI_FILE_READ_ONLY	0x0000000000000001
#define EFI_FILE_HIDDEN		0x0000000000000002
#define EFI_FILE_SYSTEM		0x0000000000000004
#define EFI_FILE_RESERVED	0x0000000000000008
#define EFI_FILE_DIRECTORY	0x0000000000000010
#define EFI_FILE_ARCHIVE	0x0000000000000020
#define EFI_FILE_VALID_ATTR	0x0000000000000037

struct efi_file_info {
	u64 size;
	u64 file_size;
	u64 physical_size;
	struct efi_time create_time;
	struct efi_time last_access_time;
	struct efi_time modification_time;
	u64 attribute;
	u16 file_name[0];
};

struct efi_file_system_info {
	u64 size;
	u8 read_only;
	u64 volume_size;
	u64 free_space;
	u32 block_size;
	u16 volume_label[0];
};

#define EFI_DRIVER_BINDING_PROTOCOL_GUID \
	EFI_GUID(0x18a031ab, 0xb443, 0x4d1a,\
		 0xa5, 0xc0, 0x0c, 0x09, 0x26, 0x1e, 0x9f, 0x71)
struct efi_driver_binding_protocol {
	efi_status_t (EFIAPI * supported)(
			struct efi_driver_binding_protocol *this,
			efi_handle_t controller_handle,
			struct efi_device_path *remaining_device_path);
	efi_status_t (EFIAPI * start)(
			struct efi_driver_binding_protocol *this,
			efi_handle_t controller_handle,
			struct efi_device_path *remaining_device_path);
	efi_status_t (EFIAPI * stop)(
			struct efi_driver_binding_protocol *this,
			efi_handle_t controller_handle,
			efi_uintn_t number_of_children,
			efi_handle_t *child_handle_buffer);
	u32 version;
	efi_handle_t image_handle;
	efi_handle_t driver_binding_handle;
};

/* Current version of the Unicode collation protocol */
#define EFI_UNICODE_COLLATION_PROTOCOL2_GUID \
	EFI_GUID(0xa4c751fc, 0x23ae, 0x4c3e, \
		 0x92, 0xe9, 0x49, 0x64, 0xcf, 0x63, 0xf3, 0x49)
struct efi_unicode_collation_protocol {
	efi_intn_t (EFIAPI *stri_coll)(
		struct efi_unicode_collation_protocol *this, u16 *s1, u16 *s2);
	bool (EFIAPI *metai_match)(struct efi_unicode_collation_protocol *this,
				   const u16 *string, const u16 *patter);
	void (EFIAPI *str_lwr)(struct efi_unicode_collation_protocol
			       *this, u16 *string);
	void (EFIAPI *str_upr)(struct efi_unicode_collation_protocol *this,
			       u16 *string);
	void (EFIAPI *fat_to_str)(struct efi_unicode_collation_protocol *this,
				  efi_uintn_t fat_size, char *fat, u16 *string);
	bool (EFIAPI *str_to_fat)(struct efi_unicode_collation_protocol *this,
				  const u16 *string, efi_uintn_t fat_size,
				  char *fat);
	char *supported_languages;
};

struct efi_load_file_protocol {
	efi_status_t (EFIAPI *load_file)(struct efi_load_file_protocol *this,
					 struct efi_device_path *file_path,
					 bool boot_policy,
					 efi_uintn_t *buffer_size,
					 void *buffer);
};

struct efi_system_resource_entry {
	efi_guid_t fw_class;
	u32 fw_type;
	u32 fw_version;
	u32 lowest_supported_fw_version;
	u32 capsule_flags;
	u32 last_attempt_version;
	u32 last_attempt_status;
} __packed;

struct efi_system_resource_table {
	u32 fw_resource_count;
	u32 fw_resource_count_max;
	u64 fw_resource_version;
	struct efi_system_resource_entry entries[];
} __packed;

/* Boot manager load options */
#define LOAD_OPTION_ACTIVE		0x00000001
#define LOAD_OPTION_FORCE_RECONNECT	0x00000002
#define LOAD_OPTION_HIDDEN		0x00000008
/* All values 0x00000200-0x00001F00 are reserved */
#define LOAD_OPTION_CATEGORY		0x00001F00
#define LOAD_OPTION_CATEGORY_BOOT	0x00000000
#define LOAD_OPTION_CATEGORY_APP	0x00000100

/*
 * System Resource Table
 */
/* Firmware Type Definitions */
#define ESRT_FW_TYPE_UNKNOWN		0x00000000
#define ESRT_FW_TYPE_SYSTEMFIRMWARE	0x00000001
#define ESRT_FW_TYPE_DEVICEFIRMWARE	0x00000002
#define ESRT_FW_TYPE_UEFIDRIVER		0x00000003

#define EFI_SYSTEM_RESOURCE_TABLE_GUID\
	EFI_GUID(0xb122a263, 0x3661, 0x4f68,\
		0x99, 0x29, 0x78, 0xf8, 0xb0, 0xd6, 0x21, 0x80)

/* Last Attempt Status Values */
#define LAST_ATTEMPT_STATUS_SUCCESS			0x00000000
#define LAST_ATTEMPT_STATUS_ERROR_UNSUCCESSFUL		0x00000001
#define LAST_ATTEMPT_STATUS_ERROR_INSUFFICIENT_RESOURCES 0x00000002
#define LAST_ATTEMPT_STATUS_ERROR_INCORRECT_VERSION	0x00000003
#define LAST_ATTEMPT_STATUS_ERROR_INVALID_FORMAT	0x00000004
#define LAST_ATTEMPT_STATUS_ERROR_AUTH_ERROR		0x00000005
#define LAST_ATTEMPT_STATUS_ERROR_PWR_EVT_AC		0x00000006
#define LAST_ATTEMPT_STATUS_ERROR_PWR_EVT_BATT		0x00000007
#define LAST_ATTEMPT_STATUS_ERROR_UNSATISFIED_DEPENDENCIES 0x00000008

/*
 * The LastAttemptStatus values of 0x1000 - 0x4000 are reserved for vendor
 * usage.
 */
#define LAST_ATTEMPT_STATUS_ERROR_UNSUCCESSFUL_VENDOR_RANGE_MIN 0x00001000
#define LAST_ATTEMPT_STATUS_ERROR_UNSUCCESSFUL_VENDOR_RANGE_MAX 0x00004000

/* Certificate types in signature database */
#define EFI_CERT_SHA1_GUID \
	EFI_GUID(0x826ca512, 0xcf10, 0x4ac9, 0xb1, 0x87, \
		 0xbe, 0x01, 0x49, 0x66, 0x31, 0xbd)
#define EFI_CERT_SHA224_GUID \
	EFI_GUID(0xb6e5233, 0xa65c, 0x44c9, 0x94, 0x07, \
		 0xd9, 0xab, 0x83, 0xbf, 0xc8, 0xbd)
#define EFI_CERT_SHA256_GUID \
	EFI_GUID(0xc1c41626, 0x504c, 0x4092, 0xac, 0xa9, \
		 0x41, 0xf9, 0x36, 0x93, 0x43, 0x28)
#define EFI_CERT_SHA384_GUID \
	EFI_GUID(0xff3e5307, 0x9fd0, 0x48c9, 0x85, 0xf1, \
		 0x8a, 0xd5, 0x6c, 0x70, 0x1e, 0x01)
#define EFI_CERT_SHA512_GUID \
	EFI_GUID(0x93e0fae, 0xa6c4, 0x4f50, 0x9f, 0x1b, \
		 0xd4, 0x1e, 0x2b, 0x89, 0xc1, 0x9a)
#define EFI_CERT_RSA2048_GUID \
	EFI_GUID(0x3c5766e8, 0x269c, 0x4e34, 0xaa, 0x14, \
		 0xed, 0x77, 0x6e, 0x85, 0xb3, 0xb6)
#define EFI_CERT_X509_GUID \
	EFI_GUID(0xa5c059a1, 0x94e4, 0x4aa7, 0x87, 0xb5, \
		 0xab, 0x15, 0x5c, 0x2b, 0xf0, 0x72)
#define EFI_CERT_X509_SHA256_GUID \
	EFI_GUID(0x3bd2a492, 0x96c0, 0x4079, 0xb4, 0x20, \
		 0xfc, 0xf9, 0x8e, 0xf1, 0x03, 0xed)
#define EFI_CERT_X509_SHA384_GUID \
	EFI_GUID(0x7076876e, 0x80c2, 0x4ee6,		\
		 0xaa, 0xd2, 0x28, 0xb3, 0x49, 0xa6, 0x86, 0x5b)
#define EFI_CERT_X509_SHA512_GUID \
	EFI_GUID(0x446dbf63, 0x2502, 0x4cda,		\
		 0xbc, 0xfa, 0x24, 0x65, 0xd2, 0xb0, 0xfe, 0x9d)
#define EFI_CERT_TYPE_PKCS7_GUID \
	EFI_GUID(0x4aafd29d, 0x68df, 0x49ee, 0x8a, 0xa9, \
		 0x34, 0x7d, 0x37, 0x56, 0x65, 0xa7)

#define EFI_LZMA_COMPRESSED \
	EFI_GUID(0xee4e5898, 0x3914, 0x4259, 0x9d, 0x6e, \
		 0xdc, 0x7b, 0xd7, 0x94, 0x03, 0xcf)
#define EFI_DXE_SERVICES \
	EFI_GUID(0x05ad34ba, 0x6f02, 0x4214, 0x95, 0x2e, \
		 0x4d, 0xa0, 0x39, 0x8e, 0x2b, 0xb9)
#define EFI_HOB_LIST \
	EFI_GUID(0x7739f24c, 0x93d7, 0x11d4, 0x9a, 0x3a,  \
		 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d)
#define EFI_MEMORY_TYPE \
	EFI_GUID(0x4c19049f, 0x4137, 0x4dd3, 0x9c, 0x10, \
		 0x8b, 0x97, 0xa8, 0x3f, 0xfd, 0xfa)
#define EFI_MEM_STATUS_CODE_REC \
	EFI_GUID(0x060cc026, 0x4c0d, 0x4dda, 0x8f, 0x41, \
		 0x59, 0x5f, 0xef, 0x00, 0xa5, 0x02)
#define EFI_GUID_EFI_ACPI1 \
	EFI_GUID(0xeb9d2d30, 0x2d88, 0x11d3,  0x9a, 0x16, \
		 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d)

/**
 * struct win_certificate_uefi_guid - A certificate that encapsulates
 * a GUID-specific signature
 *
 * @hdr:	Windows certificate header
 * @cert_type:	Certificate type
 * @cert_data:	Certificate data
 */
struct win_certificate_uefi_guid {
	WIN_CERTIFICATE	hdr;
	efi_guid_t	cert_type;
	u8		cert_data[];
} __attribute__((__packed__));

/**
 * struct efi_variable_authentication_2 - A time-based authentication method
 * descriptor
 *
 * This structure describes an authentication information for
 * a variable with EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS
 * and should be included as part of a variable's value.
 * Only EFI_CERT_TYPE_PKCS7_GUID is accepted.
 *
 * @time_stamp:	Descriptor's time stamp
 * @auth_info:	Authentication info
 */
struct efi_variable_authentication_2 {
	struct efi_time			 time_stamp;
	struct win_certificate_uefi_guid auth_info;
} __attribute__((__packed__));

/**
 * struct efi_firmware_image_authentication - Capsule authentication method
 * descriptor
 *
 * This structure describes an authentication information for
 * a capsule with IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED set
 * and should be included as part of the capsule.
 * Only EFI_CERT_TYPE_PKCS7_GUID is accepted.
 *
 * @monotonic_count: Count to prevent replay
 * @auth_info: Authentication info
 */
struct efi_firmware_image_authentication {
	uint64_t monotonic_count;
	struct win_certificate_uefi_guid auth_info;
} __attribute__((__packed__));


/**
 * struct efi_signature_data - A format of signature
 *
 * This structure describes a single signature in signature database.
 *
 * @signature_owner:	Signature owner
 * @signature_data:	Signature data
 */
struct efi_signature_data {
	efi_guid_t	signature_owner;
	u8		signature_data[];
} __attribute__((__packed__));

/**
 * struct efi_signature_list - A format of signature database
 *
 * This structure describes a list of signatures with the same type.
 * An authenticated variable's value is a concatenation of one or more
 * efi_signature_list's.
 *
 * @signature_type:		Signature type
 * @signature_list_size:	Size of signature list
 * @signature_header_size:	Size of signature header
 * @signature_size:		Size of signature
 */
struct efi_signature_list {
	efi_guid_t	signature_type;
	u32		signature_list_size;
	u32		signature_header_size;
	u32		signature_size;
/*	u8		signature_header[signature_header_size]; */
/*	struct efi_signature_data signatures[...][signature_size]; */
} __attribute__((__packed__));

/*
 * Firmware management protocol
 */
#define EFI_FIRMWARE_MANAGEMENT_PROTOCOL_GUID \
	EFI_GUID(0x86c77a67, 0x0b97, 0x4633, 0xa1, 0x87, \
		 0x49, 0x10, 0x4d, 0x06, 0x85, 0xc7)

#define IMAGE_ATTRIBUTE_IMAGE_UPDATABLE		0x0000000000000001
#define IMAGE_ATTRIBUTE_RESET_REQUIRED		0x0000000000000002
#define IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED	0x0000000000000004
#define IMAGE_ATTRIBUTE_IN_USE			0x0000000000000008
#define IMAGE_ATTRIBUTE_UEFI_IMAGE		0x0000000000000010
#define IMAGE_ATTRIBUTE_DEPENDENCY		0x0000000000000020

#define IMAGE_COMPATIBILITY_CHECK_SUPPORTED	0x0000000000000001

#define IMAGE_UPDATABLE_VALID			0x0000000000000001
#define IMAGE_UPDATABLE_INVALID			0x0000000000000002
#define IMAGE_UPDATABLE_INVALID_TYPE		0x0000000000000004
#define IMAGE_UPDATABLE_INVALID_OLLD		0x0000000000000008
#define IMAGE_UPDATABLE_VALID_WITH_VENDOR_CODE	0x0000000000000010

#define PACKAGE_ATTRIBUTE_VERSION_UPDATABLE		0x0000000000000001
#define PACKAGE_ATTRIBUTE_RESET_REQUIRED		0x0000000000000002
#define PACKAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED	0x0000000000000004

#define EFI_FIRMWARE_IMAGE_DESCRIPTOR_VERSION	4

typedef struct efi_firmware_image_dependencies {
	u8 dependencies[0];
} efi_firmware_image_dep_t;

struct efi_firmware_image_descriptor {
	u8 image_index;
	efi_guid_t image_type_id;
	u64 image_id;
	u16 *image_id_name;
	u32 version;
	u16 *version_name;
	efi_uintn_t size;
	u64 attributes_supported;
	u64 attributes_setting;
	u64 compatibilities;
	u32 lowest_supported_image_version;
	u32 last_attempt_version;
	u32 last_attempt_status;
	u64 hardware_instance;
	efi_firmware_image_dep_t *dependencies;
};

struct efi_firmware_management_protocol {
	efi_status_t (EFIAPI *get_image_info)(
			struct efi_firmware_management_protocol *this,
			efi_uintn_t *image_info_size,
			struct efi_firmware_image_descriptor *image_info,
			u32 *descriptor_version,
			u8 *descriptor_count,
			efi_uintn_t *descriptor_size,
			u32 *package_version,
			u16 **package_version_name);
	efi_status_t (EFIAPI *get_image)(
			struct efi_firmware_management_protocol *this,
			u8 image_index,
			void *image,
			efi_uintn_t *image_size);
	efi_status_t (EFIAPI *set_image)(
			struct efi_firmware_management_protocol *this,
			u8 image_index,
			const void *image,
			efi_uintn_t image_size,
			const void *vendor_code,
			efi_status_t (*progress)(efi_uintn_t completion),
			u16 **abort_reason);
	efi_status_t (EFIAPI *check_image)(
			struct efi_firmware_management_protocol *this,
			u8 image_index,
			const void *image,
			efi_uintn_t *image_size,
			u32 *image_updatable);
	efi_status_t (EFIAPI *get_package_info)(
			struct efi_firmware_management_protocol *this,
			u32 *package_version,
			u16 **package_version_name,
			u32 *package_version_name_maxlen,
			u64 *attributes_supported,
			u64 *attributes_setting);
	efi_status_t (EFIAPI *set_package_info)(
			struct efi_firmware_management_protocol *this,
			const void *image,
			efi_uintn_t *image_size,
			const void *vendor_code,
			u32 package_version,
			const u16 *package_version_name);
};

#define EFI_DISK_IO_PROTOCOL_GUID	\
	EFI_GUID(0xce345171, 0xba0b, 0x11d2, 0x8e, 0x4f, \
		 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b)

struct efi_disk {
	u64 revision;
	efi_status_t (EFIAPI *read_disk)(struct efi_disk *this, u32 media_id,
					 u64 offset, efi_uintn_t buffer_size,
					 void *buffer);

	efi_status_t (EFIAPI *write_disk)(struct efi_disk *this, u32 media_id,
					  u64 offset, efi_uintn_t buffer_size,
					  void *buffer);
};

#endif
