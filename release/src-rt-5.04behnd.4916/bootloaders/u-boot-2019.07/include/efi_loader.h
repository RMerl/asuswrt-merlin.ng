/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  EFI application loader
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#ifndef _EFI_LOADER_H
#define _EFI_LOADER_H 1

#include <common.h>
#include <part_efi.h>
#include <efi_api.h>

/* No need for efi loader support in SPL */
#if CONFIG_IS_ENABLED(EFI_LOADER)

#include <linux/list.h>

/* Maximum number of configuration tables */
#define EFI_MAX_CONFIGURATION_TABLES 16

/* GUID used by the root node */
#define U_BOOT_GUID \
	EFI_GUID(0xe61d73b9, 0xa384, 0x4acc, \
		 0xae, 0xab, 0x82, 0xe8, 0x28, 0xf3, 0x62, 0x8b)

/* Root node */
extern efi_handle_t efi_root;

int __efi_entry_check(void);
int __efi_exit_check(void);
const char *__efi_nesting(void);
const char *__efi_nesting_inc(void);
const char *__efi_nesting_dec(void);

/*
 * Enter the u-boot world from UEFI:
 */
#define EFI_ENTRY(format, ...) do { \
	assert(__efi_entry_check()); \
	debug("%sEFI: Entry %s(" format ")\n", __efi_nesting_inc(), \
		__func__, ##__VA_ARGS__); \
	} while(0)

/*
 * Exit the u-boot world back to UEFI:
 */
#define EFI_EXIT(ret) ({ \
	typeof(ret) _r = ret; \
	debug("%sEFI: Exit: %s: %u\n", __efi_nesting_dec(), \
		__func__, (u32)((uintptr_t) _r & ~EFI_ERROR_MASK)); \
	assert(__efi_exit_check()); \
	_r; \
	})

/*
 * Call non-void UEFI function from u-boot and retrieve return value:
 */
#define EFI_CALL(exp) ({ \
	debug("%sEFI: Call: %s\n", __efi_nesting_inc(), #exp); \
	assert(__efi_exit_check()); \
	typeof(exp) _r = exp; \
	assert(__efi_entry_check()); \
	debug("%sEFI: %lu returned by %s\n", __efi_nesting_dec(), \
	      (unsigned long)((uintptr_t)_r & ~EFI_ERROR_MASK), #exp); \
	_r; \
})

/*
 * Call void UEFI function from u-boot:
 */
#define EFI_CALL_VOID(exp) do { \
	debug("%sEFI: Call: %s\n", __efi_nesting_inc(), #exp); \
	assert(__efi_exit_check()); \
	exp; \
	assert(__efi_entry_check()); \
	debug("%sEFI: Return From: %s\n", __efi_nesting_dec(), #exp); \
	} while(0)

/*
 * Write an indented message with EFI prefix
 */
#define EFI_PRINT(format, ...) ({ \
	debug("%sEFI: " format, __efi_nesting(), \
		##__VA_ARGS__); \
	})

#ifdef CONFIG_SYS_CACHELINE_SIZE
#define EFI_CACHELINE_SIZE CONFIG_SYS_CACHELINE_SIZE
#else
/* Just use the greatest cache flush alignment requirement I'm aware of */
#define EFI_CACHELINE_SIZE 128
#endif

/* Key identifying current memory map */
extern efi_uintn_t efi_memory_map_key;

extern struct efi_runtime_services efi_runtime_services;
extern struct efi_system_table systab;

extern struct efi_simple_text_output_protocol efi_con_out;
extern struct efi_simple_text_input_protocol efi_con_in;
extern struct efi_console_control_protocol efi_console_control;
extern const struct efi_device_path_to_text_protocol efi_device_path_to_text;
/* implementation of the EFI_DEVICE_PATH_UTILITIES_PROTOCOL */
extern const struct efi_device_path_utilities_protocol
					efi_device_path_utilities;
/* deprecated version of the EFI_UNICODE_COLLATION_PROTOCOL */
extern const struct efi_unicode_collation_protocol
					efi_unicode_collation_protocol;
/* current version of the EFI_UNICODE_COLLATION_PROTOCOL */
extern const struct efi_unicode_collation_protocol
					efi_unicode_collation_protocol2;
extern const struct efi_hii_config_routing_protocol efi_hii_config_routing;
extern const struct efi_hii_config_access_protocol efi_hii_config_access;
extern const struct efi_hii_database_protocol efi_hii_database;
extern const struct efi_hii_string_protocol efi_hii_string;

