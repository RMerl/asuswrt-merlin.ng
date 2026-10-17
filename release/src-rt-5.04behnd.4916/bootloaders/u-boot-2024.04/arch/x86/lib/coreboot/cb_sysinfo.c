// SPDX-License-Identifier: BSD-3-Clause
/*
 * This file is part of the libpayload project.
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 * Copyright (C) 2009 coresystems GmbH
 */

#include <common.h>
#include <asm/cb_sysinfo.h>
#include <init.h>
#include <mapmem.h>
#include <net.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * This needs to be in the .data section so that it's copied over during
 * relocation. By default it's put in the .bss section which is simply filled
 * with zeroes when transitioning from "ROM", which is really RAM, to other
 * RAM.
 */
struct sysinfo_t lib_sysinfo __section(".data");

/*
 * Some of this is x86 specific, and the rest of it is generic. Right now,
 * since we only support x86, we'll avoid trying to make lots of infrastructure
 * we don't need. If in the future, we want to use coreboot on some other
 * architecture, then take out the generic parsing code and move it elsewhere.
 */

/* === Parsing code === */
/* This is the generic parsing code */

static void cb_parse_memory(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_memory *mem = (struct cb_memory *)ptr;
	int count = MEM_RANGE_COUNT(mem);
	int i;

	if (count > SYSINFO_MAX_MEM_RANGES)
		count = SYSINFO_MAX_MEM_RANGES;

	info->n_memranges = 0;

	for (i = 0; i < count; i++) {
		struct cb_memory_range *range =
		    (struct cb_memory_range *)MEM_RANGE_PTR(mem, i);

		info->memrange[info->n_memranges].base =
		    UNPACK_CB64(range->start);

		info->memrange[info->n_memranges].size =
		    UNPACK_CB64(range->size);

		info->memrange[info->n_memranges].type = range->type;

		info->n_memranges++;
	}
}

static void cb_parse_serial(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_serial *ser = (struct cb_serial *)ptr;

	info->serial = ser;
}

static void cb_parse_vboot_handoff(unsigned char *ptr, struct sysinfo_t *info)
{
	struct lb_range *vbho = (struct lb_range *)ptr;

	info->vboot_handoff = (void *)(uintptr_t)vbho->range_start;
	info->vboot_handoff_size = vbho->range_size;
}

static void cb_parse_vbnv(unsigned char *ptr, struct sysinfo_t *info)
{
	struct lb_range *vbnv = (struct lb_range *)ptr;

	info->vbnv_start = vbnv->range_start;
	info->vbnv_size = vbnv->range_size;
}

static void cb_parse_cbmem_entry(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_cbmem_entry *entry = (struct cb_cbmem_entry *)ptr;

	if (entry->id != CBMEM_ID_SMBIOS)
		return;

	info->smbios_start = entry->address;
	info->smbios_size = entry->entry_size;
}

static void cb_parse_gpios(unsigned char *ptr, struct sysinfo_t *info)
{
	int i;
	struct cb_gpios *gpios = (struct cb_gpios *)ptr;

	info->num_gpios = (gpios->count < SYSINFO_MAX_GPIOS) ?
				(gpios->count) : SYSINFO_MAX_GPIOS;

	for (i = 0; i < info->num_gpios; i++)
		info->gpios[i] = gpios->gpios[i];
}

static void cb_parse_vdat(unsigned char *ptr, struct sysinfo_t *info)
{
	struct lb_range *vdat = (struct lb_range *)ptr;

	info->vdat_addr = map_sysmem(vdat->range_start, vdat->range_size);
	info->vdat_size = vdat->range_size;
}

static void cb_parse_mac_addresses(unsigned char *ptr,
				   struct sysinfo_t *info)
{
	struct cb_macs *macs = (struct cb_macs *)ptr;
	int i;

	info->num_macs = (macs->count < ARRAY_SIZE(info->macs)) ?
		macs->count : ARRAY_SIZE(info->macs);

