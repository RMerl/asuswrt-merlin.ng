/*
 *  PCM - Params functions
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
  
#include "pcm_local.h"

#ifndef NDEBUG
/*
 * dump hw_params when $LIBASOUND_DEBUG is set to >= 1
 */
static void dump_hw_params(snd_pcm_hw_params_t *params, const char *type,
			   snd_pcm_hw_param_t var, unsigned int val, int err)
{
	const char *verbose = getenv("LIBASOUND_DEBUG");
	snd_output_t *out;

	if (! verbose || ! *verbose || atoi(verbose) < 1)
		return;
	if (snd_output_stdio_attach(&out, stderr, 0) < 0)
		return;
	fprintf(stderr, "ALSA ERROR hw_params: %s (%s)\n",
		type, snd_pcm_hw_param_name(var));
	fprintf(stderr, "           value = ");
	switch (var) {
	case SND_PCM_HW_PARAM_ACCESS:
		fprintf(stderr, "%s", snd_pcm_access_name(val));
		break;
	case SND_PCM_HW_PARAM_FORMAT:
		fprintf(stderr, "%s", snd_pcm_format_name(val));
		break;
	case SND_PCM_HW_PARAM_SUBFORMAT:
		fprintf(stderr, "%s", snd_pcm_subformat_name(val));
		break;
	default:
		fprintf(stderr, "%u", val);
	}
	fprintf(stderr, " : %s\n", snd_strerror(err));
	snd_pcm_hw_params_dump(params, out);
	snd_output_close(out);
}
#else
static inline void dump_hw_params(snd_pcm_hw_params_t *params, const char *type,
				  snd_pcm_hw_param_t var, unsigned int val, int err)
{
}
#endif

static inline int hw_is_mask(snd_pcm_hw_param_t var)
{
#if SND_PCM_HW_PARAM_FIRST_MASK == 0
	return var <= SND_PCM_HW_PARAM_LAST_MASK;
#else
	return var >= SND_PCM_HW_PARAM_FIRST_MASK &&
		var <= SND_PCM_HW_PARAM_LAST_MASK;
#endif
}

static inline int hw_is_interval(snd_pcm_hw_param_t var)
{
	return var >= SND_PCM_HW_PARAM_FIRST_INTERVAL &&
		var <= SND_PCM_HW_PARAM_LAST_INTERVAL;
}

#define hw_param_mask(params,var) \
	&((params)->masks[(var) - SND_PCM_HW_PARAM_FIRST_MASK])

#define hw_param_interval(params,var) \
	&((params)->intervals[(var) - SND_PCM_HW_PARAM_FIRST_INTERVAL])

#define hw_param_mask_c hw_param_mask
#define hw_param_interval_c hw_param_interval

static void _snd_pcm_hw_param_any(snd_pcm_hw_params_t *params, snd_pcm_hw_param_t var)
{
	if (hw_is_mask(var)) {
		snd_mask_any(hw_param_mask(params, var));
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
		return;
	}
	if (hw_is_interval(var)) {
		snd_interval_any(hw_param_interval(params, var));
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
		return;
	}
	assert(0);
}

int snd_pcm_hw_param_any(snd_pcm_t *pcm, snd_pcm_hw_params_t *params,
			 snd_pcm_hw_param_t var)
{
	_snd_pcm_hw_param_any(params, var);
	return snd_pcm_hw_refine(pcm, params);
}

void _snd_pcm_hw_params_any(snd_pcm_hw_params_t *params)
{
	unsigned int k;
	memset(params, 0, sizeof(*params));
	for (k = SND_PCM_HW_PARAM_FIRST_MASK; k <= SND_PCM_HW_PARAM_LAST_MASK; k++)
		_snd_pcm_hw_param_any(params, k);
	for (k = SND_PCM_HW_PARAM_FIRST_INTERVAL; k <= SND_PCM_HW_PARAM_LAST_INTERVAL; k++)
		_snd_pcm_hw_param_any(params, k);
	params->rmask = ~0U;
	params->cmask = 0;
	params->info = ~0U;
}

/* Return the value for field PAR if it's fixed in configuration space 
   defined by PARAMS. Return -EINVAL otherwise
*/
int snd_pcm_hw_param_get(const snd_pcm_hw_params_t *params, snd_pcm_hw_param_t var,
			 unsigned int *val, int *dir)
{
	if (hw_is_mask(var)) {
		const snd_mask_t *mask = hw_param_mask_c(params, var);
		if (snd_mask_empty(mask) || !snd_mask_single(mask))
			return -EINVAL;
		if (dir)
			*dir = 0;
		if (val)
			*val = snd_mask_value(mask);
		return 0;
	} else if (hw_is_interval(var)) {
		const snd_interval_t *i = hw_param_interval_c(params, var);
		if (snd_interval_empty(i) || !snd_interval_single(i))
			return -EINVAL;
		if (dir)
			*dir = i->openmin;
		if (val)
			*val = snd_interval_value(i);
		return 0;
	}
	assert(0);
	return -EINVAL;
}

/* Return the minimum value for field PAR. */
int snd_pcm_hw_param_get_min(const snd_pcm_hw_params_t *params, snd_pcm_hw_param_t var,
			     unsigned int *val, int *dir)
{
	if (hw_is_mask(var)) {
		const snd_mask_t *m = hw_param_mask_c(params, var);
		assert(!snd_mask_empty(m));
		if (dir)
			*dir = 0;
		if (val)
			*val = snd_mask_min(m);
		return 0;
	} else if (hw_is_interval(var)) {
		const snd_interval_t *i = hw_param_interval_c(params, var);
		assert(!snd_interval_empty(i));
		if (dir)
			*dir = i->openmin;
		if (val)
			*val = snd_interval_min(i);
		return 0;
	}
	assert(0);
	return 0;
}

/* Return the maximum value for field PAR. */
int snd_pcm_hw_param_get_max(const snd_pcm_hw_params_t *params, snd_pcm_hw_param_t var,
			     unsigned int *val, int *dir)
{
	if (hw_is_mask(var)) {
		const snd_mask_t *m = hw_param_mask_c(params, var);
		assert(!snd_mask_empty(m));
		if (dir)
			*dir = 0;
		if (val)
			*val = snd_mask_max(m);
		return 0;
	} else if (hw_is_interval(var)) {
		const snd_interval_t *i = hw_param_interval_c(params, var);
		assert(!snd_interval_empty(i));
		if (dir)
			*dir = - (int) i->openmax;
		if (val)
			*val = snd_interval_max(i);
		return 0;
	}
	assert(0);
	return 0;
}

/* Return the mask for field PAR.
   This function can be called only for SND_PCM_HW_PARAM_ACCESS,
   SND_PCM_HW_PARAM_FORMAT, SND_PCM_HW_PARAM_SUBFORMAT. */
const snd_mask_t *snd_pcm_hw_param_get_mask(const snd_pcm_hw_params_t *params,
					   snd_pcm_hw_param_t var)
{
	assert(hw_is_mask(var));
	return hw_param_mask_c(params, var);
}

/* Return the interval for field PAR.
   This function cannot be called for SND_PCM_HW_PARAM_ACCESS,
   SND_PCM_HW_PARAM_FORMAT, SND_PCM_HW_PARAM_SUBFORMAT. */
const snd_interval_t *snd_pcm_hw_param_get_interval(const snd_pcm_hw_params_t *params,
						  snd_pcm_hw_param_t var)
{
	assert(hw_is_interval(var));
	return hw_param_interval_c(params, var);
}

/* --- Refinement functions --- */

int _snd_pcm_hw_param_set_interval(snd_pcm_hw_params_t *params,
				   snd_pcm_hw_param_t var,
				   const snd_interval_t *val)
{
	int changed;
	assert(hw_is_interval(var));
	changed = snd_interval_refine(hw_param_interval(params, var), val);
	if (changed) {
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}
	return changed;
}

void _snd_pcm_hw_param_set_empty(snd_pcm_hw_params_t *params,
				 snd_pcm_hw_param_t var)
{
	if (hw_is_mask(var)) {
		snd_mask_none(hw_param_mask(params, var));
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	} else if (hw_is_interval(var)) {
		snd_interval_none(hw_param_interval(params, var));
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	} else {
		assert(0);
	}
}

