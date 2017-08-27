/*
 * \file pcm/pcm_plug.c
 * \ingroup PCM_Plugins
 * \brief PCM Route & Volume Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000-2001
 */
/*
 *  PCM - Plug
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
#include "pcm_plugin.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_plug = "";
#endif

#ifndef DOC_HIDDEN

enum snd_pcm_plug_route_policy {
	PLUG_ROUTE_POLICY_NONE,
	PLUG_ROUTE_POLICY_DEFAULT,
	PLUG_ROUTE_POLICY_COPY,
	PLUG_ROUTE_POLICY_AVERAGE,
	PLUG_ROUTE_POLICY_DUP,
};

typedef struct {
	snd_pcm_generic_t gen;
	snd_pcm_t *req_slave;
	snd_pcm_format_t sformat;
	int schannels;
	int srate;
	const snd_config_t *rate_converter;
	enum snd_pcm_plug_route_policy route_policy;
	snd_pcm_route_ttable_entry_t *ttable;
	int ttable_ok, ttable_last;
	unsigned int tt_ssize, tt_cused, tt_sused;
} snd_pcm_plug_t;

#endif

static int snd_pcm_plug_close(snd_pcm_t *pcm)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	int err, result = 0;
	free(plug->ttable);
	assert(plug->gen.slave == plug->req_slave);
	if (plug->gen.close_slave) {
		snd_pcm_unlink_hw_ptr(pcm, plug->req_slave);
		snd_pcm_unlink_appl_ptr(pcm, plug->req_slave);
		err = snd_pcm_close(plug->req_slave);
		if (err < 0)
			result = err;
	}
	free(plug);
	return result;
}

static int snd_pcm_plug_info(snd_pcm_t *pcm, snd_pcm_info_t *info)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	snd_pcm_t *slave = plug->req_slave;
	int err;
	
	if ((err = snd_pcm_info(slave, info)) < 0)
		return err;
	return 0;
}

static const snd_pcm_format_t linear_preferred_formats[] = {
#ifdef SND_LITTLE_ENDIAN
	SND_PCM_FORMAT_S16_LE,
	SND_PCM_FORMAT_U16_LE,
	SND_PCM_FORMAT_S16_BE,
	SND_PCM_FORMAT_U16_BE,
#else
	SND_PCM_FORMAT_S16_BE,
	SND_PCM_FORMAT_U16_BE,
	SND_PCM_FORMAT_S16_LE,
	SND_PCM_FORMAT_U16_LE,
#endif
#ifdef SND_LITTLE_ENDIAN
	SND_PCM_FORMAT_S32_LE,
	SND_PCM_FORMAT_U32_LE,
	SND_PCM_FORMAT_S32_BE,
	SND_PCM_FORMAT_U32_BE,
#else
	SND_PCM_FORMAT_S32_BE,
	SND_PCM_FORMAT_U32_BE,
	SND_PCM_FORMAT_S32_LE,
	SND_PCM_FORMAT_U32_LE,
#endif
	SND_PCM_FORMAT_S8,
	SND_PCM_FORMAT_U8,
#ifdef SND_LITTLE_ENDIAN
	SND_PCM_FORMAT_FLOAT_LE,
	SND_PCM_FORMAT_FLOAT64_LE,
	SND_PCM_FORMAT_FLOAT_BE,
	SND_PCM_FORMAT_FLOAT64_BE,
#else
	SND_PCM_FORMAT_FLOAT_BE,
	SND_PCM_FORMAT_FLOAT64_BE,
	SND_PCM_FORMAT_FLOAT_LE,
	SND_PCM_FORMAT_FLOAT64_LE,
#endif
#ifdef SND_LITTLE_ENDIAN
	SND_PCM_FORMAT_S24_LE,
	SND_PCM_FORMAT_U24_LE,
	SND_PCM_FORMAT_S24_BE,
	SND_PCM_FORMAT_U24_BE,
#else
	SND_PCM_FORMAT_S24_BE,
	SND_PCM_FORMAT_U24_BE,
	SND_PCM_FORMAT_S24_LE,
	SND_PCM_FORMAT_U24_LE,
#endif
#ifdef SND_LITTLE_ENDIAN
	SND_PCM_FORMAT_S24_3LE,
	SND_PCM_FORMAT_U24_3LE,
	SND_PCM_FORMAT_S24_3BE,
	SND_PCM_FORMAT_U24_3BE,
#else
	SND_PCM_FORMAT_S24_3BE,
	SND_PCM_FORMAT_U24_3BE,
	SND_PCM_FORMAT_S24_3LE,
	SND_PCM_FORMAT_U24_3LE,
#endif
#ifdef SND_LITTLE_ENDIAN
	SND_PCM_FORMAT_S20_3LE,
	SND_PCM_FORMAT_U20_3LE,
	SND_PCM_FORMAT_S20_3BE,
	SND_PCM_FORMAT_U20_3BE,
#else
	SND_PCM_FORMAT_S20_3BE,
	SND_PCM_FORMAT_U20_3BE,
	SND_PCM_FORMAT_S20_3LE,
	SND_PCM_FORMAT_U20_3LE,
#endif
#ifdef SND_LITTLE_ENDIAN
	SND_PCM_FORMAT_S18_3LE,
	SND_PCM_FORMAT_U18_3LE,
	SND_PCM_FORMAT_S18_3BE,
	SND_PCM_FORMAT_U18_3BE,
#else
	SND_PCM_FORMAT_S18_3BE,
	SND_PCM_FORMAT_U18_3BE,
	SND_PCM_FORMAT_S18_3LE,
	SND_PCM_FORMAT_U18_3LE,
#endif
};

#if defined(BUILD_PCM_PLUGIN_MULAW) || defined(BUILD_PCM_PLUGIN_ALAW) || \
	defined(BUILD_PCM_PLUGIN_ADPCM)
#define BUILD_PCM_NONLINEAR
#endif

#ifdef BUILD_PCM_NONLINEAR
static const snd_pcm_format_t nonlinear_preferred_formats[] = {
#ifdef BUILD_PCM_PLUGIN_MULAW
	SND_PCM_FORMAT_MU_LAW,
#endif
#ifdef BUILD_PCM_PLUGIN_ALAW
	SND_PCM_FORMAT_A_LAW,
#endif
#ifdef BUILD_PCM_PLUGIN_ADPCM
	SND_PCM_FORMAT_IMA_ADPCM,
#endif
};
#endif

#ifdef BUILD_PCM_PLUGIN_LFLOAT
static const snd_pcm_format_t float_preferred_formats[] = {
#ifdef SND_LITTLE_ENDIAN
	SND_PCM_FORMAT_FLOAT_LE,
	SND_PCM_FORMAT_FLOAT64_LE,
	SND_PCM_FORMAT_FLOAT_BE,
	SND_PCM_FORMAT_FLOAT64_BE,
#else
	SND_PCM_FORMAT_FLOAT_BE,
	SND_PCM_FORMAT_FLOAT64_BE,
	SND_PCM_FORMAT_FLOAT_LE,
	SND_PCM_FORMAT_FLOAT64_LE,
#endif
};
#endif

static const char linear_format_widths[32] = {
	0, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 1,
	0, 1, 0, 1, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 1,
};

static int check_linear_format(const snd_pcm_format_mask_t *format_mask, int wid, int sgn, int ed)
{
	int e, s;
	if (! linear_format_widths[wid - 1])
		return SND_PCM_FORMAT_UNKNOWN;
	for (e = 0; e < 2; e++) {
		for (s = 0; s < 2; s++) {
			int pw = ((wid + 7) / 8) * 8;
			for (; pw <= 32; pw += 8) {
				snd_pcm_format_t f;
				f = snd_pcm_build_linear_format(wid, pw, sgn, ed);
				if (f != SND_PCM_FORMAT_UNKNOWN &&
				    snd_pcm_format_mask_test(format_mask, f))
					return f;
			}
			sgn = !sgn;
		}
		ed = !ed;
	}
	return SND_PCM_FORMAT_UNKNOWN;
}

static snd_pcm_format_t snd_pcm_plug_slave_format(snd_pcm_format_t format, const snd_pcm_format_mask_t *format_mask)
{
	int w, w1, u, e;
	snd_pcm_format_t f;
	snd_pcm_format_mask_t lin = { SND_PCM_FMTBIT_LINEAR };
	snd_pcm_format_mask_t fl = {
#ifdef BUILD_PCM_PLUGIN_LFLOAT
		SND_PCM_FMTBIT_FLOAT
#else
		{ 0 }
#endif
	};
	if (snd_pcm_format_mask_test(format_mask, format))
		return format;
	if (!snd_pcm_format_mask_test(&lin, format) &&
	    !snd_pcm_format_mask_test(&fl, format)) {
		unsigned int i;
		switch (format) {
#ifdef BUILD_PCM_PLUGIN_MULAW
		case SND_PCM_FORMAT_MU_LAW:
#endif
#ifdef BUILD_PCM_PLUGIN_ALAW
		case SND_PCM_FORMAT_A_LAW:
#endif
#ifdef BUILD_PCM_PLUGIN_ADPCM
		case SND_PCM_FORMAT_IMA_ADPCM:
#endif
			for (i = 0; i < sizeof(linear_preferred_formats) / sizeof(linear_preferred_formats[0]); ++i) {
				snd_pcm_format_t f = linear_preferred_formats[i];
				if (snd_pcm_format_mask_test(format_mask, f))
					return f;
			}
			/* Fall through */
		default:
			return SND_PCM_FORMAT_UNKNOWN;
		}

	}
	snd_mask_intersect(&lin, format_mask);
	snd_mask_intersect(&fl, format_mask);
	if (snd_mask_empty(&lin) && snd_mask_empty(&fl)) {
#ifdef BUILD_PCM_NONLINEAR
		unsigned int i;
		for (i = 0; i < sizeof(nonlinear_preferred_formats) / sizeof(nonlinear_preferred_formats[0]); ++i) {
			snd_pcm_format_t f = nonlinear_preferred_formats[i];
			if (snd_pcm_format_mask_test(format_mask, f))
				return f;
		}
#endif
		return SND_PCM_FORMAT_UNKNOWN;
	}