uint16_t *efi_dp_str(struct efi_device_path *dp);

/* GUID of the U-Boot root node */
extern const efi_guid_t efi_u_boot_guid;
/* GUID of the EFI_BLOCK_IO_PROTOCOL */
extern const efi_guid_t efi_block_io_guid;
extern const efi_guid_t efi_global_variable_guid;
extern const efi_guid_t efi_guid_console_control;
extern const efi_guid_t efi_guid_device_path;
/* GUID of the EFI_DRIVER_BINDING_PROTOCOL */
extern const efi_guid_t efi_guid_driver_binding_protocol;
/* event group ExitBootServices() invoked */
extern const efi_guid_t efi_guid_event_group_exit_boot_services;
/* event group SetVirtualAddressMap() invoked */
extern const efi_guid_t efi_guid_event_group_virtual_address_change;
/* event group memory map changed */
extern const efi_guid_t efi_guid_event_group_memory_map_change;
/* event group boot manager about to boot */
extern const efi_guid_t efi_guid_event_group_ready_to_boot;
/* event group ResetSystem() invoked (before ExitBootServices) */
extern const efi_guid_t efi_guid_event_group_reset_system;
/* GUID of the device tree table */
extern const efi_guid_t efi_guid_fdt;
extern const efi_guid_t efi_guid_loaded_image;
extern const efi_guid_t efi_guid_loaded_image_device_path;
extern const efi_guid_t efi_guid_device_path_to_text_protocol;
extern const efi_guid_t efi_simple_file_system_protocol_guid;
extern const efi_guid_t efi_file_info_guid;
/* GUID for file system information */
extern const efi_guid_t efi_file_system_info_guid;
extern const efi_guid_t efi_guid_device_path_utilities_protocol;
/* GUID of the deprecated Unicode collation protocol */
extern const efi_guid_t efi_guid_unicode_collation_protocol;
/* GUID of the Unicode collation protocol */
extern const efi_guid_t efi_guid_unicode_collation_protocol2;
extern const efi_guid_t efi_guid_hii_config_routing_protocol;
extern const efi_guid_t efi_guid_hii_config_access_protocol;
extern const efi_guid_t efi_guid_hii_database_protocol;
extern const efi_guid_t efi_guid_hii_string_protocol;

extern unsigned int __efi_runtime_start, __efi_runtime_stop;
extern unsigned int __efi_runtime_rel_start, __efi_runtime_rel_stop;

/**
 * struct efi_open_protocol_info_item - open protocol info item
 *
 * When a protocol is opened a open protocol info entry is created.
 * These are maintained in a list.
 *
 * @link:	link to the list of open protocol info entries of a protocol
 * @info:	information about the opening of a protocol
 */
struct efi_open_protocol_info_item {
	struct list_head link;
	struct efi_open_protocol_info_entry info;
};

/**
 * struct efi_handler - single protocol interface of a handle
 *
 * When the UEFI payload wants to open a protocol on an object to get its
 * interface (usually a struct with callback functions), this struct maps the
 * protocol GUID to the respective protocol interface
 *
 * @link:		link to the list of protocols of a handle
 * @guid:		GUID of the protocol
 * @protocol_interface:	protocol interface
 * @open_infos		link to the list of open protocol info items
 */
struct efi_handler {
	struct list_head link;
	const efi_guid_t *guid;
	void *protocol_interface;
	struct list_head open_infos;
};

/**
 * enum efi_object_type - type of EFI object
 *
 * In UnloadImage we must be able to identify if the handle relates to a
 * started image.
 */
enum efi_object_type {
	EFI_OBJECT_TYPE_UNDEFINED = 0,
	EFI_OBJECT_TYPE_U_BOOT_FIRMWARE,
	EFI_OBJECT_TYPE_LOADED_IMAGE,
	EFI_OBJECT_TYPE_STARTED_IMAGE,
};

/**
 * struct efi_object - dereferenced EFI handle
 *
 * @link:	pointers to put the handle into a linked list
 * @protocols:	linked list with the protocol interfaces installed on this
 *		handle
 *
 * UEFI offers a flexible and expandable object model. The objects in the UEFI
 * API are devices, drivers, and loaded images. struct efi_object is our storage
 * structure for these objects.
 *
 * When including this structure into a larger structure always put it first so
 * that when deleting a handle the whole encompassing structure can be freed.
 *
 * A pointer to this structure is referred to as a handle. Typedef efi_handle_t
 * has been created for such pointers.
 */
