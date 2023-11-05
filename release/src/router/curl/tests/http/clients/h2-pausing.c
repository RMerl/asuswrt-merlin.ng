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
/* <DESC>
 * HTTP/2 download pausing
 * </DESC>
 */
/* This is based on the poc client of issue #11982
 */
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <curl/mprintf.h>

#define HANDLECOUNT 2

static void log_line_start(FILE *log, const char *idsbuf, curl_infotype type)
{
  /*
   * This is the trace look that is similar to what libcurl makes on its
   * own.
   */
  static const char * const s_infotype[] = {
    "* ", "< ", "> ", "{ ", "} ", "{ ", "} "
  };
  if(idsbuf && *idsbuf)
    fprintf(log, "%s%s", idsbuf, s_infotype[type]);
  else
    fputs(s_infotype[type], log);
}

#define TRC_IDS_FORMAT_IDS_1  "[%" CURL_FORMAT_CURL_OFF_T "-x] "
#define TRC_IDS_FORMAT_IDS_2  "[%" CURL_FORMAT_CURL_OFF_T "-%" \
                                   CURL_FORMAT_CURL_OFF_T "] "
/*
** callback for CURLOPT_DEBUGFUNCTION
*/
static int debug_cb(CURL *handle, curl_infotype type,
                    char *data, size_t size,
                    void *userdata)
{
  FILE *output = stderr;
  static int newl = 0;
  static int traced_data = 0;
  char idsbuf[60];
  curl_off_t xfer_id, conn_id;

  (void)handle; /* not used */
  (void)userdata;

  if(!curl_easy_getinfo(handle, CURLINFO_XFER_ID, &xfer_id) && xfer_id >= 0) {
    if(!curl_easy_getinfo(handle, CURLINFO_CONN_ID, &conn_id) &&
        conn_id >= 0) {
      curl_msnprintf(idsbuf, sizeof(idsbuf), TRC_IDS_FORMAT_IDS_2,
                     xfer_id, conn_id);
    }
    else {
      curl_msnprintf(idsbuf, sizeof(idsbuf), TRC_IDS_FORMAT_IDS_1, xfer_id);
    }
  }
  else
    idsbuf[0] = 0;

  switch(type) {
  case CURLINFO_HEADER_OUT:
    if(size > 0) {
      size_t st = 0;
      size_t i;
      for(i = 0; i < size - 1; i++) {
        if(data[i] == '\n') { /* LF */
          if(!newl) {
            log_line_start(output, idsbuf, type);
          }
          (void)fwrite(data + st, i - st + 1, 1, output);
          st = i + 1;
          newl = 0;
        }
      }
      if(!newl)
        log_line_start(output, idsbuf, type);
      (void)fwrite(data + st, i - st + 1, 1, output);
    }
    newl = (size && (data[size - 1] != '\n')) ? 1 : 0;
    traced_data = 0;
    break;
  case CURLINFO_TEXT:
  case CURLINFO_HEADER_IN:
    if(!newl)
      log_line_start(output, idsbuf, type);
    (void)fwrite(data, size, 1, output);
    newl = (size && (data[size - 1] != '\n')) ? 1 : 0;
    traced_data = 0;
    break;
  case CURLINFO_DATA_OUT:
  case CURLINFO_DATA_IN:
  case CURLINFO_SSL_DATA_IN:
  case CURLINFO_SSL_DATA_OUT:
    if(!traced_data) {
      if(!newl)
        log_line_start(output, idsbuf, type);
      fprintf(output, "[%ld bytes data]\n", (long)size);
      newl = 0;
      traced_data = 1;
    }
    break;
  default: /* nada */
    newl = 0;
    traced_data = 1;
    break;
  }

  return 0;
}

static int err(void)
{
  fprintf(stderr, "something unexpected went wrong - bailing out!\n");
  exit(2);
}

struct handle
{
  int idx;
  int paused;
  int resumed;
  CURL *h;
};

static size_t cb(void *data, size_t size, size_t nmemb, void *clientp)
{
  size_t realsize = size * nmemb;
  struct handle *handle = (struct handle *) clientp;
  curl_off_t totalsize;

  (void)data;
  if(curl_easy_getinfo(handle->h, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                       &totalsize) == CURLE_OK)
    fprintf(stderr, "INFO: [%d] write, Content-Length %"CURL_FORMAT_CURL_OFF_T
            "\n", handle->idx, totalsize);

  if(!handle->resumed) {
    ++handle->paused;
    fprintf(stderr, "INFO: [%d] write, PAUSING %d time on %lu bytes\n",
            handle->idx, handle->paused, (long)realsize);
    return CURL_WRITEFUNC_PAUSE;
  }
  fprintf(stderr, "INFO: [%d] write, accepting %lu bytes\n",
          handle->idx, (long)realsize);
  return realsize;
}

