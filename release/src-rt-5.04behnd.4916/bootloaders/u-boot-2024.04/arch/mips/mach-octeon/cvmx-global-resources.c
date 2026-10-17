// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 */

#include <log.h>
#include <time.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-coremask.h>
#include <mach/cvmx-range.h>

#include <mach/cvmx-global-resources.h>
#include <mach/cvmx-bootmem.h>

#include <mach/cvmx-pki.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

#define CVMX_MAX_GLOBAL_RESOURCES 128
#define CVMX_RESOURCES_ENTRIES_SIZE                                            \
	(sizeof(struct cvmx_global_resource_entry) * CVMX_MAX_GLOBAL_RESOURCES)

/**
 * This macro returns a member of the data
 * structure. The argument "field" is the member name of the
 * structure to read. The return type is a u64.
 */
#define CVMX_GLOBAL_RESOURCES_GET_FIELD(field)                                 \
	__cvmx_struct_get_unsigned_field(                                      \
		__cvmx_global_resources_addr,                                  \
		offsetof(struct cvmx_global_resources, field),                 \
		SIZEOF_FIELD(struct cvmx_global_resources, field))

/**
 * This macro writes a member of the struct cvmx_global_resourcest
 * structure. The argument "field" is the member name of the
 * struct cvmx_global_resources to write.
 */
#define CVMX_GLOBAL_RESOURCES_SET_FIELD(field, value)                          \
	__cvmx_struct_set_unsigned_field(                                      \
		__cvmx_global_resources_addr,                                  \
		offsetof(struct cvmx_global_resources, field),                 \
		SIZEOF_FIELD(struct cvmx_global_resources, field), value)

/**
 * This macro returns a member of the struct cvmx_global_resource_entry.
 * The argument "field" is the member name of this structure.
 * the return type is a u64. The "addr" parameter is the physical
 * address of the structure.
 */
#define CVMX_RESOURCE_ENTRY_GET_FIELD(addr, field)                             \
	__cvmx_struct_get_unsigned_field(                                      \
		addr, offsetof(struct cvmx_global_resource_entry, field),      \
		SIZEOF_FIELD(struct cvmx_global_resource_entry, field))

/**
 * This macro writes a member of the struct cvmx_global_resource_entry
 * structure. The argument "field" is the member name of the
 * struct cvmx_global_resource_entry to write. The "addr" parameter
 * is the physical address of the structure.
 */
#define CVMX_RESOURCE_ENTRY_SET_FIELD(addr, field, value)                      \
	__cvmx_struct_set_unsigned_field(                                      \
		addr, offsetof(struct cvmx_global_resource_entry, field),      \
		SIZEOF_FIELD(struct cvmx_global_resource_entry, field), value)

#define CVMX_GET_RESOURCE_ENTRY(count)                                         \
	(__cvmx_global_resources_addr +                                        \
	 offsetof(struct cvmx_global_resources, resource_entry) +              \
	 (count * sizeof(struct cvmx_global_resource_entry)))

#define CVMX_RESOURCE_TAG_SET_FIELD(addr, field, value)                        \
	__cvmx_struct_set_unsigned_field(                                      \
		addr, offsetof(struct global_resource_tag, field),             \
		SIZEOF_FIELD(struct global_resource_tag, field), value)

#define CVMX_RESOURCE_TAG_GET_FIELD(addr, field)                               \
	__cvmx_struct_get_unsigned_field(                                      \
		addr, offsetof(struct global_resource_tag, field),             \
		SIZEOF_FIELD(struct global_resource_tag, field))

#define MAX_RESOURCE_TAG_LEN		16
#define CVMX_GLOBAL_RESOURCE_NO_LOCKING (1)

struct cvmx_global_resource_entry {
	struct global_resource_tag tag;
	u64 phys_addr;
	u64 size;
};

struct cvmx_global_resources {
	u32 pad;
	u32 rlock;
	u64 entry_cnt;
	struct cvmx_global_resource_entry resource_entry[];
};

/* Not the right place, putting it here for now */
u64 cvmx_app_id;

/*
 * Global named memory can be accessed anywhere even in 32-bit mode
 */
static u64 __cvmx_global_resources_addr;

/**
 * This macro returns the size of a member of a structure.
 */
#define SIZEOF_FIELD(s, field) sizeof(((s *)NULL)->field)

