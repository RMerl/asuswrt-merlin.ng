// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application runtime services
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#include <command.h>
#include <cpu_func.h>
#include <dm.h>
#include <elf.h>
#include <efi_loader.h>
#include <log.h>
#include <malloc.h>
#include <rtc.h>
#include <asm/global_data.h>
#include <u-boot/crc.h>

/* For manual relocation support */
DECLARE_GLOBAL_DATA_PTR;

/* GUID of the runtime properties table */
static const efi_guid_t efi_rt_properties_table_guid =
				EFI_RT_PROPERTIES_TABLE_GUID;

struct efi_runtime_mmio_list {
	struct list_head link;
	void **ptr;
	u64 paddr;
	u64 len;
};

/* This list contains all runtime available mmio regions */
static LIST_HEAD(efi_runtime_mmio);

static efi_status_t __efi_runtime EFIAPI efi_unimplemented(void);

/*
 * TODO(sjg@chromium.org): These defines and structures should come from the ELF
 * header for each architecture (or a generic header) rather than being repeated
 * here.
 */
#if defined(__aarch64__)
#define R_RELATIVE	R_AARCH64_RELATIVE
#define R_MASK		0xffffffffULL
#define IS_RELA		1
#elif defined(__arm__)
#define R_RELATIVE	R_ARM_RELATIVE
#define R_MASK		0xffULL
#elif defined(__i386__)
#define R_RELATIVE	R_386_RELATIVE
#define R_MASK		0xffULL
#elif defined(__x86_64__)
#define R_RELATIVE	R_X86_64_RELATIVE
#define R_MASK		0xffffffffULL
#define IS_RELA		1
#elif defined(__riscv)
#define R_RELATIVE	R_RISCV_RELATIVE
#define R_MASK		0xffULL
#define IS_RELA		1

struct dyn_sym {
	ulong foo1;
	ulong addr;
	u32 foo2;
	u32 foo3;
};
#if (__riscv_xlen == 32)
#define R_ABSOLUTE	R_RISCV_32
#define SYM_INDEX	8
#elif (__riscv_xlen == 64)
#define R_ABSOLUTE	R_RISCV_64
#define SYM_INDEX	32
#else
#error unknown riscv target
#endif
#else
#error Need to add relocation awareness
#endif

struct elf_rel {
	ulong *offset;
	ulong info;
};

struct elf_rela {
	ulong *offset;
	ulong info;
	long addend;
};

static __efi_runtime_data struct efi_mem_desc *efi_virtmap;
static __efi_runtime_data efi_uintn_t efi_descriptor_count;
static __efi_runtime_data efi_uintn_t efi_descriptor_size;

/*
 * EFI runtime code lives in two stages. In the first stage, U-Boot and an EFI
 * payload are running concurrently at the same time. In this mode, we can
 * handle a good number of runtime callbacks
 */

/**
 * efi_init_runtime_supported() - create runtime properties table
 *
 * Create a configuration table specifying which services are available at
 * runtime.
 *
 * Return:	status code
 */
efi_status_t efi_init_runtime_supported(void)
{
	efi_status_t ret;
	struct efi_rt_properties_table *rt_table;

	ret = efi_allocate_pool(EFI_RUNTIME_SERVICES_DATA,
				sizeof(struct efi_rt_properties_table),
				(void **)&rt_table);
	if (ret != EFI_SUCCESS)
		return ret;

	rt_table->version = EFI_RT_PROPERTIES_TABLE_VERSION;
	rt_table->length = sizeof(struct efi_rt_properties_table);
	rt_table->runtime_services_supported =
				EFI_RT_SUPPORTED_GET_VARIABLE |
				EFI_RT_SUPPORTED_GET_NEXT_VARIABLE_NAME |
				EFI_RT_SUPPORTED_SET_VIRTUAL_ADDRESS_MAP |
				EFI_RT_SUPPORTED_CONVERT_POINTER;

	/*
	 * This value must be synced with efi_runtime_detach_list
	 * as well as efi_runtime_services.
	 */
#ifdef CONFIG_EFI_HAVE_RUNTIME_RESET
	rt_table->runtime_services_supported |= EFI_RT_SUPPORTED_RESET_SYSTEM;
#endif

	ret = efi_install_configuration_table(&efi_rt_properties_table_guid,
					      rt_table);
	return ret;
}

