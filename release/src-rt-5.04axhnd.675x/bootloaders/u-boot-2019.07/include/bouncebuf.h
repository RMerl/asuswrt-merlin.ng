/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#ifndef __INCLUDE_BOUNCEBUF_H__
#define __INCLUDE_BOUNCEBUF_H__

#include <linux/types.h>

/*
 * GEN_BB_READ -- Data are read from the buffer eg. by DMA hardware.
 * The source buffer is copied into the bounce buffer (if unaligned, otherwise
 * the source buffer is used directly) upon start() call, then the operation
 * requiring the aligned transfer happens, then the bounce buffer is lost upon
 * stop() call.
 */
#define GEN_BB_READ	(1 << 0)
/*
 * GEN_BB_WRITE -- Data are written into the buffer eg. by DMA hardware.
 * The source buffer starts in an undefined state upon start() call, then the
 * operation requiring the aligned transfer happens, then the bounce buffer is
 * copied into the destination buffer (if unaligned, otherwise destination
 * buffer is used directly) upon stop() call.
 */
#define GEN_BB_WRITE	(1 << 1)
/*
 * GEN_BB_RW -- Data are read and written into the buffer eg. by DMA hardware.
 * The source buffer is copied into the bounce buffer (if unaligned, otherwise
 * the source buffer is used directly) upon start() call, then the  operation
 * requiring the aligned transfer happens, then the bounce buffer is  copied
 * into the destination buffer (if unaligned, otherwise destination buffer is
 * used directly) upon stop() call.
 */
#define GEN_BB_RW	(GEN_BB_READ | GEN_BB_WRITE)

struct bounce_buffer {
	/* Copy of data parameter passed to start() */
	void *user_buffer;
	/*
	 * DMA-aligned buffer. This field is always set to the value that
	 * should be used for DMA; either equal to .user_buffer, or to a
	 * freshly allocated aligned buffer.
	 */
	void *bounce_buffer;
	/* Copy of len parameter passed to start() */
	size_t len;
	/* DMA-aligned buffer length */
	size_t len_aligned;
	/* Copy of flags parameter passed to start() */
	unsigned int flags;
};

/**
 * bounce_buffer_start() -- Start the bounce buffer session
 * state:	stores state passed between bounce_buffer_{start,stop}
 * data:	pointer to buffer to be aligned
 * len:		length of the buffer
 * flags:	flags describing the transaction, see above.
 */
int bounce_buffer_start(struct bounce_buffer *state, void *data,
			size_t len, unsigned int flags);
/**
 * bounce_buffer_stop() -- Finish the bounce buffer session
 * state:	stores state passed between bounce_buffer_{start,stop}
 */
int bounce_buffer_stop(struct bounce_buffer *state);

#endif
