/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file escape.c
 * \brief Escape untrusted strings before sending them to the log.
 **/

#include "lib/log/escape.h"
#include "lib/log/util_bug.h"
#include "lib/string/compat_ctype.h"
#include "lib/string/printf.h"
#include "lib/malloc/malloc.h"

/** Allocate and return a new string representing the contents of <b>s</b>,
 * surrounded by quotes and using standard C escapes.
 *
 * Generally, we use this for logging values that come in over the network to
 * keep them from tricking users, and for sending certain values to the
 * controller.
 *
 * We trust values from the resolver, OS, configuration file, and command line
 * to not be maliciously ill-formed.  We validate incoming routerdescs and
 * SOCKS requests and addresses from BEGIN cells as they're parsed;
 * afterwards, we trust them as non-malicious.
 */
char *
esc_for_log(const char *s)
{
  const char *cp;
  char *result, *outp;
  size_t len = 3;
  if (!s) {
    return tor_strdup("(null)");
  }

  for (cp = s; *cp; ++cp) {
    switch (*cp) {
      case '\\':
      case '\"':
      case '\'':
      case '\r':
      case '\n':
      case '\t':
        len += 2;
        break;
      default:
        if (TOR_ISPRINT(*cp) && ((uint8_t)*cp)<127)
          ++len;
        else
          len += 4;
        break;
    }
  }

  tor_assert(len <= SSIZE_MAX);

  result = outp = tor_malloc(len);
  *outp++ = '\"';
  for (cp = s; *cp; ++cp) {
    /* This assertion should always succeed, since we will write at least
     * one char here, and two chars for closing quote and nul later */
    tor_assert((outp-result) < (ssize_t)len-2);
    switch (*cp) {
      case '\\':
      case '\"':
      case '\'':
        *outp++ = '\\';
        *outp++ = *cp;
        break;
      case '\n':
        *outp++ = '\\';
        *outp++ = 'n';
        break;
      case '\t':
        *outp++ = '\\';
        *outp++ = 't';
        break;
      case '\r':
        *outp++ = '\\';
        *outp++ = 'r';
        break;
      default:
        if (TOR_ISPRINT(*cp) && ((uint8_t)*cp)<127) {
          *outp++ = *cp;
        } else {
          tor_assert((outp-result) < (ssize_t)len-4);
          tor_snprintf(outp, 5, "\\%03o", (int)(uint8_t) *cp);
          outp += 4;
        }
        break;
    }
  }

  tor_assert((outp-result) <= (ssize_t)len-2);
  *outp++ = '\"';
  *outp++ = 0;

  return result;
}

/** Similar to esc_for_log. Allocate and return a new string representing
 * the first n characters in <b>chars</b>, surround by quotes and using
 * standard C escapes. If a NUL character is encountered in <b>chars</b>,
 * the resulting string will be terminated there.
 */
char *
esc_for_log_len(const char *chars, size_t n)
{
  char *string = tor_strndup(chars, n);
  char *string_escaped = esc_for_log(string);
  tor_free(string);
  return string_escaped;
}

/** Allocate and return a new string representing the contents of <b>s</b>,
 * surrounded by quotes and using standard C escapes.
 *
 * THIS FUNCTION IS NOT REENTRANT.  Don't call it from outside the main
 * thread.  Also, each call invalidates the last-returned value, so don't
 * try log_warn(LD_GENERAL, "%s %s", escaped(a), escaped(b));
 */
const char *
escaped(const char *s)
{
  static char *escaped_val_ = NULL;
  tor_free(escaped_val_);

  if (s)
    escaped_val_ = esc_for_log(s);
  else
    escaped_val_ = NULL;

  return escaped_val_;
}