#ifdef BUILD_PCM_PLUGIN_LFLOAT
	if (snd_pcm_format_float(format)) {
		if (snd_pcm_format_mask_test(&fl, format)) {
			unsigned int i;
			for (i = 0; i < sizeof(float_preferred_formats) / sizeof(float_preferred_formats[0]); ++i) {
				snd_pcm_format_t f = float_preferred_formats[i];
				if (snd_pcm_format_mask_test(format_mask, f))
					return f;
			}
		}
		w = 32;
		u = 0;
		e = snd_pcm_format_big_endian(format);
	} else
#endif
	if (snd_mask_empty(&lin)) {
#ifdef BUILD_PCM_PLUGIN_LFLOAT
		unsigned int i;
		for (i = 0; i < sizeof(float_preferred_formats) / sizeof(float_preferred_formats[0]); ++i) {
			snd_pcm_format_t f = float_preferred_formats[i];
			if (snd_pcm_format_mask_test(format_mask, f))
				return f;
		}
#endif
		return SND_PCM_FORMAT_UNKNOWN;
	} else {
		w = snd_pcm_format_width(format);
		u = snd_pcm_format_unsigned(format);
		e = snd_pcm_format_big_endian(format);
	}
	for (w1 = w; w1 <= 32; w1++) {
		f = check_linear_format(format_mask, w1, u, e);
		if (f != SND_PCM_FORMAT_UNKNOWN)
			return f;
	}
	for (w1 = w - 1; w1 > 0; w1--) {
		f = check_linear_format(format_mask, w1, u, e);
		if (f != SND_PCM_FORMAT_UNKNOWN)
			return f;
	}
	return SND_PCM_FORMAT_UNKNOWN;
}

static void snd_pcm_plug_clear(snd_pcm_t *pcm)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	snd_pcm_t *slave = plug->req_slave;
	/* Clear old plugins */
	if (plug->gen.slave != slave) {
		snd_pcm_unlink_hw_ptr(pcm, plug->gen.slave);
		snd_pcm_unlink_appl_ptr(pcm, plug->gen.slave);
		snd_pcm_close(plug->gen.slave);
		plug->gen.slave = slave;
		pcm->fast_ops = slave->fast_ops;
		pcm->fast_op_arg = slave->fast_op_arg;
	}
}

