/*
 *  Mask inlines
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
  
#include <sys/types.h>

#define MASK_INLINE static inline

#define MASK_MAX SND_MASK_MAX
#define MASK_SIZE (MASK_MAX / 32)

#define MASK_OFS(i)	((i) >> 5)
#define MASK_BIT(i)	(1U << ((i) & 31))

MASK_INLINE unsigned int ld2(u_int32_t v)
{
        unsigned r = 0;

        if (v >= 0x10000) {
                v >>= 16;
                r += 16;
        }
        if (v >= 0x100) {
                v >>= 8;
                r += 8;
        }
        if (v >= 0x10) {
                v >>= 4;
                r += 4;
        }
        if (v >= 4) {
                v >>= 2;
                r += 2;
        }
        if (v >= 2)
                r++;
        return r;
}

MASK_INLINE unsigned int hweight32(u_int32_t v)
{
        v = (v & 0x55555555) + ((v >> 1) & 0x55555555);
        v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
        v = (v & 0x0F0F0F0F) + ((v >> 4) & 0x0F0F0F0F);
        v = (v & 0x00FF00FF) + ((v >> 8) & 0x00FF00FF);
        return (v & 0x0000FFFF) + ((v >> 16) & 0x0000FFFF);
}

MASK_INLINE size_t snd_mask_sizeof(void)
{
	return sizeof(snd_mask_t);
}

MASK_INLINE void snd_mask_none(snd_mask_t *mask)
{
	memset(mask, 0, sizeof(*mask));
}

MASK_INLINE void snd_mask_any(snd_mask_t *mask)
{
	memset(mask, 0xff, MASK_SIZE * sizeof(u_int32_t));
}

MASK_INLINE int snd_mask_empty(const snd_mask_t *mask)
{
	int i;
	for (i = 0; i < MASK_SIZE; i++)
		if (mask->bits[i])
			return 0;
	return 1;
}

MASK_INLINE int snd_mask_full(const snd_mask_t *mask)
{
	int i;
	for (i = 0; i < MASK_SIZE; i++)
		if (mask->bits[i] != 0xffffffff)
			return 0;
	return 1;
}

MASK_INLINE unsigned int snd_mask_count(const snd_mask_t *mask)
{
	int i, w = 0;
	for (i = 0; i < MASK_SIZE; i++)
		w += hweight32(mask->bits[i]);
	return w;
}

MASK_INLINE unsigned int snd_mask_min(const snd_mask_t *mask)
{
	int i;
	assert(!snd_mask_empty(mask));
	for (i = 0; i < MASK_SIZE; i++) {
		if (mask->bits[i])
			return ffs(mask->bits[i]) - 1 + (i << 5);
	}
	return 0;
}

MASK_INLINE unsigned int snd_mask_max(const snd_mask_t *mask)
{
	int i;
	assert(!snd_mask_empty(mask));
	for (i = MASK_SIZE - 1; i >= 0; i--) {
		if (mask->bits[i])
			return ld2(mask->bits[i]) + (i << 5);
	}
	return 0;
}

MASK_INLINE void snd_mask_set(snd_mask_t *mask, unsigned int val)
{
	assert(val <= SND_MASK_MAX);
	mask->bits[MASK_OFS(val)] |= MASK_BIT(val);
}

MASK_INLINE void snd_mask_reset(snd_mask_t *mask, unsigned int val)
{
	assert(val <= SND_MASK_MAX);
	mask->bits[MASK_OFS(val)] &= ~MASK_BIT(val);
}

MASK_INLINE void snd_mask_set_range(snd_mask_t *mask, unsigned int from, unsigned int to)
{
	unsigned int i;
	assert(to <= SND_MASK_MAX && from <= to);
	for (i = from; i <= to; i++)
		mask->bits[MASK_OFS(i)] |= MASK_BIT(i);
}

MASK_INLINE void snd_mask_reset_range(snd_mask_t *mask, unsigned int from, unsigned int to)
{
	unsigned int i;
	assert(to <= SND_MASK_MAX && from <= to);
	for (i = from; i <= to; i++)
		mask->bits[MASK_OFS(i)] &= ~MASK_BIT(i);
}

MASK_INLINE void snd_mask_leave(snd_mask_t *mask, unsigned int val)
{
	unsigned int v;
	assert(val <= SND_MASK_MAX);
	v = mask->bits[MASK_OFS(val)] & MASK_BIT(val);
	snd_mask_none(mask);
	mask->bits[MASK_OFS(val)] = v;
}

MASK_INLINE void snd_mask_intersect(snd_mask_t *mask, const snd_mask_t *v)
{
	int i;
	for (i = 0; i < MASK_SIZE; i++)
		mask->bits[i] &= v->bits[i];
}

MASK_INLINE void snd_mask_union(snd_mask_t *mask, const snd_mask_t *v)
{
	int i;
	for (i = 0; i < MASK_SIZE; i++)
		mask->bits[i] |= v->bits[i];
}

MASK_INLINE int snd_mask_eq(const snd_mask_t *mask, const snd_mask_t *v)
{
	return ! memcmp(mask, v, MASK_SIZE * 4);
}

MASK_INLINE void snd_mask_copy(snd_mask_t *mask, const snd_mask_t *v)
{
	*mask = *v;
}

MASK_INLINE int snd_mask_test(const snd_mask_t *mask, unsigned int val)
{
	assert(val <= SND_MASK_MAX);
	return mask->bits[MASK_OFS(val)] & MASK_BIT(val);
}

MASK_INLINE int snd_mask_single(const snd_mask_t *mask)
{
	int i, c = 0;
	assert(!snd_mask_empty(mask));
	for (i = 0; i < MASK_SIZE; i++) {
		if (! mask->bits[i])
			continue;
		if (mask->bits[i] & (mask->bits[i] - 1))
			return 0;
		if (c)
			return 0;
		c++;
	}
	return 1;
}

MASK_INLINE int snd_mask_refine(snd_mask_t *mask, const snd_mask_t *v)
{
	snd_mask_t old;
	if (snd_mask_empty(mask))
		return -ENOENT;
	snd_mask_copy(&old, mask);
	snd_mask_intersect(mask, v);
	if (snd_mask_empty(mask))
		return -EINVAL;
	return !snd_mask_eq(mask, &old);
}

MASK_INLINE int snd_mask_refine_first(snd_mask_t *mask)
{
	if (snd_mask_empty(mask))
		return -ENOENT;
	if (snd_mask_single(mask))
		return 0;
	snd_mask_leave(mask, snd_mask_min(mask));
	return 1;
}

MASK_INLINE int snd_mask_refine_last(snd_mask_t *mask)
{
	if (snd_mask_empty(mask))
		return -ENOENT;
	if (snd_mask_single(mask))
		return 0;
	snd_mask_leave(mask, snd_mask_max(mask));
	return 1;
}

MASK_INLINE int snd_mask_refine_min(snd_mask_t *mask, unsigned int val)
{
	if (snd_mask_empty(mask))
		return -ENOENT;
	if (snd_mask_min(mask) >= val)
		return 0;
	snd_mask_reset_range(mask, 0, val - 1);
	if (snd_mask_empty(mask))
		return -EINVAL;
	return 1;
}

MASK_INLINE int snd_mask_refine_max(snd_mask_t *mask, unsigned int val)
{
	if (snd_mask_empty(mask))
		return -ENOENT;
	if (snd_mask_max(mask) <= val)
		return 0;
	snd_mask_reset_range(mask, val + 1, SND_MASK_MAX);
	if (snd_mask_empty(mask))
		return -EINVAL;
	return 1;
}

MASK_INLINE int snd_mask_refine_set(snd_mask_t *mask, unsigned int val)
{
	int changed;
	if (snd_mask_empty(mask))
		return -ENOENT;
	changed = !snd_mask_single(mask);
	snd_mask_leave(mask, val);
	if (snd_mask_empty(mask))
		return -EINVAL;
	return changed;
}

MASK_INLINE int snd_mask_value(const snd_mask_t *mask)
{
	assert(!snd_mask_empty(mask));
	return snd_mask_min(mask);
}

MASK_INLINE int snd_mask_always_eq(const snd_mask_t *m1, const snd_mask_t *m2)
{
	return snd_mask_single(m1) && snd_mask_single(m2) &&
		snd_mask_value(m1) == snd_mask_value(m2);
}

MASK_INLINE int snd_mask_never_eq(const snd_mask_t *m1, const snd_mask_t *m2)
{
	int i;
	for (i = 0; i < MASK_SIZE; i++)
		if (m1->bits[i] & m2->bits[i])
			return 0;
	return 1;
}
