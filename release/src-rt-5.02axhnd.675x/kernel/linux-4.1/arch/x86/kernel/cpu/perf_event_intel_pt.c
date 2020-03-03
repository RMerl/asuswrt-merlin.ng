/*
 * Intel(R) Processor Trace PMU driver for perf
 * Copyright (c) 2013-2014, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * Intel PT is specified in the Intel Architecture Instruction Set Extensions
 * Programming Reference:
 * http://software.intel.com/en-us/intel-isa-extensions
 */

#undef DEBUG

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/device.h>

#include <asm/perf_event.h>
#include <asm/insn.h>
#include <asm/io.h>

#include "perf_event.h"
#include "intel_pt.h"

static DEFINE_PER_CPU(struct pt, pt_ctx);

static struct pt_pmu pt_pmu;

enum cpuid_regs {
	CR_EAX = 0,
	CR_ECX,
	CR_EDX,
	CR_EBX
};

/*
 * Capabilities of Intel PT hardware, such as number of address bits or
 * supported output schemes, are cached and exported to userspace as "caps"
 * attribute group of pt pmu device
 * (/sys/bus/event_source/devices/intel_pt/caps/) so that userspace can store
 * relevant bits together with intel_pt traces.
 *
 * These are necessary for both trace decoding (payloads_lip, contains address
 * width encoded in IP-related packets), and event configuration (bitmasks with
 * permitted values for certain bit fields).
 */
#define PT_CAP(_n, _l, _r, _m)						\
	[PT_CAP_ ## _n] = { .name = __stringify(_n), .leaf = _l,	\
			    .reg = _r, .mask = _m }

static struct pt_cap_desc {
	const char	*name;
	u32		leaf;
	u8		reg;
	u32		mask;
} pt_caps[] = {
	PT_CAP(max_subleaf,		0, CR_EAX, 0xffffffff),
	PT_CAP(cr3_filtering,		0, CR_EBX, BIT(0)),
	PT_CAP(topa_output,		0, CR_ECX, BIT(0)),
	PT_CAP(topa_multiple_entries,	0, CR_ECX, BIT(1)),
	PT_CAP(payloads_lip,		0, CR_ECX, BIT(31)),
};

static u32 pt_cap_get(enum pt_capabilities cap)
{
	struct pt_cap_desc *cd = &pt_caps[cap];
	u32 c = pt_pmu.caps[cd->leaf * 4 + cd->reg];
	unsigned int shift = __ffs(cd->mask);

	return (c & cd->mask) >> shift;
}

static ssize_t pt_cap_show(struct device *cdev,
			   struct device_attribute *attr,
			   char *buf)
{
	struct dev_ext_attribute *ea =
		container_of(attr, struct dev_ext_attribute, attr);
	enum pt_capabilities cap = (long)ea->var;

	return snprintf(buf, PAGE_SIZE, "%x\n", pt_cap_get(cap));
}

static struct attribute_group pt_cap_group = {
	.name	= "caps",
};

PMU_FORMAT_ATTR(tsc,		"config:10"	);
PMU_FORMAT_ATTR(noretcomp,	"config:11"	);

static struct attribute *pt_formats_attr[] = {
	&format_attr_tsc.attr,
	&format_attr_noretcomp.attr,
	NULL,
};

static struct attribute_group pt_format_group = {
	.name	= "format",
	.attrs	= pt_formats_attr,
};

static const struct attribute_group *pt_attr_groups[] = {
	&pt_cap_group,
	&pt_format_group,
	NULL,
};

