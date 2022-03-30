// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */


/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	RECORD_COUNT = CONFIG_VAL(BOOTSTAGE_RECORD_COUNT),
};

struct bootstage_record {
	ulong time_us;
	uint32_t start_us;
	const char *name;
	int flags;		/* see enum bootstage_flags */
	enum bootstage_id id;
};

struct bootstage_data {
	uint rec_count;
	uint next_id;
	struct bootstage_record record[RECORD_COUNT];
};

enum {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

struct bootstage_hdr {
	uint32_t version;	/* BOOTSTAGE_VERSION */
	uint32_t count;		/* Number of records */
	uint32_t size;		/* Total data size (non-zero if valid) */
	uint32_t magic;		/* Unused */
};

int bootstage_relocate(void)
{
	struct bootstage_data *data = gd->bootstage;
	int i;

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_count);
	for (i = 0; i < data->rec_count; i++)
		data->record[i].name = strdup(data->record[i].name);

	return 0;
}

struct bootstage_record *find_id(struct bootstage_data *data,
				 enum bootstage_id id)
{
	struct bootstage_record *rec;
	struct bootstage_record *end;

	for (rec = data->record, end = rec + data->rec_count; rec < end;
	     rec++) {
		if (rec->id == id)
			return rec;
	}

	return NULL;
}

struct bootstage_record *ensure_id(struct bootstage_data *data,
				   enum bootstage_id id)
{
	struct bootstage_record *rec;

	rec = find_id(data, id);
	if (!rec && data->rec_count < RECORD_COUNT) {
		rec = &data->record[data->rec_count++];
		rec->id = id;
		return rec;
	}

	return rec;
}

