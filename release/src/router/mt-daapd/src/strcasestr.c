#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#if !HAVE_STRCASESTR
/* case-independent string matching, similar to strstr but
 * matching */
#if defined(WIFI7_SDK_20250506) || defined(WIFI8_SDK_20251126)
char * strcasestr(const char* haystack, const char* needle) {
#else
char * strcasestr(char* haystack, char* needle) {
#endif
  int i;
  int nlength = strlen (needle);
  int hlength = strlen (haystack);

  if (nlength > hlength) return NULL;
  if (hlength <= 0) return NULL;
  if (nlength <= 0) return haystack;
  /* hlength and nlength > 0, nlength <= hlength */
  for (i = 0; i <= (hlength - nlength); i++) {
    if (strncasecmp (haystack + i, needle, nlength) == 0) {
      return haystack + i;
    }
  }
  /* substring not found */
  return NULL;
}

#endif /* !HAVE_STRCASESTR */