static int _snd_pcm_hw_param_set_integer(snd_pcm_hw_params_t *params,
					 snd_pcm_hw_param_t var)
{
	int changed;
	assert(hw_is_interval(var));
	changed = snd_interval_setinteger(hw_param_interval(params, var));
	if (changed) {
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}
	return changed;
}
	
/* Inside configuration space defined by PARAMS remove from PAR all 
   non integer values. Reduce configuration space accordingly.
   Return -EINVAL if the configuration space is empty
*/
int snd_pcm_hw_param_set_integer(snd_pcm_t *pcm, 
				 snd_pcm_hw_params_t *params,
				 snd_set_mode_t mode,
				 snd_pcm_hw_param_t var)
{
	snd_pcm_hw_params_t save;
	int err;
	switch (mode) {
	case SND_CHANGE:
		break;
	case SND_TRY:
		save = *params;
		break;
	case SND_TEST:
		save = *params;
		params = &save;
		break;
	default:
		assert(0);
		return -EINVAL;
	}
	err = _snd_pcm_hw_param_set_integer(params, var);
	if (err < 0)
		goto _fail;
	if (params->rmask) {
		err = snd_pcm_hw_refine(pcm, params);
		if (err < 0)
			goto _fail;
	}
	return 0;
 _fail:
	if (mode == SND_TRY)
		*params = save;
	return err;
}

static int _snd_pcm_hw_param_set_first(snd_pcm_hw_params_t *params,
				       snd_pcm_hw_param_t var)
{
	int changed;
	if (hw_is_mask(var))
		changed = snd_mask_refine_first(hw_param_mask(params, var));
	else if (hw_is_interval(var))
		changed = snd_interval_refine_first(hw_param_interval(params, var));
	else {
		assert(0);
		return -EINVAL;
	}
	if (changed > 0) {
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}
	return changed;
}


/* Inside configuration space defined by PARAMS remove from PAR all 
   values > minimum. Reduce configuration space accordingly.
   Return the minimum.
*/
int snd_pcm_hw_param_set_first(snd_pcm_t *pcm, 
			       snd_pcm_hw_params_t *params, 
			       snd_pcm_hw_param_t var,
			       unsigned int *rval, int *dir)
{
	int err;

	err = _snd_pcm_hw_param_set_first(params, var);
	if (err < 0)
		return err;
	if (params->rmask) {
		err = snd_pcm_hw_refine(pcm, params);
		if (err < 0)
			return err;
	}
	return snd_pcm_hw_param_get(params, var, rval, dir);
}

static int _snd_pcm_hw_param_set_last(snd_pcm_hw_params_t *params,
				      snd_pcm_hw_param_t var)
{
	int changed;
	if (hw_is_mask(var))
		changed = snd_mask_refine_last(hw_param_mask(params, var));
	else if (hw_is_interval(var))
		changed = snd_interval_refine_last(hw_param_interval(params, var));
	else {
		assert(0);
		return -EINVAL;
	}
	if (changed > 0) {
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}
	return changed;
}


/* Inside configuration space defined by PARAMS remove from PAR all 
   values < maximum. Reduce configuration space accordingly.
   Return the maximum.
*/
int snd_pcm_hw_param_set_last(snd_pcm_t *pcm, 
			      snd_pcm_hw_params_t *params,
			      snd_pcm_hw_param_t var,
			      unsigned int *rval, int *dir)
{
	int err;

	err = _snd_pcm_hw_param_set_last(params, var);
	if (err < 0)
		return err;
	if (params->rmask) {
		err = snd_pcm_hw_refine(pcm, params);
		if (err < 0)
			return err;
	}
	return snd_pcm_hw_param_get(params, var, rval, dir);
}

int _snd_pcm_hw_param_set_min(snd_pcm_hw_params_t *params,
			      snd_pcm_hw_param_t var, unsigned int val, int dir)
{
	int changed;
	int openmin = 0;
	if (dir) {
		if (dir > 0) {
			openmin = 1;
		} else if (dir < 0) {
			if (val > 0) {
				openmin = 1;
				val--;
			}
		}
	}
	if (hw_is_mask(var))
		changed = snd_mask_refine_min(hw_param_mask(params, var), val + !!openmin);
	else if (hw_is_interval(var))
		changed = snd_interval_refine_min(hw_param_interval(params, var), val, openmin);
	else {
		assert(0);
		return -EINVAL;
	}
	if (changed) {
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}
	return changed;
}

/* Inside configuration space defined by PARAMS remove from PAR all 
   values < VAL. Reduce configuration space accordingly.
   Return new minimum or -EINVAL if the configuration space is empty
*/
int snd_pcm_hw_param_set_min(snd_pcm_t *pcm, snd_pcm_hw_params_t *params,
			     snd_set_mode_t mode,
			     snd_pcm_hw_param_t var, unsigned int *val, int *dir)
{
	snd_pcm_hw_params_t save;
	int err;
	switch (mode) {
	case SND_CHANGE:
		break;
	case SND_TRY:
		save = *params;
		break;
	case SND_TEST:
		save = *params;
		params = &save;
		break;
	default:
		assert(0);
		return -EINVAL;
	}
	err = _snd_pcm_hw_param_set_min(params, var, *val, dir ? *dir : 0);
	if (err < 0)
		goto _fail;
	if ((mode != SND_TEST || hw_is_interval(var)) && params->rmask) {
		err = snd_pcm_hw_refine(pcm, params);
		if (err < 0)
			goto _fail;
		if (snd_pcm_hw_param_empty(params, var)) {
			err = -ENOENT;
			goto _fail;
		}
	}
	return snd_pcm_hw_param_get_min(params, var, val, dir);
 _fail:
	if (mode == SND_TRY)
		*params = save;
	if (err < 0 && mode == SND_TRY)
		dump_hw_params(params, "set_min", var, *val, err);
	return err;
}

int _snd_pcm_hw_param_set_max(snd_pcm_hw_params_t *params,
			      snd_pcm_hw_param_t var, unsigned int val, int dir)
{
	int changed;
	int openmax = 0;
	if (dir) {
		if (dir < 0) {
			openmax = 1;
		} else if (dir > 0) {
			openmax = 1;
			val++;
		}
	}
	if (hw_is_mask(var)) {
		if (val == 0 && openmax) {
		snd_mask_none(hw_param_mask(params, var));
			changed = -EINVAL;
		} else
			changed = snd_mask_refine_max(hw_param_mask(params, var), val - !!openmax);
	} else if (hw_is_interval(var))
		changed = snd_interval_refine_max(hw_param_interval(params, var), val, openmax);
	else {
		assert(0);
		return -EINVAL;
	}
	if (changed) {
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}
	return changed;
}

/* Inside configuration space defined by PARAMS remove from PAR all 
   values >= VAL + 1. Reduce configuration space accordingly.
   Return new maximum or -EINVAL if the configuration space is empty
*/
int snd_pcm_hw_param_set_max(snd_pcm_t *pcm, snd_pcm_hw_params_t *params,
			     snd_set_mode_t mode,
			     snd_pcm_hw_param_t var, unsigned int *val, int *dir)
{
	snd_pcm_hw_params_t save;
	int err;
	switch (mode) {
	case SND_CHANGE:
		break;
	case SND_TRY:
		save = *params;
		break;
	case SND_TEST:
		save = *params;
		params = &save;
		break;
	default:
		assert(0);
		return -EINVAL;
	}
	err = _snd_pcm_hw_param_set_max(params, var, *val, dir ? *dir : 0);
	if (err < 0)
		goto _fail;
	if ((mode != SND_TEST || hw_is_interval(var)) && params->rmask) {
		err = snd_pcm_hw_refine(pcm, params);
		if (err < 0)
			goto _fail;
		if (snd_pcm_hw_param_empty(params, var)) {
			err = -ENOENT;
			goto _fail;
		}
	}
	return snd_pcm_hw_param_get_max(params, var, val, dir);
 _fail:
	if (mode == SND_TRY)
		*params = save;
	if (err < 0 && mode == SND_TRY)
		dump_hw_params(params, "set_max", var, *val, err);
	return err;
}

