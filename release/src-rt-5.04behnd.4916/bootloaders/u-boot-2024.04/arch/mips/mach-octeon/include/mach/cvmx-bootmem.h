/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

/**
 * @file
 * Simple allocate only memory allocator. Used to allocate memory at application
 * start time.
 */

#ifndef __CVMX_BOOTMEM_H__
#define __CVMX_BOOTMEM_H__

/* Must be multiple of 8, changing breaks ABI */
#define CVMX_BOOTMEM_NAME_LEN		128
/* Can change without breaking ABI */
#define CVMX_BOOTMEM_NUM_NAMED_BLOCKS	64
/* minimum alignment of bootmem alloced blocks */
#define CVMX_BOOTMEM_ALIGNMENT_SIZE	(16ull)

/* Flags for cvmx_bootmem_phy_mem* functions */
/* Allocate from end of block instead of beginning */
#define CVMX_BOOTMEM_FLAG_END_ALLOC	(1 << 0)
#define CVMX_BOOTMEM_FLAG_NO_LOCKING	(1 << 1) /* Don't do any locking. */

/* Real physical addresses of memory regions */
#define OCTEON_DDR0_BASE    (0x0ULL)
/* Use 16MiB here, as 256 leads to overwriting U-Boot reloc space */
#define OCTEON_DDR0_SIZE    (0x001000000ULL)
#define OCTEON_DDR1_BASE    ((OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) \
			     ? 0x20000000ULL : 0x410000000ULL)
#define OCTEON_DDR1_SIZE    (0x010000000ULL)
#define OCTEON_DDR2_BASE    ((OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) \
			     ? 0x30000000ULL : 0x20000000ULL)
#define OCTEON_DDR2_SIZE    ((OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) \
			     ? 0x7d0000000ULL : 0x3e0000000ULL)
#define OCTEON_MAX_PHY_MEM_SIZE ((OCTEON_IS_MODEL(OCTEON_CN68XX))	\
				 ? 128 * 1024 * 1024 * 1024ULL		\
				 : (OCTEON_IS_OCTEON2())		\
				 ? 32 * 1024 * 1024 * 1024ull		\
				 : (OCTEON_IS_OCTEON3())		\
				 ? 512 * 1024 * 1024 * 1024ULL		\
				 : 16 * 1024 * 1024 * 1024ULL)

/*
 * First bytes of each free physical block of memory contain this structure,
 * which is used to maintain the free memory list.  Since the bootloader is
 * only 32 bits, there is a union providing 64 and 32 bit versions.  The
 * application init code converts addresses to 64 bit addresses before the
 * application starts.
 */
struct cvmx_bootmem_block_header {
	/* Note: these are referenced from assembly routines in the bootloader,
	 * so this structure should not be changed without changing those
	 * routines as well.
	 */
	u64 next_block_addr;
	u64 size;

};

/*
 * Structure for named memory blocks
 * Number of descriptors
 * available can be changed without affecting compatibility,
 * but name length changes require a bump in the bootmem
 * descriptor version
 * Note: This structure must be naturally 64 bit aligned, as a single
 * memory image will be used by both 32 and 64 bit programs.
 */
struct cvmx_bootmem_named_block_desc {
	u64 base_addr;	/* Base address of named block */
	/*
	 * Size actually allocated for named block (may differ from requested)
	 */
	u64 size;
	char name[CVMX_BOOTMEM_NAME_LEN]; /* name of named block */
};

/* Current descriptor versions */
/* CVMX bootmem descriptor major version */
#define CVMX_BOOTMEM_DESC_MAJ_VER	3
/* CVMX bootmem descriptor minor version */
#define CVMX_BOOTMEM_DESC_MIN_VER	0

/*
 * First three members of cvmx_bootmem_desc_t are left in original
 * positions for backwards compatibility.
 */
struct cvmx_bootmem_desc {
	/* Linux compatible proxy for __BIG_ENDIAN */
	u32 lock;	/* spinlock to control access to list */
	u32 flags;	/* flags for indicating various conditions */
	u64 head_addr;