int main(int argc, char *argv[])
{
  struct handle handles[HANDLECOUNT];
  CURLM *multi_handle;
  int i, still_running = 1, msgs_left, numfds;
  CURLMsg *msg;
  int rounds = 0;
  int rc = 0;
  CURLU *cu;
  struct curl_slist *resolve = NULL;
  char resolve_buf[1024];
  char *url, *host = NULL, *port = NULL;
  int all_paused = 0;
  int resume_round = -1;

  if(argc != 2) {
    fprintf(stderr, "ERROR: need URL as argument\n");
    return 2;
  }
  url = argv[1];

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl_global_trace("ids,time,http/2");

  cu = curl_url();
  if(!cu) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }
  if(curl_url_set(cu, CURLUPART_URL, url, 0)) {
    fprintf(stderr, "not a URL: '%s'\n", url);
    exit(1);
  }
  if(curl_url_get(cu, CURLUPART_HOST, &host, 0)) {
    fprintf(stderr, "could not get host of '%s'\n", url);
    exit(1);
  }
  if(curl_url_get(cu, CURLUPART_PORT, &port, 0)) {
    fprintf(stderr, "could not get port of '%s'\n", url);
    exit(1);
  }
  memset(&resolve, 0, sizeof(resolve));
  curl_msnprintf(resolve_buf, sizeof(resolve_buf)-1,
                 "%s:%s:127.0.0.1", host, port);
  resolve = curl_slist_append(resolve, resolve_buf);

  for(i = 0; i<HANDLECOUNT; i++) {
    handles[i].idx = i;
    handles[i].paused = 0;
    handles[i].resumed = 0;
    handles[i].h = curl_easy_init();
    if(!handles[i].h ||
      curl_easy_setopt(handles[i].h, CURLOPT_WRITEFUNCTION, cb) != CURLE_OK ||
      curl_easy_setopt(handles[i].h, CURLOPT_WRITEDATA, &handles[i])
        != CURLE_OK ||
      curl_easy_setopt(handles[i].h, CURLOPT_FOLLOWLOCATION, 1L) != CURLE_OK ||
      curl_easy_setopt(handles[i].h, CURLOPT_VERBOSE, 1L) != CURLE_OK ||
      curl_easy_setopt(handles[i].h, CURLOPT_DEBUGFUNCTION, debug_cb)
        != CURLE_OK ||
      curl_easy_setopt(handles[i].h, CURLOPT_SSL_VERIFYPEER, 0L) != CURLE_OK ||
      curl_easy_setopt(handles[i].h, CURLOPT_RESOLVE, resolve) != CURLE_OK ||
      curl_easy_setopt(handles[i].h, CURLOPT_URL, url) != CURLE_OK) {
      err();
    }
  }

  multi_handle = curl_multi_init();
  if(!multi_handle)
    err();

  for(i = 0; i<HANDLECOUNT; i++) {
    if(curl_multi_add_handle(multi_handle, handles[i].h) != CURLM_OK)
      err();
  }

  for(rounds = 0;; rounds++) {
    fprintf(stderr, "INFO: multi_perform round %d\n", rounds);
    if(curl_multi_perform(multi_handle, &still_running) != CURLM_OK)
      err();

    if(!still_running) {
      int as_expected = 1;
      fprintf(stderr, "INFO: no more handles running\n");
      for(i = 0; i<HANDLECOUNT; i++) {
        if(!handles[i].paused) {
          fprintf(stderr, "ERROR: [%d] NOT PAUSED\n", i);
          as_expected = 0;
        }
        else if(handles[i].paused != 1) {
          fprintf(stderr, "ERROR: [%d] PAUSED %d times!\n",
                  i, handles[i].paused);
          as_expected = 0;
        }
        else if(!handles[i].resumed) {
          fprintf(stderr, "ERROR: [%d] NOT resumed!\n", i);
          as_expected = 0;
        }
      }
      if(!as_expected) {
        fprintf(stderr, "ERROR: handles not in expected state "
                "after %d rounds\n", rounds);
        rc = 1;
      }
      break;
    }

    if(curl_multi_poll(multi_handle, NULL, 0, 100, &numfds) != CURLM_OK)
      err();

    while((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
      if(msg->msg == CURLMSG_DONE) {
        for(i = 0; i<HANDLECOUNT; i++) {
          if(msg->easy_handle == handles[i].h) {
            if(handles[i].paused != 1 || !handles[i].resumed) {
              fprintf(stderr, "ERROR: [%d] done, pauses=%d, resumed=%d, "
                      "result %d - wtf?\n", i, handles[i].paused,
                      handles[i].resumed, msg->data.result);
              rc = 1;
              goto out;
            }
          }
        }
      }
    }

    /* Successfully paused? */
    if(!all_paused) {
      for(i = 0; i<HANDLECOUNT; i++) {
        if(!handles[i].paused) {
          break;
        }
      }
      all_paused = (i == HANDLECOUNT);
      if(all_paused) {
        fprintf(stderr, "INFO: all transfers paused\n");
        /* give transfer some rounds to mess things up */
        resume_round = rounds + 3;
      }
    }
    if(resume_round > 0 && rounds == resume_round) {
      /* time to resume */
      for(i = 0; i<HANDLECOUNT; i++) {
        fprintf(stderr, "INFO: [%d] resumed\n", i);
        handles[i].resumed = 1;
        curl_easy_pause(handles[i].h, CURLPAUSE_CONT);
      }
    }
  }

out:
  for(i = 0; i<HANDLECOUNT; i++) {
    curl_multi_remove_handle(multi_handle, handles[i].h);
    curl_easy_cleanup(handles[i].h);
  }


  curl_slist_free_all(resolve);
  curl_free(host);
  curl_free(port);
  curl_url_cleanup(cu);
  curl_multi_cleanup(multi_handle);
  curl_global_cleanup();

  return rc;
}