#ifndef DOC_HIDDEN
typedef struct {
	snd_pcm_access_t access;
	snd_pcm_format_t format;
	unsigned int channels;
	unsigned int rate;
} snd_pcm_plug_params_t;
#endif

#ifdef BUILD_PCM_PLUGIN_RATE
static int snd_pcm_plug_change_rate(snd_pcm_t *pcm, snd_pcm_t **new, snd_pcm_plug_params_t *clt, snd_pcm_plug_params_t *slv)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	int err;
	if (clt->rate == slv->rate)
		return 0;
	assert(snd_pcm_format_linear(slv->format));
	err = snd_pcm_rate_open(new, NULL, slv->format, slv->rate, plug->rate_converter,
				plug->gen.slave, plug->gen.slave != plug->req_slave);
	if (err < 0)
		return err;
	slv->access = clt->access;
	slv->rate = clt->rate;
	if (snd_pcm_format_linear(clt->format))
		slv->format = clt->format;
	return 1;
}
#endif

#ifdef BUILD_PCM_PLUGIN_ROUTE
static int snd_pcm_plug_change_channels(snd_pcm_t *pcm, snd_pcm_t **new, snd_pcm_plug_params_t *clt, snd_pcm_plug_params_t *slv)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	unsigned int tt_ssize, tt_cused, tt_sused;
	snd_pcm_route_ttable_entry_t *ttable;
	int err;
	if (clt->channels == slv->channels &&
	    (!plug->ttable || !plug->ttable_last))
		return 0;
	if (clt->rate != slv->rate &&
	    clt->channels > slv->channels)
		return 0;
	assert(snd_pcm_format_linear(slv->format));
	tt_ssize = slv->channels;
	tt_cused = clt->channels;
	tt_sused = slv->channels;
	ttable = alloca(tt_cused * tt_sused * sizeof(*ttable));
	if (plug->ttable) {	/* expand or shrink table */
		unsigned int c = 0, s = 0;
		for (c = 0; c < tt_cused; c++) {
			for (s = 0; s < tt_sused; s++) {
				snd_pcm_route_ttable_entry_t v;
				if (c >= plug->tt_cused)
					v = 0;
				else if (s >= plug->tt_sused)
					v = 0;
				else
					v = plug->ttable[c * plug->tt_ssize + s];
				ttable[c * tt_ssize + s] = v;
			}
		}
		plug->ttable_ok = 1;
	} else {
		unsigned int k;
		unsigned int c = 0, s = 0;
		enum snd_pcm_plug_route_policy rpolicy = plug->route_policy;
		int n;
		for (k = 0; k < tt_cused * tt_sused; ++k)
			ttable[k] = 0;
		if (rpolicy == PLUG_ROUTE_POLICY_DEFAULT) {
			rpolicy = PLUG_ROUTE_POLICY_COPY;
			/* it's hack for mono conversion */
			if (clt->channels == 1 || slv->channels == 1)
				rpolicy = PLUG_ROUTE_POLICY_AVERAGE;
		}
		switch (rpolicy) {
		case PLUG_ROUTE_POLICY_AVERAGE:
		case PLUG_ROUTE_POLICY_DUP:
			if (clt->channels > slv->channels) {
				n = clt->channels;
			} else {
				n = slv->channels;
			}
			while (n-- > 0) {
				snd_pcm_route_ttable_entry_t v = SND_PCM_PLUGIN_ROUTE_FULL;
				if (rpolicy == PLUG_ROUTE_POLICY_AVERAGE) {
					if (pcm->stream == SND_PCM_STREAM_PLAYBACK &&
					    clt->channels > slv->channels) {
						int srcs = clt->channels / slv->channels;
						if (s < clt->channels % slv->channels)
							srcs++;
						v /= srcs;
					} else if (pcm->stream == SND_PCM_STREAM_CAPTURE &&
						   slv->channels > clt->channels) {
							int srcs = slv->channels / clt->channels;
						if (s < slv->channels % clt->channels)
							srcs++;
						v /= srcs;
					}
				}
				ttable[c * tt_ssize + s] = v;
				if (++c == clt->channels)
					c = 0;
				if (++s == slv->channels)
					s = 0;
			}
			break;
		case PLUG_ROUTE_POLICY_COPY:
			if (clt->channels < slv->channels) {
				n = clt->channels;
			} else {
				n = slv->channels;
			}
			for (c = 0; (int)c < n; c++)
				ttable[c * tt_ssize + c] = SND_PCM_PLUGIN_ROUTE_FULL;
			break;
		default:
			SNDERR("Invalid route policy");
			break;
		}
	}
	err = snd_pcm_route_open(new, NULL, slv->format, (int) slv->channels, ttable, tt_ssize, tt_cused, tt_sused, plug->gen.slave, plug->gen.slave != plug->req_slave);
	if (err < 0)
		return err;
	slv->channels = clt->channels;
	slv->access = clt->access;
	if (snd_pcm_format_linear(clt->format))
		slv->format = clt->format;
	return 1;
}
#endif

