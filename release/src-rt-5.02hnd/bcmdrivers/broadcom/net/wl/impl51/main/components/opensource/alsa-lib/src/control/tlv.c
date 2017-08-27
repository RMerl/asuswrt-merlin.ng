/**
 * \file control/tlv.c
 * \brief dB conversion functions from control TLV information
 * \author Takashi Iwai <tiwai@suse.de>
 * \date 2007
 */
/*
 *  Control Interface - dB conversion functions from control TLV information
 *
 *  Copyright (c) 2007 Takashi Iwai <tiwai@suse.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifndef HAVE_SOFT_FLOAT
#include <math.h>
#endif
#include "control_local.h"

#ifndef DOC_HIDDEN
/* convert to index of integer array */
#define int_index(size)	(((size) + sizeof(int) - 1) / sizeof(int))
/* max size of a TLV entry for dB information (including compound one) */
#define MAX_TLV_RANGE_SIZE	256
#endif

/**
 * \brief Parse TLV stream and retrieve dB information
 * \param tlv the TLV source
 * \param tlv_size the byte size of TLV source
 * \param db_tlvp the pointer stored the dB TLV information
 * \return the byte size of dB TLV information if found in the given
 *   TLV source, or a negative error code.
 *
 * This function parses the given TLV source and stores the TLV start
 * point if the TLV information regarding dB conversion is found.
 * The stored TLV pointer can be passed to the convesion functions
 * #snd_tlv_convert_to_dB(), #snd_tlv_convert_from_dB() and
 * #snd_tlv_get_dB_range().
 */
int snd_tlv_parse_dB_info(unsigned int *tlv,
			  unsigned int tlv_size,
			  unsigned int **db_tlvp)
{
	unsigned int type;
	unsigned int size;
	int err;

	*db_tlvp = NULL;
	type = tlv[0];
	size = tlv[1];
	tlv_size -= 2 * sizeof(int);
	if (size > tlv_size) {
		SNDERR("TLV size error");
		return -EINVAL;
	}
	switch (type) {
	case SND_CTL_TLVT_CONTAINER:
		size = int_index(size) * sizeof(int);
		tlv += 2;
		while (size > 0) {
			unsigned int len;
			err = snd_tlv_parse_dB_info(tlv, size, db_tlvp);
			if (err < 0)
				return err; /* error */
			if (err > 0)
				return err; /* found */
			len = int_index(tlv[1]) + 2;
			size -= len * sizeof(int);
			tlv += len;
		}
		break;
	case SND_CTL_TLVT_DB_SCALE:
	case SND_CTL_TLVT_DB_MINMAX:
	case SND_CTL_TLVT_DB_MINMAX_MUTE:
#ifndef HAVE_SOFT_FLOAT
	case SND_CTL_TLVT_DB_LINEAR:
#endif
	case SND_CTL_TLVT_DB_RANGE: {
		unsigned int minsize;
		if (type == SND_CTL_TLVT_DB_RANGE)
			minsize = 4 * sizeof(int);
		else
			minsize = 2 * sizeof(int);
		if (size < minsize) {
			SNDERR("Invalid dB_scale TLV size");
			return -EINVAL;
		}
		if (size > MAX_TLV_RANGE_SIZE) {
			SNDERR("Too big dB_scale TLV size: %d", size);
			return -EINVAL;
		}
		*db_tlvp = tlv;
		return size + sizeof(int) * 2;
	}
	default:
		break;
	}
	return -EINVAL; /* not found */
}

/**
 * \brief Get the dB min/max values
 * \param tlv the TLV source returned by #snd_tlv_parse_dB_info()
 * \param rangemin the minimum value of the raw volume
 * \param rangemax the maximum value of the raw volume
 * \param min the pointer to store the minimum dB value (in 0.01dB unit)
 * \param max the pointer to store the maximum dB value (in 0.01dB unit)
 * \return 0 if successful, or a negative error code
 */
