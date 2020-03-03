/*
 * comedi_buf.c
 *
 * COMEDI - Linux Control and Measurement Device Interface
 * Copyright (C) 1997-2000 David A. Schleef <ds@schleef.org>
 * Copyright (C) 2002 Frank Mori Hess <fmhess@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/vmalloc.h>
#include <linux/slab.h>

#include "comedidev.h"
#include "comedi_internal.h"

#ifdef PAGE_KERNEL_NOCACHE
#define COMEDI_PAGE_PROTECTION		PAGE_KERNEL_NOCACHE
#else
#define COMEDI_PAGE_PROTECTION		PAGE_KERNEL
#endif

static void comedi_buf_map_kref_release(struct kref *kref)
{
	struct comedi_buf_map *bm =
		container_of(kref, struct comedi_buf_map, refcount);
	struct comedi_buf_page *buf;
	unsigned int i;

	if (bm->page_list) {
		for (i = 0; i < bm->n_pages; i++) {
			buf = &bm->page_list[i];
			clear_bit(PG_reserved,
				  &(virt_to_page(buf->virt_addr)->flags));
			if (bm->dma_dir != DMA_NONE) {
#ifdef CONFIG_HAS_DMA
				dma_free_coherent(bm->dma_hw_dev,
						  PAGE_SIZE,
						  buf->virt_addr,
						  buf->dma_addr);
#endif
			} else {
				free_page((unsigned long)buf->virt_addr);
			}
		}
		vfree(bm->page_list);
	}
	if (bm->dma_dir != DMA_NONE)
		put_device(bm->dma_hw_dev);
	kfree(bm);
}

static void __comedi_buf_free(struct comedi_device *dev,
			      struct comedi_subdevice *s)
{
	struct comedi_async *async = s->async;
	struct comedi_buf_map *bm;
	unsigned long flags;

	if (async->prealloc_buf) {
		vunmap(async->prealloc_buf);
		async->prealloc_buf = NULL;
		async->prealloc_bufsz = 0;
	}

	spin_lock_irqsave(&s->spin_lock, flags);
	bm = async->buf_map;
	async->buf_map = NULL;
	spin_unlock_irqrestore(&s->spin_lock, flags);
	comedi_buf_map_put(bm);
}

static void __comedi_buf_alloc(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       unsigned n_pages)
{
	struct comedi_async *async = s->async;
	struct page **pages = NULL;
	struct comedi_buf_map *bm;
	struct comedi_buf_page *buf;
	unsigned long flags;
	unsigned i;

	if (!IS_ENABLED(CONFIG_HAS_DMA) && s->async_dma_dir != DMA_NONE) {
		dev_err(dev->class_dev,
			"dma buffer allocation not supported\n");
		return;
	}

	bm = kzalloc(sizeof(*async->buf_map), GFP_KERNEL);
	if (!bm)
		return;

	kref_init(&bm->refcount);
	spin_lock_irqsave(&s->spin_lock, flags);
	async->buf_map = bm;
	spin_unlock_irqrestore(&s->spin_lock, flags);
	bm->dma_dir = s->async_dma_dir;
	if (bm->dma_dir != DMA_NONE)
		/* Need ref to hardware device to free buffer later. */
		bm->dma_hw_dev = get_device(dev->hw_dev);

	bm->page_list = vzalloc(sizeof(*buf) * n_pages);
	if (bm->page_list)
		pages = vmalloc(sizeof(struct page *) * n_pages);

	if (!pages)
		return;

	for (i = 0; i < n_pages; i++) {
		buf = &bm->page_list[i];
		if (bm->dma_dir != DMA_NONE)
#ifdef CONFIG_HAS_DMA
			buf->virt_addr = dma_alloc_coherent(bm->dma_hw_dev,
							    PAGE_SIZE,
							    &buf->dma_addr,
							    GFP_KERNEL |
							    __GFP_COMP);
#else
			break;
#endif
		else
			buf->virt_addr = (void *)get_zeroed_page(GFP_KERNEL);
		if (!buf->virt_addr)
			break;

		set_bit(PG_reserved, &(virt_to_page(buf->virt_addr)->flags));

		pages[i] = virt_to_page(buf->virt_addr);
	}
	spin_lock_irqsave(&s->spin_lock, flags);
	bm->n_pages = i;
	spin_unlock_irqrestore(&s->spin_lock, flags);

	/* vmap the prealloc_buf if all the pages were allocated */
	if (i == n_pages)
		async->prealloc_buf = vmap(pages, n_pages, VM_MAP,
					   COMEDI_PAGE_PROTECTION);

	vfree(pages);
}