/**
 * This function is the implementation of the get macros defined
 * for individual structure members. The argument are generated
 * by the macros inorder to read only the needed memory.
 *
 * @param base   64bit physical address of the complete structure
 * @param offset from the beginning of the structure to the member being
 *               accessed.
 * @param size   Size of the structure member.
 *
 * @return Value of the structure member promoted into a u64.
 */
static inline u64 __cvmx_struct_get_unsigned_field(u64 base, int offset,
						   int size)
{
	base = (1ull << 63) | (base + offset);
	switch (size) {
	case 4:
		return cvmx_read64_uint32(base);
	case 8:
		return cvmx_read64_uint64(base);
	default:
		return 0;
	}
}

/**
 * This function is the implementation of the set macros defined
 * for individual structure members. The argument are generated
 * by the macros in order to write only the needed memory.
 *
 * @param base   64bit physical address of the complete structure
 * @param offset from the beginning of the structure to the member being
 *               accessed.
 * @param size   Size of the structure member.
 * @param value  Value to write into the structure
 */
static inline void __cvmx_struct_set_unsigned_field(u64 base, int offset,
						    int size, u64 value)
{
	base = (1ull << 63) | (base + offset);
	switch (size) {
	case 4:
		cvmx_write64_uint32(base, value);
		break;
	case 8:
		cvmx_write64_uint64(base, value);
		break;
	default:
		break;
	}
}

/* Get the global resource lock. */
static inline void __cvmx_global_resource_lock(void)
{
	u64 lock_addr =
		(1ull << 63) | (__cvmx_global_resources_addr +
				offsetof(struct cvmx_global_resources, rlock));
	unsigned int tmp;

	__asm__ __volatile__(".set noreorder\n"
			     "1: ll   %[tmp], 0(%[addr])\n"
			     "   bnez %[tmp], 1b\n"
			     "   li   %[tmp], 1\n"
			     "   sc   %[tmp], 0(%[addr])\n"
			     "   beqz %[tmp], 1b\n"
			     "   nop\n"
			     ".set reorder\n"
			     : [tmp] "=&r"(tmp)
			     : [addr] "r"(lock_addr)
			     : "memory");
}

/* Release the global resource lock. */
static inline void __cvmx_global_resource_unlock(void)
{
	u64 lock_addr =
		(1ull << 63) | (__cvmx_global_resources_addr +
				offsetof(struct cvmx_global_resources, rlock));
	CVMX_SYNCW;
	__asm__ __volatile__("sw $0, 0(%[addr])\n"
			     :
			     : [addr] "r"(lock_addr)
			     : "memory");
	CVMX_SYNCW;
}

static u64 __cvmx_alloc_bootmem_for_global_resources(int sz)
{
	void *tmp;

	tmp = cvmx_bootmem_alloc_range(sz, CVMX_CACHE_LINE_SIZE, 0, 0);
	return cvmx_ptr_to_phys(tmp);
}

static inline void __cvmx_get_tagname(struct global_resource_tag *rtag,
				      char *tagname)
{
	int i, j, k;

	j = 0;
	k = 8;
	for (i = 7; i >= 0; i--, j++, k++) {
		tagname[j] = (rtag->lo >> (i * 8)) & 0xff;
		tagname[k] = (rtag->hi >> (i * 8)) & 0xff;
	}
}

static u64 __cvmx_global_resources_init(void)
{
	struct cvmx_bootmem_named_block_desc *block_desc;
	int sz = sizeof(struct cvmx_global_resources) +
		 CVMX_RESOURCES_ENTRIES_SIZE;
	s64 tmp_phys;
	int count = 0;
	u64 base = 0;

	cvmx_bootmem_lock();

	block_desc = (struct cvmx_bootmem_named_block_desc *)
		__cvmx_bootmem_find_named_block_flags(
			CVMX_GLOBAL_RESOURCES_DATA_NAME,
			CVMX_BOOTMEM_FLAG_NO_LOCKING);
	if (!block_desc) {
		debug("%s: allocating global resources\n", __func__);

		tmp_phys = cvmx_bootmem_phy_named_block_alloc(
			sz, 0, 0, CVMX_CACHE_LINE_SIZE,
			CVMX_GLOBAL_RESOURCES_DATA_NAME,
			CVMX_BOOTMEM_FLAG_NO_LOCKING);
		if (tmp_phys < 0) {
			cvmx_printf(
				"ERROR: %s: failed to allocate global resource name block. sz=%d\n",
				__func__, sz);
			goto end;
		}
		__cvmx_global_resources_addr = (u64)tmp_phys;

		debug("%s: memset global resources %llu\n", __func__,
		      CAST_ULL(__cvmx_global_resources_addr));

		base = (1ull << 63) | __cvmx_global_resources_addr;
		for (count = 0; count < (sz / 8); count++) {
			cvmx_write64_uint64(base, 0);
			base += 8;
		}
	} else {
		debug("%s:found global resource\n", __func__);
		__cvmx_global_resources_addr = block_desc->base_addr;
	}
end:
	cvmx_bootmem_unlock();
	debug("__cvmx_global_resources_addr=%llu sz=%d\n",
	      CAST_ULL(__cvmx_global_resources_addr), sz);
	return __cvmx_global_resources_addr;
}

