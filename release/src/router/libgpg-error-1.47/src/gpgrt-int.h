/* gpgrt-int.h - Internal definitions
 * Copyright (C) 2014, 2017 g10 Code GmbH
 *
 * This file is part of libgpg-error.
 *
 * libgpg-error is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * libgpg-error is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef _GPGRT_GPGRT_INT_H
#define _GPGRT_GPGRT_INT_H

#include "gpg-error.h"
#include "visibility.h"
#include "protos.h"

/*
 * Internal i18n macros.
 */
#ifdef ENABLE_NLS
# ifdef HAVE_W32_SYSTEM
#  include "gettext.h"
# else
#  include <libintl.h>
# endif
# define _(a) gettext (a)
# ifdef gettext_noop
#  define N_(a) gettext_noop (a)
# else
#  define N_(a) (a)
# endif
#else  /*!ENABLE_NLS*/
# define _(a) (a)
# define N_(a) (a)
#endif /*!ENABLE_NLS */


/*
 * Hacks mainly required for Slowaris.
 */
#ifdef _GPGRT_NEED_AFLOCAL
# ifndef HAVE_W32_SYSTEM
#  include <sys/socket.h>
#  include <sys/un.h>
# else
#  ifdef HAVE_WINSOCK2_H
#   include <winsock2.h>
#  endif
# include <windows.h>
# endif

# ifndef PF_LOCAL
#  ifdef PF_UNIX
#   define PF_LOCAL PF_UNIX
#  else
#   define PF_LOCAL AF_UNIX
#  endif
# endif /*PF_LOCAL*/
# ifndef AF_LOCAL
#  define AF_LOCAL AF_UNIX
# endif /*AF_UNIX*/

/* We used to avoid this macro in GnuPG and inlined the AF_LOCAL name
 * length computation directly with the little twist of adding 1 extra
 * byte.  It seems that this was needed once on an old HP/UX box and
 * there are also rumours that 4.3 Reno and DEC systems need it.  This
 * one-off buglet did not harm any current system until it came to Mac
 * OS X where the kernel (as of May 2009) exhibited a strange bug: The
 * systems basically froze in the connect call if the passed name
 * contained an invalid directory part.  Ignore the old Unices.  */
# ifndef SUN_LEN
#  define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) \
                        + strlen ((ptr)->sun_path))
# endif /*SUN_LEN*/
#endif /*_GPGRT_NEED_AFLOCAL*/


/*
 * Common helper macros.
 */
#ifndef DIM
# define DIM(array) (sizeof (array) / sizeof (*array))
#endif



/*
 * Local error function prototypes.
 */
const char *_gpg_strerror (gpg_error_t err);
int _gpg_strerror_r (gpg_error_t err, char *buf, size_t buflen);
const char *_gpg_strsource (gpg_error_t err);
gpg_err_code_t _gpg_err_code_from_errno (int err);
int _gpg_err_code_to_errno (gpg_err_code_t code);
gpg_err_code_t _gpg_err_code_from_syserror (void);
void _gpg_err_set_errno (int err);

gpg_error_t _gpg_err_init (void);
void _gpg_err_deinit (int mode);

void _gpgrt_add_emergency_cleanup (void (*f)(void));
void _gpgrt_abort (void) GPGRT_ATTR_NORETURN;

void _gpgrt_set_alloc_func (void *(*f)(void *a, size_t n));

void *_gpgrt_realloc (void *a, size_t n);
void *_gpgrt_reallocarray (void *a, size_t oldnmemb, size_t nmemb, size_t size);
void *_gpgrt_malloc (size_t n);
void *_gpgrt_calloc (size_t n, size_t m);
char *_gpgrt_strdup (const char *string);
char *_gpgrt_strconcat (const char *s1, ...) GPGRT_ATTR_SENTINEL(0);
void _gpgrt_free (void *a);
/* The next is only to be used by visibility.c.  */
char *_gpgrt_strconcat_core (const char *s1, va_list arg_ptr);

#define xfree(a)         _gpgrt_free ((a))
#define xtrymalloc(a)    _gpgrt_malloc ((a))
#define xtrycalloc(a,b)  _gpgrt_calloc ((a),(b))
#define xtryrealloc(a,b) _gpgrt_realloc ((a),(b))
#define xtryreallocarray(a,b,c,d) _gpgrt_reallocarray ((a),(b),(c),(d))
#define xtrystrdup(a)    _gpgrt_strdup ((a))

void _gpgrt_pre_syscall (void);
void _gpgrt_post_syscall (void);

const char *_gpg_error_check_version (const char *req_version);

