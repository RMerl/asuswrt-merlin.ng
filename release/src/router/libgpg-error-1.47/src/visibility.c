/* visibility.c - Wrapper for all public functions.
 * Copyright (C) 2014  g10 Code GmbH
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

#include <config.h>
#include <stdarg.h>
#include <stdlib.h> /* For abort().  */

#define _GPGRT_INCL_BY_VISIBILITY_C 1
#include "gpgrt-int.h"

const char *
gpg_strerror (gpg_error_t err)
{
  return _gpg_strerror (err);
}

int
gpg_strerror_r (gpg_error_t err, char *buf, size_t buflen)
{
  return _gpg_strerror_r (err, buf, buflen);
}

const char *
gpg_strsource (gpg_error_t err)
{
  return _gpg_strsource (err);
}

gpg_err_code_t
gpg_err_code_from_errno (int err)
{
  return _gpg_err_code_from_errno (err);
}

int
gpg_err_code_to_errno (gpg_err_code_t code)
{
  return _gpg_err_code_to_errno (code);
}

gpg_err_code_t
gpg_err_code_from_syserror (void)
{
  return _gpg_err_code_from_syserror ();
}

void
gpg_err_set_errno (int err)
{
  _gpg_err_set_errno (err);
}


gpg_error_t
gpg_err_init (void)
{
  return _gpg_err_init ();
}

void
gpg_err_deinit (int mode)
{
  _gpg_err_deinit (mode);
}

void
gpgrt_add_emergency_cleanup (void (*f)(void))
{
  _gpgrt_add_emergency_cleanup (f);
}

void
gpgrt_abort (void)
{
  _gpgrt_abort ();
}

const char *
gpg_error_check_version (const char *req_version)
{
  return _gpg_error_check_version (req_version);
}

const char *
gpgrt_check_version (const char *req_version)
{
  return _gpg_error_check_version (req_version);
}

void
gpgrt_set_syscall_clamp (void (*pre)(void), void (*post)(void))
{
  _gpgrt_set_syscall_clamp (pre, post);
}

void
gpgrt_get_syscall_clamp (void (**r_pre)(void), void (**r_post)(void))
{
  _gpgrt_get_syscall_clamp (r_pre, r_post);
}

void
gpgrt_set_alloc_func (void *(*f)(void *a, size_t n))
{
  _gpgrt_set_alloc_func (f);
}


gpg_err_code_t
gpgrt_lock_init (gpgrt_lock_t *lockhd)
{
  return _gpgrt_lock_init (lockhd);
}

gpg_err_code_t
gpgrt_lock_lock (gpgrt_lock_t *lockhd)
{
  return _gpgrt_lock_lock (lockhd);
}

gpg_err_code_t
gpgrt_lock_trylock (gpgrt_lock_t *lockhd)
{
  return _gpgrt_lock_trylock (lockhd);
}

gpg_err_code_t
gpgrt_lock_unlock (gpgrt_lock_t *lockhd)
{
  return _gpgrt_lock_unlock (lockhd);
}

gpg_err_code_t
gpgrt_lock_destroy (gpgrt_lock_t *lockhd)
{
  return _gpgrt_lock_destroy (lockhd);
}

gpg_err_code_t
gpgrt_yield (void)
{
  return _gpgrt_yield ();
}



estream_t
gpgrt_fopen (const char *_GPGRT__RESTRICT path,
             const char *_GPGRT__RESTRICT mode)
{
  return _gpgrt_fopen (path, mode);
}

estream_t
gpgrt_mopen (void *_GPGRT__RESTRICT data, size_t data_n, size_t data_len,
             unsigned int grow,
             void *(*func_realloc) (void *mem, size_t size),
             void (*func_free) (void *mem),
             const char *_GPGRT__RESTRICT mode)
{
  return _gpgrt_mopen (data, data_n, data_len, grow, func_realloc, func_free,
                       mode);
}

estream_t
gpgrt_fopenmem (size_t memlimit, const char *_GPGRT__RESTRICT mode)
{
  return _gpgrt_fopenmem (memlimit, mode);
}

estream_t
gpgrt_fopenmem_init (size_t memlimit, const char *_GPGRT__RESTRICT mode,
                     const void *data, size_t datalen)
{
  return _gpgrt_fopenmem_init (memlimit, mode, data, datalen);
}

estream_t
gpgrt_fdopen (int filedes, const char *mode)
{
  return _gpgrt_fdopen (filedes, mode);
}

