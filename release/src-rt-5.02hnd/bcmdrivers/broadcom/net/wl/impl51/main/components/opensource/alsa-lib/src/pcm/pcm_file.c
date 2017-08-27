/**
 * \file pcm/pcm_file.c
 * \ingroup PCM_Plugins
 * \brief PCM File Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000-2001
 */
/*
 *  PCM - File plugin
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
  
#include <endian.h>
#include <byteswap.h>
#include <ctype.h>
#include <string.h>
#include "pcm_local.h"
#include "pcm_plugin.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_file = "";
#endif

#ifndef DOC_HIDDEN

/* keys to be replaced by real values in the filename */
#define LEADING_KEY	'%'	/* i.e. %r, %c, %b ... */
#define RATE_KEY	'r'
#define CHANNELS_KEY	'c'
#define BWIDTH_KEY	'b'
#define FORMAT_KEY	'f'

/* maximum length of a value */
#define VALUE_MAXLEN	64

typedef enum _snd_pcm_file_format {
	SND_PCM_FILE_FORMAT_RAW,
	SND_PCM_FILE_FORMAT_WAV
} snd_pcm_file_format_t;

/* WAV format chunk */
struct wav_fmt {
	short fmt;
	short chan;
	int rate;
	int bps;
	short bwidth;
	short bits;
};

typedef struct {
	snd_pcm_generic_t gen;
	char *fname;
	char *final_fname;
	int trunc;
	int perm;
	int fd;
	char *ifname;
	int ifd;
	int format;
	snd_pcm_uframes_t appl_ptr;
	snd_pcm_uframes_t file_ptr_bytes;
	snd_pcm_uframes_t wbuf_size;
	size_t wbuf_size_bytes;
	size_t wbuf_used_bytes;
	char *wbuf;
	size_t rbuf_size_bytes;
	size_t rbuf_used_bytes;
	char *rbuf;
	snd_pcm_channel_area_t *wbuf_areas;
	size_t buffer_bytes;
	struct wav_fmt wav_header;
	size_t filelen;
} snd_pcm_file_t;

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define TO_LE32(x)	(x)
#define TO_LE16(x)	(x)
#else
#define TO_LE32(x)	bswap_32(x)
#define TO_LE16(x)	bswap_16(x)
#endif

static int snd_pcm_file_append_value(char **string_p, char **index_ch_p,
		int *len_p, const char *value)
{
	char *string, *index_ch;
	int index, len, value_len;
	/* input pointer values */
	len = *(len_p);
	string = *(string_p);
	index_ch = *(index_ch_p);

	value_len = strlen(value);
	/* reallocation to accommodate the value */
	index = index_ch - string;
	len += value_len;
	string = realloc(string, len + 1);
	if (!string)
		return -ENOMEM;
	index_ch = string + index;
	/* concatenating the new value */
	strcpy(index_ch, value);
	index_ch += value_len;
	/* return values */
	*(len_p) = len;
	*(string_p) = string;
	*(index_ch_p) = index_ch;
	return 0;
}

static int snd_pcm_file_replace_fname(snd_pcm_file_t *file, char **new_fname_p)
{
	char value[VALUE_MAXLEN];
	char *fname = file->fname;
	char *new_fname = NULL;
	char *old_last_ch, *old_index_ch, *new_index_ch;
	int old_len, new_len, err;

	snd_pcm_t *pcm = file->gen.slave;

	/* we want to keep fname, const */
	old_len = new_len = strlen(fname);
	old_last_ch = fname + old_len - 1;
	new_fname = malloc(new_len + 1);
	if (!new_fname)
		return -ENOMEM;

	old_index_ch = fname;	/* first character of the old name */
	new_index_ch = new_fname;	/* first char of the new name */

	while (old_index_ch <= old_last_ch) {
		if (*(old_index_ch) == LEADING_KEY &&
				old_index_ch != old_last_ch) {
			/* is %, not last char, skipping and checking
			 next char */
			switch (*(++old_index_ch)) {
			case RATE_KEY:
				snprintf(value, sizeof(value), "%d",
						pcm->rate);
				err = snd_pcm_file_append_value(&new_fname,
					&new_index_ch, &new_len, value);
				if (err < 0)
					return err;
				break;

			case CHANNELS_KEY:
				snprintf(value, sizeof(value), "%d",
						pcm->channels);
				err = snd_pcm_file_append_value(&new_fname,
					&new_index_ch, &new_len, value);
				if (err < 0)
					return err;
				break;

			case BWIDTH_KEY:
				snprintf(value, sizeof(value), "%d",
					pcm->frame_bits/pcm->channels);
				err = snd_pcm_file_append_value(&new_fname,
						&new_index_ch, &new_len, value);
				if (err < 0)
					return err;
				break;

			case FORMAT_KEY:
				err = snd_pcm_file_append_value(&new_fname,
					&new_index_ch, &new_len,
					snd_pcm_format_name(pcm->format));
				if (err < 0)
					return err;
				break;

			default:
				/* non-key char, just copying */
				*(new_index_ch++) = *(old_index_ch);
			}
			/* next old char */
			old_index_ch++;
		} else {
			/* plain copying, shifting both strings to next chars */
			*(new_index_ch++) = *(old_index_ch++);
		}
	}
	/* closing the new string */
	*(new_index_ch) = '\0';
	*(new_fname_p) = new_fname;
	return 0;

}

