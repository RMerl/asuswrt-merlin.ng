/*
 *  Mask header
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
  
typedef struct _snd_mask snd_mask_t;

#define SND_MASK_MAX 64

#ifdef SND_MASK_INLINE
#include "mask_inline.h"
#else
void snd_mask_none(snd_mask_t *mask);
void snd_mask_any(snd_mask_t *mask);
void snd_mask_load(snd_mask_t *mask, unsigned int msk);
int snd_mask_empty(const snd_mask_t *mask);
int snd_mask_full(const snd_mask_t *mask);
void snd_mask_set(snd_mask_t *mask, unsigned int val);
void snd_mask_reset(snd_mask_t *mask, unsigned int val);
void snd_mask_copy(snd_mask_t *mask, const snd_mask_t *v);
int snd_mask_test(const snd_mask_t *mask, unsigned int val);
void snd_mask_intersect(snd_mask_t *mask, const snd_mask_t *v);
void snd_mask_union(snd_mask_t *mask, const snd_mask_t *v);
unsigned int snd_mask_count(const snd_mask_t *mask);
unsigned int snd_mask_min(const snd_mask_t *mask);
unsigned int snd_mask_max(const snd_mask_t *mask);
void snd_mask_set_range(snd_mask_t *mask, unsigned int from, unsigned int to);
void snd_mask_reset_range(snd_mask_t *mask, unsigned int from, unsigned int to);
void snd_mask_leave(snd_mask_t *mask, unsigned int val);
int snd_mask_eq(const snd_mask_t *mask, const snd_mask_t *v);
int snd_mask_single(const snd_mask_t *mask);
int snd_mask_refine(snd_mask_t *mask, const snd_mask_t *v);
int snd_mask_refine_first(snd_mask_t *mask);
int snd_mask_refine_last(snd_mask_t *mask);
int snd_mask_refine_min(snd_mask_t *mask, unsigned int val);
int snd_mask_refine_max(snd_mask_t *mask, unsigned int val);
int snd_mask_refine_set(snd_mask_t *mask, unsigned int val);
int snd_mask_value(const snd_mask_t *mask);
int snd_mask_always_eq(const snd_mask_t *m1, const snd_mask_t *m2);
int snd_mask_never_eq(const snd_mask_t *m1, const snd_mask_t *m2);
#endif