estream_t
gpgrt_fdopen_nc (int filedes, const char *mode)
{
  return _gpgrt_fdopen_nc (filedes, mode);
}

estream_t
gpgrt_sysopen (es_syshd_t *syshd, const char *mode)
{
  return _gpgrt_sysopen (syshd, mode);
}

estream_t
gpgrt_sysopen_nc (es_syshd_t *syshd, const char *mode)
{
  return _gpgrt_sysopen_nc (syshd, mode);
}

estream_t
gpgrt_fpopen (FILE *fp, const char *mode)
{
  return _gpgrt_fpopen (fp, mode);
}

estream_t
gpgrt_fpopen_nc (FILE *fp, const char *mode)
{
  return _gpgrt_fpopen_nc (fp, mode);
}

estream_t
gpgrt_freopen (const char *_GPGRT__RESTRICT path,
               const char *_GPGRT__RESTRICT mode,
               estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_freopen (path, mode, stream);
}

estream_t
gpgrt_fopencookie (void *_GPGRT__RESTRICT cookie,
                   const char *_GPGRT__RESTRICT mode,
                   gpgrt_cookie_io_functions_t functions)
{
  return _gpgrt_fopencookie (cookie, mode, functions);
}

int
gpgrt_fclose (estream_t stream)
{
  return _gpgrt_fclose (stream);
}

int
gpgrt_fcancel (estream_t stream)
{
  return _gpgrt_fcancel (stream);
}

int
gpgrt_fclose_snatch (estream_t stream, void **r_buffer, size_t *r_buflen)
{
  return _gpgrt_fclose_snatch (stream, r_buffer, r_buflen);
}

int
gpgrt_onclose (estream_t stream, int mode,
               void (*fnc) (estream_t, void*), void *fnc_value)
{
  return _gpgrt_onclose (stream, mode, fnc, fnc_value);
}

int
gpgrt_fileno (estream_t stream)
{
  return _gpgrt_fileno (stream);
}

int
gpgrt_fileno_unlocked (estream_t stream)
{
  return _gpgrt_fileno_unlocked (stream);
}

int
gpgrt_syshd (estream_t stream, es_syshd_t *syshd)
{
  return _gpgrt_syshd (stream, syshd);
}

int
gpgrt_syshd_unlocked (estream_t stream, es_syshd_t *syshd)
{
  return _gpgrt_syshd_unlocked (stream, syshd);
}

void
_gpgrt_set_std_fd (int no, int fd)
{
  _gpgrt__set_std_fd (no, fd);  /* (double dash in name) */
}

estream_t
_gpgrt_get_std_stream (int fd)
{
  return _gpgrt__get_std_stream (fd);  /* (double dash in name) */
}

void
gpgrt_flockfile (estream_t stream)
{
  _gpgrt_flockfile (stream);
}

int
gpgrt_ftrylockfile (estream_t stream)
{
  return _gpgrt_ftrylockfile (stream);
}

void
gpgrt_funlockfile (estream_t stream)
{
  _gpgrt_funlockfile (stream);
}

int
_gpgrt_pending (estream_t stream)
{
  return _gpgrt__pending (stream);
}

int
_gpgrt_pending_unlocked (estream_t stream)
{
  return _gpgrt__pending_unlocked (stream);
}

int
gpgrt_feof (estream_t stream)
{
  return _gpgrt_feof (stream);
}

int
gpgrt_feof_unlocked (estream_t stream)
{
  return _gpgrt_feof_unlocked (stream);
}

int
gpgrt_ferror (estream_t stream)
{
  return _gpgrt_ferror (stream);
}

int
gpgrt_ferror_unlocked (estream_t stream)
{
  return _gpgrt_ferror_unlocked (stream);
}

void
gpgrt_clearerr (estream_t stream)
{
  _gpgrt_clearerr (stream);
}

void
gpgrt_clearerr_unlocked (estream_t stream)
{
  _gpgrt_clearerr_unlocked (stream);
}

int
gpgrt_fflush (estream_t stream)
{
  return _gpgrt_fflush (stream);
}

int
gpgrt_fseek (estream_t stream, long int offset, int whence)
{
  return _gpgrt_fseek (stream, offset, whence);
}

int
gpgrt_fseeko (estream_t stream, gpgrt_off_t offset, int whence)
{
  return _gpgrt_fseeko (stream, offset, whence);
}

