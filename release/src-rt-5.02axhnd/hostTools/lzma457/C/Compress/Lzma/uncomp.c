/*
 * Copyright (C) 2006-2008 Junjiro Okajima
 * Copyright (C) 2006-2008 Tomas Matejicek, slax.org
 *
 * LICENSE follows the described one in lzma.txt.
 */

/* $Id: uncomp.c,v 1.7 2008-03-12 16:58:34 jro Exp $ */

/* extract some parts from lzma443/C/7zip/Compress/LZMA_C/LzmaTest.c */

#ifndef __KERNEL__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#define unlikely(x)		__builtin_expect(!!(x), 0)
#define BUG_ON(x)		assert(!(x))
/* sqlzma buffers are always larger than a page. true? */
#define kmalloc(sz,gfp)		malloc(sz)
#define kfree(p)		free(p)
#define zlib_inflate(s, f)	inflate(s, f)
#define zlib_inflateInit(s)	inflateInit(s)
#define zlib_inflateReset(s)	inflateReset(s)
#define zlib_inflateEnd(s)	inflateEnd(s)
#else
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#ifndef WARN_ON_ONCE
#define WARN_ON_ONCE(b)	WARN_ON(b)
#endif
#endif /* __KERNEL__ */

#include "sqlzma.h"
#include "LzmaDecode.h"

static int LzmaUncompress(struct sqlzma_un *un)
{
	int err, i, ret;
	SizeT outSize, inProcessed, outProcessed, srclen;
	/* it's about 24-80 bytes structure, if int is 32-bit */
	CLzmaDecoderState state;
	unsigned char *dst, *src, a[8];
	struct sized_buf *sbuf;

	/* Decode LZMA properties and allocate memory */
	err = -EINVAL;
	src = (void *)un->un_cmbuf;
	ret = LzmaDecodeProperties(&state.Properties, src,
				   LZMA_PROPERTIES_SIZE);
	src += LZMA_PROPERTIES_SIZE;
	if (unlikely(ret != LZMA_RESULT_OK))
		goto out;
	i = LzmaGetNumProbs(&state.Properties);
	if (unlikely(i <= 0))
		i = 1;
	i *= sizeof(CProb);
	sbuf = un->un_a + SQUN_PROB;
	if (unlikely(sbuf->sz < i)) {
		if (sbuf->buf && sbuf->buf != un->un_prob)
			kfree(sbuf->buf);
#ifdef __KERNEL__
		printk("%s:%d: %d --> %d\n", __func__, __LINE__, sbuf->sz, i);
#else
		printf("%d --> %d\n", sbuf->sz, i);
#endif
		err = -ENOMEM;
		sbuf->sz = 0;
		sbuf->buf = kmalloc(i, GFP_ATOMIC);
		if (unlikely(!sbuf->buf))
			goto out;
		sbuf->sz = i;
	}
	state.Probs = (void *)sbuf->buf;

	/* Read uncompressed size */
	memcpy(a, src, sizeof(a));
	src += sizeof(a);
	outSize = a[0] | (a[1] << 8) | (a[2] << 16) | (a[3] << 24);

	err = -EINVAL;
	dst = un->un_resbuf;
	if (unlikely(!dst || outSize > un->un_reslen))
		goto out;
	un->un_reslen = outSize;
	srclen = un->un_cmlen - (src - un->un_cmbuf);

	/* Decompress */
	err = LzmaDecode(&state, src, srclen, &inProcessed, dst, outSize,
			 &outProcessed);
	if (unlikely(err))
		err = -EINVAL;

 out:
#ifndef __KERNEL__
	if (err)
		fprintf(stderr, "err %d\n", err);
#endif
	return err;
}