/**
 * efi_memcpy_runtime() - copy memory area
 *
 * At runtime memcpy() is not available.
 *
 * Overlapping memory areas can be copied safely if src >= dest.
 *
 * @dest:	destination buffer
 * @src:	source buffer
 * @n:		number of bytes to copy
 * Return:	pointer to destination buffer
 */
void __efi_runtime efi_memcpy_runtime(void *dest, const void *src, size_t n)
{
	u8 *d = dest;
	const u8 *s = src;

	for (; n; --n)
		*d++ = *s++;
}

/**
 * efi_update_table_header_crc32() - Update crc32 in table header
 *
 * @table:	EFI table
 */
void __efi_runtime efi_update_table_header_crc32(struct efi_table_hdr *table)
{
	table->crc32 = 0;
	table->crc32 = crc32(0, (const unsigned char *)table,
			     table->headersize);
}

/**
 * efi_reset_system_boottime() - reset system at boot time
 *
 * This function implements the ResetSystem() runtime service before
 * SetVirtualAddressMap() is called.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @reset_type:		type of reset to perform
 * @reset_status:	status code for the reset
 * @data_size:		size of reset_data
 * @reset_data:		information about the reset
 */
static void EFIAPI efi_reset_system_boottime(
			enum efi_reset_type reset_type,
			efi_status_t reset_status,
			unsigned long data_size, void *reset_data)
{
	struct efi_event *evt;

	EFI_ENTRY("%d %lx %lx %p", reset_type, reset_status, data_size,
		  reset_data);

	/* Notify reset */
	list_for_each_entry(evt, &efi_events, link) {
		if (evt->group &&
		    !guidcmp(evt->group,
			     &efi_guid_event_group_reset_system)) {
			efi_signal_event(evt);
			break;
		}
	}
	switch (reset_type) {
	case EFI_RESET_COLD:
	case EFI_RESET_WARM:
	case EFI_RESET_PLATFORM_SPECIFIC:
		do_reset(NULL, 0, 0, NULL);
		break;
	case EFI_RESET_SHUTDOWN:
#ifdef CONFIG_CMD_POWEROFF
		do_poweroff(NULL, 0, 0, NULL);
#endif
		break;
	}

	while (1) { }
}

/**
 * efi_get_time_boottime() - get current time at boot time
 *
 * This function implements the GetTime runtime service before
 * SetVirtualAddressMap() is called.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @time:		pointer to structure to receive current time
 * @capabilities:	pointer to structure to receive RTC properties
 * Returns:		status code
 */
static efi_status_t EFIAPI efi_get_time_boottime(
			struct efi_time *time,
			struct efi_time_cap *capabilities)
{
#ifdef CONFIG_EFI_GET_TIME
	efi_status_t ret = EFI_SUCCESS;
	struct rtc_time tm;
	struct udevice *dev;

	EFI_ENTRY("%p %p", time, capabilities);

	if (!time) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	if (uclass_get_device(UCLASS_RTC, 0, &dev) ||
	    dm_rtc_get(dev, &tm)) {
		ret = EFI_UNSUPPORTED;
		goto out;
	}
	if (dm_rtc_get(dev, &tm)) {
		ret = EFI_DEVICE_ERROR;
		goto out;
	}

	memset(time, 0, sizeof(*time));
	time->year = tm.tm_year;
	time->month = tm.tm_mon;
	time->day = tm.tm_mday;
	time->hour = tm.tm_hour;
	time->minute = tm.tm_min;
	time->second = tm.tm_sec;
	if (tm.tm_isdst > 0)
		time->daylight =
			EFI_TIME_ADJUST_DAYLIGHT | EFI_TIME_IN_DAYLIGHT;
	else if (!tm.tm_isdst)
		time->daylight = EFI_TIME_ADJUST_DAYLIGHT;
	else
		time->daylight = 0;
	time->timezone = EFI_UNSPECIFIED_TIMEZONE;

