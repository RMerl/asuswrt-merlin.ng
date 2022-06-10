/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file cstring.c
 *
 * \brief Decode data that has been written as a C literal.
 **/

#include "lib/encoding/cstring.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/string/compat_ctype.h"

#include <string.h>

#define TOR_ISODIGIT(c) ('0' <= (c) && (c) <= '7')

/** Given a c-style double-quoted escaped string in <b>s</b>, extract and
 * decode its contents into a newly allocated string.  On success, assign this
 * string to *<b>result</b>, assign its length to <b>size_out</b> (if
 * provided), and return a pointer to the position in <b>s</b> immediately
 * after the string.  On failure, return NULL.
 */
const char *
unescape_string(const char *s, char **result, size_t *size_out)
{
  const char *cp;
  char *out;
  if (s[0] != '\"')
    return NULL;
  cp = s+1;
  while (1) {
    switch (*cp) {
      case '\0':
      case '\n':
        return NULL;
      case '\"':
        goto end_of_loop;
      case '\\':
        if (cp[1] == 'x' || cp[1] == 'X') {
          if (!(TOR_ISXDIGIT(cp[2]) && TOR_ISXDIGIT(cp[3])))
            return NULL;
          cp += 4;
        } else if (TOR_ISODIGIT(cp[1])) {
          cp += 2;
          if (TOR_ISODIGIT(*cp)) ++cp;
          if (TOR_ISODIGIT(*cp)) ++cp;
        } else if (cp[1] == 'n' || cp[1] == 'r' || cp[1] == 't' || cp[1] == '"'
                   || cp[1] == '\\' || cp[1] == '\'') {
          cp += 2;
        } else {
          return NULL;
        }
        break;
      default:
        ++cp;
        break;
    }
  }
 end_of_loop:
  out = *result = tor_malloc(cp-s + 1);
  cp = s+1;
  while (1) {
    switch (*cp)
      {
      case '\"':
        *out = '\0';
        if (size_out) *size_out = out - *result;
        return cp+1;

        /* LCOV_EXCL_START -- we caught this in parse_config_from_line. */
      case '\0':
        tor_fragile_assert();
        tor_free(*result);
        return NULL;
        /* LCOV_EXCL_STOP */
      case '\\':
        switch (cp[1])
          {
          case 'n': *out++ = '\n'; cp += 2; break;
          case 'r': *out++ = '\r'; cp += 2; break;
          case 't': *out++ = '\t'; cp += 2; break;
          case 'x': case 'X':
            {
              int x1, x2;

              x1 = hex_decode_digit(cp[2]);
              x2 = hex_decode_digit(cp[3]);
              if (x1 == -1 || x2 == -1) {
                /* LCOV_EXCL_START */
                /* we caught this above in the initial loop. */
                tor_assert_nonfatal_unreached();
                tor_free(*result);
                return NULL;
                /* LCOV_EXCL_STOP */
              }

              *out++ = ((x1<<4) + x2);
              cp += 4;
            }
            break;
          case '0': case '1': case '2': case '3': case '4': case '5':
          case '6': case '7':
            {
              int n = cp[1]-'0';
              cp += 2;
              if (TOR_ISODIGIT(*cp)) { n = n*8 + *cp-'0'; cp++; }
              if (TOR_ISODIGIT(*cp)) { n = n*8 + *cp-'0'; cp++; }
              if (n > 255) { tor_free(*result); return NULL; }
              *out++ = (char)n;
            }
            break;
          case '\'':
          case '\"':
          case '\\':
          case '\?':
            *out++ = cp[1];
            cp += 2;
            break;

            /* LCOV_EXCL_START */
          default:
            /* we caught this above in the initial loop. */
            tor_assert_nonfatal_unreached();
            tor_free(*result); return NULL;
            /* LCOV_EXCL_STOP */
          }
        break;
      default:
        *out++ = *cp++;
      }
  }
}