gpg_err_code_t _gpgrt_lock_init (gpgrt_lock_t *lockhd);
gpg_err_code_t _gpgrt_lock_lock (gpgrt_lock_t *lockhd);
gpg_err_code_t _gpgrt_lock_trylock (gpgrt_lock_t *lockhd);
gpg_err_code_t _gpgrt_lock_unlock (gpgrt_lock_t *lockhd);
gpg_err_code_t _gpgrt_lock_destroy (gpgrt_lock_t *lockhd);
gpg_err_code_t _gpgrt_yield (void);



/*
 * Tracing
 */

/* The trace macro is used this way:
 *   trace (("enter - foo=%d bar=%s", foo, bar));
 * Note the double parenthesis, they are important.
 * To append the current errno to the output, use
 *   trace_errno (EXTPR,("leave - baz=%d", faz));
 * If EXPR evaluates to true the output of strerror (errno)
 * is appended to the output.  Note that the trace function does
 * not modify ERRNO.  To enable tracing you need to have this
 *  #define ENABLE_TRACING "modulename"
 * before you include gpgrt-int.h.
 */
#ifdef ENABLE_TRACING
# define trace(X) do { \
                       _gpgrt_internal_trace_begin \
                         (ENABLE_TRACING, __func__, __LINE__, 0); \
                       _gpgrt_internal_trace X; \
                       _gpgrt_internal_trace_end (); \
                     } while (0)
# define trace_errno(C,X) do {                     \
                       _gpgrt_internal_trace_begin \
                         (ENABLE_TRACING, __func__, __LINE__, (C)); \
                       _gpgrt_internal_trace X;      \
                       _gpgrt_internal_trace_end (); \
                     } while (0)
# define trace_start(X) do { \
                       _gpgrt_internal_trace_begin \
                         (ENABLE_TRACING, __func__, __LINE__, 0); \
                       _gpgrt_internal_trace_printf X; \
                     } while (0)
# define trace_append(X) do { \
                       _gpgrt_internal_trace_printf X; \
                     } while (0)
# define trace_finish(X) do { \
                       _gpgrt_internal_trace_printf X; \
                       _gpgrt_internal_trace_end (); \
                     } while (0)
#else
# define trace(X) do { } while (0)
# define trace_errno(C,X) do { } while (0)
# define trace_start(X) do { } while (0)
# define trace_append(X) do { } while (0)
# define trace_finish(X) do { } while (0)
#endif /*!ENABLE_TRACING*/

void _gpgrt_internal_trace_begin (const char *mod, const char *file, int line,
                                  int with_errno);
void _gpgrt_internal_trace (const char *format,
                            ...) GPGRT_ATTR_PRINTF(1,2);
void _gpgrt_internal_trace_printf (const char *format,
                                   ...) GPGRT_ATTR_PRINTF(1,2);
void _gpgrt_internal_trace_end (void);



/*
 * Local definitions for estream.
 */

#if HAVE_W32_SYSTEM
# ifndef  O_NONBLOCK
#  define O_NONBLOCK  0x40000000	/* FIXME: Is that safe?  */
# endif
#endif

/*
 * A private cookie function to implement an internal IOCTL service.
 */
typedef int (*cookie_ioctl_function_t) (void *cookie, int cmd,
					void *ptr, size_t *len);
#define COOKIE_IOCTL_SNATCH_BUFFER 1
#define COOKIE_IOCTL_NONBLOCK      2
#define COOKIE_IOCTL_TRUNCATE      3

/* An internal variant of gpgrt_cookie_close_function_t with a slot
 * for the ioctl function.  */
struct cookie_io_functions_s
{
  struct _gpgrt_cookie_io_functions public;
  cookie_ioctl_function_t func_ioctl;
};

typedef enum
  {
    BACKEND_MEM,
    BACKEND_FD,
    BACKEND_SOCK,
    BACKEND_W32,
    BACKEND_FP,
    BACKEND_USER,
    BACKEND_W32_POLLABLE
  } gpgrt_stream_backend_kind_t;


/*
 * A type to hold notification functions.
 */
struct notify_list_s
{
  struct notify_list_s *next;
  void (*fnc) (estream_t, void*); /* The notification function.  */
  void *fnc_value;                /* The value to be passed to FNC.  */
};
typedef struct notify_list_s *notify_list_t;


/*
 * Buffer management layer.
 */

/* BUFSIZ on Windows is 512 but on current Linux it is 8k.  We better
 * use the 8k for Windows as well.  */
#ifdef HAVE_W32_SYSTEM
# define BUFFER_BLOCK_SIZE  8192
#else
# define BUFFER_BLOCK_SIZE  BUFSIZ
#endif
#define BUFFER_UNREAD_SIZE 16


/*
 * The private object describing a stream.
 */
struct _gpgrt_stream_internal
{
  unsigned char buffer[BUFFER_BLOCK_SIZE];
  unsigned char unread_buffer[BUFFER_UNREAD_SIZE];

