/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  EFI application loader
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#ifndef _EFI_LOADER_H
#define _EFI_LOADER_H 1

#include <blk.h>
#include <event.h>
#include <log.h>
#include <part_efi.h>
#include <efi_api.h>
#include <image.h>
#include <pe.h>
#include <linux/list.h>
#include <linux/oid_registry.h>

struct blk_desc;
struct jmp_buf_data;

static inline int guidcmp(const void *g1, const void *g2)
{
	return memcmp(g1, g2, sizeof(efi_guid_t));
}

static inline void *guidcpy(void *dst, const void *src)
{
	return memcpy(dst, src, sizeof(efi_guid_t));
}

#if CONFIG_IS_ENABLED(EFI_LOADER)

/**
 * __efi_runtime_data - declares a non-const variable for EFI runtime section
 *
 * This macro indicates that a variable is non-const and should go into the
 * EFI runtime section, and thus still be available when the OS is running.
 *
 * Only use on variables not declared const.
 *
 * Example:
 *
 * ::
 *
 *   static __efi_runtime_data my_computed_table[256];
 */
#define __efi_runtime_data __section(".data.efi_runtime")

/**
 * __efi_runtime_rodata - declares a read-only variable for EFI runtime section
 *
 * This macro indicates that a variable is read-only (const) and should go into
 * the EFI runtime section, and thus still be available when the OS is running.
 *
 * Only use on variables also declared const.
 *
 * Example:
 *
 * ::
 *
 *   static const __efi_runtime_rodata my_const_table[] = { 1, 2, 3 };
 */
#define __efi_runtime_rodata __section(".rodata.efi_runtime")

/**
 * __efi_runtime - declares a function for EFI runtime section
 *
 * This macro indicates that a function should go into the EFI runtime section,
 * and thus still be available when the OS is running.
 *
 * Example:
 *
 * ::
 *
 *   static __efi_runtime compute_my_table(void);
 */
#define __efi_runtime __section(".text.efi_runtime")

/*
 * Call this with mmio_ptr as the _pointer_ to a pointer to an MMIO region
 * to make it available at runtime
 */
efi_status_t efi_add_runtime_mmio(void *mmio_ptr, u64 len);

/*
 * Special case handler for error/abort that just tries to dtrt to get
 * back to u-boot world
 */
void efi_restore_gd(void);

/* Called by networking code to memorize the dhcp ack package */
void efi_net_set_dhcp_ack(void *pkt, int len);
/* Print information about all loaded images */
void efi_print_image_infos(void *pc);

/* Hook at initialization */
efi_status_t efi_launch_capsules(void);

#else /* CONFIG_IS_ENABLED(EFI_LOADER) */

/* Without CONFIG_EFI_LOADER we don't have a runtime section, stub it out */
#define __efi_runtime_data
#define __efi_runtime_rodata
#define __efi_runtime
static inline efi_status_t efi_add_runtime_mmio(void *mmio_ptr, u64 len)
{
	return EFI_SUCCESS;
}

/* No loader configured, stub out EFI_ENTRY */
static inline void efi_restore_gd(void) { }
static inline void efi_net_set_dhcp_ack(void *pkt, int len) { }
static inline void efi_print_image_infos(void *pc) { }
static inline efi_status_t efi_launch_capsules(void)
{
	return EFI_SUCCESS;
}

#endif /* CONFIG_IS_ENABLED(EFI_LOADER) */

#if CONFIG_IS_ENABLED(EFI_BINARY_EXEC)
/* Call this to unset the current device name */
void efi_clear_bootdev(void);
/* Call this to set the current device name */
void efi_set_bootdev(const char *dev, const char *devnr, const char *path,
		     void *buffer, size_t buffer_size);
#else
static inline void efi_clear_bootdev(void) { }

static inline void efi_set_bootdev(const char *dev, const char *devnr,
				   const char *path, void *buffer,
				   size_t buffer_size) { }
#endif

/* Maximum number of configuration tables */
#define EFI_MAX_CONFIGURATION_TABLES 16

/* GUID used by the root node */
#define U_BOOT_GUID \
	EFI_GUID(0xe61d73b9, 0xa384, 0x4acc, \
		 0xae, 0xab, 0x82, 0xe8, 0x28, 0xf3, 0x62, 0x8b)
/* GUID used as root for blkmap devices */
#define U_BOOT_BLKMAP_DEV_GUID \
	EFI_GUID(0x4cad859d, 0xd644, 0x42ff,	\
		 0x87, 0x0b, 0xc0, 0x2e, 0xac, 0x05, 0x58, 0x63)