int _snd_pcm_hw_param_set_minmax(snd_pcm_hw_params_t *params,
				 snd_pcm_hw_param_t var,
				 unsigned int min, int mindir,
				 unsigned int max, int maxdir)
{
	int changed, c1, c2;
	int openmin = 0, openmax = 0;
	if (mindir) {
		if (mindir > 0) {
			openmin = 1;
		} else if (mindir < 0) {
			if (min > 0) {
				openmin = 1;
				min--;
			}
		}
	}
	if (maxdir) {
		if (maxdir < 0) {
			openmax = 1;
		} else if (maxdir > 0) {
			openmax = 1;
			max++;
		}
	}
	if (hw_is_mask(var)) {
		snd_mask_t *mask = hw_param_mask(params, var);
		if (max == 0 && openmax) {
			snd_mask_none(mask);
			changed = -EINVAL;
		} else {
			c1 = snd_mask_refine_min(mask, min + !!openmin);
			if (c1 < 0)
				changed = c1;
			else {
				c2 = snd_mask_refine_max(mask, max - !!openmax);
				if (c2 < 0)
					changed = c2;
				else
					changed = (c1 || c2);
			}
		}
	}
	else if (hw_is_interval(var)) {
		snd_interval_t *i = hw_param_interval(params, var);
		c1 = snd_interval_refine_min(i, min, openmin);
		if (c1 < 0)
			changed = c1;
		else {
			c2 = snd_interval_refine_max(i, max, openmax);
			if (c2 < 0)
				changed = c2;
			else
				changed = (c1 || c2);
		}
	} else {
		assert(0);
		return -EINVAL;
	}
	if (changed) {
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}
	return changed;
}

/* Inside configuration space defined by PARAMS remove from PAR all 
   values < MIN and all values > MAX. Reduce configuration space accordingly.
   Return 0 or -EINVAL if the configuration space is empty
*/
int snd_pcm_hw_param_set_minmax(snd_pcm_t *pcm, snd_pcm_hw_params_t *params,
				snd_set_mode_t mode,
				snd_pcm_hw_param_t var,
				unsigned int *min, int *mindir,
				unsigned int *max, int *maxdir)
{
	snd_pcm_hw_params_t save;
	int err;
	switch (mode) {
	case SND_CHANGE:
		break;
	case SND_TRY:
		save = *params;
		break;
	case SND_TEST:
		save = *params;
		params = &save;
		break;
	default:
		assert(0);
		return -EINVAL;
	}
	err = _snd_pcm_hw_param_set_minmax(params, var, 
					   *min, mindir ? *mindir : 0,
					   *max, maxdir ? *maxdir : 0);
	if (err < 0)
		goto _fail;
	if ((mode != SND_TEST || hw_is_interval(var)) && params->rmask) {
		err = snd_pcm_hw_refine(pcm, params);
		if (err < 0)
			goto _fail;
	}
	err = snd_pcm_hw_param_get_min(params, var, min, mindir);
	if (err < 0)
		return err;
	return snd_pcm_hw_param_get_max(params, var, max, maxdir);
 _fail:
	if (mode == SND_TRY)
		*params = save;
	if (err < 0)
		dump_hw_params(params, "set_minmax", var, *min, err);
	return err;
}

int _snd_pcm_hw_param_set(snd_pcm_hw_params_t *params,
			  snd_pcm_hw_param_t var, unsigned int val, int dir)
{
	int changed;
	if (hw_is_mask(var)) {
		snd_mask_t *m = hw_param_mask(params, var);
		if (val == 0 && dir < 0) {
			changed = -EINVAL;
			snd_mask_none(m);
		} else {
			if (dir > 0)
				val++;
			else if (dir < 0)
				val--;
			changed = snd_mask_refine_set(hw_param_mask(params, var), val);
		}
	} else if (hw_is_interval(var)) {
		snd_interval_t *i = hw_param_interval(params, var);
		if (val == 0 && dir < 0) {
			changed = -EINVAL;
			snd_interval_none(i);
		} else if (dir == 0)
			changed = snd_interval_refine_set(i, val);
		else {
			snd_interval_t t;
			t.openmin = 1;
			t.openmax = 1;
			t.empty = 0;
			t.integer = 0;
			if (dir < 0) {
				t.min = val - 1;
				t.max = val;
			} else {
				t.min = val;
				t.max = val+1;
			}
			changed = snd_interval_refine(i, &t);
		}
	} else {
		assert(0);
		return -EINVAL;
	}
	if (changed) {
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}
	return changed;
}

/* Inside configuration space defined by PARAMS remove from PAR all 
   values != VAL. Reduce configuration space accordingly.
   Return -EINVAL if the configuration space is empty
*/
int snd_pcm_hw_param_set(snd_pcm_t *pcm, snd_pcm_hw_params_t *params,
			 snd_set_mode_t mode,
			 snd_pcm_hw_param_t var, unsigned int val, int dir)
{
	snd_pcm_hw_params_t save;
	int err;
	switch (mode) {
	case SND_CHANGE:
		break;
	case SND_TRY:
		save = *params;
		break;
	case SND_TEST:
		save = *params;
		params = &save;
		break;
	default:
		assert(0);
		return -EINVAL;
	}
	err = _snd_pcm_hw_param_set(params, var, val, dir);
	if (err < 0)
		goto _fail;
	if ((mode != SND_TEST || hw_is_interval(var)) && params->rmask) {
		err = snd_pcm_hw_refine(pcm, params);
		if (err < 0)
			goto _fail;
	}
	return 0;
 _fail:
	if (mode == SND_TRY)
		*params = save;
	if (err < 0 && mode == SND_TRY)
		dump_hw_params(params, "set", var, val, err);
	return err;
}

int _snd_pcm_hw_param_set_mask(snd_pcm_hw_params_t *params,
			       snd_pcm_hw_param_t var, const snd_mask_t *val)
{
	int changed;
	assert(hw_is_mask(var));
	changed = snd_mask_refine(hw_param_mask(params, var), val);
	if (changed) {
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}
	return changed;
}

/* Inside configuration space defined by PARAMS remove from PAR all values
   not contained in MASK. Reduce configuration space accordingly.
   This function can be called only for SND_PCM_HW_PARAM_ACCESS,
   SND_PCM_HW_PARAM_FORMAT, SND_PCM_HW_PARAM_SUBFORMAT.
   Return 0 on success or -EINVAL
   if the configuration space is empty
*/
int snd_pcm_hw_param_set_mask(snd_pcm_t *pcm, snd_pcm_hw_params_t *params,
			      snd_set_mode_t mode,
			      snd_pcm_hw_param_t var, const snd_mask_t *val)
{
	snd_pcm_hw_params_t save;
	int err;
	switch (mode) {
	case SND_CHANGE:
		break;
	case SND_TRY:
		save = *params;
		break;
	case SND_TEST:
		save = *params;
		params = &save;
		break;
	default:
		assert(0);
		return -EINVAL;
	}
	err = _snd_pcm_hw_param_set_mask(params, var, val);
	if (err < 0)
		goto _fail;
	if (mode != SND_TEST && params->rmask) {
		err = snd_pcm_hw_refine(pcm, params);
		if (err < 0)
			goto _fail;
	}
	return 0;
 _fail:
	if (mode == SND_TRY)
		*params = save;
	return err;
}

/* Inside configuration space defined by PARAMS set PAR to the available value
   nearest to VAL. Reduce configuration space accordingly.
   This function cannot be called for SND_PCM_HW_PARAM_ACCESS,
   SND_PCM_HW_PARAM_FORMAT, SND_PCM_HW_PARAM_SUBFORMAT.
   Return the value found.
 */
