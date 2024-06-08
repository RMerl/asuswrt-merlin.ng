/*
 * Copyright (c) 2019-2024 Free Software Foundation, Inc.
 *
 * This file is part of GNU Wget.
 *
 * GNU Wget is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNU Wget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Wget.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <sys/types.h>
#include <stdint.h> // uint8_t
#include <stdio.h>  // fmemopen
#include <string.h>  // strncmp
#include <stdlib.h>  // free
#include <unistd.h>  // close
#include <fcntl.h>  // open flags
#include <unistd.h>  // close
#include <unistd.h>  // close

#include "wget.h"
#include "connect.h"
#undef fopen_wgetrc

#ifdef __cplusplus
  extern "C" {
#endif
  #include "retr.h"

  // declarations for wget internal functions
  int main_wget(int argc, const char **argv);
  void cleanup(void);
//  FILE *fopen_wget(const char *pathname, const char *mode);
//  FILE *fopen_wgetrc(const char *pathname, const char *mode);
  void exit_wget(int status);
#ifdef __cplusplus
  }
#endif

#include "fuzzer.h"

FILE *fopen_wget(const char *pathname, const char *mode)
{
	return fopen("/dev/null", mode);
}

FILE *fopen_wgetrc(const char *pathname, const char *mode)
{
	return NULL;
}

#ifdef FUZZING
void exit_wget(int status)
{
}
#endif

static const uint8_t *g_data;
static size_t g_size, g_read;

struct my_context {
	int peeklen;
	char peekbuf[512];
};

static int my_peek (int fd _GL_UNUSED, char *buf, int bufsize, void *arg, double d)
{
	(void) d;
	if (g_read < g_size) {
		struct my_context *ctx = (struct my_context *) arg;
		int n = rand() % (g_size - g_read);
		if (n > bufsize)
			n = bufsize;
		if (n > (int) sizeof(ctx->peekbuf))
			n = sizeof(ctx->peekbuf);
		memcpy(buf, g_data + g_read, n);
		memcpy(ctx->peekbuf, g_data + g_read, n);
		g_read += n;
		ctx->peeklen=n;
		return n;
	}
	return 0;
}
static int my_read (int fd _GL_UNUSED, char *buf, int bufsize, void *arg, double d)
{
	(void) d;
	struct my_context *ctx = (struct my_context *) arg;

	if (ctx->peeklen) {
      /* If we have any peek data, simply return that. */
      int copysize = MIN (bufsize, ctx->peeklen);
      memcpy (buf, ctx->peekbuf, copysize);
      ctx->peeklen -= copysize;
      if (ctx->peeklen)
        memmove (ctx->peekbuf, ctx->peekbuf + copysize, ctx->peeklen);

      return copysize;
	}

	if (g_read < g_size) {
		int n = rand() % (g_size - g_read);
		if (n > bufsize)
			n = bufsize;
		memcpy(buf, g_data + g_read, n);
		g_read += n;
		return n;
	}

	return 0;
}
static int my_write (int fd _GL_UNUSED, char *buf _GL_UNUSED, int bufsize, void *arg _GL_UNUSED)
{
	return bufsize;
}
static int my_poll (int fd _GL_UNUSED, double timeout _GL_UNUSED, int wait_for _GL_UNUSED, void *arg)
{
	struct my_context *ctx = (struct my_context *) arg;

   return ctx->peeklen || g_read < g_size;
}
static const char *my_errstr (int fd _GL_UNUSED, void *arg _GL_UNUSED)
{
	return "Success";
}
static void my_close (int fd _GL_UNUSED, void *arg _GL_UNUSED)
{
}

static struct transport_implementation my_transport =
{
  my_read, my_write, my_poll,
  my_peek, my_errstr, my_close
};

/* copied from wget's http.c */
static const char *
response_head_terminator (const char *start, const char *peeked, int peeklen)
{
  const char *p, *end;

  /* If at first peek, verify whether HUNK starts with "HTTP".  If
     not, this is a HTTP/0.9 request and we must bail out without
     reading anything.  */
  if (start == peeked && 0 != memcmp (start, "HTTP", MIN (peeklen, 4)))
    return start;

  /* Look for "\n[\r]\n", and return the following position if found.
     Start two chars before the current to cover the possibility that
     part of the terminator (e.g. "\n\r") arrived in the previous
     batch.  */
  p = peeked - start < 2 ? start : peeked - 2;
  end = peeked + peeklen;

  /* Check for \n\r\n or \n\n anywhere in [p, end-2). */
  for (; p < end - 2; p++)
    if (*p == '\n')
      {
        if (p[1] == '\r' && p[2] == '\n')
          return p + 3;
        else if (p[1] == '\n')
          return p + 2;
      }
  /* p==end-2: check for \n\n directly preceding END. */
  if (peeklen >= 2 && p[0] == '\n' && p[1] == '\n')
    return p + 2;

  return NULL;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	char *hunk;

	if (size > 4096) // same as max_len = ... in .options file
		return 0;

//	CLOSE_STDERR

	g_data = data;
	g_size = size;
	g_read = 0;

	struct my_context *ctx = (struct my_context *) calloc(1, sizeof(struct my_context));
	fd_register_transport(99, &my_transport, ctx);

	while ((hunk = fd_read_hunk(99, response_head_terminator, 512, 65536)))
		free(hunk);

   connect_cleanup();
	free(ctx);

//	RESTORE_STDERR

	return 0;
}
