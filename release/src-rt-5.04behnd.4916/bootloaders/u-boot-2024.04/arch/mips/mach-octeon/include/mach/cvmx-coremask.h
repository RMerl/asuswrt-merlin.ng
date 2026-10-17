/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

/**
 * Module to support operations on bitmap of cores. Coremask can be used to
 * select a specific core, a group of cores, or all available cores, for
 * initialization and differentiation of roles within a single shared binary
 * executable image.
 *
 * The core numbers used in this file are the same value as what is found in
 * the COP0_EBASE register and the rdhwr 0 instruction.
 *
 * For the CN78XX and other multi-node environments the core numbers are not
 * contiguous.  The core numbers for the CN78XX are as follows:
 *
 * Node 0:	Cores 0 - 47
 * Node 1:	Cores 128 - 175
 * Node 2:	Cores 256 - 303
 * Node 3:	Cores 384 - 431
 *
 * The coremask environment generally tries to be node agnostic in order to
 * provide future compatibility if more cores are added to future processors
 * or more nodes are supported.
 */

#ifndef __CVMX_COREMASK_H__
#define __CVMX_COREMASK_H__

#include "cvmx-regs.h"

/* bits per holder */
#define CVMX_COREMASK_HLDRSZ	((int)(sizeof(u64) * 8))

/** Maximum allowed cores per node */
#define CVMX_COREMASK_MAX_CORES_PER_NODE	(1 << CVMX_NODE_NO_SHIFT)

/** Maximum number of bits actually used in the coremask */
#define CVMX_MAX_USED_CORES_BMP	(1 << (CVMX_NODE_NO_SHIFT + CVMX_NODE_BITS))

/* the number of valid bits in and the mask of the most significant holder */
#define CVMX_COREMASK_MSHLDR_NBITS			\
	(CVMX_MIPS_MAX_CORES % CVMX_COREMASK_HLDRSZ)

#define CVMX_COREMASK_MSHLDR_MASK				\
	((CVMX_COREMASK_MSHLDR_NBITS) ?				\
	 (((u64)1 << CVMX_COREMASK_MSHLDR_NBITS) - 1) :		\
	 ((u64)-1))

/* cvmx_coremask size in u64 */
#define CVMX_COREMASK_BMPSZ					\
	((int)(CVMX_MIPS_MAX_CORES / CVMX_COREMASK_HLDRSZ +	\
	       (CVMX_COREMASK_MSHLDR_NBITS != 0)))

#define CVMX_COREMASK_USED_BMPSZ				\
	(CVMX_MAX_USED_CORES_BMP / CVMX_COREMASK_HLDRSZ)

#define CVMX_COREMASK_BMP_NODE_CORE_IDX(node, core)			\
	((((node) << CVMX_NODE_NO_SHIFT) + (core)) / CVMX_COREMASK_HLDRSZ)
/**
 * Maximum available coremask.
 */
#define CVMX_COREMASK_MAX				\
	{ {						\
			0x0000FFFFFFFFFFFF, 0,		\
				0x0000FFFFFFFFFFFF, 0,	\
				0x0000FFFFFFFFFFFF, 0,	\
				0x0000FFFFFFFFFFFF, 0,	\
				0, 0,			\
				0, 0,			\
				0, 0,			\
				0, 0} }

/**
 * Empty coremask
 */
#define CVMX_COREMASK_EMPTY					\
	{ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} }

struct cvmx_coremask {
	u64 coremask_bitmap[CVMX_COREMASK_BMPSZ];
};

/**
 * Macro to iterate through all available cores in a coremask
 *
 * @param core - core variable to use to iterate
 * @param pcm - pointer to core mask
 *
 * Use this like a for statement
 */
#define cvmx_coremask_for_each_core(core, pcm)			\
	for ((core) = -1;					\
	     (core) = cvmx_coremask_next_core((core), pcm),	\
		     (core) >= 0;)

/**
 * Given a node and node mask, return the next available node.
 *
 * @param node		starting node number
 * @param node_mask	node mask to use to find the next node
 *
 * Return: next node number or -1 if no more nodes are available
 */