/* GUID used as host device on sandbox */
#define U_BOOT_HOST_DEV_GUID \
	EFI_GUID(0xbbe4e671, 0x5773, 0x4ea1, \
		 0x9a, 0xab, 0x3a, 0x7d, 0xbf, 0x40, 0xc4, 0x82)
/* GUID used as root for virtio devices */
#define U_BOOT_VIRTIO_DEV_GUID \
	EFI_GUID(0x63293792, 0xadf5, 0x9325, \
		 0xb9, 0x9f, 0x4e, 0x0e, 0x45, 0x5c, 0x1b, 0x1e)

/* GUID for the auto generated boot menu entry */
#define EFICONFIG_AUTO_GENERATED_ENTRY_GUID \
	EFI_GUID(0x8108ac4e, 0x9f11, 0x4d59, \
		 0x85, 0x0e, 0xe2, 0x1a, 0x52, 0x2c, 0x59, 0xb2)

/* Use internal device tree when starting UEFI application */
#define EFI_FDT_USE_INTERNAL NULL

/* Root node */
extern efi_handle_t efi_root;

/* Set to EFI_SUCCESS when initialized */
extern efi_status_t efi_obj_list_initialized;

/* Flag used by the selftest to avoid detaching devices in ExitBootServices() */
extern bool efi_st_keep_devices;

/* EFI system partition */
extern struct efi_system_partition {
	enum uclass_id uclass_id;
	int devnum;
	u8 part;
} efi_system_partition;

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

/* max bootmenu title size for volume selection */
#define BOOTMENU_DEVICE_NAME_MAX 16

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
/* current version of the EFI_UNICODE_COLLATION_PROTOCOL */
extern const struct efi_unicode_collation_protocol
					efi_unicode_collation_protocol2;
extern const struct efi_hii_config_routing_protocol efi_hii_config_routing;
extern const struct efi_hii_config_access_protocol efi_hii_config_access;
extern const struct efi_hii_database_protocol efi_hii_database;
extern const struct efi_hii_string_protocol efi_hii_string;

uint16_t *efi_dp_str(struct efi_device_path *dp);

/* GUID for the auto generated boot menu entry */
extern const efi_guid_t efi_guid_bootmenu_auto_generated;

/* GUID of the U-Boot root node */
extern const efi_guid_t efi_u_boot_guid;
#ifdef CONFIG_SANDBOX
/* GUID of U-Boot host device on sandbox */
extern const efi_guid_t efi_guid_host_dev;
#endif
/* GUID of the EFI_BLOCK_IO_PROTOCOL */
extern const efi_guid_t efi_block_io_guid;
extern const efi_guid_t efi_global_variable_guid;
extern const efi_guid_t efi_guid_console_control;
extern const efi_guid_t efi_guid_device_path;
/* GUID of the EFI system partition */
extern const efi_guid_t efi_system_partition_guid;
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
/* event group return to efibootmgr */
extern const efi_guid_t efi_guid_event_group_return_to_efibootmgr;
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
/* GUIDs of the Load File and Load File2 protocol */
extern const efi_guid_t efi_guid_load_file_protocol;
extern const efi_guid_t efi_guid_load_file2_protocol;
/* GUID of the Unicode collation protocol */
extern const efi_guid_t efi_guid_unicode_collation_protocol2;
extern const efi_guid_t efi_guid_hii_config_routing_protocol;
extern const efi_guid_t efi_guid_hii_config_access_protocol;
extern const efi_guid_t efi_guid_hii_database_protocol;
extern const efi_guid_t efi_guid_hii_string_protocol;
/* GUIDs for authentication */
extern const efi_guid_t efi_guid_image_security_database;
extern const efi_guid_t efi_guid_sha256;
extern const efi_guid_t efi_guid_cert_x509;
extern const efi_guid_t efi_guid_cert_x509_sha256;
extern const efi_guid_t efi_guid_cert_x509_sha384;
extern const efi_guid_t efi_guid_cert_x509_sha512;
extern const efi_guid_t efi_guid_cert_type_pkcs7;

/* GUID of RNG protocol */
extern const efi_guid_t efi_guid_rng_protocol;
/* GUID of capsule update result */
extern const efi_guid_t efi_guid_capsule_report;
/* GUID of firmware management protocol */
extern const efi_guid_t efi_guid_firmware_management_protocol;
/* GUID for the ESRT */
extern const efi_guid_t efi_esrt_guid;
/* GUID of the SMBIOS table */
extern const efi_guid_t smbios_guid;
extern const efi_guid_t smbios3_guid;
/*GUID of console */
extern const efi_guid_t efi_guid_text_input_protocol;
extern const efi_guid_t efi_guid_text_output_protocol;