static int __init pt_pmu_hw_init(void)
{
	struct dev_ext_attribute *de_attrs;
	struct attribute **attrs;
	size_t size;
	int ret;
	long i;

	attrs = NULL;
	ret = -ENODEV;
	if (!test_cpu_cap(&boot_cpu_data, X86_FEATURE_INTEL_PT))
		goto fail;

	for (i = 0; i < PT_CPUID_LEAVES; i++) {
		cpuid_count(20, i,
			    &pt_pmu.caps[CR_EAX + i*4],
			    &pt_pmu.caps[CR_EBX + i*4],
			    &pt_pmu.caps[CR_ECX + i*4],
			    &pt_pmu.caps[CR_EDX + i*4]);
	}

	ret = -ENOMEM;
	size = sizeof(struct attribute *) * (ARRAY_SIZE(pt_caps)+1);
	attrs = kzalloc(size, GFP_KERNEL);
	if (!attrs)
		goto fail;

	size = sizeof(struct dev_ext_attribute) * (ARRAY_SIZE(pt_caps)+1);
	de_attrs = kzalloc(size, GFP_KERNEL);
	if (!de_attrs)
		goto fail;

	for (i = 0; i < ARRAY_SIZE(pt_caps); i++) {
		struct dev_ext_attribute *de_attr = de_attrs + i;

		de_attr->attr.attr.name = pt_caps[i].name;

		sysfs_attr_init(&de_attr->attr.attr);

		de_attr->attr.attr.mode		= S_IRUGO;
		de_attr->attr.show		= pt_cap_show;
		de_attr->var			= (void *)i;

		attrs[i] = &de_attr->attr.attr;
	}

	pt_cap_group.attrs = attrs;

	return 0;

fail:
	kfree(attrs);

	return ret;
}

#define PT_CONFIG_MASK (RTIT_CTL_TSC_EN | RTIT_CTL_DISRETC)

static bool pt_event_valid(struct perf_event *event)
{
	u64 config = event->attr.config;

	if ((config & PT_CONFIG_MASK) != config)
		return false;

	return true;
}

/*
 * PT configuration helpers
 * These all are cpu affine and operate on a local PT
 */

static bool pt_is_running(void)
{
	u64 ctl;

	rdmsrl(MSR_IA32_RTIT_CTL, ctl);

	return !!(ctl & RTIT_CTL_TRACEEN);
}

static void pt_config(struct perf_event *event)
{
	u64 reg;

	reg = RTIT_CTL_TOPA | RTIT_CTL_BRANCH_EN | RTIT_CTL_TRACEEN;

	if (!event->attr.exclude_kernel)
		reg |= RTIT_CTL_OS;
	if (!event->attr.exclude_user)
		reg |= RTIT_CTL_USR;

	reg |= (event->attr.config & PT_CONFIG_MASK);

	wrmsrl(MSR_IA32_RTIT_CTL, reg);
}

static void pt_config_start(bool start)
{
	u64 ctl;

	rdmsrl(MSR_IA32_RTIT_CTL, ctl);
	if (start)
		ctl |= RTIT_CTL_TRACEEN;
	else
		ctl &= ~RTIT_CTL_TRACEEN;
	wrmsrl(MSR_IA32_RTIT_CTL, ctl);

	/*
	 * A wrmsr that disables trace generation serializes other PT
	 * registers and causes all data packets to be written to memory,
	 * but a fence is required for the data to become globally visible.
	 *
	 * The below WMB, separating data store and aux_head store matches
	 * the consumer's RMB that separates aux_head load and data load.
	 */
	if (!start)
		wmb();
}

static void pt_config_buffer(void *buf, unsigned int topa_idx,
			     unsigned int output_off)
{
	u64 reg;

	wrmsrl(MSR_IA32_RTIT_OUTPUT_BASE, virt_to_phys(buf));

	reg = 0x7f | ((u64)topa_idx << 7) | ((u64)output_off << 32);

	wrmsrl(MSR_IA32_RTIT_OUTPUT_MASK, reg);
}

/*
 * Keep ToPA table-related metadata on the same page as the actual table,
 * taking up a few words from the top
 */

#define TENTS_PER_PAGE (((PAGE_SIZE - 40) / sizeof(struct topa_entry)) - 1)

/**
 * struct topa - page-sized ToPA table with metadata at the top
 * @table:	actual ToPA table entries, as understood by PT hardware
 * @list:	linkage to struct pt_buffer's list of tables
 * @phys:	physical address of this page
 * @offset:	offset of the first entry in this table in the buffer
 * @size:	total size of all entries in this table
 * @last:	index of the last initialized entry in this table
 */
struct topa {
	struct topa_entry	table[TENTS_PER_PAGE];
	struct list_head	list;
	u64			phys;
	u64			offset;
	size_t			size;
	int			last;
};

/* make -1 stand for the last table entry */
#define TOPA_ENTRY(t, i) ((i) == -1 ? &(t)->table[(t)->last] : &(t)->table[(i)])

/**
 * topa_alloc() - allocate page-sized ToPA table
 * @cpu:	CPU on which to allocate.
 * @gfp:	Allocation flags.
 *
 * Return:	On success, return the pointer to ToPA table page.
 */