  gpgrt_lock_t lock;		 /* Lock.  Used by *_stream_lock(). */

  gpgrt_stream_backend_kind_t kind;
  void *cookie;			 /* Cookie.                */
  void *opaque;			 /* Opaque data.           */
  unsigned int modeflags;	 /* Flags for the backend. */
  char *printable_fname;         /* Malloced filename for es_fname_get.  */
  gpgrt_off_t offset;
  gpgrt_cookie_read_function_t  func_read;
  gpgrt_cookie_write_function_t func_write;
  gpgrt_cookie_seek_function_t  func_seek;
  gpgrt_cookie_close_function_t func_close;
  cookie_ioctl_function_t func_ioctl;
  int strategy;
  es_syshd_t syshd;              /* A copy of the system handle.  */
  struct
  {
    unsigned int err: 1;
    unsigned int eof: 1;
    unsigned int hup: 1;
  } indicators;
  unsigned int deallocate_buffer: 1;
  unsigned int is_stdstream:1;   /* This is a standard stream.  */
  unsigned int stdstream_fd:2;   /* 0, 1 or 2 for a standard stream.  */
  unsigned int printable_fname_inuse: 1;  /* es_fname_get has been used.  */
  unsigned int samethread: 1;    /* The "samethread" mode keyword.  */
  size_t print_ntotal;           /* Bytes written from in print_writer. */
  notify_list_t onclose;         /* On close notify function list.  */
};
typedef struct _gpgrt_stream_internal *estream_internal_t;


/*
 * Local prototypes for estream.
 */
int _gpgrt_estream_init (void);
void _gpgrt_set_syscall_clamp (void (*pre)(void), void (*post)(void));
void _gpgrt_get_syscall_clamp (void (**r_pre)(void), void (**r_post)(void));

gpgrt_stream_t _gpgrt_fopen (const char *_GPGRT__RESTRICT path,
                             const char *_GPGRT__RESTRICT mode);
gpgrt_stream_t _gpgrt_mopen (void *_GPGRT__RESTRICT data,
                             size_t data_n, size_t data_len,
                             unsigned int grow,
                             void *(*func_realloc) (void *mem, size_t size),
                             void (*func_free) (void *mem),
                             const char *_GPGRT__RESTRICT mode);
gpgrt_stream_t _gpgrt_fopenmem (size_t memlimit,
                                const char *_GPGRT__RESTRICT mode);
gpgrt_stream_t _gpgrt_fopenmem_init (size_t memlimit,
                                     const char *_GPGRT__RESTRICT mode,
                                     const void *data, size_t datalen);
gpgrt_stream_t _gpgrt_fdopen    (int filedes, const char *mode);
gpgrt_stream_t _gpgrt_fdopen_nc (int filedes, const char *mode);
gpgrt_stream_t _gpgrt_sysopen    (gpgrt_syshd_t *syshd, const char *mode);
gpgrt_stream_t _gpgrt_sysopen_nc (gpgrt_syshd_t *syshd, const char *mode);
gpgrt_stream_t _gpgrt_fpopen    (FILE *fp, const char *mode);
gpgrt_stream_t _gpgrt_fpopen_nc (FILE *fp, const char *mode);
gpgrt_stream_t _gpgrt_freopen (const char *_GPGRT__RESTRICT path,
                               const char *_GPGRT__RESTRICT mode,
                               gpgrt_stream_t _GPGRT__RESTRICT stream);
gpgrt_stream_t _gpgrt_fopencookie (void *_GPGRT__RESTRICT cookie,
                                   const char *_GPGRT__RESTRICT mode,
                                   gpgrt_cookie_io_functions_t functions);
int _gpgrt_fclose (gpgrt_stream_t stream);
int _gpgrt_fcancel (gpgrt_stream_t stream);
int _gpgrt_fclose_snatch (gpgrt_stream_t stream,
                          void **r_buffer, size_t *r_buflen);
int _gpgrt_onclose (gpgrt_stream_t stream, int mode,
                    void (*fnc) (gpgrt_stream_t, void*), void *fnc_value);
int _gpgrt_fileno (gpgrt_stream_t stream);
int _gpgrt_fileno_unlocked (gpgrt_stream_t stream);
int _gpgrt_syshd (gpgrt_stream_t stream, gpgrt_syshd_t *syshd);
int _gpgrt_syshd_unlocked (gpgrt_stream_t stream, gpgrt_syshd_t *syshd);

void _gpgrt__set_std_fd (int no, int fd);
gpgrt_stream_t _gpgrt__get_std_stream (int fd);
/* The es_stderr et al macros are pretty common so that we want to use
 * them too.  This requires that we redefine them.  */