	/* incremented changed when incompatible changes made */
	u32 major_version;
	/*
	 * incremented changed when compatible changes made, reset to
	 * zero when major incremented
	 */
	u32 minor_version;
	u64 app_data_addr;
	u64 app_data_size;

	/* number of elements in named blocks array */
	u32 named_block_num_blocks;
	/* length of name array in bootmem blocks */
	u32 named_block_name_len;
	/* address of named memory block descriptors */
	u64 named_block_array_addr;
};

/**
 * Initialize the boot alloc memory structures. This is
 * normally called inside of cvmx_user_app_init()
 *
 * @param mem_desc_addr	Address of the free memory list
 * @return
 */
int cvmx_bootmem_init(u64 mem_desc_addr);

/**
 * Allocate a block of memory from the free list that was passed
 * to the application by the bootloader.
 * This is an allocate-only algorithm, so freeing memory is not possible.
 *
 * @param size      Size in bytes of block to allocate
 * @param alignment Alignment required - must be power of 2
 *
 * Return: pointer to block of memory, NULL on error
 */
void *cvmx_bootmem_alloc(u64 size, u64 alignment);

/**
 * Allocate a block of memory from the free list that was passed
 * to the application by the bootloader from a specific node.
 * This is an allocate-only algorithm, so freeing memory is not possible.
 *
 * @param node	The node to allocate memory from
 * @param size  Size in bytes of block to allocate
 * @param alignment Alignment required - must be power of 2
 *
 * Return: pointer to block of memory, NULL on error
 */
void *cvmx_bootmem_alloc_node(u64 node, u64 size, u64 alignment);

/**
 * Allocate a block of memory from the free list that was
 * passed to the application by the bootloader at a specific
 * address. This is an allocate-only algorithm, so
 * freeing memory is not possible. Allocation will fail if
 * memory cannot be allocated at the specified address.
 *
 * @param size      Size in bytes of block to allocate
 * @param address   Physical address to allocate memory at.  If this
 *                  memory is not available, the allocation fails.
 * @param alignment Alignment required - must be power of 2
 * Return: pointer to block of memory, NULL on error
 */
void *cvmx_bootmem_alloc_address(u64 size, u64 address,
				 u64 alignment);

/**
 * Allocate a block of memory from the free list that was
 * passed to the application by the bootloader within a specified
 * address range. This is an allocate-only algorithm, so
 * freeing memory is not possible. Allocation will fail if
 * memory cannot be allocated in the requested range.
 *
 * @param size      Size in bytes of block to allocate
 * @param min_addr  defines the minimum address of the range
 * @param max_addr  defines the maximum address of the range
 * @param alignment Alignment required - must be power of 2
 * Return: pointer to block of memory, NULL on error
 */
void *cvmx_bootmem_alloc_range(u64 size, u64 alignment,
			       u64 min_addr, u64 max_addr);

/**
 * Allocate a block of memory from the free list that was passed
 * to the application by the bootloader, and assign it a name in the
 * global named block table.  (part of the cvmx_bootmem_descriptor_t structure)
 * Named blocks can later be freed.
 *
 * @param size  Size in bytes of block to allocate
 * @param alignment Alignment required - must be power of 2
 * @param name  name of block - must be less than CVMX_BOOTMEM_NAME_LEN bytes
 *
 * Return: pointer to block of memory, NULL on error
 */
void *cvmx_bootmem_alloc_named(u64 size, u64 alignment,
			       const char *name);

/**
 * Allocate a block of memory from the free list that was passed
 * to the application by the bootloader, and assign it a name in the
 * global named block table.  (part of the cvmx_bootmem_descriptor_t structure)
 * Named blocks can later be freed.
 *
 * @param size Size in bytes of block to allocate
 * @param alignment Alignment required - must be power of 2
 * @param name name of block - must be less than CVMX_BOOTMEM_NAME_LEN bytes
 * @param flags     Flags to control options for the allocation.
 *
 * Return: pointer to block of memory, NULL on error
 */
