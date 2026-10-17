// SPDX-License-Identifier: GPL-2.0
/*
 * (C) 2006 - Cambridge University
 * (C) 2020 - EPAM Systems Inc.
 *
 * File: gnttab.c [1]
 * Author: Steven Smith (sos22@cam.ac.uk)
 * Changes: Grzegorz Milos (gm281@cam.ac.uk)
 *
 * Date: July 2006
 *
 * Description: Simple grant tables implementation. About as stupid as it's
 * possible to be and still work.
 *
 * [1] - http://xenbits.xen.org/gitweb/?p=mini-os.git;a=summary
 */
#include <common.h>
#include <asm/global_data.h>
#include <linux/compiler.h>
#include <log.h>
#include <malloc.h>

#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/xen/system.h>

#include <linux/bug.h>

#include <xen/gnttab.h>
#include <xen/hvm.h>

#include <xen/interface/memory.h>

DECLARE_GLOBAL_DATA_PTR;

#define NR_RESERVED_ENTRIES 8

/* NR_GRANT_FRAMES must be less than or equal to that configured in Xen */
#define NR_GRANT_FRAMES 1
#define NR_GRANT_ENTRIES (NR_GRANT_FRAMES * PAGE_SIZE / sizeof(struct grant_entry_v1))

static struct grant_entry_v1 *gnttab_table;
static grant_ref_t gnttab_list[NR_GRANT_ENTRIES];

static void put_free_entry(grant_ref_t ref)
{
	unsigned long flags;

	local_irq_save(flags);
	gnttab_list[ref] = gnttab_list[0];
	gnttab_list[0]  = ref;
	local_irq_restore(flags);
}

static grant_ref_t get_free_entry(void)
{
	unsigned int ref;
	unsigned long flags;

	local_irq_save(flags);
	ref = gnttab_list[0];
	BUG_ON(ref < NR_RESERVED_ENTRIES || ref >= NR_GRANT_ENTRIES);
	gnttab_list[0] = gnttab_list[ref];
	local_irq_restore(flags);
	return ref;
}

/**
 * gnttab_grant_access() - Allow access to the given frame.
 * The function creates an entry in the grant table according
 * to the specified parameters.
 * @domid: the id of the domain for which access is allowed
 * @frame: the number of the shared frame
 * @readonly: determines whether the frame is shared read-only or read-write
 *
 * Return: relevant grant reference
 */
grant_ref_t gnttab_grant_access(domid_t domid, unsigned long frame, int readonly)
{
	grant_ref_t ref;

	ref = get_free_entry();
	gnttab_table[ref].frame = frame;
	gnttab_table[ref].domid = domid;
	wmb();
	readonly *= GTF_readonly;
	gnttab_table[ref].flags = GTF_permit_access | readonly;

	return ref;
}

/**
 * gnttab_end_access() - End of memory sharing. The function invalidates
 * the entry in the grant table.
 */
int gnttab_end_access(grant_ref_t ref)
{
	u16 flags, nflags;

	BUG_ON(ref >= NR_GRANT_ENTRIES || ref < NR_RESERVED_ENTRIES);

	nflags = gnttab_table[ref].flags;
	do {
		flags = nflags;
		if ((flags) & (GTF_reading | GTF_writing)) {
			printf("WARNING: g.e. still in use! (%x)\n", flags);
			return 0;
		}
	} while ((nflags = synch_cmpxchg(&gnttab_table[ref].flags, flags, 0)) !=
		 flags);

	put_free_entry(ref);
	return 1;
}

grant_ref_t gnttab_alloc_and_grant(void **map)
{
	unsigned long mfn;
	grant_ref_t gref;

	*map = (void *)memalign(PAGE_SIZE, PAGE_SIZE);
	mfn = virt_to_mfn(*map);
	gref = gnttab_grant_access(0, mfn, 0);
	return gref;
}

static const char * const gnttabop_error_msgs[] = GNTTABOP_error_msgs;

const char *gnttabop_error(int16_t status)
{
	status = -status;
	if (status < 0 || status >= ARRAY_SIZE(gnttabop_error_msgs))
		return "bad status";
	else
		return gnttabop_error_msgs[status];
}

/* Get Xen's suggested physical page assignments for the grant table. */
void get_gnttab_base(phys_addr_t *gnttab_base, phys_size_t *gnttab_sz)
{
	const void *blob = gd->fdt_blob;
	struct fdt_resource res;
	int mem;

	mem = fdt_node_offset_by_compatible(blob, -1, "xen,xen");
	if (mem < 0) {
		printf("No xen,xen compatible found\n");
		BUG();
	}

	mem = fdt_get_resource(blob, mem, "reg", 0, &res);
	if (mem == -FDT_ERR_NOTFOUND) {
		printf("No grant table base in the device tree\n");
		BUG();
	}

	*gnttab_base = (phys_addr_t)res.start;
	if (gnttab_sz)
		*gnttab_sz = (phys_size_t)(res.end - res.start + 1);

	debug("FDT suggests grant table base at %llx\n",
	      *gnttab_base);
}

void init_gnttab(void)
{
	struct xen_add_to_physmap xatp;
	struct gnttab_setup_table setup;
	xen_pfn_t frames[NR_GRANT_FRAMES];
	int i, rc;

	debug("%s\n", __func__);

	for (i = NR_RESERVED_ENTRIES; i < NR_GRANT_ENTRIES; i++)
		put_free_entry(i);

	get_gnttab_base((phys_addr_t *)&gnttab_table, NULL);

	for (i = 0; i < NR_GRANT_FRAMES; i++) {
		xatp.domid = DOMID_SELF;
		xatp.size = 0;
		xatp.space = XENMAPSPACE_grant_table;
		xatp.idx = i;
		xatp.gpfn = PFN_DOWN((unsigned long)gnttab_table) + i;
		rc = HYPERVISOR_memory_op(XENMEM_add_to_physmap, &xatp);
		if (rc)
			printf("XENMEM_add_to_physmap failed; status = %d\n",
			       rc);
		BUG_ON(rc != 0);
	}

	setup.dom = DOMID_SELF;
	setup.nr_frames = NR_GRANT_FRAMES;
	set_xen_guest_handle(setup.frame_list, frames);
}

void fini_gnttab(void)
{
	struct xen_remove_from_physmap xrtp;
	struct gnttab_setup_table setup;
	int i, rc;

	debug("%s\n", __func__);

	for (i = 0; i < NR_GRANT_FRAMES; i++) {
		xrtp.domid = DOMID_SELF;
		xrtp.gpfn = PFN_DOWN((unsigned long)gnttab_table) + i;
		rc = HYPERVISOR_memory_op(XENMEM_remove_from_physmap, &xrtp);
		if (rc)
			printf("XENMEM_remove_from_physmap failed; status = %d\n",
			       rc);
		BUG_ON(rc != 0);
	}

	setup.dom = DOMID_SELF;
	setup.nr_frames = 0;
}