#undef es_stdin
#define es_stdin  _gpgrt__get_std_stream (0)
#undef es_stdout
#define es_stdout _gpgrt__get_std_stream (1)
#undef es_stderr
#define es_stderr _gpgrt__get_std_stream (2)

void _gpgrt_flockfile (gpgrt_stream_t stream);
int  _gpgrt_ftrylockfile (gpgrt_stream_t stream);
void _gpgrt_funlockfile (gpgrt_stream_t stream);

int _gpgrt_feof (gpgrt_stream_t stream);
int _gpgrt_feof_unlocked (gpgrt_stream_t stream);
int _gpgrt_ferror (gpgrt_stream_t stream);
int _gpgrt_ferror_unlocked (gpgrt_stream_t stream);
void _gpgrt_clearerr (gpgrt_stream_t stream);
void _gpgrt_clearerr_unlocked (gpgrt_stream_t stream);
int _gpgrt__pending (gpgrt_stream_t stream);
int _gpgrt__pending_unlocked (gpgrt_stream_t stream);

int _gpgrt_fflush (gpgrt_stream_t stream);
int _gpgrt_fseek (gpgrt_stream_t stream, long int offset, int whence);
int _gpgrt_fseeko (gpgrt_stream_t stream, gpgrt_off_t offset, int whence);
long int _gpgrt_ftell (gpgrt_stream_t stream);
gpgrt_off_t _gpgrt_ftello (gpgrt_stream_t stream);
void _gpgrt_rewind (gpgrt_stream_t stream);
int  _gpgrt_ftruncate (estream_t stream, gpgrt_off_t length);

int _gpgrt_fgetc (gpgrt_stream_t stream);
int _gpgrt_fputc (int c, gpgrt_stream_t stream);

int _gpgrt__getc_underflow (gpgrt_stream_t stream);
int _gpgrt__putc_overflow (int c, gpgrt_stream_t stream);

/* Note: Keeps the next two macros in sync
         with their counterparts in gpg-error.h. */
#define _gpgrt_getc_unlocked(stream)				\
  (((!(stream)->flags.writing)					\
    && ((stream)->data_offset < (stream)->data_len)		\
    && (! (stream)->unread_data_len))				\
  ? ((int) (stream)->buffer[((stream)->data_offset)++])		\
  : _gpgrt__getc_underflow ((stream)))

#define _gpgrt_putc_unlocked(c, stream)				\
  (((stream)->flags.writing					\
    && ((stream)->data_offset < (stream)->buffer_size)		\
    && (c != '\n'))						\
  ? ((int) ((stream)->buffer[((stream)->data_offset)++] = (c)))	\
  : _gpgrt__putc_overflow ((c), (stream)))

int _gpgrt_ungetc (int c, gpgrt_stream_t stream);

int _gpgrt_read (gpgrt_stream_t _GPGRT__RESTRICT stream,
                 void *_GPGRT__RESTRICT buffer, size_t bytes_to_read,
                 size_t *_GPGRT__RESTRICT bytes_read);
int _gpgrt_write (gpgrt_stream_t _GPGRT__RESTRICT stream,
                  const void *_GPGRT__RESTRICT buffer, size_t bytes_to_write,
                  size_t *_GPGRT__RESTRICT bytes_written);
int _gpgrt_write_sanitized (gpgrt_stream_t _GPGRT__RESTRICT stream,
                            const void *_GPGRT__RESTRICT buffer, size_t length,
                            const char *delimiters,
                            size_t *_GPGRT__RESTRICT bytes_written);
int _gpgrt_write_hexstring (gpgrt_stream_t _GPGRT__RESTRICT stream,
                            const void *_GPGRT__RESTRICT buffer, size_t length,
                            int reserved,
                            size_t *_GPGRT__RESTRICT bytes_written);

size_t _gpgrt_fread (void *_GPGRT__RESTRICT ptr, size_t size, size_t nitems,
                     gpgrt_stream_t _GPGRT__RESTRICT stream);
size_t _gpgrt_fwrite (const void *_GPGRT__RESTRICT ptr,
                      size_t size, size_t memb,
                      gpgrt_stream_t _GPGRT__RESTRICT stream);

char *_gpgrt_fgets (char *_GPGRT__RESTRICT s, int n,
                    gpgrt_stream_t _GPGRT__RESTRICT stream);
int _gpgrt_fputs (const char *_GPGRT__RESTRICT s,
                  gpgrt_stream_t _GPGRT__RESTRICT stream);
int _gpgrt_fputs_unlocked (const char *_GPGRT__RESTRICT s,
                           gpgrt_stream_t _GPGRT__RESTRICT stream);

gpgrt_ssize_t _gpgrt_getline (char *_GPGRT__RESTRICT *_GPGRT__RESTRICT lineptr,
                              size_t *_GPGRT__RESTRICT n,
                              gpgrt_stream_t stream);
