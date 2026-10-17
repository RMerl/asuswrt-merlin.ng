/* SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause) */
#ifndef LIBFDT_INTERNAL_H
#define LIBFDT_INTERNAL_H
/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 */
#include <fdt.h>

#define FDT_ALIGN(x, a)		(((x) + (a) - 1) & ~((a) - 1))
#define FDT_TAGALIGN(x)		(FDT_ALIGN((x), FDT_TAGSIZE))

int fdt_ro_probe_(const void *fdt);
#define FDT_RO_PROBE(fdt)					\
	{							\
		int totalsize_;					\
		if (fdt_chk_basic()) {				\
			totalsize_ = fdt_ro_probe_(fdt);	\
			if (totalsize_ < 0)			\
				return totalsize_;		\
		}						\
	}

int fdt_check_node_offset_(const void *fdt, int offset);
int fdt_check_prop_offset_(const void *fdt, int offset);
const char *fdt_find_string_(const char *strtab, int tabsize, const char *s);
int fdt_node_end_offset_(void *fdt, int nodeoffset);

static inline const void *fdt_offset_ptr_(const void *fdt, int offset)
{
	return (const char *)fdt + fdt_off_dt_struct(fdt) + offset;
}

static inline void *fdt_offset_ptr_w_(void *fdt, int offset)
{
	return (void *)(uintptr_t)fdt_offset_ptr_(fdt, offset);
}

static inline const struct fdt_reserve_entry *fdt_mem_rsv_(const void *fdt, int n)
{
	const struct fdt_reserve_entry *rsv_table =
		(const struct fdt_reserve_entry *)
		((const char *)fdt + fdt_off_mem_rsvmap(fdt));

	return rsv_table + n;
}
static inline struct fdt_reserve_entry *fdt_mem_rsv_w_(void *fdt, int n)
{
	return (void *)(uintptr_t)fdt_mem_rsv_(fdt, n);
}

#define FDT_SW_MAGIC		(~FDT_MAGIC)

/**********************************************************************/
/* Checking controls                                                  */
/**********************************************************************/

#ifndef FDT_ASSUME_MASK
#define FDT_ASSUME_MASK 0
#endif

/*
 * Defines assumptions which can be enabled. Each of these can be enabled
 * individually. For maximum saftey, don't enable any assumptions!
 *
 * For minimal code size and no safety, use FDT_ASSUME_PERFECT at your own risk.
 * You should have another method of validating the device tree, such as a
 * signature or hash check before using libfdt.
 *
 * For situations where security is not a concern it may be safe to enable
 * FDT_ASSUME_FRIENDLY.
 */
enum {
	/*
	 * This does essentially no checks. Only the latest device-tree
	 * version is correctly handled. Incosistencies or errors in the device
	 * tree may cause undefined behaviour or crashes.
	 *
	 * If an error occurs when modifying the tree it may leave the tree in
	 * an intermediate (but valid) state. As an example, adding a property
	 * where there is insufficient space may result in the property name
	 * being added to the string table even though the property itself is
	 * not added to the struct section.
	 *
	 * Only use this if you have a fully validated device tree with
	 * the latest supported version and wish to minimise code size.
	 */
	FDT_ASSUME_PERFECT	= 0xff,

	/*
	 * This assumes that the device tree is sane. i.e. header metadata
	 * and basic hierarchy are correct.
	 *
	 * These checks will be sufficient if you have a valid device tree with
	 * no internal inconsistencies. With this assumption, libfdt will
	 * generally not return -FDT_ERR_INTERNAL, -FDT_ERR_BADLAYOUT, etc.
	 */
	FDT_ASSUME_SANE		= 1 << 0,

	/*
	 * This disables checks for device-tree version and removes all code
	 * which handles older versions.
	 *
	 * Only enable this if you know you have a device tree with the latest
	 * version.
	 */
	FDT_ASSUME_LATEST	= 1 << 1,

	/*
	 * This disables any extensive checking of parameters and the device
	 * tree, making various assumptions about correctness. Normal device
	 * trees produced by libfdt and the compiler should be handled safely.
	 * Malicious device trees and complete garbage may cause libfdt to
	 * behave badly or crash.
	 */
	FDT_ASSUME_FRIENDLY	= 1 << 2,
};

/** fdt_chk_basic() - see if basic checking of params and DT data is enabled */
static inline bool fdt_chk_basic(void)
{
	return !(FDT_ASSUME_MASK & FDT_ASSUME_SANE);
}

/** fdt_chk_version() - see if we need to handle old versions of the DT */
static inline bool fdt_chk_version(void)
{
	return !(FDT_ASSUME_MASK & FDT_ASSUME_LATEST);
}

/** fdt_chk_extra() - see if extra checking is enabled */
static inline bool fdt_chk_extra(void)
{
	return !(FDT_ASSUME_MASK & FDT_ASSUME_FRIENDLY);
}

#endif /* LIBFDT_INTERNAL_H */
