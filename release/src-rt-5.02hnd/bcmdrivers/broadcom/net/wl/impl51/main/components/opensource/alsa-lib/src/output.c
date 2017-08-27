/**
 * \file output.c
 * \brief Generic stdio-like output interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000
 *
 * Generic stdio-like output interface
 */
/*
 *  Output object
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "local.h"

#ifndef DOC_HIDDEN
typedef struct _snd_output_ops {
	int (*close)(snd_output_t *output);
	int (*print)(snd_output_t *output, const char *format, va_list args);
	int (*puts)(snd_output_t *output, const char *str);
	int (*putch)(snd_output_t *output, int c);
	int (*flush)(snd_output_t *output);
} snd_output_ops_t;

struct _snd_output {
	snd_output_type_t type;
	const snd_output_ops_t *ops;
	void *private_data;
};
#endif

/**
 * \brief Closes an output handle.
 * \param output The output handle to be closed.
 * \return Zero if successful, otherwise a negative error code.
 */
int snd_output_close(snd_output_t *output)
{
	int err = output->ops->close(output);
	free(output);
	return err;
}

/**
 * \brief Writes formatted output (like \c fprintf(3)) to an output handle.
 * \param output The output handle.
 * \param format Format string in \c fprintf format.
 * \param ... Other \c fprintf arguments.
 * \return The number of characters written, or a negative error code.
 */
int snd_output_printf(snd_output_t *output, const char *format, ...)
{
	int result;
	va_list args;
	va_start(args, format);
	result = output->ops->print(output, format, args);
	va_end(args);
	return result;
}

/**
 * \brief Writes formatted output (like \c fprintf(3)) to an output handle.
 * \param output The output handle.
 * \param format Format string in \c fprintf format.
 * \param args Other \c fprintf arguments.
 * \return The number of characters written, or a negative error code.
 */
int snd_output_vprintf(snd_output_t *output, const char *format, va_list args)
{
	return output->ops->print(output, format, args);
}

/**
 * \brief Writes a string to an output handle (like \c fputs(3)).
 * \param output The output handle.
 * \param str Pointer to the string.
 * \return Zero if successful, otherwise a negative error code or \c EOF.
 */
int snd_output_puts(snd_output_t *output, const char *str)
{
	return output->ops->puts(output, str);
}
			
/**
 * \brief Writes a character to an output handle (like \c putc(3)).
 * \param output The output handle.
 * \param c The character.
 * \return Zero if successful, otherwise a negative error code or \c EOF.
 */
int snd_output_putc(snd_output_t *output, int c)
{
	return output->ops->putch(output, c);
}

/**
 * \brief Flushes an output handle (like fflush(3)).
 * \param output The output handle.
 * \return Zero if successful, otherwise \c EOF.
 *
 * If the underlying destination is a stdio stream, this function calls
 * \c fflush. If the underlying destination is a memory buffer, the write
 * position is reset to the beginning of the buffer. \c =:-o
 */
int snd_output_flush(snd_output_t *output)
{
	return output->ops->flush(output);
}

#ifndef DOC_HIDDEN
typedef struct _snd_output_stdio {
	int close;
	FILE *fp;
} snd_output_stdio_t;

static int snd_output_stdio_close(snd_output_t *output)
{
	snd_output_stdio_t *stdio = output->private_data;
	if (stdio->close)
		fclose(stdio->fp);
	free(stdio);
	return 0;
}

static int snd_output_stdio_print(snd_output_t *output, const char *format, va_list args)
{
	snd_output_stdio_t *stdio = output->private_data;
	return vfprintf(stdio->fp, format, args);
}

static int snd_output_stdio_puts(snd_output_t *output, const char *str)
{
	snd_output_stdio_t *stdio = output->private_data;
	return fputs(str, stdio->fp);
}
			
static int snd_output_stdio_putc(snd_output_t *output, int c)
{
	snd_output_stdio_t *stdio = output->private_data;
	return putc(c, stdio->fp);
}

static int snd_output_stdio_flush(snd_output_t *output)
{
	snd_output_stdio_t *stdio = output->private_data;
	return fflush(stdio->fp);
}

static const snd_output_ops_t snd_output_stdio_ops = {
	.close		= snd_output_stdio_close,
	.print		= snd_output_stdio_print,
	.puts		= snd_output_stdio_puts,
	.putch		= snd_output_stdio_putc,
	.flush		= snd_output_stdio_flush,
};

#endif

/**
 * \brief Creates a new output object using an existing stdio \c FILE pointer.
 * \param outputp The function puts the pointer to the new output object
 *                at the address specified by \p outputp.
 * \param fp The \c FILE pointer to write to. Characters are written
 *           to the file starting at the current file position.
 * \param _close Close flag. Set this to 1 if #snd_output_close should close
 *              \p fp by calling \c fclose.
 * \return Zero if successful, otherwise a negative error code.
 */
int snd_output_stdio_attach(snd_output_t **outputp, FILE *fp, int _close)
{
	snd_output_t *output;
	snd_output_stdio_t *stdio;
	assert(outputp && fp);
	stdio = calloc(1, sizeof(*stdio));
	if (!stdio)
		return -ENOMEM;
	output = calloc(1, sizeof(*output));
	if (!output) {
		free(stdio);
		return -ENOMEM;
	}
	stdio->fp = fp;
	stdio->close = _close;
	output->type = SND_OUTPUT_STDIO;
	output->ops = &snd_output_stdio_ops;
	output->private_data = stdio;
	*outputp = output;
	return 0;
}
	