int snd_pcm_hw_param_set_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params,
			      snd_pcm_hw_param_t var,
			      unsigned int *val, int *dir)
{
	snd_pcm_hw_params_t save;
	int err;
	unsigned int best = *val, saved_min;
	int last = 0;
	unsigned int min, max;
	int mindir, maxdir;
	int valdir = dir ? *dir : 0;
	snd_interval_t *i;
	if (best > INT_MAX)
		best = INT_MAX;
	min = max = best;
	mindir = maxdir = valdir;
	if (maxdir > 0)
		maxdir = 0;
	else if (maxdir == 0)
		maxdir = -1;
	else {
		maxdir = 1;
		max--;
	}
	save = *params;
	saved_min = min;
	err = snd_pcm_hw_param_set_min(pcm, params, SND_CHANGE, var, &min, &mindir);

	i = hw_param_interval(params, var);
	if (!snd_interval_empty(i) && snd_interval_single(i)) {
		err = snd_pcm_hw_param_get_min(params, var, val, dir);
		if (err < 0)
			dump_hw_params(params, "set_near", var, *val, err);
		return err;
	}
	
	if (err >= 0) {
		snd_pcm_hw_params_t params1;
		if (min == saved_min && mindir == valdir)
			goto _end;
		params1 = save;
		err = snd_pcm_hw_param_set_max(pcm, &params1, SND_CHANGE, var, &max, &maxdir);
		if (err < 0)
			goto _end;
		if (boundary_nearer(max, maxdir, best, valdir, min, mindir)) {
			*params = params1;
			last = 1;
		}
	} else {
		*params = save;
		err = snd_pcm_hw_param_set_max(pcm, params, SND_CHANGE, var, &max, &maxdir);
		if (err < 0) {
			dump_hw_params(params, "set_near", var, *val, err);
			return err;
		}
		last = 1;
	}
 _end:
	if (last)
		err = snd_pcm_hw_param_set_last(pcm, params, var, val, dir);
	else
		err = snd_pcm_hw_param_set_first(pcm, params, var, val, dir);
	if (err < 0)
		dump_hw_params(params, "set_near", var, *val, err);
	return err;
}


static int snd_pcm_hw_param_set_near_minmax(snd_pcm_t *pcm,
					    snd_pcm_hw_params_t *params,
					    snd_pcm_hw_param_t var,
					    unsigned int min, int *mindir,
					    unsigned int max, int *maxdir)
{
	snd_pcm_hw_params_t tmp;
	int err;
	if (!boundary_lt(min, *mindir, max, *maxdir))
		return snd_pcm_hw_param_set_near(pcm, params, var, &min, mindir);
	tmp = *params;
	err = snd_pcm_hw_param_set_near(pcm, &tmp, var, &min, mindir);
	if (err < 0)
		return err;
	if (boundary_lt(min, *mindir, max, *maxdir)) {
		tmp = *params;
		err = snd_pcm_hw_param_set_near(pcm, &tmp, var, &max, maxdir);
	} else {
		max = min;
		*maxdir = *mindir;
	}
	err = snd_pcm_hw_param_set_minmax(pcm, params, SND_CHANGE, var, &min, mindir,
					  &max, maxdir);
	if (err < 0)
		return err;
	return 0;
}

int snd_pcm_hw_param_refine_near(snd_pcm_t *pcm,
				 snd_pcm_hw_params_t *params,
				 snd_pcm_hw_param_t var,
				 const snd_pcm_hw_params_t *src)
{
	unsigned int min, max;
	int mindir, maxdir, err;

	if ((err = snd_pcm_hw_param_get_min(src, var, &min, &mindir)) < 0)
		return err;
	if ((err = snd_pcm_hw_param_get_max(src, var, &max, &maxdir)) < 0)
		return err;
	if ((err = snd_pcm_hw_param_set_near_minmax(pcm, params, var,
						    min, &mindir, max, &maxdir)) < 0)
		return err;
	return 0;
}

int snd_pcm_hw_param_refine_multiple(snd_pcm_t *pcm,
				     snd_pcm_hw_params_t *params,
				     snd_pcm_hw_param_t var,
				     const snd_pcm_hw_params_t *src)
{
	const snd_interval_t *it = hw_param_interval_c(src, var);
	const snd_interval_t *st = hw_param_interval_c(params, var);
	if (snd_interval_single(it)) {
		unsigned int best = snd_interval_min(it), cur, prev;
		cur = best;
		for (;;) {
			if (st->max < cur || (st->max == cur && st->openmax))
				break;
			if (it->min <= cur && ! (it->min == cur && st->openmin)) {
				if (! snd_pcm_hw_param_set(pcm, params, SND_TRY, var, cur, 0))
					return 0; /* ok */
			}
			prev = cur;
			cur += best;
			if (cur <= prev)
				break;
		}
	}
	return snd_pcm_hw_param_refine_near(pcm, params, var, src);
}

/* ---- end of refinement functions ---- */

int snd_pcm_hw_param_empty(const snd_pcm_hw_params_t *params,
			   snd_pcm_hw_param_t var)
{
	if (hw_is_mask(var))
		return snd_mask_empty(hw_param_mask_c(params, var));
	if (hw_is_interval(var))
		return snd_interval_empty(hw_param_interval_c(params, var));
	assert(0);
	return -EINVAL;
}

int snd_pcm_hw_param_always_eq(const snd_pcm_hw_params_t *params,
			       snd_pcm_hw_param_t var,
			       const snd_pcm_hw_params_t *params1)
{
	if (hw_is_mask(var))
		return snd_mask_always_eq(hw_param_mask_c(params, var),
					  hw_param_mask_c(params1, var));
	if (hw_is_interval(var))
		return snd_interval_always_eq(hw_param_interval_c(params, var),
					      hw_param_interval_c(params1, var));
	assert(0);
	return -EINVAL;
}

int snd_pcm_hw_param_never_eq(const snd_pcm_hw_params_t *params,
			      snd_pcm_hw_param_t var,
			      const snd_pcm_hw_params_t *params1)
{
	if (hw_is_mask(var))
		return snd_mask_never_eq(hw_param_mask_c(params, var),
					 hw_param_mask_c(params1, var));
	if (hw_is_interval(var))
		return snd_interval_never_eq(hw_param_interval_c(params, var),
					     hw_param_interval_c(params1, var));
	assert(0);
	return -EINVAL;
}


/* Choose one configuration from configuration space defined by PARAMS
   The configuration chosen is that obtained fixing in this order:
   first access
   first format
   first subformat
   min channels
   min rate
   min period time
   max buffer size
   min tick time
*/
static int snd_pcm_hw_params_choose(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	int err;
	const char *compat = getenv("LIBASOUND_COMPAT");
#ifdef CHOOSE_DEBUG
	snd_output_t *log;
	snd_output_stdio_attach(&log, stderr, 0);
	snd_output_printf(log, "CHOOSE called:\n");
	snd_pcm_hw_params_dump(params, log);
#endif

	err = snd_pcm_hw_param_set_first(pcm, params, SND_PCM_HW_PARAM_ACCESS, NULL, 0);
	if (err < 0)
		return err;
	err = snd_pcm_hw_param_set_first(pcm, params, SND_PCM_HW_PARAM_FORMAT, NULL, 0);
	if (err < 0)
		return err;
	err = snd_pcm_hw_param_set_first(pcm, params, SND_PCM_HW_PARAM_SUBFORMAT, NULL, 0);
	if (err < 0)
		return err;
	err = snd_pcm_hw_param_set_first(pcm, params, SND_PCM_HW_PARAM_CHANNELS, NULL, 0);
	if (err < 0)
		return err;
	err = snd_pcm_hw_param_set_first(pcm, params, SND_PCM_HW_PARAM_RATE, NULL, 0);
	if (err < 0)
		return err;
	if (compat && *compat) {
		/* old mode */
		err = snd_pcm_hw_param_set_first(pcm, params, SND_PCM_HW_PARAM_PERIOD_TIME, NULL, 0);
		if (err < 0)
			return err;
		err = snd_pcm_hw_param_set_first(pcm, params, SND_PCM_HW_PARAM_PERIOD_SIZE, NULL, 0);
		if (err < 0)
			return err;
		err = snd_pcm_hw_param_set_last(pcm, params, SND_PCM_HW_PARAM_BUFFER_SIZE, NULL, 0);
		if (err < 0)
			return err;
	} else {
		/* determine buffer size first */
		err = snd_pcm_hw_param_set_last(pcm, params, SND_PCM_HW_PARAM_BUFFER_SIZE, NULL, 0);
		if (err < 0)
			return err;
		err = snd_pcm_hw_param_set_first(pcm, params, SND_PCM_HW_PARAM_PERIOD_SIZE, NULL, 0);
		if (err < 0)
			return err;
		err = snd_pcm_hw_param_set_first(pcm, params, SND_PCM_HW_PARAM_PERIOD_TIME, NULL, 0);
		if (err < 0)
			return err;
	}
	err = snd_pcm_hw_param_set_first(pcm, params, SND_PCM_HW_PARAM_TICK_TIME, NULL, 0);
	if (err < 0)
		return err;
#ifdef CHOOSE_DEBUG
	snd_output_printf(log, "choose done\n");
	snd_pcm_hw_params_dump(params, log);
	snd_output_close(log);
#endif
	return 0;
}


