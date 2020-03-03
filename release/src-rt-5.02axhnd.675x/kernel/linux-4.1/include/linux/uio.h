/*
 *	Berkeley style UIO structures	-	Alan Cox 1994.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef __LINUX_UIO_H
#define __LINUX_UIO_H

#include <linux/kernel.h>
#include <uapi/linux/uio.h>

struct page;

struct kvec {
	void *iov_base; /* and that should *never* hold a userland pointer */
	size_t iov_len;
};

enum {
	ITER_IOVEC = 0,
	ITER_KVEC = 2,
	ITER_BVEC = 4,
};

struct iov_iter {
	int type;
	size_t iov_offset;
	size_t count;
	union {
		const struct iovec *iov;
		const struct kvec *kvec;
		const struct bio_vec *bvec;
	};
	unsigned long nr_segs;
};

/*
 * Total number of bytes covered by an iovec.
 *
 * NOTE that it is not safe to use this function until all the iovec's
 * segment lengths have been validated.  Because the individual lengths can
 * overflow a size_t when added together.
 */
static inline size_t iov_length(const struct iovec *iov, unsigned long nr_segs)
{
	unsigned long seg;
	size_t ret = 0;

	for (seg = 0; seg < nr_segs; seg++)
		ret += iov[seg].iov_len;
	return ret;
}

static inline struct iovec iov_iter_iovec(const struct iov_iter *iter)
{
	return (struct iovec) {
		.iov_base = iter->iov->iov_base + iter->iov_offset,
		.iov_len = min(iter->count,
			       iter->iov->iov_len - iter->iov_offset),
	};
}

#define iov_for_each(iov, iter, start)				\
	if (!((start).type & ITER_BVEC))			\
	for (iter = (start);					\
	     (iter).count &&					\
	     ((iov = iov_iter_iovec(&(iter))), 1);		\
	     iov_iter_advance(&(iter), (iov).iov_len))

unsigned long iov_shorten(struct iovec *iov, unsigned long nr_segs, size_t to);

size_t iov_iter_copy_from_user_atomic(struct page *page,
		struct iov_iter *i, unsigned long offset, size_t bytes);
void iov_iter_advance(struct iov_iter *i, size_t bytes);
int iov_iter_fault_in_readable(struct iov_iter *i, size_t bytes);
#define iov_iter_fault_in_multipages_readable iov_iter_fault_in_readable
size_t iov_iter_single_seg_count(const struct iov_iter *i);
size_t copy_page_to_iter(struct page *page, size_t offset, size_t bytes,
			 struct iov_iter *i);
size_t copy_page_from_iter(struct page *page, size_t offset, size_t bytes,
			 struct iov_iter *i);
size_t copy_to_iter(void *addr, size_t bytes, struct iov_iter *i);
size_t copy_from_iter(void *addr, size_t bytes, struct iov_iter *i);
size_t copy_from_iter_nocache(void *addr, size_t bytes, struct iov_iter *i);
size_t iov_iter_zero(size_t bytes, struct iov_iter *);
unsigned long iov_iter_alignment(const struct iov_iter *i);
void iov_iter_init(struct iov_iter *i, int direction, const struct iovec *iov,
			unsigned long nr_segs, size_t count);
void iov_iter_kvec(struct iov_iter *i, int direction, const struct kvec *kvec,
			unsigned long nr_segs, size_t count);
void iov_iter_bvec(struct iov_iter *i, int direction, const struct bio_vec *bvec,
			unsigned long nr_segs, size_t count);
ssize_t iov_iter_get_pages(struct iov_iter *i, struct page **pages,
			size_t maxsize, unsigned maxpages, size_t *start);
ssize_t iov_iter_get_pages_alloc(struct iov_iter *i, struct page ***pages,
			size_t maxsize, size_t *start);
int iov_iter_npages(const struct iov_iter *i, int maxpages);

const void *dup_iter(struct iov_iter *new, struct iov_iter *old, gfp_t flags);

static inline size_t iov_iter_count(struct iov_iter *i)
{
	return i->count;
}

static inline bool iter_is_iovec(struct iov_iter *i)
{
	return !(i->type & (ITER_BVEC | ITER_KVEC));
}

/*
 * Get one of READ or WRITE out of iter->type without any other flags OR'd in
 * with it.
 *
 * The ?: is just for type safety.
 */
#define iov_iter_rw(i) ((0 ? (struct iov_iter *)0 : (i))->type & RW_MASK)

/*
 * Cap the iov_iter by given limit; note that the second argument is
 * *not* the new size - it's upper limit for such.  Passing it a value
 * greater than the amount of data in iov_iter is fine - it'll just do
 * nothing in that case.
 */
static inline void iov_iter_truncate(struct iov_iter *i, u64 count)
{
	/*
	 * count doesn't have to fit in size_t - comparison extends both
	 * operands to u64 here and any value that would be truncated by
	 * conversion in assignement is by definition greater than all
	 * values of size_t, including old i->count.
	 */
	if (i->count > count)
		i->count = count;
}

/*
 * reexpand a previously truncated iterator; count must be no more than how much
 * we had shrunk it.
 */
static inline void iov_iter_reexpand(struct iov_iter *i, size_t count)
{
	i->count = count;
}
size_t csum_and_copy_to_iter(void *addr, size_t bytes, __wsum *csum, struct iov_iter *i);
size_t csum_and_copy_from_iter(void *addr, size_t bytes, __wsum *csum, struct iov_iter *i);

int import_iovec(int type, const struct iovec __user * uvector,
		 unsigned nr_segs, unsigned fast_segs,
		 struct iovec **iov, struct iov_iter *i);

#ifdef CONFIG_COMPAT
struct compat_iovec;
int compat_import_iovec(int type, const struct compat_iovec __user * uvector,
		 unsigned nr_segs, unsigned fast_segs,
		 struct iovec **iov, struct iov_iter *i);
#endif

int import_single_range(int type, void __user *buf, size_t len,
		 struct iovec *iov, struct iov_iter *i);

#endif