extern char __efi_runtime_start[], __efi_runtime_stop[];
extern char __efi_runtime_rel_start[], __efi_runtime_rel_stop[];

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
 * @open_infos:		link to the list of open protocol info items
 */
struct efi_handler {
	struct list_head link;
	const efi_guid_t guid;
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
	/** @EFI_OBJECT_TYPE_UNDEFINED: undefined image type */
	EFI_OBJECT_TYPE_UNDEFINED = 0,
	/** @EFI_OBJECT_TYPE_U_BOOT_FIRMWARE: U-Boot firmware */
	EFI_OBJECT_TYPE_U_BOOT_FIRMWARE,
	/** @EFI_OBJECT_TYPE_LOADED_IMAGE: loaded image (not started) */
	EFI_OBJECT_TYPE_LOADED_IMAGE,
	/** @EFI_OBJECT_TYPE_STARTED_IMAGE: started image */
	EFI_OBJECT_TYPE_STARTED_IMAGE,
};

/**
 * struct efi_object - dereferenced EFI handle
 *
 * @link:	pointers to put the handle into a linked list
 * @protocols:	linked list with the protocol interfaces installed on this
 *		handle
 * @type:	image type if the handle relates to an image
 * @dev:	pointer to the DM device which is associated with this EFI handle
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
	struct udevice *dev;
};

enum efi_image_auth_status {
	EFI_IMAGE_AUTH_FAILED = 0,
	EFI_IMAGE_AUTH_PASSED,
};

/**
 * struct efi_loaded_image_obj - handle of a loaded image
 *
 * @header:		EFI object header
 * @exit_status:	exit status passed to Exit()
 * @exit_data_size:	exit data size passed to Exit()
 * @exit_data:		exit data passed to Exit()
 * @exit_jmp:		long jump buffer for returning from started image
 * @entry:		entry address of the relocated image
 * @image_type:		indicates if the image is an applicition or a driver
 * @auth_status:	indicates if the image is authenticated
 */
struct efi_loaded_image_obj {
	struct efi_object header;
	efi_status_t *exit_status;
	efi_uintn_t *exit_data_size;
	u16 **exit_data;
	struct jmp_buf_data *exit_jmp;
	EFIAPI efi_status_t (*entry)(efi_handle_t image_handle,
				     struct efi_system_table *st);
	u16 image_type;
	enum efi_image_auth_status auth_status;
};

/**
 * struct efi_event
 *
 * @link:		Link to list of all events
 * @queue_link:		Link to the list of queued events
 * @type:		Type of event, see efi_create_event
 * @notify_tpl:		Task priority level of notifications
 * @notify_function:	Function to call when the event is triggered
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
 * struct efi_register_notify_event - event registered by
 *				      RegisterProtocolNotify()
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

/* called at pre-initialization */
int efi_init_early(void);
/* Initialize efi execution environment */
efi_status_t efi_init_obj_list(void);
/* Append new boot option in BootOrder variable */
efi_status_t efi_bootmgr_append_bootorder(u16 index);
/* Get unused "Boot####" index */
efi_status_t efi_bootmgr_get_unused_bootoption(u16 *buf,
					       efi_uintn_t buf_size, u32 *index);
/* Generate the media device boot option */
efi_status_t efi_bootmgr_update_media_device_boot_option(void);
/* Delete selected boot option */
efi_status_t efi_bootmgr_delete_boot_option(u16 boot_index);
/* Invoke EFI boot manager */
efi_status_t efi_bootmgr_run(void *fdt);
/* search the boot option index in BootOrder */
bool efi_search_bootorder(u16 *bootorder, efi_uintn_t num, u32 target, u32 *index);
/* Set up console modes */
void efi_setup_console_size(void);
/* Set up load options from environment variable */
efi_status_t efi_env_set_load_options(efi_handle_t handle, const char *env_var,
				      u16 **load_options);