static struct topa *topa_alloc(int cpu, gfp_t gfp)
{
	int node = cpu_to_node(cpu);
	struct topa *topa;
	struct page *p;

	p = alloc_pages_node(node, gfp | __GFP_ZERO, 0);
	if (!p)
		return NULL;

	topa = page_address(p);
	topa->last = 0;
	topa->phys = page_to_phys(p);

	/*
	 * In case of singe-entry ToPA, always put the self-referencing END
	 * link as the 2nd entry in the table
	 */
	if (!pt_cap_get(PT_CAP_topa_multiple_entries)) {
		TOPA_ENTRY(topa, 1)->base = topa->phys >> TOPA_SHIFT;
		TOPA_ENTRY(topa, 1)->end = 1;
	}

	return topa;
}

/**
 * topa_free() - free a page-sized ToPA table
 * @topa:	Table to deallocate.
 */
static void topa_free(struct topa *topa)
{
	free_page((unsigned long)topa);
}

/**
 * topa_insert_table() - insert a ToPA table into a buffer
 * @buf:	 PT buffer that's being extended.
 * @topa:	 New topa table to be inserted.
 *
 * If it's the first table in this buffer, set up buffer's pointers
 * accordingly; otherwise, add a END=1 link entry to @topa to the current
 * "last" table and adjust the last table pointer to @topa.
 */
static void topa_insert_table(struct pt_buffer *buf, struct topa *topa)
{
	struct topa *last = buf->last;

	list_add_tail(&topa->list, &buf->tables);

	if (!buf->first) {
		buf->first = buf->last = buf->cur = topa;
		return;
	}

	topa->offset = last->offset + last->size;
	buf->last = topa;

	if (!pt_cap_get(PT_CAP_topa_multiple_entries))
		return;

	BUG_ON(last->last != TENTS_PER_PAGE - 1);

	TOPA_ENTRY(last, -1)->base = topa->phys >> TOPA_SHIFT;
	TOPA_ENTRY(last, -1)->end = 1;
}

/**
 * topa_table_full() - check if a ToPA table is filled up
 * @topa:	ToPA table.
 */
static bool topa_table_full(struct topa *topa)
{
	/* single-entry ToPA is a special case */
	if (!pt_cap_get(PT_CAP_topa_multiple_entries))
		return !!topa->last;

	return topa->last == TENTS_PER_PAGE - 1;
}

/**
 * topa_insert_pages() - create a list of ToPA tables
 * @buf:	PT buffer being initialized.
 * @gfp:	Allocation flags.
 *
 * This initializes a list of ToPA tables with entries from
 * the data_pages provided by rb_alloc_aux().
 *
 * Return:	0 on success or error code.
 */
static int topa_insert_pages(struct pt_buffer *buf, gfp_t gfp)
{
	struct topa *topa = buf->last;
	int order = 0;
	struct page *p;

	p = virt_to_page(buf->data_pages[buf->nr_pages]);
	if (PagePrivate(p))
		order = page_private(p);

	if (topa_table_full(topa)) {
		topa = topa_alloc(buf->cpu, gfp);
		if (!topa)
			return -ENOMEM;

		topa_insert_table(buf, topa);
	}

	TOPA_ENTRY(topa, -1)->base = page_to_phys(p) >> TOPA_SHIFT;
	TOPA_ENTRY(topa, -1)->size = order;
	if (!buf->snapshot && !pt_cap_get(PT_CAP_topa_multiple_entries)) {
		TOPA_ENTRY(topa, -1)->intr = 1;
		TOPA_ENTRY(topa, -1)->stop = 1;
	}

	topa->last++;
	topa->size += sizes(order);

	buf->nr_pages += 1ul << order;

	return 0;
}

/**
 * pt_topa_dump() - print ToPA tables and their entries
 * @buf:	PT buffer.
 */
