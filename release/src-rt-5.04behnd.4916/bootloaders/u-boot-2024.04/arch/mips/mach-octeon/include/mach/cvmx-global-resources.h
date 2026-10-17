/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef _CVMX_GLOBAL_RESOURCES_T_
#define _CVMX_GLOBAL_RESOURCES_T_

#define CVMX_GLOBAL_RESOURCES_DATA_NAME "cvmx-global-resources"

/*In macros below abbreviation GR stands for global resources. */
#define CVMX_GR_TAG_INVALID                                                                        \
	cvmx_get_gr_tag('i', 'n', 'v', 'a', 'l', 'i', 'd', '.', '.', '.', '.', '.', '.', '.', '.', \
			'.')
/*Tag for pko que table range. */
#define CVMX_GR_TAG_PKO_QUEUES                                                                     \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'p', 'k', 'o', '_', 'q', 'u', 'e', 'u', 's', '.', '.', \
			'.')
/*Tag for a pko internal ports range */
#define CVMX_GR_TAG_PKO_IPORTS                                                                     \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'p', 'k', 'o', '_', 'i', 'p', 'o', 'r', 't', '.', '.', \
			'.')
#define CVMX_GR_TAG_FPA                                                                            \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'f', 'p', 'a', '.', '.', '.', '.', '.', '.', '.', '.', \
			'.')
#define CVMX_GR_TAG_FAU                                                                            \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'f', 'a', 'u', '.', '.', '.', '.', '.', '.', '.', '.', \
			'.')
#define CVMX_GR_TAG_SSO_GRP(n)                                                                     \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 's', 's', 'o', '_', '0', (n) + '0', '.', '.', '.',     \
			'.', '.', '.');
#define CVMX_GR_TAG_TIM(n)                                                                         \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 't', 'i', 'm', '_', (n) + '0', '.', '.', '.', '.',     \
			'.', '.', '.')
#define CVMX_GR_TAG_CLUSTERS(x)                                                                    \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'c', 'l', 'u', 's', 't', 'e', 'r', '_', (x + '0'),     \
			'.', '.', '.')
#define CVMX_GR_TAG_CLUSTER_GRP(x)                                                                 \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'c', 'l', 'g', 'r', 'p', '_', (x + '0'), '.', '.',     \
			'.', '.', '.')
#define CVMX_GR_TAG_STYLE(x)                                                                       \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 's', 't', 'y', 'l', 'e', '_', (x + '0'), '.', '.',     \
			'.', '.', '.')
#define CVMX_GR_TAG_QPG_ENTRY(x)                                                                   \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'q', 'p', 'g', 'e', 't', '_', (x + '0'), '.', '.',     \
			'.', '.', '.')
#define CVMX_GR_TAG_BPID(x)                                                                        \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'b', 'p', 'i', 'd', 's', '_', (x + '0'), '.', '.',     \
			'.', '.', '.')
#define CVMX_GR_TAG_MTAG_IDX(x)                                                                    \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'm', 't', 'a', 'g', 'x', '_', (x + '0'), '.', '.',     \
			'.', '.', '.')
#define CVMX_GR_TAG_PCAM(x, y, z)                                                                  \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'p', 'c', 'a', 'm', '_', (x + '0'), (y + '0'),         \
			(z + '0'), '.', '.', '.', '.')

#define CVMX_GR_TAG_CIU3_IDT(_n)                                                                   \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'c', 'i', 'u', '3', '_', ((_n) + '0'), '_', 'i', 'd',  \
			't', '.', '.')

/* Allocation of the 512 SW INTSTs (in the  12 bit SW INTSN space) */
#define CVMX_GR_TAG_CIU3_SWINTSN(_n)                                                               \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'c', 'i', 'u', '3', '_', ((_n) + '0'), '_', 's', 'w',  \
			'i', 's', 'n')

#define TAG_INIT_PART(A, B, C, D, E, F, G, H)                                                      \
	((((u64)(A) & 0xff) << 56) | (((u64)(B) & 0xff) << 48) | (((u64)(C) & 0xff) << 40) |             \
	 (((u64)(D) & 0xff) << 32) | (((u64)(E) & 0xff) << 24) | (((u64)(F) & 0xff) << 16) |             \
	 (((u64)(G) & 0xff) << 8) | (((u64)(H) & 0xff)))

struct global_resource_tag {
	u64 lo;
	u64 hi;
};

enum cvmx_resource_err { CVMX_RESOURCE_ALLOC_FAILED = -1, CVMX_RESOURCE_ALREADY_RESERVED = -2 };

/*
 * @INTERNAL
 * Creates a tag from the specified characters.
 */