static int snd_pcm_file_open_output_file(snd_pcm_file_t *file)
{
	int err, fd;

	/* fname can contain keys, generating final_fname */
	err = snd_pcm_file_replace_fname(file, &(file->final_fname));
	if (err < 0)
		return err;
	/*printf("DEBUG - original fname: %s, final fname: %s\n",
	  file->fname, file->final_fname);*/

	if (file->final_fname[0] == '|') {
		/* pipe mode */
		FILE *pipe;
		/* clearing */
		pipe = popen(file->final_fname + 1, "w");
		if (!pipe) {
			SYSERR("running %s for writing failed",
					file->final_fname);
			return -errno;
		}
		fd = fileno(pipe);
	} else {
		if (file->trunc)
			fd = open(file->final_fname, O_WRONLY|O_CREAT|O_TRUNC,
					file->perm);
		else {
			fd = open(file->final_fname, O_WRONLY|O_CREAT|O_EXCL,
					file->perm);
			if (fd < 0) {
				char *tmpfname = NULL;
				int idx, len;
				len = strlen(file->final_fname) + 6;
				tmpfname = malloc(len);
				if (!tmpfname)
					return -ENOMEM;
				for (idx = 1; idx < 10000; idx++) {
					snprintf(tmpfname, len,
						"%s.%04d", file->final_fname,
						idx);
					fd = open(tmpfname,
							O_WRONLY|O_CREAT|O_EXCL,
							file->perm);
					if (fd >= 0) {
						free(file->final_fname);
						file->final_fname = tmpfname;
						break;
					}
				}
				if (fd < 0) {
					SYSERR("open %s for writing failed",
							file->final_fname);
					free(tmpfname);
					return -errno;
				}
			}
		}
	}
	file->fd = fd;
	return 0;
}

static void setup_wav_header(snd_pcm_t *pcm, struct wav_fmt *fmt)
{
	fmt->fmt = TO_LE16(0x01);
	fmt->chan = TO_LE16(pcm->channels);
	fmt->rate = TO_LE32(pcm->rate);
	fmt->bwidth = pcm->frame_bits / 8;
	fmt->bps = fmt->bwidth * pcm->rate;
	fmt->bits = snd_pcm_format_width(pcm->format);
	fmt->bps = TO_LE32(fmt->bps);
	fmt->bwidth = TO_LE16(fmt->bwidth);
	fmt->bits = TO_LE16(fmt->bits);
}

static int write_wav_header(snd_pcm_t *pcm)
{
	snd_pcm_file_t *file = pcm->private_data;
	static const char header[] = {
		'R', 'I', 'F', 'F',
		0x24, 0, 0, 0,
		'W', 'A', 'V', 'E',
		'f', 'm', 't', ' ',
		0x10, 0, 0, 0,
	};
	static const char header2[] = {
		'd', 'a', 't', 'a',
		0, 0, 0, 0
	};
	
	setup_wav_header(pcm, &file->wav_header);

	if (write(file->fd, header, sizeof(header)) != sizeof(header) ||
	    write(file->fd, &file->wav_header, sizeof(file->wav_header)) !=
	    sizeof(file->wav_header) ||
	    write(file->fd, header2, sizeof(header2)) != sizeof(header2)) {
		int err = errno;
		SYSERR("Write error.\n");
		return -err;
	}
	return 0;
}