static void pt_topa_dump(struct pt_buffer *buf)
{
	struct topa *topa;

	list_for_each_entry(topa, &buf->tables, list) {
		int i;

		pr_debug("# table @%p (%016Lx), off %llx size %zx\n", topa->table,
			 topa->phys, topa->offset, topa->size);
		for (i = 0; i < TENTS_PER_PAGE; i++) {
			pr_debug("# entry @%p (%lx sz %u %c%c%c) raw=%16llx\n",
				 &topa->table[i],
				 (unsigned long)topa->table[i].base << TOPA_SHIFT,
				 sizes(topa->table[i].size),
				 topa->table[i].end ?  'E' : ' ',
				 topa->table[i].intr ? 'I' : ' ',
				 topa->table[i].stop ? 'S' : ' ',
				 *(u64 *)&topa->table[i]);
			if ((pt_cap_get(PT_CAP_topa_multiple_entries) &&
			     topa->table[i].stop) ||
			    topa->table[i].end)
				break;
		}
	}
}

/**
 * pt_buffer_advance() - advance to the next output region
 * @buf:	PT buffer.
 *
 * Advance the current pointers in the buffer to the next ToPA entry.
 */
static void pt_buffer_advance(struct pt_buffer *buf)
{
	buf->output_off = 0;
	buf->cur_idx++;

	if (buf->cur_idx == buf->cur->last) {
		if (buf->cur == buf->last)
			buf->cur = buf->first;
		else
			buf->cur = list_entry(buf->cur->list.next, struct topa,
					      list);
		buf->cur_idx = 0;
	}
}

/**
 * pt_update_head() - calculate current offsets and sizes
 * @pt:		Per-cpu pt context.
 *
 * Update buffer's current write pointer position and data size.
 */
static void pt_update_head(struct pt *pt)
{
	struct pt_buffer *buf = perf_get_aux(&pt->handle);
	u64 topa_idx, base, old;

	/* offset of the first region in this table from the beginning of buf */
	base = buf->cur->offset + buf->output_off;

	/* offset of the current output region within this table */
	for (topa_idx = 0; topa_idx < buf->cur_idx; topa_idx++)
		base += sizes(buf->cur->table[topa_idx].size);

	if (buf->snapshot) {
		local_set(&buf->data_size, base);
	} else {
		old = (local64_xchg(&buf->head, base) &
		       ((buf->nr_pages << PAGE_SHIFT) - 1));
		if (base < old)
			base += buf->nr_pages << PAGE_SHIFT;

		local_add(base - old, &buf->data_size);
	}
}

/**
 * pt_buffer_region() - obtain current output region's address
 * @buf:	PT buffer.
 */
static void *pt_buffer_region(struct pt_buffer *buf)
{
	return phys_to_virt(buf->cur->table[buf->cur_idx].base << TOPA_SHIFT);
}

/**
 * pt_buffer_region_size() - obtain current output region's size
 * @buf:	PT buffer.
 */
static size_t pt_buffer_region_size(struct pt_buffer *buf)
{
	return sizes(buf->cur->table[buf->cur_idx].size);
}

/**
 * pt_handle_status() - take care of possible status conditions
 * @pt:		Per-cpu pt context.
 */
static void pt_handle_status(struct pt *pt)
{
	struct pt_buffer *buf = perf_get_aux(&pt->handle);
	int advance = 0;
	u64 status;

	rdmsrl(MSR_IA32_RTIT_STATUS, status);

	if (status & RTIT_STATUS_ERROR) {
		pr_err_ratelimited("ToPA ERROR encountered, trying to recover\n");
		pt_topa_dump(buf);
		status &= ~RTIT_STATUS_ERROR;
	}

	if (status & RTIT_STATUS_STOPPED) {
		status &= ~RTIT_STATUS_STOPPED;

		/*
		 * On systems that only do single-entry ToPA, hitting STOP
		 * means we are already losing data; need to let the decoder
		 * know.
		 */
		if (!pt_cap_get(PT_CAP_topa_multiple_entries) ||
		    buf->output_off == sizes(TOPA_ENTRY(buf->cur, buf->cur_idx)->size)) {
			local_inc(&buf->lost);
			advance++;
		}
	}

	/*
	 * Also on single-entry ToPA implementations, interrupt will come
	 * before the output reaches its output region's boundary.
	 */
	if (!pt_cap_get(PT_CAP_topa_multiple_entries) && !buf->snapshot &&
	    pt_buffer_region_size(buf) - buf->output_off <= TOPA_PMI_MARGIN) {
		void *head = pt_buffer_region(buf);

		/* everything within this margin needs to be zeroed out */
		memset(head + buf->output_off, 0,
		       pt_buffer_region_size(buf) -
		       buf->output_off);
		advance++;
	}

	if (advance)
		pt_buffer_advance(buf);

	wrmsrl(MSR_IA32_RTIT_STATUS, status);
}

