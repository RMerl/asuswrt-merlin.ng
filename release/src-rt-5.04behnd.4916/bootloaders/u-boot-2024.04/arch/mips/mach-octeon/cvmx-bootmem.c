// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2020 Marvell International Ltd.
 */

/*
 * Simple allocate only memory allocator. Used to allocate memory at
 * application start time.
 */

#include <asm/global_data.h>

#include <linux/compat.h>
#include <linux/io.h>
#include <linux/types.h>

#include <mach/octeon-model.h>
#include <mach/cvmx-bootmem.h>
#include <mach/cvmx-coremask.h>
#include <mach/cvmx-regs.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * This is the physical location of a struct cvmx_bootmem_desc
 * structure in Octeon's memory. Note that dues to addressing
 * limits or runtime environment it might not be possible to
 * create a C pointer to this structure.
 */
static u64 cvmx_bootmem_desc_addr;

/**
 * This macro returns the size of a member of a structure.
 * Logically it is the same as "sizeof(s::field)" in C++, but
 * C lacks the "::" operator.
 */
#define SIZEOF_FIELD(s, field) sizeof(((s *)NULL)->field)

/**
 * This macro returns a member of the struct cvmx_bootmem_desc
 * structure. These members can't be directly addressed as
 * they might be in memory not directly reachable. In the case
 * where bootmem is compiled with LINUX_HOST, the structure
 * itself might be located on a remote Octeon. The argument
 * "field" is the member name of the struct cvmx_bootmem_desc to read.
 * Regardless of the type of the field, the return type is always
 * a u64.
 */
#define CVMX_BOOTMEM_DESC_GET_FIELD(field)				\
	__cvmx_bootmem_desc_get(cvmx_bootmem_desc_addr,			\
				offsetof(struct cvmx_bootmem_desc, field), \
				SIZEOF_FIELD(struct cvmx_bootmem_desc, field))

/**
 * This macro writes a member of the struct cvmx_bootmem_desc
 * structure. These members can't be directly addressed as
 * they might be in memory not directly reachable. In the case
 * where bootmem is compiled with LINUX_HOST, the structure
 * itself might be located on a remote Octeon. The argument
 * "field" is the member name of the struct cvmx_bootmem_desc to write.
 */
#define CVMX_BOOTMEM_DESC_SET_FIELD(field, value)			\
	__cvmx_bootmem_desc_set(cvmx_bootmem_desc_addr,			\
				offsetof(struct cvmx_bootmem_desc, field), \
				SIZEOF_FIELD(struct cvmx_bootmem_desc, field), \
				value)

/**
 * This macro returns a member of the
 * struct cvmx_bootmem_named_block_desc structure. These members can't
 * be directly addressed as they might be in memory not directly
 * reachable. In the case where bootmem is compiled with
 * LINUX_HOST, the structure itself might be located on a remote
 * Octeon. The argument "field" is the member name of the
 * struct cvmx_bootmem_named_block_desc to read. Regardless of the type
 * of the field, the return type is always a u64. The "addr"
 * parameter is the physical address of the structure.
 */
#define CVMX_BOOTMEM_NAMED_GET_FIELD(addr, field)			\
	__cvmx_bootmem_desc_get(addr,					\
		offsetof(struct cvmx_bootmem_named_block_desc,  field),	\
		SIZEOF_FIELD(struct cvmx_bootmem_named_block_desc, field))

/**
 * This macro writes a member of the struct cvmx_bootmem_named_block_desc
 * structure. These members can't be directly addressed as
 * they might be in memory not directly reachable. In the case
 * where bootmem is compiled with LINUX_HOST, the structure
 * itself might be located on a remote Octeon. The argument
 * "field" is the member name of the
 * struct cvmx_bootmem_named_block_desc to write. The "addr" parameter
 * is the physical address of the structure.
 */
#define CVMX_BOOTMEM_NAMED_SET_FIELD(addr, field, value)		\
	__cvmx_bootmem_desc_set(addr,					\
		offsetof(struct cvmx_bootmem_named_block_desc, field),	\
		SIZEOF_FIELD(struct cvmx_bootmem_named_block_desc, field), \
				value)

/**
 * This function is the implementation of the get macros defined
 * for individual structure members. The argument are generated
 * by the macros inorder to read only the needed memory.
 *
 * @param base   64bit physical address of the complete structure
 * @param offset Offset from the beginning of the structure to the member being
 *               accessed.
 * @param size   Size of the structure member.
 *
 * Return: Value of the structure member promoted into a u64.
 */