	if (capabilities) {
		/* Set reasonable dummy values */
		capabilities->resolution = 1;		/* 1 Hz */
		capabilities->accuracy = 100000000;	/* 100 ppm */
		capabilities->sets_to_zero = false;
	}
out:
	return EFI_EXIT(ret);
#else
	EFI_ENTRY("%p %p", time, capabilities);
	return EFI_EXIT(EFI_UNSUPPORTED);
#endif
}

#ifdef CONFIG_EFI_SET_TIME

/**
 * efi_validate_time() - checks if timestamp is valid
 *
 * @time:	timestamp to validate
 * Returns:	0 if timestamp is valid, 1 otherwise
 */
static int efi_validate_time(struct efi_time *time)
{
	return (!time ||
		time->year < 1900 || time->year > 9999 ||
		!time->month || time->month > 12 || !time->day ||
		time->day > rtc_month_days(time->month - 1, time->year) ||
		time->hour > 23 || time->minute > 59 || time->second > 59 ||
		time->nanosecond > 999999999 ||
		time->daylight &
		~(EFI_TIME_IN_DAYLIGHT | EFI_TIME_ADJUST_DAYLIGHT) ||
		((time->timezone < -1440 || time->timezone > 1440) &&
		time->timezone != EFI_UNSPECIFIED_TIMEZONE));
}

#endif

/**
 * efi_set_time_boottime() - set current time
 *
 * This function implements the SetTime() runtime service before
 * SetVirtualAddressMap() is called.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @time:		pointer to structure to with current time
 * Returns:		status code
 */
static efi_status_t EFIAPI efi_set_time_boottime(struct efi_time *time)
{
#ifdef CONFIG_EFI_SET_TIME
	efi_status_t ret = EFI_SUCCESS;
	struct rtc_time tm;
	struct udevice *dev;

	EFI_ENTRY("%p", time);

	if (efi_validate_time(time)) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (uclass_get_device(UCLASS_RTC, 0, &dev)) {
		ret = EFI_UNSUPPORTED;
		goto out;
	}

	memset(&tm, 0, sizeof(tm));
	tm.tm_year = time->year;
	tm.tm_mon = time->month;
	tm.tm_mday = time->day;
	tm.tm_hour = time->hour;
	tm.tm_min = time->minute;
	tm.tm_sec = time->second;
	switch (time->daylight) {
	case EFI_TIME_ADJUST_DAYLIGHT:
		tm.tm_isdst = 0;
		break;
	case EFI_TIME_ADJUST_DAYLIGHT | EFI_TIME_IN_DAYLIGHT:
		tm.tm_isdst = 1;
		break;
	default:
		tm.tm_isdst = -1;
		break;
	}
	/* Calculate day of week */
	rtc_calc_weekday(&tm);

	if (dm_rtc_set(dev, &tm))
		ret = EFI_DEVICE_ERROR;
out:
	return EFI_EXIT(ret);
#else
	EFI_ENTRY("%p", time);
	return EFI_EXIT(EFI_UNSUPPORTED);
#endif
}
/**
 * efi_reset_system() - reset system
 *
 * This function implements the ResetSystem() runtime service after
 * SetVirtualAddressMap() is called. As this placeholder cannot reset the
 * system it simply return to the caller.
 *
 * Boards may override the helpers below to implement reset functionality.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @reset_type:		type of reset to perform
 * @reset_status:	status code for the reset
 * @data_size:		size of reset_data
 * @reset_data:		information about the reset
 */
void __weak __efi_runtime EFIAPI efi_reset_system(
			enum efi_reset_type reset_type,
			efi_status_t reset_status,
			unsigned long data_size, void *reset_data)
{
	return;
}

/**
 * efi_reset_system_init() - initialize the reset driver
 *
 * Boards may override this function to initialize the reset driver.
 */
efi_status_t __weak efi_reset_system_init(void)
{
	return EFI_SUCCESS;
}

/**
 * efi_get_time() - get current time
 *
 * This function implements the GetTime runtime service after
 * SetVirtualAddressMap() is called. As the U-Boot driver are not available
 * anymore only an error code is returned.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @time:		pointer to structure to receive current time
 * @capabilities:	pointer to structure to receive RTC properties
 * Returns:		status code
 */
