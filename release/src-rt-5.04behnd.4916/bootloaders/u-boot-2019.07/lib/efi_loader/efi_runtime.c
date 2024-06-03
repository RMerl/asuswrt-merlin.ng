// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application runtime services
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <elf.h>
#include <efi_loader.h>
#include <rtc.h>

/* For manual relocation support */
DECLARE_GLOBAL_DATA_PTR;

struct efi_runtime_mmio_list {
	struct list_head link;
	void **ptr;
	u64 paddr;
	u64 len;
};

/* This list contains all runtime available mmio regions */
LIST_HEAD(efi_runtime_mmio);

static efi_status_t __efi_runtime EFIAPI efi_unimplemented(void);
static efi_status_t __efi_runtime EFIAPI efi_device_error(void);
static efi_status_t __efi_runtime EFIAPI efi_invalid_parameter(void);

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

/*
 * EFI runtime code lives in two stages. In the first stage, U-Boot and an EFI
 * payload are running concurrently at the same time. In this mode, we can
 * handle a good number of runtime callbacks
 */

efi_status_t efi_init_runtime_supported(void)
{
	u16 efi_runtime_services_supported = 0;

	/*
	 * This value must be synced with efi_runtime_detach_list
	 * as well as efi_runtime_services.
	 */
#if CONFIG_IS_ENABLED(ARCH_BCM283X) || \
    CONFIG_IS_ENABLED(FSL_LAYERSCAPE) || \
    CONFIG_IS_ENABLED(SYSRESET_X86) || \
    CONFIG_IS_ENABLED(PSCI_RESET)
	efi_runtime_services_supported |= EFI_RT_SUPPORTED_RESET_SYSTEM;
#endif
	efi_runtime_services_supported |=
				EFI_RT_SUPPORTED_SET_VIRTUAL_ADDRESS_MAP;
	return EFI_CALL(efi_set_variable(L"RuntimeServicesSupported",
					 &efi_global_variable_guid,
					 EFI_VARIABLE_BOOTSERVICE_ACCESS |
					 EFI_VARIABLE_RUNTIME_ACCESS,
					 sizeof(efi_runtime_services_supported),
					 &efi_runtime_services_supported));
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
	if (tm.tm_isdst)
		time->daylight =
			EFI_TIME_ADJUST_DAYLIGHT | EFI_TIME_IN_DAYLIGHT;
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
	tm.tm_isdst = time->daylight ==
		      (EFI_TIME_ADJUST_DAYLIGHT | EFI_TIME_IN_DAYLIGHT);
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
 * SetVirtualAddressMap() is called. It only executes an endless loop.
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
	/* Nothing we can do */
	while (1) { }
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

struct efi_runtime_detach_list_struct {
	void *ptr;
	void *patchto;
};

static const struct efi_runtime_detach_list_struct efi_runtime_detach_list[] = {
	{
		/* do_reset is gone */
		.ptr = &efi_runtime_services.reset_system,
		.patchto = efi_reset_system,
	}, {
		/* invalidate_*cache_all are gone */
		.ptr = &efi_runtime_services.set_virtual_address_map,
		.patchto = &efi_unimplemented,
	}, {
		/* RTC accessors are gone */
		.ptr = &efi_runtime_services.get_time,
		.patchto = &efi_get_time,
	}, {
		.ptr = &efi_runtime_services.set_time,
		.patchto = &efi_set_time,
	}, {
		/* Clean up system table */
		.ptr = &systab.con_in,
		.patchto = NULL,
	}, {
		/* Clean up system table */
		.ptr = &systab.con_out,
		.patchto = NULL,
	}, {
		/* Clean up system table */
		.ptr = &systab.std_err,
		.patchto = NULL,
	}, {
		/* Clean up system table */
		.ptr = &systab.boottime,
		.patchto = NULL,
	}, {
		.ptr = &efi_runtime_services.get_variable,
		.patchto = &efi_device_error,
	}, {
		.ptr = &efi_runtime_services.get_next_variable_name,
		.patchto = &efi_device_error,
	}, {
		.ptr = &efi_runtime_services.set_variable,
		.patchto = &efi_device_error,
	}
};

static bool efi_runtime_tobedetached(void *p)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(efi_runtime_detach_list); i++)
		if (efi_runtime_detach_list[i].ptr == p)
			return true;

	return false;
}