static inline int cvmx_coremask_next_node(int node, u8 node_mask)
{
	int next_offset;

	next_offset = __builtin_ffs(node_mask >> (node + 1));
	if (next_offset == 0)
		return -1;
	else
		return node + next_offset;
}

/**
 * Iterate through all nodes in a node mask
 *
 * @param node		node iterator variable
 * @param node_mask	mask to use for iterating
 *
 * Use this like a for statement
 */
#define cvmx_coremask_for_each_node(node, node_mask)		\
	for ((node) = __builtin_ffs(node_mask) - 1;		\
	     (node) >= 0 && (node) < CVMX_MAX_NODES;		\
	     (node) = cvmx_coremask_next_node(node, node_mask))

/**
 * Is ``core'' set in the coremask?
 *
 * @param pcm is the pointer to the coremask.
 * @param core
 * Return: 1 if core is set and 0 if not.
 */
static inline int cvmx_coremask_is_core_set(const struct cvmx_coremask *pcm,
					    int core)
{
	int n, i;

	n = core % CVMX_COREMASK_HLDRSZ;
	i = core / CVMX_COREMASK_HLDRSZ;

	return (pcm->coremask_bitmap[i] & ((u64)1 << n)) != 0;
}

/**
 * Is ``current core'' set in the coremask?
 *
 * @param pcm is the pointer to the coremask.
 * Return: 1 if core is set and 0 if not.
 */
static inline int cvmx_coremask_is_self_set(const struct cvmx_coremask *pcm)
{
	return cvmx_coremask_is_core_set(pcm, (int)cvmx_get_core_num());
}

/**
 * Is coremask empty?
 * @param pcm is the pointer to the coremask.
 * Return: 1 if *pcm is empty (all zeros), 0 if not empty.
 */
static inline int cvmx_coremask_is_empty(const struct cvmx_coremask *pcm)
{
	int i;

	for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)
		if (pcm->coremask_bitmap[i] != 0)
			return 0;

	return 1;
}

/**
 * Set ``core'' in the coremask.
 *
 * @param pcm is the pointer to the coremask.
 * @param core
 * Return: 0.
 */
static inline int cvmx_coremask_set_core(struct cvmx_coremask *pcm, int core)
{
	int n, i;

	n = core % CVMX_COREMASK_HLDRSZ;
	i = core / CVMX_COREMASK_HLDRSZ;
	pcm->coremask_bitmap[i] |= ((u64)1 << n);

	return 0;
}

/**
 * Set ``current core'' in the coremask.
 *
 * @param pcm is the pointer to the coremask.
 * Return: 0.
 */
static inline int cvmx_coremask_set_self(struct cvmx_coremask *pcm)
{
	return cvmx_coremask_set_core(pcm, (int)cvmx_get_core_num());
}

/**
 * Clear ``core'' from the coremask.
 *
 * @param pcm is the pointer to the coremask.
 * @param core
 * Return: 0.
 */
static inline int cvmx_coremask_clear_core(struct cvmx_coremask *pcm, int core)
{
	int n, i;

	n = core % CVMX_COREMASK_HLDRSZ;
	i = core / CVMX_COREMASK_HLDRSZ;
	pcm->coremask_bitmap[i] &= ~((u64)1 << n);

	return 0;
}

/**
 * Clear ``current core'' from the coremask.
 *
 * @param pcm is the pointer to the coremask.
 * Return: 0.
 */
static inline int cvmx_coremask_clear_self(struct cvmx_coremask *pcm)
{
	return cvmx_coremask_clear_core(pcm, cvmx_get_core_num());
}

/**
 * Toggle ``core'' in the coremask.
 *
 * @param pcm is the pointer to the coremask.
 * @param core
 * Return: 0.
 */
static inline int cvmx_coremask_toggle_core(struct cvmx_coremask *pcm, int core)
{
	int n, i;

	n = core % CVMX_COREMASK_HLDRSZ;
	i = core / CVMX_COREMASK_HLDRSZ;
	pcm->coremask_bitmap[i] ^= ((u64)1 << n);

	return 0;
}

/**
 * Toggle ``current core'' in the coremask.
 *
 * @param pcm is the pointer to the coremask.
 * Return: 0.
 */
