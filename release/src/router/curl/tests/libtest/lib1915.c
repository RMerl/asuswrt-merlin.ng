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
#include "warnless.h"
#include "memdebug.h"

struct entry {
  const char *name;
  const char *exp;
};

static const struct entry preload_hosts[] = {
  /* curl turns 39 that day just before 31-bit time_t overflow */
  { "1.example.com", "20370320 01:02:03" },
  { "2.example.com", "20370320 03:02:01" },
  { "3.example.com", "20370319 01:02:03" },
  { "4.example.com", "" },
  { NULL, NULL } /* end of list marker */
};

struct state {
  int index;
};

/* "read" is from the point of the library, it wants data from us */
static CURLSTScode hstsread(CURL *easy, struct curl_hstsentry *e,
                            void *userp)
{
  const char *host;
  const char *expire;
  struct state *s = (struct state *)userp;
  (void)easy;
  host = preload_hosts[s->index].name;
  expire = preload_hosts[s->index++].exp;

  if(host && (strlen(host) < e->namelen)) {
    strcpy(e->name, host);
    e->includeSubDomains = FALSE;
    strcpy(e->expire, expire);
    fprintf(stderr, "add '%s'\n", host);
  }
  else
    return CURLSTS_DONE;
  return CURLSTS_OK;
}

/* verify error from callback */
static CURLSTScode hstsreadfail(CURL *easy, struct curl_hstsentry *e,
                                void *userp)
{
  (void)easy;
  (void)e;
  (void)userp;
  return CURLSTS_FAIL;
}

/* check that we get the hosts back in the save */
static CURLSTScode hstswrite(CURL *easy, struct curl_hstsentry *e,
                             struct curl_index *i, void *userp)
{
  (void)easy;
  (void)userp;
  printf("[%zu/%zu] %s %s\n", i->index, i->total, e->name, e->expire);
  return CURLSTS_OK;
}

/*
 * Read/write HSTS cache entries via callback.
 */

int test(char *URL)
{
  CURLcode ret = CURLE_OK;
  CURL *hnd;
  struct state st = {0};

  curl_global_init(CURL_GLOBAL_ALL);

  hnd = curl_easy_init();
  if(hnd) {
    curl_easy_setopt(hnd, CURLOPT_URL, URL);
    curl_easy_setopt(hnd, CURLOPT_HSTSREADFUNCTION, hstsread);
    curl_easy_setopt(hnd, CURLOPT_HSTSREADDATA, &st);
    curl_easy_setopt(hnd, CURLOPT_HSTSWRITEFUNCTION, hstswrite);
    curl_easy_setopt(hnd, CURLOPT_HSTSWRITEDATA, &st);
    curl_easy_setopt(hnd, CURLOPT_HSTS_CTRL, CURLHSTS_ENABLE);
    ret = curl_easy_perform(hnd);
    curl_easy_cleanup(hnd);
    printf("First request returned %d\n", (int)ret);
  }
  hnd = curl_easy_init();
  if(hnd) {
    curl_easy_setopt(hnd, CURLOPT_URL, URL);
    curl_easy_setopt(hnd, CURLOPT_HSTSREADFUNCTION, hstsreadfail);
    curl_easy_setopt(hnd, CURLOPT_HSTSREADDATA, &st);
    curl_easy_setopt(hnd, CURLOPT_HSTSWRITEFUNCTION, hstswrite);
    curl_easy_setopt(hnd, CURLOPT_HSTSWRITEDATA, &st);
    curl_easy_setopt(hnd, CURLOPT_HSTS_CTRL, CURLHSTS_ENABLE);
    ret = curl_easy_perform(hnd);
    curl_easy_cleanup(hnd);
    printf("Second request returned %d\n", (int)ret);
  }
  curl_global_cleanup();
  return (int)ret;
}