static int snd_pcm_plug_change_format(snd_pcm_t *pcm, snd_pcm_t **new, snd_pcm_plug_params_t *clt, snd_pcm_plug_params_t *slv)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	int err;
	snd_pcm_format_t cfmt;
	int (*f)(snd_pcm_t **_pcm, const char *name, snd_pcm_format_t sformat, snd_pcm_t *slave, int close_slave);

	/* No conversion is needed */
	if (clt->format == slv->format &&
	    clt->rate == slv->rate &&
	    clt->channels == clt->channels)
		return 0;

	if (snd_pcm_format_linear(slv->format)) {
		/* Conversion is done in another plugin */
		if (clt->rate != slv->rate ||
		    clt->channels != slv->channels)
			return 0;
		cfmt = clt->format;
		switch (clt->format) {
#ifdef BUILD_PCM_PLUGIN_MULAW
		case SND_PCM_FORMAT_MU_LAW:
			f = snd_pcm_mulaw_open;
			break;
#endif
#ifdef BUILD_PCM_PLUGIN_ALAW
		case SND_PCM_FORMAT_A_LAW:
			f = snd_pcm_alaw_open;
			break;
#endif
#ifdef BUILD_PCM_PLUGIN_ADPCM
		case SND_PCM_FORMAT_IMA_ADPCM:
			f = snd_pcm_adpcm_open;
			break;
#endif
		default:
#ifdef BUILD_PCM_PLUGIN_LFLOAT
			if (snd_pcm_format_float(clt->format))
				f = snd_pcm_lfloat_open;

			else
#endif
				f = snd_pcm_linear_open;
			break;
		}
#ifdef BUILD_PCM_PLUGIN_LFLOAT
	} else if (snd_pcm_format_float(slv->format)) {
		/* Conversion is done in another plugin */
		if (clt->format == slv->format &&
		    clt->rate == slv->rate &&
		    clt->channels == slv->channels)
			return 0;
		cfmt = clt->format;
		if (snd_pcm_format_linear(clt->format))
			f = snd_pcm_lfloat_open;
		else
			return -EINVAL;
#endif
#ifdef BUILD_PCM_NONLINEAR
	} else {
		switch (slv->format) {
#ifdef BUILD_PCM_PLUGIN_MULAW
		case SND_PCM_FORMAT_MU_LAW:
			f = snd_pcm_mulaw_open;
			break;
#endif
#ifdef BUILD_PCM_PLUGIN_ALAW
		case SND_PCM_FORMAT_A_LAW:
			f = snd_pcm_alaw_open;
			break;
#endif
#ifdef BUILD_PCM_PLUGIN_ADPCM
		case SND_PCM_FORMAT_IMA_ADPCM:
			f = snd_pcm_adpcm_open;
			break;
#endif
		default:
			return -EINVAL;
		}
		if (snd_pcm_format_linear(clt->format))
			cfmt = clt->format;
		else
			cfmt = SND_PCM_FORMAT_S16;
#endif /* NONLINEAR */
	}
	err = f(new, NULL, slv->format, plug->gen.slave, plug->gen.slave != plug->req_slave);
	if (err < 0)
		return err;
	slv->format = cfmt;
	slv->access = clt->access;
	return 1;
}

static int snd_pcm_plug_change_access(snd_pcm_t *pcm, snd_pcm_t **new, snd_pcm_plug_params_t *clt, snd_pcm_plug_params_t *slv)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	int err;
	if (clt->access == slv->access)
		return 0;
	err = snd_pcm_copy_open(new, NULL, plug->gen.slave, plug->gen.slave != plug->req_slave);
	if (err < 0)
		return err;
	slv->access = clt->access;
	return 1;
}

#ifdef BUILD_PCM_PLUGIN_MMAP_EMUL
static int snd_pcm_plug_change_mmap(snd_pcm_t *pcm, snd_pcm_t **new,
				    snd_pcm_plug_params_t *clt,
				    snd_pcm_plug_params_t *slv)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	int err;

	if (clt->access == slv->access)
		return 0;

	switch (slv->access) {
	case SND_PCM_ACCESS_MMAP_INTERLEAVED:
	case SND_PCM_ACCESS_MMAP_NONINTERLEAVED:
	case SND_PCM_ACCESS_MMAP_COMPLEX:
		return 0;
	default:
		break;
	}

	err = __snd_pcm_mmap_emul_open(new, NULL, plug->gen.slave,
				       plug->gen.slave != plug->req_slave);
	if (err < 0)
		return err;
	switch (slv->access) {
	case SND_PCM_ACCESS_RW_INTERLEAVED:
		slv->access = SND_PCM_ACCESS_MMAP_INTERLEAVED;
		break;
	case SND_PCM_ACCESS_RW_NONINTERLEAVED:
		slv->access = SND_PCM_ACCESS_MMAP_NONINTERLEAVED;
		break;
	default:
		break;
	}
	return 1;
}
#endif

static int snd_pcm_plug_insert_plugins(snd_pcm_t *pcm,
				       snd_pcm_plug_params_t *client,
				       snd_pcm_plug_params_t *slave)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	static int (*const funcs[])(snd_pcm_t *_pcm, snd_pcm_t **new, snd_pcm_plug_params_t *s, snd_pcm_plug_params_t *d) = {
#ifdef BUILD_PCM_PLUGIN_MMAP_EMUL
		snd_pcm_plug_change_mmap,
#endif
		snd_pcm_plug_change_format,
#ifdef BUILD_PCM_PLUGIN_ROUTE
		snd_pcm_plug_change_channels,
#endif
#ifdef BUILD_PCM_PLUGIN_RATE
		snd_pcm_plug_change_rate,
#endif
#ifdef BUILD_PCM_PLUGIN_ROUTE
		snd_pcm_plug_change_channels,
#endif
		snd_pcm_plug_change_format,
		snd_pcm_plug_change_access
	};
	snd_pcm_plug_params_t p = *slave;
	unsigned int k = 0;
	plug->ttable_ok = plug->ttable_last = 0;
	while (client->format != p.format ||
	       client->channels != p.channels ||
	       client->rate != p.rate ||
	       client->access != p.access) {
		snd_pcm_t *new;
		int err;
		if (k >= sizeof(funcs)/sizeof(*funcs))
			return -EINVAL;
		err = funcs[k](pcm, &new, client, &p);
		if (err < 0) {
			snd_pcm_plug_clear(pcm);
			return err;
		}
		if (err) {
			plug->gen.slave = new;
			pcm->fast_ops = new->fast_ops;
			pcm->fast_op_arg = new->fast_op_arg;
		}
		k++;
	}
#ifdef BUILD_PCM_PLUGIN_ROUTE
	/* it's exception, user specified ttable, but no reduction/expand */
	if (plug->ttable && !plug->ttable_ok) {
		snd_pcm_t *new;
		int err;
		plug->ttable_last = 1;
		err = snd_pcm_plug_change_channels(pcm, &new, client, &p);
		if (err < 0) {
			snd_pcm_plug_clear(pcm);
			return err;
		}
		assert(err);
		assert(plug->ttable_ok);
		plug->gen.slave = new;
		pcm->fast_ops = new->fast_ops;
		pcm->fast_op_arg = new->fast_op_arg;
	}
#endif
	return 0;
}