/* Get EFI configuration table */
void *efi_get_configuration_table(const efi_guid_t *guid);
/* Install device tree */
efi_status_t efi_install_fdt(void *fdt);
/* Execute loaded UEFI image */
efi_status_t do_bootefi_exec(efi_handle_t handle, void *load_options);
/* Run loaded UEFI image with given fdt */
efi_status_t efi_binary_run(void *image, size_t size, void *fdt);
/* Initialize variable services */
efi_status_t efi_init_variables(void);
/* Notify ExitBootServices() is called */
void efi_variables_boot_exit_notify(void);
efi_status_t efi_tcg2_notify_exit_boot_services_failed(void);
/* Measure efi application invocation */
efi_status_t efi_tcg2_measure_efi_app_invocation(struct efi_loaded_image_obj *handle);
/* Measure efi application exit */
efi_status_t efi_tcg2_measure_efi_app_exit(void);
/* Measure DTB */
efi_status_t efi_tcg2_measure_dtb(void *dtb);
/* Called by bootefi to initialize root node */
efi_status_t efi_root_node_register(void);
/* Called by bootefi to initialize runtime */
efi_status_t efi_initialize_system_table(void);
/* efi_runtime_detach() - detach unimplemented runtime functions */
void efi_runtime_detach(void);
/* efi_convert_pointer() - convert pointer to virtual address */
efi_status_t EFIAPI efi_convert_pointer(efi_uintn_t debug_disposition,
					void **address);
/* Carve out DT reserved memory ranges */
void efi_carve_out_dt_rsv(void *fdt);
/* Purge unused kaslr-seed */
void efi_try_purge_kaslr_seed(void *fdt);
/* Called by bootefi to make console interface available */
efi_status_t efi_console_register(void);
/* Called by efi_init_obj_list() to proble all block devices */
efi_status_t efi_disks_register(void);
/* Called by efi_init_obj_list() to install EFI_RNG_PROTOCOL */
efi_status_t efi_rng_register(void);
/* Called by efi_init_obj_list() to install EFI_TCG2_PROTOCOL */
efi_status_t efi_tcg2_register(void);
/* Called by efi_init_obj_list() to install RISCV_EFI_BOOT_PROTOCOL */
efi_status_t efi_riscv_register(void);
/* Called by efi_init_obj_list() to do initial measurement */
efi_status_t efi_tcg2_do_initial_measurement(void);
/* measure the pe-coff image, extend PCR and add Event Log */
efi_status_t tcg2_measure_pe_image(void *efi, u64 efi_size,
				   struct efi_loaded_image_obj *handle,
				   struct efi_loaded_image *loaded_image_info);
/* Create handles and protocols for the partitions of a block device */
int efi_disk_create_partitions(efi_handle_t parent, struct blk_desc *desc,
			       const char *uclass_idname, int diskid,
			       const char *pdevname);
/* Called by bootefi to make GOP (graphical) interface available */
efi_status_t efi_gop_register(void);
/* Called by bootefi to make the network interface available */
efi_status_t efi_net_register(void);
/* Called by bootefi to make the watchdog available */
efi_status_t efi_watchdog_register(void);
efi_status_t efi_initrd_register(void);
efi_status_t efi_initrd_deregister(void);
/* Called by bootefi to make SMBIOS tables available */
/**
 * efi_acpi_register() - write out ACPI tables
 *
 * Called by bootefi to make ACPI tables available
 *
 * Return: 0 if OK, -ENOMEM if no memory is available for the tables
 */
efi_status_t efi_acpi_register(void);
/**
 * efi_smbios_register() - write out SMBIOS tables
 *
 * Called by bootefi to make SMBIOS tables available
 *
 * Return: 0 if OK, -ENOMEM if no memory is available for the tables
 */
efi_status_t efi_smbios_register(void);

struct efi_simple_file_system_protocol *
efi_fs_from_path(struct efi_device_path *fp);

/* Called by efi_set_watchdog_timer to reset the timer */
efi_status_t efi_set_watchdog(unsigned long timeout);

/* Called from places to check whether a timer expired */
void efi_timer_check(void);
/* Check if a buffer contains a PE-COFF image */
efi_status_t efi_check_pe(void *buffer, size_t size, void **nt_header);
/* PE loader implementation */
efi_status_t efi_load_pe(struct efi_loaded_image_obj *handle,
			 void *efi, size_t efi_size,
			 struct efi_loaded_image *loaded_image_info);
