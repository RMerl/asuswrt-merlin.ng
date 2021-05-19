/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file nickname.c
 * \brief Check and manipulate relay nicknames.
 */

#include "core/or/or.h"
#include "feature/nodelist/nickname.h"

/** Return true iff <b>s</b> is a valid server nickname. (That is, a string
 * containing between 1 and MAX_NICKNAME_LEN characters from
 * LEGAL_NICKNAME_CHARACTERS.) */
int
is_legal_nickname(const char *s)
{
  size_t len;
  tor_assert(s);
  len = strlen(s);
  return len > 0 && len <= MAX_NICKNAME_LEN &&
    strspn(s,LEGAL_NICKNAME_CHARACTERS) == len;
}

/** Return true iff <b>s</b> is a valid server nickname or
 * hex-encoded identity-key digest. */
int
is_legal_nickname_or_hexdigest(const char *s)
{
  if (*s!='$')
    return is_legal_nickname(s);
  else
    return is_legal_hexdigest(s);
}

/** Return true iff <b>s</b> is a valid hex-encoded identity-key
 * digest. (That is, an optional $, followed by 40 hex characters,
 * followed by either nothing, or = or ~ followed by a nickname, or
 * a character other than =, ~, or a hex character.)
 */
int
is_legal_hexdigest(const char *s)
{
  size_t len;
  tor_assert(s);
  if (s[0] == '$') s++;
  len = strlen(s);
  if (len > HEX_DIGEST_LEN) {
    if (s[HEX_DIGEST_LEN] == '=' ||
        s[HEX_DIGEST_LEN] == '~') {
      if (!is_legal_nickname(s+HEX_DIGEST_LEN+1))
        return 0;
    } else {
      return 0;
    }
  }
  return (len >= HEX_DIGEST_LEN &&
          strspn(s,HEX_CHARACTERS)==HEX_DIGEST_LEN);
}