int sqlzma_un(struct sqlzma_un *un, struct sized_buf *src,
	      struct sized_buf *dst)
{
	int err, by_lzma = 1;
	if (un->un_lzma && is_lzma(*src->buf)) {
		un->un_cmbuf = src->buf;
		un->un_cmlen = src->sz;
		un->un_resbuf = dst->buf;
		un->un_reslen = dst->sz;

		/* this library is thread-safe */
		err = LzmaUncompress(un);
		goto out;
	}

	by_lzma = 0;
	err = zlib_inflateReset(&un->un_stream);
	if (unlikely(err != Z_OK))
		goto out;
	un->un_stream.next_in = src->buf;
	un->un_stream.avail_in = src->sz;
	un->un_stream.next_out = dst->buf;
	un->un_stream.avail_out = dst->sz;
	err = zlib_inflate(&un->un_stream, Z_FINISH);
	if (err == Z_STREAM_END)
		err = 0;

 out:
	if (unlikely(err)) {
#ifdef __KERNEL__
		WARN_ON_ONCE(1);
#else
		char a[64] = "ZLIB ";
		if (by_lzma) {
			strcpy(a, "LZMA ");
#ifdef _REENTRANT
			strerror_r(err, a + 5, sizeof(a) - 5);
#else
			strncat(a, strerror(err), sizeof(a) - 5);
#endif
		} else
			strncat(a, zError(err), sizeof(a) - 5);
		fprintf(stderr, "%s: %.*s\n", __func__, sizeof(a), a);
#endif
	}
	return err;
}

int sqlzma_init(struct sqlzma_un *un, int do_lzma, unsigned int res_sz)
{
	int err;

	err = -ENOMEM;
	un->un_lzma = do_lzma;
	memset(un->un_a, 0, sizeof(un->un_a));
	un->un_a[SQUN_PROB].buf = un->un_prob;
	un->un_a[SQUN_PROB].sz = sizeof(un->un_prob);
	if (res_sz) {
		un->un_a[SQUN_RESULT].buf = kmalloc(res_sz, GFP_KERNEL);
		if (unlikely(!un->un_a[SQUN_RESULT].buf))
			return err;
		un->un_a[SQUN_RESULT].sz = res_sz;
	}

	un->un_stream.next_in = NULL;
	un->un_stream.avail_in = 0;
#ifdef __KERNEL__
	un->un_stream.workspace = kmalloc(zlib_inflate_workspacesize(),
					  GFP_KERNEL);
	if (unlikely(!un->un_stream.workspace))
		return err;
#else
	un->un_stream.opaque = NULL;
	un->un_stream.zalloc = Z_NULL;
	un->un_stream.zfree = Z_NULL;
#endif
	err = zlib_inflateInit(&un->un_stream);
	if (unlikely(err == Z_MEM_ERROR))
		return -ENOMEM;
	BUG_ON(err);
	return err;
}

void sqlzma_fin(struct sqlzma_un *un)
{
	int i;
	for (i = 0; i < SQUN_LAST; i++)
		if (un->un_a[i].buf && un->un_a[i].buf != un->un_prob)
			kfree(un->un_a[i].buf);
	BUG_ON(zlib_inflateEnd(&un->un_stream) != Z_OK);
}

#ifdef __KERNEL__
EXPORT_SYMBOL(sqlzma_un);
EXPORT_SYMBOL(sqlzma_init);
EXPORT_SYMBOL(sqlzma_fin);

#if 0
static int __init sqlzma_init(void)
{
	return 0;
}

static void __exit sqlzma_exit(void)
{
}

module_init(sqlzma_init);
module_exit(sqlzma_exit);
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Junjiro Okajima <sfjro at users dot sf dot net>");
MODULE_VERSION("$Id: uncomp.c,v 1.7 2008-03-12 16:58:34 jro Exp $");
MODULE_DESCRIPTION("LZMA uncompress for squashfs. "
		   "Some functions for squashfs to support LZMA and "
		   "a tiny wrapper for LzmaDecode.c in LZMA SDK from www.7-zip.org.");
#endif