static inline struct global_resource_tag cvmx_get_gr_tag(char a, char b, char c, char d, char e,
							 char f, char g, char h, char i, char j,
							 char k, char l, char m, char n, char o,
							 char p)
{
	struct global_resource_tag tag;

	tag.lo = TAG_INIT_PART(a, b, c, d, e, f, g, h);
	tag.hi = TAG_INIT_PART(i, j, k, l, m, n, o, p);
	return tag;
}

static inline int cvmx_gr_same_tag(struct global_resource_tag gr1, struct global_resource_tag gr2)
{
	return (gr1.hi == gr2.hi) && (gr1.lo == gr2.lo);
}

/*
 * @INTERNAL
 * Creates a global resource range that can hold the specified number of
 * elements
 * @param tag is the tag of the range. The taga is created using the method
 * cvmx_get_gr_tag()
 * @param nelements is the number of elements to be held in the resource range.
 */
int cvmx_create_global_resource_range(struct global_resource_tag tag, int nelements);

/*
 * @INTERNAL
 * Allocate nelements in the global resource range with the specified tag. It
 * is assumed that prior
 * to calling this the global resource range has already been created using
 * cvmx_create_global_resource_range().
 * @param tag is the tag of the global resource range.
 * @param nelements is the number of elements to be allocated.
 * @param owner is a 64 bit number that identifes the owner of this range.
 * @aligment specifes the required alignment of the returned base number.
 * Return: returns the base of the allocated range. -1 return value indicates
 * failure.
 */
int cvmx_allocate_global_resource_range(struct global_resource_tag tag, u64 owner, int nelements,
					int alignment);

/*
 * @INTERNAL
 * Allocate nelements in the global resource range with the specified tag.
 * The elements allocated need not be contiguous. It is assumed that prior to
 * calling this the global resource range has already
 * been created using cvmx_create_global_resource_range().
 * @param tag is the tag of the global resource range.
 * @param nelements is the number of elements to be allocated.
 * @param owner is a 64 bit number that identifes the owner of the allocated
 * elements.
 * @param allocated_elements returns indexs of the allocated entries.
 * Return: returns 0 on success and -1 on failure.
 */
int cvmx_resource_alloc_many(struct global_resource_tag tag, u64 owner, int nelements,
			     int allocated_elements[]);
int cvmx_resource_alloc_reverse(struct global_resource_tag, u64 owner);
/*
 * @INTERNAL
 * Reserve nelements starting from base in the global resource range with the
 * specified tag.
 * It is assumed that prior to calling this the global resource range has
 * already been created using cvmx_create_global_resource_range().
 * @param tag is the tag of the global resource range.
 * @param nelements is the number of elements to be allocated.
 * @param owner is a 64 bit number that identifes the owner of this range.
 * @base specifies the base start of nelements.
 * Return: returns the base of the allocated range. -1 return value indicates
 * failure.
 */
int cvmx_reserve_global_resource_range(struct global_resource_tag tag, u64 owner, int base,
				       int nelements);
/*
 * @INTERNAL
 * Free nelements starting at base in the global resource range with the
 * specified tag.
 * @param tag is the tag of the global resource range.
 * @param base is the base number
 * @param nelements is the number of elements that are to be freed.
 * Return: returns 0 if successful and -1 on failure.
 */
int cvmx_free_global_resource_range_with_base(struct global_resource_tag tag, int base,
					      int nelements);

/*
 * @INTERNAL
 * Free nelements with the bases specified in bases[] with the
 * specified tag.
 * @param tag is the tag of the global resource range.
 * @param bases is an array containing the bases to be freed.
 * @param nelements is the number of elements that are to be freed.
 * Return: returns 0 if successful and -1 on failure.
 */
int cvmx_free_global_resource_range_multiple(struct global_resource_tag tag, int bases[],
					     int nelements);
/*
 * @INTERNAL
 * Free elements from the specified owner in the global resource range with the
 * specified tag.
 * @param tag is the tag of the global resource range.
 * @param owner is the owner of resources that are to be freed.
 * Return: returns 0 if successful and -1 on failure.
 */
int cvmx_free_global_resource_range_with_owner(struct global_resource_tag tag, int owner);

/*
 * @INTERNAL
 * Frees all the global resources that have been created.
 * For use only from the bootloader, when it shutdown and boots up the
 * application or kernel.
 */
int free_global_resources(void);

u64 cvmx_get_global_resource_owner(struct global_resource_tag tag, int base);
/*
 * @INTERNAL
 * Shows the global resource range with the specified tag. Use mainly for debug.
 */
void cvmx_show_global_resource_range(struct global_resource_tag tag);

/*
 * @INTERNAL
 * Shows all the global resources. Used mainly for debug.
 */
void cvmx_global_resources_show(void);

u64 cvmx_allocate_app_id(void);
u64 cvmx_get_app_id(void);

#endif