void comedi_buf_map_get(struct comedi_buf_map *bm)
{
	if (bm)
		kref_get(&bm->refcount);
}

int comedi_buf_map_put(struct comedi_buf_map *bm)
{
	if (bm)
		return kref_put(&bm->refcount, comedi_buf_map_kref_release);
	return 1;
}

/* returns s->async->buf_map and increments its kref refcount */
struct comedi_buf_map *
comedi_buf_map_from_subdev_get(struct comedi_subdevice *s)
{
	struct comedi_async *async = s->async;
	struct comedi_buf_map *bm = NULL;
	unsigned long flags;

	if (!async)
		return NULL;

	spin_lock_irqsave(&s->spin_lock, flags);
	bm = async->buf_map;
	/* only want it if buffer pages allocated */
	if (bm && bm->n_pages)
		comedi_buf_map_get(bm);
	else
		bm = NULL;
	spin_unlock_irqrestore(&s->spin_lock, flags);

	return bm;
}

bool comedi_buf_is_mmapped(struct comedi_subdevice *s)
{
	struct comedi_buf_map *bm = s->async->buf_map;

	return bm && (atomic_read(&bm->refcount.refcount) > 1);
}

int comedi_buf_alloc(struct comedi_device *dev, struct comedi_subdevice *s,
		     unsigned long new_size)
{
	struct comedi_async *async = s->async;

	/* Round up new_size to multiple of PAGE_SIZE */
	new_size = (new_size + PAGE_SIZE - 1) & PAGE_MASK;

	/* if no change is required, do nothing */
	if (async->prealloc_buf && async->prealloc_bufsz == new_size)
		return 0;

	/* deallocate old buffer */
	__comedi_buf_free(dev, s);

	/* allocate new buffer */
	if (new_size) {
		unsigned n_pages = new_size >> PAGE_SHIFT;

		__comedi_buf_alloc(dev, s, n_pages);

		if (!async->prealloc_buf) {
			/* allocation failed */
			__comedi_buf_free(dev, s);
			return -ENOMEM;
		}
	}
	async->prealloc_bufsz = new_size;

	return 0;
}

void comedi_buf_reset(struct comedi_subdevice *s)
{
	struct comedi_async *async = s->async;

	async->buf_write_alloc_count = 0;
	async->buf_write_count = 0;
	async->buf_read_alloc_count = 0;
	async->buf_read_count = 0;

	async->buf_write_ptr = 0;
	async->buf_read_ptr = 0;

	async->cur_chan = 0;
	async->scans_done = 0;
	async->scan_progress = 0;
	async->munge_chan = 0;
	async->munge_count = 0;
	async->munge_ptr = 0;

	async->events = 0;
}

static unsigned int comedi_buf_write_n_available(struct comedi_subdevice *s)
{
	struct comedi_async *async = s->async;
	unsigned int free_end = async->buf_read_count + async->prealloc_bufsz;

	return free_end - async->buf_write_alloc_count;
}

/* allocates chunk for the writer from free buffer space */
unsigned int comedi_buf_write_alloc(struct comedi_subdevice *s,
				    unsigned int nbytes)
{
	struct comedi_async *async = s->async;
	unsigned int available = comedi_buf_write_n_available(s);

	if (nbytes > available)
		nbytes = available;

	async->buf_write_alloc_count += nbytes;

	/*
	 * ensure the async buffer 'counts' are read and updated
	 * before we write data to the write-alloc'ed buffer space
	 */
	smp_mb();

	return nbytes;
}
EXPORT_SYMBOL_GPL(comedi_buf_write_alloc);

/*
 * munging is applied to data by core as it passes between user
 * and kernel space
 */