/* fix up the length fields in WAV header */
static void fixup_wav_header(snd_pcm_t *pcm)
{
	snd_pcm_file_t *file = pcm->private_data;
	int len, ret;

	/* RIFF length */
	if (lseek(file->fd, 4, SEEK_SET) == 4) {
		len = (file->filelen + 0x24) > 0x7fffffff ?
			0x7fffffff : (int)(file->filelen + 0x24);
		len = TO_LE32(len);
		ret = write(file->fd, &len, 4);
		if (ret < 0)
			return;
	}
	/* data length */
	if (lseek(file->fd, 0x28, SEEK_SET) == 0x28) {
		len = file->filelen > 0x7fffffff ?
			0x7fffffff : (int)file->filelen;
		len = TO_LE32(len);
		ret = write(file->fd, &len, 4);
		if (ret < 0)
			return;
	}
}
#endif /* DOC_HIDDEN */



static void snd_pcm_file_write_bytes(snd_pcm_t *pcm, size_t bytes)
{
	snd_pcm_file_t *file = pcm->private_data;
	assert(bytes <= file->wbuf_used_bytes);

	if (file->format == SND_PCM_FILE_FORMAT_WAV &&
	    !file->wav_header.fmt) {
		if (write_wav_header(pcm) < 0)
			return;
	}

	while (bytes > 0) {
		snd_pcm_sframes_t err;
		size_t n = bytes;
		size_t cont = file->wbuf_size_bytes - file->file_ptr_bytes;
		if (n > cont)
			n = cont;
		err = write(file->fd, file->wbuf + file->file_ptr_bytes, n);
		if (err < 0) {
			SYSERR("write failed");
			break;
		}
		bytes -= err;
		file->wbuf_used_bytes -= err;
		file->file_ptr_bytes += err;
		if (file->file_ptr_bytes == file->wbuf_size_bytes)
			file->file_ptr_bytes = 0;
		file->filelen += err;
		if ((snd_pcm_uframes_t)err != n)
			break;
	}
}

static void snd_pcm_file_add_frames(snd_pcm_t *pcm, 
				    const snd_pcm_channel_area_t *areas,
				    snd_pcm_uframes_t offset,
				    snd_pcm_uframes_t frames)
{
	snd_pcm_file_t *file = pcm->private_data;
	while (frames > 0) {
		snd_pcm_uframes_t n = frames;
		snd_pcm_uframes_t cont = file->wbuf_size - file->appl_ptr;
		snd_pcm_uframes_t avail = file->wbuf_size - snd_pcm_bytes_to_frames(pcm, file->wbuf_used_bytes);
		if (n > cont)
			n = cont;
		if (n > avail)
			n = avail;
		snd_pcm_areas_copy(file->wbuf_areas, file->appl_ptr, 
				   areas, offset,
				   pcm->channels, n, pcm->format);
		frames -= n;
		offset += n;
		file->appl_ptr += n;
		if (file->appl_ptr == file->wbuf_size)
			file->appl_ptr = 0;
		file->wbuf_used_bytes += snd_pcm_frames_to_bytes(pcm, n);
		if (file->wbuf_used_bytes > file->buffer_bytes)
			snd_pcm_file_write_bytes(pcm, file->wbuf_used_bytes - file->buffer_bytes);
		assert(file->wbuf_used_bytes < file->wbuf_size_bytes);
	}
}

static int snd_pcm_file_close(snd_pcm_t *pcm)
{
	snd_pcm_file_t *file = pcm->private_data;
	if (file->fname) {
		if (file->wav_header.fmt)
			fixup_wav_header(pcm);
		free((void *)file->fname);
		close(file->fd);
	}
	if (file->ifname) {
		free((void *)file->ifname);
		close(file->ifd);
	}
	return snd_pcm_generic_close(pcm);
}

static int snd_pcm_file_reset(snd_pcm_t *pcm)
{
	snd_pcm_file_t *file = pcm->private_data;
	int err = snd_pcm_reset(file->gen.slave);
	if (err >= 0) {
		snd_pcm_file_write_bytes(pcm, file->wbuf_used_bytes);
		assert(file->wbuf_used_bytes == 0);
	}
	return err;
}