int _snd_pcm_hw_param_refine(snd_pcm_hw_params_t *params,
			     snd_pcm_hw_param_t var,
			     const snd_pcm_hw_params_t *src)
{
	int changed = 0;
	if (hw_is_mask(var)) {
		snd_mask_t *d = hw_param_mask(params, var);
		const snd_mask_t *s = hw_param_mask_c(src, var);
		changed = snd_mask_refine(d, s);
	} else if (hw_is_interval(var)) {
		snd_interval_t *d = hw_param_interval(params, var);
		const snd_interval_t *s = hw_param_interval_c(src, var);
		changed = snd_interval_refine(d, s);
	} else
		return 0; /* NOP / reserved */
	if (changed) {
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}
	return changed;
}
			     

void snd_pcm_hw_param_dump(const snd_pcm_hw_params_t *params,
			   snd_pcm_hw_param_t var, snd_output_t *out)
{
	if (hw_is_mask(var)) {
		const snd_mask_t *mask = hw_param_mask_c(params, var);
		if (snd_mask_empty(mask))
			snd_output_puts(out, " NONE");
		else if (snd_mask_full(mask))
			snd_output_puts(out, " ALL");
		else {
			unsigned int k;
			for (k = 0; k <= SND_MASK_MAX; ++k) {
				if (snd_mask_test(mask, k)) {
					const char *s;
					switch (var) {
					case SND_PCM_HW_PARAM_ACCESS:
						s = snd_pcm_access_name(k);
						break;
					case SND_PCM_HW_PARAM_FORMAT:
						s = snd_pcm_format_name(k);
						break;
					case SND_PCM_HW_PARAM_SUBFORMAT:
						s = snd_pcm_subformat_name(k);
						break;
					default:
						assert(0);
						s = NULL;
					}
					if (s) {
						snd_output_putc(out, ' ');
						snd_output_puts(out, s);
					}
				}
			}
		}
		return;
	}
	if (hw_is_interval(var)) {
		snd_interval_print(hw_param_interval_c(params, var), out);
		return;
	}
	assert(0);
}

#define HW_PARAM(v) [SND_PCM_HW_PARAM_##v] = #v

static const char *const snd_pcm_hw_param_names[] = {
	HW_PARAM(ACCESS),
	HW_PARAM(FORMAT),
	HW_PARAM(SUBFORMAT),
	HW_PARAM(SAMPLE_BITS),
	HW_PARAM(FRAME_BITS),
	HW_PARAM(CHANNELS),
	HW_PARAM(RATE),
	HW_PARAM(PERIOD_TIME),
	HW_PARAM(PERIOD_SIZE),
	HW_PARAM(PERIOD_BYTES),
	HW_PARAM(PERIODS),
	HW_PARAM(BUFFER_TIME),
	HW_PARAM(BUFFER_SIZE),
	HW_PARAM(BUFFER_BYTES),
	HW_PARAM(TICK_TIME),
};

const char *snd_pcm_hw_param_name(snd_pcm_hw_param_t param)
{
	assert(param <= SND_PCM_HW_PARAM_LAST_INTERVAL);
	return snd_pcm_hw_param_names[param];
}


typedef struct _snd_pcm_hw_rule snd_pcm_hw_rule_t;

typedef int (*snd_pcm_hw_rule_func_t)(snd_pcm_hw_params_t *params,
				      const snd_pcm_hw_rule_t *rule);

struct _snd_pcm_hw_rule {
	int var;
	snd_pcm_hw_rule_func_t func;
	int deps[4];
	void *private_data;
};

static int snd_pcm_hw_rule_mul(snd_pcm_hw_params_t *params,
			       const snd_pcm_hw_rule_t *rule)
{
	snd_interval_t t;
	snd_interval_mul(hw_param_interval_c(params, rule->deps[0]),
		     hw_param_interval_c(params, rule->deps[1]), &t);
	return snd_interval_refine(hw_param_interval(params, rule->var), &t);
}

static int snd_pcm_hw_rule_div(snd_pcm_hw_params_t *params,
			const snd_pcm_hw_rule_t *rule)
{
	snd_interval_t t;
	snd_interval_div(hw_param_interval_c(params, rule->deps[0]),
		     hw_param_interval_c(params, rule->deps[1]), &t);
	return snd_interval_refine(hw_param_interval(params, rule->var), &t);
}

static int snd_pcm_hw_rule_muldivk(snd_pcm_hw_params_t *params,
				   const snd_pcm_hw_rule_t *rule)
{
	snd_interval_t t;
	snd_interval_muldivk(hw_param_interval_c(params, rule->deps[0]),
			 hw_param_interval_c(params, rule->deps[1]),
			 (unsigned long) rule->private_data, &t);
	return snd_interval_refine(hw_param_interval(params, rule->var), &t);
}

static int snd_pcm_hw_rule_mulkdiv(snd_pcm_hw_params_t *params,
				   const snd_pcm_hw_rule_t *rule)
{
	snd_interval_t t;
	snd_interval_mulkdiv(hw_param_interval_c(params, rule->deps[0]),
			 (unsigned long) rule->private_data,
			 hw_param_interval_c(params, rule->deps[1]), &t);
	return snd_interval_refine(hw_param_interval(params, rule->var), &t);
}

static int snd_pcm_hw_rule_format(snd_pcm_hw_params_t *params,
				  const snd_pcm_hw_rule_t *rule)
{
	int changed = 0;
	snd_pcm_format_t k;
	snd_mask_t *mask = hw_param_mask(params, rule->var);
	snd_interval_t *i = hw_param_interval(params, rule->deps[0]);
	for (k = 0; k <= SND_PCM_FORMAT_LAST; k++) {
		int bits;
		if (!snd_pcm_format_mask_test(mask, k))
			continue;
		bits = snd_pcm_format_physical_width(k);
		if (bits < 0)
			continue;
		if (!snd_interval_test(i, (unsigned int) bits)) {
			snd_pcm_format_mask_reset(mask, k);
			if (snd_mask_empty(mask))
				return -EINVAL;
			changed = 1;
		}
	}
	return changed;
}


static int snd_pcm_hw_rule_sample_bits(snd_pcm_hw_params_t *params,
				       const snd_pcm_hw_rule_t *rule)
{
	unsigned int min, max;
	snd_pcm_format_t k;
	snd_interval_t *i = hw_param_interval(params, rule->var);
	snd_mask_t *mask = hw_param_mask(params, rule->deps[0]);
	int c, changed = 0;
	min = UINT_MAX;
	max = 0;
	for (k = 0; k <= SND_PCM_FORMAT_LAST; k++) {
		int bits;
		if (!snd_pcm_format_mask_test(mask, k))
			continue;
		bits = snd_pcm_format_physical_width(k);
		if (bits < 0)
			continue;
		if (min > (unsigned)bits)
			min = bits;
		if (max < (unsigned)bits)
			max = bits;
	}
	c = snd_interval_refine_min(i, min, 0);
	if (c < 0)
		return c;
	if (c)
		changed = 1;
	c = snd_interval_refine_max(i, max, 0);
	if (c < 0)
		return c;
	if (c)
		changed = 1;
	return changed;
}