static inline u64 __cvmx_bootmem_desc_get(u64 base, int offset,
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
 * @param offset Offset from the beginning of the structure to the member being
 *               accessed.
 * @param size   Size of the structure member.
 * @param value  Value to write into the structure
 */
static inline void __cvmx_bootmem_desc_set(u64 base, int offset, int size,
					   u64 value)
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

/**
 * This function returns the address of the bootmem descriptor lock.
 *
 * Return: 64-bit address in KSEG0 of the bootmem descriptor block
 */
static inline u64 __cvmx_bootmem_get_lock_addr(void)
{
	return (1ull << 63) |
		(cvmx_bootmem_desc_addr + offsetof(struct cvmx_bootmem_desc, lock));
}

/**
 * This function retrieves the string name of a named block. It is
 * more complicated than a simple memcpy() since the named block
 * descriptor may not be directly accessible.
 *
 * @param addr   Physical address of the named block descriptor
 * @param str    String to receive the named block string name
 * @param len    Length of the string buffer, which must match the length
 *               stored in the bootmem descriptor.
 */
static void CVMX_BOOTMEM_NAMED_GET_NAME(u64 addr, char *str, int len)
{
	int l = len;
	char *ptr = str;

	addr |= (1ull << 63);
	addr += offsetof(struct cvmx_bootmem_named_block_desc, name);
	while (l) {
		/*
		 * With big-endian in memory byte order, this gives uniform
		 * results for the CPU in either big or Little endian mode.
		 */
		u64 blob = cvmx_read64_uint64(addr);
		int sa = 56;

		addr += sizeof(u64);
		while (l && sa >= 0) {
			*ptr++ = (char)(blob >> sa);
			l--;
			sa -= 8;
		}
	}
	str[len] = 0;
}

/**
 * This function stores the string name of a named block. It is
 * more complicated than a simple memcpy() since the named block
 * descriptor may not be directly accessible.
 *
 * @param addr   Physical address of the named block descriptor
 * @param str    String to store into the named block string name
 * @param len    Length of the string buffer, which must match the length
 *               stored in the bootmem descriptor.
 */
void CVMX_BOOTMEM_NAMED_SET_NAME(u64 addr, const char *str, int len)
{
	int l = len;

	addr |= (1ull << 63);
	addr += offsetof(struct cvmx_bootmem_named_block_desc, name);

	while (l) {
		/*
		 * With big-endian in memory byte order, this gives uniform
		 * results for the CPU in either big or Little endian mode.
		 */
		u64 blob = 0;
		int sa = 56;

		while (l && sa >= 0) {
			u64 c = (u8)(*str++);

			l--;
			if (l == 0)
				c = 0;
			blob |= c << sa;
			sa -= 8;
		}
		cvmx_write64_uint64(addr, blob);
		addr += sizeof(u64);
	}
}

/* See header file for descriptions of functions */

/*
 * Wrapper functions are provided for reading/writing the size and next block
 * values as these may not be directly addressible (in 32 bit applications, for
 * instance.)
 *
 * Offsets of data elements in bootmem list, must match
 * struct cvmx_bootmem_block_header
 */
#define NEXT_OFFSET 0
#define SIZE_OFFSET 8

static void cvmx_bootmem_phy_set_size(u64 addr, u64 size)
{
	cvmx_write64_uint64((addr + SIZE_OFFSET) | (1ull << 63), size);
}

static void cvmx_bootmem_phy_set_next(u64 addr, u64 next)
{
	cvmx_write64_uint64((addr + NEXT_OFFSET) | (1ull << 63), next);
}

static u64 cvmx_bootmem_phy_get_size(u64 addr)
{
	return cvmx_read64_uint64((addr + SIZE_OFFSET) | (1ull << 63));
}

static u64 cvmx_bootmem_phy_get_next(u64 addr)
{
	return cvmx_read64_uint64((addr + NEXT_OFFSET) | (1ull << 63));
}

/**
 * Check the version information on the bootmem descriptor
 *
 * @param exact_match
 *               Exact major version to check against. A zero means
 *               check that the version supports named blocks.
 *
 * Return: Zero if the version is correct. Negative if the version is
 *         incorrect. Failures also cause a message to be displayed.
 */
static int __cvmx_bootmem_check_version(int exact_match)
{
	int major_version;

	major_version = CVMX_BOOTMEM_DESC_GET_FIELD(major_version);
	if ((major_version > 3) ||
	    (exact_match && major_version != exact_match)) {
		debug("ERROR: Incompatible bootmem descriptor version: %d.%d at addr: 0x%llx\n",
		      major_version,
		      (int)CVMX_BOOTMEM_DESC_GET_FIELD(minor_version),
		      CAST_ULL(cvmx_bootmem_desc_addr));
		return -1;
	} else {
		return 0;
	}
}

/**
 * Get the low level bootmem descriptor lock. If no locking
 * is specified in the flags, then nothing is done.
 *
 * @param flags  CVMX_BOOTMEM_FLAG_NO_LOCKING means this functions should do
 *               nothing. This is used to support nested bootmem calls.
 */
static inline void __cvmx_bootmem_lock(u32 flags)
{
	if (!(flags & CVMX_BOOTMEM_FLAG_NO_LOCKING)) {
		/*
		 * Unfortunately we can't use the normal cvmx-spinlock code as
		 * the memory for the bootmem descriptor may be not accessible
		 * by a C pointer. We use a 64bit XKPHYS address to access the
		 * memory directly
		 */
		u64 lock_addr = (1ull << 63) |
			(cvmx_bootmem_desc_addr + offsetof(struct cvmx_bootmem_desc,
							   lock));
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
}

/**
 * Release the low level bootmem descriptor lock. If no locking
 * is specified in the flags, then nothing is done.
 *
 * @param flags  CVMX_BOOTMEM_FLAG_NO_LOCKING means this functions should do
 *               nothing. This is used to support nested bootmem calls.
 */
static inline void __cvmx_bootmem_unlock(u32 flags)
{
	if (!(flags & CVMX_BOOTMEM_FLAG_NO_LOCKING)) {
		/*
		 * Unfortunately we can't use the normal cvmx-spinlock code as
		 * the memory for the bootmem descriptor may be not accessible
		 * by a C pointer. We use a 64bit XKPHYS address to access the
		 * memory directly
		 */
		u64 lock_addr = __cvmx_bootmem_get_lock_addr();

		CVMX_SYNCW;
		__asm__ __volatile__("sw $0, 0(%[addr])\n"
				     : : [addr] "r"(lock_addr)
				     : "memory");
		CVMX_SYNCW;
	}
}

/*
 * Some of the cvmx-bootmem functions dealing with C pointers are not
 * supported when we are compiling for CVMX_BUILD_FOR_LINUX_HOST. This
 * ifndef removes these functions when they aren't needed.
 *
 * This functions takes an address range and adjusts it as necessary
 * to match the ABI that is currently being used.  This is required to
 * ensure that bootmem_alloc* functions only return valid pointers for
 * 32 bit ABIs
 */
static int __cvmx_validate_mem_range(u64 *min_addr_ptr,
				     u64 *max_addr_ptr)
{
	u64 max_phys = (1ull << 29) - 0x10;	/* KSEG0 */

	*min_addr_ptr = min_t(u64, max_t(u64, *min_addr_ptr, 0x0), max_phys);
	if (!*max_addr_ptr) {
		*max_addr_ptr = max_phys;
	} else {
		*max_addr_ptr = max_t(u64, min_t(u64, *max_addr_ptr,
						 max_phys), 0x0);
	}

	return 0;
}

u64 cvmx_bootmem_phy_alloc_range(u64 size, u64 alignment,
				 u64 min_addr, u64 max_addr)
{
	s64 address;

	__cvmx_validate_mem_range(&min_addr, &max_addr);
	address = cvmx_bootmem_phy_alloc(size, min_addr, max_addr,
					 alignment, 0);
	if (address > 0)
		return address;
	else
		return 0;
}

void *cvmx_bootmem_alloc_range(u64 size, u64 alignment,
			       u64 min_addr, u64 max_addr)
{
	s64 address;

	__cvmx_validate_mem_range(&min_addr, &max_addr);
	address = cvmx_bootmem_phy_alloc(size, min_addr, max_addr,
					 alignment, 0);

	if (address > 0)
		return cvmx_phys_to_ptr(address);
	else
		return NULL;
}

void *cvmx_bootmem_alloc_address(u64 size, u64 address,
				 u64 alignment)
{
	return cvmx_bootmem_alloc_range(size, alignment, address,
					address + size);
}

void *cvmx_bootmem_alloc_node(u64 node, u64 size, u64 alignment)
{
	return cvmx_bootmem_alloc_range(size, alignment,
					node << CVMX_NODE_MEM_SHIFT,
					((node + 1) << CVMX_NODE_MEM_SHIFT) - 1);
}

void *cvmx_bootmem_alloc(u64 size, u64 alignment)
{
	return cvmx_bootmem_alloc_range(size, alignment, 0, 0);
}

void *cvmx_bootmem_alloc_named_range_once(u64 size, u64 min_addr,
					  u64 max_addr, u64 align,
					  const char *name,
					  void (*init)(void *))
{
	u64 named_block_desc_addr;
	void *ptr;
	s64 addr;

	__cvmx_bootmem_lock(0);

	__cvmx_validate_mem_range(&min_addr, &max_addr);
	named_block_desc_addr =
		cvmx_bootmem_phy_named_block_find(name,
						  CVMX_BOOTMEM_FLAG_NO_LOCKING);

	if (named_block_desc_addr) {
		addr = CVMX_BOOTMEM_NAMED_GET_FIELD(named_block_desc_addr,
						    base_addr);
		__cvmx_bootmem_unlock(0);
		return cvmx_phys_to_ptr(addr);
	}

	addr = cvmx_bootmem_phy_named_block_alloc(size, min_addr, max_addr,
						  align, name,
						  CVMX_BOOTMEM_FLAG_NO_LOCKING);

	if (addr < 0) {
		__cvmx_bootmem_unlock(0);
		return NULL;
	}
	ptr = cvmx_phys_to_ptr(addr);

	if (init)
		init(ptr);
	else
		memset(ptr, 0, size);

	__cvmx_bootmem_unlock(0);
	return ptr;
}

void *cvmx_bootmem_alloc_named_range_flags(u64 size, u64 min_addr,
					   u64 max_addr, u64 align,
					   const char *name, u32 flags)
{
	s64 addr;

	__cvmx_validate_mem_range(&min_addr, &max_addr);
	addr = cvmx_bootmem_phy_named_block_alloc(size, min_addr, max_addr,
						  align, name, flags);
	if (addr >= 0)
		return cvmx_phys_to_ptr(addr);
	else
		return NULL;
}

void *cvmx_bootmem_alloc_named_range(u64 size, u64 min_addr,
				     u64 max_addr, u64 align,
				     const char *name)
{
	return cvmx_bootmem_alloc_named_range_flags(size, min_addr, max_addr,
						    align, name, 0);
}

void *cvmx_bootmem_alloc_named_address(u64 size, u64 address,
				       const char *name)
{
	return cvmx_bootmem_alloc_named_range(size, address, address + size,
					      0, name);
}

void *cvmx_bootmem_alloc_named(u64 size, u64 alignment,
			       const char *name)
{
	return cvmx_bootmem_alloc_named_range(size, 0, 0, alignment, name);
}

void *cvmx_bootmem_alloc_named_flags(u64 size, u64 alignment,
				     const char *name, u32 flags)
{
	return cvmx_bootmem_alloc_named_range_flags(size, 0, 0, alignment,
						    name, flags);
}

int cvmx_bootmem_free_named(const char *name)
{
	return cvmx_bootmem_phy_named_block_free(name, 0);
}

/**
 * Find a named block with flags
 *
 * @param name is the block name
 * @param flags indicates the need to use locking during search
 * Return: pointer to named block descriptor
 *
 * Note: this function returns a pointer to a static structure,
 * and is therefore not re-entrant.
 * Making this function re-entrant will break backward compatibility.
 */
const struct cvmx_bootmem_named_block_desc *
__cvmx_bootmem_find_named_block_flags(const char *name, u32 flags)
{
	static struct cvmx_bootmem_named_block_desc desc;
	u64 named_addr = cvmx_bootmem_phy_named_block_find(name, flags);

	if (named_addr) {
		desc.base_addr = CVMX_BOOTMEM_NAMED_GET_FIELD(named_addr,
							      base_addr);
		desc.size = CVMX_BOOTMEM_NAMED_GET_FIELD(named_addr, size);
		strncpy(desc.name, name, sizeof(desc.name));
		desc.name[sizeof(desc.name) - 1] = 0;
		return &desc;
	} else {
		return NULL;
	}
}

const struct cvmx_bootmem_named_block_desc *
cvmx_bootmem_find_named_block(const char *name)
{
	return __cvmx_bootmem_find_named_block_flags(name, 0);
}

void cvmx_bootmem_print_named(void)
{
	cvmx_bootmem_phy_named_block_print();
}

int cvmx_bootmem_init(u64 mem_desc_addr)
{
	if (!cvmx_bootmem_desc_addr)
		cvmx_bootmem_desc_addr = mem_desc_addr;

	return 0;
}

u64 cvmx_bootmem_available_mem(u64 min_block_size)
{
	return cvmx_bootmem_phy_available_mem(min_block_size);
}

/*
 * The cvmx_bootmem_phy* functions below return 64 bit physical
 * addresses, and expose more features that the cvmx_bootmem_functions
 * above.  These are required for full memory space access in 32 bit
 * applications, as well as for using some advance features.  Most
 * applications should not need to use these.
 */

s64 cvmx_bootmem_phy_alloc(u64 req_size, u64 address_min,
			   u64 address_max, u64 alignment,
			   u32 flags)
{
	u64 head_addr, ent_addr, ent_size;
	u64 target_ent_addr = 0, target_prev_addr = 0;
	u64 target_size = ~0ull;
	u64 free_start, free_end;
	u64 next_addr, prev_addr = 0;
	u64 new_ent_addr = 0, new_ent_size;
	u64 desired_min_addr, usable_max;
	u64 align, align_mask;

	debug("%s: req_size: 0x%llx, min_addr: 0x%llx, max_addr: 0x%llx, align: 0x%llx\n",
	      __func__, CAST_ULL(req_size), CAST_ULL(address_min),
	      CAST_ULL(address_max), CAST_ULL(alignment));

	if (__cvmx_bootmem_check_version(0))
		return -1;

	/*
	 * Do a variety of checks to validate the arguments.  The
	 * allocator code will later assume that these checks have
	 * been made.  We validate that the requested constraints are
	 * not self-contradictory before we look through the list of
	 * available memory
	 */

	/* 0 is not a valid req_size for this allocator */
	if (!req_size)
		return -1;

	/* Round req_size up to multiple of minimum alignment bytes */
	req_size = (req_size + (CVMX_BOOTMEM_ALIGNMENT_SIZE - 1)) &
		~(CVMX_BOOTMEM_ALIGNMENT_SIZE - 1);

	/* Make sure alignment is power of 2, and at least the minimum */
	for (align = CVMX_BOOTMEM_ALIGNMENT_SIZE;
	     align < (1ull << 48);
	     align <<= 1) {
		if (align >= alignment)
			break;
	}

	align_mask = ~(align - 1);

	/*
	 * Adjust address minimum based on requested alignment (round
	 * up to meet alignment).  Do this here so we can reject
	 * impossible requests up front. (NOP for address_min == 0)
	 */
	address_min = (address_min + (align - 1)) & align_mask;

	/*
	 * Convert !0 address_min and 0 address_max to special case of
	 * range that specifies an exact memory block to allocate.  Do
	 * this before other checks and adjustments so that this
	 * tranformation will be validated
	 */
	if (address_min && !address_max)
		address_max = address_min + req_size;
	else if (!address_min && !address_max)
		address_max = ~0ull;	/* If no limits given, use max */

	/*
	 * Reject inconsistent args.  We have adjusted these, so this
	 * may fail due to our internal changes even if this check
	 * would pass for the values the user supplied.
	 */
	if (req_size > address_max - address_min)
		return -1;

	__cvmx_bootmem_lock(flags);

	/* Walk through the list entries to find the right fit */
	head_addr = CVMX_BOOTMEM_DESC_GET_FIELD(head_addr);

	for (ent_addr = head_addr;
	     ent_addr != 0ULL && ent_addr < address_max;
	     prev_addr = ent_addr,
		     ent_addr = cvmx_bootmem_phy_get_next(ent_addr)) {
		/* Raw free block size */
		ent_size = cvmx_bootmem_phy_get_size(ent_addr);
		next_addr = cvmx_bootmem_phy_get_next(ent_addr);

		/* Validate the free list ascending order */
		if (ent_size < CVMX_BOOTMEM_ALIGNMENT_SIZE ||
		    (next_addr && ent_addr > next_addr)) {
			debug("ERROR: %s: bad free list ent: %#llx, next: %#llx\n",
			      __func__, CAST_ULL(ent_addr),
			      CAST_ULL(next_addr));
			goto error_out;
		}

		/* adjust free block edges for alignment */
		free_start = (ent_addr + align - 1) & align_mask;
		free_end = (ent_addr + ent_size) &  align_mask;

		/* check that free block is large enough */
		if ((free_start + req_size) > free_end)
			continue;

		/* check that desired start is within the free block */
		if (free_end < address_min || free_start > address_max)
			continue;
		if ((free_end - address_min) < req_size)
			continue;
		if ((address_max - free_start) < req_size)
			continue;

		/* Found usebale free block */
		target_ent_addr = ent_addr;
		target_prev_addr = prev_addr;
		target_size = ent_size;

		/* Continue looking for highest/best block that fits */
	}

	/* Bail if the search has resulted in no eligible free blocks */
	if (target_ent_addr == 0) {
		debug("%s: eligible free block not found\n", __func__);
		goto error_out;
	}

	/* Found the free block to allocate from */
	ent_addr = target_ent_addr;
	prev_addr = target_prev_addr;
	ent_size = target_size;

	debug("%s: using free block at %#010llx size %#llx\n",
	      __func__, CAST_ULL(ent_addr), CAST_ULL(ent_size));

	/* Always allocate from the end of a free block */
	usable_max = min_t(u64, address_max, ent_addr + ent_size);
	desired_min_addr = usable_max - req_size;
	desired_min_addr &= align_mask;

	/* Split current free block into up to 3 free blocks */

	/* Check for head room */
	if (desired_min_addr > ent_addr) {
		/* Create a new free block at the allocation address */
		new_ent_addr = desired_min_addr;
		new_ent_size = ent_size - (desired_min_addr - ent_addr);

		cvmx_bootmem_phy_set_next(new_ent_addr,
					  cvmx_bootmem_phy_get_next(ent_addr));
		cvmx_bootmem_phy_set_size(new_ent_addr, new_ent_size);

		/* Split out head room into a new free block */
		ent_size -= new_ent_size;
		cvmx_bootmem_phy_set_next(ent_addr, new_ent_addr);
		cvmx_bootmem_phy_set_size(ent_addr, ent_size);

		debug("%s: splitting head, addr %#llx size %#llx\n",
		      __func__, CAST_ULL(ent_addr), CAST_ULL(ent_size));

		/* Make the allocation target the current free block */
		prev_addr = ent_addr;
		ent_addr = new_ent_addr;
		ent_size = new_ent_size;
	}

	/* Check for tail room */
	if ((desired_min_addr + req_size) < (ent_addr + ent_size)) {
		new_ent_addr = ent_addr + req_size;
		new_ent_size = ent_size - req_size;

		/* Create a new free block from tail room */
		cvmx_bootmem_phy_set_next(new_ent_addr,
					  cvmx_bootmem_phy_get_next(ent_addr));
		cvmx_bootmem_phy_set_size(new_ent_addr, new_ent_size);

		debug("%s: splitting tail, addr %#llx size %#llx\n",
		      __func__, CAST_ULL(new_ent_addr), CAST_ULL(new_ent_size));

		/* Adjust the current block to exclude tail room */
		ent_size = ent_size - new_ent_size;
		cvmx_bootmem_phy_set_next(ent_addr, new_ent_addr);
		cvmx_bootmem_phy_set_size(ent_addr, ent_size);
	}

	/* The current free block IS the allocation target */
	if (desired_min_addr != ent_addr || ent_size != req_size)
		debug("ERROR: %s: internal error - addr %#llx %#llx size %#llx %#llx\n",
		      __func__, CAST_ULL(desired_min_addr), CAST_ULL(ent_addr),
		      CAST_ULL(ent_size), CAST_ULL(req_size));

	/* Remove the current free block from list */
	if (prev_addr) {
		cvmx_bootmem_phy_set_next(prev_addr,
					  cvmx_bootmem_phy_get_next(ent_addr));
	} else {
		/* head of list being returned, so update head ptr */
		CVMX_BOOTMEM_DESC_SET_FIELD(head_addr,
					    cvmx_bootmem_phy_get_next(ent_addr));
	}

	__cvmx_bootmem_unlock(flags);
	debug("%s: allocated size: %#llx, at addr: %#010llx\n",
	      __func__,
	      CAST_ULL(req_size),
	      CAST_ULL(desired_min_addr));

	return desired_min_addr;

error_out:
	/* Requested memory not found or argument error */
	__cvmx_bootmem_unlock(flags);
	return -1;
}

int __cvmx_bootmem_phy_free(u64 phy_addr, u64 size, u32 flags)
{
	u64 cur_addr;
	u64 prev_addr = 0;	/* zero is invalid */
	int retval = 0;

	debug("%s addr: %#llx, size: %#llx\n", __func__,
	      CAST_ULL(phy_addr), CAST_ULL(size));

	if (__cvmx_bootmem_check_version(0))
		return 0;

	/* 0 is not a valid size for this allocator */
	if (!size || !phy_addr)
		return 0;

	/* Round size up to mult of minimum alignment bytes */
	size = (size + (CVMX_BOOTMEM_ALIGNMENT_SIZE - 1)) &
		~(CVMX_BOOTMEM_ALIGNMENT_SIZE - 1);

	__cvmx_bootmem_lock(flags);
	cur_addr = CVMX_BOOTMEM_DESC_GET_FIELD(head_addr);
	if (cur_addr == 0 || phy_addr < cur_addr) {
		/* add at front of list - special case with changing head ptr */
		if (cur_addr && phy_addr + size > cur_addr)
			goto bootmem_free_done;	/* error, overlapping section */
		else if (phy_addr + size == cur_addr) {
			/* Add to front of existing first block */
			cvmx_bootmem_phy_set_next(phy_addr,
						  cvmx_bootmem_phy_get_next(cur_addr));
			cvmx_bootmem_phy_set_size(phy_addr,
						  cvmx_bootmem_phy_get_size(cur_addr) + size);
			CVMX_BOOTMEM_DESC_SET_FIELD(head_addr, phy_addr);

		} else {
			/* New block before first block */
			/* OK if cur_addr is 0 */
			cvmx_bootmem_phy_set_next(phy_addr, cur_addr);
			cvmx_bootmem_phy_set_size(phy_addr, size);
			CVMX_BOOTMEM_DESC_SET_FIELD(head_addr, phy_addr);
		}
		retval = 1;
		goto bootmem_free_done;
	}

	/* Find place in list to add block */
	while (cur_addr && phy_addr > cur_addr) {
		prev_addr = cur_addr;
		cur_addr = cvmx_bootmem_phy_get_next(cur_addr);
	}

	if (!cur_addr) {
		/*
		 * We have reached the end of the list, add on to end, checking
		 * to see if we need to combine with last block
		 */
		if (prev_addr + cvmx_bootmem_phy_get_size(prev_addr) == phy_addr) {
			cvmx_bootmem_phy_set_size(prev_addr,
						  cvmx_bootmem_phy_get_size(prev_addr) + size);
		} else {
			cvmx_bootmem_phy_set_next(prev_addr, phy_addr);
			cvmx_bootmem_phy_set_size(phy_addr, size);
			cvmx_bootmem_phy_set_next(phy_addr, 0);
		}
		retval = 1;
		goto bootmem_free_done;
	} else {
		/*
		 * insert between prev and cur nodes, checking for merge with
		 * either/both
		 */
		if (prev_addr + cvmx_bootmem_phy_get_size(prev_addr) == phy_addr) {
			/* Merge with previous */
			cvmx_bootmem_phy_set_size(prev_addr,
						  cvmx_bootmem_phy_get_size(prev_addr) + size);
			if (phy_addr + size == cur_addr) {
				/* Also merge with current */
				cvmx_bootmem_phy_set_size(prev_addr,
							  cvmx_bootmem_phy_get_size(cur_addr) +
							  cvmx_bootmem_phy_get_size(prev_addr));
				cvmx_bootmem_phy_set_next(prev_addr,
							  cvmx_bootmem_phy_get_next(cur_addr));
			}
			retval = 1;
			goto bootmem_free_done;
		} else if (phy_addr + size == cur_addr) {
			/* Merge with current */
			cvmx_bootmem_phy_set_size(phy_addr,
						  cvmx_bootmem_phy_get_size(cur_addr) + size);
			cvmx_bootmem_phy_set_next(phy_addr,
						  cvmx_bootmem_phy_get_next(cur_addr));
			cvmx_bootmem_phy_set_next(prev_addr, phy_addr);
			retval = 1;
			goto bootmem_free_done;
		}

		/* It is a standalone block, add in between prev and cur */
		cvmx_bootmem_phy_set_size(phy_addr, size);
		cvmx_bootmem_phy_set_next(phy_addr, cur_addr);
		cvmx_bootmem_phy_set_next(prev_addr, phy_addr);
	}
	retval = 1;

bootmem_free_done:
	__cvmx_bootmem_unlock(flags);
	return retval;
}

void cvmx_bootmem_phy_list_print(void)
{
	u64 addr;

	addr = CVMX_BOOTMEM_DESC_GET_FIELD(head_addr);
	printf("\n\n\nPrinting bootmem block list, descriptor: 0x%llx, head is 0x%llx\n",
	       CAST_ULL(cvmx_bootmem_desc_addr), CAST_ULL(addr));
	printf("Descriptor version: %d.%d\n",
	       (int)CVMX_BOOTMEM_DESC_GET_FIELD(major_version),
	       (int)CVMX_BOOTMEM_DESC_GET_FIELD(minor_version));
	if (CVMX_BOOTMEM_DESC_GET_FIELD(major_version) > 3)
		debug("Warning: Bootmem descriptor version is newer than expected\n");

	if (!addr)
		printf("mem list is empty!\n");

	while (addr) {
		printf("Block address: 0x%08llx, size: 0x%08llx, next: 0x%08llx\n", CAST_ULL(addr),
		       CAST_ULL(cvmx_bootmem_phy_get_size(addr)),
		       CAST_ULL(cvmx_bootmem_phy_get_next(addr)));
		addr = cvmx_bootmem_phy_get_next(addr);
	}
	printf("\n\n");
}

u64 cvmx_bootmem_phy_available_mem(u64 min_block_size)
{
	u64 addr;

	u64 available_mem = 0;

	__cvmx_bootmem_lock(0);
	addr = CVMX_BOOTMEM_DESC_GET_FIELD(head_addr);
	while (addr) {
		if (cvmx_bootmem_phy_get_size(addr) >= min_block_size)
			available_mem += cvmx_bootmem_phy_get_size(addr);
		addr = cvmx_bootmem_phy_get_next(addr);
	}
	__cvmx_bootmem_unlock(0);
	return available_mem;
}

u64 cvmx_bootmem_phy_named_block_find(const char *name, u32 flags)
{
	u64 result = 0;

	debug("%s: %s\n", __func__, name);

	__cvmx_bootmem_lock(flags);
	if (!__cvmx_bootmem_check_version(3)) {
		int i;
		u64 named_block_array_addr =
			CVMX_BOOTMEM_DESC_GET_FIELD(named_block_array_addr);
		int num_blocks =
			CVMX_BOOTMEM_DESC_GET_FIELD(named_block_num_blocks);
		int name_length =
			CVMX_BOOTMEM_DESC_GET_FIELD(named_block_name_len);
		u64 named_addr = named_block_array_addr;

		for (i = 0; i < num_blocks; i++) {
			u64 named_size =
				CVMX_BOOTMEM_NAMED_GET_FIELD(named_addr, size);
			if (name && named_size) {
				char name_tmp[name_length + 1];

				CVMX_BOOTMEM_NAMED_GET_NAME(named_addr,
							    name_tmp,
							    name_length);
				if (!strncmp(name, name_tmp, name_length)) {
					result = named_addr;
					break;
				}
			} else if (!name && !named_size) {
				result = named_addr;
				break;
			}

			named_addr +=
				sizeof(struct cvmx_bootmem_named_block_desc);
		}
	}
	__cvmx_bootmem_unlock(flags);
	return result;
}

int cvmx_bootmem_phy_named_block_free(const char *name, u32 flags)
{
	u64 named_block_addr;

	if (__cvmx_bootmem_check_version(3))
		return 0;

	debug("%s: %s\n", __func__, name);

	/*
	 * Take lock here, as name lookup/block free/name free need to be
	 * atomic
	 */
	__cvmx_bootmem_lock(flags);

	named_block_addr = cvmx_bootmem_phy_named_block_find(name,
							     CVMX_BOOTMEM_FLAG_NO_LOCKING);
	if (named_block_addr) {
		u64 named_addr =
			CVMX_BOOTMEM_NAMED_GET_FIELD(named_block_addr,
						     base_addr);
		u64 named_size =
			CVMX_BOOTMEM_NAMED_GET_FIELD(named_block_addr, size);

		debug("%s: %s, base: 0x%llx, size: 0x%llx\n",
		      __func__, name, CAST_ULL(named_addr),
		      CAST_ULL(named_size));

		__cvmx_bootmem_phy_free(named_addr, named_size,
					CVMX_BOOTMEM_FLAG_NO_LOCKING);

		/* Set size to zero to indicate block not used. */
		CVMX_BOOTMEM_NAMED_SET_FIELD(named_block_addr, size, 0);
	}

	__cvmx_bootmem_unlock(flags);
	return !!named_block_addr;	/* 0 on failure, 1 on success */
}

s64 cvmx_bootmem_phy_named_block_alloc(u64 size, u64 min_addr,
				       u64 max_addr,
				       u64 alignment, const char *name,
				       u32 flags)
{
	s64 addr_allocated;
	u64 named_block_desc_addr;

	debug("%s: size: 0x%llx, min: 0x%llx, max: 0x%llx, align: 0x%llx, name: %s\n",
	      __func__, CAST_ULL(size), CAST_ULL(min_addr), CAST_ULL(max_addr),
	      CAST_ULL(alignment), name);

	if (__cvmx_bootmem_check_version(3))
		return -1;

	/*
	 * Take lock here, as name lookup/block alloc/name add need to be
	 * atomic
	 */
	__cvmx_bootmem_lock(flags);

	named_block_desc_addr =
		cvmx_bootmem_phy_named_block_find(name, flags |
						  CVMX_BOOTMEM_FLAG_NO_LOCKING);
	if (named_block_desc_addr) {
		__cvmx_bootmem_unlock(flags);
		return -1;
	}

	/* Get pointer to first available named block descriptor */
	named_block_desc_addr =
		cvmx_bootmem_phy_named_block_find(NULL, flags |
						  CVMX_BOOTMEM_FLAG_NO_LOCKING);
	if (!named_block_desc_addr) {
		__cvmx_bootmem_unlock(flags);
		return -1;
	}

	/*
	 * Round size up to mult of minimum alignment bytes
	 * We need the actual size allocated to allow for blocks to be
	 * coallesced when they are freed.  The alloc routine does the
	 * same rounding up on all allocations.
	 */
	size = (size + (CVMX_BOOTMEM_ALIGNMENT_SIZE - 1)) &
		~(CVMX_BOOTMEM_ALIGNMENT_SIZE - 1);

	addr_allocated = cvmx_bootmem_phy_alloc(size, min_addr, max_addr,
						alignment,
						flags | CVMX_BOOTMEM_FLAG_NO_LOCKING);
	if (addr_allocated >= 0) {
		CVMX_BOOTMEM_NAMED_SET_FIELD(named_block_desc_addr, base_addr,
					     addr_allocated);
		CVMX_BOOTMEM_NAMED_SET_FIELD(named_block_desc_addr, size, size);
		CVMX_BOOTMEM_NAMED_SET_NAME(named_block_desc_addr, name,
					    CVMX_BOOTMEM_DESC_GET_FIELD(named_block_name_len));
	}

	__cvmx_bootmem_unlock(flags);
	return addr_allocated;
}

void cvmx_bootmem_phy_named_block_print(void)
{
	int i;
	int printed = 0;

	u64 named_block_array_addr =
		CVMX_BOOTMEM_DESC_GET_FIELD(named_block_array_addr);
	int num_blocks = CVMX_BOOTMEM_DESC_GET_FIELD(named_block_num_blocks);
	int name_length = CVMX_BOOTMEM_DESC_GET_FIELD(named_block_name_len);
	u64 named_block_addr = named_block_array_addr;

	debug("%s: desc addr: 0x%llx\n",
	      __func__, CAST_ULL(cvmx_bootmem_desc_addr));

	if (__cvmx_bootmem_check_version(3))
		return;

	printf("List of currently allocated named bootmem blocks:\n");
	for (i = 0; i < num_blocks; i++) {
		u64 named_size =
			CVMX_BOOTMEM_NAMED_GET_FIELD(named_block_addr, size);
		if (named_size) {
			char name_tmp[name_length + 1];
			u64 named_addr =
				CVMX_BOOTMEM_NAMED_GET_FIELD(named_block_addr,
							     base_addr);
			CVMX_BOOTMEM_NAMED_GET_NAME(named_block_addr, name_tmp,
						    name_length);
			printed++;
			printf("Name: %s, address: 0x%08llx, size: 0x%08llx, index: %d\n", name_tmp,
			       CAST_ULL(named_addr),
			       CAST_ULL(named_size), i);
		}
		named_block_addr +=
			sizeof(struct cvmx_bootmem_named_block_desc);
	}

	if (!printed)
		printf("No named bootmem blocks exist.\n");
}

s64 cvmx_bootmem_phy_mem_list_init(u64 mem_size,
				   u32 low_reserved_bytes,
				   struct cvmx_bootmem_desc *desc_buffer)
{
	u64 cur_block_addr;
	s64 addr;
	int i;

	debug("%s (arg desc ptr: %p, cvmx_bootmem_desc: 0x%llx)\n",
	      __func__, desc_buffer, CAST_ULL(cvmx_bootmem_desc_addr));

	/*
	 * Descriptor buffer needs to be in 32 bit addressable space to be
	 * compatible with 32 bit applications
	 */
	if (!desc_buffer) {
		debug("ERROR: no memory for cvmx_bootmem descriptor provided\n");
		return 0;
	}

	if (mem_size > OCTEON_MAX_PHY_MEM_SIZE) {
		mem_size = OCTEON_MAX_PHY_MEM_SIZE;
		debug("ERROR: requested memory size too large, truncating to maximum size\n");
	}

	if (cvmx_bootmem_desc_addr)
		return 1;

	/* Initialize cvmx pointer to descriptor */
	cvmx_bootmem_init(cvmx_ptr_to_phys(desc_buffer));

	/* Fill the bootmem descriptor */
	CVMX_BOOTMEM_DESC_SET_FIELD(lock, 0);
	CVMX_BOOTMEM_DESC_SET_FIELD(flags, 0);
	CVMX_BOOTMEM_DESC_SET_FIELD(head_addr, 0);
	CVMX_BOOTMEM_DESC_SET_FIELD(major_version, CVMX_BOOTMEM_DESC_MAJ_VER);
	CVMX_BOOTMEM_DESC_SET_FIELD(minor_version, CVMX_BOOTMEM_DESC_MIN_VER);
	CVMX_BOOTMEM_DESC_SET_FIELD(app_data_addr, 0);
	CVMX_BOOTMEM_DESC_SET_FIELD(app_data_size, 0);

	/*
	 * Set up global pointer to start of list, exclude low 64k for exception
	 * vectors, space for global descriptor
	 */
	cur_block_addr = (OCTEON_DDR0_BASE + low_reserved_bytes);

	if (mem_size <= OCTEON_DDR0_SIZE) {
		__cvmx_bootmem_phy_free(cur_block_addr,
					mem_size - low_reserved_bytes, 0);
		goto frees_done;
	}

	__cvmx_bootmem_phy_free(cur_block_addr,
				OCTEON_DDR0_SIZE - low_reserved_bytes, 0);

	mem_size -= OCTEON_DDR0_SIZE;

	/* Add DDR2 block next if present */
	if (mem_size > OCTEON_DDR1_SIZE) {
		__cvmx_bootmem_phy_free(OCTEON_DDR1_BASE, OCTEON_DDR1_SIZE, 0);
		__cvmx_bootmem_phy_free(OCTEON_DDR2_BASE,
					mem_size - OCTEON_DDR2_BASE, 0);
	} else {
		__cvmx_bootmem_phy_free(OCTEON_DDR1_BASE, mem_size, 0);
	}
frees_done:

	/* Initialize the named block structure */
	CVMX_BOOTMEM_DESC_SET_FIELD(named_block_name_len, CVMX_BOOTMEM_NAME_LEN);
	CVMX_BOOTMEM_DESC_SET_FIELD(named_block_num_blocks,
				    CVMX_BOOTMEM_NUM_NAMED_BLOCKS);
	CVMX_BOOTMEM_DESC_SET_FIELD(named_block_array_addr, 0);

	/* Allocate this near the top of the low 256 MBytes of memory */
	addr = cvmx_bootmem_phy_alloc(CVMX_BOOTMEM_NUM_NAMED_BLOCKS *
				      sizeof(struct cvmx_bootmem_named_block_desc),
				      0, 0x10000000, 0,
				      CVMX_BOOTMEM_FLAG_END_ALLOC);
	if (addr >= 0)
		CVMX_BOOTMEM_DESC_SET_FIELD(named_block_array_addr, addr);

	debug("%s: named_block_array_addr: 0x%llx)\n",
	      __func__, CAST_ULL(addr));

	if (addr < 0) {
		debug("FATAL ERROR: unable to allocate memory for bootmem descriptor!\n");
		return 0;
	}

	for (i = 0; i < CVMX_BOOTMEM_NUM_NAMED_BLOCKS; i++) {
		CVMX_BOOTMEM_NAMED_SET_FIELD(addr, base_addr, 0);
		CVMX_BOOTMEM_NAMED_SET_FIELD(addr, size, 0);
		addr += sizeof(struct cvmx_bootmem_named_block_desc);
	}

	return 1;
}

s64 cvmx_bootmem_phy_mem_list_init_multi(u8 node_mask,
					 u32 mem_sizes[],
					 u32 low_reserved_bytes,
					 struct cvmx_bootmem_desc *desc_buffer)
{
	u64 cur_block_addr;
	u64 mem_size;
	s64 addr;
	int i;
	int node;
	u64 node_base;	/* Make u64 to reduce type casting */

	mem_sizes[0] = gd->ram_size / (1024 * 1024);

	debug("cvmx_bootmem_phy_mem_list_init (arg desc ptr: %p, cvmx_bootmem_desc: 0x%llx)\n",
	      desc_buffer, CAST_ULL(cvmx_bootmem_desc_addr));

	/*
	 * Descriptor buffer needs to be in 32 bit addressable space to be
	 * compatible with 32 bit applications
	 */
	if (!desc_buffer) {
		debug("ERROR: no memory for cvmx_bootmem descriptor provided\n");
		return 0;
	}

	cvmx_coremask_for_each_node(node, node_mask) {
		if ((mem_sizes[node] * 1024 * 1024) > OCTEON_MAX_PHY_MEM_SIZE) {
			mem_sizes[node] = OCTEON_MAX_PHY_MEM_SIZE /
				(1024 * 1024);
			debug("ERROR node#%lld: requested memory size too large, truncating to maximum size\n",
			      CAST_ULL(node));
		}
	}

	if (cvmx_bootmem_desc_addr)
		return 1;

	/* Initialize cvmx pointer to descriptor */
	cvmx_bootmem_init(cvmx_ptr_to_phys(desc_buffer));

	/* Fill the bootmem descriptor */
	CVMX_BOOTMEM_DESC_SET_FIELD(lock, 0);
	CVMX_BOOTMEM_DESC_SET_FIELD(flags, 0);
	CVMX_BOOTMEM_DESC_SET_FIELD(head_addr, 0);
	CVMX_BOOTMEM_DESC_SET_FIELD(major_version, CVMX_BOOTMEM_DESC_MAJ_VER);
	CVMX_BOOTMEM_DESC_SET_FIELD(minor_version, CVMX_BOOTMEM_DESC_MIN_VER);
	CVMX_BOOTMEM_DESC_SET_FIELD(app_data_addr, 0);
	CVMX_BOOTMEM_DESC_SET_FIELD(app_data_size, 0);

	cvmx_coremask_for_each_node(node, node_mask) {
		if (node != 0)	/* do not reserve memory on remote nodes */
			low_reserved_bytes = 0;

		mem_size = (u64)mem_sizes[node] * (1024 * 1024); /* MBytes */

		/*
		 * Set up global pointer to start of list, exclude low 64k
		 * for exception vectors, space for global descriptor
		 */

		node_base = (u64)node << CVMX_NODE_MEM_SHIFT;
		cur_block_addr = (OCTEON_DDR0_BASE + low_reserved_bytes) |
			node_base;

		if (mem_size <= OCTEON_DDR0_SIZE) {
			__cvmx_bootmem_phy_free(cur_block_addr,
						mem_size - low_reserved_bytes,
						0);
			continue;
		}

		__cvmx_bootmem_phy_free(cur_block_addr,
					OCTEON_DDR0_SIZE - low_reserved_bytes,
					0);

		mem_size -= OCTEON_DDR0_SIZE;

		/* Add DDR2 block next if present */
		if (mem_size > OCTEON_DDR1_SIZE) {
			__cvmx_bootmem_phy_free(OCTEON_DDR1_BASE |
						node_base,
						OCTEON_DDR1_SIZE, 0);
			__cvmx_bootmem_phy_free(OCTEON_DDR2_BASE |
						node_base,
						mem_size - OCTEON_DDR1_SIZE, 0);
		} else {
			__cvmx_bootmem_phy_free(OCTEON_DDR1_BASE |
						node_base,
						mem_size, 0);
		}
	}

	debug("%s: Initialize the named block\n", __func__);

	/* Initialize the named block structure */
	CVMX_BOOTMEM_DESC_SET_FIELD(named_block_name_len, CVMX_BOOTMEM_NAME_LEN);
	CVMX_BOOTMEM_DESC_SET_FIELD(named_block_num_blocks,
				    CVMX_BOOTMEM_NUM_NAMED_BLOCKS);
	CVMX_BOOTMEM_DESC_SET_FIELD(named_block_array_addr, 0);

	/* Allocate this near the top of the low 256 MBytes of memory */
	addr = cvmx_bootmem_phy_alloc(CVMX_BOOTMEM_NUM_NAMED_BLOCKS *
				      sizeof(struct cvmx_bootmem_named_block_desc),
				      0, 0x10000000, 0,
				      CVMX_BOOTMEM_FLAG_END_ALLOC);
	if (addr >= 0)
		CVMX_BOOTMEM_DESC_SET_FIELD(named_block_array_addr, addr);

	debug("cvmx_bootmem_phy_mem_list_init: named_block_array_addr: 0x%llx)\n",
	      CAST_ULL(addr));

	if (addr < 0) {
		debug("FATAL ERROR: unable to allocate memory for bootmem descriptor!\n");
		return 0;
	}

	for (i = 0; i < CVMX_BOOTMEM_NUM_NAMED_BLOCKS; i++) {
		CVMX_BOOTMEM_NAMED_SET_FIELD(addr, base_addr, 0);
		CVMX_BOOTMEM_NAMED_SET_FIELD(addr, size, 0);
		addr += sizeof(struct cvmx_bootmem_named_block_desc);
	}

	cvmx_bootmem_phy_list_print();

	return 1;
}

int cvmx_bootmem_reserve_memory(u64 start_addr, u64 size,
				const char *name, u32 flags)
{
	u64 addr;
	int rc = 1;
	static unsigned int block_num;
	char block_name[CVMX_BOOTMEM_NAME_LEN];

	debug("%s: start %#llx, size: %#llx, name: %s, flags:%#x)\n",
	      __func__, CAST_ULL(start_addr), CAST_ULL(size), name, flags);

	if (__cvmx_bootmem_check_version(3))
		return 0;

	addr = CVMX_BOOTMEM_DESC_GET_FIELD(head_addr);
	if (!addr)
		return 0;

	if (!name)
		name = "__cvmx_bootmem_reserved";

	while (addr && rc) {
		u64 block_size = cvmx_bootmem_phy_get_size(addr);
		u64 reserve_size = 0;

		if (addr >= start_addr && addr < start_addr + size) {
			reserve_size = size - (addr - start_addr);
			if (block_size < reserve_size)
				reserve_size = block_size;
		} else if (start_addr > addr &&
			   start_addr < (addr + block_size)) {
			reserve_size = block_size - (start_addr - addr);
		}

		if (reserve_size) {
			snprintf(block_name, sizeof(block_name),
				 "%.32s_%012llx_%u",
				 name, (unsigned long long)start_addr,
				 (unsigned int)block_num);

			debug("%s: Reserving 0x%llx bytes at address 0x%llx with name %s\n",
			      __func__, CAST_ULL(reserve_size),
			      CAST_ULL(addr), block_name);

			if (cvmx_bootmem_phy_named_block_alloc(reserve_size,
							       addr, 0, 0,
							       block_name,
							       flags) == -1) {
				debug("%s: Failed to reserve 0x%llx bytes at address 0x%llx\n",
				      __func__, CAST_ULL(reserve_size),
				      (unsigned long long)addr);
				rc = 0;
				break;
			}

			debug("%s: Reserved 0x%llx bytes at address 0x%llx with name %s\n",
			      __func__, CAST_ULL(reserve_size),
			      CAST_ULL(addr), block_name);
		}

		addr = cvmx_bootmem_phy_get_next(addr);
		block_num++;
	}

	return rc;
}

void cvmx_bootmem_lock(void)
{
	__cvmx_bootmem_lock(0);
}

void cvmx_bootmem_unlock(void)
{
	__cvmx_bootmem_unlock(0);
}

void *__cvmx_phys_addr_to_ptr(u64 phys, int size)
{
	void *tmp;

	if (sizeof(void *) == 8) {
		tmp = CASTPTR(void, CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS, phys));
	} else {
		u32 phy32 = (u32)(phys & 0x7fffffffULL);

		tmp = CASTPTR(void, CVMX_ADD_SEG32(CVMX_MIPS32_SPACE_KSEG0,
						   phy32));
	}

	return tmp;
}

void *__cvmx_bootmem_internal_get_desc_ptr(void)
{
	return cvmx_phys_to_ptr(cvmx_bootmem_desc_addr);
}