static int snd_pcm_file_drop(snd_pcm_t *pcm)
{
	snd_pcm_file_t *file = pcm->private_data;
	int err = snd_pcm_drop(file->gen.slave);
	if (err >= 0) {
		snd_pcm_file_write_bytes(pcm, file->wbuf_used_bytes);
		assert(file->wbuf_used_bytes == 0);
	}
	return err;
}

static int snd_pcm_file_drain(snd_pcm_t *pcm)
{
	snd_pcm_file_t *file = pcm->private_data;
	int err = snd_pcm_drain(file->gen.slave);
	if (err >= 0) {
		snd_pcm_file_write_bytes(pcm, file->wbuf_used_bytes);
		assert(file->wbuf_used_bytes == 0);
	}
	return err;
}

static snd_pcm_sframes_t snd_pcm_file_rewindable(snd_pcm_t *pcm)
{
	snd_pcm_file_t *file = pcm->private_data;
	snd_pcm_sframes_t res = snd_pcm_rewindable(pcm);
	snd_pcm_sframes_t n = snd_pcm_bytes_to_frames(pcm, file->wbuf_used_bytes);
	if (res > n)
		res = n;
	return res;
}

static snd_pcm_sframes_t snd_pcm_file_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_file_t *file = pcm->private_data;
	snd_pcm_sframes_t err;
	snd_pcm_uframes_t n;
	
	n = snd_pcm_frames_to_bytes(pcm, frames);
	if (n > file->wbuf_used_bytes)
		frames = snd_pcm_bytes_to_frames(pcm, file->wbuf_used_bytes);
	err = snd_pcm_rewind(file->gen.slave, frames);
	if (err > 0) {
		file->appl_ptr = (file->appl_ptr - err + file->wbuf_size) % file->wbuf_size;
		n = snd_pcm_frames_to_bytes(pcm, err);
		file->wbuf_used_bytes -= n;
	}
	return err;
}

static snd_pcm_sframes_t snd_pcm_file_forwardable(snd_pcm_t *pcm)
{
	snd_pcm_file_t *file = pcm->private_data;
	snd_pcm_sframes_t res = snd_pcm_forwardable(pcm);
	snd_pcm_sframes_t n = snd_pcm_bytes_to_frames(pcm, file->wbuf_size_bytes - file->wbuf_used_bytes);
	if (res > n)
		res = n;
	return res;
}

static snd_pcm_sframes_t snd_pcm_file_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_file_t *file = pcm->private_data;
	snd_pcm_sframes_t err;
	snd_pcm_uframes_t n;
	
	n = snd_pcm_frames_to_bytes(pcm, frames);
	if (file->wbuf_used_bytes + n > file->wbuf_size_bytes)
		frames = snd_pcm_bytes_to_frames(pcm, file->wbuf_size_bytes - file->wbuf_used_bytes);
	err = INTERNAL(snd_pcm_forward)(file->gen.slave, frames);
	if (err > 0) {
		file->appl_ptr = (file->appl_ptr + err) % file->wbuf_size;
		n = snd_pcm_frames_to_bytes(pcm, err);
		file->wbuf_used_bytes += n;
	}
	return err;
}

static snd_pcm_sframes_t snd_pcm_file_writei(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size)
{
	snd_pcm_file_t *file = pcm->private_data;
	snd_pcm_channel_area_t areas[pcm->channels];
	snd_pcm_sframes_t n = snd_pcm_writei(file->gen.slave, buffer, size);
	if (n > 0) {
		snd_pcm_areas_from_buf(pcm, areas, (void*) buffer);
		snd_pcm_file_add_frames(pcm, areas, 0, n);
	}
	return n;
}

static snd_pcm_sframes_t snd_pcm_file_writen(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size)
{
	snd_pcm_file_t *file = pcm->private_data;
	snd_pcm_channel_area_t areas[pcm->channels];
	snd_pcm_sframes_t n = snd_pcm_writen(file->gen.slave, bufs, size);
	if (n > 0) {
		snd_pcm_areas_from_bufs(pcm, areas, bufs);
		snd_pcm_file_add_frames(pcm, areas, 0, n);
	}
	return n;
}