long int
gpgrt_ftell (estream_t stream)
{
  return _gpgrt_ftell (stream);
}

gpgrt_off_t
gpgrt_ftello (estream_t stream)
{
  return _gpgrt_ftello (stream);
}

void
gpgrt_rewind (estream_t stream)
{
  _gpgrt_rewind (stream);
}

int
gpgrt_ftruncate (estream_t stream, gpgrt_off_t length)
{
  return _gpgrt_ftruncate (stream, length);
}

int
gpgrt_fgetc (estream_t stream)
{
  return _gpgrt_fgetc (stream);
}

int
_gpgrt_getc_underflow (estream_t stream)
{
  return _gpgrt__getc_underflow (stream);
}

int
gpgrt_fputc (int c, estream_t stream)
{
  return _gpgrt_fputc (c, stream);
}

int
_gpgrt_putc_overflow (int c, estream_t stream)
{
  return _gpgrt__putc_overflow (c, stream);
}

int
gpgrt_ungetc (int c, estream_t stream)
{
  return _gpgrt_ungetc (c, stream);
}

int
gpgrt_read (estream_t _GPGRT__RESTRICT stream,
            void *_GPGRT__RESTRICT buffer, size_t bytes_to_read,
            size_t *_GPGRT__RESTRICT bytes_read)
{
  return _gpgrt_read (stream, buffer, bytes_to_read, bytes_read);
}

int
gpgrt_write (estream_t _GPGRT__RESTRICT stream,
             const void *_GPGRT__RESTRICT buffer, size_t bytes_to_write,
             size_t *_GPGRT__RESTRICT bytes_written)
{
  return _gpgrt_write (stream, buffer, bytes_to_write, bytes_written);
}

int
gpgrt_write_sanitized (estream_t _GPGRT__RESTRICT stream,
                       const void * _GPGRT__RESTRICT buffer, size_t length,
                       const char * delimiters,
                       size_t * _GPGRT__RESTRICT bytes_written)
{
  return _gpgrt_write_sanitized (stream, buffer, length, delimiters,
                                 bytes_written);
}

int
gpgrt_write_hexstring (estream_t _GPGRT__RESTRICT stream,
                       const void *_GPGRT__RESTRICT buffer, size_t length,
                       int reserved, size_t *_GPGRT__RESTRICT bytes_written )
{
  return _gpgrt_write_hexstring (stream, buffer, length, reserved,
                                 bytes_written);
}

size_t
gpgrt_fread (void *_GPGRT__RESTRICT ptr, size_t size, size_t nitems,
             estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_fread (ptr, size, nitems, stream);
}

size_t
gpgrt_fwrite (const void *_GPGRT__RESTRICT ptr, size_t size, size_t nitems,
              estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_fwrite (ptr, size, nitems, stream);
}

char *
gpgrt_fgets (char *_GPGRT__RESTRICT buffer, int length,
             estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_fgets (buffer, length, stream);
}

int
gpgrt_fputs (const char *_GPGRT__RESTRICT s, estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_fputs (s, stream);
}

int
gpgrt_fputs_unlocked (const char *_GPGRT__RESTRICT s,
                      estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_fputs_unlocked (s, stream);
}

gpgrt_ssize_t
gpgrt_getline (char *_GPGRT__RESTRICT *_GPGRT__RESTRICT lineptr,
               size_t *_GPGRT__RESTRICT n, estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_getline (lineptr, n, stream);
}

gpgrt_ssize_t
gpgrt_read_line (estream_t stream,
                 char **addr_of_buffer, size_t *length_of_buffer,
                 size_t *max_length)
{
  return _gpgrt_read_line (stream, addr_of_buffer, length_of_buffer,
                           max_length);
}

int
gpgrt_vfprintf (estream_t _GPGRT__RESTRICT stream,
                const char *_GPGRT__RESTRICT format,
                va_list ap)
{
  return _gpgrt_vfprintf (stream, NULL, NULL, format, ap);
}

int
gpgrt_vfprintf_unlocked (estream_t _GPGRT__RESTRICT stream,
                          const char *_GPGRT__RESTRICT format,
                          va_list ap)
{
  return _gpgrt_vfprintf_unlocked (stream, NULL, NULL, format, ap);
}

int
gpgrt_printf (const char *_GPGRT__RESTRICT format, ...)
{
  va_list ap;
  int rc;

  va_start (ap, format);
  rc = _gpgrt_vfprintf (es_stdout, NULL, NULL, format, ap);
  va_end (ap);

  return rc;
}