/* Called once to store the pristine gd pointer */
void efi_save_gd(void);
/* Call this to relocate the runtime section to an address space */
void efi_runtime_relocate(ulong offset, struct efi_mem_desc *map);
/* Call this to get image parameters */
void efi_get_image_parameters(void **img_addr, size_t *img_size);
/* Add a new object to the object list. */
void efi_add_handle(efi_handle_t obj);
/* Create handle */
efi_status_t efi_create_handle(efi_handle_t *handle);
/* Delete handle */
efi_status_t efi_delete_handle(efi_handle_t obj);
/* Call this to validate a handle and find the EFI object for it */
struct efi_object *efi_search_obj(const efi_handle_t handle);
/* Locate device_path handle */
efi_status_t EFIAPI efi_locate_device_path(const efi_guid_t *protocol,
					   struct efi_device_path **device_path,
					   efi_handle_t *device);
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
/* Open protocol */
efi_status_t efi_protocol_open(struct efi_handler *handler,
			       void **protocol_interface, void *agent_handle,
			       void *controller_handle, uint32_t attributes);

/* Install multiple protocol interfaces */
efi_status_t EFIAPI
efi_install_multiple_protocol_interfaces(efi_handle_t *handle, ...);
efi_status_t EFIAPI
efi_uninstall_multiple_protocol_interfaces(efi_handle_t handle, ...);
/* Get handles that support a given protocol */
efi_status_t EFIAPI efi_locate_handle_buffer(
			enum efi_locate_search_type search_type,
			const efi_guid_t *protocol, void *search_key,
			efi_uintn_t *no_handles, efi_handle_t **buffer);
/* Close a previously opened protocol interface */
efi_status_t efi_close_protocol(efi_handle_t handle, const efi_guid_t *protocol,
				efi_handle_t agent_handle,
				efi_handle_t controller_handle);
/* Open a protocol interface */
efi_status_t EFIAPI efi_handle_protocol(efi_handle_t handle,
					const efi_guid_t *protocol,
					void **protocol_interface);
/* Call this to create an event */
efi_status_t efi_create_event(uint32_t type, efi_uintn_t notify_tpl,
			      void (EFIAPI *notify_function) (
					struct efi_event *event,
					void *context),
			      void *notify_context, const efi_guid_t *group,
			      struct efi_event **event);
/* Call this to set a timer */
efi_status_t efi_set_timer(struct efi_event *event, enum efi_timer_delay type,
			   uint64_t trigger_time);
/* Call this to signal an event */
void efi_signal_event(struct efi_event *event);

/* return true if the device is removable */
bool efi_disk_is_removable(efi_handle_t handle);

/**
 * efi_create_simple_file_system() - create simple file system protocol
 *
 * Create a simple file system protocol for a partition.
 *
 * @desc:	block device descriptor
 * @part:	partition number
 * @dp:		device path
 * @fsp:	simple file system protocol
 * Return:	status code
 */
efi_status_t
efi_create_simple_file_system(struct blk_desc *desc, int part,
			      struct efi_device_path *dp,
			      struct efi_simple_file_system_protocol **fsp);

/* open file from device-path: */
struct efi_file_handle *efi_file_from_path(struct efi_device_path *fp);

/* Registers a callback function for a notification event. */
efi_status_t EFIAPI efi_register_protocol_notify(const efi_guid_t *protocol,
						 struct efi_event *event,
						 void **registration);
efi_status_t efi_file_size(struct efi_file_handle *fh, efi_uintn_t *size);

/* get a device path from a Boot#### option */
struct efi_device_path *efi_get_dp_from_boot(const efi_guid_t guid);

/* get len, string (used in u-boot crypto from a guid */
const char *guid_to_sha_str(const efi_guid_t *guid);
int algo_to_len(const char *algo);

int efi_link_dev(efi_handle_t handle, struct udevice *dev);
int efi_unlink_dev(efi_handle_t handle);
bool efi_varname_is_load_option(u16 *var_name16, int *index);
efi_status_t efi_next_variable_name(efi_uintn_t *size, u16 **buf,
				    efi_guid_t *guid);

/**
 * efi_size_in_pages() - convert size in bytes to size in pages
 *
 * This macro returns the number of EFI memory pages required to hold 'size'
 * bytes.
 *
 * @size:	size in bytes
 * Return:	size in pages
 */
#define efi_size_in_pages(size) (((size) + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT)
/* Allocate boot service data pool memory */
void *efi_alloc(size_t len);
/* Allocate pages on the specified alignment */
void *efi_alloc_aligned_pages(u64 len, int memory_type, size_t align);
/* More specific EFI memory allocator, called by EFI payloads */
efi_status_t efi_allocate_pages(enum efi_allocate_type type,
				enum efi_memory_type memory_type,
				efi_uintn_t pages, uint64_t *memory);