static int snd_pcm_plug_hw_refine_cprepare(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params)
{
	unsigned int rate_min, channels_max;
	int err;

	/* HACK: to avoid overflow in PARTBIT_RATE code */
	err = snd_pcm_hw_param_get_min(params, SND_PCM_HW_PARAM_RATE, &rate_min, NULL);
	if (err < 0)
		return err;
	if (rate_min < 4000) {
		_snd_pcm_hw_param_set_min(params, SND_PCM_HW_PARAM_RATE, 4000, 0);
		if (snd_pcm_hw_param_empty(params, SND_PCM_HW_PARAM_RATE))
			return -EINVAL;
	}
	/* HACK: to avoid overflow in PERIOD_SIZE code */
	err = snd_pcm_hw_param_get_max(params, SND_PCM_HW_PARAM_CHANNELS, &channels_max, NULL);
	if (err < 0)
		return err;
	if (channels_max > 10000) {
		_snd_pcm_hw_param_set_max(params, SND_PCM_HW_PARAM_CHANNELS, 10000, 0);
		if (snd_pcm_hw_param_empty(params, SND_PCM_HW_PARAM_CHANNELS))
			return -EINVAL;
	}
	return 0;
}

static int snd_pcm_plug_hw_refine_sprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *sparams)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	int err;
	
	_snd_pcm_hw_params_any(sparams);
	if (plug->sformat >= 0) {
		_snd_pcm_hw_params_set_format(sparams, plug->sformat);
		_snd_pcm_hw_params_set_subformat(sparams, SND_PCM_SUBFORMAT_STD);
	}
	if (plug->schannels > 0)
		_snd_pcm_hw_param_set(sparams, SND_PCM_HW_PARAM_CHANNELS,
				      plug->schannels, 0);
	if (plug->srate > 0)
		_snd_pcm_hw_param_set_minmax(sparams, SND_PCM_HW_PARAM_RATE,
					      plug->srate, 0, plug->srate + 1, -1);
	/* reduce the available configurations */
	err = snd_pcm_hw_refine(plug->req_slave, sparams);
	if (err < 0)
		return err;
	return 0;
}

static int check_access_change(snd_pcm_hw_params_t *cparams,
			       snd_pcm_hw_params_t *sparams)
{
	snd_pcm_access_mask_t *smask;
#ifdef BUILD_PCM_PLUGIN_MMAP_EMUL
	const snd_pcm_access_mask_t *cmask;
	snd_pcm_access_mask_t mask;
#endif

	smask = (snd_pcm_access_mask_t *)
		snd_pcm_hw_param_get_mask(sparams,
					  SND_PCM_HW_PARAM_ACCESS);
	if (snd_pcm_access_mask_test(smask, SND_PCM_ACCESS_MMAP_INTERLEAVED) ||
	    snd_pcm_access_mask_test(smask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED) ||
	    snd_pcm_access_mask_test(smask, SND_PCM_ACCESS_MMAP_COMPLEX))
		return 0; /* OK, we have mmap support */
#ifdef BUILD_PCM_PLUGIN_MMAP_EMUL
	/* no mmap support - we need mmap emulation */

	if (!snd_pcm_access_mask_test(smask, SND_PCM_ACCESS_RW_INTERLEAVED) &&
	    !snd_pcm_access_mask_test(smask, SND_PCM_ACCESS_RW_NONINTERLEAVED)) 
		return -EINVAL; /* even no RW access?  no way! */

	cmask = (const snd_pcm_access_mask_t *)
		snd_pcm_hw_param_get_mask(cparams,
					  SND_PCM_HW_PARAM_ACCESS);
	snd_mask_none(&mask);
	if (snd_pcm_access_mask_test(cmask, SND_PCM_ACCESS_RW_INTERLEAVED) ||
	    snd_pcm_access_mask_test(cmask, SND_PCM_ACCESS_MMAP_INTERLEAVED)) {
		if (snd_pcm_access_mask_test(smask, SND_PCM_ACCESS_RW_INTERLEAVED))
			snd_pcm_access_mask_set(&mask,
						SND_PCM_ACCESS_RW_INTERLEAVED);
	}
	if (snd_pcm_access_mask_test(cmask, SND_PCM_ACCESS_RW_NONINTERLEAVED) ||
	    snd_pcm_access_mask_test(cmask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED)) {
		if (snd_pcm_access_mask_test(smask, SND_PCM_ACCESS_RW_NONINTERLEAVED))
			snd_pcm_access_mask_set(&mask,
						SND_PCM_ACCESS_RW_NONINTERLEAVED);
	}
	if (!snd_mask_empty(&mask))
		*smask = mask; /* prefer the straight conversion */
	return 0;
#else
	return -EINVAL;
#endif
}

