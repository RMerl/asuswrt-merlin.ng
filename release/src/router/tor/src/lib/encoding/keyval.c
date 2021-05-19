/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file keyval.c
 *
 * \brief Handle data encoded as a key=value pair.
 **/

#include "orconfig.h"
#include "lib/encoding/keyval.h"
#include "lib/log/escape.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"

#include <stdlib.h>
#include <string.h>

/** Return true if <b>string</b> is a valid 'key=[value]' string.
 *  "value" is optional, to indicate the empty string. Log at logging
 *  <b>severity</b> if something ugly happens. */
int
string_is_key_value(int severity, const char *string)
{
  /* position of equal sign in string */
  const char *equal_sign_pos = NULL;

  tor_assert(string);

  if (strlen(string) < 2) { /* "x=" is shortest args string */
    tor_log(severity, LD_GENERAL, "'%s' is too short to be a k=v value.",
            escaped(string));
    return 0;
  }

  equal_sign_pos = strchr(string, '=');
  if (!equal_sign_pos) {
    tor_log(severity, LD_GENERAL, "'%s' is not a k=v value.", escaped(string));
    return 0;
  }

  /* validate that the '=' is not in the beginning of the string. */
  if (equal_sign_pos == string) {
    tor_log(severity, LD_GENERAL, "'%s' is not a valid k=v value.",
            escaped(string));
    return 0;
  }

  return 1;
}
