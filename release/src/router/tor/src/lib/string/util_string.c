/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file util_string.c
 * \brief Non-standard string functions used throughout Tor.
 **/

#include "lib/string/util_string.h"
#include "lib/string/compat_ctype.h"
#include "lib/err/torerr.h"
#include "lib/ctime/di_ops.h"
#include "lib/defs/digest_sizes.h"

#include <string.h>
#include <stdlib.h>

/** Given <b>hlen</b> bytes at <b>haystack</b> and <b>nlen</b> bytes at
 * <b>needle</b>, return a pointer to the first occurrence of the needle
 * within the haystack, or NULL if there is no such occurrence.
 *
 * This function is <em>not</em> timing-safe.
 *
 * Requires that <b>nlen</b> be greater than zero.
 */
const void *
tor_memmem(const void *_haystack, size_t hlen,
           const void *_needle, size_t nlen)
{
#if defined(HAVE_MEMMEM) && (!defined(__GNUC__) || __GNUC__ >= 2)
  raw_assert(nlen);
  return memmem(_haystack, hlen, _needle, nlen);
#else
  /* This isn't as fast as the GLIBC implementation, but it doesn't need to
   * be. */
  const char *p, *last_possible_start;
  const char *haystack = (const char*)_haystack;
  const char *needle = (const char*)_needle;
  char first;
  raw_assert(nlen);

  if (nlen > hlen)
    return NULL;

  p = haystack;
  /* Last position at which the needle could start. */
  last_possible_start = haystack + hlen - nlen;
  first = *(const char*)needle;
  while ((p = memchr(p, first, last_possible_start + 1 - p))) {
    if (fast_memeq(p, needle, nlen))
      return p;
    if (++p > last_possible_start) {
      /* This comparison shouldn't be necessary, since if p was previously
       * equal to last_possible_start, the next memchr call would be
       * "memchr(p, first, 0)", which will return NULL. But it clarifies the
       * logic. */
      return NULL;
    }
  }
  return NULL;
#endif /* defined(HAVE_MEMMEM) && (!defined(__GNUC__) || __GNUC__ >= 2) */
}

const void *
tor_memstr(const void *haystack, size_t hlen, const char *needle)
{
  return tor_memmem(haystack, hlen, needle, strlen(needle));
}

/** Return true iff the 'len' bytes at 'mem' are all zero. */
int
fast_mem_is_zero(const char *mem, size_t len)
{
  static const char ZERO[] = {
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
  };
  while (len >= sizeof(ZERO)) {
    /* It's safe to use fast_memcmp here, since the very worst thing an
     * attacker could learn is how many initial bytes of a secret were zero */
    if (fast_memcmp(mem, ZERO, sizeof(ZERO)))
      return 0;
    len -= sizeof(ZERO);
    mem += sizeof(ZERO);
  }
  /* Deal with leftover bytes. */
  if (len)
    return fast_memeq(mem, ZERO, len);

  return 1;
}

/** Return true iff the DIGEST_LEN bytes in digest are all zero. */
int
tor_digest_is_zero(const char *digest)
{
  return safe_mem_is_zero(digest, DIGEST_LEN);
}

/** Return true iff the DIGEST256_LEN bytes in digest are all zero. */
int
tor_digest256_is_zero(const char *digest)
{
  return safe_mem_is_zero(digest, DIGEST256_LEN);
}

/** Remove from the string <b>s</b> every character which appears in
 * <b>strip</b>. */
void
tor_strstrip(char *s, const char *strip)
{
  char *readp = s;
  while (*readp) {
    if (strchr(strip, *readp)) {
      ++readp;
    } else {
      *s++ = *readp++;
    }
  }
  *s = '\0';
}

/** Convert all alphabetic characters in the nul-terminated string <b>s</b> to
 * lowercase. */
void
tor_strlower(char *s)
{
  while (*s) {
    *s = TOR_TOLOWER(*s);
    ++s;
  }
}

/** Convert all alphabetic characters in the nul-terminated string <b>s</b> to
 * lowercase. */
void
tor_strupper(char *s)
{
  while (*s) {
    *s = TOR_TOUPPER(*s);
    ++s;
  }
}

/** Replaces <b>old</b> with <b>replacement</b> in <b>s</b> */
void
tor_strreplacechar(char *s, char find, char replacement)
{
  for (s = strchr(s, find); s; s = strchr(s + 1, find)) {
    *s = replacement;
  }
}

/** Return 1 if every character in <b>s</b> is printable, else return 0.
 */
int
tor_strisprint(const char *s)
{
  while (*s) {
    if (!TOR_ISPRINT(*s))
      return 0;
    s++;
  }
  return 1;
}

/** Return 1 if no character in <b>s</b> is uppercase, else return 0.
 */
int
tor_strisnonupper(const char *s)
{
  while (*s) {
    if (TOR_ISUPPER(*s))
      return 0;
    s++;
  }
  return 1;
}

/** Return true iff every character in <b>s</b> is whitespace space; else
 * return false. */
int
tor_strisspace(const char *s)
{
  while (*s) {
    if (!TOR_ISSPACE(*s))
      return 0;
    s++;
  }
  return 1;
}