static int snd_pcm_plug_hw_refine_schange(snd_pcm_t *pcm, snd_pcm_hw_params_t *params,
					  snd_pcm_hw_params_t *sparams)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	snd_pcm_t *slave = plug->req_slave;
	unsigned int links = (SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);
	const snd_pcm_format_mask_t *format_mask, *sformat_mask;
	snd_pcm_format_mask_t sfmt_mask;
	int err;
	snd_pcm_format_t format;
	snd_interval_t t, buffer_size;
	const snd_interval_t *srate, *crate;

	if (plug->srate == -2 ||
	    (pcm->mode & SND_PCM_NO_AUTO_RESAMPLE) ||
	    (params->flags & SND_PCM_HW_PARAMS_NORESAMPLE))
		links |= SND_PCM_HW_PARBIT_RATE;
	else {
		err = snd_pcm_hw_param_refine_multiple(slave, sparams, SND_PCM_HW_PARAM_RATE, params);
		if (err < 0)
			return err;
	}
	
	if (plug->schannels == -2 || (pcm->mode & SND_PCM_NO_AUTO_CHANNELS))
		links |= SND_PCM_HW_PARBIT_CHANNELS;
	else {
		err = snd_pcm_hw_param_refine_near(slave, sparams, SND_PCM_HW_PARAM_CHANNELS, params);
		if (err < 0)
			return err;
	}
	if (plug->sformat == -2 || (pcm->mode & SND_PCM_NO_AUTO_FORMAT))
		links |= SND_PCM_HW_PARBIT_FORMAT;
	else {
		format_mask = snd_pcm_hw_param_get_mask(params, SND_PCM_HW_PARAM_FORMAT);
		sformat_mask = snd_pcm_hw_param_get_mask(sparams, SND_PCM_HW_PARAM_FORMAT);
		snd_mask_none(&sfmt_mask);
		for (format = 0; format <= SND_PCM_FORMAT_LAST; format++) {
			snd_pcm_format_t f;
			if (!snd_pcm_format_mask_test(format_mask, format))
				continue;
			if (snd_pcm_format_mask_test(sformat_mask, format))
				f = format;
			else {
				f = snd_pcm_plug_slave_format(format, sformat_mask);
				if (f == SND_PCM_FORMAT_UNKNOWN)
					continue;
			}
			snd_pcm_format_mask_set(&sfmt_mask, f);
		}

		if (snd_pcm_format_mask_empty(&sfmt_mask)) {
			SNDERR("Unable to find an usable slave format for '%s'", pcm->name);
			for (format = 0; format <= SND_PCM_FORMAT_LAST; format++) {
				if (!snd_pcm_format_mask_test(format_mask, format))
					continue;
				SNDERR("Format: %s", snd_pcm_format_name(format));
			}
			for (format = 0; format <= SND_PCM_FORMAT_LAST; format++) {
				if (!snd_pcm_format_mask_test(sformat_mask, format))
					continue;
				SNDERR("Slave format: %s", snd_pcm_format_name(format));
			}
			return -EINVAL;
		}
		err = snd_pcm_hw_param_set_mask(slave, sparams, SND_CHANGE,
						SND_PCM_HW_PARAM_FORMAT, &sfmt_mask);
		if (err < 0)
			return -EINVAL;
	}

	if (snd_pcm_hw_param_never_eq(params, SND_PCM_HW_PARAM_ACCESS, sparams)) {
		err = check_access_change(params, sparams);
		if (err < 0) {
			SNDERR("Unable to find an usable access for '%s'",
			       pcm->name);
			return err;
		}
	}

	if ((links & SND_PCM_HW_PARBIT_RATE) ||
	    snd_pcm_hw_param_always_eq(params, SND_PCM_HW_PARAM_RATE, sparams))
		links |= (SND_PCM_HW_PARBIT_PERIOD_SIZE |
			  SND_PCM_HW_PARBIT_BUFFER_SIZE);
	else {
		snd_interval_copy(&buffer_size, snd_pcm_hw_param_get_interval(params, SND_PCM_HW_PARAM_BUFFER_SIZE));
		snd_interval_unfloor(&buffer_size);
		crate = snd_pcm_hw_param_get_interval(params, SND_PCM_HW_PARAM_RATE);
		srate = snd_pcm_hw_param_get_interval(sparams, SND_PCM_HW_PARAM_RATE);
		snd_interval_muldiv(&buffer_size, srate, crate, &t);
		err = _snd_pcm_hw_param_set_interval(sparams, SND_PCM_HW_PARAM_BUFFER_SIZE, &t);
		if (err < 0)
			return err;
	}
	err = _snd_pcm_hw_params_refine(sparams, links, params);
	if (err < 0)
		return err;
	return 0;
}
	
static int snd_pcm_plug_hw_refine_cchange(snd_pcm_t *pcm ATTRIBUTE_UNUSED,
					  snd_pcm_hw_params_t *params,
					  snd_pcm_hw_params_t *sparams)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	unsigned int links = (SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);
	const snd_pcm_format_mask_t *format_mask, *sformat_mask;
	snd_pcm_format_mask_t fmt_mask;
	int err;
	snd_pcm_format_t format;
	snd_interval_t t;
	const snd_interval_t *sbuffer_size;
	const snd_interval_t *srate, *crate;

	if (plug->schannels == -2 || (pcm->mode & SND_PCM_NO_AUTO_CHANNELS))
		links |= SND_PCM_HW_PARBIT_CHANNELS;

	if (plug->sformat == -2 || (pcm->mode & SND_PCM_NO_AUTO_FORMAT))
		links |= SND_PCM_HW_PARBIT_FORMAT;
	else {
		format_mask = snd_pcm_hw_param_get_mask(params,
							SND_PCM_HW_PARAM_FORMAT);
		sformat_mask = snd_pcm_hw_param_get_mask(sparams,
							 SND_PCM_HW_PARAM_FORMAT);
		snd_mask_none(&fmt_mask);
		for (format = 0; format <= SND_PCM_FORMAT_LAST; format++) {
			snd_pcm_format_t f;
			if (!snd_pcm_format_mask_test(format_mask, format))
				continue;
			if (snd_pcm_format_mask_test(sformat_mask, format))
				f = format;
			else {
				f = snd_pcm_plug_slave_format(format, sformat_mask);
				if (f == SND_PCM_FORMAT_UNKNOWN)
					continue;
			}
			snd_pcm_format_mask_set(&fmt_mask, format);
		}

		if (snd_pcm_format_mask_empty(&fmt_mask)) {
			SNDERR("Unable to find an usable client format");
			for (format = 0; format <= SND_PCM_FORMAT_LAST; format++) {
				if (!snd_pcm_format_mask_test(format_mask, format))
					continue;
				SNDERR("Format: %s", snd_pcm_format_name(format));
			}
			for (format = 0; format <= SND_PCM_FORMAT_LAST; format++) {
				if (!snd_pcm_format_mask_test(sformat_mask, format))
					continue;
				SNDERR("Slave format: %s", snd_pcm_format_name(format));
			}
			return -EINVAL;
		}
		
		err = _snd_pcm_hw_param_set_mask(params, 
						 SND_PCM_HW_PARAM_FORMAT, &fmt_mask);
		if (err < 0)
			return err;
	}

	if (plug->srate == -2 ||
	    (pcm->mode & SND_PCM_NO_AUTO_RESAMPLE) ||
	    (params->flags & SND_PCM_HW_PARAMS_NORESAMPLE))
		links |= SND_PCM_HW_PARBIT_RATE;
	else {
		unsigned int rate_min, srate_min;
		int rate_mindir, srate_mindir;
		
		/* This is a temporary hack, waiting for a better solution */
		err = snd_pcm_hw_param_get_min(params, SND_PCM_HW_PARAM_RATE, &rate_min, &rate_mindir);
		if (err < 0)
			return err;
		err = snd_pcm_hw_param_get_min(sparams, SND_PCM_HW_PARAM_RATE, &srate_min, &srate_mindir);
		if (err < 0)
			return err;
		if (rate_min == srate_min && srate_mindir > rate_mindir) {
			err = _snd_pcm_hw_param_set_min(params, SND_PCM_HW_PARAM_RATE, srate_min, srate_mindir);
			if (err < 0)
				return err;
		}
	}
	if ((links & SND_PCM_HW_PARBIT_RATE) ||
	    snd_pcm_hw_param_always_eq(params, SND_PCM_HW_PARAM_RATE, sparams))
		links |= (SND_PCM_HW_PARBIT_PERIOD_SIZE |
			  SND_PCM_HW_PARBIT_BUFFER_SIZE);
	else {
		sbuffer_size = snd_pcm_hw_param_get_interval(sparams, SND_PCM_HW_PARAM_BUFFER_SIZE);
		crate = snd_pcm_hw_param_get_interval(params, SND_PCM_HW_PARAM_RATE);
		srate = snd_pcm_hw_param_get_interval(sparams, SND_PCM_HW_PARAM_RATE);
		snd_interval_muldiv(sbuffer_size, crate, srate, &t);
		snd_interval_floor(&t);
		if (snd_interval_empty(&t))
			return -EINVAL;
		err = _snd_pcm_hw_param_set_interval(params, SND_PCM_HW_PARAM_BUFFER_SIZE, &t);
		if (err < 0)
			return err;
	}
	err = _snd_pcm_hw_params_refine(params, links, sparams);
	if (err < 0)
		return err;
	params->info &= ~(SND_PCM_INFO_MMAP | SND_PCM_INFO_MMAP_VALID);
	return 0;
}