efi_status_t __weak __efi_runtime EFIAPI efi_get_time(
			struct efi_time *time,
			struct efi_time_cap *capabilities)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_set_time() - set current time
 *
 * This function implements the SetTime runtime service after
 * SetVirtualAddressMap() is called. As the U-Boot driver are not available
 * anymore only an error code is returned.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @time:		pointer to structure to with current time
 * Returns:		status code
 */
efi_status_t __weak __efi_runtime EFIAPI efi_set_time(struct efi_time *time)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_update_capsule_unsupported() - process information from operating system
 *
 * This function implements the UpdateCapsule() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @capsule_header_array:	pointer to array of virtual pointers
 * @capsule_count:		number of pointers in capsule_header_array
 * @scatter_gather_list:	pointer to array of physical pointers
 * Returns:			status code
 */
static efi_status_t __efi_runtime EFIAPI efi_update_capsule_unsupported(
			struct efi_capsule_header **capsule_header_array,
			efi_uintn_t capsule_count,
			u64 scatter_gather_list)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_query_capsule_caps_unsupported() - check if capsule is supported
 *
 * This function implements the QueryCapsuleCapabilities() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @capsule_header_array:	pointer to array of virtual pointers
 * @capsule_count:		number of pointers in capsule_header_array
 * @maximum_capsule_size:	maximum capsule size
 * @reset_type:			type of reset needed for capsule update
 * Returns:			status code
 */
static efi_status_t __efi_runtime EFIAPI efi_query_capsule_caps_unsupported(
			struct efi_capsule_header **capsule_header_array,
			efi_uintn_t capsule_count,
			u64 *maximum_capsule_size,
			u32 *reset_type)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_is_runtime_service_pointer() - check if pointer points to runtime table
 *
 * @p:		pointer to check
 * Return:	true if the pointer points to a service function pointer in the
 *		runtime table
 */
static bool efi_is_runtime_service_pointer(void *p)
{
	return (p >= (void *)&efi_runtime_services.get_time &&
		p <= (void *)&efi_runtime_services.query_variable_info) ||
	       p == (void *)&efi_events.prev ||
	       p == (void *)&efi_events.next;
}

/**
 * efi_runtime_detach() - detach unimplemented runtime functions
 */
void efi_runtime_detach(void)
{
	efi_runtime_services.reset_system = efi_reset_system;
	efi_runtime_services.get_time = efi_get_time;
	efi_runtime_services.set_time = efi_set_time;
	if (IS_ENABLED(CONFIG_EFI_RUNTIME_UPDATE_CAPSULE)) {
		/* won't support at runtime */
		efi_runtime_services.update_capsule =
				efi_update_capsule_unsupported;
		efi_runtime_services.query_capsule_caps =
				efi_query_capsule_caps_unsupported;
	}

	/* Update CRC32 */
	efi_update_table_header_crc32(&efi_runtime_services.hdr);
}

/**
 * efi_set_virtual_address_map_runtime() - change from physical to virtual
 *					   mapping
 *
 * This function implements the SetVirtualAddressMap() runtime service after
 * it is first called.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @memory_map_size:	size of the virtual map
 * @descriptor_size:	size of an entry in the map
 * @descriptor_version:	version of the map entries
 * @virtmap:		virtual address mapping information
 * Return:		status code EFI_UNSUPPORTED
 */
static __efi_runtime efi_status_t EFIAPI efi_set_virtual_address_map_runtime(
			efi_uintn_t memory_map_size,
			efi_uintn_t descriptor_size,
			uint32_t descriptor_version,
			struct efi_mem_desc *virtmap)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_convert_pointer_runtime() - convert from physical to virtual pointer
 *
 * This function implements the ConvertPointer() runtime service after
 * the first call to SetVirtualAddressMap().
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @debug_disposition:	indicates if pointer may be converted to NULL
 * @address:		pointer to be converted
 * Return:		status code EFI_UNSUPPORTED
 */
static __efi_runtime efi_status_t EFIAPI efi_convert_pointer_runtime(
			efi_uintn_t debug_disposition, void **address)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_convert_pointer() - convert from physical to virtual pointer
 *
 * This function implements the ConvertPointer() runtime service until
 * the first call to SetVirtualAddressMap().
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @debug_disposition:	indicates if pointer may be converted to NULL
 * @address:		pointer to be converted
 * Return:		status code
 */
