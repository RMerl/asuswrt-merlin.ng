/* source: utils.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* useful additions to C library */

#include "config.h"

#include "sysincludes.h"

#include "compat.h"	/* socklen_t */
#include "mytypes.h"
#include "sycls.h"
#include "utils.h"


#if !HAVE_PROTOTYPE_LIB_memrchr
/* GNU extension, available since glibc 2.1.91 */
void *memrchr(const void *s, int c, size_t n) {
   const unsigned char *t = ((unsigned char *)s)+n;
   while (--t >= (unsigned char *)s) {
      if (*t == c)  break;
   }
   if (t < (unsigned char *)s)
      return NULL;
   return (void *)t;
}
#endif /* !HAVE_PROTOTYPE_LIB_memrchr */

void *memdup(const void *src, size_t n) {
   void *dest;

   if ((dest = Malloc(n)) == NULL) {
      return NULL;
   }

   memcpy(dest, src, n);
   return dest;
}

/* search the keyword-table for a match of the leading part of name. */
/* returns the pointer to the matching field of the keyword or NULL if no
   keyword was found. */
const struct wordent *keyw(const struct wordent *keywds, const char *name, unsigned int nkeys) {
   unsigned int lower, upper, mid;
   int r;

   lower = 0;
   upper = nkeys;

   while (upper - lower > 1)
   {
      mid = (upper + lower) >> 1;
      if (!(r = strcasecmp(keywds[mid].name, name)))
      {
	 return &keywds[mid];
      }
      if (r < 0)
	 lower = mid;
      else
	 upper = mid;
   }
   if (nkeys > 0 && !(strcasecmp(keywds[lower].name, name)))
   {
      return &keywds[lower];
   }
   return NULL;
}

/* Linux: setenv(), AIX (4.3?): putenv() */
#if !HAVE_SETENV
int setenv(const char *name, const char *value, int overwrite) {
   int result;
   char *env;
   if (!overwrite) {
      if (getenv(name))  return 0;	/* already exists */
   }
   if ((env = Malloc(strlen(name)+strlen(value)+2)) == NULL) {
      return -1;
   }
   sprintf(env, "%s=%s", name, value);
   if ((result = putenv(env)) != 0) {	/* AIX docu says "... nonzero ..." */
      free(env);
      result = -1;
   }
   /* linux "man putenv" says: ...this string becomes part of the environment*/
   return result;
}
#endif /* !HAVE_SETENV */



/* sanitizes an "untrusted" character. output buffer must provide at least 4
   characters space.
   Does not append \0. returns length of output (currently: max 4) */
static size_t sanitize_char(char c, char *o, int style) {
   int hn;	/* high nibble */
   int ln;	/* low nibble */
   int n;	/* written chars */
   if (isprint(c)) {
      *o = c;
      return 1;
   }
   *o++ = '\\';
   n = 2;
   switch (c) {
   case '\0': *o++ = '0';  break;
   case '\a': *o++ = 'a';  break;
   case '\b': *o++ = 'b';  break;
   case '\t': *o++ = 't';  break;
   case '\n': *o++ = 'n';  break;
   case '\v': *o++ = 'v';  break;
   case '\f': *o++ = 'f';  break;
   case '\r': *o++ = 'r';  break;
   case '\'': *o++ = '\''; break;
   case '\"': *o++ = '"';  break;
   case '\\': *o++ = '\\'; break;
   default:
      *o++ = 'x';
      hn = (c>>4)&0x0f;
      ln = c&0x0f;
      *o++ = (hn>=10 ? (('A'-1)+(hn-10)) : ('0'+hn));
      *o++ = (ln>=10 ? (('A'-1)+(ln-10)) : ('0'+ln));
      n = 4;
   }
   return n;
}

/* sanitizes "untrusted" text, replacing special control characters with the C
   string version (eg."\n"), and replacing unprintable chars with hex
   representation ("\xAB").
   text can grow to four times of input, so keep output buffer long enough!
   returns a pointer to the first untouched byte of the output buffer.
   Output is not \0 terminated.
*/
char *sanitize_string(const char *data,	/* input data */
		   size_t bytes,	/* length of input data, >=0 */
		   char *coded,	/* output buffer, must be long enough */
		   int style
		   ) {
   int c;

   while (bytes > 0) {
      c = *(unsigned char *)data++;
      coded += sanitize_char(c, coded, style);
      --bytes;
   }
   return coded;
}

/* copies a substring out of a given buff
   returns scratch, \0 terminated; scratch must provide len+1 bytes
*/
char *xiosubstr(char *scratch, const char *str, size_t from, size_t len) {
   char *scratch0 = scratch;
   str += from;
   while (len--) {
      *scratch++ = *str++;
   }
   *scratch = '\0';
   return scratch0;
}
      

/* since version 1.7.2.4 socat supports C-99 behaviour of snprintf but still
   can handle the old glibc case with -1 return on truncation.
   Do not rely on exact return value in case of truncation
 */
int xio_snprintf(char *str, size_t size, const char *format, ...) {
   va_list ap;
   int result;

   va_start(ap, format);
   result = vsnprintf(str, size, format, ap);
#if ! HAVE_C99_SNPRINTF
   if (result < 0) {
      result = size+63;	/* indicate truncation with just some guess */
   }
#endif /* !HAVE_C99_SNPRINTF */
   va_end(ap);
   return result;
}