int snd_tlv_get_dB_range(unsigned int *tlv, long rangemin, long rangemax,
			 long *min, long *max)
{
	int err;

	switch (tlv[0]) {
	case SND_CTL_TLVT_DB_RANGE: {
		unsigned int pos, len;
		len = int_index(tlv[1]);
		if (len > MAX_TLV_RANGE_SIZE)
			return -EINVAL;
		pos = 2;
		while (pos + 4 <= len) {
			long rmin, rmax;
			rangemin = (int)tlv[pos];
			rangemax = (int)tlv[pos + 1];
			err = snd_tlv_get_dB_range(tlv + pos + 2,
						   rangemin, rangemax,
						   &rmin, &rmax);
			if (err < 0)
				return err;
			if (pos > 2) {
				if (rmin < *min)
					*min = rmin;
				if (rmax > *max)
					*max = rmax;
			} else {
				*min = rmin;
				*max = rmax;
			}
			pos += int_index(tlv[pos + 3]) + 4;
		}
		return 0;
	}
	case SND_CTL_TLVT_DB_SCALE: {
		int step;
		*min = (int)tlv[2];
		step = (tlv[3] & 0xffff);
		*max = *min + (long)(step * (rangemax - rangemin));
		return 0;
	}
	case SND_CTL_TLVT_DB_MINMAX:
	case SND_CTL_TLVT_DB_MINMAX_MUTE:
	case SND_CTL_TLVT_DB_LINEAR:
		*min = (int)tlv[2];
		*max = (int)tlv[3];
		return 0;
	}
	return -EINVAL;
}

/**
 * \brief Convert the given raw volume value to a dB gain
 * \param tlv the TLV source returned by #snd_tlv_parse_dB_info()
 * \param rangemin the minimum value of the raw volume
 * \param rangemax the maximum value of the raw volume
 * \param volume the raw volume value to convert
 * \param db_gain the dB gain (in 0.01dB unit)
 * \return 0 if successful, or a negative error code
 */
int snd_tlv_convert_to_dB(unsigned int *tlv, long rangemin, long rangemax,
			  long volume, long *db_gain)
{
	switch (tlv[0]) {
	case SND_CTL_TLVT_DB_RANGE: {
		unsigned int pos, len;
		len = int_index(tlv[1]);
		if (len > MAX_TLV_RANGE_SIZE)
			return -EINVAL;
		pos = 2;
		while (pos + 4 <= len) {
			rangemin = (int)tlv[pos];
			rangemax = (int)tlv[pos + 1];
			if (volume >= rangemin && volume <= rangemax)
				return snd_tlv_convert_to_dB(tlv + pos + 2,
							     rangemin, rangemax,
							     volume, db_gain);
			pos += int_index(tlv[pos + 3]) + 4;
		}
		return -EINVAL;
	}
	case SND_CTL_TLVT_DB_SCALE: {
		int min, step, mute;
		min = tlv[2];
		step = (tlv[3] & 0xffff);
		mute = (tlv[3] >> 16) & 1;
		if (mute && volume == rangemin)
			*db_gain = SND_CTL_TLV_DB_GAIN_MUTE;
		else
			*db_gain = (volume - rangemin) * step + min;
		return 0;
	}
	case SND_CTL_TLVT_DB_MINMAX:
	case SND_CTL_TLVT_DB_MINMAX_MUTE: {
		int mindb, maxdb;
		mindb = tlv[2];
		maxdb = tlv[3];
		if (volume <= rangemin || rangemax <= rangemin) {
			if (tlv[0] == SND_CTL_TLVT_DB_MINMAX_MUTE)
				*db_gain = SND_CTL_TLV_DB_GAIN_MUTE;
			else
				*db_gain = mindb;
		} else if (volume >= rangemax)
			*db_gain = maxdb;
		else
			*db_gain = (maxdb - mindb) * (volume - rangemin) /
				(rangemax - rangemin) + mindb;
		return 0;
	}
#ifndef HAVE_SOFT_FLOAT
	case SND_CTL_TLVT_DB_LINEAR: {
		int mindb = tlv[2];
		int maxdb = tlv[3];
		if (volume <= rangemin || rangemax <= rangemin)
			*db_gain = mindb;
		else if (volume >= rangemax)
			*db_gain = maxdb;
		else {
			double val = (double)(volume - rangemin) /
				(double)(rangemax - rangemin);
			if (mindb <= SND_CTL_TLV_DB_GAIN_MUTE)
				*db_gain = (long)(100.0 * 20.0 * log10(val)) +
					maxdb;
			else {
				double lmin = pow(10.0, mindb/2000.0);
				double lmax = pow(10.0, maxdb/2000.0);
				val = (lmax - lmin) * val + lmin;
				*db_gain = (long)(100.0 * 20.0 * log10(val));
			}
		}
		return 0;
	}
#endif
	}
	return -EINVAL;
}