static int snd_pcm_plug_hw_refine_slave(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	return snd_pcm_hw_refine(plug->req_slave, params);
}

static int snd_pcm_plug_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_plug_hw_refine_cprepare,
				       snd_pcm_plug_hw_refine_cchange,
				       snd_pcm_plug_hw_refine_sprepare,
				       snd_pcm_plug_hw_refine_schange,
				       snd_pcm_plug_hw_refine_slave);
}

static int snd_pcm_plug_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	snd_pcm_t *slave = plug->req_slave;
	snd_pcm_plug_params_t clt_params, slv_params;
	snd_pcm_hw_params_t sparams;
	int err;

	err = snd_pcm_plug_hw_refine_sprepare(pcm, &sparams);
	if (err < 0)
		return err;
	err = snd_pcm_plug_hw_refine_schange(pcm, params, &sparams);
	if (err < 0)
		return err;
	err = snd_pcm_hw_refine_soft(slave, &sparams);
	if (err < 0)
		return err;

	INTERNAL(snd_pcm_hw_params_get_access)(params, &clt_params.access);
	INTERNAL(snd_pcm_hw_params_get_format)(params, &clt_params.format);
	INTERNAL(snd_pcm_hw_params_get_channels)(params, &clt_params.channels);
	INTERNAL(snd_pcm_hw_params_get_rate)(params, &clt_params.rate, 0);

	INTERNAL(snd_pcm_hw_params_get_format)(&sparams, &slv_params.format);
	INTERNAL(snd_pcm_hw_params_get_channels)(&sparams, &slv_params.channels);
	INTERNAL(snd_pcm_hw_params_get_rate)(&sparams, &slv_params.rate, 0);
	snd_pcm_plug_clear(pcm);
	if (!(clt_params.format == slv_params.format &&
	      clt_params.channels == slv_params.channels &&
	      clt_params.rate == slv_params.rate &&
	      !plug->ttable &&
	      snd_pcm_hw_params_test_access(slave, &sparams,
					    clt_params.access) >= 0)) {
		INTERNAL(snd_pcm_hw_params_set_access_first)(slave, &sparams, &slv_params.access);
		err = snd_pcm_plug_insert_plugins(pcm, &clt_params, &slv_params);
		if (err < 0)
			return err;
	}
	slave = plug->gen.slave;
	err = _snd_pcm_hw_params(slave, params);
	if (err < 0) {
		snd_pcm_plug_clear(pcm);
		return err;
	}
	snd_pcm_unlink_hw_ptr(pcm, plug->req_slave);
	snd_pcm_unlink_appl_ptr(pcm, plug->req_slave);
	snd_pcm_link_hw_ptr(pcm, slave);
	snd_pcm_link_appl_ptr(pcm, slave);
	return 0;
}

static int snd_pcm_plug_hw_free(snd_pcm_t *pcm)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	snd_pcm_t *slave = plug->gen.slave;
	int err = snd_pcm_hw_free(slave);
	snd_pcm_plug_clear(pcm);
	return err;
}

static void snd_pcm_plug_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_plug_t *plug = pcm->private_data;
	snd_output_printf(out, "Plug PCM: ");
	snd_pcm_dump(plug->gen.slave, out);
}