struct efi_object {
	/* Every UEFI object is part of a global object list */
	struct list_head link;
	/* The list of protocols */
	struct list_head protocols;
	enum efi_object_type type;
};

/**
 * struct efi_loaded_image_obj - handle of a loaded image
 *
 * @header:		EFI object header
 * @exit_status:	exit status passed to Exit()
 * @exit_data_size:	exit data size passed to Exit()
 * @exit_data:		exit data passed to Exit()
 * @exit_jmp:		long jump buffer for returning form started image
 * @entry:		entry address of the relocated image
 */
struct efi_loaded_image_obj {
	struct efi_object header;
	efi_status_t exit_status;
	efi_uintn_t *exit_data_size;
	u16 **exit_data;
	struct jmp_buf_data exit_jmp;
	EFIAPI efi_status_t (*entry)(efi_handle_t image_handle,
				     struct efi_system_table *st);
	u16 image_type;
};

/**
 * struct efi_event
 *
 * @link:		Link to list of all events
 * @queue_link:		Link to the list of queued events
 * @type:		Type of event, see efi_create_event
 * @notify_tpl:		Task priority level of notifications
 * @nofify_function:	Function to call when the event is triggered
 * @notify_context:	Data to be passed to the notify function
 * @group:		Event group
 * @trigger_time:	Period of the timer
 * @trigger_next:	Next time to trigger the timer
 * @trigger_type:	Type of timer, see efi_set_timer
 * @is_signaled:	The event occurred. The event is in the signaled state.
 */
struct efi_event {
	struct list_head link;
	struct list_head queue_link;
	uint32_t type;
	efi_uintn_t notify_tpl;
	void (EFIAPI *notify_function)(struct efi_event *event, void *context);
	void *notify_context;
	const efi_guid_t *group;
	u64 trigger_next;
	u64 trigger_time;
	enum efi_timer_delay trigger_type;
	bool is_signaled;
};

/* This list contains all UEFI objects we know of */
extern struct list_head efi_obj_list;
/* List of all events */
extern struct list_head efi_events;

/**
 * struct efi_protocol_notification - handle for notified protocol
 *
 * When a protocol interface is installed for which an event was registered with
 * the RegisterProtocolNotify() service this structure is used to hold the
 * handle on which the protocol interface was installed.
 *
 * @link:	link to list of all handles notified for this event
 * @handle:	handle on which the notified protocol interface was installed
 */
struct efi_protocol_notification {
	struct list_head link;
	efi_handle_t handle;
};

/**
 * efi_register_notify_event - event registered by RegisterProtocolNotify()
 *
 * The address of this structure serves as registration value.
 *
 * @link:	link to list of all registered events
 * @event:	registered event. The same event may registered for multiple
 *		GUIDs.
 * @protocol:	protocol for which the event is registered
 * @handles:	linked list of all handles on which the notified protocol was
 *		installed
 */
struct efi_register_notify_event {
	struct list_head link;
	struct efi_event *event;
	efi_guid_t protocol;
	struct list_head handles;
};

/* List of all events registered by RegisterProtocolNotify() */
extern struct list_head efi_register_notify_events;

/* Initialize efi execution environment */
efi_status_t efi_init_obj_list(void);
/* Called by bootefi to initialize root node */
efi_status_t efi_root_node_register(void);
/* Called by bootefi to initialize runtime */
efi_status_t efi_initialize_system_table(void);
/* Called by bootefi to make console interface available */
efi_status_t efi_console_register(void);
/* Called by bootefi to make all disk storage accessible as EFI objects */
efi_status_t efi_disk_register(void);
/* Create handles and protocols for the partitions of a block device */
int efi_disk_create_partitions(efi_handle_t parent, struct blk_desc *desc,
			       const char *if_typename, int diskid,
			       const char *pdevname);
/* Called by bootefi to make GOP (graphical) interface available */
efi_status_t efi_gop_register(void);
/* Called by bootefi to make the network interface available */
efi_status_t efi_net_register(void);
/* Called by bootefi to make the watchdog available */
efi_status_t efi_watchdog_register(void);
/* Called by bootefi to make SMBIOS tables available */
/**
 * efi_acpi_register() - write out ACPI tables
 *
 * Called by bootefi to make ACPI tables available
 *
 * @return 0 if OK, -ENOMEM if no memory is available for the tables
 */