int
gpgrt_printf_unlocked (const char *_GPGRT__RESTRICT format, ...)
{
  va_list ap;
  int rc;

  va_start (ap, format);
  rc = _gpgrt_vfprintf_unlocked (es_stdout, NULL, NULL, format, ap);
  va_end (ap);

  return rc;
}

int
gpgrt_fprintf (estream_t _GPGRT__RESTRICT stream,
               const char *_GPGRT__RESTRICT format, ...)
{
  va_list ap;
  int rc;

  va_start (ap, format);
  rc = _gpgrt_vfprintf (stream, NULL, NULL, format, ap);
  va_end (ap);

  return rc;
}

int
gpgrt_fprintf_unlocked (estream_t _GPGRT__RESTRICT stream,
                        const char *_GPGRT__RESTRICT format, ...)
{
  va_list ap;
  int rc;

  va_start (ap, format);
  rc = _gpgrt_vfprintf_unlocked (stream, NULL, NULL, format, ap);
  va_end (ap);

  return rc;
}

int
gpgrt_fprintf_sf (estream_t _GPGRT__RESTRICT stream,
                  gpgrt_string_filter_t sf, void *sfvalue,
                  const char *_GPGRT__RESTRICT format, ...)
{
  va_list ap;
  int rc;

  va_start (ap, format);
  rc = _gpgrt_vfprintf (stream, sf, sfvalue, format, ap);
  va_end (ap);

  return rc;
}

int
gpgrt_fprintf_sf_unlocked (estream_t _GPGRT__RESTRICT stream,
                           gpgrt_string_filter_t sf, void *sfvalue,
                           const char *_GPGRT__RESTRICT format, ...)
{
  va_list ap;
  int rc;

  va_start (ap, format);
  rc = _gpgrt_vfprintf_unlocked (stream, sf, sfvalue, format, ap);
  va_end (ap);

  return rc;
}

int
gpgrt_setvbuf (estream_t _GPGRT__RESTRICT stream,
                char *_GPGRT__RESTRICT buf, int type, size_t size)
{
  return _gpgrt_setvbuf (stream, buf, type, size);
}

void
gpgrt_setbuf (estream_t _GPGRT__RESTRICT stream, char *_GPGRT__RESTRICT buf)
{
  _gpgrt_setvbuf (stream, buf, buf? _IOFBF : _IONBF, BUFSIZ);
}

void
gpgrt_set_binary (estream_t stream)
{
  _gpgrt_set_binary (stream);
}

int
gpgrt_set_nonblock (estream_t stream, int onoff)
{
  return _gpgrt_set_nonblock (stream, onoff);
}

int
gpgrt_get_nonblock (estream_t stream)
{
  return _gpgrt_get_nonblock (stream);
}

int
gpgrt_poll (gpgrt_poll_t *fds, unsigned int nfds, int timeout)
{
  return _gpgrt_poll (fds, nfds, timeout);
}

estream_t
gpgrt_tmpfile (void)
{
  return _gpgrt_tmpfile ();
}

void
gpgrt_opaque_set (estream_t stream, void *opaque)
{
  _gpgrt_opaque_set (stream, opaque);
}

void *
gpgrt_opaque_get (estream_t stream)
{
  return _gpgrt_opaque_get (stream);
}

void
gpgrt_fname_set (estream_t stream, const char *fname)
{
  _gpgrt_fname_set (stream, fname);
}

const char *
gpgrt_fname_get (estream_t stream)
{
  return _gpgrt_fname_get (stream);
}

int
gpgrt_asprintf (char **r_buf, const char *_GPGRT__RESTRICT format, ...)
{
  va_list ap;
  int rc;

  va_start (ap, format);
  rc = _gpgrt_estream_vasprintf (r_buf, format, ap);
  va_end (ap);

  return rc;
}

int
gpgrt_vasprintf (char **r_buf, const char *_GPGRT__RESTRICT format, va_list ap)
{
  return _gpgrt_estream_vasprintf (r_buf, format, ap);
}

char *
gpgrt_bsprintf (const char *_GPGRT__RESTRICT format, ...)
{
  int rc;
  va_list ap;
  char *buf;

  va_start (ap, format);
  rc = _gpgrt_estream_vasprintf (&buf, format, ap);
  va_end (ap);
  if (rc < 0)
    return NULL;
  return buf;
}