static const snd_pcm_ops_t snd_pcm_plug_ops = {
	.close = snd_pcm_plug_close,
	.info = snd_pcm_plug_info,
	.hw_refine = snd_pcm_plug_hw_refine,
	.hw_params = snd_pcm_plug_hw_params,
	.hw_free = snd_pcm_plug_hw_free,
	.sw_params = snd_pcm_generic_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_plug_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

/**
 * \brief Creates a new Plug PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param sformat Slave (destination) format
 * \param slave Slave PCM handle
 * \param close_slave When set, the slave PCM handle is closed with copy PCM
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_plug_open(snd_pcm_t **pcmp,
		      const char *name,
		      snd_pcm_format_t sformat, int schannels, int srate,
		      const snd_config_t *rate_converter,
		      enum snd_pcm_plug_route_policy route_policy,
		      snd_pcm_route_ttable_entry_t *ttable,
		      unsigned int tt_ssize,
		      unsigned int tt_cused, unsigned int tt_sused,
		      snd_pcm_t *slave, int close_slave)
{
	snd_pcm_t *pcm;
	snd_pcm_plug_t *plug;
	int err;
	assert(pcmp && slave);

	plug = calloc(1, sizeof(snd_pcm_plug_t));
	if (!plug)
		return -ENOMEM;
	plug->sformat = sformat;
	plug->schannels = schannels;
	plug->srate = srate;
	plug->rate_converter = rate_converter;
	plug->gen.slave = plug->req_slave = slave;
	plug->gen.close_slave = close_slave;
	plug->route_policy = route_policy;
	plug->ttable = ttable;
	plug->tt_ssize = tt_ssize;
	plug->tt_cused = tt_cused;
	plug->tt_sused = tt_sused;
	
	err = snd_pcm_new(&pcm, SND_PCM_TYPE_PLUG, name, slave->stream, slave->mode);
	if (err < 0) {
		free(plug);
		return err;
	}
	pcm->ops = &snd_pcm_plug_ops;
	pcm->fast_ops = slave->fast_ops;
	pcm->fast_op_arg = slave->fast_op_arg;
	pcm->private_data = plug;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	pcm->mmap_shadow = 1;
	pcm->monotonic = slave->monotonic;
	snd_pcm_link_hw_ptr(pcm, slave);
	snd_pcm_link_appl_ptr(pcm, slave);
	*pcmp = pcm;

	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_plug Automatic conversion plugin

This plugin converts channels, rate and format on request.

\code
pcm.name {
        type plug               # Automatic conversion PCM
        slave STR               # Slave name
        # or
        slave {                 # Slave definition
                pcm STR         # Slave PCM name
                # or
                pcm { }         # Slave PCM definition
		[format STR]	# Slave format (default nearest) or "unchanged"
		[channels INT]	# Slave channels (default nearest) or "unchanged"
		[rate INT]	# Slave rate (default nearest) or "unchanged"
        }
	route_policy STR	# route policy for automatic ttable generation
				# STR can be 'default', 'average', 'copy', 'duplicate'
				# average: result is average of input channels
				# copy: only first channels are copied to destination
				# duplicate: duplicate first set of channels
				# default: copy policy, except for mono capture - sum
	ttable {		# Transfer table (bi-dimensional compound of cchannels * schannels numbers)
		CCHANNEL {
			SCHANNEL REAL	# route value (0.0 - 1.0)
		}
	}
	rate_converter STR	# type of rate converter
	# or
	rate_converter [ STR1 STR2 ... ]
				# type of rate converter
				# default value is taken from defaults.pcm.rate_converter
}
\endcode

\subsection pcm_plugins_plug_funcref Function reference

<UL>
  <LI>snd_pcm_plug_open()
  <LI>_snd_pcm_plug_open()
</UL>

*/

/**
 * \brief Creates a new Plug PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with Plug PCM description
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_plug_open(snd_pcm_t **pcmp, const char *name,
		       snd_config_t *root, snd_config_t *conf, 
		       snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	int err;
	snd_pcm_t *spcm;
	snd_config_t *slave = NULL, *sconf;
	snd_config_t *tt = NULL;
	enum snd_pcm_plug_route_policy route_policy = PLUG_ROUTE_POLICY_DEFAULT;
	snd_pcm_route_ttable_entry_t *ttable = NULL;
	unsigned int csize, ssize;
	unsigned int cused, sused;
	snd_pcm_format_t sformat = SND_PCM_FORMAT_UNKNOWN;
	int schannels = -1, srate = -1;
	const snd_config_t *rate_converter = NULL;

	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_pcm_conf_generic_id(id))
			continue;
		if (strcmp(id, "slave") == 0) {
			slave = n;
			continue;
		}
#ifdef BUILD_PCM_PLUGIN_ROUTE
		if (strcmp(id, "ttable") == 0) {
			route_policy = PLUG_ROUTE_POLICY_NONE;
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_COMPOUND) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			tt = n;
			continue;
		}
		if (strcmp(id, "route_policy") == 0) {
			const char *str;
			if ((err = snd_config_get_string(n, &str)) < 0) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			if (tt != NULL)
				SNDERR("Table is defined, route policy is ignored");
			if (!strcmp(str, "default"))
				route_policy = PLUG_ROUTE_POLICY_DEFAULT;
			else if (!strcmp(str, "average"))
				route_policy = PLUG_ROUTE_POLICY_AVERAGE;
			else if (!strcmp(str, "copy"))
				route_policy = PLUG_ROUTE_POLICY_COPY;
			else if (!strcmp(str, "duplicate"))
				route_policy = PLUG_ROUTE_POLICY_DUP;
			continue;
		}
#endif
#ifdef BUILD_PCM_PLUGIN_RATE
		if (strcmp(id, "rate_converter") == 0) {
			rate_converter = n;
			continue;
		}
#endif
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!slave) {
		SNDERR("slave is not defined");
		return -EINVAL;
	}
	err = snd_pcm_slave_conf(root, slave, &sconf, 3,
				 SND_PCM_HW_PARAM_FORMAT, SCONF_UNCHANGED, &sformat,
				 SND_PCM_HW_PARAM_CHANNELS, SCONF_UNCHANGED, &schannels,
				 SND_PCM_HW_PARAM_RATE, SCONF_UNCHANGED, &srate);
	if (err < 0)
		return err;
#ifdef BUILD_PCM_PLUGIN_ROUTE
	if (tt) {
		err = snd_pcm_route_determine_ttable(tt, &csize, &ssize);
		if (err < 0) {
			snd_config_delete(sconf);
			return err;
		}
		ttable = malloc(csize * ssize * sizeof(*ttable));
		if (ttable == NULL) {
			snd_config_delete(sconf);
			return err;
		}
		err = snd_pcm_route_load_ttable(tt, ttable, csize, ssize, &cused, &sused, -1);
		if (err < 0) {
			snd_config_delete(sconf);
			return err;
		}
	}
#endif
	
#ifdef BUILD_PCM_PLUGIN_RATE
	if (! rate_converter)
		rate_converter = snd_pcm_rate_get_default_converter(root);
#endif

	err = snd_pcm_open_slave(&spcm, root, sconf, stream, mode, conf);
	snd_config_delete(sconf);
	if (err < 0)
		return err;
	err = snd_pcm_plug_open(pcmp, name, sformat, schannels, srate, rate_converter,
				route_policy, ttable, ssize, cused, sused, spcm, 1);
	if (err < 0)
		snd_pcm_close(spcm);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_plug_open, SND_PCM_DLSYM_VERSION);
#endif