static inline int cvmx_coremask_toggle_self(struct cvmx_coremask *pcm)
{
	return cvmx_coremask_toggle_core(pcm, cvmx_get_core_num());
}

/**
 * Set the lower 64-bit of the coremask.
 * @param pcm	pointer to coremask
 * @param coremask_64	64-bit coremask to apply to the first node (0)
 */
static inline void cvmx_coremask_set64(struct cvmx_coremask *pcm,
				       u64 coremask_64)
{
	pcm->coremask_bitmap[0] = coremask_64;
}

/**
 * Set the 64-bit of the coremask for a particular node.
 * @param pcm	pointer to coremask
 * @param node	node to set
 * @param coremask_64	64-bit coremask to apply to the specified node
 */
static inline void cvmx_coremask_set64_node(struct cvmx_coremask *pcm,
					    u8 node,
					    u64 coremask_64)
{
	pcm->coremask_bitmap[CVMX_COREMASK_BMP_NODE_CORE_IDX(node, 0)] =
		coremask_64;
}

/**
 * Gets the lower 64-bits of the coremask
 *
 * @param[in] pcm - pointer to coremask
 * Return: 64-bit coremask for the first node
 */
static inline u64 cvmx_coremask_get64(const struct cvmx_coremask *pcm)
{
	return pcm->coremask_bitmap[0];
}

/**
 * Gets the lower 64-bits of the coremask for the specified node
 *
 * @param[in] pcm - pointer to coremask
 * @param node - node to get coremask for
 * Return: 64-bit coremask for the first node
 */
static inline u64 cvmx_coremask_get64_node(const struct cvmx_coremask *pcm,
					   u8 node)
{
	return pcm->coremask_bitmap[CVMX_COREMASK_BMP_NODE_CORE_IDX(node, 0)];
}

/**
 * Gets the lower 32-bits of the coremask for compatibility
 *
 * @param[in] pcm - pointer to coremask
 * Return: 32-bit coremask for the first node
 * @deprecated This function is to maintain compatibility with older
 *             SDK applications and may disappear at some point.
 * This function is not compatible with the CN78XX or any other
 * Octeon device with more than 32 cores.
 */
static inline u32 cvmx_coremask_get32(const struct cvmx_coremask *pcm)
{
	return pcm->coremask_bitmap[0] & 0xffffffff;
}

/*
 * cvmx_coremask_cmp() returns an integer less than, equal to, or
 * greater than zero if *pcm1 is found, respectively, to be less than,
 * to match, or be greater than *pcm2.
 */
static inline int cvmx_coremask_cmp(const struct cvmx_coremask *pcm1,
				    const struct cvmx_coremask *pcm2)
{
	int i;

	/* Start from highest node for arithemtically correct result */
	for (i = CVMX_COREMASK_USED_BMPSZ - 1; i >= 0; i--)
		if (pcm1->coremask_bitmap[i] != pcm2->coremask_bitmap[i]) {
			return (pcm1->coremask_bitmap[i] >
				pcm2->coremask_bitmap[i]) ? 1 : -1;
		}

	return 0;
}

/*
 * cvmx_coremask_OPx(pcm1, pcm2[, pcm3]), where OPx can be
 * - and
 * - or
 * - xor
 * - not
 * ...
 * For binary operators, pcm3 <-- pcm1 OPX pcm2.
 * For unaries, pcm2 <-- OPx pcm1.
 */
#define CVMX_COREMASK_BINARY_DEFUN(binary_op, op)		\
	static inline int cvmx_coremask_##binary_op(		\
		struct cvmx_coremask *pcm1,				\
		const struct cvmx_coremask *pcm2,			\
		const struct cvmx_coremask *pcm3)			\
	{							\
		int i;						\
								\
		for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)	\
			pcm1->coremask_bitmap[i] =		\
				pcm2->coremask_bitmap[i]	\
				op				\
				pcm3->coremask_bitmap[i];	\
								\
		return 0;					\
	}

#define CVMX_COREMASK_UNARY_DEFUN(unary_op, op)			\
	static inline int cvmx_coremask_##unary_op(		\
		struct cvmx_coremask *pcm1,				\
		const struct cvmx_coremask *pcm2)			\
	{							\
		int i;						\
								\
		for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)	\
			pcm1->coremask_bitmap[i] =		\
				op				\
				pcm2->coremask_bitmap[i];	\
								\
		return 0;					\
	}