static const snd_pcm_hw_rule_t refine_rules[] = {
	{
		.var = SND_PCM_HW_PARAM_FORMAT,
		.func = snd_pcm_hw_rule_format,
		.deps = { SND_PCM_HW_PARAM_SAMPLE_BITS, -1 },
		.private_data = 0,
	},
	{
		.var = SND_PCM_HW_PARAM_SAMPLE_BITS, 
		.func = snd_pcm_hw_rule_sample_bits,
		.deps = { SND_PCM_HW_PARAM_FORMAT, 
			SND_PCM_HW_PARAM_SAMPLE_BITS, -1 },
		.private_data = 0,
	},
	{
		.var = SND_PCM_HW_PARAM_SAMPLE_BITS, 
		.func = snd_pcm_hw_rule_div,
		.deps = { SND_PCM_HW_PARAM_FRAME_BITS,
			SND_PCM_HW_PARAM_CHANNELS, -1 },
		.private_data = 0,
	},
	{
		.var = SND_PCM_HW_PARAM_FRAME_BITS, 
		.func = snd_pcm_hw_rule_mul,
		.deps = { SND_PCM_HW_PARAM_SAMPLE_BITS,
			SND_PCM_HW_PARAM_CHANNELS, -1 },
		.private_data = 0,
	},
	{
		.var = SND_PCM_HW_PARAM_FRAME_BITS, 
		.func = snd_pcm_hw_rule_mulkdiv,
		.deps = { SND_PCM_HW_PARAM_PERIOD_BYTES,
			SND_PCM_HW_PARAM_PERIOD_SIZE, -1 },
		.private_data = (void*) 8,
	},
	{
		.var = SND_PCM_HW_PARAM_FRAME_BITS, 
		.func = snd_pcm_hw_rule_mulkdiv,
		.deps = { SND_PCM_HW_PARAM_BUFFER_BYTES,
			SND_PCM_HW_PARAM_BUFFER_SIZE, -1 },
		.private_data = (void*) 8,
	},
	{
		.var = SND_PCM_HW_PARAM_CHANNELS, 
		.func = snd_pcm_hw_rule_div,
		.deps = { SND_PCM_HW_PARAM_FRAME_BITS,
			SND_PCM_HW_PARAM_SAMPLE_BITS, -1 },
		.private_data = 0,
	},
	{
		.var = SND_PCM_HW_PARAM_RATE, 
		.func = snd_pcm_hw_rule_mulkdiv,
		.deps = { SND_PCM_HW_PARAM_PERIOD_SIZE,
			SND_PCM_HW_PARAM_PERIOD_TIME, -1 },
		.private_data = (void*) 1000000,
	},
	{
		.var = SND_PCM_HW_PARAM_RATE, 
		.func = snd_pcm_hw_rule_mulkdiv,
		.deps = { SND_PCM_HW_PARAM_BUFFER_SIZE,
			SND_PCM_HW_PARAM_BUFFER_TIME, -1 },
		.private_data = (void*) 1000000,
	},
	{
		.var = SND_PCM_HW_PARAM_PERIODS, 
		.func = snd_pcm_hw_rule_div,
		.deps = { SND_PCM_HW_PARAM_BUFFER_SIZE,
			SND_PCM_HW_PARAM_PERIOD_SIZE, -1 },
		.private_data = 0,
	},
	{
		.var = SND_PCM_HW_PARAM_PERIOD_SIZE, 
		.func = snd_pcm_hw_rule_div,
		.deps = { SND_PCM_HW_PARAM_BUFFER_SIZE,
			SND_PCM_HW_PARAM_PERIODS, -1 },
		.private_data = 0,
	},
	{
		.var = SND_PCM_HW_PARAM_PERIOD_SIZE, 
		.func = snd_pcm_hw_rule_mulkdiv,
		.deps = { SND_PCM_HW_PARAM_PERIOD_BYTES,
			SND_PCM_HW_PARAM_FRAME_BITS, -1 },
		.private_data = (void*) 8,
	},
	{
		.var = SND_PCM_HW_PARAM_PERIOD_SIZE, 
		.func = snd_pcm_hw_rule_muldivk,
		.deps = { SND_PCM_HW_PARAM_PERIOD_TIME,
			SND_PCM_HW_PARAM_RATE, -1 },
		.private_data = (void*) 1000000,
	},
	{
		.var = SND_PCM_HW_PARAM_BUFFER_SIZE, 
		.func = snd_pcm_hw_rule_mul,
		.deps = { SND_PCM_HW_PARAM_PERIOD_SIZE,
			SND_PCM_HW_PARAM_PERIODS, -1 },
		.private_data = 0,
	},
	{
		.var = SND_PCM_HW_PARAM_BUFFER_SIZE, 
		.func = snd_pcm_hw_rule_mulkdiv,
		.deps = { SND_PCM_HW_PARAM_BUFFER_BYTES,
			SND_PCM_HW_PARAM_FRAME_BITS, -1 },
		.private_data = (void*) 8,
	},
	{
		.var = SND_PCM_HW_PARAM_BUFFER_SIZE, 
		.func = snd_pcm_hw_rule_muldivk,
		.deps = { SND_PCM_HW_PARAM_BUFFER_TIME,
			SND_PCM_HW_PARAM_RATE, -1 },
		.private_data = (void*) 1000000,
	},
	{
		.var = SND_PCM_HW_PARAM_PERIOD_BYTES, 
		.func = snd_pcm_hw_rule_muldivk,
		.deps = { SND_PCM_HW_PARAM_PERIOD_SIZE,
			SND_PCM_HW_PARAM_FRAME_BITS, -1 },
		.private_data = (void*) 8,
	},
	{
		.var = SND_PCM_HW_PARAM_BUFFER_BYTES, 
		.func = snd_pcm_hw_rule_muldivk,
		.deps = { SND_PCM_HW_PARAM_BUFFER_SIZE,
			SND_PCM_HW_PARAM_FRAME_BITS, -1 },
		.private_data = (void*) 8,
	},
	{
		.var = SND_PCM_HW_PARAM_PERIOD_TIME, 
		.func = snd_pcm_hw_rule_mulkdiv,
		.deps = { SND_PCM_HW_PARAM_PERIOD_SIZE,
			SND_PCM_HW_PARAM_RATE, -1 },
		.private_data = (void*) 1000000,
	},
	{
		.var = SND_PCM_HW_PARAM_BUFFER_TIME, 
		.func = snd_pcm_hw_rule_mulkdiv,
		.deps = { SND_PCM_HW_PARAM_BUFFER_SIZE,
			SND_PCM_HW_PARAM_RATE, -1 },
		.private_data = (void*) 1000000,
	},
};

#define RULES (sizeof(refine_rules) / sizeof(refine_rules[0]))

static const snd_mask_t refine_masks[SND_PCM_HW_PARAM_LAST_MASK - SND_PCM_HW_PARAM_FIRST_MASK + 1] = {
	[SND_PCM_HW_PARAM_ACCESS - SND_PCM_HW_PARAM_FIRST_MASK] = {
		.bits = { 0x1f },
	},
	[SND_PCM_HW_PARAM_FORMAT - SND_PCM_HW_PARAM_FIRST_MASK] = {
		.bits = { 0x81ffffff, 0xfff},
	},
	[SND_PCM_HW_PARAM_SUBFORMAT - SND_PCM_HW_PARAM_FIRST_MASK] = {
		.bits = { 0x1 },
	},
};
  