static void efi_runtime_detach(ulong offset)
{
	int i;
	ulong patchoff = offset - (ulong)gd->relocaddr;

	for (i = 0; i < ARRAY_SIZE(efi_runtime_detach_list); i++) {
		ulong patchto = (ulong)efi_runtime_detach_list[i].patchto;
		ulong *p = efi_runtime_detach_list[i].ptr;
		ulong newaddr = patchto ? (patchto + patchoff) : 0;

		debug("%s: Setting %p to %lx\n", __func__, p, newaddr);
		*p = newaddr;
	}

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
	static ulong lastoff = CONFIG_SYS_TEXT_BASE;
#endif

	debug("%s: Relocating to offset=%lx\n", __func__, offset);
	for (; (ulong)rel < (ulong)&__efi_runtime_rel_stop; rel++) {
		ulong base = CONFIG_SYS_TEXT_BASE;
		ulong *p;
		ulong newaddr;

		p = (void*)((ulong)rel->offset - base) + gd->relocaddr;

		debug("%s: rel->info=%#lx *p=%#lx rel->offset=%p\n", __func__,
		      rel->info, *p, rel->offset);

		switch (rel->info & R_MASK) {
		case R_RELATIVE:
#ifdef IS_RELA
		newaddr = rel->addend + offset - CONFIG_SYS_TEXT_BASE;
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
			newaddr -= CONFIG_SYS_TEXT_BASE;
#endif
			break;
		}
#endif
		default:
			if (!efi_runtime_tobedetached(p))
				printf("%s: Unknown relocation type %llx\n",
				       __func__, rel->info & R_MASK);
			continue;
		}

		/* Check if the relocation is inside bounds */
		if (map && ((newaddr < map->virtual_start) ||
		    newaddr > (map->virtual_start +
			      (map->num_pages << EFI_PAGE_SHIFT)))) {
			if (!efi_runtime_tobedetached(p))
				printf("%s: Relocation at %p is out of "
				       "range (%lx)\n", __func__, p, newaddr);
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
			unsigned long memory_map_size,
			unsigned long descriptor_size,
			uint32_t descriptor_version,
			struct efi_mem_desc *virtmap)
{
	int n = memory_map_size / descriptor_size;
	int i;
	int rt_code_sections = 0;

	EFI_ENTRY("%lx %lx %x %p", memory_map_size, descriptor_size,
		  descriptor_version, virtmap);

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
		return EFI_EXIT(EFI_INVALID_PARAMETER);
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

	/* Move the actual runtime code over */
	for (i = 0; i < n; i++) {
		struct efi_mem_desc *map;

		map = (void*)virtmap + (descriptor_size * i);
		if (map->type == EFI_RUNTIME_SERVICES_CODE) {
			ulong new_offset = map->virtual_start -
					   map->physical_start + gd->relocaddr;

			efi_runtime_relocate(new_offset, map);
			/* Once we're virtual, we can no longer handle
			   complex callbacks */
			efi_runtime_detach(new_offset);
			return EFI_EXIT(EFI_SUCCESS);
		}
	}

	return EFI_EXIT(EFI_INVALID_PARAMETER);
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
	u64 pages = (len + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT;
	uint64_t addr = *(uintptr_t *)mmio_ptr;
	uint64_t retaddr;

	retaddr = efi_add_memory_map(addr, pages, EFI_MMAP_IO, false);
	if (retaddr != addr)
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

/**
 * efi_device_error() - replacement function, returns EFI_DEVICE_ERROR
 *
 * This function is used after SetVirtualAddressMap() is called as replacement
 * for services that are not available anymore due to constraints of the U-Boot
 * implementation.
 *
 * Return:	EFI_DEVICE_ERROR
 */
static efi_status_t __efi_runtime EFIAPI efi_device_error(void)
{
	return EFI_DEVICE_ERROR;
}

/**
 * efi_invalid_parameter() - replacement function, returns EFI_INVALID_PARAMETER
 *
 * This function is used after SetVirtualAddressMap() is called as replacement
 * for services that are not available anymore due to constraints of the U-Boot
 * implementation.
 *
 * Return:	EFI_INVALID_PARAMETER
 */
static efi_status_t __efi_runtime EFIAPI efi_invalid_parameter(void)
{
	return EFI_INVALID_PARAMETER;
}

/**
 * efi_update_capsule() - process information from operating system
 *
 * This function implements the UpdateCapsule() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @capsule_header_array:	pointer to array of virtual pointers
 * @capsule_count:		number of pointers in capsule_header_array
 * @scatter_gather_list:	pointer to arry of physical pointers
 * Returns:			status code
 */
efi_status_t __efi_runtime EFIAPI efi_update_capsule(
			struct efi_capsule_header **capsule_header_array,
			efi_uintn_t capsule_count,
			u64 scatter_gather_list)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_query_capsule_caps() - check if capsule is supported
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
efi_status_t __efi_runtime EFIAPI efi_query_capsule_caps(
			struct efi_capsule_header **capsule_header_array,
			efi_uintn_t capsule_count,
			u64 *maximum_capsule_size,
			u32 *reset_type)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_query_variable_info() - get information about EFI variables
 *
 * This function implements the QueryVariableInfo() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @attributes:				bitmask to select variables to be
 *					queried
 * @maximum_variable_storage_size:	maximum size of storage area for the
 *					selected variable types
 * @remaining_variable_storage_size:	remaining size of storage are for the
 *					selected variable types
 * @maximum_variable_size:		maximum size of a variable of the
 *					selected type
 * Returns:				status code
 */
efi_status_t __efi_runtime EFIAPI efi_query_variable_info(
			u32 attributes,
			u64 *maximum_variable_storage_size,
			u64 *remaining_variable_storage_size,
			u64 *maximum_variable_size)
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
	.convert_pointer = (void *)&efi_invalid_parameter,
	.get_variable = efi_get_variable,
	.get_next_variable_name = efi_get_next_variable_name,
	.set_variable = efi_set_variable,
	.get_next_high_mono_count = (void *)&efi_device_error,
	.reset_system = &efi_reset_system_boottime,
	.update_capsule = efi_update_capsule,
	.query_capsule_caps = efi_query_capsule_caps,
	.query_variable_info = efi_query_variable_info,
};