/* cvmx_coremask_and(pcm1, pcm2, pcm3): pcm1 = pmc2 & pmc3 */
CVMX_COREMASK_BINARY_DEFUN(and, &)
/* cvmx_coremask_or(pcm1, pcm2, pcm3): pcm1 = pmc2 | pmc3  */
CVMX_COREMASK_BINARY_DEFUN(or, |)
/* cvmx_coremask_xor(pcm1, pcm2, pcm3): pcm1 = pmc2 ^ pmc3 */
CVMX_COREMASK_BINARY_DEFUN(xor, ^)
/* cvmx_coremask_maskoff(pcm1, pcm2, pcm3): pcm1 = pmc2 & ~pmc3 */
CVMX_COREMASK_BINARY_DEFUN(maskoff, & ~)
/* cvmx_coremask_not(pcm1, pcm2): pcm1 = ~pcm2       */
CVMX_COREMASK_UNARY_DEFUN(not, ~)
/* cvmx_coremask_fill(pcm1, pcm2): pcm1 = -1      */
CVMX_COREMASK_UNARY_DEFUN(fill, -1 |)
/* cvmx_coremask_clear(pcm1, pcm2): pcm1 = 0     */
CVMX_COREMASK_UNARY_DEFUN(clear, 0 &)
/* cvmx_coremask_dup(pcm1, pcm2): pcm1 = pcm2       */
CVMX_COREMASK_UNARY_DEFUN(dup, +)

/*
 * Macros using the unary functions defined w/
 * CVMX_COREMASK_UNARY_DEFUN
 * - set *pcm to its complement
 * - set all bits in *pcm to 0
 * - set all (valid) bits in *pcm to 1
 */
#define cvmx_coremask_complement(pcm)	cvmx_coremask_not(pcm, pcm)
/* On clear, even clear the unused bits */
#define cvmx_coremask_clear_all(pcm)					\
	*(pcm) = (struct cvmx_coremask)CVMX_COREMASK_EMPTY
#define cvmx_coremask_set_all(pcm)	cvmx_coremask_fill(pcm, NULL)

/*
 * convert a string of hex digits to struct cvmx_coremask
 *
 * @param pcm
 * @param hexstr can be
 *	- "[1-9A-Fa-f][0-9A-Fa-f]*", or
 *	- "-1" to set the bits for all the cores.
 * return
 *	 0 for success,
 *	-1 for string too long (i.e., hexstr takes more bits than
 *	   CVMX_MIPS_MAX_CORES),
 *	-2 for conversion problems from hex string to an unsigned
 *	   long long, e.g., non-hex char in hexstr, and
 *	-3 for hexstr starting with '0'.
 * NOTE:
 *	This function clears the bitmask in *pcm before the conversion.
 */
int cvmx_coremask_str2bmp(struct cvmx_coremask *pcm, char *hexstr);

/*
 * convert a struct cvmx_coremask to a string of hex digits
 *
 * @param pcm
 * @param hexstr is "[1-9A-Fa-f][0-9A-Fa-f]*"
 *
 * return 0.
 */
int cvmx_coremask_bmp2str(const struct cvmx_coremask *pcm, char *hexstr);

/*
 * Returns the index of the lowest bit in a coremask holder.
 */
static inline int cvmx_coremask_lowest_bit(u64 h)
{
	return __builtin_ctzll(h);
}

/*
 * Returns the 0-based index of the highest bit in a coremask holder.
 */
static inline int cvmx_coremask_highest_bit(u64 h)
{
	return (64 - __builtin_clzll(h) - 1);
}

/**
 * Returns the last core within the coremask and -1 when the coremask
 * is empty.
 *
 * @param[in] pcm - pointer to coremask
 * @returns last core set in the coremask or -1 if all clear
 *
 */
