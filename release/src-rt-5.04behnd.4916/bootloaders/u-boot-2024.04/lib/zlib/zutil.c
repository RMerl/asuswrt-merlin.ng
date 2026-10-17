/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2005 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#include "zutil.h"
#include <hang.h>

#ifndef NO_DUMMY_DECL
struct internal_state      {int dummy;}; /* for buggy compilers */
#endif

const char * const z_errmsg[10] = {
"need dictionary",     /* Z_NEED_DICT       2  */
"stream end",          /* Z_STREAM_END      1  */
"",                    /* Z_OK              0  */
"file error",          /* Z_ERRNO         (-1) */
"stream error",        /* Z_STREAM_ERROR  (-2) */
"data error",          /* Z_DATA_ERROR    (-3) */
"insufficient memory", /* Z_MEM_ERROR     (-4) */
"buffer error",        /* Z_BUF_ERROR     (-5) */
"incompatible version",/* Z_VERSION_ERROR (-6) */
""};

#ifdef DEBUG

#ifndef verbose
#define verbose 0
#endif
int z_verbose = verbose;

void z_error (m)
    char *m;
{
	fprintf(stderr, "%s\n", m);
	hang();
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifdef __UBOOT__
#include <malloc.h>
#else
#ifndef STDC
extern voidp    malloc OF((uInt size));
extern voidp    calloc OF((uInt items, uInt size));
extern void     free   OF((voidpf ptr));
#endif
#endif

voidpf zcalloc(voidpf opaque, unsigned items, unsigned size)
{
	if (opaque)
		items += size - size; /* make compiler happy */
	return sizeof(uInt) > 2 ? (voidpf)malloc(items * size) :
		(voidpf)calloc(items, size);
}

void  zcfree(voidpf opaque, voidpf ptr, unsigned nb)
{
	free(ptr);
	if (opaque)
		return; /* make compiler happy */
}

#endif /* MY_ZCALLOC */