static snd_pcm_sframes_t snd_pcm_file_readi(snd_pcm_t *pcm, void *buffer, snd_pcm_uframes_t size)
{
	snd_pcm_file_t *file = pcm->private_data;
	snd_pcm_channel_area_t areas[pcm->channels];
	snd_pcm_sframes_t n;

	n = snd_pcm_readi(file->gen.slave, buffer, size);
	if (n <= 0)
		return n;
	if (file->ifd >= 0) {
		n = read(file->ifd, buffer, n * pcm->frame_bits / 8);
		if (n < 0)
			return n;
		return n * 8 / pcm->frame_bits;
	}
	snd_pcm_areas_from_buf(pcm, areas, buffer);
	snd_pcm_file_add_frames(pcm, areas, 0, n);
	return n;
}

static snd_pcm_sframes_t snd_pcm_file_readn(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size)
{
	snd_pcm_file_t *file = pcm->private_data;
	snd_pcm_channel_area_t areas[pcm->channels];
	snd_pcm_sframes_t n;

	if (file->ifd >= 0) {
		SNDERR("DEBUG: Noninterleaved read not yet implemented.\n");
		return 0;	/* TODO: Noninterleaved read */
	}

	n = snd_pcm_readn(file->gen.slave, bufs, size);
	if (n > 0) {
		snd_pcm_areas_from_bufs(pcm, areas, bufs);
		snd_pcm_file_add_frames(pcm, areas, 0, n);
	}
	return n;
}

static snd_pcm_sframes_t snd_pcm_file_mmap_commit(snd_pcm_t *pcm,
					          snd_pcm_uframes_t offset,
						  snd_pcm_uframes_t size)
{
	snd_pcm_file_t *file = pcm->private_data;
	snd_pcm_uframes_t ofs;
	snd_pcm_uframes_t siz = size;
	const snd_pcm_channel_area_t *areas;
	snd_pcm_sframes_t result;

	snd_pcm_mmap_begin(file->gen.slave, &areas, &ofs, &siz);
	assert(ofs == offset && siz == size);
	result = snd_pcm_mmap_commit(file->gen.slave, ofs, siz);
	if (result > 0)
		snd_pcm_file_add_frames(pcm, areas, ofs, result);
	return result;
}

static int snd_pcm_file_hw_free(snd_pcm_t *pcm)
{
	snd_pcm_file_t *file = pcm->private_data;
	free(file->wbuf);
	free(file->wbuf_areas);
	file->wbuf = NULL;
	file->wbuf_areas = NULL;
	return snd_pcm_hw_free(file->gen.slave);
}

static int snd_pcm_file_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	snd_pcm_file_t *file = pcm->private_data;
	unsigned int channel;
	snd_pcm_t *slave = file->gen.slave;
	int err = _snd_pcm_hw_params(slave, params);
	if (err < 0)
		return err;
	file->buffer_bytes = snd_pcm_frames_to_bytes(slave, slave->buffer_size);
	file->wbuf_size = slave->buffer_size * 2;
	file->wbuf_size_bytes = snd_pcm_frames_to_bytes(slave, file->wbuf_size);
	file->wbuf_used_bytes = 0;
	assert(!file->wbuf);
	file->wbuf = malloc(file->wbuf_size_bytes);
	if (file->wbuf == NULL) {
		snd_pcm_file_hw_free(pcm);
		return -ENOMEM;
	}
	file->wbuf_areas = malloc(sizeof(*file->wbuf_areas) * slave->channels);
	if (file->wbuf_areas == NULL) {
		snd_pcm_file_hw_free(pcm);
		return -ENOMEM;
	}
	file->appl_ptr = file->file_ptr_bytes = 0;
	for (channel = 0; channel < slave->channels; ++channel) {
		snd_pcm_channel_area_t *a = &file->wbuf_areas[channel];
		a->addr = file->wbuf;
		a->first = slave->sample_bits * channel;
		a->step = slave->frame_bits;
	}
	if (file->fd < 0) {
		err = snd_pcm_file_open_output_file(file);
		if (err < 0) {
			SYSERR("failed opening output file %s", file->fname);
			return err;
		}
	}
	return 0;
}

static void snd_pcm_file_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_file_t *file = pcm->private_data;
	if (file->fname)
		snd_output_printf(out, "File PCM (file=%s)\n", file->fname);
	else
		snd_output_printf(out, "File PCM (fd=%d)\n", file->fd);
	if (file->final_fname)
		snd_output_printf(out, "Final file PCM (file=%s)\n",
				file->final_fname);

	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(file->gen.slave, out);
}