/**
 * pt_read_offset() - translate registers into buffer pointers
 * @buf:	PT buffer.
 *
 * Set buffer's output pointers from MSR values.
 */
static void pt_read_offset(struct pt_buffer *buf)
{
	u64 offset, base_topa;

	rdmsrl(MSR_IA32_RTIT_OUTPUT_BASE, base_topa);
	buf->cur = phys_to_virt(base_topa);

	rdmsrl(MSR_IA32_RTIT_OUTPUT_MASK, offset);
	/* offset within current output region */
	buf->output_off = offset >> 32;
	/* index of current output region within this table */
	buf->cur_idx = (offset & 0xffffff80) >> 7;
}

/**
 * pt_topa_next_entry() - obtain index of the first page in the next ToPA entry
 * @buf:	PT buffer.
 * @pg:		Page offset in the buffer.
 *
 * When advancing to the next output region (ToPA entry), given a page offset
 * into the buffer, we need to find the offset of the first page in the next
 * region.
 */
static unsigned int pt_topa_next_entry(struct pt_buffer *buf, unsigned int pg)
{
	struct topa_entry *te = buf->topa_index[pg];

	/* one region */
	if (buf->first == buf->last && buf->first->last == 1)
		return pg;

	do {
		pg++;
		pg &= buf->nr_pages - 1;
	} while (buf->topa_index[pg] == te);

	return pg;
}

/**
 * pt_buffer_reset_markers() - place interrupt and stop bits in the buffer
 * @buf:	PT buffer.
 * @handle:	Current output handle.
 *
 * Place INT and STOP marks to prevent overwriting old data that the consumer
 * hasn't yet collected.
 */
static int pt_buffer_reset_markers(struct pt_buffer *buf,
				   struct perf_output_handle *handle)

{
	unsigned long head = local64_read(&buf->head);
	unsigned long idx, npages, wakeup;

	if (buf->snapshot)
		return 0;

	/* can't stop in the middle of an output region */
	if (buf->output_off + handle->size + 1 <
	    sizes(TOPA_ENTRY(buf->cur, buf->cur_idx)->size))
		return -EINVAL;


	/* single entry ToPA is handled by marking all regions STOP=1 INT=1 */
	if (!pt_cap_get(PT_CAP_topa_multiple_entries))
		return 0;

	/* clear STOP and INT from current entry */
	buf->topa_index[buf->stop_pos]->stop = 0;
	buf->topa_index[buf->stop_pos]->intr = 0;
	buf->topa_index[buf->intr_pos]->intr = 0;

	/* how many pages till the STOP marker */
	npages = handle->size >> PAGE_SHIFT;

	/* if it's on a page boundary, fill up one more page */
	if (!offset_in_page(head + handle->size + 1))
		npages++;

	idx = (head >> PAGE_SHIFT) + npages;
	idx &= buf->nr_pages - 1;
	buf->stop_pos = idx;

	wakeup = handle->wakeup >> PAGE_SHIFT;

	/* in the worst case, wake up the consumer one page before hard stop */
	idx = (head >> PAGE_SHIFT) + npages - 1;
	if (idx > wakeup)
		idx = wakeup;

	idx &= buf->nr_pages - 1;
	buf->intr_pos = idx;

	buf->topa_index[buf->stop_pos]->stop = 1;
	buf->topa_index[buf->stop_pos]->intr = 1;
	buf->topa_index[buf->intr_pos]->intr = 1;

	return 0;
}

/**
 * pt_buffer_setup_topa_index() - build topa_index[] table of regions
 * @buf:	PT buffer.
 *
 * topa_index[] references output regions indexed by offset into the
 * buffer for purposes of quick reverse lookup.
 */
static void pt_buffer_setup_topa_index(struct pt_buffer *buf)
{
	struct topa *cur = buf->first, *prev = buf->last;
	struct topa_entry *te_cur = TOPA_ENTRY(cur, 0),
		*te_prev = TOPA_ENTRY(prev, prev->last - 1);
	int pg = 0, idx = 0, ntopa = 0;

	while (pg < buf->nr_pages) {
		int tidx;

		/* pages within one topa entry */
		for (tidx = 0; tidx < 1 << te_cur->size; tidx++, pg++)
			buf->topa_index[pg] = te_prev;

		te_prev = te_cur;

		if (idx == cur->last - 1) {
			/* advance to next topa table */
			idx = 0;
			cur = list_entry(cur->list.next, struct topa, list);
			ntopa++;
		} else
			idx++;
		te_cur = TOPA_ENTRY(cur, idx);
	}

}