gpgrt_ssize_t _gpgrt_read_line (gpgrt_stream_t stream,
                                char **addr_of_buffer, size_t *length_of_buffer,
                                size_t *max_length);

int _gpgrt_fprintf (gpgrt_stream_t _GPGRT__RESTRICT stream,
                    const char *_GPGRT__RESTRICT format, ...)
                    GPGRT_ATTR_PRINTF(2,3);
int _gpgrt_fprintf_unlocked (gpgrt_stream_t _GPGRT__RESTRICT stream,
                             const char *_GPGRT__RESTRICT format, ...)
                             GPGRT_ATTR_PRINTF(2,3);

int _gpgrt_vfprintf (gpgrt_stream_t _GPGRT__RESTRICT stream,
                     gpgrt_string_filter_t sf, void *sfvalue,
                     const char *_GPGRT__RESTRICT format, va_list ap)
                     GPGRT_ATTR_PRINTF(4,0);
int _gpgrt_vfprintf_unlocked (gpgrt_stream_t _GPGRT__RESTRICT stream,
                              gpgrt_string_filter_t sf, void *sfvalue,
                              const char *_GPGRT__RESTRICT format, va_list ap)
                              GPGRT_ATTR_PRINTF(4,0);

int _gpgrt_setvbuf (gpgrt_stream_t _GPGRT__RESTRICT stream,
                    char *_GPGRT__RESTRICT buf, int mode, size_t size);

void _gpgrt_set_binary (gpgrt_stream_t stream);
int  _gpgrt_set_nonblock (gpgrt_stream_t stream, int onoff);
int  _gpgrt_get_nonblock (gpgrt_stream_t stream);

int _gpgrt_poll (gpgrt_poll_t *fds, unsigned int nfds, int timeout);

gpgrt_stream_t _gpgrt_tmpfile (void);

void _gpgrt_opaque_set (gpgrt_stream_t _GPGRT__RESTRICT stream,
                        void *_GPGRT__RESTRICT opaque);
void *_gpgrt_opaque_get (gpgrt_stream_t stream);

void _gpgrt_fname_set (gpgrt_stream_t stream, const char *fname);
const char *_gpgrt_fname_get (gpgrt_stream_t stream);

#include "estream-printf.h"

/* Make sure we always use our snprintf */
#undef snprintf
#define snprintf _gpgrt_estream_snprintf


#if HAVE_W32_SYSTEM
/* Prototypes for w32-estream.c. */
extern struct cookie_io_functions_s _gpgrt_functions_w32_pollable;
int _gpgrt_w32_pollable_create (void *_GPGRT__RESTRICT *_GPGRT__RESTRICT cookie,
                                unsigned int modeflags,
                                struct cookie_io_functions_s next_functions,
                                void *next_cookie);
int _gpgrt_w32_poll (gpgrt_poll_t *fds, size_t nfds, int timeout);
#endif /*HAVE_W32_SYSTEM*/



/*
 * Local prototypes for the encoders.
 */

struct _gpgrt_b64state
{
  int idx;
  int quad_count;
  estream_t stream;
  char *title;
  unsigned char radbuf[4];
  unsigned int crc;
  gpg_err_code_t lasterr;
  unsigned int flags;
  unsigned int stop_seen:1;
  unsigned int invalid_encoding:1;
  unsigned int using_decoder:1;
};

gpgrt_b64state_t _gpgrt_b64enc_start (estream_t stream, const char *title);
gpg_err_code_t   _gpgrt_b64enc_write (gpgrt_b64state_t state,
                                      const void *buffer, size_t nbytes);
gpg_err_code_t   _gpgrt_b64enc_finish (gpgrt_b64state_t state);

gpgrt_b64state_t _gpgrt_b64dec_start (const char *title);
gpg_err_code_t _gpgrt_b64dec_proc (gpgrt_b64state_t state, void *buffer,
                                   size_t length, size_t *r_nbytes);
gpg_err_code_t _gpgrt_b64dec_finish (gpgrt_b64state_t state);



/*
 * Local prototypes for logging
 */
int  _gpgrt_get_errorcount (int clear);
void _gpgrt_inc_errorcount (void);
void _gpgrt_log_set_sink (const char *name, estream_t stream, int fd);
void _gpgrt_log_set_socket_dir_cb (const char *(*fnc)(void));
void _gpgrt_log_set_pid_suffix_cb (int (*cb)(unsigned long *r_value));
void _gpgrt_log_set_prefix (const char *text, unsigned int flags);
const char *_gpgrt_log_get_prefix (unsigned int *flags);
int  _gpgrt_log_test_fd (int fd);
int  _gpgrt_log_get_fd (void);
estream_t _gpgrt_log_get_stream (void);