	for (i = 0; i < info->num_macs; i++)
		info->macs[i] = macs->mac_addrs[i];
}

static void cb_parse_tstamp(void *ptr, struct sysinfo_t *info)
{
	struct cb_cbmem_tab *const cbmem = ptr;

	info->tstamp_table = map_sysmem(cbmem->cbmem_tab, 0);
}

static void cb_parse_cbmem_cons(void *ptr, struct sysinfo_t *info)
{
	struct cb_cbmem_tab *const cbmem = ptr;

	info->cbmem_cons = map_sysmem(cbmem->cbmem_tab, 0);
}

static void cb_parse_acpi_gnvs(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_cbmem_tab *const cbmem = (struct cb_cbmem_tab *)ptr;

	info->acpi_gnvs = map_sysmem(cbmem->cbmem_tab, 0);
}

static void cb_parse_board_id(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_board_id *const cbbid = (struct cb_board_id *)ptr;

	info->board_id = cbbid->board_id;
}

static void cb_parse_ram_code(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_ram_code *const ram_code = (struct cb_ram_code *)ptr;

	info->ram_code = ram_code->ram_code;
}

static void cb_parse_optiontable(void *ptr, struct sysinfo_t *info)
{
	/* ptr points to a coreboot table entry and is already virtual */
	info->option_table = ptr;
}

static void cb_parse_checksum(void *ptr, struct sysinfo_t *info)
{
	struct cb_cmos_checksum *cmos_cksum = ptr;

	info->cmos_range_start = cmos_cksum->range_start;
	info->cmos_range_end = cmos_cksum->range_end;
	info->cmos_checksum_location = cmos_cksum->location;
}

static void cb_parse_framebuffer(void *ptr, struct sysinfo_t *info)
{
	/* ptr points to a coreboot table entry and is already virtual */
	info->framebuffer = ptr;
}

static void cb_parse_string(unsigned char *ptr, char **info)
{
	*info = (char *)((struct cb_string *)ptr)->string;
}

static void cb_parse_wifi_calibration(void *ptr, struct sysinfo_t *info)
{
	struct cb_cbmem_tab *const cbmem = (struct cb_cbmem_tab *)ptr;

	info->wifi_calibration = map_sysmem(cbmem->cbmem_tab, 0);
}

static void cb_parse_ramoops(void *ptr, struct sysinfo_t *info)
{
	struct lb_range *ramoops = (struct lb_range *)ptr;

	info->ramoops_buffer = ramoops->range_start;
	info->ramoops_buffer_size = ramoops->range_size;
}

static void cb_parse_mtc(void *ptr, struct sysinfo_t *info)
{
	struct lb_range *mtc = (struct lb_range *)ptr;

	info->mtc_start = mtc->range_start;
	info->mtc_size = mtc->range_size;
}

static void cb_parse_spi_flash(void *ptr, struct sysinfo_t *info)
{
	struct cb_spi_flash *flash = (struct cb_spi_flash *)ptr;

	info->spi_flash.size = flash->flash_size;
	info->spi_flash.sector_size = flash->sector_size;
	info->spi_flash.erase_cmd = flash->erase_cmd;
}

static void cb_parse_boot_media_params(unsigned char *ptr,
				       struct sysinfo_t *info)
{
	struct cb_boot_media_params *const bmp =
			(struct cb_boot_media_params *)ptr;

	info->fmap_offset = bmp->fmap_offset;
	info->cbfs_offset = bmp->cbfs_offset;
	info->cbfs_size = bmp->cbfs_size;
	info->boot_media_size = bmp->boot_media_size;
}

static void cb_parse_vpd(void *ptr, struct sysinfo_t *info)
{
	struct cb_cbmem_tab *const cbmem = (struct cb_cbmem_tab *)ptr;

	info->chromeos_vpd = map_sysmem(cbmem->cbmem_tab, 0);
}