/**
 * pt_buffer_reset_offsets() - adjust buffer's write pointers from aux_head
 * @buf:	PT buffer.
 * @head:	Write pointer (aux_head) from AUX buffer.
 *
 * Find the ToPA table and entry corresponding to given @head and set buffer's
 * "current" pointers accordingly.
 */
static void pt_buffer_reset_offsets(struct pt_buffer *buf, unsigned long head)
{
	int pg;

	if (buf->snapshot)
		head &= (buf->nr_pages << PAGE_SHIFT) - 1;

	pg = (head >> PAGE_SHIFT) & (buf->nr_pages - 1);
	pg = pt_topa_next_entry(buf, pg);

	buf->cur = (struct topa *)((unsigned long)buf->topa_index[pg] & PAGE_MASK);
	buf->cur_idx = ((unsigned long)buf->topa_index[pg] -
			(unsigned long)buf->cur) / sizeof(struct topa_entry);
	buf->output_off = head & (sizes(buf->cur->table[buf->cur_idx].size) - 1);

	local64_set(&buf->head, head);
	local_set(&buf->data_size, 0);
}

/**
 * pt_buffer_fini_topa() - deallocate ToPA structure of a buffer
 * @buf:	PT buffer.
 */
static void pt_buffer_fini_topa(struct pt_buffer *buf)
{
	struct topa *topa, *iter;

	list_for_each_entry_safe(topa, iter, &buf->tables, list) {
		/*
		 * right now, this is in free_aux() path only, so
		 * no need to unlink this table from the list
		 */
		topa_free(topa);
	}
}

/**
 * pt_buffer_init_topa() - initialize ToPA table for pt buffer
 * @buf:	PT buffer.
 * @size:	Total size of all regions within this ToPA.
 * @gfp:	Allocation flags.
 */
static int pt_buffer_init_topa(struct pt_buffer *buf, unsigned long nr_pages,
			       gfp_t gfp)
{
	struct topa *topa;
	int err;

	topa = topa_alloc(buf->cpu, gfp);
	if (!topa)
		return -ENOMEM;

	topa_insert_table(buf, topa);

	while (buf->nr_pages < nr_pages) {
		err = topa_insert_pages(buf, gfp);
		if (err) {
			pt_buffer_fini_topa(buf);
			return -ENOMEM;
		}
	}

	pt_buffer_setup_topa_index(buf);

	/* link last table to the first one, unless we're double buffering */
	if (pt_cap_get(PT_CAP_topa_multiple_entries)) {
		TOPA_ENTRY(buf->last, -1)->base = buf->first->phys >> TOPA_SHIFT;
		TOPA_ENTRY(buf->last, -1)->end = 1;
	}

	pt_topa_dump(buf);
	return 0;
}

/**
 * pt_buffer_setup_aux() - set up topa tables for a PT buffer
 * @cpu:	Cpu on which to allocate, -1 means current.
 * @pages:	Array of pointers to buffer pages passed from perf core.
 * @nr_pages:	Number of pages in the buffer.
 * @snapshot:	If this is a snapshot/overwrite counter.
 *
 * This is a pmu::setup_aux callback that sets up ToPA tables and all the
 * bookkeeping for an AUX buffer.
 *
 * Return:	Our private PT buffer structure.
 */
static void *
pt_buffer_setup_aux(int cpu, void **pages, int nr_pages, bool snapshot)
{
	struct pt_buffer *buf;
	int node, ret;

	if (!nr_pages)
		return NULL;

	if (cpu == -1)
		cpu = raw_smp_processor_id();
	node = cpu_to_node(cpu);

	buf = kzalloc_node(offsetof(struct pt_buffer, topa_index[nr_pages]),
			   GFP_KERNEL, node);
	if (!buf)
		return NULL;

	buf->cpu = cpu;
	buf->snapshot = snapshot;
	buf->data_pages = pages;

	INIT_LIST_HEAD(&buf->tables);

	ret = pt_buffer_init_topa(buf, nr_pages, GFP_KERNEL);
	if (ret) {
		kfree(buf);
		return NULL;
	}

	return buf;
}