void *cvmx_bootmem_alloc_named_flags(u64 size, u64 alignment,
				     const char *name, u32 flags);

/**
 * Allocate a block of memory from the free list that was passed
 * to the application by the bootloader, and assign it a name in the
 * global named block table.  (part of the cvmx_bootmem_descriptor_t structure)
 * Named blocks can later be freed.
 *
 * @param size    Size in bytes of block to allocate
 * @param address Physical address to allocate memory at.  If this
 *                memory is not available, the allocation fails.
 * @param name    name of block - must be less than CVMX_BOOTMEM_NAME_LEN bytes
 *
 * Return: pointer to block of memory, NULL on error
 */
void *cvmx_bootmem_alloc_named_address(u64 size, u64 address,
				       const char *name);

/**
 * Allocate a block of memory from a specific range of the free list
 * that was passed to the application by the bootloader, and assign it
 * a name in the global named block table.  (part of the
 * cvmx_bootmem_descriptor_t structure) Named blocks can later be
 * freed.  If request cannot be satisfied within the address range
 * specified, NULL is returned
 *
 * @param size      Size in bytes of block to allocate
 * @param min_addr  minimum address of range
 * @param max_addr  maximum address of range
 * @param align  Alignment of memory to be allocated. (must be a power of 2)
 * @param name   name of block - must be less than CVMX_BOOTMEM_NAME_LEN bytes
 *
 * Return: pointer to block of memory, NULL on error
 */
void *cvmx_bootmem_alloc_named_range(u64 size, u64 min_addr,
				     u64 max_addr, u64 align,
				     const char *name);

/**
 * Allocate if needed a block of memory from a specific range of the
 * free list that was passed to the application by the bootloader, and
 * assign it a name in the global named block table.  (part of the
 * cvmx_bootmem_descriptor_t structure) Named blocks can later be
 * freed.  If the requested name block is already allocated, return
 * the pointer to block of memory.  If request cannot be satisfied
 * within the address range specified, NULL is returned
 *
 * @param size   Size in bytes of block to allocate
 * @param min_addr  minimum address of range
 * @param max_addr  maximum address of range
 * @param align  Alignment of memory to be allocated. (must be a power of 2)
 * @param name   name of block - must be less than CVMX_BOOTMEM_NAME_LEN bytes
 * @param init   Initialization function
 *
 * The initialization function is optional, if omitted the named block
 * is initialized to all zeros when it is created, i.e. once.
 *
 * Return: pointer to block of memory, NULL on error
 */
void *cvmx_bootmem_alloc_named_range_once(u64 size,
					  u64 min_addr,
					  u64 max_addr,
					  u64 align,
					  const char *name,
					  void (*init)(void *));

/**
 * Allocate all free memory starting at the start address.  This is used to
 * prevent any free blocks from later being allocated within the reserved space.
 * Note that any memory allocated with this function cannot be later freed.
 *
 * @param start_addr  Starting address to reserve
 * @param size        Size in bytes to reserve starting at start_addr
 * @param name        Name to assign to reserved blocks
 * @param flags       Flags to use when reserving memory
 *
 * Return: 0 on failure,
 *         !0 on success
 */
int cvmx_bootmem_reserve_memory(u64 start_addr, u64 size,
				const char *name, u32 flags);

/**
 * Frees a previously allocated named bootmem block.
 *
 * @param name   name of block to free
 *
 * Return: 0 on failure,
 *         !0 on success
 */
int cvmx_bootmem_free_named(const char *name);

/**
 * Finds a named bootmem block by name.
 *
 * @param name   name of block to free
 *
 * Return: pointer to named block descriptor on success
 *         0 on failure
 */
const struct cvmx_bootmem_named_block_desc *
cvmx_bootmem_find_named_block(const char *name);