void _gpgrt_log (int level, const char *fmt, ...) GPGRT_ATTR_PRINTF(2,3);
void _gpgrt_logv (int level, const char *fmt, va_list arg_ptr);
void _gpgrt_logv_prefix (int level, const char *prefix,
                         const char *fmt, va_list arg_ptr);

void _gpgrt_log_string (int level, const char *string);

void _gpgrt_log_bug (const char *fmt, ...)    GPGRT_ATTR_NR_PRINTF(1,2);
void _gpgrt_log_fatal (const char *fmt, ...)  GPGRT_ATTR_NR_PRINTF(1,2);
void _gpgrt_log_error (const char *fmt, ...)  GPGRT_ATTR_PRINTF(1,2);
void _gpgrt_log_info (const char *fmt, ...)   GPGRT_ATTR_PRINTF(1,2);
void _gpgrt_log_debug (const char *fmt, ...)  GPGRT_ATTR_PRINTF(1,2);
void _gpgrt_log_debug_string (const char *string, const char *fmt,
                              ...) GPGRT_ATTR_PRINTF(2,3);

void _gpgrt_log_printf (const char *fmt, ...) GPGRT_ATTR_PRINTF(1,2);

void _gpgrt_log_flush (void);

void _gpgrt_logv_printhex (const void *buffer, size_t length,
                           const char *fmt, va_list arg_ptr);
void _gpgrt_log_printhex (const void *buffer, size_t length,
                          const char *fmt, ...) GPGRT_ATTR_PRINTF(3,4);

void _gpgrt_logv_clock (const char *fmt, va_list arg_ptr);
void _gpgrt_log_clock (const char *fmt, ...) GPGRT_ATTR_PRINTF(1,2);

void _gpgrt__log_assert (const char *expr, const char *file, int line,
                         const char *func) GPGRT_ATTR_NORETURN;