/**
 * pt_buffer_free_aux() - perf AUX deallocation path callback
 * @data:	PT buffer.
 */
static void pt_buffer_free_aux(void *data)
{
	struct pt_buffer *buf = data;

	pt_buffer_fini_topa(buf);
	kfree(buf);
}

/**
 * pt_buffer_is_full() - check if the buffer is full
 * @buf:	PT buffer.
 * @pt:		Per-cpu pt handle.
 *
 * If the user hasn't read data from the output region that aux_head
 * points to, the buffer is considered full: the user needs to read at
 * least this region and update aux_tail to point past it.
 */
static bool pt_buffer_is_full(struct pt_buffer *buf, struct pt *pt)
{
	if (buf->snapshot)
		return false;

	if (local_read(&buf->data_size) >= pt->handle.size)
		return true;

	return false;
}

/**
 * intel_pt_interrupt() - PT PMI handler
 */
void intel_pt_interrupt(void)
{
	struct pt *pt = this_cpu_ptr(&pt_ctx);
	struct pt_buffer *buf;
	struct perf_event *event = pt->handle.event;

	/*
	 * There may be a dangling PT bit in the interrupt status register
	 * after PT has been disabled by pt_event_stop(). Make sure we don't
	 * do anything (particularly, re-enable) for this event here.
	 */
	if (!ACCESS_ONCE(pt->handle_nmi))
		return;

	pt_config_start(false);

	if (!event)
		return;

	buf = perf_get_aux(&pt->handle);
	if (!buf)
		return;

	pt_read_offset(buf);

	pt_handle_status(pt);

	pt_update_head(pt);

	perf_aux_output_end(&pt->handle, local_xchg(&buf->data_size, 0),
			    local_xchg(&buf->lost, 0));

	if (!event->hw.state) {
		int ret;

		buf = perf_aux_output_begin(&pt->handle, event);
		if (!buf) {
			event->hw.state = PERF_HES_STOPPED;
			return;
		}

		pt_buffer_reset_offsets(buf, pt->handle.head);
		ret = pt_buffer_reset_markers(buf, &pt->handle);
		if (ret) {
			perf_aux_output_end(&pt->handle, 0, true);
			return;
		}

		pt_config_buffer(buf->cur->table, buf->cur_idx,
				 buf->output_off);
		wrmsrl(MSR_IA32_RTIT_STATUS, 0);
		pt_config(event);
	}
}

/*
 * PMU callbacks
 */

static void pt_event_start(struct perf_event *event, int mode)
{
	struct pt *pt = this_cpu_ptr(&pt_ctx);
	struct pt_buffer *buf = perf_get_aux(&pt->handle);

	if (pt_is_running() || !buf || pt_buffer_is_full(buf, pt)) {
		event->hw.state = PERF_HES_STOPPED;
		return;
	}

	ACCESS_ONCE(pt->handle_nmi) = 1;
	event->hw.state = 0;

	pt_config_buffer(buf->cur->table, buf->cur_idx,
			 buf->output_off);
	wrmsrl(MSR_IA32_RTIT_STATUS, 0);
	pt_config(event);
}

static void pt_event_stop(struct perf_event *event, int mode)
{
	struct pt *pt = this_cpu_ptr(&pt_ctx);

	/*
	 * Protect against the PMI racing with disabling wrmsr,
	 * see comment in intel_pt_interrupt().
	 */
	ACCESS_ONCE(pt->handle_nmi) = 0;
	pt_config_start(false);

	if (event->hw.state == PERF_HES_STOPPED)
		return;

	event->hw.state = PERF_HES_STOPPED;

	if (mode & PERF_EF_UPDATE) {
		struct pt *pt = this_cpu_ptr(&pt_ctx);
		struct pt_buffer *buf = perf_get_aux(&pt->handle);

		if (!buf)
			return;

		if (WARN_ON_ONCE(pt->handle.event != event))
			return;

		pt_read_offset(buf);

		pt_handle_status(pt);

		pt_update_head(pt);
	}
}