/**
 * Returns the size of available memory in bytes, only
 * counting blocks that are at least as big as the minimum block
 * size.
 *
 * @param min_block_size
 *               Minimum block size to count in total.
 *
 * Return: Number of bytes available for allocation that meet the
 * block size requirement
 */
u64 cvmx_bootmem_available_mem(u64 min_block_size);

/**
 * Prints out the list of named blocks that have been allocated
 * along with their addresses and sizes.
 * This is primarily used for debugging purposes
 */
void cvmx_bootmem_print_named(void);

/**
 * Allocates a block of physical memory from the free list, at
 * (optional) requested address and alignment.
 *
 * @param req_size size of region to allocate.  All requests are
 * rounded up to be a multiple CVMX_BOOTMEM_ALIGNMENT_SIZE bytes size
 *
 * @param address_min Minimum address that block can occupy.
 *
 * @param address_max Specifies the maximum address_min (inclusive)
 * that the allocation can use.
 *
 * @param alignment Requested alignment of the block.  If this
 *                  alignment cannot be met, the allocation fails.
 *                  This must be a power of 2.  (Note: Alignment of
 *                  CVMX_BOOTMEM_ALIGNMENT_SIZE bytes is required, and
 *                  internally enforced.  Requested alignments of less
 *                  than CVMX_BOOTMEM_ALIGNMENT_SIZE are set to
 *                  CVMX_BOOTMEM_ALIGNMENT_SIZE.)
 * @param flags     Flags to control options for the allocation.
 *
 * Return: physical address of block allocated, or -1 on failure
 */
s64 cvmx_bootmem_phy_alloc(u64 req_size, u64 address_min, u64 address_max,
			   u64 alignment, u32 flags);

/**
 * Allocates a named block of physical memory from the free list, at
 * (optional) requested address and alignment.
 *
 * @param size size of region to allocate.  All requests are rounded
 * up to be a multiple CVMX_BOOTMEM_ALIGNMENT_SIZE bytes size
 *
 * @param min_addr  Minimum address that block can occupy.
 *
 * @param max_addr Specifies the maximum address_min (inclusive) that
 * the allocation can use.
 *
 * @param alignment Requested alignment of the block.  If this
 *                  alignment cannot be met, the allocation fails.
 *                  This must be a power of 2.  (Note: Alignment of
 *                  CVMX_BOOTMEM_ALIGNMENT_SIZE bytes is required, and
 *                  internally enforced.  Requested alignments of less
 *                  than CVMX_BOOTMEM_ALIGNMENT_SIZE are set to
 *                  CVMX_BOOTMEM_ALIGNMENT_SIZE.)
 *
 * @param name      name to assign to named block
 *
 * @param flags     Flags to control options for the allocation.
 *
 * Return: physical address of block allocated, or -1 on failure
 */
s64 cvmx_bootmem_phy_named_block_alloc(u64 size, u64 min_addr, u64 max_addr,
				       u64 alignment, const char *name,
				       u32 flags);

/**
 * Finds a named memory block by name.
 * Also used for finding an unused entry in the named block table.
 *
 * @param name Name of memory block to find.  If NULL pointer given,
 *             then finds unused descriptor, if available.
 *
 * @param flags  Flags to control options for the allocation.
 *
 * Return: Physical address of the memory block descriptor, zero if not
 *         found. If zero returned when name parameter is NULL, then no
 *         memory block descriptors are available.
 */
u64 cvmx_bootmem_phy_named_block_find(const char *name, u32 flags);

/**
 * Returns the size of available memory in bytes, only
 * counting blocks that are at least as big as the minimum block
 * size.
 *
 * @param min_block_size
 *               Minimum block size to count in total.
 *
 * Return: Number of bytes available for allocation that meet the
 * block size requirement
 */
u64 cvmx_bootmem_phy_available_mem(u64 min_block_size);

/**
 * Frees a named block.
 *
 * @param name   name of block to free
 * @param flags  flags for passing options
 *
 * Return: 0 on failure
 *         1 on success
 */