static const snd_interval_t refine_intervals[SND_PCM_HW_PARAM_LAST_INTERVAL - SND_PCM_HW_PARAM_FIRST_INTERVAL + 1] = {
	[SND_PCM_HW_PARAM_SAMPLE_BITS - SND_PCM_HW_PARAM_FIRST_INTERVAL] = {
		.min = 1, .max = UINT_MAX,
		.openmin = 0, .openmax = 0, .integer = 1, .empty = 0,
	},
	[SND_PCM_HW_PARAM_FRAME_BITS - SND_PCM_HW_PARAM_FIRST_INTERVAL] = {
		.min = 1, .max = UINT_MAX,
		.openmin = 0, .openmax = 0, .integer = 1, .empty = 0,
	},
	[SND_PCM_HW_PARAM_CHANNELS - SND_PCM_HW_PARAM_FIRST_INTERVAL] = {
		.min = 1, .max = UINT_MAX,
		.openmin = 0, .openmax = 0, .integer = 1, .empty = 0,
	},
	[SND_PCM_HW_PARAM_RATE - SND_PCM_HW_PARAM_FIRST_INTERVAL] = {
		.min = 1, .max = UINT_MAX,
		.openmin = 0, .openmax = 0, .integer = 0, .empty = 0,
	},
	[SND_PCM_HW_PARAM_PERIOD_TIME - SND_PCM_HW_PARAM_FIRST_INTERVAL] = {
		.min = 0, .max = UINT_MAX,
		.openmin = 0, .openmax = 0, .integer = 0, .empty = 0,
	},
	[SND_PCM_HW_PARAM_PERIOD_SIZE - SND_PCM_HW_PARAM_FIRST_INTERVAL] = {
		.min = 0, .max = UINT_MAX,
		.openmin = 0, .openmax = 0, .integer = 0, .empty = 0,
	},
	[SND_PCM_HW_PARAM_PERIOD_BYTES - SND_PCM_HW_PARAM_FIRST_INTERVAL] = {
		.min = 0, .max = UINT_MAX,
		.openmin = 0, .openmax = 0, .integer = 0, .empty = 0,
	},
	[SND_PCM_HW_PARAM_PERIODS - SND_PCM_HW_PARAM_FIRST_INTERVAL] = {
		.min = 0, .max = UINT_MAX,
		.openmin = 0, .openmax = 0, .integer = 0, .empty = 0,
	},
	[SND_PCM_HW_PARAM_BUFFER_TIME - SND_PCM_HW_PARAM_FIRST_INTERVAL] = {
		.min = 1, .max = UINT_MAX,
		.openmin = 0, .openmax = 0, .integer = 0, .empty = 0,
	},
	[SND_PCM_HW_PARAM_BUFFER_SIZE - SND_PCM_HW_PARAM_FIRST_INTERVAL] = {
		.min = 1, .max = UINT_MAX,
		.openmin = 0, .openmax = 0, .integer = 1, .empty = 0,
	},
	[SND_PCM_HW_PARAM_BUFFER_BYTES - SND_PCM_HW_PARAM_FIRST_INTERVAL] = {
		.min = 1, .max = UINT_MAX,
		.openmin = 0, .openmax = 0, .integer = 1, .empty = 0,
	},
	[SND_PCM_HW_PARAM_TICK_TIME - SND_PCM_HW_PARAM_FIRST_INTERVAL] = {
		.min = 0, .max = UINT_MAX,
		.openmin = 0, .openmax = 0, .integer = 0, .empty = 0,
	},
};


int snd_pcm_hw_refine_soft(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params)
{
	unsigned int k;
	snd_interval_t *i;
	unsigned int rstamps[RULES];
	unsigned int vstamps[SND_PCM_HW_PARAM_LAST_INTERVAL + 1];
	unsigned int stamp = 2;
	int changed, again;
#ifdef RULES_DEBUG
	snd_output_t *log;
	snd_output_stdio_attach(&log, stderr, 0);
	snd_output_printf(log, "refine_soft '%s' (begin)\n", pcm->name);
	snd_pcm_hw_params_dump(params, log);
#endif

	for (k = SND_PCM_HW_PARAM_FIRST_MASK; k <= SND_PCM_HW_PARAM_LAST_MASK; k++) {
		if (!(params->rmask & (1 << k)))
			continue;
		changed = snd_mask_refine(hw_param_mask(params, k),
					  &refine_masks[k - SND_PCM_HW_PARAM_FIRST_MASK]);
		if (changed)
			params->cmask |= 1 << k;
		if (changed < 0)
			goto _err;
	}

	for (k = SND_PCM_HW_PARAM_FIRST_INTERVAL; k <= SND_PCM_HW_PARAM_LAST_INTERVAL; k++) {
		if (!(params->rmask & (1 << k)))
			continue;
		changed = snd_interval_refine(hw_param_interval(params, k),
				      &refine_intervals[k - SND_PCM_HW_PARAM_FIRST_INTERVAL]);
		if (changed)
			params->cmask |= 1 << k;
		if (changed < 0)
			goto _err;
	}

	for (k = 0; k < RULES; k++)
		rstamps[k] = 0;
	for (k = 0; k <= SND_PCM_HW_PARAM_LAST_INTERVAL; k++)
		vstamps[k] = (params->rmask & (1 << k)) ? 1 : 0;
	do {
		again = 0;
		for (k = 0; k < RULES; k++) {
			const snd_pcm_hw_rule_t *r = &refine_rules[k];
			unsigned int d;
			int doit = 0;
			for (d = 0; r->deps[d] >= 0; d++) {
				if (vstamps[r->deps[d]] > rstamps[k]) {
					doit = 1;
					break;
				}
			}
			if (!doit)
				continue;
#ifdef RULES_DEBUG
			snd_output_printf(log, "Rule %d (%p): ", k, r->func);
			if (r->var >= 0) {
				snd_output_printf(log, "%s=", snd_pcm_hw_param_name(r->var));
				snd_pcm_hw_param_dump(params, r->var, log);
				snd_output_puts(log, " -> ");
			}
#endif
			changed = r->func(params, r);
#ifdef RULES_DEBUG
			if (r->var >= 0)
				snd_pcm_hw_param_dump(params, r->var, log);
			for (d = 0; r->deps[d] >= 0; d++) {
				snd_output_printf(log, " %s=", snd_pcm_hw_param_name(r->deps[d]));
				snd_pcm_hw_param_dump(params, r->deps[d], log);
			}
			snd_output_putc(log, '\n');
#endif
			rstamps[k] = stamp;
			if (changed && r->var >= 0) {
				params->cmask |= 1 << r->var;
				vstamps[r->var] = stamp;
				again = 1;
			}
			if (changed < 0)
				goto _err;
			stamp++;
		}
	} while (again);
	if (!params->msbits) {
		i = hw_param_interval(params, SND_PCM_HW_PARAM_SAMPLE_BITS);
		if (snd_interval_single(i))
			params->msbits = snd_interval_value(i);
	}

	if (!params->rate_den) {
		i = hw_param_interval(params, SND_PCM_HW_PARAM_RATE);
		if (snd_interval_single(i)) {
			params->rate_num = snd_interval_value(i);
			params->rate_den = 1;
		}
	}
	params->rmask = 0;
	return 0;
 _err:
#ifdef RULES_DEBUG
	snd_output_printf(log, "refine_soft '%s' (end-%i)\n", pcm->name, changed);
	snd_pcm_hw_params_dump(params, log);
	snd_output_close(log);
#endif
	return changed;
}

int _snd_pcm_hw_params_refine(snd_pcm_hw_params_t *params,
			      unsigned int vars,
			      const snd_pcm_hw_params_t *src)
{
	int changed, err = 0;
	unsigned int k;
	for (k = 0; k <= SND_PCM_HW_PARAM_LAST_INTERVAL; ++k) {
		if (!(vars & (1 << k)))
			continue;
		changed = _snd_pcm_hw_param_refine(params, k, src);
		if (changed < 0)
			err = changed;
	}
	params->info &= src->info;
	return err;
}

int snd_pcm_hw_refine_slave(snd_pcm_t *pcm, snd_pcm_hw_params_t *params,
			    int (*cprepare)(snd_pcm_t *pcm,
					    snd_pcm_hw_params_t *params),
			    int (*cchange)(snd_pcm_t *pcm,
					   snd_pcm_hw_params_t *params,
					   snd_pcm_hw_params_t *sparams),
			    int (*sprepare)(snd_pcm_t *pcm,
					    snd_pcm_hw_params_t *params),
			    int (*schange)(snd_pcm_t *pcm,
					   snd_pcm_hw_params_t *params,
					   snd_pcm_hw_params_t *sparams),
			    int (*srefine)(snd_pcm_t *pcm,
					   snd_pcm_hw_params_t *sparams))