static void pt_event_del(struct perf_event *event, int mode)
{
	struct pt *pt = this_cpu_ptr(&pt_ctx);
	struct pt_buffer *buf;

	pt_event_stop(event, PERF_EF_UPDATE);

	buf = perf_get_aux(&pt->handle);

	if (buf) {
		if (buf->snapshot)
			pt->handle.head =
				local_xchg(&buf->data_size,
					   buf->nr_pages << PAGE_SHIFT);
		perf_aux_output_end(&pt->handle, local_xchg(&buf->data_size, 0),
				    local_xchg(&buf->lost, 0));
	}
}

static int pt_event_add(struct perf_event *event, int mode)
{
	struct pt_buffer *buf;
	struct pt *pt = this_cpu_ptr(&pt_ctx);
	struct hw_perf_event *hwc = &event->hw;
	int ret = -EBUSY;

	if (pt->handle.event)
		goto fail;

	buf = perf_aux_output_begin(&pt->handle, event);
	ret = -EINVAL;
	if (!buf)
		goto fail_stop;

	pt_buffer_reset_offsets(buf, pt->handle.head);
	if (!buf->snapshot) {
		ret = pt_buffer_reset_markers(buf, &pt->handle);
		if (ret)
			goto fail_end_stop;
	}

	if (mode & PERF_EF_START) {
		pt_event_start(event, 0);
		ret = -EBUSY;
		if (hwc->state == PERF_HES_STOPPED)
			goto fail_end_stop;
	} else {
		hwc->state = PERF_HES_STOPPED;
	}

	return 0;

fail_end_stop:
	perf_aux_output_end(&pt->handle, 0, true);
fail_stop:
	hwc->state = PERF_HES_STOPPED;
fail:
	return ret;
}

static void pt_event_read(struct perf_event *event)
{
}

static void pt_event_destroy(struct perf_event *event)
{
	x86_del_exclusive(x86_lbr_exclusive_pt);
}

static int pt_event_init(struct perf_event *event)
{
	if (event->attr.type != pt_pmu.pmu.type)
		return -ENOENT;

	if (!pt_event_valid(event))
		return -EINVAL;

	if (x86_add_exclusive(x86_lbr_exclusive_pt))
		return -EBUSY;

	event->destroy = pt_event_destroy;

	return 0;
}

static __init int pt_init(void)
{
	int ret, cpu, prior_warn = 0;

	BUILD_BUG_ON(sizeof(struct topa) > PAGE_SIZE);
	get_online_cpus();
	for_each_online_cpu(cpu) {
		u64 ctl;

		ret = rdmsrl_safe_on_cpu(cpu, MSR_IA32_RTIT_CTL, &ctl);
		if (!ret && (ctl & RTIT_CTL_TRACEEN))
			prior_warn++;
	}
	put_online_cpus();

	if (prior_warn) {
		x86_add_exclusive(x86_lbr_exclusive_pt);
		pr_warn("PT is enabled at boot time, doing nothing\n");

		return -EBUSY;
	}

	ret = pt_pmu_hw_init();
	if (ret)
		return ret;

	if (!pt_cap_get(PT_CAP_topa_output)) {
		pr_warn("ToPA output is not supported on this CPU\n");
		return -ENODEV;
	}

	if (!pt_cap_get(PT_CAP_topa_multiple_entries))
		pt_pmu.pmu.capabilities =
			PERF_PMU_CAP_AUX_NO_SG | PERF_PMU_CAP_AUX_SW_DOUBLEBUF;

	pt_pmu.pmu.capabilities	|= PERF_PMU_CAP_EXCLUSIVE | PERF_PMU_CAP_ITRACE;
	pt_pmu.pmu.attr_groups	= pt_attr_groups;
	pt_pmu.pmu.task_ctx_nr	= perf_sw_context;
	pt_pmu.pmu.event_init	= pt_event_init;
	pt_pmu.pmu.add		= pt_event_add;
	pt_pmu.pmu.del		= pt_event_del;
	pt_pmu.pmu.start	= pt_event_start;
	pt_pmu.pmu.stop		= pt_event_stop;
	pt_pmu.pmu.read		= pt_event_read;
	pt_pmu.pmu.setup_aux	= pt_buffer_setup_aux;
	pt_pmu.pmu.free_aux	= pt_buffer_free_aux;
	ret = perf_pmu_register(&pt_pmu.pmu, "intel_pt", -1);

	return ret;
}

module_init(pt_init);