char *
gpgrt_vbsprintf (const char *_GPGRT__RESTRICT format, va_list ap)
{
  int rc;
  char *buf;

  rc = _gpgrt_estream_vasprintf (&buf, format, ap);
  if (rc < 0)
    return NULL;
  return buf;
}

int
gpgrt_snprintf (char *buf, size_t bufsize, const char *format, ...)
{
  int rc;
  va_list arg_ptr;

  va_start (arg_ptr, format);
  rc = _gpgrt_estream_vsnprintf (buf, bufsize, format, arg_ptr);
  va_end (arg_ptr);

  return rc;
}

int
gpgrt_vsnprintf (char *buf, size_t bufsize,
                 const char *format, va_list arg_ptr)
{
  return _gpgrt_estream_vsnprintf (buf, bufsize, format, arg_ptr);
}



void *
gpgrt_realloc (void *a, size_t n)
{
  return _gpgrt_realloc (a, n);
}

void *
gpgrt_reallocarray (void *a, size_t oldnmemb, size_t nmemb, size_t size)
{
  return _gpgrt_reallocarray (a, oldnmemb, nmemb, size);
}

void *
gpgrt_malloc (size_t n)
{
  return _gpgrt_malloc (n);
}

void *
gpgrt_calloc (size_t n, size_t m)
{
  return _gpgrt_calloc (n, m);
}

char *
gpgrt_strdup (const char *string)
{
  return _gpgrt_strdup (string);
}

char *
gpgrt_strconcat (const char *s1, ...)
{
  va_list arg_ptr;
  char *result;

  if (!s1)
    result = _gpgrt_strdup ("");
  else
    {
      va_start (arg_ptr, s1);
      result = _gpgrt_strconcat_core (s1, arg_ptr);
      va_end (arg_ptr);
    }
  return result;
}

void
gpgrt_free (void *a)
{
  if (a)
    _gpgrt_free (a);
}

char *
gpgrt_getenv (const char *name)
{
  return _gpgrt_getenv (name);
}

gpg_err_code_t
gpgrt_setenv (const char *name, const char *value, int overwrite)
{
  return _gpgrt_setenv (name, value, overwrite);
}

gpg_err_code_t
gpgrt_mkdir (const char *name, const char *modestr)
{
  return _gpgrt_mkdir (name, modestr);
}

gpg_err_code_t
gpgrt_chdir (const char *name)
{
  return _gpgrt_chdir (name);
}

char *
gpgrt_getcwd (void)
{
  return _gpgrt_getcwd ();
}

gpg_err_code_t
gpgrt_access (const char *fname, int mode)
{
  return _gpgrt_access (fname, mode);
}



gpgrt_b64state_t
gpgrt_b64enc_start (estream_t stream, const char *title)
{
  return _gpgrt_b64enc_start (stream, title);
}

gpg_err_code_t
gpgrt_b64enc_write (gpgrt_b64state_t state, const void *buffer, size_t nbytes)
{
  return _gpgrt_b64enc_write (state, buffer, nbytes);
}

gpg_err_code_t
gpgrt_b64enc_finish (gpgrt_b64state_t state)
{
  return _gpgrt_b64enc_finish (state);
}

gpgrt_b64state_t
gpgrt_b64dec_start (const char *title)
{
  return _gpgrt_b64dec_start (title);
}

gpg_error_t
gpgrt_b64dec_proc (gpgrt_b64state_t state, void *buffer,
                   size_t length, size_t *r_nbytes)
{
  return _gpgrt_b64dec_proc (state, buffer, length, r_nbytes);
}

gpg_error_t
gpgrt_b64dec_finish (gpgrt_b64state_t state)
{
  return _gpgrt_b64dec_finish (state);
}



int
gpgrt_get_errorcount (int clear)
{
  return _gpgrt_get_errorcount (clear);
}

void
gpgrt_inc_errorcount (void)
{
  _gpgrt_inc_errorcount ();
}

void
gpgrt_log_set_sink (const char *name, estream_t stream, int fd)
{
  _gpgrt_log_set_sink (name, stream, fd);
}

void
gpgrt_log_set_socket_dir_cb (const char *(*fnc)(void))
{
  _gpgrt_log_set_socket_dir_cb (fnc);
}

void
gpgrt_log_set_pid_suffix_cb (int (*cb)(unsigned long *r_value))
{
  _gpgrt_log_set_pid_suffix_cb (cb);
}