u64 cvmx_get_global_resource(struct global_resource_tag tag, int no_lock)
{
	u64 entry_cnt = 0;
	u64 resource_entry_addr = 0;
	int count = 0;
	u64 rphys_addr = 0;
	u64 tag_lo = 0, tag_hi = 0;

	if (__cvmx_global_resources_addr == 0)
		__cvmx_global_resources_init();
	if (!no_lock)
		__cvmx_global_resource_lock();

	entry_cnt = CVMX_GLOBAL_RESOURCES_GET_FIELD(entry_cnt);
	while (entry_cnt > 0) {
		resource_entry_addr = CVMX_GET_RESOURCE_ENTRY(count);
		tag_lo = CVMX_RESOURCE_TAG_GET_FIELD(resource_entry_addr, lo);
		tag_hi = CVMX_RESOURCE_TAG_GET_FIELD(resource_entry_addr, hi);

		if (tag_lo == tag.lo && tag_hi == tag.hi) {
			debug("%s: Found global resource entry\n", __func__);
			break;
		}
		entry_cnt--;
		count++;
	}

	if (entry_cnt == 0) {
		debug("%s: no matching global resource entry found\n",
		      __func__);
		if (!no_lock)
			__cvmx_global_resource_unlock();
		return 0;
	}
	rphys_addr =
		CVMX_RESOURCE_ENTRY_GET_FIELD(resource_entry_addr, phys_addr);
	if (!no_lock)
		__cvmx_global_resource_unlock();

	return rphys_addr;
}

u64 cvmx_create_global_resource(struct global_resource_tag tag, u64 size,
				int no_lock, int *_new_)
{
	u64 entry_count = 0;
	u64 resource_entry_addr = 0;
	u64 phys_addr;

	if (__cvmx_global_resources_addr == 0)
		__cvmx_global_resources_init();

	if (!no_lock)
		__cvmx_global_resource_lock();

	phys_addr =
		cvmx_get_global_resource(tag, CVMX_GLOBAL_RESOURCE_NO_LOCKING);
	if (phys_addr != 0) {
		/* we already have the resource, return it */
		*_new_ = 0;
		goto end;
	}

	*_new_ = 1;
	entry_count = CVMX_GLOBAL_RESOURCES_GET_FIELD(entry_cnt);
	if (entry_count >= CVMX_MAX_GLOBAL_RESOURCES) {
		char tagname[MAX_RESOURCE_TAG_LEN + 1];

		__cvmx_get_tagname(&tag, tagname);
		cvmx_printf(
			"ERROR: %s: reached global resources limit for %s\n",
			__func__, tagname);
		phys_addr = 0;
		goto end;
	}

	/* Allocate bootmem for the resource*/
	phys_addr = __cvmx_alloc_bootmem_for_global_resources(size);
	if (!phys_addr) {
		char tagname[MAX_RESOURCE_TAG_LEN + 1];

		__cvmx_get_tagname(&tag, tagname);
		debug("ERROR: %s: out of memory %s, size=%d\n", __func__,
		      tagname, (int)size);
		goto end;
	}

	resource_entry_addr = CVMX_GET_RESOURCE_ENTRY(entry_count);
	CVMX_RESOURCE_ENTRY_SET_FIELD(resource_entry_addr, phys_addr,
				      phys_addr);
	CVMX_RESOURCE_ENTRY_SET_FIELD(resource_entry_addr, size, size);
	CVMX_RESOURCE_TAG_SET_FIELD(resource_entry_addr, lo, tag.lo);
	CVMX_RESOURCE_TAG_SET_FIELD(resource_entry_addr, hi, tag.hi);
	/* update entry_cnt */
	entry_count += 1;
	CVMX_GLOBAL_RESOURCES_SET_FIELD(entry_cnt, entry_count);

end:
	if (!no_lock)
		__cvmx_global_resource_unlock();

	return phys_addr;
}