efi_status_t efi_acpi_register(void);
/**
 * efi_smbios_register() - write out SMBIOS tables
 *
 * Called by bootefi to make SMBIOS tables available
 *
 * @return 0 if OK, -ENOMEM if no memory is available for the tables
 */
efi_status_t efi_smbios_register(void);

struct efi_simple_file_system_protocol *
efi_fs_from_path(struct efi_device_path *fp);

/* Called by networking code to memorize the dhcp ack package */
void efi_net_set_dhcp_ack(void *pkt, int len);
/* Called by efi_set_watchdog_timer to reset the timer */
efi_status_t efi_set_watchdog(unsigned long timeout);

/* Called from places to check whether a timer expired */
void efi_timer_check(void);
/* PE loader implementation */
efi_status_t efi_load_pe(struct efi_loaded_image_obj *handle, void *efi,
			 struct efi_loaded_image *loaded_image_info);
/* Called once to store the pristine gd pointer */
void efi_save_gd(void);
/* Special case handler for error/abort that just tries to dtrt to get
 * back to u-boot world */
void efi_restore_gd(void);
/* Call this to relocate the runtime section to an address space */
void efi_runtime_relocate(ulong offset, struct efi_mem_desc *map);
/* Call this to set the current device name */
void efi_set_bootdev(const char *dev, const char *devnr, const char *path);
/* Add a new object to the object list. */
void efi_add_handle(efi_handle_t obj);
/* Create handle */
efi_status_t efi_create_handle(efi_handle_t *handle);
/* Delete handle */
void efi_delete_handle(efi_handle_t obj);
/* Call this to validate a handle and find the EFI object for it */
struct efi_object *efi_search_obj(const efi_handle_t handle);
/* Load image */
efi_status_t EFIAPI efi_load_image(bool boot_policy,
				   efi_handle_t parent_image,
				   struct efi_device_path *file_path,
				   void *source_buffer,
				   efi_uintn_t source_size,
				   efi_handle_t *image_handle);
/* Start image */
efi_status_t EFIAPI efi_start_image(efi_handle_t image_handle,
				    efi_uintn_t *exit_data_size,
				    u16 **exit_data);
/* Unload image */
efi_status_t EFIAPI efi_unload_image(efi_handle_t image_handle);
/* Find a protocol on a handle */
efi_status_t efi_search_protocol(const efi_handle_t handle,
				 const efi_guid_t *protocol_guid,
				 struct efi_handler **handler);
/* Install new protocol on a handle */
efi_status_t efi_add_protocol(const efi_handle_t handle,
			      const efi_guid_t *protocol,
			      void *protocol_interface);
/* Delete protocol from a handle */
efi_status_t efi_remove_protocol(const efi_handle_t handle,
				 const efi_guid_t *protocol,
				 void *protocol_interface);
/* Delete all protocols from a handle */
efi_status_t efi_remove_all_protocols(const efi_handle_t handle);
/* Install multiple protocol interfaces */
efi_status_t EFIAPI efi_install_multiple_protocol_interfaces
				(efi_handle_t *handle, ...);
/* Call this to create an event */
efi_status_t efi_create_event(uint32_t type, efi_uintn_t notify_tpl,
			      void (EFIAPI *notify_function) (
					struct efi_event *event,
					void *context),
			      void *notify_context, efi_guid_t *group,
			      struct efi_event **event);
/* Call this to set a timer */
efi_status_t efi_set_timer(struct efi_event *event, enum efi_timer_delay type,
			   uint64_t trigger_time);
/* Call this to signal an event */
void efi_signal_event(struct efi_event *event);

/* open file system: */
struct efi_simple_file_system_protocol *efi_simple_file_system(
		struct blk_desc *desc, int part, struct efi_device_path *dp);

/* open file from device-path: */
struct efi_file_handle *efi_file_from_path(struct efi_device_path *fp);

/**
 * efi_size_in_pages() - convert size in bytes to size in pages
 *
 * This macro returns the number of EFI memory pages required to hold 'size'
 * bytes.
 *
 * @size:	size in bytes
 * Return:	size in pages
 */
#define efi_size_in_pages(size) ((size + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT)
/* Generic EFI memory allocator, call this to get memory */
void *efi_alloc(uint64_t len, int memory_type);
/* More specific EFI memory allocator, called by EFI payloads */
efi_status_t efi_allocate_pages(int type, int memory_type, efi_uintn_t pages,
				uint64_t *memory);