__efi_runtime efi_status_t EFIAPI
efi_convert_pointer(efi_uintn_t debug_disposition, void **address)
{
	efi_physical_addr_t addr;
	efi_uintn_t i;
	efi_status_t ret = EFI_NOT_FOUND;

	if (!efi_virtmap) {
		ret = EFI_UNSUPPORTED;
		goto out;
	}

	if (!address) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	if (!*address) {
		if (debug_disposition & EFI_OPTIONAL_PTR)
			return EFI_SUCCESS;
		else
			return EFI_INVALID_PARAMETER;
	}

	addr = (uintptr_t)*address;
	for (i = 0; i < efi_descriptor_count; i++) {
		struct efi_mem_desc *map = (void *)efi_virtmap +
					   (efi_descriptor_size * i);

		if (addr >= map->physical_start &&
		    (addr < map->physical_start
			    + (map->num_pages << EFI_PAGE_SHIFT))) {
			*address = (void *)(uintptr_t)
				   (addr + map->virtual_start -
				    map->physical_start);

			ret = EFI_SUCCESS;
			break;
		}
	}

out:
	return ret;
}

static __efi_runtime void efi_relocate_runtime_table(ulong offset)
{
	ulong patchoff;
	void **pos;

	/* Relocate the runtime services pointers */
	patchoff = offset - gd->relocaddr;
	for (pos = (void **)&efi_runtime_services.get_time;
	     pos <= (void **)&efi_runtime_services.query_variable_info; ++pos) {
		if (*pos)
			*pos += patchoff;
	}

	/*
	 * The entry for SetVirtualAddress() must point to a physical address.
	 * After the first execution the service must return EFI_UNSUPPORTED.
	 */
	efi_runtime_services.set_virtual_address_map =
			&efi_set_virtual_address_map_runtime;

	/*
	 * The entry for ConvertPointer() must point to a physical address.
	 * The service is not usable after SetVirtualAddress().
	 */
	efi_runtime_services.convert_pointer = &efi_convert_pointer_runtime;

	/*
	 * TODO: Update UEFI variable RuntimeServicesSupported removing flags
	 * EFI_RT_SUPPORTED_SET_VIRTUAL_ADDRESS_MAP and
	 * EFI_RT_SUPPORTED_CONVERT_POINTER as required by the UEFI spec 2.8.
	 */

	/* Update CRC32 */
	efi_update_table_header_crc32(&efi_runtime_services.hdr);
}

/* Relocate EFI runtime to uboot_reloc_base = offset */
void efi_runtime_relocate(ulong offset, struct efi_mem_desc *map)
{
#ifdef IS_RELA
	struct elf_rela *rel = (void*)&__efi_runtime_rel_start;
#else
	struct elf_rel *rel = (void*)&__efi_runtime_rel_start;
	static ulong lastoff = CONFIG_TEXT_BASE;
#endif

	debug("%s: Relocating to offset=%lx\n", __func__, offset);
	for (; (ulong)rel < (ulong)&__efi_runtime_rel_stop; rel++) {
		ulong base = CONFIG_TEXT_BASE;
		ulong *p;
		ulong newaddr;

		p = (void*)((ulong)rel->offset - base) + gd->relocaddr;

		/*
		 * The runtime services table is updated in
		 * efi_relocate_runtime_table()
		 */
		if (map && efi_is_runtime_service_pointer(p))
			continue;

		debug("%s: rel->info=%#lx *p=%#lx rel->offset=%p\n", __func__,
		      rel->info, *p, rel->offset);

		switch (rel->info & R_MASK) {
		case R_RELATIVE:
#ifdef IS_RELA
		newaddr = rel->addend + offset - CONFIG_TEXT_BASE;
#else
		newaddr = *p - lastoff + offset;
#endif
			break;
#ifdef R_ABSOLUTE
		case R_ABSOLUTE: {
			ulong symidx = rel->info >> SYM_INDEX;
			extern struct dyn_sym __dyn_sym_start[];
			newaddr = __dyn_sym_start[symidx].addr + offset;
#ifdef IS_RELA
			newaddr -= CONFIG_TEXT_BASE;
#endif
			break;
		}
#endif
		default:
			printf("%s: Unknown relocation type %llx\n",
			       __func__, rel->info & R_MASK);
			continue;
		}

		/* Check if the relocation is inside bounds */
		if (map && ((newaddr < map->virtual_start) ||
		    newaddr > (map->virtual_start +
			      (map->num_pages << EFI_PAGE_SHIFT)))) {
			printf("%s: Relocation at %p is out of range (%lx)\n",
			       __func__, p, newaddr);
			continue;
		}

		debug("%s: Setting %p to %lx\n", __func__, p, newaddr);
		*p = newaddr;
		flush_dcache_range((ulong)p & ~(EFI_CACHELINE_SIZE - 1),
			ALIGN((ulong)&p[1], EFI_CACHELINE_SIZE));
	}

#ifndef IS_RELA
	lastoff = offset;
#endif

        invalidate_icache_all();
}

