/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/
#include "test.h"

#include "testutil.h"
#include "timediff.h"
#include "warnless.h"
#include "memdebug.h"

int test(char *URL)
{
  CURL *ch = NULL;
  curl_global_init(CURL_GLOBAL_ALL);

  ch = curl_easy_init();
  if(!ch)
    goto cleanup;

  curl_easy_setopt(ch, CURLOPT_URL, URL);
  curl_easy_setopt(ch, CURLOPT_COOKIEFILE, "log/cookies1903");
  curl_easy_perform(ch);

  curl_easy_reset(ch);

  curl_easy_setopt(ch, CURLOPT_URL, URL);
  curl_easy_setopt(ch, CURLOPT_COOKIEFILE, "log/cookies1903");
  curl_easy_perform(ch);

  cleanup:
  curl_easy_cleanup(ch);
  curl_global_cleanup();

  return 0;
}