void
gpgrt_log_set_prefix (const char *text, unsigned int flags)
{
  _gpgrt_log_set_prefix (text, flags);
}

const char *
gpgrt_log_get_prefix (unsigned int *flags)
{
  return _gpgrt_log_get_prefix (flags);
}

int
gpgrt_log_test_fd (int fd)
{
  return _gpgrt_log_test_fd (fd);
}

int
gpgrt_log_get_fd (void)
{
  return _gpgrt_log_get_fd ();
}

estream_t
gpgrt_log_get_stream (void)
{
  return _gpgrt_log_get_stream ();
}

void
gpgrt_log (int level, const char *fmt, ...)
{
  va_list arg_ptr ;

  va_start (arg_ptr, fmt) ;
  _gpgrt_logv (level, fmt, arg_ptr);
  va_end (arg_ptr);
}

void
gpgrt_logv (int level, const char *fmt, va_list arg_ptr)
{
  _gpgrt_logv (level, fmt, arg_ptr);
}

void
gpgrt_logv_prefix (int level, const char *prefix,
                    const char *fmt, va_list arg_ptr)
{
  _gpgrt_logv_prefix (level, prefix, fmt, arg_ptr);
}

void
gpgrt_log_string (int level, const char *string)
{
  _gpgrt_log_string (level, string);
}

void
gpgrt_log_info (const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  _gpgrt_logv (GPGRT_LOGLVL_INFO, fmt, arg_ptr);
  va_end (arg_ptr);
}

void
gpgrt_log_error (const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  _gpgrt_logv (GPGRT_LOGLVL_ERROR, fmt, arg_ptr);
  va_end (arg_ptr);
}

void
gpgrt_log_fatal (const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  _gpgrt_logv (GPGRT_LOGLVL_FATAL, fmt, arg_ptr);
  va_end (arg_ptr);
  _gpgrt_abort (); /* Never called; just to make the compiler happy.  */
}

void
gpgrt_log_bug (const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  _gpgrt_logv (GPGRT_LOGLVL_BUG, fmt, arg_ptr);
  va_end (arg_ptr);
  _gpgrt_abort (); /* Never called; just to make the compiler happy.  */
}

void
gpgrt_log_debug (const char *fmt, ...)
{
  va_list arg_ptr ;

  va_start (arg_ptr, fmt);
  _gpgrt_logv (GPGRT_LOGLVL_DEBUG, fmt, arg_ptr);
  va_end (arg_ptr);
}

void
gpgrt_log_debug_string (const char *string, const char *fmt, ...)
{
  va_list arg_ptr ;

  va_start (arg_ptr, fmt);
  _gpgrt_logv_internal (GPGRT_LOGLVL_DEBUG, 0, string, NULL, fmt, arg_ptr);
  va_end (arg_ptr);
}

void
gpgrt_log_printf (const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  _gpgrt_logv (fmt ? GPGRT_LOGLVL_CONT : GPGRT_LOGLVL_BEGIN, fmt, arg_ptr);
  va_end (arg_ptr);
}

void
gpgrt_log_flush (void)
{
  _gpgrt_log_flush ();
}

void
gpgrt_log_printhex (const void *buffer, size_t length, const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  _gpgrt_logv_printhex (buffer, length, fmt, arg_ptr);
  va_end (arg_ptr);
}

void
gpgrt_log_clock (const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  _gpgrt_logv_clock (fmt, arg_ptr);
  va_end (arg_ptr);
}

void
_gpgrt_log_assert (const char *expr, const char *file,
                   int line, const char *func)
{
#ifdef GPGRT_HAVE_MACRO_FUNCTION
  _gpgrt__log_assert (expr, file, line, func);
#else
  _gpgrt__log_assert (expr, file, line);
#endif
}


#if 0
gpg_err_code_t
gpgrt_make_pipe (int filedes[2], estream_t *r_fp, int direction, int nonblock)
{
  return _gpgrt_make_pipe (filedes, r_fp, direction, nonblock);
}

gpg_err_code_t
gpgrt_spawn_process (const char *pgmname, const char *argv[],
                     int *except, unsigned int flags,
                     estream_t *r_infp, estream_t *r_outfp, estream_t *r_errfp,
                     gpgrt_process_t *r_process_id)
{
  return _gpgrt_spawn_process (pgmname, argv, except, flags,
                               r_infp, r_outfp, r_errfp, r_process_id);
}

