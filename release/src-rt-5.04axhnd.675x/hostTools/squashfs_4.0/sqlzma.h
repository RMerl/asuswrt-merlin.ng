/*
 * Copyright (C) 2006-2008 Junjiro Okajima
 * Copyright (C) 2006-2008 Tomas Matejicek, slax.org
 *
 * LICENSE follows the described one in lzma.
 */

/* $Id: sqlzma.h,v 1.20 2008-03-12 16:58:34 jro Exp $ */

#ifndef __sqlzma_h__
#define __sqlzma_h__

#ifndef __KERNEL__
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#ifdef _REENTRANT
#include <pthread.h>
#endif
#else
#include <linux/zlib.h>
#endif
#define _7ZIP_BYTE_DEFINED

/*
 * detect the compression method automatically by the first byte of compressed
 * data.
 * according to rfc1950, the first byte of zlib compression must be 0x?8.
 */
#define is_lzma(c)	(c == 0x5d)

/* ---------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __KERNEL__
/* for mksquashfs only */
struct sqlzma_opts {
	unsigned int	try_lzma;
	unsigned int 	dicsize;
};
int sqlzma_cm(struct sqlzma_opts *opts, z_stream *stream, Bytef *next_in,
	      uInt avail_in, Bytef *next_out, uInt avail_out);
#endif

/* ---------------------------------------------------------------------- */
/*
 * Three patterns for sqlzma uncompression. very dirty code.
 * - kernel space (squashfs kernel module)
 * - user space with pthread (mksquashfs)
 * - user space without pthread (unsquashfs)
 */

struct sized_buf {
	unsigned int	sz;
	unsigned char	*buf;
};

enum {SQUN_PROB, SQUN_RESULT, SQUN_LAST};
struct sqlzma_un {
	int			un_lzma;
	struct sized_buf	un_a[SQUN_LAST];
	unsigned char           un_prob[31960]; /* unlzma 64KB - 1MB */
	z_stream		un_stream;
#define un_cmbuf	un_stream.next_in
#define un_cmlen	un_stream.avail_in
#define un_resbuf	un_stream.next_out
#define un_resroom	un_stream.avail_out
#define un_reslen	un_stream.total_out
};

int sqlzma_init(struct sqlzma_un *un, int do_lzma, unsigned int res_sz);
int sqlzma_un(struct sqlzma_un *un, struct sized_buf *src,
	      struct sized_buf *dst);
void sqlzma_fin(struct sqlzma_un *un);

/* ---------------------------------------------------------------------- */

#ifdef __cplusplus
};
#endif
#endif