/**
 * \brief Convert from dB gain to the corresponding raw value
 * \param tlv the TLV source returned by #snd_tlv_parse_dB_info()
 * \param rangemin the minimum value of the raw volume
 * \param rangemax the maximum value of the raw volume
 * \param db_gain the dB gain to convert (in 0.01dB unit)
 * \param value the pointer to store the converted raw volume value
 * \param xdir the direction for round-up. The value is round up
 *        when this is positive.
 * \return 0 if successful, or a negative error code
 */
int snd_tlv_convert_from_dB(unsigned int *tlv, long rangemin, long rangemax,
			    long db_gain, long *value, int xdir)
{
	switch (tlv[0]) {
	case SND_CTL_TLVT_DB_RANGE: {
		unsigned int pos, len;
		len = int_index(tlv[1]);
		if (len > MAX_TLV_RANGE_SIZE)
			return -EINVAL;
		pos = 2;
		while (pos + 4 <= len) {
			long dbmin, dbmax;
			rangemin = (int)tlv[pos];
			rangemax = (int)tlv[pos + 1];
			if (!snd_tlv_get_dB_range(tlv + pos + 2,
						  rangemin, rangemax,
						  &dbmin, &dbmax) &&
			    db_gain >= dbmin && db_gain <= dbmax)
				return snd_tlv_convert_from_dB(tlv + pos + 2,
							       rangemin, rangemax,
							       db_gain, value, xdir);
			pos += int_index(tlv[pos + 3]) + 4;
		}
		return -EINVAL;
	}
	case SND_CTL_TLVT_DB_SCALE: {
		int min, step, max;
		min = tlv[2];
		step = (tlv[3] & 0xffff);
		max = min + (int)(step * (rangemax - rangemin));
		if (db_gain <= min)
			*value = rangemin;
		else if (db_gain >= max)
			*value = rangemax;
		else {
			long v = (db_gain - min) * (rangemax - rangemin);
			if (xdir > 0)
				v += (max - min) - 1;
			v = v / (max - min) + rangemin;
			*value = v;
		}
		return 0;
	}
	case SND_CTL_TLVT_DB_MINMAX:
	case SND_CTL_TLVT_DB_MINMAX_MUTE: {
		int min, max;
		min = tlv[2];
		max = tlv[3];
		if (db_gain <= min)
			*value = rangemin;
		else if (db_gain >= max)
			*value = rangemax;
		else {
			long v = (db_gain - min) * (rangemax - rangemin);
			if (xdir > 0)
				v += (max - min) - 1;
			v = v / (max - min) + rangemin;
			*value = v;
		}
		return 0;
	}
#ifndef HAVE_SOFT_FLOAT
	case SND_CTL_TLVT_DB_LINEAR: {
		int min, max;
		min = tlv[2];
		max = tlv[3];
		if (db_gain <= min)
			*value = rangemin;
		else if (db_gain >= max)
			*value = rangemax;
		else {
			double vmin, vmax, v;
			vmin = (min <= SND_CTL_TLV_DB_GAIN_MUTE) ? 0.0 :
				pow(10.0,  (double)min / 2000.0);
			vmax = !max ? 1.0 : pow(10.0,  (double)max / 2000.0);
			v = pow(10.0, (double)db_gain / 2000.0);
			v = (v - vmin) * (rangemax - rangemin) / (vmax - vmin);
			if (xdir > 0)
				v = ceil(v);
			*value = (long)v + rangemin;
		}
		return 0;
	}
#endif
	default:
		break;
	}
	return -EINVAL;
}