ulong bootstage_add_record(enum bootstage_id id, const char *name,
			   int flags, ulong mark)
{
	struct bootstage_data *data = gd->bootstage;
	struct bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	if (!data)
		return mark;
	if (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	if (!rec && data->rec_count < RECORD_COUNT) {
		rec = &data->record[data->rec_count++];
		rec->time_us = mark;
		rec->name = name;
		rec->flags = flags;
		rec->id = id;
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	return mark;
}


ulong bootstage_mark(enum bootstage_id id)
{
	return bootstage_add_record(id, NULL, 0, timer_get_boot_us());
}

ulong bootstage_error(enum bootstage_id id)
{
	return bootstage_add_record(id, NULL, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(enum bootstage_id id, const char *name)
{
	int flags = 0;

	if (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	return bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(const char *file, const char *func, int linenum)
{
	char *str, *p;
	__maybe_unused char *end;
	int len = 0;

	/* First work out the length we need to allocate */
	if (linenum != -1)
		len = 11;
	if (func)
		len += strlen(func);
	if (file)
		len += strlen(file);

	str = malloc(len + 1);
	p = str;
	end = p + len;
	if (file)
		p += snprintf(p, end - p, "%s,", file);
	if (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	if (func)
		p += snprintf(p, end - p, ": %s", func);

	return bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(enum bootstage_id id, const char *name)
{
	struct bootstage_data *data = gd->bootstage;
	struct bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	if (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	return start_us;
}

uint32_t bootstage_accum(enum bootstage_id id)
{
	struct bootstage_data *data = gd->bootstage;
	struct bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	if (!rec)
		return 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	return duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * @return pointer to name, either from the record or pointing to buf.
 */
static const char *get_record_name(char *buf, int len,
				   const struct bootstage_record *rec)
{
	if (rec->name)
		return rec->name;
	else if (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	else
		snprintf(buf, len, "id=%d", rec->id);

	return buf;
}

static uint32_t print_time_record(struct bootstage_record *rec, uint32_t prev)
{
	char buf[20];

	if (prev == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} else {
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, sizeof(buf), rec));

	return rec->time_us;
}

static int h_compare_record(const void *r1, const void *r2)
{
	const struct bootstage_record *rec1 = r1, *rec2 = r2;

	return rec1->time_us > rec2->time_us ? 1 : -1;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * @return 0 on success, != 0 on failure.
 */
static int add_bootstages_devicetree(struct fdt_header *blob)
{
	struct bootstage_data *data = gd->bootstage;
	int bootstage;
	char buf[20];
	int recnum;
	int i;

	if (!blob)
		return 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	if (bootstage < 0)
		return -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	for (recnum = data->rec_count - 1, i = 0; recnum >= 0; recnum--, i++) {
		struct bootstage_record *rec = &data->record[recnum];
		int node;

		if (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			continue;

		node = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		if (node < 0)
			break;

		/* add properties to the node. */
		if (fdt_setprop_string(blob, node, "name",
				       get_record_name(buf, sizeof(buf), rec)))
			return -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		if (fdt_setprop_cell(blob, node,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			return -EINVAL;
	}

	return 0;
}

int bootstage_fdt_add_report(void)
{
	if (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	return 0;
}
#endif

void bootstage_report(void)
{
	struct bootstage_data *data = gd->bootstage;
	struct bootstage_record *rec = data->record;
	uint32_t prev;
	int i;

	printf("Timer summary in microseconds (%d records):\n",
	       data->rec_count);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	/* Sort records by increasing time */
	qsort(data->record, data->rec_count, sizeof(*rec), h_compare_record);

	for (i = 1, rec++; i < data->rec_count; i++, rec++) {
		if (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	if (data->rec_count > RECORD_COUNT)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(SPL_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_count - RECORD_COUNT);

	puts("\nAccumulated time:\n");
	for (i = 0, rec = data->record; i < data->rec_count; i++, rec++) {
		if (rec->start_us)
			prev = print_time_record(rec, -1);
	}
}

/**
 * Append data to a memory buffer
 *
 * Write data to the buffer if there is space. Whether there is space or not,
 * the buffer pointer is incremented.
 *
 * @param ptrp	Pointer to buffer, updated by this function
 * @param end	Pointer to end of buffer
 * @param data	Data to write to buffer
 * @param size	Size of data
 */
static void append_data(char **ptrp, char *end, const void *data, int size)
{
	char *ptr = *ptrp;

	*ptrp += size;
	if (*ptrp > end)
		return;

	memcpy(ptr, data, size);
}

int bootstage_stash(void *base, int size)
{
	const struct bootstage_data *data = gd->bootstage;
	struct bootstage_hdr *hdr = (struct bootstage_hdr *)base;
	const struct bootstage_record *rec;
	char buf[20];
	char *ptr = base, *end = ptr + size;
	uint32_t count;
	int i;

	if (hdr + 1 > (struct bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		return -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	/* Count the number of records, and write that value first */
	for (rec = data->record, i = count = 0; i < data->rec_count;
	     i++, rec++) {
		if (rec->id != 0)
			count++;
	}
	hdr->count = count;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	ptr += sizeof(*hdr);

	/* Write the records, silently stopping when we run out of space */
	for (rec = data->record, i = 0; i < data->rec_count; i++, rec++) {
		append_data(&ptr, end, rec, sizeof(*rec));
	}

	/* Write the name strings */
	for (rec = data->record, i = 0; i < data->rec_count; i++, rec++) {
		const char *name;

		name = get_record_name(buf, sizeof(buf), rec);
		append_data(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	if (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		return -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (char *)base;
	debug("Stashed %d records\n", hdr->count);

	return 0;
}

int bootstage_unstash(const void *base, int size)
{
	const struct bootstage_hdr *hdr = (struct bootstage_hdr *)base;
	struct bootstage_data *data = gd->bootstage;
	const char *ptr = base, *end = ptr + size;
	struct bootstage_record *rec;
	uint rec_size;
	int i;

	if (size == -1)
		end = (char *)(~(uintptr_t)0);

	if (hdr + 1 > (struct bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		return -EPERM;
	}

	if (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		return -ENOENT;
	}

	if (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		return -ENOSPC;
	}

	if (hdr->count * sizeof(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->count,
		      (ulong)hdr->count * sizeof(*rec), hdr->size);
		return -ENOSPC;
	}

	if (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		return -EINVAL;
	}

	if (data->rec_count + hdr->count > RECORD_COUNT) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(SPL_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->count, RECORD_COUNT - data->rec_count);
		return -ENOSPC;
	}

	ptr += sizeof(*hdr);

	/* Read the records */
	rec_size = hdr->count * sizeof(*data->record);
	memcpy(data->record + data->rec_count, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	for (rec = data->record + data->next_id, i = 0; i < hdr->count;
	     i++, rec++) {
		rec->name = ptr;

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_count += hdr->count;
	debug("Unstashed %d records\n", hdr->count);

	return 0;
}

int bootstage_get_size(void)
{
	return sizeof(struct bootstage_data);
}

int bootstage_init(bool first)
{
	struct bootstage_data *data;
	int size = sizeof(struct bootstage_data);

	gd->bootstage = (struct bootstage_data *)malloc(size);
	if (!gd->bootstage)
		return -ENOMEM;
	data = gd->bootstage;
	memset(data, '\0', size);
	if (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	return 0;
}
