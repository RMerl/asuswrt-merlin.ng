/**
 * \file input.c
 * \brief Generic stdio-like input interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000
 *
 * Generic stdio-like input interface
 */
/*
 *  Input object
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

typedef struct _snd_input_ops {
	int (*close)(snd_input_t *input);
	int (*scan)(snd_input_t *input, const char *format, va_list args);
	char *(*(gets))(snd_input_t *input, char *str, size_t size);
	int (*getch)(snd_input_t *input);
	int (*ungetch)(snd_input_t *input, int c);
} snd_input_ops_t;

struct _snd_input {
	snd_input_type_t type;
	const snd_input_ops_t *ops;
	void *private_data;
};
#endif

/**
 * \brief Closes an input handle.
 * \param input The input handle to be closed.
 * \return Zero if successful, otherwise a negative error code.
 */
int snd_input_close(snd_input_t *input)
{
	int err = input->ops->close(input);
	free(input);
	return err;
}

/**
 * \brief Reads formatted input (like \c fscanf(3)) from an input handle.
 * \param input The input handle.
 * \param format Format string in \c fscanf format.
 * \param ... Other \c fscanf arguments.
 * \return The number of input items assigned, or \c EOF.
 *
 * \bug Reading from a memory buffer doesn't work.
 */
int snd_input_scanf(snd_input_t *input, const char *format, ...)
{
	int result;
	va_list args;
	va_start(args, format);
	result = input->ops->scan(input, format, args);
	va_end(args);
	return result;
}

/**
 * \brief Reads a line from an input handle (like \c fgets(3)).
 * \param input The input handle.
 * \param str Address of the destination buffer.
 * \param size The size of the destination buffer.
 * \return Pointer to the buffer if successful, otherwise \c NULL.
 *
 * Like \c fgets, the returned string is zero-terminated, and contains
 * the new-line character \c '\\n' if the line fits into the buffer.
 */
char *snd_input_gets(snd_input_t *input, char *str, size_t size)
{
	return (input->ops->gets)(input, str, size);
}
			
/**
 * \brief Reads a character from an input handle (like \c fgetc(3)).
 * \param input The input handle.
 * \return The character read, or \c EOF on end of file or error.
 */
int snd_input_getc(snd_input_t *input)
{
	return input->ops->getch(input);
}

/**
 * \brief Puts the last character read back to an input handle (like \c ungetc(3)).
 * \param input The input handle.
 * \param c The character to push back.
 * \return The character pushed back, or \c EOF on error.
 */
int snd_input_ungetc(snd_input_t *input, int c)
{
	return input->ops->ungetch(input, c);
}

#ifndef DOC_HIDDEN
typedef struct _snd_input_stdio {
	int close;
	FILE *fp;
} snd_input_stdio_t;

static int snd_input_stdio_close(snd_input_t *input ATTRIBUTE_UNUSED)
{
	snd_input_stdio_t *stdio = input->private_data;
	if (stdio->close)
		fclose(stdio->fp);
	free(stdio);
	return 0;
}

static int snd_input_stdio_scan(snd_input_t *input, const char *format, va_list args)
{
	snd_input_stdio_t *stdio = input->private_data;
	extern int vfscanf(FILE *, const char *, va_list);
	return vfscanf(stdio->fp, format, args);
}

static char *snd_input_stdio_gets(snd_input_t *input, char *str, size_t size)
{
	snd_input_stdio_t *stdio = input->private_data;
	return fgets(str, (int) size, stdio->fp);
}
			
static int snd_input_stdio_getc(snd_input_t *input)
{
	snd_input_stdio_t *stdio = input->private_data;
	return getc(stdio->fp);
}

static int snd_input_stdio_ungetc(snd_input_t *input, int c)
{
	snd_input_stdio_t *stdio = input->private_data;
	return ungetc(c, stdio->fp);
}

static const snd_input_ops_t snd_input_stdio_ops = {
	.close		= snd_input_stdio_close,
	.scan		= snd_input_stdio_scan,
	.gets		= snd_input_stdio_gets,
	.getch		= snd_input_stdio_getc,
	.ungetch	= snd_input_stdio_ungetc,
};
#endif

/**
 * \brief Creates a new input object using an existing stdio \c FILE pointer.
 * \param inputp The function puts the pointer to the new input object
 *               at the address specified by \p inputp.
 * \param fp The \c FILE pointer to read from.
 *           Reading begins at the current file position.
 * \param _close Close flag. Set this to 1 if #snd_input_close should close
 *              \p fp by calling \c fclose.
 * \return Zero if successful, otherwise a negative error code.
 */