int cvmx_create_global_resource_range(struct global_resource_tag tag,
				      int nelements)
{
	int sz = cvmx_range_memory_size(nelements);
	int _new_;
	u64 addr;
	int rv = 0;

	if (__cvmx_global_resources_addr == 0)
		__cvmx_global_resources_init();

	__cvmx_global_resource_lock();
	addr = cvmx_create_global_resource(tag, sz, 1, &_new_);
	if (!addr) {
		__cvmx_global_resource_unlock();
		return -1;
	}
	if (_new_)
		rv = cvmx_range_init(addr, nelements);
	__cvmx_global_resource_unlock();
	return rv;
}

int cvmx_allocate_global_resource_range(struct global_resource_tag tag,
					u64 owner, int nelements, int alignment)
{
	u64 addr = cvmx_get_global_resource(tag, 1);
	int base;

	if (addr == 0) {
		char tagname[256];

		__cvmx_get_tagname(&tag, tagname);
		cvmx_printf("ERROR: %s: cannot find resource %s\n", __func__,
			    tagname);
		return -1;
	}
	__cvmx_global_resource_lock();
	base = cvmx_range_alloc(addr, owner, nelements, alignment);
	__cvmx_global_resource_unlock();
	return base;
}

int cvmx_resource_alloc_reverse(struct global_resource_tag tag, u64 owner)
{
	u64 addr = cvmx_get_global_resource(tag, 1);
	int rv;

	if (addr == 0) {
		char tagname[256];

		__cvmx_get_tagname(&tag, tagname);
		debug("ERROR: cannot find resource %s\n", tagname);
		return -1;
	}
	__cvmx_global_resource_lock();
	rv = cvmx_range_alloc_ordered(addr, owner, 1, 1, 1);
	__cvmx_global_resource_unlock();
	return rv;
}

int cvmx_reserve_global_resource_range(struct global_resource_tag tag,
				       u64 owner, int base, int nelements)
{
	u64 addr = cvmx_get_global_resource(tag, 1);
	int start;

	__cvmx_global_resource_lock();
	start = cvmx_range_reserve(addr, owner, base, nelements);
	__cvmx_global_resource_unlock();
	return start;
}

int cvmx_free_global_resource_range_with_base(struct global_resource_tag tag,
					      int base, int nelements)
{
	u64 addr = cvmx_get_global_resource(tag, 1);
	int rv;

	/* Resource was not created, nothing to release */
	if (addr == 0)
		return 0;

	__cvmx_global_resource_lock();
	rv = cvmx_range_free_with_base(addr, base, nelements);
	__cvmx_global_resource_unlock();
	return rv;
}

int cvmx_free_global_resource_range_multiple(struct global_resource_tag tag,
					     int bases[], int nelements)
{
	u64 addr = cvmx_get_global_resource(tag, 1);
	int rv;

	/* Resource was not created, nothing to release */
	if (addr == 0)
		return 0;

	__cvmx_global_resource_lock();
	rv = cvmx_range_free_mutiple(addr, bases, nelements);
	__cvmx_global_resource_unlock();
	return rv;
}

void cvmx_app_id_init(void *bootmem)
{
	u64 *p = (u64 *)bootmem;

	*p = 0;
}

u64 cvmx_allocate_app_id(void)
{
	u64 *vptr;

	vptr = (u64 *)cvmx_bootmem_alloc_named_range_once(sizeof(cvmx_app_id),
							  0, 1 << 31, 128,
							  "cvmx_app_id",
							  cvmx_app_id_init);

	cvmx_app_id = __atomic_add_fetch(vptr, 1, __ATOMIC_SEQ_CST);

	debug("CVMX_APP_ID = %lx\n", (unsigned long)cvmx_app_id);
	return cvmx_app_id;
}

u64 cvmx_get_app_id(void)
{
	if (cvmx_app_id == 0)
		cvmx_allocate_app_id();
	return cvmx_app_id;
}