/** As strcmp, except that either string may be NULL.  The NULL string is
 * considered to be before any non-NULL string. */
int
strcmp_opt(const char *s1, const char *s2)
{
  if (!s1) {
    if (!s2)
      return 0;
    else
      return -1;
  } else if (!s2) {
    return 1;
  } else {
    return strcmp(s1, s2);
  }
}

/** Compares the first strlen(s2) characters of s1 with s2.  Returns as for
 * strcmp.
 */
int
strcmpstart(const char *s1, const char *s2)
{
  size_t n = strlen(s2);
  return strncmp(s1, s2, n);
}

/** Compares the first strlen(s2) characters of s1 with s2.  Returns as for
 * strcasecmp.
 */
int
strcasecmpstart(const char *s1, const char *s2)
{
  size_t n = strlen(s2);
  return strncasecmp(s1, s2, n);
}

/** Compare the value of the string <b>prefix</b> with the start of the
 * <b>memlen</b>-byte memory chunk at <b>mem</b>.  Return as for strcmp.
 *
 * [As fast_memcmp(mem, prefix, strlen(prefix)) but returns -1 if memlen is
 * less than strlen(prefix).]
 */
int
fast_memcmpstart(const void *mem, size_t memlen,
                const char *prefix)
{
  size_t plen = strlen(prefix);
  if (memlen < plen)
    return -1;
  return fast_memcmp(mem, prefix, plen);
}

/** Compares the last strlen(s2) characters of s1 with s2.  Returns as for
 * strcmp.
 */
int
strcmpend(const char *s1, const char *s2)
{
  size_t n1 = strlen(s1), n2 = strlen(s2);
  if (n2>n1)
    return strcmp(s1,s2);
  else
    return strncmp(s1+(n1-n2), s2, n2);
}

/** Compares the last strlen(s2) characters of s1 with s2.  Returns as for
 * strcasecmp.
 */
int
strcasecmpend(const char *s1, const char *s2)
{
  size_t n1 = strlen(s1), n2 = strlen(s2);
  if (n2>n1) /* then they can't be the same; figure out which is bigger */
    return strcasecmp(s1,s2);
  else
    return strncasecmp(s1+(n1-n2), s2, n2);
}

/** Return a pointer to the first char of s that is not whitespace and
 * not a comment, or to the terminating NUL if no such character exists.
 */
const char *
eat_whitespace(const char *s)
{
  raw_assert(s);

  while (1) {
    switch (*s) {
    case '\0':
    default:
      return s;
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      ++s;
      break;
    case '#':
      ++s;
      while (*s && *s != '\n')
        ++s;
    }
  }
}

/** Return a pointer to the first char of s that is not whitespace and
 * not a comment, or to the terminating NUL if no such character exists.
 */
const char *
eat_whitespace_eos(const char *s, const char *eos)
{
  raw_assert(s);
  raw_assert(eos && s <= eos);

  while (s < eos) {
    switch (*s) {
    case '\0':
    default:
      return s;
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      ++s;
      break;
    case '#':
      ++s;
      while (s < eos && *s && *s != '\n')
        ++s;
    }
  }
  return s;
}

/** Return a pointer to the first char of s that is not a space or a tab
 * or a \\r, or to the terminating NUL if no such character exists. */
const char *
eat_whitespace_no_nl(const char *s)
{
  while (*s == ' ' || *s == '\t' || *s == '\r')
    ++s;
  return s;
}

/** As eat_whitespace_no_nl, but stop at <b>eos</b> whether we have
 * found a non-whitespace character or not. */
const char *
eat_whitespace_eos_no_nl(const char *s, const char *eos)
{
  while (s < eos && (*s == ' ' || *s == '\t' || *s == '\r'))
    ++s;
  return s;
}

/** Return a pointer to the first char of s that is whitespace or <b>#</b>,
 * or to the terminating NUL if no such character exists.
 */
const char *
find_whitespace(const char *s)
{
  /* tor_assert(s); */
  while (1) {
    switch (*s)
    {
    case '\0':
    case '#':
    case ' ':
    case '\r':
    case '\n':
    case '\t':
      return s;
    default:
      ++s;
    }
  }
}

/** As find_whitespace, but stop at <b>eos</b> whether we have found a
 * whitespace or not. */
const char *
find_whitespace_eos(const char *s, const char *eos)
{
  /* tor_assert(s); */
  while (s < eos) {
    switch (*s)
    {
    case '\0':
    case '#':
    case ' ':
    case '\r':
    case '\n':
    case '\t':
      return s;
    default:
      ++s;
    }
  }
  return s;
}

/** Return the first occurrence of <b>needle</b> in <b>haystack</b> that
 * occurs at the start of a line (that is, at the beginning of <b>haystack</b>
 * or immediately after a newline).  Return NULL if no such string is found.
 */
const char *
find_str_at_start_of_line(const char *haystack, const char *needle)
{
  size_t needle_len = strlen(needle);

  do {
    if (!strncmp(haystack, needle, needle_len))
      return haystack;

    haystack = strchr(haystack, '\n');
    if (!haystack)
      return NULL;
    else
      ++haystack;
  } while (*haystack);

  return NULL;
}