gpg_err_code_t
gpgrt_spawn_process_fd (const char *pgmname, const char *argv[],
                        int infd, int outfd, int errfd,
                        int (*spawn_cb)(void *),
                        void *spawn_cb_arg, gpgrt_process_t *r_process_id)
{
  return _gpgrt_spawn_process_fd (pgmname, argv, infd, outfd, errfd,
                                  spawn_cb, spawn_cb_arg, r_process_id);
}

gpg_err_code_t
gpgrt_spawn_process_detached (const char *pgmname, const char *argv[],
                              const char *envp[])
{
  return _gpgrt_spawn_process_detached (pgmname, argv, envp);
}

gpg_err_code_t
gpgrt_wait_process (const char *pgmname, gpgrt_process_t process_id, int hang,
                    int *r_exitcode)
{
  return _gpgrt_wait_process (pgmname, process_id, hang, r_exitcode);
}

gpg_err_code_t
gpgrt_wait_processes (const char **pgmnames, gpgrt_process_t *process_ids,
                      size_t count, int hang, int *r_exitcodes)
{
  return _gpgrt_wait_processes (pgmnames, process_ids, count, hang,
                                r_exitcodes);
}

void
gpgrt_kill_process (gpgrt_process_t process_id)
{
  _gpgrt_kill_process (process_id);
}

void
gpgrt_release_process (gpgrt_process_t process_id)
{
  _gpgrt_release_process (process_id);
}

void
gpgrt_close_all_fds (int from, int *keep_fds)
{
  _gpgrt_close_all_fds (from, keep_fds);
}
#endif /*0*/


int
gpgrt_argparse (estream_t fp, gpgrt_argparse_t *arg, gpgrt_opt_t *opts)
{
  return _gpgrt_argparse (fp, arg, opts);
}

int
gpgrt_argparser (gpgrt_argparse_t *arg, gpgrt_opt_t *opts, const char *name)
{
  return _gpgrt_argparser (arg, opts, name);
}

void
gpgrt_usage (int level)
{
  _gpgrt_usage (level);
}

const char *
gpgrt_strusage (int level)
{
  return _gpgrt_strusage (level);
}

void
gpgrt_set_strusage (const char *(*f)(int))
{
  _gpgrt_set_strusage (f);
}

void
gpgrt_set_usage_outfnc (int (*f)(int, const char *))
{
  _gpgrt_set_usage_outfnc (f);
}

void
gpgrt_set_fixed_string_mapper (const char *(*f)(const char*))
{
  _gpgrt_set_fixed_string_mapper (f);
}

void
gpgrt_set_confdir (int what, const char *name)
{
  _gpgrt_set_confdir (what, name);
}



/* Compare program versions.  */
int
gpgrt_cmp_version (const char *a, const char *b, int level)
{
  return _gpgrt_cmp_version (a, b, level);
}



/* String utilities.  */
char *
gpgrt_fnameconcat (const char *first, ... )
{
  va_list arg_ptr;
  char *result;

  va_start (arg_ptr, first);
  result = _gpgrt_vfnameconcat (0, first, arg_ptr);
  va_end (arg_ptr);
  return result;
}

char *
gpgrt_absfnameconcat (const char *first, ... )
{
  va_list arg_ptr;
  char *result;

  va_start (arg_ptr, first);
  result = _gpgrt_vfnameconcat (1, first, arg_ptr);
  va_end (arg_ptr);
  return result;
}



/* For consistency reasons we use function wrappers also for Windows
 * specific function despite that they are technically not needed.  */
#ifdef HAVE_W32_SYSTEM

void
gpgrt_free_wchar (wchar_t *wstring)
{
  if (wstring)
    _gpgrt_free_wchar (wstring);
}

wchar_t *
gpgrt_fname_to_wchar (const char *fname)
{
  return _gpgrt_fname_to_wchar (fname);
}

wchar_t *
gpgrt_utf8_to_wchar (const char *string)
{
  return _gpgrt_utf8_to_wchar (string);
}

char *
gpgrt_wchar_to_utf8 (const wchar_t *string)
{
  return _gpgrt_wchar_to_utf8 (string, (size_t)(-1));
}

char *
gpgrt_w32_reg_query_string (const char *root, const char *dir, const char *name)
{
  return _gpgrt_w32_reg_query_string (root, dir, name);
}

#endif /*HAVE_W32_SYSTEM*/