int snd_input_stdio_attach(snd_input_t **inputp, FILE *fp, int _close)
{
	snd_input_t *input;
	snd_input_stdio_t *stdio;
	assert(inputp && fp);
	stdio = calloc(1, sizeof(*stdio));
	if (!stdio)
		return -ENOMEM;
	input = calloc(1, sizeof(*input));
	if (!input) {
		free(stdio);
		return -ENOMEM;
	}
	stdio->fp = fp;
	stdio->close = _close;
	input->type = SND_INPUT_STDIO;
	input->ops = &snd_input_stdio_ops;
	input->private_data = stdio;
	*inputp = input;
	return 0;
}
	
/**
 * \brief Creates a new input object reading from a file.
 * \param inputp The functions puts the pointer to the new input object
 *               at the address specified by \p inputp.
 * \param file The name of the file to read from.
 * \param mode The open mode, like \c fopen(3).
 * \return Zero if successful, otherwise a negative error code.
 */
int snd_input_stdio_open(snd_input_t **inputp, const char *file, const char *mode)
{
	int err;
	FILE *fp = fopen(file, mode);
	if (!fp) {
		//SYSERR("fopen");
		return -errno;
	}
	err = snd_input_stdio_attach(inputp, fp, 1);
	if (err < 0)
		fclose(fp);
	return err;
}

#ifndef DOC_HIDDEN

typedef struct _snd_input_buffer {
	unsigned char *buf;
	unsigned char *ptr;
	size_t size;
} snd_input_buffer_t;

static int snd_input_buffer_close(snd_input_t *input)
{
	snd_input_buffer_t *buffer = input->private_data;
	free(buffer->buf);
	free(buffer);
	return 0;
}

static int snd_input_buffer_scan(snd_input_t *input, const char *format, va_list args)
{
	snd_input_buffer_t *buffer = input->private_data;
	extern int vsscanf(const char *, const char *, va_list);
	assert(0);
	return vsscanf((char *)buffer->ptr, format, args);
}

static char *snd_input_buffer_gets(snd_input_t *input, char *str, size_t size)
{
	snd_input_buffer_t *buffer = input->private_data;
	size_t bsize = buffer->size;
	while (--size > 0 && bsize > 0) {
		unsigned char c = *buffer->ptr++;
		bsize--;
		*str++ = c;
		if (c == '\n')
			break;
	}
	if (bsize == buffer->size)
		return NULL;
	buffer->size = bsize;
	*str = '\0';
	return str;
}
			
static int snd_input_buffer_getc(snd_input_t *input)
{
	snd_input_buffer_t *buffer = input->private_data;
	if (buffer->size == 0)
		return EOF;
	buffer->size--;
	return *buffer->ptr++;
}

static int snd_input_buffer_ungetc(snd_input_t *input, int c)
{
	snd_input_buffer_t *buffer = input->private_data;
	if (buffer->ptr == buffer->buf)
		return EOF;
	buffer->ptr--;
	assert(*buffer->ptr == (unsigned char) c);
	buffer->size++;
	return c;
}

static const snd_input_ops_t snd_input_buffer_ops = {
	.close		= snd_input_buffer_close,
	.scan		= snd_input_buffer_scan,
	.gets		= snd_input_buffer_gets,
	.getch		= snd_input_buffer_getc,
	.ungetch	= snd_input_buffer_ungetc,
};
#endif

/**
 * \brief Creates a new input object from a memory buffer.
 * \param inputp The function puts the pointer to the new input object
 *               at the address specified by \p inputp.
 * \param buf Address of the input buffer.
 * \param size Size of the input buffer.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This functions creates a copy of the input buffer, so the application is
 * not required to preserve the buffer after this function has been called.
 */
int snd_input_buffer_open(snd_input_t **inputp, const char *buf, ssize_t size)
{
	snd_input_t *input;
	snd_input_buffer_t *buffer;
	assert(inputp);
	buffer = calloc(1, sizeof(*buffer));
	if (!buffer)
		return -ENOMEM;
	input = calloc(1, sizeof(*input));
	if (!input) {
		free(buffer);
		return -ENOMEM;
	}
	if (size < 0)
		size = strlen(buf);
	buffer->buf = malloc((size_t)size + 1);
	if (!buffer->buf) {
		free(input);
		free(buffer);
		return -ENOMEM;
	}
	memcpy(buffer->buf, buf, (size_t) size);
	buffer->buf[size] = 0;
	buffer->ptr = buffer->buf;
	buffer->size = size;
	input->type = SND_INPUT_BUFFER;
	input->ops = &snd_input_buffer_ops;
	input->private_data = buffer;
	*inputp = input;
	return 0;
}
	
