/*
 * bcmallocache.h
 * Header file for a working-set malloc cache
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * $Id: bcmallocache.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef _bcm_alloc_cache_h
#define	_bcm_alloc_cache_h

typedef struct cache_s bcmcache_t;

extern int bcmcache_count_elems(bcmcache_t *cachep);
extern bcmcache_t* bcmcache_create(osl_t *osh, char *name, uint size);
extern void bcmcache_destroy(bcmcache_t *cachep);
extern void *bcmcache_alloc(bcmcache_t *cachep);
extern void bcmcache_free(bcmcache_t *cachep, void *objp);
extern void bcmcache_reclaim(bcmcache_t *cachep);

#ifdef BCMDBG
extern void bcmcache_info(bcmcache_t *cachep, char *buf);
#endif /* BCMDBG */

#endif	/* _bcm_alloc_cache_h */