static unsigned int comedi_buf_munge(struct comedi_subdevice *s,
				     unsigned int num_bytes)
{
	struct comedi_async *async = s->async;
	unsigned int count = 0;
	const unsigned num_sample_bytes = comedi_bytes_per_sample(s);

	if (!s->munge || (async->cmd.flags & CMDF_RAWDATA)) {
		async->munge_count += num_bytes;
		count = num_bytes;
	} else {
		/* don't munge partial samples */
		num_bytes -= num_bytes % num_sample_bytes;
		while (count < num_bytes) {
			int block_size = num_bytes - count;
			unsigned int buf_end;

			buf_end = async->prealloc_bufsz - async->munge_ptr;
			if (block_size > buf_end)
				block_size = buf_end;

			s->munge(s->device, s,
				 async->prealloc_buf + async->munge_ptr,
				 block_size, async->munge_chan);

			/*
			 * ensure data is munged in buffer before the
			 * async buffer munge_count is incremented
			 */
			smp_wmb();

			async->munge_chan += block_size / num_sample_bytes;
			async->munge_chan %= async->cmd.chanlist_len;
			async->munge_count += block_size;
			async->munge_ptr += block_size;
			async->munge_ptr %= async->prealloc_bufsz;
			count += block_size;
		}
	}

	return count;
}

unsigned int comedi_buf_write_n_allocated(struct comedi_subdevice *s)
{
	struct comedi_async *async = s->async;

	return async->buf_write_alloc_count - async->buf_write_count;
}

/* transfers a chunk from writer to filled buffer space */
unsigned int comedi_buf_write_free(struct comedi_subdevice *s,
				   unsigned int nbytes)
{
	struct comedi_async *async = s->async;
	unsigned int allocated = comedi_buf_write_n_allocated(s);

	if (nbytes > allocated)
		nbytes = allocated;

	async->buf_write_count += nbytes;
	async->buf_write_ptr += nbytes;
	comedi_buf_munge(s, async->buf_write_count - async->munge_count);
	if (async->buf_write_ptr >= async->prealloc_bufsz)
		async->buf_write_ptr %= async->prealloc_bufsz;

	return nbytes;
}
EXPORT_SYMBOL_GPL(comedi_buf_write_free);

unsigned int comedi_buf_read_n_available(struct comedi_subdevice *s)
{
	struct comedi_async *async = s->async;
	unsigned num_bytes;

	if (!async)
		return 0;

	num_bytes = async->munge_count - async->buf_read_count;

	/*
	 * ensure the async buffer 'counts' are read before we
	 * attempt to read data from the buffer
	 */
	smp_rmb();

	return num_bytes;
}
EXPORT_SYMBOL_GPL(comedi_buf_read_n_available);

/* allocates a chunk for the reader from filled (and munged) buffer space */
unsigned int comedi_buf_read_alloc(struct comedi_subdevice *s,
				   unsigned int nbytes)
{
	struct comedi_async *async = s->async;
	unsigned int available;

	available = async->munge_count - async->buf_read_alloc_count;
	if (nbytes > available)
		nbytes = available;

	async->buf_read_alloc_count += nbytes;

	/*
	 * ensure the async buffer 'counts' are read before we
	 * attempt to read data from the read-alloc'ed buffer space
	 */
	smp_rmb();

	return nbytes;
}
EXPORT_SYMBOL_GPL(comedi_buf_read_alloc);

static unsigned int comedi_buf_read_n_allocated(struct comedi_async *async)
{
	return async->buf_read_alloc_count - async->buf_read_count;
}

/* transfers control of a chunk from reader to free buffer space */
unsigned int comedi_buf_read_free(struct comedi_subdevice *s,
				  unsigned int nbytes)
{
	struct comedi_async *async = s->async;
	unsigned int allocated;

	/*
	 * ensure data has been read out of buffer before
	 * the async read count is incremented
	 */
	smp_mb();

	allocated = comedi_buf_read_n_allocated(async);
	if (nbytes > allocated)
		nbytes = allocated;

	async->buf_read_count += nbytes;
	async->buf_read_ptr += nbytes;
	async->buf_read_ptr %= async->prealloc_bufsz;
	return nbytes;
}
EXPORT_SYMBOL_GPL(comedi_buf_read_free);