/* EFI memory free function. */
efi_status_t efi_free_pages(uint64_t memory, efi_uintn_t pages);
/* EFI memory allocator for small allocations */
efi_status_t efi_allocate_pool(enum efi_memory_type pool_type,
			       efi_uintn_t size, void **buffer);
/* EFI pool memory free function. */
efi_status_t efi_free_pool(void *buffer);
/* Allocate and retrieve EFI memory map */
efi_status_t efi_get_memory_map_alloc(efi_uintn_t *map_size,
				      struct efi_mem_desc **memory_map);
/* Returns the EFI memory map */
efi_status_t efi_get_memory_map(efi_uintn_t *memory_map_size,
				struct efi_mem_desc *memory_map,
				efi_uintn_t *map_key,
				efi_uintn_t *descriptor_size,
				uint32_t *descriptor_version);
/* Adds a range into the EFI memory map */
efi_status_t efi_add_memory_map(u64 start, u64 size, int memory_type);
/* Adds a conventional range into the EFI memory map */
efi_status_t efi_add_conventional_memory_map(u64 ram_start, u64 ram_end,
					     u64 ram_top);

/* Called by board init to initialize the EFI drivers */
efi_status_t efi_driver_init(void);
/* Called when a block device is added */
int efi_disk_probe(void *ctx, struct event *event);
/* Called when a block device is removed */
int efi_disk_remove(void *ctx, struct event *event);
/* Called by board init to initialize the EFI memory map */
int efi_memory_init(void);
/* Adds new or overrides configuration table entry to the system table */
efi_status_t efi_install_configuration_table(const efi_guid_t *guid, void *table);
/* Sets up a loaded image */
efi_status_t efi_setup_loaded_image(struct efi_device_path *device_path,
				    struct efi_device_path *file_path,
				    struct efi_loaded_image_obj **handle_ptr,
				    struct efi_loaded_image **info_ptr);

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
extern void *efi_bounce_buffer;
#define EFI_LOADER_BOUNCE_BUFFER_SIZE (64 * 1024 * 1024)
#endif

/* shorten device path */
struct efi_device_path *efi_dp_shorten(struct efi_device_path *dp);
struct efi_device_path *efi_dp_next(const struct efi_device_path *dp);
int efi_dp_match(const struct efi_device_path *a,
		 const struct efi_device_path *b);
efi_handle_t efi_dp_find_obj(struct efi_device_path *dp,
			     const efi_guid_t *guid,
			     struct efi_device_path **rem);
/* get size of the first device path instance excluding end node */
efi_uintn_t efi_dp_instance_size(const struct efi_device_path *dp);
/* size of multi-instance device path excluding end node */
efi_uintn_t efi_dp_size(const struct efi_device_path *dp);
struct efi_device_path *efi_dp_dup(const struct efi_device_path *dp);
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

