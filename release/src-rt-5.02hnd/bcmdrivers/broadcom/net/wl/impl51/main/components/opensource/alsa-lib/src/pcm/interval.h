/*
 *  Interval header
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
  
typedef struct _snd_interval snd_interval_t;

#ifdef SND_INTERVAL_INLINE
#include "interval_inline.h"
#else
void snd_interval_any(snd_interval_t *i);
void snd_interval_none(snd_interval_t *i);
int snd_interval_setinteger(snd_interval_t *i);
int snd_interval_empty(const snd_interval_t *i);
int snd_interval_single(const snd_interval_t *i);
int snd_interval_value(const snd_interval_t *i);
void snd_interval_set_value(snd_interval_t *i, unsigned int val);
int snd_interval_min(const snd_interval_t *i);
int snd_interval_max(const snd_interval_t *i);
void snd_interval_set_minmax(snd_interval_t *i, unsigned int min, unsigned int max);
int snd_interval_test(const snd_interval_t *i, unsigned int val);
void snd_interval_copy(snd_interval_t *dst, const snd_interval_t *src);
void snd_interval_floor(snd_interval_t *i);
void snd_interval_unfloor(snd_interval_t *i);
int snd_interval_always_eq(const snd_interval_t *i1, const snd_interval_t *i2);
int snd_interval_never_eq(const snd_interval_t *i1, const snd_interval_t *i2);
#endif

/* make local functions really local */
#define snd_interval_add	snd1_interval_add
#define snd_interval_sub	snd1_interval_sub
#define snd_interval_mul	snd1_interval_mul
#define snd_interval_div	snd1_interval_div
#define snd_interval_muldiv	snd1_interval_muldiv
#define snd_interval_muldivk	snd1_interval_muldivk
#define snd_interval_mulkdiv	snd1_interval_mulkdiv
#define snd_interval_print	snd1_interval_print
#define snd_interval_refine_min	snd1_interval_refine_min
#define snd_interval_refine_max	snd1_interval_refine_max
#define snd_interval_refine	snd1_interval_refine
#define snd_interval_refine_first snd1_interval_refine_first
#define snd_interval_refine_last snd1_interval_refine_last
#define snd_interval_refine_set	snd1_interval_refine_set

void snd_interval_add(const snd_interval_t *a, const snd_interval_t *b, snd_interval_t *c);
void snd_interval_sub(const snd_interval_t *a, const snd_interval_t *b, snd_interval_t *c);
void snd_interval_mul(const snd_interval_t *a, const snd_interval_t *b, snd_interval_t *c);
void snd_interval_div(const snd_interval_t *a, const snd_interval_t *b, snd_interval_t *c);
void snd_interval_muldiv(const snd_interval_t *a, const snd_interval_t *b, 
		     const snd_interval_t *c, snd_interval_t *d);
void snd_interval_muldivk(const snd_interval_t *a, const snd_interval_t *b, 
		      unsigned int k, snd_interval_t *c);
void snd_interval_mulkdiv(const snd_interval_t *a, unsigned int k,
		      const snd_interval_t *b, snd_interval_t *c);
void snd_interval_print(const snd_interval_t *i, snd_output_t *out);
int snd_interval_refine_min(snd_interval_t *i, unsigned int min, int openmin);
int snd_interval_refine_max(snd_interval_t *i, unsigned int max, int openmax);
int snd_interval_refine(snd_interval_t *i, const snd_interval_t *v);
int snd_interval_refine_first(snd_interval_t *i);
int snd_interval_refine_last(snd_interval_t *i);
int snd_interval_refine_set(snd_interval_t *i, unsigned int val);
void boundary_sub(int a, int adir, int b, int bdir, int *c, int *cdir);
int boundary_lt(unsigned int a, int adir, unsigned int b, int bdir);
int boundary_nearer(int min, int mindir, int best, int bestdir, int max, int maxdir);