static void comedi_buf_memcpy_to(struct comedi_subdevice *s,
				 const void *data, unsigned int num_bytes)
{
	struct comedi_async *async = s->async;
	unsigned int write_ptr = async->buf_write_ptr;

	while (num_bytes) {
		unsigned int block_size;

		if (write_ptr + num_bytes > async->prealloc_bufsz)
			block_size = async->prealloc_bufsz - write_ptr;
		else
			block_size = num_bytes;

		memcpy(async->prealloc_buf + write_ptr, data, block_size);

		data += block_size;
		num_bytes -= block_size;

		write_ptr = 0;
	}
}

static void comedi_buf_memcpy_from(struct comedi_subdevice *s,
				   void *dest, unsigned int nbytes)
{
	void *src;
	struct comedi_async *async = s->async;
	unsigned int read_ptr = async->buf_read_ptr;

	while (nbytes) {
		unsigned int block_size;

		src = async->prealloc_buf + read_ptr;

		if (nbytes >= async->prealloc_bufsz - read_ptr)
			block_size = async->prealloc_bufsz - read_ptr;
		else
			block_size = nbytes;

		memcpy(dest, src, block_size);
		nbytes -= block_size;
		dest += block_size;
		read_ptr = 0;
	}
}

/**
 * comedi_buf_write_samples - write sample data to comedi buffer
 * @s: comedi_subdevice struct
 * @data: samples
 * @nsamples: number of samples
 *
 * Writes nsamples to the comedi buffer associated with the subdevice, marks
 * it as written and updates the acquisition scan progress.
 *
 * Returns the amount of data written in bytes.
 */
unsigned int comedi_buf_write_samples(struct comedi_subdevice *s,
				      const void *data, unsigned int nsamples)
{
	unsigned int max_samples;
	unsigned int nbytes;

	/*
	 * Make sure there is enough room in the buffer for all the samples.
	 * If not, clamp the nsamples to the number that will fit, flag the
	 * buffer overrun and add the samples that fit.
	 */
	max_samples = comedi_bytes_to_samples(s,
					      comedi_buf_write_n_available(s));
	if (nsamples > max_samples) {
		dev_warn(s->device->class_dev, "buffer overrun\n");
		s->async->events |= COMEDI_CB_OVERFLOW;
		nsamples = max_samples;
	}

	if (nsamples == 0)
		return 0;

	nbytes = comedi_buf_write_alloc(s,
					comedi_samples_to_bytes(s, nsamples));
	comedi_buf_memcpy_to(s, data, nbytes);
	comedi_buf_write_free(s, nbytes);
	comedi_inc_scan_progress(s, nbytes);
	s->async->events |= COMEDI_CB_BLOCK;

	return nbytes;
}
EXPORT_SYMBOL_GPL(comedi_buf_write_samples);

/**
 * comedi_buf_read_samples - read sample data from comedi buffer
 * @s: comedi_subdevice struct
 * @data: destination
 * @nsamples: maximum number of samples to read
 *
 * Reads up to nsamples from the comedi buffer associated with the subdevice,
 * marks it as read and updates the acquisition scan progress.
 *
 * Returns the amount of data read in bytes.
 */
unsigned int comedi_buf_read_samples(struct comedi_subdevice *s,
				     void *data, unsigned int nsamples)
{
	unsigned int max_samples;
	unsigned int nbytes;

	/* clamp nsamples to the number of full samples available */
	max_samples = comedi_bytes_to_samples(s,
					      comedi_buf_read_n_available(s));
	if (nsamples > max_samples)
		nsamples = max_samples;

	if (nsamples == 0)
		return 0;

	nbytes = comedi_buf_read_alloc(s,
				       comedi_samples_to_bytes(s, nsamples));
	comedi_buf_memcpy_from(s, data, nbytes);
	comedi_buf_read_free(s, nbytes);
	comedi_inc_scan_progress(s, nbytes);
	s->async->events |= COMEDI_CB_BLOCK;

	return nbytes;
}
EXPORT_SYMBOL_GPL(comedi_buf_read_samples);
