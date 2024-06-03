// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#define IOTRACE_IMPL

#include <common.h>
#include <mapmem.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct iotrace - current trace status and checksum
 *
 * @start:	Start address of iotrace buffer
 * @size:	Actual size of iotrace buffer in bytes
 * @needed_size: Needed of iotrace buffer in bytes
 * @offset:	Current write offset into iotrace buffer
 * @region_start: Address of IO region to trace
 * @region_size: Size of region to trace. if 0 will trace all address space
 * @crc32:	Current value of CRC chceksum of trace records
 * @enabled:	true if enabled, false if disabled
 */
static struct iotrace {
	ulong start;
	ulong size;
	ulong needed_size;
	ulong offset;
	ulong region_start;
	ulong region_size;
	u32 crc32;
	bool enabled;
} iotrace;

static void add_record(int flags, const void *ptr, ulong value)
{
	struct iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	if (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		return;

	if (iotrace.region_size)
		if ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_size)
			return;

	/* Store it if there is room */
	if (iotrace.offset + sizeof(*rec) < iotrace.size) {
		rec = (struct iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					sizeof(value));
	} else {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_size += sizeof(struct iotrace_record);
		return;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->value = value;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (unsigned char *)rec,
			      sizeof(*rec));

	iotrace.needed_size += sizeof(struct iotrace_record);
	iotrace.offset += sizeof(struct iotrace_record);
}

u32 iotrace_readl(const void *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	return v;
}

void iotrace_writel(ulong value, const void *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, value);
	writel(value, ptr);
}

u16 iotrace_readw(const void *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	return v;
}

void iotrace_writew(ulong value, const void *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, value);
	writew(value, ptr);
}

u8 iotrace_readb(const void *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	return v;
}

void iotrace_writeb(ulong value, const void *ptr)
{
	add_record(IOT_8 | IOT_WRITE, ptr, value);
	writeb(value, ptr);
}

void iotrace_reset_checksum(void)
{
	iotrace.crc32 = 0;
}

u32 iotrace_get_checksum(void)
{
	return iotrace.crc32;
}

void iotrace_set_region(ulong start, ulong size)
{
	iotrace.region_start = start;
	iotrace.region_size = size;
}

void iotrace_reset_region(void)
{
	iotrace.region_start = 0;
	iotrace.region_size = 0;
}

void iotrace_get_region(ulong *start, ulong *size)
{
	*start = iotrace.region_start;
	*size = iotrace.region_size;
}

void iotrace_set_enabled(int enable)
{
	iotrace.enabled = enable;
}

int iotrace_get_enabled(void)
{
	return iotrace.enabled;
}

void iotrace_set_buffer(ulong start, ulong size)
{
	iotrace.start = start;
	iotrace.size = size;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

void iotrace_get_buffer(ulong *start, ulong *size, ulong *needed_size, ulong *offset, ulong *count)
{
	*start = iotrace.start;
	*size = iotrace.size;
	*needed_size = iotrace.needed_size;
	*offset = iotrace.offset;
	*count = iotrace.offset / sizeof(struct iotrace_record);
}