/* EFI memory free function. */
efi_status_t efi_free_pages(uint64_t memory, efi_uintn_t pages);
/* EFI memory allocator for small allocations */
efi_status_t efi_allocate_pool(int pool_type, efi_uintn_t size,
			       void **buffer);
/* EFI pool memory free function. */
efi_status_t efi_free_pool(void *buffer);
/* Returns the EFI memory map */
efi_status_t efi_get_memory_map(efi_uintn_t *memory_map_size,
				struct efi_mem_desc *memory_map,
				efi_uintn_t *map_key,
				efi_uintn_t *descriptor_size,
				uint32_t *descriptor_version);
/* Adds a range into the EFI memory map */
uint64_t efi_add_memory_map(uint64_t start, uint64_t pages, int memory_type,
			    bool overlap_only_ram);
/* Called by board init to initialize the EFI drivers */
efi_status_t efi_driver_init(void);
/* Called by board init to initialize the EFI memory map */
int efi_memory_init(void);
/* Adds new or overrides configuration table entry to the system table */
efi_status_t efi_install_configuration_table(const efi_guid_t *guid, void *table);
/* Sets up a loaded image */
efi_status_t efi_setup_loaded_image(struct efi_device_path *device_path,
				    struct efi_device_path *file_path,
				    struct efi_loaded_image_obj **handle_ptr,
				    struct efi_loaded_image **info_ptr);
/* Print information about all loaded images */
void efi_print_image_infos(void *pc);

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
extern void *efi_bounce_buffer;
#define EFI_LOADER_BOUNCE_BUFFER_SIZE (64 * 1024 * 1024)
#endif


struct efi_device_path *efi_dp_next(const struct efi_device_path *dp);
int efi_dp_match(const struct efi_device_path *a,
		 const struct efi_device_path *b);
struct efi_object *efi_dp_find_obj(struct efi_device_path *dp,
				   struct efi_device_path **rem);
/* get size of the first device path instance excluding end node */
efi_uintn_t efi_dp_instance_size(const struct efi_device_path *dp);
/* size of multi-instance device path excluding end node */
efi_uintn_t efi_dp_size(const struct efi_device_path *dp);
struct efi_device_path *efi_dp_dup(const struct efi_device_path *dp);
struct efi_device_path *efi_dp_append(const struct efi_device_path *dp1,
				      const struct efi_device_path *dp2);
struct efi_device_path *efi_dp_append_node(const struct efi_device_path *dp,
					   const struct efi_device_path *node);
/* Create a device path node of given type, sub-type, length */
struct efi_device_path *efi_dp_create_device_node(const u8 type,
						  const u8 sub_type,
						  const u16 length);
/* Append device path instance */
struct efi_device_path *efi_dp_append_instance(
		const struct efi_device_path *dp,
		const struct efi_device_path *dpi);
/* Get next device path instance */
struct efi_device_path *efi_dp_get_next_instance(struct efi_device_path **dp,
						 efi_uintn_t *size);
/* Check if a device path contains muliple instances */
bool efi_dp_is_multi_instance(const struct efi_device_path *dp);

struct efi_device_path *efi_dp_from_dev(struct udevice *dev);
struct efi_device_path *efi_dp_from_part(struct blk_desc *desc, int part);
/* Create a device node for a block device partition. */
struct efi_device_path *efi_dp_part_node(struct blk_desc *desc, int part);
struct efi_device_path *efi_dp_from_file(struct blk_desc *desc, int part,
					 const char *path);
struct efi_device_path *efi_dp_from_eth(void);
struct efi_device_path *efi_dp_from_mem(uint32_t mem_type,
					uint64_t start_address,
					uint64_t end_address);
/* Determine the last device path node that is not the end node. */
const struct efi_device_path *efi_dp_last_node(
			const struct efi_device_path *dp);
efi_status_t efi_dp_split_file_path(struct efi_device_path *full_path,
				    struct efi_device_path **device_path,
				    struct efi_device_path **file_path);
efi_status_t efi_dp_from_name(const char *dev, const char *devnr,
			      const char *path,
			      struct efi_device_path **device,
			      struct efi_device_path **file);