static inline int cvmx_coremask_get_last_core(const struct cvmx_coremask *pcm)
{
	int i;
	int found = -1;

	for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++) {
		if (pcm->coremask_bitmap[i])
			found = i;
	}

	if (found == -1)
		return -1;

	return found * CVMX_COREMASK_HLDRSZ +
		cvmx_coremask_highest_bit(pcm->coremask_bitmap[found]);
}

/**
 * Returns the first core within the coremask and -1 when the coremask
 * is empty.
 *
 * @param[in] pcm - pointer to coremask
 * @returns first core set in the coremask or -1 if all clear
 *
 */
static inline int cvmx_coremask_get_first_core(const struct cvmx_coremask *pcm)
{
	int i;

	for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)
		if (pcm->coremask_bitmap[i])
			break;

	if (i == CVMX_COREMASK_USED_BMPSZ)
		return -1;

	return i * CVMX_COREMASK_HLDRSZ +
		cvmx_coremask_lowest_bit(pcm->coremask_bitmap[i]);
}

/**
 * Given a core and coremask, return the next available core in the coremask
 * or -1 if none are available.
 *
 * @param core - starting core to check (can be -1 for core 0)
 * @param pcm - pointer to coremask to check for the next core.
 *
 * Return: next core following the core parameter or -1 if no more cores.
 */
static inline int cvmx_coremask_next_core(int core,
					  const struct cvmx_coremask *pcm)
{
	int n, i;

	core++;
	n = core % CVMX_COREMASK_HLDRSZ;
	i = core / CVMX_COREMASK_HLDRSZ;

	if (pcm->coremask_bitmap[i] != 0) {
		for (; n < CVMX_COREMASK_HLDRSZ; n++)
			if (pcm->coremask_bitmap[i] & (1ULL << n))
				return ((i * CVMX_COREMASK_HLDRSZ) + n);
	}

	for (i = i + 1; i < CVMX_COREMASK_USED_BMPSZ; i++) {
		if (pcm->coremask_bitmap[i] != 0)
			return (i * CVMX_COREMASK_HLDRSZ) +
				cvmx_coremask_lowest_bit(pcm->coremask_bitmap[i]);
	}
	return -1;
}

/**
 * Compute coremask for count cores starting with start_core.
 * Note that the coremask for multi-node processors may have
 * gaps.
 *
 * @param[out]  pcm        pointer to core mask data structure
 * @param	start_core starting code number
 * @param       count      number of cores
 *
 */
static inline void cvmx_coremask_set_cores(struct cvmx_coremask *pcm,
					   unsigned int start_core,
					   unsigned int count)
{
	int node;
	int core;	/** Current core in node */
	int cores_in_node;
	int i;

	assert(CVMX_MAX_CORES < CVMX_COREMASK_HLDRSZ);
	node = start_core >> CVMX_NODE_NO_SHIFT;
	core = start_core & ((1 << CVMX_NODE_NO_SHIFT) - 1);
	assert(core < CVMX_MAX_CORES);

	cvmx_coremask_clear_all(pcm);
	while (count > 0) {
		if (count + core > CVMX_MAX_CORES)
			cores_in_node = CVMX_MAX_CORES - core;
		else
			cores_in_node = count;

		i = CVMX_COREMASK_BMP_NODE_CORE_IDX(node, core);
		pcm->coremask_bitmap[i] = ((1ULL << cores_in_node) - 1) << core;
		count -= cores_in_node;
		core = 0;
		node++;
	}
}

/**
 * Makes a copy of a coremask
 *
 * @param[out] dest - pointer to destination coremask
 * @param[in]  src  - pointer to source coremask
 */
static inline void cvmx_coremask_copy(struct cvmx_coremask *dest,
				      const struct cvmx_coremask *src)
{
	memcpy(dest, src, sizeof(*dest));
}

/**
 * Test to see if the specified core is first core in coremask.
 *
 * @param[in]  pcm  pointer to the coremask to test against
 * @param[in]  core core to check
 *
 * Return:  1 if the core is first core in the coremask, 0 otherwise
 *
 */
