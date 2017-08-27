/*
 *  Interval functions
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
  
#define SND_INTERVAL_C
#define SND_INTERVAL_INLINE

#include <sys/types.h>
#include <limits.h>
#include "pcm_local.h"

static inline void div64_32(u_int64_t *n, u_int32_t d, u_int32_t *rem)
{
	*rem = *n % d;
	*n /= d;
}

static inline unsigned int div32(unsigned int a, unsigned int b, 
				 unsigned int *r)
{
	if (b == 0) {
		*r = 0;
		return UINT_MAX;
	}
	*r = a % b;
	return a / b;
}

static inline unsigned int div_down(unsigned int a, unsigned int b)
{
	if (b == 0)
		return UINT_MAX;
	return a / b;
}

static inline unsigned int div_up(unsigned int a, unsigned int b)
{
	unsigned int r;
	unsigned int q;
	if (b == 0)
		return UINT_MAX;
	q = div32(a, b, &r);
	if (r)
		++q;
	return q;
}

static inline unsigned int mul(unsigned int a, unsigned int b)
{
	if (a == 0)
		return 0;
	if (div_down(UINT_MAX, a) < b)
		return UINT_MAX;
	return a * b;
}

static inline unsigned int add(unsigned int a, unsigned int b)
{
	if (a >= UINT_MAX - b)
		return UINT_MAX;
	return a + b;
}

static inline unsigned int sub(unsigned int a, unsigned int b)
{
	if (a > b)
		return a - b;
	return 0;
}

static inline unsigned int muldiv32(unsigned int a, unsigned int b,
				    unsigned int c, unsigned int *r)
{
	u_int64_t n = (u_int64_t) a * b;
	if (c == 0) {
		assert(n > 0);
		*r = 0;
		return UINT_MAX;
	}
	div64_32(&n, c, r);
	if (n >= UINT_MAX) {
		*r = 0;
		return UINT_MAX;
	}
	return n;
}

int snd_interval_refine_min(snd_interval_t *i, unsigned int min, int openmin)
{
	int changed = 0;
	if (snd_interval_empty(i))
		return -ENOENT;
	if (i->min < min) {
		i->min = min;
		i->openmin = openmin;
		changed = 1;
	} else if (i->min == min && !i->openmin && openmin) {
		i->openmin = 1;
		changed = 1;
	}
	if (i->integer) {
		if (i->openmin) {
			i->min++;
			i->openmin = 0;
		}
	}
	if (snd_interval_checkempty(i)) {
		snd_interval_none(i);
		return -EINVAL;
	}
	return changed;
}

int snd_interval_refine_max(snd_interval_t *i, unsigned int max, int openmax)
{
	int changed = 0;
	if (snd_interval_empty(i))
		return -ENOENT;
	if (i->max > max) {
		i->max = max;
		i->openmax = openmax;
		changed = 1;
	} else if (i->max == max && !i->openmax && openmax) {
		i->openmax = 1;
		changed = 1;
	}
	if (i->integer) {
		if (i->openmax) {
			i->max--;
			i->openmax = 0;
		}
	}
	if (snd_interval_checkempty(i)) {
		snd_interval_none(i);
		return -EINVAL;
	}
	return changed;
}

/* r <- v */
int snd_interval_refine(snd_interval_t *i, const snd_interval_t *v)
{
	int changed = 0;
	if (snd_interval_empty(i))
		return -ENOENT;
	if (i->min < v->min) {
		i->min = v->min;
		i->openmin = v->openmin;
		changed = 1;
	} else if (i->min == v->min && !i->openmin && v->openmin) {
		i->openmin = 1;
		changed = 1;
	}
	if (i->max > v->max) {
		i->max = v->max;
		i->openmax = v->openmax;
		changed = 1;
	} else if (i->max == v->max && !i->openmax && v->openmax) {
		i->openmax = 1;
		changed = 1;
	}
	if (!i->integer && v->integer) {
		i->integer = 1;
		changed = 1;
	}
	if (i->integer) {
		if (i->openmin) {
			i->min++;
			i->openmin = 0;
		}
		if (i->openmax) {
			i->max--;
			i->openmax = 0;
		}
	} else if (!i->openmin && !i->openmax && i->min == i->max)
		i->integer = 1;
	if (snd_interval_checkempty(i)) {
		snd_interval_none(i);
		return -EINVAL;
	}
	return changed;
}

int snd_interval_refine_first(snd_interval_t *i)
{
	if (snd_interval_empty(i))
		return -ENOENT;
	if (snd_interval_single(i))
		return 0;
	i->max = i->min;
	i->openmax = i->openmin;
	if (i->openmax)
		i->max++;
	return 1;
}

int snd_interval_refine_last(snd_interval_t *i)
{
	if (snd_interval_empty(i))
		return -ENOENT;
	if (snd_interval_single(i))
		return 0;
	i->min = i->max;
	i->openmin = i->openmax;
	if (i->openmin)
		i->min--;
	return 1;
}

int snd_interval_refine_set(snd_interval_t *i, unsigned int val)
{
	snd_interval_t t;
	t.empty = 0;
	t.min = t.max = val;
	t.openmin = t.openmax = 0;
	t.integer = 1;
	return snd_interval_refine(i, &t);
}

void snd_interval_add(const snd_interval_t *a, const snd_interval_t *b, snd_interval_t *c)
{
	if (a->empty || b->empty) {
		snd_interval_none(c);
		return;
	}
	c->empty = 0;
	c->min = add(a->min, b->min);
	c->openmin = (a->openmin || b->openmin);
	c->max = add(a->max,  b->max);
	c->openmax = (a->openmax || b->openmax);
	c->integer = (a->integer && b->integer);
}