/**
 * efi_set_virtual_address_map() - change from physical to virtual mapping
 *
 * This function implements the SetVirtualAddressMap() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @memory_map_size:	size of the virtual map
 * @descriptor_size:	size of an entry in the map
 * @descriptor_version:	version of the map entries
 * @virtmap:		virtual address mapping information
 * Return:		status code
 */
static efi_status_t EFIAPI efi_set_virtual_address_map(
			efi_uintn_t memory_map_size,
			efi_uintn_t descriptor_size,
			uint32_t descriptor_version,
			struct efi_mem_desc *virtmap)
{
	efi_uintn_t n = memory_map_size / descriptor_size;
	efi_uintn_t i;
	efi_status_t ret = EFI_INVALID_PARAMETER;
	int rt_code_sections = 0;
	struct efi_event *event;

	EFI_ENTRY("%zx %zx %x %p", memory_map_size, descriptor_size,
		  descriptor_version, virtmap);

	if (descriptor_version != EFI_MEMORY_DESCRIPTOR_VERSION ||
	    descriptor_size < sizeof(struct efi_mem_desc))
		goto out;

	efi_virtmap = virtmap;
	efi_descriptor_size = descriptor_size;
	efi_descriptor_count = n;

	/*
	 * TODO:
	 * Further down we are cheating. While really we should implement
	 * SetVirtualAddressMap() events and ConvertPointer() to allow
	 * dynamically loaded drivers to expose runtime services, we don't
	 * today.
	 *
	 * So let's ensure we see exactly one single runtime section, as
	 * that is the built-in one. If we see more (or less), someone must
	 * have tried adding or removing to that which we don't support yet.
	 * In that case, let's better fail rather than expose broken runtime
	 * services.
	 */
	for (i = 0; i < n; i++) {
		struct efi_mem_desc *map = (void*)virtmap +
					   (descriptor_size * i);

		if (map->type == EFI_RUNTIME_SERVICES_CODE)
			rt_code_sections++;
	}

	if (rt_code_sections != 1) {
		/*
		 * We expose exactly one single runtime code section, so
		 * something is definitely going wrong.
		 */
		goto out;
	}

	/* Notify EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE */
	list_for_each_entry(event, &efi_events, link) {
		if (event->notify_function)
			EFI_CALL_VOID(event->notify_function(
					event, event->notify_context));
	}

	/* Rebind mmio pointers */
	for (i = 0; i < n; i++) {
		struct efi_mem_desc *map = (void*)virtmap +
					   (descriptor_size * i);
		struct list_head *lhandle;
		efi_physical_addr_t map_start = map->physical_start;
		efi_physical_addr_t map_len = map->num_pages << EFI_PAGE_SHIFT;
		efi_physical_addr_t map_end = map_start + map_len;
		u64 off = map->virtual_start - map_start;

		/* Adjust all mmio pointers in this region */
		list_for_each(lhandle, &efi_runtime_mmio) {
			struct efi_runtime_mmio_list *lmmio;

			lmmio = list_entry(lhandle,
					   struct efi_runtime_mmio_list,
					   link);
			if ((map_start <= lmmio->paddr) &&
			    (map_end >= lmmio->paddr)) {
				uintptr_t new_addr = lmmio->paddr + off;
				*lmmio->ptr = (void *)new_addr;
			}
		}
		if ((map_start <= (uintptr_t)systab.tables) &&
		    (map_end >= (uintptr_t)systab.tables)) {
			char *ptr = (char *)systab.tables;

			ptr += off;
			systab.tables = (struct efi_configuration_table *)ptr;
		}
	}