/** Returns true if <b>string</b> could be a C identifier.
    A C identifier must begin with a letter or an underscore and the
    rest of its characters can be letters, numbers or underscores. No
    length limit is imposed. */
int
string_is_C_identifier(const char *string)
{
  size_t iter;
  size_t length = strlen(string);
  if (!length)
    return 0;

  for (iter = 0; iter < length ; iter++) {
    if (iter == 0) {
      if (!(TOR_ISALPHA(string[iter]) ||
            string[iter] == '_'))
        return 0;
    } else {
      if (!(TOR_ISALPHA(string[iter]) ||
            TOR_ISDIGIT(string[iter]) ||
            string[iter] == '_'))
        return 0;
    }
  }

  return 1;
}

/** A byte with the top <b>x</b> bits set. */
#define TOP_BITS(x) ((uint8_t)(0xFF << (8 - (x))))
/** A byte with the lowest <b>x</b> bits set. */
#define LOW_BITS(x) ((uint8_t)(0xFF >> (8 - (x))))

/** Given the leading byte <b>b</b>, return the total number of bytes in the
 * UTF-8 character. Returns 0 if it's an invalid leading byte.
 */
static uint8_t
bytes_in_char(uint8_t b)
{
  if ((TOP_BITS(1) & b) == 0x00)
    return 1; // a 1-byte UTF-8 char, aka ASCII
  if ((TOP_BITS(3) & b) == TOP_BITS(2))
    return 2; // a 2-byte UTF-8 char
  if ((TOP_BITS(4) & b) == TOP_BITS(3))
    return 3; // a 3-byte UTF-8 char
  if ((TOP_BITS(5) & b) == TOP_BITS(4))
    return 4; // a 4-byte UTF-8 char

  // Invalid: either the top 2 bits are 10, or the top 5 bits are 11111.
  return 0;
}

/** Returns true iff <b>b</b> is a UTF-8 continuation byte. */
static bool
is_continuation_byte(uint8_t b)
{
  uint8_t top2bits = b & TOP_BITS(2);
  return top2bits == TOP_BITS(1);
}

/** Returns true iff the <b>len</b> bytes in <b>c</b> are a valid UTF-8
 * character.
 */
static bool
validate_char(const uint8_t *c, uint8_t len)
{
  if (len == 1)
    return true; // already validated this is an ASCII char earlier.

  uint8_t mask = LOW_BITS(7 - len); // bitmask for the leading byte.
  uint32_t codepoint = c[0] & mask;

  mask = LOW_BITS(6); // bitmask for continuation bytes.
  for (uint8_t i = 1; i < len; i++) {
    if (!is_continuation_byte(c[i]))
      return false;
    codepoint <<= 6;
    codepoint |= (c[i] & mask);
  }

  if (len == 2 && codepoint <= 0x7f)
    return false; // Invalid, overly long encoding, should have fit in 1 byte.

  if (len == 3 && codepoint <= 0x7ff)
    return false; // Invalid, overly long encoding, should have fit in 2 bytes.

  if (len == 4 && codepoint <= 0xffff)
    return false; // Invalid, overly long encoding, should have fit in 3 bytes.

  if (codepoint >= 0xd800 && codepoint <= 0xdfff)
    return false; // Invalid, reserved for UTF-16 surrogate pairs.

  return codepoint <= 0x10ffff; // Check if within maximum.
}

/** Returns true iff the first <b>len</b> bytes in <b>str</b> are a
    valid UTF-8 string. */
int
string_is_utf8(const char *str, size_t len)
{
  // If str is NULL, don't try to read it
  if (!str) {
    // We could test for this case, but the low-level logs would produce
    // confusing test output.
    // LCOV_EXCL_START
    if (len) {
      // Use the low-level logging function, so that the log module can
      // validate UTF-8 (if needed in future code)
      tor_log_err_sigsafe(
        "BUG: string_is_utf8() called with NULL str but non-zero len.");
      // Since it's a bug, we should probably reject this string
      return false;
    }
    // LCOV_EXCL_STOP
    return true;
  }

  for (size_t i = 0; i < len;) {
    uint8_t num_bytes = bytes_in_char(str[i]);
    if (num_bytes == 0) // Invalid leading byte found.
      return false;

    size_t next_char = i + num_bytes;
    if (next_char > len)
      return false;

    // Validate the continuation bytes in this multi-byte character,
    // and advance to the next character in the string.
    if (!validate_char((const uint8_t*)&str[i], num_bytes))
      return false;
    i = next_char;
  }
  return true;
}

/** As string_is_utf8(), but returns false if the string begins with a UTF-8
 * byte order mark (BOM).
 */
int
string_is_utf8_no_bom(const char *str, size_t len)
{
  if (str && len >= 3 && (!strcmpstart(str, "\uFEFF") ||
                          !strcmpstart(str, "\uFFFE"))) {
    return false;
  }
  return string_is_utf8(str, len);
}