/* Redefine the assert macro to use our internal function.  */
#undef gpgrt_assert
#ifdef GPGRT_HAVE_MACRO_FUNCTION
#define gpgrt_assert(expr)                                      \
  ((expr)                                                       \
   ? (void) 0                                                   \
   : _gpgrt__log_assert (#expr, __FILE__, __LINE__, __FUNCTION__))
#else /*!GPGRT_HAVE_MACRO_FUNCTION*/
/* # define BUG() bug_at( __FILE__ , __LINE__ ) */
#define gpgrt_assert(expr)                                      \
  ((expr)                                                       \
   ? (void) 0                                                   \
   : _gpgrt__log_assert (#expr, __FILE__, __LINE__, NULL))
#endif /*!GPGRT_HAVE_MACRO_FUNCTION*/

/* Note: The next function is only to be used by visibility.c.  */
int _gpgrt_logv_internal (int level, int ignore_arg_ptr,
                          const char *extrastring,
                          const char *prefmt, const char *fmt,
                          va_list arg_ptr);


/*
 * Local prototypes for the spawn functions.
 *
 * We put the docs here because we have separate implementations in
 * the files spawn-posix.c and spawn-w32.c
 */

/* Return the maximum number of currently allowed file descriptors.
 * Only useful on POSIX systems.  */
/* int get_max_fds (void); */


/* Close all file descriptors starting with descriptor FIRST.  If
 * EXCEPT is not NULL, it is expected to be a list of file descriptors
 * which are not to close.  This list shall be sorted in ascending
 * order with its end marked by -1.  */
/* void close_all_fds (int first, int *except); */


/* Returns an array with all currently open file descriptors.  The end
 * of the array is marked by -1.  The caller needs to release this
 * array using the *standard free* and not with xfree.  This allow the
 * use of this function right at startup even before libgcrypt has
 * been initialized.  Returns NULL on error and sets ERRNO accordingly.  */
/* int *get_all_open_fds (void); */

/* Create a pipe.  The DIRECTION parameter gives the type of the created pipe:
 *   DIRECTION < 0 := Inbound pipe: On Windows the write end is inheritable.
 *   DIRECTION > 0 := Outbound pipe: On Windows the read end is inheritable.
 * If R_FP is NULL a standard pipe and no stream is created, DIRECTION
 * should then be 0.   */
gpg_err_code_t _gpgrt_make_pipe (int filedes[2], estream_t *r_fp,
                                 int direction, int nonblock);

/* Convenience macros to create a pipe.  */
#define _gpgrt_create_pipe(a)              _gpgrt_make_pipe ((a),NULL, 0,    0)
#define _gpgrt_create_inbound_pipe(a,b,c)  _gpgrt_make_pipe ((a), (b), -1, (c))
#define _gpgrt_create_outbound_pipe(a,b,c) _gpgrt_make_pipe ((a), (b),  1, (c))


/* Fork and exec the program PGMNAME.
 *
 * If R_INFP is NULL connect stdin of the new process to /dev/null; if
 * it is not NULL store the address of a pointer to a new estream
 * there. If R_OUTFP is NULL connect stdout of the new process to
 * /dev/null; if it is not NULL store the address of a pointer to a
 * new estream there.  If R_ERRFP is NULL connect stderr of the new
 * process to /dev/null; if it is not NULL store the address of a
 * pointer to a new estream there.  On success the process id of the
 * new process is stored at R_PID.  On error -1 is stored at R_PID and
 * if R_OUTFP or R_ERRFP are not NULL, NULL is stored there.
 *
 * The arguments for the process are expected in the NULL terminated
 * array ARGV.  The program name itself should not be included there.
 *
 * IF EXCEPT is not NULL, it is expected to be an ordered list of file
 * descriptors, terminated by an entry with the value (-1).  These
 * file descriptors won't be closed before spawning a new program.
 *
 * Returns 0 on success or an error code.  Calling gpgrt_wait_process
 * and gpgrt_release_process is required if the function succeeded.
 *
 * FLAGS is a bit vector:
 *
 * GPGRT_SPAWN_NONBLOCK
 *        If set the two output streams are created in non-blocking
 *        mode and the input stream is switched to non-blocking mode.
 *        This is merely a convenience feature because the caller
 *        could do the same with gpgrt_set_nonblock.  Does not yet
 *        work for Windows.
 *
 * GPGRT_SPAWN_DETACHED
 *        If set the process will be started as a background process.
 *        This flag is only useful under W32 systems, so that no new
 *        console is created and pops up a console window when starting
 *        the server.
 *
 * GPGRT_SPAWN_RUN_ASFW
 *        On W32 run AllowSetForegroundWindow for the child.  Note that
 *        due to unknown problems this actually allows
 *        SetForegroundWindow for all children of this process.
 *
 * GNUPG_SPAWN_KEEP_STDIN
 * GNUPG_SPAWN_KEEP_STDOUT
 * GNUPG_SPAWN_KEEP_STDERR
 *        Do not assign /dev/null to a non-required standard file
 *        descriptor.
 *
 */
gpg_err_code_t
_gpgrt_spawn_process (const char *pgmname, const char *argv[],
                      int *execpt, unsigned int flags,
                      estream_t *r_infp,
                      estream_t *r_outfp,
                      estream_t *r_errfp,
                      gpgrt_process_t *r_process_id);


/* Variant of gpgrt_spawn_process.  This function forks and then execs
 * PGMNAME, while connecting INFD to stdin, OUTFD to stdout and ERRFD
 * to stderr (any of them may be -1 to connect them to /dev/null).
 * The arguments for the process are expected in the NULL terminated
 * array ARGV.  The program name itself should not be included there.
 * Calling gpgrt_wait_process and gpgrt_release_process is required.
 * Returns 0 on success or an error code.  If SPAWN_CB is not NULL,
 * the given function will be called with SPAWN_CB_ARG to determine if
 * file descriptors/handles should be inherited or not.  The callback
 * function should return 1 to ask keeping file descriptors/handles.
 * If SPAWN_CB is NULL, or it returns 0, all file descriptors (except
 * INFD, OUTFD, and ERRFD) will be closed on POSIX machine.  On POSIX
 * machine, it is called right after the fork, by child process.
 */
gpg_err_code_t _gpgrt_spawn_process_fd (const char *pgmname,
                                        const char *argv[],
                                        int infd, int outfd, int errfd,
                                        int (*spawn_cb) (void *),
                                        void *spawn_cb_arg,
                                        gpgrt_process_t *r_process_id);

/* Spawn a new process and immediately detach from it.  The name of
 * the program to exec is PGMNAME and its arguments are in ARGV (the
 * programname is automatically passed as first argument).
 * Environment strings in ENVP are set.  An error is returned if
 * pgmname is not executable; to make this work it is necessary to
 * provide an absolute file name.  */
gpg_err_code_t _gpgrt_spawn_process_detached (const char *pgmname,
                                              const char *argv[],
                                              const char *envp[]);

/* If HANG is true, waits for the process identified by PROCESS_ID to
 * exit; if HANG is false, checks whether the process has terminated.
 * PGMNAME should be the same as supplied to the spawn function and is
 * only used for diagnostics.  Return values:
 *
 * 0
 *     The process exited successful.  0 is stored at R_EXITCODE.
 *
 * GPG_ERR_GENERAL
 *     The process exited without success.  The exit code of process
 *     is then stored at R_EXITCODE.  An exit code of -1 indicates
 *     that the process terminated abnormally (e.g. due to a signal).
 *
 * GPG_ERR_TIMEOUT
 *     The process is still running (returned only if HANG is false).
 *
 * GPG_ERR_INV_VALUE
 *     An invalid PID has been specified.
 *
 * Other error codes may be returned as well.  Unless otherwise noted,
 * -1 will be stored at R_EXITCODE.  R_EXITCODE may be passed as NULL
 * if the exit code is not required (in that case an error message will
 * be printed).  Note that under Windows PID is not the process id but
 * the handle of the process.  */
gpg_err_code_t _gpgrt_wait_process (const char *pgmname,
                                    gpgrt_process_t process_id, int hang,
                                    int *r_exitcode);

/* Like _gpgrt_wait_process, but for COUNT processes.  */
gpg_err_code_t _gpgrt_wait_processes (const char **pgmnames,
                                      gpgrt_process_t *process_ids,
                                      size_t count, int hang,
                                      int *r_exitcodes);

/* Kill a process; that is send an appropriate signal to the process.
 * gpgrt_wait_process must be called to actually remove the process
 * from the system.  An invalid PROCESS_ID is ignored.  */
void _gpgrt_kill_process (gpgrt_process_t process_id);

/* Release the process identified by PROCESS_ID.  This function is
 * actually only required for Windows but it does not harm to always
 * call it.  It is a nop if PROCESS_ID is invalid.  */
void _gpgrt_release_process (gpgrt_process_t process_id);

/* Close all file resources (descriptors), except KEEP_FDS.  */
void _gpgrt_close_all_fds (int from, int *keep_fds);


/*
 * Local prototypes for argparse.
 */
int _gpgrt_argparse (estream_t fp, gpgrt_argparse_t *arg, gpgrt_opt_t *opts);
int _gpgrt_argparser (gpgrt_argparse_t *arg, gpgrt_opt_t *opts,
                      const char *confname);
void _gpgrt_usage (int level);
const char *_gpgrt_strusage (int level);
void _gpgrt_set_strusage (const char *(*f)(int));
void _gpgrt_set_usage_outfnc (int (*fnc)(int, const char *));
void _gpgrt_set_fixed_string_mapper (const char *(*f)(const char*));
void _gpgrt_set_confdir (int what, const char *name);


/*
 * Various helper functions
 */
int _gpgrt_cmp_version (const char *a, const char *b, int level);



/*
 * Internal platform abstraction functions (sysutils.c)
 */

/* Return true if FD is valid.  */
int _gpgrt_fd_valid_p (int fd);

/* A getenv variant which returns a malloced copy.  */
char *_gpgrt_getenv (const char *name);

/* A setenv variant which can be used for unsetenv by setting VALUE to
 * NULL and OVERRIDE to true.  */
gpg_err_code_t _gpgrt_setenv (const char *name,
                              const char *value, int overwrite);

/* A wrapper around mkdir using a string for the mode (permissions).  */
gpg_err_code_t _gpgrt_mkdir (const char *name, const char *modestr);

/* A simple wrapper around chdir.  */
gpg_err_code_t _gpgrt_chdir (const char *name);

/* Return the current WD as a malloced string.  */
char *_gpgrt_getcwd (void);

/* Wrapper for Windows to allow utf8 file names.  */
gpg_err_code_t _gpgrt_access (const char *fname, int mode);

/* Return the home directory of user NAME.  */
char *_gpgrt_getpwdir (const char *name);

/* Return the account name of the current user.  */
char *_gpgrt_getusername (void);

/* Expand and concat file name parts.  */
char *_gpgrt_vfnameconcat (int want_abs, const char *first_part,
                           va_list arg_ptr);
char *_gpgrt_fnameconcat (const char *first_part,
                          ... ) GPGRT_ATTR_SENTINEL(0);
char *_gpgrt_absfnameconcat (const char *first_part,
                             ... ) GPGRT_ATTR_SENTINEL(0);


/*
 * Platform specific functions (Windows)
 */
#ifdef HAVE_W32_SYSTEM

char *_gpgrt_w32_reg_query_string (const char *root,
                                   const char *dir,
                                   const char *name);
char *_gpgrt_w32_reg_get_string (const char *key);

wchar_t *_gpgrt_fname_to_wchar (const char *fname);


#endif /*HAVE_W32_SYSTEM*/

/*
 * Missing functions implemented inline.
 */

#ifndef HAVE_STPCPY
static GPG_ERR_INLINE char *
_gpgrt_stpcpy (char *a, const char *b)
{
  while (*b)
    *a++ = *b++;
  *a = 0;
  return a;
}
#define stpcpy(a,b) _gpgrt_stpcpy ((a), (b))
#endif /*!HAVE_STPCPY*/


#endif /*_GPGRT_GPGRT_INT_H*/