{
#ifdef RULES_DEBUG
	snd_output_t *log;
#endif
	snd_pcm_hw_params_t sparams;
	int err;
	unsigned int cmask, changed;
#ifdef RULES_DEBUG
	snd_output_stdio_attach(&log, stderr, 0);
#endif
	err = cprepare(pcm, params);
	if (err < 0)
		return err;
	err = sprepare(pcm, &sparams);
	if (err < 0) {
		SNDERR("Slave PCM not usable");
		return err;
	}
#ifdef RULES_DEBUG
	snd_output_printf(log, "hw_refine_slave - enter '%s'\n", pcm->name);
#endif
	do {
		cmask = params->cmask;
		params->cmask = 0;
#ifdef RULES_DEBUG
		snd_output_printf(log, "schange '%s' (client)\n", pcm->name);
		snd_pcm_hw_params_dump(params, log);
		snd_output_printf(log, "schange '%s' (slave)\n", pcm->name);
		snd_pcm_hw_params_dump(&sparams, log);
#endif
		err = schange(pcm, params, &sparams);
		if (err >= 0) {
#ifdef RULES_DEBUG
			snd_output_printf(log, "srefine '%s' (client)\n", pcm->name);
			snd_pcm_hw_params_dump(params, log);
			snd_output_printf(log, "srefine '%s' (slave)\n", pcm->name);
			snd_pcm_hw_params_dump(&sparams, log);
#endif
			err = srefine(pcm, &sparams);
			if (err < 0) {
#ifdef RULES_DEBUG
				snd_output_printf(log, "srefine '%s', err < 0 (%i) (client)\n", pcm->name, err);
				snd_pcm_hw_params_dump(params, log);
				snd_output_printf(log, "srefine '%s', err < 0 (%i) (slave)\n", pcm->name, err);
				snd_pcm_hw_params_dump(&sparams, log);
#endif
				cchange(pcm, params, &sparams);
				return err;
			}
		} else {
#ifdef RULES_DEBUG
			snd_output_printf(log, "schange '%s', err < 0 (%i) (client)\n", pcm->name, err);
			snd_pcm_hw_params_dump(params, log);
			snd_output_printf(log, "schange '%s', err < 0 (%i) (slave)\n", pcm->name, err);
			snd_pcm_hw_params_dump(&sparams, log);
#endif
			cchange(pcm, params, &sparams);
			return err;
		}
#ifdef RULES_DEBUG
		snd_output_printf(log, "cchange '%s'\n", pcm->name);
#endif
		err = cchange(pcm, params, &sparams);
		if (err < 0)
			return err;
#ifdef RULES_DEBUG
		snd_output_printf(log, "refine_soft '%s'\n", pcm->name);
#endif
		err = snd_pcm_hw_refine_soft(pcm, params);
		changed = params->cmask;
		params->cmask |= cmask;
		if (err < 0)
			return err;
#ifdef RULES_DEBUG
		snd_output_printf(log, "refine_soft ok '%s'\n", pcm->name);
#endif
	} while (changed);
#ifdef RULES_DEBUG
	snd_output_printf(log, "refine_slave - leave '%s'\n", pcm->name);
	snd_output_close(log);
#endif
	return 0;
}

int snd_pcm_hw_params_slave(snd_pcm_t *pcm, snd_pcm_hw_params_t *params,
			    int (*cchange)(snd_pcm_t *pcm,
					   snd_pcm_hw_params_t *params,
					   snd_pcm_hw_params_t *sparams),
			    int (*sprepare)(snd_pcm_t *pcm,
					    snd_pcm_hw_params_t *params),
			    int (*schange)(snd_pcm_t *pcm,
					   snd_pcm_hw_params_t *params,
					   snd_pcm_hw_params_t *sparams),
			    int (*sparams)(snd_pcm_t *pcm,
					   snd_pcm_hw_params_t *sparams))

{
	snd_pcm_hw_params_t slave_params;
	int err;
	err = sprepare(pcm, &slave_params);
	assert(err >= 0);
	err = schange(pcm, params, &slave_params);
	assert(err >= 0);
	err = sparams(pcm, &slave_params);
	if (err < 0)
		cchange(pcm, params, &slave_params);
	return err;
}

static int snd_pcm_sw_params_default(snd_pcm_t *pcm, snd_pcm_sw_params_t *params)
{
	assert(pcm && params);
	assert(pcm->setup);
	params->tstamp_mode = SND_PCM_TSTAMP_NONE;
	params->period_step = 1;
	params->sleep_min = 0;
	params->avail_min = pcm->period_size;
	params->xfer_align = 1;
	params->start_threshold = 1;
	params->stop_threshold = pcm->buffer_size;
	params->silence_threshold = 0;
	params->silence_size = 0;
	params->boundary = pcm->buffer_size;
	while (params->boundary * 2 <= LONG_MAX - pcm->buffer_size)
		params->boundary *= 2;
	return 0;
}


int snd_pcm_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	int res;
#ifdef REFINE_DEBUG
	snd_output_t *log;
	snd_output_stdio_attach(&log, stderr, 0);
#endif
	assert(pcm && params);
#ifdef REFINE_DEBUG
	snd_output_printf(log, "REFINE called:\n");
	snd_pcm_hw_params_dump(params, log);
#endif
	res = pcm->ops->hw_refine(pcm->op_arg, params);
#ifdef REFINE_DEBUG
	snd_output_printf(log, "refine done - result = %i\n", res);
	snd_pcm_hw_params_dump(params, log);
	snd_output_close(log);
#endif
	return res;
}

/* Install one of the configurations present in configuration
   space defined by PARAMS.
   The configuration chosen is that obtained fixing in this order:
   first access
   first format
   first subformat
   min channels
   min rate
   min period_size
   max periods
   Return 0 on success otherwise a negative error code
*/
int _snd_pcm_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	int err;
	snd_pcm_sw_params_t sw;
	int fb, min_align;
	err = snd_pcm_hw_refine(pcm, params);
	if (err < 0)
		return err;
	snd_pcm_hw_params_choose(pcm, params);
	if (pcm->setup) {
		err = snd_pcm_hw_free(pcm);
		if (err < 0)
			return err;
	}
	err = pcm->ops->hw_params(pcm->op_arg, params);
	if (err < 0)
		return err;

	pcm->setup = 1;
	INTERNAL(snd_pcm_hw_params_get_access)(params, &pcm->access);
	INTERNAL(snd_pcm_hw_params_get_format)(params, &pcm->format);
	INTERNAL(snd_pcm_hw_params_get_subformat)(params, &pcm->subformat);
	INTERNAL(snd_pcm_hw_params_get_channels)(params, &pcm->channels);
	INTERNAL(snd_pcm_hw_params_get_rate)(params, &pcm->rate, 0);
	INTERNAL(snd_pcm_hw_params_get_period_time)(params, &pcm->period_time, 0);
	INTERNAL(snd_pcm_hw_params_get_period_size)(params, &pcm->period_size, 0);
	INTERNAL(snd_pcm_hw_params_get_buffer_size)(params, &pcm->buffer_size);
	pcm->sample_bits = snd_pcm_format_physical_width(pcm->format);
	pcm->frame_bits = pcm->sample_bits * pcm->channels;
	fb = pcm->frame_bits;
	min_align = 1;
	while (fb % 8) {
		fb *= 2;
		min_align *= 2;
	}
	pcm->min_align = min_align;
	
	pcm->hw_flags = params->flags;
	pcm->info = params->info;
	pcm->msbits = params->msbits;
	pcm->rate_num = params->rate_num;
	pcm->rate_den = params->rate_den;
	pcm->fifo_size = params->fifo_size;
	
	/* Default sw params */
	memset(&sw, 0, sizeof(sw));
	snd_pcm_sw_params_default(pcm, &sw);
	err = snd_pcm_sw_params(pcm, &sw);
	assert(err >= 0);

	if (pcm->mmap_rw || 
	    pcm->access == SND_PCM_ACCESS_MMAP_INTERLEAVED ||
	    pcm->access == SND_PCM_ACCESS_MMAP_NONINTERLEAVED ||
	    pcm->access == SND_PCM_ACCESS_MMAP_COMPLEX) {
		err = snd_pcm_mmap(pcm);
	}
	if (err < 0)
		return err;
	return 0;
}