void snd_interval_sub(const snd_interval_t *a, const snd_interval_t *b, snd_interval_t *c)
{
	if (a->empty || b->empty) {
		snd_interval_none(c);
		return;
	}
	c->empty = 0;
	c->min = sub(a->min, b->max);
	c->openmin = (a->openmin || b->openmax);
	c->max = add(a->max,  b->min);
	c->openmax = (a->openmax || b->openmin);
	c->integer = (a->integer && b->integer);
}

void snd_interval_mul(const snd_interval_t *a, const snd_interval_t *b, snd_interval_t *c)
{
	if (a->empty || b->empty) {
		snd_interval_none(c);
		return;
	}
	c->empty = 0;
	c->min = mul(a->min, b->min);
	c->openmin = (a->openmin || b->openmin);
	c->max = mul(a->max,  b->max);
	c->openmax = (a->openmax || b->openmax);
	c->integer = (a->integer && b->integer);
}

void snd_interval_div(const snd_interval_t *a, const snd_interval_t *b, snd_interval_t *c)
{
	unsigned int r;
	if (a->empty || b->empty) {
		snd_interval_none(c);
		return;
	}
	c->empty = 0;
	c->min = div32(a->min, b->max, &r);
	c->openmin = (r || a->openmin || b->openmax);
	if (b->min > 0) {
		c->max = div32(a->max, b->min, &r);
		if (r) {
			c->max++;
			c->openmax = 1;
		} else
			c->openmax = (a->openmax || b->openmin);
	} else {
		c->max = UINT_MAX;
		c->openmax = 0;
	}
	c->integer = 0;
}

/* a * b / c */
void snd_interval_muldiv(const snd_interval_t *a, const snd_interval_t *b,
		     const snd_interval_t *c, snd_interval_t *d)
{
	unsigned int r;
	if (a->empty || b->empty || c->empty) {
		snd_interval_none(d);
		return;
	}
	d->empty = 0;
	d->min = muldiv32(a->min, b->min, c->max, &r);
	d->openmin = (r || a->openmin || b->openmin || c->openmax);
	d->max = muldiv32(a->max, b->max, c->min, &r);
	if (r) {
		d->max++;
		d->openmax = 1;
	} else
		d->openmax = (a->openmax || b->openmax || c->openmin);
	d->integer = 0;
}

/* a * b / k */
void snd_interval_muldivk(const snd_interval_t *a, const snd_interval_t *b,
		      unsigned int k, snd_interval_t *c)
{
	unsigned int r;
	if (a->empty || b->empty) {
		snd_interval_none(c);
		return;
	}
	c->empty = 0;
	c->min = muldiv32(a->min, b->min, k, &r);
	c->openmin = (r || a->openmin || b->openmin);
	c->max = muldiv32(a->max, b->max, k, &r);
	if (r) {
		c->max++;
		c->openmax = 1;
	} else
		c->openmax = (a->openmax || b->openmax);
	c->integer = 0;
}

/* a * k / b */
void snd_interval_mulkdiv(const snd_interval_t *a, unsigned int k,
		      const snd_interval_t *b, snd_interval_t *c)
{
	unsigned int r;
	if (a->empty || b->empty) {
		snd_interval_none(c);
		return;
	}
	c->empty = 0;
	c->min = muldiv32(a->min, k, b->max, &r);
	c->openmin = (r || a->openmin || b->openmax);
	if (b->min > 0) {
		c->max = muldiv32(a->max, k, b->min, &r);
		if (r) {
			c->max++;
			c->openmax = 1;
		} else
			c->openmax = (a->openmax || b->openmin);
	} else {
		c->max = UINT_MAX;
		c->openmax = 0;
	}
	c->integer = 0;
}

void snd_interval_print(const snd_interval_t *i, snd_output_t *out)
{
	if (snd_interval_empty(i))
		snd_output_printf(out, "NONE");
	else if (i->min == 0 && i->openmin == 0 && 
		 i->max == UINT_MAX && i->openmax == 0)
		snd_output_printf(out, "ALL");
	else if (snd_interval_single(i) && i->integer)
		snd_output_printf(out, "%u", snd_interval_value(i));
	else
		snd_output_printf(out, "%c%u %u%c",
				i->openmin ? '(' : '[',
				i->min, i->max,
				i->openmax ? ')' : ']');
}


void boundary_sub(int a, int adir, int b, int bdir, int *c, int *cdir)
{
	adir = adir < 0 ? -1 : (adir > 0 ? 1 : 0);
	bdir = bdir < 0 ? -1 : (bdir > 0 ? 1 : 0);
	*c = a - b;
	*cdir = adir - bdir;
	if (*cdir == -2) {
		assert(*c > INT_MIN);
		(*c)--;
	} else if (*cdir == 2) {
		assert(*c < INT_MAX);
		(*c)++;
	}
}

int boundary_lt(unsigned int a, int adir, unsigned int b, int bdir)
{
	assert(a > 0 || adir >= 0);
	assert(b > 0 || bdir >= 0);
	if (adir < 0) {
		a--;
		adir = 1;
	} else if (adir > 0)
		adir = 1;
	if (bdir < 0) {
		b--;
		bdir = 1;
	} else if (bdir > 0)
		bdir = 1;
	return a < b || (a == b && adir < bdir);
}

/* Return 1 if min is nearer to best than max */
int boundary_nearer(int min, int mindir, int best, int bestdir, int max, int maxdir)
{
	int dmin, dmindir;
	int dmax, dmaxdir;
	boundary_sub(best, bestdir, min, mindir, &dmin, &dmindir);
	boundary_sub(max, maxdir, best, bestdir, &dmax, &dmaxdir);
	return boundary_lt(dmin, dmindir, dmax, dmaxdir);
}