#define EFI_DP_TYPE(_dp, _type, _subtype) \
	(((_dp)->type == DEVICE_PATH_TYPE_##_type) && \
	 ((_dp)->sub_type == DEVICE_PATH_SUB_TYPE_##_subtype))

/**
 * ascii2unicode() - convert ASCII string to UTF-16 string
 *
 * A zero terminated ASCII string is converted to a zero terminated UTF-16
 * string. The output buffer must be preassigned.
 *
 * @unicode:	preassigned output buffer for UTF-16 string
 * @ascii:	ASCII string to be converted
 */
static inline void ascii2unicode(u16 *unicode, const char *ascii)
{
	while (*ascii)
		*(unicode++) = *(ascii++);
	*unicode = 0;
}

static inline int guidcmp(const efi_guid_t *g1, const efi_guid_t *g2)
{
	return memcmp(g1, g2, sizeof(efi_guid_t));
}

/*
 * Use these to indicate that your code / data should go into the EFI runtime
 * section and thus still be available when the OS is running
 */
#define __efi_runtime_data __attribute__ ((section (".data.efi_runtime")))
#define __efi_runtime __attribute__ ((section (".text.efi_runtime")))

/* Indicate supported runtime services */
efi_status_t efi_init_runtime_supported(void);

/* Update CRC32 in table header */
void __efi_runtime efi_update_table_header_crc32(struct efi_table_hdr *table);

/* Call this with mmio_ptr as the _pointer_ to a pointer to an MMIO region
 * to make it available at runtime */
efi_status_t efi_add_runtime_mmio(void *mmio_ptr, u64 len);

/* Boards may provide the functions below to implement RTS functionality */

void __efi_runtime EFIAPI efi_reset_system(
			enum efi_reset_type reset_type,
			efi_status_t reset_status,
			unsigned long data_size, void *reset_data);

/* Architecture specific initialization of the EFI subsystem */
efi_status_t efi_reset_system_init(void);

efi_status_t __efi_runtime EFIAPI efi_get_time(
			struct efi_time *time,
			struct efi_time_cap *capabilities);

efi_status_t __efi_runtime EFIAPI efi_set_time(struct efi_time *time);

#ifdef CONFIG_CMD_BOOTEFI_SELFTEST
/*
 * Entry point for the tests of the EFI API.
 * It is called by 'bootefi selftest'
 */
efi_status_t EFIAPI efi_selftest(efi_handle_t image_handle,
				 struct efi_system_table *systab);
#endif

efi_status_t EFIAPI efi_get_variable(u16 *variable_name,
				     const efi_guid_t *vendor, u32 *attributes,
				     efi_uintn_t *data_size, void *data);
efi_status_t EFIAPI efi_get_next_variable_name(efi_uintn_t *variable_name_size,
					       u16 *variable_name,
					       const efi_guid_t *vendor);
efi_status_t EFIAPI efi_set_variable(u16 *variable_name,
				     const efi_guid_t *vendor, u32 attributes,
				     efi_uintn_t data_size, const void *data);

/*
 * See section 3.1.3 in the v2.7 UEFI spec for more details on
 * the layout of EFI_LOAD_OPTION.  In short it is:
 *
 *    typedef struct _EFI_LOAD_OPTION {
 *        UINT32 Attributes;
 *        UINT16 FilePathListLength;
 *        // CHAR16 Description[];   <-- variable length, NULL terminated
 *        // EFI_DEVICE_PATH_PROTOCOL FilePathList[];
 *						 <-- FilePathListLength bytes
 *        // UINT8 OptionalData[];
 *    } EFI_LOAD_OPTION;
 */
struct efi_load_option {
	u32 attributes;
	u16 file_path_length;
	u16 *label;
	struct efi_device_path *file_path;
	const u8 *optional_data;
};

void efi_deserialize_load_option(struct efi_load_option *lo, u8 *data);
unsigned long efi_serialize_load_option(struct efi_load_option *lo, u8 **data);
efi_status_t efi_bootmgr_load(efi_handle_t *handle);

#else /* CONFIG_IS_ENABLED(EFI_LOADER) */

/* Without CONFIG_EFI_LOADER we don't have a runtime section, stub it out */
#define __efi_runtime_data
#define __efi_runtime
static inline efi_status_t efi_add_runtime_mmio(void *mmio_ptr, u64 len)
{
	return EFI_SUCCESS;
}

/* No loader configured, stub out EFI_ENTRY */
static inline void efi_restore_gd(void) { }
static inline void efi_set_bootdev(const char *dev, const char *devnr,
				   const char *path) { }
static inline void efi_net_set_dhcp_ack(void *pkt, int len) { }
static inline void efi_print_image_infos(void *pc) { }

#endif /* CONFIG_IS_ENABLED(EFI_LOADER) */

#endif /* _EFI_LOADER_H */