/**
 * \brief Creates a new output object writing to a file.
 * \param outputp The function puts the pointer to the new output object
 *                at the address specified by \p outputp.
 * \param file The name of the file to open.
 * \param mode The open mode, like \c fopen(3).
 * \return Zero if successful, otherwise a negative error code.
 */
int snd_output_stdio_open(snd_output_t **outputp, const char *file, const char *mode)
{
	int err;
	FILE *fp = fopen(file, mode);
	if (!fp) {
		//SYSERR("fopen");
		return -errno;
	}
	err = snd_output_stdio_attach(outputp, fp, 1);
	if (err < 0)
		fclose(fp);
	return err;
}

#ifndef DOC_HIDDEN

typedef struct _snd_output_buffer {
	unsigned char *buf;
	size_t alloc;
	size_t size;
} snd_output_buffer_t;

static int snd_output_buffer_close(snd_output_t *output)
{
	snd_output_buffer_t *buffer = output->private_data;
	free(buffer->buf);
	free(buffer);
	return 0;
}

static int snd_output_buffer_need(snd_output_t *output, size_t size)
{
	snd_output_buffer_t *buffer = output->private_data;
	size_t _free = buffer->alloc - buffer->size;
	size_t alloc;
	unsigned char *buf;

	if (_free >= size)
		return _free;
	if (buffer->alloc == 0)
		alloc = 256;
	else
		alloc = buffer->alloc;
	while (alloc < buffer->size + size)
		alloc *= 2;
	buf = realloc(buffer->buf, alloc);
	if (!buf)
		return -ENOMEM;
	buffer->buf = buf;
	buffer->alloc = alloc;
	return buffer->alloc - buffer->size;
}

static int snd_output_buffer_print(snd_output_t *output, const char *format, va_list args)
{
	snd_output_buffer_t *buffer = output->private_data;
	size_t size = 256;
	int result;
	result = snd_output_buffer_need(output, size);
	if (result < 0)
		return result;
	result = vsnprintf((char *)buffer->buf + buffer->size, size, format, args);
	assert(result >= 0);
	if ((size_t)result <= size) {
		buffer->size += result;
		return result;
	}
	size = result;
	result = snd_output_buffer_need(output, size);
	if (result < 0)
		return result;
	result = vsnprintf((char *)buffer->buf + buffer->size, result, format, args);
	assert(result == (int)size);
	buffer->size += result;
	return result;
}

static int snd_output_buffer_puts(snd_output_t *output, const char *str)
{
	snd_output_buffer_t *buffer = output->private_data;
	size_t size = strlen(str);
	int err;
	err = snd_output_buffer_need(output, size);
	if (err < 0)
		return err;
	memcpy(buffer->buf + buffer->size, str, size);
	buffer->size += size;
	return size;
}
			
static int snd_output_buffer_putc(snd_output_t *output, int c)
{
	snd_output_buffer_t *buffer = output->private_data;
	int err;
	err = snd_output_buffer_need(output, 1);
	if (err < 0)
		return err;
	buffer->buf[buffer->size++] = c;
	return 0;
}

static int snd_output_buffer_flush(snd_output_t *output ATTRIBUTE_UNUSED)
{
	snd_output_buffer_t *buffer = output->private_data;
	buffer->size = 0;
	return 0;
}

static const snd_output_ops_t snd_output_buffer_ops = {
	.close		= snd_output_buffer_close,
	.print		= snd_output_buffer_print,
	.puts		= snd_output_buffer_puts,
	.putch		= snd_output_buffer_putc,
	.flush		= snd_output_buffer_flush,
};
#endif

/**
 * \brief Returns the address of the buffer of a #SND_OUTPUT_BUFFER output handle.
 * \param output The output handle.
 * \param buf The functions puts the current address of the buffer at the
 *            address specified by \p buf.
 * \return The current size of valid data in the buffer.
 *
 * The address of the buffer may become invalid when output functions or
 * #snd_output_close are called.
 */
size_t snd_output_buffer_string(snd_output_t *output, char **buf)
{
	snd_output_buffer_t *buffer = output->private_data;
	*buf = (char *)buffer->buf;
	return buffer->size;
}

/**
 * \brief Creates a new output object with an auto-extending memory buffer.
 * \param outputp The function puts the pointer to the new output object
 *                at the address specified by \p outputp.
 * \return Zero if successful, otherwise a negative error code.
 */
int snd_output_buffer_open(snd_output_t **outputp)
{
	snd_output_t *output;
	snd_output_buffer_t *buffer;
	assert(outputp);
	buffer = calloc(1, sizeof(*buffer));
	if (!buffer)
		return -ENOMEM;
	output = calloc(1, sizeof(*output));
	if (!output) {
		free(buffer);
		return -ENOMEM;
	}
	buffer->buf = NULL;
	buffer->alloc = 0;
	buffer->size = 0;
	output->type = SND_OUTPUT_BUFFER;
	output->ops = &snd_output_buffer_ops;
	output->private_data = buffer;
	*outputp = output;
	return 0;
}
	