#ifndef DOC_HIDDEN
#define TEMP_TLV_SIZE		4096
struct tlv_info {
	long minval, maxval;
	unsigned int *tlv;
	unsigned int buf[TEMP_TLV_SIZE];
};
#endif

static int get_tlv_info(snd_ctl_t *ctl, const snd_ctl_elem_id_t *id,
			struct tlv_info *rec)
{
	snd_ctl_elem_info_t *info;
	int err;

	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_info_set_id(info, id);
	err = snd_ctl_elem_info(ctl, info);
	if (err < 0)
		return err;
	if (!snd_ctl_elem_info_is_tlv_readable(info))
		return -EINVAL;
	if (snd_ctl_elem_info_get_type(info) != SND_CTL_ELEM_TYPE_INTEGER)
		return -EINVAL;
	rec->minval = snd_ctl_elem_info_get_min(info);
	rec->maxval = snd_ctl_elem_info_get_max(info);
	err = snd_ctl_elem_tlv_read(ctl, id, rec->buf, sizeof(rec->buf));
	if (err < 0)
		return err;
	err = snd_tlv_parse_dB_info(rec->buf, sizeof(rec->buf), &rec->tlv);
	if (err < 0)
		return err;
	return 0;
}

/**
 * \brief Get the dB min/max values on the given control element
 * \param ctl the control handler
 * \param id the element id
 * \param min the pointer to store the minimum dB value (in 0.01dB unit)
 * \param max the pointer to store the maximum dB value (in 0.01dB unit)
 * \return 0 if successful, or a negative error code
 */
int snd_ctl_get_dB_range(snd_ctl_t *ctl, const snd_ctl_elem_id_t *id,
			 long *min, long *max)
{
	struct tlv_info info;
	int err;

	err = get_tlv_info(ctl, id, &info);
	if (err < 0)
		return err;
	return snd_tlv_get_dB_range(info.tlv, info.minval, info.maxval,
				    min, max);
}
	
/**
 * \brief Convert the volume value to dB on the given control element
 * \param ctl the control handler
 * \param id the element id
 * \param volume the raw volume value to convert
 * \param db_gain the dB gain (in 0.01dB unit)
 * \return 0 if successful, or a negative error code
 */
int snd_ctl_convert_to_dB(snd_ctl_t *ctl, const snd_ctl_elem_id_t *id,
			  long volume, long *db_gain)
{
	struct tlv_info info;
	int err;

	err = get_tlv_info(ctl, id, &info);
	if (err < 0)
		return err;
	return snd_tlv_convert_to_dB(info.tlv, info.minval, info.maxval,
				     volume, db_gain);
}

/**
 * \brief Convert from dB gain to the raw volume value on the given control element
 * \param ctl the control handler
 * \param id the element id
 * \param db_gain the dB gain to convert (in 0.01dB unit)
 * \param value the pointer to store the converted raw volume value
 * \param xdir the direction for round-up. The value is round up
 *        when this is positive.
 * \return 0 if successful, or a negative error code
 */
int snd_ctl_convert_from_dB(snd_ctl_t *ctl, const snd_ctl_elem_id_t *id,
			    long db_gain, long *value, int xdir)
{
	struct tlv_info info;
	int err;

	err = get_tlv_info(ctl, id, &info);
	if (err < 0)
		return err;
	return snd_tlv_convert_from_dB(info.tlv, info.minval, info.maxval,
				       db_gain, value, xdir);
}