static void cb_parse_tsc_info(void *ptr, struct sysinfo_t *info)
{
	const struct cb_tsc_info *tsc_info = ptr;

	if (tsc_info->freq_khz == 0)
		return;

	/* Honor the TSC frequency passed to the payload */
	info->cpu_khz = tsc_info->freq_khz;
}

static void cb_parse_x86_rom_var_mtrr(void *ptr, struct sysinfo_t *info)
{
	struct cb_x86_rom_mtrr *rom_mtrr = ptr;

	info->x86_rom_var_mtrr_index = rom_mtrr->index;
}

static void cb_parse_mrc_cache(void *ptr, struct sysinfo_t *info)
{
	struct cb_cbmem_tab *const cbmem = (struct cb_cbmem_tab *)ptr;

	info->mrc_cache = map_sysmem(cbmem->cbmem_tab, 0);
}

static void cb_parse_acpi_rsdp(void *ptr, struct sysinfo_t *info)
{
	struct cb_cbmem_tab *const cbmem = (struct cb_cbmem_tab *)ptr;

	info->rsdp = map_sysmem(cbmem->cbmem_tab, 0);
}

__weak void cb_parse_unhandled(u32 tag, unsigned char *ptr)
{
}

static int cb_parse_header(void *addr, int len, struct sysinfo_t *info)
{
	unsigned char *ptr = addr;
	struct cb_header *header;
	int i;

	header = (struct cb_header *)ptr;
	if (!header->table_bytes)
		return 0;

	/* Make sure the checksums match */
	if (!ip_checksum_ok(header, sizeof(*header)))
		return -1;

	if (compute_ip_checksum(ptr + sizeof(*header), header->table_bytes) !=
	    header->table_checksum)
		return -1;

	info->header = header;

	/*
	 * Board straps represented by numerical values are small numbers.
	 * Preset them to an invalid value in case the firmware does not
	 * supply the info.
	 */
	info->board_id = ~0;
	info->ram_code = ~0;

	/* Now, walk the tables */
	ptr += header->header_bytes;

	/* Inintialize some fields to sentinel values */
	info->vbnv_start = info->vbnv_size = (uint32_t)(-1);

	for (i = 0; i < header->table_entries; i++) {
		struct cb_record *rec = (struct cb_record *)ptr;

		/* We only care about a few tags here (maybe more later) */
		switch (rec->tag) {
		case CB_TAG_FORWARD:
			return cb_parse_header(
				(void *)(unsigned long)
				((struct cb_forward *)rec)->forward,
				len, info);
			continue;
		case CB_TAG_MEMORY:
			cb_parse_memory(ptr, info);
			break;
		case CB_TAG_SERIAL:
			cb_parse_serial(ptr, info);
			break;
		case CB_TAG_VERSION:
			cb_parse_string(ptr, &info->cb_version);
			break;
		case CB_TAG_EXTRA_VERSION:
			cb_parse_string(ptr, &info->extra_version);
			break;
		case CB_TAG_BUILD:
			cb_parse_string(ptr, &info->build);
			break;
		case CB_TAG_COMPILE_TIME:
			cb_parse_string(ptr, &info->compile_time);
			break;
		case CB_TAG_COMPILE_BY:
			cb_parse_string(ptr, &info->compile_by);
			break;
		case CB_TAG_COMPILE_HOST:
			cb_parse_string(ptr, &info->compile_host);
			break;
		case CB_TAG_COMPILE_DOMAIN:
			cb_parse_string(ptr, &info->compile_domain);
			break;
		case CB_TAG_COMPILER:
			cb_parse_string(ptr, &info->compiler);
			break;
		case CB_TAG_LINKER:
			cb_parse_string(ptr, &info->linker);
			break;
		case CB_TAG_ASSEMBLER:
			cb_parse_string(ptr, &info->assembler);
			break;
		case CB_TAG_CMOS_OPTION_TABLE:
			cb_parse_optiontable(ptr, info);
			break;
		case CB_TAG_OPTION_CHECKSUM:
			cb_parse_checksum(ptr, info);
			break;
		/*
		 * FIXME we should warn on serial if coreboot set up a
		 * framebuffer buf the payload does not know about it.
		 */
		case CB_TAG_FRAMEBUFFER:
			cb_parse_framebuffer(ptr, info);
			break;
		case CB_TAG_MAINBOARD:
			info->mainboard = (struct cb_mainboard *)ptr;
			break;
		case CB_TAG_GPIO:
			cb_parse_gpios(ptr, info);
			break;
		case CB_TAG_VDAT:
			cb_parse_vdat(ptr, info);
			break;
		case CB_TAG_VBNV:
			cb_parse_vbnv(ptr, info);
			break;
		case CB_TAG_VBOOT_HANDOFF:
			cb_parse_vboot_handoff(ptr, info);
			break;
		case CB_TAG_MAC_ADDRS:
			cb_parse_mac_addresses(ptr, info);
			break;
		case CB_TAG_SERIALNO:
			cb_parse_string(ptr, &info->serialno);
			break;
		case CB_TAG_TIMESTAMPS:
			cb_parse_tstamp(ptr, info);
			break;
		case CB_TAG_CBMEM_CONSOLE:
			cb_parse_cbmem_cons(ptr, info);
			break;
		case CB_TAG_ACPI_GNVS:
			cb_parse_acpi_gnvs(ptr, info);
			break;
		case CB_TAG_CBMEM_ENTRY:
			cb_parse_cbmem_entry(ptr, info);
			break;
		case CB_TAG_BOARD_ID:
			cb_parse_board_id(ptr, info);
			break;
		case CB_TAG_RAM_CODE:
			cb_parse_ram_code(ptr, info);
			break;
		case CB_TAG_WIFI_CALIBRATION:
			cb_parse_wifi_calibration(ptr, info);
			break;
		case CB_TAG_RAM_OOPS:
			cb_parse_ramoops(ptr, info);
			break;
		case CB_TAG_SPI_FLASH:
			cb_parse_spi_flash(ptr, info);
			break;
		case CB_TAG_MTC:
			cb_parse_mtc(ptr, info);
			break;
		case CB_TAG_BOOT_MEDIA_PARAMS:
			cb_parse_boot_media_params(ptr, info);
			break;
		case CB_TAG_TSC_INFO:
			cb_parse_tsc_info(ptr, info);
			break;
		case CB_TAG_VPD:
			cb_parse_vpd(ptr, info);
			break;
		case CB_TAG_X86_ROM_MTRR:
			cb_parse_x86_rom_var_mtrr(rec, info);
			break;
		case CB_TAG_MRC_CACHE:
			cb_parse_mrc_cache(rec, info);
			break;
		case CB_TAG_ACPI_RSDP:
			cb_parse_acpi_rsdp(rec, info);
			break;
		default:
			if (info->unimpl_count < SYSINFO_MAX_UNIMPL)
				info->unimpl[info->unimpl_count++] = rec->tag;
			cb_parse_unhandled(rec->tag, ptr);
			break;
		}

		ptr += rec->size;
	}
	info->table_size += (void *)ptr - (void *)header;
	info->rec_count += header->table_entries;

	return 1;
}

/* == Architecture specific == */
/* This is the x86 specific stuff */

int get_coreboot_info(struct sysinfo_t *info)
{
	long addr;
	int ret;

	addr = locate_coreboot_table();
	if (addr < 0)
		return addr;
	info->table_size = 0;
	info->rec_count = 0;
	ret = cb_parse_header((void *)addr, 0x1000, info);
	if (!ret)
		return -ENOENT;
	gd->arch.coreboot_table = addr;
	gd_set_acpi_start(map_to_sysmem(info->rsdp));
	gd_set_smbios_start(info->smbios_start);
	gd->flags |= GD_FLG_SKIP_LL_INIT;

	return 0;
}

const struct sysinfo_t *cb_get_sysinfo(void)
{
	if (!ll_boot_init())
		return &lib_sysinfo;

	return NULL;
}