	/* Relocate the runtime. See TODO above */
	for (i = 0; i < n; i++) {
		struct efi_mem_desc *map;

		map = (void*)virtmap + (descriptor_size * i);
		if (map->type == EFI_RUNTIME_SERVICES_CODE) {
			ulong new_offset = map->virtual_start -
					   map->physical_start + gd->relocaddr;

			efi_relocate_runtime_table(new_offset);
			efi_runtime_relocate(new_offset, map);
			ret = EFI_SUCCESS;
			goto out;
		}
	}

out:
	return EFI_EXIT(ret);
}

/**
 * efi_add_runtime_mmio() - add memory-mapped IO region
 *
 * This function adds a memory-mapped IO region to the memory map to make it
 * available at runtime.
 *
 * @mmio_ptr:		pointer to a pointer to the start of the memory-mapped
 *			IO region
 * @len:		size of the memory-mapped IO region
 * Returns:		status code
 */
efi_status_t efi_add_runtime_mmio(void *mmio_ptr, u64 len)
{
	struct efi_runtime_mmio_list *newmmio;
	uint64_t addr = *(uintptr_t *)mmio_ptr;
	efi_status_t ret;

	ret = efi_add_memory_map(addr, len, EFI_MMAP_IO);
	if (ret != EFI_SUCCESS)
		return EFI_OUT_OF_RESOURCES;

	newmmio = calloc(1, sizeof(*newmmio));
	if (!newmmio)
		return EFI_OUT_OF_RESOURCES;
	newmmio->ptr = mmio_ptr;
	newmmio->paddr = *(uintptr_t *)mmio_ptr;
	newmmio->len = len;
	list_add_tail(&newmmio->link, &efi_runtime_mmio);

	return EFI_SUCCESS;
}

/*
 * In the second stage, U-Boot has disappeared. To isolate our runtime code
 * that at this point still exists from the rest, we put it into a special
 * section.
 *
 *        !!WARNING!!
 *
 * This means that we can not rely on any code outside of this file in any
 * function or variable below this line.
 *
 * Please keep everything fully self-contained and annotated with
 * __efi_runtime and __efi_runtime_data markers.
 */

/*
 * Relocate the EFI runtime stub to a different place. We need to call this
 * the first time we expose the runtime interface to a user and on set virtual
 * address map calls.
 */

/**
 * efi_unimplemented() - replacement function, returns EFI_UNSUPPORTED
 *
 * This function is used after SetVirtualAddressMap() is called as replacement
 * for services that are not available anymore due to constraints of the U-Boot
 * implementation.
 *
 * Return:	EFI_UNSUPPORTED
 */
static efi_status_t __efi_runtime EFIAPI efi_unimplemented(void)
{
	return EFI_UNSUPPORTED;
}

struct efi_runtime_services __efi_runtime_data efi_runtime_services = {
	.hdr = {
		.signature = EFI_RUNTIME_SERVICES_SIGNATURE,
		.revision = EFI_SPECIFICATION_VERSION,
		.headersize = sizeof(struct efi_runtime_services),
	},
	.get_time = &efi_get_time_boottime,
	.set_time = &efi_set_time_boottime,
	.get_wakeup_time = (void *)&efi_unimplemented,
	.set_wakeup_time = (void *)&efi_unimplemented,
	.set_virtual_address_map = &efi_set_virtual_address_map,
	.convert_pointer = efi_convert_pointer,
	.get_variable = efi_get_variable,
	.get_next_variable_name = efi_get_next_variable_name,
	.set_variable = efi_set_variable,
	.get_next_high_mono_count = (void *)&efi_unimplemented,
	.reset_system = &efi_reset_system_boottime,
#ifdef CONFIG_EFI_RUNTIME_UPDATE_CAPSULE
	.update_capsule = efi_update_capsule,
	.query_capsule_caps = efi_query_capsule_caps,
#else
	.update_capsule = efi_update_capsule_unsupported,
	.query_capsule_caps = efi_query_capsule_caps_unsupported,
#endif
	.query_variable_info = efi_query_variable_info,
};