static const snd_pcm_ops_t snd_pcm_file_ops = {
	.close = snd_pcm_file_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_generic_hw_refine,
	.hw_params = snd_pcm_file_hw_params,
	.hw_free = snd_pcm_file_hw_free,
	.sw_params = snd_pcm_generic_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_file_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

static const snd_pcm_fast_ops_t snd_pcm_file_fast_ops = {
	.status = snd_pcm_generic_status,
	.state = snd_pcm_generic_state,
	.hwsync = snd_pcm_generic_hwsync,
	.delay = snd_pcm_generic_delay,
	.prepare = snd_pcm_generic_prepare,
	.reset = snd_pcm_file_reset,
	.start = snd_pcm_generic_start,
	.drop = snd_pcm_file_drop,
	.drain = snd_pcm_file_drain,
	.pause = snd_pcm_generic_pause,
	.rewindable = snd_pcm_file_rewindable,
	.rewind = snd_pcm_file_rewind,
	.forwardable = snd_pcm_file_forwardable,
	.forward = snd_pcm_file_forward,
	.resume = snd_pcm_generic_resume,
	.link = snd_pcm_generic_link,
	.link_slaves = snd_pcm_generic_link_slaves,
	.unlink = snd_pcm_generic_unlink,
	.writei = snd_pcm_file_writei,
	.writen = snd_pcm_file_writen,
	.readi = snd_pcm_file_readi,
	.readn = snd_pcm_file_readn,
	.avail_update = snd_pcm_generic_avail_update,
	.mmap_commit = snd_pcm_file_mmap_commit,
	.poll_descriptors_count = snd_pcm_generic_poll_descriptors_count,
	.poll_descriptors = snd_pcm_generic_poll_descriptors,
	.poll_revents = snd_pcm_generic_poll_revents,
};

/**
 * \brief Creates a new File PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param fname Output filename (or NULL if file descriptor fd is available)
 * \param fd Output file descriptor
 * \param ifname Input filename (or NULL if file descriptor ifd is available)
 * \param ifd Input file descriptor (if (ifd < 0) && (ifname == NULL), no input
 *            redirection will be performed)
 * \param trunc Truncate the file if it already exists
 * \param fmt File format ("raw" or "wav" are available)
 * \param perm File permission
 * \param slave Slave PCM handle
 * \param close_slave When set, the slave PCM handle is closed with copy PCM
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_file_open(snd_pcm_t **pcmp, const char *name,
		      const char *fname, int fd, const char *ifname, int ifd,
		      int trunc,
		      const char *fmt, int perm, snd_pcm_t *slave, int close_slave)
{
	snd_pcm_t *pcm;
	snd_pcm_file_t *file;
	snd_pcm_file_format_t format;
	struct timespec timespec;
	int err;

	assert(pcmp);
	if (fmt == NULL ||
	    strcmp(fmt, "raw") == 0)
		format = SND_PCM_FILE_FORMAT_RAW;
	else if (!strcmp(fmt, "wav"))
		format = SND_PCM_FILE_FORMAT_WAV;
	else {
		SNDERR("file format %s is unknown", fmt);
		return -EINVAL;
	}
	file = calloc(1, sizeof(snd_pcm_file_t));
	if (!file) {
		return -ENOMEM;
	}

	/* opening output fname is delayed until writing,
	 when PCM params are known */
	if (fname)
		file->fname = strdup(fname);
	file->trunc = trunc;
	file->perm = perm;

	if (ifname) {
		ifd = open(ifname, O_RDONLY);	/* TODO: mind blocking mode */
		if (ifd < 0) {
			SYSERR("open %s for reading failed", ifname);
			free(file);
			return -errno;
		}
		file->ifname = strdup(ifname);
	}
	file->fd = fd;
	file->ifd = ifd;
	file->format = format;
	file->gen.slave = slave;
	file->gen.close_slave = close_slave;

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_FILE, name, slave->stream, slave->mode);
	if (err < 0) {
		free(file->fname);
		free(file);
		return err;
	}
	pcm->ops = &snd_pcm_file_ops;
	pcm->fast_ops = &snd_pcm_file_fast_ops;
	pcm->private_data = file;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	pcm->mmap_shadow = 1;
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
	pcm->monotonic = clock_gettime(CLOCK_MONOTONIC, &timespec) == 0;