struct efi_device_path *efi_dp_from_part(struct blk_desc *desc, int part);
/* Create a device node for a block device partition. */
struct efi_device_path *efi_dp_part_node(struct blk_desc *desc, int part);
struct efi_device_path *efi_dp_from_file(const struct efi_device_path *dp,
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
struct efi_device_path *efi_dp_from_uart(void);
efi_status_t efi_dp_from_name(const char *dev, const char *devnr,
			      const char *path,
			      struct efi_device_path **device,
			      struct efi_device_path **file);
ssize_t efi_dp_check_length(const struct efi_device_path *dp,
			    const size_t maxlen);

#define EFI_DP_TYPE(_dp, _type, _subtype) \
	(((_dp)->type == DEVICE_PATH_TYPE_##_type) && \
	 ((_dp)->sub_type == DEVICE_PATH_SUB_TYPE_##_subtype))

/* template END node: */
extern const struct efi_device_path END;

/* Indicate supported runtime services */
efi_status_t efi_init_runtime_supported(void);

/* Update CRC32 in table header */
void __efi_runtime efi_update_table_header_crc32(struct efi_table_hdr *table);

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

/*
 * Entry point for the tests of the EFI API.
 * It is called by 'bootefi selftest'
 */
efi_status_t EFIAPI efi_selftest(efi_handle_t image_handle,
				 struct efi_system_table *systab);

efi_status_t EFIAPI efi_get_variable(u16 *variable_name,
				     const efi_guid_t *vendor, u32 *attributes,
				     efi_uintn_t *data_size, void *data);
efi_status_t EFIAPI efi_get_next_variable_name(efi_uintn_t *variable_name_size,
					       u16 *variable_name,
					       efi_guid_t *vendor);
efi_status_t EFIAPI efi_set_variable(u16 *variable_name,
				     const efi_guid_t *vendor, u32 attributes,
				     efi_uintn_t data_size, const void *data);

efi_status_t EFIAPI efi_query_variable_info(
			u32 attributes, u64 *maximum_variable_storage_size,
			u64 *remaining_variable_storage_size,
			u64 *maximum_variable_size);

void *efi_get_var(const u16 *name, const efi_guid_t *vendor, efi_uintn_t *size);

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

struct efi_device_path *efi_dp_from_lo(struct efi_load_option *lo,
				       const efi_guid_t *guid);
struct efi_device_path *efi_dp_concat(const struct efi_device_path *dp1,
				      const struct efi_device_path *dp2,
				      bool split_end_node);
struct efi_device_path *search_gpt_dp_node(struct efi_device_path *device_path);
efi_status_t efi_deserialize_load_option(struct efi_load_option *lo, u8 *data,
					 efi_uintn_t *size);
unsigned long efi_serialize_load_option(struct efi_load_option *lo, u8 **data);
efi_status_t efi_set_load_options(efi_handle_t handle,
				  efi_uintn_t load_options_size,
				  void *load_options);
efi_status_t efi_bootmgr_load(efi_handle_t *handle, void **load_options);

/**
 * struct efi_image_regions - A list of memory regions
 *
 * @max:	Maximum number of regions
 * @num:	Number of regions
 * @reg:	array of regions
 */
struct efi_image_regions {
	int			max;
	int			num;
	struct image_region	reg[];
};

/**
 * struct efi_sig_data - A decoded data of struct efi_signature_data
 *
 * This structure represents an internal form of signature in
 * signature database. A listed list may represent a signature list.
 *
 * @next:	Pointer to next entry
 * @owner:	Signature owner
 * @data:	Pointer to signature data
 * @size:	Size of signature data
 */
struct efi_sig_data {
	struct efi_sig_data *next;
	efi_guid_t owner;
	void *data;
	size_t size;
};

/**
 * struct efi_signature_store - A decoded data of signature database
 *
 * This structure represents an internal form of signature database.
 *
 * @next:		Pointer to next entry
 * @sig_type:		Signature type
 * @sig_data_list:	Pointer to signature list
 */
struct efi_signature_store {
	struct efi_signature_store *next;
	efi_guid_t sig_type;
	struct efi_sig_data *sig_data_list;
};

struct x509_certificate;
struct pkcs7_message;

/**
 * struct eficonfig_media_boot_option - boot option for (removable) media device
 *
 * This structure is used to enumerate possible boot option
 *
 * @lo:		Serialized load option data
 * @size:	Size of serialized load option data
 * @exist:	Flag to indicate the load option already exists
 *		in Non-volatile load option
 */
struct eficonfig_media_boot_option {
	void *lo;
	efi_uintn_t size;
	bool exist;
};

bool efi_hash_regions(struct image_region *regs, int count,
		      void **hash, const char *hash_algo, int *len);
bool efi_signature_lookup_digest(struct efi_image_regions *regs,
				 struct efi_signature_store *db,
				 bool dbx);
bool efi_signature_verify(struct efi_image_regions *regs,
			  struct pkcs7_message *msg,
			  struct efi_signature_store *db,
			  struct efi_signature_store *dbx);
static inline bool efi_signature_verify_one(struct efi_image_regions *regs,
					    struct pkcs7_message *msg,
					    struct efi_signature_store *db)
{
	return efi_signature_verify(regs, msg, db, NULL);
}
bool efi_signature_check_signers(struct pkcs7_message *msg,
				 struct efi_signature_store *dbx);

efi_status_t efi_image_region_add(struct efi_image_regions *regs,
				  const void *start, const void *end,
				  int nocheck);

void efi_sigstore_free(struct efi_signature_store *sigstore);
struct efi_signature_store *efi_build_signature_store(void *sig_list,
						      efi_uintn_t size);
struct efi_signature_store *efi_sigstore_parse_sigdb(u16 *name);

bool efi_secure_boot_enabled(void);

bool efi_capsule_auth_enabled(void);

void *efi_prepare_aligned_image(void *efi, u64 *efi_size);

bool efi_image_parse(void *efi, size_t len, struct efi_image_regions **regp,
		     WIN_CERTIFICATE **auth, size_t *auth_len);

struct pkcs7_message *efi_parse_pkcs7_header(const void *buf,
					     size_t buflen,
					     u8 **tmpbuf);

/* runtime implementation of memcpy() */
void efi_memcpy_runtime(void *dest, const void *src, size_t n);

/* commonly used helper functions */
u16 *efi_create_indexed_name(u16 *buffer, size_t buffer_size, const char *name,
			     unsigned int index);
efi_string_t efi_convert_string(const char *str);

extern const struct efi_firmware_management_protocol efi_fmp_fit;
extern const struct efi_firmware_management_protocol efi_fmp_raw;

/* Capsule update */
efi_status_t EFIAPI efi_update_capsule(
		struct efi_capsule_header **capsule_header_array,
		efi_uintn_t capsule_count,
		u64 scatter_gather_list);
efi_status_t EFIAPI efi_query_capsule_caps(
		struct efi_capsule_header **capsule_header_array,
		efi_uintn_t capsule_count,
		u64 *maximum_capsule_size,
		u32 *reset_type);

efi_status_t efi_capsule_authenticate(const void *capsule,
				      efi_uintn_t capsule_size,
				      void **image, efi_uintn_t *image_size);

#define EFI_CAPSULE_DIR u"\\EFI\\UpdateCapsule\\"

/**
 * struct efi_fw_image -  Information on firmware images updatable through
 *                        capsule update
 *
 * This structure gives information about the firmware images on the platform
 * which can be updated through the capsule update mechanism
 *
 * @image_type_id:	Image GUID. Same value is to be used in the capsule
 * @fw_name:		Name of the firmware image
 * @image_index:	Image Index, same as value passed to SetImage FMP
 *                      function
 */
struct efi_fw_image {
	efi_guid_t image_type_id;
	u16 *fw_name;
	u8 image_index;
};

/**
 * struct efi_capsule_update_info - Information needed for capsule updates
 *
 * This structure provides information needed for performing firmware
 * updates. The structure needs to be initialised per platform, for all
 * platforms which enable capsule updates
 *
 * @dfu_string:		String used to populate dfu_alt_info
 * @num_images:		The number of images array entries
 * @images:		Pointer to an array of updatable images
 */
struct efi_capsule_update_info {
	const char *dfu_string;
	int num_images;
	struct efi_fw_image *images;
};

extern struct efi_capsule_update_info update_info;

/**
 * Install the ESRT system table.
 *
 * Return:	status code
 */
efi_status_t efi_esrt_register(void);

/**
 * efi_ecpt_register() - Install the ECPT system table.
 *
 * Return: status code
 */
efi_status_t efi_ecpt_register(void);

/**
 * efi_esrt_populate() - Populates the ESRT entries from the FMP instances
 * present in the system.
 * If an ESRT already exists, the old ESRT is replaced in the system table.
 * The memory of the old ESRT is deallocated.
 *
 * Return:
 * - EFI_SUCCESS if the ESRT is correctly created
 * - error code otherwise.
 */
efi_status_t efi_esrt_populate(void);
efi_status_t efi_load_capsule_drivers(void);

efi_status_t platform_get_eventlog(struct udevice *dev, u64 *addr, u32 *sz);

efi_status_t efi_locate_handle_buffer_int(enum efi_locate_search_type search_type,
					  const efi_guid_t *protocol, void *search_key,
					  efi_uintn_t *no_handles, efi_handle_t **buffer);

efi_status_t efi_open_volume_int(struct efi_simple_file_system_protocol *this,
				 struct efi_file_handle **root);
efi_status_t efi_file_open_int(struct efi_file_handle *this,
			       struct efi_file_handle **new_handle,
			       u16 *file_name, u64 open_mode,
			       u64 attributes);
efi_status_t efi_file_close_int(struct efi_file_handle *file);
efi_status_t efi_file_read_int(struct efi_file_handle *this,
			       efi_uintn_t *buffer_size, void *buffer);
efi_status_t efi_file_setpos_int(struct efi_file_handle *file, u64 pos);

typedef efi_status_t (*efi_console_filter_func)(struct efi_input_key *key);
efi_status_t efi_console_get_u16_string
		(struct efi_simple_text_input_protocol *cin,
		 u16 *buf, efi_uintn_t count, efi_console_filter_func filer_func,
		 int row, int col);

efi_status_t efi_disk_get_device_name(const efi_handle_t handle, char *buf, int size);

/**
 * efi_add_known_memory() - add memory banks to EFI memory map
 *
 * This weak function may be overridden for specific architectures.
 */
void efi_add_known_memory(void);

#endif /* _EFI_LOADER_H */