static inline int cvmx_coremask_is_core_first_core(const struct cvmx_coremask *pcm,
						   unsigned int core)
{
	int n, i;

	n = core / CVMX_COREMASK_HLDRSZ;

	for (i = 0; i < n; i++)
		if (pcm->coremask_bitmap[i] != 0)
			return 0;

	/* From now on we only care about the core number within an entry */
	core &= (CVMX_COREMASK_HLDRSZ - 1);
	if (__builtin_ffsll(pcm->coremask_bitmap[n]) < (core + 1))
		return 0;

	return (__builtin_ffsll(pcm->coremask_bitmap[n]) == core + 1);
}

/*
 * NOTE:
 * cvmx_coremask_is_first_core() was retired due to improper usage.
 * For inquiring about the current core being the initializing
 * core for an application, use cvmx_is_init_core().
 * For simply inquring if the current core is numerically
 * lowest in a given mask, use :
 *	cvmx_coremask_is_core_first_core( pcm, dvmx_get_core_num())
 */

/**
 * Returns the number of 1 bits set in a coremask
 *
 * @param[in] pcm - pointer to core mask
 *
 * Return: number of bits set in the coremask
 */
static inline int cvmx_coremask_get_core_count(const struct cvmx_coremask *pcm)
{
	int i;
	int count = 0;

	for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)
		count += __builtin_popcountll(pcm->coremask_bitmap[i]);

	return count;
}

/**
 * For multi-node systems, return the node a core belongs to.
 *
 * @param core - core number (0-1023)
 *
 * Return: node number core belongs to
 */
static inline int cvmx_coremask_core_to_node(int core)
{
	return (core >> CVMX_NODE_NO_SHIFT) & CVMX_NODE_MASK;
}

/**
 * Given a core number on a multi-node system, return the core number for a
 * particular node.
 *
 * @param core - global core number
 *
 * @returns core number local to the node.
 */
static inline int cvmx_coremask_core_on_node(int core)
{
	return (core & ((1 << CVMX_NODE_NO_SHIFT) - 1));
}

/**
 * Returns if one coremask is a subset of another coremask
 *
 * @param main - main coremask to test
 * @param subset - subset coremask to test
 *
 * Return: 0 if the subset contains cores not in the main coremask or 1 if
 *         the subset is fully contained in the main coremask.
 */
static inline int cvmx_coremask_is_subset(const struct cvmx_coremask *main,
					  const struct cvmx_coremask *subset)
{
	int i;

	for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)
		if ((main->coremask_bitmap[i] & subset->coremask_bitmap[i]) !=
		    subset->coremask_bitmap[i])
			return 0;
	return 1;
}

/**
 * Returns if one coremask intersects another coremask
 *
 * @param c1 - main coremask to test
 * @param c2 - subset coremask to test
 *
 * Return: 1 if coremask c1 intersects coremask c2, 0 if they are exclusive
 */
static inline int cvmx_coremask_intersects(const struct cvmx_coremask *c1,
					   const struct cvmx_coremask *c2)
{
	int i;

	for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)
		if ((c1->coremask_bitmap[i] & c2->coremask_bitmap[i]) != 0)
			return 1;
	return 0;
}

/**
 * Masks a single node of a coremask
 *
 * @param pcm - coremask to mask [inout]
 * @param node       - node number to mask against
 */
static inline void cvmx_coremask_mask_node(struct cvmx_coremask *pcm, int node)
{
	int i;

	for (i = 0; i < CVMX_COREMASK_BMP_NODE_CORE_IDX(node, 0); i++)
		pcm->coremask_bitmap[i] = 0;

	for (i = CVMX_COREMASK_BMP_NODE_CORE_IDX(node + 1, 0);
	     i < CVMX_COREMASK_USED_BMPSZ; i++)
		pcm->coremask_bitmap[i] = 0;
}

/**
 * Prints out a coremask in the form of node X: 0x... 0x...
 *
 * @param[in] pcm - pointer to core mask
 *
 * Return: nothing
 */
void cvmx_coremask_print(const struct cvmx_coremask *pcm);

static inline void cvmx_coremask_dprint(const struct cvmx_coremask *pcm)
{
#if defined(DEBUG)
	cvmx_coremask_print(pcm);
#endif
}

struct cvmx_coremask *octeon_get_available_coremask(struct cvmx_coremask *pcm);

int validate_coremask(struct cvmx_coremask *pcm);

#endif /* __CVMX_COREMASK_H__ */