#else
	pcm->monotonic = 0;
#endif
	snd_pcm_link_hw_ptr(pcm, slave);
	snd_pcm_link_appl_ptr(pcm, slave);
	*pcmp = pcm;
	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_file Plugin: File

This plugin stores contents of a PCM stream to file or pipes the stream
to a command, and optionally uses an existing file as an input data source
(i.e., "virtual mic")

\code
pcm.name {
        type file               # File PCM
        slave STR               # Slave name
        # or
        slave {                 # Slave definition
                pcm STR         # Slave PCM name
                # or
                pcm { }         # Slave PCM definition
        }
	file STR		# Output filename (or shell command the stream
				# will be piped to if STR starts with the pipe
				# char).
				# STR can contain format keys, replaced by
				# real values corresponding to the stream:
				# %r	rate (replaced with: 48000)
				# %c	channels (replaced with: 2)
				# %b	bits per sample (replaced with: 16)
				# %f	sample format string
				#			(replaced with: S16_LE)
				# %%	replaced with %
	or
	file INT		# Output file descriptor number
	infile STR		# Input filename - only raw format
	or
	infile INT		# Input file descriptor number
	[format STR]		# File format ("raw" or "wav")
	[perm INT]		# Output file permission (octal, def. 0600)
}
\endcode

\subsection pcm_plugins_file_funcref Function reference

<UL>
  <LI>snd_pcm_file_open()
  <LI>_snd_pcm_file_open()
</UL>

*/

/**
 * \brief Creates a new File PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with File PCM description
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_file_open(snd_pcm_t **pcmp, const char *name,
		       snd_config_t *root, snd_config_t *conf, 
		       snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	int err;
	snd_pcm_t *spcm;
	snd_config_t *slave = NULL, *sconf;
	const char *fname = NULL, *ifname = NULL;
	const char *format = NULL;
	long fd = -1, ifd = -1, trunc = 1;
	long perm = 0600;
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
		if (strcmp(id, "format") == 0) {
			err = snd_config_get_string(n, &format);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			continue;
		}
		if (strcmp(id, "file") == 0) {
			err = snd_config_get_string(n, &fname);
			if (err < 0) {
				err = snd_config_get_integer(n, &fd);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					return -EINVAL;
				}
			}
			continue;
		}
		if (strcmp(id, "infile") == 0) {
			err = snd_config_get_string(n, &ifname);
			if (err < 0) {
				err = snd_config_get_integer(n, &ifd);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					return -EINVAL;
				}
			}
			continue;
		}
		if (strcmp(id, "perm") == 0) {
			err = snd_config_get_integer(n, &perm);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return err;
			}
			if ((perm & ~0777) != 0) {
				SNDERR("The field perm must be a valid file permission");
				return -EINVAL;
			}
			continue;
		}
		if (strcmp(id, "truncate") == 0) {
			err = snd_config_get_bool(n);
			if (err < 0)
				return -EINVAL;
			trunc = err;
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!format) {
		snd_config_t *n;
		/* read defaults */
		if (snd_config_search(root, "defaults.pcm.file_format", &n) >= 0) {
			err = snd_config_get_string(n, &format);
			if (err < 0) {
				SNDERR("Invalid file format");
				return -EINVAL;
			}
		}
	}
	if (!slave) {
		SNDERR("slave is not defined");
		return -EINVAL;
	}
	err = snd_pcm_slave_conf(root, slave, &sconf, 0);
	if (err < 0)
		return err;
	if ((!fname || strlen(fname) == 0) && fd < 0 && !ifname) {
		snd_config_delete(sconf);
		SNDERR("file is not defined");
		return -EINVAL;
	}
	err = snd_pcm_open_slave(&spcm, root, sconf, stream, mode, conf);
	snd_config_delete(sconf);
	if (err < 0)
		return err;
	err = snd_pcm_file_open(pcmp, name, fname, fd, ifname, ifd,
				trunc, format, perm, spcm, 1);
	if (err < 0)
		snd_pcm_close(spcm);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_file_open, SND_PCM_DLSYM_VERSION);
#endif