int cvmx_bootmem_phy_named_block_free(const char *name, u32 flags);

/**
 * Frees a block to the bootmem allocator list.  This must
 * be used with care, as the size provided must match the size
 * of the block that was allocated, or the list will become
 * corrupted.
 *
 * IMPORTANT:  This is only intended to be used as part of named block
 * frees and initial population of the free memory list.
 *                                                      *
 *
 * @param phy_addr physical address of block
 * @param size     size of block in bytes.
 * @param flags    flags for passing options
 *
 * Return: 1 on success,
 *         0 on failure
 */
int __cvmx_bootmem_phy_free(u64 phy_addr, u64 size, u32 flags);

/**
 * Prints the list of currently allocated named blocks
 *
 */
void cvmx_bootmem_phy_named_block_print(void);

/**
 * Prints the list of available memory.
 *
 */
void cvmx_bootmem_phy_list_print(void);

/**
 * This function initializes the free memory list used by cvmx_bootmem.
 * This must be called before any allocations can be done.
 *
 * @param mem_size Total memory available, in bytes
 *
 * @param low_reserved_bytes Number of bytes to reserve (leave out of
 * free list) at address 0x0.
 *
 * @param desc_buffer Buffer for the bootmem descriptor.  This must be
 *                 a 32 bit addressable address.
 *
 * Return: 1 on success
 *         0 on failure
 */
s64 cvmx_bootmem_phy_mem_list_init(u64 mem_size, u32 low_reserved_bytes,
				   struct cvmx_bootmem_desc *desc_buffer);

/**
 * This function initializes the free memory list used by cvmx_bootmem.
 * This must be called before any allocations can be done.
 *
 * @param nodemask Nodemask - one bit per node (bit0->node0, bit1->node1,...)
 *
 * @param mem_size[] Array of memory sizes in MBytes per node ([0]->node0,...)
 *
 * @param low_reserved_bytes Number of bytes to reserve (leave out of
 * free list) at address 0x0.
 *
 * @param desc_buffer Buffer for the bootmem descriptor.  This must be
 *                 a 32 bit addressable address.
 *
 * Return: 1 on success
 *         0 on failure
 */
s64 cvmx_bootmem_phy_mem_list_init_multi(u8 nodemask, u32 mem_size[],
					 u32 low_reserved_bytes,
					 struct cvmx_bootmem_desc *desc_buffer);
/**
 * Locks the bootmem allocator.  This is useful in certain situations
 * where multiple allocations must be made without being interrupted.
 * This should be used with the CVMX_BOOTMEM_FLAG_NO_LOCKING flag.
 *
 */
void cvmx_bootmem_lock(void);

/**
 * Unlocks the bootmem allocator.  This is useful in certain situations
 * where multiple allocations must be made without being interrupted.
 * This should be used with the CVMX_BOOTMEM_FLAG_NO_LOCKING flag.
 *
 */
void cvmx_bootmem_unlock(void);

/**
 * Internal use function to get the current descriptor pointer
 */
void *__cvmx_bootmem_internal_get_desc_ptr(void);

/**
 * Internal use.  This is userd to get a pointer to a physical
 * address.  For linux n32 the physical address in mmaped to a virtual
 * address and the virtual address is returned.  For n64 the address
 * is converted to an xkphys address and the xkhpys address is
 * returned.
 */
void *__cvmx_phys_addr_to_ptr(u64 phys, int size);
const struct cvmx_bootmem_named_block_desc *
__cvmx_bootmem_find_named_block_flags(const char *name, u32 flags);
void *cvmx_bootmem_alloc_named_range_flags(u64 size, u64 min_addr,
					   u64 max_addr, u64 align,
					   const char *name, u32 flags);
u64 cvmx_bootmem_phy_alloc_range(u64 size, u64 alignment,
				 u64 min_addr, u64 max_addr);

#endif /*   __CVMX_BOOTMEM_H__ */
